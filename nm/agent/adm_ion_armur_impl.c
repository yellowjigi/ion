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
#include "instr.h"
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
} SbrParms;

typedef struct {
	tnvc_t *reportToEids;
} ArmurStartParms;

typedef struct {
	tnvc_t *rxmgrs;
} ArmurReportParms;

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

static int buildParms(tnvc_t *parms/*ari_t *id*/, tnv_t *parm[], int num)
{
	int i;

	for (i = 0; i < num; i++)
	{
		if (vec_push(&(parms->values), parm[i]) != VEC_OK)
		{
			for (; i >= 0; i--)
			{
				tnv_release(parm[i], 1);
			}
			return -1;
		}
	}
	return 0;
}

static int buildArmurSbrStartParms(ari_t *id, SbrParms sbrParms)
{
	tnv_t *parm[6];

	parm[0] = tnv_from_obj(AMP_TYPE_ARI, sbrParms.id);
	parm[1] = tnv_from_uvast(sbrParms.start);
	parm[2] = tnv_from_obj(AMP_TYPE_EXPR, sbrParms.state);
	parm[3] = tnv_from_uvast(sbrParms.max_eval);
	parm[4] = tnv_from_uvast(sbrParms.count);
	parm[5] = tnv_from_obj(AMP_TYPE_AC, sbrParms.action);

	return buildParms(&(id->as_reg.parms), parm, 6);
}

static int buildArmurStartParms(ari_t *id, ArmurStartParms armurStartParms)
{
	tnv_t *parm[1];

	parm[0] = tnv_from_obj(AMP_TYPE_TNVC, armurStartParms.reportToEids);

	return buildParms(&(id->as_reg.parms), parm, 1);
}

static int buildArmurReportParms(ari_t *id, ArmurReportParms armurReportParms)
{
	tnv_t *parm[1];

	parm[0] = tnv_from_obj(AMP_TYPE_TNVC, armurReportParms.rxmgrs);

	return buildParms(&(id->as_reg.parms), parm, 1);
}

static int populateArmurSbrStartParms(SbrParms *sbrParms, uvast sbrMaxEval, tnvc_t *reportToEids)
{
	ari_t *armurCtrlStartId;
	ArmurStartParms armurStartParms;

	/*	Prepare armur_ctrl_start	*/
	armurCtrlStartId = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_START);

	armurStartParms.reportToEids = reportToEids;

	if (buildArmurStartParms(armurCtrlStartId, armurStartParms) < 0)
	{
		armurAppendRptMsg("Can't add ARMUR parms.", ARMUR_RPT_ERROR);
		ari_release(armurCtrlStartId, 1);
		return -1;
	}

	/*	SBR definition			*/
	sbrParms->id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_START);
	sbrParms->start = 0;
	sbrParms->state = expr_create(AMP_TYPE_UINT);
	expr_add_item(sbrParms->state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_STATE));
	expr_add_item(sbrParms->state, adm_build_ari_lit_uint(ARMUR_STAT_DOWNLOADED));
	expr_add_item(sbrParms->state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_EQUAL));
	sbrParms->max_eval = sbrMaxEval;
	sbrParms->count = 1;
	sbrParms->action = ac_create();
	ac_insert(sbrParms->action, armurCtrlStartId);

	return 0;
}

static int populateArmurSbrReportParms(SbrParms *sbrParms, tnvc_t *reportToEids)
{
	ari_t *armurCtrlReportId;
	ArmurReportParms armurReportParms;

	/*	Prepare armur_ctrl_report	*/
	armurCtrlReportId = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_REPORT);

	armurReportParms.rxmgrs = reportToEids;

	if (buildArmurReportParms(armurCtrlReportId, armurReportParms) < 0)
	{
		armurAppendRptMsg("Can't add ARMUR parms.", ARMUR_RPT_ERROR);
		ari_release(armurCtrlReportId, 1);
		return -1;
	}

	/*	SBR definition			*/
	sbrParms->id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_REPORT);
	sbrParms->start = 0;
	sbrParms->state = expr_create(AMP_TYPE_UINT);
	expr_add_item(sbrParms->state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_STATE));
	expr_add_item(sbrParms->state, adm_build_ari_lit_uint(ARMUR_STAT_REPORT_PENDING));
	expr_add_item(sbrParms->state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_EQUAL));
	sbrParms->max_eval = 2; // Provide a margin for possible delay in restart procedure.
	sbrParms->count = 1;
	sbrParms->action = ac_create();
	ac_insert(sbrParms->action, armurCtrlReportId);

	return 0;
}

static int populateArmurSbrFinParms(SbrParms *sbrParms)
{
	ari_t *armurCtrlFinId;

	armurCtrlFinId = adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_FIN);

	/*	SBR definition			*/
	sbrParms->id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_FIN);
	sbrParms->start = 0;
	sbrParms->state = expr_create(AMP_TYPE_UINT);
	expr_add_item(sbrParms->state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_STATE));
	expr_add_item(sbrParms->state, adm_build_ari_lit_uint(ARMUR_STAT_FIN));
	expr_add_item(sbrParms->state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_EQUAL));
	sbrParms->max_eval = 2; // Provide a margin for possible delay in restart procedure.
	sbrParms->count = 1;
	sbrParms->action = ac_create();
	ac_insert(sbrParms->action, armurCtrlFinId);

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
	tnvc_t *reportToEids = adm_get_parm_obj(parms, 3, AMP_TYPE_TNVC);

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
	if (populateArmurSbrStartParms(&sbrParms, sbrMaxEval, reportToEids) < 0)
	{
		armurAppendRptMsg("Can't define SBR parms.", ARMUR_RPT_ERROR);
		return result;
	}

	/*	Build ARI & parms	*/
	if (buildArmurSbrStartParms(ampCtrlAddSbrId, sbrParms) < 0)
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
 * Triggered by Manager, install the downloaded archive, restart the applicable daemon programs and activate
 * the armur_sbr_fin to report the results.
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
	ARMUR_DB	armurdbBuf;
	Object		armurdbObj = getArmurDbObject();
	SbrParms	sbrParms;
	tnvc_t		*reportToEids = adm_get_parm_obj(parms, 0, AMP_TYPE_TNVC);

	printf("ctrl_start in>\n");//dbg

	/*	Activate armur_sbr_report.	*/
	if (populateArmurSbrReportParms(&sbrParms, reportToEids) < 0)
	{
		armurAppendRptMsg("Can't define SBR parms.", ARMUR_RPT_ERROR);
		return result;
	}

	if (adm_add_sbr(sbrParms.id, sbrParms.start, sbrParms.state, sbrParms.max_eval, sbrParms.count, sbrParms.action) != AMP_OK)
	{
		armurAppendRptMsg("adm_add_sbr failed.", ARMUR_RPT_ERROR);
		return result;
	}
	gAgentInstr.num_sbrs++;

	/*	Activate armur_sbr_fin.		*/
	if (populateArmurSbrFinParms(&sbrParms) < 0)
	{
		armurAppendRptMsg("Can't define SBR parms.", ARMUR_RPT_ERROR);
		oK(sdr_begin_xn(sdr));
		armurUpdateStat(ARMUR_STAT_REPORT_PENDING);
		oK(sdr_end_xn(sdr));
		return result;
	}

	if (adm_add_sbr(sbrParms.id, sbrParms.start, sbrParms.state, sbrParms.max_eval, sbrParms.count, sbrParms.action) != AMP_OK)
	{
		armurAppendRptMsg("adm_add_sbr failed.", ARMUR_RPT_ERROR);
		oK(sdr_begin_xn(sdr));
		armurUpdateStat(ARMUR_STAT_REPORT_PENDING);
		oK(sdr_end_xn(sdr));
		return result;
	}
	gAgentInstr.num_sbrs++;

	/*	Store the parameters		*/
	if (sdr_begin_xn(sdr) < 0)
	{
		armurAppendRptMsg("SDR transaction failed.", ARMUR_RPT_ERROR);
		oK(sdr_begin_xn(sdr));
		armurUpdateStat(ARMUR_STAT_REPORT_PENDING);
		oK(sdr_end_xn(sdr));
		return result;
	}
	sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
	if ((armurdbBuf.reportToEids = sdr_malloc(sdr, sizeof(tnvc_t))) == 0)
	{
		armurAppendRptMsg("Can't store parameters.", ARMUR_RPT_ERROR);
		oK(sdr_begin_xn(sdr));
		armurUpdateStat(ARMUR_STAT_REPORT_PENDING);
		oK(sdr_end_xn(sdr));
		return result;
	}

	sdr_write(sdr, armurdbBuf.reportToEids, (char *)reportToEids, sizeof(tnvc_t));
	sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));
	if (sdr_end_xn(sdr) < 0)
	{
		armurAppendRptMsg("SDR transaction failed.", ARMUR_RPT_ERROR);
		oK(sdr_begin_xn(sdr));
		armurUpdateStat(ARMUR_STAT_REPORT_PENDING);
		oK(sdr_end_xn(sdr));
		return result;
	}

	if (armurStart(NULL) < 0)
	{
		armurAppendRptMsg("armurStart failed.", ARMUR_RPT_ERROR);
		oK(sdr_begin_xn(sdr));
		armurUpdateStat(ARMUR_STAT_REPORT_PENDING);
		oK(sdr_end_xn(sdr));
		return result;
	}
	/*	Install/Restart procedures have been completed.	*/

	*status = CTRL_SUCCESS;

	printf("ctrl_start ok<\n");//dbg
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
 * Generate a report indicating the result of the remote software update
 * (amp_agent_ctrl_gen_rpts wrapper function).
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
	Sdr		sdr = getIonsdr();
	Object		armurdbObj = getArmurDbObject();
	ARMUR_DB	armurdb;
	tnvc_t		*armurReportParms;
	ac_t		*ids;
	tnv_t		*parm[2];
	tnvc_t		*reportToEids = adm_get_parm_obj(parms, 0, AMP_TYPE_TNVC);

	printf("ctrl_report in>\n");//dbg

	if (reportToEids == NULL)
	{
		/*	Retrieve the parameters.		*/
		if (sdr_begin_xn(sdr) < 0)
		{
			armurAppendRptMsg("SDR transaction failed.", ARMUR_RPT_ERROR);
			return result;
		}
		sdr_read(sdr, (char *)&armurdb, armurdbObj, sizeof(ARMUR_DB));
		if (armurdb.reportToEids)
		{
			if ((reportToEids = (tnvc_t *)STAKE(sizeof(tnvc_t))) == NULL)
			{
				armurAppendRptMsg("SDR transaction failed.", ARMUR_RPT_ERROR);
				return result;
			}
			sdr_read(sdr, (char *)reportToEids, armurdb.reportToEids, sizeof(tnvc_t));
		}
		else
		{
			//TODO: when the SDR is deleted and then restarted: send to default mgrs.
			sdr_exit_xn(sdr);
			printf("Can't find reportToEids.\n");
			return result;
		}
		sdr_exit_xn(sdr);
	}

	/*	Start report procedure.			*/

	/*	Prepare parms for gen_rpts.		*/
	armurReportParms = tnvc_create(2);
	ids = ac_create();
	ac_insert(ids, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_RECORDS));
	parm[0] = tnv_from_obj(AMP_TYPE_AC, ids);
	parm[1] = tnv_from_obj(AMP_TYPE_TNVC, reportToEids);
	if (buildParms(armurReportParms, parm, 2) < 0)
	{
		armurAppendRptMsg("Can't add parms for gen_rpts.", ARMUR_RPT_ERROR);
		tnvc_release(armurReportParms, 1);
		return result;
	}

	/*	Now call amp_agent_ctrl_gen_rpts.	*/
	result = amp_agent_ctrl_gen_rpts(def_mgr, armurReportParms, status);
	if (*status == CTRL_FAILURE)
	{
		armurAppendRptMsg("amp_agent_ctrl_gen_rpts failed.", ARMUR_RPT_ERROR);
		return result;
	}

	if (sdr_begin_xn(sdr) < 0)
	{
		*status = CTRL_FAILURE;
		armurAppendRptMsg("SDR transaction failed.", ARMUR_RPT_ERROR);
		return result;
	}
	armurUpdateStat(ARMUR_STAT_FIN);
	if (sdr_end_xn(sdr) < 0)
	{
		*status = CTRL_FAILURE;
		armurAppendRptMsg("SDR transaction failed.", ARMUR_RPT_ERROR);
		return result;
	}

	printf("ctrl_report ok<\n");//dbg
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_report BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * Do some postprocessing jobs for ARMUR DB/VDB.
 */
tnv_t *dtn_ion_armur_ctrl_fin(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_fin BODY
	 * +-------------------------------------------------------------------------+
	 */
	Sdr		sdr = getIonsdr();
	Object		armurdbObject = getArmurDbObject();
	ARMUR_DB	armurdb;
	Object		recordObj;
	Object		elt;

	printf("ctrl_fin in>\n");//dbg

	/*	Now the entire duty cycle of ARMUR processing has been
	 *	completed. Do post-processing of some remaining tasks.	*/
	if (sdr_begin_xn(sdr) < 0)
	{
		armurAppendRptMsg("SDR transaction failed.", ARMUR_RPT_ERROR);
		return result;
	}

	sdr_stage(sdr, (char *)&armurdb, armurdbObject, sizeof(ARMUR_DB));
	while ((elt = sdr_list_first(sdr, armurdb.records)) != 0)
	{
		recordObj = sdr_list_data(sdr, elt);
		sdr_free(sdr, recordObj);
		sdr_list_delete(sdr, elt, NULL, NULL);
	}

	sdr_free(sdr, armurdb.reportToEids);

	armurdb.stat = ARMUR_STAT_IDLE;
	sdr_write(sdr, armurdbObject, (char *)&armurdb, sizeof(ARMUR_DB));
	if (sdr_end_xn(sdr) < 0)
	{
		armurAppendRptMsg("SDR transaction failed.", ARMUR_RPT_ERROR);
		return result;
	}

	*status = CTRL_SUCCESS;

	printf("ctrl_fin ok<\n");//dbg
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_fin BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */
