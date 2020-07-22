// 2020-07-22 --- initial implementation
// jigi

#include "armur.h"
#include "armur_restart.h"
#include <archive.h>
#include <archive_entry.h>

/*	*	*	Utility functions	*	*	*/

static Object _armurdbObject(Object *newDbObj)
{
	static Object obj = 0;

	if (newDbObj)
	{
		obj = *newDbObj;
	}

	return obj;
}

static ARMUR_DB *_armurConstants()
{
	static ARMUR_DB	buf;
	static ARMUR_DB *db = NULL;
	Sdr		sdr;
	Object		dbObject;

	if (db == NULL)
	{
		/*	Load constants into a conveniently accessed
		 *	structure. Note that this CANNOT be treated
		 *	as a current database image in later
		 *	processing.					*/

		sdr = getIonsdr();
		CHKNULL(sdr);
		dbObject = _armurdbObject(NULL);
		if (dbObject)
		{
			if (sdr_heap_is_halted(sdr))
			{
				sdr_read(sdr, (char *)&buf, dbObject, sizeof(ARMUR_DB));
			}
			else
			{
				CHKNULL(sdr_begin_xn(sdr));
				sdr_read(sdr, (char *)&buf, dbObject, sizeof(ARMUR_DB));
				sdr_exit_xn(sdr);
			}

			db = &buf;
		}
	}

	return db;
}

static void	dropImageRef(PsmAddress imageRefElt)
{
	PsmPartition	wm = getIonwm();
	PsmAddress	imageRefAddr;

	imageRefAddr = sm_list_data(wm, imageRefElt);
	oK(sm_list_delete(wm, imageRefElt, NULL, NULL));
	psm_free(wm, imageRefAddr);
}

static char *_armurvdbName()
{
	return "armurvdb";
}

static ARMUR_VDB *_armurvdb(char **name)
{
	static ARMUR_VDB	*vdb = NULL;
	PsmPartition		wm;
	PsmAddress		vdbAddress;
	PsmAddress		elt;
	Sdr			sdr;
	ARMUR_DB		*db;

	if (name)
	{
		if (*name == NULL)
		{
			vdb = NULL;
			return vdb;
		}

		/*	Attaching to volatile database.			*/

		wm = getIonwm();
		if (psm_locate(wm, *name, &vdbAddress, &elt) < 0)
		{
			putErrmsg("Failed searching for vdb.", NULL);
			return vdb;
		}

		if (elt)
		{
			vdb = (ARMUR_VDB *)psp(wm, vdbAddress);
			return vdb;
		}

		/*	ARMUR volatile database doesn't exist yet.	*/

		sdr = getIonsdr();
		CHKNULL(sdr_begin_xn(sdr));
		vdbAddress = psm_zalloc(wm, sizeof(ARMUR_VDB));
		if (vdbAddress == 0)
		{
			sdr_exit_xn(sdr);
			putErrmsg("No space for volatile database.", NULL);
			return NULL;
		}

		db = _armurConstants();
		vdb = (ARMUR_VDB *)psp(wm, vdbAddress);
		memset((char *)vdb, 0, sizeof(ARMUR_VDB));

		/*	restartMask is always initialized to 0 in volatile database.
		 *	This enables no restart when unexpected system reboot occurs.	*/
		vdb->restartMask = 0;

		vdb->cfdpInfo = db->cfdpInfo;
		vdb->nmagentPid = ERROR;

		/*	Prepare queues to retain references of restart-pending images	*/
		if ((vdb->restartQueue[ARMUR_LV0] = sm_list_create(wm)) == 0
		|| (vdb->restartQueue[ARMUR_LV1] = sm_list_create(wm)) == 0
		|| (vdb->restartQueue[ARMUR_LV2] = sm_list_create(wm)) == 0
		|| psm_catlg(wm, *name, vdbAddress) < 0)
		{
			psm_free(wm, vdbAddress);
			sdr_exit_xn(sdr);
			putErrmsg("Can't initialize volatile database.", NULL);
			return NULL;
		}

		sdr_exit_xn(sdr);
	}

	return vdb;
}

static char *_armurdbName()
{
	return "armurdb";
}

static int	constructRestartFnHashKey(char *buffer, char *imageName)
{
	memset(buffer, 0, ARMUR_RESTARTFN_HASH_KEY_BUFLEN);
	isprintf(buffer, ARMUR_RESTARTFN_HASH_KEY_BUFLEN, "%s", imageName);
	return strlen(buffer);
}

