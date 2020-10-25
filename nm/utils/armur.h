// 2020-06-29
// jigi

#ifndef _ARMUR_H_
#define _ARMUR_H_

#include "ion.h"

/*	Default install paths	*/
#define ARMUR_PATH_LIB_DEFAULT	"/usr/local/lib"
#define ARMUR_PATH_APP_DEFAULT	"/usr/local/bin"

/*	Image types		*/
#define ARMUR_IMAGETYPE_LIB	0
#define ARMUR_IMAGETYPE_APP	1
#define ARMUR_IMAGETYPES	2

/*	Image levels		*/
#define	ARMUR_LEVEL_0	0	/*	Core library		*/
#define	ARMUR_LEVEL_1	1	/*	Protocol library	*/
#define	ARMUR_LEVEL_2	2	/*	Applications		*/
#define	ARMUR_LEVELS	3

/*	Image layers		*/
#define	ARMUR_LAYER_APPLICATION		0
#define	ARMUR_LAYER_BUNDLE		1
#define	ARMUR_LAYER_CONVERGENCE		2
#define	ARMUR_LAYERS			3

/*	Application types	*/
#define	ARMUR_APPTYPE_DAEMON		0
#define ARMUR_APPTYPE_NORMAL		1
#define	ARMUR_APPTYPES			2

/*	ARMUR states		*/
#define	ARMUR_STAT_IDLE		0
//#define	ARMUR_STAT_DOWNLOADING	1
#define	ARMUR_STAT_DOWNLOADED	1
#define ARMUR_STAT_INSTALLED	2

/*	ARMUR volatile states	*/
#define	ARMUR_VSTAT_IDLE	0
#define	ARMUR_VSTAT_LV0_PENDING	1
#define	ARMUR_VSTAT_LV1_PENDING	2
#define	ARMUR_VSTAT_LV2_PENDING	4
#define ARMUR_VSTAT_LV2_STOPPED	1
#define ARMUR_VSTAT_LV2_STARTED	2
#define ARMUR_VSTAT(level)	(1 << level)

/*	Buffer size		*/
#define	ARMUR_PATHNAME_LEN_MAX	64
#define	ARMUR_FILENAME_LEN_MAX	32

/*	Others			*/
#define	TMP_EXT		".tmp"

typedef int		(*ARMUR_StopFn)();
typedef int		(*ARMUR_StartFn)();

typedef struct {
	char		name[ARMUR_FILENAME_LEN_MAX];
	char		packageName[32];		/*	TODO: chage to flag?	*/
	char		level;
	char		tag;				/*	Additional info.	*/
	time_t		installedTime;
} ARMUR_Image;

typedef struct {
	uvast		srcNbr;
	Object		archiveName;			/*	SDR string		*/
} ARMUR_CfdpInfo;

typedef struct {
	char		stat;
	char		numInstalled[ARMUR_IMAGETYPES];
	Object		installPath[ARMUR_IMAGETYPES];	/*	SDR strings		*/
	Object		images[ARMUR_LEVELS];		/*	SDR lists		*/
	Object		cfdpInfo;			/*	CfdpInfo address	*/
	Object		nmagentCmd;			/*	SDR string		*/
} ARMUR_DB;

typedef struct {
	PsmAddress	packages[ARMUR_LAYERS];		/*	SM lists of packages	*/
	PsmAddress	applications[ARMUR_APPTYPES];	/*	SM lists of applications*/
} ARMUR_VImageLv0;

typedef struct {
	char		layer;
	PsmAddress	applications[ARMUR_APPTYPES];	/*	SM lists of applications*/
} ARMUR_VImageLv1;

typedef struct {
	char		vstat;
	ARMUR_StopFn	stop;				/*	StopFn pointer		*/
	ARMUR_StartFn	start;				/*	StartFn pointer		*/
} ARMUR_VImageLv2;

typedef struct {
	Object		addr;				/*	Direct ref. to Image	*/
	union {
		ARMUR_VImageLv0	lv0;
		ARMUR_VImageLv1 lv1;
		ARMUR_VImageLv2 lv2;
	} as;
} ARMUR_VImage;

typedef struct {
	char		name[8];
	PsmAddress	applications[ARMUR_APPTYPES];	/*	SM lists		*/
} ARMUR_VPackageDescr;

typedef struct {
	char		vstat;
	PsmAddress	vimages;			/*	Hash table of VImages	*/
	PsmAddress	corePackages;			/*	SM list			*/
	PsmAddress	packages[ARMUR_LAYERS];		/*	SM lists		*/
	PsmAddress	applications;			/*	SM list			*/
	PsmAddress	restartQueue[ARMUR_LEVELS];	/*	SM lists of VImages	*/
	Object		cfdpInfo;			/*	CfdpInfo address	*/
	pid_t		nmagentPid;			/*	For stopping nm_agent	*/
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

extern void		armurFindPackageDescr(char *packageName,
				ARMUR_VPackageDescr **vdescr, PsmAddress *vdescrElt);
extern void		armurFindImage(char *imageName, ARMUR_VImage **vimage,
				PsmAddress *vimageAddr);
extern int		armurAddImageLv0(char *imageName, char *packageName);
extern int		armurAddImageLv1(char *imageName, char *packageName, int layer);
extern int		armurAddImageLv2(char *imageName, char *packageName, int apptype);
extern void		armurUpdateStat(int armurStat);
extern int		armurUpdateCfdpSrcNbr(uvast cfdpSrcNbr);
//extern int		armurUpdateCfdpTxnNbr(uvast cfdpTxnNbr);
extern int		armurUpdateCfdpArchiveName(char *archiveName);
extern int		armurWait();
extern int		armurInstall();
//extern int		armurRestart();

#endif	/* _ARMUR_H_ */
