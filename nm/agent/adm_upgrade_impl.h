/****************************************************************************
 ** ### OBSOLETE ###
 ** File Name: adm_upgrade_impl.h
 **
 ** Description: upgrade implementation header file
 **
 ** Notes: simple remote upgrade test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-08-12  jigi             migrate to adm_ls_impl.h
 **  2020-05-18  jigi             integrate into adm_kplo_telecommand_impl
 **  2020-04-17  jigi             initial implementation v1.0
 **
 ****************************************************************************/

#ifndef ADM_UPGRADE_IMPL_H_
#define ADM_UPGRADE_IMPL_H_

/*   START CUSTOM INCLUDES HERE  */
/*             TODO              */
/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/utils/utils.h"
#include "../shared/primitives/ctrl.h"
#include "../shared/primitives/table.h"
#include "../shared/primitives/tnv.h"

/*   START typeENUM */
/*   STOP typeENUM  */

void name_adm_init_agent();



/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                            Retrieval Functions                                            +
 * +-----------------------------------------------------------------------------------------------------------+
 */
/*   START CUSTOM FUNCTIONS HERE */

#define MSGLEN_MAX	128
#define PATHLEN_MAX	32
#define CMDLEN_MAX	64

#define INSTALL		1
#define RESTART		2
#define RUN_CHECK	4

#define PARM_DEFAULT_ENC_SIZE	32

#define INFO_APPEND(format, ...) do { \
	_isprintf(gMsg + strlen(gMsg), MSGLEN_MAX - strlen(gMsg), format, __VA_ARGS__); \
	strcat(gMsg, "\n"); \
	} while (0)

int build_ari_auto(ari_t *id, tnvc_t *parms, char *agent_eid_name);

/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_upgrade_setup();
void dtn_upgrade_cleanup();


/* Metadata Functions */
tnv_t *dtn_upgrade_meta_name(tnvc_t *parms);
tnv_t *dtn_upgrade_meta_namespace(tnvc_t *parms);
tnv_t *dtn_upgrade_meta_version(tnvc_t *parms);
tnv_t *dtn_upgrade_meta_organization(tnvc_t *parms);

/* Constant Functions */

/* Collect Functions */

/* Control Functions */
tnv_t *dtn_upgrade_ctrl_install(eid_t *def_mgr, tnvc_t *parms, int8_t *status);
tnv_t *dtn_upgrade_ctrl_restart(eid_t *def_mgr, tnvc_t *parms, int8_t *status);

/* OP Functions */

/* Table Build Functions */

#endif //ADM_UPGRADE_IMPL_H_
