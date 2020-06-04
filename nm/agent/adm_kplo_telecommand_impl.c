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
 **  2020-05-18  jigi             integrated with adm_kplo_ls_impl & adm_kplo_upgrade_impl
 **  2020-04-16  jigi             initial implementation v1.0
 **
 ****************************************************************************/

/*   START CUSTOM INCLUDES HERE  */
#include "rfx.h"
#include "bpP.h"
#include "ltpP.h"
#include "nmagent.h"
#include "rda.h"
#include "../shared/adm/adm_kplo_telecommand.h"

/*   STOP CUSTOM INCLUDES HERE  */


#include "../shared/adm/adm.h"
#include "adm_kplo_telecommand_impl.h"

/*   START CUSTOM FUNCTIONS HERE */

#define PROXY_PUT_RESPONSE_RECVED	1
#define ORIGINATING_TXN_ID_MATCHED	2

char g_rpt_msg[GMSGLEN_MAX];

int kplo_exec(char *cmd, char *out)
{
	FILE *fp;
	char line[LINELEN_MAX];
	char *cursor;
	int offset, left;

	if ((fp = popen(cmd, "r")) == NULL)
	{
		fprintf(stderr, "cannot popen %s.\n", cmd);
		return -1;
	}
	
	if ((cursor = out) == NULL)
	{
		pclose(fp);
		return 0;
	}

	left = BUF_DEFAULT_ENC_SIZE;
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		if (feof(fp) || ferror(fp))
		{
			fprintf(stderr, "error occured while reading lines from %s\n", cmd);
			pclose(fp);
			return -1;
		}

		offset = strlen(line);

		/* TODO: QCBOR code modification */
		if (left - 1 < offset)
		{
			strncpy(cursor, line, left - 1);
			TRUNC(out, BUF_DEFAULT_ENC_SIZE);
			fprintf(stderr, "output too large, truncated.\n");
			break;
		}

		strcpy(cursor, line);
		cursor += offset;
		left -= offset;
	}

	pclose(fp);

	return 0;
}

void kplo_add_parms(ari_t *id, tnvc_t *parms)
{
	vecit_t parm_it;
	tnv_t *cur_parm;

	for (parm_it = vecit_first(&(parms->values)); vecit_valid(parm_it); parm_it = vecit_next(parm_it))
	{
		cur_parm = vecit_data(parm_it);
		vec_push(&(id->as_reg.parms.values), cur_parm);
	}
}

void kplo_gen_rpt(eid_t mgr, ari_t *id, tnv_t *val)
{
	msg_rpt_t *msg_rpt;
	rpt_t *rpt;
	
	msg_rpt = rda_get_msg_rpt(mgr);
	rpt = rpt_create(ari_copy_ptr(id), getCtime(), NULL);

	rpt_add_entry(rpt, val);

	msg_rpt_add_rpt(msg_rpt, rpt);
}

int kplo_pid_get(char *application)
{
	/* temporary implementation, it should be modified later
	 * to search the /proc/ directory to be more portable. */

	/* or killall command? */

	FILE *fp;
	char line[LINELEN_MAX];
	pid_t pid;
	char proc_name[16];

	fp = popen("ps --no-headers", "r");
	if (fp == NULL)
	{
		INFO_APPEND("cannot run ps.", NULL);
		return -1;
	}

	/* now we start a loop to find the process */
	while (fgets(line, sizeof(line), fp))
	{
		sscanf(line, "%d %*s %*s %s", &pid, proc_name);
		if (strcmp(proc_name, application) == 0)
		{
			pclose(fp);
			return pid;
		}
	}
	
	pclose(fp);
	return -1;
}

int kplo_kill(char *application, int signum)
{
	pid_t pid;

	if ((pid = kplo_pid_get(application)) == -1)
	{
		INFO_APPEND("cannot get pid of %s.", application);
		return -1;
	}

	kill(pid, signum);

	return 0;
}

int kplo_get_full_path_name(char *application_name, char *install_path, char *full_path_name)
{
	int result;

	if (install_path[strlen(install_path) - 1] == '/')
	{
		result = snprintf(full_path_name, PATHLEN_MAX, "%s%s", install_path, application_name);
	}
	else
	{
		result = snprintf(full_path_name, PATHLEN_MAX, "%s/%s", install_path, application_name);
	}

	if (result < 0 || result >= PATHLEN_MAX)
	{
		return -1;
	}

	return 0;
}

