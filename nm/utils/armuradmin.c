/*
 *
 *	umadmin.c:	Update Manager for ARMUR
 *	*** referring to the bp/utils/bpadmin.c ***
 *
 *	Author: jigi
 *	Date: 2020-06-27
 *
 *									*/

#include "armur.h"

static void	printText(char *text)
{
	PUTS(text);
}

static void	printSyntaxError(int lineNbr)
{
	char	buffer[80];

	isprintf(buffer, sizeof buffer, "Syntax error at line %d of umadmin.c",
			lineNbr);
	printText(buffer);
}

#define	SYNTAX_ERROR	printSyntaxError(__LINE__)

static void	initializeArmur(int tokenCount, char **tokens)
{
	if (tokenCount != 1)
	{
		SYNTAX_ERROR;
		return;
	}

	if (armurInit() < 0)
	{
		putErrmsg("armuradmin can't initialize ARMUR.", NULL);
		return;
	}
}

static int	attachToArmur()
{
	if (armurAttach() < 0)
	{
		printText("ARMUR not initialized yet.");
		return -1;
	}

	return 0;
}

static void	addCfdpInfo(char *token, char *info)
{
	if (strcmp(token, "src") == 0)
	{
		armurUpdateCfdpSrcNbr(strtouvast(info));
		return;
	}
	
	if (strcmp(token, "txn") == 0)
	{
		armurUpdateCfdpTxnNbr(strtouvast(info));
		return;
	}

	if (strcmp(token, "ar") == 0)
	{
		armurUpdateCfdpArchiveName(info);
		return;
	}

	SYNTAX_ERROR;
}

static void	executeAdd(int tokenCount, char **tokens)
{
	char	*protocol;
	char	protocolFlag = 0;

	if (tokenCount < 2)
	{
		printText("Add what?");
		return;
	}

	if (strcmp(tokens[1], "cfdp") == 0)
	{
		if (tokenCount != 4)
		{
			SYNTAX_ERROR;
			return;
		}

		addCfdpInfo(tokens[2], tokens[3]);
		return;
	}

	if (strcmp(tokens[1], "lib") == 0 || strcmp(tokens[1], "app") == 0)
	{
		if (tokenCount != 4)
		{
			SYNTAX_ERROR;
			return;
		}

		while ((protocol = strtok(tokens[3], ",")) != NULL)
		{
			if (strcmp(protocol, "core") == 0)
			{
				protocolFlag |= ARMUR_CORE;
			}
			else if (strcmp(protocol, "ltp") == 0)
			{
				protocolFlag |= ARMUR_LTP;
			}
			else if (strcmp(protocol, "bp") == 0)
			{
				protocolFlag |= ARMUR_BP;
			}
			else if (strcmp(protocol, "cfdp") == 0)
			{
				protocolFlag |= ARMUR_CFDP;
			}
			else
			{
				SYNTAX_ERROR;
				return;
			}
			tokens[3] = NULL;
		}

		armurAddImage(tokens[2], protocolFlag);
		return;
	}

	SYNTAX_ERROR;
}

//static void	executeChange(int tokenCount, char **tokens)
//{
//	char		*path;
//
//	if (tokenCount < 2)
//	{
//		printText("Change what?");
//		return;
//	}
//
//	if (strcmp(tokens[1], "path") == 0)
//	{
//		if (tokenCount != 4)
//		{
//			SYNTAX_ERROR;
//			return;
//		}
//
//		armurUpdatePath(tokens[2], tokens[3]);
//		return;
//	}
//
//	SYNTAX_ERROR;
//}

static void	printStat(char stat)
{
	char	*buffer;

	switch (stat)
	{
	case ARMUR_STAT_IDLE:
		buffer = "idle";
		break;

	case ARMUR_STAT_DOWNLOADING:
		buffer = "downloading";
		break;

	case ARMUR_STAT_DOWNLOADED:
		buffer = "downloaded";
		break;

	case ARMUR_STAT_INSTALLED:
		buffer = "installed";
		break;

	default:
		buffer = "unknown";
	}

	printText(buffer);
}

static void	infoStat(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();

	if (tokenCount != 2)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));

	printStat((getArmurConstants())->stat);

	sdr_exit_xn(sdr);
}

static void	printPath(int type)
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	*armurdb = getArmurConstants();
	char		buf[SDRSTRING_BUFSZ];
	char		*pathBuf;

	if (armurdb->installPath[type])
	{
		sdr_string_read(sdr, buf, armurdb->installPath[type]);
		pathBuf = buf;
	}
	else
	{
		pathBuf = "?";
	}

	printText(pathBuf);
}

static void	infoPath(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();
	int	type;

	if (tokenCount != 3)
	{
		SYNTAX_ERROR;
		return;
	}

	if (strcmp(tokens[2], "lib") == 0)
	{
		type = ARMUR_LIB;
	}
	else if (strcmp(tokens[2], "app") == 0)
	{
		type = ARMUR_APP;
	}
	else
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));

	printPath(type);

	sdr_exit_xn(sdr);
}

static void	printImage(ARMUR_Image *image)
{
	char	*nameBuf;
	char	*typeBuf;
	char	*protocolBuf;
	char	protocolStr[32] = { 0 };
	char	*restartBuf;
	char	installedTimeBuf[TIMESTAMPBUFSZ];
	char	buffer[1024];
	int	p;

	nameBuf = image->name ? image->name : "unknown";

	if (image->type == ARMUR_LIB)
	{
		typeBuf = "library";
	}
	else if (image->type == ARMUR_APP)
	{
		typeBuf = "daemon application";
	}
	else
	{
		typeBuf = "unknown";
	}

	if (image->protocol < ARMUR_RESERVED)
	{
		for (p = ARMUR_CORE; p < ARMUR_RESERVED; p <<= 1)
		{
			if (p & image->protocol)
			{
				switch (p)
				{
				case ARMUR_CORE:
					strcat(protocolStr, "CORE,");
					break;

				case ARMUR_LTP:
					strcat(protocolStr, "LTP,");
					break;
				
				case ARMUR_BP:
					strcat(protocolStr, "BP,");
					break;

				case ARMUR_CFDP:
					strcat(protocolStr, "CFDP,");
					break;
				}
			}
		}
		protocolStr[strlen(protocolStr) - 1] = '\0';
		protocolBuf = protocolStr;
	}
	else
	{
		protocolBuf = "unknown";
	}

	restartBuf = image->restartFnObj ? "configured" : "unknown";
	writeTimestampUTC(image->installedTime, installedTimeBuf);

	isprintf(buffer, sizeof buffer,
			"--------------------\n"
			"Name: %s\n"
			"Type: %s\n"
			"Protocol: %s\n"
			"RestartFn: %s\n"
			"Installed: %s",
			nameBuf, typeBuf, protocolBuf, restartBuf, installedTimeBuf);
	printText(buffer);
}

static void	infoImage(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();
		OBJ_POINTER(ARMUR_Image, image);
	Object	obj;
	Object	elt;
	int	imageType;

	if (tokenCount != 3)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));

	armurParseImageName(tokens[2], &imageType, NULL);
	armurFindImage(tokens[2], imageType, &obj, &elt);
	if (elt == 0)
	{
		sdr_exit_xn(sdr);
		printText("Unknown image.");
		return;
	}
	GET_OBJ_POINTER(sdr, ARMUR_Image, image, obj);
	printImage(image);

	sdr_exit_xn(sdr);
}

static void	printCfdp(ARMUR_CfdpInfo *cfdpInfo)
{
	Sdr	sdr = getIonsdr();
	char	buf[SDRSTRING_BUFSZ];
	char	*archiveNameBuf;
	char	buffer[1024];

	if (cfdpInfo->archiveName)
	{
		sdr_string_read(sdr, buf, cfdpInfo->archiveName);
		archiveNameBuf = buf;
	}
	else
	{
		archiveNameBuf = "unknown";
	}

	isprintf(buffer, sizeof buffer,
			"Source number: %lu\nTransaction number: %lu\nArchive name: %s",
			cfdpInfo->srcNbr, cfdpInfo->txnNbr, archiveNameBuf);
	printText(buffer);
}

static void	infoCfdp(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();
		OBJ_POINTER(ARMUR_CfdpInfo, cfdpInfo);

	if (tokenCount != 2)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));

	GET_OBJ_POINTER(sdr, ARMUR_CfdpInfo, cfdpInfo, (getArmurVdb())->cfdpInfo);
	printCfdp(cfdpInfo);

	sdr_exit_xn(sdr);
}

static void	executeInfo(int tokenCount, char **tokens)
{
	if (tokenCount < 2)
	{
		printText("Information on what?");
		return;
	}

	if (strcmp(tokens[1], "stat") == 0)
	{
		infoStat(tokenCount, tokens);
		return;
	}

	if (strcmp(tokens[1], "path") == 0)
	{
		infoPath(tokenCount, tokens);
		return;
	}

	if (strcmp(tokens[1], "image") == 0)
	{
		infoImage(tokenCount, tokens);
		return;
	}

	if (strcmp(tokens[1], "cfdp") == 0)
	{
		infoCfdp(tokenCount, tokens);
		return;
	}

	SYNTAX_ERROR;
}

