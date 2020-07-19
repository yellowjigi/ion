// 2020-06-29
// jigi

#include "armur.h"
#include <archive.h>
#include <archive_entry.h>

/*	*	*	Restart functions	*	*	*/

/*------------------------------*
 *	Client programs		*
 *------------------------------*/

/*	ION		*/

/*	BP		*/
static void bputaStop();
static void bputaStart();

/*	LTP		*/
static void ltpcliStart();

/*	CFDP		*/

/*------------------------------*
 *		ION		*
 *------------------------------*/

/*	ION		*/
static int ionRestart()
{
	if (bpAttach() < 0 || ltpAttach() < 0 || cfdpAttach() < 0)
	{
		return -1;
	}

	/*	Stop	*/
	bpStop();
	cfdpStop();
	ltpStop();
	rfx_stop();

	/*	Start	*/
	if (rfx_start() < 0
	|| ltpStart(NULL) < 0
	|| bpStart() < 0
	|| cfdpStart("bputa") < 0)
	{
		return -1;
	}

	while (!rfx_system_is_started()
		|| !bp_agent_is_started()
		|| !ltp_engine_is_started()
		|| !cfdp_entity_is_started());

	return 0;
}

/*	rfxclock	*/
static int rfxclockRestart()
{
	/*	Stop	*/
	rfx_stop();

	/*	Start	*/
	if (rfx_start() < 0)
	{
		return -1;
	}

	while (!rfx_system_is_started());

	return 0;
}

/*------------------------------*
 *		BP		*
 *------------------------------*/

/*	BP		*/
static int bpRestart()
{
	if (bpAttach() < 0 || cfdpAttach() < 0)
	{
		return -1;
	}

	/*	Stop	*/
	bputaStop(); // First stop the client program
	bpStop();

	/*	Start	*/
	if (bpStart() < 0)
	{
		return -1;
	}

	while (!bp_agent_is_started());

	bputaStart(); // Start the client program

	return 0;
}

