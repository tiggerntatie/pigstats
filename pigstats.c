/****************************************************************************

$Header: pigstats.c  Revision:1.31  Tue Nov 30 11:20:52 2004  ericd $
  
PigStats.c

PigStats: Half-Life log file analyzer and statistics generator.
Copyright (C) 2000  Eric Dennison

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*****************************************************************************/


/*
 QVCS Logging:
$Log: C:\Documents and Settings\All Users\Documents\WORK\PigStats\Archive\SourceDist\CSource\pigstats.d $
  
    Auto-added
  
  Revision 1.31  by: ericd  Rev date: Tue Nov 30 11:20:52 2004
    Revised to improve Linux build
  
  Revision 1.30  by: ericd  Rev date: Wed Aug 01 21:49:46 2001
    Initial release for custom TFC support for LP Boss (1.7a)
  
  Revision 1.29  by: ericd  Rev date: Thu Mar 29 22:11:18 2001
    rules.pig -> pigrules.txt
  
  Revision 1.28  by: ericd  Rev date: Tue Mar 27 22:00:00 2001
    Fixed memory overwrite bug in server thread.
  
  Revision 1.27  by: ericd  Rev date: Sun Mar 25 22:29:34 2001
    rule.pig name exclusion/inclusion.. debugging related to new logging
    standard.
  
  Revision 1.26  by: ericd  Rev date: Thu Mar 22 22:16:26 2001
    Multiple server threads, support for arbitrarily large file transfers,
    bad html character substitution, beginning rule.pig name exclusion
    code.
  
  Revision 1.25  by: ericd  Rev date: Thu Mar 15 19:45:14 2001
    1.5a  support for new logging standard
  
  Revision 1.24  by: ericd  Rev date: Tue Jan 30 19:57:58 2001
    Changed all pigfile references to lower case
  
  Revision 1.23  by: ericd  Rev date: Sun Jan 07 14:28:54 2001
    Version 1.5 candidate
  
  Revision 1.22  by: ericd  Rev date: Sat Nov 11 11:14:50 2000
    Update to forwarding protocol to include an Acknowledge message.
    Unacknowledged files are resent.
  
  Revision 1.21  by: ericd  Rev date: Fri Nov 03 06:58:02 2000
    Fix bug w/ midnight rollover on daily log filtering...
  
  Revision 1.20  by: ericd  Rev date: Sat Oct 28 15:40:12 2000
    First changes for 1.4f client server architecture
  
  Revision 1.19  by: ericd  Rev date: Fri Aug 25 23:30:08 2000
    Fix default rep rate (not tested yet)
  
  Revision 1.18  by: ericd  Rev date: Fri Aug 25 23:20:10 2000
    Full repetitive and archiving implementation, Linux safe, some bugs:
    default rep rate is off
  
  Revision 1.17  by: ericd  Rev date: Wed Aug 23 20:59:22 2000
    State archiving almost full implementation
  
  Revision 1.16  by: ericd  Rev date: Tue Aug 22 21:59:32 2000
    1) Not using memlib any more 
    2) Repetitive operation fully implemented (lightly tested) 
    3) Rep operation optimized to not repetitively parse logs 
    4) Initial work on stats archiving
  
  Revision 1.15  by: ericd  Rev date: Mon Aug 21 22:23:00 2000
    Initial support for parsed file logging using avl tree index
  
  Revision 1.14  by: ericd  Rev date: Sat Aug 19 09:34:28 2000
    1) Fixed memory leaks for repetitive operation 
    2) Changes for log file deletion 
    3) Initial implementation of repetitive operation
  
  Revision 1.13  by: ericd  Rev date: Thu Jul 27 22:06:30 2000
    1) Workaround for CS dedicated server log splitting
  
  Revision 1.12  by: ericd  Rev date: Thu Jul 20 22:45:02 2000
    Meta data generation
  
  Revision 1.11  by: ericd  Rev date: Wed Jul 19 22:18:16 2000
    1) Modified to use btree lookup when locating player info
  
  Revision 1.10  by: ericd  Rev date: Mon Jul 03 22:47:12 2000
    Eliminated use of regex for sake of speed
  
  Revision 1.9  by: ericd  Rev date: Tue Jun 13 22:00:22 2000
    Rev. 1.4c (Linux port) candidate
  
  Revision 1.8  by: ericd  Rev date: Sun May 07 22:42:54 2000
    Fixed bug that was repeating match wins calculation
  
  Revision 1.7  by: ericd  Rev date: Tue Apr 25 22:28:56 2000
    Fixed bug that crashes PS if one map is parsed with no map name.
  
  Revision 1.6  by: ericd  Rev date: Tue Apr 18 23:18:36 2000
    Completed log filtering implementation
  
  Revision 1.5  by: ericd  Rev date: Mon Apr 17 23:03:16 2000
    Support for log file parse filtering by day, week, month.  Completed
    support for JS game filtering.
  
  Revision 1.4  by: ericd  Rev date: Sun Apr 16 22:46:56 2000
    Changed startup banner to include new web site
  
  Revision 1.3  by: ericd  Rev date: Sun Apr 16 22:28:52 2000
    Added "press any key" if help text is displayed (for use with shortcut
    execution).
  
  Revision 1.2  by: ericd  Rev date: Sun Apr 09 23:44:20 2000
    Initial support for map/game statistics
  
  Revision 1.1  by: ericd  Rev date: Sat Apr 01 18:43:48 2000
    Split pigstats.c into pigjs.c, pigparse.c, pigtext.c and pigutil.c
  
  Revision 1.0  by: ericd  Rev date: Sat Apr 01 17:00:38 2000
    Initial revision.
  
  $Endlog$
 */

#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <zlib.h>
#include <zconf.h>
//#include <include/asm-i386/termios.h>
#include <sys/ioctl.h>
#else
#include <io.h>
#include <windows.h>
#include <process.h>
#include "zlib.h"
#include "zconf.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "pigstats.h"
#include "pigparse.h"
#include "pigtext.h"
#include "pigjs.h"
#include "pigutil.h"



#define SECONDSINDAY (60L*60L*24L)
#define SECONDSINHOUR (60*60)

#define DEFAULTREPEATRATE (5)

const char* apSpanTypeNames[LASTPARSESPANTYPE] = {
	"today",
	"yesterday",
	"this week",
	"last week",
	"this month",
	"last month",
	"days"};

const char* apRepTypeNames[LASTREPTYPE] = {
	"",
	"1/4 hour+",
	"hour+",
	"day+",
	"week+",
	"month+"};


/* global struct for PLAYER stats */
struct PDB gPStat = {0};
/* global stat pointer for WEAPON stats */
struct PDB gWStat = {0};
/* global stat pointer for GAME stats */
struct PDB gGStat = {0};
/* global stat pointer for TEAM stats */
struct PDB gTStat = {0};

/* global path to jscript files */
char gcJPath[DEFAULT_STRING_LENGTH] = "";
/* global path to log files */
char cInPath[DEFAULT_STRING_LENGTH] = "";
/* global path to output files */
char gcOutPath[DEFAULT_STRING_LENGTH] = "";
/* global root path of output files */
char gcRootPath[DEFAULT_STRING_LENGTH] = "";
/* global root name of output files */
char gcRootName[DEFAULT_STRING_LENGTH] = "";
/* global html extension text */
char gcExtPath[10] = "";
/* global cross ref. path for weapon page */
char gcWeaponRef[DEFAULT_STRING_LENGTH] = "";
/* global cross ref. path for general page */
char gcGeneralRef[DEFAULT_STRING_LENGTH] = "";
/* global cross ref. path for game page */
char gcGameRef[DEFAULT_STRING_LENGTH] = "";
/* global path for archive file */
char gcArchivePath[DEFAULT_STRING_LENGTH] = "";
/* global output file type */
BOOL bIsHTML = FALSE;
/* global jscript switch */
BOOL bIsJScript = FALSE;
int	*gpiPStatList=NULL;	/* pointer to a list of indeces */
int	gPStatListSize = 0;
int	gPStatListQty;
struct PStat *gpWorkingPStat;
/* -s silent operation flag */
BOOL gbSilent = FALSE;
/* -f filter out zero statistics (no argument) or N players (N argument) */
BOOL bFilter = FALSE;
int iLimit = 0; /* limit to N players */
/* -u ignores unranked (i.e. filtered) players when crossreferencing */
BOOL bIgnoreUnranked = FALSE;
int iXLimit = PLAYER_XREF_SIZE; /* limit cross ref. lists to this many elements */
int iGLimit = 0; /* limit list of games in JavaScript datafile */
/* -m multiple html file output */
BOOL bMux = FALSE;
/* -l log file parse filtering flag globals */
BOOL gbLogFilter = FALSE;
PARSESPANTYPE eLogFilterType = TODAY;
REPTYPE eRepType = REP_NONE;
int iLogFilterDays = 0; /* 0 means TODAY */
/* -d delete files not parsed due to -l option */
BOOL gbDeleteFiles = FALSE; 
/* global time stamp data */
time_t StartTime,TimeStamp,RunTime;
/* global flag indicating team play */
BOOL bTeamPlay = TRUE;
/* global flag indicating match wins have been computed */
BOOL bMatchWinsComputed = FALSE;
/* global flag indicating suppression of text file output */
BOOL bSuppressText = FALSE;
/* global flag indicating repeat mode */
BOOL gbRepeat;
/* 5 minutes if no repeat type specified */
int giRepeatModifier = DEFAULTREPEATRATE;  
BOOL bFirstTimeThrough = TRUE; /* for suppressing startup banner */
/* global pointer to the working pstree base */
struct PStat *gTreeBase = NULL;
/* global pointer to void for finding tree nodes */
PVOID gTreeSearchValue = NULL;
/* global total bytes processed */
unsigned long gBytesProcessed = 0;
/* global files processed */
unsigned long gFilesProcessed = 0;
/* global process duration */
double gProcessDurationSecs = 0.0;
/* global process timedate */
char gcProcessTime[TIMEDATESTRLENGTH] = {0};
/* global pointer to the filename index tree */
avl_tree *gpFilenameTree = NULL;
/* global flag indicating at least one kill was detected during run */
BOOL gbKillDetected = FALSE;
/* archiving in use */
BOOL gbArchive = FALSE;  
/* log file forwarding in use */
BOOL gbLogForwarding = FALSE;
/* destination address string */
char *pszForwardingDestination;
/* destination ip address */
unsigned long gulForwardingDestination;
/* dest hostent */
struct hostent * pstHostEnt;
/* tcp/ip socket info for transmitting */
struct HostConn gstClient;
/* file server */
BOOL gbLogServer = FALSE;
/* client port number (i.e. sending TO port) */
unsigned short gwClientPort;
/* tcp/ip socket info for receiving ACK*/
struct HostConn gstClientAck;
/* array of server port flags */
int gaiServerPort[26];
/* exclude file used */
BOOL gbExcludeFileUsed = FALSE;
/* pointer to first exclude/include struct */
struct ExInData *gpstExInData = NULL;
/* booleans indicate which types of exin are defined */
BOOL gbXXDefined = FALSE;
BOOL gbXADefined = FALSE;
BOOL gbIADefined = FALSE;
BOOL gbIXDefined = FALSE;
/* boolean indicates if tfc logs were parsed */
BOOL gbIsTFC = FALSE;



/*
 * PIG_HelpWithArgs
 *
 * Help for the lost
 */
void PIG_HelpWithArgs(void)
{
	printf("PigStats [-i<input path>] [-o<output path>] [-s] [-m] [-f[N]] [-xN]\n");
	printf("         [-j<javascript output path>] [-gN] [-l[y|w|l|m|n|N]] [-d]\n");
	printf("         [-n] [-r[h|q|d|w|m][N]] [-a] [-v<port id>]\n");
	printf("         [-c<port id><server ip address or name>] [-u]\n\n");
	printf("Where <input path> is the path and file name (including wild cards) of \n");
	printf("the input log file(s) and <output path> is the path and file name of \n");
	printf("the output file. <javascript output path> is the path (only) to\n");
	printf("receive JavaScript include files.\n\n");
	printf("If the extension of the output file (provided with -o option) is .htm \n");
	printf("or .html then output is in HTML, otherwise it's straight text.\n\n");
	printf("If no input path is provided PigStats looks for log files in the current\n");
	printf("directory.  If no output path is provided, PigStats generates a text file\n");
	printf("called \"stats.txt\" in the current directory.\n\n");
	printf("'-s' silent operation; default displays a startup banner and parsed \n");
	printf("     file names to the console.\n\n");
	printf("'-m' multi-file html output.  If the output file is specified as html\n");
	printf("     then this flag results in generation of individual output files \n");
	printf("     for game, weapons and player statistics.  This is helpful when a \n");
	printf("     single html file would be too large to quickly load in a browser.\n\n");
	printf("'-f' filter zero scores.  Doesn't display players with zero kills in \n");
	printf("     the general statistics section.  If N is provided and -j option is\n");
	printf("     selected, limits JavaScript data lists to the top N players.\n");
	printf("'-x' customize cross reference list sizes (default is 5).\n");
	printf("'-g' limits game/map statistics to N games.  Default is unlimited.  This\n");
	printf("     option is only effective on JavaScript output.\n");
	printf("'-l' parse certain log files according to next character or number:\n");
	printf("     (no next char or num): only parse log files from today\n");
	printf("     y: only parse log files from yesterday\n");
	printf("     w: only parse log files from this week\n");
	printf("     l: only parse log files from last week\n");
	printf("     m: only parse log files from this month\n");
	printf("     n: only parse log files from last month\n");
	printf("     N (integer number): only parse log files from the last N days, \n");
	printf("       where 1 means today and yesterday, 2 means same as 1 plus the \n");
	printf("       day before, and so on...\n");
	printf("'-d' DELETES all log files that weren't parsed as a result of the -l \n");
	printf("     option.  USE WITH CAUTION.\n");
	printf("'-n' suppress default generation of .txt file output.\n");
	printf("'-r' parse after 5 minutes, and repeat automatically every 5 minutes.\n");
	printf("     Following modifiers affect repetition rate:\n");
	printf("     N: wait for N minutes, then repeat every N minutes.  If preceded\n");
	printf("        by one of the following modifiers, indicates an additional N \n");
	printf("        minute delay before parsing.\n");
	printf("     h: parse hourly (+ N minutes)\n");
	printf("     q: parse on the quarter hour (+ N minutes)\n");
	printf("     d: parse daily (midnight + N minutes)\n");
	printf("     w: parse weekly (midnight sat/sun + N minutes)\n");
	printf("     m: parse monthly (midnight AM of 1st, + N minutes)\n");
	printf("'-a' archive pigstats' internal state between runs.  This allows you \n");
	printf("     to efficiently repeat cumulative stats runs without parsing the \n");
	printf("     same logs over and over again.  The state is archived to a file \n");
	printf("     called archive.pig if you have selected the -j option and <html \n");
	printf("     or text file name>.archive.pig if only the -o option is used AND\n");
	printf("     a file name was provided.  If no output path/filename is given \n");
	printf("     or neither -j and -o are provided, then -a does nothing.  The \n");
	printf("     archive file is saved to the same directory as the specified out-\n");
	printf("     put file(s).  See readme for more information and examples.\n");
	printf("'-v' act as pigstats server.  Port id is a single character from a..z.\n");
	printf("     In this mode, pigstats may receive log files from other computers\n");
	printf("     running pigstats in 'client' mode, putting them in the log file \n");
	printf("     path indicated with the '-i' option.  This option is only effect-\n");
	printf("     ive in conjunction with the '-r' option.\n");
	printf("'-c' act as pigstats client.  Port id is a single character from a..z,\n");
	printf("     that much match the port id of the destination server, permitting\n");
	printf("     multiple servers running on the same computer.  Server ip address\n");
	printf("     or name is either the ip address (i.e. 192.168.0.1), or a valid\n");
	printf("     name for a machine running the pigstats server\n");
	printf("     (i.e. pig.pigindustries.com).  No spaces are allowed between\n");
	printf("     the option, the port id and the destination address.  The pigstats\n");
	printf("     client will forward all processed log files to the server machine\n");
	printf("     where they may be collected with logs from multiple machines and \n");
	printf("     re-processed as a whole.\n");
	printf("'-u' ignore unranked players when generating player cross-reference\n");
	printf("     lists.  This must be used with the '-f' option and is useful for\n");
	printf("     limiting the output file size and improving stats readability.\n");
	printf("     Any player that doesn't make the 'top N' list will not appear in\n");
	printf("     the player detail view cross-reference lists, but will appear in \n");
	printf("     weapon and game details.\n");
	printf("'-p' use the user defined file, 'pigrules.txt', in the output directory\n");
	printf("     to include/exclude players from the statistics.\n");
	printf("Press any key to continue...\n");
	getchar();
}


/* 
 * Pause until time for next scheduled run
 * assumes bRepeat is true..
 */
static int PIG_WaitForNextTime(void)
{
	int iTimeToSleep = SECONDSINDAY;
	int iLast = 0;
	struct tm tmNow, tmThen;
	time_t theTime, theTimeThen;

	if (eRepType == REP_NONE)
	{
		SLEEP(giRepeatModifier * 60 * 1000);  /* SLEEP expects mseconds */
	}
	else
	{
		/* repeat until time to sleep is less than SECONDSINDAY */
		while (!iLast)
		{
			/* get a tm structure for the current time */
			time(&theTime);
			tmNow = *localtime(&theTime);
			/* advance the tm struct appropriately depending on type */
			switch (eRepType) 
			{
			case REP_QUARTERLY :
				iLast = 1;
				tmNow.tm_sec = 0;
				tmNow.tm_min = ((tmNow.tm_min/15)+1)*15;
				break;
			case REP_HOURLY :
				iLast = 1;
				tmNow.tm_sec = 0;
				tmNow.tm_min = 0;
				tmNow.tm_hour++;
				break;
			case REP_DAILY :
				iLast = 1;
				tmNow.tm_sec = 0;
				tmNow.tm_min = 0;
				tmNow.tm_hour = 0;
				tmNow.tm_mday++;
				break;
			case REP_WEEKLY :
				tmNow.tm_sec = 0;
				tmNow.tm_min = 0;
				tmNow.tm_hour = 0;
				if (tmNow.tm_wday == 6)
				{
					iLast = 1;
				}
				tmNow.tm_mday += 7-tmNow.tm_wday;
				break;
			case REP_MONTHLY :
				tmNow.tm_sec = 0;
				tmNow.tm_min = 0;
				tmNow.tm_hour = 0;
				tmThen = tmNow;
				tmThen.tm_mday++;
				mktime(&tmThen);
				if (tmThen.tm_mday != (tmNow.tm_mday+1))
				{
					/* on the verge of a month rollover */
					iLast = 1;
				}
				tmNow.tm_mday = 0;
				tmNow.tm_mon++;
				break;
			}
			/* get wakeup time in time_t */
			theTimeThen = mktime(&tmNow);
			/* sleep lesser of delta and SECONDSINDAY */
			iTimeToSleep = theTimeThen - theTime;
			if (iTimeToSleep > SECONDSINDAY)
			{
				iTimeToSleep = SECONDSINDAY;
			}
			/* snoooore */
			SLEEP(iTimeToSleep*1000);  
		}
		/* end repeat */
		/* sleep for modifier minutes */
		SLEEP(giRepeatModifier * 60 * 1000);  /* SLEEP expects mseconds */
	}
	return 1;
}


/* release all internal resources */
static void PIG_ResetAllInternalData(FILE* hOutFile)
{
	struct ExInData *gpstExInX;
	/* release parse buffer */
	PIG_ReleaseResources();
	/* release list */
	if (gpiPStatList)
	{
		free(gpiPStatList);
	}
	gpiPStatList = NULL;
	/* close output file */
	if (hOutFile)
	{
		fclose(hOutFile);
	}
	/* clean up dynamically allocated memory */
	PIG_CleanPStat(&gPStat);
	PIG_CleanPStat(&gWStat);
	PIG_CleanPStat(&gGStat);
	PIG_CleanPStat(&gTStat);
	/* free the avl filename tree */
	if (gpFilenameTree)
	{
		avl_free(gpFilenameTree);
	}
	gpFilenameTree = NULL;

	/* init globals */
	gPStatListSize = 0;
	gTreeBase = NULL;
	/* global pointer to void for finding tree nodes */
	gTreeSearchValue = NULL;
	/* release exclude/include data */
	gpstExInX = gpstExInData;
	while (gpstExInX)
	{
		gpstExInData = gpstExInData->pstNext;
		free(gpstExInX);
		gpstExInX = gpstExInData;
	}
	return;
}

/*
 * InternalStateValid only really comes in to play if the program is 
 * running in a repetitive mode.. this function determines whether
 * the internal state (i.e. the current compiled data from all log files
 * run to-date) needs to be reset because the time filtering would 
 * dictate that data compiled in the past is no longer wanted. */

static int PIG_InternalStateValid(time_t tLastTime,time_t tProcessTime)
{
	int iRet = 0;
	struct tm tmLast = {0};
	struct tm tmNow = {0};
	if (tLastTime)
	{
		/* repeat operation with time filtering .. state might be valid */
		iRet = 1;
		if (gbLogFilter)
		{
			tmLast = *localtime(&tLastTime);
			tmNow = *localtime(&tProcessTime);
			/* 
			 * time filtering log files... state might not be valid.
			 * not going to worry about runs spanning years or handling
			 * leap years, fer chrissakes... I mean, this isn't the 
			 * flight control system for the frikkin space shuttle is it?
			 * ;)
			 */
			switch (eLogFilterType)
			{
			case THISWEEK :
			case LASTWEEK :
				/* invalid if week has passed */
				if ((tmLast.tm_wday > tmNow.tm_wday) || /* rolled over */
					 ((tmNow.tm_yday-tmLast.tm_yday) > 6) || /* more than 1 week */
					 ((tmNow.tm_yday+365-tmLast.tm_yday) > 6)) /* crossing Jan 1 */
				{
					iRet = 0;
				}
				break;
			case THISMONTH :
			case LASTMONTH :
				/* invalid if month has changed */
				if (tmLast.tm_mon != tmNow.tm_mon)
				{
					iRet = 0;
				}
				break;
			case YESTERDAY : 
			case TODAY :
			case NDAYS :
				/* invalid if day has changed */
				if (tmLast.tm_yday != tmNow.tm_yday)
				{
					iRet = 0;
				}
				break;
			}
		}
	}
	return iRet;
}


/* 
 * PIG_CheckAckSocket
 *
 */
void PIG_CheckAckSocket(void)
{
	struct sockaddr_in stFrom;
	int Fromlen;
	int recverr;
	static PROCESSEDNAME stBuffIn;
	PROCESSEDNAME* pPFName;

	/* sniff the ack socket and handle if received.. */
	do 
	{
		Fromlen = sizeof stFrom;
		recverr = recvfrom(	(int)gstClientAck.sock,
									(char*)&stBuffIn.cFName,
									sizeof stBuffIn.cFName,
									0,
									(struct sockaddr*) &stFrom,
									&Fromlen);
		/* check error return and packet version */
		if (recverr != SOCKET_ERROR)
		{
			/* locate the name record here... */
			pPFName = avl_find(gpFilenameTree,&stBuffIn);
			if (pPFName)
			{
				/* this will stop us from re-sending the file in future */
				pPFName->byWaitingACK = 0;
			}
		}

	} while (recverr != SOCKET_ERROR);

}



/*
 * PIG_FileServerThread
 *
 * Multiple instances of this thread are possible, depending on how many
 * times the -v option is included.  This is useful if multiple clients
 * on a single machine are sending to one server.  In this case, each
 * server thread is listening/responding on different ports, so there 
 * is no confusion as to where the acks will go.
 *
 * Client takes the source file, fragments it into 4K pieces, compresses
 * each of those and UDP's it to the server.  The server uncompresses
 * the fragments and reassembles them into a whole image of the original
 * file.  Once the checksum of the assembled file matches the checksum
 * of the original file, then the transmission is presumed to be 
 * complete.  If the server starts to receive a different file, interleaved
 * with the first, then it will ignore that file until the original is 
 * complete.  A file fragment that follows its peers by more than 30 seconds
 * will be presumed to be a new file transmission.
 *
 * I'm simply amazed that this stuff works at all.
 *
 */
#ifdef __linux__
static void* PIG_FileServerThread(void* pvArgs)
#else
static void PIG_FileServerThread(void* pvArgs)
#endif
{
	static struct PIGBuffer stBuffIn; /* raw, as received */
	static struct PIGBuffer stBuff; /* uncompressed */
	int index;
	struct sockaddr_in stFrom;
	int Fromlen;
	int recverr;
	static char cWritePath[DEFAULT_STRING_LENGTH];
	FILE* fWriteFile;
	char cIPStr[16];
	unsigned long SenderIP;
	uLongf destlen;
	int zlibret;
	int sent;
	/* pointer to file server buffer */
	char * pFileServerBuff;
	/* and its size */
	unsigned long ulFileServerBuffLen = 0;
	static char cWorkingName[PIGMAXNAME];
	unsigned short wWorkingPathChecksum = 0;
	unsigned long ulWorkingChecksum = 0;
	char *pX, *pXDest;
	int iX;
	int iDupe;
	char cX;
	time_t tLastFragTime,tFragTime;
	int iDiffFile;
	/* tcp/ip socket info for receiving */
	struct HostConn stServer;
	/* tcp/ip socket info for transmitting ACK*/
	struct HostConn stServerAck;
	/* Server port number (i.e. receiving FROM port) */
	unsigned short wServerPort;

	wServerPort = *((unsigned short*)pvArgs);
	cWorkingName[0] = 0;
	cWritePath[0] = 0;
	pFileServerBuff = malloc(0);
	tLastFragTime = (time_t)0;

	/* set up the ACK socket */
	stServerAck.sock = (SOCKET)socket(AF_INET, SOCK_DGRAM, 0);
	if (stServerAck.sock != INVALID_SOCKET)
	{
		memset(&stServerAck.addr, 0, sizeof gstClient.addr);
		gstClient.addr.sin_family = AF_INET;
		/* set up the socket */
		stServer.sock = (SOCKET) socket(AF_INET, SOCK_DGRAM, 0);
		if (stServer.sock != INVALID_SOCKET)
		{
			memset(&stServer.addr, 0, sizeof stServer.addr);
			stServer.addr.sin_port = htons(wServerPort);
			stServer.addr.sin_family = AF_INET;
			stServer.addr.sin_addr.s_addr = htonl(INADDR_ANY);
			for (index=0;index < 8;index++) stServer.addr.sin_zero[index] = 0;
			if (bind((int)stServer.sock,(struct sockaddr*) &stServer.addr, sizeof(struct sockaddr_in)) != SOCKET_ERROR)
			{
				do 
				{
					Fromlen = sizeof stFrom;
					recverr = recvfrom(	(int)stServer.sock,
												(char*)&stBuffIn,
												sizeof stBuffIn,
												0,
												(struct sockaddr*) &stFrom,
												&Fromlen);
					/* check error return and packet version */
					if ((recverr != SOCKET_ERROR) &&
						 !memcmp(stBuffIn.cProtoVersion,PIGPROTOVERSION,4))
					{
						/* note the receive time on this packet fragment */
						time(&tFragTime);
						/* expand the received packet */
						destlen = sizeof stBuff.cName + sizeof stBuff.cBuffStart;
						zlibret = uncompress((Bytef*)&stBuff.cName, &destlen,
												  (Bytef*) &stBuffIn.cName, stBuffIn.wSizeThis);
						if (zlibret == Z_OK)
						{
							stBuff.wSizeThis = (unsigned short)destlen - sizeof stBuff.cName;
							/* check to see if this belongs to a different file than what
							 * we had been working on up til now */
							iDiffFile = strcmp(cWorkingName,stBuff.cName) || 
									(stBuffIn.wPathChecksum != wWorkingPathChecksum);
							if ((iDiffFile && ((tFragTime - tLastFragTime) > 1)) || 
								 ((tFragTime - tLastFragTime) > 30))
							{
								/* give up on previous file and set up for handling a new file */
								ulWorkingChecksum = 0;
								wWorkingPathChecksum = stBuffIn.wPathChecksum;
								strcpy(cWorkingName,stBuff.cName);
								if (stBuffIn.dwSizeAll > ulFileServerBuffLen)
								{
									/* reallocate file buffer */
									free(pFileServerBuff);
									pFileServerBuff = malloc(stBuffIn.dwSizeAll);
									if (!pFileServerBuff) return;
									ulFileServerBuffLen = stBuffIn.dwSizeAll;
								}
								/* clear the file buffer as needed */
								memset(pFileServerBuff,0,stBuffIn.dwSizeAll);
								iDiffFile = 0;
							}
							if (!iDiffFile)
							{
								/* make a note of the time we got this fragment */
								tLastFragTime = tFragTime;
								/* buffer this portion of the file, but don't dupe */
								iDupe = 0;
								for (pX = pFileServerBuff+stBuffIn.dwOffsetThis, iX=0;
											iX < stBuff.wSizeThis;
											pX++,iX++)
								{
									if (*pX) 
									{
										iDupe = 1;
										break;
									}
								}
								if (!iDupe)
								{
									pXDest = pFileServerBuff+stBuffIn.dwOffsetThis;
									pX = stBuff.cBuffStart;
									for (iX = 0; iX < stBuff.wSizeThis;iX++,pX++,pXDest++)
									{
										/* copy and update checksum as we go */
										cX = *pX;
										ulWorkingChecksum += cX;
										(*pXDest) = cX;
									}
								}
								/* if checksums match, then expansion is complete (we hope!) */
								if (ulWorkingChecksum == stBuffIn.dwChecksum)
								{
									/* null last fragment time allows immediate new file */
									tLastFragTime = 0;
									/* write the file using unique file name */
									memset(cWritePath,0,sizeof cWritePath);
									SenderIP = htonl(stFrom.sin_addr.s_addr);
									sprintf(cIPStr,"L%04x%08x",stBuffIn.wPathChecksum,SenderIP);
									strcpy(cWritePath,cInPath);
									strcat(cWritePath,cIPStr);
									strcat(cWritePath,&stBuff.cName[1]);
									fWriteFile = fopen(cWritePath,"w");
									if (fWriteFile)
									{
										fwrite(pFileServerBuff, 1, stBuffIn.dwSizeAll, fWriteFile);
										fclose(fWriteFile);
									}
									/* send the acknowledgement */
									stServerAck.addr.sin_port = htons((unsigned short)(wServerPort+30));
									stFrom.sin_port = htons((unsigned short)(wServerPort+30));
									sent = sendto(	(int)stServerAck.sock,
												(char*)&stBuff.cName,
												strlen(stBuff.cName),
												0,
												(struct sockaddr*)&stFrom, 
												sizeof stFrom);
								}
							}
						}
					}
				} while (TRUE);
			}
		}
	}
}



/*
 * main
 *
 * 
 */
