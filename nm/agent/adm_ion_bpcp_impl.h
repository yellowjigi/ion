/****************************************************************************
 **
 ** File Name: adm_ion_bpcp_impl.h
 **
 ** Description: bpcp implementation header file
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

#ifndef ADM_ION_BPCP_IMPL_H_
#define ADM_ION_BPCP_IMPL_H_

/*   START CUSTOM INCLUDES HERE  */
/*             TODO              */
/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/utils/utils.h"
#include "../shared/primitives/ctrl.h"
#include "../shared/primitives/table.h"
#include "../shared/primitives/tnv.h"

/*   START typeENUM */
/*   STOP typeENUM  */

void name_adm_init_agent();



/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                            Retrieval Functions                                            +
 * +-----------------------------------------------------------------------------------------------------------+
 */
/*   START CUSTOM FUNCTIONS HERE */

int cfdp_put_wrapper(int bundle_lifetime, int bp_custody, int class_of_service,
			char *local_file, uvast remote_host, char *remote_file);

/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_ion_bpcp_setup();
void dtn_ion_bpcp_cleanup();


/* Metadata Functions */
tnv_t *dtn_ion_bpcp_meta_name(tnvc_t *parms);
tnv_t *dtn_ion_bpcp_meta_namespace(tnvc_t *parms);
tnv_t *dtn_ion_bpcp_meta_version(tnvc_t *parms);
tnv_t *dtn_ion_bpcp_meta_organization(tnvc_t *parms);

/* Constant Functions */

/* Collect Functions */
tnv_t *dtn_ion_bpcp_get_bpcp_version(tnvc_t *parms);


/* Control Functions */
tnv_t *dtn_ion_bpcp_ctrl_bpcp_local_to_remote(eid_t *def_mgr, tnvc_t *parms, int8_t *status);


/* OP Functions */

/* Table Build Functions */

#endif //ADM_ION_BPCP_IMPL_H_
