/****************************************************************************

$Header: pigutil.c  Revision:1.16  Wed Sep 05 20:41:48 2001  ericd $
  
PigUtil.c

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
$Log: C:\WORK\PigStats\Archive\SourceDist\CSource\pigutil.d $
  
    Split pigstats.c into pigjs.c, pigparse.c, pigtext.c and pigutil.c
  
  Revision 1.16  by: ericd  Rev date: Wed Sep 05 20:41:48 2001
    T-score and L-scores inaccurate when doing archived or multiple runs.. 
    
  
  Revision 1.15  by: ericd  Rev date: Wed Aug 01 21:49:46 2001
    Initial release for custom TFC support for LP Boss (1.7a)
  
  Revision 1.14  by: ericd  Rev date: Sun Mar 25 22:29:34 2001
    rule.pig name exclusion/inclusion.. debugging related to new logging
    standard.
  
  Revision 1.13  by: ericd  Rev date: Thu Mar 22 22:16:26 2001
    Multiple server threads, support for arbitrarily large file transfers,
    bad html character substitution, beginning rule.pig name exclusion
    code.
  
  Revision 1.12  by: ericd  Rev date: Tue Jan 30 19:57:58 2001
    Changed all pigfile references to lower case
  
  Revision 1.11  by: ericd  Rev date: Sat Nov 11 11:14:50 2000
    Update to forwarding protocol to include an Acknowledge message.
    Unacknowledged files are resent.
  
  Revision 1.10  by: ericd  Rev date: Sat Oct 28 15:40:12 2000
    First changes for 1.4f client server architecture
  
  Revision 1.9  by: ericd  Rev date: Fri Aug 25 23:20:10 2000
    Full repetitive and archiving implementation, Linux safe, some bugs:
    default rep rate is off
  
  Revision 1.8  by: ericd  Rev date: Wed Aug 23 20:59:22 2000
    State archiving almost full implementation
  
  Revision 1.7  by: ericd  Rev date: Tue Aug 22 21:59:32 2000
    1) Not using memlib any more 
    2) Repetitive operation fully implemented (lightly tested) 
    3) Rep operation optimized to not repetitively parse logs 
    4) Initial work on stats archiving
  
  Revision 1.6  by: ericd  Rev date: Sun Aug 20 22:46:52 2000
    Initial support for GNU AVL Trees (balanced b-trees)
  
  Revision 1.5  by: ericd  Rev date: Sat Aug 19 09:34:28 2000
    1) Fixed memory leaks for repetitive operation 
    2) Changes for log file deletion 
    3) Initial implementation of repetitive operation
  
  Revision 1.4  by: ericd  Rev date: Wed Jul 19 22:18:16 2000
    1) Modified to use btree lookup when locating player info
  
  Revision 1.3  by: ericd  Rev date: Mon Jul 03 22:47:12 2000
    Eliminated use of regex for sake of speed
  
  Revision 1.2  by: ericd  Rev date: Tue Jun 13 22:00:22 2000
    Rev. 1.4c (Linux port) candidate
  
  Revision 1.1  by: ericd  Rev date: Sat Apr 01 18:57:00 2000
    Removed junk in expanded keywords
  
  Revision 1.0  by: ericd  Rev date: Sat Apr 01 18:44:16 2000
    Initial revision.
  
  $Endlog$
 */

#ifndef __linux__
#include <windows.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "pigstats.h"


/* archive defines */
#define ARCHIVEHEADERTEXT "PigStats Archive File"
#define ARCHIVEHEADERLEN (21)

#ifndef BYTE
#define BYTE unsigned char
#endif


/* archive protos */
static void PIG_ArchivePDB(struct PDB*, FILE* );
static void PIG_RecoverPDB(struct PDB*, FILE*);

/*
 * compute a "L-Score" or skill for an individual player
 *
 * Original Excel Formula by Jed Wallace 7/18/01 (thanks!):
 * 
 
=sqrt(IF(c=0,IF(w=0,(((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25))),MAX(((
(k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25))),(((k/(d-s))*(k/m)*10)+((((k/
(d-s))+(k/m))/2*25)))+(50-((m/w)/(k/(d-s)))))),MAX(IF(w=0,(((k/(d-s))*(k/m)*
10)+((((k/(d-s))+(k/m))/2*25))),MAX((((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m)
)/2*25))),(((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25)))+(50-((m/w)/(k/(d
-s))))))+(100-m/c*5),IF(w=0,(((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25))
),MAX((((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25))),(((k/(d-s))*(k/m)*10
)+((((k/(d-s))+(k/m))/2*25)))+(50-((m/w)/(k/(d-s))))))))/2)*100

 * NOTE: all IF conditionals are of the form x=0.  If this changes, the IFEX
 * macro below must be rewritten!
 *
 */

