/****************************************************************************
 **
 ** File Name: adm_ion_armur_mgr.c
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
#include "metadata.h"
#include "nm_mgr_ui.h"




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


	dtn_ion_armur_init_meta();
	dtn_ion_armur_init_cnst();
	dtn_ion_armur_init_edd();
	dtn_ion_armur_init_op();
	dtn_ion_armur_init_var();
	dtn_ion_armur_init_ctrl();
	dtn_ion_armur_init_mac();
	dtn_ion_armur_init_rpttpl();
	dtn_ion_armur_init_tblt();
}

void dtn_ion_armur_init_meta()
{

	ari_t *id = NULL;

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_armur_idx[ADM_META_IDX], DTN_ION_ARMUR_META_NAME);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_ARMUR, "name", "The human-readable name of the ADM.");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_armur_idx[ADM_META_IDX], DTN_ION_ARMUR_META_NAMESPACE);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_ARMUR, "namespace", "The namespace of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_armur_idx[ADM_META_IDX], DTN_ION_ARMUR_META_VERSION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_ARMUR, "version", "The version of the ADM");

	id = adm_build_ari(AMP_TYPE_CNST, 0, g_dtn_ion_armur_idx[ADM_META_IDX], DTN_ION_ARMUR_META_ORGANIZATION);
	adm_add_cnst(id, NULL);
	meta_add_cnst(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_ARMUR, "organization", "The name of the issuing organization of the ADM");

}

void dtn_ion_armur_init_cnst()
{

}

void dtn_ion_armur_init_edd()
{

	ari_t *id = NULL;

	id = adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_STATE);
	adm_add_edd(id, NULL);
	meta_add_edd(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_ARMUR, "state", "The current state of ARMUR.");

	id = adm_build_ari(AMP_TYPE_EDD, 0, g_dtn_ion_armur_idx[ADM_EDD_IDX], DTN_ION_ARMUR_EDD_RECORDS);
	adm_add_edd(id, NULL);
	meta_add_edd(AMP_TYPE_STR, id, ADM_ENUM_DTN_ION_ARMUR, "records", "New line (LF) delimited list of ARMUR.");
}

void dtn_ion_armur_init_op()
{

}

void dtn_ion_armur_init_var()
{

}

void dtn_ion_armur_init_ctrl()
{

	ari_t *id = NULL;

	metadata_t *meta = NULL;


	/* INIT */

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_INIT);
	adm_add_ctrldef_ari(id, 4, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_ION_ARMUR, "init", "Automate the full ARMUR procedures with a few parameters in a single control.");
	meta_add_parm(meta, "remoteAgentEid", AMP_TYPE_STR);
	meta_add_parm(meta, "archiveName", AMP_TYPE_STR);
	meta_add_parm(meta, "sbrMaxEval", AMP_TYPE_UVAST);
	meta_add_parm(meta, "reportToEids", AMP_TYPE_TNVC);

	/* START */

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_START);
	adm_add_ctrldef_ari(id, 1, NULL);
	meta = meta_add_ctrl(id, ADM_ENUM_DTN_ION_ARMUR, "start", "Triggered by Manager, install the downloaded archive, restart the applicable daemon programs and activate the armur_sbr_fin to report the results.");
	meta_add_parm(meta, "reportToEids", AMP_TYPE_TNVC);

	/* INSTALL */

	id = adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_INSTALL);
	adm_add_ctrldef_ari(id, 0, NULL);
	meta_add_ctrl(id, ADM_ENUM_DTN_ION_ARMUR, "install", "Extract the binary archive and install the images.");

	/* RESTART */

	id = adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_RESTART);
	adm_add_ctrldef_ari(id, 0, NULL);
	meta_add_ctrl(id, ADM_ENUM_DTN_ION_ARMUR, "restart", "Restart daemon applications according to the images on the restart queues from ARMUR DB.");

	/* REPORT */

	id = adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_REPORT);
	adm_add_ctrldef_ari(id, 0, NULL);
	meta_add_ctrl(id, ADM_ENUM_DTN_ION_ARMUR, "report", "Generate a report indicating the result of the remote software update (amp_agent_ctrl_gen_rpts wrapper function)");

	/* FIN */

	id = adm_build_ari(AMP_TYPE_CTRL, 0, g_dtn_ion_armur_idx[ADM_CTRL_IDX], DTN_ION_ARMUR_CTRL_FIN);
	adm_add_ctrldef_ari(id, 0, NULL);
	meta_add_ctrl(id, ADM_ENUM_DTN_ION_ARMUR, "fin", "Do some postprocessing jobs for ARMUR DB/VDB.");
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

#endif // _HAVE_DTN_ION_ARMUR_ADM_
