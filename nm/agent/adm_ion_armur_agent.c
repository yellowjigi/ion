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
 **  2021-05-16  Hoyeon Hwang     Added AMPSU_RA-specific code by macro.
 **  2020-10-26  jigi             added EDD_RECORDS, CTRL_INIT.
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

	adm_add_edd(adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_STATE), dtn_ion_armur_get_state);
	adm_add_edd(adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_RECORDS), dtn_ion_armur_get_records);
}

void dtn_ion_armur_init_op()
{

}

void dtn_ion_armur_init_var()
{

}

void dtn_ion_armur_init_ctrl()
{

	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_INIT, 4, dtn_ion_armur_ctrl_init);
	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_START, 1, dtn_ion_armur_ctrl_start);
	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_INSTALL, 0, dtn_ion_armur_ctrl_install);
	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_RESTART, 0, dtn_ion_armur_ctrl_restart);
	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_REPORT, 1, dtn_ion_armur_ctrl_report);
	adm_add_ctrldef(g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_FIN, 0, dtn_ion_armur_ctrl_fin);
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
#ifdef AMPSU_RA
	ari_t	*id = NULL;
	expr_t	*state = NULL;
	ac_t	*action = NULL;
	
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

	/* REPORT */
	ari_t	*armurCtrlReportId;
	tnvc_t	*reportToEids;
	tnv_t	*parm;

	id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_REPORT);
	state = expr_create(AMP_TYPE_UINT);
	/*	(current ARMUR state) & ARMUR_STAT_REPORT_PENDING	*/
	expr_add_item(state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_STATE));
	expr_add_item(state, adm_build_ari_lit_uint(4));
	expr_add_item(state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_BITAND));
	action = ac_create();
	armurCtrlReportId = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_REPORT);
	reportToEids = tnvc_create(0);
	parm = tnv_from_obj(AMP_TYPE_TNVC, reportToEids);
	if (vec_push(&(armurCtrlReportId->as_reg.parms.values), parm) != VEC_OK)
	{
		tnv_release(parm, 1);
		tnvc_release(reportToEids, 1);
		ari_release(armurCtrlReportId, 1);
	}
	else
	{
		ac_insert(action, armurCtrlReportId);
		if (adm_add_sbr(id, 0, state, 3, 1, action) == AMP_OK)
		{
			gAgentInstr.num_sbrs++;
		}
	}

	/* FIN */

	id = adm_build_ari(AMP_TYPE_SBR, 0, g_dtn_ion_armur_idx[ADM_SBR_IDX], DTN_ION_ARMUR_SBR_FIN);
	state = expr_create(AMP_TYPE_UINT);
	/*	(current ARMUR state) & ARMUR_STAT_FIN		*/
	expr_add_item(state, adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_STATE));
	expr_add_item(state, adm_build_ari_lit_uint(8));
	expr_add_item(state, adm_build_ari(AMP_TYPE_OPER, 1, g_amp_agent_idx[ADM_OPER_IDX], AMP_AGENT_OP_BITAND));
	action = ac_create();
	ac_insert(action, adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_FIN));
	if (adm_add_sbr(id, 0, state, 3, 1, action) == AMP_OK)
	{
		gAgentInstr.num_sbrs++;
	}
#endif //AMPSU_RA
}

#endif // _HAVE_DTN_ION_ARMUR_ADM_