#define MAXEX(a,b) (((a)>(b))?(a):(b))
#define IFEX(a,b,c) (!(a)?(b):(c))

void PIG_ComputeLScore(struct PStat* pS)
{
	double c = (double)pS->ncaps;
	double w = (double)pS->nmw;
	double k = (double)pS->nk;
	double d = (double)pS->nd;
	double s = (double)pS->ns;
	double m = (double)pS->duration/60;
	
	if ((m == 0) || (d == s)) 
	{
		pS->skill = 0;
	}
	else
	{
		pS->skill = (int) 10*
sqrt(IFEX(c,IFEX(w,(((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25))),MAXEX((((k/(d-
s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25))),(((k/(d-s))*(k/m)*10)+((((k/(d-s))
+(k/m))/2*25)))+(50-((m/w)/(k/(d-s)))))),MAXEX(IFEX(w,(((k/(d-s))*(k/m)*10)+((
((k/(d-s))+(k/m))/2*25))),MAXEX((((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25
))),(((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25)))+(50-((m/w)/(k/(d-s))))
))+(100-m/c*5),IFEX(w,(((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25))),MAXEX(
(((k/(d-s))*(k/m)*10)+((((k/(d-s))+(k/m))/2*25))),(((k/(d-s))*(k/m)*10)+((((
k/(d-s))+(k/m))/2*25)))+(50-((m/w)/(k/(d-s))))))))/2.0)*100
;
	}
}

/* 
 * Morph a text string until it has only valid filename chars 
 * At this point we're going to have a name with only a-z, A-z and 0-9 
 */
char* PIG_MakeStringValidFileName(char* pNameTemp, char* pDestName)
{
	int Source = 0;
	int Dest = 0;
	char c;
	int c1,c2;

	while (*(pNameTemp+Source))
	{
		c = *(pNameTemp+Source++);
		if (!isalnum(c))
		{
			c1 = c/16;
			if (c1 < 10) {c1 += '0';} else {c1 += 'A'-10;}
			c2 = c & 0x0f;
			if (c2 < 10) {c2 += '0';} else {c2 += 'A'-10;}
			*(pDestName+Dest++) = c1;
			*(pDestName+Dest++) = c2;
		}
		else
		{
			*(pDestName+Dest++) = c;
		}
	}
	*(pDestName+Dest) = 0;
	return pDestName;
}


/* initialize the pstat pointer list */
void PIG_SetupPStatList(struct PDB *pPDB)
{
	int Index;
	if (pPDB->pPStat)
	{
		/* note which list we're working on */
		gpWorkingPStat = pPDB->pPStat;
		/* resize gpiPStatList if necessary */
		if (pPDB->iCount > gPStatListSize)
		{
			gPStatListSize = pPDB->iCount;
			gpiPStatList = (int*)
				realloc(gpiPStatList,gPStatListSize*sizeof(int));
		}
		if (gpiPStatList)
		{
			gPStatListQty = pPDB->iCount;
			for (Index=0; Index<pPDB->iCount; Index++)
			{
				*(gpiPStatList+Index) = Index;
			}
		}
	}
	else
	{
		/* no list */
		gPStatListQty = 0;
	}
	if (!gpiPStatList)
	{
		printf("Allocation failure\n");
		gPStatListQty = 0;
	}
}

/* sort a pstat pointer list by Number of Match Wins */
int PIG_NMWComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return p2->nmw - p1->nmw;
}
void PIG_SortPStatListByNMW(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_NMWComp);
}

/* sort a pstat pointer list by Kills */
int PIG_KComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return p2->nk - p1->nk;
}
void PIG_SortPStatListByK(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_KComp);
}

/* sort a pstat pointer list by Kills and Deaths*/
int PIG_KandDComp( const void *arg1, const void *arg2 )
{
	int ret;
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	/* sort first by kills, */
	ret = (p2->nk-p2->ns) - (p1->nk-p1->ns);
	if (!ret)
	{
		/* then by deaths */
		ret = p1->nd - p2->nd;
	}
	return ret;
}
void PIG_SortPStatListByKandD(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_KandDComp);
}

/* sort a pstat pointer list by deaths */
int PIG_DComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return p1->nd - p2->nd;
}
void PIG_SortPStatListByD(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_DComp);
}

/* sort a pstat pointer list by start time */
int PIG_STComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return p1->start - p2->start;
}
void PIG_SortPStatListByST(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_STComp);
}

/* sort a pstat pointer list by suicides */
int PIG_SComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return p1->ns - p2->ns;
}
void PIG_SortPStatListByS(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_SComp);
}

/* sort a pstat pointer list by kill rate */
int PIG_KRComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	if (!p1->duration)
	{
		return -1;
	}
	if (!p2->duration)
	{
		return 1;
	}
	return (1.0*p2->nk/p2->duration) - (1.0*p1->nk/p1->duration) < 0.0 ? -1 : 1;
}
void PIG_SortPStatListByKR(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_KRComp);
}

/* sort a pstat pointer list by ratio */
int PIG_RComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	if (!p1->nd)
	{
		return -1;
	}
	if (!p2->nd)
	{
		return 1;
	}
	return ((1.0*p2->nk/p2->nd) - (1.0*p1->nk/p1->nd)) < 0 ? -1 : 1;
}

void PIG_SortPStatListByR(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_RComp);
}

/* sort a pstat pointer list by duration */
int PIG_TComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return (p2->duration - p1->duration) < 0 ? -1 : 1;
}

void PIG_SortPStatListByT(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_TComp);
}

/* sort a pstat pointer list by caps (tfc) */
int PIG_CComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return (p2->ncaps - p1->ncaps) < 0 ? -1 : 1;
}
void PIG_SortPStatListByC(void)
{
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_CComp);
}


/* sort a pstat pointer list by skill (tfc) */
int PIG_SKComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return (p2->skill - p1->skill) < 0 ? -1 : 1;
}
void PIG_SortPStatListBySK(void)
{
	// recompute all the L-Scores
	int Index;
	for (Index=0;Index<gPStatListQty;Index++)
	{
		PIG_ComputeLScore(gpWorkingPStat+Index);
	}
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_SKComp);
}

/* sort a pstat pointer list by T-score (tfc) */
int PIG_TSComp( const void *arg1, const void *arg2 )
{
	struct PStat* p1 = gpWorkingPStat + *((int*)arg1);
	struct PStat* p2 = gpWorkingPStat + *((int*)arg2);
	return (p2->score - p1->score) < 0 ? -1 : 1;
}

void PIG_SortPStatListByTS(void)
{
	// recompute all the T-Scores
	int Index;
	struct PStat* p;
	for (Index=0;Index<gPStatListQty;Index++)
	{
		p = gpWorkingPStat+Index;
		p->score = p->nk + 10*p->ncaps;
	}
	qsort(gpiPStatList,gPStatListQty,sizeof(int),PIG_TSComp);
}



/* clean up allocated memory */
void	PIG_CleanPStat(struct PDB *pPDB)
{
	int Index;
	struct PStat* pPS;

	for (Index=0;pPDB && pPDB->pPStat && (pPDB->pPStat+Index)->pPName;Index++)
	{
		pPS = pPDB->pPStat+Index;
		free(pPS->pPName);
		/* free any child pStats */
		if (pPS->OPStat.pPStat)
		{
			PIG_CleanPStat(&pPS->OPStat);
		}
		if (pPS->WStat.pPStat)
		{
			PIG_CleanPStat(&pPS->WStat);
		}
		if (pPS->TStat.pPStat)
		{
			PIG_CleanPStat(&pPS->TStat);
		}
	}
	if (pPDB->pTree)
	{
		avl_destroy (pPDB->pTree, NULL); /* nothing special for data */
		pPDB->pTree = NULL;
	}
	if (pPDB->pPStat)
	{
		free(pPDB->pPStat);
	}
	/* clean out the structure */
	memset(pPDB,0,sizeof(struct PDB));
}


/* compare function for tree */
SHORT PIG_TreeCompare(PVOID pVal1, PVOID pVal2)
{
	if ((PVOID)-1 == pVal1)
	{
		return (SHORT) strcmp(gTreeSearchValue,(gTreeBase+(int)pVal2)->pPName);
	}
	else if ((PVOID)-1 == pVal2)
	{
		return (SHORT) strcmp((gTreeBase+(int)pVal1)->pPName,gTreeSearchValue);
	}
	else
	{
		return (SHORT) strcmp((gTreeBase+(int)pVal1)->pPName,(gTreeBase+(int)pVal2)->pPName);
	}
}


/* compare function for tree */
int PIG_AVLTreeCompare(const void * pVal1, const void * pVal2, void * pParam)
{
	/* AVL assumes we're dealing with pointers, so we need to avoid
	   values of zero.. everything's offset +1, hence the -1's below */
	if ((PVOID)-1 == pVal1)
	{
		return (SHORT) strcmp(gTreeSearchValue,(gTreeBase+(int)pVal2-1)->pPName);
	}
	else if ((PVOID)-1 == pVal2)
	{
		return (SHORT) strcmp((gTreeBase+(int)pVal1-1)->pPName,gTreeSearchValue);
	}
	else
	{
		return (SHORT) strcmp((gTreeBase+(int)pVal1-1)->pPName,(gTreeBase+(int)pVal2-1)->pPName);
	}
}


/* get ptr to pstat structure, and index.. create if not there yet */
struct PStat* PIG_GetPStat(struct PDB* pPDB, char* pName, int* piIndex)
{
	int Index;
	BOOL bFound = FALSE;
	struct PStat* pPStat = NULL;

	/* allocate initial PStat, if necessary */
	if (!pPDB->pPStat)
	{
		pPDB->pPStat = (struct PStat*)calloc(sizeof (struct PStat),PSTAT_INCREMENT);
		pPDB->iAllocated = PSTAT_INCREMENT;
	}

	if (pPDB->pPStat)
	{
		if (pPDB->iCount < WALKSEARCHCOUNT)
		{
			/* linear search for match */
			for (	Index=0; 
					!bFound  && (Index < pPDB->iCount); 
					Index++)
			{
			if (!strcmp((pPDB->pPStat+Index)->pPName, pName))
				{
					/* found */
					bFound = TRUE;
					Index--;
				}
			}
		}
		else
		{
			/* tree search for match */
			if (pPDB->pTree)
			{
				gTreeBase = pPDB->pPStat;
				gTreeSearchValue = (void*) pName;
				Index = (int)avl_find (pPDB->pTree, (void*)-1);
				if (Index)
				{
					bFound = TRUE;
					/* adjust index down to what we really use */
					Index--;
				}
			}
		}

		if (bFound)
		{
			pPStat = pPDB->pPStat+Index;
		}
		else
		{
			/* not found, create new */
			Index = pPDB->iCount;
			pPStat = pPDB->pPStat + Index;
			pPStat->pPName = (char*) malloc(strlen(pName)+1);
			strcpy(pPStat->pPName, pName);
			/* realloc the array to add new entries, if nec. */
			if (pPDB->iCount == pPDB->iAllocated-1)
			{
				pPDB->iAllocated += PSTAT_INCREMENT;
				pPDB->pPStat = (struct PStat*)realloc(pPDB->pPStat, 
										(sizeof(struct PStat))*pPDB->iAllocated);
				/* reinitialize pPStat because we moved the base */
				pPStat = pPDB->pPStat+Index;
				memset(pPStat+1,0,sizeof(struct PStat)*PSTAT_INCREMENT);
			}
			pPDB->iCount = Index+1;
			/* insert */
			if (!pPDB->pTree)
			{
				pPDB->pTree = avl_create (&PIG_AVLTreeCompare, NULL);
			}
			gTreeBase = pPDB->pPStat;
			avl_probe (pPDB->pTree, (void*)(Index+1));

		}
		if (piIndex) *piIndex = Index;
	}
	return pPStat;
}



