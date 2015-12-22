
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

#ifdef MACVERSION
#include <land.h>
#include "mac.h"
#endif

#include "memory.h"
#include "dialogs.h"
#include "error.h"
#include "file.h"
#include "launch.h"
#include "kb.h"
#include "ops.h"
#include "strings.h"
#include "threads.h"
#include "timedate.h"

#include "lang.h"
#include "langinternal.h"
#include "shell.h"
#include "shellmenu.h"
#include "shellprivate.h"
#include "shellhooks.h"
#include "command.h"

#include "about.h"
#include "shell.rsrc.h"
#include "scripts.h"
#include "tablestructure.h"
#include "cancoon.h"
#include "process.h"
#include "processinternal.h"
#include "tableinternal.h"

#include "frontierdebug.h" //6.2b7 AR
#include "oplist.h" //6.2b11 AR
#include "langsystem7.h"


/*
#include <profile.h>
*/

hdlprocessrecord currentprocess = nil; /*process that's currently running*/

hdlprocessrecord newlyaddedprocess = nil;

unsigned short fldisableyield = 0;

boolean flthreadkilled = false;

boolean flcanusethreads = false;


#ifdef WIN95VERSION
	
static CRITICAL_SECTION processlistsection;

static boolean processlistsectioninitialized = false;


static void _entercriticalprocesssection (void) {

	if (!processlistsectioninitialized) {

		InitializeCriticalSection (&processlistsection);

		processlistsectioninitialized = true;
		}
	
	EnterCriticalSection (&processlistsection);
	}

static void _leavecriticalprocesssection (void) {

	LeaveCriticalSection (&processlistsection);
	}

#else

#define _entercriticalprocesssection()

#define _leavecriticalprocesssection()

#endif

#pragma pack(2)
typedef struct tythreadlist {
	
	hdlthreadglobals hfirst;
	} tythreadlist, **hdlthreadlist;
#pragma options align=reset


static hdlthreadlist processthreadlist;

/*
xxx maintain a pool of two threads at all times
*/

#define threadpoolsize 2

/*
static ThreadID threadpool [threadpoolsize] = {kNoThread, kNoThread}
*/


static hdlprocessthread agentthread = 0;

static boolean agentthreadsleeping = false;

#define getthreadglobals(hthread) hthread

static short ctprocessthreads = 0;


static hdlprocesslist processlist = nil; /*the current process list*/

static hdlprocesslist agentprocesslist = nil; /*the process list that agents came from*/

static boolean flprocesscodedisposed = false; /*was process dispose code called?*/

static hdltreenode honeshotcode = nil; /*used as a semaphore and real data*/

static boolean flagentsenabled = true; /*the user's setting*/

static int flagentsdisabled = 0; /*temporary, internal disable*/

static boolean flpostedmemorymessage = false;

static hdlthreadglobals hthreadglobals = nil;

static typrocessstack processstack = {0};

static boolean flpostedthreadsmessage = false;

static boolean flcreatedagentprocess = false;

static boolean flexitingthread = false;

static boolean flinhibiterrorclear = false; // avoid adding a parm to several layers of global functions

static long processonehottimeslice = 6;

static long processagenttimeslice = 4;


boolean setagentsenable (boolean flagents) {
	
	/*
	call this routine to permanently change the agents setting.  this is not 
	a push or a pop -- it's the global setting
	*/
	
	langsetuserflag (idagentsenabledscript, flagents);
	
	flagentsenabled = flagents;
	
	return (true);
	} /*setagentsenable*/


boolean pushprocess (register hdlprocessrecord hp) {
	
	/*
	4.1b3 dmb: carry new hthread field from current process to new process

	5.0a11 dmb: don't dereference nil hp to grab hthread field
	*/
	
	typrocessstackrecord item;
	
	if (processstack.top >= ctprocesses) {
		
		shellinternalerror (idprocessstackfull, BIGSTRING ("\x17" "process stack overflow!"));
		
		return (false);
		}
	
	if ((currentprocess != nil) && (hp != nil))
		(**hp).hthread = (**currentprocess).hthread;
	
	item.hprocess = currentprocess;
	
	item.errormessagecallback = langcallbacks.errormessagecallback;
	
	item.debugerrormessagecallback = langcallbacks.debugerrormessagecallback;
	
//	item.clearerrorcallback = langcallbacks.clearerrorcallback;
	
	item.herrorstack = langcallbacks.scripterrorstack;
	
	item.htablestack = hashtablestack;
	
	processstack.stack [processstack.top++] = item;
	
	currentprocess = hp;
	
	if (hp != nil) {
		
		langcallbacks.errormessagecallback = (**hp).errormessagecallback;
		
		langcallbacks.debugerrormessagecallback = (**hp).debugerrormessagecallback;
		
	//	langcallbacks.clearerrorcallback = (**hp).clearerrorcallback;
		
		langcallbacks.scripterrorstack = (**hp).herrorstack;
		
		hashtablestack = (**hp).htablestack;
		}
	
	return (true);
	} /*pushprocess*/


boolean popprocess (void) {
	
	typrocessstackrecord item;
	
	assert (processstack.top > 0);
	
	item = processstack.stack [--processstack.top];
	
	currentprocess = item.hprocess;
	
	langcallbacks.scripterrorstack = item.herrorstack;
	
	langcallbacks.errormessagecallback = item.errormessagecallback;
	
	langcallbacks.debugerrormessagecallback = item.debugerrormessagecallback;
	
//	langcallbacks.clearerrorcallback = item.clearerrorcallback;
	
	hashtablestack = item.htablestack;
	
	return (true);
	} /*popprocess*/


void disposeprocess (hdlprocessrecord hprocess) {
	
	/*
	8/26/90 DW: we don't dispose of the code tree anymore -- it's up to scripts.c
	to dispose of it when the user installs a new version of the script.
	
	8/28/90 DW: this is a source of a memory leak.  new policy -- dispose of the code
	tree if it is a one-shot process.  does this make sense?  well -- if the process
	is one-shot then it must come from someone who wants us to manage the code, the
	guy is no longer around to dispose of it.  check it out, oneshots come from the
	menubar, and from the IPC back-door.
	
	12/27/90 dmb: if it's the current process, clear global
	
	1/8/91 dmb: instead of doing above (12/27/90), assert that process list 
	protection should avoid deletion of a running process
	
	5.0.2b15 dmb: profiling
	*/
	
	register hdlprocessrecord hp = hprocess;
	tyvaluerecord val;
	
	assert (!(**hp).flrunning);
	
	if ((**hp).floneshot)
		langdisposetree ((**hp).hcode);
	
	langdisposetree ((**hp).holdcode);
	
	disposehandle ((Handle) (**hp).htablestack);
	
	disposehandle ((Handle) (**hp).herrorstack);
	
	if ((**hp).hprofiledata != nil) {
		
		assert ((**hp).flprofiling);
		
		setexternalvalue ((Handle) (**hp).hprofiledata, &val);
		
		disposevaluerecord (val, false);
		}
	
	disposehandle ((Handle) hp);
	} /*disposeprocess*/


boolean newprocess (hdltreenode hcode, boolean floneshot, langerrorcallback errorcallback, long errorrefcon, hdlprocessrecord *hprocess) {
	
	/*
	create a new process with the indicated attributes
	
	12/25/91 dmb: dispose all handles on error
	
	1/2/92 dmb: check result of pushprocess

	5.0a22 dmb: xxxadded clearerrorcallback to process record, and flinhibiterrorclear
				xxxrenamed process's errorcallback field to errormessagecallback
	*/
	
	register hdlprocessrecord hp;
	hdlerrorstack herrorstack;
	hdltablestack htablestack;
	
	if (!newclearhandle (sizeof (typrocessrecord), (Handle *) hprocess))
		return (false);
	
	hp = *hprocess; /*copy into register*/
	
	if (!newclearhandle (sizeof (tyerrorstack), (Handle *) &herrorstack)) {
		
		disposehandle ((Handle) hp);
		
		return (false);
		}
	
	if (!newclearhandle (sizeof (tytablestack), (Handle *) &htablestack)) {
		
		disposehandle ((Handle) hp);
		
		disposehandle ((Handle) herrorstack);
		
		return (false);
		}
	
	(**hp).hcode = hcode;
	
	(**hp).floneshot = floneshot;
	
	(**hp).errormessagecallback = &langerrordialog;

	(**hp).debugerrormessagecallback = (langerrormessagecallback) &truenoop;
	
	(**hp).htablestack = htablestack;
	
	(**hp).herrorstack = herrorstack;
	
	(**hp).processstartedroutine = (langvoidcallback) &truenoop;
	
	(**hp).processkilledroutine = (langvoidcallback) &truenoop;
	
	if (!pushprocess (hp)) {
		
		disposeprocess (hp);
		
		return (false);
		}
	
	langpusherrorcallback (errorcallback, errorrefcon);
	
	popprocess ();
	
	#if 1
	{
	hdlhashtable htable;
	bigstring bsname;
	
	setemptystring (bsname);
	
	if (errorcallback != nil)
		(*errorcallback) (errorrefcon, 0, 0, &htable, bsname);
	
	if (isemptystring (bsname))
		langgetstringlist (anomynousthreadstring, bsname);

	copystring (bsname, (**hp).bsname);
	}
	#endif

	if (!floneshot)
		flcreatedagentprocess = true; /*for thread error reporting*/
	
	return (true);
	} /*newprocess*/


static boolean deletefromprocesslist (hdlprocessrecord hprocess) {
	
	/*
	remove the process record from the process list.  don't dispose of 
	anything
	*/
	
	register hdlprocessrecord hp = hprocess;
	boolean fl;
	
	_entercriticalprocesssection();

	fl = listunlink ((hdllinkedlist) (**hp).hprocesslist, (hdllinkedlist) hp);

	_leavecriticalprocesssection();
	
	if (!fl)
		return (false);
	
	(**hp).hprocesslist = nil; /*it's not in a list anymore*/
	
	return (true);
	} /*deletefromprocesslist*/


static boolean deleteprocess (hdlprocessrecord hprocess) {
	
	deletefromprocesslist (hprocess);
	
	disposeprocess (hprocess);
	
	return (true);
	} /*deleteprocess*/


boolean newprocesslist (hdlprocesslist *hlist) {
		
	return (newclearhandle (sizeof (typrocesslist), (Handle *) hlist));
	} /*newprocesslist*/


void setcurrentprocesslist (hdlprocesslist hlist) {
	
	_entercriticalprocesssection ();

	processlist = hlist;

	_leavecriticalprocesssection ();
	} /*setcurrentprocesslist*/


boolean disposeprocesslist (hdlprocesslist hprocesslist) {
	
	register hdlprocesslist hlist = hprocesslist;
	register hdlprocessrecord hp;
	register hdlprocessrecord hnext;
	register boolean flbusy;
	
	if (hlist == nil) /*empty list*/
		return (true);
	
	flbusy = (**hlist).ctrunning > 0; /*something in list is running now*/
	
	hp = (**hlist).hfirstprocess;
	
	while (hp != nil) {
		
		hnext = (**hp).hnextprocess;
		
		if (flbusy)
			(**hp).fldisposewhenidle = true;
		else
			disposeprocess (hp);
		
		hp = hnext; /*advance to next node*/
		} /*while*/
	
	if (flbusy)
		(**hlist).fldisposewhenidle = true;
	else
		disposehandle ((Handle) hlist);
	
	return (true);
	} /*disposeprocesslist*/


