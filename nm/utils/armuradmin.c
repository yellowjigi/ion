/*
 *
 *	armuradmin.c:	Update Manager for ARMUR
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

	isprintf(buffer, sizeof buffer, "Syntax error at line %d of armuradmin.c",
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
	
	//if (strcmp(token, "txn") == 0)
	//{
	//	armurUpdateCfdpTxnNbr(strtouvast(info));
	//	return;
	//}

	if (strcmp(token, "ar") == 0)
	{
		armurUpdateCfdpArchiveName(info);
		return;
	}

	SYNTAX_ERROR;
}

static void	executeAdd(int tokenCount, char **tokens)
{
	int	layer;
	int	apptype;

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

	if (strcmp(tokens[1], "image") == 0)
	{
		if (tokenCount < 5)
		{
			SYNTAX_ERROR;
			return;
		}

		switch (*(tokens[2]))
		{
		case '0':
			if (tokenCount != 5)
			{
				SYNTAX_ERROR;
				return;
			}

			armurAddImageLv0(tokens[3], tokens[4]);
			break;

		case '1':
			if (tokenCount != 6)
			{
				SYNTAX_ERROR;
				return;
			}

			switch (*(tokens[5]))
			{
			case 'a':
				layer = ARMUR_LAYER_APPLICATION;
				break;

			case 'b':
				layer = ARMUR_LAYER_BUNDLE;
				break;

			case 'c':
				layer = ARMUR_LAYER_CONVERGENCE;
				break;

			default:
				SYNTAX_ERROR;
				return;
			}

			armurAddImageLv1(tokens[3], tokens[4], layer);
			break;

		case '2':
			if (tokenCount != 6)
			{
				SYNTAX_ERROR;
				return;
			}

			switch (*(tokens[5]))
			{
			case 'd':
				apptype = ARMUR_APPTYPE_DAEMON;
				break;

			case 'n':
				apptype = ARMUR_APPTYPE_DAEMON;
				break;

			default:
				SYNTAX_ERROR;
				return;
			}

			armurAddImageLv2(tokens[3], tokens[4], apptype);
			break;

		default:
			SYNTAX_ERROR;
		}

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

	printf("stat: %hhd.\n", stat);
	switch (stat)
	{
	case ARMUR_STAT_IDLE:
		buffer = "idle";
		break;

	//case ARMUR_STAT_DOWNLOADING:
	//	buffer = "downloading...";
	//	break;

	case ARMUR_STAT_DOWNLOADED:
		buffer = "installing...";
		break;

	case ARMUR_STAT_INSTALLED:
		buffer = "restarting...";
		break;

	case ARMUR_STAT_REPORT_PENDING:
		buffer = "reporting...";
		break;

	case ARMUR_STAT_FIN:
		buffer = "finishing...";
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
		type = ARMUR_IMAGETYPE_LIB;
	}
	else if (strcmp(tokens[2], "app") == 0)
	{
		type = ARMUR_IMAGETYPE_APP;
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
	char	*typeBuf = NULL;
	char	*packageNameBuf;
	char	installedTimeBuf[TIMESTAMPBUFSZ];
	char	buffer[1024];

	nameBuf = image->name ? image->name : "unknown";

	switch (image->level)
	{
	case ARMUR_LEVEL_0:
		typeBuf = "core library";
		break;

	case ARMUR_LEVEL_1:
		switch (image->tag)
		{
		case ARMUR_LAYER_APPLICATION:
			typeBuf = "application-layer library";
			break;

		case ARMUR_LAYER_BUNDLE:
			typeBuf = "bundle-layer library";
			break;

		case ARMUR_LAYER_CONVERGENCE:
			typeBuf = "convergence-layer library";
		}
		break;

	case ARMUR_LEVEL_2:
		switch (image->tag)
		{
		case ARMUR_APPTYPE_DAEMON:
			typeBuf = "daemon application";
			break;

		case ARMUR_APPTYPE_NORMAL:
			typeBuf = "application";
		}
	}

	packageNameBuf = image->packageName ? image->packageName : "unknown";

	writeTimestampUTC(image->installedTime, installedTimeBuf);

	isprintf(buffer, sizeof buffer,
			"--------------------\n"
			"Name: %s\n"
			"Type: %s\n"
			"Package: %s\n"
			"Installed: %s",
			nameBuf, typeBuf, packageNameBuf, installedTimeBuf);
	printText(buffer);
}

static void	infoImage(int tokenCount, char **tokens)
{
	Sdr		sdr = getIonsdr();
			OBJ_POINTER(ARMUR_Image, image);
	ARMUR_VImage	*vimage;
	PsmAddress	addr;

	if (tokenCount != 3)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));

	armurFindImage(tokens[2], &vimage, &addr);
	if (addr == 0)
	{
		sdr_exit_xn(sdr);
		printText("Unknown image.");
		return;
	}
	GET_OBJ_POINTER(sdr, ARMUR_Image, image, vimage->addr);
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

	isprintf(buffer, sizeof buffer, "Source number: %lu\nArchive name: %s",
			cfdpInfo->srcNbr, archiveNameBuf);
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

	if (tokenCount != 2)
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));

	printf("libPath: ");
	printPath(ARMUR_IMAGETYPE_LIB);
	printf("appPath: ");
	printPath(ARMUR_IMAGETYPE_APP);

	sdr_exit_xn(sdr);
}

static void	listImagesForLevel(int level, char *packageName)
{
	Sdr		sdr = getIonsdr();
	ARMUR_DB	*armurdb = getArmurConstants();
			OBJ_POINTER(ARMUR_Image, image);
	Object		elt;

	if (!(level == ARMUR_LEVEL_0 || level == ARMUR_LEVEL_1 || level == ARMUR_LEVEL_2))
	{
		printText("Unknown level.");
		return;
	}

	for (elt = sdr_list_first(sdr, armurdb->images[level]); elt;
		elt = sdr_list_next(sdr, elt))
	{
		GET_OBJ_POINTER(sdr, ARMUR_Image, image, sdr_list_data(sdr, elt));
		if (packageName)
		{
			if (strstr(image->packageName, packageName))
			{
				printImage(image);
			}
		}
		else
		{
			printImage(image);
		}
	}
}

static void	listImages(int tokenCount, char **tokens)
{
	Sdr	sdr = getIonsdr();
	int	level;

	if (tokenCount == 2)
	{
		CHKVOID(sdr_begin_xn(sdr));
		listImagesForLevel(ARMUR_LEVEL_0, NULL);
		listImagesForLevel(ARMUR_LEVEL_1, NULL);
		listImagesForLevel(ARMUR_LEVEL_2, NULL);
		sdr_exit_xn(sdr);
		return;
	}

	/*	tokenCount >= 3		*/
	if (strcmp(tokens[2], "0") == 0)
	{
		level = ARMUR_LEVEL_0;
	}
	else if (strcmp(tokens[2], "1") == 0)
	{
		level = ARMUR_LEVEL_1;
	}
	else if (strcmp(tokens[2], "2") == 0)
	{
		level = ARMUR_LEVEL_2;
	}
	else
	{
		SYNTAX_ERROR;
		return;
	}

	CHKVOID(sdr_begin_xn(sdr));
	if (tokenCount == 3)
	{
		listImagesForLevel(level, NULL);
	}
	else if (tokenCount == 4)
	{
		listImagesForLevel(level, tokens[3]);
	}
	else
	{
		SYNTAX_ERROR;
	}
	sdr_exit_xn(sdr);
}