static int	catalogueRestartFn(Object restartFns,
			ARMUR_RestartFn *restartFn, Object restartFnObj)
{
	Sdr			sdr = getIonsdr();
	char			restartFnKey[ARMUR_RESTARTFN_HASH_KEY_BUFLEN];
	Address			restartFnSetObj;
	ARMUR_RestartFnSet	restartFnSet;
	Object			hashElt;
	int			result = 0;

	if (constructRestartFnHashKey(restartFnKey, restartFn->imageName)
		> ARMUR_RESTARTFN_HASH_KEY_LEN)
	{
		writeMemoNote("[?] Max hash key length exceeded", restartFnKey);
		return -1;
	}

	/*	If a hash table entry for this key already exists,
	 *	then we've got a non-unique key and no single restartFn
	 *	address can be associated with this key. So our first
	 *	step is to retrieve that entry if it exists. If we
	 *	find it, we set its restartFnObj to zero and add 1
	 *	to its count. If not, we insert a new entry.		*/

	switch (sdr_hash_retrieve(sdr, restartFns, restartFnKey,
			&restartFnSetObj, &hashElt))
	{
	case -1:
		putErrmsg("Can't revise hash table entry.", NULL);
		result = -1;
		break;

	case 1:		/*	Retrieval succeeded, non-unique key.	*/
		sdr_stage(sdr, (char *)&restartFnSet, restartFnSetObj,
				sizeof(ARMUR_RestartFnSet));
		restartFnSet.restartFnObj = 0;
		restartFnSet.count++;
		sdr_write(sdr, restartFnSetObj, (char *)&restartFnSet,
				sizeof(ARMUR_RestartFnSet));
		restartFn->hashEntry = hashElt;
		break;

	default:	/*	No such pre-existing entry.		*/
		restartFnSetObj = sdr_malloc(sdr, sizeof(ARMUR_RestartFnSet));
		if (restartFnSetObj == 0)
		{
			putErrmsg("Can't create hash table entry.", NULL);
			result = -1;
			break;
		}

		restartFnSet.restartFnObj = restartFnObj;
		restartFnSet.count = 1;
		sdr_write(sdr, restartFnSetObj, (char *)&restartFnSet,
				sizeof(ARMUR_RestartFnSet));
		if (sdr_hash_insert(sdr, restartFns, restartFnKey,
			restartFnSetObj, &(restartFn->hashEntry)) < 0)
		{
			putErrmsg("Can't insert into hash table.", NULL);
			result = -1;
		}
	}

	return result;
}

static int	restartFnInit(Object hashTable, char *imageName, RestartFn restartFn)
{
	Sdr			sdr = getIonsdr();
	ARMUR_RestartFn		restartFnBuf;
	Object			restartFnObj;

	restartFnObj = sdr_malloc(sdr, sizeof(ARMUR_RestartFn));
	if (restartFnObj == 0)
	{
		return -1;
	}

	memset((char *)&restartFnBuf, 0, sizeof(ARMUR_RestartFn));
	istrcpy(restartFnBuf.imageName, imageName, sizeof(restartFnBuf.imageName));
	restartFnBuf.restart = restartFn;

	sdr_write(sdr, restartFnObj, (char *)&restartFnBuf, sizeof(ARMUR_RestartFn));
	if (catalogueRestartFn(hashTable, &restartFnBuf, restartFnObj) < 0)
	{
		sdr_free(sdr, restartFnObj);
		return -1;
	}

	return 0;
}

static int	_restartFnInit(Object hashTable)
{
	/*		Restart functions		*/

	/*	Core & protocol restart functions	*/
	if (restartFnInit(hashTable, "libici.so", ionRestart)	< 0
	|| restartFnInit(hashTable, "libltp.so", ltpRestart)	< 0
	|| restartFnInit(hashTable, "libbp.so", bpRestart)	< 0
	|| restartFnInit(hashTable, "libcfdp.so", cfdpRestart)	< 0
	/*	Daemon-applications restart functions	*/
	|| restartFnInit(hashTable, "rfxclock", rfxclockRestart)	< 0
	|| restartFnInit(hashTable, "ltpclock", ltpclockRestart)	< 0
	|| restartFnInit(hashTable, "ltpdeliv", ltpdelivRestart)	< 0
	|| restartFnInit(hashTable, "ltpmeter", ltpmeterRestart)	< 0
	|| restartFnInit(hashTable, "udplsi", udplsiRestart)		< 0
	|| restartFnInit(hashTable, "udplso", udplsoRestart)		< 0
	|| restartFnInit(hashTable, "ltpcli", ltpcliRestart)		< 0
	|| restartFnInit(hashTable, "ltpclo", ltpcloRestart)		< 0
	|| restartFnInit(hashTable, "bpclock", bpclockRestart)		< 0
	|| restartFnInit(hashTable, "bptransit", bptransitRestart)	< 0
	|| restartFnInit(hashTable, "ipnfw", ipnfwRestart)		< 0
	|| restartFnInit(hashTable, "ipnadminep", ipnadminepRestart)	< 0
	|| restartFnInit(hashTable, "bpclm", bpclmRestart)		< 0
	|| restartFnInit(hashTable, "bputa", bputaRestart)		< 0
	|| restartFnInit(hashTable, "cfdpclock", cfdpclockRestart)	< 0)
	{
		return -1;
	}

	return 0;
}

