PigStats 1.7  README file September 17, 2001
Copyright (c) 2001 Eric Dennison

Disclaimer:

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

******************************************************************************
LINUX INSTALLATION AND USAGE INFORMATION IS AT THE END OF THIS FILE
******************************************************************************

Sections included in this file:

PigStats introduction
PigStats features
Supported platforms
Installation
Usage
HTML Configuration
FAQ/examples
License
Source code
Contact
Credits
***Linux distribution***


***PIGSTATS INTRODUCTION******************************************************

PigStats is a Half-Life server log analyzer that produces multiplayer game
statistics in a convenient HTML (web browser) format.  This program should be
of interest to you only if you are running a Half-Life, Counter-Strike,
Opposing Force, or Team Fortress server.  PigStats doesn't do anything with
the single player version of any Half-Life based game.

In order to use PigStats, you must configure the game server to generate
server log files.  These files simply record when and what games were played
and who killed who and with what weapon.  The Counter-Strike and TFC servers
produce some additional information, but PigStats only uses the portion that
is in common with the basic Half-Life game.

PigStats is a command line or console application.  There is no fancy GUI, but
it is highly configurable by means of command line arguments.  The idea is
that you figure out which options you want to use, set it up to run
automagically (using a task scheduler, or the built-in PigStats scheduler),
then forget about it while it keeps your stats web page up-to-date.

There are many different ways to use PigStats.  You can:

  -Run it on an as-needed basis to generate statistics. 
  -Schedule it to run periodically using your operating system's task
   scheduling facilities, updating statistics with each run.
  -Run it using its internal scheduler, which updates the statistics any time
   new log files are produced.
  -Use it to generate statistics on log files covering a specific time span.
  -Use it to automatically delete old log files.
  -Use it to archive statistics with each run, so that subsequent runs only
   analyze recent log files.
  -Run several instances ot PigStats to maintain statitistics covering several
   different time spans (i.e. today's stats, yesterday's, last week's, etc.)
  -Collect statistics from several servers at one master server.
  -Filter out undesirable player names or clans from the statistics.
  
PigStats also offers complete flexibility in the the HTML presentation.  You
can choose either to generate HTML in a fixed format, or generate a JavaScript
data file that can be included in customized HTML (a default HTML format is
provided for this method).  The JavaScript output method is the most flexible
and supports more of the PigStats feature set than the fixed HTML method.

PigStats is efficient and FAST. It completes its parsing and analysis in less
time than most (all?) other stats programs.  This comes at the cost of some
flexibility, however.  PigStats only generates the following results:

  -Number of different weapons used, and for each weapon:
     Total kills with that weapon
     Number of players that used that weapon and for each of them:
       How many times they killed with it
  -Number of players logged
  -List of players, sorted by their total kills
  -List of players, sorted by their TFC score (TFC only)
  -List of players, sorted by their kill ratio (kill/death)
  -List of players, sorted by their kill rate (kills/minute)
  -List of players, sorted by their total deaths
  -List of players, sorted by their total suicides
  -List of players, sorted by number of match wins
  -List of players, sorted by time played
  -List of players, sorted by skill level (TFC only)
  -List of players, sorted by flag captures (TFC only)
  -List of player names, and for each player:
    His total kills
    His TFC score (if applicable)
    His total deaths
    His total suicides
    His kill to death ratio
    His kill rate
    His total play time
    His skill level (TFC only)
    His total flag captures (TFC only)
    The total number of players he has killed, and for each of them:
      The number of times he killed that player
      The name of the victim
    The total number of players that have killed him, and for each of them:
      The number of times that player killed him
      The name of the killer
    The total number of players for which a personal kill/death ratio exists:
      The personal ratio against that player (times he killed that player
      divided by times that player killed him)
      The name of the other player
    The total number of weapons he has used, and for each of them:
      The number of times he used it
      The name of the weapon
    The total number of weapons he has died by, and for each of them:
      The number of times he died by it
      The name of the weapon
  -The number of games logged for these statistics
  -The map name, time and total number of players in each game logged
  -The names of each player in each game logged, and their total kills and
   deaths in that game
  -The names of each player that won a match (highest kills in a game) and
   the number of matches they won
 
If you need statistics other than these, then you need a different stats
program.  :)


***PIGSTATS FEATURES**********************************************************

Revision 1.7

* Full support for tracking TFC score, and a new "skill rating".
  (thank you LP Boss for tireless nagging, optimism and the courage
  to come up with a TFC skill rating for me!)
* Summary display is sortable by time played.
* Support for Gunman Chronicles and Front Line Force (thank you 
  eternal_loser)

Revision 1.6

* Full support for the new logging standard with all of the features available
    in the original PigStats 1.5. This means that the client-server feature
    (-c -v options) fully supports very long log files, and that suicides
    are recorded correctly again.
* Client-server options are subtly improved.  The '-v' option can be included
    more than once on the command line, as long as each instance uses a 
    unique port id.  This allows multiple clients on a single machine to 
    direct their logs to a single server on the same or different machine.
    Also, there is no arbitrary limit to log files when using these options
    (previous versions were limited to 64k per file).
* Introduces two levels of player name filtering.  Names can be excluded from
    the statistics in 4 different ways at the 'front end', and one way at the 
    'back end'.  See the instructions in 'pigrules.txt' in the html directory
    for details on front end filtering, or 'pigcfig.js' for details on 
    expanded back end filtering.
* Since changes in 1.6 are extensive, a Linux release won't be made until I
    get a chance to test it on the real thing.

Revision 1.5a

* Initial support for new logging standard implemented in Counter-Strike 1.1
    and HL 1.0.1.6.  This version has been rushed to release and does not
    support all of the features of the previous releases.  Specifically:
    - Suicides and "killed by world" events are not logged
    - Client-Server feature may not work with TFC, due to the obscene size
      of the new TFC log files.
    - Other problems will be addressed as they are brought to my attention.
* This release maintains FULL backwards compatibility with old log files.
    There is no need to remove old log files before using 1.5a.
    
Revision 1.5 (Client-Server Edition):

* One or two minor bug fixes
* Support for log file forwarding across the network.  A single PigStats
    server can collect statistics from any number of PigStats clients. ('-c' 
    and '-v' options)
* Run time switch (-u) for ignoring unranked players (those that were dropped
    because they didn't meet the criteria of the '-f' switch) when generating
    player cross-reference lists.  This must be used with the '-f' option 
    and is useful for limiting the output file size and improving stats 
    readability.  Any player that doesn't make the 'top N' list will not 
    appear in the player detail view cross-reference lists, but will appear in
    weapon and game details.
* Brand-spanky new HTML and award images!  With this release, the author 
    dives timidly into the CSS pool.  I rewrote all of the HTML from the 
    ground up, lending it a simpler, more elegant style.  Early reviews 
    suggest that it's easier to understand because it doesn't have all those
    frames.  Although I develop with IE 5.5, it still seems to work passably
    well with Netscape and not at all with Opera (who uses Opera?).
    
    THANK YOU to Darren H. for the new award artwork.  It really gives 
    PigStats a lift.

* A HTML Help file is now integrated with the standard HTML output.  It's 
    nothing fancy, but it's THERE (finally..).
    
    THANK YOU to ZenMaster for the original text and much needed inspiration 
    to GET IT DONE (finally..).

* The old 1.4e HTML is still included in the standard installation.  If you
    want to use the old HTML, copy the tree at 
        \Program Files\PigStats\HTML\1.4e\ 
    to your HTML output folder(s).

Revision 1.4e:

* Performance enhancements:
    Switched to a better (faster) binary tree indexing library
    Support for scheduled analysis ('-r' option)
    Eliminated need to re-analyze files on consecutive PigStats runs ('-a'
    option)

Revision 1.4d:

* Performance enhancements dramatically decrease processing time required.
* Inaccurate play time statistics corrected from 1.4b.
* Inclusion of meta-data about the processing, including:
  Process time and date
  Total bytes, files and players processed
  Time required for processing
* Corrects "unknown" map reporting for Counter-Strike 6.5+ dedicated server
  logs.
* Common code base supports both Win32 and Linux platforms (OK, so this isn't
  much of a "feature", but I'm pretty darned excited about it).

Revision 1.4b:

* Support for new log format in Counter-Strike 6.5 release.
* Known defects in 1.4a repaired.

Revision 1.4:

* Output as JavaScript data files allows user customization of HTML format,
  style and content
* All new hang-yourself-with-the-power command line arguments
  (effective with -j JavaScript output option only):
  -x: Allows limiting size of cross- reference listings
  -g: Limit game/map statistics lists
  -l: Specify log files from today, yesterday, this week, last week, this
      month,last month, or any number of days up to today
* New PigStats installer automatically detects TFC, OP4 and Counterstrike
  installations and adds shortcuts

Revision 1.3:

* No limit on number of log files, or players analyzed
* HTML output to a single file, or split into separate files for general,
  weapon, game and individual player statistics
* Filter out players with zero kills
* Simplified installation: PigStats comes as a single executable with no
  additional dll's to install

Revision 1.2:

* Generates output in text or html
* Statistics for total kills, deaths, suicides, kill rate, and kill  ratio
  (kills/deaths)
* Statistics for weapon usage
* Individual player statistics including kill ratios against other players and
  most favored/feared weapons
* Player kills and deaths for each match played
* Match totals
* Displays team information for each match and match totals, when using
  teamplay


***SUPPORTED PLATFORMS********************************************************

PigStats was built and tested under both Win98 and WinNT4.0.  It should work
under Win95, but I'm not promising anything. ;)


***INSTALLATION***************************************************************

Put pigstats.exe wherever you want.  It's not a GUI app and you will probably
want to run it from a DOS box command prompt or a batch file.  So you should
either execute it from your current directory, or set your path system
variable to point to its final resting place.

Now in Version 1.4, PigStats is brought to you with a professional grade
installation package: Installer VISE by MindVision. This installer can place
shortcuts for parsing log files in your start menu. To see an example of how
to use PigStats, look at the batch files that the shortcuts point to. We (I)
expect Installer VISE to add considerably to your overall PigStats experience.

If you use the -j option with PigStats, your JavaScript output should be
directed (by the path you provide with the option) to a folder that has HTML
files capable of parsing it. If you allowed the installer to create output
directories and shortcuts for you, then you needn't worry about this. If you
elect to send the output somewhere else, then copy the files and folder(s) in

    <pigstats program folder>\HTML\default

to the same destination specified with the -j option.


***USAGE**********************************************************************

PigStats [-i<input path>] [-o<output path>] [-s] [-m] [-f[N]] [-xN]
         [-j<javascript output path>] [-gN] [-l[y|w|l|m|n|N]] [-d]
         [-n] [-r[h|q|d|w|m][N]] [-a] [-u] [-p] [-v<port id>]
         [-c<port id><server ip address or name>]

Where <input path> is the path and file name (including wild cards) of the
input log file(s), <output path> is the path and file name of an optional text
of html output file. <javascript output path> is the path (only) to receive
JavaScript include files (this is the recommended output option). Don't add
any spaces between the '-i', '-o' or '-j' and the respective paths.

If the extension of the output file is .htm or .html then output is in HTML,
otherwise it's straight text (-o option only).

If no input path is provided PigStats looks for log files in the current
directory.  If no output path is provided, PigStats generates a text file
called "stats.txt" in the current directory.

'-s' causes PigStats to execute silently. If this parameter is omitted,
PigStats prints a startup banner and parsed file names to the console.

'-m' causes PigStats to generate individual output files for game, weapons and
player statistics (if an html output file is specified).  This is helpful when
a single html file would be too large to quickly load in a browser (-o option
only).

'-f' causes PigStats to filter out zero scores.  Players with zero kills are
not displayed in the general statistics section. If N is provided and the -j
option is used, this limits the JavaScript data listings to the top N players
(by total kills).

'-x' customizes cross reference list sizes (default is 5). If the number of
logs or players is relatively small, increase this value to show more cross
reference info (i.e. each player's top N favorite victims, etc.). This feature
is only available with the -j option.

'-g' limits game/map statistics to N games. The default is unlimited. This
feature is only available with the -j option.

'-l' directs PigStats to parse a limited set of log files, according to the
next character or number (only available with the -j option):

    (no next char or num): only parse log files from today
    y: only parse log files from yesterday
    w: only parse log files from this week
    l: only parse log files from last week
    m: only parse log files from this month
    n: only parse log files from last month
    N (integer number): only parse log files from the last N days, where 1
    means today and yesterday, 2 means same as 1 plus the day before, and so
    on...

'-d' DELETES all log files that weren't parsed as a result of the -l
option.  USE WITH CAUTION.

'-r' parse after 5 minutes, and repeat automatically every 5 minutes.
The following modifiers affect repetition rate:
    N: wait for N minutes, then repeat every N minutes.  If preceded
    by one of the following modifiers, indicates an additional N
    minute delay before parsing.
    h: parse hourly (+ N minutes)
    q: parse on the quarter hour (+ N minutes)
    d: parse daily (midnight + N minutes)
    w: parse weekly (midnight sat/sun + N minutes)
    m: parse monthly (midnight AM of 1st, + N minutes)

'-a' archive pigstats' internal state between runs.  This allows you
to efficiently repeat cumulative stats runs without parsing the
same logs over and over again.  The state is archived to a file
called archive.pig if you have selected the -j option and <html
or text file name>.archive.pig if only the -o option is used AND
a file name was provided.  If no output path/filename is given
or neither -j and -o are provided, then -a does nothing.  The
archive file is saved to the same directory as the specified output file(s).

'-n' suppresses default generation of "stats.txt" file when no text or html
output file is specified (this option is recommended if you use the -j option.

'-v' act as pigstats server.  Port id is a single character from a..z.
In this mode, pigstats may receive log files from other computers running 
pigstats in 'client' mode, putting them in the log file path indicated with 
the '-i' option.  This option is only effective in conjunction with the '-r' 
option.  The '-v' option can be added more than once in the command line
as long as each instance uses a different port id.  This permits multiple
clients from one machine to direct logs to a single server on the same or
different machine.

'-c' act as pigstats client.  Port id is a single character from a..z, that 
much match the port id of the destination server, permitting multiple servers 
running on the same computer.  Server ip address or name is either the ip 
address (i.e. 192.168.0.1), or a valid name for a machine running the pigstats 
server (i.e. pig.pigindustries.com).  No spaces are allowed between the option, 
the port id and the destination address.  The pigstats client will forward all 
processed log files to the server machine where they may be collected with logs 
from multiple machines and re-processed as a whole.  This option is only 
effective with the '-a' option.

'-u' ignore unranked players when generating player cross-reference lists.  
This must be used with the '-f' option and is useful for limiting the output 
file size and improving stats readability.  Any player that doesn't make the 
'top N' list will not appear in the player detail view cross-reference lists, 
but will appear in weapon and game details.

'-p' use player name filtering as specified in the file 'pigrules.txt', located
in the HTML output directory specified with the '-j' option.  See the sample
'pigrules.txt' file for instructions on creating the rules file.

	
Oh yeah... almost forgot:  To generate Half-Life log files you have to turn on
logging.  Bring down the Half-Life console (`) and type "log on".  Log files
will appear in a valve\logs folder as if by magic!.


***HTML CONFIGURATION********************************************************

With the introduction of PigStats 1.5, you have some ability to customize
the behavior of the html by editing the following files:

** pigcfig.js (JavaScript, in the html output folder) **

The following items are configurable by editing pigcfig.js:

* pscfgminkill: The minimum player kills needed to appear in the summary.  
  This is handy for eliminating players that just dropped in for a minute 
  or two.  Similar in effect to the '-fN' run-time option.  The default 
  is 1.

* pscfgignoreplayerswithnodeaths: Set true to ignore players that have no
  deaths logged.  This eliminates players with '-1' kill/death ratio values
  ('N/A' in PigStats 1.4).  The default is true.

* pscfgignoreplayer: Set to the (string) name of a player to ignore in the
  summary statistics.  The default is ["Sir_Knumskull"].  Multiple names
  can be ignored thusly:
  pscfgignoreplayer = ["Sir_Knumskull","Player","cheater dude"]

* pscfigignoreunranked: Set true to ignore unranked players (i.e. that don't
  appear in the summary statistics) in the player detail cross-reference 
  lists.  Default is false.  
  
** pswntt.js (JavaScript, in the html output folder) **

This file allows you to associate logged weapon names with more readable 
text strings.  For example, when generating stats for Counter-Strike games,
this file causes the weapon named "deagle" to be replaced by "Desert Eagle".

To edit this file, just examine the existing file and follow by example.

** style.css (Style sheet, in the html output folder) **

If you're comfy with CSS and HTML editing, then the quickest way to modify 
the look and feel of your HTML is by playing around with the style.css file.

Of course *all* of the HTML files are modifiable, if you're in to that sort
of thing!  Have a ball.  Go crazy.  If you're really proud of your changes,
don't be shy about it: let me know!


***FAQ/EXAMPLES**************************************************************

Q: Every time I run PigStats, I get " sorry... Win32 doesn't seem to like your
choice for input file name". I've checked and re-checked my arguments, and
nothing's wrong.

A: This often happens when people try to test PigStats for the first time.
Realizing that PigStats won't work without a log file, they host a multiplayer
game just long enough to generate a log file. Unfortunately, the log file you
generate in this scenario is woefully lacking as far as PigStats is concerned.
If you want to generate a test log file, be sure that somebody, somewhere,
gets killed! PigStats really wants to see bloodshed in order to work properly.
---

Q: I ran PigStats OK, but why does it take so long to load the html output
file?

A:If you run a lan server a couple of times a week to amuse yourself and some
friends at the office, then the output probably isn't overwhelming. But if
you're running an Internet server that's up 24 hours a day, seven days a week,
then you're probably trying to display more information than you really want
to (who cares how many kills "Player" had for the 10 minutes he was connected
last Tuesday?). You can place limits on the size of the generated data files
by using the -j option, together with the -f (limit number of players listed),
-x (limit size of cross-reference lists), -g (limit the number of games
listed), -l (limit the number of logs analyzed) and -u (ignore unranked 
players in the cross-reference lists.. use with -f only) options.
---

Q: What are log files, and how do I get them?

A: PigStats is what's known as a server tool, which means that it only works
if you are running a Half-Life server. If you are running a server you must
turn on logging, either by including a "log on" command in the autoexec.cfg
file, or by manually typing "log on" in the server console.
---

Q: Ok.. what's a server console?

A: If you're running a Half-Life server and you don't know what the console
is, then you're already in over your head. ;)
---

Q: How come you don't have [name your Half-Life mod here] support?

A:I created PigStats for use with the original Half-Life. It so happens that
log files generated by most modifications are pretty much compatible (OP4,
Counterstrike, TFC). However, some of the team-oriented modifications provide
additional information in their logs regarding goals, team scores, etc., which
PigStats ignores. Whenever possible, I would like to provide specialized
support for these games, but I need your feedback to know where to put my
priorities.
---

Q: How come I don't see team statistics?

A: PigStats 1.4 still supports generating team statistics for Half-Life
teamplay games, but only in the text and html output modes. JavaScript
teamplay stats may be added in a future release if there is any demand from
users.
---

Q: So you say that PigStats 1.4 lets me customize my HTML output.. How do I do
that?

A: Unfortunately, time doesn't permit me to produce an exhaustive manual for
customizing PigStats. But here's a quickie:

When you run PigStats with the -j option, it generates two JavaScript source
files: psovera.js and psgame.js. These files are pulled in by the HTML files
provided in the PigStats\HTML\default directory, which may have already been
installed to your Half-Life\valve, ..\tfc, ..\gearbox or ..\cstrike folders as
part of the PigStats installation process.

To a limited degree, these JavaScript files are self documenting. You are free
to examine the HTML files I have provided to understand how they work. Once
you have a glimmer of understanding, you can modify them to your heart's
desire.

I never said it would be easy.
---

Q: I don't want to see statistics for games played more than a month ago. 
Most of those people don't play on this server any more anyway.

A: You could delete the old log files, of course, or you can limit the number
of logs parsed using the '-l' option.  For example:

pigstats -ic:sierra\half-life\cstrike\logs\L*.log
-jc:\sierra\half-life\cstrike\pigstats\ -lm

will only parse files from this calendar month, and

pigstats -ic:sierra\half-life\cstrike\logs\L*.log
-jc:\sierra\half-life\cstrike\pigstats\ -l30

will only parse files from the last 30 days.  See the usage section about the
'-l' option for more possibilities and modifiers.
---

Q:  I'm sure PigStats is wasting a lot of time looking for this week's log
files, because I've got 20 months of server logs on my hard drive.  How will I
ever figure out which log files to delete?

A:  You're kidding.  That's too hard?  If managing your log files is just more
than you can handle, use the '-d' option to delete any log files that were
passed over in the process of carrying out the '-l' option.  Be careful,
though.  There's no pigstats option for bringing the dead back to life.

If you're running pigstats several times, using different '-l' option
modifiers (last month, this week, last week, etc.), then only use the '-d'
option on the run that you're sure will include the oldest log files.  For
example, if you're generating stats on last month, last week and yesterday,
then only include the '-d' option the "last month" run.  Otherwise, the "last
week" run will delete most, if not all, of the "last month" log files!

Example:

pigstats -ic:sierra\half-life\cstrike\logs\L*.log
-jc:\sierra\half-life\cstrike\pigstats_yesterday\ -ly

pigstats -ic:sierra\half-life\cstrike\logs\L*.log
-jc:\sierra\half-life\cstrike\pigstats_lastweek\ -ll

pigstats -ic:sierra\half-life\cstrike\logs\L*.log
-jc:\sierra\half-life\cstrike\pigstats_lastmonth\ -ln -d
---

Q:  How do I get PigStats to run automatically?

A:  Use your operating system's scheduling facilities to run PigStats as often
as you like.  If that looks like a daunting task, then use the PigStats '-r'
option.

If you run PigStats with the '-r' option, it will not return to the command
prompt.  Ever.  With no further options, it will wait 5 minutes, then begin
repeating the analysis process every 5 minutes thereafter.  Add a number
immediately after the '-r' to change the cycle rate (in minutes).  Or add a
character modifier to make PigStats run on the quarter hour, hour, day, week
or month.  Further add a number to the option, and it will repeat at the
specified time, plus that number of minutes (5 minutes is default).

For example, to make PigStats run the analysis hourly, at 5 minutes past the
hour:

pigstats -ic:sierra\half-life\cstrike\logs\L*.log
-jc:\sierra\half-life\cstrike\pigstats\ -rh5 -s&

Note that I threw in the extra '-s' option so that PigStats won't attempt to
send any status info to the console.  This is useful if you're running using
the '-r' option in a Linux environment, and you want it to run as a background
task.  The '&' character tells Linux to run the program as a background task,
and returns the console prompt to you.
---

Q:  Ok, I'm running PigStats hourly, generating stats for the entire month. 
But there is a noticeable hit on the server performance while it's parsing
thousands of log files.  Isn't there any way for it to remember what it's
already parsed?

A:  Yes, there is.  Use the '-a' option to "archive" the last PigStats run. 
The next time you run PigStats, it will read the archive file created during
the previous run, and pick up exactly where it left off, parsing only the
files that have been created since then.  The '-a' option only works if you're
using the '-j' and/or '-o' options.
---

Q:  How do I use the client/server options with PigStats?  What's it all
about?

A:  Say you've got 5 separate servers, all running de_dust counterstrike 
servers all day, all the time.  And you want to present a single stats page
that accumulates data from all the servers.  Ok?  So what you need to do is
choose one of the servers as your web server, where your stats page will be.
That box will be the PigStats server.  The other four will be PigStats clients.
Run PigStats on all of the four clients using the -c option, thusly:

-ca192.168.0.1

- or -

-camydustserver.com

The 'a' immediately following -c specifies which of 26 (a..z) ports you are
using, and the ip address or domain name specifies a computer name that can
be resolved to an ip address.

I also recommend that you run the client PigStats in a background, archiving
and repeating mode (-r, -a and -s options) with no time span filtering (-l).

On your server box, run PigStats using the -v option, thusly:

-va

You must run the server in a repeating mode (-r option), in addition to any 
other options you wish to use.  What happens when you use the -v option is
PigStats starts a child thread that does nothing but listen for server log
files sent by PigStats clients.  When it receives one, it unzips it and saves
it to the same directory where it looks for log files on its local machine.
It's important that the user account under which PigStats is running have the
privileges required to write to this directory.  The main PigStats thread
sees and analyzes the log files from client boxes as if they were ordinary 
server log files created locally.
---

Q:  Can a single machine be both a PigStats server and a PigStats client?

A:  Yes.  You can build a whole tree of PigStats client/server connections.
A true megalomaniac would buy a suitable domain name, and set up a PigStats
server for use by the general public.  Everyone who runs PigStats could
have their log files automatically sent (through the magic of networking) to 
the central server machine.  If you're the one, let me know and I'll spread
the word.  :)
---

Q:  Why would I use a port other than 'a' in the -va or -ca... options?

A:  Suppose your central server was serving statistics for several client
boxes, but each of the boxes was running a different Half-Life modification:
CS on one box, HL on another, TFC on a third and OP4 on the fourth.  And 
suppose you didn't want all of the stats to be munged together; you want to 
keep them separate.  In this case, you must run four separate instances of 
PigStats on the server box, each with a different port identifier, as in
-va, -vb, -vc and -bd.  Each of the clients would use its own identifier, a,
b, c, or d, to match.
---

Q:  Can the client/server options span different operating systems?

A:  Yes.  Any x86 machine that runs PigStats can play.
---

Q:  I'm using the client/server options, but nothing gets through my router
or firewall.

A:  You may need to configure your router or firewall to enable UDP on certain 
ports.  If you're using the 'a' port, then you must enable UDP on ports 5027 
and 5057.  For 'b' port, it's 5028 and 5058, and so on (see a trend?).
---

Q:  Will the client/server options drag down my network bandwidth?

A:  No.  The PigStats client sends no more than five compressed log files 
every second, which amounts to such a small data rate that no reasonable 
network will ever notice the traffic.  What's more, since the log files are
compressed, there is little chance that a suspicious network administrator
will ever figure out exactly what it is that you're pumping over the wires.
---



***LICENSE*******************************************************************

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA  02111-1307, USA.


***SOURCE CODE***************************************************************

Full source code is available for pigstats under the terms of the GPL  and is
now included as part of the PigStats 1.4 installation.  To install the source
code, run the PigStats installer program and select the "custom installation"
option.  Then select the source code portion of the install and proceed.


***CONTACT*******************************************************************

Contact the author, Eric Dennison, at betterdead@planethalflife.com, or visit 
the lovely PigStats web site at www.planethalflife.com/pigstats.


***CREDITS*******************************************************************

The installer for this product was created using Installer VISE from
MindVision Software. For more information on Installer VISE, contact:
MindVision Software
7201 North 7th Street
Lincoln, NE 68521-8913


Voice: (402) 477-3269
Fax: (402) 477-1395
E-mail: mindvision@mindvision.com
http://www.mindvision.com


This project uses Quma Software's QVCS version control software Version 3.4.
This package provides unbelievable functionality for the price. Check it out!

Thanks to Valve Software for keeping me safely occupied for well over a year!

Thanks to SLAG, Blight, Space Ranger and everyone else who's shown an interest
and offered criticisms (you know who you are).

My deepest appreciation to everyone out there who ever downloaded PigStats and
tried it out, even when they didn't have the slightest clue what they were
doing.  Your kind words and encouragement have been more uplifting than you
can ever imagine!  Because of Half-Life I have met some of the nicest people
on the 'net - and killed them.

And finally, the time has come to recognize one courageous individual, who I 
will refer to simply as "andy", who installed the first release of 1.4a, 
decided he didn't like it, and deleted his entire half-life directory tree 
when he ran the uninstaller.  He could have trashed my name all over usenet.  
He could have mailed me a dead fish.  He could have put me on His List.  
Instead, he graciously helped me to identify the bug in my install script and 
fix the problem before anyone else found it.  To be honest, no beta software 
that I've tried ever did anything that bad to my computer.  But mine did it 
to his.  Andy:  I salute you.

BTW, the install/uninstall is pretty well bullet proof at this point and no 
one else in thousands of downloads has ever lost their Half-Life tree.  And, 
BTW, the fault was all mine, and not MindVision Software's.  Thank you!



******************************************************************************

***LINUX DISTRIBUTION*********************************************************

Installation:

Build PigStats by executing the script: pigbuild

Place the resulting executable, pigstats, in a directory referenced by your
path and away you go.  Rudimentary help is available at our web site:
www.planethalflife.com/pigstats, or by executing pigstats with no arguments.

Usage Exceptions:

!!All paths must end in '/' and no wildcards are allowed in the input path!!
       
Linux distribution file list:

avl.c           Gnu binary tree library (really good stuff!)
avl.h
avl.info
copying.txt     gpl license
pigbuild        build script
PigJS.c         JavaScript output routines
PigJS.h
PigParse.c      parsing routines
PigParse.h
PigStats.c      Main
PigStats.h
PigText.c       HTML and text output routines
PigText.h
PigUtil.c       utility routines
PigUtil.h
readme.txt      this file
resource.h
[HTML]          html documents for use with JavaScript output (-j option)