boolean processfindcode (hdltreenode hcode, hdlprocessrecord *hprocess) {
	
	/*
	search the process list for a record that points to the indicated code.
	*/
	
	register hdlprocessrecord x;
	register hdltreenode hsearch = hcode;
	
	if (processlist == nil)
		return (false);
	
	x = (**processlist).hfirstprocess;
	
	while (x != nil) { /*list is nil-terminated*/
		
		if ((**x).hcode == hsearch) { /*perform replacement*/
			
			*hprocess = x;
			
			return (true);
			}
		
		x = (**x).hnextprocess;
		} /*while*/
	
	return (false);
	} /*processfindcode*/


static boolean flvisitingthreads = false; // *** debug

static boolean visitprocessthreads (pascal boolean (*visit) (hdlthreadglobals, long), long refcon) {
	
	/*
	visit all process threads until the visit routine returns true

	6.26.97 dmb: keep globals in sync for current thread. visit routine must
	no longer operate directly on globals
	*/
	
	register hdlthreadglobals hg;
	hdlthreadglobals hnext, hprev = NULL;
	boolean fl = false;
	
#ifdef landinclude
	if (landvisitsleepingthreads ((landqueuepopcallback) visit, refcon))
		return (true);
#endif
	
	flvisitingthreads = true;

	for (hg = (**processthreadlist).hfirst; hg != nil; hg = hnext) {
		
		hnext = (**hg).hnextglobals;

		if (hg == hthreadglobals)
			copythreadglobals (hg);

		fl = (*visit) (hg, refcon);
		
		if (hg == hthreadglobals)
			swapinthreadglobals (hg);
		
		if (fl)
			break;

		hprev = hg;
		}
	
	flvisitingthreads = false;
	
	return (fl);
	} /*visitprocessthreads*/


boolean processkill (hdlprocessrecord hprocess) {
	
	/*
	8/31/92 dmb: wake up the thread if necessary
	
	4.1b3 dmb: use wakeprocessthread to wake any sleeping thread, not 
	just threads waiting for AE replies (managed by landsystem7)
	*/
	
	register hdlprocessrecord hp = hprocess;
	
	if (hp == nil) /*defensive driving*/
		return (false);
	
	if ((**hp).flscheduled || (**hp).flrunning) {
	
		(**hp).fldisposewhenidle = true;
		
		wakeprocessthread ((**hp).hthread);
		}
	else {
		(*(**hp).processkilledroutine) (); /*6.13.97 dmb*/
		
		deleteprocess (hp);
		}
	
	return (true);
	} /*processkill*/


static pascal boolean codedisposedvisit (hdlthreadglobals hthread, long refcon) {
	
	/*
	search the thread's error stack for a refcon that matches the one 
	of the disposed code.  if one is found, kill the thread's process 
	and return true.
	
	2.1b1 dmb: never return true; there may be more threads to kill
	
	2.1b13 dmb: for runtime, check the process' code before going thru 
	the stack.

	5.1.3 dmb: check for nil process. I notice that this is weird; we're 
	walking the thread list, looking for processes, while other similar 
	routines walk the processlist. the role of the process list might 
	need some rethinking
	*/
	
	register hdlthreadglobals hg = getthreadglobals (hthread);
	register hdlerrorstack hs;
	short ix, ixtop;
	
	if ((**hg).hprocess == nil) // 5.1.3 dmb: if nil, error stack isn't valid
		return (false);
	
	hs = (**hg).langcallbacks.scripterrorstack;
	
	if (hs == nil)
		return (false);
	
	ixtop = (**hs).toperror;
	
	for (ix = 0; ix < ixtop; ++ix) {
		
		if ((**hs).stack [ix].errorrefcon == refcon) {
			
			(**hg).flthreadkilled = true; /*maim it*/
			
			(**hg).flretryagent = true;
			
			break;
			}
		}
	
	return (false); /*keep visiting*/
	} /*codedisposedvisit*/


void processcodedisposed (long refcon) {
	
	/*
	disable all process threads who have code with the indicated refcon
	in their runtime stack (the error stack).
	
	it might be better to generate an error instead of allowing this to 
	occur, but for now we don't
	
	7/25/92 dmb: must check current thread for code, and kill the script if 
	found.

	6.26.97 dmb: visitprocessthreads now handles the current thread fine; no 
	need to call langfinderrorrefcon directly too.
	*/
	
	visitprocessthreads (&codedisposedvisit, refcon); /*terminates if visit proc returns true*/
	} /*processcodedisposed*/


boolean processreplacecode (hdltreenode holdcode, hdltreenode hnewcode) {
	
	/*
	search the process list for a record that points to the indicated "old" code.
	
	if we find it, replace with the new code handle.
	
	search the whole list -- allow for possibility of fork verb someday that
	clones a process.
	
	3/12/91 dmb: if process is sleeping, wake it up to let new code run
	
	9/25/91 dmb: even when old code isn't found, call the codereplacedcallback
	*/
	
	register hdlprocessrecord x;
	register boolean flreplaced = false;
	register hdltreenode hsearch = holdcode;
	
	if (processlist == nil)
		return (false);
	
	x = (**processlist).hfirstprocess;
	
	while (x != nil) { /*list is nil-terminated*/
		
		if ((**x).hcode == hsearch) { /*perform replacement*/
			
			if ((**x).flrunning)
				(**x).holdcode = hsearch;
			else
				langdisposetree (hsearch);
			
			(**x).hcode = hnewcode;
			
			(**x).sleepuntil = 0; /*wake agent up if sleeping*/
			
			flreplaced = true;
			
			/*
			(*langcallbacks.codereplacedcallback) (hsearch, hnewcode);
			*/
			}
		
		x = (**x).hnextprocess;
		} /*while*/
	
	(*langcallbacks.codereplacedcallback) (hsearch, hnewcode); /*always notify callback*/
	
	return (flreplaced);
	} /*processreplacecode*/


boolean processdisposecode (hdltreenode hcode) {
	
	/*
	search the process list for a record that points to the indicated code.
	
	if we find it, dispose of the record and the code, and return true.
	
	search the whole list -- allow for possibility of fork verb someday that clones
	a process.
	
	1/8/91 dmb: if the process turns out to be running, generate an assert failure 
	for now.  we could possibly generate an langerror if neccessary.  the only 
	way I can see this happening is if the user (or a script) deletes a running 
	script from a table in the background -- possibly easy to do while debugging, 
	but also possibly a well-deserved crash!
	
	7/2/92 dmb: we've come a long way since 1/8/91; there's enough action going on 
	in the background that we have to handle the case where the code is running 
	without crashing -- into the debugger or otherwise.  fortunately, we have a nice 
	mechanisms for this now: processkill.
	*/
	
	register hdlprocessrecord x;
	register hdlprocessrecord hnext;
	register boolean flfound = false;
	register hdltreenode hsearch = hcode;
	
	if (processlist == nil)
		return (false);
	
	x = (**processlist).hfirstprocess;
	
	while (x != nil) { /*list is nil-terminated*/
		
		hnext = (**x).hnextprocess;
		
		if ((**x).hcode == hsearch) { /*remove it*/
			
			processkill (x); /*dispose or mark as killed for later disposal*/
			
			/*
			assert (!(**x).flrunning); /%if this can happen, maybe generate langerror?%/
			
			deletefromprocesslist (x);
			
			disposehandle ((Handle) x);
			*/
			
			flprocesscodedisposed = true;
			
			flfound = true;
			}
		
		x = hnext;
		} /*while*/
	
	return (flfound);
	} /*processdisposecode*/


boolean processagentsleep (long ctseconds) {
	
	/*
	2/12/92 dmb: added flsleepinbackground logic
	
	4/21/92 dmb: removed flsleepinbackground logic -- too ugly
	
	8/14/92 dmb: handle overflow. also ensure that negative value causes no sleep.
	*/
	
	register hdlprocessrecord hp = currentprocess;
	register unsigned long x;
	unsigned long now;
	
	if (hp == nil) 
		return (false);
	
	/*
	fldontbackground = ctseconds < 0;
	
	if (fldontbackground)
		ctseconds = -ctseconds;
	*/
	
	now = timenow ();
	
	if (ctseconds < 0)
		x = now;
	
	else {
		x = 0xFFFFFFFF;
		
		if ((unsigned) ctseconds < (x - now)) /*check for less-than-infinite sleep, avoiding overflow*/
			x = now + ctseconds;
		}
	
	(**hp).sleepuntil = x;
	
	/*
	(**hp).flsleepinbackground = fldontbackground;
	*/
	
	return (true);
	} /*processagentsleep*/


boolean processisoneshot (boolean flnilok) {
	
	/*
	the interpreter is allowed to know if it's running a one-shot process or not.
	
	example: if it's a one shot process, then cmd-period will allow it to be killed.
	
	9/20/90 dmb:  check currentprocess for nil
	
	1/3/90 dmb: added flnilok parameter.  if true, nil process is considered one-shot
	*/
	
	if (currentprocess == nil)
		return (flnilok);
	
	return ((**currentprocess).floneshot);
	} /*processisoneshot*/


static boolean processbackgroundtask (boolean flresting) {
	
	/*
	return true if background tasks should be given a shot while 
	interpreting a script.
	
	7/4/91 dmb: don't background while langerror is disabled.  (otherwise, 
	would need to save in thread state)
	*/
	
	if (langerrorenabled ())
		if (flagentsenabled || processisoneshot (false))
			return (scriptbackgroundtask (flresting));
	
	return (true);
	} /*processbackgroundtask*/


boolean debuggingcurrentprocess (void) {
	
	register hdlprocessrecord hp = currentprocess;
	
	if (hp == nil)
		return (false);
	
	if (!(**hp).floneshot) /*no debugging inside background processes*/
		return (false);
	
	return ((**hp).fldebugging);
	} /*debuggingcurrentprocess*/


boolean processstartprofiling (boolean fltimesliced) {

	/*
	5.0.2b15 dmb: enable profiling. create a new table if needed. (profiling can 
	be "paused" without losing data)
	*/
	
	hdlprocessrecord hp = currentprocess;
	tyvaluerecord val;
	hdltablevariable hvariable;
	short errcode;
	
	if (hp == nil)
		return (false);
	
	(**hp).flprofiling = true;
	
	(**hp).flprofilesliced = fltimesliced;
	
	if ((**hp).hprofiledata == nil) {// create table to store data

		if (!langexternalnewvalue (idtableprocessor, nil, &val))
			return (false);
		
		gettablevariable (val, &hvariable, &errcode);
		
		(**hp).hprofiledata = (hdlexternalvariable) hvariable;
		}
	
	langstartprofiling ();
	
	return (true);
	} /*processstartprofiling*/


boolean processstopprofiling (void) {

	/*
	5.0.2b15 dmb: disable profiling. don't automatically throw away the table, 
	so that profiling can be "paused" without losing data.
	*/
	
	hdlprocessrecord hp = currentprocess;
	
	if (!hp || !(**hp).flprofiling)
		return (false);
	
	langstopprofiling ();
	
	(**hp).flprofiling = false;
	
	return (true);
	} /*processstopprofiling*/


boolean profilingcurrentprocess (void) {
	
	register hdlprocessrecord hp = currentprocess;
	
	if (hp == nil)
		return (false);
	
	return ((**hp).flprofiling);
	} /*profilingcurrentprocess*/


static boolean processdebugger (hdltreenode hnode) {
	
	/*
	1/8/91 dmb: hopefully, at this point, we no longer have to disable 
	agents before entering the debugger
	*/
	
	if (!debuggingcurrentprocess ())
		return (true);
	
	return (scriptdebugger (hnode)); /*pass the message to the script editor*/
	} /*processdebugger*/


static boolean processscriptkilled (boolean flchecknow) {
	
	/*
	returns true if the current running script should be killed by the
	interpreter.  it's called back from within the interpreter.
	
	7/2/91: this used to be in scripts.c, but is now more appropriate 
	at this level.  the debugger takes care of it's own killing
	
	9/27/92 dmb: added flichecknow parameter
	*/
	
	register hdlprocessrecord hp = currentprocess;
	
	if (flthreadkilled)
		return (true);
	
	if ((hp != nil) && (**hp).fldisposewhenidle)
		return (true);
	
	if (!flchecknow || fldisableyield)
		return (false);
	
	if (processisoneshot (true)) { /*cmd-period doesn't kill background tasks*/
		
		if (keyboardescape ()) { /*user pressed cmd-period or something like that*/
		
			keyboardclearescape (); /*consume it*/
			
			return (true);
			}
		}
	
	return (false);
	
	/*
	return (scriptkilled ());
	*/
	} /*processscriptkilled*/


static boolean processpushsourcecode (hdlhashtable htable, hdlhashnode hnode, bigstring bsname) {
	
	return (scriptpushsourcecode (htable, hnode, bsname));
	} /*processpushsourcecode*/


static boolean processpopsourcecode (void) {
	
	return (scriptpopsourcecode ());
	} /*processpopsourcecode*/


static boolean processpushtable (hdlhashtable *htable) {
	
	if (!debuggingcurrentprocess ())
		return (langdefaultpushtable (htable));
	
	return (scriptpushtable (htable));
	} /*processpushtable*/


static boolean processpoptable (hdlhashtable htable) {
	
	if (!debuggingcurrentprocess ())
		return (langdefaultpoptable (htable));
	
	return (scriptpoptable (htable));
	} /*processpoptable*/


boolean goodthread (hdlprocessthread hthread) {
	
	/*
	make sure that the thread hasn't been killed.  (see killdependentprocesses)
	*/
	
	register hdlthreadglobals hp = (hdlthreadglobals) hthread;
	
	if (hp == nil)
		return (false);
	
	return (!(**getthreadglobals (hp)).flthreadkilled);
	} /*goodthread*/


boolean ingoodthread (void) {
	
	return (goodthread (getcurrentthread ()));
	} /*ingoodthread*/


#ifndef version42orgreater

boolean inmainthread (void) {
	
	/*
	2.1a7 dmb: the main thread's true id isn't idapplicationthread, so 
	check the id stored in the current globals record instead
	
	5.0b17 dmb: handle nil globals
	*/
	
	return (hthreadglobals && (**hthreadglobals).idthread == idapplicationthread);
	
	/*
	ThreadID id;
	
	if (!flcanusethreads)
		return (true);
	
	if (GetCurrentThread (&id) != noErr)
		return (true);
	
	return (id == idapplicationthread);
	*/
	} /*inmainthread*/

#endif


boolean infrontierthread (void) {
	
	/*
	3.0.4b8 dmb: return true if we're in _any_ frontier thread, as 
	opposed to an osaclient process
	*/
	
	return (!hthreadglobals || (**hthreadglobals).idthread);
	} /*infrontierthread*/


boolean processsleep (hdlprocessthread hthread, unsigned long timeout) {
	
	/*
	sleep until another thread wakes us.  when control returns, call 
	ingoodthread to verify that we haven't been maimed while sleeping
	
	4.1b5 dmb: make sure that TickCount + timeout don't overflow unsigned long.
	
	5.1.5b16 dmb: like threads.c, we now accept nil to mean the current thread.

	6.2b7 AR: Properly handle wrapping around of tick count (happens every 16.5 days on Win32).
	If timeout is 0xffffffff, processchecktimeouts will never wake the thread.
	*/
	
	unsigned long timetowake;
	boolean fl;
	unsigned long ticks = gettickcount ();
	
	if (hthread == nil) //sleep the current thread
		hthread = hthreadglobals;
	
	if (processissleeping (hthread)) {
	
		if (hthread == agentthread)
			agentthreadsleeping = false;	/*no longer in standard, every-second sleep*/
		
		return (true);
		}
	
	if (((long) timeout >= 0) && (ticks + timeout > timeout)) /*positive, not wrapping around*/
		timetowake = ticks + timeout;
	else
		timetowake = 0xffffffff;
	
	(**(hdlthreadglobals) hthread).timetowake = timetowake;
	
	(**(hdlthreadglobals) hthread).sleepticks = timeout;

	(**(hdlthreadglobals) hthread).timebeginsleep = ticks;

	#if flruntime
	
		++ctsleepingthreads;
	
	#else
		
		if (processlist != nil)
			++(**processlist).ctsleeping;
		
	#endif
	
	if (hthread == hthreadglobals)
		fl = threadsleep (nil);
	else
		fl = threadsleep ((**(hdlthreadglobals) hthread).idthread);
	
	return (fl && goodthread (hthread));
	} /*processsleep*/


boolean processissleeping (hdlprocessthread hthread) {
	
	/*
	see if the thread is sleeping
	
	5.0a19 dmb: use our own data to find the answer; don't know
	now to query the thread's state directly (or if it's possible)
	*/
	
	if (hthread == nil)
		return (false);
	
	if ((hthread == agentthread) && agentthreadsleeping)
		return (true);

	return ((**(hdlthreadglobals) hthread).timetowake != 0);
	} /*processissleeping*/


boolean processwake (hdlprocessthread hthread) {
	
	/*
	wake the thread

	5.0b12 dmb: do the thread releasing here, after we've made
	sure that the thread is actually asleep. otherwise, it can 
	die after we release, before we kill it. also, we must update
	the data structure (timetowake) before we wake it. it may 
	disappear immediately

	6.2b11 AR: We sometimes see threads hanging spontaneously (e.g. on www.weblogs.com).
	Our theory is that for some reason, the first attempt at waking the thread fails.
	But since we reset the timetowake field before we attempt to wake the thread,
	we never get a second chance at waking it. Our data structures tell us its alive
	when its actually still sleeping.
	Now, we only reset the timetowake field if we successfully woke the thread.
	Since we no longer release the global semaphore, the thread can no longer die
	before we've reset the timetowake field. It won't have a chance to die until
	our caller eventually releases releases the global semaphore.
	*/
	
	boolean fl;
	hdlthread idthread;
	
	if (!processissleeping (hthread))
		return (false);

/*	(**(hdlthreadglobals) hthread).timetowake = 0; //first -- 6.2b11 AR: commented out
	
	(**(hdlthreadglobals) hthread).sleepticks = 0;

	(**(hdlthreadglobals) hthread).timebeginsleep = 0;
*/
	idthread = (**(hdlthreadglobals) hthread).idthread;

	//releasethreadglobals (); /*6.1b5 AR: commented out*/
	//releasethreadglobalsnopriority ();/*6.1b12 AR;  commented out again in 6.2b8 AR*/

	fl = threadwake (idthread, hthread != agentthread);
	
	//grabthreadglobalsnopriority ();
	//grabthreadglobals ();
	
	if (fl) {

		(**(hdlthreadglobals) hthread).timetowake = 0; //6.2b11 AR: only if successful
		
		(**(hdlthreadglobals) hthread).sleepticks = 0;

		(**(hdlthreadglobals) hthread).timebeginsleep = 0;
		
		#if flruntime
		
			--ctsleepingthreads;
		
		#else
			
			if (processlist != nil)
				--(**processlist).ctsleeping;
			
		#endif
		}
	
	THREADS_ASSERT_1 (fl);
	
	return (fl);
	} /*processwake*/


boolean processyield (void) {
	
	/*
	yield to other process threads.  when control returns, check the first 
	long in the userbytes array to verify that we haven't been maimed by 
	the loss of our globals.  (see killdependentprocesses)
	
	8/21/91 dmb: preserve shellwindow by popping it off the stack, pushing 
	it back on after we've yielded.  for now, we only handle a single window on 
	the stack; it we can't assume this, it may be reasonable to refuse to yield 
	if there are more globals pushed
	
	11/6/92 dmb: must check for nil processlist on pre-yield reference too

	4.30.97 dmb: big change for the PC: the main thread never needs to call 
	threadyield. why? because if the main thread is running a script, 
	fldisableyield is always true. if it's _not_ running a script, it isn't 
	using any of the thread globals, so it shouldn't block waiting to possess 
	them. this implies that, to run a script, the main thread must first 
	grab the thread globals semaphore.
	*/
	
	WindowPtr oldwindow;
	boolean flscriptwasrunning;
	
	if (fldisableyield)
		return (true);
	
	if (!flcanusethreads)
		return (true);
	
	if (!infrontierthread()) {	// should never happen
		
		#ifdef fldebug
		DebugStr (BIGSTRING ("\x0c" "don't yield!"));
		#endif
		
		return (true);
		}
	
	/*
	if (_profile)
		return (true);
	*/
	
	#if flruntime
	
		if (flscriptresting)
			++ctsleepingthreads;
	
	#else
	
		oldwindow = shellwindow;
		
		flscriptwasrunning = flscriptrunning;
		
		if (flscriptresting && (processlist != nil))
			++(**processlist).ctsleeping;
		
	#endif
	
	threadyield (flscriptresting);
	
	#if flruntime
	
		if (flscriptresting)
			--ctsleepingthreads;
		
	#else
	
		if (flscriptresting && (processlist != nil))
			--(**processlist).ctsleeping;
		
		assert ((shellwindow == oldwindow) || (shellwindow == nil));
		
		assert (flscriptwasrunning == flscriptrunning);
	
	#endif
	
	return (ingoodthread ());
	} /*processyield*/


boolean processyieldtoagents (void) {
	
	/*
	yield the processor until the current agent thread has completed.
	
	call this routine when you want to do something that doesn't sit well 
	with agents, like throwing away code trees.
	
	10/23/91 dmb: if we're an agent, return true so caller doesn't think 
	yield actually failed (in which case it would terminate).
	
	6/1/93 dmb: save/restore flagentsenabled instead of using agentsdisable, 
	with was never really doing anything anyway.

	5.0a25 dmb: if agent thread is nil, we're done
	*/
	
	boolean fl = true;
	
	if (fldisableyield)
		return (true);
	
	if (!processisoneshot (true)) /*we're an agent!*/
		return (true);
	
	if (agentthread == nil)
		return (true);
	
	#if flruntime
	
		fl = processyield ();
	
	#else
		
		++flagentsdisabled;
		
		wakeprocessthread (agentthread);
		
		while (agentthread != nil) {
			
		//	#ifdef MACVERSION
			if (!processyield ()) {
				
				fl = false;
				
				break;
				}
		//	#endif
			}
		
		--flagentsdisabled;
	
	#endif
	
	return (fl);
	} /*processyieldtoagents*/


void disposethreadglobals (hdlthreadglobals hglobals) {
	
	/*
	3.0.4b8 dmb: if this is hthreadglobals, set it to nil
	*/
	
	register hdlthreadglobals hg = hglobals;
	
	if (flvisitingthreads)
		assert (false);
	else
		listunlink ((hdllinkedlist) processthreadlist, (hdllinkedlist) hg);
	
	if (hg == hthreadglobals) {

		#ifdef WIN95VERSION
			if ( flcominitialized )
				shutdownCOM();
		#endif
		
		disposehandle ((Handle) hashtablestack);
		
		hashtablestack = nil;
		
		hthreadglobals = nil;

		/*
		//6.1b4 AR: Prevent us from crashing after the serial number dialog was cancelled
		
		assert (!shellwindow);
		
		assert (!outlinedata && !topoutlinestack);
		*/
		
		/*
		while (shellpopglobals ())
			;

		while (oppopoutline ())
			;

		opsetoutline (nil);
		*/
		}
	else {
		
		assert (!(**hg).shellwindow);

		assert (!(**hg).outlinedata && !(**hg).topoutlinestack);

		disposehandle ((Handle) (**hg).htablestack);
		}
	
	disposehandle ((Handle) hg);
	} /*disposethreadglobals*/


boolean newthreadglobals (hdlthreadglobals *hglobals) {
	
	register hdlthreadglobals hg;
	hdltablestack htablestack;
#ifdef landinclude
	tyapplicationid id;
#endif
	
	if (!newclearhandle (sizeof (tythreadglobals), (Handle *) hglobals))
		return (false);
	
	hg = *hglobals;
	
	if (!newclearhandle (sizeof (tytablestack), (Handle *) &htablestack)) {
		
		disposehandle ((Handle) hg);
		
		return (false);
		}
	
	(**hg).htablestack = htablestack;
	
	(**hg).langcallbacks = langcallbacks;
	
	(**hg).timesliceticks = processonehottimeslice;

	/*
		2006-04-24 smd
		has COM been initialized in this thread?
	*/
	(**hg).flcominitialized = false;

#ifdef landinclude	
	id = getprocesscreator ();
	
	(**hg).applicationid = id;
	
	(**hg).eventcreatecallback = &landsystem7defaultcreate;
	
	(**hg).eventsendcallback = &landsystem7defaultsend;
	
	(**hg).eventsettings.timeoutticks = kNoTimeOut;
#endif

	listlink ((hdllinkedlist) processthreadlist, (hdllinkedlist) hg);
	
	return (true);
	} /*newthreadglobals*/


hdlthreadglobals getcurrentthreadglobals (void) {
	
	return (hthreadglobals);
	} /*getcurrentthreadglobals*/

#define flprofile
#ifdef flprofile
static unsigned long totalticksin = 0;
static unsigned long totalticksout = 0;
static unsigned long lastswapticks = 0;
#endif

void copythreadglobals (hdlthreadglobals hglobals) {
	
	/*
	6/27/91 dmb: once this stuff has settled down, we should probably define 
	a global thread record that contains all the globals that need to be 
	swapped, instead of doing it piecemeal like this.
	
	7/15/91 dmb: now use threadglobals record
	
	1/28/92 dmb: processstack and multiple error hooks are now maintained in context
	
	5.1.5b10 dmb: don't set shellwindow, etc. unless context has been swapped in
	*/
	
	register hdlthreadglobals hg = hglobals;
#ifdef landinclude
	register hdllandglobals hlg;
#endif

	assert (!flvisitingthreads || hg == hthreadglobals);

	#ifdef flprofile
	if (cancoonglobals && !flvisitingthreads) {	
		unsigned long ticks;
	
		aboutsetthreadstring (nil, false);

		#ifdef MACVERSION
		ticks = gettickcount ();
		#else
		ticks = GetTickCount ();
		#endif

		totalticksin += ticks - lastswapticks;
		lastswapticks = ticks;
		}

	#endif
	
	(**hg).hccglobals = cancoonglobals;
	
	if ((**hg).timestarted != 0) { /*context has been swapped in*/
		
		(**hg).hprocess = currentprocess;
		
		(**hg).htable = currenthashtable;
		
		(**hg).flthreadkilled = flthreadkilled;
		
		(**hg).fldisableyield = fldisableyield;
		
		(**hg).htablestack = hashtablestack;
		
		(**hg).processstack = processstack;
		
		(**hg).globalsstack = globalsstack;
		
		(**hg).flscriptrunning = flscriptrunning;
		
		(**hg).flscriptresting = flscriptresting;
		
		(**hg).cterrorhooks = cterrorhooks;
		
		moveleft (errorhooks, (**hg).errorhooks, sizeof (errorhookcallback) * maxerrorhooks);
		
		(**hg).shellwindow = shellwindow;
		
	//	if (shellwindow == nil && topoutlinestack == 0) // if we're not working in a window, outlinedata is stray
	//		assert (outlinedata == nil); //(**hg).outlinedata = nil;
		
		(**hg).outlinedata = outlinedata;
		
		(**hg).topoutlinestack = topoutlinestack;
		
		moveleft  (outlinestack, (**hg).outlinestack, sizeof (hdloutlinerecord) * ctoutlinestack);
		
		outlinedata = nil; //don't let anyone else mess with us

		topoutlinestack = 0;
		
		(**hg).ctscanlines = ctscanlines;
		
		(**hg).ctscanchars = ctscanchars;
		
		(**hg).herrornode = herrornode;
		
		(**hg).flreturn = flreturn;
		
		(**hg).flbreak = flbreak;
		
		(**hg).flcontinue = flcontinue;
		
		(**hg).fllangerror = fllangerror;
		
		(**hg).langerrordisable = langerrordisable;  /*6.1.1b2 AR*/
		
		(**hg).tryerror = tryerror;

		(**hg).tryerrorstack = tryerrorstack;

		/*
			2006-04-24 smd
			has COM been initialized in this thread?
		*/
		(**hg).flcominitialized = flcominitialized;
		}
	
	(**hg).langcallbacks = langcallbacks;
	
	#ifdef flcomponent
	
	hlg = landgetglobals ();
	
	(**hg).eventcreatecallback = (**hlg).eventcreatecallback;
	
	(**hg).eventsendcallback = (**hlg).eventsendcallback;
	
	(**hg).applicationid = (**hlg).applicationid;
	
	(**hg).eventsettings = (**hlg).eventsettings;
	
	#endif
	
	} /*copythreadglobals*/


void swapinthreadglobals (hdlthreadglobals hglobals) {
	
	/*
	1/28/92 dmb: processstack and multiple error hooks are now maintained 
	in context
	
	1/20/93 dmb: if thread doesn't have a set of cancoon globals, leave 
	everything alone.  (currently, this occurs in component threads.)
	
	5.0b17 dmb: handle nil for dbverbs
	
	5.0.2b20 dmb: added tryerror to globals

	6.0b5 rab: added tryerrorstack
	*/
	
	register hdlthreadglobals hg = hglobals;
	register hdlcancoonrecord hc;
#ifdef landinclude
	register hdllandglobals hlg;
#endif
	long ticksnow = gettickcount ();
	
	assert (!flvisitingthreads || hg == hthreadglobals);

	hthreadglobals = hg; /*so current thread can easily access its globals*/
	
	if (hg == nil)
		return;
	
	hashtablestack = (**hg).htablestack;
	
	hc = (hdlcancoonrecord) (**hg).hccglobals;
	
	if (hc != cancoonglobals) {
		
		if (hc == nil)
			/*cancoonglobals = nil*/; /*clearcancoonglobals ();*/
		else
			setcancoonglobals (hc);
		}
	
	if ((**hg).timestarted == 0) /*first time being swapped in*/
		(**hg).timestarted = ticksnow;
	
	(**hg).timeswappedin = ticksnow;

	flthreadkilled = (**hg).flthreadkilled;
	
	fldisableyield = (**hg).fldisableyield;
	
	currentprocess = (**hg).hprocess;
	
	processstack = (**hg).processstack;
	
	currenthashtable = (**hg).htable;
	
	langcallbacks = (**hg).langcallbacks;
		
	#if !flruntime
	
		cterrorhooks = (**hg).cterrorhooks;
		
		moveleft ((**hg).errorhooks, errorhooks, sizeof (errorhookcallback) * maxerrorhooks);
		
		globalsstack = (**hg).globalsstack;
		
		if (shellwindow != (**hg).shellwindow) {
			
			outlinedata = nil; //set current outlinedata won't lose ctpushes

			shellsetglobals ((**hg).shellwindow);

			if (outlinedata)
				--(**outlinedata).ctpushes; //it just got a free one
			}
		
		outlinedata = (**hg).outlinedata;
		
		topoutlinestack = (**hg).topoutlinestack;
		
		moveleft  ((**hg).outlinestack, outlinestack, sizeof (hdloutlinerecord) * ctoutlinestack);

	#endif
	
	ctscanlines = (**hg).ctscanlines;

	ctscanchars = (**hg).ctscanchars;
	
	flreturn = (**hg).flreturn;
	
	flbreak = (**hg).flbreak;
	
	flcontinue = (**hg).flcontinue;
	
	fllangerror = (**hg).fllangerror;
	
	langerrordisable = (**hg).langerrordisable; /*6.1.1b2 AR*/

	tryerror = (**hg).tryerror;

	tryerrorstack = (**hg).tryerrorstack;
	
	flscriptrunning = (**hg).flscriptrunning;
	
	flscriptresting = (**hg).flscriptresting;
	
	herrornode = (**hg).herrornode;
	
#ifdef landinclude
	
	hlg = landgetglobals ();
	
	(**hlg).eventcreatecallback = (**hg).eventcreatecallback;
	
	(**hlg).eventsendcallback = (**hg).eventsendcallback;
	
	(**hlg).applicationid = (**hg).applicationid;
	
	(**hlg).eventsettings = (**hg).eventsettings;
	
#endif
	

	#ifdef flprofile
	if (cancoonglobals && !flvisitingthreads) {	
		unsigned long ticks;
		
		aboutsetthreadstring (hg, true);

		#ifdef MACVERSION
		ticks = gettickcount ();
		#else
		ticks = GetTickCount ();
		#endif

		if (lastswapticks)
			totalticksout += ticks - lastswapticks;
		lastswapticks = ticks;
		}

	#endif
	
	} /*swapinthreadglobals*/


boolean getprocesstimeslice (unsigned long *ticks) {
	
	hdlthreadglobals hg = getcurrentthread ();

	if (hg == nil) {

		if (processisoneshot (true))
			*ticks = processonehottimeslice;
		else
			*ticks = processagenttimeslice;

		return (false);
		}

	*ticks = (**hg).timesliceticks;

	return (true);
	} /*getprocesstimeslice*/


boolean setprocesstimeslice (unsigned long ticks) {
	
	hdlthreadglobals hg = getcurrentthread ();

	if (hg == nil)
		return (false);

	(**hg).timesliceticks = ticks;
	
	return (true);
	} /*setprocesstimeslice*/


boolean getdefaulttimeslice (unsigned long *ticks) {
	
	*ticks = processonehottimeslice;

	return (true);
	} /*setprocesstimeslice*/


boolean setdefaulttimeslice (unsigned long ticks) {
	
	processonehottimeslice = max (1, ticks);
	
	processagenttimeslice = max (1, ticks * 2 / 3);

	return (true);
	} /*setprocesstimeslice*/


boolean processtimesliceelapsed (void) {

	hdlthreadglobals hg = getcurrentthread ();
	
	assert (hg != nil);

	return (gettickcount () >= (**hg).timeswappedin + (**hg).timesliceticks);
	} /*processtimesliceelapsed*/


static pascal boolean maimprocessvisit (hdlthreadglobals hthread, long hcancoon) {
	
	/*
	if this process is dependent on hcancoon, "maim" it by clearing its global 
	data field.  actually killing the process at this point would require much 
	more substantial code changes outside of this file, since processyield would 
	never return.
	
	for runtime, all threads other than the main thread must be maimed.

	6.1b12 AR: Made 5.1.5b7 change Mac-only because we can't swap in the new thread's
	globals while flvisitingthreads == true.
	*/
	
	register hdlthreadglobals hg = getthreadglobals (hthread);
	
	#if flruntime
		
		if ((**hg).idthread != idapplicationthread)
			(**hg).flthreadkilled = true;
	
	#else
		
		if (hcancoon == (long) (**hg).hccglobals) /*it's dependent; maim it*/ {
			
			(**hg).hccglobals = nil; /*no longer valid*/
			
			(**hg).flthreadkilled = true;

//#ifdef MACVERSION			
			if (hg != agentthread)
				wakeprocessthread (hg); //5.1.5b7
//#endif
			}
		
	#endif
	
	return (false); /*keep visiting*/
	} /*maimprocessvisit*/


void killdependentprocesses (long hcancoon) {
	
	/*
	disable all process threads whose cancoonglobals field matches hcancoon.
	*/
	
	++flagentsdisabled;
	
	visitprocessthreads (&maimprocessvisit, (long) hcancoon); /*terminates if visit proc returns true*/
	
	processyield (); //give threads a chance to die
	
	--flagentsdisabled;
	} /*killdependentprocesses*/


static boolean gettracebacklist (Handle herrorstack, tyvaluerecord *v) {

	register hdlerrorstack hs = (hdlerrorstack) herrorstack;
	register short ix;
	register short ixtop;
	bigstring bsname;
	bigstring bspath;
	hdlhashtable htable;
	tyerrorrecord *pe;
	hdllistrecord outerlist;
	
	setnilvalue (v);
	
	if ((hs == nil) || ((**hs).toperror == 0))
		return (true); /*no info available -- not an error*/
	
	if (!opnewlist (&outerlist, false))
		return (false);

	ixtop = (**hs).toperror;
	
	for (ix = ixtop - 1; ix >= 0; --ix) {
		
		hdllistrecord innerlist;
		tyvaluerecord val;

		pe = &(**hs).stack [ix];
		
		if ((*pe).errorcallback == nil ||
			!(*(*pe).errorcallback) ((*pe).errorrefcon, 0, 0, &htable, bsname) ||
			!langexternalgetquotedpath (htable, bsname, bspath)) {
		
			langgetstringlist (anomynousthreadstring, bspath); 
			}
		
		if (!opnewlist (&innerlist, false))
			goto exit;
		
		if (!langpushlistlong (innerlist, (*pe).errorline) ||
			!langpushlistlong (innerlist, (*pe).errorchar) ||
			!langpushliststring (innerlist, bspath))
			
			goto exit;
		
		setheapvalue ((Handle) innerlist, listvaluetype, &val);
		
		if (!langpushlistval (outerlist, nil, &val))
			goto exit;
		}
	
	setheapvalue ((Handle) outerlist, listvaluetype, v);
	
	return (true);

exit:
	
	disposehandle ((Handle) outerlist);
	
	return (false);
	}/*gettracebacklist*/


static boolean getstatsvisit (hdlthreadglobals hthread, long refcon) {

	register hdlthreadglobals hg = getthreadglobals (hthread);
	register hdlprocessrecord hp = (hdlprocessrecord) (**hg).hprocess;
	hdlhashtable ht = (hdlhashtable) refcon;
	bigstring bsname, bsdata;
	long id = (long) (**hg).idthread;
	tyvaluerecord vlist;
	
	/*init bsdata*/
	
	setemptystring (bsdata);
	
	/*fill bsdata with info*/
	
	pushlong (id, bsdata);
	
	pushchar (',', bsdata);

	pushlong ((**hg).timestarted, bsdata);
	
	pushchar (',', bsdata);
	
	pushlong ((**hg).timetowake, bsdata);
	
	pushchar (',', bsdata);
	
	pushlong ((**hg).timeswappedin, bsdata);
	
	pushchar (',', bsdata);
	
	pushlong ((**hg).timesliceticks, bsdata);
	
	if (hp != nil) {

		pushchar (',', bsdata);
	
		pushlong ((**hp).sleepuntil, bsdata);
		
		pushchar (',', bsdata);
		
		pushlong ((**hp).flsleepinbackground, bsdata);
		
		pushchar (',', bsdata);
		
		pushlong ((**hp).flscheduled, bsdata);
		
		pushchar (',', bsdata);
		
		pushlong ((**hp).floneshot, bsdata);
		
		pushchar (',', bsdata);
		
		pushlong ((**hp).flrunning, bsdata);
		}
		
	pushchar (',', bsdata);
		
	pushlong ((**hg).debugthreadingcookie, bsdata);

	/*compute name for table entry*/

	copystring (BIGSTRING ("\x06" "thread"), bsname);

	pushlong (id, bsname);

	/*create subtable*/
	
	if (!langsuretablevalue (ht, bsname, &ht))
		return (false);
	
	/*assign info string*/
	
	if (!langassignstringvalue (ht, BIGSTRING ("\x0b" "threadstats"), bsdata))
		return (false);

	/*create and assign stack traceback*/
	
	if (hp != nil && (**hp).herrorstack != nil) {
	
		if (!gettracebacklist ((Handle) (**hp).herrorstack, &vlist))
			return (false);

		if (!hashtableassign (ht, BIGSTRING ("\x09" "traceback"), vlist))
			return (false);
		
		exemptfromtmpstack (&vlist);
		}
	
	return (false);
	}/*getstatsvisit*/


boolean processgetstats (hdlhashtable ht) {
	
	/*
	6.2b6 AR: For debugging dead/hung thread problems
	*/

	visitprocessthreads (&getstatsvisit, (long) ht);

	return (true);
	}/*processgetstats*/


static unsigned long latesttime;

static hdlthreadglobals hlatest1shot;

static pascal boolean findlatest1shotvisit (hdlthreadglobals hthread, long refcon) {
#pragma unused (refcon)

	/*
	if this thread contains a 1-shot process, kill it and stop the visit
	*/
	
	register hdlthreadglobals hg = getthreadglobals (hthread);
	register hdlprocessrecord hp = (hdlprocessrecord) (**hg).hprocess;
	
	//if ((hp != nil) && ((**hg).hccglobals != nil)) { /*a living process*/
	
	if ((hp != nil) && (**hp).floneshot && !(**hg).flthreadkilled) { /*a living 1shot process*/
		
		register unsigned long timestarted = (**hg).timestarted;
		
		if (timestarted > latesttime) { /*most recent seen so far*/
			
			latesttime = timestarted;
			
			hlatest1shot = hg;
			}
		}
	
	return (false); /*keep visiting*/
	} /*findlatest1shotvisit*/


boolean abort1shotprocess (void) {

	/*
	kill the most recently sharted 1-shot process we find.  maybe there's a 
	better way to  connect cmd-period with a particular process, but this 
	is a reasonable heuristic -- better than 1.0
	*/
	
	register hdlthreadglobals hg;
	
	latesttime = 0;
	
	hlatest1shot = nil; /*clear global*/
	
	visitprocessthreads (&findlatest1shotvisit, (long) 0); /*terminates if visit proc returns true*/
	
	hg = hlatest1shot; /*copy global into register*/
	
	if (hg == nil) /*no living 1shot found to kill*/
		return (false);
	
	killprocessthread ((hdlprocessthread) hg);
	
	return (true);
	} /*abort1shotprocess*/


static void setprocesslangcallbacks (void) {
	
	/*
	3/23/93 dmb: by setting these callbacks only when appropriate, instead of 
	at init time, lang.c doesn't have to work around them when being called 
	in other contexts
	*/
	
	langcallbacks.backgroundtaskcallback = &processbackgroundtask; 
	
	langcallbacks.pushsourcecodecallback = &processpushsourcecode;
	
	langcallbacks.popsourcecodecallback = &processpopsourcecode;
	
	#if !flruntime
	
	langcallbacks.debuggercallback = &processdebugger;
	
	langcallbacks.pushtablecallback = &processpushtable;
	
	langcallbacks.poptablecallback = &processpoptable;
	
	#endif
	
	langcallbacks.scriptkilledcallback = &processscriptkilled;
	} /*setprocesslangcallbacks*/


/*
2.1a6, 6/8/93 dmb: ok, the new thread manager is a whole new ball of wax...

designed to be portable and pre-emptive, the data structures are competely 
hidden, and the API is simpler and lower-level.  we now use our own thread 
globals handle as the main way to identify a thread, and define the psueudo-
type hdlprocessthread for use by clients other than the OSA routine, which are more 
intimately familiar with the globals structure.
*/

static boolean findthreadvisit (bigstring bs, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
#pragma unused (bs, hnode, refcon)

	return (val.data.longvalue == (long) (**hthreadglobals).idthread);
	} /*findvaluevisit*/


boolean initprocessthread (bigstring bsname) {
	
	/*
	8/22/91 dmb: start each thread off with an empty globals stack
	
	3/30/93 dmb: set up process-specific langcallbacks here so we don't 
	apply them to the main thread
	
	6/8/93 dmb: newly-packaged routine for thread manager 1.1. this must 
	be called at the beginning of every process thread main.  [we could 
	internalize this by calling it from the swappin routine the first 
	time only, but that adds extra overhead to the swap, and is a little 
	less clean]
	
	4.0.1b1 dmb: added bsname parameter, agenttable stuff
	
	9.1b3 AR: Changed code for computing unique name of entry in threadstable
	to not get stuck in an infinite loop if there are already 256 threads
	with the same bsname present in that table.
	*/
	
#if threadverbs
	tyvaluerecord val;
	bigstring bs;
#endif

	if (!threadstartup ())
		return (false);
	
#if threadverbs
	assert (threadtable);

	setlongvalue ((long) (**hthreadglobals).idthread, &val);
	
	copystring (bsname, bs);
	
	pushhashtable (threadtable);

	if (hashsymbolexists (bs)) {	/* 9.1b3 AR */
	
		int len = stringlength (bs) + 1;
		int x = 0;

		pushchar ('-', bs);
		
		do {
			setstringlength (bs, len);
			
			pushlong (++x, bs);
			
			} while (hashsymbolexists (bs));

		/*
		char c = '1';
		pushchar ('-', bs);
		pushchar (c, bs);
		while (hashsymbolexists (bs))
			bs [stringlength (bs)] = ++c;
		*/
		}

	hashinsert (bs, val);
	
	pophashtable ();
#endif

	(**hthreadglobals).timestarted = gettickcount ();
	
	#if !flruntime
	
		clearbytes (&globalsstack, sizeof (globalsstack));
		
		shellwindow = nil;
		
	#endif
	
	setprocesslangcallbacks ();
	
	return (true);
	} /*initprocessthread*/


void exitprocessthread (void) {
	/*
	4.0.1b1 dmb: new routine

	5.0a18 dmb: make sure threadtable isn't nil
	
	9.1b3 AR: Nuke currentprocess to prevent opCursorMoved callback
	from running if the system.compiler.threads table is visible.
	We can't run the callbacks at this point because all the infrastructure
	for executing scripts has already been disposed of at this point.
	*/
	
#if threadverbs
	bigstring bsname;
	
	currentprocess = nil;	/* 91.b3 AR */
	
	if (threadtable &&
			hashinversesearch (threadtable, &findthreadvisit, nil, bsname)) { /*symbol is defined in htable*/
		
		flexitingthread = true;
		
		hashtabledelete (threadtable, bsname);
		
		flexitingthread = false;
		}
#endif
	
	threadshutdown ();
	} /*exitprocessthread*/


/*
void exitprocessthread (void *result) {
	
	register short i;
	ThreadID idthread;
	boolean flrecycle = false; /%default%/
	
	GetCurrentThread (&idthread);
	
	for (i = 0; i < threadpoolsize; ++i) {
		
		if (threadpool [i] == idthread) { /%came from the pool...%/
			
			threadpool [i] = kNoThread;
			
			flrecycle = true; /%...return to the pool%/
			
			break;
			}
		}
	
	DisposeThread (kCurrentThread, result, flrecycle)
	} /%exitprocessthread%/
*/


hdlprocessthread getcurrentthread (void) {
	
	return (getcurrentthreadglobals ());
	} /*getcurrentthread*/


boolean processpsuedothread (tythreadmaincallback threadmain, tythreadmainparams threadparams) {
	
	/*
	2.1b5 dmb: set the thread id to identify it as the application thread, so 
	that inmainthread will return true
	
	3.0b16 dmb: must copythreadglobals (hglobals) before swapping them out.
	*/
	
	hdlthreadglobals hglobals;
	hdlthreadglobals mainglobals = hthreadglobals;
	
	if (!newthreadglobals (&hglobals))
		return (false);
	
	(**hglobals).idthread = idapplicationthread; /*make it look like main thread*/
	
	copythreadglobals (mainglobals);
	
	swapinthreadglobals (hglobals);
	
	threadmain (threadparams);
	
	copythreadglobals (hglobals); /*3.0b16*/
	
	swapinthreadglobals (mainglobals);
	
	disposethreadglobals (hglobals);
	
	return (true);
	} /*processpsuedothread*/


static void disposeprocessthread (hdlthreadglobals htread) {
	
	disposethreadglobals (htread);
	
	--ctprocessthreads;
	} /*disposeprocessthread*/


boolean newprocessthread (tythreadmaincallback threadmain, tythreadmainparams threadparams, hdlthreadglobals *hthread) {
	
	/*
	7/10/92 dmb: call shellforcebackgroundtask
	
	6/8/93 dmb: newly-packaged routine for new thread manager. call this to 
	create a new thread
	
	2.1b2 dmb: use macmemoryconfig.minstacksize when creating thread pool
	
	5.1.1 dmb: limit trial size thread count
	*/
	
	hdlthread idthread;
	hdlthreadglobals hglobals;
	
	if (!flcanusethreads)
		return (false);
	
	#ifdef fltrialsize
	
		if (ctprocessthreads > 2) {
		
			shelltrialerror (threadlimitstring);
			
			return (false);
			}
	
	#endif
	
	if (!newthreadglobals (&hglobals))
		return (false);
	
	if (!newthread (threadmain, threadparams, hglobals, &idthread)) {
		
		disposethreadglobals (hglobals);
		
		return (false);
		}
	
	(**hglobals).idthread = idthread;
	
	/*
	for (i = 0; i < threadpoolsize; ++i) {
		
		if (threadpool [i] == kNoThread) { /%must have come from the pool...%/
			
			threadpool [i] = idthread; /%...keep track of who owns this slot%/
			
			break;
			}
		}
	*/
	
	shellforcebackgroundtask (); /*make sure new thread gets control before shell yields to other apps*/
	
	*hthread = hglobals;
	
	++ctprocessthreads;
	
	return (true);
	} /*newprocessthread*/


short processthreadcount (void) {
	
	/*
	return the number of threads, not counting the main thread
	*/
	
	return (ctprocessthreads);
	} /*processthreadcount*/


long getthreadid (hdlprocessthread hthread) {

	if (hthread == nil)
		return ((long) idnullthread);
	
	return ((long) (**(hdlthreadglobals) hthread).idthread);
	} /*getthreadid*/


hdlprocessthread getprocessthread (long id) {

	register hdlthreadglobals hg;

	for (hg = (**processthreadlist).hfirst; hg != nil; hg = (**hg).hnextglobals) {
		
		if ((long) (**hg).idthread == id)
			return (hg);
		}
	
	return (nil);
	} /*getprocessthread*/


hdlprocessthread nthprocessthread (long n) {

	register hdlthreadglobals hg;

	for (hg = (**processthreadlist).hfirst; hg != nil; hg = (**hg).hnextglobals) {
		
		if ((**hg).idthread > idapplicationthread) {
		
			if (--n == 0)
				return (hg);
			}
		}
	
	return (nil);
	} /*nthprocessthread*/


static pascal boolean wakeupvisit (Handle hthread, long refcon) {
	
	/*
	8/31/92 dmb: make sure that the thread we're trying to kill isn't 
	sleeping through its time or reckoning
	*/
	
	return ((long) hthread == refcon);
	} /*wakeupvisit*/


boolean wakeprocessthread (hdlprocessthread hthread) {
	
	/*
	wake any thread. if its sleep is being managed by landsystem7, make 
	sure it's woken by that system
	*/
	
	boolean fl;
	
#ifdef landinclude
	if (landvisitsleepingthreads (&wakeupvisit, (long) hthread))
		fl = true;
	else
#endif
	if (shellcallwakeuphooks (hthread)) //hook didn't wake it up
		fl = processwake (hthread);
	else
		fl = true;
	
	if (fl)
		shellforcebackgroundtask (); /*make sure new thread gets control asap*/

	return (fl);
	} /*wakeprocessthread*/


boolean killprocessthread (hdlprocessthread hthread) {
	
	/*
	5.0.1 dmb: don't wake here before kill. Under Windows, it could 
	be gone when wake is done. processkill will do the wake itself.
	*/

	register hdlthreadglobals hg = (hdlthreadglobals) hthread;

	if (hg == nil)
		return (false);
	
	(**hg).flthreadkilled = true; /*maim the thread*/
	
	if (hg == hthreadglobals)	/*mirror to current global*/
		flthreadkilled = true;
	
	if (!processkill ((hdlprocessrecord) (**hg).hprocess))
		wakeprocessthread (hg);
	
	return (true);
	} /*killprocessthread*/
	

static boolean initmainprocessthread (void) {
	
	/*
	2.1a6, 6/8/93 dmb: new thread manager 1.1 code. make sure the thread manager 
	is present, and pre-allocate two threads -- one for agents, plus a one-shot.
	also, install the custom swapping routine into the main thread
	
	2.1b1 dmb: set hthreadglobals so that it's never nil, and make sure idthread 
	is set even when threads aren't available
	
	2.1b2 dmb: use macmemoryconfig.minstacksize when creating thread pool
	
	2.1b5 dmb: don't create a thread pool; all it does is fragment our heap, 
	since we don't end up recycling threads. there's new code to do the 
	recycling commented out (search for "threadpool"), but it's never been 
	tested. to be safe, we'd really need to be more careful about how much memory
	a script can consume, since right now when a script runs out of memory & stops, 
	it's thread is released, creating room to Save, etc.
	
	2.1b13 dmb: don't post needthreadmanager error at startup. wait until an 
	agent tries to run.
	*/
	
	hdlthreadglobals hglobals;
	
	if (!newthreadglobals (&hglobals))
		return (false);
	
	hthreadglobals = hglobals;
	
	if (!initthreads ()) {
		
		(**hglobals).idthread = idapplicationthread;
		
		/*
		alertstring (needthreadmanagerstring);
		*/
		
		return (false);
		}
	
	threadcallbacks.disposecallback = (tythreadglobalscallback) &disposeprocessthread;
	
	threadcallbacks.swapincallback = (tythreadglobalscallback) &swapinthreadglobals;
	
	threadcallbacks.swapoutcallback = (tythreadglobalscallback) &copythreadglobals;
	
	initmainthread (hglobals);
	
	(**hglobals).idthread = idapplicationthread;
	
	(**hglobals).timestarted = gettickcount ();
	
	flcanusethreads = true;
	
	return (true);
	} /*initmainprocessthread*/


boolean processruncode (hdlprocessrecord hprocess, tyvaluerecord *vreturned) {
	
	/*
	7/2/91 dmb: use new process callback routines for processtarted, processkilled
	
	12/2/91 dmb: don't force menu adjust if processnotbusy has been called.
	
	3/10/92 dmb: call new fifcloseallfiles for clean up on error
	*/
	
	register hdlprocessrecord hp = hprocess;
	register boolean floneshot = (**hp).floneshot;
	register hdltreenode hcode = (**hp).hcode;
	register boolean fl;
	
	if (hcode == nil) /*defensive driving*/
		return (false);
	
	if (!pushprocess (hp)) /*error setting the current process to this one*/
		return (false);
	
	if (floneshot) {
		
		/*
		_profile = true;
		*/
		
		honeshotcode = hcode; /*set global so other threads can kill script*/
		
		(*(**hp).processstartedroutine) ();
		}
	
	(**hp).flrunning = true;
	
	fl = langruncode (hcode, (**hp).hcontext, vreturned); 
	
	(**hp).flrunning = false;
	
	if ((**hp).holdcode != nil) { /*code was replace while process was running*/
		
		langdisposetree ((**hp).holdcode); /*in case code was replaced*/
		
		(**hp).holdcode = nil;
		}
	
	if (floneshot) {
		
		if (!fl) { /*an error occurred*/
		
			fifcloseallfiles ((long) hp);
			
			langreleasesemaphores (hp);
			}
		
		(*(**hp).processkilledroutine) ();
		
		if (processbusy ())
			processnotbusy ();
		
		/*
		_profile = false;
		
		DumpProfile ();
		*/
		}
	
	popprocess ();
	
	return (fl);
	} /*processruncode*/


boolean processruntext (Handle htext) {
	
	/*
	schedule the text to run in the background, in its own thread
	htext is consumed
	*/
	
	hdltreenode hcode;
	
	if (!langbuildtree (htext, false, &hcode)) /*consumes htext*/
		return (false);
	
	if (!flinhibiterrorclear)
		langerrorclear (); /*compilation produced no error, be sure error window is empty*/
	
	if (!addnewprocess (hcode, true, nil, (long) 0)) {
		
		langdisposetree (hcode);
		
		return (false);
		}
	
	return (true);
	} /*processruntext*/


boolean processrunstring (bigstring bs) {
	
	/*
	schedule the string to run in the background, in its own thread
	
	5.0a6 dmb: check processlist for nil. the shell calls us directly now
	*/
	
	Handle htext;
	
	if (processlist == nil)
		return (false);
	
	if (!newtexthandle (bs, &htext))
		return (false);
	
	return (processruntext (htext));
	} /*processrunstring*/


boolean processrunstringnoerrorclear (bigstring bs) {
	
	/*
	5.0a22 dmb: new function, for automatically-run scripts that 
	shouldn't clear the error info window
	*/

	boolean fl;
	
	flinhibiterrorclear = true;

	fl = processrunstring (bs);
	
	flinhibiterrorclear = false;

	return (fl);
	} /*processrunstringnoerrorclear*/


boolean processbusy (void) {
	
	/*
	return true if we're busy running a one-shot process.  although we 
	can run many such processes at once, user interface simplification 
	calls for limiting the number to one.
	*/
	
	return (honeshotcode != nil);
	} /*processbusy*/


boolean processnotbusy (void) {
	
	/*
	the caller is indicating that we should no longer consider the 
	current process to be tying us up; event though we may be 
	running a 1-shot process, we're not busy.
	*/
	
	#if !flruntime
	
	if (honeshotcode != nil) {
		
		honeshotcode = nil;
		
		shellforcemenuadjust ();
		}
	
	#endif
	
	return (true);
	} /*processnotbusy*/


boolean processrunning (void) {
	
	#if flruntime
	
	return (ctprocessthreads - ctsleepingthreads > 0);
	
	#else
	
	register hdlprocesslist hlist = processlist;
	
	if (hlist == nil)
		return (false);
	
	return ((**hlist).ctrunning - (**hlist).ctsleeping > 0);
	
	#endif
	
	} /*processrunning*/


unsigned long processstackspace (void) {
	
	/*
	3.0.1b2 dmb: new routine. using this instead of StackSpace yields 
	correct result, fixes incompatibility with RAMDoubler and Mac SE
	
	3.0.4b8 dmb: don't use ThreadCurrentStackSpace when we're in an osa 
	client process. On the PPC, it appears to drop to zero randomly, 
	possibly due to the strange heap/globals context. Neither ThreadCurrentStackSpace
	nor GetCurrentThread returns an error, but the space returned is bogus.
	*/
	
#ifdef MACVERSION
	#if __powerc
	if (infrontierthread ())	// not in an osa client
	#endif
	
		if (flcanusethreads) {
			
			unsigned long space;
			unsigned long currentThread;
			//Changed this to wored in OSX kCurrentThread wasn't working
			//for some reason.
			GetCurrentThread(&currentThread);
			if (ThreadCurrentStackSpace (currentThread, &space) == noErr)
				return (space);
			}
	
	return (StackSpace ());
#endif

#ifdef WIN95VERSION
	return (0x04000);	// a lot; *** don't know how to check this under Windows
#endif
	} /*processstackspace*/


static void postthreadsmessage (void) {
	
	/*
	2.1b13 dmb: new routine for alerting the user of the lack of threads. 
	we're only called when an agent would normally be run, so removing 
	all agents will avoid the message. also, we check system.startup.firsttime 
	to avoid scaring the user when they haven't even run the installer
	
	with runtime, since agents often don't exist, we want to put up a warning 
	when trying to run a non-agent thread. but we'll use a "softer" warning, 
	a message in the main window, instead of the alert.
	*/
	
	bigstring bs;
	
	if (!flpostedthreadsmessage) {
		
		flpostedthreadsmessage = true; /*whether we actually do or not, we won't try again*/
		
		if (getsystemtablescript (idisfirsttimescript, bs)) { /*system.startup.firsttime*/
			
			if (langrunstringnoerror (bs, bs) && equalstrings (bs, bstrue)) /*first time*/
				return;
			}
		
		#if flruntime
		
		getstringlist (alertstringlistnumber, needthreadmanagerstring, bs);
		
		setwindowmessage (bs);
		
		#else
		
		alertstring (needthreadmanagerstring);
		
		#endif
		}
	} /*postthreadsmessage*/


boolean addnewprocess (hdltreenode hcode, boolean floneshot, langerrorcallback errorcallback, long errorrefcon) {
	
	/*
	a wrapper for conventional process creation
	*/
	
	hdlprocessrecord hnewnode;
	
	if (!newprocess (hcode, floneshot, errorcallback, errorrefcon, &hnewnode))
		return (false);
	
	addprocess (hnewnode);
	
	return (true);
	} /*addnewprocess*/


boolean addprocess (hdlprocessrecord hprocess) {
	
	/*
	add a new process to the process list.  if floneshot is true then it will
	be run once and then disposed of.  if fldebugging is true, the debugger will
	get a shot at the process before every "meaty" instruction.
	
	2/20/91 dmb: insert one-shot processes in front of other (agent) processes 
	for faster response to user commands
	
	5/23/91 dmb: after inserting a one-shot process, force a shell background 
	task to minimize delay.
	
	7/31/91 dmb: ***now that we have the one-shot scheduler loop, we don't 
	need to do the fancy insertion; we should be able to use the generic 
	listinsert routine.  no time to test it now, though, so just this comment
	*/
	
	register hdlprocessrecord hp = hprocess; /*copy into register*/
	register hdlprocessrecord nomad;
	register hdlprocessrecord prevnomad = nil;
	
	if (hp == nil)
		return (false);

	_entercriticalprocesssection ();

	if (processlist == nil) {
		
		_leavecriticalprocesssection ();

		return (false);
		}

	nomad = (**processlist).hfirstprocess;

	while (nomad != nil) {
		
		if ((**hp).floneshot && !(**nomad).floneshot) /*insert 1-shot before agent*/
			break;
		
		prevnomad = nomad;
		
		nomad = (**nomad).hnextprocess;
		}

	if (prevnomad == nil)
		(**processlist).hfirstprocess = hp; 
	else
		(**prevnomad).hnextprocess =  hp;
	
	(**hp).hnextprocess = nomad;
	
	(**hp).hprocesslist = processlist;
	
	_leavecriticalprocesssection ();

	if ((**hp).floneshot)
		shellforcebackgroundtask ();
	
	newlyaddedprocess = hp; /*set global -- needed by landipcmenus and shellsysverbs.c*/
	
	return (true);
	} /*addprocess*/


static boolean processtimeslice (hdlprocessrecord hprocess) {
	
	/*
	10/27/91 dmb: don't dispose result value if processruncode returns false
	*/
	
	register hdlprocessrecord hp = hprocess;
	register boolean fl;
	tyvaluerecord val;
	
	if ((**hp).flrunning) /*this guy's already running!*/
		return (true);
	
	fl = processruncode (hp, &val);
	
	if (fl)
		disposetmpvalue (&val);
	
	return (fl);
	} /*processtimeslice*/


static pascal void *oneshotthreadmain (void *hprocess) {
	
	/*
	4.0.1b1 dmb: pass name of thread to initprocessthread; call exitprocessthread
	*/
	
	register hdlprocessrecord hp = (hdlprocessrecord) hprocess;
	register hdlprocesslist hlist = (**hp).hprocesslist;
	bigstring bsname;

	#if 0

	hdlerrorstack hs = (**hp).herrorstack;
	hdlhashtable htable;
	
	tyerrorrecord *pe = &(**hs).stack [(**hs).toperror - 1];
	
	setemptystring (bsname);
	
	if ((*pe).errorcallback != nil)
		(*(*pe).errorcallback) ((*pe).errorrefcon, 0, 0, &htable, bsname);
	
	if (isemptystring (bsname))
		langgetstringlist (anomynousthreadstring, bsname); 
	#else

	copystring ((**hp).bsname, bsname);
	#endif

	if (!initprocessthread (bsname)) /*must call from every thread main, before using globals*/
		return (nil);
	
	if (!(**hp).fldisposewhenidle)
		processtimeslice (hp);
	
	deleteprocess (hp);
	
	(**hlist).ctrunning--;
	
	if ((**hlist).fldisposewhenidle) /*try disposing now; will check ctrunning again*/
		disposeprocesslist (hlist);
	
	exitprocessthread ();
	
	return (nil);
	} /*oneshotthreadmain*/


boolean scheduleprocess (hdlprocessrecord hprocess, hdlprocessthread *hnewthread) {

	/*
	4.1b3 dmb: now callable externally, for the thread verbs in shellsysverbs.c
	
	return true if we actually schedule the process and give (**hp).hthread a value.
	*/
	
	register hdlprocessrecord hp = hprocess;
	
	*hnewthread = nil;
	
	if ((**hp).flscheduled)
		return (false);
	
	(**hp).flscheduled = true;
	
	if (flcanusethreads) {
		
		if (newprocessthread (&oneshotthreadmain, (tythreadmainparams) hp, hnewthread)) {
			
			(**hp).hthread = *hnewthread;
			
			(**processlist).ctrunning++;
			
			return (true);
			}
		else {
			
			deleteprocess (hp);
			
			return (false);
			}
		}
	else {
		
		postthreadsmessage ();
		
		return (processpsuedothread (&oneshotthreadmain, (tythreadmainparams) hp));
		}
	} /*scheduleprocess*/


static void oneshotscheduler (void) {
	
	/*
	06/26/91 dmb: a special loop for one-shots
	
	6/9/93 dmb: updated for new thread. note that we try to run oneshots event if 
	threads are not available, except under the debugger where we let it fail.
	
	4.1b3 dmb: set new hthread field in process record
	*/
	
	register hdlprocesslist hlist = processlist;
	register hdlprocessrecord hp;
	hdlprocessrecord hnext;
	hdlprocessthread hthread;
	
	if (hlist == nil)
		return;
	
	for (hp = (**hlist).hfirstprocess; hp != nil; hp = hnext) { /*walk through process list*/
		
		if (!(**hp).floneshot) /*we're done*/
			break;
		
		hnext = (**hp).hnextprocess;
		
		scheduleprocess (hp, &hthread);
		}
	} /*oneshotscheduler*/


static void postmemorymessage (void) {
	
	/*
	12/25/91 dmb: this is done in its own routine to that every thread doesn't 
	brear the brunt of an extra bigstring on the stack.
	*/
	
	bigstring bs;
	
	if (!flpostedmemorymessage) { /*didn't already put up message*/
		
		if (shellgetstring (outofmemorystring, bs))
			shellfrontrootwindowmessage (bs);
		
		flpostedmemorymessage = true;
		}
	} /*postmemorymessage*/


static void agentscheduler (void) {
	
	/*
	the new process scheduler -- not round-robin.
	
	we visit every process in the list, and if it isn't blocked from running,
	we give it a shot at the processor.
	
	this is designed to be invoked once every second, and we must give each
	process a shot once per second.
	
	9/24/90 dmb: only give the first one-shot in the queue any time
	
	1/4/90 dmb: new rules.  don't re-enter loop if there is a process 
	running that isn't being debugged.  otherwise, allow all one-shot 
	processes in the queue to run.
	
	2/12/92 dmb: added flsleepinbackground logic
	*/
	
	register hdlprocesslist hlist = processlist;
	register hdlprocessrecord hp;
	register unsigned long x;
	register hdlprocessrecord hnext;
	
	if (hlist == nil)
		return;
	
	flprocesscodedisposed = false; /*must reset every time*/
	
	x = timenow ();
	
	for (hp = (**hlist).hfirstprocess; hp != nil; hp = hnext) { /*find a process that's not sleeping*/
		
		register unsigned long sleepuntil = (**hp).sleepuntil;
		
		hnext = (**hp).hnextprocess;
		
		if ((**hp).floneshot) /*we don't deal with one-shots here*/
			continue;
		
		if ((sleepuntil != 0) && (sleepuntil > x)) /*process is sleeping*/
			continue;		

		/*
		if ((**hp).flsleepinbackground && !shellisactive ())
			continue;
		*/
		
		//(**hp).sleepuntil++; //6.1b8 AR: old rescheduling code by dmb -- commented out
		
		(**hp).sleepuntil = x + 1; /*6.1b8 AR: reschedule for a second later*/

		(**hlist).ctrunning++;
		
		assert (!(**hthreadglobals).flretryagent);
		
		if (!processtimeslice (hp)) {
			
			if ((**hthreadglobals).flretryagent)
				(**hthreadglobals).flretryagent = false; /*reset*/
			else
				deleteprocess (hp);
			}
		
		(**hlist).ctrunning--;
		
		if (processlist != hlist) /*massive context change occured*/
			break;
		
		if (flprocesscodedisposed) /*may have deleted code for nextnomad; safest to exit*/
			break;
		} /*while*/
	
	if ((**hlist).fldisposewhenidle) /*try disposing now; will check ctrunning again*/
		disposeprocesslist (hlist);
	} /*agentscheduler*/


static pascal void *agentthreadmain (void *ignore) {
#pragma unused (ignore)

	/*
	4.0.1b1 dmb: pass name of thread to initprocessthread; call exitprocessthread
	
	4.1b2 dmb: instead of running the agentscheduler just once and quitting, 
	run it in a loop, sleeping after each time. now the processscheduler can 
	just wake us each second instead of having the thread destroyed and re-
	created every time. if flagentsenabled is turned off while we're sleeping,
	the processscheduler will still wake us to we can terminate.
	
	4.1b5 dmb: added agentthreadsleeping flag. processscheduler should only 
	wake the agent thread if this is true, i.e. it's asleep because it has 
	finished a loop. If it's asleep for any other reason, i.e. a script called 
	thread.sleep or is sending an Apple event, it should be left sleeping. At 
	the very least, processscheduler would have to call wakeprocessthread instead 
	of processwake to handle AE sleep. But it really shouldn't have to.

	5.0b16 dmb: need to check state between scheduling and sleeping, or we 
	can hang if someone is trying to kill us
	*/
	
	initprocessthread (BIGSTRING ("\x06" "agents")); /*must call from every thread main*/
	
	while (flagentsenabled && ingoodthread () && !flagentsdisabled && !flshellclosingall) {
		
		setprocesstimeslice (processagenttimeslice); //do this every time (new in 5.1.5)

		agentscheduler ();
		
		if (!ingoodthread () || flagentsdisabled) //check again now, before we sleep
			break;
		
		agentthreadsleeping = true;
		
		threadsleep (nil);
		
		agentthreadsleeping = false;
		
		flagentsenabled = langgetuserflag (idagentsenabledscript, flagentsenabled);
		}
	
	agentthread = nil; /*clear global / semaphore*/
	
	exitprocessthread ();
	
	return (nil);
	} /*agentthreadmain*/


void processscheduler (void) {
	
	/*
	06/26/91 dmb: new version for multithreaded environment.
	
	10/16/91 dmb: don't do anything if the process list is nil
	
	2.1b13 dmb: if threads aren't available, tell the user here, when 
	we're actually about to try to run an agent.
	
	3.0b15 dmb: use macmemoryconfig.minstacksize, not the Thread Manager's 
	default stack size when preflighting agent thread. also, add 2K to that 
	number, not 1K.
	
	5.0a23 dmb: don't create agent thread until startup scripts are done

	5.1.5b15 dmb: don't schedule anything if we're closing all
	*/
	
	#ifdef MACVERSION
		long stacksize;
	#endif
	
	if (processlist == nil)
		return;
	
	if (currentprocess != nil) { /*this call is nested under language execution*/
		
		shellforcebackgroundtask (); /*reset, since we're waiting for better context*/
		
		return;
		}
	
	/*
	if (globalsstack.top > 1) { /%can't keep things clean if more than 1 window pushed%/
		
		return;
		}
	*/
	
	oneshotscheduler (); /*do all the one-shots*/
	
	if (processlist == nil) /*file was closed during background processing*/
		return;
	
	#ifdef MACVERSION
	/*
		//6.1b8 AR: This bit of code stopped the agents from being called
		//			on the Mac while the scriptdebugger was active and
		//			while the Find & Replace dialog was open.
		
		if (threadiswaiting ()) {
			
			processyield ();
			
			return;
			}
	*/
	#endif
	
	if (agentthread != nil) { /*agent scheduler is not to be re-entered*/
		
		if (processlist != agentprocesslist) { /*we've swapped active roots*/
		
			killprocessthread (agentthread);
			}
		else {
			
			if (agentthreadsleeping)
				processwake (agentthread);
			}
		
		#ifdef MACVERSION
			processyield (); /*let previous thread die, reawaken, or try to finish, respectively*/
		#endif
		
		return;
		}
	
	#ifdef version5orgreater
		flagentsenabled = langgetuserflag (idagentsenabledscript, flagentsenabled);
	#endif
	
	if (!flagentsenabled || flagentsdisabled)
		return;
	
	if (specialoneshotscriptsrunning () > 0)
		return;
	
	if (!flcreatedagentprocess) /*2.1b13: never created an agent*/
		return;
	
	if (!flcanusethreads) { /*2.1b13: now's the time to tell user threads aren't available*/
		
		postthreadsmessage ();
		
		return;
		}
	
	/*
	if (GetDefaultThreadStackSize (kCooperativeThread, &stacksize) != noErr)
		return;
	*/

	#ifdef MACVERSION	
		stacksize = macmemoryconfig.minstacksize;
		
		if (!haveheapspace (stacksize + 2 * 1024)) { /*not enough memory to run agents*/
			
			postmemorymessage (); /*use subroutine to avoid bigstring on stack here*/
			
			return;
			}
	#endif

	if (flpostedmemorymessage)
		shellfrontrootwindowmessage (zerostring); /*clear it*/
	
	newprocessthread (&agentthreadmain, (tythreadmainparams) 0, &agentthread);
	
	agentprocesslist = processlist;
	} /*processscheduler*/


static boolean processkeyboardhook (void) {
	
	/*
	if a one-shot process is running, intercept command-period to kill it
	
	return false if we consume the keystroke
	
	10/29/91 dmb: watch for escape key too
	*/
	
	if (processlist == nil)
		return (true);
	
	if ((keyboardstatus.chkb == chescape) || (keyboardstatus.flcmdkey && (keyboardstatus.chkb == '.'))) {
		
		abort1shotprocess (); /*abort the first 1-shot process we find*/
		
		return (false); /*consumed*/
		}
	
	return (true);
	} /*processkeyboardhook*/


#ifdef MACVERSION
	#define maxticks (0xffffffffUL)
#endif
#ifdef WIN95VERSION
	#define maxticks (0xffffffffUL / 50UL)
#endif

void processchecktimeouts (void) {

	/*
	5.0b15 dmb: only wake one process at a time. the act of waking can 
	break our list walk under Windows

	6.2b7 AR: If the system tick count just wrapped around, the current
	tick count will be less than the tick count when the thread was last
	swapped in. In that case, we want to wake the thread if it's sleeping.
	
	11/21/01 7.1b31 dmb: don't break out of the wake loop when a thread for
	waking is found. This behavior as archaic, thinking that waking on thread 
	at a time would be all we can really handle. But this can cause deadlock,
	which we found with the script debugger.
	*/

	register hdlthreadglobals hg;
	unsigned long ticks = gettickcount ();
	static unsigned long lastprocesscheckticks = 0L;

	if (ticks < lastprocesscheckticks) { /*tick count wrapped around, re-compute sleep times*/

		for (hg = (**processthreadlist).hfirst; hg != nil; hg = (**hg).hnextglobals) {
			
			unsigned long timetowake = (**hg).timetowake;
			unsigned long sleepticks = (**hg).sleepticks;
			unsigned long timebeginsleep = (**hg).timebeginsleep;
			
			if ((timetowake > 0) && (timebeginsleep > ticks) && (sleepticks != 0xffffffffUL)) {

				unsigned long deltaticks = maxticks - timebeginsleep;

				(**hg).timetowake = (sleepticks > deltaticks) ? (sleepticks - deltaticks) : 1UL;
				}
			}
		}

	lastprocesscheckticks = ticks;
	
	for (hg = (**processthreadlist).hfirst; hg != nil; hg = (**hg).hnextglobals) {
		
		unsigned long timetowake = (**hg).timetowake;
		
		if ((timetowake > 0) && (ticks > timetowake) && ((**hg).sleepticks != 0xffffffffUL)) {
		
			wakeprocessthread ((hdlprocessthread) hg);
			
			shellforcebackgroundtask ();
			
			//break; //7.1b31
			}
		}
	} /*processchecktimeouts*/


boolean processsymbolunlinking (hdlhashtable htable, hdlhashnode hnode) {
	
	/*
	4.1b3 dmb: new langhash callback allows us to examine the hashnode 
	before its been deleted. makes it a lot easier for us to respond to 
	user deletion of an item in the threads table
	*/
	
	if (htable == threadtable && !flexitingthread) {
		
		long idthread = (**hnode).val.data.longvalue;
		
		killprocessthread (getprocessthread (idthread));
		}
	
	return (true);
	} /*processsymbolunlinking*/



void processclose (void) {
	
	} /*processclose*/


boolean initprocess (void) {
	
	/*
	called once when the program is booting -- do dynamic linking stuff
	for the language.
	*/
	
	/*
	InitProfile (200, 200);
	
	_profile = false;
	*/
	
	#if !flruntime
	
		shellpushkeyboardhook (&processkeyboardhook);
	
	#endif
	
	if (!newclearhandle (sizeof (tythreadlist), (Handle *) &processthreadlist))
		return (false);
	
	return (initmainprocessthread ());
	} /*initprocess*/




