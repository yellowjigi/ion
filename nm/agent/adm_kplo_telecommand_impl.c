/****************************************************************************
 **
 ** File Name: adm_kplo_telecommand_impl.c
 **
 ** Description: implementation of AMM objects of the telecommand data model.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-04-16  jigi             initial implementation v1.0
 **
 ****************************************************************************/

/*   START CUSTOM INCLUDES HERE  */
#include "bp.h"
#include "../shared/adm/adm_kplo_telecommand.h"
#include "../shared/primitives/tnv.h"
#include "../shared/msg/msg.h"
#include "rda.h"

/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/adm/adm.h"
#include "adm_kplo_telecommand_impl.h"


/*   START CUSTOM FUNCTIONS HERE */

#define CMDLEN_MAX		256
#define FILESIZE_MAX		65536

static int read_output(char *cmd, char *out)
{
	FILE *fp;
	char line[FILENAME_MAX] = { 0 };
	char *cursor;
	int offset;

	if ((fp = popen(cmd, "r")) == NULL)
	{
		fprintf(stderr, "cannot popen %s.\n", cmd);
		return -1;
	}

	cursor = out;
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		if (feof(fp) || ferror(fp))
		{
			fprintf(stderr, "error occured while reading lines from %s\n", cmd);
			pclose(fp);
			return -1;
		}

		offset = strlen(line);

		if (strlen(out) + offset > FILESIZE_MAX)
		{
			fprintf(stderr, "input data exceeded size limit\n");
			if (fclose(fp) == EOF)
			{
				fprintf(stderr, "cannot close %s\n", cmd);
				return -1;
			}
			return -1;
		}

		strncpy(cursor, line, offset);
		cursor += offset;
	}

	pclose(fp);

	return 0;
}

static void get_this_ari(ari_t **id)
{
	*id = adm_build_ari(AMP_TYPE_CTRL, 1,
			g_dtn_kplo_telecommand_idx[ADM_CTRL_IDX], DTN_KPLO_TELECOMMAND_CTRL);
}

static void gen_rpt(eid_t *def_mgr, ari_t *id, tnv_t *val)
{
	eid_t mgr_eid;
	tnv_t *mgr;
	msg_rpt_t *msg_rpt;
	rpt_t *rpt;
	
	mgr = tnv_from_str(def_mgr->name);
	strncpy(mgr_eid.name, mgr->value.as_ptr, AMP_MAX_EID_LEN - 1);

	msg_rpt = rda_get_msg_rpt(mgr_eid);
	rpt = rpt_create(ari_copy_ptr(id), getCtime(), NULL);

	rpt_add_entry(rpt, val);

	msg_rpt_add_rpt(msg_rpt, rpt);
}

/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_kplo_telecommand_setup()
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

void dtn_kplo_telecommand_cleanup()
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


tnv_t *dtn_kplo_telecommand_meta_name(tnvc_t *parms)
{
	return tnv_from_str("telecommand");
}


tnv_t *dtn_kplo_telecommand_meta_namespace(tnvc_t *parms)
{
	return tnv_from_str("DTN/KPLO/telecommand");
}


tnv_t *dtn_kplo_telecommand_meta_version(tnvc_t *parms)
{
	return tnv_from_str("v1.0");
}


tnv_t *dtn_kplo_telecommand_meta_organization(tnvc_t *parms)
{
	return tnv_from_str("HYUMNI");
}


/* Constant Functions */

/* Table Functions */

/* Collect Functions */

/* Control Functions */

/*
 * Execute local shell commands and report following output to Manager.
 */
tnv_t *dtn_kplo_telecommand_ctrl(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;
	char buf[FILESIZE_MAX];
	ari_t *id;
	vecit_t parm_it;

	char *cmd = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);

	if (read_output(cmd, buf) == -1)
	{
		fprintf(stderr, "cannot read the command output.\n");
		return result;
	}

	tnv_t *val = tnv_from_str(buf);
	get_this_ari(&id);

	for (parm_it = vecit_first(&(parms->values)); vecit_valid(parm_it); parm_it = vecit_next(parm_it))
	{
		tnv_t *cur_parm = vecit_data(parm_it);
		vec_push(&(id->as_reg.parms.values), cur_parm);
	}

	gen_rpt(def_mgr, id, val);

	*status = CTRL_SUCCESS;

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */

