/****************************************************************************
 **
 ** File Name: adm_ion_armur_impl.c
 **
 ** Description: implementation of the ADM for ARMUR.
 **
 ** Notes: AMP-rule-based remote software update framework
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-10-26  jigi             added EDD_RECORDS, CTRL_INIT.
 **  2020-08-12  jigi             initial integration of ARMUR to the nm module.
 **
 ****************************************************************************/

/*   START CUSTOM INCLUDES HERE  */
#include "armur.h"
#include "armurnm.h"
#include "cfdp.h"
#include "bputa.h"
#include "../mgr/nm_mgr.h"
#include "../shared/adm/adm_amp_agent.h"
#include "../shared/adm/adm_ion_armur.h"


/*   STOP CUSTOM INCLUDES HERE  */

#include "../shared/adm/adm.h"
#include "adm_ion_armur_impl.h"
#include "adm_amp_agent_impl.h"

/*   START CUSTOM FUNCTIONS HERE */
typedef struct {
	CfdpHandler		faultHandlers[16];
	CfdpNumber		destinationEntityNbr;
	char			sourceFileNameBuf[256];
	char			*sourceFileName;
	char			destFileNameBuf[256];
	char			*destFileName;
	BpUtParms		utParms;
	unsigned int		closureLatency;
	CfdpMetadataFn		segMetadataFn;
	MetadataList		msgsToUser;
	MetadataList		fsRequests;
	CfdpTransactionId	transactionId;
} CfdpReqParms;

typedef struct {
	ari_t	*id;
	uvast	start;
	expr_t	*state;
	uvast	max_eval;
	uvast	count;
	ac_t	*action;
	char	*description;
} SbrParms;

typedef struct {
	char	testString[16];
} ArmurParms;

static void populateCfdpParms(CfdpReqParms *cfdpReqParms, uvast destEntityNbr, char *archiveName)
{
	memset((char *)cfdpReqParms, 0, sizeof(CfdpReqParms));

	cfdp_compress_number(&(cfdpReqParms->destinationEntityNbr), destEntityNbr);
	isprintf(cfdpReqParms->sourceFileNameBuf, 256, "%.255s", archiveName);
	isprintf(cfdpReqParms->destFileNameBuf, 256, "%.255s", archiveName);
	cfdpReqParms->sourceFileName = cfdpReqParms->sourceFileNameBuf;
	cfdpReqParms->destFileName = cfdpReqParms->destFileNameBuf;

	/*	Temporarily use default values		*/
	cfdpReqParms->utParms.lifespan = 86400;
	cfdpReqParms->utParms.classOfService = BP_STD_PRIORITY;
	cfdpReqParms->utParms.custodySwitch = NoCustodyRequested;

	cfdpReqParms->msgsToUser = cfdp_create_usrmsg_list();
	cfdp_add_usrmsg(cfdpReqParms->msgsToUser, (unsigned char *)ARMUR_CFDP_USRMSG,
		strlen(ARMUR_CFDP_USRMSG) + 1);
}

static int buildSbrParms(ari_t *ampCtrlAddSbrId, SbrParms sbrParms)
{
	tnv_t *parms[7];
	int i;

	parms[0] = tnv_from_obj(AMP_TYPE_ARI, sbrParms.id);
	parms[1] = tnv_from_uvast(sbrParms.start);
	parms[2] = tnv_from_obj(AMP_TYPE_EXPR, sbrParms.state);
	parms[3] = tnv_from_uvast(sbrParms.max_eval);
	parms[4] = tnv_from_uvast(sbrParms.count);
	parms[5] = tnv_from_obj(AMP_TYPE_AC, sbrParms.action);
	parms[6] = tnv_from_obj(AMP_TYPE_STR, sbrParms.description);
	for (i = 0; i < 7; i++)
	{
		if (vec_push(&(ampCtrlAddSbrId->as_reg.parms.values), parms[i]) != VEC_OK)
		{
			for (; i >= 0; i--)
			{
				tnv_release(parms[i], 1);
			}
			return -1;
		}
	}
	return 0;
}

