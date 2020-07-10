/*
 	bpum.h:	definitions supporting BP ARMUR API.

	Author: jigi
	Date: 2020/06/27
 									*/
#ifndef _BPUM_H_
#define _BPUM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BPNM_ENDPOINT_NAME_FMT_STRING           ("%s:%s")
#define BPNM_ENDPOINT_EIDSTRING_LEN             (32)


/*****************************************/

typedef struct
{
	pid_t		pid;
	char		appName[16];
	time_t		lastInstalledTime;
	unsigned int	running;		/*	Boolean		*/
	unsigned int	daemon;			/*	Boolean		*/
	unsigned int	_new;			/*	Boolean		*/
} UmbpProc;

extern void	bpum_proc_get(UmbpProc *buffer);

#ifdef __cplusplus
}
#endif

#endif  /* _BPUM_H_ */
