//JIGI

#include "armur.h"
#include "armurnm.h"

void	armurnm_state_get(NmArmurDB *buf)
{
	Sdr		sdr = getIonsdr();
	Object		armurDbObject = getArmurDbObject();
	ARMUR_DB	armurdb;

	CHKVOID(buf);

	CHKVOID(sdr_begin_xn(sdr));
	sdr_read(sdr, (char *)&armurdb, armurDbObject, sizeof(ARMUR_DB));
	buf->currentStat = armurdb.stat;
	sdr_exit_xn(sdr);
}

void	armurnm_record_get(char *msgBuf, int bufLen, char *msgPtr[], int *numMsgs)
{
	Sdr		sdr = getIonsdr();
	Object		armurDbObject = getArmurDbObject();
	ARMUR_DB	armurdb;
	Object		elt;
	Object		recordObj;
	char		*cursor;
	int		len;

	CHKVOID(msgBuf);
	CHKVOID(msgPtr);
	CHKVOID(numMsgs);

	*numMsgs = 0;
	cursor = msgBuf;

	CHKVOID(sdr_begin_xn(sdr));
	sdr_read(sdr, (char *)&armurdb, armurDbObject, sizeof(ARMUR_DB));
	for (elt = sdr_list_first(sdr, armurdb.records); elt; elt = sdr_list_next(sdr, elt))
	{
		recordObj = sdr_list_data(sdr, elt);
		sdr_string_read(sdr, cursor, recordObj);

		msgPtr[*numMsgs] = cursor;
		len = istrlen(cursor, SDRSTRING_BUFSZ);
		cursor += len + 1;
		bufLen -= len + 1;
		if (bufLen < 0)
		{
			putErrmsg("Buffer too small for all log records.", NULL);
			return;
		}

            	(*numMsgs)++;
	}
	sdr_exit_xn(sdr);
}

//void	armurnm_image_get(NmArmurImage *buf)
//{
//
//}
