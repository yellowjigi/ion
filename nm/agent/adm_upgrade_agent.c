/****************************************************************************
 ** ### OBSOLETE ###
 ** File Name: adm_upgrade_agent.c
 **
 ** Description: initialize upgrade ADM on the remote Agent node.
 **
 ** Notes: simple remote upgrade test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-08-12  jigi             migrate to adm_upgrade_agent.c
 **  2020-05-18  jigi		  integrate into adm_kplo_telecommand_agent
 **  2020-04-17  jigi             initialize upgrade data model v1.0
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_upgrade.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "adm_upgrade_impl.h"
#include "rda.h"



#define _HAVE_DTN_UPGRADE_ADM_
#ifdef _HAVE_DTN_UPGRADE_ADM_

vec_idx_t g_dtn_upgrade_idx[11];

void dtn_upgrade_init()
{
	adm_add_adm_info("dtn_upgrade", ADM_ENUM_DTN_UPGRADE);

	VDB_ADD_NN(((ADM_ENUM_DTN_UPGRADE * 20) + ADM_META_IDX), &(g_dtn_upgrade_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_UPGRADE * 20) + ADM_CTRL_IDX), &(g_dtn_upgrade_idx[ADM_CTRL_IDX]));


	dtn_upgrade_setup();
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

	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_upgrade_idx[ADM_META_IDX], DTN_UPGRADE_META_NAME), dtn_upgrade_meta_name);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_upgrade_idx[ADM_META_IDX], DTN_UPGRADE_META_NAMESPACE), dtn_upgrade_meta_namespace);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_upgrade_idx[ADM_META_IDX], DTN_UPGRADE_META_VERSION), dtn_upgrade_meta_version);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_upgrade_idx[ADM_META_IDX], DTN_UPGRADE_META_ORGANIZATION), dtn_upgrade_meta_organization);
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

	adm_add_ctrldef(g_dtn_upgrade_idx[ADM_CTRL_IDX], DTN_UPGRADE_CTRL_INSTALL, 1, dtn_upgrade_ctrl_install);
	adm_add_ctrldef(g_dtn_upgrade_idx[ADM_CTRL_IDX], DTN_UPGRADE_CTRL_RESTART, 1, dtn_upgrade_ctrl_restart);
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
