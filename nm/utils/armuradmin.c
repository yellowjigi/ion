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

static void	executeWait(int tokenCount, char **tokens)
{
	if (tokenCount != 1)
	{
		SYNTAX_ERROR;
		return;
	}

	oK(armurWait());
	return;
}

static void	executeInstall(int tokenCount, char **tokens)
{
	if (tokenCount < 2)
	{
		printText("Install what?");
		return;
	}

	if (tokenCount > 2)
	{
		SYNTAX_ERROR;
		return;
	}

	oK(armurInstall(tokens[1]));
	return;
}

static void	executeRestart(int tokenCount, char **tokens)
{
	if (tokenCount != 1)
	{
		SYNTAX_ERROR;
		return;
	}

	oK(armurRestart());
	return;
}

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
	//JIGI
	//case ARMUR_STAT_INSTALLING:
	//	buffer = "installing";
	//	break;

	case ARMUR_STAT_RESTART_PENDING:
		buffer = "restart queue pending";
		break;

	//case ARMUR_STAT_LV0_PENDING:
	//	buffer = "lv0 restart queue pending";
	//	break;

	//case ARMUR_STAT_LV1_PENDING:
	//	buffer = "lv1 restart queue pending";
	//	break;

	//case ARMUR_STAT_LV2_PENDING:
	//	buffer = "lv2 restart queue pending";
	//	break;

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
	char		pathBuffer[SDRSTRING_BUFSZ];
	char		*path;

	if (sdr_string_read(sdr, pathBuffer, (getArmurConstants())->installPath[type]) < 0)
	{
		path = "?";
	}
	else
	{
		path = pathBuffer;
	}

	printText(path);
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
		type = ARMUR_LIBS;
	}
	else if (strcmp(tokens[2], "app") == 0)
	{
		type = ARMUR_APPS;
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
	char	*nameBuffer;
	char	*restartBuffer;
	char	*protocolBuffer;
	char	installedTimeBuffer[TIMESTAMPBUFSZ];
	char	buffer[1024];

	nameBuffer = image->name ? image->name : "unknown";
	restartBuffer = image->restart ? "configured" : "not configured";

	switch (image->protocol)
	{
	case ARMUR_ALL:
	case ARMUR_ION:
		protocolBuffer = "ION";
		break;

	case ARMUR_BP:
		protocolBuffer = "BP";
		break;

	case ARMUR_LTP:
		protocolBuffer = "LTP";
		break;
	
	case ARMUR_CFDP:
		protocolBuffer = "CFDP";
		break;
	
	default:
		protocolBuffer = "unknown";
	}

	writeTimestampUTC(image->installedTime, installedTimeBuffer);

	isprintf(buffer, sizeof buffer,
			"--------------------\n"
			"Name: %s\nRestart: %s\nProtocol:%s\nInstalled Time:%s",
			nameBuffer, restartBuffer, protocolBuffer, installedTimeBuffer);
	printText(buffer);
}

static void	infoImage(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();
		OBJ_POINTER(ARMUR_Image, image);
	Object	addr;
	Object	elt;

	if (tokenCount != 3)
	{
		SYNTAX_ERROR;
		return;
	}

	armurFindImage(tokens[2], &addr, &elt);
	if (elt == 0)
	{
		printText("Unknown image.");
	}

	CHKVOID(sdr_begin_xn(sdr));
	GET_OBJ_POINTER(sdr, ARMUR_Image, image, addr);
	sdr_exit_xn(sdr);

	printImage(image);
}

static void	printCfdp(ARMUR_CfdpInfo *cfdpInfo)
{
	Sdr	sdr = getIonsdr();
	char	archiveNameBuf[SDRSTRING_BUFSZ];
	char	*archiveName;
	char	buffer[1024];

	if (sdr_string_read(sdr, archiveNameBuf, cfdpInfo->archiveName) < 0)
	{
		archiveName = "unknown";
	}
	else
	{
		archiveName = archiveNameBuf;
	}

	isprintf(buffer, sizeof buffer,
			"Source number: %lu\nTransaction number: %lu\nArchive name: %s",
			cfdpInfo->srcNbr, cfdpInfo->txnNbr, archiveName);
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

	GET_OBJ_POINTER(sdr, ARMUR_CfdpInfo, cfdpInfo, (getArmurConstants())->cfdpInfo);
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
	char		libPathBuffer[SDRSTRING_BUFSZ];
	char		*libPath;
	char		appPathBuffer[SDRSTRING_BUFSZ];
	char		*appPath;
	char		buffer[1024];

	if (tokenCount != 2)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));
	if (sdr_string_read(sdr, libPathBuffer, armurdb->installPath[ARMUR_LIBS]) < 0)
	{
		libPath = "?";
	}
	else
	{
		libPath = libPathBuffer;
	}
	if (sdr_string_read(sdr, appPathBuffer, armurdb->installPath[ARMUR_APPS]) < 0)
	{
		appPath = "?";
	}
	else
	{
		appPath = appPathBuffer;
	}

	isprintf(buffer, sizeof buffer, "libPath: %s\nappPath: %s", libPath, appPath);
	printText(buffer);
}

static void	listImagesForType(int type, char mask)
{
	Sdr	sdr = getIonsdr();
		OBJ_POINTER(ARMUR_Image, image);
	Object	elt;

	for (elt = sdr_list_first(sdr, (getArmurConstants())->images[type]); elt;
			elt = sdr_list_next(sdr, elt))
	{
		GET_OBJ_POINTER(sdr, ARMUR_Image, image, sdr_list_data(sdr, elt));
		if (image->protocol & mask)
		{
			printImage(image);
		}
	}
}

static void	listImages(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();
	char	type;
	char	mask;

	if (tokenCount > 4)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));
	if (tokenCount == 2)
	{
		listImagesForType(ARMUR_LIBS, ARMUR_ALL);
		listImagesForType(ARMUR_APPS, ARMUR_ALL);
		sdr_exit_xn(sdr);
		return;
	}

	/*	tokenCount is 3 or 4	*/
	if (strcmp(tokens[2], "lib") == 0)
	{
		type = ARMUR_LIBS;
	}
	else if (strcmp(tokens[2], "app") == 0)
	{
		type = ARMUR_APPS;
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
		mask = ARMUR_ALL;
		break;
	
	case 4:
		if (strcmp(tokens[3], "core") == 0)
		{
			mask = ARMUR_ION;
		}
		else if (strcmp(tokens[3], "bp") == 0)
		{
			mask = ARMUR_BP;
		}
		else if (strcmp(tokens[3], "ltp") == 0)
		{
			mask = ARMUR_LTP;
		}
		else if (strcmp(tokens[3], "cfdp") == 0)
		{
			mask = ARMUR_CFDP;
		}
		else
		{
			SYNTAX_ERROR;
			sdr_exit_xn(sdr);
			return;
		}
	}

	listImagesForType(type, mask);
	
	sdr_exit_xn(sdr);
}

static void	listQueuesForLevel(int level)
{
	Sdr		sdr = getIonsdr();
	Object		imageRefObj;
	Object		imageRefElt;
			OBJ_POINTER(ARMUR_Image, image);
	ARMUR_ImageRef	imageRef;

	if (level < 0 || level > 2)
	{
		SYNTAX_ERROR;
		return;
	}

	for (imageRefElt = sdr_list_first(sdr, (getArmurConstants())->queue[level]);
		imageRefElt; imageRefElt = sdr_list_next(sdr, imageRefElt))
	{
		imageRefObj = sdr_list_data(sdr, imageRefElt);
		sdr_read(sdr, (char *)&imageRef, imageRefObj, sizeof(ARMUR_ImageRef));
		GET_OBJ_POINTER(sdr, ARMUR_Image, image, imageRef.obj);
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

		case 'w':
			if (attachToArmur() == 0)
			{
				executeWait(tokenCount, tokens);
			}
			return 0;

		case 's':
			if (attachToArmur() == 0)
			{
				executeInstall(tokenCount, tokens);
			}
			return 0;

		case 'r':
			if (attachToArmur() == 0)
			{
				executeRestart(tokenCount, tokens);
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
