/****************************************************************************
 **
 ** File Name: adm_kplo_ls_impl.h
 **
 ** Description: ls (list) implementation header file
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-04-07  jigi             initial implementation v1.0
 **
 ****************************************************************************/

#ifndef ADM_KPLO_LS_IMPL_H_
#define ADM_KPLO_LS_IMPL_H_

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


/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_kplo_ls_setup();
void dtn_kplo_ls_cleanup();


/* Metadata Functions */
tnv_t *dtn_kplo_ls_meta_name(tnvc_t *parms);
tnv_t *dtn_kplo_ls_meta_namespace(tnvc_t *parms);
tnv_t *dtn_kplo_ls_meta_version(tnvc_t *parms);
tnv_t *dtn_kplo_ls_meta_organization(tnvc_t *parms);

/* Constant Functions */

/* Collect Functions */

/* Control Functions */
tnv_t *dtn_kplo_ls_ctrl_list(eid_t *def_mgr, tnvc_t *parms, int8_t *status);


/* OP Functions */

/* Table Build Functions */

#endif //ADM_KPLO_LS_IMPL_H_
