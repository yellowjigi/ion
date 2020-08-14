/****************************************************************************
 ** ### OBSOLETE ###
 ** File Name: adm_ls_mgr.c
 **
 ** Description: initialize ls (list) data model on the Manager node.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-08-12  jigi             migrate to adm_ls_mgr.c
 **  2020-05-21  jigi		  integrate into adm_kplo_telecommand_mgr
 **  2020-04-07  jigi             initial version v1.0 
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_ls.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "metadata.h"
#include "nm_mgr_ui.h"




#define _HAVE_DTN_LS_ADM_
#ifdef _HAVE_DTN_LS_ADM_
vec_idx_t g_dtn_ls_idx[11];

void dtn_ls_init()
{
	adm_add_adm_info("dtn_ls", ADM_ENUM_DTN_LS);

	VDB_ADD_NN(((ADM_ENUM_DTN_LS * 20) + ADM_META_IDX), &(g_dtn_ls_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_LS * 20) + ADM_CTRL_IDX), &(g_dtn_ls_idx[ADM_CTRL_IDX]));


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

	ari_t *id = NULL;

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ls_idx[ADM_META_IDX], DTN_LS_META_NAME);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_LS, "name", "The human-readable name of the ADM.");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ls_idx[ADM_META_IDX], DTN_LS_META_NAMESPACE);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_LS, "namespace", "The namespace of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ls_idx[ADM_META_IDX], DTN_LS_META_VERSION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_LS, "version", "The version of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ls_idx[ADM_META_IDX], DTN_LS_META_ORGANIZATION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_LS, "organization", "The name of the issuing organization of the ADM");

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

	ari_t *id = NULL;

	metadata_t *meta = NULL;


	/* LIST */

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ls_idx[ADM_CTRL_IDX], DTN_LS_CTRL_LIST);
	adm_add_ctrldef_ari(id, 1, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_LS, "list", "List information about the files of a given directory.");

	meta_add_parm(meta, "directory_path", AMP_TYPE_STR);
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