int main(int argc, char* argv[])
{
#ifdef __linux__
	DIR *pDir;
	struct dirent *pDirent;
#else
	long hFoundFile;
	struct _finddata_t stFileData;
	WORD wVersionRequested;
	int err;
	WSADATA wsaData;
#endif
	FILE *hOutFile = NULL;
	int iRet = 0;
	char cSearchPath[DEFAULT_STRING_LENGTH];
	char cTemp[DEFAULT_STRING_LENGTH];
	char cOutPath[DEFAULT_STRING_LENGTH];
	char cExInPath[DEFAULT_STRING_LENGTH];
	char *pFound;
	char *pNext;
	char *pRootName;
	char *pLast=NULL;
	int Index;
	int PathLen;
	char *pDirEntry;
	clock_t cCount;
	time_t tProcessTime = 0;
	time_t tLastTime = 0;
	time_t tThisTime = 0;
	struct PStat* pPlayer;
	BOOL bStateRecovered = FALSE;
	unsigned long index;

#ifdef TEST
	test();
#endif


	/* global total bytes processed */
	gBytesProcessed = 0;
	/* global files processed */
	gFilesProcessed = 0;

	for (Index=0;Index<26;Index++) gaiServerPort[Index]=0;

	#ifndef __linux__
	wVersionRequested = MAKEWORD(2,0);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (	(err != 0) || 
			(LOBYTE(wsaData.wVersion)!=2) ||
			(HIBYTE(wsaData.wVersion)!=0))
	{
		printf("No usable WinSock DLL!\n");
		return 1;
	}
	#endif


	do
	{
		/* init locals */
		iRet = 0;
		hOutFile = NULL;
		/* global process duration */
		gProcessDurationSecs = 0.0;
		/* force recomputation of match wins */
		bMatchWinsComputed = FALSE;
		gbKillDetected = FALSE;

		/* get to work */
		cSearchPath[0]=0;
		cInPath[0]=0;
		cOutPath[0]=0;
		cTemp[0]=0;

		/* default search path */
		strcpy(cSearchPath,"L*.log");

		for (Index=1;Index<argc;Index++)
		{
			strcpy(cTemp,argv[Index]);
			if (!memcmp(cTemp,"-i",2))
			{
				/* user specified input path */
				strcpy(cInPath,cTemp+2);
				strcpy(cSearchPath,cInPath);
				/* trim the path down to just a path */
				pFound = strstr(cInPath,"\\");
				if (!pFound)
				{
					/* try to catch LINUX style slash */
					pFound = strstr(cInPath,"/");
				}
				if (pFound)
				{
					while (pFound)
					{
						pLast = pFound;
						pNext = strstr(pFound+1,"\\");
						if (!pNext)
						{
							/* try to catch LINUX style slash */
							pNext = strstr(pFound+1,"/");
						}
						pFound = pNext;
					}
					*(pLast+1) = 0;
				}
				else
				{
					*cInPath = 0;
				}
			}
			else if (!memcmp(cTemp,"-o",2))
			{
				strcpy(cOutPath,cTemp+2);
			}
			else if (!memcmp(cTemp,"-s",2))
			{
				gbSilent = TRUE;
			}
			else if (!memcmp(cTemp,"-f",2))
			{
				/* -f filter out zero statistics */
				bFilter = TRUE;
				if (cTemp[2])
				{
					iLimit = atoi(&cTemp[2]);
				}
			}
			else if (!memcmp(cTemp,"-m",2))
			{
				/* -m multiple html file output */
				bMux = TRUE;
			}
			else if (!memcmp(cTemp,"-j",2))
			{
				bIsJScript = TRUE;
				/* user specified javascript path */
				strcpy(gcJPath,cTemp+2);
				PathLen = strlen(gcJPath);
				/* make sure it ends properly */
	#ifdef __linux__
				if (PathLen && gcJPath[PathLen-1] != '/')
				{
					strcat(gcJPath,"/");
				}
	#else
				if (PathLen && gcJPath[PathLen-1] != '\\')
				{
					strcat(gcJPath,"\\");
				}
	#endif
			}
			else if (!memcmp(cTemp,"-x",2))
			{
				if (cTemp[2])
				{
					iXLimit = atoi(&cTemp[2]);
				}
			}
			else if (!memcmp(cTemp,"-g",2))
			{
				if (cTemp[2])
				{
					iGLimit = atoi(&cTemp[2]);
				}
			}
			else if (!memcmp(cTemp,"-l",2))
			{
				gbLogFilter = TRUE;
				switch (cTemp[2])
				{
					case 'y' : eLogFilterType = YESTERDAY;break;
					case 'w' : eLogFilterType = THISWEEK;break;
					case 'l' : eLogFilterType = LASTWEEK;break;
					case 'm' : eLogFilterType = THISMONTH;break;
					case 'n' : eLogFilterType = LASTMONTH;break;
					default:	
						if (cTemp[2])
						{
							eLogFilterType = NDAYS;
							iLogFilterDays = atoi(&cTemp[2]);
						}
						else
						{
							eLogFilterType = TODAY;
						}
						break;
				}
			}
			else if (!memcmp(cTemp,"-d",2))
			{
				gbDeleteFiles = TRUE;
			}
			else if (!memcmp(cTemp,"-r",2))
			{
				gbRepeat = TRUE;
				eRepType = REP_NONE;
				if (cTemp[2])
				{
					switch (cTemp[2])
					{
						case 'q' : eRepType = REP_QUARTERLY;break;
						case 'h' : eRepType = REP_HOURLY;break;
						case 'd' : eRepType = REP_DAILY;break;
						case 'w' : eRepType = REP_WEEKLY;break;
						case 'm' : eRepType = REP_MONTHLY;break;
						default:	
							giRepeatModifier = atoi(&cTemp[2]);
							if (!giRepeatModifier)
							{
								/* let's not allow zero! */
								giRepeatModifier = DEFAULTREPEATRATE;
							}
							break;
					}
					if (eRepType != REP_NONE)
					{
						if (cTemp[3])
						{
							/* repeat of zero won't kill us here */
							giRepeatModifier = atoi(&cTemp[3]);
						}
					}
				}
				else
				{
					/* no modifiers, default is 5 */
					giRepeatModifier = DEFAULTREPEATRATE;
				}
			}
			else if (!memcmp(cTemp,"-n",2))
			{
				bSuppressText = TRUE;
			}
			else if (!memcmp(cTemp,"-a",2))
			{
				gbArchive = TRUE;
			}
			else if (!memcmp(cTemp,"-c",2))
			{
				if (strlen(cTemp) < 4)
				{
					PIG_HelpWithArgs();
					return 1;
				}
				cTemp[2] = toupper(cTemp[2]);
				if (cTemp[2]<'A' || cTemp[2]>'Z')
				{
					PIG_HelpWithArgs();
					return 1;
				}
				/* assign client port */
				gwClientPort = UDPPORT + cTemp[2] - 'A';
				/* determine destination IP address */
				pszForwardingDestination = &cTemp[3];
				if (*pszForwardingDestination)
				{
					gulForwardingDestination = inet_addr(pszForwardingDestination);
					if (gulForwardingDestination != INADDR_NONE)
					{
						gbLogForwarding = TRUE;
					}
					else
					{
						/* evaluate by name */
						pstHostEnt = (struct hostent *)gethostbyname(pszForwardingDestination);
						if (pstHostEnt)
						{
							gbLogForwarding = TRUE;
							memcpy(&gulForwardingDestination, (void*)pstHostEnt->h_addr, 4);
						}
						else
						{
							printf("Invalid log file forwarding destination address\n");
							PIG_HelpWithArgs();
							return 1;
						}
					}
				}
			}
			else if (!memcmp(cTemp,"-v",2))
			{
				/* file forwarding serVer */
				gbLogServer = TRUE;
				if (strlen(cTemp) < 3)
				{
					PIG_HelpWithArgs();
					return 1;
				}
				cTemp[2] = toupper(cTemp[2]);
				if (cTemp[2]<'A' || cTemp[2]>'Z')
				{
					PIG_HelpWithArgs();
					return 1;
				}
				/* assign server port */
				if (bFirstTimeThrough)
				{
					/* note that we will serve this port */
					gaiServerPort[cTemp[2]-'A'] = UDPPORT + cTemp[2] - 'A';
				}
			}
			else if (!memcmp(cTemp,"-u",2))
			{
				/* ignore unranked players in crossreference */
				bIgnoreUnranked = TRUE;
			}
			else if (!memcmp(cTemp,"-p",2))
			{
				/* use the exclude data file in the output directory */
				gbExcludeFileUsed = TRUE;
			}
			else 
			{
				PIG_HelpWithArgs();
				return 1;
			}

		}

		/* suppress log file forwarding, if necessary */
		/* 
		 * forwarding only works if in archiving mode, 
		 * and log filtering is not N days where N>0.
		 * this *should* prevent repeat transmissions of
		 * duplicate files.
		 */

		if (gbLogForwarding &&
				(!gbArchive ||
				((eLogFilterType == NDAYS) && iLogFilterDays)))
		{
			gbLogForwarding = FALSE;
		}

		/* suppress archiving, if necessary */
		if (gbArchive)
		{
			if (!cOutPath[0] && !gcJPath[0])
			{
				gbArchive = FALSE;
			}
			else
			{
				if (gcJPath[0])
				{
					/* use the javascript path first */
					strcpy(gcArchivePath,gcJPath);
				}
			}
		}

		if ((!cOutPath[0]) && !bSuppressText)
		{
			/* no output file specified, make one */
			strcpy(cOutPath,"stats.txt");
		}
		if (cOutPath[0])
		{
			/* is output file HTML? */
			PathLen = strlen(cOutPath);
			pFound = strstr(cOutPath,".html");
			if (pFound)
			{
				PathLen--;
			}
			else
			{
				pFound = strstr(cOutPath,".htm");
			}
			if (pFound == (cOutPath+PathLen-4))
			{
				bIsHTML = TRUE;
				strcpy(gcRootPath,cOutPath);
				/* gcRootPath is the full path, minus the '.htm(l)' */
				gcRootPath[(int)pFound - (int)cOutPath]=0;
				/* check to see if we need to use this as an archive path */
				if (gbArchive && !gcArchivePath[0])
				{
					strcpy(gcArchivePath,gcRootPath);
					strcat(gcArchivePath,".");
				}
				/* gcExtPath is simply the '.htm(l)' portion */
				strcpy(gcExtPath,pFound);
				/* locate the root name */
				pRootName = gcRootPath + strlen(gcRootPath) - 1;
				while ((pRootName != gcRootPath) && strcspn(pRootName,INVALID_CHARS))
				{
					pRootName--;
				}
				if (pRootName != gcRootPath)
				{
					/* back up one place */
					pRootName++;
				}
				/* copy the root name */
				strcpy(gcRootName,pRootName);
				/* create the cross reference paths */
				strcpy(gcWeaponRef,"./"); 
				strcat(gcWeaponRef,gcRootName);
				strcat(gcWeaponRef,WEAPONEXTENSION);
				strcat(gcWeaponRef,gcExtPath);
				strcpy(gcGeneralRef,"./");
				strcat(gcGeneralRef,gcRootName);
				strcat(gcGeneralRef,gcExtPath);
				strcpy(gcGameRef,"./");
				strcat(gcGameRef,gcRootName);
				strcat(gcGameRef,GAMEEXTENSION);
				strcat(gcGameRef,gcExtPath);
			}
			else
			{
				/* if not HTML, don't split the files */
				bMux = FALSE;
			}
		} /* output path supplied */

		/* fully construct the archive path */
		if (gbArchive)
		{
			strcat(gcArchivePath,"archive.pig");
			/* recover previous internal state, if nec. */
			if (!bStateRecovered)
			{
				PIG_RecoverInternalState(&tThisTime);
				/* don't ever recover again! */
				bStateRecovered = TRUE;
			}
		}

		/* Do things that only happen once, first time through! */
		if (bFirstTimeThrough)
		{
			/* print the startup banner */
			if (!gbSilent)
			{
				printf("PigStats %s Copyright (C) 2001 Eric Dennison\n",PIGVERSTRING);
				printf("PigStats comes with ABSOLUTELY NO WARRANTY.  This\n");
				printf("is free software, and you are welcome to redistribute\n");
				printf("it under the terms of the GNU Public License.\n");
				printf("See www.gnu.org/copyleft/gpl.html for more details.\n");
				printf("Source code is available from www.planethalflife.com/pigstats\n");
			}

			/* launch the file server */
			if (gbLogServer)
			{
				/* launch the server thread(s)! */
				for (Index=0;Index<26;Index++)
				{
					if (gaiServerPort[Index])
					{
	#ifdef __linux__
						pthread_t *pthread = malloc(sizeof(pthread_t));
						pthread_create(pthread, NULL, PIG_FileServerThread, &gaiServerPort[Index]);
	#else
						_beginthread( PIG_FileServerThread, 0, &gaiServerPort[Index]);
	#endif
					}
				}
			}

			
			if (gbLogForwarding)
			{
				/* set up the sockets */
				gstClient.sock = (SOCKET)socket(AF_INET, SOCK_DGRAM, 0);
				gstClientAck.sock = (SOCKET)socket(AF_INET, SOCK_DGRAM, 0);
				if (gstClient.sock != INVALID_SOCKET &&
					 gstClientAck.sock != INVALID_SOCKET)
				{
					unsigned short wTemp = 30;
					memset(&gstClient.addr, 0, sizeof gstClient.addr);
					gstClient.addr.sin_port = htons(gwClientPort);
					gstClient.addr.sin_family = AF_INET;
					gstClient.addr.sin_addr.s_addr = gulForwardingDestination;
					memset(&gstClientAck.addr, 0, sizeof gstClientAck.addr);
					gstClientAck.addr.sin_port = htons((unsigned short)(gwClientPort+wTemp));  /* offset port address */
					gstClientAck.addr.sin_family = AF_INET;
					gstClientAck.addr.sin_addr.s_addr = htonl(INADDR_ANY);
					for (index=0;index < 8;index++) gstClientAck.addr.sin_zero[index] = 0;
					if (bind((int)gstClientAck.sock,(struct sockaddr*) &gstClientAck.addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
					{
						/* undo it, something's wrong */
						gbLogForwarding = FALSE;
					}
						/* make socket non blocking */
					index = 1;
#ifdef __linux__
					if (ioctl((int)gstClientAck.sock,FIONBIO,(char *) &index) == SOCKET_ERROR)
#else
					if (ioctlsocket(gstClientAck.sock,FIONBIO, &index) == SOCKET_ERROR)
#endif
					{
						/* undo it, something's wrong */
						gbLogForwarding = FALSE;
					}

				}
				else
				{
					/* undo it, something's wrong */
					gbLogForwarding = FALSE;
				}
			}

			/* set the flag false.. this code never runs again */
			bFirstTimeThrough = FALSE;
		}

		/* pause for configured time, if necessary */
		if (gbRepeat)
		{
			PIG_WaitForNextTime();
		}

		/* 
		 * initialize some of the globals, and release existing resources 
		 * if the internal state is no longer valid (due to date rollover
		 * and time filtering, for example.
		 */
		tLastTime = tThisTime;
		tThisTime = time(NULL);
		if (!PIG_InternalStateValid(tLastTime,tThisTime))
		{
			PIG_ResetAllInternalData(cOutPath[0]?hOutFile:NULL);
		}

		/* grab the exclude/include data, if necessary */
		if (gbExcludeFileUsed && (cOutPath[0] || gcJPath[0]))
		{
			if (gcJPath[0])
			{
				strcpy(cExInPath, gcJPath);
			}
			else
			{
				strcpy(cExInPath, cOutPath);
			}
			strcat(cExInPath, "pigrules.txt");
			PIG_GetExcludeIncludeData(cExInPath);
		}

		/* create the filename index tree, if it isn't already there */
		if (!gpFilenameTree)
		{
			gpFilenameTree = avl_create(&PIG_AVLFilenameCompare,NULL);
		}
		
		/* initialize the run time */
		RunTime = time(NULL);

		/* allocate a null player list */
		if (!gpiPStatList)
		{
			gpiPStatList = (int*)malloc(1);
			if (!gpiPStatList)
			{
				printf("Allocation failure\n");
				return 1;
			}
		}

		/* start logging processing time */
		tProcessTime = time(NULL);
		cCount = clock();
		/* note the starting time */
		strcpy(gcProcessTime,asctime(localtime(&tProcessTime)));
		if (!gbSilent)
		{
			printf("Working...            \r");
		}

	#ifdef __linux__
		pDir = opendir(cInPath);
		if (!pDir)
		{
			iRet = 1;
		}
		if (!iRet)
		{
			pDirent = readdir(pDir);
			if (!pDirent)
			{
				iRet = 1;
			}
			else
			{
				pDirEntry = pDirent->d_name;
			}
		}
	#else /* Win32 compile */
		hFoundFile = _findfirst(cSearchPath, &stFileData);
		pDirEntry = stFileData.name;
		if (hFoundFile == -1)
		{
			iRet = 1;
		}
	#endif
		if (!iRet) 
		{
			if (PIG_ParseFile(pDirEntry))
			{
				return 1;
			}
			while (!iRet)
			{
	#ifdef __linux__
				pDirent = readdir(pDir);
				if (!pDirent)
				{
					iRet = 1;
				}
				else
				{
					pDirEntry = pDirent->d_name;
				}
	#else /* Win32 compile */
				iRet = _findnext(hFoundFile, &stFileData);
				pDirEntry = stFileData.name;
	#endif
				if (!iRet) 
				{
					if (PIG_ParseFile(pDirEntry))
					{
						return 1;
					}
				}
			}
	#ifdef __linux__
			if (gbLogForwarding)
			{
				SLEEP(1000);
				PIG_CheckAckSocket();
			}
			closedir(pDir);
	#else
			_findclose(hFoundFile);
	#endif

			/* get total time */
			cCount = clock()-cCount;
			gProcessDurationSecs = cCount/(1.0*CLOCKS_PER_SEC);
			/* display something */
			if (gbKillDetected && gPStat.pPStat)
			{
				/* reset the match wins count for each player */
				pPlayer = gPStat.pPStat;
				while(pPlayer->pPName)
				{
					pPlayer->nmw = 0;
					pPlayer++;
				}
				/* output path supplied? */
				if (cOutPath[0])
				{
					/* open the output file */
					hOutFile = fopen(cOutPath,"w");
					if (!hOutFile)
					{
						printf("sorry... someone doesn't seem to like your choice for output file name\n\n");
						PIG_HelpWithArgs();
						return 1;
					}
					/* allocate a global temporary list */
					PIG_StartHTML(hOutFile,"PigStats Half-Life Statistics");
					PIG_ShowOverallStats(hOutFile);
					PIG_ShowWeaponStats(hOutFile);
					PIG_ShowPlayerStats(hOutFile);
					if (bTeamPlay)
					{
						/* try to figure out what the teams are */
						PIG_FindTeams();
						if (bTeamPlay)
						{
							PIG_ShowTeamGameStats(hOutFile);
						}
					}
					else
					{
						PIG_ShowGameStats(hOutFile);
					}
					PIG_EndHTML(hOutFile);
				} /* if output path supplied */
				/* generate jscript data, if required */
				if (bIsJScript)
				{
					/* note the order of following calls is critical! */
					if (	!(PIG_JSGameStats() && 
							PIG_JSOverallStats())
						)
					{
						return 1;
					}
				}
				/* archive the internal state */
				if (gbArchive)
				{
					PIG_ArchiveInternalState(tThisTime);
				}
			}
		}
/*  This is dropping us out when the log directory is empty
		else
		{
			printf("sorry... someone doesn't seem to like your choice for input file name\n\n");
			PIG_HelpWithArgs();
			return 1;
		}
*/
	if (!gbSilent)
	{
		printf("                               \r");
	}

	} while (gbRepeat);

	/* release all resources.. our job is done here */
	PIG_ResetAllInternalData(cOutPath[0]?hOutFile:NULL);

	return 0;
}
