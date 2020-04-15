/****************************************************************************
 **
 ** File Name: adm_ion_bpsource_impl.h
 **
 ** Description: bpsource implementation header file
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

#ifndef ADM_ION_BPSOURCE_IMPL_H_
#define ADM_ION_BPSOURCE_IMPL_H_

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

void dtn_ion_bpsource_setup();
void dtn_ion_bpsource_cleanup();


/* Metadata Functions */
tnv_t *dtn_ion_bpsource_meta_name(tnvc_t *parms);
tnv_t *dtn_ion_bpsource_meta_namespace(tnvc_t *parms);
tnv_t *dtn_ion_bpsource_meta_version(tnvc_t *parms);
tnv_t *dtn_ion_bpsource_meta_organization(tnvc_t *parms);

/* Constant Functions */

/* Collect Functions */

/* Control Functions */
tnv_t *dtn_ion_bpsource_ctrl_bpsource(eid_t *def_mgr, tnvc_t *parms, int8_t *status);


/* OP Functions */

/* Table Build Functions */

#endif //ADM_ION_BPSOURCE_IMPL_H_
