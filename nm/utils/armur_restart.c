// 2020-07-20
// jigi

#include "armur_restart.h"

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
int ionRestart()
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
int rfxclockRestart()
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
int bpRestart()
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
int bpclockRestart()
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
int bptransitRestart()
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
int ipnfwRestart()
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
int ipnadminepRestart()
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
int bpclmRestart()
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

int ltpcliRestart()
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
int ltpcloRestart()
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
int ltpRestart()
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
int ltpclockRestart()
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
int ltpdelivRestart()
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
int ltpmeterRestart()
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
int udplsiRestart()
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
int udplsoRestart()
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
int cfdpRestart()
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
int cfdpclockRestart()
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

int bputaRestart()
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
int nmagentRestart(char *startCmd)
{
	Sdr		sdr = getIonsdr();
	ARMUR_VDB	*armurvdb = getArmurVdb();
	pid_t		pid;

	CHKERR(sdr_begin_xn(sdr));

	/*	Stop	*/
	if ((pid = armurvdb->nmagentPid) != ERROR)
	{
		sm_TaskKill(pid, SIGTERM);
		armurvdb->nmagentPid = ERROR;
	}
	sdr_exit_xn(sdr);

	if (pid != ERROR)
	{
		while (sm_TaskExists(pid));
	}

	CHKERR(sdr_begin_xn(sdr));

	/*	Start	*/
	if (armurvdb->nmagentPid == ERROR || !sm_TaskExists(armurvdb->nmagentPid))
	{
		armurvdb->nmagentPid = pseudoshell(startCmd);
	}
	sdr_exit_xn(sdr);

	return 0;
}
