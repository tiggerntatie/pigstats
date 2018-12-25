/****************************************************************************

$Header: pigtext.c  Revision:1.10  Tue Jan 30 19:57:58 2001  ericd $
  
PigText.c

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
$Log: C:\WORK\PigStats\Archive\SourceDist\CSource\pigtext.d $
  
    Split pigstats.c into pigjs.c, pigparse.c, pigtext.c and pigutil.c
  
  Revision 1.10  by: ericd  Rev date: Tue Jan 30 19:57:58 2001
    Changed all pigfile references to lower case
  
  Revision 1.9  by: ericd  Rev date: Sat Oct 28 15:40:12 2000
    First changes for 1.4f client server architecture
  
  Revision 1.8  by: ericd  Rev date: Fri Aug 25 23:20:10 2000
    Full repetitive and archiving implementation, Linux safe, some bugs:
    default rep rate is off
  
  Revision 1.7  by: ericd  Rev date: Thu Jul 27 22:06:30 2000
    1) Workaround for CS dedicated server log splitting
  
  Revision 1.6  by: ericd  Rev date: Wed Jul 19 22:18:16 2000
    1) Modified to use btree lookup when locating player info
  
  Revision 1.5  by: ericd  Rev date: Mon Jul 03 22:47:12 2000
    Eliminated use of regex for sake of speed
  
  Revision 1.4  by: ericd  Rev date: Tue Jun 13 22:00:22 2000
    Rev. 1.4c (Linux port) candidate
  
  Revision 1.3  by: ericd  Rev date: Sun May 07 22:42:56 2000
    Fixed bug that was repeating match wins calculation
  
  Revision 1.2  by: ericd  Rev date: Sun Apr 16 22:12:26 2000
    New web site
  
  Revision 1.1  by: ericd  Rev date: Sat Apr 01 18:57:00 2000
    Removed junk in expanded keywords
  
  Revision 1.0  by: ericd  Rev date: Sat Apr 01 18:44:16 2000
    Initial revision.
  
  $Endlog$
 */

#ifndef __linux__
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <string.h>
#include "pigstats.h"
#include "pigutil.h"


char *pBanner = 
"\r\n\r\n**********************************************************\r\n**** %s\r\n**********************************************************\r\n";


void PIG_StartHTML(FILE *hOutFile, char* pTitle)
{
	size_t sWritten;
	static char* pHTML = "<html>\r\n<head>\r\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\r\n<title>%s</title>\r\n</head>\r\n<body>\r\n";
	static char cHeader[1000];
	if (bIsHTML)
	{
		sprintf(cHeader,pHTML,pTitle);
		sWritten = fprintf(hOutFile,cHeader);
	}
}

void PIG_EndHTML(FILE *hOutFile)
{
	size_t sWritten;
	static char* pHTML = "<p>PigStats Half-Life Statistics Generator is available from <a href=\"http://www.planethalflife.com/pigstats\">BetterDead</a></p></html>\r\n</body>\r\n";
	if (bIsHTML)
	{
		sWritten = fprintf(hOutFile,pHTML);
	}
}

void PIG_PrintBanner(FILE *hOutFile, char* pText)
{
	static char cBuff[700];
	static char cBuff2[700];
	size_t sWritten;
	static char* pHTML = "<p><a name=\"%s\"><font size=\"6\" face=\"Arial, sans-serif\"><b><i>%s</i></b></font></a></p>\r\n";
	static char* pHTMLLinks = "<p><a href=\"%s\"><font size=\"3\" \
face=\"Arial, sans-serif\">GENERAL STATISTICS</font></a><font \
size=\"3\" face=\"Arial, sans-serif\"> / </font><a \
href=\"%s\"><font size=\"3\" face=\"Arial, sans-serif\">WEAPON \
STATISTICS</font></a><font size=\"3\" face=\"Arial, sans-serif\"> / </font><a \
href=\"%s\"><font size=\"3\" face=\"Arial, sans-serif\">GAME \
STATISTICS</font></a></p>\r\n";

	cBuff[0]=0;

	if (bIsHTML)
	{
		sprintf(cBuff,pHTML,pText,pText);
		if (bMux)
		{
			sprintf(cBuff2,pHTMLLinks,gcGeneralRef,gcWeaponRef,gcGameRef);
		}
		else
		{
			sprintf(cBuff2,pHTMLLinks,"#GENERAL STATISTICS","#WEAPON STATISTICS","#GAME STATISTICS");
		}
		strcat(cBuff,cBuff2);
	}
	else
	{
		sprintf(cBuff,pBanner,pText);
	}
	sWritten = fprintf(hOutFile,cBuff);
}


int giWidth;
int giLeft;
int giRight;
int giMoreRight;
void PIG_TableStart(FILE *hOutFile, int Width, int LeftSize, int RightSize, int Columns, char* pHeaderText)
{
	static char cBuff[512];
	size_t sWritten;
	static char* pHTML = "<table border=\"2\" cellpadding=\"3\" cellspacing=\"0\" width=\"%d\" bgcolor=\"#FFFFFF\" bordercolor=\"#000000\" bordercolordark=\"#000000\" bordercolorlight=\"#000000\">\r\n<tr>\r\n<td colspan=\"%d\" bgcolor=\"#0000A0\"><font color=\"#FFFFFF\" face=\"Arial\"><strong>%s</strong></font>\r\n</td>\r\n</tr>\r\n";

	cBuff[0]=0;

	giWidth = Width;
	giLeft = LeftSize;
	giRight = RightSize;
	giMoreRight = Width-(LeftSize+RightSize);

	if (bIsHTML)
	{
		sprintf(cBuff,pHTML,Width,Columns,pHeaderText);
	}
	else
	{
		sprintf(cBuff, "\r\n%s\r\n", pHeaderText);
	}
	sWritten = fprintf(hOutFile,cBuff);
}

void PIG_TableEnd(FILE *hOutFile)
{
	size_t sWritten;
	static char* pHTML = "</table>\r\n<p><br></p>\r\n";

	if (bIsHTML)
	{
		sWritten = fprintf(hOutFile,pHTML);
	}
}

void PIG_TableLine(FILE *hOutFile, BOOL bXRef, char* pLeft, char* pRight, char* pMoreRight)
{
	static char cBuffl[512];
	static char cBuffr[512];
	static char cBuffTempXref[256];
	static char cBuffTempName[256];
	static char* pHTML = "<tr>\r\n<td width=\"%d\" bgcolor=\"#818181\"><font color=\"#FFFF00\" face=\"Arial\">%s</font>\r\n</td>\r\n<td width=\"%d\">\r\n<font face=\"Arial\">%s</font>\r\n</td>\r\n";
	static char* pHTMLXref = "<tr>\r\n<td width=\"%d\" bgcolor=\"#818181\"><a href=\"%s\"><font color=\"#FFFF00\" face=\"Arial\">%s</font></a>\r\n</td>\r\n<td width=\"%d\">\r\n<font face=\"Arial\">%s</font>\r\n</td>\r\n";
	static char* pHTMLEnd = "</tr>\r\n";
	static char* pHTMLMore = "<td width=\"%d\">\r\n<font face=\"Arial\">%s</font>\r\n</td>\r\n";

	size_t sWritten;
	cBuffl[0]=0;
	cBuffr[0]=0;

	if (bIsHTML)
	{
		if (bMux)
		{
			strcpy(cBuffTempXref,"./");
			PIG_MakeStringValidFileName(pLeft,cBuffTempName);
			strcat(cBuffTempXref,gcRootName);
			strcat(cBuffTempXref,"-");
			strcat(cBuffTempXref,cBuffTempName);
			strcat(cBuffTempXref,gcExtPath);
		}
		else
		{
			strcpy(cBuffTempXref,"#");
			strcat(cBuffTempXref,pLeft);
		}
		if (!pMoreRight)
		{
			if (bXRef)
			{
				sprintf(cBuffl,pHTMLXref,giLeft,cBuffTempXref,pLeft,giRight,pRight);
			}
			else
			{
				sprintf(cBuffl,pHTML,giLeft,pLeft,giRight,pRight);
			}
			sprintf(cBuffr,pHTMLEnd);
		}
		else
		{
			if (bXRef)
			{
				sprintf(cBuffl,pHTMLXref,giLeft,cBuffTempXref,pLeft,giRight,pRight);
			}
			else
			{
				sprintf(cBuffl,pHTML,giLeft,pLeft,giRight,pRight);
			}
			sprintf(cBuffr,pHTMLMore,giMoreRight,pMoreRight);
			strcat(cBuffr,pHTMLEnd);
		}
	}
	else
	{
		if (!pMoreRight)
		{
			sprintf(cBuffl,"%-20s %s\r\n",pLeft,pRight);
		}
		else
		{
			sprintf(cBuffl,"%-20s %s%s\r\n",pLeft,pRight,pMoreRight);
		}
	}
	sWritten = fprintf(hOutFile,cBuffl);
	sWritten = fprintf(hOutFile,cBuffr);
}