static void	listPaths(int tokenCount, char **tokens)
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	*armurdb = getArmurConstants();
	char		buf1[SDRSTRING_BUFSZ];
	char		*libPathBuf;
	char		buf2[SDRSTRING_BUFSZ];
	char		*appPathBuf;
	char		buffer[1024];

	if (tokenCount != 2)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));
	if (armurdb->installPath[ARMUR_LIB])
	{
		sdr_string_read(sdr, buf1, armurdb->installPath[ARMUR_LIB]);
		libPathBuf = buf1;
	}
	else
	{
		libPathBuf = "?";
	}
	
	if (armurdb->installPath[ARMUR_APP])
	{
		sdr_string_read(sdr, buf2, armurdb->installPath[ARMUR_APP]);
		appPathBuf = buf2;
	}
	else
	{
		appPathBuf = "?";
	}
	sdr_exit_xn(sdr);

	isprintf(buffer, sizeof buffer, "libPath: %s\nappPath: %s", libPathBuf, appPathBuf);
	printText(buffer);
}

static void	listImagesForProtocol(int type, int protocolMask)
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	*armurdb = getArmurConstants();
			OBJ_POINTER(ARMUR_Image, image);
	Object		obj;
	Object		elt;
	int		p;
	int		i;

	for (p = ARMUR_CORE; p < ARMUR_RESERVED; p <<= 1)
	{
		if (p & protocolMask)
		{
			i = armurGetProtocolIndex(p);
			for (elt = sdr_list_first(sdr, armurdb->images[type][i]);
				elt; elt = sdr_list_next(sdr, elt))
			{
				obj = sdr_list_data(sdr, elt);
				GET_OBJ_POINTER(sdr, ARMUR_Image, image, obj);
				printImage(image);
			}
		}
	}
}

static void	listImages(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();
	int	type;
	int	protocolMask;

	if (tokenCount > 4)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));
	if (tokenCount == 2)
	{
		listImagesForProtocol(ARMUR_LIB, ARMUR_ALL);
		listImagesForProtocol(ARMUR_APP, ARMUR_ALL);
		sdr_exit_xn(sdr);
		return;
	}

	/*	tokenCount is 3 or 4	*/
	if (strcmp(tokens[2], "lib") == 0)
	{
		type = ARMUR_LIB;
	}
	else if (strcmp(tokens[2], "app") == 0)
	{
		type = ARMUR_APP;
	}
	else
	{
		SYNTAX_ERROR;
		sdr_exit_xn(sdr);
		return;
	}

	switch (tokenCount)
	{
	case 3:
		protocolMask = ARMUR_ALL;
		break;
	
	case 4:
		if (strcmp(tokens[3], "core") == 0)
		{
			protocolMask = ARMUR_CORE;
		}
		else if (strcmp(tokens[3], "ltp") == 0)
		{
			protocolMask = ARMUR_LTP;
		}
		else if (strcmp(tokens[3], "bp") == 0)
		{
			protocolMask = ARMUR_BP;
		}
		else if (strcmp(tokens[3], "cfdp") == 0)
		{
			protocolMask = ARMUR_CFDP;
		}
		else
		{
			SYNTAX_ERROR;
			sdr_exit_xn(sdr);
			return;
		}
	}

	listImagesForProtocol(type, protocolMask);
	
	sdr_exit_xn(sdr);
}

static void	listQueuesForLevel(int level)
{
	Sdr		sdr = getIonsdr();
			OBJ_POINTER(ARMUR_Image, image);
	PsmPartition	wm = getIonwm();
	ARMUR_ImageRef	*imageRef;
	PsmAddress	elt;

	if (level < 0 || level > 2)
	{
		SYNTAX_ERROR;
		return;
	}

	for (elt = sm_list_first(wm, (getArmurVdb())->restartQueue[level]);
		elt; elt = sm_list_next(wm, elt))
	{
		imageRef = (ARMUR_ImageRef *)psp(wm, sm_list_data(wm, elt));
		GET_OBJ_POINTER(sdr, ARMUR_Image, image, imageRef->imageObj);
		printImage(image);
	}
}

static void	listQueues(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();

	CHKVOID(sdr_begin_xn(sdr));
	switch (tokenCount)
	{
	case 2:
		listQueuesForLevel(ARMUR_LV0);
		listQueuesForLevel(ARMUR_LV1);
		listQueuesForLevel(ARMUR_LV2);
		break;
	
	case 3:
		listQueuesForLevel(atoi(tokens[2]));
		break;

	default:
		SYNTAX_ERROR;
	}

	sdr_exit_xn(sdr);
}

static void	executeList(int tokenCount, char **tokens)
{
	if (tokenCount < 2)
	{
		printText("List what?");
		return;
	}

	if (strcmp(tokens[1], "path") == 0)
	{
		listPaths(tokenCount, tokens);
		return;
	}

	if (strcmp(tokens[1], "image") == 0)
	{
		listImages(tokenCount, tokens);
		return;
	}

	if (strcmp(tokens[1], "queue") == 0)
	{
		listQueues(tokenCount, tokens);
		return;
	}

	SYNTAX_ERROR;
}