/*	bpclock		*/
static int bpclockRestart()
{
	Sdr		sdr = getIonsdr();
	BpVdb		*bpvdb;
	pid_t		pid;

	if (bpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	bpvdb = getBpVdb();

	/*	Stop	*/
	if ((pid = bpvdb->clockPid) != ERROR)
	{
		sm_TaskKill(bpvdb->clockPid, SIGTERM);
		bpvdb->clockPid = ERROR;
	}
	sdr_exit_xn(sdr);

	while (sm_TaskExists(pid));

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
	if (bpvdb->clockPid == ERROR || !sm_TaskExists(bpvdb->clockPid))
	{
		bpvdb->clockPid = pseudoshell("bpclock");
	}
	sdr_exit_xn(sdr);

	while (!bp_agent_is_started());

	return 0;
}

/*	bptransit	*/
static int bptransitRestart()
{
	Sdr	sdr = getIonsdr();
	BpVdb	*bpvdb;
	pid_t	pid;

	if (bpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	bpvdb = getBpVdb();

	/*	Stop	*/
	if ((pid = bpvdb->transitPid) != ERROR)
	{
		sm_TaskKill(bpvdb->transitPid, SIGTERM);
		bpvdb->transitPid = ERROR;
	}
	sdr_exit_xn(sdr);

	while (sm_TaskExists(pid));

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
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
	if (bpvdb->transitPid == ERROR || !sm_TaskExists(bpvdb->transitPid))
	{
		bpvdb->transitPid = pseudoshell("bptransit");
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*	ipnfw		*/
static int ipnfwRestart()
{
	Sdr		sdr = getIonsdr();
	VScheme		*vscheme;
	PsmAddress	elt;
	pid_t		pid;

	if (bpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));

	findScheme("ipn", &vscheme, &elt);
	if (elt == 0)
	{
		sdr_exit_xn(sdr);
		return -1;
	}

	/*	Stop	*/
	if ((pid = vscheme->fwdPid) != ERROR)
	{
		sm_TaskKill(vscheme->fwdPid, SIGTERM);
		vscheme->fwdPid = ERROR;
	}
	sdr_exit_xn(sdr);

	while (sm_TaskExists(pid));

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
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
	if (vscheme->fwdPid == ERROR || !sm_TaskExists(vscheme->fwdPid))
	{
		vscheme->fwdPid = pseudoshell("ipnfw");
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*	ipnadminep	*/
static int ipnadminepRestart()
{
	Sdr		sdr = getIonsdr();
	VScheme		*vscheme;
	VEndpoint	*vpoint;
	PsmAddress	elt;
	pid_t		pid;

	if (bpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));

	findScheme("ipn", &vscheme, &elt);
	if (elt == 0)
	{
		sdr_exit_xn(sdr);
		return -1;
	}

	findEndpoint(NULL, vscheme->adminEid + 4, vscheme, &vpoint, &elt);
	if (elt == 0)
	{
		sdr_exit_xn(sdr);
		return -1;
	}

	/*	Stop	*/
	if (vpoint->semaphore != SM_SEM_NONE)
	{
		sm_SemEnd(vpoint->semaphore);
	}
	if (vpoint->appPid != ERROR)
	{
		vpoint->appPid = ERROR;
	}
	if ((pid = vscheme->admAppPid) != ERROR)
	{
		vscheme->admAppPid = ERROR;
	}
	sdr_exit_xn(sdr);

	while (sm_TaskExists(pid));

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
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
	if (vscheme->admAppPid == ERROR || !sm_TaskExists(vscheme->admAppPid))
	{
		vscheme->admAppPid = pseudoshell("ipnadminep");
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*	bpclm		*/
static int bpclmRestart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VPlan		*vplan;
	PsmAddress	elt;
	char		cmdString[6 + MAX_EID_LEN + 1];
	pid_t		pid;

	if (bpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->plans); elt; elt = sm_list_next(wm, elt))
	{
		vplan = (VPlan *)psp(wm, sm_list_data(wm, elt));

		/*	Stop	*/
		if (vplan->semaphore != SM_SEM_NONE)
		{
			sm_SemEnd(vplan->semaphore);
		}
		if ((pid = vplan->clmPid) != ERROR)
		{
			vplan->clmPid = ERROR;
		}
		sdr_exit_xn(sdr);

		while (sm_TaskExists(pid));

		CHKERR(sdr_begin_xn(sdr));

		/*	Start	*/
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
		if (vplan->clmPid == ERROR || !sm_TaskExists(vplan->clmPid))
		{
			isprintf(cmdString, sizeof cmdString,
					"bpclm %s", vplan->neighborEid);
			vplan->clmPid = pseudoshell(cmdString);
		}
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*------------------------------*
 *	     BP + LTP		*
 *------------------------------*/

/*	ltpcli		*/
static void ltpcliStart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb = getBpVdb();
	VInduct		*vduct;
	PsmAddress	elt;
	char		cmdString[7 + MAX_CL_DUCT_NAME_LEN + 1];

	CHKVOID(sdr_begin_xn(sdr));
	for (elt = sm_list_first(wm, bpvdb->inducts); elt; elt = sm_list_next(wm, elt))
	{
		vduct = (VInduct *)psp(wm, sm_list_data(wm, elt));
		if (strcmp(vduct->protocolName, "ltp") == 0)
		{
			if (vduct->cliPid == ERROR || !sm_TaskExists(vduct->cliPid))
			{
				isprintf(cmdString, sizeof cmdString,
						"ltpcli %s", vduct->ductName);
				vduct->cliPid = pseudoshell(cmdString);
			}
			break;
		}
	}
	sdr_exit_xn(sdr);
}

static int ltpcliRestart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VInduct		*vduct;
	PsmAddress	elt;
	char		cmdString[7 + MAX_CL_DUCT_NAME_LEN + 1];
	pid_t		pid;

	if (bpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->inducts); elt; elt = sm_list_next(wm, elt))
	{
		vduct = (VInduct *)psp(wm, sm_list_data(wm, elt));
		if (strcmp(vduct->protocolName, "ltp") == 0)
		{
			/*	Stop	*/
			if ((pid = vduct->cliPid) != ERROR)
			{
				sm_TaskKill(vduct->cliPid, SIGTERM);
				vduct->cliPid = ERROR;
			}
			sdr_exit_xn(sdr);

			while (sm_TaskExists(pid));
			
			CHKERR(sdr_begin_xn(sdr));

			/*	Start	*/
			if (vduct->cliPid == ERROR || !sm_TaskExists(vduct->cliPid))
			{
				isprintf(cmdString, sizeof cmdString,
						"ltpcli %s", vduct->ductName);
				vduct->cliPid = pseudoshell(cmdString);
			}
			break;
		}
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*	ltpclo		*/
static int ltpcloRestart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	BpVdb		*bpvdb;
	VOutduct	*vduct;
	PsmAddress	elt;
	char		cmdString[7 + MAX_CL_DUCT_NAME_LEN + 1];
	pid_t		pid;

	if (bpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	bpvdb = getBpVdb();

	for (elt = sm_list_first(wm, bpvdb->outducts); elt; elt = sm_list_next(wm, elt))
	{
		vduct = (VOutduct *)psp(wm, sm_list_data(wm, elt));
		if (strcmp(vduct->protocolName, "ltp") == 0)
		{
			/*	Stop	*/
			if (vduct->semaphore != SM_SEM_NONE)
			{
				sm_SemEnd(vduct->semaphore);
			}
			if ((pid = vduct->cloPid) != ERROR)
			{
				vduct->cloPid = ERROR;
			}
			sdr_exit_xn(sdr);

			while (sm_TaskExists(pid));

			CHKERR(sdr_begin_xn(sdr));

			/*	Start	*/
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
			if (vduct->cloPid == ERROR || !sm_TaskExists(vduct->cloPid))
			{
				isprintf(cmdString, sizeof cmdString,
						"ltpclo %s", vduct->ductName);
				vduct->cloPid = pseudoshell(cmdString);
			}
		}
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*------------------------------*
 *		LTP		*
 *------------------------------*/

/*	LTP		*/
static int ltpRestart()
{
	if (bpAttach() < 0 || ltpAttach() < 0)
	{
		return -1;
	}

	/*	Stop	*/
	ltpStop();

	/*	Start	*/
	if (ltpStart(NULL) < 0)
	{
		return -1;
	}

	while (!ltp_engine_is_started());

	ltpcliStart(); // Start the client program

	return 0;
}

/*	ltpclock	*/
static int ltpclockRestart()
{
	Sdr	sdr = getIonsdr();
	LtpVdb	*ltpvdb;
	pid_t	pid;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	ltpvdb = getLtpVdb();

	/*	Stop	*/
	if ((pid = ltpvdb->clockPid) != ERROR)
	{
		sm_TaskKill(ltpvdb->clockPid, SIGTERM);
		ltpvdb->clockPid = ERROR;
	}
	sdr_exit_xn(sdr);

	while (sm_TaskExists(pid));

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
	if (ltpvdb->clockPid == ERROR || !sm_TaskExists(ltpvdb->clockPid))
	{
		ltpvdb->clockPid = pseudoshell("ltpclock");
	}
	sdr_exit_xn(sdr);

	while (!ltp_engine_is_started());

	return 0;
}

/*	ltpdeliv	*/
static int ltpdelivRestart()
{
	Sdr	sdr = getIonsdr();
	LtpVdb	*ltpvdb;
	pid_t	pid;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	ltpvdb = getLtpVdb();

	/*	Stop	*/
	if (ltpvdb->deliverySemaphore != SM_SEM_NONE)
	{
		sm_SemEnd(ltpvdb->deliverySemaphore);
	}
	if ((pid = ltpvdb->delivPid) != ERROR)
	{
		ltpvdb->delivPid = ERROR;
	}
	sdr_exit_xn(sdr);

	while (sm_TaskExists(pid));

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
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
	if (ltpvdb->delivPid == ERROR || !sm_TaskExists(ltpvdb->delivPid))
	{
		ltpvdb->delivPid = pseudoshell("ltpdeliv");
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*	ltpmeter	*/
static int ltpmeterRestart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	LtpVdb		*ltpvdb;
	LtpVspan	*vspan;
	PsmAddress	elt;
	char		cmdString[20];
	pid_t		pid;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	ltpvdb = getLtpVdb();

	for (elt = sm_list_first(wm, ltpvdb->spans); elt; elt = sm_list_next(wm, elt))
	{
		vspan = (LtpVspan *)psp(wm, sm_list_data(wm, elt));
		
		/*	Stop	*/
		if (vspan->bufClosedSemaphore != SM_SEM_NONE)
		{
			sm_SemEnd(vspan->bufClosedSemaphore);
		}
		if ((pid = vspan->meterPid) != ERROR)
		{
			vspan->meterPid = ERROR;
		}
		sdr_exit_xn(sdr);

		while (sm_TaskExists(pid));

		CHKERR(sdr_begin_xn(sdr));

		/*	Start	*/
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
		if (vspan->meterPid == ERROR || !sm_TaskExists(vspan->meterPid))
		{
			isprintf(cmdString, sizeof cmdString,
				"ltpmeter " UVAST_FIELDSPEC, vspan->engineId);
			vspan->meterPid = pseudoshell(cmdString);
		}
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*	udplsi		*/
static int udplsiRestart()
{
	Sdr	sdr = getIonsdr();
	LtpVdb	*ltpvdb;
	LtpDB	*ltpdb;
	pid_t	pid;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	ltpvdb = getLtpVdb();
	ltpdb = getLtpConstants();

	if (strncmp(ltpdb->lsiCmd, "udplsi", 6) == 0)
	{
		/*	Stop	*/
		if ((pid = ltpvdb->lsiPid) != ERROR)
		{
			sm_TaskKill(ltpvdb->lsiPid, SIGTERM);
			ltpvdb->lsiPid = ERROR;
		}
		sdr_exit_xn(sdr);

		while (sm_TaskExists(pid));

		CHKERR(sdr_begin_xn(sdr));

		/*	Start	*/
		if (ltpvdb->lsiPid == ERROR || !sm_TaskExists(ltpvdb->lsiPid))
		{
			ltpvdb->lsiPid = pseudoshell(ltpdb->lsiCmd);
		}
	}
	
	sdr_exit_xn(sdr);

	return 0;
}

/*	udplso		*/
static int udplsoRestart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	LtpVdb		*ltpvdb;
	LtpSpan		span;
	LtpVspan	*vspan;
	PsmAddress	elt;
	char		cmd[SDRSTRING_BUFSZ];
	char		cmdString[SDRSTRING_BUFSZ + 11];
	pid_t		pid;

	if (ltpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	ltpvdb = getLtpVdb();

	for (elt = sm_list_first(wm, ltpvdb->spans); elt; elt = sm_list_next(wm, elt))
	{
		vspan = (LtpVspan *)psp(wm, sm_list_data(wm, elt));
		sdr_read(sdr, (char *)&span, sdr_list_data(sdr, vspan->spanElt),
				sizeof(LtpSpan));
		sdr_string_read(sdr, cmd, span.lsoCmd);
		if (strncmp(cmd, "udplso", 6) == 0)
		{
			/*	Stop	*/
			if (vspan->segSemaphore != SM_SEM_NONE)
			{
				sm_SemEnd(vspan->segSemaphore);
			}
			if ((pid = vspan->lsoPid) != ERROR)
			{
				vspan->lsoPid = ERROR;
			}
			sdr_exit_xn(sdr);

			while (sm_TaskExists(pid));

			CHKERR(sdr_begin_xn(sdr));

			/*	Start	*/
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
			if (vspan->lsoPid == ERROR || sm_TaskExists(vspan->lsoPid) == 0)
			{
				isprintf(cmdString, sizeof cmdString, "%s " UVAST_FIELDSPEC,
						cmd, vspan->engineId);
				vspan->lsoPid = pseudoshell(cmdString);
			}
		}
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*------------------------------*
 *		CFDP		*
 *------------------------------*/

/*	CFDP		*/
static int cfdpRestart()
{
	if (cfdpAttach() < 0)
	{
		return -1;
	}

	/*	Stop	*/
	cfdpStop();

	/*	Start	*/
	if (cfdpStart("bputa") < 0)
	{
		return -1;
	}

	while (!cfdp_entity_is_started());

	return 0;
}

/*	cfdpclock	*/
static int cfdpclockRestart()
{
	Sdr	sdr = getIonsdr();
	CfdpVdb	*cfdpvdb;
	pid_t	pid;

	if (cfdpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	cfdpvdb = getCfdpVdb();

	/*	Stop	*/
	if ((pid = cfdpvdb->clockPid) != ERROR)
	{
		sm_TaskKill(cfdpvdb->clockPid, SIGTERM);
		cfdpvdb->clockPid = ERROR;
	}
	sdr_exit_xn(sdr);

	while (sm_TaskExists(pid));

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
	if (cfdpvdb->clockPid == ERROR || !sm_TaskExists(cfdpvdb->clockPid))
	{
		cfdpvdb->clockPid = pseudoshell("cfdpclock");
	}
	sdr_exit_xn(sdr);

	while (!cfdp_entity_is_started());

	return 0;
}

/*	bputa		*/
static void bputaStop()
{
	Sdr	sdr = getIonsdr();
	CfdpVdb	*cfdpvdb = getCfdpVdb();
	pid_t	pid;

	CHKVOID(sdr_begin_xn(sdr));
	if (cfdpvdb->fduSemaphore != SM_SEM_NONE)
	{
		sm_SemEnd(cfdpvdb->fduSemaphore);
	}
	if ((pid = cfdpvdb->utaPid) != ERROR)
	{
		cfdpvdb->utaPid = ERROR;
	}
	sdr_exit_xn(sdr);
	while (sm_TaskExists(pid));
}

static void bputaStart()
{
	Sdr	sdr = getIonsdr();
	CfdpVdb	*cfdpvdb = getCfdpVdb();

	CHKVOID(sdr_begin_xn(sdr));
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
	if (cfdpvdb->utaPid == ERROR || !sm_TaskExists(cfdpvdb->utaPid))
	{
		cfdpvdb->utaPid = pseudoshell("bputa");
	}
	sdr_exit_xn(sdr);
}

static int bputaRestart()
{
	Sdr	sdr = getIonsdr();
	CfdpVdb	*cfdpvdb;
	pid_t	pid;

	if (cfdpAttach() < 0)
	{
		return -1;
	}

	CHKERR(sdr_begin_xn(sdr));
	cfdpvdb = getCfdpVdb();

	/*	Stop	*/
	if (cfdpvdb->fduSemaphore != SM_SEM_NONE)
	{
		sm_SemEnd(cfdpvdb->fduSemaphore);
	}
	if ((pid = cfdpvdb->utaPid) != ERROR)
	{
		cfdpvdb->utaPid = ERROR;
	}
	sdr_exit_xn(sdr);

	while (sm_TaskExists(pid));

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
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
	if (cfdpvdb->utaPid == ERROR || !sm_TaskExists(cfdpvdb->utaPid))
	{
		cfdpvdb->utaPid = pseudoshell("bputa");
	}
	sdr_exit_xn(sdr);

	return 0;
}

/*------------------------------*
 *		NM		*
 *------------------------------*/

/*	nm_agent	*/
//static int nmagentRestart()
//{
//	Sdr	sdr = getIonsdr();
//	CfdpVdb	*cfdpvdb;
//	pid_t	pid;
//
//	if (cfdpAttach() < 0)
//	{
//		return -1;
//	}
//
//	CHKERR(sdr_begin_xn(sdr));
//	cfdpvdb = getCfdpVdb();
//
//	/*	Stop	*/
//	if (cfdpvdb->fduSemaphore != SM_SEM_NONE)
//	{
//		sm_SemEnd(cfdpvdb->fduSemaphore);
//	}
//	if ((pid = cfdpvdb->utaPid) != ERROR)
//	{
//		cfdpvdb->utaPid = ERROR;
//	}
//	sdr_exit_xn(sdr);
//
//	while (sm_TaskExists(pid));
//
//	CHKERR(sdr_begin_xn(sdr));
//
//	/*	Start	*/
//	if (cfdpvdb->fduSemaphore == SM_SEM_NONE)
//	{
//		cfdpvdb->fduSemaphore = sm_SemCreate(SM_NO_KEY, SM_SEM_FIFO);
//	}
//	else
//	{
//		sm_SemUnend(cfdpvdb->fduSemaphore);
//		sm_SemGive(cfdpvdb->fduSemaphore);
//	}
//	sm_SemTake(cfdpvdb->fduSemaphore);
//	if (cfdpvdb->utaPid == ERROR || !sm_TaskExists(cfdpvdb->utaPid))
//	{
//		cfdpvdb->utaPid = pseudoshell("bputa");
//	}
//	sdr_exit_xn(sdr);
//
//	return 0;
//}

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

		vdb = (ARMUR_VDB *)psp(wm, vdbAddress);
		/*	restartMask is always initialized to 0 in volatile database.
		 *	This enables no restart when unexpected system reboot occurs.	*/
		vdb->restartMask = 0;
		if (psm_catlg(wm, *name, vdbAddress) < 0)
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


int	armurInit()
{
	Sdr		sdr;
	ARMUR_DB	armurdbBuf;
	Object		armurdbObject;
	Object		imageList;
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

		/*			Default values				*/

		armurdbBuf.numInstalled[ARMUR_LIBS] = 4;
		armurdbBuf.numInstalled[ARMUR_APPS] = 15;
		armurdbBuf.installPath[ARMUR_LIBS] =
			sdr_string_create(sdr, ARMUR_LIBPATH_DEFAULT);
		armurdbBuf.installPath[ARMUR_APPS] =
			sdr_string_create(sdr, ARMUR_APPPATH_DEFAULT);

		/*			Default libraries			*/
		imageList = armurdbBuf.images[ARMUR_LIBS] = sdr_list_create(sdr);
		if (armurAddImage("libici.so",	ionRestart, ARMUR_ALL, imageList)	< 0
		|| armurAddImage("libbp.so",	bpRestart, ARMUR_BP, imageList)		< 0
		|| armurAddImage("libltp.so",	ltpRestart, ARMUR_LTP, imageList)	< 0
		|| armurAddImage("libcfdp.so",	cfdpRestart, ARMUR_CFDP, imageList)	< 0)
		{
			sdr_cancel_xn(sdr);
			putErrmsg("No space for database.", NULL);
		}

		/*		Default daemon applications			*/
		imageList = armurdbBuf.images[ARMUR_APPS] = sdr_list_create(sdr);
		if (armurAddImage("rfxclock", rfxclockRestart, ARMUR_ION, imageList)	< 0
		|| armurAddImage("bpclock", bpclockRestart, ARMUR_BP, imageList)	< 0
		|| armurAddImage("bptransit", bptransitRestart, ARMUR_BP, imageList)	< 0
		|| armurAddImage("ipnfw",	ipnfwRestart, ARMUR_BP, imageList)	< 0
		|| armurAddImage("ipnadminep", ipnadminepRestart, ARMUR_BP, imageList)	< 0
		|| armurAddImage("bpclm",	bpclmRestart, ARMUR_BP, imageList)	< 0
		|| armurAddImage("ltpcli", ltpcliRestart, ARMUR_BP|ARMUR_LTP, imageList)< 0
		|| armurAddImage("ltpclo", ltpcloRestart, ARMUR_BP|ARMUR_LTP, imageList)< 0
		|| armurAddImage("ltpclock", ltpclockRestart, ARMUR_LTP, imageList)	< 0
		|| armurAddImage("ltpdeliv", ltpdelivRestart, ARMUR_LTP, imageList)	< 0
		|| armurAddImage("ltpmeter", ltpmeterRestart, ARMUR_LTP, imageList)	< 0
		|| armurAddImage("udplsi",	udplsiRestart, ARMUR_LTP, imageList)	< 0
		|| armurAddImage("udplso",	udplsoRestart, ARMUR_LTP, imageList)	< 0
		|| armurAddImage("cfdpclock", cfdpclockRestart, ARMUR_CFDP, imageList)	< 0
		|| armurAddImage("bputa", bputaRestart, ARMUR_BP|ARMUR_CFDP, imageList)	< 0)
		{
			sdr_cancel_xn(sdr);
			putErrmsg("No space for database.", NULL);
		}

		/*	Prepare queues to retain references of pending images		*/
		armurdbBuf.queue[ARMUR_LV0] = sdr_list_create(sdr);
		armurdbBuf.queue[ARMUR_LV1] = sdr_list_create(sdr);
		armurdbBuf.queue[ARMUR_LV2] = sdr_list_create(sdr);
		armurdbBuf.cfdpInfo = sdr_malloc(sdr, sizeof(ARMUR_CfdpInfo));
		if (armurdbBuf.cfdpInfo)
		{
			memset((char *)&cfdpInfoInit, 0, sizeof(ARMUR_CfdpInfo));
			sdr_write(sdr, armurdbBuf.cfdpInfo,
					(char *)&cfdpInfoInit, sizeof(ARMUR_CfdpInfo));
		}

		/*	Write to SDR and catalogue it	*/
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

int	armurStart(char *nmagentCmd)
{
	Sdr		sdr = getIonsdr();
	Object		armurdbObj = _armurdbObject(NULL);
	ARMUR_DB	armurdbBuf;
	ARMUR_CfdpInfo	cfdpInfoBuf;

	if (nmagentCmd)
	{
		if (strlen(nmagentCmd) > MAX_SDRSTRING)
		{
			return -1;
		}

		CHKERR(sdr_begin_xn(sdr));
		sdr_stage(sdr, (char *)&armurdbBuf, armurdbObj, sizeof(ARMUR_DB));
		armurdbBuf.nmagentCmd = sdr_string_create(sdr, nmagentCmd);
		sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));
		if (sdr_end_xn(sdr))
		{
			return -1;
		}

		if (armurdbBuf.nmagentCmd == 0)
		{
			return -1;
		}
	}
		
	switch ((_armurConstants())->stat)
	{
	case ARMUR_STAT_IDLE:
		break;
		//if (ampTrigger == NULL)
		//{
		//	break;
		//}

		/*	ARMUR start has been triggerred by a network manager.
		 *	Update the ARMUR stat and go to the next step.			*/
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
				armurdbBuf.cfdpInfo, sizeof(ARMUR_CfdpInfo));
		cfdpInfoBuf.srcNbr = 0;
		cfdpInfoBuf.txnNbr = 0;
		sdr_write(sdr, armurdbBuf.cfdpInfo,
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
		armurdbBuf.stat = ARMUR_STAT_RESTART_PENDING;
		sdr_write(sdr, armurdbObj, (char *)&armurdbBuf, sizeof(ARMUR_DB));

		sdr_stage(sdr, (char *)&cfdpInfoBuf,
				armurdbBuf.cfdpInfo, sizeof(ARMUR_CfdpInfo));
		sdr_free(sdr, cfdpInfoBuf.archiveName);
		cfdpInfoBuf.archiveName = 0;
		sdr_write(sdr, armurdbBuf.cfdpInfo,
				(char *)&cfdpInfoBuf, sizeof(ARMUR_CfdpInfo));
		if (sdr_end_xn(sdr))
		{
			return -1;
		}

	case ARMUR_STAT_RESTART_PENDING:
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

	//nmagentRestart(nmagentCmd);
	//CHKERR(sdr_begin_xn(sdr));
	//if ((pid = armurvdb->nmagentPid) != ERROR)
	//{
	//	sm_TaskKill(pid, SIGTERM);
	//	armurvdb->nmagentPid = ERROR;
	//}
	//sdr_exit_xn(sdr);

	//while (sm_TaskExists(pid));

	//CHKERR(sdr_begin_xn(sdr));
	//if (armurvdb->nmagentPid == ERROR || !sm_TaskExists(armurvdb->nmagentPid))
	//{
	//	armurvdb->nmagentPid = pseudoshell(nmagentCmd);
	//}
	//sdr_exit_xn(sdr);

	return 0;
}

//void armurStop()
//{
//}

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

static int	armurParseImageName(char *imageName)
{
	if (strcmp(imageName + strlen(imageName) - 3, ".so") == 0)
	{
		if (strcmp(imageName, "libici.so") == 0)
		{
			return ARMUR_CORE_LIBRARY;
		}

		return ARMUR_PROTOCOL_LIBRARY;
	}

	return ARMUR_APPLICATION;
}

static Object	locateImage(char *imageName, int imageType)
{
	Sdr		sdr = getIonsdr();
	Object		elt;
			OBJ_POINTER(ARMUR_Image, image);

	CHKZERO(ionLocked());
	CHKZERO(imageName);
	CHKZERO(imageType == ARMUR_LIBS || imageType == ARMUR_APPS);

	for (elt = sdr_list_first(sdr, (_armurConstants())->images[imageType]); elt;
			elt = sdr_list_next(sdr, elt))
	{
		GET_OBJ_POINTER(sdr, ARMUR_Image, image, sdr_list_data(sdr, elt));
		if (strcmp(image->name, imageName) == 0)
		{
			break;
		}
	}

	return elt;
}

int	armurFindImage(char *imageName, Object *imageObj, Object *imageElt)
{
	Sdr	sdr = getIonsdr();
	Object	elt = 0;
	int	imageType;

	CHKERR(imageName);
	CHKERR(imageObj);
	CHKERR(imageElt);

	imageType = armurParseImageName(imageName);

	CHKERR(sdr_begin_xn(sdr));
	switch (imageType)
	{
	case ARMUR_CORE_LIBRARY:
	case ARMUR_PROTOCOL_LIBRARY:
		elt = locateImage(imageName, ARMUR_LIBS);
		break;
	
	case ARMUR_APPLICATION:
		elt = locateImage(imageName, ARMUR_APPS);
	}

	if (elt == 0)
	{
		sdr_exit_xn(sdr);
		*imageElt = 0;
		return -1;
	}

	*imageObj = sdr_list_data(sdr, elt);
	*imageElt = elt;

	sdr_exit_xn(sdr);
	return imageType;
}

static int	enqueueImage(Object imageObj, int imageType)
{
	Sdr		sdr = getIonsdr();
	Object		imageRefObj;
	Object		imageRefElt;
			OBJ_POINTER(ARMUR_Image, image);
	ARMUR_ImageRef	imageRef;

	CHKERR(imageObj);
	CHKERR(imageType == ARMUR_LV0 || imageType == ARMUR_LV1 || imageType == ARMUR_LV2);

	imageRef.obj = imageObj;

	CHKERR(sdr_begin_xn(sdr));
	if ((imageRefObj = sdr_malloc(sdr, sizeof(ARMUR_ImageRef))) == 0)
	{
		sdr_cancel_xn(sdr);
		putErrmsg("Not enough space for database.", NULL);
		return -1;
	}
	sdr_write(sdr, imageRefObj, (char *)&imageRef, sizeof(ARMUR_ImageRef));
	if ((imageRefElt = sdr_list_insert_last(sdr,
		(_armurConstants())->queue[imageType], imageRefObj)) == 0)
	{
		sdr_cancel_xn(sdr);
		putErrmsg("No space for database.", NULL);
		return -1;
	}

	GET_OBJ_POINTER(sdr, ARMUR_Image, image, imageObj);
	(_armurvdb(NULL))->restartMask |= image->protocol;

	if (sdr_end_xn(sdr))
	{
		putErrmsg("Can't enqueue images.", NULL);
		return -1;
	}

	return 0;
}

int	armurAddImage(char *imageName, ARMUR_RestartFn restartFn,
				int protocol, Object imageList)
{
	Sdr		sdr = getIonsdr();
	ARMUR_Image	armurImageBuf;
	Object		obj;
	Object		elt;

	istrcpy(armurImageBuf.name, imageName, sizeof armurImageBuf.name);
	armurImageBuf.restart = restartFn;
	armurImageBuf.installedTime = 0;
	armurImageBuf.protocol = protocol;
	obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
	if (obj)
	{
		elt = sdr_list_insert_last(sdr, imageList, obj);
		if (elt == 0)
		{
			return -1;
		}
		sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
	}
	else
	{
		return -1;
	}

	return 0;
}

//static void resetRestartMask()
//{
//	ARMUR_VDB	*vdb = _armurvdb(NULL);
//
//	CHKVOID(ionLocked());
//	vdb->restartMask = 0;
//}

//static void resetRestartQueues()
//{
//	PsmPartition	wm = getIonwm();
//	ARMUR_VDB	*vdb = _armurvdb(NULL);
//	PsmAddress	elt;
//	int level;
//
//	CHKVOID(ionLocked());
//	for (level = 0; level < 3; level++)
//	{
//		while ((elt = sm_list_first(wm, vdb->restartQueue[level])) != 0)
//		{
//			psm_free(wm, sm_list_data(wm, elt));
//			oK(sm_list_delete(wm, elt, NULL, NULL));
//		}
//	}
//}
//
//void armurResetVdb()
//{
//	Sdr	sdr = getIonsdr();
//
//	CHKVOID(sdr_begin_xn(sdr));
//
//	resetRestartMask();
//	resetRestartQueues();
//
//	sdr_exit_xn(sdr);
//}

static void	setInstallTimestamp(Object imageObj, time_t installedTime)
{
	Sdr		sdr = getIonsdr();
	ARMUR_Image	imageBuf;

	CHKVOID(imageObj);
	CHKVOID(installedTime);

	CHKVOID(sdr_begin_xn(sdr));
	sdr_stage(sdr, (char *)&imageBuf, imageObj, sizeof(ARMUR_Image));
	imageBuf.installedTime = installedTime;
	sdr_write(sdr, imageObj, (char *)&imageBuf, sizeof(ARMUR_Image));
	oK(sdr_end_xn(sdr));
}

static void	getInstallDir(int imageType, char *installDir)
{
	Sdr	sdr = getIonsdr();
	char	buf[SDRSTRING_BUFSZ];

	CHKVOID(imageType);
	CHKVOID(installDir);

	CHKVOID(sdr_begin_xn(sdr));
	switch (imageType)
	{
	case ARMUR_LV0:
	case ARMUR_LV1:
		sdr_string_read(sdr, buf, (_armurConstants())->installPath[ARMUR_LIBS]);
		break;

	case ARMUR_LV2:
		sdr_string_read(sdr, buf, (_armurConstants())->installPath[ARMUR_APPS]);
	}
	sdr_exit_xn(sdr);

	istrcpy(installDir, buf, PATHNAME_LEN_MAX);
}

int	armurUpdateCfdpSrcNbr(uvast cfdpSrcNbr)
{
	Sdr		sdr = getIonsdr();
	ARMUR_CfdpInfo	cfdpInfoBuf;
	Object		cfdpInfoObj;
	
	CHKERR(sdr_begin_xn(sdr));

	cfdpInfoObj = (_armurConstants())->cfdpInfo;
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

	cfdpInfoObj = (_armurConstants())->cfdpInfo;
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

	cfdpInfoObj = (_armurConstants())->cfdpInfo;
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
	GET_OBJ_POINTER(sdr, ARMUR_CfdpInfo, cfdpInfo, (_armurConstants())->cfdpInfo);
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
		CHKERR(sdr_begin_xn(sdr));
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
		 * or something that might cause the cfdp_get_event to block infinitely
		 * implement it as a thread ? */

		cfdp_decompress_number(&srcNbr, &transactionId.sourceEntityNbr);
		cfdp_decompress_number(&txnNbr, &transactionId.transactionNbr);

		if (type == CfdpTransactionFinishedInd
			&& srcNbr == cfdpInfo->srcNbr)
			//&& txnNbr == cfdpInfo->txnNbr)//JIGI
		{
			/*	Now the download has been finished.	*/
			printf("Download has been finished.\n");//JIGI
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
	Object			imageObj;
	Object			imageElt;
				OBJ_POINTER(ARMUR_CfdpInfo, cfdpInfo);
	char			archiveNameBuf[SDRSTRING_BUFSZ];
	char			imageName[FILENAME_LEN_MAX];
	char			pathName[PATHNAME_LEN_MAX];
	char			pathNameTmp[PATHNAME_LEN_MAX];
	char			installDir[PATHNAME_LEN_MAX];
	int			imageType;
	struct archive		*a;
	struct archive_entry	*entry;
	int			result;
	time_t			installedTime = getCtime();

	CHKERR(sdr_begin_xn(sdr));
	GET_OBJ_POINTER(sdr, ARMUR_CfdpInfo, cfdpInfo, (_armurConstants())->cfdpInfo);
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
		imageType = armurFindImage(imageName, &imageObj, &imageElt);
		if (imageElt == 0)
		{
			/*	TODO: Append error message	*/
			archive_read_free(a);
			return -1;
		}

		/*	Retain the image reference in the queue of ARMUR DB.	*/
		if (enqueueImage(imageObj, imageType) < 0)
		{
			/*	TODO: Append error message	*/
			//putErrmsg("Can't enqueue image", imageName);
			archive_read_free(a);
			return -1;
		}

		getInstallDir(imageType, installDir);
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
		setInstallTimestamp(imageObj, installedTime);

		if (archive_read_data_skip(a) != ARCHIVE_OK)
		{
			archive_read_free(a);
			return -1;
		}
	}
	archive_read_free(a);
	oK(remove(archiveNameBuf));

	return 0;
}

int	armurRestart()
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	*armurdb = _armurConstants();
	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
			OBJ_POINTER(ARMUR_Image, image);
	Object		imageRefAddr;
	Object		imageRefElt;
	ARMUR_ImageRef	imageRef;
	int		level;

	for (level = 0; level < 3; level++)
	{
		CHKERR(sdr_begin_xn(sdr));
		while ((imageRefElt = sdr_list_first(sdr, armurdb->queue[level])) != 0)
		{
			/*	Item was found. Let's get the image.			*/
			imageRefAddr = sdr_list_data(sdr, imageRefElt);
			sdr_read(sdr, (char *)&imageRef, imageRefAddr,
					sizeof(ARMUR_ImageRef));
			GET_OBJ_POINTER(sdr, ARMUR_Image, image, imageRef.obj);

			/*	We will restart applications only if the restart mask
			 *	is not zero, i.e., the items are not already restarted
			 *	(which might have been restarted because of some
			 *	unexpected system reboot)				*/
			if (armurvdb->restartMask == 0)
			{
				/*	Delete the element from the queue.		*/
				sdr_free(sdr, imageRefAddr);
				oK(sdr_list_delete(sdr, imageRefElt, NULL, NULL));
				if (sdr_end_xn(sdr))
				{
					return -1;
				}
				CHKERR(sdr_begin_xn(sdr));	/*	For next loop	*/
				continue;
			}

			switch (level)
			{
			case ARMUR_LV0:
			/*	Items in level 0 will set the restart mask to 0.	*/
				armurvdb->restartMask = 0;
				break;

			case ARMUR_LV1:
			/*	Items in level 1 will xor the restart mask.
			 *	(i.e., will turn off corresponding bit flags.)		*/
				armurvdb->restartMask ^= image->protocol;
				break;

			case ARMUR_LV2:
			/*	Items in level 2 will be restarted only if
			 *	it has not been restarted by the upper level.
			 *	(i.e., if the bit has not yet been turned off.		*/
				if ((armurvdb->restartMask & image->protocol) == 0)
				{
					sdr_free(sdr, imageRefAddr);
					oK(sdr_list_delete(sdr, imageRefElt, NULL, NULL));
					if (sdr_end_xn(sdr))
					{
						return -1;
					}
					CHKERR(sdr_begin_xn(sdr));/*	For next loop	*/
					continue;
				}
			}

			sdr_exit_xn(sdr);
			printf("%s will restart\n", image->name);//dbg
			if (image->restart() < 0)
			{
				return -1;
			}

			/*	Delete the element from the queue.			*/
			CHKERR(sdr_begin_xn(sdr));
			sdr_free(sdr, imageRefAddr);
			oK(sdr_list_delete(sdr, imageRefElt, NULL, NULL));
			if (sdr_end_xn(sdr))
			{
				return -1;
			}

			CHKERR(sdr_begin_xn(sdr));	/*	For next loop		*/
		}
		sdr_exit_xn(sdr);
	}

	CHKERR(sdr_begin_xn(sdr));
	armurvdb->restartMask = 0;
	sdr_exit_xn(sdr);

	return 0;
}