void PIG_FindTeams()
{
	struct PStat *pPStat;
	struct PStat *pFPStat; /* following players */
	struct PStat *pOPStat; /* other player */
	int HighestTeam = 0;
	int TeamX;
	char *pName = "";
	int Kills,TeamKills;
	int Deaths,TeamDeaths;
	int TeamSuicides;
	char TeamName[100];
	int Index;
	struct PStat *pTStat; /* team stats */

	pPStat = gPStat.pPStat;
	while (pPStat && pPStat->pPName)
	{
		
		pFPStat = pPStat + 1;
		/* for every following player */
		while (pFPStat && pFPStat->pPName)
		{
			pOPStat = pPStat->OPStat.pPStat;
			/* for every player this guy ever touched */
			while (pOPStat && pOPStat->pPName)
			{
				/* if the name matches a following player */
				/* and team ids are the same...				*/
				if (	!strcmp(pOPStat->pPName,pFPStat->pPName) &&
						(pFPStat->team == pPStat->team))
				{
					/* bump the following player's team index */
					pFPStat->team++;
					if (pFPStat->team > HighestTeam)
					{
						HighestTeam = pFPStat->team;
					}
					break;
				}
				/* next guy */
				pOPStat++;
			}
			/* next player */
			pFPStat++;
		}
		/* next player */
		pPStat++;
	}
	/* now each player has an appropriate team index */
	/* create some teams                             */
	if (HighestTeam > 0)
	{
		for (TeamX=0;TeamX<=HighestTeam;TeamX++)
		{
			pPStat = gPStat.pPStat;
			Kills = 0;
			Deaths = 0;
			TeamKills = 0;
			TeamDeaths = 0;
			TeamSuicides = 0;
			while (pPStat && pPStat->pPName)
			{
				if (pPStat->team == TeamX)
				{
					if ((pPStat->nk>Kills) || 
							((pPStat->nk==Kills) && (pPStat->nd<Deaths)) || 
							((Deaths==0) && (Kills==0)))
					{
						/* team leader so far.. */
						pName = pPStat->pPName;
						Kills = pPStat->nk;
						Deaths = pPStat->nd;
					}
					/* update team stats */
					TeamKills += pPStat->nk;
					TeamDeaths += pPStat->nd;
					TeamSuicides += pPStat->ns;
				}
				pPStat++;
			}
			TeamName[0]=0;
			strcpy(TeamName,"Team ");
			strcat(TeamName,pName);
			/* create a team! */
			pTStat = PIG_GetPStat(&gTStat, TeamName, &Index);
			pTStat->team = TeamX;
			pTStat->nk = TeamKills;
			pTStat->nd = TeamDeaths;
			pTStat->ns = TeamSuicides;
		}
	}
	else
	{
		/* team play not detected after all! */
		bTeamPlay = FALSE;
	}

}


/* compare function for filename index tree */
int PIG_AVLFilenameCompare(const void * pVal1, const void * pVal2, void * pParam)
{
	return strcmp(((PROCESSEDNAME*)pVal1)->cFName,((PROCESSEDNAME*)pVal2)->cFName);
}


/* 
 * Archive functions provide methods for making the pigstats internal
 * state persistent.
 */

static void PIG_ArchiveFilenameAVLFunc(void *data, void *param)
{
	BYTE byLen;
	FILE* hFile = *((FILE**)param);
	/* write the length of the name */
	byLen = (BYTE) strlen(((PROCESSEDNAME*)data)->cFName);
	fwrite(&byLen,sizeof byLen,  1, hFile); /* length of string portion, only */
	/* the name */
	fwrite(data,sizeof(PROCESSEDNAME)-DEFAULT_STRING_LENGTH + byLen,  1, hFile);
	return;
	/* (so easy, no?) */
}

static void PIG_ArchiveFilenames(FILE* hFile)
{
	/* archive the number of file names */
	int iCount = avl_count(gpFilenameTree);
	fwrite(&iCount,sizeof iCount, 1,hFile);
	/* then the names.. one after the other */
	avl_walk(gpFilenameTree,&PIG_ArchiveFilenameAVLFunc,&hFile);
	return;
}


static void PIG_RecoverFilenames(FILE* hFile)
{
	int i,iCount;
	BYTE byLen=0;
	PROCESSEDNAME * pName;
	int iLen;
	/* free the avl filename tree */
	if (gpFilenameTree)
	{
		avl_free(gpFilenameTree);
	}
	/* create the filename tree */
	gpFilenameTree = avl_create(&PIG_AVLFilenameCompare,NULL);
	fread(&iCount, sizeof iCount, 1, hFile);
	for (i=0;i<iCount;i++)
	{
		/* get file name */
		fread(&byLen,sizeof byLen,1,hFile);
		iLen = (sizeof(PROCESSEDNAME)-DEFAULT_STRING_LENGTH)+byLen+1;
		pName = calloc(1,iLen);
		fread(pName,iLen-1,1,hFile);
		/* index it */
		avl_probe(gpFilenameTree,pName);
	}

}