static int buildArmurParms(ari_t *armurCtrlStartId, ArmurParms armurParms)
{
	tnv_t *parms[1];
	int i;

	parms[0] = tnv_from_obj(AMP_TYPE_STR, armurParms.testString);
	for (i = 0; i < 1; i++)
	{
		if (vec_push(&(armurCtrlStartId->as_reg.parms.values), parms[i]) != VEC_OK)
		{
			for (; i >= 0; i--)
			{
				tnv_release(parms[i], 1);
			}
			return -1;
		}
	}
	return 0;
}

static int populateSbrParms(SbrParms *sbrParms, uvast sbrMaxEval)
{
	ari_t *armurCtrlStartId;
	ArmurParms armurParms;

	/*	Prepare armur_ctrl_start	*/
	armurCtrlStartId = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_START);

	armurParms.testString[0] = 'J';
	armurParms.testString[1] = '\0';
	//armurParms.reportToEid = tnvc_create(0);

	if (buildArmurParms(armurCtrlStartId, armurParms) < 0)
	{
		armurAppendRptMsg("Can't add ARMUR parms.", ARMUR_RPT_ERROR);
		ari_release(armurCtrlStartId, 1);
		return -1;
	}

	/*	SBR definition			*/
	sbrParms->id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_DOWNLOADED);
	sbrParms->start = 0;
	sbrParms->state = expr_create(AMP_TYPE_UINT);
	expr_add_item(sbrParms->state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_STATE));
	expr_add_item(sbrParms->state, adm_build_ari_lit_uint(ARMUR_STAT_DOWNLOADED));
	expr_add_item(sbrParms->state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_EQUAL));
	sbrParms->max_eval = sbrMaxEval;
	sbrParms->count = 1;
	sbrParms->action = ac_create();
	ac_insert(sbrParms->action, armurCtrlStartId);
	sbrParms->description = "downloaded";

	return 0;
}

/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_ion_armur_setup()
{

	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION setup BODY
	 * +-------------------------------------------------------------------------+
	 */

	 armurAttach();

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION setup BODY
	 * +-------------------------------------------------------------------------+
	 */
}

void dtn_ion_armur_cleanup()
{

	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION cleanup BODY
	 * +-------------------------------------------------------------------------+
	 */
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION cleanup BODY
	 * +-------------------------------------------------------------------------+
	 */
}


/* Metadata Functions */


tnv_t *dtn_ion_armur_meta_name(tnvc_t *parms)
{
	return tnv_from_str("armur");
}


tnv_t *dtn_ion_armur_meta_namespace(tnvc_t *parms)
{
	return tnv_from_str("DTN/ION/armur");
}


tnv_t *dtn_ion_armur_meta_version(tnvc_t *parms)
{
	return tnv_from_str("v1.0");
}


tnv_t *dtn_ion_armur_meta_organization(tnvc_t *parms)
{
	return tnv_from_str("HYUMNI");
}


/* Constant Functions */

/* Table Functions */


/* Collect Functions */
/*
 * The current state of ARMUR
 */
tnv_t *dtn_ion_armur_get_state(tnvc_t *parms)
{
	tnv_t *result = NULL;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION get_state BODY
	 * +-------------------------------------------------------------------------+
	 */
	NmArmurDB	snapshot;

	armurnm_state_get(&snapshot);
	result = tnv_from_uint(snapshot.currentStat);
	//result = tnv_from_byte(snapshot.currentStat);

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION get_state BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * New line (LF) delimited list of ARMUR log records in string
 */
tnv_t *dtn_ion_armur_get_records(tnvc_t *parms)
{
	tnv_t *result = NULL;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION get_records BODY
	 * +-------------------------------------------------------------------------+
	 */
	char records[512];
	char *ptrs[32];
	int num = 0;
	int i = 0;

	armurnm_record_get(records, 511, ptrs, &num);

	for (i = 0; i < num - 1; i++)
	{
		*(ptrs[i + 1] - sizeof(char)) = '\n';
	}

	result = tnv_from_str(records);

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION get_records BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* Control Functions */


/*
 * Automate the full ARMUR procedures with a few parameters in a single control.
 */
tnv_t *dtn_ion_armur_ctrl_init(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_init BODY
	 * +-------------------------------------------------------------------------+
	 */

	int success;
	CfdpReqParms cfdpReqParms;
	uvast destEntityNbr;
	SbrParms sbrParms;
	ari_t *ampCtrlAddSbrId;
	msg_ctrl_t *msg;
	char *remoteAgentEid = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);
	char *archiveName = adm_get_parm_obj(parms, 1, AMP_TYPE_STR);
	uvast sbrMaxEval = adm_get_parm_uvast(parms, 2, &success);

	printf("ctrl_init in>\n");//dbg

	/*	1. Send an ION package by CFDP.		*/
	if (cfdp_attach() < 0)
	{
		armurAppendRptMsg("cfdp_attach failed.", ARMUR_RPT_ERROR);
		return result;
	}

	if (sscanf(remoteAgentEid, "%*[^:]:" UVAST_FIELDSPEC ".", &destEntityNbr) != 1)
	{
		armurAppendRptMsg("invalid remote_agent_eid.", ARMUR_RPT_ERROR);
		return result;
	}

	/*	Populate CFDP parms	*/
	populateCfdpParms(&cfdpReqParms, destEntityNbr, archiveName);

	/*	CFDP put request	*/
	if (cfdp_put(&(cfdpReqParms.destinationEntityNbr),
			sizeof(BpUtParms),
			(unsigned char *)&(cfdpReqParms.utParms),
			cfdpReqParms.sourceFileName,
			cfdpReqParms.destFileName,
			NULL, NULL, NULL, 0, NULL, 0,
			cfdpReqParms.msgsToUser, 0,
			&(cfdpReqParms.transactionId)) < 0)
	{
		armurAppendRptMsg("cfdp_put failed.", ARMUR_RPT_ERROR);
		return result;
	}

	armurAppendRptMsg("cfdp_put successful.", ARMUR_RPT_SUCCESS);

	/*	2. Define SBRs with proper parms.	*/
	ampCtrlAddSbrId = adm_build_ari(AMP_TYPE_CTRL, 1, g_amp_agent_idx[ADM_CTRL_IDX], AMP_AGENT_CTRL_ADD_SBR);

	/*	Prepare SBR parms	*/
	if (populateSbrParms(&sbrParms, sbrMaxEval) < 0)
	{
		armurAppendRptMsg("Can't define SBR parms.", ARMUR_RPT_ERROR);
		return result;
	}

	/*	Build ARI & parms	*/
	if (buildSbrParms(ampCtrlAddSbrId, sbrParms) < 0)
	{
		armurAppendRptMsg("Can't add SBR parms.", ARMUR_RPT_ERROR);
		ari_release(ampCtrlAddSbrId, 1);
		return result;
	}

	/*	3. Create a message and send it.	*/
	/*	Create messasge		*/
	if ((msg = msg_ctrl_create_ari(ampCtrlAddSbrId)) == NULL)
	{
		armurAppendRptMsg("Can't create a message for add_sbr.", ARMUR_RPT_ERROR);
		ari_release(ampCtrlAddSbrId, 1);
		return result;
	}

	msg->start = 0;
	/*	Send messasge		*/
	if (iif_send_msg(&ion_ptr, MSG_TYPE_PERF_CTRL, msg, remoteAgentEid) < 0)
	{
		armurAppendRptMsg("Can't send add_sbr.", ARMUR_RPT_ERROR);
		msg_ctrl_release(msg, 1);
		return result;
	}

	msg_ctrl_release(msg, 1);

	/*	Init procedure has been completed.	*/
	*status = CTRL_SUCCESS;

	printf("ctrl_init ok<\n");//dbg
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_init BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * Extract the binary archive and install the images.
 */
tnv_t *dtn_ion_armur_ctrl_start(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_start BODY
	 * +-------------------------------------------------------------------------+
	 */
	Sdr		sdr = getIonsdr();
	ARMUR_VDB	*armurvdb = getArmurVdb();
	ARMUR_CfdpInfo	cfdpInfoBuf;
	char		archiveNameBuf[SDRSTRING_BUFSZ];

	char *testString = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);

	printf("ctrl_start in>\n");//dbg
	if (armurInstall() < 0)
	{
		AMP_DEBUG_ERR("dtn_ion_armur_ctrl_install", "ARMUR_INSTALL failed.", NULL);
		return result;
	}

	/*	Install has been finished.		*/

	/*	If we safely arrived here, the downloaded
	 *	archive file is no longer needed. Delete it.		*/
	CHKNULL(sdr_begin_xn(sdr));
	sdr_read(sdr, (char *)&cfdpInfoBuf, armurvdb->cfdpInfo, sizeof(ARMUR_CfdpInfo));
	sdr_string_read(sdr, archiveNameBuf, cfdpInfoBuf.archiveName);
	sdr_exit_xn(sdr);
	//if (fopen(buf, "r") != NULL)
	//{
		/*	If the file has been already removed, call to remove
		 *	will throw a file-not-exist error and then return.
		 *	So we skip the call to fopen to check if it exists.	*/
		remove(archiveNameBuf);
	//}

	/*	Start a restart program.		*/
	if (pseudoshell("armurrestart") < 0)
	{
		AMP_DEBUG_ERR("dtn_ion_armur_ctrl_restart", "ARMUR_RESTART failed.", NULL);
		return result;
	}

	printf("testString: %s.\n", testString);//dbg

	*status = CTRL_SUCCESS;
	
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_start BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * Extract the binary archive and install the images.
 */
tnv_t *dtn_ion_armur_ctrl_install(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_install BODY
	 * +-------------------------------------------------------------------------+
	 */

	printf("ctrl_install in>\n");//dbg
	if (armurInstall() < 0)
	{
		AMP_DEBUG_ERR("dtn_ion_armur_ctrl_install", "ARMUR_INSTALL failed.", NULL);
		return result;
	}

	/*	Install has been finished.		*/
	*status = CTRL_SUCCESS;
	
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_install BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * Restart daemon applications according to the images on the restart queues from ARMUR VDB.
 */
tnv_t *dtn_ion_armur_ctrl_restart(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_restart BODY
	 * +-------------------------------------------------------------------------+
	 */
	Sdr		sdr = getIonsdr();
	ARMUR_VDB	*armurvdb = getArmurVdb();
	ARMUR_CfdpInfo	cfdpInfoBuf;
	char		archiveNameBuf[SDRSTRING_BUFSZ];

	printf("ctrl_restart in>\n");//dbg

	/*	If we safely arrived here, the downloaded
	 *	archive file is no longer needed. Delete it.		*/
	CHKNULL(sdr_begin_xn(sdr));
	sdr_read(sdr, (char *)&cfdpInfoBuf, armurvdb->cfdpInfo, sizeof(ARMUR_CfdpInfo));
	sdr_string_read(sdr, archiveNameBuf, cfdpInfoBuf.archiveName);
	sdr_exit_xn(sdr);
	//if (fopen(buf, "r") != NULL)
	//{
		/*	If the file has been already removed, call to remove
		 *	will throw a file-not-exist error and then return.
		 *	So we skip the call to fopen to check if it exists.	*/
		remove(archiveNameBuf);
	//}

	/*	Start a restart program.		*/
	if (pseudoshell("armurrestart") < 0)
	{
		AMP_DEBUG_ERR("dtn_ion_armur_ctrl_restart", "ARMUR_RESTART failed.", NULL);
		return result;
	}

	*status = CTRL_SUCCESS;

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_restart BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * Generate a report indicating the result of the remote software update.
 */
tnv_t *dtn_ion_armur_ctrl_report(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_report BODY
	 * +-------------------------------------------------------------------------+
	 */

	result = amp_agent_ctrl_gen_rpts(def_mgr, parms, status);

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_report BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */

