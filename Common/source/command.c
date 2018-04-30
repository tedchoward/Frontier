
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

#include "kb.h"
#include "memory.h"
#include "mouse.h"
#include "strings.h"
#include "resources.h"
#include "shell.rsrc.h"
#include "lang.h"
#include "langsystem7.h"
#include "miniwindow.h"
#include "process.h"
#include "cancoon.h"
#include "scripts.h"
#include "command.h"


#define cmdtextitem 1


static hdlprocessrecord quickscriptprocess = nil;


static boolean cmdsavestring ( short stringnumber, Handle htext ) {

	#pragma unused(stringnumber)
	
	//
	// the cmd dialog only has one string, so we ignore the stringnumber param.
	//
	// set the script string in the cancoondata record, for saving -- so we
	// have a permanent copy of the last script he wrote across invocations of
	// the program.
	//
	// 2004-12-03 creedon, aradke: bug fix for hscriptstring being NULL
	//
	// 5.0d14 dmb: hscriptstring is now a text handle, not a hdlstring.
	//
	// 1993-01-21 dmb: don't set superglobals manually anymore
	//
	
	register hdlcancoonrecord hc;
	
	Handle h;
	
	assert ( cancoonglobals != nil );
	
	hc = cancoonglobals; // copy into register
	
	if ( ! equalhandles ( htext, ( **hc ).hscriptstring ) ) {
	
		if ( ( **hc ).hscriptstring == NULL ) {
		
			copyhandle ( htext, &h );
			
			( **hc ).hscriptstring = h;
			
			}
		else
			copyhandlecontents ( htext, ( **hc ).hscriptstring );
			
		( **hc ).fldirty = true;
		
		}
	
	return ( true );
	
	} // cmdsavestring


static boolean cmdloadstring (short stringnumber, Handle *h) {
#pragma unused(stringnumber)

	/*
	1/21/93 dmb: don't set superglobals manually anymore
	
	5.0d14 dmb: hscriptstring is now a text handle, not a hdlstring.
	*/
	
	assert (cancoonglobals != nil);
	
	return (copyhandle ((**cancoonglobals).hscriptstring, h));
	} /*cmdloadstring*/


static boolean cmderrorroutine (long refcon, long lnum, short charnum, hdlhashtable *htable, bigstring bsname) {
#pragma unused (refcon, htable)

	/*
	a lang error occurred; select the offending text in the quickscript window
	*/
	
	
	if (bsname != nil) { /*request is for name*/
		
		getstringlist (commandlistnumber, commandtitlestring, bsname);
		
		return (true);
		}
	
	
	if (!startcmddialog ())
		return (false);
	
	if (!shellpushglobals (miniwindow))
		return (false);
	
	charnum = langgetsourceoffset (lnum, charnum);
		
	minisetselect (charnum, charnum);
	
	shellpopglobals ();
	
	return (true);
	} /*cmderrorroutine*/


static boolean cmdtexthit (Point pt) {
#pragma unused (pt)

	/*
	2.1b6 dmb: use debugging context for zooming, if available
	*/
	
	if (keyboardstatus.ctmodifiers && mousestatus.fldoubleclick) {
		
		bigstring bs;
		hdlhashtable hcontext = nil;
		
		minigetselstring (0, bs);
		
		if (scriptgetdebuggingcontext (&hcontext)) /*guaranteed to be non-nil*/
			pushhashtable (hcontext);
		
		langzoomobject (bs);
		
		if (hcontext != nil) {
			
			pophashtable ();
			
			scriptunlockdebuggingcontext ();
			}
		}
	
	return (true);
	} /*cmdtexthit*/


static boolean cmdprocesscallback (void) {
	
	miniinvalicon (idcommandconfig);
	
	return (true);
	} /*cmdprocesscallback*/


static pascal void *cmdthreadmain (tythreadmainparams hprocess) {
	
	/*
	2.1b2 dmb: use getobjectmodeldisplaystring to diplay result
	
	2.1b4 dmb: use the debugger context, if available (i.e. a script 
	is currently suspended).
	
	4.0.1b1 dmb: pass name of thread to initprocessthread; call exitprocessthread
	*/
	
	register hdlprocessrecord hp = (hdlprocessrecord) hprocess;
	tyvaluerecord val;
	bigstring bsresult;
	hdlhashtable hcontext = nil;
	boolean fl;
	
	initprocessthread (BIGSTRING ("\x0c" "quick script")); /*must call from every thread main*/
	
	if ((**hp).fldisposewhenidle)
		fl = false;
	else {
		
		if (scriptgetdebuggingcontext (&hcontext))
			(**hp).hcontext = hcontext;
		
		fl = processruncode (hp, &val);
		
		if (hcontext != nil)
			scriptunlockdebuggingcontext ();
		}
	
	disposeprocess (hp);
	
	if (!fl)
		setbooleanvalue (false, &val);
	
	getobjectmodeldisplaystring (&val, bsresult); /*hashgetvaluestring (val, bsresult)*/
	
	disposetmpvalue (&val);
	
	minisetwindowmessage (idcommandconfig, bsresult); /*can't assume miniwindow is around anymore*/
	
	quickscriptprocess = nil; /*clear "semaphore"*/
	
	exitprocessthread ();
	
	return (nil);
	} /*cmdthreadmain*/


