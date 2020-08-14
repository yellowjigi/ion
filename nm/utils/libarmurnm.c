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

//void	armurnm_image_get(NmArmurImage *buf)
//{
//
//}
