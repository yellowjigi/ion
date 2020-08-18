// 2020-07-22 --- initial implementation
// jigi

#include "armur.h"
//#include "armur_restart.h"
#include "armur_rhht.h"
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

static int	raiseImage(Object imageAddr, ARMUR_VDB *armurvdb)
{
	Sdr			sdr = getIonsdr();
	PsmPartition		wm = getIonwm();
	ARMUR_Image		imageBuf;
	ARMUR_VPackageDescr	*vdescr;
	ARMUR_VImage		*vimage;
	PsmAddress		vimageAddr;
	PsmAddress		addr;
	PsmAddress		elt;
	int			i;
	char			*packageName;

	if ((vimageAddr = psm_malloc(wm, sizeof(ARMUR_VImage))) == 0)
	{
		return -1;
	}

	sdr_read(sdr, (char *)&imageBuf, imageAddr, sizeof(ARMUR_Image));

	/*	Insert the vimage address in the hash table.		*/
	if (ARMUR_rhht_insert(armurvdb->vimages, imageBuf.name, vimageAddr) != RH_OK)
	{
		psm_free(wm, vimageAddr);
		return -1;
	}

	vimage = (ARMUR_VImage *)psp(wm, vimageAddr);
	vimage->addr = imageAddr;

	/*	Walk the packages possibly given in CSV formats.	*/
	packageName = imageBuf.packageName;
	while ((packageName = strtok(packageName, ",")) != NULL)
	{
		armurFindPackageDescr(packageName, &vdescr, &elt);
		if (elt == 0)
		{
			//if (armurAddPackageDescr(packageName) < 0)
			//{
			//	return -1;
			//}
			//armurFindPackageDescr(packageName, &vdescr, &elt);
			//if (elt == 0)
			//{
			//	return -1;
			//}

			/*	Not found. Need to add a new package descriptor.	*/
			if ((addr = psm_malloc(wm, sizeof(ARMUR_VPackageDescr))) == 0)
			{
				return -1;
			}
			if (sm_list_insert_last(wm, armurvdb->applications, addr) == 0)
			{
				ARMUR_rhht_del_key(armurvdb->vimages, imageBuf.name);
				psm_free(wm, addr);
				return -1;
			}

			vdescr = (ARMUR_VPackageDescr *)psp(wm, addr);
			istrcpy(vdescr->name, packageName, sizeof vdescr->name);
			if ((vdescr->applications[ARMUR_APPTYPE_DAEMON] =
				sm_list_create(wm)) == 0
			|| (vdescr->applications[ARMUR_APPTYPE_NORMAL] =
				sm_list_create(wm)) == 0)
			{
				// TODO: release memories.
				return -1;
			}
		}

		/*	Follow with level-specific procedures.			*/
		switch (imageBuf.level)
		{
		case ARMUR_LEVEL_0:
			/*	Populate the imageLv0-specific buf.			*/
			/*	Copy the addresses of the lists of packages.		*/
			for (i = 0; i < ARMUR_LAYERS; i++)
			{
				vimage->as.lv0.packages[i] = armurvdb->packages[i];
			}
			for (i = 0; i < ARMUR_APPTYPES; i++)
			{
				vimage->as.lv0.applications[i] = vdescr->applications[i];
			}
			elt = sm_list_insert_last(wm, armurvdb->corePackages, vimageAddr);
			break;

		case ARMUR_LEVEL_1:
			/*	Populate the imageLv1-specific buf.			*/
			vimage->as.lv1.layer = imageBuf.tag;
			/*	Copy the addresses of the lists of applications.	*/
			for (i = 0; i < ARMUR_APPTYPES; i++)
			{
				vimage->as.lv1.applications[i] = vdescr->applications[i];
			}
			elt = sm_list_insert_last(wm,
				armurvdb->packages[(int)imageBuf.tag], vimageAddr);
			break;

		case ARMUR_LEVEL_2:
			/*	Populate the imageLv2-specific buf.			*/
			vimage->as.lv2.vstat = 0;
			vimage->as.lv2.stop = NULL;
			vimage->as.lv2.start = NULL;
			elt = sm_list_insert_last(wm,
				vdescr->applications[(int)imageBuf.tag], vimageAddr);
		}
		if (elt == 0)
		{
			// TODO: release memories.
			return -1;
		}
		packageName = NULL;
	}

	return 0;
}

static char *_armurvdbName()
{
	return "armurvdb";
}