int	armurInit()
{
	Sdr		sdr;
	ARMUR_DB	armurdbBuf;
	Object		armurdbObject;
	ARMUR_CfdpInfo	cfdpInfoInit;
	char 		*armurvdbName = _armurvdbName();

	if (ionAttach() < 0)
	{
		putErrmsg("ARMUR can't attach to ION.", NULL);
		return -1;
	}

	sdr = getIonsdr();

	/*	Recover the ARMUR database, creating it if necessary.	*/

	CHKERR(sdr_begin_xn(sdr));
	armurdbObject = sdr_find(sdr, _armurdbName(), NULL);
	switch (armurdbObject)
	{
	case -1:		/*	SDR error.			*/
		sdr_cancel_xn(sdr);
		putErrmsg("Can't search for ARMUR database in SDR.", NULL);
		return -1;

	case 0:			/*	Not found; must create new DB.	*/
		armurdbObject = sdr_malloc(sdr, sizeof(ARMUR_DB));
		if (armurdbObject == 0)
		{
			sdr_cancel_xn(sdr);
			putErrmsg("No space for database.", NULL);
			return -1;
		}

		/*	Initialize the non-volatile database.		*/

		memset((char *)&armurdbBuf, 0, sizeof(ARMUR_DB));
		armurdbBuf.stat = ARMUR_STAT_IDLE;

		/*		Default paths				*/
		armurdbBuf.installPath[ARMUR_LIB] =
			sdr_string_create(sdr, ARMUR_LIBPATH_DEFAULT);
		armurdbBuf.installPath[ARMUR_APP] =
			sdr_string_create(sdr, ARMUR_APPPATH_DEFAULT);

		armurdbBuf.restartFns = sdr_hash_create(sdr,
					ARMUR_RESTARTFN_HASH_KEY_LEN,
					ARMUR_RESTARTFN_HASH_ENTRIES,
					ARMUR_RESTARTFN_HASH_SEARCH_LEN);
		if (armurdbBuf.restartFns)
		{
			if (_restartFnInit(armurdbBuf.restartFns) < 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("Can't initialize restart functions.", NULL);
			}
		}

		armurdbBuf.images[ARMUR_LIB][armurGetProtocolIndex(ARMUR_CORE)]
			= sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_LIB][armurGetProtocolIndex(ARMUR_LTP)]
			= sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_LIB][armurGetProtocolIndex(ARMUR_BP)]
			= sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_LIB][armurGetProtocolIndex(ARMUR_CFDP)]
			= sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_APP][armurGetProtocolIndex(ARMUR_CORE)]
			= sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_APP][armurGetProtocolIndex(ARMUR_LTP)]
			= sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_APP][armurGetProtocolIndex(ARMUR_BP)]
			= sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_APP][armurGetProtocolIndex(ARMUR_CFDP)]
			= sdr_list_create(sdr);

		armurdbBuf.cfdpInfo = sdr_malloc(sdr, sizeof(ARMUR_CfdpInfo));
		if (armurdbBuf.cfdpInfo)
		{
			memset((char *)&cfdpInfoInit, 0, sizeof(ARMUR_CfdpInfo));
			sdr_write(sdr, armurdbBuf.cfdpInfo,
				(char *)&cfdpInfoInit, sizeof(ARMUR_CfdpInfo));
		}

		/*		Write to SDR and catalogue it		*/
		sdr_write(sdr, armurdbObject, (char *)&armurdbBuf, sizeof(ARMUR_DB));
		sdr_catlg(sdr, _armurdbName(), 0, armurdbObject);

		if (sdr_end_xn(sdr))
		{
			putErrmsg("Can't create ARMUR database.", NULL);
			return -1;
		}
		
		break;

	default:		/*	Found DB in the SDR.		*/
		sdr_exit_xn(sdr);
	}

	oK(_armurdbObject(&armurdbObject));
	oK(_armurConstants());

	/*	Load volatile database, initializing as necessary.	*/

	if (_armurvdb(&armurvdbName) == NULL)
	{
		putErrmsg("ARMUR can't initialize vdb.", NULL);
		return -1;
	}

	return 0;		/*	ARMUR service is now available.	*/
}

static void	dropVdb(PsmPartition wm, PsmAddress vdbAddress)
{
	ARMUR_VDB	*vdb;
	PsmAddress	elt;
	int		level;

	vdb = (ARMUR_VDB *)psp(wm, vdbAddress);
	for (level = 0; level < 3; level++)
	{
		while ((elt = sm_list_first(wm, vdb->restartQueue[level])) != 0)
		{
			dropImageRef(elt);
		}
		sm_list_destroy(wm, vdb->restartQueue[level], NULL, NULL);
	}
}

void	armurDropVdb()
{
	PsmPartition	wm = getIonwm();
	char		*armurvdbName = _armurvdbName();
	PsmAddress	vdbAddress;
	PsmAddress	elt;
	char		*stop = NULL;

	if (psm_locate(wm, armurvdbName, &vdbAddress, &elt) < 0)
	{
		putErrmsg("Failed searching for vdb.", NULL);
		return;
	}

	if (elt)
	{
		dropVdb(wm, vdbAddress);	/*	Destroy Vdb.	*/
		psm_free(wm, vdbAddress);
		if (psm_uncatlg(wm, armurvdbName) < 0)
		{
			putErrmsg("Failed uncataloging vdb.", NULL);
		}
	}

	oK(_armurvdb(&stop));			/*	Forget old Vdb.	*/
}

void	armurRaiseVdb()
{
	char	*armurvdbName = _armurvdbName();

	if (_armurvdb(&armurvdbName) == NULL)	/*	Create new Vdb.	*/
	{
		putErrmsg("ARMUR can't reinitialize vdb.", NULL);
	}
}