void	PIG_ShowOverallStats(FILE *hOutFile)
{
	double fTemp;
	int Index;
	struct PStat* pPS;
	char cNum[DEFAULT_STRING_LENGTH];
	/* sort by kills */
	PIG_SetupPStatList(&gPStat);
	PIG_SortPStatListByK();
	PIG_PrintBanner(hOutFile, "GENERAL STATISTICS");
	PIG_TableStart(hOutFile,500,200,300,2,"Sorted by Kills:");
	for(Index=0;Index<gPStatListQty;Index++)
	{
		pPS = gPStat.pPStat + *(gpiPStatList+Index);
		if (!bFilter || pPS->nk)
		{
			sprintf(cNum,"%d",pPS->nk);
			PIG_TableLine(hOutFile,TRUE,pPS->pPName,cNum,NULL);
		}
	}
	PIG_TableEnd(hOutFile);

	PIG_SortPStatListByD();
	PIG_TableStart(hOutFile,500,200,300,2,"Sorted by Deaths:");
	for(Index=0;Index<gPStatListQty;Index++)
	{
		pPS = gPStat.pPStat + *(gpiPStatList+Index);
		if (!bFilter || pPS->nd)
		{
			sprintf(cNum,"%d",pPS->nd);
			PIG_TableLine(hOutFile,TRUE,pPS->pPName,cNum,NULL);
		}
	}
	PIG_TableEnd(hOutFile);

	PIG_SortPStatListByS();
	PIG_TableStart(hOutFile,500,200,300,2,"Sorted by Suicides:");
	for(Index=0;Index<gPStatListQty;Index++)
	{
		pPS = gPStat.pPStat + *(gpiPStatList+Index);
		if (!bFilter || pPS->ns)
		{
			sprintf(cNum,"%d",pPS->ns);
			PIG_TableLine(hOutFile,TRUE,pPS->pPName,cNum,NULL);
		}
	}
	PIG_TableEnd(hOutFile);

	PIG_SortPStatListByR();
	PIG_TableStart(hOutFile,500,200,300,2,"Sorted by Ratio:");
	for(Index=0;Index<gPStatListQty;Index++)
	{
		pPS = gPStat.pPStat + *(gpiPStatList+Index);
		if (pPS->nd)
		{
			fTemp = 1.0*pPS->nk/pPS->nd;
			if (!bFilter || (fTemp != 0.0))
			{
				sprintf(cNum,"%f",fTemp);
				PIG_TableLine(hOutFile,TRUE,pPS->pPName,cNum,NULL);
			}
		}
		else
		{
			PIG_TableLine(hOutFile,TRUE,pPS->pPName,"N/A",NULL);
		}
	}
	PIG_TableEnd(hOutFile);

	PIG_SortPStatListByKR();
	PIG_TableStart(hOutFile,500,200,300,2,"Sorted by Kill Rate (while playing):");
	for(Index=0;Index<gPStatListQty;Index++)
	{
		pPS = gPStat.pPStat + *(gpiPStatList+Index);
		if (pPS->duration)
		{
			fTemp = 60.0*pPS->nk/pPS->duration;
			if (!bFilter || (fTemp != 0.0))
			{
				sprintf(cNum,"%f Kills/Min",fTemp);
				PIG_TableLine(hOutFile,TRUE,pPS->pPName,cNum,NULL);
			}
		}
		else
		{
			PIG_TableLine(hOutFile,TRUE,pPS->pPName,"N/A",NULL);
		}
	}
	PIG_TableEnd(hOutFile);
} /* END PIG_ShowOverallStats */



void PIG_ShowPlayerStats(FILE *hOutFileArg)
{
	double fTemp;
	int Index;
	struct PStat* pPS;
	struct PStat* pPSX;
	char* pName;
	static char cTemp[256];
	static char cNewName[256];
	FILE *hOutFile = hOutFileArg;

	pPSX = gPStat.pPStat;
	while (pPSX->pPName)
	{
		/* multiple output files? */
		if (bMux)
		{
			strcpy(gcOutPath, gcRootPath);
			strcat(gcOutPath, "-");
			pName = pPSX->pPName;
			PIG_MakeStringValidFileName(pName,cNewName);
			strcat(gcOutPath, cNewName);
			strcat(gcOutPath, gcExtPath);
			/* open the output file */
			hOutFile = fopen(gcOutPath,"w");
			if (!hOutFile)
			{
				printf("sorry... file name error\n\n");
				return ;
			}
			PIG_StartHTML(hOutFile,"PigStats Player Statistics");
		}
		pName = pPSX->pPName;
		PIG_PrintBanner(hOutFile,pName);
		sprintf(cTemp,"General Statistics for %s",pName);
		PIG_TableStart(hOutFile,500,200,300,2,cTemp);
		sprintf(cTemp,"%d",pPSX->nk);
		PIG_TableLine(hOutFile,FALSE,"Total kills",cTemp,NULL);
		sprintf(cTemp,"%d",pPSX->nd);
		PIG_TableLine(hOutFile,FALSE,"Total deaths",cTemp,NULL);
		sprintf(cTemp,"%d",pPSX->ns);
		PIG_TableLine(hOutFile,FALSE,"Total suicides",cTemp,NULL);
		if (pPSX->nd)
		{
			sprintf(cTemp,"%f",1.0*pPSX->nk/pPSX->nd);
			PIG_TableLine(hOutFile,FALSE,"Overall kill/death",cTemp,NULL);
		}
		else
		{
			PIG_TableLine(hOutFile,FALSE,"Overall kill/death","N/A",NULL);
		}
		if (pPSX->duration)
		{
			sprintf(cTemp,"%f",60.0*pPSX->nk/pPSX->duration);
			PIG_TableLine(hOutFile,FALSE,"Kill rate (K/Min)",cTemp,NULL);
		}
		else
		{
			PIG_TableLine(hOutFile,FALSE,"Kill rate (K/Min)","N/A",NULL);
		}
		sprintf(cTemp,"%f Min",pPSX->duration/60.0);
		PIG_TableLine(hOutFile,FALSE,"Total play time",cTemp,NULL);
		PIG_TableEnd(hOutFile);

		pPS = pPSX->OPStat.pPStat;
		sprintf(cTemp,"%s most often killed",pName);
		PIG_TableStart(hOutFile,500,200,300,2,cTemp);
		/* sort by kills */
		PIG_SetupPStatList(&pPSX->OPStat);
		PIG_SortPStatListByK();
		for(Index=0;Index<gPStatListQty;Index++)
		{
			pPS = pPSX->OPStat.pPStat + *(gpiPStatList+Index);
			sprintf(cTemp,"%d",pPS->nk);
			PIG_TableLine(hOutFile,TRUE,pPS->pPName,cTemp,NULL);
		}
		PIG_TableEnd(hOutFile);


		PIG_SortPStatListByD();
		sprintf(cTemp,"%s most often killed by",pName);
		PIG_TableStart(hOutFile,500,200,300,2,cTemp);
		for(Index=gPStatListQty;Index;Index--)
		{
			pPS = pPSX->OPStat.pPStat + *(gpiPStatList+Index-1);
			sprintf(cTemp,"%d",pPS->nd);
			PIG_TableLine(hOutFile,TRUE,pPS->pPName,cTemp,NULL);
		}
		PIG_TableEnd(hOutFile);

		PIG_SortPStatListByR();
		sprintf(cTemp,"%s kill/death ratio against",pName);
		PIG_TableStart(hOutFile,500,200,300,2,cTemp);
		for(Index=0;Index<gPStatListQty;Index++)
		{
			pPS = pPSX->OPStat.pPStat + *(gpiPStatList+Index);
			if (pPS->nd)
			{
				fTemp = 1.0*pPS->nk/pPS->nd;
				sprintf(cTemp,"%f",fTemp);
				PIG_TableLine(hOutFile,TRUE,pPS->pPName,cTemp,NULL);
			}
			else
			{
				PIG_TableLine(hOutFile,TRUE,pPS->pPName,"N/A",NULL);
			}
		}
		PIG_TableEnd(hOutFile);

		/* sort by weapon kills */
		pPS = pPSX->WStat.pPStat;
		PIG_SetupPStatList(&pPSX->WStat);
		PIG_SortPStatListByK();
		sprintf(cTemp,"%s most often kills with",pName);
		PIG_TableStart(hOutFile,500,200,300,2,cTemp);
		for(Index=0;Index<gPStatListQty;Index++)
		{
			pPS = pPSX->WStat.pPStat + *(gpiPStatList+Index);
			if (pPS->nk)
			{
				sprintf(cTemp,"%d",pPS->nk);
				PIG_TableLine(hOutFile,FALSE,pPS->pPName,cTemp,NULL);
			}
		}
		PIG_TableEnd(hOutFile);

		PIG_SortPStatListByD();
		sprintf(cTemp,"%s most often killed by",pName);
		PIG_TableStart(hOutFile,500,200,300,2,cTemp);
		for(Index=gPStatListQty;Index;Index--)
		{
			pPS = pPSX->WStat.pPStat + *(gpiPStatList+Index-1);
			if (pPS->nd)
			{
				sprintf(cTemp,"%d",pPS->nd);
				PIG_TableLine(hOutFile,FALSE,pPS->pPName,cTemp,NULL);
			}
		}
		PIG_TableEnd(hOutFile);
		if (bMux)
		{
			PIG_EndHTML(hOutFile);
			fclose(hOutFile);
		}
		/* next player */
		pPSX++;
	}
}