static void	listQueuesForLevel(int level)
{
	Sdr		sdr = getIonsdr();
	PsmPartition	wm = getIonwm();
			OBJ_POINTER(ARMUR_Image, image);
	ARMUR_VImage	*vimage;
	PsmAddress	elt;

	if (!(level == ARMUR_LEVEL_0 || level == ARMUR_LEVEL_1 || level == ARMUR_LEVEL_2))
	{
		printText("Unknown level.");
		return;
	}

	for (elt = sm_list_first(wm, (getArmurVdb())->restartQueue[level]); elt;
		elt = sm_list_next(wm, elt))
	{
		vimage = (ARMUR_VImage *)psp(wm, sm_list_data(wm, elt));
		GET_OBJ_POINTER(sdr, ARMUR_Image, image, vimage->addr);
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
		listQueuesForLevel(ARMUR_LEVEL_0);
		listQueuesForLevel(ARMUR_LEVEL_1);
		listQueuesForLevel(ARMUR_LEVEL_2);
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
				switch (tokenCount)
				{
				case 1:
					if (armurStart(NULL) < 0)
					{
						putErrmsg("Can't start ARMUR.", NULL);
					}
					break;

				case 2:
					if (armurStart(tokens[1]) < 0)
					{
						putErrmsg("Can't start ARMUR.", NULL);
					}
					break;

				default:
					SYNTAX_ERROR;
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
