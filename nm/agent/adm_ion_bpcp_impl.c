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
#include "bpP.h"
#include "csi.h"
#include "../../cfdp/utils/bpcp.h"
#include "../../cfdp/utils/bpcp.c"

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
	char remote_path_buf[256];
	struct transfer t;
	int res;

	char *remote_host = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);
	char *local_file = adm_get_parm_obj(parms, 1, AMP_TYPE_STR);
	char *remote_file = adm_get_parm_obj(parms, 2, AMP_TYPE_STR);

	/* need to manage maximum string length of the buffer */
	/* if () */

	/* Initialize CFDP */
	ion_cfdp_init();

	if (cfdp_attach() >= 0)
	{
		t.type = Local_Remote;
		snprintf(t.sfile, 255, "%.254s", local_file);
		memset(t.shost, 0, 256);
		snprintf(t.dfile, 255, "%.254s", remote_file);
		snprintf(t.dhost, 255, "%.254s", remote_host);
		//manage_src(&t);

		if (0 == (res = ion_cfdp_put(t)))
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