static unsigned char PIG_Compress(void* pvDest,void* pvSrc,int iLen)
{
	unsigned char *pSrc = pvSrc;
	unsigned char *pDest = pvDest;
	unsigned char *pOrigDest = pDest;
	int iCount = 0;
	int iZCount;

	while (iCount < iLen)
	{
		if (*pSrc == 0)
		{
			/* count zeroes */
			iZCount = 1;
			iCount++;
			pSrc++;
			while (!*pSrc && (iCount < iLen))
			{
				pSrc++;
				iZCount++;
				iCount++;
			}
			if (iZCount < 3)
			{
				memset(pDest,0,iZCount);
				pDest+=iZCount;
			}
			else
			{
				*pDest = 0xff;
				pDest++;
				*pDest = (unsigned char) iZCount;
				pDest++;
			}
		}
		else if (*pSrc == 0xff)
		{
			/* escape char.. emit two */
			pSrc++;
			*pDest = 0xff;
			pDest++;
			*pDest = 0xff;
			pDest++;
			iCount += 2;
		}
		else
		{
			/* just copy the char */
			*pDest = *pSrc;
			pDest++;
			pSrc++;
			iCount++;
		}
	}
	return (unsigned char) ((unsigned int)pDest - (unsigned int)pOrigDest);
}


static void PIG_DecompressPStat(void* pvDest, void* pvSrc,int iLen)
{
	BYTE *pSrc = pvSrc;
	BYTE *pDest = pvDest;
	BYTE *pOrigDest = pDest;
	int iCount = 0;

	while (iCount < iLen)
	{
		if (*pSrc == 0xff)
		{
			/* escape char.. look for next */
			pSrc++;
			if (*pSrc == 0xff)
			{
				/* escaped 'ff', emit 0xff */
				*pDest = 0xff;
				pDest++;
				pSrc++;
			}
			else
			{
				/* compressed string of zeros */
				memset(pDest,0,*pSrc);
				pDest += *pSrc;
				pSrc++;
			}
			iCount+=2;
		}
		else
		{
			/* just copy the char */
			*pDest = *pSrc;
			pDest++;
			pSrc++;
			iCount++;
		}
	}
	return;
}


static void PIG_ArchivePStat(int iCount, struct PStat* pPStat, FILE* hFile)
{
	int i;
	struct PStat* pX = pPStat;
	BYTE byLen;
	struct PStat stPS;
	BYTE byComp;
	BYTE byCode;

	/* for every PStat in the list... */
	for (i=0;i<iCount;i++,pX++)
	{
		/* compress the PStat (data only) */
		byComp = PIG_Compress(&stPS,&pX->nk,
			sizeof stPS - offsetof(struct PStat,nk));
		/* write the compressed size */
		fwrite(&byComp,sizeof byComp,1,hFile);
		/* write the PStat data itself */
		fwrite(&stPS,byComp,1,hFile);
		/* write the length of the name */
		byLen = (BYTE)strlen(pX->pPName);
		fwrite(&byLen, sizeof (BYTE), 1, hFile);
		/* the name */
		fwrite(pX->pPName,byLen,  1, hFile);
		/* archive the attached PDB's */
		byCode = 0;
		if (pX->OPStat.iCount)
		{
			byCode = 1;
		}
		if (pX->WStat.iCount)
		{
			byCode |= 2;
		}
		if (pX->TStat.iCount)
		{
			byCode |= 4;
		}
		/* send the code */
		fwrite (&byCode, sizeof byCode, 1, hFile);
		if (byCode & 1)
		{
			PIG_ArchivePDB(&pX->OPStat,hFile);
		}
		if (byCode & 2)
		{
			PIG_ArchivePDB(&pX->WStat,hFile);
		}
		if (byCode & 4)
		{
			PIG_ArchivePDB(&pX->TStat,hFile);
		}
	}
}

