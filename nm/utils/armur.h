// 2020-06-29
// jigi

#ifndef _ARMUR_H_
#define _ARMUR_H_

#include "rfx.h"
#include "bpP.h"
#include "ltpP.h"
#include "cfdpP.h"

/*	Image types		*/
#define	ARMUR_LIB	0
#define	ARMUR_APP	1

/*	Default install paths	*/
#define ARMUR_LIBPATH_DEFAULT	"/usr/local/lib"
#define ARMUR_APPPATH_DEFAULT	"/usr/local/bin"

/*	ARMUR states		*/
#define	ARMUR_STAT_IDLE		0
#define	ARMUR_STAT_DOWNLOADING	1
#define	ARMUR_STAT_DOWNLOADED	2
#define	ARMUR_STAT_INSTALLED	3

/*	For restart queues	*/
#define	ARMUR_LV0	0	/*	Core library		*/
#define	ARMUR_LV1	1	/*	Protocol library	*/
#define	ARMUR_LV2	2	/*	Daemon applications	*/

/*	For restart mask	*/
#define ARMUR_CORE	1
#define ARMUR_LTP	2
#define	ARMUR_BP	4
#define ARMUR_CFDP	8
#define	ARMUR_ALL	15
#define	ARMUR_RESERVED	16

/*	Buffer size		*/
#define	ARMUR_PATHNAME_LEN_MAX	64
#define	ARMUR_FILENAME_LEN_MAX	16

/*	Hash values		*/
#define ARMUR_RESTARTFN_HASH_KEY_LEN		ARMUR_FILENAME_LEN_MAX
#define ARMUR_RESTARTFN_HASH_KEY_BUFLEN		(ARMUR_RESTARTFN_HASH_KEY_LEN << 1)
#define ARMUR_RESTARTFN_HASH_ENTRIES		32
#define ARMUR_RESTARTFN_HASH_SEARCH_LEN		4

/*	Others			*/
#define	TMP_EXT		".tmp"

typedef int		(*RestartFn)();

typedef struct {
	char		imageName[ARMUR_FILENAME_LEN_MAX];
	RestartFn	restart;		/*	Local restart function pointer	*/
	Object		hashEntry;		/*	Entry in restartFns hash	*/
} ARMUR_RestartFn;

typedef struct {
	Object		restartFnObj;		/*	RestartFn address		*/
	unsigned int	count;
} ARMUR_RestartFnSet;

typedef struct {
	char		name[ARMUR_FILENAME_LEN_MAX];
	char		type;
	char		protocol;
	Object		restartFnObj;		/*	RestartFn address		*/
	time_t		installedTime;
} ARMUR_Image;

typedef struct {
	Object		imageObj;		/*	Direct reference to Image	*/
} ARMUR_ImageRef;

typedef struct {
	uvast		srcNbr;
	uvast		txnNbr;
	Object		archiveName;		/*	SDR string			*/
} ARMUR_CfdpInfo;

typedef struct {
	char		stat;
	char		numInstalled[2];
	Object		installPath[2];		/*	SDR strings			*/
	Object		restartFns;		/*	SDR hash of RestartFnSets	*/
	Object		images[2][8];		/*	SDR lists of Images		*/
	Object		nmagentCmd;		/*	SDR string			*/
	Object		cfdpInfo;		/*	CfdpInfo address		*/
} ARMUR_DB;

typedef struct {
	char		restartMask;
	Object		cfdpInfo;		/*	CfdpInfo address		*/
	pid_t		nmagentPid;		/*	For stopping nm_agent		*/
	PsmAddress	restartQueue[3];	/*	SM lists of ImageRefs		*/
} ARMUR_VDB;

extern int		armurInit();
extern void		armurDropVdb();
extern void		armurRaiseVdb();
extern int		armurStart(char *nmagentCmd);
extern int		armurAttach();
extern void		armurDetach();
extern Object		getArmurDbObject();
extern ARMUR_DB		*getArmurConstants();
extern ARMUR_VDB	*getArmurVdb();

extern void		armurParseImageName(char *imageName, int *type, int *level);
extern int		armurGetProtocolIndex(char protocolFlag);
extern int		armurFindRestartFn(char *imageName, Object *restartFnObj);
extern void		armurFindImage(char *imageName, int imageType,
					Object *imageObj, Object *imageElt);
extern int		armurAddImage(char *imageName, int protocol);
extern int		armurUpdateCfdpSrcNbr(uvast cfdpSrcNbr);
extern int		armurUpdateCfdpTxnNbr(uvast cfdpTxnNbr);
extern int		armurUpdateCfdpArchiveName(char *archiveName);
extern int		armurWait();
extern int		armurInstall();
extern int		armurRestart();

#endif	/* _ARMUR_H_ */
