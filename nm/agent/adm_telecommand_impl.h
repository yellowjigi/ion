/****************************************************************************
 **
 ** File Name: adm_telecommand_impl.h
 **
 ** Description: telecommand implementation header file
 **
 ** Notes: simple telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-08-12  jigi             migrate to adm_telecommand_impl.h
 **  2020-05-18  jigi             integrate with adm_kplo_ls_impl & adm_kplo_upgrade_impl
 **  2020-04-16  jigi             initial implementation v1.0
 **
 ****************************************************************************/

#ifndef ADM_TELECOMMAND_IMPL_H_
#define ADM_TELECOMMAND_IMPL_H_

/*   START CUSTOM INCLUDES HERE  */
#include "../../cfdp/utils/bpcp.h"

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

#define LINELEN_MAX		1024
#define USRMSGLEN_MAX		256
#define CMDLEN_MAX		128
#define PATHLEN_MAX		64
#define NAMELEN_MAX		32
#define FILECNT_MAX		32
#define PROCCNT_MAX		16

#define BUF_DEFAULT_ENC_SIZE	(TNVC_DEFAULT_ENC_SIZE - 10)
#define GMSGLEN_MAX		BUF_DEFAULT_ENC_SIZE

/* some macro utils */
#define TRUNC(buf, len)		strcpy(&buf[(len) - 4], "...")
#define INFO_APPEND(format, ...) do { \
	_isprintf(g_rpt_msg + strlen(g_rpt_msg), GMSGLEN_MAX - strlen(g_rpt_msg), format "\n", __VA_ARGS__); \
	} while (0)
#define FLUSH(buf)		buf[0] = '\0'
#define APPLICATION_IS(app)	strcmp(application, #app) == 0

/* list of ION applications */
#ifndef ION_OPEN_SOURCE
#define ION_OPEN_SOURCE
#endif
#define ACSADMIN	"acsadmin"
#define ACSLIST		"acslist"
#define AOSLSI		"aoslsi"
#define AOSLSO		"aoslso"
#define BEACON		"beacon"
#define BIBECLO 	"bibeclo"
#define BPADMIN		"bpadmin"
#define BPCANCEL	"bpcancel"
#define BPCHAT		"bpchat"
#define BPCLM		"bpclm"
#define BPCLOCK		"bpclock"
#define BPCOUNTER	"bpcounter"
#define BPDRIVER	"bpdriver"
#define BPECHO		"bpecho"
#define BPING		"bping"
#define BPLIST		"bplist"
#define BPNMTEST	"bpnmtest"
#define BPRECVFILE	"bprecvfile"
#define BPSENDFILE	"bpsendfile"
#define BPSINK		"bpsink"
#define BPSOURCE	"bpsource"
#define BPSTATS		"bpstats"
#define BPSTATS2	"bpstats2"
#define BPTRACE		"bptrace"
#define BPTRANSIT	"bptransit"
#define BRSCCLA		"brsccla"
#define BRSSCLA		"brsscla"
#define BSSCOUNTER	"bsscounter"
#define BSSDRIVER	"bssdriver"
#define BSSPADMIN	"bsspadmin"
#define BSSPCLI		"bsspcli"
#define BSSPCLO		"bsspclo"
#define BSSPCLOCK	"bsspclock"
#define BSSRECV		"bssrecv"
#define BSSSTREAMINGAPP	"bssStreamingApp"
#define CGRFETCH	"cgrfetch"
#define DCCPCLI		"dccpcli"
#define DCCPCLO		"dccpclo"
#define DCCPLSI		"dccplsi"
#define DCCPLSO		"dccplso"
#define DGR2FILE	"dgr2file"
#define DGRCLI		"dgrcli"
#define DGRCLO		"dgrclo"
#define DTN2ADMIN	"dtn2admin"
#define DTN2ADMINEP	"dtn2adminep"
#define DTN2FW		"dtn2fw"
#define DTPCADMIN	"dtpcadmin"
#define DTPCCLOCK	"dtpcclock"
#define DTPCD		"dtpcd"
#define DTPCRECEIVE	"dtpcreceive"
#define DTPCSEND	"dtpcsend"
#define FILE2DGR	"file2dgr"
#define FILE2SDR	"file2sdr"
#define FILE2SM		"file2sm"
#define FILE2TCP	"file2tcp"
#define FILE2UDP	"file2udp"
#define HMACKEYS	"hmackeys"
#define IMCADMIN	"imcadmin"
#define IMCFW		"imcfw"
#define IONADMIN	"ionadmin"
#define IONEXIT		"ionexit"
#define IONRESTART	"ionrestart"
#define IONSECADMIN	"ionsecadmin"
#define IONUNLOCK	"ionunlock"
#define IONWARN		"ionwarn"
#define IPNADMIN	"ipnadmin"
#define IPNADMINEP	"ipnadminep"
#define IPND		"ipnd"
#define IPNFW		"ipnfw"
#define LGAGENT		"lgagent"
#define LGSEND		"lgsend"
#define LTPADMIN	"ltpadmin"
#define LTPCLI		"ltpcli"
#define LTPCLO		"ltpclo"
#define LTPCLOCK	"ltpclock"
#define LTPCOUNTER	"ltpcounter"
#define LTPDELIV	"ltpdeliv"
#define LTPDRIVER	"ltpdriver"
#define LTPMETER	"ltpmeter"
#define NM_AGENT	"nm_agent"
#define NM_MGR		"nm_mgr"
#define NODE		"node"
#define OWLTSIM		"owltsim"
#define OWLTTB		"owlttb"
#define PSMSHELL	"psmshell"
#define PSMWATCH	"psmwatch"
#define RAMSGATE	"ramsgate"
#define RFXCLOCK	"rfxclock"
#define SDATEST 	"sdatest"
#define SDR2FILE	"sdr2file"
#define SDRMEND		"sdrmend"
#define SDRWATCH	"sdrwatch"
#define SM2FILE		"sm2file"
#define SMLISTSH	"smlistsh"
#define SMRBTSH		"smrbtsh"
#define STCPCLI		"stcpcli"
#define STCPCLO		"stcpclo"
#define TCP2FILE	"tcp2file"
#define TCPBSI		"tcpbsi"
#define TCPBSO		"tcpbso"
#define TCPCLI		"tcpcli"
#define TCPCLO		"tcpclo"
#define UDP2FILE	"udp2file"
#define UDPBSI		"udpbsi"
#define UDPBSO		"udpbso"
#define UDPCLI		"udpcli"
#define UDPCLO		"udpclo"
#define UDPLSI		"udplsi"
#define UDPLSO		"udplso"
#ifdef ION_OPEN_SOURCE
#define AMSBENCHR	"amsbenchr"
#define AMSBENCHS	"amsbenchs"
#define AMSD		"amsd"
#define AMSHELLO	"amshello"
#define AMSLOG		"amslog"
#define AMSLOGPRT	"amslogprt"
#define AMSSHELL	"amsshell"
#define AMSSTOP 	"amsstop"
#define BPCP		"bpcp"
#define BPCPD		"bpcpd"
#define BPUTA		"bputa"
#define CFDPADMIN	"cfdpadmin"
#define CFDPCLOCK	"cfdpclock"
#define CFDPTEST	"cfdptest"
#endif /* ION_OPEN_SOURCE */

int kplo_exec(char *cmd, char *out);
int kplo_args_get(char *application, char (*args)[CMDLEN_MAX]);
int kplo_pid_get(char *application);
int kplo_kill(char *application, int signum);
char *kplo_cut_filename(char *full_path_filename);
int kplo_cfdp_get_wrapper(int bundle_lifetime, BpCustodySwitch bp_custody, int class_of_service,
				uvast remote_host, char *remote_file, char *local_file);
int kplo_cfdp_get_event_wrapper(CfdpTransactionId orig_xn_id);
int kplo_prepend_path(char *filename);
int kplo_extract_archive(char *archive_name, char (*filename)[PATHLEN_MAX],
				char (*filename_tmp)[PATHLEN_MAX + 4], int *file_count);
int kplo_replace(char *filename_dest, char *filename_src, char *filename_back);
int kplo_install(uvast file_server_eid, char *file_server_file);
int kplo_restart(char *application);
void kplo_add_parms(ari_t *id, tnvc_t *parms);
void kplo_gen_rpt(eid_t mgr, ari_t *id, tnv_t *val);

/*   STOP CUSTOM FUNCTIONS HERE  */

void dtn_telecommand_setup();
void dtn_telecommand_cleanup();


/* Metadata Functions */
tnv_t *dtn_telecommand_meta_name(tnvc_t *parms);
tnv_t *dtn_telecommand_meta_namespace(tnvc_t *parms);
tnv_t *dtn_telecommand_meta_version(tnvc_t *parms);
tnv_t *dtn_telecommand_meta_organization(tnvc_t *parms);

/* Constant Functions */

/* Collect Functions */

/* Control Functions */
tnv_t *dtn_telecommand_ctrl_exec(eid_t *def_mgr, tnvc_t *parms, int8_t *status);
tnv_t *dtn_telecommand_ctrl_kill(eid_t *def_mgr, tnvc_t *parms, int8_t *status);
tnv_t *dtn_telecommand_ctrl_update(eid_t *def_mgr, tnvc_t *parms, int8_t *status);


/* OP Functions */

/* Table Build Functions */

#endif //ADM_TELECOMMAND_IMPL_H_
