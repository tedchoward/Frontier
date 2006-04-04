
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

#ifndef processinclude
#define processinclude /*so other includes can tell if we've been loaded*/

#ifndef langinclude
	
	#include "lang.h"

#endif

#ifndef threadsinclude

	#include "threads.h"

#endif


typedef struct tythreadglobals **hdlprocessthread;		// it's a hdlthreadglobals really; we're avoiding dependencies

typedef struct typrocessrecord { 
	
	struct typrocessrecord **hnextprocess; /*nil-terminated list*/
	
	struct typrocesslist **hprocesslist; /*this list process is in, or nil*/
	
	hdltreenode hcode; /*the code that gets executed for this process*/
	
	hdltreenode holdcode; /*non-nil if code was replaced while agent was running*/
	
	hdlhashtable hcontext; /*non-nil if running from component API or other special cases*/
	
	langerrormessagecallback errormessagecallback;
	
	langerrormessagecallback debugerrormessagecallback;

	langvoidcallback clearerrorcallback;
	
	unsigned long sleepuntil; /*indicates when the process should wake up*/
	
	hdltablestack htablestack;
	
	hdlerrorstack herrorstack;
	
	hdlprocessthread hthread;
	
	boolean flsleepinbackground; /*if true, agent shouldn't be scheduled in the background*/
	
	boolean fldebugging; /*trap into debugger on every "meaty" instruction*/
	
	boolean flscheduled; /*has this processing been scheduled to run?*/
	
	boolean floneshot; /*if true process runs once and is then disposed*/
	
	boolean flrunning; /*is this processing running now?*/
	
	boolean fldisposewhenidle; /*was there an attempt to dispose of this process?*/
	
	boolean flprofiling; /*track time slice ticks*/
	
	boolean flprofilesliced; /*slice totals are not comulative*/
	
	struct tyexternalvariable **hprofiledata; /*table variable for perf data*/
	
	langvoidcallback processstartedroutine; /*execution is beginning*/
	
	langvoidcallback processkilledroutine; /*execution is terminating*/
	
	bigstring bsmsg; /*the last message posted by this agent*/
	
	bigstring bsname; // 5.0a22 dmb: why not maintain the name from the start?
	
	long processrefcon; /*for client use only*/
	} typrocessrecord, *ptrprocessrecord, **hdlprocessrecord;


#if !flruntime

typedef struct typrocesslist {
	
	hdlprocessrecord hfirstprocess; /*head of list*/
	
	/*hdlprocessrecord processlistmarker; /%for scheduler, pick up search from here*/
	
	short ctrunning; /*number of processes in this list currently running*/
	
	short ctsleeping; /*number of processes in this list currently sleeping*/
	
	boolean fldisposewhenidle; /*dispose when ctrunning returns to zero?*/
	} typrocesslist, *ptrprocesslist, **hdlprocesslist;

#endif



/*globals*/

extern hdlprocessrecord currentprocess; /*process that's currently running*/

extern hdlprocessrecord newlyaddedprocess; /*process that was just added to list*/

extern unsigned short fldisableyield;

extern boolean flcanusethreads;


/*prototypes*/

extern boolean setagentsenable (boolean);

extern boolean agentsenabled (void);

extern boolean agentsdisable (boolean);

#if !flruntime

extern boolean disposeprocesslist (hdlprocesslist);

extern boolean newprocesslist (hdlprocesslist *);

extern void setcurrentprocesslist (hdlprocesslist);

#endif

extern boolean pushprocess (hdlprocessrecord);

extern boolean popprocess (void);

extern void disposeprocess (hdlprocessrecord hprocess);

extern boolean newprocess (hdltreenode, boolean, langerrorcallback, long, hdlprocessrecord *);

extern boolean addprocess (hdlprocessrecord);

extern boolean addnewprocess (hdltreenode, boolean, langerrorcallback, long);

extern boolean processbackgroundtasks (void);

extern boolean processisoneshot (boolean);

extern boolean processagentsleep (long);

extern boolean processfindcode (hdltreenode, hdlprocessrecord *);

extern void processcodedisposed (long);

extern boolean processreplacecode (hdltreenode, hdltreenode);

extern boolean processdisposecode (hdltreenode);

extern boolean debuggingcurrentprocess (void);

extern boolean processstartprofiling (boolean);

extern boolean processstopprofiling (void);

extern boolean profilingcurrentprocess (void);

extern boolean processruncode (hdlprocessrecord, tyvaluerecord *);

extern boolean processruntext (Handle htext);

extern boolean processrunstring (bigstring);

extern boolean processrunstringnoerrorclear (bigstring);

extern boolean processkill (hdlprocessrecord);

extern void processinvalidglobals (WindowPtr);

extern boolean abort1shotprocess (void);

extern void killdependentprocesses (long);

extern boolean newprocessthread (tythreadmaincallback, tythreadmainparams, hdlprocessthread *);

extern boolean processpsuedothread (tythreadmaincallback, tythreadmainparams);

extern short processthreadcount (void);

extern boolean initprocessthread (bigstring);

extern void exitprocessthread (void);

extern hdlprocessthread getcurrentthread (void);

extern void endprocessthread (hdlprocessthread);

extern boolean infrontierthread (void);

extern boolean goodthread (hdlprocessthread);

extern boolean ingoodthread (void);

extern long getthreadid (hdlprocessthread);

extern hdlprocessthread getprocessthread (long);

extern hdlprocessthread nthprocessthread (long);

extern boolean wakeprocessthread (hdlprocessthread);

extern boolean killprocessthread (hdlprocessthread);

extern boolean processsleep (hdlprocessthread, unsigned long);

extern boolean processissleeping (hdlprocessthread hthread);

extern boolean processwake (hdlprocessthread);

extern boolean processyield (void);

extern boolean processyieldtoagents (void);

extern boolean processbusy (void);

extern boolean processnotbusy (void);

extern boolean processrunning (void);

extern boolean setprocesstimeslice (unsigned long);

extern boolean getprocesstimeslice (unsigned long *);

extern boolean setdefaulttimeslice (unsigned long);

extern boolean getdefaulttimeslice (unsigned long *);

extern boolean processtimesliceelapsed (void);

extern unsigned long processstackspace (void);

extern boolean scheduleprocess (hdlprocessrecord hprocess, hdlprocessthread *pnewthread);

extern void processscheduler (void);

extern void processchecktimeouts (void);

extern boolean processsymbolunlinking (hdlhashtable, hdlhashnode);

extern void processclose (void);

extern boolean processwaitforquiet (long timeoutticks);	

extern boolean initprocess (void);

extern boolean processgetstats (hdlhashtable); /*6.2b6 AR*/

#endif