int	armurStart(char *nmagentCmd)
{
	Sdr		sdr = getIonsdr();
	Object		armurdbObj = _armurdbObject(NULL);
	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
	ARMUR_DB	armurdbBuf;
	ARMUR_CfdpInfo	cfdpInfoBuf;
	char		cmdBuf[SDRSTRING_BUFSZ];

	CHKERR(sdr_begin_xn(sdr));
	if (nmagentCmd)
	{
		if (strlen(nmagentCmd) > MAX_SDRSTRING)
		{
			return -1;
		}

		sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		armurdbBuf.nmagentCmd = sdr_string_create(sdr, nmagentCmd);
		sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));
		if (sdr_end_xn(sdr))
		{
			return -1;
		}
	}
	else
	{
		sdr_read(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		sdr_exit_xn(sdr);
	}

	if (armurdbBuf.nmagentCmd == 0)
	{
		/*	It's assumed that ARMUR has never yet been started by
		 *	an AMP message. There is nothing to do.				*/
		return 0;
	}

	switch ((_armurConstants())->stat)
	{
	case ARMUR_STAT_IDLE:
		/*	Update the ARMUR stat and go to the next step.			*/

		CHKERR(sdr_begin_xn(sdr));
		sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		armurdbBuf.stat = ARMUR_STAT_DOWNLOADING;
		sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));
		if (sdr_end_xn(sdr))
		{
			return -1;
		}

	case ARMUR_STAT_DOWNLOADING:
		/*	Archive is being downloaded.	*/

		if (armurWait() < 0)
		{
			putErrmsg("ARMUR wait failed.", NULL);
			return -1;
		}
		/*	Download has been finished.	*/

		CHKERR(sdr_begin_xn(sdr));
		sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		armurdbBuf.stat = ARMUR_STAT_DOWNLOADED;
		sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));

		sdr_stage(sdr, (char *)&cfdpInfoBuf,
				armurvdb->cfdpInfo, sizeof(ARMUR_CfdpInfo));
		cfdpInfoBuf.srcNbr = 0;
		cfdpInfoBuf.txnNbr = 0;
		sdr_write(sdr, armurvdb->cfdpInfo,
				(char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));
		if (sdr_end_xn(sdr))
		{
			return -1;
		}

	case ARMUR_STAT_DOWNLOADED:
		/*	Start install procedure.	*/

		if (armurInstall() < 0)
		{
			putErrmsg("ARMUR install failed.", NULL);
			return -1;
		}
		/*	Install has been finished.	*/

		CHKERR(sdr_begin_xn(sdr));
		sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		armurdbBuf.stat = ARMUR_STAT_INSTALLED;
		sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));

		sdr_stage(sdr, (char *)&cfdpInfoBuf,
				armurvdb->cfdpInfo, sizeof(ARMUR_CfdpInfo));
		sdr_free(sdr, cfdpInfoBuf.archiveName);
		cfdpInfoBuf.archiveName = 0;
		sdr_write(sdr, armurvdb->cfdpInfo,
				(char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));
		if (sdr_end_xn(sdr))
		{
			return -1;
		}

	case ARMUR_STAT_INSTALLED:
		/*	Start restart procedure.	*/

		if (armurRestart() < 0)
		{
			putErrmsg("ARMUR restart failed.", NULL);
			return -1;
		}
		/*	Restart has been finished.	*/

		CHKERR(sdr_begin_xn(sdr));
		sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		armurdbBuf.stat = ARMUR_STAT_IDLE;
		sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));
		if (sdr_end_xn(sdr))
		{
			return -1;
		}
	}

	CHKERR(sdr_begin_xn(sdr));
	sdr_string_read(sdr, cmdBuf, armurdbBuf.nmagentCmd);
	sdr_exit_xn(sdr);

	oK(nmagentRestart(cmdBuf));

	return 0;
}

int armurAttach()
{
	Object		armurdbObject = _armurdbObject(NULL);
	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
	Sdr		sdr;
	char		*armurvdbName = _armurvdbName();

	if (armurdbObject && armurvdb)
	{
		/*	Already attached.	*/
		return 0;
	}

	if (ionAttach() < 0)
	{
		putErrmsg("ARMUR can't attach to ION.", NULL);
		return -1;
	}

	sdr = getIonsdr();

	if (armurdbObject == 0)
	{
		CHKERR(sdr_begin_xn(sdr));
		armurdbObject = sdr_find(sdr, _armurdbName(), NULL);
		sdr_exit_xn(sdr);
		if (armurdbObject == 0)
		{
			putErrmsg("Can't find ARMUR database.", NULL);
			return -1;
		}

		oK(_armurdbObject(&armurdbObject));
	}

	oK(_armurConstants());

	/*	Locate the ARMUR volatile database.			*/

	if (armurvdb == NULL)
	{
		if (_armurvdb(&armurvdbName) == NULL)
		{
			putErrmsg("ARMUR volatile database not found.", NULL);
			return -1;
		}
	}

	return 0;
}

void armurDetach()
{
	char	*stop = NULL;

	oK(_armurvdb(&stop));
}

Object getArmurDbObject()
{
	return _armurdbObject(NULL);
}

ARMUR_DB *getArmurConstants()
{
	return _armurConstants();
}

ARMUR_VDB *getArmurVdb()
{
	return _armurvdb(NULL);
}