void	PIG_ShowWeaponStats(FILE *hOutFileArg)
{
	int	PStatListQty;
	int*  piPStatList;
	int Index;
	struct PStat* pPS;
	struct PStat* pPSFav;
	static char cTemp[256];
	static char cTemp1[256];
	FILE *hOutFile = hOutFileArg;

	/* sort by kill usage */
	PIG_SetupPStatList(&gWStat);
	PIG_SortPStatListByK();
	/* make a local copy of the list */
	PStatListQty = gPStatListQty;
	piPStatList = (int*)calloc(gPStatListQty,sizeof(int));
	if (!piPStatList)
	{
		printf("Allocation failure\n");
		return;
	}
	memcpy(piPStatList,gpiPStatList,gPStatListQty*sizeof(int));
	/* multiple output files? */
	if (bMux)
	{
		strcpy(gcOutPath, gcRootPath);
		strcat(gcOutPath, WEAPONEXTENSION);
		strcat(gcOutPath, gcExtPath);
		/* open the output file */
		hOutFile = fopen(gcOutPath,"w");
		if (!hOutFile)
		{
			printf("sorry... file name error\n\n");
			return ;
		}
		PIG_StartHTML(hOutFile,"PigStats Weapon Statistics");
	}
	PIG_PrintBanner(hOutFile,"WEAPON STATISTICS");
	sprintf(cTemp,"Sorted by Kills");
	PIG_TableStart(hOutFile,500,150,100,3,cTemp);
	for(Index=0;Index<PStatListQty;Index++)
	{
		pPS = gWStat.pPStat + *(piPStatList+Index);
		if (pPS->nk)
		{
			/* sort to find most common killer with this weapon */
			PIG_SetupPStatList(&pPS->OPStat);
			PIG_SortPStatListByK();
			pPSFav = pPS->OPStat.pPStat + *(gpiPStatList);
			sprintf(cTemp,"%-6d ",pPS->nk);
			sprintf(cTemp1,"(%d by %s)",pPSFav->nk,pPSFav->pPName);
			PIG_TableLine(hOutFile,FALSE,pPS->pPName,cTemp,cTemp1);
		}
	}
	PIG_TableEnd(hOutFile);
	if (bMux)
	{
		PIG_EndHTML;
		fclose(hOutFile);
	}
	free(piPStatList);
}




