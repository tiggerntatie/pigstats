/****************************************************************************

$Header: pigparse.c  Revision:1.30  Tue Nov 30 11:20:32 2004  ericd $
  
PigParse.c

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
$Log: C:\Documents and Settings\All Users\Documents\WORK\PigStats\Archive\SourceDist\CSource\pigparse.d $
  
    Split pigstats.c into pigjs.c, pigparse.c, pigtext.c and pigutil.c
  
  Revision 1.30  by: ericd  Rev date: Tue Nov 30 11:20:32 2004
    CS:Source support for log file end text
  
  Revision 1.29  by: ericd  Rev date: Sat Mar 01 15:44:34 2003
    Support for weapons with spaces in their names: "akimbo m31"
  
  Revision 1.28  by: ericd  Rev date: Sat Sep 29 22:52:50 2001
    Fixed bug that occurs when someone chats a phrase ending in ..hand"
  
  Revision 1.27  by: ericd  Rev date: Sat Aug 11 10:56:52 2001
    
  
  Revision 1.26  by: ericd  Rev date: Wed Aug 01 21:49:46 2001
    Initial release for custom TFC support for LP Boss (1.7a)
  
  Revision 1.25  by: ericd  Rev date: Sun Mar 25 22:29:34 2001
    rule.pig name exclusion/inclusion.. debugging related to new logging
    standard.
  
  Revision 1.24  by: ericd  Rev date: Thu Mar 22 22:16:26 2001
    Multiple server threads, support for arbitrarily large file transfers,
    bad html character substitution, beginning rule.pig name exclusion
    code.
  
  Revision 1.23  by: ericd  Rev date: Tue Mar 20 17:23:38 2001
    Support for new suicide and killed by world syntax
  
  Revision 1.22  by: ericd  Rev date: Thu Mar 15 19:45:14 2001
    1.5a  support for new logging standard
  
  Revision 1.21  by: ericd  Rev date: Tue Jan 30 19:57:58 2001
    Changed all pigfile references to lower case
  
  Revision 1.20  by: ericd  Rev date: Sat Nov 11 11:17:24 2000
    Modified to always check for ack 200 msec after forwarding a file.
  
  Revision 1.19  by: ericd  Rev date: Sat Nov 11 11:14:50 2000
    Update to forwarding protocol to include an Acknowledge message.
    Unacknowledged files are resent.
  
  Revision 1.18  by: ericd  Rev date: Sat Oct 28 15:40:12 2000
    First changes for 1.4f client server architecture
  
  Revision 1.17  by: ericd  Rev date: Tue Sep 12 22:27:26 2000
    Change logic for qualifying new files for parsing
  
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
  
  Revision 1.13  by: ericd  Rev date: Thu Aug 03 21:50:52 2000
    Fix file loading bug (again!) and parsing errors in "has entered"
    events on Internet servers
  
  Revision 1.12  by: ericd  Rev date: Thu Jul 27 22:06:30 2000
    1) Workaround for CS dedicated server log splitting
  
  Revision 1.11  by: ericd  Rev date: Mon Jul 24 22:33:04 2000
    More parsing bugs fixed.. (regex was more robust, but slower!)
  
  Revision 1.10  by: ericd  Rev date: Sat Jul 22 20:34:12 2000
    Fixed bug when no "player has entered" events occur.
  
  Revision 1.9  by: ericd  Rev date: Thu Jul 20 22:45:02 2000
    Meta data generation
  
  Revision 1.8  by: ericd  Rev date: Wed Jul 19 22:18:16 2000
    1) Modified to use btree lookup when locating player info
  
  Revision 1.7  by: ericd  Rev date: Mon Jul 03 22:47:12 2000
    Eliminated use of regex for sake of speed
  
  Revision 1.6  by: ericd  Rev date: Tue Jun 13 22:00:22 2000
    Rev. 1.4c (Linux port) candidate
  
  Revision 1.5  by: ericd  Rev date: Sun Jun 11 14:45:18 2000
    Fixed handle leak in PigParse, Counter-Strike 6.5 log change and upped
    rev to 1.4b or 1.4.3
  
  Revision 1.4  by: ericd  Rev date: Tue Apr 25 22:29:28 2000
    Fixed bug that truncated weapons with space in the name, and ignored
    tfc weapons.
  
  Revision 1.3  by: ericd  Rev date: Fri Apr 21 18:21:58 2000
    Fixed the inevitable DST bugs
  
  Revision 1.2  by: ericd  Rev date: Tue Apr 18 23:18:36 2000
    Completed log filtering implementation
  
  Revision 1.1  by: ericd  Rev date: Sat Apr 01 18:57:00 2000
    Removed junk in expanded keywords
  
  Revision 1.0  by: ericd  Rev date: Sat Apr 01 18:44:16 2000
    Initial revision.
  
  $Endlog$
 */

#ifndef __linux__
#include <windows.h>
#include "zlib.h"
#include "zconf.h"
#else
#include <zlib.h>
#include <zconf.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include "pigstats.h"
#include "pigparse.h"
#include "pigutil.h"


/* Globals for PigParse */
DWORD gdwBufferSize = 0;
char *gpBuffer = NULL;
char *gpBufferTop = NULL;

BOOL bNewLogFormat = TRUE;

#define OLD_LOG_PERIOD_OFFSET (41)

static char *phasentered = ">\" has entered the game";
static char *pnewhasentered = ">\" entered the game";
static char *pdisconnected = ">\" disconnected";