void	armurParseImageName(char *imageName, int *type, int *level)
{
	if (strcmp(imageName + strlen(imageName) - 3, ".so") == 0)
	{
		if (strcmp(imageName, "libici.so") == 0)
		{
			if (level)
			{
				*level = ARMUR_LV0;
			}
		}
		else
		{
			if (level)
			{
				*level = ARMUR_LV1;
			}
		}
		*type = ARMUR_LIB;
		return;
	}

	if (level)
	{
		*level = ARMUR_LV2;
	}
	*type = ARMUR_APP;
}

int	armurGetProtocolIndex(char protocolFlag)
{
	int	flag;
	int	idx = 0;

	for (flag = protocolFlag >> 1; flag; flag >>= 1)
	{
		idx++;
	}

	return idx;
}

int	armurFindRestartFn(char *imageName, Object *restartFnObj)
{
	Sdr			sdr = getIonsdr();
	char			key[ARMUR_RESTARTFN_HASH_KEY_BUFLEN];
	Address			restartFnSetObj;
	ARMUR_RestartFnSet	restartFnSet;
	Object			hashElt;

	CHKERR(imageName);
	CHKERR(restartFnObj);

	CHKERR(ionLocked());
	*restartFnObj = 0;	/*	Default: not found.		*/
	if (constructRestartFnHashKey(key, imageName) > ARMUR_RESTARTFN_HASH_KEY_LEN)
	{
		return 0;	/*	Can't be in hash table.		*/
	}

	switch (sdr_hash_retrieve(sdr, (_armurConstants())->restartFns, key,
			&restartFnSetObj, &hashElt))
	{
	case -1:
		putErrmsg("Failed locating restart function in hash table.", NULL);
		return -1;

	case 0:
		return 0;	/*	No such entry in hash table.	*/

	default:
		sdr_read(sdr, (char *)&restartFnSet, restartFnSetObj,
				sizeof(ARMUR_RestartFnSet));
		*restartFnObj = restartFnSet.restartFnObj;
		return restartFnSet.count;
	}
}

static Object	locateImage(char *imageName, int imageType)
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	*armurdb = _armurConstants();
			OBJ_POINTER(ARMUR_Image, image);
	Object		elt;
	int		protocol;
	int		protocolIdx;
	int		eltFound = 0;

	CHKZERO(imageName);
	CHKZERO(imageType == ARMUR_LIB || imageType == ARMUR_APP);

	CHKZERO(ionLocked());
	for (protocol = ARMUR_CORE; protocol < ARMUR_RESERVED; protocol <<= 1)
	{
		protocolIdx = armurGetProtocolIndex(protocol);
		for (elt = sdr_list_first(sdr, armurdb->images[imageType][protocolIdx]);
			elt; elt = sdr_list_next(sdr, elt))
		{
			GET_OBJ_POINTER(sdr, ARMUR_Image, image, sdr_list_data(sdr, elt));
			if (strcmp(image->name, imageName) == 0)
			{
				eltFound = 1;
				break;
			}
		}
		if (eltFound)
		{
			break;
		}
	}

	return elt;
}

void	armurFindImage(char *imageName, int imageType, Object *imageObj, Object *imageElt)
{
	Sdr	sdr = getIonsdr();
	Object	elt;

	CHKVOID(imageName);
	CHKVOID(imageType == ARMUR_LIB || imageType == ARMUR_APP);
	CHKVOID(imageObj);
	CHKVOID(imageElt);

	CHKVOID(ionLocked());
	elt = locateImage(imageName, imageType);
	if (elt == 0)
	{
		*imageElt = 0;
		return;
	}

	*imageObj = sdr_list_data(sdr, elt);
	*imageElt = elt;
}

static int	enqueueImage(Object imageObj, int level)
{
	PsmPartition	wm = getIonwm();
	ARMUR_ImageRef	*imageRef;
	PsmAddress	addr;
	PsmAddress	elt;

	CHKERR(imageObj);
	CHKERR(level == ARMUR_LV0 || level == ARMUR_LV1 || level == ARMUR_LV2);

	CHKERR(ionLocked());
	addr = psm_malloc(wm, sizeof(ARMUR_ImageRef));
	if (addr == 0)
	{
		putErrmsg("No space for working memory.", NULL);
		return -1;
	}

	elt = sm_list_insert_last(wm, (_armurvdb(NULL))->restartQueue[level], addr);
	if (elt == 0)
	{
		psm_free(wm, addr);
		putErrmsg("No space for working memory.", NULL);
		return -1;
	}

	imageRef = (ARMUR_ImageRef *)psp(wm, addr);
	imageRef->imageObj = imageObj;

	return 0;
}