static boolean cmdiconhit (void) {
	
	/*
	5/21/91 dmb: run the string as a process so we can handle errors nicely.
	
	8/20/91 dmb: when the process completes, we're in another thread and can't 
	assume that the window is around anymore, let alone that its globals are 
	set.  to handle this (crashing) situation, use new minisetwindowmessage 
	to display the result.
	
	10/25/91 dmb: check new process's disposewhenidleflag before running it.
	
	10/27/91 dmb: don't dispose result value if processruncode returns false
	
	12/12/91 dmb: extract the text handle from the dialog record's TE record to 
	overcome the miniwindow's string-oriented architecture.  Unfortunately, the
	text also saved in the database as a string, to this is just a band-aid, not 
	a real fix.
	
	2.1a6 dmb: reworked threading for thread mgr 1.1
	
	2.1b2 dmb: pass -1 for errorrefcon instead of zero to prevent top level 
	lexical scope from being transparent (i.e. visible to subroutines)
	
	5.0d14 dmb: minidialog now uses full text handles, not bigstrings
	
	5.0b17 dmb: minigetstring result is ours to consume
	*/
	
	Handle hscript;
	hdltreenode hcode;
	boolean fl;
	hdlprocessrecord hprocess;
	register hdlprocessrecord hp;
	hdlprocessthread hthread;
	
	if (quickscriptprocess != nil) {
		
		sysbeep ();
		
		return (false);
		}
	
	if (!langpusherrorcallback (&cmderrorroutine, (long) -1))
		return (false);
	
	minigetstring (0, &hscript);
	
	fl = langbuildtree (hscript, false, &hcode); /*consumes htext*/
	
	langpoperrorcallback ();
	
	if (!fl) /*syntax error*/
		return (false);
	
	langerrorclear (); /*compilation produced no error, be sure error window is empty*/
	
	if (!newprocess (hcode, true, &cmderrorroutine, (long) -1, &hprocess)) {
		
		langdisposetree (hcode);
		
		return (false);
		}
	
	hp = hprocess; /*copy into register*/
	
	(**hp).processstartedroutine = &cmdprocesscallback;
	
	(**hp).processkilledroutine = &cmdprocesscallback;
	
	#ifndef oldthreads
	
	if (!newprocessthread (&cmdthreadmain, (tythreadmainparams) hp, &hthread)) {
		
		disposeprocess (hp);
		
		return (false);
		}
	
	quickscriptprocess = hp; /*can only run one of these at a time*/
	
	return (true);	
	
	#else
	
	quickscriptprocess = hp; /*can only run one of these at a time*/
	
	if (!innewprocessthread (&hthread)) /*not in new thread -- we're done with synchonous stuff*/
		return (true);
	
	if ((hthread == nil) || (**hp).fldisposewhenidle)
		fl = false;
	else
		fl = processruncode (hp, &val);
	
	disposeprocess (hp);
	
	if (!fl)
		setbooleanvalue (false, &val);
	
	hashgetvaluestring (val, bsresult);
	
	disposetmpvalue (val);
	
	minisetwindowmessage (idcommandconfig, bsresult); /*can't assume miniwindow is around anymore*/
	
	quickscriptprocess = nil; /*clear "semaphore"*/
	
	endprocessthread (hthread);
	
	return (false); /*if we got here, hthread was nil*/
	
	#endif
	} /*cmdiconhit*/


static boolean cmdgettargetdata (short id) {
	
	/*
	we don't want our script to operate on our window
	
	2.1b14 dmb: make sure that the current process and our process aren't 
	both nil before rejecting targetship.

	5.0b6 dmb: allow wp verbs to target QuickScript
	*/
	
	if ((currentprocess != nil) && (currentprocess == quickscriptprocess))
		return (false);
	
	return (id == -1) || (id == idwordprocessor);
	} /*cmdgettargetdata*/


static void cmdcheckrunbutton (void) {
	
	/*
	boolean fl = debuggingcurrentprocess () || !processisoneshot (false);
	*/
	
	boolean fl = (quickscriptprocess == nil) /*&& !processbusy ()*/ ;
	
	(**minidata).fliconenabled = fl;
	} /*cmdcheckrunbutton*/


static boolean cmdsetfields (void) {
	
	register hdlminirecord hm = minidata;
	bigstring bs;
	
	(**hm).idconfig = idcommandconfig;
	
	(**hm).windowtype = ixcommandinfo;
	
	getstringlist (commandlistnumber, runiconstring, bs);
	
	copystring (bs, (**hm).iconlabel);
	
	getstringlist (commandlistnumber, commandtitlestring, bs);
	
	copystring (bs, (**hm).windowtitle);
	
	(**hm).savestringroutine = &cmdsavestring;
	
	(**hm).loadstringroutine = &cmdloadstring;
	
	(**hm).texthitroutine = &cmdtexthit;
	
	(**hm).iconenableroutine = &cmdcheckrunbutton;
	
	(**hm).iconhitroutine = &cmdiconhit;
	
	(**hm).gettargetdataroutine = &cmdgettargetdata;
	
	(**hm).cttextitems = 1;
	
	(**hm).textitems [0] = cmdtextitem;
	
	return (true);
	} /*cmdsetfields*/


boolean startcmddialog (void) {
	
	return (startminidialog (idcommandconfig, &cmdsetfields));
	} /*startcmddialog*/


boolean cmdstart (void) {
	
	return (ministart (idcommandconfig));
	} /*cmdstart*/


boolean cmdsetstring ( Handle h ) {

	//
	// 2007-07-25 creedon: created
	//
	
	if ( ! cmdsavestring ( -1, h ) )
		return ( false );
		
	minireloadstrings ( idcommandconfig );
	
	return ( true );
	
	}

