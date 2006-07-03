
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
#include <SetUpA5.h>
#include "mac.h"
#include "processinternal.h"
#endif

#include "error.h"
#include "ops.h"
#include "memory.h"
#include "threads.h"
#include "shell.h"

//#define flprofile

#ifdef flprofile
static long ctcreated = 0;
static long cttickstostart = 0;
static long latency;
#endif

tythreadcallbacks threadcallbacks;


#ifdef MACVERSION
	static ThreadID idwaitingthread = kNoThreadID;

	static ThreadID idmainthread;

#endif

#ifdef WIN95VERSION
	typedef struct tythreadmaininfo {
		
		HANDLE threadhandle;

		tythreadmaincallback threadmain;

		tythreadmainparams threadparams;

		void *threadglobals;

	#ifdef flprofile
		unsigned long tickscreated;
	#endif
		} tythreadmaininfo, **hdlthreadmaininfo;

	HANDLE hsharedthreadglobals;			// the mutex object for thread globals sharing
		
	DWORD ixlocalthreadglobals;				// Tls index for each thread's globals
	DWORD ixthreadglobalsgrabcount;			// Tls index of counter for nest globals grabbing
	DWORD idwaitingthread = 0;
	DWORD idmainthread;

	long GLOBALgrabcount;
#endif


#ifdef MACVERSION

static pascal void copythreadcontext (ThreadID hthread, void * hglobals) {
#pragma unused (hthread)

#if TARGET_API_MAC_OS8
	long curA5 = SetUpAppA5 ();
#endif
	
	(*threadcallbacks.swapoutcallback) (hglobals);
	
#if TARGET_API_MAC_OS8
	RestoreA5 (curA5);
#endif
	} /*copythreadcontext*/


static pascal void swapinthreadcontext (ThreadID hthread, void * hglobals) {
#pragma unused (hthread)

#if TARGET_API_MAC_OS8
	long curA5 = SetUpAppA5 ();
#endif
	
	(*threadcallbacks.swapincallback) (hglobals);
	
#if TARGET_API_MAC_OS8
	RestoreA5 (curA5);
#endif
	} /*swapinthreadcontext*/


static pascal void disposethreadcontext (ThreadID hthread, void * hglobals) {
#pragma unused (hthread)

#if TARGET_API_MAC_OS8
	long curA5 = SetUpAppA5 ();
#endif
	//Code change by Timothy Paustian Thursday, May 11, 2000 4:44:59 PM
	//Get rid of the UPPs for carbon here
	#if TARGET_API_MAC_CARBON == 1
	hdlthreadglobals	globals = (hdlthreadglobals)hglobals;
	HLock((Handle) globals);
	DisposeThreadSwitchUPP((**globals).threadInCallbackUPP);
	DisposeThreadSwitchUPP((**globals).threadOutCallbackUPP);
	DisposeThreadTerminationUPP((**globals).threadTerminateUPP);
	DisposeThreadEntryUPP((**globals).threadEntryCallbackUPP);
	HUnlock((Handle) globals);
	#endif
		
	(*threadcallbacks.disposecallback) (hglobals);
	
#if TARGET_API_MAC_OS8
	RestoreA5 (curA5);
#endif
	} /*disposethreadcontext*/


static void setthreadprocs (ThreadID idthread, void * hglobals) {
	
	/*
	before a thread is set loose, establish our custom swapping routines
	*/
	//Code change by Timothy Paustian Thursday, May 11, 2000 4:38:48 PM
	//creating the UPPs for use on carbon.
	#if TARGET_API_MAC_CARBON == 1
	hdlthreadglobals	globals = (hdlthreadglobals)hglobals;
	HLock((Handle) globals);
	(**globals).threadInCallbackUPP = NewThreadSwitchUPP(&swapinthreadcontext);
	(**globals).threadOutCallbackUPP = NewThreadSwitchUPP(&copythreadcontext);
	(**globals).threadTerminateUPP = NewThreadTerminationUPP(&disposethreadcontext);
	
	SetThreadSwitcher (idthread, (**globals).threadInCallbackUPP, hglobals, true);
	
	SetThreadSwitcher (idthread, (**globals).threadOutCallbackUPP, hglobals, false);
	
	SetThreadTerminator (idthread, (**globals).threadTerminateUPP, hglobals);
	
	HUnlock((Handle) globals);
	#else
		
	SetThreadSwitcher (idthread, &swapinthreadcontext, hglobals, true);
	
	SetThreadSwitcher (idthread, &copythreadcontext, hglobals, false);
	
	SetThreadTerminator (idthread, &disposethreadcontext, hglobals);
	#endif
		} /*setthreadprocs*/

