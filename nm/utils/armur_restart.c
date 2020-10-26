// 2020-07-20
// jigi

#include "armur.h"
#include "armur_restart.h"
#include "armur_rhht.h"

/*	*	*	Restart functions	*	*	*/

/*------------------------------*
 *		ION		*
 *------------------------------*/

/*	rfxclock	*/
static int rfxclockStop()
{
	IonVdb	*ionvdb = getIonVdb();

	if (ionvdb->clockPid != ERROR)
	{
		sm_TaskKill(ionvdb->clockPid, SIGTERM);
	}

	return 0;
}

static int rfxclockStart()
{
	Sdr	sdr = getIonsdr();
	IonVdb	*ionvdb = getIonVdb();

	if (ionvdb->clockPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(ionvdb->clockPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		ionvdb->clockPid = ERROR;
	}

	ionvdb->clockPid = pseudoshell("rfxclock");

	return 0;
}

/*------------------------------*
 *		BP		*
 *------------------------------*/

/*	bpclock		*/
static int bpclockStop()
{
	BpVdb		*bpvdb;

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	if (bpvdb->clockPid != ERROR)
	{
		sm_TaskKill(bpvdb->clockPid, SIGTERM);
	}

	return 0;
}

static int bpclockStart()
{
	Sdr	sdr = getIonsdr();
	BpVdb	*bpvdb;

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	if (bpvdb->clockPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(bpvdb->clockPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		bpvdb->clockPid = ERROR;
	}

	bpvdb->clockPid = pseudoshell("bpclock");

	return 0;
}

/*	bptransit	*/
static int bptransitStop()
{
	BpVdb	*bpvdb;

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	if (bpvdb->transitPid != ERROR)
	{
		sm_TaskKill(bpvdb->transitPid, SIGTERM);
	}

	return 0;
}

static int bptransitStart()
{
	Sdr	sdr = getIonsdr();
	BpVdb	*bpvdb;

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	if (bpvdb->transitPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(bpvdb->transitPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		bpvdb->transitPid = ERROR;
	}

	if (bpvdb->transitSemaphore == SM_SEM_NONE)
	{
		bpvdb->transitSemaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
	}
	else
	{
		sm_SemUnend(bpvdb->transitSemaphore);
		sm_SemGive(bpvdb->transitSemaphore);
	}
	sm_SemTake(bpvdb->transitSemaphore);

	bpvdb->transitPid = pseudoshell("bptransit");

	return 0;
}

/*	ipnfw		*/
static int ipnfwStop()
{
	VScheme		*vscheme;
	PsmAddress	elt;

	if (bpAttach() < 0)
	{
		return -1;
	}

	findScheme("ipn", &vscheme, &elt);
	if (elt == 0)
	{
		return -1;
	}

	if (vscheme->fwdPid != ERROR)
	{
		sm_TaskKill(vscheme->fwdPid, SIGTERM);
	}

	return 0;
}

static int ipnfwStart()
{
	Sdr		sdr = getIonsdr();
	VScheme		*vscheme;
	PsmAddress	elt;

	if (bpAttach() < 0)
	{
		return -1;
	}

	findScheme("ipn", &vscheme, &elt);
	if (elt == 0)
	{
		return -1;
	}

	if (vscheme->fwdPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(vscheme->fwdPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		vscheme->fwdPid = ERROR;
	}

	if (vscheme->semaphore == SM_SEM_NONE)
	{
		vscheme->semaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
	}
	else
	{
		sm_SemUnend(vscheme->semaphore);
		sm_SemGive(vscheme->semaphore);
	}
	sm_SemTake(vscheme->semaphore);

	vscheme->fwdPid = pseudoshell("ipnfw");

	return 0;
}

/*	ipnadminep	*/
static int ipnadminepStop()
{
	VScheme		*vscheme;
	VEndpoint	*vpoint;
	PsmAddress	elt;

	if (bpAttach() < 0)
	{
		return -1;
	}

	findScheme("ipn", &vscheme, &elt);
	if (elt == 0)
	{
		return -1;
	}

	findEndpoint(NULL, vscheme->adminEid + 4, vscheme, &vpoint, &elt);
	if (elt == 0)
	{
		return -1;
	}

	if (vpoint->appPid != ERROR)
	{
		if (vpoint->semaphore != SM_SEM_NONE)
		{
			sm_SemEnd(vpoint->semaphore);
		}
	}

	return 0;
}

static int ipnadminepStart()
{
	Sdr		sdr = getIonsdr();
	VScheme		*vscheme;
	VEndpoint	*vpoint;
	PsmAddress	elt;

	if (bpAttach() < 0)
	{
		return -1;
	}

	findScheme("ipn", &vscheme, &elt);
	if (elt == 0)
	{
		return -1;
	}

	findEndpoint(NULL, vscheme->adminEid + 4, vscheme, &vpoint, &elt);
	if (elt == 0)
	{
		return -1;
	}

	if (vpoint->appPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(vpoint->appPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		vpoint->appPid = ERROR;
	}
	if (vscheme->admAppPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(vscheme->admAppPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		vscheme->admAppPid = ERROR;
	}

	if (vpoint->semaphore == SM_SEM_NONE)
	{
		vpoint->semaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
	}
	else
	{
		sm_SemUnend(vpoint->semaphore);
		sm_SemGive(vpoint->semaphore);
	}
	sm_SemTake(vpoint->semaphore);

	vscheme->admAppPid = pseudoshell("ipnadminep");

	return 0;
}

/*	bpclm		*/
static int bpclmStop()
{
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VPlan		*vplan;
	PsmAddress	elt;

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->plans); elt; elt = sm_list_next(wm, elt))
	{
		vplan = (VPlan *)psp(wm, sm_list_data(wm, elt));

		if (vplan->clmPid != ERROR)
		{
			if (vplan->semaphore != SM_SEM_NONE)
			{
				sm_SemEnd(vplan->semaphore);
			}
		}
	}

	return 0;
}

static int bpclmStart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VPlan		*vplan;
	PsmAddress	elt;
	char		cmdString[6 + MAX_EID_LEN + 1];

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->plans); elt; elt = sm_list_next(wm, elt))
	{
		vplan = (VPlan *)psp(wm, sm_list_data(wm, elt));

		if (vplan->clmPid != ERROR)
		{
			/*	TODO: Improve waiting procedure.	*/
			sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
			while (sm_TaskExists(vplan->clmPid));
			CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
			vplan->clmPid = ERROR;
		}

		if (vplan->semaphore == SM_SEM_NONE)
		{
			vplan->semaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
		}
		else
		{
			sm_SemUnend(vplan->semaphore);
			sm_SemGive(vplan->semaphore);
		}
		sm_SemTake(vplan->semaphore);

		isprintf(cmdString, sizeof cmdString, "bpclm %s", vplan->neighborEid);
		vplan->clmPid = pseudoshell(cmdString);
	}

	return 0;
}

/*------------------------------*
 *	     BP + LTP		*
 *------------------------------*/

/*	ltpcli		*/
static int ltpcliStop()
{
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VInduct		*vduct;
	PsmAddress	elt;

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->inducts); elt; elt = sm_list_next(wm, elt))
	{
		vduct = (VInduct *)psp(wm, sm_list_data(wm, elt));
		if (strcmp(vduct->protocolName, "ltp") == 0)
		{
			if (vduct->cliPid != ERROR)
			{
				sm_TaskKill(vduct->cliPid, SIGTERM);
			}
		}
	}

	return 0;
}