static void PIG_RecoverPStat(int iCount, struct PDB* pPDB, FILE* hFile)
{
	int i;
	struct PStat* pX;
	struct PStat stPStat;
	BYTE byLen;
	static char cBuf[DEFAULT_STRING_LENGTH];
	struct PStat stPS;
	BYTE byComp = 0;
	BYTE byCode;

	for (i=0;i<iCount;i++)
	{
		/* read the compressed PStat size */
		fread(&byComp,sizeof byComp,1,hFile);
		/* read the compressed PStat */
		fread(&stPS,byComp,1,hFile);
		/* decompress it into the data section */
		PIG_DecompressPStat(&stPStat.nk,&stPS,byComp);
		/* read the PStat itself */
		/* read the length of the name */
		fread(&byLen,sizeof byLen, 1, hFile);
		/* and the name */
		fread(cBuf, byLen, 1, hFile);
		/* terminate it */
		cBuf[byLen]=0;
		/* create and index a proper PStat using the API */
		pX = PIG_GetPStat(pPDB, cBuf, NULL);
		/* copy over the saved data */
		memcpy(&pX->nk,&stPStat.nk, sizeof stPStat - offsetof(struct PStat,nk));
		/* which PDB's need recovery */
		fread(&byCode, sizeof byCode, 1, hFile);
		/* recover the attached PDB's */
		if (byCode & 1)
		{
			PIG_RecoverPDB(&pX->OPStat,hFile);
		}
		if (byCode & 2)
		{
			PIG_RecoverPDB(&pX->WStat,hFile);
		}
		if (byCode & 4)
		{
			PIG_RecoverPDB(&pX->TStat,hFile);
		}
	}
	return;
}

static void PIG_ArchivePDB(struct PDB* pPDB, FILE* hFile)
{
	/* only archive iCount from the PDB struct */
	fwrite(&pPDB->iCount,sizeof pPDB->iCount,1,hFile);
	/* then archive the pPStat list */
	PIG_ArchivePStat(pPDB->iCount,pPDB->pPStat,hFile);
	return;
}


static void PIG_RecoverPDB(struct PDB* pPDB, FILE* hFile)
{
	int iCount;
	/* read the PDB.iCount value */
	fread(&iCount,sizeof iCount,1,hFile);
	/* prepare to rebuild index dynamically */
	/* clear out the PDB */
	memset(pPDB, 0, sizeof *pPDB);
	/* then recover the pPStat list */
	PIG_RecoverPStat(iCount,pPDB,hFile);
	return;
}


/*
 * PIG_ArchiveInternalState serializes the internal structures
 * to a data file that can be reused on the next run 
 */
void PIG_ArchiveInternalState(time_t tTheTime)
{
	/* this has a lot of test code right now */
	FILE* hFile;
	const char * pHeader = ARCHIVEHEADERTEXT;
	char cSchema = ARCHIVESCHEMA;

	hFile = fopen(gcArchivePath,"wb");
	if (hFile)
	{
		fwrite(pHeader,ARCHIVEHEADERLEN,1,hFile);
		fwrite(&cSchema,sizeof (char),1,hFile);
		fwrite(&tTheTime,sizeof (time_t),1,hFile);
		PIG_ArchivePDB(&gPStat,hFile);
		PIG_ArchivePDB(&gWStat,hFile);
		PIG_ArchivePDB(&gGStat,hFile);
		PIG_ArchivePDB(&gTStat,hFile);
		PIG_ArchiveFilenames(hFile);
		fclose(hFile);
	}

	return;
}

/* 
 * PIG_RecoverInternalState recovers the archived internal 
 * state, returning the time that it was last archived.
 */
int PIG_RecoverInternalState(time_t *tTheTime)
{
	/* this has a lot of test code right now */
	int iRet = 1;
	FILE* hFile;
	static char cHeader[ARCHIVEHEADERLEN];
	char cSchema;
	hFile = fopen(gcArchivePath,"rb");
	if (hFile)
	{
		iRet = 0;
		fread(cHeader,ARCHIVEHEADERLEN,1,hFile);
		if (!memcmp(cHeader,ARCHIVEHEADERTEXT,ARCHIVEHEADERLEN))
		{
			fread(&cSchema,1,sizeof(char),hFile);
			if (cSchema == ARCHIVESCHEMA)
			{
				/* recover time */
				fread(tTheTime,sizeof (time_t),1,hFile);
				/* recover each global PDB */
				PIG_RecoverPDB(&gPStat,hFile);
				PIG_RecoverPDB(&gWStat,hFile);
				PIG_RecoverPDB(&gGStat,hFile);
				PIG_RecoverPDB(&gTStat,hFile);
				/* recover the filenames */
				PIG_RecoverFilenames(hFile);
			}
			else
			{
				iRet = 1;
			}
		}
		else
		{
			iRet = 1;
		}
		fclose(hFile);
	}

	return iRet;
}


