/****************************************************************************
 **
 ** File Name: adm_ion_bpsource_impl.c
 **
 ** Description: implementation of AMM objects of the bpsource data model.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-04-06  jigi             initial implementation v1.0
 **
 ****************************************************************************/

/*   START CUSTOM INCLUDES HERE  */
#include "bp.h"

/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/adm/adm.h"
#include "adm_ion_bpsource_impl.h"


/*   START CUSTOM FUNCTIONS HERE */

static ReqAttendant *_attendant(ReqAttendant *newAttendant)
{
	static ReqAttendant *attendant = NULL;
	if (newAttendant)
	{
		attendant = newAttendant;
	}
	return attendant;
}

/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_ion_bpsource_setup()
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

void dtn_ion_bpsource_cleanup()
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


tnv_t *dtn_ion_bpsource_meta_name(tnvc_t *parms)
{
	return tnv_from_str("ion_bpsource");
}


tnv_t *dtn_ion_bpsource_meta_namespace(tnvc_t *parms)
{
	return tnv_from_str("DTN/ION/bpsource");
}


tnv_t *dtn_ion_bpsource_meta_version(tnvc_t *parms)
{
	return tnv_from_str("v1.0");
}


tnv_t *dtn_ion_bpsource_meta_organization(tnvc_t *parms)
{
	return tnv_from_str("HYUMNI");
}


/* Constant Functions */

/* Table Functions */

/* Collect Functions */

/* Control Functions */

/*
 * Use Bundle Protocol to send text to a counterpart bpsink application task that has opened
 * the BP endpoint identified by destinationEndpointId.
 */
tnv_t *dtn_ion_bpsource_ctrl_bpsource(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_bpsource BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;
	Sdr		sdr;
	int		lineLength;
	Object		extent;
	Object		bundleZco;
	ReqAttendant	attendant;
	Object		newBundle;

	char *destinationEndpointId = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);
	char *text = adm_get_parm_obj(parms, 1, AMP_TYPE_STR);
	int ttl = adm_get_parm_uint(parms, 2, &success);

	if (bp_attach() < 0)
	{
		return result;
	}

	if (ionStartAttendant(&attendant))
	{
		return result;
	}

	_attendant(&attendant);
	sdr = bp_get_sdr();

	if ((lineLength = strlen(text)) == 0)
	{
		return result;
	}

	CHKZERO(sdr_begin_xn(sdr));
	extent = sdr_malloc(sdr, lineLength);
	if (extent)
	{
		sdr_write(sdr, extent, text, lineLength);
	}

	if (sdr_end_xn(sdr) < 0)
	{
		return result;
	}

	bundleZco = ionCreateZco(ZcoSdrSource, extent, 0, lineLength,
			BP_STD_PRIORITY, 0, ZcoOutbound, &attendant);
	if (bundleZco == 0 || bundleZco == (Object) ERROR)
	{
		return result;
	}

	if (bp_send(NULL, destinationEndpointId, NULL, ttl,
		BP_STD_PRIORITY, NoCustodyRequested, 0, 0, NULL, bundleZco, &newBundle) >= 1)
	{
		*status = CTRL_SUCCESS;
	}

	ionStopAttendant(_attendant(NULL));

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_bpsource BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */

