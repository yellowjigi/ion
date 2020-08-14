/****************************************************************************
 ** ### OBSOLETE ###
 ** File Name: adm_upgrade_impl.c
 **
 ** Description: implementation of AMM objects of the upgrade data model.
 **
 ** Notes: simple remote upgrade test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-08-12  jigi             migrate to adm_ls_impl.c
 **  2020-05-18  jigi             integrate into adm_kplo_telecommand_impl
 **  2020-04-17  jigi             initial implementation v1.0
 **
 ****************************************************************************/

/*   START CUSTOM INCLUDES HERE  */
#include "bp.h"
#include "../shared/msg/msg.h"
#include "../shared/msg/ion_if.h"
#include "rda.h"
#include "nmagent.h"
#include "../shared/adm/adm_upgrade.h"
#include "../../cfdp/utils/bpcp.h"
#include "adm_ion_bpcp_impl.h"
#include "adm_kplo_telecommand_impl.h"

/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/adm/adm.h"
#include "adm_upgrade_impl.h"


/*   START CUSTOM FUNCTIONS HERE */

char gMsg[MSGLEN_MAX];

int build_ctrl_auto(ari_t *id, tnvc_t *parms, char *agent_eid_name)
{
	vecit_t parm_it;
	tnv_t *cur_parm;
	uvast ts = 0;
	msg_ctrl_t *msg;
	int rtv;

	for (parm_it = vecit_first(&(parms->values)); vecit_valid(parm_it); parm_it = vecit_next(parm_it))
	{
		cur_parm = vecit_data(parm_it);
		if (vec_push(&(id->as_reg.parms.values), cur_parm) != VEC_OK)
		{
			INFO_APPEND("cannot add a parameter.", NULL);
			return -1;
		}
	}

	if ((msg = msg_ctrl_create_ari(id)) == NULL)
	{
		INFO_APPEND("cannot create a message.", NULL);
		ari_release(id, 1);
		return -1;
	}

	msg->start = ts;
	rtv = iif_send_msg(&ion_ptr, MSG_TYPE_PERF_CTRL, msg, agent_eid_name);
	msg_ctrl_release(msg, 1);

	return rtv;
}

static int get_full_path_name(char *application, char *install_path, char *full_path_out)
{
	int result;

	if (install_path[strlen(install_path) - 1] != '/')
	{
		strcat(install_path, "/");
	}

	result = snprintf(full_path_out, PATHLEN_MAX, "%s%s", install_path, application);

	if (result >= 0 && result < PATHLEN_MAX)
	{
		return 0;
	}
	else
	{
		INFO_APPEND("cannot get the full path name of %s.", application);
		return -1;
	}
}

static int run_nm_agent(char *full_path_name, eid_t *def_mgr)
{
	char cmd[CMDLEN_MAX];
	/* TODO: find available agent_eid */
	char *agent_eid = "ipn:22311.2";
	tnv_t *mgr;
	eid_t mgr_eid;
	pid_t pid;

	snprintf(cmd, CMDLEN_MAX, "%s %s %s", full_path_name, agent_eid, def_mgr->name);

	if ((pid = pseudoshell(cmd)) < 0)
	{
		INFO_APPEND("cannot start %s.", full_path_name);
		return -1;
	}

	return pid;
}

//static int is_running(char *application)
//{
//	if 
//	return 1;
//}

static int bin_restart(char *application, char *full_path_name, eid_t *def_mgr)
{
	if (strcmp(application, "nm_agent") == 0)
	{
		/* we will invoke another application-specific endpoint ID
		 * to start the new nm_agent program. */
		pid_t pid;

		if ((pid = run_nm_agent(full_path_name, def_mgr)) < 0)
		{
			return -1;
		}

		INFO_APPEND("restart ok, %s on ipn:22311.1 will be shut down.", application);

		return pid;
	}
	else
	{
		//if (is_running(application))
		//{
		//	stop_the_program(application);
		//}
		//start the installed program;
	}

	return -1;
}

void get_ari(ari_t **id, amp_type_e type, int ctrl_name)
{
	*id = adm_build_ari(type, 1, g_dtn_upgrade_idx[ADM_CTRL_IDX], ctrl_name);
}

static void gen_rpt(char *rx_mgr, ari_t *id, tnv_t *val)
{
	eid_t mgr_eid;
	msg_rpt_t *msg_rpt;
	rpt_t *rpt;
	
	strncpy(mgr_eid.name, rx_mgr, AMP_MAX_EID_LEN - 1);

	msg_rpt = rda_get_msg_rpt(mgr_eid);
	rpt = rpt_create(ari_copy_ptr(id), getCtime(), NULL);

	rpt_add_entry(rpt, val);

	msg_rpt_add_rpt(msg_rpt, rpt);
}

static void kill_myself(void)
{
	char cmd[16];
	pid_t pid = getpid();

	kill(pid, SIGINT);
}