static int ltpcliStart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VInduct		*vduct;
	PsmAddress	elt;
	char		cmdString[7 + MAX_CL_DUCT_NAME_LEN + 1];

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->inducts); elt; elt = sm_list_next(wm, elt))
	{
		vduct = (VInduct *)psp(wm, sm_list_data(wm, elt));
		if (strcmp(vduct->protocolName, "ltp") == 0)
		{
			if (vduct->cliPid != ERROR)
			{
				/*	TODO: Improve waiting procedure.	*/
				sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
				while (sm_TaskExists(vduct->cliPid));
				CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
				vduct->cliPid = ERROR;
			}

			isprintf(cmdString, sizeof cmdString, "ltpcli %s", vduct->ductName);
			vduct->cliPid = pseudoshell(cmdString);
		}
	}

	return 0;
}

/*	ltpclo		*/
static int ltpcloStop()
{
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VOutduct	*vduct;
	PsmAddress	elt;

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->outducts); elt; elt = sm_list_next(wm, elt))
	{
		vduct = (VOutduct *)psp(wm, sm_list_data(wm, elt));
		if (strcmp(vduct->protocolName, "ltp") == 0)
		{
			if (vduct->cloPid != ERROR)
			{
				if (vduct->semaphore != SM_SEM_NONE)
				{
					sm_SemEnd(vduct->semaphore);
				}
			}
		}
	}

	return 0;
}

static int ltpcloStart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VOutduct	*vduct;
	PsmAddress	elt;
	char		cmdString[7 + MAX_CL_DUCT_NAME_LEN + 1];

	if (bpAttach() < 0)
	{
		return -1;
	}

	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->outducts); elt; elt = sm_list_next(wm, elt))
	{
		vduct = (VOutduct *)psp(wm, sm_list_data(wm, elt));
		if (strcmp(vduct->protocolName, "ltp") == 0)
		{
			if (vduct->cloPid != ERROR)
			{
				/*	TODO: Improve waiting procedure.	*/
				sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
				while (sm_TaskExists(vduct->cloPid));
				CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
				vduct->cloPid = ERROR;
			}

			if (vduct->semaphore == SM_SEM_NONE)
			{
				vduct->semaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
			}
			else
			{
				sm_SemUnend(vduct->semaphore);
				sm_SemGive(vduct->semaphore);
			}
			sm_SemTake(vduct->semaphore);

			isprintf(cmdString, sizeof cmdString, "ltpclo %s", vduct->ductName);
			vduct->cloPid = pseudoshell(cmdString);
		}
	}

	return 0;
}

/*------------------------------*
 *		LTP		*
 *------------------------------*/

/*	ltpclock	*/
static int ltpclockStop()
{
	LtpVdb	*ltpvdb;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();

	if (ltpvdb->clockPid != ERROR)
	{
		sm_TaskKill(ltpvdb->clockPid, SIGTERM);
	}

	return 0;
}