#endif

boolean initmainthread (void * hglobals) {


	/*
	if (oserror (CreateThreadPool (kCooperativeThread, threadpoolsize, (Size) macmemoryconfig.minstacksize)))
		return (false);
	*/
	
#ifdef MACVERSION
	#if !TARGET_API_MAC_CARBON
		RememberA5 ();
		#endif /*for thread procs*/
	
	setthreadprocs (kApplicationThreadID, hglobals);
	
	GetCurrentThread (&idmainthread);
	
	return (true);
#endif

#ifdef WIN95VERSION

#ifdef PIKE
#ifndef OPMLEDITOR
	hsharedthreadglobals = CreateMutex (NULL, false, "Pike Thread Globals");
#else  // OPMLEDITOR
	hsharedthreadglobals = CreateMutex (NULL, false, "OPML Thread Globals");
#endif // !OPMLEDITOR
#else // !PIKE
	hsharedthreadglobals = CreateMutex (NULL, false, "Frontier Thread Globals");
#endif //!PIKE

	ixlocalthreadglobals = TlsAlloc ();
	
	if (ixlocalthreadglobals == (DWORD) -1) {
		
		oserror (GetLastError ());

		return (false);
		}

	if (!TlsSetValue (ixlocalthreadglobals, hglobals)) {
		
		oserror (GetLastError ());

		return (false);
		}
	
	(*threadcallbacks.swapincallback) (hglobals);

	ixthreadglobalsgrabcount = TlsAlloc ();
	
	TlsSetValue (ixthreadglobalsgrabcount, 0);

	GLOBALgrabcount = 0;

	idmainthread = GetCurrentThreadId ();

	return (true);
#endif
	} /*initmainthread*/


boolean inmainthread (void) {
	
#ifdef MACVERSION
	ThreadID idthread;

	GetCurrentThread (&idthread);
#endif

#ifdef WIN95VERSION
	DWORD idthread;
	
	idthread = GetCurrentThreadId ();
#endif
	
	return (idthread == idmainthread);
	} /*inmainthread*/


boolean attachtomainthread (long idthread) {
#ifdef MACVERSION
#	pragma unused (idthread)
#endif

#ifdef WIN95VERSION
	return (AttachThreadInput (idthread, idmainthread, true));
#else
	return (true); /*it's a no-op*/
#endif
	}/*attachtomainthread*/


#ifdef WIN95VERSION

static void *threadentry (hdlthreadmaininfo htmi) {
	
	/*
	5.0b14 dmb: plugged one memory leak: the handle to our thread

	5.0.2 dmb/rab: call disposecallback before releasing mutex

	5.1.5 dmb: attach thread input to main thread
	*/

	tythreadmaininfo tmi = **htmi;
	ptrvoid hglobals = tmi.threadglobals;
	ptrvoid x;

	disposehandle ((Handle) htmi); // we have local copy, we're done with them

	// attach globals to the thread
	if (!TlsSetValue (ixlocalthreadglobals, tmi.threadglobals)) {
		
		oserror (GetLastError ());

		return (NULL);
		}
	
	// swap self in
	TlsSetValue (ixthreadglobalsgrabcount, (ptrvoid) 1);

	GLOBALgrabcount = 1;
	
	if (WaitForSingleObject (hsharedthreadglobals, INFINITE) == WAIT_FAILED) {
		
		oserror (GetLastError ());
		}
	else {
		
		DWORD idthread = GetCurrentThreadId ();
		BOOL res;

		#ifdef flprofile
		cttickstostart += gettickcount () - tmi.tickscreated;
		ctcreated += 1;
		latency = cttickstostart / ctcreated;
		#endif

		if (idthread == idwaitingthread)
			idwaitingthread = 0;
		
		(*threadcallbacks.swapincallback) (tmi.threadglobals);
		
		res = attachtomainthread (idthread); //5.1.5

		srand (GetTickCount ()); // 5.1.5 dmb: for multithreaded stdlib

		// call main
		x = (*tmi.threadmain) (tmi.threadparams);
		
		(*threadcallbacks.swapoutcallback) (tmi.threadglobals);
		
		// clean up
		(*threadcallbacks.disposecallback) (tmi.threadglobals);

		// swap self out, clean up
		ReleaseMutex (hsharedthreadglobals);
		}
	
	verify (CloseHandle (tmi.threadhandle));
	
	return (x);
	} /*threadentry*/

