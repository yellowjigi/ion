/****************************************************************************
 ** ### OBSOLETE ###
 ** File Name: adm_ls_agent.c
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
 **  2020-08-12  jigi             migrate to adm_ls_agent.c
 **  2020-05-18  jigi             integrate into adm_kplo_telecommand_agent
 **  2020-04-07  jigi             initialize ls (list) data model v1.0
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_ls.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "adm_ls_impl.h"
#include "rda.h"



#define _HAVE_DTN_LS_ADM_
#ifdef _HAVE_DTN_LS_ADM_

vec_idx_t g_dtn_ls_idx[11];

void dtn_ls_init()
{
	adm_add_adm_info("dtn_ls", ADM_ENUM_DTN_LS);

	VDB_ADD_NN(((ADM_ENUM_DTN_LS * 20) + ADM_META_IDX), &(g_dtn_ls_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_LS * 20) + ADM_CTRL_IDX), &(g_dtn_ls_idx[ADM_CTRL_IDX]));


	dtn_ls_setup();
	dtn_ls_init_meta();
	dtn_ls_init_cnst();
	dtn_ls_init_edd();
	dtn_ls_init_op();
	dtn_ls_init_var();
	dtn_ls_init_ctrl();
	dtn_ls_init_mac();
	dtn_ls_init_rpttpl();
	dtn_ls_init_tblt();
}

void dtn_ls_init_meta()
{

	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ls_idx[ADM_META_IDX], DTN_LS_META_NAME), dtn_ls_meta_name);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ls_idx[ADM_META_IDX], DTN_LS_META_NAMESPACE), dtn_ls_meta_namespace);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ls_idx[ADM_META_IDX], DTN_LS_META_VERSION), dtn_ls_meta_version);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ls_idx[ADM_META_IDX], DTN_LS_META_ORGANIZATION), dtn_ls_meta_organization);
}

void dtn_ls_init_cnst()
{

}

void dtn_ls_init_edd()
{

}

void dtn_ls_init_op()
{

}

void dtn_ls_init_var()
{

}

void dtn_ls_init_ctrl()
{

	adm_add_ctrldef(g_dtn_ls_idx[ADM_CTRL_IDX], DTN_LS_CTRL_LIST, 1, dtn_ls_ctrl_list);
}

void dtn_ls_init_mac()
{

}

void dtn_ls_init_rpttpl()
{

}

void dtn_ls_init_tblt()
{

}

#endif // _HAVE_DTN_LS_ADM_
