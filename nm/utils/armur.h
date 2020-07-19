// 2020-06-29
// jigi

#include "rfx.h"
#include "bpP.h"
#include "ltpP.h"
#include "cfdpP.h"

/*	Image class		*/
#define ARMUR_CORE_LIBRARY	0
#define ARMUR_PROTOCOL_LIBRARY	1
#define ARMUR_APPLICATION	2

#define	ARMUR_LIBS	0
#define	ARMUR_APPS	1
#define ARMUR_LIBPATH_DEFAULT	"/usr/local/lib"
#define ARMUR_APPPATH_DEFAULT	"/usr/local/bin"

/*	ARMUR states		*/
#define	ARMUR_STAT_IDLE		0
#define	ARMUR_STAT_DOWNLOADING	1
#define	ARMUR_STAT_DOWNLOADED	2
#define	ARMUR_STAT_RESTART_PENDING	3
//#define	ARMUR_STAT_INSTALLING	3
//#define	ARMUR_STAT_LV0_PENDING	4
//#define ARMUR_STAT_LV1_PENDING	5
//#define ARMUR_STAT_LV2_PENDING	6

/*	For retention queues	*/
#define	ARMUR_LV0	0		/*	Core library		*/
#define	ARMUR_LV1	1		/*	Protocol library	*/
#define	ARMUR_LV2	2		/*	Daemon applications	*/

/*	For restart mask	*/
#define	ARMUR_ALL	15
#define ARMUR_ION	8
#define	ARMUR_BP	4
#define ARMUR_LTP	2
#define ARMUR_CFDP	1

/*	Buffer size		*/
#define	PATHNAME_LEN_MAX	64
#define	FILENAME_LEN_MAX	16

/*	Others			*/
#define	TMP_EXT		".tmp"

typedef int		(*ARMUR_RestartFn)();

typedef struct {
	char		name[FILENAME_LEN_MAX];
	ARMUR_RestartFn	restart;
	char		protocol;
	time_t		installedTime;
} ARMUR_Image;

typedef struct {
	Object		obj;			/*	Direct reference to Image	*/
} ARMUR_ImageRef;

typedef struct {
	uvast		srcNbr;
	uvast		txnNbr;
	Object		archiveName;		/*	SDR string		*/
} ARMUR_CfdpInfo;

typedef struct {
	char		stat;
	char		numInstalled[2];
	Object		installPath[2];		/*	SDR strings		*/
	Object		images[2];		/*	SDR lists of Image	*/
	Object		queue[3];		/*	SDR lists of ImageRef	*/
	Object		nmagentCmd;		/*	SDR string		*/
	Object		cfdpInfo;		/*	SDR address of CfdpInfo	*/
} ARMUR_DB;

typedef struct {
	char		restartMask;
	//pid_t		nmagentPid;
} ARMUR_VDB;

extern int		armurInit();
extern int		armurAttach();
extern void		armurDetach();
extern Object		getArmurDbObject();
extern ARMUR_DB		*getArmurConstants();
extern ARMUR_VDB	*getArmurVdb();
extern int		armurStart(char *ampTrigger);
extern int		armurFindImage(char *imageName, Object *imageObj, Object *imageElt);
extern int		armurAddImage(char *imageName, ARMUR_RestartFn restartFn,
					int protocol, Object imageList);
extern int		armurUpdateCfdpSrcNbr(uvast cfdpSrcNbr);
extern int		armurUpdateCfdpTxnNbr(uvast cfdpTxnNbr);
extern int		armurUpdateCfdpArchiveName(char *archiveName);
extern int		armurWait();
extern int		armurInstall();
extern int		armurRestart();
//void			armurResetVdb();