static ARMUR_VDB *_armurvdb(char **name)
{
	Sdr			sdr;
	static ARMUR_VDB	*vdb = NULL;
	PsmPartition		wm;
	PsmAddress		vdbAddress;
	PsmAddress		elt;
	ARMUR_DB		*db;
	Object			sdrElt;
	int			level;

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

		/*	TODO: Free memory when an error occurs.		*/
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

		vdb->vstat = ARMUR_VSTAT_IDLE;

		/*	Prepare a hash table for vimages.		*/
		/*	TODO: pass the number of buckets.		*/
		if ((vdb->vimages = ARMUR_rhht_create()) == 0)
		{
			sdr_exit_xn(sdr);
			putErrmsg("No space for volatile database.", NULL);
			return NULL;
		}

		/*	Prepare lists of vimages for navigation.	*/
		if ((vdb->corePackages = sm_list_create(wm)) == 0
		|| (vdb->packages[ARMUR_LAYER_APPLICATION] = sm_list_create(wm)) == 0
		|| (vdb->packages[ARMUR_LAYER_BUNDLE] = sm_list_create(wm)) == 0
		|| (vdb->packages[ARMUR_LAYER_CONVERGENCE] = sm_list_create(wm)) == 0
		|| (vdb->applications = sm_list_create(wm)) == 0)
		{
			sdr_exit_xn(sdr);
			putErrmsg("No space for volatile database.", NULL);
			return NULL;
		}

		/*	Prepare lists of restart queues.		*/
		if ((vdb->restartQueue[ARMUR_LEVEL_0] = sm_list_create(wm)) == 0
		|| (vdb->restartQueue[ARMUR_LEVEL_1] = sm_list_create(wm)) == 0
		|| (vdb->restartQueue[ARMUR_LEVEL_2] = sm_list_create(wm)) == 0)
		{
			sdr_exit_xn(sdr);
			putErrmsg("No space for volatile database.", NULL);
			return NULL;
		}

		vdb->cfdpInfo = db->cfdpInfo;
		vdb->nmagentPid = ERROR;

		if (psm_catlg(wm, *name, vdbAddress) < 0)
		{
			sdr_exit_xn(sdr);
			putErrmsg("Can't initialize volatile database.", NULL);
			return NULL;
		}

		/*	Raise all the image instances of lv.0, 1 and 2 from the lists.	*/
		for (level = 0; level < ARMUR_LEVELS; level++)
		{
			for (sdrElt = sdr_list_first(sdr, db->images[level]); sdrElt;
				sdrElt = sdr_list_next(sdr, sdrElt))
			{
				if (raiseImage(sdr_list_data(sdr, sdrElt), vdb) < 0)
				{
					sdr_exit_xn(sdr);
					putErrmsg("Can't raise images.", NULL);
					return NULL;
				}
			}
		}

		sdr_exit_xn(sdr);
	}

	return vdb;
}

static char *_armurdbName()
{
	return "armurdb";
}

