/****************************************************************************
 **
 ** File Name: adm_kplo_ls_agent.c
 **
 ** Description: initialize ls (list) ADM on the remote Agent node.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-04-07  jigi             initialize ls (list) data model v1.0
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_kplo_ls.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "adm_kplo_ls_impl.h"
#include "rda.h"



#define _HAVE_DTN_KPLO_LS_ADM_
#ifdef _HAVE_DTN_KPLO_LS_ADM_

vec_idx_t g_dtn_kplo_ls_idx[11];

void dtn_kplo_ls_init()
{
	adm_add_adm_info("dtn_kplo_ls", ADM_ENUM_DTN_KPLO_LS);

	VDB_ADD_NN(((ADM_ENUM_DTN_KPLO_LS * 20) + ADM_META_IDX), &(g_dtn_kplo_ls_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_KPLO_LS * 20) + ADM_CTRL_IDX), &(g_dtn_kplo_ls_idx[ADM_CTRL_IDX]));


	dtn_kplo_ls_setup();
	dtn_kplo_ls_init_meta();
	dtn_kplo_ls_init_cnst();
	dtn_kplo_ls_init_edd();
	dtn_kplo_ls_init_op();
	dtn_kplo_ls_init_var();
	dtn_kplo_ls_init_ctrl();
	dtn_kplo_ls_init_mac();
	dtn_kplo_ls_init_rpttpl();
	dtn_kplo_ls_init_tblt();
}

void dtn_kplo_ls_init_meta()
{

	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_kplo_ls_idx[ADM_META_IDX], DTN_KPLO_LS_META_NAME), dtn_kplo_ls_meta_name);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_kplo_ls_idx[ADM_META_IDX], DTN_KPLO_LS_META_NAMESPACE), dtn_kplo_ls_meta_namespace);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_kplo_ls_idx[ADM_META_IDX], DTN_KPLO_LS_META_VERSION), dtn_kplo_ls_meta_version);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_kplo_ls_idx[ADM_META_IDX], DTN_KPLO_LS_META_ORGANIZATION), dtn_kplo_ls_meta_organization);
}

void dtn_kplo_ls_init_cnst()
{

}

void dtn_kplo_ls_init_edd()
{

}

void dtn_kplo_ls_init_op()
{

}

void dtn_kplo_ls_init_var()
{

}

void dtn_kplo_ls_init_ctrl()
{

	adm_add_ctrldef(g_dtn_kplo_ls_idx[ADM_CTRL_IDX], DTN_KPLO_LS_CTRL_LIST, 1, dtn_kplo_ls_ctrl_list);
}

void dtn_kplo_ls_init_mac()
{

}

void dtn_kplo_ls_init_rpttpl()
{

}

void dtn_kplo_ls_init_tblt()
{

}

#endif // _HAVE_DTN_KPLO_LS_ADM_
