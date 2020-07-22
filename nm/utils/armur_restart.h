// 2020-07-20
// jigi

#ifndef _ARMUR_RESTART_H_
#define _ARMUR_RESTART_H_

#include "armur.h"

/*	Libraries	*/
extern int	ionRestart();
extern int	ltpRestart();
extern int	bpRestart();
extern int	cfdpRestart();

/*	Daemons		*/
extern int	rfxclockRestart();

extern int	ltpclockRestart();
extern int	ltpdelivRestart();
extern int	ltpmeterRestart();
extern int	udplsiRestart();
extern int	udplsoRestart();

extern int	ltpcliRestart();
extern int	ltpcloRestart();

extern int	bpclockRestart();
extern int	bptransitRestart();
extern int	ipnfwRestart();
extern int	ipnadminepRestart();
extern int	bpclmRestart();

extern int	bputaRestart();

extern int	cfdpclockRestart();

/*	NM	*/
extern int	nmagentRestart(char *startCmd);

#endif	/* _ARMUR_RESTART_H_ */