int	armurInit()
{
	Sdr			sdr;
	ARMUR_DB		armurdbBuf;
	Object			armurdbObject;
	ARMUR_CfdpInfo		cfdpInfoInit;
	char 			*armurvdbName = _armurvdbName();

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

		/*	Default paths					*/
		armurdbBuf.installPath[ARMUR_IMAGETYPE_LIB] =
			sdr_string_create(sdr, ARMUR_PATH_LIB_DEFAULT);
		armurdbBuf.installPath[ARMUR_IMAGETYPE_APP] =
			sdr_string_create(sdr, ARMUR_PATH_APP_DEFAULT);

		/*	Prepare lists of image templates.		*/
		armurdbBuf.images[ARMUR_LEVEL_0] = sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_LEVEL_1] = sdr_list_create(sdr);
		armurdbBuf.images[ARMUR_LEVEL_2] = sdr_list_create(sdr);

		/*	Prepare space for CFDP information.		*/
		armurdbBuf.cfdpInfo = sdr_malloc(sdr, sizeof(ARMUR_CfdpInfo));
		if (armurdbBuf.cfdpInfo)
		{
			memset((char *)&cfdpInfoInit, 0, sizeof(ARMUR_CfdpInfo));
			sdr_write(sdr, armurdbBuf.cfdpInfo,
				(char *)&cfdpInfoInit, sizeof(ARMUR_CfdpInfo));
		}

		/*	Write to SDR and catalogue it.			*/
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
	ARMUR_VDB		*vdb;
	ARMUR_VPackageDescr	*vdescr;
	PsmAddress		elt;
	PsmAddress		descrElt;
	int			i;

	vdb = (ARMUR_VDB *)psp(wm, vdbAddress);

	/*	Drop the vimages hash table and free the psm memories.		*/
	ARMUR_rhht_release(vdb->vimages);

	/*	Clean the image elements of lv.0 from the core packages.	*/
	while ((elt = sm_list_first(wm, vdb->corePackages)) != 0)
	{
		oK(sm_list_delete(wm, elt, NULL, NULL));
	}
	sm_list_destroy(wm, vdb->corePackages, NULL, NULL);

	/*	Clean the image elements of lv.1 from the packages.		*/
	for (i = 0; i < ARMUR_LAYERS; i++)
	{
		while ((elt = sm_list_first(wm, vdb->packages[i])) != 0)
		{
			oK(sm_list_delete(wm, elt, NULL, NULL));
		}
		sm_list_destroy(wm, vdb->packages[i], NULL, NULL);
	}

	/*	Clean the image elements of lv.2 from the applications.	*/
	while ((descrElt = sm_list_first(wm, vdb->applications)) != 0)
	{
		vdescr = (ARMUR_VPackageDescr *)psp(wm, sm_list_data(wm, descrElt));
		for (i = 0; i < ARMUR_APPTYPES; i++)
		{
			while ((elt = sm_list_first(wm, vdescr->applications[i])) != 0)
			{
				oK(sm_list_delete(wm, elt, NULL, NULL));
			}
			sm_list_destroy(wm, vdescr->applications[i], NULL, NULL);
		}
		oK(sm_list_delete(wm, descrElt, NULL, NULL));
	}
	sm_list_destroy(wm, vdb->applications, NULL, NULL);

	/*	Drop & destroy the restart queues.	*/
	for (i = 0; i < ARMUR_LEVELS; i++)
	{
		while ((elt = sm_list_first(wm, vdb->restartQueue[i])) != 0)
		{
			oK(sm_list_delete(wm, elt, NULL, NULL));
		}
		sm_list_destroy(wm, vdb->restartQueue[i], NULL, NULL);
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
	//ARMUR_CfdpInfo	cfdpInfoBuf;
	char		buf[SDRSTRING_BUFSZ];

	if (nmagentCmd)
	{
		if (strlen(nmagentCmd) > MAX_SDRSTRING)
		{
			putErrmsg("nm_agent command is too long.", nmagentCmd);
			return -1;
		}

		CHKERR(sdr_begin_xn(sdr));
		sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		if (armurdbBuf.nmagentCmd != 0)
		{
			sdr_free(sdr, armurdbBuf.nmagentCmd);
		}
		armurdbBuf.nmagentCmd = sdr_string_create(sdr, nmagentCmd);
		sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));
		if (sdr_end_xn(sdr) < 0)
		{
			putErrmsg("ARMUR can't set nm_agent command.", NULL);
			return -1;
		}
	}
	else
	{
		CHKERR(sdr_begin_xn(sdr));
		sdr_read(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		sdr_exit_xn(sdr);
	}

	CHKERR(sdr_begin_xn(sdr));
	if (armurvdb->nmagentPid == ERROR)
	{
		if (armurdbBuf.nmagentCmd == 0)	/*	No nm_agent command.	*/
		{
			putErrmsg("ARMUR can't start nm_agent.", NULL);
			return -1;
		}

		sdr_string_read(sdr, buf, armurdbBuf.nmagentCmd);
		armurvdb->nmagentPid = pseudoshell(buf);
	}
	sdr_exit_xn(sdr);

	//switch ((_armurConstants())->stat)
	//{
	//case ARMUR_STAT_IDLE:
/*	MIGRATED TO ARMUR_CTRL_WAIT 08/18/2020		*/
	//	if (cfdpAttach() < 0)
	//	{
	//		return -1;
	//	}

	//	CHKERR(sdr_begin_xn(sdr));
	//	if (sdr_list_first(sdr, (getCfdpConstants())->events) == 0)
	//	{
	//		/*	No CFDP PDUs have yet been received
	//		 *	until now. There is nothing to do
	//		 *	and ARMUR is still idle. To avoid
	//		 *	deadlock, we will exit.			*/
	//		sdr_exit_xn(sdr);
	//		return 0;
	//	}
	//	sdr_exit_xn(sdr);

	//	/*	Check CFDP events and block as
	//	 *	necessary.				*/

	//	if (armurWait() < 0)
	//	{
	//		putErrmsg("ARMUR wait failed.", NULL);
	//		return -1;
	//	}
	//	/*	Download has been finished.		*/

	//case ARMUR_STAT_DOWNLOADED:
/*	MIGRATED TO ARMUR_CTRL_INSTALL 08/18/2020	*/
	//	/*	Start install procedure.		*/

	//	if (armurInstall() < 0)
	//	{
	//		putErrmsg("ARMUR install failed.", NULL);
	//		return -1;
	//	}
	//	/*	Install has been finished.		*/

	//case ARMUR_STAT_INSTALLED:
/*	MIGRATED TO ARMUR_CTRL_RESTART 08/18/2020	*/
	//	/*	If we safely arrived here, the
	//	 *	downloaded archive file is no longer
	//	 *	needed. Delete it.			*/
	//	CHKERR(sdr_begin_xn(sdr));
	//	sdr_read(sdr, (char *)&cfdpInfoBuf, armurvdb->cfdpInfo,
	//		sizeof(ARMUR_CfdpInfo));
	//	sdr_string_read(sdr, buf, cfdpInfoBuf.archiveName);
	//	sdr_exit_xn(sdr);
	//	if (fopen(buf, "r") != NULL)
	//	{
	//		oK(remove(buf));
	//	}
	//	/*	Start restart procedure.		*/

	//	if (armurRestart() < 0)
	//	{
	//		putErrmsg("ARMUR restart failed.", NULL);
	//		return -1;
	//	}

	//	/*	Restart has been finished.		*/
	//}

	///*	TODO: Send a report message.		*/

	///*	The entire procedure has been completed.
	// *	Reset the CFDP-related information and state.		*/
	//CHKERR(sdr_begin_xn(sdr));
	//sdr_stage(sdr, (char *)&cfdpInfoBuf, armurvdb->cfdpInfo, sizeof(ARMUR_CfdpInfo));
	//sdr_free(sdr, cfdpInfoBuf.archiveName);
	//cfdpInfoBuf.archiveName = 0;
	//sdr_write(sdr, armurvdb->cfdpInfo, (char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));

	//sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
	//armurdbBuf.stat = ARMUR_STAT_IDLE;
	//sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));
	//if (sdr_end_xn(sdr) < 0)
	//{
	//	return -1;
	//}

	return 0;
}

