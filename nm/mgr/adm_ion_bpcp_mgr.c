/****************************************************************************
 **
 ** File Name: adm_ion_bpcp_mgr.c
 **
 ** Description: initialize bpcp data model on the Manager node.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-03-31  jigi             initial version v1.0 
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_ion_bpcp.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "metadata.h"
#include "nm_mgr_ui.h"




#define _HAVE_DTN_ION_BPCP_ADM_
#ifdef _HAVE_DTN_ION_BPCP_ADM_
vec_idx_t g_dtn_ion_bpcp_idx[11];

void dtn_ion_bpcp_init()
{
	adm_add_adm_info("dtn_ion_bpcp", ADM_ENUM_DTN_ION_BPCP);

	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPCP * 20) + ADM_META_IDX), &(g_dtn_ion_bpcp_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPCP * 20) + ADM_EDD_IDX), &(g_dtn_ion_bpcp_idx[ADM_EDD_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPCP * 20) + ADM_CTRL_IDX), &(g_dtn_ion_bpcp_idx[ADM_CTRL_IDX]));


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

	ari_t *id = NULL;

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpcp_idx[ADM_META_IDX], DTN_ION_BPCP_META_NAME);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPCP, "name", "The human-readable name of the ADM.");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpcp_idx[ADM_META_IDX], DTN_ION_BPCP_META_NAMESPACE);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPCP, "namespace", "The namespace of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpcp_idx[ADM_META_IDX], DTN_ION_BPCP_META_VERSION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPCP, "version", "The version of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpcp_idx[ADM_META_IDX], DTN_ION_BPCP_META_ORGANIZATION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPCP, "organization", "The name of the issuing organization of the ADM");

}

void dtn_ion_bpcp_init_cnst()
{

}

void dtn_ion_bpcp_init_edd()
{

	ari_t *id = NULL;

	id = adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_bpcp_idx[ADM_EDD_IDX], DTN_ION_BPCP_EDD_BPCP_VERSION);
	adm_add_edd(id, NULL);
	meta_add_edd(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPCP, "bpcp_version", "Version of installed ION BPCP utility.");

}

void dtn_ion_bpcp_init_op()
{

}

void dtn_ion_bpcp_init_var()
{

}

void dtn_ion_bpcp_init_ctrl()
{

	ari_t *id = NULL;

	metadata_t *meta = NULL;


	/* BPCP_LOCAL_TO_REMOTE */

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ion_bpcp_idx[ADM_CTRL_IDX], DTN_ION_BPCP_CTRL_BPCP_LOCAL_TO_REMOTE);
	adm_add_ctrldef_ari(id, 6, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_ION_BPCP, "bpcp_local_to_remote", "Copy files between hosts utilizing NASA JPL's Interplanetary Overlay Network (ION) to provide a delay tolerant network. File copies from local to remote are executed.");

	meta_add_parm(meta, "bundle_lifetime", AMP_TYPE_UINT);
	meta_add_parm(meta, "bp_custody", AMP_TYPE_UINT);
	meta_add_parm(meta, "class_of_service", AMP_TYPE_UINT);
	meta_add_parm(meta, "local_file", AMP_TYPE_STR);
	meta_add_parm(meta, "remote_host", AMP_TYPE_UVAST);
	meta_add_parm(meta, "remote_file", AMP_TYPE_STR);
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