#endif


boolean newthread (tythreadmaincallback threadmain, tythreadmainparams threadparams, void *hglobals, hdlthread *hthread) {

#ifdef MACVERSION
	OSErr err;
	ThreadID idthread;
	//Code change by Timothy Paustian Thursday, May 11, 2000 4:49:58 PM
	//we need to create a thread callback UPP for carbon. 
	#if TARGET_API_MAC_CARBON == 1
	hdlthreadglobals	globals = (hdlthreadglobals)hglobals;
	(**globals).threadEntryCallbackUPP = NewThreadEntryUPP((ThreadEntryProcPtr)threadmain);
	err = NewThread (kCooperativeThread, (**globals).threadEntryCallbackUPP, threadparams, (Size) macmemoryconfig.minstacksize, kUsePremadeThread + kCreateIfNeeded + kFPUNotNeeded, nil, &idthread);
	#else
		

	err = NewThread (kCooperativeThread, threadmain, threadparams, (Size) macmemoryconfig.minstacksize, 
		
		kUsePremadeThread + kCreateIfNeeded + kFPUNotNeeded, nil, &idthread);
	#endif
		
	if (oserror (err))
		return (false);
	
	setthreadprocs (idthread, hglobals);
	
	idwaitingthread = idthread; /*make sure we get scheduled*/
	
	*hthread = (hdlthread) idthread;
	
	return (true);
#endif

#ifdef WIN95VERSION
	Handle h;
	DWORD idthread;
	tythreadmaininfo tmi;
	hdlthreadmaininfo htmi;
	
	tmi.threadmain = threadmain;
	tmi.threadparams = threadparams;
	tmi.threadglobals = hglobals;
	
	if (!newfilledhandle (&tmi, sizeof (tmi), (Handle *) &htmi))
		return (false);

	h = CreateThread (NULL,  // no security attributes 
					0,      // use default stack size 
					(LPTHREAD_START_ROUTINE) threadentry, 
					(LPVOID) htmi, // param to thread func 
					CREATE_SUSPENDED, // creation flag 
					&idthread);       // thread identifier

	if (h == NULL) {
		
		oserror (GetLastError ());
		
		return (false);
		}
	
	idwaitingthread = idthread;

	#ifdef flprofile
	(**htmi).tickscreated = gettickcount();
	#endif

	(**htmi).threadhandle = h;

	*hthread = (hdlthread) h;
	
	ResumeThread (h); // let'r fly next yield

	return (true);
#endif
	} /*newthread*/


boolean threadiswaiting (void) {

	/*
	5.1.5 dmb: support this verb on windows
	*/

#ifdef MACVERSION
	return (idwaitingthread != kNoThreadID);
#endif

#ifdef WIN95VERSION
	return (idwaitingthread != 0);
#endif
	} /*threadiswaiting*/


boolean threadyield (boolean flresting) {
#pragma unused(flresting)

	/*
	5.0a19 dmb: added flresting parameter
	*/

#ifdef MACVERSION
	ThreadID idthread = idwaitingthread;
	
	idwaitingthread = kNoThreadID;
	
	return (YieldToThread (idthread) == noErr);
#endif

#ifdef WIN95VERSION
	ptrvoid hglobals = TlsGetValue (ixlocalthreadglobals);
	
	(*threadcallbacks.swapoutcallback) (hglobals);
	
	if (flresting)
		SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_BELOW_NORMAL);

	ReleaseMutex (hsharedthreadglobals);

	Sleep (0L); /*6.1b12 AR: give up the rest of our timeslice*/
	
	WaitForSingleObject (hsharedthreadglobals, INFINITE);

	if (flresting) {
		
	//	if (inmainthread ())
	//		SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_ABOVE_NORMAL);
	//	else
			SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_NORMAL);
		}

	(*threadcallbacks.swapincallback) (hglobals);
	
	return (true);
#endif
	} /*threadyield*/


#ifdef MACVERSION

long grabthreadglobals (void) {return 0;}

long releasethreadglobals (void) {return 0;}

long grabthreadglobalsnopriority (void) {return 0;}

long releasethreadglobalsnopriority (void) {return 0;}

#endif

#ifdef WIN95VERSION

static threadglobalsgrabcount = 0; // to allow nested calls

#ifdef fldebug

void checkthreadglobals (void) {
	
	extern ptrvoid getcurrentthreadglobals ();
	
	assert (TlsGetValue (ixlocalthreadglobals) == getcurrentthreadglobals ());
	} /*checkthreadglobals*/

#endif

long grabthreadglobals (void) {
	
	/*
	intended to be called only from main, when it actually needs 
	its globals to be set, and is willing to be blocked.to get them.
	must be balanced with a call to releasethreadglobals
	*/
	
	ptrvoid hglobals;
	long grabcount = (long) TlsGetValue (ixthreadglobalsgrabcount);
	
//	assert (inmainthread ());

	if (grabcount++ == 0) {
	
		DWORD idthread = GetCurrentThreadId ();

		idwaitingthread = idthread;
		
		WaitForSingleObject (hsharedthreadglobals, INFINITE);
		
		if (idwaitingthread == idthread)
			idwaitingthread = 0;

	//	if (idthread == idmainthread)
	//		SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_ABOVE_NORMAL);
	//	else
			SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_NORMAL);

		hglobals = TlsGetValue (ixlocalthreadglobals);
		
		(*threadcallbacks.swapincallback) (hglobals);
		}

	TlsSetValue (ixthreadglobalsgrabcount, (ptrvoid) grabcount);

	GLOBALgrabcount = grabcount;

	return (grabcount);
	} /*grabthreadglobals*/


long releasethreadglobals (void) {

	ptrvoid hglobals;
	long grabcount = (long) TlsGetValue (ixthreadglobalsgrabcount);
	
	if (--grabcount == 0) {
		
		hglobals = TlsGetValue (ixlocalthreadglobals);
		
		(*threadcallbacks.swapoutcallback) (hglobals);
		
		SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_ABOVE_NORMAL);

		ReleaseMutex (hsharedthreadglobals);
		}
	
	TlsSetValue (ixthreadglobalsgrabcount, (ptrvoid) grabcount);

	GLOBALgrabcount = grabcount;

	return (grabcount);
	} /*releasethreadglobals*/

long grabthreadglobalsnopriority (void) {
	
	/*
	intended to be called only from main, when it actually needs 
	its globals to be set, and is willing to be blocked.to get them.
	must be balanced with a call to releasethreadglobalsnopriority
	*/
	
	ptrvoid hglobals;
	long grabcount = (long) TlsGetValue (ixthreadglobalsgrabcount);
	
//	assert (inmainthread ());

	if (grabcount++ == 0) {
		
		DWORD idthread = GetCurrentThreadId ();

		idwaitingthread = idthread;
		
		WaitForSingleObject (hsharedthreadglobals, INFINITE);
		
		if (idwaitingthread == idthread)
			idwaitingthread = 0;
		
		hglobals = TlsGetValue (ixlocalthreadglobals);
		
		(*threadcallbacks.swapincallback) (hglobals);
		}

	TlsSetValue (ixthreadglobalsgrabcount, (ptrvoid) grabcount);

	GLOBALgrabcount = grabcount;

	return (grabcount);
	} /*grabthreadglobalsnopriority*/


long releasethreadglobalsnopriority (void) {

	ptrvoid hglobals;
	long grabcount = (long) TlsGetValue (ixthreadglobalsgrabcount);
	
	if (--grabcount == 0) {
		
		hglobals = TlsGetValue (ixlocalthreadglobals);
		
		(*threadcallbacks.swapoutcallback) (hglobals);
		
		ReleaseMutex (hsharedthreadglobals);
		}
	
	TlsSetValue (ixthreadglobalsgrabcount, (ptrvoid) grabcount);

	GLOBALgrabcount = grabcount;

	return (grabcount);
	} /*releasethreadglobalsnopriority*/

#endif


boolean threadstartup (void) {
	
	/*
	called by thread mains to let us know that hnow thead is waiting anymore

	under windows, we must grab a critical section before we can proceed.

	5.0a10 dmb: return true for windows! (was random)
	*/

#ifdef MACVERSION
	idwaitingthread = kNoThreadID; /*not waiting any more*/
	
	return (true);
#endif

#ifdef WIN95VERSION
	return (true);
#endif
	} /*threadstartup*/


void threadshutdown (void) {
	
#ifdef WIN95VERSION
#endif
	} /*threadshutdown*/


boolean threadsleep (hdlthread hthread) {

#ifdef MACVERSION
	ThreadID idthread = kCurrentThreadID;
	OSErr err;
	
	if (hthread != nil)
		idthread = (ThreadID) hthread;
	
	err = SetThreadState (idthread, kStoppedThreadState, kNoThreadID);

	if (err != noErr)
		return (false);
	
	if (idthread == idwaitingthread) //set by wakeup?
		idwaitingthread = 0;
	
	return (true);
#endif

#ifdef WIN95VERSION
	DWORD count;
	HANDLE hcurrent = GetCurrentThread ();

	if (hthread == nil)
		hthread = hcurrent;
	
	if (hthread == hcurrent)
		releasethreadglobalsnopriority ();

	count = SuspendThread (hthread);

	//if (idthread == idwaitingthread) //set by wakeup?
	//	idwaitingthread = 0;
	
	if (hthread == hcurrent)
		grabthreadglobalsnopriority ();

	return (count == 0);
#endif
	} /*threadsleep*/


boolean threadissleeping (hdlthread hthread) {
	
	/*
	see if the thread is sleeping
	*/
	
#ifdef MACVERSION
	ThreadState state;
	
	if (hthread == nil)
		return (false);
	
	if (GetThreadState ((ThreadID) hthread, &state) != noErr)
		return (false);
	
	return (state == kStoppedThreadState);
#endif

#ifdef WIN95VERSION
	shellerrormessage ("\x29" "Can't call threadissleeping under Windows");
	
	return (true);
#endif
	} /*threadissleeping*/


boolean threadwake (hdlthread hthread, boolean flpriority) {
	
	/*
	wake the thread
	*/
	
#ifdef MACVERSION
	OSErr err;
	
	if (!threadissleeping (hthread))
		return (false);
	
	err = SetThreadState ((ThreadID) hthread, kReadyThreadState, kNoThreadID);
	
	if (err != noErr)
		return (false);
	
	if (flpriority && !idwaitingthread)
		idwaitingthread = (ThreadID) hthread;
	
	return (true);
#endif

#ifdef WIN95VERSION
	DWORD count;
	
	count = ResumeThread (hthread);
	
	if (count != (DWORD) 1)
		return (false); /*either an error or it's still suspended*/
	
	// can't use idwaitingthread; maybe we should make it and idmainthread HANDLEs
	
	return (true);
#endif
	} /*threadwake*/


boolean initthreads (void) {
	
#ifdef MACVERSION
	long attrs;
	
	if (!gestalt (gestaltThreadMgrAttr, &attrs))
		return (false);
	
	if ((attrs & (1 << gestaltThreadMgrPresent)) == 0)
		return (false);
	
	return (true);
#endif

#ifdef WIN95VERSION	
	return (true);
#endif
	} /*inittheads*/