void kplo_cfdp_init(CfdpReqParms *parms)
{
	int i;
	
	memset((char *)parms, 0, sizeof(CfdpReqParms));
	cfdp_compress_number(&parms->destinationEntityNbr, 0);

	for (i = 0; i < 16; i++)
	{
		parms->faultHandlers[i] = CfdpIgnore;
	}
	parms->faultHandlers[CfdpFilestoreRejection] = CfdpIgnore;
	parms->faultHandlers[CfdpCheckLimitReached] = CfdpCancel;
	parms->faultHandlers[CfdpChecksumFailure] = CfdpCancel;
	parms->faultHandlers[CfdpInactivityDetected] = CfdpCancel;
	parms->faultHandlers[CfdpFileSizeError] = CfdpCancel;
}

int kplo_cfdp_get_wrapper(int bundle_lifetime, BpCustodySwitch bp_custody, int class_of_service,
				uvast remote_host, char *remote_file, char *local_file)
{
	CfdpReqParms parms;
	int res;

	/* need to manage maximum string length of the buffer */
	/* if () */

	kplo_cfdp_init(&parms);

	parms.utParms.lifespan = bundle_lifetime;
	parms.utParms.classOfService = class_of_service;
	parms.utParms.custodySwitch = bp_custody;

	cfdp_compress_number(&parms.destinationEntityNbr, remote_host);
	memset((char*)&parms.transactionId, 0 , sizeof(CfdpTransactionId));
	snprintf(parms.sourceFileNameBuf, 255, "%.254s", remote_file);
	snprintf(parms.destFileNameBuf, 255, "%.254s", local_file);
	parms.sourceFileName = parms.sourceFileNameBuf;
	parms.destFileName = parms.destFileNameBuf;

	parms.proxytask.destFileName = parms.destFileNameBuf;
	parms.proxytask.faultHandlers = parms.faultHandlers;
	parms.proxytask.filestoreRequests = parms.fsRequests;
	parms.proxytask.messagesToUser = 0;
	parms.proxytask.recordBoundsRespected = 0;
	parms.proxytask.sourceFileName = parms.sourceFileNameBuf;
	parms.proxytask.unacknowledged = 1;

	res = cfdp_get(&(parms.destinationEntityNbr),
			sizeof(BpUtParms),
			(unsigned char *)&(parms.utParms),
			NULL,
			NULL,
			0,
			parms.faultHandlers,
			0,
			NULL,
			0,
			parms.msgsToUser,
			0,
			&parms.proxytask,
			&(parms.transactionId));

	if (res < 0)
	{
		return -1;
	}

	/* pass the transactionId of proxy put request of which following transaction
	 * would cite that transactionId as its originatingTransactionId */
	if (kplo_cfdp_get_event_wrapper(parms.transactionId) == -1)
	{
		return -1;
	}

	return 0;
}