int	armurAddImage(char *imageName, int protocol)
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	*armurdb = _armurConstants();
	ARMUR_Image	imageBuf;
	Object		imageObj;
	Object		imageElt;
	Object		restartFnObj;
	Object		obj;
	Object		elt = 0;
	int		level;
	int		imageType;
	int		p;
	int		i;

	CHKERR(imageName);
	CHKERR(protocol);

	if (*imageName == 0)
	{
		writeMemoNote("[?] Zero-length image name", imageName);
		return 0;
	}

	if (strlen(imageName) > ARMUR_FILENAME_LEN_MAX - 1)
	{
		writeMemoNote("[?] Image name is too long", imageName);
		return 0;
	}

	if (protocol >= ARMUR_RESERVED)
	{
		writeMemoNote("[?] Protocol not supported", itoa(protocol));
		return 0;
	}

	CHKERR(sdr_begin_xn(sdr));
	armurParseImageName(imageName, &imageType, &level);
	armurFindImage(imageName, imageType, &imageObj, &imageElt);
	if (imageElt != 0)
	{
		sdr_exit_xn(sdr);
		writeMemoNote("[?] Duplicate image", imageName);
		return 0;
	}

	memset((char *)&imageBuf, 0, sizeof(ARMUR_Image));
	istrcpy(imageBuf.name, imageName, sizeof imageBuf.name);
	imageBuf.type = imageType;
	imageBuf.protocol = protocol;

	oK(armurFindRestartFn(imageName, &restartFnObj));
	if (restartFnObj)
	{
		imageBuf.restartFnObj = restartFnObj;
	}

	obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
	if (obj)
	{
		for (p = ARMUR_CORE; p < ARMUR_RESERVED; p <<= 1)
		{
			if (p & protocol)
			{
				i = armurGetProtocolIndex(p);
				elt = sdr_list_insert_last(sdr,
					armurdb->images[imageType][i], obj);
			}
		}
		sdr_write(sdr, obj, (char *)&imageBuf, sizeof(ARMUR_Image));
	}
	/*	TODO: Increment the number of images installed.		*/
	if (sdr_end_xn(sdr) < 0 || elt == 0)
	{
		putErrmsg("Can't add image.", imageName);
		return -1;
	}

	return 0;
}

static void	setInstallTimestamp(Object imageObj, time_t installedTime)
{
	Sdr		sdr = getIonsdr();
	ARMUR_Image	imageBuf;

	CHKVOID(imageObj);
	CHKVOID(installedTime);

	CHKVOID(ionLocked());
	sdr_stage(sdr, (char *)&imageBuf, imageObj, sizeof(ARMUR_Image));
	imageBuf.installedTime = installedTime;
	sdr_write(sdr, imageObj, (char *)&imageBuf, sizeof(ARMUR_Image));
}

static void	getInstallDir(int type, char *installDir)
{
	Sdr	sdr = getIonsdr();
	char	buf[SDRSTRING_BUFSZ];

	CHKVOID(type == ARMUR_LIB || type == ARMUR_APP);
	CHKVOID(installDir);

	CHKVOID(ionLocked());
	sdr_string_read(sdr, buf, (_armurConstants())->installPath[type]);

	istrcpy(installDir, buf, ARMUR_PATHNAME_LEN_MAX);
}

int	armurUpdateCfdpSrcNbr(uvast cfdpSrcNbr)
{
	Sdr		sdr = getIonsdr();
	ARMUR_CfdpInfo	cfdpInfoBuf;
	Object		cfdpInfoObj;
	
	CHKERR(sdr_begin_xn(sdr));

	cfdpInfoObj = (_armurvdb(NULL))->cfdpInfo;
	sdr_stage(sdr, (char *)&cfdpInfoBuf, cfdpInfoObj, sizeof(ARMUR_CfdpInfo));
	cfdpInfoBuf.srcNbr = cfdpSrcNbr;
	sdr_write(sdr, cfdpInfoObj, (char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));

	return sdr_end_xn(sdr);
}

int	armurUpdateCfdpTxnNbr(uvast cfdpTxnNbr)
{
	Sdr		sdr = getIonsdr();
	ARMUR_CfdpInfo	cfdpInfoBuf;
	Object		cfdpInfoObj;
	
	CHKERR(sdr_begin_xn(sdr));

	cfdpInfoObj = (_armurvdb(NULL))->cfdpInfo;
	sdr_stage(sdr, (char *)&cfdpInfoBuf, cfdpInfoObj, sizeof(ARMUR_CfdpInfo));
	cfdpInfoBuf.txnNbr = cfdpTxnNbr;
	sdr_write(sdr, cfdpInfoObj, (char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));

	return sdr_end_xn(sdr);
}

int	armurUpdateCfdpArchiveName(char *archiveName)
{
	Sdr		sdr = getIonsdr();
	ARMUR_CfdpInfo	cfdpInfoBuf;
	Object		cfdpInfoObj;
	
	CHKERR(sdr_begin_xn(sdr));

	cfdpInfoObj = (_armurvdb(NULL))->cfdpInfo;
	sdr_stage(sdr, (char *)&cfdpInfoBuf, cfdpInfoObj, sizeof(ARMUR_CfdpInfo));
	cfdpInfoBuf.archiveName = sdr_string_create(sdr, archiveName);
	sdr_write(sdr, cfdpInfoObj, (char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));

	return sdr_end_xn(sdr);
}

