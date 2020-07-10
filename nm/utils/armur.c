// 2020-06-29
// jigi

#include "armur.h"

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
		vdb->restartMask = 0;
		if ((vdb->restartQueue[ARMUR_RESTART_LV0] = sm_list_create(wm)) == 0
		|| (vdb->restartQueue[ARMUR_RESTART_LV1] = sm_list_create(wm)) == 0
		|| (vdb->restartQueue[ARMUR_RESTART_LV2] = sm_list_create(wm)) == 0
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

int armurInit()
{
	Sdr		sdr;
	Object		armurdbObject;
	ARMUR_DB	armurdbBuf;
	char 		*armurvdbName = _armurvdbName();
	ARMUR_Image	armurImageBuf;
	Object		elt;
	Object		obj;

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

		armurdbBuf.images[ARMUR_LIBS] = sdr_list_create(sdr);

		/*	ION core library	*/
		istrcpy(armurImageBuf.name, "libici.so", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ionRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_ALL;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_LIBS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}

		/*	BP library	*/
		istrcpy(armurImageBuf.name, "libbp.so", sizeof armurImageBuf.name);
		armurImageBuf.restart = &bpRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_BP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_LIBS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}

		/*	LTP library	*/
		istrcpy(armurImageBuf.name, "libltp.so", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ltpRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_LTP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_LIBS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}

		/*	CFDP library	*/
		istrcpy(armurImageBuf.name, "libcfdp.so", sizeof armurImageBuf.name);
		armurImageBuf.restart = &cfdpRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_CFDP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_LIBS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}

		/*		Default daemon applications			*/

		armurdbBuf.images[ARMUR_APPS] = sdr_list_create(sdr);
		
		/*	ION daemons	*/
		istrcpy(armurImageBuf.name, "rfxclock", sizeof armurImageBuf.name);
		armurImageBuf.restart = &rfxclockRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_ION;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}

		/*	BP daemons	*/
		istrcpy(armurImageBuf.name, "bpclock", sizeof armurImageBuf.name);
		armurImageBuf.restart = &bpclockRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_BP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "bptransit", sizeof armurImageBuf.name);
		armurImageBuf.restart = &bptransitRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_BP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "ipnfw", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ipnfwRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_BP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "ipnadminep", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ipnadminepRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_BP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "bpclm", sizeof armurImageBuf.name);
		armurImageBuf.restart = &bpclmRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_BP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "ltpcli", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ltpcliRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_BP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "ltpclo", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ltpcloRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_BP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}

		/*	LTP daemons	*/
		istrcpy(armurImageBuf.name, "ltpclock", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ltpclockRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_LTP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "ltpdeliv", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ltpdelivRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_LTP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "ltpmeter", sizeof armurImageBuf.name);
		armurImageBuf.restart = &ltpmeterRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_LTP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "udplsi", sizeof armurImageBuf.name);
		armurImageBuf.restart = &udplsiRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_LTP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "udplso", sizeof armurImageBuf.name);
		armurImageBuf.restart = &udplsoRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_LTP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}

		/*	CFDP daemons	*/
		istrcpy(armurImageBuf.name, "cfdpclock", sizeof armurImageBuf.name);
		armurImageBuf.restart = &cfdpclockRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_CFDP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
		}
		istrcpy(armurImageBuf.name, "bputa", sizeof armurImageBuf.name);
		armurImageBuf.restart = &bputaRestart;
		armurImageBuf.installedTime = getCtime();
		armurImageBuf.protocol = ARMUR_CFDP;
		obj = sdr_malloc(sdr, sizeof(ARMUR_Image));
		if (obj)
		{
			elt = sdr_list_insert_last(sdr, armurdbBuf.images[ARMUR_APPS], obj);
			if (elt == 0)
			{
				sdr_cancel_xn(sdr);
				putErrmsg("No space for database.", NULL);
			}
			sdr_write(sdr, obj, (char *)&armurImageBuf, sizeof(ARMUR_Image));
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

//int armurStart()
//{
//	Sdr		sdr = getIonsdr();
//	PsmPartition	wm = getIonwm();
//	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
//	Object		armurdbobj = _armurdbObject(NULL);
//	ARMUR_DB	armurdb;
//	PsmAddress	elt;
//
//	CHKERR(sdr_begin_xn(sdr));
//	sdr_exit_xn(sdr);
//
//	return 0;
//}

//void armurStop()
//{
//	Sdr		sdr = getIonsdr();
//	PsmPartition	wm = getIonwm();
//	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
//	PsmAddress	elt;
//
//	CHKVOID(sdr_begin_xn(sdr));
//	sdr_exit_xn(sdr);
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

int	armurParseImageName(char *imageName)
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

int	armurFindImage(char *imageName, Object *imageObj, Object *imageElt)
{
	Sdr	sdr = getIonsdr();
	Object	elt = 0;
	int	imageType;

	CHKERR(ionLocked());
	CHKERR(imageName);
	CHKERR(imageObj);
	CHKERR(imageElt);

	imageType = armurParseImageName(imageName);
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
		*imageElt = 0;
		return -1;
	}

	*imageObj = sdr_list_data(sdr, elt);
	*imageElt = elt;
	return imageType;
}