#define TIME_STAMP_LENGTH (25)
#define TIME_STAMP_SHORTLENGTH (21)
#define TIME_STAMP_OFFSET (2)

typedef enum 
{
	PARSE_KILLS,
	PARSE_SUICIDES,
	PARSE_DROPOFFS,
	PARSE_DONE
} PARSE_MODE;



#define PARSE_FIRST (PARSE_KILLS)


void PIG_ReleaseResources(void)
{
	if (gpBuffer)
	{
		free(gpBuffer);
		gpBuffer = NULL;
		gdwBufferSize = 0;
		gpBufferTop = NULL;
	}
}


void PIG_UpdatePSTimeStamp(struct PStat* pStat)
{
	pStat->used = TRUE;
	if (!pStat->gameduration)
	{
		/* no connect info was gathered previously */
		pStat->gameduration -= TimeStamp-StartTime;
	}
#if 0
	if (pStat->start)
	{
		/* update duration and end stamp */
		if (TimeStamp > pStat->end)
		{
			pStat->duration += TimeStamp-pStat->end;
			pStat->end = TimeStamp;
		}
	}
	else
	{
		/* initialize start and end */
		pStat->start = StartTime;
		pStat->end = StartTime;
	}
#endif
}


void PIG_ClearPSTimeStamps(void)
{
	struct PStat* pStat = gPStat.pPStat;

	while (pStat && pStat->pPName)
	{
		if (pStat->used)
		{
			if (pStat->gameduration < 0)
			{
				/* more connects than disconnects.. use end stamp */
				pStat->gameduration += TimeStamp-StartTime;
			}
			pStat->duration += pStat->gameduration;
			pStat->gameduration = 0;
			pStat->used = FALSE;
		}
		pStat++;
	}
}


int PIG_InvalidPlayer(EXINTYPES nn,char* pName,char* pVict)
{
	struct ExInData *pstEI;
	int iDone = 0;
	int iRet = 0;

	pstEI = gpstExInData;
	while(pstEI && !iDone)
	{
		switch (nn)
		{
		case XX : 
			if ((pstEI->eType == XX) &&
				 ((pName && !strcmp(pName,pstEI->caName)) ||
				 (pVict && !strcmp(pVict,pstEI->caName))))
			{
				iRet = 1;
				iDone = 1;
			}
			break;
		case XA :
			if ((pstEI->eType == XA) &&
				 ((pName && strstr(pName,pstEI->caName)) ||
				 (pVict && strstr(pVict,pstEI->caName))))
			{
				iRet = 1;
				iDone = 1;
			}
			break;
		case IA :
			if (pstEI->eType == IA)
			{
				iRet = 1;
				if (pName && strstr(pName,pstEI->caName) && !pVict)
				{
					iRet = 0;
					iDone = 1;
				}
				else if (pVict && strstr(pVict,pstEI->caName) && !pName)
				{
					iRet = 0;
					iDone = 1;
				}
				else if (pName && pVict && strstr(pName,pstEI->caName) && 
							!PIG_InvalidPlayer(nn,pVict,NULL))
				{
					iRet = 0;
					iDone = 1;
				}
			}
			break;
		case IX :
			if (pstEI->eType == IX)
			{
				iRet = 1;
				if (pName && !strcmp(pName,pstEI->caName) && !pVict)
				{
					iRet = 0;
					iDone = 1;
				}
				else if (pVict && !strcmp(pVict,pstEI->caName) && !pName)
				{
					iRet = 0;
					iDone = 1;
				}
				else if (pName && pVict && !strcmp(pName,pstEI->caName) && 
							!PIG_InvalidPlayer(nn,pVict,NULL))
				{
					iRet = 0;
					iDone = 1;
				}
			}
			break;
		}
		pstEI = pstEI->pstNext;
	}
	return iRet;
}



int PIG_UpdatePStat(time_t tstamp, char* pName, char* pVict, char* pWp, struct PStat *pGameStat, PARSESTATE eParseState)
{
	struct PStat* pKrStat;
	struct PStat* pKeStat;
	struct PStat* pKrKeStat;
	struct PStat* pKeKrStat;
	int IndexR, IndexE, Index, IndexW;
	struct PStat* pWpStat;
	struct PStat* pWpKrStat;
	struct PStat* pWpKeStat;
	struct PStat* pWFavStat;
	struct PStat* pGmKrStat;	/* game killer stat */
	struct PStat* pGmKeStat;	/* game killee stat */
	char *pCS65TS = "[TERRORIST] ";
	int iCS65TSL = 12; /* string length */
	char *pCS65CTS = "[CT] ";
	int iCS65CTSL = 5; /* string length */
	char **ppDeadGuy;
	BOOL bFix = TRUE;


	/* HACK fix Counter-Strike 6.5 log format change (errr....): */
	switch (eParseState)
	{
	case PARSEKILLS:
		ppDeadGuy = &pVict;
		break;
	case PARSESUICIDES:
		ppDeadGuy = &pName;
		break;
	default:
		bFix = FALSE;
		break;
	}
	if (bFix)
	{
		/* quick check */
		if (**ppDeadGuy == '[')
		{
			/* possible CS 6.5 victim: check further */
			if (!memcmp(*ppDeadGuy,pCS65CTS,iCS65CTSL))
			{
				/* CS 6.5 Counter Terrorist victim: adjust the pointer */
				*ppDeadGuy+=iCS65CTSL;
			}
			else if (!memcmp(*ppDeadGuy,pCS65TS,iCS65TSL))
			{
				/* CS 6.5 Terrorist victim: adjust the pointer*/
				*ppDeadGuy+=iCS65TSL;
			}
		}
	}
	/* End HACK fix */
	/* validate player names */
	if (gbXXDefined && PIG_InvalidPlayer(XX,pName,pVict)) return 0;
	if (gbXADefined && PIG_InvalidPlayer(XA,pName,pVict)) return 0;
	if (gbIADefined && PIG_InvalidPlayer(IA,pName,pVict)) return 0;
	if (gbIXDefined && PIG_InvalidPlayer(IX,pName,pVict)) return 0;
	pKrStat = PIG_GetPStat(&gPStat, pName, &IndexR);
	pGmKrStat = PIG_GetPStat(&pGameStat->OPStat, pName, &Index);
	/* update killer time stamp data */
	PIG_UpdatePSTimeStamp(pKrStat);
	pWpStat = PIG_GetPStat(&gWStat, pWp, &IndexW);
	switch (eParseState)
	{
	case PARSEKILLS:
		/* killer's overall kill count */
		pKrStat->nk++;
		/* and game count */
		pGmKrStat->nk++;
		/* increment killer's weapon kill count */
		pWpKrStat = PIG_GetPStat(&(pKrStat->WStat),pWp, &Index);
		pWpKrStat->nk++;
		/* times killer killed this victim */
		pKrKeStat = PIG_GetPStat(&(pKrStat->OPStat), pVict, &Index);
		pKrKeStat->nk++; /* times killed this victim */
		/* update killee's stats */
		pKeStat = PIG_GetPStat(&gPStat, pVict, &IndexE);
		pGmKeStat = PIG_GetPStat(&pGameStat->OPStat, pVict, &Index);
		pKeStat->nd++;
		pGmKeStat->nd++;
		/* update killee's time stamp data */
		PIG_UpdatePSTimeStamp(pKeStat);
		/* increment killee's weapon death count */
		pWpKeStat = PIG_GetPStat(&(pKeStat->WStat), pWp, &Index);
		pWpKeStat->nd++;
		/* and killee's stats for killer */
		pKeKrStat = PIG_GetPStat(&(pKeStat->OPStat), pName, &Index);
		pKeKrStat->nd++; /* times killed by this killer */
		/* update weapon info: times weapon was used to kill another player only */
		pWpStat->nk++;
		/* update weapon favorite usage info */
		pWFavStat = PIG_GetPStat(&(pWpStat->OPStat), pName, &Index);
		pWFavStat->nk++;
		break;
	case PARSESUICIDES:
		pKrStat->ns++;
		pGmKrStat->ns++;
	case PARSEDEATHS:
		pKrStat->nd++;
		pGmKrStat->nd++;
		/* times killee has died by this weapon */
		pWpKeStat = PIG_GetPStat(&(pWpStat->OPStat), pName, &Index);
		pWpKeStat->nd++;
		break;
	case PARSEDROPOFFS:
		pKrStat->ncaps++;
		pGmKrStat->ncaps++;
		break;
	default:
		break;
	}
	return 0;
}


/* 
 * PIG_TimeStampIsAccepted
 * 
 * return non zero if StartTime indicates this log matches the desired
 * log filter criteria.
 */
int PIG_TimeStampIsAccepted(int* pbTooOld)
{
	/* compare StartTime against RunTime */
	static int aDaysInMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	struct tm LocalStartTime;
	struct tm LocalRunTime;
	struct tm stWorking;
	time_t RunTimeDayStart;
	time_t RunTimeWeekStart;
	time_t RunTimeMonthStart;
	int iRet = 1;

	/* initialize return value */
	*pbTooOld = FALSE;

	memcpy(&LocalStartTime, localtime(&StartTime), sizeof (struct tm));
	memcpy(&LocalRunTime, localtime(&RunTime), sizeof (struct tm));
	stWorking = LocalRunTime;
	stWorking.tm_sec = 0;
	stWorking.tm_min = 0;
	stWorking.tm_hour = 0;
	RunTimeDayStart = mktime(&stWorking);
	RunTimeWeekStart = RunTimeDayStart - stWorking.tm_wday*60*60*24;
	RunTimeMonthStart = RunTimeDayStart - (stWorking.tm_mday-1)*60*60*24;

	switch (eLogFilterType)
	{
	case TODAY :
		iRet = StartTime >= RunTimeDayStart;
		if (!iRet) *pbTooOld = TRUE;
		break;
	case YESTERDAY :
		if (StartTime < RunTimeDayStart-60*60*24) *pbTooOld = TRUE;
		iRet = (StartTime < RunTimeDayStart) && (! *pbTooOld);
		break;
	case THISWEEK :
		iRet = StartTime >= RunTimeWeekStart;
		if (!iRet) *pbTooOld = TRUE;
		break;
	case LASTWEEK :
		if (StartTime < RunTimeWeekStart -(60*60*24*7)) *pbTooOld = TRUE;
		iRet = (StartTime < RunTimeWeekStart) && (! *pbTooOld);
		break;
	case THISMONTH :
		iRet = StartTime >= RunTimeMonthStart;
		if (!iRet) *pbTooOld = TRUE;
		break;
	case LASTMONTH :
		/* Ok, this won't quite work in March of a leap year, but who gives a... */
		if (StartTime < RunTimeMonthStart-60*60*24*
						aDaysInMonth[(stWorking.tm_mon+11)%12]) *pbTooOld = TRUE;
		iRet = (StartTime < RunTimeMonthStart) && (! *pbTooOld);
		break;
	case NDAYS :
		iRet = StartTime >= RunTimeDayStart - iLogFilterDays*60*60*24;
		if (!iRet) *pbTooOld = TRUE;
	default: 
		break;
	}
	return iRet;
}


time_t PIG_GetTimeStamp(char *pText, char **ppStampEnd)
{
	struct tm stT; /* formatted time */
	time_t tRet;

	stT.tm_mon = atoi(pText+2)-1;
	stT.tm_mday = atoi(pText+5);
	stT.tm_year = atoi(pText+8)-1900;
	stT.tm_hour = atoi(pText+15);
	stT.tm_min = atoi(pText+18);
	stT.tm_sec = atoi(pText+21);
	stT.tm_isdst = -1;
	tRet = mktime(&stT);
	if (ppStampEnd)
	{
		*ppStampEnd = pText+TIME_STAMP_LENGTH;
	}
	return tRet;
}


/* return pointer to start of line */
static char* PIG_prevnewline(char* pBuff)
{
	int iChar;
	do
	{
		iChar = *(--pBuff);
	} while ((iChar != '\n') && (iChar != '\r') && iChar && (pBuff > gpBuffer));

	return pBuff+1;
}

/* parse player name block, returning next character */
static void PIG_GetPlayerName(char *source,char* dest,char** ppnext)
{
	char* pSearch;
	char* pEnd;
	char* pInnerEnd;
	BOOL bError = FALSE;
	char  saved;
	int	iBracketSetCount = 0;
	BOOL	bLoopDone = FALSE;
	

	/* name starts after first quote char */
	pSearch = strchr(source, '\"');
	if (pSearch)
	{
		pSearch++;
		/* and ends before  <#>  */
		pEnd = strchr(pSearch,'\"');
		if (pEnd)
		{
			/* and last occurence of < */
			pInnerEnd = pEnd;
			while ((pInnerEnd != pSearch) && !bLoopDone)
			{
				if (('<' == *pInnerEnd) && memcmp(pInnerEnd,"<WON:",5))
				{
					iBracketSetCount++;
					if (bNewLogFormat)
					{
						/* if a new log, count no more than three bracket sets */
						if ((iBracketSetCount == 3) || (*(pInnerEnd-1) != '>'))
						{
							bLoopDone = TRUE;
						}
					}
					else
					{
						/* old log files must have only one bracket set */
						if (iBracketSetCount)
						{
							bLoopDone = TRUE;
						}
					}
				}
				pInnerEnd--;
			}
			pInnerEnd++;
			saved = *pInnerEnd;
			*pInnerEnd = 0;
			/* look for and replace Html unfriendly characters */
			PIG_ConvertNameCopy(dest, pSearch);
			*pInnerEnd = saved;
			if (ppnext)
			{
				/* end of player name garbage follows next quote char */
				*ppnext = pEnd;
				(*ppnext)++;
			}
		}
		else
		{
			bError = TRUE;
		}
	}
	else
	{
		bError = TRUE;
	}
	if (bError)
	{
		strcpy(dest,"error");
	}
}


/* parse weapon name block, returning next character */
static void PIG_GetWeaponName(char *source,char* dest,char** ppnext)
{
	char* pSearch;
	char* pEnd;
	BOOL bError = FALSE;
	BOOL bQuotedName = FALSE;

	/* name starts after optional first quote char */
	pSearch = source;
	while (*pSearch && ((' ' == *pSearch) || ('\"' == *pSearch)))
	{
		if ('\"' == *pSearch) bQuotedName = TRUE;
		pSearch++;
	}
	if (bQuotedName)
	{
		pEnd = strpbrk(pSearch,"\n\r\"");
	}
	else
	{
		pEnd = strpbrk(pSearch,"\n\r \"");
	}
	if (pEnd)
	{
		*pEnd = 0;
		strcpy(dest,pSearch);
		if (ppnext)
		{
			*ppnext = pEnd+1;
		}
	}
	else
	{
		bError = TRUE;
	}
	if (bError)
	{
		strcpy(dest,"error");
	}
}

/* determine if probable flag dropoff is really and truly a flag dropoff */
static int PIG_ValidateDropoff(char *pTempSearch)
{
	/* 
	 * A flag dropoff can be indicated by one of the following string 
	 * segments, immediately preceding the pTempSearch pointer passed in:
	 * > triggered "Team 1<
	 * > triggered "Team 2<
	 * > triggered "team one<
	 * > triggered "team two<
	 * The first two options are the most likely, by far..
	 */
	int iRet = 0;
	static char* pTeam1Preamble = " triggered \"Team ";
	static char* pteam1Preamble = " triggered \"team ";
	static char* pteamonePreamble = " triggered \"team ";
	int iTPLength = 17;
	char* pSearch;

	pSearch = pTempSearch - iTPLength - 1;
	if (!memcmp(pSearch, pTeam1Preamble, iTPLength))
	{
		/* matches the Team 1/2 format */
		pSearch += iTPLength;
		if ((*pSearch == '1') || (*pSearch == '2'))
		{
			/* validated */
			iRet = 1;
		}
	}
	else /* check the team one/two format */
	{
		pSearch = pTempSearch - iTPLength - 3;
		if (!memcmp(pSearch, pteamonePreamble, iTPLength))
		{
			/* matches the team one/two format */
			pSearch += iTPLength;
			if (!memcmp(pSearch,"one",3) || !memcmp(pSearch,"two",3))
			{
				/* validated */
				iRet = 1;
			}
		}
	}
	return iRet;
}

/* determine if probable flag dropoff is really and truly a flag dropoff */
static int PIG_ValidateHand(char *pTempSearch)
{
	/* 
	 * A flag dropoff can be indicated by one of the following string 
	 * segments, immediately preceding the pTempSearch pointer passed in:
	 * > triggered "r<
	 * > triggered "b<
	 */
	int iRet = 0;
	static char* pPreamble = " triggered \"";
	int iTPLength = 12;
	char* pSearch;

	pSearch = pTempSearch - iTPLength - 1;
	if (!memcmp(pSearch, pPreamble, iTPLength))
	{
		/* matches the preamble */
		pSearch += iTPLength;
		if ((*pSearch == 'r') || (*pSearch == 'b'))
		{
			/* validated */
			iRet = 1;
		}
	}
	return iRet;
}


char * PIG_SearchDropoff(char* pSearch, int* pdropofflength)
{
	static char *pdropoffstr = " dropoff\"";
	int dropofflength = 9;  /* must adjust ! */
	static char *phandstr = "hand\"";
	int handlength = 5; /* must adjust ! */
	char* pNewSearch;

	*pdropofflength = 0;
	pNewSearch = strstr(pSearch, pdropoffstr);
	if (pNewSearch && PIG_ValidateDropoff(pNewSearch))
	{
		*pdropofflength = dropofflength;
	}
	else
	{
		pNewSearch = strstr(pSearch,phandstr);
		if (pNewSearch && PIG_ValidateHand(pNewSearch))
		{
			*pdropofflength = handlength;
		}
		else
		{
			// validation failed, return NULL
			pNewSearch = NULL;
		}
	}

	return pNewSearch;
}


/* compute our first guess at player times */
static void PIG_GetPlayerTimes(char *source)
{
	BOOL bParseStarts = TRUE;
	char *pSearchStr = pnewhasentered;
	int SearchSize = strlen(pSearchStr);
	char *pSearch = source;
	char *pSaveSearch;
	char *pScan;
	time_t tstamp;
	char pname[DEFAULT_PLAYER_LENGTH];
	BOOL bDone = FALSE;
	struct PStat *pStat;

	while (!bDone)
	{
		pSaveSearch = pSearch;
		pSearch = strstr(pSearch, pSearchStr);
		if (!pSearch && bParseStarts)
		{
			/* check for old style entered */
			pSearch = pSaveSearch;
			pSearchStr = phasentered;
			SearchSize = strlen(pSearchStr);
			pSearch = strstr(pSearch, pSearchStr);
		}
		if (pSearch)
		{
			pScan = PIG_prevnewline(pSearch);
			tstamp = PIG_GetTimeStamp(pScan, &pScan);
			PIG_GetPlayerName(pScan,pname,&pScan);
			pSearch += SearchSize;
			/* validate player names */
			if (!(gbXXDefined && PIG_InvalidPlayer(XX,pname,NULL)) &&
			    !(gbXADefined && PIG_InvalidPlayer(XA,pname,NULL)) &&
				 !(gbIADefined && PIG_InvalidPlayer(IA,pname,NULL)) &&
				 !(gbIXDefined && PIG_InvalidPlayer(IX,pname,NULL)))
			{
				pStat = PIG_GetPStat(&gPStat, pname, NULL);
				pStat->used = TRUE;
				switch (bParseStarts)
				{
					case TRUE :
						{
							pStat->gameduration -= tstamp-StartTime;
							break;
						}
					case FALSE :
						{
							pStat->gameduration += tstamp-StartTime;
							break;
						}
				}
			}
		}
		else
		{
			if (bParseStarts)
			{
				bParseStarts = FALSE;
				pSearchStr = pdisconnected;
				SearchSize = strlen(pSearchStr);
				pSearch = source;
			}
			else
			{
				bDone = TRUE;
			}
		}
	}

}


/* set about figuring out a map name and teamplay */
static struct PStat * 
	PIG_GetMapInfo(char* pFirstKillPosition, char* timedate)
{
	BOOL bMapDetected = FALSE;
	char* pSearchHeader;
	static char *pspawningserver = "Spawning server \"";
	static char *ploadingmap = "Loading map \"";
	char* pNewMapString;
	int spawningserverlength;
	char* pNext;
	static char mapname[100];
	static char gm[100]; /* game (map and time stamp) */
	int Index;
	char *mp_teamplay1 = "\"mp_teamplay\" = \"1\"";
	struct PStat *pGameStat;
	
	if (pFirstKillPosition != gpBuffer)
	{
		pFirstKillPosition--;
		*pFirstKillPosition = 0;
	}
	if (bNewLogFormat)
	{
		pNewMapString = ploadingmap;
	}
	else
	{
		pNewMapString = pspawningserver;
	}
	spawningserverlength = strlen(pNewMapString);
	pSearchHeader = strstr(gpBuffer, pNewMapString);
	if (pSearchHeader)
	{
		pSearchHeader += spawningserverlength;
		pNext = strchr(pSearchHeader, '\"');
		*pNext = 0;
		strcpy(mapname,pSearchHeader);
		bMapDetected = TRUE;
	}
	if (!bMapDetected)
	{
		strcpy(mapname,UNKNOWNMAP);
	}
	sprintf(gm,"%-20s (%s)",mapname,timedate);
	/* get a reference to its data structure */
	pGameStat = PIG_GetPStat(&gGStat, gm, &Index);
	/* set its start time.. */
	pGameStat->start = StartTime;
	/* only bother with teamplay if we think it's on already */
	if (bTeamPlay)
	{
		pSearchHeader = strstr(gpBuffer, mp_teamplay1);
		if (!pSearchHeader)
		{
			bTeamPlay = FALSE;
		}
	}
	return pGameStat;
}


