/****************************************************************************
 **
 ** File Name: adm_ion_bpsource_agent.c
 **
 ** Description: initialize bpsource ADM on the remote Agent node.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-04-06  jigi             initialize bpsource data model v1.0
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_ion_bpsource.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "adm_ion_bpsource_impl.h"
#include "rda.h"



#define _HAVE_DTN_ION_BPSOURCE_ADM_
#ifdef _HAVE_DTN_ION_BPSOURCE_ADM_

vec_idx_t g_dtn_ion_bpsource_idx[11];

void dtn_ion_bpsource_init()
{
	adm_add_adm_info("dtn_ion_bpsource", ADM_ENUM_DTN_ION_BPSOURCE);

	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPSOURCE * 20) + ADM_META_IDX), &(g_dtn_ion_bpsource_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPSOURCE * 20) + ADM_CTRL_IDX), &(g_dtn_ion_bpsource_idx[ADM_CTRL_IDX]));


	dtn_ion_bpsource_setup();
	dtn_ion_bpsource_init_meta();
	dtn_ion_bpsource_init_cnst();
	dtn_ion_bpsource_init_edd();
	dtn_ion_bpsource_init_op();
	dtn_ion_bpsource_init_var();
	dtn_ion_bpsource_init_ctrl();
	dtn_ion_bpsource_init_mac();
	dtn_ion_bpsource_init_rpttpl();
	dtn_ion_bpsource_init_tblt();
}

void dtn_ion_bpsource_init_meta()
{

	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpsource_idx[ADM_META_IDX], DTN_ION_BPSOURCE_META_NAME), dtn_ion_bpsource_meta_name);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpsource_idx[ADM_META_IDX], DTN_ION_BPSOURCE_META_NAMESPACE), dtn_ion_bpsource_meta_namespace);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpsource_idx[ADM_META_IDX], DTN_ION_BPSOURCE_META_VERSION), dtn_ion_bpsource_meta_version);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpsource_idx[ADM_META_IDX], DTN_ION_BPSOURCE_META_ORGANIZATION), dtn_ion_bpsource_meta_organization);
}

void dtn_ion_bpsource_init_cnst()
{

}

void dtn_ion_bpsource_init_edd()
{

}

void dtn_ion_bpsource_init_op()
{

}

void dtn_ion_bpsource_init_var()
{

}

void dtn_ion_bpsource_init_ctrl()
{

	adm_add_ctrldef(g_dtn_ion_bpsource_idx[ADM_CTRL_IDX], DTN_ION_BPSOURCE_CTRL_BPSOURCE, 3, dtn_ion_bpsource_ctrl_bpsource);
}

void dtn_ion_bpsource_init_mac()
{

}

void dtn_ion_bpsource_init_rpttpl()
{

}

void dtn_ion_bpsource_init_tblt()
{

}

#endif // _HAVE_DTN_ION_BPSOURCE_ADM_
