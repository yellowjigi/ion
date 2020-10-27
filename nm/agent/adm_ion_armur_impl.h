/****************************************************************************
 **
 ** File Name: adm_ion_armur_impl.h
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

#ifndef ADM_ION_ARMUR_IMPL_H_
#define ADM_ION_ARMUR_IMPL_H_

/*   START CUSTOM INCLUDES HERE  */
/*             TODO              */
/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/utils/utils.h"
#include "../shared/primitives/ctrl.h"
#include "../shared/primitives/table.h"
#include "../shared/primitives/tnv.h"

/*   START typeENUM */
/*             TODO              */
/*   STOP typeENUM  */

void name_adm_init_agent();



/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                            Retrieval Functions                                            +
 * +-----------------------------------------------------------------------------------------------------------+
 */
/*   START CUSTOM FUNCTIONS HERE */
/*             TODO              */
/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_ion_armur_setup();
void dtn_ion_armur_cleanup();


/* Metadata Functions */
tnv_t *dtn_ion_armur_meta_name(tnvc_t *parms);
tnv_t *dtn_ion_armur_meta_namespace(tnvc_t *parms);
tnv_t *dtn_ion_armur_meta_version(tnvc_t *parms);
tnv_t *dtn_ion_armur_meta_organization(tnvc_t *parms);

/* Constant Functions */

/* Collect Functions */
tnv_t *dtn_ion_armur_get_state(tnvc_t *parms);
tnv_t *dtn_ion_armur_get_records(tnvc_t *parms);

/* Control Functions */
tnv_t *dtn_ion_armur_ctrl_init(eid_t *def_mgr, tnvc_t *parms, int8_t *status);
tnv_t *dtn_ion_armur_ctrl_start(eid_t *def_mgr, tnvc_t *parms, int8_t *status);
tnv_t *dtn_ion_armur_ctrl_install(eid_t *def_mgr, tnvc_t *parms, int8_t *status);
tnv_t *dtn_ion_armur_ctrl_restart(eid_t *def_mgr, tnvc_t *parms, int8_t *status);
tnv_t *dtn_ion_armur_ctrl_report(eid_t *def_mgr, tnvc_t *parms, int8_t *status);

/* OP Functions */

/* Table Build Functions */

#endif //ADM_ION_ARMUR_IMPL_H_
