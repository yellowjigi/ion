/****************************************************************************
 **
 ** File Name: adm_ion_bpcp_impl.c
 **
 ** Description: implementation of AMM objects of the bpcp data model.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-03-31  jigi             initial implementation v1.0
 **
 ****************************************************************************/

/*   START CUSTOM INCLUDES HERE  */
#include "cfdp.h"
#include "cfdpops.h"
#include "bputa.h"
#include "../../cfdp/utils/bpcp.h"

/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/adm/adm.h"
#include "adm_ion_bpcp_impl.h"


/*   START CUSTOM FUNCTIONS HERE */


/*             TODO              */
/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_ion_bpcp_setup()
{

	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION setup BODY
	 * +-------------------------------------------------------------------------+
	 */
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION setup BODY
	 * +-------------------------------------------------------------------------+
	 */
}

void dtn_ion_bpcp_cleanup()
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


tnv_t *dtn_ion_bpcp_meta_name(tnvc_t *parms)
{
	return tnv_from_str("ion_bpcp");
}


tnv_t *dtn_ion_bpcp_meta_namespace(tnvc_t *parms)
{
	return tnv_from_str("DTN/ION/bpcp");
}


tnv_t *dtn_ion_bpcp_meta_version(tnvc_t *parms)
{
	return tnv_from_str("v1.0");
}


tnv_t *dtn_ion_bpcp_meta_organization(tnvc_t *parms)
{
	return tnv_from_str("HYUMNI");
}


/* Constant Functions */
/* Table Functions */


/* Collect Functions */
/*
 * Version of installed ION BPCP utility.
 */
tnv_t *dtn_ion_bpcp_get_bpcp_version(tnvc_t *parms)
{
	tnv_t *result = NULL;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION get_bpcp_version BODY
	 * +-------------------------------------------------------------------------+
	 */
	char buffer[80];
	isprintf(buffer, sizeof buffer, "%s", BPCP_VERSION_STRING);
	result = tnv_from_str(buffer);

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION get_bpcp_version BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}



/* Control Functions */

/*
 * Copy files between hosts utilizing NASA JPL's Interplanetary Overlay Network (ION) to provide
 * a delay tolerant network. File copies from local to remote will be executed.
 */
tnv_t *dtn_ion_bpcp_ctrl_bpcp_local_to_remote(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_bpcp_local_to_remote BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;
	CfdpReqParms cfdpReqParms;
	BpCustodySwitch custodySwitch;
	int i;

	int bundle_lifetime = adm_get_parm_uint(parms, 0, &success);
	int bp_custody = adm_get_parm_uint(parms, 1, &success);
	int class_of_service = adm_get_parm_uint(parms, 2, &success);
	char *local_file = adm_get_parm_obj(parms, 3, AMP_TYPE_STR);
	uvast remote_host = adm_get_parm_uvast(parms, 4, &success);
	char *remote_file = adm_get_parm_obj(parms, 5, AMP_TYPE_STR);

	/* need to manage maximum string length of the buffer */
	/* if () */

	memset((char *)&cfdpReqParms, 0, sizeof(CfdpReqParms));

	if (bp_custody == 1)
	{
		custodySwitch = SourceCustodyRequired;
	}
	else
	{
		custodySwitch = NoCustodyRequested;
	}

	cfdpReqParms.utParms.lifespan = bundle_lifetime;
	cfdpReqParms.utParms.classOfService = class_of_service;
	cfdpReqParms.utParms.custodySwitch = custodySwitch;

	/* temporarily use default values as below,
	 * rather than solicit inputs from users	*/
	for (i = 0; i < 16; i++)
	{
		cfdpReqParms.faultHandlers[i] = CfdpIgnore;
	}
	cfdpReqParms.faultHandlers[CfdpFilestoreRejection] = CfdpIgnore;
	cfdpReqParms.faultHandlers[CfdpCheckLimitReached] = CfdpCancel;
	cfdpReqParms.faultHandlers[CfdpChecksumFailure] = CfdpCancel;
	cfdpReqParms.faultHandlers[CfdpInactivityDetected] = CfdpCancel;
	cfdpReqParms.faultHandlers[CfdpFileSizeError] = CfdpCancel;

	cfdp_compress_number(&cfdpReqParms.destinationEntityNbr, remote_host);
	snprintf(cfdpReqParms.sourceFileNameBuf, 255, "%.254s", local_file);
	snprintf(cfdpReqParms.destFileNameBuf, 255, "%.254s", remote_file);
	cfdpReqParms.sourceFileName = cfdpReqParms.sourceFileNameBuf;
	cfdpReqParms.destFileName = cfdpReqParms.destFileNameBuf;

	if (cfdp_attach() >= 0)
	{
		if (cfdp_put(&(cfdpReqParms.destinationEntityNbr),
					sizeof(BpUtParms),
					(unsigned char *)&(cfdpReqParms.utParms),
					cfdpReqParms.sourceFileName,
					cfdpReqParms.destFileName,
					NULL,
					NULL,
					cfdpReqParms.faultHandlers,
					0,
					NULL,
					0,
					cfdpReqParms.msgsToUser,
					cfdpReqParms.fsRequests,
					&(cfdpReqParms.transactionId)) >= 0)
		{
			*status = CTRL_SUCCESS;
		}
	}

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_bpcp_local_to_remote BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */

