/****************************************************************************
 **
 ** File Name: adm_ion_armur_agent.c
 **
 ** Description: implementation of the ADM for ARMUR.
 **
 ** Notes: AMP-rule-based remote software update framework
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-10-26  jigi             added EDD_ARMUR_RECORDS.
 **  2020-08-12  jigi             initial integration of ARMUR to the nm module.
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_ion_armur.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "adm_ion_armur_impl.h"
#include "rda.h"

#include "../shared/adm/adm_amp_agent.h"
#include "instr.h"


#define _HAVE_DTN_ION_ARMUR_ADM_
#ifdef _HAVE_DTN_ION_ARMUR_ADM_

vec_idx_t g_dtn_ion_armur_idx[11];

void dtn_ion_armur_init()
{
	adm_add_adm_info("dtn_ion_armur", ADM_ENUM_DTN_ION_ARMUR);

	VDB_ADD_NN(((ADM_ENUM_DTN_ION_ARMUR * 20) + ADM_META_IDX), &(g_dtn_ion_armur_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_ARMUR * 20) + ADM_EDD_IDX), &(g_dtn_ion_armur_idx[ADM_EDD_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_ARMUR * 20) + ADM_CTRL_IDX), &(g_dtn_ion_armur_idx[ADM_CTRL_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_ARMUR * 20) + ADM_SBR_IDX), &(g_dtn_ion_armur_idx[ADM_SBR_IDX]));


	dtn_ion_armur_setup();
	dtn_ion_armur_init_meta();
	dtn_ion_armur_init_cnst();
	dtn_ion_armur_init_edd();
	dtn_ion_armur_init_op();
	dtn_ion_armur_init_var();
	dtn_ion_armur_init_ctrl();
	dtn_ion_armur_init_mac();
	dtn_ion_armur_init_rpttpl();
	dtn_ion_armur_init_tblt();
	dtn_ion_armur_init_sbr();
}

void dtn_ion_armur_init_meta()
{

	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_armur_idx[ADM_META_IDX], DTN_ION_ARMUR_META_NAME), dtn_ion_armur_meta_name);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_armur_idx[ADM_META_IDX], DTN_ION_ARMUR_META_NAMESPACE), dtn_ion_armur_meta_namespace);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_armur_idx[ADM_META_IDX], DTN_ION_ARMUR_META_VERSION), dtn_ion_armur_meta_version);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_armur_idx[ADM_META_IDX], DTN_ION_ARMUR_META_ORGANIZATION), dtn_ion_armur_meta_organization);
}

void dtn_ion_armur_init_cnst()
{

}

void dtn_ion_armur_init_edd()
{

	adm_add_edd(adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_ARMUR_STAT), dtn_ion_armur_get_armur_stat);
	adm_add_edd(adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_ARMUR_RECORDS), dtn_ion_armur_get_armur_records);
}

void dtn_ion_armur_init_op()
{

}

void dtn_ion_armur_init_var()
{

}

void dtn_ion_armur_init_ctrl()
{

	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_INSTALL, 0, dtn_ion_armur_ctrl_install);
	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_RESTART, 0, dtn_ion_armur_ctrl_restart);
	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_REPORT, 2, dtn_ion_armur_ctrl_report);
}

void dtn_ion_armur_init_mac()
{

}

void dtn_ion_armur_init_rpttpl()
{

}

void dtn_ion_armur_init_tblt()
{

}

void dtn_ion_armur_init_sbr()
{
	//ari_t	*id = NULL;
	//expr_t	*state = NULL;
	//ac_t	*action = NULL;
	
	/* IDLE */

	//id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_IDLE);
	//state = expr_create(AMP_TYPE_UINT);
	//expr_add_item(state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_ARMUR_STAT));
	//expr_add_item(state, adm_build_ari_lit_uint(0));
	//expr_add_item(state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_EQUAL));
	//action = ac_create();
	//ac_insert(action, adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_WAIT));
	//if (adm_add_sbr(id, 0, state, 1000, 1, action) == AMP_OK)
	//{
	//	gAgentInstr.num_sbrs++;
	//}

	/* DOWNLOADED */

	//id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_DOWNLOADED);
	//state = expr_create(AMP_TYPE_UINT);
	//expr_add_item(state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_ARMUR_STAT));
	//expr_add_item(state, adm_build_ari_lit_uint(1));
	//expr_add_item(state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_EQUAL));
	//action = ac_create();
	//ac_insert(action, adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_INSTALL));
	//if (adm_add_sbr(id, 0, state, 1000, 1, action) == AMP_OK)
	//{
	//	gAgentInstr.num_sbrs++;
	//}

	///* INSTALLED */

	//id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_INSTALLED);
	//state = expr_create(AMP_TYPE_UINT);
	//expr_add_item(state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_ARMUR_STAT));
	//expr_add_item(state, adm_build_ari_lit_uint(2));
	//expr_add_item(state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_EQUAL));
	//action = ac_create();
	//ac_insert(action, adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_RESTART));
	//if (adm_add_sbr(id, 0, state, 1000, 1, action) == AMP_OK)
	//{
	//	gAgentInstr.num_sbrs++;
	//}
}

#endif // _HAVE_DTN_ION_ARMUR_ADM_
