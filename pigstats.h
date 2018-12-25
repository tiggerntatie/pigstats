/****************************************************************************

$Header: pigstats.h  Revision:1.27  Tue Nov 30 11:40:56 2004  ericd $
  
PigStats.h

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
$Log: C:\Documents and Settings\All Users\Documents\WORK\PigStats\Archive\SourceDist\CSource\pigstats.i $
  
    Split pigstats.c into pigjs.c, pigparse.c, pigtext.c and pigutil.c
  
  Revision 1.27  by: ericd  Rev date: Tue Nov 30 11:40:56 2004
    "1.8"
  
  Revision 1.26  by: ericd  Rev date: Tue Nov 30 11:20:52 2004
    Revised to improve Linux build
  
  Revision 1.25  by: ericd  Rev date: Sat Aug 11 10:56:52 2001
    
  
  Revision 1.24  by: ericd  Rev date: Wed Aug 01 21:49:46 2001
    Initial release for custom TFC support for LP Boss (1.7a)
  
  Revision 1.23  by: ericd  Rev date: Mon Mar 26 22:09:24 2001
    "1.6"
  
  Revision 1.22  by: ericd  Rev date: Thu Mar 22 22:16:26 2001
    Multiple server threads, support for arbitrarily large file transfers,
    bad html character substitution, beginning rule.pig name exclusion
    code.
  
  Revision 1.21  by: ericd  Rev date: Thu Mar 15 19:45:14 2001
    1.5a  support for new logging standard
  
  Revision 1.20  by: ericd  Rev date: Sun Jan 07 14:28:54 2001
    Version 1.5 candidate
  
  Revision 1.19  by: ericd  Rev date: Wed Jan 03 21:49:54 2001
    Version 1.5
  
  Revision 1.18  by: ericd  Rev date: Sat Nov 11 11:14:50 2000
    Update to forwarding protocol to include an Acknowledge message.
    Unacknowledged files are resent.
  
  Revision 1.17  by: ericd  Rev date: Sat Oct 28 15:40:12 2000
    First changes for 1.4f client server architecture
  
  Revision 1.16  by: ericd  Rev date: Fri Aug 25 23:20:10 2000
    Full repetitive and archiving implementation, Linux safe, some bugs:
    default rep rate is off
  
  Revision 1.15  by: ericd  Rev date: Tue Aug 22 21:59:32 2000
    1) Not using memlib any more 
    2) Repetitive operation fully implemented (lightly tested) 
    3) Rep operation optimized to not repetitively parse logs 
    4) Initial work on stats archiving
  
  Revision 1.14  by: ericd  Rev date: Mon Aug 21 22:23:00 2000
    Initial support for parsed file logging using avl tree index
  
  Revision 1.13  by: ericd  Rev date: Sun Aug 20 22:46:52 2000
    Initial support for GNU AVL Trees (balanced b-trees)
  
  Revision 1.12  by: ericd  Rev date: Sat Aug 19 09:34:28 2000
    1) Fixed memory leaks for repetitive operation 
    2) Changes for log file deletion 
    3) Initial implementation of repetitive operation
  
  Revision 1.11  by: ericd  Rev date: Thu Jul 27 22:06:30 2000
    1) Workaround for CS dedicated server log splitting
  
  Revision 1.10  by: ericd  Rev date: Mon Jul 24 22:34:16 2000
    Minor fixes for Linux compatibility
  
  Revision 1.9  by: ericd  Rev date: Thu Jul 20 22:45:02 2000
    Meta data generation
  
  Revision 1.8  by: ericd  Rev date: Wed Jul 19 22:18:16 2000
    1) Modified to use btree lookup when locating player info
  
  Revision 1.7  by: ericd  Rev date: Mon Jul 03 22:47:12 2000
    Eliminated use of regex for sake of speed
  
  Revision 1.6  by: ericd  Rev date: Tue Jun 13 22:00:22 2000
    Rev. 1.4c (Linux port) candidate
  
  Revision 1.5  by: ericd  Rev date: Sun May 07 22:42:54 2000
    Fixed bug that was repeating match wins calculation
  
  Revision 1.4  by: ericd  Rev date: Tue Apr 18 23:18:36 2000
    Completed log filtering implementation
  
  Revision 1.3  by: ericd  Rev date: Mon Apr 17 23:03:16 2000
    Support for log file parse filtering by day, week, month.  Completed
    support for JS game filtering.
  
  Revision 1.2  by: ericd  Rev date: Sun Apr 09 23:44:20 2000
    Initial support for map/game statistics
  
  Revision 1.1  by: ericd  Rev date: Sat Apr 01 18:57:00 2000
    Removed junk in expanded keywords
  
  Revision 1.0  by: ericd  Rev date: Sat Apr 01 18:44:16 2000
    Initial revision.
  
  $Endlog$
 */


#ifndef PIGSTATS_H
#define PIGSTATS_H


#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <tcl.h>
#include <errno.h>
#include <netdb.h>
#endif

#include "avl.h"

#ifndef BOOL
#define BOOL int
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef TRUE
#define TRUE (1==1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

/* Version String ** MUST FIX ** */
#define PIGVERSTRING "1.8"

/* UDP port for log file forwarding */
#define UDPPORT (5027)

/* Archive Schema # */
#define ARCHIVESCHEMA (3)

#define BUFFER_INCREMENT  (0x2000)
#define INVALID_CHARS "\\//:*?\"<>|"
#define WEAPONEXTENSION "-weapons"
#define GAMEEXTENSION "-games"
#define DEFAULT_STRING_LENGTH (1000)
#define DEFAULT_PLAYER_LENGTH (PIGMAXNAME)
#define WALKSEARCHCOUNT (5)
#define PSTAT_INCREMENT (10)
#define UNKNOWNMAP "unknown"
#define UNKNOWNMAP_NAME_LENGTH (7)

#define PVOID void*
#define SHORT short

#ifndef __linux__
#pragma pack(1)
#endif
typedef struct 
{
	unsigned char byWaitingACK;
	unsigned char byDeletePending;
	char cFName[DEFAULT_STRING_LENGTH]; 
} PROCESSEDNAME;
#ifndef __linux__
#pragma pack()
#endif

typedef enum
{
	TODAY,
	YESTERDAY,
	THISWEEK,
	LASTWEEK,
	THISMONTH,
	LASTMONTH,
	NDAYS,
	LASTPARSESPANTYPE
} PARSESPANTYPE;

typedef enum
{
	REP_NONE,
	REP_QUARTERLY,
	REP_HOURLY,
	REP_DAILY,
	REP_WEEKLY,
	REP_MONTHLY,
	LASTREPTYPE
} REPTYPE;

extern const char* apSpanTypeNames[LASTPARSESPANTYPE];
extern const char* apRepTypeNames[LASTREPTYPE];


struct PDB  /* player (or anything for that matter) DataBase */
{
	struct PStat *pPStat;   /* associated list of PStats */
	avl_tree *pTree;				/* index binary tree */
	int iCount;					/* how many PStats */
	int iAllocated;			/* how many allocated for */
};

struct PStat
{
	char *pPName;
	struct PDB OPStat; /* other players, related to this one */
	struct PDB WStat;  /* weapon stat list, related to this one */
	struct PDB TStat;  /* linked teams stats related to this one */
	int nk; /* excluding suicides */
	int nd; /* including suicides */
	int ns; /* num suicides */
	int nmw; /* num match wins (teams) */
	int ncaps; /* num flag caps (tfc) */
	int score; /* score.. arbitrary definition */
	int skill; /* custom score formula (tfc) */
	int duration;
	int gameduration;
	int used;
	time_t start;
	int team;
};

typedef enum 
{
	PARSEKILLS,
	PARSESUICIDES,
	PARSEDEATHS,
	PARSEDROPOFFS
} PARSESTATE;


#ifdef __linux__
#define  SOCKET struct socket *
#define INVALID_SOCKET ((SOCKET)-1)
#define SOCKET_ERROR (-1)
#endif


#ifdef __linux__
#define SLEEP(s) Tcl_Sleep(s)
#else
#define SLEEP(s) Sleep(s)
#endif


struct HostConn
{
	SOCKET sock;
	struct sockaddr_in addr;
};

/* maximum forwarded file size */
#define PIGMAXFWD (4000)
#define PIGMAXFWDPADDED (PIGMAXFWD+1000)
#define PIGMAXNAME (256)

struct PIGBuffer
{
	char cProtoVersion[4];
	unsigned long dwSizeAll;
	unsigned short wSizeThis;
	unsigned long dwOffsetThis;
	unsigned long dwChecksum;
	unsigned short wPathChecksum;
	char cName[PIGMAXNAME];
	char cBuffStart[PIGMAXFWDPADDED];
};

#define PIGBUFFOFF (offsetof(struct PIGBuffer,cBuffStart))
#define PIGNAMEOFF (offsetof(struct PIGBuffer,cName))

/* file forwarding protocol version string (must be 4 chars) */
#define PIGPROTOVERSION "PS02"

