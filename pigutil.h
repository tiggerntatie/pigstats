/****************************************************************************

$Header: pigutil.h  Revision:1.6  Wed Aug 01 21:49:46 2001  ericd $
  
PigUtil.h

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
$Log: C:\WORK\PigStats\Archive\SourceDist\CSource\pigutil.i $
  
    Split pigstats.c into pigjs.c, pigparse.c, pigtext.c and pigutil.c
  
  Revision 1.6  by: ericd  Rev date: Wed Aug 01 21:49:46 2001
    Initial release for custom TFC support for LP Boss (1.7a)
  
  Revision 1.5  by: ericd  Rev date: Sun Mar 25 22:29:34 2001
    rule.pig name exclusion/inclusion.. debugging related to new logging
    standard.
  
  Revision 1.4  by: ericd  Rev date: Thu Mar 22 22:16:26 2001
    Multiple server threads, support for arbitrarily large file transfers,
    bad html character substitution, beginning rule.pig name exclusion
    code.
  
  Revision 1.3  by: ericd  Rev date: Wed Aug 23 20:59:22 2000
    State archiving almost full implementation
  
  Revision 1.2  by: ericd  Rev date: Wed Jul 19 22:18:16 2000
    1) Modified to use btree lookup when locating player info
  
  Revision 1.1  by: ericd  Rev date: Sat Apr 01 18:57:00 2000
    Removed junk in expanded keywords
  
  Revision 1.0  by: ericd  Rev date: Sat Apr 01 18:44:16 2000
    Initial revision.
  
  $Endlog$
 */


#ifndef PIGUTIL_H
#define PIGUTIL_H

/* 
 * Morph a text string until it has only valid filename chars 
 * At this point we're going to have a name with only a-z, A-z and 0-9 
 */
extern char* PIG_MakeStringValidFileName(char* pNameTemp, char* pDestName);


/* initialize the pstat pointer list */
extern void PIG_SetupPStatList(struct PDB *pPDB);

/* sort a pstat pointer list by Number of Match Wins */
extern int PIG_NMWComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByNMW(void);

/* sort a pstat pointer list by Kills */
extern int PIG_KComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByK(void);

/* sort a pstat pointer list by Kills and Deaths*/
extern int PIG_KandDComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByKandD(void);

/* sort a pstat pointer list by deaths */
extern int PIG_DComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByD(void);

/* sort a pstat pointer list by start time */
extern int PIG_STComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByST(void);

/* sort a pstat pointer list by suicides */
extern int PIG_SComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByS(void);

/* sort a pstat pointer list by kill rate */
extern int PIG_KRComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByKR(void);

/* sort a pstat pointer list by ratio */
extern int PIG_RComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByR(void);

/* sort a pstat pointer list by time (duration) */
extern int PIG_TComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByT(void);

/* sort a pstat pointer list by caps (tfc) */
extern int PIG_CComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByC(void);

/* sort a pstat pointer list by skill (tfc) */
extern int PIG_SKComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListBySK(void);

/* sort a pstat pointer list by T-score (tfc) */
extern int PIG_TSComp( const void *arg1, const void *arg2 );
extern void PIG_SortPStatListByTS(void);


/* clean up allocated memory */
extern void	PIG_CleanPStat(struct PDB *pPDB);

/* get ptr to pstat structure, and index.. create if not there yet */
extern struct PStat* PIG_GetPStat(struct PDB *pPDB, char* pName, int* piIndex);

extern void PIG_FindTeams();

/* compare function for filename index tree */
extern int PIG_AVLFilenameCompare(const void *, const void *, void *);

/* archiving API */
extern int PIG_RecoverInternalState(time_t *tTheTime);
extern void PIG_ArchiveInternalState(time_t tTheTime);
extern void PIG_GetExcludeIncludeData(char* pszExInPath);

/* html safe name conversions */
extern void PIG_ConvertNameCopy(char* pszDest, char* pszSrc);


#endif