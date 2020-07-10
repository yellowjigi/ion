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
#define ARMUR_LIBPATH_DEFAULT	"/usr/local/lib/"
#define ARMUR_APPPATH_DEFAULT	"/usr/local/bin/"

/*	ARMUR states		*/
#define	ARMUR_STAT_IDLE		0
#define	ARMUR_STAT_DOWNLOADING	1
#define	ARMUR_STAT_DOWNLOADED	2
#define	ARMUR_STAT_INSTALLING	3
#define	ARMUR_STAT_LV0_PENDING	4
#define ARMUR_STAT_LV1_PENDING	5
#define ARMUR_STAT_LV2_PENDING	6

/*	For restart queues	*/
#define	ARMUR_RESTART_LV0	0		/*	Core library		*/
#define	ARMUR_RESTART_LV1	1		/*	Protocol library	*/
#define	ARMUR_RESTART_LV2	2		/*	Daemon applications	*/

/*	For restart mask	*/
#define	ARMUR_ALL	15
#define ARMUR_ION	8
#define	ARMUR_BP	4
#define ARMUR_LTP	2
#define ARMUR_CFDP	1

typedef int		(*restartFn)();

typedef struct {
	char		name[16];
	restartFn	restart;
	char		protocol;
	time_t		installedTime;
} ARMUR_Image;

typedef struct {
	Object		obj;			/*	Direct reference to Image	*/
} ARMUR_ImageRef;

typedef struct {
	char		stat;
	char		numInstalled[2];
	Object		installPath[2];		/*	SDR strings		*/
	Object		images[2];		/*	SDR lists		*/
} ARMUR_DB;

typedef struct {
	char		restartMask;
	PsmAddress	restartQueue[3];	/*	SM lists of ImageRef	*/
} ARMUR_VDB;

extern int		armurInit();
extern int		armurAttach();
extern void		armurDetach();
extern Object		getArmurDbObject();
extern ARMUR_DB		*getArmurConstants();
extern ARMUR_VDB	*getArmurVdb();
extern int		armurParseImageName(char *imageName);
extern int		armurFindImage(char *imageName, Object *imageObj, Object *imageElt);
int			armurEnqueue(char *imageName);
int			armurRestart();
//void			armurResetVdb();