/*
 * PIG_ConvertNameCopy  convert to html safe name
 */

void PIG_ConvertNameCopy(char* pszDest, char* pszSrc)
{
	char *pszNext, *pszDestNext, *pszFixup;
	int iNextLen;
	int iBadCharacter;
	int iFixLen;

	pszNext = pszSrc;
	pszDestNext = pszDest;
	iNextLen = strlen(pszNext);
	while ((iBadCharacter = strcspn(pszNext,"\"<'\\/&")) != iNextLen)
	{
		switch (*(pszNext+iBadCharacter))
		{
		case '\"' : pszFixup = "&quot;";iFixLen=6;break;
		case '<' : pszFixup = "&lt;";iFixLen=4;break;
		case '\'' : pszFixup = "&rsquo;";iFixLen=7;break;
		case '\\' : pszFixup = "\\\\";iFixLen=2;break;
		case '/' : pszFixup = "&frasl;";iFixLen=7;break;
		case '&' : pszFixup = "&amp;";iFixLen=5;break;
		default : pszFixup = "&nbsp;";iFixLen=6;break;
		}
		strncpy(pszDestNext,pszNext,iBadCharacter);
		pszDestNext = pszDestNext+iBadCharacter;
		strcpy(pszDestNext,pszFixup);
		pszDestNext += iFixLen;
		iNextLen -= iBadCharacter+1;
		pszNext += iBadCharacter+1;
	}
	strcpy(pszDestNext, pszNext);

}



/*
 * PIG_GetExcludeIncludeData  open file and retrieve strings
 */

void PIG_GetExcludeIncludeData(char* pszExInPath)
{
	FILE* hFile;
	static char cBuff[DEFAULT_STRING_LENGTH];
	int iCount;
	int iRead = 1;
	int iCharRead;
	char* pszX;
	EXINTYPES eType;
	struct ExInData *pstExIn;
	int iValidLine;
	int iGotSomething;

	hFile = fopen(pszExInPath,"rb");
	if (hFile)
	{
		/* read in chars 'til eol is reached */
		while (iRead)
		{
			for (	iCount = 0, iCharRead = 0, iGotSomething = 0;
					iCharRead != '\n' &&
						iCharRead != '\r' &&
						iCharRead != '\f' &&
						!feof(hFile) &&
						iRead &&
						iCount < DEFAULT_STRING_LENGTH;
					iCount++)
			{
				iRead = fread(&cBuff[iCount],1,1,hFile);  /* attempt to read 1 char */
				if (iRead) iGotSomething = 1;
				iCharRead = cBuff[iCount];
			}
			cBuff[iCount-1] = 0;
			if (iGotSomething && (strlen(cBuff) > 3) && (cBuff[0] != '#'))
			{
				pszX = cBuff;
				iValidLine = TRUE;
				if (cBuff == strstr(cBuff,"XX:"))
				{
					eType = XX;
					gbXXDefined = TRUE;
				}
				else if (cBuff == strstr(cBuff,"XA:"))
				{
					eType = XA;
					gbXADefined = TRUE;
				}
				else if (cBuff == strstr(cBuff,"IA:"))
				{
					eType = IA;
					gbIADefined = TRUE;
				}
				else if (cBuff == strstr(cBuff,"IX:"))
				{
					eType = IX;
					gbIXDefined = TRUE;
				}
				else
				{
					iValidLine = FALSE;
				}
				if (iValidLine)
				{
					pstExIn = (struct ExInData *)malloc(sizeof(struct ExInData));
					if (!pstExIn) return;
					pstExIn->pstNext = gpstExInData;
					gpstExInData = pstExIn;
					pstExIn->eType = eType;
					PIG_ConvertNameCopy(pstExIn->caName, &cBuff[3]);
				}
			}
		}
		fclose(hFile);
	}
}

