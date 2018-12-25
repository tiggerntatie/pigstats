/****************************************************************************

$Header: pigjs.c  Revision:1.24  Sat Sep 29 22:52:50 2001  ericd $
  
pigjs.c

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
$Log: C:\WORK\PigStats\Archive\SourceDist\CSource\pigjs.d $
  
    Split pigstats.c into pigjs.c, pigparse.c, pigtext.c and pigutil.c
  
  Revision 1.24  by: ericd  Rev date: Sat Sep 29 22:52:50 2001
    Fixed bug that occurs when someone chats a phrase ending in ..hand"
  
  Revision 1.23  by: ericd  Rev date: Wed Sep 05 20:41:48 2001
    T-score and L-scores inaccurate when doing archived or multiple runs.. 
    
  
  Revision 1.22  by: ericd  Rev date: Wed Aug 01 21:49:46 2001
    Initial release for custom TFC support for LP Boss (1.7a)
  
  Revision 1.21  by: ericd  Rev date: Tue Jan 30 19:32:28 2001
    change case
  
  Revision 1.20  by: ericd  Rev date: Sun Jan 07 14:28:54 2001
    Version 1.5 candidate
  
  Revision 1.19  by: ericd  Rev date: Sat Oct 28 15:40:12 2000
    First changes for 1.4f client server architecture
  
  Revision 1.18  by: ericd  Rev date: Wed Sep 13 22:43:42 2000
    1.4e candidate
  
  Revision 1.17  by: ericd  Rev date: Fri Aug 25 23:20:10 2000
    Full repetitive and archiving implementation, Linux safe, some bugs:
    default rep rate is off
  
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
  
  Revision 1.13  by: ericd  Rev date: Wed Aug 02 22:06:14 2000
    Fixed bug in reporting argument metadata
  
  Revision 1.12  by: ericd  Rev date: Thu Jul 27 22:06:30 2000
    1) Workaround for CS dedicated server log splitting
  
  Revision 1.11  by: ericd  Rev date: Thu Jul 20 22:45:02 2000
    Meta data generation
  
  Revision 1.10  by: ericd  Rev date: Wed Jul 19 22:18:16 2000
    1) Modified to use btree lookup when locating player info
  
  Revision 1.9  by: ericd  Rev date: Mon Jul 03 22:47:12 2000
    Eliminated use of regex for sake of speed
  
  Revision 1.8  by: ericd  Rev date: Tue Jun 13 22:00:22 2000
    Rev. 1.4c (Linux port) candidate
  
  Revision 1.7  by: ericd  Rev date: Sun May 07 22:42:54 2000
    Fixed bug that was repeating match wins calculation
  
  Revision 1.6  by: ericd  Rev date: Mon May 01 22:53:10 2000
    Added match win totals
  
  Revision 1.5  by: ericd  Rev date: Wed Apr 19 22:39:04 2000
    Fixed bug in game filtering
  
  Revision 1.4  by: ericd  Rev date: Sun Apr 16 17:44:22 2000
    Generate lower case js file names.
  
  Revision 1.3  by: ericd  Rev date: Mon Apr 10 23:16:56 2000
    Support for game stats
  
  Revision 1.2  by: ericd  Rev date: Sun Apr 09 23:44:20 2000
    Initial support for map/game statistics
  
  Revision 1.1  by: ericd  Rev date: Sat Apr 01 18:50:10 2000
    Removed junk in expanded keywords
  
  Revision 1.0  by: ericd  Rev date: Sat Apr 01 18:44:14 2000
    Initial revision.
  
  $Endlog$
 */

#ifndef __linux__
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pigstats.h"
#include "pigparse.h"
#include "pigutil.h"


//#define WRITEFILE(s) 	bRes&=WriteFile(hOutFile,(s),strlen(s),&dwWritten,NULL );
#define WRITEFILE(s) 	bRes&=(0<fprintf(hOutFile,(s)))


/* boilerplate */
const char* pPSOverAStart1 = 
"// PigStats for JavaScript \r\n\
// Overall player statistics\r\n\r\n\
// wi[\"<weapon name>\"] gives weapon index from name\r\n\
// wq quantity of weapons logged\r\n\
// w[<weapon index>][0] gives weapon name\r\n\
// w[<weapon index>][1] gives total kills with weapon \r\n\
// w[<weapon index>][2] gives number of players listed in wk for this weapon \r\n\
// wk[<weapon index][<0..w[<weapon index>][1]>][0] gives kills by player, listed in descending order\r\n\
// wk[<weapon index][<0..w[<weapon index>][1]>][1] gives player name\r\n\
// pq gives player quantity\r\n\
// pk[<0..pq-1>] list of player indices, sorted by kills (best first)\r\n\
// pkq quantity of players in sorted kills list\r\n\
// pts[<0..pq-1>] list of pl. indices, sort by TFC Score (tfc only, best first)\r\n\
// ptsq quantity of players in sorted TFC Score list (tfc only)\r\n\
// pr[<0..pq-1>] list of player indices, sorted by ratio (best first)\r\n\
// prq quantity of players in sorted ratio list\r\n\
// prt[<0..pq-1>] list of player indices, sorted by rate (best first)\r\n\
// prtq quantity of players in sorted rate list\r\n\
// pd[<0..pq-1>] list of player indices, sorted by deaths (best first)\r\n\
// pdq quantity of players in sorted deaths list\r\n\
// ps[<0..pq-1>] list of player indices, sorted by suicides (best first)\r\n";
const char* pPSOverAStart2 = "\
// psq quantity of players in sorted suicides list\r\n\
// pmw[<0..pq-1>] list of player indices, sorted by match wins (best first)\r\n\
// pmwq quantity of players in sorted match wins list\r\n\
// pt[<0..pq-1>] list of pl. indices, sort by time played (highest first)\r\n\
// ptq quantity of players in sorted time played list\r\n\
// pc[<0..pq-1>] list of player indices, sorted by caps (tfc only, best first)\r\n\
// pcq quantity of players in sorted caps list (tfc only)\r\n\
// psk[<0..pq-1>] list of pl. indices, sort by skill (tfc only, best first)\r\n\
// pskq quantity of players in sorted skill list (tfc only)\r\n\
// pi[\"<player name>\"] gives player index from name\r\n";
const char* pPSOverAStart3 = "\
// p[<player index>][0] gives player name\r\n\
// p[<player index>][1] gives player link file name\r\n\
// p[<player index>][2] gives player kills total\r\n\
// p[<player index>][3] gives player deaths total\r\n\
// p[<player index>][4] gives player suicides total\r\n\
// p[<player index>][5] gives player kill/death ratio (-1.0 means no deaths)\r\n\
// p[<player index>][6] gives player kill rate (kills/minute)(-1.0 means play time is zero)\r\n\
// p[<player index>][7] gives player play time (minutes)\r\n\
// p[<player index>][8] gives quantity of killed players in px\r\n\
// p[<player index>][9] gives quantity of players killed by in px\r\n\
// p[<player index>][10] gives quantity of kill/death ratio against players in px\r\n\
// p[<player index>][11] gives quantity of kill weapons in px\r\n\
// p[<player index>][12] gives quantity of killed by weapons in px\r\n\
// p[<player index>][13] gives player match wins total\r\n\
// p[<player index>][14] gives player caps (tfc only)\r\n";
const char* pPSOverAStart4 = "\
// p[<player index>][15] gives player skill (tfc only)\r\n\
// p[<player index>][16] gives player TFC Score (tfc only)\r\n\
// px[<player index>][0][n][0] gives kill count against player\r\n\
// px[<player index>][0][n][1] gives kill against player name\r\n\
// px[<player index>][1][n][0] gives killed by count against player\r\n\
// px[<player index>][1][n][1] gives killed by player name\r\n\
// px[<player index>][2][n][0] gives kill ratio against player\r\n\
// px[<player index>][2][n][1] gives kill ratio against player name\r\n\
// px[<player index>][3][n][0] gives kill count with weapon\r\n\
// px[<player index>][3][n][1] gives kill by weapon name\r\n\
// px[<player index>][4][n][0] gives killed by count with weapon\r\n\
// px[<player index>][4][n][1] gives killed by weapon name\r\n\
// metabp gives total bytes processed\r\n\
// metafp gives total files processed\r\n\
// metapds gives total processing duration, in seconds\r\n\
// metaptd gives time and date of processing\r\n\
// metatpp gives total players processed (differs from pq if filtering is used)\r\n\
// metaos gives operating system used to process\r\n\
// metapsv gives pigstats version string\r\n\
// metapsa gives pigstats arguments description\r\n\
// aretfcstats boolean is true if any tfc maps were parsed\r\n\
\r\n";

const char* pPSGameStart = 
"// PigStats for JavaScript \r\n\
// Game and map statistics\r\n\r\n\
// gq quantity of games logged\r\n\
// mwq quantity of players listed in match wins\r\n\
// tp indicates teamplay if true\r\n\
// g[<game index>][0] gives map name\r\n\
// g[<game index>][1] gives game time\r\n\
// g[<game index>][2] gives number of players in game\r\n\
// gp[<game index>][<player index>][0] gives player name\r\n\
// gp[<game index>][<player index>][1] gives player kills\r\n\
// gp[<game index>][<player index>][2] gives player deaths\r\n\
// gp[<game index>][<player index>][3] gives player caps (tfc only)\r\n\
// mw[<index>][0] gives player name in match wins list\r\n\
// mw[<index>][1] gives player match wins in match wins list \r\n\
\r\n";


 /*
 * PIG_JSOverallStats
 *
 * Generate JavaScript summary of overall statistics
 */

int	PIG_JSOverallStats(void)
{
	double fRatioTemp,fRateTemp,fRatioTempOA;
	int Temp;
	int Index;
	int Index2;
	int FCount;
	int FCount2;
	struct PStat* pPS;
	struct PStat* pPSFav;
	struct PStat* pOPS;
	struct PStat* pWPS;
	static char cOut[DEFAULT_STRING_LENGTH];
	static char cTmp[DEFAULT_STRING_LENGTH];
	static char cName[DEFAULT_STRING_LENGTH];
	FILE *hOutFile;
	BOOL bRes = TRUE;
	BOOL *pbFiltered = NULL;
	int *piPStatList;
	int PStatListQty;
	BOOL bPrintLine;
	int PKCount,PVCount,PRCount,WKCount,WVCount;
	BOOL bXRefOK;
	int iXIndex;

	strcpy(gcOutPath, gcJPath);
#ifdef __linux__
	strcat(gcOutPath, "/psovera.js");
#else
	strcat(gcOutPath, "\\psovera.js");
#endif
	/* open the output file */
	hOutFile = fopen(gcOutPath,"w");

	if (!hOutFile)
	{
		printf("sorry... file name error\r\n");
		return 0;
	}
	
	/* display a banner */
	WRITEFILE(pPSOverAStart1);
	WRITEFILE(pPSOverAStart2);
	WRITEFILE(pPSOverAStart3);
	WRITEFILE(pPSOverAStart4);

	/* display weapon information */
	PIG_SetupPStatList(&gWStat);
	PIG_SortPStatListByK();
	/* make a local copy of the list */
	PStatListQty = gPStatListQty;
	piPStatList = (int*)calloc(gPStatListQty,sizeof(int));
	if (!piPStatList)
	{
		printf("Allocation failure\n");
		fclose(hOutFile);
		return 0;
	}
	memcpy(piPStatList,gpiPStatList,gPStatListQty*sizeof(int));
	/* wi: weapon index, w: general weapon info, wk: weapon killer ranking */
	sprintf(cOut,"wi=new(Array)\r\nw=new(Array)\r\nwk=new(Array)\r\n");
	WRITEFILE(cOut);
	PStatListQty = gPStatListQty;
	FCount2 = 0;
	for(Index=0;Index<PStatListQty;Index++)
	{
		pPS = gWStat.pPStat + *(piPStatList+Index);
		if (pPS->nk)
		{
			sprintf(cOut,"wi[\"%s\"]=%d\r\n",
				(gWStat.pPStat+*(piPStatList+Index))->pPName,FCount2);
			WRITEFILE(cOut);
			/* sort to find most common killers with this weapon */
			PIG_SetupPStatList(&pPS->OPStat);
			PIG_SortPStatListByK();  
			sprintf(cOut,"wk[%d]=[",FCount2);
			WRITEFILE(cOut);
			FCount = 0;
			for (Index2=0;Index2<gPStatListQty;Index2++)
			{
				if (bFilter && iLimit && (FCount==iLimit)) break;
				pPSFav = pPS->OPStat.pPStat + *(gpiPStatList+Index2);
				sprintf(cOut,"[%d,\"%s\"],",pPSFav->nk,pPSFav->pPName);
				WRITEFILE(cOut);
				FCount++;
			}
			sprintf(cOut,"]\r\n");
			WRITEFILE(cOut);
			/* now print the weapon kill total, and number of players listed in wk */
			sprintf(cOut,"w[%d]=[\"%s\",%d,%d]\r\n",FCount2,pPS->pPName,pPS->nk,FCount);
			WRITEFILE(cOut);
			FCount2++;
		}
	}
	/* how many weapons are listed here: */
	sprintf(cOut,"wq=%d\r\n",FCount2);
	WRITEFILE(cOut);
	free (piPStatList);

	/* set up for filtering if nec. */
	PIG_SetupPStatList(&gPStat);
	if (bFilter)
	{
		pbFiltered = (BOOL*) calloc(gPStatListQty,sizeof(BOOL));
		if (!pbFiltered)
		{
			printf("Allocation failure\n");
			fclose(hOutFile);
			return 0;
		}
	}

	if (gbIsTFC)  /* if TFC, filter by score, not kills */
	{
		/* sort by t-score (tfc only) */
		sprintf(cOut,"pts=[");
		WRITEFILE(cOut);
		PIG_SortPStatListByTS();
		FCount = 0;
		for(Index=0;Index<gPStatListQty;Index++)
		{
			if (bFilter && iLimit && (FCount==iLimit)) break;
			Temp = *(gpiPStatList+Index);
			if (!bFilter || (gPStat.pPStat+Temp)->score)
			{
				FCount++;
				sprintf(cOut,"%d,",Temp);
				WRITEFILE(cOut);
				if (bFilter)
				{
					*(pbFiltered+Temp) = TRUE;
				}
			}
		}
		sprintf(cOut,"]\r\n");
		WRITEFILE(cOut);
		sprintf(cOut,"ptsq=%d\r\n",FCount);
		WRITEFILE(cOut);

		/* sort by kills */
		sprintf(cOut,"pk=[");
		WRITEFILE(cOut);
		PIG_SortPStatListByK();
		FCount = 0;
		for(Index=0;Index<gPStatListQty;Index++)
		{
			if (bFilter && iLimit && (FCount==iLimit)) break;
			Temp = *(gpiPStatList+Index);
			if (!bFilter || iLimit || (gPStat.pPStat+Temp)->nk)
			{
				/* if filtering, only take ones that already passed */
				if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
				{
					FCount++;
					sprintf(cOut,"%d,",Temp);
					WRITEFILE(cOut);
				}
			}
		}
		sprintf(cOut,"]\r\n");
		WRITEFILE(cOut);
		sprintf(cOut,"pkq=%d\r\n",FCount);
		WRITEFILE(cOut);
	}
	else  /* not TFC .. filter by kills*/
	{
		/* sort by kills */
		sprintf(cOut,"pk=[");
		WRITEFILE(cOut);
		PIG_SortPStatListByK();
		FCount = 0;
		for(Index=0;Index<gPStatListQty;Index++)
		{
			if (bFilter && iLimit && (FCount==iLimit)) break;
			Temp = *(gpiPStatList+Index);
			if (!bFilter || (gPStat.pPStat+Temp)->nk)
			{
				FCount++;
				sprintf(cOut,"%d,",Temp);
				WRITEFILE(cOut);
				if (bFilter)
				{
					*(pbFiltered+Temp) = TRUE;
				}
			}
		}
		sprintf(cOut,"]\r\n");
		WRITEFILE(cOut);
		sprintf(cOut,"pkq=%d\r\n",FCount);
		WRITEFILE(cOut);

	}

	/* sort by ratio */
	sprintf(cOut,"pr=[");
	WRITEFILE(cOut);
	PIG_SortPStatListByR();
	FCount = 0;
	for(Index=0;Index<gPStatListQty;Index++)
	{
		if (bFilter && iLimit && (FCount==iLimit)) break;
		Temp = *(gpiPStatList+Index);
		pPS = gPStat.pPStat+Temp;
		if (!bFilter || pPS->nk)
		{
			/* if filtering, only take ones that already passed */
			if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
			{
				FCount++;
				sprintf(cOut,"%d,",Temp);
				WRITEFILE(cOut);
			}
		}
	}
	sprintf(cOut,"]\r\n");
	WRITEFILE(cOut);
	sprintf(cOut,"prq=%d\r\n",FCount);
	WRITEFILE(cOut);
	/* sort by rate */
	sprintf(cOut,"prt=[");
	WRITEFILE(cOut);
	PIG_SortPStatListByKR();
	FCount = 0;
	for(Index=0;Index<gPStatListQty;Index++)
	{
		if (bFilter && iLimit && (FCount==iLimit)) break;
		Temp = *(gpiPStatList+Index);
		pPS = gPStat.pPStat+Temp;
		if (!bFilter || pPS->nk)
		{
			/* if filtering, only take ones that already passed */
			if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
			{
				FCount++;
				sprintf(cOut,"%d,",Temp);
				WRITEFILE(cOut);
			}
		}
	}
	sprintf(cOut,"]\r\n");
	WRITEFILE(cOut);
	sprintf(cOut,"prtq=%d\r\n",FCount);
	WRITEFILE(cOut);
	/* sort by deaths */
	sprintf(cOut,"pd=[");
	WRITEFILE(cOut);
	PIG_SortPStatListByD();
	FCount = 0;
	for(Index=0;Index<gPStatListQty;Index++)
	{
		if (bFilter && iLimit && (FCount==iLimit)) break;
		Temp = *(gpiPStatList+Index);
		/* always show deaths of zero if a limit is specified */
		if (!bFilter || iLimit || (gPStat.pPStat+Temp)->nd)
		{
			/* if filtering, only take ones that already passed */
			if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
			{
				FCount++;
				sprintf(cOut,"%d,",Temp);
				WRITEFILE(cOut);
			}
		}
	}
	sprintf(cOut,"]\r\n");
	WRITEFILE(cOut);
	sprintf(cOut,"pdq=%d\r\n",FCount);
	WRITEFILE(cOut);
	/* sort by suicides */
	sprintf(cOut,"ps=[");
	WRITEFILE(cOut);
	PIG_SortPStatListByS();
	FCount = 0;
	for(Index=0;Index<gPStatListQty;Index++)
	{
		if (bFilter && iLimit && (FCount==iLimit)) break;
		Temp = *(gpiPStatList+Index);
		/* always show suicides of zero if a limit is specified */
		if (!bFilter || iLimit || (gPStat.pPStat+Temp)->ns)
		{
			/* if filtering, only take ones that already passed */
			if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
			{
				FCount++;
				sprintf(cOut,"%d,",Temp);
				WRITEFILE(cOut);
			}
		}
	}
	sprintf(cOut,"]\r\n");
	WRITEFILE(cOut);
	sprintf(cOut,"psq=%d\r\n",FCount);
	WRITEFILE(cOut);

	/* sort by match wins */
	sprintf(cOut,"pmw=[");
	WRITEFILE(cOut);
	PIG_SortPStatListByNMW();
	FCount = 0;
	for(Index=0;Index<gPStatListQty;Index++)
	{
		if (bFilter && iLimit && (FCount==iLimit)) break;
		Temp = *(gpiPStatList+Index);
		if (!bFilter || iLimit || (gPStat.pPStat+Temp)->nmw)
		{
			/* if filtering, only take ones that already passed */
			if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
			{
				FCount++;
				sprintf(cOut,"%d,",Temp);
				WRITEFILE(cOut);
			}
		}
	}
	sprintf(cOut,"]\r\n");
	WRITEFILE(cOut);
	sprintf(cOut,"pmwq=%d\r\n",FCount);
	WRITEFILE(cOut);

	/* sort by time played */
	sprintf(cOut,"pt=[");
	WRITEFILE(cOut);
	PIG_SortPStatListByT();
	FCount = 0;
	for(Index=0;Index<gPStatListQty;Index++)
	{
		if (bFilter && iLimit && (FCount==iLimit)) break;
		Temp = *(gpiPStatList+Index);
		if (!bFilter || iLimit || (gPStat.pPStat+Temp)->duration)
		{
			/* if filtering, only take ones that already passed */
			if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
			{
				FCount++;
				sprintf(cOut,"%d,",Temp);
				WRITEFILE(cOut);
			}
		}
	}
	sprintf(cOut,"]\r\n");
	WRITEFILE(cOut);
	sprintf(cOut,"ptq=%d\r\n",FCount);
	WRITEFILE(cOut);

	/* sort by caps (tfc only) */
	if (gbIsTFC)
	{
		sprintf(cOut,"pc=[");
		WRITEFILE(cOut);
		PIG_SortPStatListByC();
		FCount = 0;
		for(Index=0;Index<gPStatListQty;Index++)
		{
			if (bFilter && iLimit && (FCount==iLimit)) break;
			Temp = *(gpiPStatList+Index);
			if (!bFilter || iLimit || (gPStat.pPStat+Temp)->ncaps)
			{
				/* if filtering, only take ones that already passed */
				if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
				{
					FCount++;
					sprintf(cOut,"%d,",Temp);
					WRITEFILE(cOut);
				}
			}
		}
		sprintf(cOut,"]\r\n");
		WRITEFILE(cOut);
		sprintf(cOut,"pcq=%d\r\n",FCount);
		WRITEFILE(cOut);
	}

	/* sort by skill (tfc only) */
	if (gbIsTFC)
	{
		sprintf(cOut,"psk=[");
		WRITEFILE(cOut);
		PIG_SortPStatListBySK();
		FCount = 0;
		for(Index=0;Index<gPStatListQty;Index++)
		{
			if (bFilter && iLimit && (FCount==iLimit)) break;
			Temp = *(gpiPStatList+Index);
			if (!bFilter || iLimit || (gPStat.pPStat+Temp)->skill)
			{
				/* if filtering, only take ones that already passed */
				if (!bFilter || (bFilter && *(pbFiltered+Temp)==TRUE))
				{
					FCount++;
					sprintf(cOut,"%d,",Temp);
					WRITEFILE(cOut);
				}
			}
		}
		sprintf(cOut,"]\r\n");
		WRITEFILE(cOut);
		sprintf(cOut,"pskq=%d\r\n",FCount);
		WRITEFILE(cOut);
	}

	/* generate data for all players */
	sprintf(cOut,"pi=new(Array)\r\n");
	WRITEFILE(cOut);
	FCount = 0;
	for (Index=0;Index<gPStatListQty;Index++)
	{
		if (!bFilter || (bFilter && *(pbFiltered+Index)))
		{
			FCount++;
			sprintf(cOut,"pi[\"%s\"]=%d\r\n",(gPStat.pPStat+Index)->pPName,Index);
			WRITEFILE(cOut);
		}
	}
	if (!bFilter)
	{
		FCount = gPStatListQty;
	}
	sprintf(cOut,"pq=%d\r\n",FCount);
	WRITEFILE(cOut);
	/* detailed player information */
	sprintf(cOut,"p=new(Array)\r\n");
	WRITEFILE(cOut);
	sprintf(cOut,"px=new(Array)\r\n");
	WRITEFILE(cOut);
	PStatListQty = gPStatListQty;
	for (Index=0;Index<PStatListQty;Index++)
	{
		pPS = (gPStat.pPStat+Index);
		/* compute ratio */
		if (pPS->nd)
		{
			fRatioTempOA = 1.0*pPS->nk/pPS->nd;
		}
		else
		{
			fRatioTempOA = -1.0;
		}
		/* and rate */
		if (pPS->duration)
		{
			fRateTemp = 60.0*pPS->nk/pPS->duration;
		}
		else
		{
			fRateTemp = -1.0;
		}
		bPrintLine = FALSE;
		if (!bFilter || (bFilter && *(pbFiltered+Index)))
		{
			bPrintLine = TRUE;
		}
		if (bPrintLine)
		{
			/* get list of other players interacted with */
			sprintf(cOut,"px[%d]=[[",Index);
			WRITEFILE(cOut);
			pOPS = pPS->OPStat.pPStat;
			/* sort by kills */
			PIG_SetupPStatList(&pPS->OPStat);
			PIG_SortPStatListByK();
			PKCount = 0;
			for(Index2=0;Index2<gPStatListQty;Index2++)
			{
				if (PKCount == iXLimit) break;
				pOPS = pPS->OPStat.pPStat + *(gpiPStatList+Index2);
				/* if bIgnoreUnranked don't crossreference unranked players */
				bXRefOK = TRUE;
				if (bFilter && bIgnoreUnranked)
				{
					PIG_GetPStat(&gPStat,pOPS->pPName,&iXIndex);
					bXRefOK = *(pbFiltered+iXIndex);
				}
				if (bXRefOK)
				{
					sprintf(cOut,"[%d,\"%s\"],",pOPS->nk,pOPS->pPName);
					WRITEFILE(cOut);
					PKCount++;
				}
			}
			WRITEFILE("],[");
			PIG_SortPStatListByD();
			PVCount = 0;
			for(Index2=gPStatListQty;Index2;Index2--)
			{
				if (PVCount == iXLimit) break;
				pOPS = pPS->OPStat.pPStat + *(gpiPStatList+Index2-1);
				/* if bIgnoreUnranked don't crossreference unranked players */
				bXRefOK = TRUE;
				if (bFilter && bIgnoreUnranked)
				{
					PIG_GetPStat(&gPStat,pOPS->pPName,&iXIndex);
					bXRefOK = *(pbFiltered+iXIndex);
				}
				if (bXRefOK)
				{
					sprintf(cOut,"[%d,\"%s\"],",pOPS->nd,pOPS->pPName);
					WRITEFILE(cOut);
					PVCount++;
				}
			}
			WRITEFILE("],[");
			PIG_SortPStatListByR();
			PRCount = 0;
			for(Index2=0;Index2<gPStatListQty;Index2++)
			{
				if (PRCount == iXLimit) break;
				pOPS = pPS->OPStat.pPStat + *(gpiPStatList+Index2);
				/* if bIgnoreUnranked don't crossreference unranked players */
				bXRefOK = TRUE;
				if (bFilter && bIgnoreUnranked)
				{
					PIG_GetPStat(&gPStat,pOPS->pPName,&iXIndex);
					bXRefOK = *(pbFiltered+iXIndex);
				}
				if (bXRefOK)
				{
					if (pOPS->nd)
					{
						fRatioTemp = 1.0*pOPS->nk/pOPS->nd;
						sprintf(cOut,"[%.3f,\"%s\"],",fRatioTemp,pOPS->pPName);
					}
					else
					{
						sprintf(cOut,"[-1.0,\"%s\"],",pOPS->pPName);
					}
					WRITEFILE(cOut);
					PRCount++;
				}
			}
			WRITEFILE("],[");
			pWPS = pPS->WStat.pPStat;
			PIG_SetupPStatList(&pPS->WStat);
			PIG_SortPStatListByK();
			WKCount = 0;
			for(Index2=0;Index2<gPStatListQty;Index2++)
			{
				if (WKCount == iXLimit) break;
				pOPS = pPS->WStat.pPStat + *(gpiPStatList+Index2);
				if (!pOPS->nk) break;
				sprintf(cOut,"[%d,\"%s\"],",pOPS->nk,pOPS->pPName);
				WRITEFILE(cOut);
				WKCount++;
			}
			WRITEFILE("],[");
			PIG_SortPStatListByD();
			WVCount = 0;
			for(Index2=gPStatListQty;Index2;Index2--)
			{
				if (WVCount == iXLimit) break;
				pOPS = pPS->WStat.pPStat + *(gpiPStatList+Index2-1);
				if (!pOPS->nd) break;
				sprintf(cOut,"[%d,\"%s\"],",pOPS->nd,pOPS->pPName);
				WRITEFILE(cOut);
				WVCount++;
			}
			WRITEFILE("]]\r\n");

			sprintf(cOut,"p[%d]=[\"%s\",\"%s\",%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%d,%d,%d",
				Index,
				pPS->pPName,
				PIG_MakeStringValidFileName(pPS->pPName,cName),
				pPS->nk,pPS->nd,pPS->ns,
				fRatioTempOA,
				fRateTemp,
				pPS->duration/60.0,
				PKCount, // killed N players
				PVCount, // killed by N players
				PRCount, // ratio against N players
				WKCount, // killed with N weapons
				WVCount, // killed by N weapons
				pPS->nmw);// match wins
			WRITEFILE(cOut);
			if (gbIsTFC)
			{
				sprintf(cOut,",%d,%.2f,%d",
					pPS->ncaps,
					pPS->skill/10.0,
					pPS->score);
				WRITEFILE(cOut);
			}
			WRITEFILE("]\r\n");
		}
	}
	/* generate meta data for the processing */
	sprintf(cOut,"metabp=%d\r\n",gBytesProcessed);
	WRITEFILE(cOut);
	sprintf(cOut,"metafp=%d\r\n",gFilesProcessed);
	WRITEFILE(cOut);
	sprintf(cOut,"metapds=%g\r\n",gProcessDurationSecs);
	WRITEFILE(cOut);
	/* strip out the newline stuff */
	gcProcessTime[24]=0;
	sprintf(cOut,"metaptd='%s'\r\n",gcProcessTime);
	WRITEFILE(cOut);
	sprintf(cOut,"metapsv='%s'\r\n",PIGVERSTRING);
	WRITEFILE(cOut);
	sprintf(cOut,"metatpp=%d\r\n",gPStat.iCount);
	WRITEFILE(cOut);
	sprintf(cOut,"metapsa='");
	if (bFilter)
	{
		sprintf(cTmp,"top %d players, ",iLimit);
	}
	else
	{
		sprintf(cTmp,"all players, ");
	}
	strcat(cOut,cTmp);
	sprintf(cTmp,"%d cross-ref limit, ",iXLimit);
	strcat(cOut,cTmp);
	if (iGLimit)
	{
		sprintf(cTmp,"last %d games, ",iGLimit);
	}
	else
	{
		sprintf(cTmp,"all games, ");
	}
	strcat(cOut,cTmp);
	if (gbLogFilter)
	{
		if (eLogFilterType == NDAYS)
		{
			sprintf(cTmp,"last %d days",iLogFilterDays);
		}
		else
		{
			sprintf(cTmp,"%s only",apSpanTypeNames[eLogFilterType]);
		}
	}
	else
	{
		sprintf(cTmp,"all days");
	}
	strcat(cOut,cTmp);
	if (gbRepeat)
	{
		sprintf(cTmp,", repeat ea. %s%d min.",
					apRepTypeNames[eRepType],giRepeatModifier);
		strcat(cOut,cTmp);
	}
	if (gbArchive)
	{
		sprintf(cTmp,", archived");
		strcat(cOut,cTmp);
	}
	strcat(cOut,"'\r\n");
	WRITEFILE(cOut);
#ifdef __linux__
	sprintf(cOut,"metaos='LINUX'\r\n");
#else
	sprintf(cOut,"metaos='Windows %d.%d build %d'\r\n", _winmajor, _winminor, _osver);
#endif
	WRITEFILE(cOut);
	sprintf(cOut,"aretfcstats=");
	WRITEFILE(cOut);
	if (gbIsTFC)
	{
		sprintf(cOut,"true\r\n");
	}
	else
	{
		sprintf(cOut,"false\r\n");
	}
	WRITEFILE(cOut);
	/* close the output file */
	fclose(hOutFile);
	if (pbFiltered) free(pbFiltered);
	return bRes;
} /* END PIG_JSOverallStats */


 /*
 * PIG_JSGameStats
 *
 * Generate JavaScript summary of game statistics
 */

