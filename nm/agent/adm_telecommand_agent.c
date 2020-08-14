/****************************************************************************
 **
 ** File Name: adm_telecommand_agent.c
 **
 ** Description: initialize telecommand ADM on the remote Agent node.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-08-12  jigi		  migrate to adm_telecommand_agent.c
 **  2020-05-18  jigi		  integrate with adm_kplo_ls_agent & adm_kplo_upgrade_agent
 **  2020-04-16  jigi             initialize telecommand data model v1.0
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_telecommand.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "adm_telecommand_impl.h"
#include "rda.h"



#define _HAVE_DTN_TELECOMMAND_ADM_
#ifdef _HAVE_DTN_TELECOMMAND_ADM_

vec_idx_t g_dtn_telecommand_idx[11];

void dtn_telecommand_init()
{
	adm_add_adm_info("dtn_telecommand", ADM_ENUM_DTN_TELECOMMAND);

	VDB_ADD_NN(((ADM_ENUM_DTN_TELECOMMAND * 20) + ADM_META_IDX), &(g_dtn_telecommand_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_TELECOMMAND * 20) + ADM_CTRL_IDX), &(g_dtn_telecommand_idx[ADM_CTRL_IDX]));


	dtn_telecommand_setup();
	dtn_telecommand_init_meta();
	dtn_telecommand_init_cnst();
	dtn_telecommand_init_edd();
	dtn_telecommand_init_op();
	dtn_telecommand_init_var();
	dtn_telecommand_init_ctrl();
	dtn_telecommand_init_mac();
	dtn_telecommand_init_rpttpl();
	dtn_telecommand_init_tblt();
}

void dtn_telecommand_init_meta()
{

	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_telecommand_idx[ADM_META_IDX], DTN_TELECOMMAND_META_NAME), dtn_telecommand_meta_name);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_telecommand_idx[ADM_META_IDX], DTN_TELECOMMAND_META_NAMESPACE), dtn_telecommand_meta_namespace);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_telecommand_idx[ADM_META_IDX], DTN_TELECOMMAND_META_VERSION), dtn_telecommand_meta_version);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_telecommand_idx[ADM_META_IDX], DTN_TELECOMMAND_META_ORGANIZATION), dtn_telecommand_meta_organization);
}

void dtn_telecommand_init_cnst()
{

}

void dtn_telecommand_init_edd()
{

}

void dtn_telecommand_init_op()
{

}

void dtn_telecommand_init_var()
{

}

void dtn_telecommand_init_ctrl()
{

	adm_add_ctrldef(g_dtn_telecommand_idx[ADM_CTRL_IDX], DTN_TELECOMMAND_CTRL_EXEC, 1, dtn_telecommand_ctrl_exec);
	adm_add_ctrldef(g_dtn_telecommand_idx[ADM_CTRL_IDX], DTN_TELECOMMAND_CTRL_KILL, 1, dtn_telecommand_ctrl_kill);
	adm_add_ctrldef(g_dtn_telecommand_idx[ADM_CTRL_IDX], DTN_TELECOMMAND_CTRL_UPDATE, 1, dtn_telecommand_ctrl_update);
}

void dtn_telecommand_init_mac()
{

}

void dtn_telecommand_init_rpttpl()
{

}

void dtn_telecommand_init_tblt()
{

}

#endif // _HAVE_DTN_TELECOMMAND_ADM_