static int 
PIG_LogIsFinished(char* pBuffer, DWORD dwTotalRead)
{
	static char* plogclosed = ": Log closed.";
	static char* pnewlogclosed = ": Log file closed";
	char* pX = PIG_prevnewline(pBuffer+dwTotalRead-1);
	void* pvRet;

	if (*(pBuffer+OLD_LOG_PERIOD_OFFSET) == '.')
	{
		bNewLogFormat = FALSE;
		pvRet = strstr(pX,plogclosed);
	}
	else
	{
		bNewLogFormat = TRUE;
		pvRet = strstr(pX,pnewlogclosed);
		/* CS:Source logs are "new" format, but with old closing string! */
		if (!pvRet)
		{
			pvRet = strstr(pX,plogclosed);
		}
	}

	/* just look for log closed at the end of the buffer */
	return (pvRet != NULL);
}


static PROCESSEDNAME *PIG_AddFilenameToTree(char * pFileName)
{
	PROCESSEDNAME* pFilenameStruct;
	PROCESSEDNAME* pRet = NULL;

	/* only index filenames if it's appropriate to do so */
	if (gbRepeat || gbArchive)
	{
		pFilenameStruct = malloc((sizeof(PROCESSEDNAME)-DEFAULT_STRING_LENGTH)+strlen(pFileName)+1);
		if (pFilenameStruct)
		{
			strcpy(pFilenameStruct->cFName,pFileName);
			pFilenameStruct->byWaitingACK = 0;
			pFilenameStruct->byDeletePending = 0;
			pRet = *((PROCESSEDNAME**)avl_probe(gpFilenameTree,pFilenameStruct));
		}
	}
	return pRet;
}



static void PIG_ForwardLogFile(char* pPath, char* pFileName, char* pBuffer, DWORD dwBytes)
{
	static struct PIGBuffer stBuff;
	static struct PIGBuffer stSrcBuff = {0};
	int sent;
	int zlibret;
	char* pX;
	DWORD dwSent = 0;
	DWORD dwThisPacket = 0;
	uLongf destlen;
	
	/* send version, name and file... */
	memcpy(stBuff.cProtoVersion,PIGPROTOVERSION,4);
	memset(stSrcBuff.cName,0,PIGMAXNAME);
	strcpy(stSrcBuff.cName, pFileName);
	stBuff.dwSizeAll = dwBytes;
	for (pX = pBuffer, stBuff.dwChecksum = 0; *pX; pX++) 
	{
		stBuff.dwChecksum += *pX;
	}
	for (pX = pPath, stBuff.wPathChecksum = 0; *pX; pX++) 
	{
		stBuff.wPathChecksum += *pX;
	}
	
	while (dwBytes && (dwSent < dwBytes))
	{
		dwThisPacket = dwBytes-dwSent;
		if (dwThisPacket > PIGMAXFWD)
		{
			dwThisPacket = PIGMAXFWD;
		}
		destlen = sizeof stBuff.cBuffStart + sizeof stBuff.cName;
		stBuff.dwOffsetThis = dwSent;
		memcpy(&stSrcBuff.cBuffStart, pBuffer+dwSent, dwThisPacket);
		gstClient.addr.sin_port = htons(gwClientPort);
		zlibret = compress((Bytef *)&stBuff.cName,&destlen,
                                 (Bytef*)&stSrcBuff.cName, dwThisPacket+PIGMAXNAME);
		if (zlibret == Z_OK)
		{
			/* size is now compressed size of name AND file! */
			stBuff.wSizeThis = (unsigned short) destlen;
			sent = sendto(	(int)gstClient.sock,
						(char*)&stBuff,
						destlen + PIGNAMEOFF,
						0,
						(struct sockaddr*)&gstClient.addr, 
						sizeof gstClient.addr);
		}
		dwSent += dwThisPacket;
	}
	SLEEP(200);
	/*
	 * This may or may not catch the ack for this file, but it will
	 * catch acks for others.. ? 
	 *
	 */
	PIG_CheckAckSocket();
}