/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_upgrade_setup()
{

	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION setup BODY
	 * +-------------------------------------------------------------------------+
	 */
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION setup BODY
	 * +-------------------------------------------------------------------------+
	 */
}

void dtn_upgrade_cleanup()
{

	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION cleanup BODY
	 * +-------------------------------------------------------------------------+
	 */
	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION cleanup BODY
	 * +-------------------------------------------------------------------------+
	 */
}


/* Metadata Functions */


tnv_t *dtn_upgrade_meta_name(tnvc_t *parms)
{
	return tnv_from_str("upgrade");
}


tnv_t *dtn_upgrade_meta_namespace(tnvc_t *parms)
{
	return tnv_from_str("DTN/KPLO/upgrade");
}


tnv_t *dtn_upgrade_meta_version(tnvc_t *parms)
{
	return tnv_from_str("v1.0");
}


tnv_t *dtn_upgrade_meta_organization(tnvc_t *parms)
{
	return tnv_from_str("HYUMNI");
}


/* Constant Functions */

/* Table Functions */

/* Collect Functions */

/* Control Functions */

/*
 * Install an ION application on the remote Agent node, using the bpcp util via a (local)
 * server Agent.
 */
tnv_t *dtn_upgrade_ctrl_install(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_install BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;

	char buf_msg[MSGLEN_MAX];
	char buf_path[PATHLEN_MAX];
	uvast remote_host;

	char *application = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);
	char *local_file = adm_get_parm_obj(parms, 1, AMP_TYPE_STR);
	char *remote_install_path = adm_get_parm_obj(parms, 2, AMP_TYPE_STR);
	char *remote_agent = adm_get_parm_obj(parms, 3, AMP_TYPE_STR);
	uint8_t *restart_option = adm_get_parm_obj(parms, 4, AMP_TYPE_BYTE);

	if (sscanf(remote_agent, "ipn:%lu", &remote_host) != 1)
	{
		INFO_APPEND("remote_agent in an invalid form.", NULL);
		return result;
	}

	if (get_full_path_name(application, remote_install_path, buf_path) == -1)
	{
		return result;
	}

	/* send the bin image to the remote Agent */
	if (cfdp_attach() >= 0)
	{
		if (cfdp_put_wrapper(86400, 0, 1, /* use default values */
					local_file, remote_host, application) < 0)
		{
			INFO_APPEND("install failed.", NULL);
			return result;
		}
		else
		{
			INFO_APPEND("install ok.", NULL);
		}
	}

	sleep(5);

	if (restart_option)
	{
		ari_t *id;
		tnvc_t *parms;

		id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_upgrade_idx[ADM_CTRL_IDX], DTN_UPGRADE_CTRL_RESTART);
		parms = tnvc_create(3);

		if (vec_push(&(parms->values), tnv_from_str(application)) != VEC_OK ||
			vec_push(&(parms->values), tnv_from_str(remote_install_path)) != VEC_OK ||
			vec_push(&(parms->values), tnv_from_str(def_mgr->name)) != VEC_OK)
		{
			INFO_APPEND("cannot construct tnvc of parameters.", NULL);
			return result;
		}

		if (build_ctrl_auto(id, parms, remote_agent) == -1)
		{
			return result;
		}
	}

	*status = CTRL_SUCCESS;

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_install BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * Restart an ION application on the Agent node.
 */
tnv_t *dtn_upgrade_ctrl_restart(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_restart BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;

	char buf_path[PATHLEN_MAX];
	pid_t pid;

	char *application = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);
	char *install_path = adm_get_parm_obj(parms, 1, AMP_TYPE_STR);
	char *rx_mgr_eid = adm_get_parm_obj(parms, 2, AMP_TYPE_STR);

	if (application == NULL || install_path == NULL)
	{
		INFO_APPEND("path corrupted.", NULL);
	}

	if (get_full_path_name(application, install_path, buf_path) == -1)
	{
		INFO_APPEND("path length may be too long.", NULL);
	}

	if ((pid = bin_restart(application, buf_path, def_mgr)) == -1)
	{
		INFO_APPEND("restart failed.", NULL);
	}
	else
	{
	}

	//if (command & RUN_CHECK)
	//{
	//	append_info(buf_msg, bin_run_check(application) ? "running" : "not running");
	//}

	ari_t *id;
	tnv_t *val;
	vecit_t parm_it;

	get_this_ari(&id, AMP_TYPE_CTRL, DTN_UPGRADE_CTRL_RESTART);
	val = tnv_from_str(buf_msg);

	for (parm_it = vecit_first(&(parms->values)); vecit_valid(parm_it); parm_it = vecit_next(parm_it))
	{
		tnv_t *cur_parm = vecit_data(parm_it);
		vec_push(&(id->as_reg.parms.values), cur_parm);
	}

	gen_rpt(rx_mgr_eid, id, val);

	if (pid != -1)
	{
		kill_myself();
	}

	*status = CTRL_SUCCESS;

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */

