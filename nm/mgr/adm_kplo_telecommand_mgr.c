/****************************************************************************
 **
 ** File Name: adm_kplo_telecommand_mgr.c
 **
 ** Description: initialize telecommand data model on the Manager node.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-05-21  jigi		  integrated with adm_kplo_ls_mgr & adm_kplo_upgrade_mgr
 **  2020-04-16  jigi             initial version v1.0 
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_kplo_telecommand.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "metadata.h"
#include "nm_mgr_ui.h"




#define _HAVE_DTN_KPLO_TELECOMMAND_ADM_
#ifdef _HAVE_DTN_KPLO_TELECOMMAND_ADM_
vec_idx_t g_dtn_kplo_telecommand_idx[11];

void dtn_kplo_telecommand_init()
{
	adm_add_adm_info("dtn_kplo_telecommand", ADM_ENUM_DTN_KPLO_TELECOMMAND);

	VDB_ADD_NN(((ADM_ENUM_DTN_KPLO_TELECOMMAND * 20) + ADM_META_IDX), &(g_dtn_kplo_telecommand_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_KPLO_TELECOMMAND * 20) + ADM_CTRL_IDX), &(g_dtn_kplo_telecommand_idx[ADM_CTRL_IDX]));


	dtn_kplo_telecommand_init_meta();
	dtn_kplo_telecommand_init_cnst();
	dtn_kplo_telecommand_init_edd();
	dtn_kplo_telecommand_init_op();
	dtn_kplo_telecommand_init_var();
	dtn_kplo_telecommand_init_ctrl();
	dtn_kplo_telecommand_init_mac();
	dtn_kplo_telecommand_init_rpttpl();
	dtn_kplo_telecommand_init_tblt();
}

void dtn_kplo_telecommand_init_meta()
{

	ari_t *id = NULL;

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_kplo_telecommand_idx[ADM_META_IDX], DTN_KPLO_TELECOMMAND_META_NAME);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_KPLO_TELECOMMAND, "name", "The human-readable name of the ADM.");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_kplo_telecommand_idx[ADM_META_IDX], DTN_KPLO_TELECOMMAND_META_NAMESPACE);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_KPLO_TELECOMMAND, "namespace", "The namespace of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_kplo_telecommand_idx[ADM_META_IDX], DTN_KPLO_TELECOMMAND_META_VERSION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_KPLO_TELECOMMAND, "version", "The version of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_kplo_telecommand_idx[ADM_META_IDX], DTN_KPLO_TELECOMMAND_META_ORGANIZATION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_KPLO_TELECOMMAND, "organization", "The name of the issuing organization of the ADM");

}

void dtn_kplo_telecommand_init_cnst()
{

}

void dtn_kplo_telecommand_init_edd()
{

}

void dtn_kplo_telecommand_init_op()
{

}

void dtn_kplo_telecommand_init_var()
{

}

void dtn_kplo_telecommand_init_ctrl()
{

	ari_t *id = NULL;

	metadata_t *meta = NULL;


	/* EXEC */

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_kplo_telecommand_idx[ADM_CTRL_IDX], DTN_KPLO_TELECOMMAND_CTRL_EXEC);
	adm_add_ctrldef_ari(id, 1, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_KPLO_TELECOMMAND, "exec", "Execute a local command.");

	meta_add_parm(meta, "cmd", AMP_TYPE_STR);

	/* KILL */

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_kplo_telecommand_idx[ADM_CTRL_IDX], DTN_KPLO_TELECOMMAND_CTRL_KILL);
	adm_add_ctrldef_ari(id, 2, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_KPLO_TELECOMMAND, "kill", "Terminate (or send a corresponding signal to) a process.");

	meta_add_parm(meta, "application", AMP_TYPE_STR);
	meta_add_parm(meta, "signum", AMP_TYPE_UINT);

	/* UPDATE */

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_kplo_telecommand_idx[ADM_CTRL_IDX], DTN_KPLO_TELECOMMAND_CTRL_UPDATE);
	adm_add_ctrldef_ari(id, 4, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_KPLO_TELECOMMAND, "update", "Install given binary archives using CFDP via a (local) file server Agent and restart any applications that need to be updated.");

	meta_add_parm(meta, "application_name", AMP_TYPE_STR);
	meta_add_parm(meta, "file_server_eid", AMP_TYPE_UVAST);
	meta_add_parm(meta, "file_server_file", AMP_TYPE_STR);
	meta_add_parm(meta, "install_path", AMP_TYPE_STR);
}

void dtn_kplo_telecommand_init_mac()
{

}

void dtn_kplo_telecommand_init_rpttpl()
{

}

void dtn_kplo_telecommand_init_tblt()
{

}

#endif // _HAVE_DTN_KPLO_TELECOMMAND_ADM_