int kplo_cfdp_get_event_wrapper(CfdpTransactionId orig_xn_id)
{
	CfdpEventType		type;
	time_t			time;
	int			reqNbr;
	CfdpTransactionId	transactionId;
	char			sourceFileNameBuf[256];
	char			destFileNameBuf[256];
	uvast			fileSize;
	MetadataList		messagesToUser;
	uvast			offset;
	unsigned int		length;
	unsigned int		recordBoundsRespected;
	CfdpContinuationState	continuationState;
	unsigned int		segMetadataLength;
	char			segMetadata[63];
	CfdpCondition		condition;
	uvast			progress;
	CfdpFileStatus		fileStatus;
	CfdpDeliveryCode	deliveryCode;
	CfdpTransactionId	originatingTransactionId;
	char			statusReportBuf[256];
	MetadataList		filestoreResponses;
	unsigned char		usrmsgBuf[USRMSGLEN_MAX];
	uvast			orig_xn_nbr, cited_xn_nbr;
	uvast			orig_source_entity_nbr, cited_source_entity_nbr;
	char			stat_flag = 0;

	while (1)
	{
		if (cfdp_get_event(&type, &time, &reqNbr, &transactionId,
				sourceFileNameBuf, destFileNameBuf,
				&fileSize, &messagesToUser, &offset, &length,
				&recordBoundsRespected, &continuationState,
				&segMetadataLength, segMetadata,
				&condition, &progress, &fileStatus,
				&deliveryCode, &originatingTransactionId,
				statusReportBuf, &filestoreResponses) < 0)
		{
			return -1;
		}

		/* TODO: we need to check if any file data have been corrupted
		 * or something that might cause the cfdp_get_event to block infinitely
		 * implement it as a thread ? */

		if (type == CfdpMetadataRecvInd &&
			*sourceFileNameBuf == '\0' && *destFileNameBuf == '\0')
		{
			/* this is metadata PDU */

			/* parse Messages to User to check for proxy operation */
			while (messagesToUser)
			{
				memset(usrmsgBuf, 0, sizeof(usrmsgBuf));
				if (cfdp_get_usrmsg(&messagesToUser, usrmsgBuf,
						(int *)&length) < 0)
				{
					INFO_APPEND("error in sdr system.", NULL);
					return -1;
				}

				if (length < 5 ||
					strncmp((char *)&usrmsgBuf, "cfdp", 4) != 0)
				{
					continue;
				}

				if (usrmsgBuf[4] == 7)
				{
					/* this is proxy put response user message */
					stat_flag |= PROXY_PUT_RESPONSE_RECVED;

					/* check if the originating transaction ID of it
					 * indicates the initial proxy put request */
					cfdp_decompress_number(&orig_xn_nbr,
						&orig_xn_id.transactionNbr);
					cfdp_decompress_number(&cited_xn_nbr,
						&originatingTransactionId.transactionNbr);
					cfdp_decompress_number(&orig_source_entity_nbr,
						&orig_xn_id.sourceEntityNbr);
					cfdp_decompress_number(&cited_source_entity_nbr,
						&originatingTransactionId.sourceEntityNbr);
			
					if (orig_xn_nbr == cited_xn_nbr ||
					orig_source_entity_nbr == cited_source_entity_nbr)
					{
						/* originating transaction ID of this event
						 * is the one assigned to the initial proxy
						 * put request. */
						 stat_flag |= ORIGINATING_TXN_ID_MATCHED;
					}

					break;
				}
			}
		}

		if (type == CfdpTransactionFinishedInd)
		{
			/* This is a transaction finished event */

			if ((stat_flag & PROXY_PUT_RESPONSE_RECVED) &&
				(stat_flag & ORIGINATING_TXN_ID_MATCHED))
			{
				/* proxy operations have been completed.
				 * now we will return to the caller */
				break;
			}
		}
	}

	return 0;
}

int kplo_replace(char *dest_filename, char *src_filename)
{
	char filename_back[PATHLEN_MAX + 4];

	oK(snprintf(filename_back, sizeof(filename_back), "%s.old", dest_filename));

	oK(rename(dest_filename, filename_back));
	if (rename(src_filename, dest_filename) != 0)
	{
		oK(rename(filename_back, dest_filename));
		return -1;
	}

	oK(remove(filename_back));
	return 0;
}

int kplo_install(char *application, uvast file_server_eid, char *file_server_file, char *install_path)
{
	char filename[PATHLEN_MAX];
	char filename_tmp[PATHLEN_MAX + 4];

	if (kplo_get_full_path_name(application, install_path, filename) == -1)
	{
		INFO_APPEND("install_path may be too long.", NULL);
		return -1;
	}

	/* query bin images from the file server using CFDP */
	if (cfdp_attach() < 0)
	{
		INFO_APPEND("cannot attach cfdp.", NULL);
		return -1;
	}

	oK(snprintf(filename_tmp, sizeof(filename_tmp), "%s.tmp", filename));

	/* use default values */
	if (kplo_cfdp_get_wrapper(86400, BP_STD_PRIORITY, SourceCustodyRequired,
				file_server_eid, file_server_file, filename_tmp) == -1)
	{
		INFO_APPEND("cfdp_get failed.", NULL);
		oK(remove(filename_tmp));
		/* TODO: cleanup of all the residual ltpblock files should be implemented */
		return -1;
	}

	/* TODO: extract the binary archive and loop through all the files */

	if (kplo_replace(filename, filename_tmp) == -1)
	{
		INFO_APPEND("cannot replace image.", NULL);
		oK(remove(filename_tmp));
		return -1;
	}

	return 0;
}

