/****************************************************************************

$Header: pigparse.h  Revision:1.3  Mon Jul 03 22:47:12 2000  ericd $
  
PigParse.h

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
$Log: C:\WORK\PigStats\Archive\SourceDist\CSource\pigparse.i $
  
    Split pigstats.c into pigjs.c, pigparse.c, pigtext.c and pigutil.c
  
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

#ifndef PIGPARSE_H
#define PIGPARSE_H


extern void PIG_FindTeams();

extern int PIG_ParseFile(char *pFileName);

extern void PIG_ReleaseResources(void);


#endif