void	PIG_ShowGameStats(FILE *hOutFileArg)
{
	int	PStatListQty;
	int* piPStatList;
	int Index,PlayerIndex;
	struct PStat* pPS;
	struct PStat* pPrevPS;
	struct PStat* pPSPlayer;
	static char cTemp[256];
	static char cTemp1[256];
	FILE *hOutFile = hOutFileArg;
	char* pMapName;

	/* sort by map start time usage */
	PIG_SetupPStatList(&gGStat);
	PIG_SortPStatListByST();
	/* make a local copy of the list */
	PStatListQty = gPStatListQty;
	piPStatList = (int*)calloc(gPStatListQty,sizeof(int));
	if (!piPStatList)
	{
		printf("Allocation failure\n");
		return;
	}
	/* multiple output files? */
	if (bMux)
	{
		strcpy(gcOutPath, gcRootPath);
		strcat(gcOutPath, GAMEEXTENSION);
		strcat(gcOutPath, gcExtPath);
		/* open the output file */
		hOutFile = fopen(gcOutPath,"w");
		if (!hOutFile)
		{
			printf("sorry... file name error\n\n");
			return ;
		}
		PIG_StartHTML(hOutFile,"PigStats Game Statistics");
	}
	memcpy(piPStatList,gpiPStatList,gPStatListQty*sizeof(int));
	PIG_PrintBanner(hOutFile,"GAME STATISTICS");
	for(Index=0;Index<PStatListQty;Index++)
	{
		pPS = gGStat.pPStat + *(piPStatList+Index);
		if (pPS->OPStat.pPStat)
		{
			pMapName = pPS->pPName;
			/* check for "unknown" maps, and attempt to replace the name */
			if (!memcmp(pPS->pPName,UNKNOWNMAP,UNKNOWNMAP_NAME_LENGTH) &&
				 (Index != 0))
			{
				/* grab the map name from the previous logged map */
				pPrevPS = gGStat.pPStat + *(piPStatList+Index-1);
				/* and only if no players logged in it */
				if (!pPrevPS->OPStat.pPStat)
				{
					/* successful switch.. no one will notice a thing */
					pMapName = pPrevPS->pPName;
				}
			}

			PIG_TableStart(hOutFile,500,200,150,3,pMapName);
			/* sort to find best killers */
			PIG_SetupPStatList(&pPS->OPStat);
			PIG_SortPStatListByKandD();
			/* bump the match wins count for the best of the best */
			pPSPlayer = PIG_GetPStat(&gPStat, (pPS->OPStat.pPStat+ (*gpiPStatList))->pPName, &PlayerIndex);
			if (!bMatchWinsComputed)
			{
				pPSPlayer->nmw++;
			}
			for (PlayerIndex=0;PlayerIndex<gPStatListQty;PlayerIndex++)
			{
				pPSPlayer = pPS->OPStat.pPStat + *(gpiPStatList+PlayerIndex);
				sprintf(cTemp,"%-6d ",pPSPlayer->nk-pPSPlayer->ns);
				sprintf(cTemp1,"%-6d",pPSPlayer->nd);
				PIG_TableLine(hOutFile,TRUE,pPSPlayer->pPName,cTemp,cTemp1);
			}
			PIG_TableEnd(hOutFile);
		}
	}
	/* sort players by match wins */
	PIG_TableStart(hOutFile,500,200,300,2,"Match Totals");
	PIG_SetupPStatList(&gPStat);
	PIG_SortPStatListByNMW();
	for (PlayerIndex=0;PlayerIndex<gPStatListQty;PlayerIndex++)
	{
		pPSPlayer = gPStat.pPStat + *(gpiPStatList+PlayerIndex);
		sprintf(cTemp,"%-6d",pPSPlayer->nmw);
		PIG_TableLine(hOutFile,TRUE,pPSPlayer->pPName,cTemp,NULL);
	}
	PIG_TableEnd(hOutFile);
	if (bMux)
	{
		PIG_EndHTML(hOutFile);
		fclose(hOutFile);
	}
	free(piPStatList);
	bMatchWinsComputed = TRUE;
}