int	armurEnqueue(char *imageName)
{
	Sdr		sdr = getIonsdr();
	PsmPartition	ionwm = getIonwm();
			OBJ_POINTER(ARMUR_Image, image);
	ARMUR_VDB	*armurvdb = _armurvdb(NULL);
	Object		imageObj;
	Object		imageElt;
	PsmAddress	imageRefElt = 0;
	PsmAddress	imageRefAddr;
	ARMUR_ImageRef	*imageRef;
	int		imageType;

	CHKERR(imageName);

	if (*imageName == '\0')
	{
		writeMemoNote("[?] Invalid image name", imageName);
		return 0;
	}

	CHKERR(sdr_begin_xn(sdr));
	imageType = armurFindImage(imageName, &imageObj, &imageElt);
	if (imageElt == 0)
	{
		sdr_exit_xn(sdr);
		putErrmsg("Can't find image.", imageName);
		return 0;
	}

	if ((imageRefAddr = psm_zalloc(ionwm, sizeof(ARMUR_ImageRef))) == 0)
	{
		sdr_exit_xn(sdr);
		putErrmsg("Not enough working memory.", NULL);
		return -1;
	}

	switch (imageType)
	{
	case ARMUR_CORE_LIBRARY:
		imageRefElt = sm_list_insert_last(ionwm,
			armurvdb->restartQueue[ARMUR_RESTART_LV0], imageRefAddr);
		break;
	
	case ARMUR_PROTOCOL_LIBRARY:
		imageRefElt = sm_list_insert_last(ionwm,
			armurvdb->restartQueue[ARMUR_RESTART_LV1], imageRefAddr);
		break;

	case ARMUR_APPLICATION:
		imageRefElt = sm_list_insert_last(ionwm,
			armurvdb->restartQueue[ARMUR_RESTART_LV2], imageRefAddr);
	}

	if (imageRefElt == 0)
	{
		psm_free(ionwm, imageRefAddr);
		sdr_exit_xn(sdr);
		putErrmsg("Not enough working memory.", NULL);
		return -1;
	}

	imageRef = (ARMUR_ImageRef *)psp(ionwm, imageRefAddr);
	imageRef->obj = imageObj;
	GET_OBJ_POINTER(sdr, ARMUR_Image, image, imageObj);
	armurvdb->restartMask |= image->protocol;

	sdr_exit_xn(sdr);
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

int	armurRestart()
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
	ARMUR_VDB	*vdb = _armurvdb(NULL);
			OBJ_POINTER(ARMUR_Image, image);
	PsmAddress	elt;
	ARMUR_ImageRef	*imageRef;
	int level;

	for (level = 0; level < 3; level++)
	{
		while ((elt = sm_list_first(wm, vdb->restartQueue[level])) != 0)
		{
			CHKERR(sdr_begin_xn(sdr));

			/*	Item was found. Let's get the image.			*/
			imageRef = (ARMUR_ImageRef *)psp(wm, sm_list_data(wm, elt));
			GET_OBJ_POINTER(sdr, ARMUR_Image, image, imageRef->obj);

			printf("%s\n", image->name);//dbg
			/*	Now that we know the image address in SDR,
			 *	we will delete the element from the restart queue.	*/
			psm_free(wm, sm_list_data(wm, elt));
			oK(sm_list_delete(wm, elt, NULL, NULL));

			/*	We will restart protocols & applications only if the
			 *	restart mask is not zero.				*/
			if (vdb->restartMask == 0)
			{
				sdr_exit_xn(sdr);
				continue;
			}

			switch (level)
			{
			case ARMUR_RESTART_LV0:
			/*	Items in level 0 will set the restart mask to 0.	*/
				vdb->restartMask = 0;
				break;

			case ARMUR_RESTART_LV1:
			/*	Items in level 1 will xor the restart mask.
			 *	(i.e., will turn off corresponding bit flags.)		*/
				vdb->restartMask ^= image->protocol;
				break;

			case ARMUR_RESTART_LV2:
			/*	Items in level 2 will be restarted only if
			 *	it has not been restarted by the upper level.
			 *	(i.e., if the bit has not yet been turned off.		*/
				if ((vdb->restartMask & image->protocol) == 0)
				{
					sdr_exit_xn(sdr);
					continue;
				}
			}

			sdr_exit_xn(sdr);
			printf("vdb->restartMask: %hhd\n", vdb->restartMask);//dbg
			printf("%s will restart\n", image->name);//dbg
			if (image->restart() < 0)
			{
				return -1;
			}
		}
	}

	vdb->restartMask = 0;

	return 0;
}
