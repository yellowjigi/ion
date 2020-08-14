/****************************************************************************
 ** ### OBSOLETE ###
 ** File Name: adm_upgrade_mgr.c
 **
 ** Description: initialize upgrade data model on the Manager node.
 **
 ** Notes: simple remote upgrade test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-08-12  jigi             migrate to adm_upgrade_mgr.c
 **  2020-05-21  jigi		  integrate into adm_kplo_telecommand_mgr
 **  2020-04-17  jigi             initial version v1.0 
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_upgrade.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "metadata.h"
#include "nm_mgr_ui.h"




#define _HAVE_DTN_UPGRADE_ADM_
#ifdef _HAVE_DTN_UPGRADE_ADM_
vec_idx_t g_dtn_upgrade_idx[11];

void dtn_upgrade_init()
{
	adm_add_adm_info("dtn_upgrade", ADM_ENUM_DTN_UPGRADE);

	VDB_ADD_NN(((ADM_ENUM_DTN_UPGRADE * 20) + ADM_META_IDX), &(g_dtn_upgrade_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_UPGRADE * 20) + ADM_CTRL_IDX), &(g_dtn_upgrade_idx[ADM_CTRL_IDX]));


	dtn_upgrade_init_meta();
	dtn_upgrade_init_cnst();
	dtn_upgrade_init_edd();
	dtn_upgrade_init_op();
	dtn_upgrade_init_var();
	dtn_upgrade_init_ctrl();
	dtn_upgrade_init_mac();
	dtn_upgrade_init_rpttpl();
	dtn_upgrade_init_tblt();
}

void dtn_upgrade_init_meta()
{

	ari_t *id = NULL;

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_upgrade_idx[ADM_META_IDX], DTN_UPGRADE_META_NAME);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_UPGRADE, "name", "The human-readable name of the ADM.");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_upgrade_idx[ADM_META_IDX], DTN_UPGRADE_META_NAMESPACE);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_UPGRADE, "namespace", "The namespace of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_upgrade_idx[ADM_META_IDX], DTN_UPGRADE_META_VERSION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_UPGRADE, "version", "The version of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_upgrade_idx[ADM_META_IDX], DTN_UPGRADE_META_ORGANIZATION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_UPGRADE, "organization", "The name of the issuing organization of the ADM");

}

void dtn_upgrade_init_cnst()
{

}

void dtn_upgrade_init_edd()
{

}

void dtn_upgrade_init_op()
{

}

void dtn_upgrade_init_var()
{

}

void dtn_upgrade_init_ctrl()
{

	ari_t *id = NULL;

	metadata_t *meta = NULL;


	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_upgrade_idx[ADM_CTRL_IDX], DTN_UPGRADE_CTRL_INSTALL);
	adm_add_ctrldef_ari(id, 5, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_UPGRADE, "install", "Install an ION application on the remote Agent node, using the bpcp util via a (local) server Agent.");

	meta_add_parm(meta, "application", AMP_TYPE_STR);
	meta_add_parm(meta, "local_file", AMP_TYPE_STR);
	meta_add_parm(meta, "remote_install_path", AMP_TYPE_STR);
	meta_add_parm(meta, "remote_agent", AMP_TYPE_STR);
	meta_add_parm(meta, "restart_option", AMP_TYPE_BYTE);

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_upgrade_idx[ADM_CTRL_IDX], DTN_UPGRADE_CTRL_RESTART);
	adm_add_ctrldef_ari(id, 3, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_UPGRADE, "restart", "Restart an ION application on the remote Agent node.");

	meta_add_parm(meta, "application", AMP_TYPE_STR);
	meta_add_parm(meta, "install_path", AMP_TYPE_STR);
	meta_add_parm(meta, "rx_mgr_eid", AMP_TYPE_STR);
}

void dtn_upgrade_init_mac()
{

}

void dtn_upgrade_init_rpttpl()
{

}

void dtn_upgrade_init_tblt()
{

}

#endif // _HAVE_DTN_UPGRADE_ADM_