int	armurWait()
{
	CfdpEventType		type;
	time_t			time;
	int			reqNbr;
	CfdpTransactionId	transactionId;
	char			sourceFileNameBuf[256];
	char			destFileNameBuf[256];
	uvast			fileSize;
	MetadataList		messagesToUser;
	uvast			offset;
	unsigned int		length;
	unsigned int		recordBoundsRespected;
	CfdpContinuationState	continuationState;
	unsigned int		segMetadataLength;
	char			segMetadata[63];
	CfdpCondition		condition;
	uvast			progress;
	CfdpFileStatus		fileStatus;
	CfdpDeliveryCode	deliveryCode;
	CfdpTransactionId	originatingTransactionId;
	char			statusReportBuf[256];
	MetadataList		filestoreResponses;
	uvast			srcNbr;
	uvast			txnNbr;
	Sdr			sdr = getIonsdr();
	CfdpDB			*cfdpdb;
	Object			elt;
				OBJ_POINTER(ARMUR_CfdpInfo, cfdpInfo);

	/*	Check first to see if cfdpInfo has been stored in the ARMUR DB.		*/
	CHKERR(sdr_begin_xn(sdr));
	GET_OBJ_POINTER(sdr, ARMUR_CfdpInfo, cfdpInfo, (_armurvdb(NULL))->cfdpInfo);
	sdr_exit_xn(sdr);

	CHKERR(cfdpInfo->srcNbr);
	//CHKERR(cfdpInfo->txnNbr);//JIGI

	if (cfdpAttach() < 0)
	{
		return -1;
	}

	cfdpdb = getCfdpConstants();

	while (1)
	{
		/*	Nested SDR Txn to preserve CFDP event queue
		 *	in case of unexpected system crash and reboot.		*/
		CHKERR(sdr_begin_xn(sdr));

		/*	To avoid deadlock.					*/
		elt = sdr_list_first(sdr, cfdpdb->events);
		if (elt == 0)
		{
			sdr_exit_xn(sdr);
			continue;
		}
		if (cfdp_get_event(&type, &time, &reqNbr, &transactionId,
				sourceFileNameBuf, destFileNameBuf,
				&fileSize, &messagesToUser, &offset, &length,
				&recordBoundsRespected, &continuationState,
				&segMetadataLength, segMetadata,
				&condition, &progress, &fileStatus,
				&deliveryCode, &originatingTransactionId,
				statusReportBuf, &filestoreResponses) < 0)
		{
			return -1;
		}

		/* TODO: we need to check if any file data have been corrupted
		 * or something that might cause the cfdp_get_event to block infinitely.
		 * Implement it as a thread ?						*/

		cfdp_decompress_number(&srcNbr, &transactionId.sourceEntityNbr);
		cfdp_decompress_number(&txnNbr, &transactionId.transactionNbr);

		if (type == CfdpTransactionFinishedInd && srcNbr == cfdpInfo->srcNbr)
			//&& txnNbr == cfdpInfo->txnNbr)//JIGI
		{
			/*	Now the download has been finished.	*/
			printf("Download has been completed.\n");//dbg
			if (sdr_end_xn(sdr))
			{
				return -1;
			}
			break;
		}

		if (sdr_end_xn(sdr))
		{
			return -1;
		}
	}

	return 0;
}

int	armurInstall()
{
	Sdr			sdr = getIonsdr();
				OBJ_POINTER(ARMUR_Image, image);
				OBJ_POINTER(ARMUR_CfdpInfo, cfdpInfo);
	ARMUR_VDB		*armurvdb = _armurvdb(NULL);
	Object			imageObj;
	Object			imageElt;
	int			imageType;
	int			imageLevel;
	char			archiveNameBuf[SDRSTRING_BUFSZ];
	char			imageName[ARMUR_FILENAME_LEN_MAX];
	char			pathName[ARMUR_PATHNAME_LEN_MAX];
	char			pathNameTmp[ARMUR_PATHNAME_LEN_MAX];
	char			installDir[ARMUR_PATHNAME_LEN_MAX];
	struct archive		*a;
	struct archive_entry	*entry;
	int			result;
	time_t			installedTime = getCtime();

	CHKERR(sdr_begin_xn(sdr));
	GET_OBJ_POINTER(sdr, ARMUR_CfdpInfo, cfdpInfo, armurvdb->cfdpInfo);
	CHKERR(cfdpInfo->archiveName);
	sdr_string_read(sdr, archiveNameBuf, cfdpInfo->archiveName);
	sdr_exit_xn(sdr);

	/*	LIBARCHIVE APIs			*/
	if ((a = archive_read_new()) == NULL)
	{
		return -1;
	}

	if (archive_read_support_filter_gzip(a) != ARCHIVE_OK
	|| archive_read_support_format_tar(a) != ARCHIVE_OK)
	{
		archive_read_free(a);
		return -1;
	}

	if (archive_read_open_filename(a, archiveNameBuf, 0) != ARCHIVE_OK)
	{
		archive_read_free(a);
		return -1;
	}

	while ((result = archive_read_next_header(a, &entry)) != ARCHIVE_EOF)
	{
		if (result != ARCHIVE_OK)
		{
			archive_read_free(a);
			return -1;
		}

		istrcpy(imageName, archive_entry_pathname(entry), sizeof imageName);

		CHKERR(sdr_begin_xn(sdr));
		armurParseImageName(imageName, &imageType, &imageLevel);
		armurFindImage(imageName, imageType, &imageObj, &imageElt);
		if (imageElt == 0)
		{
			/*	TODO: Append error message	*/
			sdr_exit_xn(sdr);
			archive_read_free(a);
			return -1;
		}

		/*	Retain the image reference in the restart queue.	*/
		if (enqueueImage(imageObj, imageLevel) < 0)
		{
			/*	TODO: Append error message	*/
			//putErrmsg("Can't enqueue image", imageName);
			sdr_exit_xn(sdr);
			archive_read_free(a);
			return -1;
		}

		GET_OBJ_POINTER(sdr, ARMUR_Image, image, imageObj);
		armurvdb->restartMask |= image->protocol;

		getInstallDir(imageType, installDir);
		sdr_exit_xn(sdr);

		isprintf(pathName, sizeof pathName, "%s/%s", installDir, imageName);
		isprintf(pathNameTmp, sizeof pathNameTmp, "%s/%s" TMP_EXT,
				installDir, imageName);
		archive_entry_set_pathname(entry, pathNameTmp);

		/*	Extract the file.					*/
		if (archive_read_extract(a, entry, 0) != ARCHIVE_OK)
		{
			archive_read_free(a);
			return -1;
		}

		/*	Lastly, replace each file.
		 *	(rename in C supports atomic operation.)		*/
		if (rename(pathNameTmp, pathName) != 0)
		{
			archive_read_free(a);
			return -1;
		}

		/*	Replace is successfully finished with this file.
		 *	Now let's update the installedTime of the image.	*/
		CHKERR(sdr_begin_xn(sdr));
		setInstallTimestamp(imageObj, installedTime);
		sdr_end_xn(sdr);

		if (archive_read_data_skip(a) != ARCHIVE_OK)
		{
			archive_read_free(a);
			return -1;
		}
	}
	archive_read_free(a);
	oK(remove(archiveNameBuf));
	printf("Install has been completed.\n");//dbg

	return 0;
}