int	armurAttach()
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

void	armurDetach()
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

void	armurFindPackageDescr(char *packageName, ARMUR_VPackageDescr **vdescr,
			PsmAddress *vdescrElt)
{
	PsmPartition	wm = getIonwm();
	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
	Object		elt;

	for (elt = sm_list_first(wm, armurvdb->applications); elt;
		elt = sm_list_next(wm, elt))
	{
		*vdescr = (ARMUR_VPackageDescr *)psp(wm, sm_list_data(wm, elt));
		if (strcmp((*vdescr)->name, packageName) == 0)
		{
			break;
		}
	}
	*vdescrElt = elt;
}

void	armurFindImage(char *imageName, ARMUR_VImage **vimage, PsmAddress *vimageAddr)
{
	PsmPartition	wm = getIonwm();
	ARMUR_VDB	*armurvdb = _armurvdb(NULL);

	CHKVOID(imageName);
	CHKVOID(vimage);
	CHKVOID(vimageAddr);

	CHKVOID(ionLocked());
	if ((*vimageAddr = ARMUR_rhht_retrieve_key(armurvdb->vimages, imageName)) != 0)
	{
		*vimage = (ARMUR_VImage *)psp(wm, *vimageAddr);
	}
}

static int	enqueueImage(int level, ARMUR_VImage *vimage, PsmAddress vimageAddr)
{
	PsmPartition	wm = getIonwm();
	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
	PsmAddress	elt = 0;

	CHKERR(ionLocked());
	if (level == ARMUR_LEVEL_1)
	{
		/*	Compare the layer values of element data and then
		 *	insert the given image address at the right place.	*/
		ARMUR_VImage *thisVimage;
		for (elt = sm_list_first(wm, armurvdb->restartQueue[level]); elt;
			elt = sm_list_next(wm, elt))
		{
			thisVimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			if (thisVimage->as.lv1.layer < vimage->as.lv1.layer)
			{
				continue;
			}

			break;	/*	Insert before this element.		*/
		}
	}

	if (elt)
	{
		if ((elt = sm_list_insert_before(wm, elt, vimageAddr)) == 0)
		{
			return -1;
		}
	}
	else
	{
		if ((elt = sm_list_insert_last(wm, armurvdb->restartQueue[level],
			vimageAddr)) == 0)
		{
			return -1;
		}
	}

	if (!(armurvdb->vstat & ARMUR_VSTAT(level)))
	{
		armurvdb->vstat |= ARMUR_VSTAT(level);
	}

	return 0;
}

static int	addImage(char *imageName, int level, char *packageName,
			int layer, int apptype)
{
	Sdr			sdr = getIonsdr();
	ARMUR_DB		*armurdb = _armurConstants();
	ARMUR_Image		imageBuf;
	Object			addr;
	ARMUR_VImage		*vimage;
	PsmAddress		vimageAddr;

	CHKERR(imageName);
	CHKERR(packageName);

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

	CHKERR(sdr_begin_xn(sdr));

	/*	Begin with common procedures.				*/
	armurFindImage(imageName, &vimage, &vimageAddr);
	if (vimageAddr != 0)
	{
		sdr_exit_xn(sdr);
		writeMemoNote("[?] Duplicate image", imageName);
		return 0;
	}

	/*	Allocate the data in the SDR heap space.		*/
	if ((addr = sdr_malloc(sdr, sizeof(ARMUR_Image))) == 0)
	{
		sdr_exit_xn(sdr);
		putErrmsg("No space for image object.", imageName);
		return -1;
	}

	if (sdr_list_insert_last(sdr, armurdb->images[level], addr) == 0)
	{
		sdr_cancel_xn(sdr);
		putErrmsg("No space for image object.", imageName);
		return -1;
	}

	memset((char *)&imageBuf, 0, sizeof(ARMUR_Image));
	istrcpy(imageBuf.name, imageName, sizeof imageBuf.name);
	istrcpy(imageBuf.packageName, packageName, sizeof imageBuf.packageName);
	imageBuf.level = level;

	/*	Follow with level-specific procedures.			*/
	switch (level)
	{
	case ARMUR_LEVEL_0:
		imageBuf.tag = 0;
		break;

	case ARMUR_LEVEL_1:
		imageBuf.tag = layer;
		break;

	case ARMUR_LEVEL_2:
		imageBuf.tag = apptype;
	}

	/*	Write the data in the allocated SDR heap space.		*/
	sdr_write(sdr, addr, (char *)&imageBuf, sizeof(ARMUR_Image));

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("Can't add image.", imageName);
		return -1;
	}

	/*	Now insert the address of the image in the hash table.	*/
	CHKERR(sdr_begin_xn(sdr));
	if (raiseImage(addr, _armurvdb(NULL)) < 0)
	{
		sdr_exit_xn(sdr);
		putErrmsg("Can't raise image.", imageName);
		return -1;
	}
	sdr_exit_xn(sdr);

	return 0;
}

int	armurAddImageLv0(char *imageName, char *packageName)
{
	return addImage(imageName, ARMUR_LEVEL_0, packageName, 0, 0);
}

int	armurAddImageLv1(char *imageName, char *packageName, int layer)
{
	CHKERR(layer == ARMUR_LAYER_APPLICATION
		|| layer == ARMUR_LAYER_BUNDLE
		|| layer == ARMUR_LAYER_CONVERGENCE);

	return addImage(imageName, ARMUR_LEVEL_1, packageName, layer, 0);
}

int	armurAddImageLv2(char *imageName, char *packageName, int apptype)
{
	CHKERR(apptype == ARMUR_APPTYPE_DAEMON
		|| apptype == ARMUR_APPTYPE_NORMAL);

	return addImage(imageName, ARMUR_LEVEL_2, packageName, 0, apptype);
}

static void	setInstallTimestamp(ARMUR_VImage *vimage, time_t installedTime)
{
	Sdr		sdr = getIonsdr();
	ARMUR_Image	imageBuf;

	CHKVOID(ionLocked());
	sdr_stage(sdr, (char *)&imageBuf, vimage->addr, sizeof(ARMUR_Image));
	imageBuf.installedTime = installedTime;
	sdr_write(sdr, vimage->addr, (char *)&imageBuf, sizeof(ARMUR_Image));
}

static void	getInstallDir(ARMUR_Image image, char *installDir)
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	*armurdb = _armurConstants();
	char		buf[SDRSTRING_BUFSZ];

	CHKVOID(ionLocked());
	if (image.level == ARMUR_LEVEL_2)
	{
		sdr_string_read(sdr, buf, armurdb->installPath[ARMUR_IMAGETYPE_APP]);
	}
	else
	{
		sdr_string_read(sdr, buf, armurdb->installPath[ARMUR_IMAGETYPE_LIB]);
	}

	istrcpy(installDir, buf, ARMUR_PATHNAME_LEN_MAX);
}

void	armurUpdateStat(int armurStat)
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	armurdbBuf;
	Object		armurdbObj = _armurdbObject(NULL);

	CHKVOID(armurStat == ARMUR_STAT_IDLE
		|| armurStat == ARMUR_STAT_DOWNLOADED
		|| armurStat == ARMUR_STAT_INSTALLED);

	CHKVOID(ionLocked());
	sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
	armurdbBuf.stat = armurStat;
	sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));
}

