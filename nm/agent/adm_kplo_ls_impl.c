/****************************************************************************
 **
 ** File Name: adm_kplo_ls_impl.c
 **
 ** Description: implementation of AMM objects of the ls (list) data model.
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-04-07  jigi             initial implementation v1.0
 **
 ****************************************************************************/

/*   START CUSTOM INCLUDES HERE  */
#include "bp.h"
#include "../shared/adm/adm_kplo_ls.h"
#include "../shared/primitives/tnv.h"
#include "../shared/msg/msg.h"
#include "rda.h"

/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/adm/adm.h"
#include "adm_kplo_ls_impl.h"


/*   START CUSTOM FUNCTIONS HERE */

#define CMDLEN_MAX		256
#define FILESIZE_MAX		65536

//static int stdout_set(int *fd_back)
//{
//	if ((*fd_back = dup(STDOUT_FILENO)) == -1)
//	{
//		AMP_DEBUG_ERR("stdout_set", "cannot back up fd", NULL);
//		return -1;
//	}
//
//	return 0;
//}
//
//static int redirect_output(char *cmd, char *filename)
//{
//	int fd;
//
//	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
//	{
//		AMP_DEBUG_ERR("redirect_output", "cannot open a file", NULL);
//		return -1;
//	}
//
//	if (dup2(fd, STDOUT_FILENO) == -1)
//	{
//		AMP_DEBUG_ERR("redirect_output", "cannot dup fd", NULL);
//		return -1;
//	}
//
//	if (close(fd) == -1)
//	{
//		AMP_DEBUG_ERR("redirect_output", "cannot close fd", NULL);
//		return -1;
//	}
//
//	pseudoshell(cmd);
//
//	return 0;
//}
//
//static int stdout_reset(int *fd_back)
//{
//	if (dup2(*fd_back, STDOUT_FILENO) == -1)
//	{
//		AMP_DEBUG_ERR("stdout_reset", "cannot dup fd back", NULL);
//		return -1;
//	}
//
//	if (close(*fd_back) == -1)
//	{
//		AMP_DEBUG_ERR("stdout_reset", "cannot close fd", NULL);
//		return -1;
//	}
//
//	return 0;
//}

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
			//fprintf(stderr, "error occured while reading lines from %s\n",
			//		filename);
			if (fclose(fp) == EOF)
			{
				return -1;
				//fprintf(stderr, "cannot close %s\n", filename);
				//exit(EXIT_FAILURE);
			}
			return -1;
			//exit(EXIT_FAILURE);
		}

		offset = strlen(line);

		if (strlen(out) + offset > FILESIZE_MAX)
		{
			//fprintf(stderr, "input data exceeded size limit\n");
			if (fclose(fp) == EOF)
			{
				return -1;
				//fprintf(stderr, "cannot close %s\n", filename);
				//exit(EXIT_FAILURE);
			}
			return -1;
			//exit(EXIT_FAILURE);
		}

		strncpy(cursor, line, offset);
		cursor += offset;
	}
	*(cursor - 1) = '\0';

	if (pclose(fp) == -1)
	{
		fprintf(stderr, "cannot pclose %s.\n", cmd);
		return -1;
	}

	return 0;
}

static void get_this_ari(ari_t **id)
{
	*id = adm_build_ari(AMP_TYPE_CTRL, 1,
			g_dtn_kplo_ls_idx[ADM_CTRL_IDX], DTN_KPLO_LS_CTRL_LIST);
}

static void gen_rpt(eid_t *def_mgr, ari_t *id, char *contents)
{
	eid_t mgr_eid;
	tnv_t *mgr;
	msg_rpt_t *msg_rpt;
	rpt_t *rpt;
	tnv_t *val;
	
	mgr = tnv_from_str(def_mgr->name);
	strncpy(mgr_eid.name, mgr->value.as_ptr, AMP_MAX_EID_LEN - 1);

	msg_rpt = rda_get_msg_rpt(mgr_eid);
	rpt = rpt_create(ari_copy_ptr(id), getCtime(), NULL);

	val = tnv_from_str(contents);
	rpt_add_entry(rpt, val);

	msg_rpt_add_rpt(msg_rpt, rpt);
}

/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_kplo_ls_setup()
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

void dtn_kplo_ls_cleanup()
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


tnv_t *dtn_kplo_ls_meta_name(tnvc_t *parms)
{
	return tnv_from_str("ls");
}


tnv_t *dtn_kplo_ls_meta_namespace(tnvc_t *parms)
{
	return tnv_from_str("DTN/KPLO/ls");
}


tnv_t *dtn_kplo_ls_meta_version(tnvc_t *parms)
{
	return tnv_from_str("v1.0");
}


tnv_t *dtn_kplo_ls_meta_organization(tnvc_t *parms)
{
	return tnv_from_str("HYUMNI");
}


/* Constant Functions */

/* Table Functions */

/* Collect Functions */

/* Control Functions */

/*
 * List information about the files of a given directory.
 */
tnv_t *dtn_kplo_ls_ctrl_list(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_list BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;
	//int stdout_fd_back;
	char cmd[CMDLEN_MAX];
	char buf[FILESIZE_MAX];
	//char *filename = tmpnam(NULL);
	ari_t *id;
	vecit_t parm_it;

	//char *cmd = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);
	char *directory_path = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);

	snprintf(cmd, CMDLEN_MAX, "ls %s", directory_path);

	//if ((pid = fork()) == -1)
	//{
	//	fprintf(stderr, "cannot fork.\n");
	//	return result;
	//}

	//if (pid == 0)
	//{
	//	/* child */

	//	/* child process calls this freopen */
	//	if ((fp = freopen(filename, "w", stdout)) == NULL)
	//	{
	//		fprintf(stderr, "child: cannot associate \"%s\" with stdout.\n",
	//			filename);
	//		exit(EXIT_FAILURE);
	//	}

	//	/* output of this will go to the filename */
	//	pseudoshell(cmd);
	//	fflush(NULL);
	//	printf("hagomanda\n");//jigi

	//	/* closing all standard IO streams & terminate child process */
	//	exit(EXIT_SUCCESS);
	//}

	///* parent */
	//
	///* parent will be waiting until the child process terminates */
	//if (pid == wait(&stat))
	//{
	//	if (!WIFEXITED(stat))
	//	{
	//		fprintf(stderr, "parent: child process not terminated normally.\n");
	//		return result;
	//	}
	//}

	if (read_output(cmd, buf) == -1)
	{
		fprintf(stderr, "cannot read the command output.\n");
		return result;
	}

	//if (stdout_set(&stdout_fd_back) == -1)
	//{
	//	return result;
	//}

	//if (redirect_output(command_buf, filename) == -1)
	//{
	//	return result;
	//}

	//if (stdout_reset(&stdout_fd_back) == -1)
	//{
	//	return result;
	//}

	//if (remove(filename))
	//{
	//	return result;
	//}

	get_this_ari(&id);

	for (parm_it = vecit_first(&(parms->values)); vecit_valid(parm_it); parm_it = vecit_next(parm_it))
	{
		tnv_t *cur_parm = vecit_data(parm_it);
		vec_push(&(id->as_reg.parms.values), cur_parm);
	}

	gen_rpt(def_mgr, id, buf);

	*status = CTRL_SUCCESS;

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_list BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */

