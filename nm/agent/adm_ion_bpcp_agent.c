/****************************************************************************
 **
 ** File Name: adm_ion_bpcp_agent.c
 **
 ** Description: initialize bpcp ADM on the remote Agent node.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-03-31  jigi             initialize bpcp data model v1.0
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_ion_bpcp.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "adm_ion_bpcp_impl.h"
#include "rda.h"



#define _HAVE_DTN_ION_BPCP_ADM_
#ifdef _HAVE_DTN_ION_BPCP_ADM_

vec_idx_t g_dtn_ion_bpcp_idx[11];

void dtn_ion_bpcp_init()
{
	adm_add_adm_info("dtn_ion_bpcp", ADM_ENUM_DTN_ION_BPCP);

	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPCP * 20) + ADM_META_IDX), &(g_dtn_ion_bpcp_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPCP * 20) + ADM_EDD_IDX), &(g_dtn_ion_bpcp_idx[ADM_EDD_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPCP * 20) + ADM_CTRL_IDX), &(g_dtn_ion_bpcp_idx[ADM_CTRL_IDX]));


	dtn_ion_bpcp_setup();
	dtn_ion_bpcp_init_meta();
	dtn_ion_bpcp_init_cnst();
	dtn_ion_bpcp_init_edd();
	dtn_ion_bpcp_init_op();
	dtn_ion_bpcp_init_var();
	dtn_ion_bpcp_init_ctrl();
	dtn_ion_bpcp_init_mac();
	dtn_ion_bpcp_init_rpttpl();
	dtn_ion_bpcp_init_tblt();
}

void dtn_ion_bpcp_init_meta()
{

	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpcp_idx[ADM_META_IDX], DTN_ION_BPCP_META_NAME), dtn_ion_bpcp_meta_name);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpcp_idx[ADM_META_IDX], DTN_ION_BPCP_META_NAMESPACE), dtn_ion_bpcp_meta_namespace);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpcp_idx[ADM_META_IDX], DTN_ION_BPCP_META_VERSION), dtn_ion_bpcp_meta_version);
	adm_add_cnst(adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpcp_idx[ADM_META_IDX], DTN_ION_BPCP_META_ORGANIZATION), dtn_ion_bpcp_meta_organization);
}

void dtn_ion_bpcp_init_cnst()
{

}

void dtn_ion_bpcp_init_edd()
{

	adm_add_edd(adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_bpcp_idx[ADM_EDD_IDX], DTN_ION_BPCP_EDD_BPCP_VERSION), dtn_ion_bpcp_get_bpcp_version);
}

void dtn_ion_bpcp_init_op()
{

}

void dtn_ion_bpcp_init_var()
{

}

void dtn_ion_bpcp_init_ctrl()
{

	adm_add_ctrldef(g_dtn_ion_bpcp_idx[ADM_CTRL_IDX], DTN_ION_BPCP_CTRL_BPCP_LOCAL_TO_REMOTE, 6, dtn_ion_bpcp_ctrl_bpcp_local_to_remote);
}

void dtn_ion_bpcp_init_mac()
{

}

void dtn_ion_bpcp_init_rpttpl()
{

}

void dtn_ion_bpcp_init_tblt()
{

}

#endif // _HAVE_DTN_ION_BPCP_ADM_
