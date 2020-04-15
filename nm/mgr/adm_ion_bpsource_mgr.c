/****************************************************************************
 **
 ** File Name: adm_ion_bpsource_mgr.c
 **
 ** Description: initialize bpsource data model on the Manager node.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-04-06  jigi             initial version v1.0 
 **
 ****************************************************************************/


#include "ion.h"
#include "platform.h"
#include "../shared/adm/adm_ion_bpsource.h"
#include "../shared/utils/utils.h"
#include "../shared/primitives/report.h"
#include "../shared/primitives/blob.h"
#include "metadata.h"
#include "nm_mgr_ui.h"




#define _HAVE_DTN_ION_BPSOURCE_ADM_
#ifdef _HAVE_DTN_ION_BPSOURCE_ADM_
vec_idx_t g_dtn_ion_bpsource_idx[11];

void dtn_ion_bpsource_init()
{
	adm_add_adm_info("dtn_ion_bpsource", ADM_ENUM_DTN_ION_BPSOURCE);

	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPSOURCE * 20) + ADM_META_IDX), &(g_dtn_ion_bpsource_idx[ADM_META_IDX]));
	VDB_ADD_NN(((ADM_ENUM_DTN_ION_BPSOURCE * 20) + ADM_CTRL_IDX), &(g_dtn_ion_bpsource_idx[ADM_CTRL_IDX]));


	dtn_ion_bpsource_init_meta();
	dtn_ion_bpsource_init_cnst();
	dtn_ion_bpsource_init_edd();
	dtn_ion_bpsource_init_op();
	dtn_ion_bpsource_init_var();
	dtn_ion_bpsource_init_ctrl();
	dtn_ion_bpsource_init_mac();
	dtn_ion_bpsource_init_rpttpl();
	dtn_ion_bpsource_init_tblt();
}

void dtn_ion_bpsource_init_meta()
{

	ari_t *id = NULL;

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpsource_idx[ADM_META_IDX], DTN_ION_BPSOURCE_META_NAME);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPSOURCE, "name", "The human-readable name of the ADM.");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpsource_idx[ADM_META_IDX], DTN_ION_BPSOURCE_META_NAMESPACE);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPSOURCE, "namespace", "The namespace of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpsource_idx[ADM_META_IDX], DTN_ION_BPSOURCE_META_VERSION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPSOURCE, "version", "The version of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_bpsource_idx[ADM_META_IDX], DTN_ION_BPSOURCE_META_ORGANIZATION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_BPSOURCE, "organization", "The name of the issuing organization of the ADM");

}

void dtn_ion_bpsource_init_cnst()
{

}

void dtn_ion_bpsource_init_edd()
{

}

void dtn_ion_bpsource_init_op()
{

}

void dtn_ion_bpsource_init_var()
{

}

void dtn_ion_bpsource_init_ctrl()
{

	ari_t *id = NULL;

	metadata_t *meta = NULL;


	/* BPSOURCE */

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ion_bpsource_idx[ADM_CTRL_IDX], DTN_ION_BPSOURCE_CTRL_BPSOURCE);
	adm_add_ctrldef_ari(id, 3, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_ION_BPSOURCE, "bpsource", "Use Bundle Protocol to send text to a counterpart bpsink application task that has opened the BP endpoint identified by destinationEndpointId.");

	meta_add_parm(meta, "destinationEndpointId", AMP_TYPE_STR);
	meta_add_parm(meta, "text", AMP_TYPE_STR);
	meta_add_parm(meta, "TTL", AMP_TYPE_UINT);
}

void dtn_ion_bpsource_init_mac()
{

}

void dtn_ion_bpsource_init_rpttpl()
{

}

void dtn_ion_bpsource_init_tblt()
{

}

#endif // _HAVE_DTN_ION_BPSOURCE_ADM_