int	PIG_JSGameStats(void)
{
	struct PStat* pPS;
	struct PStat* pPrevPS;
	static char cOut[DEFAULT_STRING_LENGTH];
	static char cGameName[DEFAULT_STRING_LENGTH];
	char* pcStrEnd;
	FILE *hOutFile;
	BOOL bRes = TRUE;
	int *piPStatList;
	int PStatListQty;
	int GCount,PCount,GCountAct;
	struct PStat* pPSPlayer;
	int Index,PlayerIndex;

	strcpy(gcOutPath, gcJPath);
#ifdef __linux__
	strcat(gcOutPath, "/psgame.js");
#else
	strcat(gcOutPath, "\\psgame.js");
#endif
	/* open the output file */
	hOutFile = fopen(gcOutPath,"w");
	if (!hOutFile)
	{
		printf("sorry... file name error\r\n");
		return 0;
	}
	
	/* display a banner */
	WRITEFILE(pPSGameStart);
	
	/* sort by map start time usage */
	PIG_SetupPStatList(&gGStat);
	PIG_SortPStatListByST();
	/* make a local copy of the list */
	PStatListQty = gPStatListQty;
	piPStatList = (int*)calloc(gPStatListQty,sizeof(int));
	if (!piPStatList)
	{
		printf("Allocation failure\n");
		return 0;
	}
	memcpy(piPStatList,gpiPStatList,gPStatListQty*sizeof(int));
	WRITEFILE("g=new(Array)\r\ngp=new(Array)\r\nmw=new(Array)\r\n");
	GCount = 0;
	GCountAct = 0;
	for(GCount=0,Index=PStatListQty-1;Index>=0;Index--)
	{
		pPS = gGStat.pPStat + *(piPStatList+Index);
		if (pPS->OPStat.pPStat)
		{
			PCount = 0;
			/* sort to find best killers */
			PIG_SetupPStatList(&pPS->OPStat);
			PIG_SortPStatListByKandD();
			/* bump the match wins count for the best of the best */
			pPSPlayer = PIG_GetPStat(&gPStat, (pPS->OPStat.pPStat + (*gpiPStatList))->pPName, &PlayerIndex);
			if (!bMatchWinsComputed)
			{
				pPSPlayer->nmw++;
			}
			if (!iGLimit || (GCount < iGLimit))
			{
				sprintf(cOut,"gp[%d]=[",GCount);
				WRITEFILE(cOut);
				for (PlayerIndex=0;PlayerIndex<gPStatListQty;PlayerIndex++)
				{
					pPSPlayer = pPS->OPStat.pPStat + *(gpiPStatList+PlayerIndex);
					sprintf(cOut,"[\"%s\",%d,%d",	pPSPlayer->pPName,
															pPSPlayer->nk-pPSPlayer->ns,
															pPSPlayer->nd);
					WRITEFILE(cOut);
					if (gbIsTFC)
					{
						sprintf(cOut,",%d",pPSPlayer->ncaps);
						WRITEFILE(cOut);
					}
					sprintf(cOut,"],");
					WRITEFILE(cOut);
				}
				WRITEFILE("]\r\n");
				strcpy(cGameName,pPS->pPName);
				/* check for "unknown" maps, and attempt to replace the name */
				if (!memcmp(cGameName,UNKNOWNMAP,UNKNOWNMAP_NAME_LENGTH) &&
					 (Index != 0))
				{
					/* grab the map name from the previous logged map */
					pPrevPS = gGStat.pPStat + *(piPStatList+Index-1);
					/* and only if no players logged in it */
					if (!pPrevPS->OPStat.pPStat)
					{
						/* successful switch.. no one will notice a thing */
						strcpy(cGameName,pPrevPS->pPName);
					}
				}
				pcStrEnd = strchr(cGameName,' ');
				if (pcStrEnd)
				{
					// terminate the string with just the game name present
					*pcStrEnd = 0;
				}
				sprintf(cOut,"g[%d]=[\"%s\",%d,%d]\r\n",GCount,cGameName,pPS->start,gPStatListQty);
				WRITEFILE(cOut);
				GCountAct++;
			}
			GCount++;
		}
	}
	sprintf(cOut,"gq=%d\r\n",GCountAct);
	WRITEFILE(cOut);

	/* sort players by match wins */
	PIG_SetupPStatList(&gPStat);
	PIG_SortPStatListByNMW();
	for (PlayerIndex=0;PlayerIndex<gPStatListQty;PlayerIndex++)
	{
		pPSPlayer = gPStat.pPStat + *(gpiPStatList+PlayerIndex);
		if (!pPSPlayer->nmw) break;
		sprintf(cOut,"mw[%d]=[\"%s\",%d]\r\n",PlayerIndex,pPSPlayer->pPName,pPSPlayer->nmw);
		WRITEFILE(cOut);
	}
	sprintf(cOut,"mwq=%d\r\n",PlayerIndex);
	WRITEFILE(cOut);
	
	fclose(hOutFile);
	free(piPStatList);
	bMatchWinsComputed = TRUE;
	return bRes;
} /* END PIG_JSOverallStats */


