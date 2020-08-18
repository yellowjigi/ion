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
 **  2020-08-12  jigi             initial integration of ARMUR to the nm module.
 **
 ****************************************************************************/

/*   START CUSTOM INCLUDES HERE  */
#include "armur.h"
#include "armurnm.h"


/*   STOP CUSTOM INCLUDES HERE  */

#include "../shared/adm/adm.h"
#include "adm_ion_armur_impl.h"

/*   START CUSTOM FUNCTIONS HERE */
/*             TODO              */
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
tnv_t *dtn_ion_armur_get_armur_stat(tnvc_t *parms)
{
	tnv_t *result = NULL;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION get_armur_stat BODY
	 * +-------------------------------------------------------------------------+
	 */
	NmArmurDB	snapshot;

	armurnm_state_get(&snapshot);
	result = tnv_from_uint(snapshot.currentStat);
	//result = tnv_from_byte(snapshot.currentStat);

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION get_armur_stat BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* Control Functions */


/*
 * Trigger ARMUR and wait for the download to be finished.
 */
tnv_t *dtn_ion_armur_ctrl_wait(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_wait BODY
	 * +-------------------------------------------------------------------------+
	 */
	Sdr	sdr = getIonsdr();

	printf("ctrl_wait in>\n");//dbg

	if (cfdpAttach() < 0)
	{
		return result;
	}

	CHKNULL(sdr_begin_xn(sdr));
	if (sdr_list_first(sdr, (getCfdpConstants())->events) == 0)
	{
		/*	No CFDP PDUs have yet been received until now.
		 *	There is nothing to do and ARMUR is still idle.
		 *	To avoid deadlock, we will exit.			*/
		sdr_exit_xn(sdr);
		*status = CTRL_SUCCESS;
		//AMP_DEBUG_ALWAYS("dtn_ion_armur_ctrl_wait", "No CFDP events.", NULL);//dbg
		return result;
	}
	sdr_exit_xn(sdr);

	/*	Check CFDP events and block as necessary.		*/
	if (armurWait() < 0)
	{
		AMP_DEBUG_ERR("dtn_ion_armur_ctrl_wait", "ARMUR_WAIT failed.", NULL);
		return result;
	}

	/*	Download has been finished.		*/
	*status = CTRL_SUCCESS;
	
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_wait BODY
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

	//if (armurReport() == 0)
	//{
		*status = CTRL_SUCCESS;
	//}

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_report BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */
