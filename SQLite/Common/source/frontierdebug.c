
/*	$Id$    */

/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

#include "frontier.h"
#include "standard.h"

#include "about.h"
#include "frontierdebug.h"
#include "process.h"
#include "processinternal.h"
#include "strings.h"


static const char debuglogname[] = "frontierdebuglog.txt";

static FILE * logfile = nil;

static boolean flreentering = false;


/* compiler messages for GENERAL category */

#if (LAND_GENERALLOG_LEVEL >= LAND_LOGLEVEL_3)
	#pragma message ("*********************** GENERAL LOG: Level 3")
#elif (LAND_GENERALLOG_LEVEL >= LAND_LOGLEVEL_2)
	#pragma message ("*********************** GENERAL LOG: Level 2")
#elif (LAND_GENERALLOG_LEVEL >= LAND_LOGLEVEL_1)
	#pragma message ("*********************** GENERAL LOG: Level 1")
/*
#else
	#pragma message ("*********************** GENERAL LOG: --OFF--")
*/
#endif

#if (LAND_GENERALLOG_LEVEL > LAND_LOGLEVEL_OFF)
	#if (LAND_GENERALLOG_TARGET & LAND_LOGTARGET_FILE)
		#pragma message ("*********************** GENERAL LOG: output to FILE")
	#endif
	#if (LAND_GENERALLOG_TARGET & LAND_LOGTARGET_ABOUT)
		#pragma message ("*********************** GENERAL LOG: output to ABOUT WINDOW")
	#endif
	#if (LAND_GENERALLOG_TARGET & LAND_LOGTARGET_DIALOG)
		#pragma message ("*********************** GENERAL LOG: output to DIALOG")
	#endif
	#if (LAND_GENERALLOG_TARGET & LAND_LOGTARGET_DEBUGGER)
		#pragma message ("*********************** GENERAL LOG: output to DEBUGGER")
	#endif
#endif


/* compiler messages for TCP category */

#if (LAND_TCPLOG_LEVEL >= LAND_LOGLEVEL_3)
	#pragma message ("*********************** TCP LOG: Level 3")
#elif (LAND_TCPLOG_LEVEL >= LAND_LOGLEVEL_2)
	#pragma message ("*********************** TCP LOG: Level 2")
#elif (LAND_TCPLOG_LEVEL >= LAND_LOGLEVEL_1)
	#pragma message ("*********************** TCP LOG: Level 1")
/*
#else
	#pragma message ("*********************** TCP LOG: --OFF--")
*/
#endif

#if (LAND_TCPLOG_LEVEL > LAND_LOGLEVEL_OFF)
	#if (LAND_TCPLOG_TARGET & LAND_LOGTARGET_FILE)
		#pragma message ("*********************** TCP LOG: output to FILE")
	#endif
	#if (LAND_TCPLOG_TARGET & LAND_LOGTARGET_ABOUT)
		#pragma message ("*********************** TCP LOG: output to ABOUT WINDOW")
	#endif
	#if (LAND_TCPLOG_TARGET & LAND_LOGTARGET_DIALOG)
		#pragma message ("*********************** TCP LOG: output to DIALOG")
	#endif
	#if (LAND_TCPLOG_TARGET & LAND_LOGTARGET_DEBUGGER)
		#pragma message ("*********************** TCP LOG: output to DEBUGGER")
	#endif
#endif


/* compiler messages for DB category */

#if (LAND_DBLOG_LEVEL >= LAND_LOGLEVEL_3)
	#pragma message ("*********************** DB LOG: Level 3")
#elif (LAND_DBLOG_LEVEL >= LAND_LOGLEVEL_2)
	#pragma message ("*********************** DB LOG: Level 2")
#elif (LAND_DBLOG_LEVEL >= LAND_LOGLEVEL_1)
	#pragma message ("*********************** DB LOG: Level 1")
/*
#else
	#pragma message ("*********************** DB LOG: --OFF--")
*/
#endif

#if (LAND_DBLOG_LEVEL > LAND_LOGLEVEL_OFF)
	#if (LAND_DBLOG_TARGET & LAND_LOGTARGET_FILE)
		#pragma message ("*********************** DB LOG: output to FILE")
	#endif
	#if (LAND_DBLOG_TARGET & LAND_LOGTARGET_ABOUT)
		#pragma message ("*********************** DB LOG: output to ABOUT WINDOW")
	#endif
	#if (LAND_DBLOG_TARGET & LAND_LOGTARGET_DIALOG)
		#pragma message ("*********************** DB LOG: output to DIALOG")
	#endif
	#if (LAND_DBLOG_TARGET & LAND_LOGTARGET_DEBUGGER)
		#pragma message ("*********************** DB LOG: output to DEBUGGER")
	#endif
#endif


/* compiler messages for THREADS category */

#if (LAND_THREADSLOG_LEVEL >= LAND_LOGLEVEL_3)
	#pragma message ("*********************** THREADS LOG: Level 3")
#elif (LAND_THREADSLOG_LEVEL >= LAND_LOGLEVEL_2)
	#pragma message ("*********************** THREADS LOG: Level 2")
#elif (LAND_THREADSLOG_LEVEL >= LAND_LOGLEVEL_1)
	#pragma message ("*********************** THREADS LOG: Level 1")
/*
#else
	#pragma message ("*********************** THREADS LOG: --OFF--")
*/
#endif

#if (LAND_THREADSLOG_LEVEL > LAND_LOGLEVEL_OFF)
	#if (LAND_THREADSLOG_TARGET & LAND_LOGTARGET_FILE)
		#pragma message ("*********************** THREADS LOG: output to FILE")
	#endif
	#if (LAND_THREADSLOG_TARGET & LAND_LOGTARGET_ABOUT)
		#pragma message ("*********************** THREADS LOG: output to ABOUT WINDOW")
	#endif
	#if (LAND_THREADSLOG_TARGET & LAND_LOGTARGET_DIALOG)
		#pragma message ("*********************** THREADS LOG: output to DIALOG")
	#endif
	#if (LAND_THREADSLOG_TARGET & LAND_LOGTARGET_DEBUGGER)
		#pragma message ("*********************** THREADS LOG: output to DEBUGGER")
	#endif
#endif


/* functions */

#ifdef WIN95VERSION
	extern DWORD ixthreadglobalsgrabcount;
#endif

static void logtofile (char *str, char *category) {

	unsigned long ticks = gettickcount ();
	static unsigned long lastticks = 0;
	#ifdef WIN95VERSION
		DWORD idthread = GetCurrentThreadId();
		static DWORD idlastthread = 0;
		long grabcount = (long) TlsGetValue (ixthreadglobalsgrabcount);
	#endif
	#ifdef MACVERSION
		long idthread = (long) (**getcurrentthread ()).idthread;
		static long idlastthread = 0;
	#endif

	if (logfile == NULL) {
		logfile = fopen (debuglogname, "a");
		}

	if (idthread != idlastthread) {
		fprintf (logfile, "\n");
		idlastthread = idthread;
		}

	#ifdef WIN95VERSION
		fprintf (logfile, "%08X (%04ld) | %04X (%02ld) | %s | &s\n", (unsigned long) ticks, (ticks - lastticks), idthread, grabcount, category, str);
	#endif

	#ifdef MACVERSION
		fprintf (logfile, "%08lX (%04ld) | %08lX | %s | %s\n", (unsigned long) ticks, (ticks - lastticks), idthread, category, str);
	#endif

	lastticks = ticks;

	fflush (logfile);
	}/*logtofile*/


static void logtoaboutwindow (char *str) {

	bigstring bs;
	
	copyctopstring (str, bs);

	aboutsetmiscstring (bs);
	}/*logtoaboutwindow*/


static void logtodialog (char *str) {
#pragma unused(str)

	}/*logtodialog*/


static void logtodebugger (char *str) {

	bigstring bs;
	
	copyctopstring (str, bs);
	
	DebugStr (bs);
	}/*logtodebugger*/


static void logtotargets (char *str, long targetflags, char *category) {

	if (flreentering)
		return;
	
	flreentering = true;
			
	if (targetflags & LAND_LOGTARGET_FILE)
		logtofile (str, category);

	if (targetflags & LAND_LOGTARGET_ABOUT)
		logtoaboutwindow (str);
		
	if (targetflags & LAND_LOGTARGET_DIALOG)
		logtodialog (str);
		
	if (targetflags & LAND_LOGTARGET_DEBUGGER)
		logtodebugger (str);
	
	flreentering = false;
	}/*logtotargets*/


void logmessage (char *msg, char *file, long line, long targetflags, char *category) {
	
	char str[400];
	
	str[0] = 0;
	
	sprintf (str, "%s [%s,%ld]", msg, file, line);
	
	logtotargets (str, targetflags, category);
	}/*logmessage*/
	

long logassert (char *expr, char *file, long line, long targetflags, char *category) {
	
	char str[400];
	
	str[0] = 0;
	
	sprintf (str, "Assertion failed: %s [%s,%ld]", expr, file, line);
	
	logtotargets (str, targetflags, category);
	
	return (0);
	}/*logassert*/


void logstartup () {

	/*perhaps get location of app here, so we can more precisely place our log file*/

	}/*logstartup*/


void logshutdown () {

	if (logfile != NULL)
		fclose (logfile);
	}/*DBTRACKERCLOSE*/





