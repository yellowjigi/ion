//JIGI

#ifndef _ARMURNM_H_
#define _ARMURNM_H_

typedef struct {
	char	currentStat;
} NmArmurDB;

extern void	armurnm_state_get(NmArmurDB *buf);
extern void	armurnm_record_get(char *msgBuf, int bufLen, char *msgPtr[], int *numMsgs);

#endif	/* _ARMURNM_H_ */