/* enums of different types of excludes/includes */
typedef enum
{
	XX, /* exclude, exact */
	XA, /* exclude, any */
	IA, /* include, any */
	IX	 /* include, exact */
} EXINTYPES;  /* exclude include types */

/* structure for holding a single exclude/include name */
struct ExInData
{
	struct ExInData* pstNext;  /* pointer to next */
	char caName[PIGMAXNAME];
	EXINTYPES eType;
};

/* global struct for PLAYER stats */
extern struct PDB gPStat;
/* global stat pointer for WEAPON stats */
extern struct PDB gWStat;
/* global stat pointer for GAME stats */
extern struct PDB gGStat;
/* global stat pointer for TEAM stats */
extern struct PDB gTStat;

/* global path to jscript files */
extern char gcJPath[];
/* global path to log files */
extern char cInPath[];
/* global path to output files */
extern char gcOutPath[];
/* global root path of output files */
extern char gcRootPath[];
/* global root name of output files */
extern char gcRootName[];
/* global html extension text */
extern char gcExtPath[];
/* global cross ref. path for weapon page */
extern char gcWeaponRef[];
/* global cross ref. path for general page */
extern char gcGeneralRef[];
/* global cross ref. path for game page */
extern char gcGameRef[];
/* global path for archive file */
extern char gcArchivePath[];
/* global output file type */
extern BOOL bIsHTML;
/* global jscript switch */
extern BOOL bIsJScript;
extern int	*gpiPStatList;	/* pointer to a list of indeces */
extern int	gPStatListSize;
extern int	gPStatListQty;
extern struct PStat *gpWorkingPStat;
/* -s silent operation flag */
extern BOOL gbSilent;
/* -f filter out zero statistics (no argument) or N players (N argument) */
extern BOOL bFilter;
extern int iLimit; /* limit to N players */
/* -u ignores unranked (i.e. filtered) players when crossreferencing */
extern BOOL bIgnoreUnranked;
extern int iXLimit; /* limit cross ref. lists to this many elements */
extern int iGLimit; /* limit list of games in JavaScript datafile */
/* -m multiple html file output */
extern BOOL bMux;
/* -l log file parse filtering flag globals */
extern BOOL gbLogFilter;
extern PARSESPANTYPE eLogFilterType;
extern REPTYPE eRepType;
extern int iLogFilterDays;
/* -d delete files not parsed due to -l option */
extern BOOL gbDeleteFiles; 
/* global time stamp data */
extern time_t StartTime,TimeStamp,RunTime;
/* global flag indicating team play */
extern BOOL bTeamPlay;
/* global flag indicating match wins have been computed */
extern BOOL bMatchWinsComputed;
/* global flag indicating suppression of text file output */
extern BOOL bSuppressText;
/* global pointer to the working pstree base */
extern struct PStat *gTreeBase;
/* global pointer to void for finding tree nodes */
extern PVOID gTreeSearchValue;
/* global total bytes processed */
extern unsigned long gBytesProcessed;
/* global files processed */
extern unsigned long gFilesProcessed;
/* global process duration */
extern double gProcessDurationSecs;
/* global process timedate */
#define TIMEDATESTRLENGTH (26)
extern char gcProcessTime[TIMEDATESTRLENGTH];
/* global pointer to the filename index tree */
extern avl_tree *gpFilenameTree;
/* global flag indicating repeat mode */
extern BOOL gbRepeat;
extern int giRepeatModifier;  /* 5 minutes if no repeat type specified */
/* global flag indicating at least one kill was detected during run */
extern BOOL gbKillDetected;
/* archiving in use */
extern BOOL gbArchive;  
/* log file forwarding in use */
extern BOOL gbLogForwarding;
/* destination ip address */
extern unsigned long gulForwardingDestination;
/* tcp/ip socket info for transmitting */
extern struct HostConn gstClient;
/* tcp/ip socket info for receiving */
extern struct HostConn gstServer;
/* file server */
extern BOOL gbLogServer;
/* client port number (i.e. sending TO port) */
extern unsigned short gwClientPort;
/* tcp/ip socket info for receiving ACK*/
extern struct HostConn gstClientAck;
/* exclude file used */
extern BOOL gbExcludeFileUsed;
/* pointer to first exclude/include struct */
extern struct ExInData *gpstExInData;
/* booleans indicate which types of exin are defined */
extern BOOL gbXXDefined;
extern BOOL gbXADefined;
extern BOOL gbIADefined;
extern BOOL gbIXDefined;
/* boolean indicates if tfc logs were parsed */
extern BOOL gbIsTFC;

extern void PIG_CheckAckSocket(void);


#endif