//static void	executeStart(int tokenCount, char **tokens)
//{
//	if (tokenCount != 2)
//	{
//		SYNTAX_ERROR;
//		return;
//	}
//
//	if (strcmp(tokens[1], "wait") == 0)
//	{
//		if (armurWait() < 0)
//		{
//			putErrmsg("armurWait failed.", NULL);
//		}
//		return;
//	}
//
//	if (strcmp(tokens[1], "install") == 0)
//	{
//		if (armurInstall() < 0)
//		{
//			putErrmsg("armurInstall failed.", NULL);
//		}
//		return;
//	}
//
//	if (strcmp(tokens[1], "restart") == 0)
//	{
//		if (armurRestart() < 0)
//		{
//			putErrmsg("armurRestart failed.", NULL);
//		}
//		return;
//	}
//
//	SYNTAX_ERROR;
//}

static int	processLine(char *line, int lineLength, int *rc)
{
	int		tokenCount;
	char		*cursor;
	int		i;
	char		*tokens[9];

	tokenCount = 0;
	for (cursor = line, i = 0; i < 9; i++)
	{
		if (*cursor == '\0')
		{
			tokens[i] = NULL;
		}
		else
		{
			findToken(&cursor, &(tokens[i]));
			if (tokens[i])
			{
				tokenCount++;
			}
		}
	}

	if (tokenCount == 0)
	{
		return 0;
	}

	/*	Skip over any trailing whitespace.			*/

	while (isspace((int) *cursor))
	{
		cursor++;
	}

	/*	Make sure we've parsed everything.			*/

	if (*cursor != '\0')
	{
		printText("Too many tokens.");
		return 0;
	}

	/*	Have parsed the command.  Now execute it.		*/

	switch (*(tokens[0]))		/*	Command code.		*/
	{
		case 0:			/*	Empty line.		*/
		case '#':		/*	Comment.		*/
			return 0;

		case '1':
			initializeArmur(tokenCount, tokens);
			return 0;

		case 'a':
			if (attachToArmur() == 0)
			{
				executeAdd(tokenCount, tokens);
			}
			return 0;

		//case 'c':
		//	if (attachToArmur() == 0)
		//	{
		//		executeChange(tokenCount, tokens);
		//	}
		//	return 0;

		case 'i':
			if (attachToArmur() == 0)
			{
				executeInfo(tokenCount, tokens);
			}
			return 0;

		case 'l':
			if (attachToArmur() == 0)
			{
				executeList(tokenCount, tokens);
			}
			return 0;

		case 's':
			if (attachToArmur() == 0)
			{
				if (tokenCount < 2)
				{
					printText("Can't start ARMUR: no nm_agent command.");
				}
				else
				{
					if (armurStart(tokens[1]) < 0)
					{
						putErrmsg("Can't start ARMUR.", NULL);
					}
				}
			}
			return 0;

		case 'q':
			return 1;	/*	End program.		*/

		default:
			printText("Invalid command.");
			return 0;
	}
}

int	main(int argc, char **argv)
{
	char	*cmdFileName = (argc > 1 ? argv[1] : NULL);
	int	rc = 0;
	int	cmdFile;
	char	line[256];
	int	len;
	
	if (cmdFileName == NULL)		/*	Interactive.	*/
	{
		cmdFile = fileno(stdin);
		while (1)
		{
			printf(": ");
			fflush(stdout);
			if (igets(cmdFile, line, sizeof line, &len) == NULL)
			{
				if (len == 0)
				{
					break;
				}

				putErrmsg("igets failed.", NULL);
				break;		/*	Out of loop.	*/
			}
			
			if (len == 0)
			{
				continue;
			}

			if (processLine(line, len, &rc))
			{
				break;		/*	Out of loop.	*/
			}
		}
	}
	else					/*	Scripted.	*/
	{
		cmdFile = iopen(cmdFileName, O_RDONLY, 0777);
		if (cmdFile < 0)
		{
			PERROR("Can't open command file");
		}
		else
		{
			while (1)
			{
				if (igets(cmdFile, line, sizeof line, &len)
						== NULL)
				{
					if (len == 0)
					{
						break;	/*	Loop.	*/
					}

					putErrmsg("igets failed.", NULL);
					break;		/*	Loop.	*/
				}

				if (len == 0
				|| line[0] == '#')	/*	Comment.*/
				{
					continue;
				}

				if (processLine(line, len, &rc))
				{
					break;	/*	Out of loop.	*/
				}
			}

			close(cmdFile);
		}
	}

	writeErrmsgMemos();
	printText("Stopping armuradmin.");
	ionDetach();
	return rc;
}