static int ltpclockStart()
{
	Sdr	sdr = getIonsdr();
	LtpVdb	*ltpvdb;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();

	if (ltpvdb->clockPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(ltpvdb->clockPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		ltpvdb->clockPid = ERROR;
	}

	ltpvdb->clockPid = pseudoshell("ltpclock");

	return 0;
}

/*	ltpdeliv	*/
static int ltpdelivStop()
{
	LtpVdb	*ltpvdb;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();

	if (ltpvdb->delivPid != ERROR)
	{
		if (ltpvdb->deliverySemaphore != SM_SEM_NONE)
		{
			sm_SemEnd(ltpvdb->deliverySemaphore);
		}
	}

	return 0;
}

static int ltpdelivStart()
{
	Sdr	sdr = getIonsdr();
	LtpVdb	*ltpvdb;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();

	if (ltpvdb->delivPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(ltpvdb->delivPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		ltpvdb->delivPid = ERROR;
	}

	if (ltpvdb->deliverySemaphore == SM_SEM_NONE)
	{
		ltpvdb->deliverySemaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
	}
	else
	{
		sm_SemUnend(ltpvdb->deliverySemaphore);
		sm_SemGive(ltpvdb->deliverySemaphore);
	}
	sm_SemTake(ltpvdb->deliverySemaphore);

	ltpvdb->delivPid = pseudoshell("ltpdeliv");

	return 0;
}

/*	ltpmeter	*/
static int ltpmeterStop()
{
	PsmPartition	wm = getIonwm();
	LtpVdb		*ltpvdb;
	LtpVspan	*vspan;
	PsmAddress	elt;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();

	for (elt = sm_list_first(wm, ltpvdb->spans); elt; elt = sm_list_next(wm, elt))
	{
		vspan = (LtpVspan *)psp(wm, sm_list_data(wm, elt));
		
		if (vspan->meterPid != ERROR)
		{
			if (vspan->bufClosedSemaphore != SM_SEM_NONE)
			{
				sm_SemEnd(vspan->bufClosedSemaphore);
			}
		}
	}

	return 0;
}

static int ltpmeterStart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	LtpVdb		*ltpvdb;
	LtpVspan	*vspan;
	PsmAddress	elt;
	char		cmdString[20];

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();

	for (elt = sm_list_first(wm, ltpvdb->spans); elt; elt = sm_list_next(wm, elt))
	{
		vspan = (LtpVspan *)psp(wm, sm_list_data(wm, elt));

		if (vspan->meterPid != ERROR)
		{
			/*	TODO: Improve waiting procedure.	*/
			sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
			while (sm_TaskExists(vspan->meterPid));
			CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
			vspan->meterPid = ERROR;
		}

		if (vspan->bufClosedSemaphore == SM_SEM_NONE)
		{
			vspan->bufClosedSemaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
		}
		else
		{
			sm_SemUnend(vspan->bufClosedSemaphore);
			sm_SemGive(vspan->bufClosedSemaphore);
		}
		sm_SemTake(vspan->bufClosedSemaphore);

		isprintf(cmdString, sizeof cmdString, "ltpmeter " UVAST_FIELDSPEC,
			vspan->engineId);
		vspan->meterPid = pseudoshell(cmdString);
	}

	return 0;
}

/*	udplsi		*/
static int udplsiStop()
{
	LtpVdb	*ltpvdb;
	LtpDB	*ltpdb;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();
	ltpdb = getLtpConstants();

	if (strncmp(ltpdb->lsiCmd, "udplsi", 6) == 0)
	{
		if (ltpvdb->lsiPid != ERROR)
		{
			sm_TaskKill(ltpvdb->lsiPid, SIGTERM);
		}
	}

	return 0;
}

static int udplsiStart()
{
	Sdr	sdr = getIonsdr();
	LtpVdb	*ltpvdb;
	LtpDB	*ltpdb;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();
	ltpdb = getLtpConstants();

	if (strncmp(ltpdb->lsiCmd, "udplsi", 6) == 0)
	{
		if (ltpvdb->lsiPid != ERROR)
		{
			/*	TODO: Improve waiting procedure.	*/
			sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
			while (sm_TaskExists(ltpvdb->lsiPid));
			CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
			ltpvdb->lsiPid = ERROR;
		}

		ltpvdb->lsiPid = pseudoshell(ltpdb->lsiCmd);
	}

	return 0;
}

/*	udplso		*/
static int udplsoStop()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	LtpVdb		*ltpvdb;
	LtpSpan		span;
	LtpVspan	*vspan;
	PsmAddress	elt;
	char		cmd[SDRSTRING_BUFSZ];

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();

	for (elt = sm_list_first(wm, ltpvdb->spans); elt; elt = sm_list_next(wm, elt))
	{
		vspan = (LtpVspan *)psp(wm, sm_list_data(wm, elt));
		sdr_read(sdr, (char *)&span, sdr_list_data(sdr, vspan->spanElt),
			sizeof(LtpSpan));
		sdr_string_read(sdr, cmd, span.lsoCmd);
		if (strncmp(cmd, "udplso", 6) == 0)
		{
			if (vspan->lsoPid != ERROR)
			{
				if (vspan->segSemaphore != SM_SEM_NONE)
				{
					sm_SemEnd(vspan->segSemaphore);
				}
			}
		}
	}

	return 0;
}

static int udplsoStart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	LtpVdb		*ltpvdb;
	LtpSpan		span;
	LtpVspan	*vspan;
	PsmAddress	elt;
	char		cmd[SDRSTRING_BUFSZ];
	char		cmdString[SDRSTRING_BUFSZ + 11];

	if (ltpAttach() < 0)
	{
		return -1;
	}

	ltpvdb = getLtpVdb();

	for (elt = sm_list_first(wm, ltpvdb->spans); elt; elt = sm_list_next(wm, elt))
	{
		vspan = (LtpVspan *)psp(wm, sm_list_data(wm, elt));
		sdr_read(sdr, (char *)&span, sdr_list_data(sdr, vspan->spanElt),
				sizeof(LtpSpan));
		sdr_string_read(sdr, cmd, span.lsoCmd);
		if (strncmp(cmd, "udplso", 6) == 0)
		{
			if (vspan->lsoPid != ERROR)
			{
				/*	TODO: Improve waiting procedure.	*/
				sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
				while (sm_TaskExists(vspan->lsoPid));
				CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
				vspan->lsoPid = ERROR;
			}

			if (vspan->segSemaphore == SM_SEM_NONE)
			{
				vspan->segSemaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
			}
			else
			{
				sm_SemUnend(vspan->segSemaphore);
				sm_SemGive(vspan->segSemaphore);
			}
			sm_SemTake(vspan->segSemaphore);

			isprintf(cmdString, sizeof cmdString, "%s " UVAST_FIELDSPEC,
				cmd, vspan->engineId);
			vspan->lsoPid = pseudoshell(cmdString);
		}
	}

	return 0;
}