int	armurUpdateCfdpSrcNbr(uvast cfdpSrcNbr)
{
	Sdr		sdr = getIonsdr();
	ARMUR_CfdpInfo	cfdpInfoBuf;
	Object		addr;
	
	CHKERR(sdr_begin_xn(sdr));

	addr = (_armurvdb(NULL))->cfdpInfo;
	sdr_stage(sdr, (char *)&cfdpInfoBuf, addr, sizeof(ARMUR_CfdpInfo));
	cfdpInfoBuf.srcNbr = cfdpSrcNbr;
	sdr_write(sdr, addr, (char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("Can't update CFDP source number.", itoa(cfdpSrcNbr));
		return -1;
	}

	return 0;
}

//int	armurUpdateCfdpTxnNbr(uvast cfdpTxnNbr)
//{
//	Sdr		sdr = getIonsdr();
//	ARMUR_CfdpInfo	cfdpInfoBuf;
//	Object		addr;
//	
//	CHKERR(sdr_begin_xn(sdr));
//
//	addr = (_armurvdb(NULL))->cfdpInfo;
//	sdr_stage(sdr, (char *)&cfdpInfoBuf, addr, sizeof(ARMUR_CfdpInfo));
//	cfdpInfoBuf.txnNbr = cfdpTxnNbr;
//	sdr_write(sdr, addr, (char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));
//
//	if (sdr_end_xn(sdr) < 0)
//	{
//		putErrmsg("Can't update CFDP transaction number.", itoa(cfdpTxnNbr));
//		return -1;
//	}
//
//	return 0;
//}

int	armurUpdateCfdpArchiveName(char *archiveName)
{
	Sdr		sdr = getIonsdr();
	ARMUR_CfdpInfo	cfdpInfoBuf;
	Object		addr;
	
	CHKERR(sdr_begin_xn(sdr));

	addr = (_armurvdb(NULL))->cfdpInfo;
	sdr_stage(sdr, (char *)&cfdpInfoBuf, addr, sizeof(ARMUR_CfdpInfo));
	cfdpInfoBuf.archiveName = sdr_string_create(sdr, archiveName);
	sdr_write(sdr, addr, (char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("Can't update CFDP archive name.", archiveName);
		return -1;
	}

	return 0;
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

	Sdr			sdr = getIonsdr();
	ARMUR_VDB		*armurvdb = _armurvdb(NULL);
	ARMUR_CfdpInfo		cfdpInfoBuf;
	char			buf[SDRSTRING_BUFSZ];
	int			len;
	uvast			srcNbr;

	/*	Fetch the cfdpInfo stored in the ARMUR DB.		*/
	CHKERR(sdr_begin_xn(sdr));
	sdr_read(sdr, (char *)&cfdpInfoBuf, armurvdb->cfdpInfo, sizeof(ARMUR_CfdpInfo));
	sdr_exit_xn(sdr);

	while (1)
	{
		/*	Nested SDR transaction to enable recovery of CFDP event
		 *	items in case of unexpected system crash and reboot.	*/
		CHKERR(sdr_begin_xn(sdr));
		
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

		/*	TODO: we need to check if any file data have been
		 *	corrupted or something might cause the cfdp_get_event
		 *	to block indefinitely. Implement it as a thread ?	*/

		cfdp_decompress_number(&srcNbr, &transactionId.sourceEntityNbr);

		if (type == CfdpMetadataRecvInd && srcNbr == cfdpInfoBuf.srcNbr)
		{
			while (messagesToUser)
			{
				if (cfdp_get_usrmsg(&messagesToUser,
					(unsigned char *)buf, &len) < 0)
				{
					putErrmsg("Failed getting user msg.", NULL);
					return -1;
				}

				if (len > 0)
				{
					buf[len] = '\0';
					printf("\tMessage to user: %s\n", buf);//dbg
					if (strcmp(buf, "armur") == 0)
					{
						/*	Confirmed downloading.		*/
						printf("\tFilename: %s.\n",
							destFileNameBuf);//dbg

						/*	Configure CFDP information.	*/
						cfdpInfoBuf.archiveName =
							sdr_string_create(sdr,
							destFileNameBuf);
						sdr_write(sdr, armurvdb->cfdpInfo,
							(char *)&cfdpInfoBuf,
							sizeof(ARMUR_CfdpInfo));
					}
				}
			}
		}

		if (type == CfdpTransactionFinishedInd && srcNbr == cfdpInfoBuf.srcNbr)
		{
			/*	Now the download has been finished.		*/
			break;
		}

		if (sdr_end_xn(sdr) < 0)
		{
			return -1;
		}
	}

	/*	Now the download has been finished.
	 *	New archive name to install has been configured.
	 *	Update the ARMUR stat and go to the next step.		*/
	armurUpdateStat(ARMUR_STAT_DOWNLOADED);
	if (sdr_end_xn(sdr) < 0)
	{
		return -1;
	}

	printf("***Download has been completed.\n");//dbg
	return 0;
}

int	armurInstall()
{
	Sdr			sdr = getIonsdr();
	ARMUR_VDB		*armurvdb = _armurvdb(NULL);
	ARMUR_CfdpInfo		cfdpInfoBuf;
	ARMUR_Image		imageBuf;
	ARMUR_VImage		*vimage;
	PsmAddress		vimageAddr;
	char			archiveNameBuf[SDRSTRING_BUFSZ];
	char			imageName[ARMUR_FILENAME_LEN_MAX];
	char			imageNameTmp[ARMUR_FILENAME_LEN_MAX];
	char			*ext;
	char			pathName[ARMUR_PATHNAME_LEN_MAX];
	char			pathNameTmp[ARMUR_PATHNAME_LEN_MAX];
	char			installDir[ARMUR_PATHNAME_LEN_MAX];
	struct archive		*a;
	struct archive_entry	*entry;
	int			result;
	time_t			installedTime = getCtime();

	CHKERR(sdr_begin_xn(sdr));
	sdr_read(sdr, (char *)&cfdpInfoBuf, armurvdb->cfdpInfo, sizeof(ARMUR_CfdpInfo));
	sdr_string_read(sdr, archiveNameBuf, cfdpInfoBuf.archiveName);
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
		strcpy(imageNameTmp, imageName);
		if ((ext = strstr(imageNameTmp, ".so")) != NULL)
		{
			/*	This is a shared library.	*/
			if (strcmp(ext, ".so") != 0)
			{
				/*	ABI version numbers are appended.
				 *	We store the image name without them.	*/
				ext[3] = '\0';
			}
		}

		CHKERR(sdr_begin_xn(sdr));
		armurFindImage(imageNameTmp, &vimage, &vimageAddr);
		if (vimageAddr == 0)
		{
			/*	This is an unknown image. Skip it.		*/
			sdr_exit_xn(sdr);
			putErrmsg("Unknown image, skipped.", imageName);
			if (archive_read_data_skip(a) != ARCHIVE_OK)
			{
				archive_read_free(a);
				return -1;
			}
			continue;
		}

		sdr_read(sdr, (char *)&imageBuf, vimage->addr, sizeof(ARMUR_Image));
		printf("\tfound image: %s.\n", imageBuf.name);//dbg

		/*	Retain the vimage in the restart queue.			*/
		if (enqueueImage(imageBuf.level, vimage, vimageAddr) < 0)
		{
			sdr_exit_xn(sdr);
			putErrmsg("Can't enqueue image", imageName);
			archive_read_free(a);
			return -1;
		}

		getInstallDir(imageBuf, installDir);
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
		setInstallTimestamp(vimage, installedTime);
		if (sdr_end_xn(sdr) < 0)
		{
			archive_read_free(a);
			return -1;
		}

		if (archive_read_data_skip(a) != ARCHIVE_OK)
		{
			archive_read_free(a);
			return -1;
		}
	}
	/*	TODO: Increment the number of images installed.		*/
	archive_read_free(a);

	CHKERR(sdr_begin_xn(sdr));
	armurUpdateStat(ARMUR_STAT_INSTALLED);
	if (sdr_end_xn(sdr) < 0)
	{
		return -1;
	}

	printf("***Install has been completed.\n");//dbg
	return 0;
}

/*	MIGRATED TO ARMUR_RESTART.C MAIN THREAD 08/18/2020	*/
//static int	stop(ARMUR_VImage *vimage, int level)
//{
//	PsmPartition	wm = getIonwm();
//	Object		elt;
//	ARMUR_VImage	*_vimage;
//	int		layer;
//
//	switch (level)
//	{
//	case ARMUR_LEVEL_0:
//		for (layer = ARMUR_LAYER_APPLICATION; layer <= ARMUR_LAYER_CONVERGENCE;
//			layer++)
//		{
//			for (elt = sm_list_first(wm, vimage->as.lv0.packages[layer]);
//				elt; elt = sm_list_next(wm, elt))
//			{
//				_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//				if (stop(_vimage, ARMUR_LEVEL_1) < 0)
//				{
//					return -1;
//				}
//			}
//		}
//		for (elt = sm_list_first(wm,
//			vimage->as.lv0.applications[ARMUR_APPTYPE_DAEMON]);
//			elt; elt = sm_list_next(wm, elt))
//		{
//			_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (stop(_vimage, ARMUR_LEVEL_2) < 0)
//			{
//				return -1;
//			}
//		}
//		break;
//
//	case ARMUR_LEVEL_1:
//		for (elt = sm_list_first(wm,
//			vimage->as.lv1.applications[ARMUR_APPTYPE_DAEMON]);
//			elt; elt = sm_list_next(wm, elt))
//		{
//			_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (stop(_vimage, ARMUR_LEVEL_2) < 0)
//			{
//				return -1;
//			}
//		}
//		break;
//
//	case ARMUR_LEVEL_2:
//		if (vimage->as.lv2.vstat & ARMUR_VSTAT_LV2_STOPPED)
//		{
//			break;
//		}
//		if (vimage->as.lv2.stop() < 0)
//		{
//			return -1;
//		}
//		vimage->as.lv2.vstat |= ARMUR_VSTAT_LV2_STOPPED;
//	}
//
//	return 0;
//}
//
//static int	start(ARMUR_VImage *vimage, int level)
//{
//	PsmPartition	wm = getIonwm();
//	Object		elt;
//	ARMUR_VImage	*_vimage;
//	int		layer;
//
//	/*	Start: reverse order of stop	*/
//	switch (level)
//	{
//	case ARMUR_LEVEL_0:
//		for (elt = sm_list_first(wm,
//			vimage->as.lv0.applications[ARMUR_APPTYPE_DAEMON]);
//			elt; elt = sm_list_next(wm, elt))
//		{
//			_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (start(_vimage, ARMUR_LEVEL_2) < 0)
//			{
//				return -1;
//			}
//		}
//		for (layer = ARMUR_LAYER_CONVERGENCE; layer >= ARMUR_LAYER_APPLICATION;
//			layer--)
//		{
//			for (elt = sm_list_first(wm, vimage->as.lv0.packages[layer]);
//				elt; elt = sm_list_next(wm, elt))
//			{
//				_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//				if (start(_vimage, ARMUR_LEVEL_1) < 0)
//				{
//					return -1;
//				}
//			}
//		}
//		break;
//
//	case ARMUR_LEVEL_1:
//		for (elt = sm_list_first(wm,
//			vimage->as.lv1.applications[ARMUR_APPTYPE_DAEMON]);
//			elt; elt = sm_list_next(wm, elt))
//		{
//			_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (start(_vimage, ARMUR_LEVEL_2) < 0)
//			{
//				return -1;
//			}
//		}
//		break;
//	
//	case ARMUR_LEVEL_2:
//		if (vimage->as.lv2.vstat & ARMUR_VSTAT_LV2_STARTED)
//		{
//			break;
//		}
//		if (vimage->as.lv2.start() < 0)
//		{
//			return -1;
//		}
//		vimage->as.lv2.vstat |= ARMUR_VSTAT_LV2_STARTED;
//	}
//
//	return 0;
//}
//
//int	armurRestart()
//{
//	Sdr			sdr = getIonsdr();
//	PsmPartition		wm = getIonwm();
//	ARMUR_VDB		*armurvdb = _armurvdb(NULL);
//	PsmAddress		elt;
//	PsmAddress		vdescrElt;
//	ARMUR_VImage		*vimage;
//	ARMUR_VPackageDescr	*vdescr;
//	int			i;
//
//	/*	Lock until the restart procedure is completed.	*/
//	CHKERR(sdr_begin_xn(sdr));
//	/*	We will restart applications only if the vstat is not idle, i.e.,
//	 *	if the restart procedure is still pending. The volatile state will
//	 *	be ARMUR_VSTAT_IDLE, in case either 1) the restart queues are empty,
//	 *	2) the items have been restarted by an unexpected system reboot that
//	 *	reset it or 3) SDR recovery procedure has been conducted leading to
//	 *	restart of the entire ION and reset of the volatile database.		*/
//	if (armurvdb->vstat == ARMUR_VSTAT_IDLE)
//	{
//		sdr_exit_xn(sdr);
//		return 0;
//	}
//
//	/*	There ARE items to be restarted.		*/
//	restartFnInit();
//
//	if (armurvdb->vstat & ARMUR_VSTAT_LV0_PENDING)
//	{
//		/*	Restart items from the queue of LEVEL 0.	*/
//		for (elt = sm_list_first(wm, armurvdb->restartQueue[ARMUR_LEVEL_0]); elt;
//			elt = sm_list_next(wm, elt))
//		{
//			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (stop(vimage, ARMUR_LEVEL_0) < 0
//			|| start(vimage, ARMUR_LEVEL_0) < 0)
//			{
//				return -1;
//			}
//		}
//
//		goto FIN;
//	}
//
//	if (armurvdb->vstat & ARMUR_VSTAT_LV1_PENDING)
//	{
//		/*	Restart items from the queue of LEVEL 1.	*/
//		for (elt = sm_list_first(wm, armurvdb->restartQueue[ARMUR_LEVEL_1]); elt;
//			elt = sm_list_next(wm, elt))
//		{
//			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (stop(vimage, ARMUR_LEVEL_1) < 0)
//			{
//				return -1;
//			}
//		}
//		for (elt = sm_list_last(wm, armurvdb->restartQueue[ARMUR_LEVEL_1]); elt;
//			elt = sm_list_prev(wm, elt))
//		{
//			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (start(vimage, ARMUR_LEVEL_1) < 0)
//			{
//				return -1;
//			}
//		}
//	}
//	
//	if (armurvdb->vstat & ARMUR_VSTAT_LV2_PENDING)
//	{
//		/*	Restart items from the queue of LEVEL 2.	*/
//		for (elt = sm_list_first(wm, armurvdb->restartQueue[ARMUR_LEVEL_2]); elt;
//			elt = sm_list_next(wm, elt))
//		{
//			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (vimage->as.lv2.vstat & ARMUR_VSTAT_LV2_STARTED)
//			{
//				/*	Items in level 2 will be restarted only
//				 *	if they have not yet been restarted by the
//				 *	upper level.					*/
//				continue;
//			}
//
//			/*	Stop & start	*/
//			if (stop(vimage, ARMUR_LEVEL_2) < 0
//			|| start(vimage, ARMUR_LEVEL_2) < 0)
//			{
//				return -1;
//			}
//		}
//	}
//
//	/*	Now reset the volatile states of the VDB.	*/
//FIN:
//	/*	Reset the restart queues.			*/
//	for (i = 0; i < ARMUR_LEVELS; i++)
//	{
//		while ((elt = sm_list_first(wm, armurvdb->restartQueue[i])) != 0)
//		{
//			oK(sm_list_delete(wm, elt, NULL, NULL));
//		}
//	}
//
//	/*	Reset the stop/start states of the lv2 images.	*/
//	for (vdescrElt = sm_list_first(wm, armurvdb->applications); vdescrElt;
//		vdescrElt = sm_list_next(wm, vdescrElt))
//	{
//		vdescr = (ARMUR_VPackageDescr *)psp(wm, sm_list_data(wm, vdescrElt));
//		for (elt = sm_list_first(wm, vdescr->applications[ARMUR_APPTYPE_DAEMON]);
//			elt; elt = sm_list_next(wm, elt))
//		{
//			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
//			if (vimage->as.lv2.vstat)
//			{
//				vimage->as.lv2.vstat = 0;
//			}
//		}
//	}
//
//	/*	Reset the vstat of the ARMUR VDB.		*/
//	armurvdb->vstat = ARMUR_VSTAT_IDLE;
//	sdr_exit_xn(sdr);
//
//	printf("Restart has been completed.\n");//dbg
//	return 0;
//}