int	armurRestart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
			OBJ_POINTER(ARMUR_Image, image);
			OBJ_POINTER(ARMUR_RestartFn, restartFn);
	PsmAddress	imageRefAddr;
	PsmAddress	imageRefElt;
	ARMUR_ImageRef	*imageRef;
	int		level;

	CHKERR(sdr_begin_xn(sdr));
	/*	Restart by ARMUR does not have to be done if the restart mask is zero,
	 *	i.e., either 1) if restart queue is empty, 2) if the items have been
	 *	restarted by an unexpected system reboot that reset the restart mask,
	 *	or 3) if SDR recovery procedure has been conducted leading to
	 *	restart of the entire ION and reset of the volatile database.		*/
	if (armurvdb->restartMask == 0)
	{
		sdr_exit_xn(sdr);
		return 0;
	}
	sdr_exit_xn(sdr);

	/*	There ARE some items to be restarted.					*/
	for (level = 0; level < 3; level++)
	{
		CHKERR(sdr_begin_xn(sdr));
		while ((imageRefElt = sm_list_first(wm, armurvdb->restartQueue[level])) != 0)
		{
			/*	Item was found. Let's get the image.			*/
			imageRefAddr = sm_list_data(wm, imageRefElt);
			imageRef = (ARMUR_ImageRef *)psp(wm, imageRefAddr);
			GET_OBJ_POINTER(sdr, ARMUR_Image, image, imageRef->imageObj);

			/*	We will restart applications only if the restart
			 *	mask is not zero, i.e., if the items have not
			 *	yet been restarted.					*/
			if (armurvdb->restartMask == 0)
			{
				/*	We will just delete the item from the queue.	*/
				psm_free(wm, imageRefAddr);
				oK(sm_list_delete(wm, imageRefElt, NULL, NULL));
				sdr_exit_xn(sdr);

				CHKERR(sdr_begin_xn(sdr));	/*	For next loop	*/
				continue;
			}

			if (level == ARMUR_LV2)
			{
				/*	Items in level 2 will be restarted only if
				 *	they have not yet been restarted by the upper
				 *	level (i.e., if the corresponding bits have
				 *	not yet been turned off).			*/
				if ((armurvdb->restartMask & image->protocol)
					!= image->protocol)
				{
					/*	We will just delete the item
					 *	from the queue.				*/
					psm_free(wm, imageRefAddr);
					oK(sm_list_delete(wm, imageRefElt, NULL, NULL));
					sdr_exit_xn(sdr);

					CHKERR(sdr_begin_xn(sdr));/*	For next loop	*/
					continue;
				}
			}

			sdr_exit_xn(sdr);
			GET_OBJ_POINTER(sdr, ARMUR_RestartFn, restartFn,
					image->restartFnObj);
			printf("%s will restart\n", image->name);//dbg
			if (restartFn->restart() < 0)
			{
				/*	TODO: Append error message.			*/
				return -1;
			}

			if (level == ARMUR_LV0 || level == ARMUR_LV1)
			{
				/*	Items in level 0 or 1 will xor the restart mask
				 *	(i.e., will turn off corresponding bit flags).	*/
				CHKERR(sdr_begin_xn(sdr));
				armurvdb->restartMask ^= image->protocol;
				sdr_exit_xn(sdr);
			}

			/*	Delete the item from the queue.				*/
			CHKERR(sdr_begin_xn(sdr));
			psm_free(wm, imageRefAddr);
			oK(sm_list_delete(wm, imageRefElt, NULL, NULL));
			sdr_exit_xn(sdr);

			CHKERR(sdr_begin_xn(sdr));	/*	For next loop		*/
		}
		sdr_exit_xn(sdr);
	}

	CHKERR(sdr_begin_xn(sdr));
	armurvdb->restartMask = 0;
	sdr_exit_xn(sdr);
	printf("Restart has been completed.\n");//dbg

	return 0;
}