int PIG_ParseFile(char *pFileName)
{
	FILE *hFile;
	size_t sBytesRead;
	DWORD dwTotalRead = 0;
	int	Index;
	BOOL bDone = FALSE;
	static char *pgamestr = "(game ";
	int gamestrlength = strlen(pgamestr);
	static char *pkilledstr = "\" killed ";
	int killedlength = strlen(pkilledstr);
	static char *psuicidestr = "\" committed suicide with ";
	int suicidelength = strlen(psuicidestr);
	static char *pself = "self";
	int selflength = strlen(pself);
	static char *pbyworld = "by world";
	int byworldlength = strlen(pbyworld);
	static char *pwith = " with ";
	int withlength = strlen(pwith);
	int dropofflength;
	char *mp_teamplay1 = "\"mp_teamplay\" = \"1\"";
	int disconnectedlength = strlen(pdisconnected);
	char *pparsestr;
	int parselength;
	PARSE_MODE eMode = PARSE_FIRST;

	int nomatch = 0;
	PARSESTATE eParseState = PARSEKILLS;
	static char cFilePath[DEFAULT_STRING_LENGTH];
	static char kr[DEFAULT_PLAYER_LENGTH];
	static char ke[DEFAULT_PLAYER_LENGTH];
	static char wp[DEFAULT_PLAYER_LENGTH];
	static char timedate[100];
	struct PStat *pGameStat;
	BOOL bMapDetected = FALSE;
	char* pSearch;
	char* pTempSearch;
	char* pFirstEntered;
	char* pFirstKillPosition = NULL;
	char* pResumeSearch;
	BOOL bKillDetected = FALSE;
	BOOL bLogFinished = FALSE;
	BOOL bTooOld = FALSE;
	PROCESSEDNAME *pPFName;
	PROCESSEDNAME *pAddedPFName = NULL;
	static PROCESSEDNAME PFName;
	BOOL bReSend = FALSE;
	time_t tTempTimeStamp;
	BOOL bIsTFC = FALSE;


	TimeStamp = 0;

	strcpy(PFName.cFName,pFileName);

	if (!memcmp(pFileName,".",1) || !memcmp(pFileName,"..",2))
	{
		/* don't process directory self and parent, if present */
		goto bailwithextremeprejudice;
	}

	strcpy(cFilePath,cInPath);
	strcat(cFilePath,pFileName);

	/* check if we really should bother with this file */
	if (gbRepeat || gbArchive)
	{
		/* we're in repeat mode, so let's not parse the same files! */
		pPFName = avl_find(gpFilenameTree,&PFName);
		if (pPFName)
		{
			/* yup.. it's either a non log, or we've parsed it */
			if (pPFName->byWaitingACK)
			{
				/* re-send the file */
				bReSend = TRUE;
			}
			else if (pPFName->byDeletePending && !pPFName->byWaitingACK)
			{
				/* delete the file now */
				remove(cFilePath);
				pPFName->byDeletePending = 0;
				goto bailwithextremeprejudice;
			}
			else
			{
				goto bailwithextremeprejudice;
			}
		}
	}

	hFile = fopen(cFilePath,"r");
	if (hFile)
	{
		if (!gpBuffer)
		{
			gdwBufferSize = BUFFER_INCREMENT;
			gpBuffer = (char*) calloc(1,gdwBufferSize);
		}
		Index = 0;
		/* file opened */
		while (gpBuffer && !bDone)
		{
			sBytesRead = fread(gpBuffer+Index,1,gdwBufferSize-Index,hFile);
			dwTotalRead += sBytesRead;
			if (sBytesRead == (gdwBufferSize-Index))
			{
				/* jack up the buffer size */
				Index = gdwBufferSize;
				gdwBufferSize += BUFFER_INCREMENT;
				gpBuffer = (char*)realloc(gpBuffer, gdwBufferSize);
			}
			else if (feof(hFile))
			{
				/* got our bufferload */
				bDone = TRUE;
				gpBufferTop = gpBuffer+dwTotalRead;
				*gpBufferTop = 0;
				gBytesProcessed += dwTotalRead;
				gFilesProcessed++;
			}
			else
			{
				/* error case */
				bDone = TRUE;
				gpBufferTop = gpBuffer+dwTotalRead;
				*gpBufferTop = 0;
			}
		}

		/* check to see if it must be resent */
		if (bReSend)
		{
			PIG_ForwardLogFile(cInPath, pFileName, gpBuffer, dwTotalRead);
			/* that much done, we don't need to reprocess it */
			goto bail;
		}

		/* otherwise, take a look at what we've caught in our net */

		/* what do we have here */
		if (dwTotalRead < 2)
		{
			/* nothing yet.. could be a log file-to-be, however */
			goto bail;
		}
		else if ((*gpBuffer != 'L') || (*(gpBuffer+1) != ' '))
		{
			/* first two bytes are wrong for a log file.. skip forever */
			pAddedPFName = PIG_AddFilenameToTree(pFileName);
			goto bail;
		}
		else if (dwTotalRead < 26)
		{
			/* log file, but not long enough yet */
			goto bail;
		} 
		else 
		{
			/* long enough log file candidate: check to see if it's finished */
			bLogFinished = PIG_LogIsFinished(gpBuffer,dwTotalRead);
			if (bLogFinished)
			{
				StartTime = PIG_GetTimeStamp(gpBuffer, NULL);
				/* a good log file.. don't allow us to repeat it */
				pAddedPFName = PIG_AddFilenameToTree(pFileName);
			}
			else
			{
				/* it may be a log file, but the end marker aint there */
				goto bail;
			}
		}
		
		/* check for log filtering */
		if (gbLogFilter && !PIG_TimeStampIsAccepted(&bTooOld))
		{
			fclose(hFile);
			/* delete old files? */
			if (gbDeleteFiles && bTooOld && pAddedPFName)
			{ 
				if(!pAddedPFName->byWaitingACK)
				{
					/* not waiting for ACK */
					remove(cFilePath);
				}
				else
				{
					/* waiting for ACK, set flag saying that delete is pending */
					pAddedPFName->byDeletePending = 1;
				}
			}
			goto bailwithextremeprejudice; /* don't close again */
		}
		/* do we need to forward it to another machine? */
		if (gbLogForwarding)
		{
			if (pAddedPFName)
			{
				/* mark the file name as needing ACK */
				pAddedPFName->byWaitingACK = 1;
			}
			PIG_ForwardLogFile(cInPath, pFileName, gpBuffer, dwTotalRead);
		}
		memcpy(timedate,gpBuffer+TIME_STAMP_OFFSET, TIME_STAMP_SHORTLENGTH);
		timedate[TIME_STAMP_SHORTLENGTH]=0;

		/* do we have a tfc map on our hands? */
		pSearch = strstr(gpBuffer, pgamestr);
		if (pSearch && !memcmp(pSearch + gamestrlength,"\"tfc\"",5))
		{
			gbIsTFC = TRUE;
			bIsTFC = TRUE;
		}


		/* search for the "killed" transactions */
		while (eMode != PARSE_DONE)
		{
			switch (eMode)
			{
			case PARSE_KILLS : 
				pparsestr = pkilledstr;
				parselength = killedlength;
				break;
			case PARSE_SUICIDES :
				pparsestr = psuicidestr;
				parselength = suicidelength;
				break;
			default :
			case PARSE_DROPOFFS :
				break;
			}
			
			pSearch = gpBuffer;
			while (	(pSearch < gpBufferTop) )
			{
				if (PARSE_DROPOFFS == eMode)
				{
					pTempSearch = PIG_SearchDropoff(pSearch, &dropofflength);
				}
				else
				{
					pTempSearch = strstr(pSearch, pparsestr);
				}
				if (NULL != pTempSearch)
				{
					if (eMode == PARSE_DROPOFFS)
					{
						pResumeSearch = pTempSearch + dropofflength;
					}
					pSearch = pTempSearch;
					/* set flag indicating kills were found */
					bKillDetected = TRUE;
					/* position at start of this line */
					pSearch = PIG_prevnewline(pSearch);
					/* note the position of the first kill, if appropriate */
					if (!pFirstKillPosition)
					{
						/* or the first "has entered" event */
						pFirstEntered = strstr(gpBuffer,pnewhasentered);
						if (!pFirstEntered)
						{
							/* try old style */
							pFirstEntered = strstr(gpBuffer,phasentered);
						}
						if (pFirstEntered && (pFirstEntered < pSearch))
						{
							pFirstEntered = PIG_prevnewline(pFirstEntered);
							pFirstKillPosition = pFirstEntered;
						}
						else
						{
							pFirstKillPosition = pSearch;
						}
						/* compute our first guess at player times */
						PIG_GetPlayerTimes(pFirstKillPosition);
						/* set about figuring out a map name and teamplay */
						pGameStat = PIG_GetMapInfo(pFirstKillPosition,timedate);
					}
					/* get time stamp, advance search index */
					tTempTimeStamp = PIG_GetTimeStamp(pSearch, &pSearch);
					if (tTempTimeStamp > TimeStamp) TimeStamp = tTempTimeStamp;

					/* get player name, advance search index */
					PIG_GetPlayerName(pSearch,kr,&pSearch);
					/* advance index past " killed " */
					pSearch += parselength-1; /* positions after last quote in first player */
					/* look for different things, depending on mode */
					switch (eMode)
					{
					case PARSE_KILLS :
						/* next thing a victim, self or by world? */
						switch (*pSearch)
						{
						case '\"' :  /* killed victim */
							{
								eParseState = PARSEKILLS;
								PIG_GetPlayerName(pSearch, ke, &pSearch);
								/* check for new "by world" nomenclature */
								if (!strcmp(ke,"-1"))
								{
									eParseState = PARSEDEATHS;
									strcpy(ke,pbyworld);
								}
								break;
							}
						case 's' : /* killed self */
							{
								eParseState = PARSESUICIDES;
								strcpy(ke,pself);
								pSearch += selflength;
								break;
							}
						case 'b' : /* killed by world */
						default :
							{
								eParseState = PARSEDEATHS;
								strcpy(ke,pbyworld);
								pSearch += byworldlength;
								break;
							}
						}
						/* skip over the with */
						pSearch += withlength;
						/* retrieve the agent of this disaster */
						PIG_GetWeaponName(pSearch, wp, &pSearch);
						break;
					case PARSE_SUICIDES : /* for compatibility w/ new standard */
						eParseState = PARSESUICIDES;
						strcpy(ke,pself);
						PIG_GetWeaponName(pSearch, wp, &pSearch);
						break;
					case PARSE_DROPOFFS : 
					default :
						eParseState = PARSEDROPOFFS;
						pSearch = pResumeSearch;
						ke[0] = 0;
						break;
					}
					if (PIG_UpdatePStat(0,kr,ke,wp,pGameStat,eParseState))
					{
						return 1;
					}
				}
				else
				{
					pSearch = pSearch + strlen(pSearch) + 1;
				}
			}
			/* increment the parsing mode */
			eMode++;
			if ((eMode == PARSE_DROPOFFS) && !bIsTFC)
			{
				/* skip dropoffs if not a tfc map */
				eMode++;
			}
		}



		/* check for kills, and if not, generate a map entry anyway */
		if (bKillDetected)
		{
			/* kills, print the file name if not silent */
			if (!gbSilent)
			{
				printf("Parsed: %s            \r",pFileName);
			}
			/* set our global state */
			gbKillDetected = TRUE;
		}
		else
		{
			/* no kills detected */
			PIG_GetMapInfo(gpBuffer, timedate);
		}
		

		/* initialize the player time stamp registers */
		PIG_ClearPSTimeStamps();

bail:
		fclose(hFile);
 	}
bailwithextremeprejudice:
	return 0;
}