int kplo_restart(char *application)
{
	char args[PROCCNT_MAX][CMDLEN_MAX];
	int i = 0;

	/* admin */
	if (APPLICATION_IS(acsadmin))
	{
	}
	else if (APPLICATION_IS(ionadmin))
	{
	}
	else if (APPLICATION_IS(bpadmin))
	{
	}
	else if (APPLICATION_IS(ltpadmin))
	{
	}
	else if (APPLICATION_IS(ionsecadmin))
	{
	}

	/* daemon */
	/* ionadmin's */
	else if (APPLICATION_IS(rfxclock))
	{
		if (ionAttach() != 0)
		{
			INFO_APPEND("ION node not initialized yet.", NULL);
			return -1;
		}

		rfx_stop();
		if (rfx_start() < 0)
		{
			INFO_APPEND("cannot start RFX.", NULL);
			return -1;
		}

		time_t refTime = 0;
		ionReferenceTime(&refTime);

		return 0;
	}

	/* bpadmin's */
	else if (APPLICATION_IS(bpclm))
	{
		if (bpAttach() != 0)
		{
			INFO_APPEND("BP not initialized yet.", NULL);
			return -1;
		}
		
		if (kplo_args_get(application, args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		for (i = 0; i < PROCCNT_MAX && *args[i] != '\0'; i++)
		{
			bpStopPlan(args[i] + strlen(application) + 1);
			bpStartPlan(args[i] + strlen(application) + 1);
		}
	}
	else if (APPLICATION_IS(bpclock))
	{
		if (bpAttach() != 0)
		{
			INFO_APPEND("BP not initialized yet.", NULL);
			return -1;
		}
		
		bpStop();
		if (bpStart() < 0)
		{
			INFO_APPEND("cannot start BP.", NULL);
			return -1;
		}

		return 0;
	}
	else if (APPLICATION_IS(bptransit))
	{
		if (bpAttach() != 0)
		{
			INFO_APPEND("BP not initialized yet.", NULL);
			return -1;
		}
		
		bpStop();
		if (bpStart() < 0)
		{
			INFO_APPEND("cannot start BP.", NULL);
			return -1;
		}

		return 0;
	}
	else if (APPLICATION_IS(ipnadminep))
	{
		if (bpAttach() != 0)
		{
			INFO_APPEND("BP not initialized yet.", NULL);
			return -1;
		}
		
		/* find scheme TODO */
		bpStopScheme("ipn");
		bpStartScheme("ipn");

		return 0;
	}
	else if (APPLICATION_IS(ipnfw))
	{
		if (bpAttach() != 0)
		{
			INFO_APPEND("BP not initialized yet.", NULL);
			return -1;
		}
		
		/* find scheme TODO */
		bpStopScheme("ipn");
		bpStartScheme("ipn");

		return 0;
	}
	else if (APPLICATION_IS(ltpcli))
	{
		if (bpAttach() != 0)
		{
			INFO_APPEND("BP not initialized yet.", NULL);
			return -1;
		}
		
		if (kplo_args_get(application, args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		/* find protocol TODO */
		for (i = 0; i < PROCCNT_MAX && *args[i] != '\0'; i++)
		{
			bpStopInduct("ltp", args[i] + strlen(application) + 1);
			bpStartInduct("ltp", args[i] + strlen(application) + 1);
		}

		return 0;
	}
	else if (APPLICATION_IS(ltpclo))
	{
		if (bpAttach() != 0)
		{
			INFO_APPEND("BP not initialized yet.", NULL);
			return -1;
		}
		
		if (kplo_args_get(application, args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		/* find protocol TODO */
		for (i = 0; i < PROCCNT_MAX && *args[i] != '\0'; i++)
		{
			bpStopOutduct("ltp", args[i] + strlen(application) + 1);
			bpStartOutduct("ltp", args[i] + strlen(application) + 1);
		}

		return 0;
	}

	/* ltpadmin's */
	else if (APPLICATION_IS(ltpclock))
	{
		if (ltpAttach() != 0)
		{
			INFO_APPEND("LTP not initialized yet.", NULL);
			return -1;
		}

		if (kplo_args_get(application, args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		ltpStop();
		if (ltpStart(args[i]) < 0)
		{
			INFO_APPEND("cannot start LTP.", NULL);
			return -1;
		}

		return 0;
	}
	else if (APPLICATION_IS(ltpdeliv))
	{
		if (ltpAttach() != 0)
		{
			INFO_APPEND("LTP not initialized yet.", NULL);
			return -1;
		}

		if (kplo_args_get(application, args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		ltpStop();
		if (ltpStart(args[i]) < 0)
		{
			INFO_APPEND("cannot start LTP.", NULL);
			return -1;
		}

		return 0;
	}
	else if (APPLICATION_IS(ltpmeter))
	{
		if (ltpAttach() != 0)
		{
			INFO_APPEND("LTP not initialized yet.", NULL);
			return -1;
		}

		if (kplo_args_get(application, args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		ltpStop();
		if (ltpStart(args[i]) < 0)
		{
			INFO_APPEND("cannot start LTP.", NULL);
			return -1;
		}

		return 0;
	}
	else if (APPLICATION_IS(udplsi))
	{
		if (ltpAttach() != 0)
		{
			INFO_APPEND("LTP not initialized yet.", NULL);
			return -1;
		}

		if (kplo_args_get(application, args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		ltpStop();
		if (ltpStart(args[i]) < 0)
		{
			INFO_APPEND("cannot start LTP.", NULL);
			return -1;
		}

		return 0;
	}
	else if (APPLICATION_IS(udplso))
	{
		char args_tmp[PROCCNT_MAX][CMDLEN_MAX];
		if (ltpAttach() != 0)
		{
			INFO_APPEND("LTP not initialized yet.", NULL);
			return -1;
		}

		if (kplo_args_get("udplsi", args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		if (kplo_args_get("ltpcli", args_tmp) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		ltpStop();

		if (ltpStart(args[0]) < 0)
		{
			INFO_APPEND("cannot start LTP.", NULL);
			return -1;
		}
		while (!ltp_engine_is_started())
		{
			snooze(1);
		}

		/* start ltp clients */
		for (i = 0; i < PROCCNT_MAX && *args_tmp[i] != '\0'; i++)
		{
			bpStartInduct("ltp", args_tmp[i] + strlen("ltpcli") + 1);
		}

		return 0;
	}

	/* other major */
	else if (APPLICATION_IS(bpchat))
	{
		if (kplo_args_get(application, args) == -1)
		{
			INFO_APPEND("cannot get arguments of %s.", application);
			return -1;
		}

		for (i = 0; i < PROCCNT_MAX && *args[i] != '\0'; i++)
		{
			if (kplo_exec(args[i], NULL) == -1)
			{
				INFO_APPEND("cannot execute %s.", args);
				return -1;
			}
		}

		return 0;
	}
	else if (APPLICATION_IS(bpcounter))
	{
	}
	else if (APPLICATION_IS(bpdriver))
	{
	}
	else if (APPLICATION_IS(bpecho))
	{
	}
	else if (APPLICATION_IS(bping))
	{
	}
	else if (APPLICATION_IS(bpsink))
	{
	}
	else if (APPLICATION_IS(bpsource))
	{
	}
	//else if (APPLICATION_IS(nm_agent))
	//{
	//	/* we will invoke another application-specific endpoint ID
	//	 * to start the new nm_agent program. */
	//	char args[CMDLEN_MAX];
	//	if (kplo_args_get(application, args) == -1)
	//	{
	//		INFO_APPEND("cannot get arguments of %s.", application);
	//		return -1;
	//	}

	//	if (kplo_exec(args, NULL) == -1)
	//	{
	//		INFO_APPEND("cannot execute %s.", args);
	//		return -1;
	//	}

	//	return 0;

	//	/* TODO: find available agent_eid */
	//	char *agent_eid = "ipn:22311.2";

	//	INFO_APPEND("restart ok, %s on ipn:22311.1 will be shut down.", application);
	//}

	/* etc. TODO */
	else if (APPLICATION_IS(acslist))
	{
	}
	else if (APPLICATION_IS(aoslsi))
	{
	}
	else if (APPLICATION_IS(aoslso))
	{
	}
	else if (APPLICATION_IS(beacon))
	{
	}
	else if (APPLICATION_IS(bibeclo))
	{
	}
	else if (APPLICATION_IS(bpcancel))
	{
	}
	else if (APPLICATION_IS(bplist))
	{
	}
	else if (APPLICATION_IS(bpnmtest))
	{
	}
	else if (APPLICATION_IS(bprecvfile))
	{
	}
	else if (APPLICATION_IS(bpsendfile))
	{
	}
	else if (APPLICATION_IS(bpstats))
	{
	}
	else if (APPLICATION_IS(bpstats2))
	{
	}
	else if (APPLICATION_IS(bptrace))
	{
	}
	else if (APPLICATION_IS(brsccla))
	{
	}
	else if (APPLICATION_IS(brsscla))
	{
	}
	else if (APPLICATION_IS(bsscounter))
	{
	}
	else if (APPLICATION_IS(bssdriver))
	{
	}
	else if (APPLICATION_IS(bsspadmin))
	{
	}
	else if (APPLICATION_IS(bsspcli))
	{
	}
	else if (APPLICATION_IS(bsspclo))
	{
	}
	else if (APPLICATION_IS(bsspclock))
	{
	}
	else if (APPLICATION_IS(bssrecv))
	{
	}
	else if (APPLICATION_IS(bssStreamingApp))
	{
	}
	else if (APPLICATION_IS(cgrfetch))
	{
	}
	else if (APPLICATION_IS(dccpcli))
	{
	}
	else if (APPLICATION_IS(dccpclo))
	{
	}
	else if (APPLICATION_IS(dccplsi))
	{
	}
	else if (APPLICATION_IS(dccplso))
	{
	}
	else if (APPLICATION_IS(dgr2file))
	{
	}
	else if (APPLICATION_IS(dgrcli))
	{
	}
	else if (APPLICATION_IS(dgrclo))
	{
	}
	else if (APPLICATION_IS(dtn2admin))
	{
	}
	else if (APPLICATION_IS(dtn2adminep))
	{
	}
	else if (APPLICATION_IS(dtn2fw))
	{
	}
	else if (APPLICATION_IS(dtpcadmin))
	{
	}
	else if (APPLICATION_IS(dtpcclock))
	{
	}
	else if (APPLICATION_IS(dtpcd))
	{
	}
	else if (APPLICATION_IS(dtpcreceive))
	{
	}
	else if (APPLICATION_IS(dtpcsend))
	{
	}
	else if (APPLICATION_IS(file2dgr))
	{
	}
	else if (APPLICATION_IS(file2sdr))
	{
	}
	else if (APPLICATION_IS(file2sm))
	{
	}
	else if (APPLICATION_IS(file2tcp))
	{
	}
	else if (APPLICATION_IS(file2udp))
	{
	}
	else if (APPLICATION_IS(hmackeys))
	{
	}
	else if (APPLICATION_IS(imcadmin))
	{
	}
	else if (APPLICATION_IS(imcfw))
	{
	}
	else if (APPLICATION_IS(ionexit))
	{
	}
	else if (APPLICATION_IS(ionrestart))
	{
	}
	else if (APPLICATION_IS(ionunlock))
	{
	}
	else if (APPLICATION_IS(ionwarn))
	{
	}
	else if (APPLICATION_IS(ipnadmin))
	{
	}
	else if (APPLICATION_IS(ipnd))
	{
	}
	else if (APPLICATION_IS(lgagent))
	{
	}
	else if (APPLICATION_IS(lgsend))
	{
	}
	else if (APPLICATION_IS(ltpcounter))
	{
	}
	else if (APPLICATION_IS(ltpdriver))
	{
	}
	else if (APPLICATION_IS(nm_mgr))
	{
	}
	else if (APPLICATION_IS(node))
	{
	}
	else if (APPLICATION_IS(owltsim))
	{
	}
	else if (APPLICATION_IS(owlttb))
	{
	}
	else if (APPLICATION_IS(psmshell))
	{
	}
	else if (APPLICATION_IS(psmwatch))
	{
	}
	else if (APPLICATION_IS(ramsgate))
	{
	}
	else if (APPLICATION_IS(sdatest))
	{
	}
	else if (APPLICATION_IS(sdr2file))
	{
	}
	else if (APPLICATION_IS(sdrmend))
	{
	}
	else if (APPLICATION_IS(sdrwatch))
	{
	}
	else if (APPLICATION_IS(sm2file))
	{
	}
	else if (APPLICATION_IS(smlistsh))
	{
	}
	else if (APPLICATION_IS(smrbtsh))
	{
	}
	else if (APPLICATION_IS(stcpcli))
	{
	}
	else if (APPLICATION_IS(stcpclo))
	{
	}
	else if (APPLICATION_IS(tcp2file))
	{
	}
	else if (APPLICATION_IS(tcpbsi))
	{
	}
	else if (APPLICATION_IS(tcpbso))
	{
	}
	else if (APPLICATION_IS(tcpcli))
	{
	}
	else if (APPLICATION_IS(tcpclo))
	{
	}
	else if (APPLICATION_IS(udp2file))
	{
	}
	else if (APPLICATION_IS(udpbsi))
	{
	}
	else if (APPLICATION_IS(udpbso))
	{
	}
	else if (APPLICATION_IS(udpcli))
	{
	}
	else if (APPLICATION_IS(udpclo))
	{
	}
#ifdef ION_OPEN_SOURCE
	/* admin */
	else if (APPLICATION_IS(cfdpadmin))
	{
	}

	/* daemon */
	/* cfdpadmin's */
	else if (APPLICATION_IS(bpcpd))
	{
	}
	else if (APPLICATION_IS(bputa))
	{
	}
	else if (APPLICATION_IS(cfdpclock))
	{
	}

	/* other major */
	else if (APPLICATION_IS(bpcp))
	{
	}

	/* etc. TODO */

	else if (APPLICATION_IS(amsbenchr))
	{
	}
	else if (APPLICATION_IS(amsbenchs))
	{
	}
	else if (APPLICATION_IS(amsd))
	{
	}
	else if (APPLICATION_IS(amshello))
	{
	}
	else if (APPLICATION_IS(amslog))
	{
	}
	else if (APPLICATION_IS(amslogprt))
	{
	}
	else if (APPLICATION_IS(amsshell))
	{
	}
	else if (APPLICATION_IS(amsstop))
	{
	}
	else if (APPLICATION_IS(cfdptest))
	{
	}
#endif /* ION_OPEN_SOURCE */
	return -1;
}

int kplo_args_get(char *application, char (*args)[CMDLEN_MAX])
{
	/* temporary implementation, it should be modified later
	 * first to use pid of the ION vdb, and then ultimately
	 * to search the /proc/ directory to be more portable. */

	FILE *fp;
	char line[LINELEN_MAX];
	//pid_t pid;
	char proc_name[24];
	int i;

	fp = popen("ps --no-headers -o args", "r");
	if (fp == NULL)
	{
		INFO_APPEND("cannot run ps.", NULL);
		return -1;
	}

	/* now we start a loop to find the process */
	i = 0;
	while (fgets(line, sizeof(line), fp))
	{
		sscanf(line, "%s", proc_name);
		if (strcmp(proc_name, application) == 0)
		{
			/* TODO: implement linked list */
			if (i == PROCCNT_MAX)
			{
				INFO_APPEND("too many instances of %s.", application);
				return -1;
			}

			/* strcpy -> strncpy TODO */
			strcpy(args[i], line);
			/* replace a new-line character to null */
			args[i][strlen(args[i]) - 1] = '\0';
			i++;
		}
	}

	pclose(fp);

	if (i < PROCCNT_MAX)
	{
		args[i][0] = '\0';
	}
	
	return 0;
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
 * Execute a local command.
 */
tnv_t *dtn_kplo_telecommand_ctrl_exec(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_exec BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;
	char output[BUF_DEFAULT_ENC_SIZE];
	int ret;
	ari_t *id;
	tnv_t *val;

	char *cmd = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);

	if ((ret = kplo_exec(cmd, output)) == -1)
	{
		INFO_APPEND("cannot read the command output.", NULL);
	}

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_kplo_telecommand_idx[ADM_CTRL_IDX], DTN_KPLO_TELECOMMAND_CTRL_EXEC);
	kplo_add_parms(id, parms);
	if (ret == -1)
	{
		val = tnv_from_str(g_rpt_msg);
	}
	else
	{
		val = tnv_from_str(output);
	}

	kplo_gen_rpt(*def_mgr, id, val);
	FLUSH(g_rpt_msg);

	*status = CTRL_SUCCESS;

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_exec BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * Kill (or send a corresponding signal to) a process.
 */
tnv_t *dtn_kplo_telecommand_ctrl_kill(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_kill BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;
	ari_t *id;
	tnv_t *val;

	char *application = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);
	unsigned int signum = adm_get_parm_uint(parms, 1, &success);
	
	if (kplo_kill(application, signum) == -1)
	{
		INFO_APPEND("cannot send %u to %s.", signum, application);
	}
	else
	{
		INFO_APPEND("successfully sent %u to %s.", signum, application);
	}

	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_kplo_telecommand_idx[ADM_CTRL_IDX], DTN_KPLO_TELECOMMAND_CTRL_KILL);
	kplo_add_parms(id, parms);
	val = tnv_from_str(g_rpt_msg);

	kplo_gen_rpt(*def_mgr, id, val);
	FLUSH(g_rpt_msg);

	*status = CTRL_SUCCESS;

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_kill BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}

/*
 * Install given binary archives using CFDP via a (local) file server Agent and restart
 * any applications that need to be updated.
 */
tnv_t *dtn_kplo_telecommand_ctrl_update(eid_t *def_mgr, tnvc_t *parms, int8_t *status)
{
	tnv_t *result = NULL;
	*status = CTRL_FAILURE;
	/*
	 * +-------------------------------------------------------------------------+
	 * |START CUSTOM FUNCTION ctrl_update BODY
	 * +-------------------------------------------------------------------------+
	 */
	int success;

	ari_t *id;
	tnv_t *val;
	pid_t pid;

	char *application_name = adm_get_parm_obj(parms, 0, AMP_TYPE_STR);
	uvast file_server_eid = adm_get_parm_uvast(parms, 1, &success);
	char *file_server_file = adm_get_parm_obj(parms, 2, AMP_TYPE_STR);
	char *install_path = adm_get_parm_obj(parms, 3, AMP_TYPE_STR);

	if (application_name == NULL || install_path == NULL)
	{
		INFO_APPEND("application_name or install_path is invalid.", NULL);
		goto GEN_RPT;
	}

	if (kplo_install(application_name, file_server_eid, file_server_file, install_path) == -1)
	{
		INFO_APPEND("installation failed (%s).", application_name);
		goto GEN_RPT;
	}

	/* now installation has been successfully finished */
	INFO_APPEND("successfully installed %s at %s.", application_name, install_path);
	
	/* we will examine the name of the application and restart it
	 * if it has been running */

	/* TODO: we need to make DB to store process names and
	 * possibly multiple instances of each of them */
	if (kplo_pid_get(application_name) > 0)
	{
		if (kplo_restart(application_name) == -1)
		{
			INFO_APPEND("restart failed.", NULL);
			goto GEN_RPT;
		}

		INFO_APPEND("successfully restarted %s.", application_name);
	}
	
GEN_RPT:
	id = adm_build_ari(AMP_TYPE_CTRL, 1, g_dtn_kplo_telecommand_idx[ADM_CTRL_IDX], DTN_KPLO_TELECOMMAND_CTRL_UPDATE);
	kplo_add_parms(id, parms);
	val = tnv_from_str(g_rpt_msg);

	kplo_gen_rpt(*def_mgr, id, val);
	FLUSH(g_rpt_msg);

	*status = CTRL_SUCCESS;

	/*
	 * +-------------------------------------------------------------------------+
	 * |STOP CUSTOM FUNCTION ctrl_update BODY
	 * +-------------------------------------------------------------------------+
	 */
	return result;
}


/* OP Functions */