/*------------------------------*
 *		CFDP		*
 *------------------------------*/

/*	cfdpclock	*/
static int cfdpclockStop()
{
	CfdpVdb	*cfdpvdb;

	if (cfdpAttach() < 0)
	{
		return -1;
	}

	cfdpvdb = getCfdpVdb();

	if (cfdpvdb->clockPid != ERROR)
	{
		sm_TaskKill(cfdpvdb->clockPid, SIGTERM);
	}

	return 0;
}

static int cfdpclockStart()
{
	Sdr	sdr = getIonsdr();
	CfdpVdb	*cfdpvdb;

	if (cfdpAttach() < 0)
	{
		return -1;
	}

	cfdpvdb = getCfdpVdb();

	if (cfdpvdb->clockPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(cfdpvdb->clockPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		cfdpvdb->clockPid = ERROR;
	}

	cfdpvdb->clockPid = pseudoshell("cfdpclock");

	return 0;
}

/*	bputa		*/
static int bputaStop()
{
	CfdpVdb	*cfdpvdb;

	if (cfdpAttach() < 0)
	{
		return -1;
	}

	cfdpvdb = getCfdpVdb();

	if (cfdpvdb->utaPid != ERROR)
	{
		if (cfdpvdb->fduSemaphore != SM_SEM_NONE)
		{
			sm_SemEnd(cfdpvdb->fduSemaphore);
		}
	}

	return 0;
}

static int bputaStart()
{
	Sdr	sdr = getIonsdr();
	CfdpVdb	*cfdpvdb;

	if (cfdpAttach() < 0)
	{
		return -1;
	}

	cfdpvdb = getCfdpVdb();

	if (cfdpvdb->utaPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(cfdpvdb->utaPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		cfdpvdb->utaPid = ERROR;
	}

	if (cfdpvdb->fduSemaphore == SM_SEM_NONE)
	{
		cfdpvdb->fduSemaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
	}
	else
	{
		sm_SemUnend(cfdpvdb->fduSemaphore);
		sm_SemGive(cfdpvdb->fduSemaphore);
	}
	sm_SemTake(cfdpvdb->fduSemaphore);

	cfdpvdb->utaPid = pseudoshell("bputa");

	return 0;
}

/*------------------------------*
 *		NM		*
 *------------------------------*/

/*	nm_agent	*/
static int nmagentStop()
{
	ARMUR_VDB	*armurvdb = getArmurVdb();

	if (armurvdb->nmagentPid != ERROR)
	{
		sm_TaskKill(armurvdb->nmagentPid, SIGTERM);
	}

	return 0;
}

static int nmagentStart()
{
	Sdr		sdr = getIonsdr();
	ARMUR_VDB	*armurvdb = getArmurVdb();
	ARMUR_DB	*armurdb = getArmurConstants();
	char		cmd[SDRSTRING_BUFSZ];

	if (armurvdb->nmagentPid != ERROR)
	{
		/*	TODO: Improve waiting procedure.	*/
		sdr_exit_xn(sdr); // Unlock to yield the SDR to the application.
		while (sm_TaskExists(armurvdb->nmagentPid));
		CHKERR(sdr_begin_xn(sdr)); // Retrieve the SDR access.
		armurvdb->nmagentPid = ERROR;
	}

	sdr_string_read(sdr, cmd, armurdb->nmagentCmd);
	//isprintf(ownEid, sizeof ownEid, "ipn:" UVAST_FIELDSPEC ".%u",
	//	getOwnNodeNbr(), NM_AGENT_SVC_NBR);
	//isprintf(cmdString, sizeof cmdString, "nm_agent %s %s", ownEid, armurdb->mgrEid);
	armurvdb->nmagentPid = pseudoshell(cmd);

	return 0;
}

static void	addRestartFn(char *imageName, ARMUR_StopFn stopFn, ARMUR_StartFn startFn)
{
	PsmPartition	wm = getIonwm();
	ARMUR_VDB	*armurvdb = getArmurVdb();
	ARMUR_VImage	*vimage;
	PsmAddress	addr;

	if ((addr = ARMUR_rhht_retrieve_key(armurvdb->vimages, imageName)) == 0)
	{
		return;
	}

	vimage = (ARMUR_VImage *)psp(wm, addr);
	vimage->as.lv2.stop = stopFn;
	vimage->as.lv2.start = startFn;
}

static void	restartFnInit()
{
	addRestartFn("rfxclock",	rfxclockStop,	rfxclockStart);
	addRestartFn("ltpclock",	ltpclockStop,	ltpclockStart);
	addRestartFn("ltpdeliv",	ltpdelivStop,	ltpdelivStart);
	addRestartFn("ltpmeter",	ltpmeterStop,	ltpmeterStart);
	addRestartFn("udplsi",		udplsiStop,	udplsiStart);
	addRestartFn("udplso",		udplsoStop,	udplsoStart);
	addRestartFn("ltpcli",		ltpcliStop,	ltpcliStart);
	addRestartFn("ltpclo",		ltpcloStop,	ltpcloStart);
	addRestartFn("bpclock",		bpclockStop,	bpclockStart);
	addRestartFn("bptransit",	bptransitStop,	bptransitStart);
	addRestartFn("ipnfw",		ipnfwStop,	ipnfwStart);
	addRestartFn("ipnadminep",	ipnadminepStop,	ipnadminepStart);
	addRestartFn("bpclm",		bpclmStop,	bpclmStart);
	addRestartFn("bputa",		bputaStop,	bputaStart);
	addRestartFn("cfdpclock",	cfdpclockStop,	cfdpclockStart);
	addRestartFn("nm_agent",	nmagentStop,	nmagentStart);
}

static int	stop(ARMUR_VImage *vimage, int level)
{
	PsmPartition	wm = getIonwm();
	Object		elt;
	ARMUR_VImage	*_vimage;
	int		layer;

	switch (level)
	{
	case ARMUR_LEVEL_0:
		for (layer = ARMUR_LAYER_APPLICATION; layer <= ARMUR_LAYER_CONVERGENCE;
			layer++)
		{
			for (elt = sm_list_first(wm, vimage->as.lv0.packages[layer]);
				elt; elt = sm_list_next(wm, elt))
			{
				_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
				if (stop(_vimage, ARMUR_LEVEL_1) < 0)
				{
					return -1;
				}
			}
		}
		for (elt = sm_list_first(wm,
			vimage->as.lv0.applications[ARMUR_APPTYPE_DAEMON]);
			elt; elt = sm_list_next(wm, elt))
		{
			_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			if (stop(_vimage, ARMUR_LEVEL_2) < 0)
			{
				return -1;
			}
		}
		break;

	case ARMUR_LEVEL_1:
		for (elt = sm_list_first(wm,
			vimage->as.lv1.applications[ARMUR_APPTYPE_DAEMON]);
			elt; elt = sm_list_next(wm, elt))
		{
			_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			if (stop(_vimage, ARMUR_LEVEL_2) < 0)
			{
				return -1;
			}
		}
		break;

	case ARMUR_LEVEL_2:
	{
		if (vimage->as.lv2.vstat & ARMUR_VSTAT_LV2_STOPPED)
		{
			break;
		}
		ARMUR_Image	imageBuf;
		Sdr		sdr = getIonsdr();
		sdr_read(sdr, (char *)&imageBuf, vimage->addr, sizeof(ARMUR_Image));
		printf("\tStop %s.\n", imageBuf.name);//dbg
		if (vimage->as.lv2.stop() < 0)
		{
			return -1;
		}
		vimage->as.lv2.vstat |= ARMUR_VSTAT_LV2_STOPPED;
	}
	}

	return 0;
}

static int	start(ARMUR_VImage *vimage, int level)
{
	PsmPartition	wm = getIonwm();
	Object		elt;
	ARMUR_VImage	*_vimage;
	int		layer;

	/*	Start: reverse order of stop	*/
	switch (level)
	{
	case ARMUR_LEVEL_0:
		for (elt = sm_list_first(wm,
			vimage->as.lv0.applications[ARMUR_APPTYPE_DAEMON]);
			elt; elt = sm_list_next(wm, elt))
		{
			_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			if (start(_vimage, ARMUR_LEVEL_2) < 0)
			{
				return -1;
			}
		}
		for (layer = ARMUR_LAYER_CONVERGENCE; layer >= ARMUR_LAYER_APPLICATION;
			layer--)
		{
			for (elt = sm_list_first(wm, vimage->as.lv0.packages[layer]);
				elt; elt = sm_list_next(wm, elt))
			{
				_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
				if (start(_vimage, ARMUR_LEVEL_1) < 0)
				{
					return -1;
				}
			}
		}
		break;

	case ARMUR_LEVEL_1:
		for (elt = sm_list_first(wm,
			vimage->as.lv1.applications[ARMUR_APPTYPE_DAEMON]);
			elt; elt = sm_list_next(wm, elt))
		{
			_vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			if (start(_vimage, ARMUR_LEVEL_2) < 0)
			{
				return -1;
			}
		}
		break;
	
	case ARMUR_LEVEL_2:
	{
		if (vimage->as.lv2.vstat & ARMUR_VSTAT_LV2_STARTED)
		{
			break;
		}
		ARMUR_Image	imageBuf;
		Sdr		sdr = getIonsdr();
		sdr_read(sdr, (char *)&imageBuf, vimage->addr, sizeof(ARMUR_Image));
		printf("\tStart %s.\n", imageBuf.name);//dbg
		if (vimage->as.lv2.start() < 0)
		{
			return -1;
		}
		vimage->as.lv2.vstat |= ARMUR_VSTAT_LV2_STARTED;
	}
	}

	return 0;
}

int	main(int argc, char *argv[])
{
	Sdr			sdr;
	PsmPartition		wm;
	ARMUR_VDB		*armurvdb;
	PsmAddress		elt;
	PsmAddress		vdescrElt;
	ARMUR_VImage		*vimage;
	ARMUR_VPackageDescr	*vdescr;
	ARMUR_CfdpInfo		cfdpInfoBuf;
	int			i;

	if (armurAttach() < 0)
	{
		return -1;
	}

	sdr = getIonsdr();
	wm = getIonwm();
	armurvdb = getArmurVdb();

	/*	Lock until the restart procedure is completed.	*/
	CHKERR(sdr_begin_xn(sdr));

	/*	We will restart applications only if the vstat is not idle, i.e.,
	 *	if the restart procedure is still pending. The volatile state will
	 *	be ARMUR_VSTAT_IDLE, in case either 1) the restart queues are empty,
	 *	2) the items have been restarted by an unexpected system reboot that
	 *	reset it or 3) SDR recovery procedure has been conducted leading to
	 *	restart of the entire ION and reset of the volatile database.		*/
	if (armurvdb->vstat == ARMUR_VSTAT_IDLE)
	{
		armurUpdateStat(ARMUR_STAT_IDLE);
		sdr_stage(sdr, (char *)&cfdpInfoBuf, armurvdb->cfdpInfo,
			sizeof(ARMUR_CfdpInfo));
		sdr_free(sdr, cfdpInfoBuf.archiveName);
		cfdpInfoBuf.archiveName = 0;
		sdr_write(sdr, armurvdb->cfdpInfo, (char *)&cfdpInfoBuf,
			sizeof(ARMUR_CfdpInfo));
		if (sdr_end_xn(sdr) < 0)
		{
			return -1;
		}
		return 0;
	}

	/*	There ARE items to be restarted.		*/
	restartFnInit();

	if (armurvdb->vstat & ARMUR_VSTAT_LV0_PENDING)
	{
		/*	Restart items from the queue of LEVEL 0.	*/
		for (elt = sm_list_first(wm, armurvdb->restartQueue[ARMUR_LEVEL_0]); elt;
			elt = sm_list_next(wm, elt))
		{
			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			OBJ_POINTER(ARMUR_Image, image);//dbg
			GET_OBJ_POINTER(sdr, ARMUR_Image, image, vimage->addr);//dbg
			printf("\trestart image: %s.\n", image->name);//dbg
			if (stop(vimage, ARMUR_LEVEL_0) < 0
			|| start(vimage, ARMUR_LEVEL_0) < 0)
			{
				return -1;
			}
		}

		goto FIN;
	}

	if (armurvdb->vstat & ARMUR_VSTAT_LV1_PENDING)
	{
		/*	Restart items from the queue of LEVEL 1.	*/
		for (elt = sm_list_first(wm, armurvdb->restartQueue[ARMUR_LEVEL_1]); elt;
			elt = sm_list_next(wm, elt))
		{
			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			OBJ_POINTER(ARMUR_Image, image);//dbg
			GET_OBJ_POINTER(sdr, ARMUR_Image, image, vimage->addr);//dbg
			printf("\trestart image: %s.\n", image->name);//dbg
			if (stop(vimage, ARMUR_LEVEL_1) < 0)
			{
				return -1;
			}
		}
		for (elt = sm_list_last(wm, armurvdb->restartQueue[ARMUR_LEVEL_1]); elt;
			elt = sm_list_prev(wm, elt))
		{
			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			if (start(vimage, ARMUR_LEVEL_1) < 0)
			{
				return -1;
			}
		}
	}
	
	if (armurvdb->vstat & ARMUR_VSTAT_LV2_PENDING)
	{
		/*	Restart items from the queue of LEVEL 2.	*/
		for (elt = sm_list_first(wm, armurvdb->restartQueue[ARMUR_LEVEL_2]); elt;
			elt = sm_list_next(wm, elt))
		{
			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			OBJ_POINTER(ARMUR_Image, image);//dbg
			GET_OBJ_POINTER(sdr, ARMUR_Image, image, vimage->addr);//dbg
			printf("\trestart image: %s.\n", image->name);//dbg
			if (vimage->as.lv2.vstat & ARMUR_VSTAT_LV2_STARTED)
			{
				/*	Items in level 2 will be restarted only
				 *	if they have not yet been restarted by the
				 *	upper level.					*/
				continue;
			}

			/*	Stop & start	*/
			if (stop(vimage, ARMUR_LEVEL_2) < 0
			|| start(vimage, ARMUR_LEVEL_2) < 0)
			{
				return -1;
			}
		}
	}
	/*	Restart has been finished.
	 *	Now reset the volatile states of the VDB.	*/
FIN:
	/*	Reset the restart queues.			*/
	for (i = 0; i < ARMUR_LEVELS; i++)
	{
		while ((elt = sm_list_first(wm, armurvdb->restartQueue[i])) != 0)
		{
			oK(sm_list_delete(wm, elt, NULL, NULL));
		}
	}

	/*	Reset the stop/start states of the lv2 images.	*/
	for (vdescrElt = sm_list_first(wm, armurvdb->applications); vdescrElt;
		vdescrElt = sm_list_next(wm, vdescrElt))
	{
		vdescr = (ARMUR_VPackageDescr *)psp(wm, sm_list_data(wm, vdescrElt));
		for (elt = sm_list_first(wm, vdescr->applications[ARMUR_APPTYPE_DAEMON]);
			elt; elt = sm_list_next(wm, elt))
		{
			vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
			if (vimage->as.lv2.vstat)
			{
				vimage->as.lv2.vstat = 0;
			}
		}
	}

	/*	Reset the vstat of the ARMUR VDB.		*/
	armurvdb->vstat = ARMUR_VSTAT_IDLE;
	sdr_exit_xn(sdr);

	printf("***Restart has been completed.\n");//dbg

	/*	The entire procedure has been completed.
	 *	Reset the CFDP-related information and state.	*/
	CHKERR(sdr_begin_xn(sdr));
	armurAppendRptMsg("Successfully restarted.", 0);//JIGI
	armurUpdateStat(ARMUR_STAT_FIN);
	//armurUpdateStat(ARMUR_STAT_IDLE);
	sdr_stage(sdr, (char *)&cfdpInfoBuf, armurvdb->cfdpInfo, sizeof(ARMUR_CfdpInfo));
	sdr_free(sdr, cfdpInfoBuf.archiveName);
	cfdpInfoBuf.archiveName = 0;
	sdr_write(sdr, armurvdb->cfdpInfo, (char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));
	if (sdr_end_xn(sdr) < 0)
	{
		return -1;
	}

	return 0;
}