void	PIG_ShowTeamGameStats(FILE *hOutFileArg)
{
	int	PStatListQty;
	int PTeamListQty;
	int* piPStatList;
	int* piPTeamList;
	int Index,PlayerIndex,TeamIndex,TempIndex;
	struct PStat* pPS;
	struct PStat* pPSTeam;
	struct PStat* pPSPlayer;
	struct PStat* pPSMasterPlayer;
	struct PStat* pPSTempPlayer;
	static char cTemp[256];
	static char cTemp1[256];
	static char cTemp2[256];
	FILE *hOutFile = hOutFileArg;
	
	/* sort by map start time usage */
	PIG_SetupPStatList(&gGStat);
	PIG_SortPStatListByST();
	/* make a local copy of the list */
	PStatListQty = gPStatListQty;
	piPStatList = (int*)calloc(gPStatListQty,sizeof(int));
	if (!piPStatList)
	{
		printf("Allocation failure\n");
		return;
	}
	/* multiple output files? */
	if (bMux)
	{
		strcpy(gcOutPath, gcRootPath);
		strcat(gcOutPath, GAMEEXTENSION);
		strcat(gcOutPath, gcExtPath);
		/* open the output file */
		hOutFile = fopen(gcOutPath,"w");
		if (!hOutFile)
		{
			printf("sorry... file name error\n\n");
			return ;
		}
		PIG_StartHTML(hOutFile,"PigStats Game Statistics");
	}
	memcpy(piPStatList,gpiPStatList,gPStatListQty*sizeof(int));
	PIG_PrintBanner(hOutFile,"GAME STATISTICS");
	for(Index=0;Index<PStatListQty;Index++)
	{
		pPS = gGStat.pPStat + *(piPStatList+Index);
		PIG_TableStart(hOutFile,500,200,150,3,pPS->pPName);
		if (pPS->OPStat.pPStat)
		{
			/* assign team ids to the players for the game */
			pPSPlayer = pPS->OPStat.pPStat;
			while (pPSPlayer && pPSPlayer->pPName)
			{
				pPSMasterPlayer = PIG_GetPStat(&gPStat, pPSPlayer->pPName, &PlayerIndex);
				pPSPlayer->team = pPSMasterPlayer->team;
				/* figure out which team name it is */
				pPSTeam = gTStat.pPStat;
				while (pPSTeam && pPSTeam->pPName)
				{
					if (pPSTeam->team == pPSPlayer->team)
					{
						/* identified a team involved in this game, add it to the game */
						pPSTeam = PIG_GetPStat(&pPS->TStat, pPSTeam->pPName, &TeamIndex);
						/* update team stats for this game */
						pPSTeam->nk += pPSPlayer->nk;
						pPSTeam->nd += pPSPlayer->nd;
						pPSTeam->ns += pPSPlayer->ns;
						/* and add the player to the team! */
						pPSTempPlayer = PIG_GetPStat(&pPSTeam->OPStat, pPSPlayer->pPName, &TempIndex);
						pPSTempPlayer->nk = pPSPlayer->nk;
						pPSTempPlayer->nd = pPSPlayer->nd;
						pPSTempPlayer->ns = pPSPlayer->ns;
						break;
					}
					pPSTeam++;
				}
				pPSPlayer++;
			}

			/* sort to find best teams for this game */
			PIG_SetupPStatList(&pPS->TStat);
			PIG_SortPStatListByKandD();
			/* make a local copy of the list */
			PTeamListQty = gPStatListQty;
			piPTeamList = (int*)calloc(gPStatListQty,sizeof(int));
			if (!piPTeamList)
			{
				free (piPStatList);
				printf("Allocation failure\n");
				return;
			}
			memcpy(piPTeamList,gpiPStatList,gPStatListQty*sizeof(int));
			/* bump the match wins count for the best of the best */
			pPSTeam = PIG_GetPStat(&gTStat, (pPS->TStat.pPStat + (*piPTeamList))->pPName, &TeamIndex);
			pPSTeam->nmw++;
			/* for each team in the game */
			for (TeamIndex=0;TeamIndex<PTeamListQty;TeamIndex++)
			{
				/* sort that team's players */
				pPSTeam = pPS->TStat.pPStat + *(piPTeamList+TeamIndex);
				sprintf(cTemp,"%-6d ",pPSTeam->nk-pPSTeam->ns);
				sprintf(cTemp2,"%-6d",pPSTeam->nd);
				PIG_TableLine(hOutFile,FALSE,pPSTeam->pPName,cTemp,cTemp2);
				PIG_SetupPStatList(&pPSTeam->OPStat);
				PIG_SortPStatListByKandD();
				for (PlayerIndex=0;PlayerIndex<gPStatListQty;PlayerIndex++)
				{
					pPSPlayer = pPSTeam->OPStat.pPStat + *(gpiPStatList+PlayerIndex);
					sprintf(cTemp,"%-6d ",pPSPlayer->nk-pPSPlayer->ns);
					sprintf(cTemp2,"%-6d",pPSPlayer->nd);
					sprintf(cTemp1,"..%-18s",pPSPlayer->pPName);
					PIG_TableLine(hOutFile,FALSE,cTemp1,cTemp,cTemp2);
				}
			}
		}
		PIG_TableEnd(hOutFile);
	}
	/* sort by teams by match wins */
	PIG_TableStart(hOutFile,500,200,300,2,"Match Totals");
	PIG_SetupPStatList(&gTStat);
	PIG_SortPStatListByNMW();
	for (TeamIndex=0;TeamIndex<gPStatListQty;TeamIndex++)
	{
		pPSTeam = gTStat.pPStat + *(gpiPStatList+TeamIndex);
		sprintf(cTemp,"%-6d",pPSTeam->nmw);
		PIG_TableLine(hOutFile,FALSE,pPSTeam->pPName,cTemp,NULL);
	}
	PIG_TableEnd(hOutFile);
	if (bMux)
	{
		PIG_EndHTML(hOutFile);
		fclose(hOutFile);
	}
	free(piPStatList);
	free(piPTeamList);
}

