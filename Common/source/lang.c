
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

#include "memory.h"
#include "db.h"
#include "error.h"
#include "cursor.h"
#include "kb.h"
#include "font.h"
#include "resources.h"
#include "strings.h"
#include "threads.h"
#include "ops.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langparser.h"
#include "langsystem7.h"
#include "op.h"
#include "shell.rsrc.h"
#include "shellhooks.h"
#include "process.h"
#include "tablestructure.h"




byte bstrue [] = "\x04" "true"; /*so we don't replicate this constant*/

byte bsfalse [] = "\x05" "false"; /*or this one, either*/


hdlhashtable hmagictable = nil; /*for communication with evaluatelist*/

hdlhashtable hkeywordtable; /*holds the language's keywords*/

hdlhashtable hbuiltinfunctions; /*holds the names of the built-in functions*/

hdlhashtable hconsttable; /*holds the names of built-in constants, like true and up*/

boolean flbreak = false; /*for loop and break ops*/

boolean flcontinue = false; /*for loops*/

boolean flreturn = false; /*for return op*/

boolean flscriptrunning = false; /*for nesting within thread*/

boolean flscriptresting = false; /*for being friendly, performance-wise*/

hdltreenode herrornode = nil;

tylangcallbacks langcallbacks; /*routines that wire the language into environment*/


static short ctrootchains = 0; /*number of hashtable chains that include the root*/

static boolean flevaluateglobalwith = false; /*dmb 4.1b12 - new flag for langcard support*/



static boolean langtracktimeslice (tyerrorrecord *slice) {
	
	hdlerrorstack hs = langcallbacks.scripterrorstack;
	hdlprocessrecord hp = currentprocess;
	hdlhashtable hprofiledata;
	tyerrorrecord *pe = slice;
	hdlhashtable htable;
	bigstring bsname, bspath;
	hdlhashnode hnode;
	tyvaluerecord val;
	
	if (hp == nil)
		return (false);
	
	if (hs == nil)
		return (false);
	
	hprofiledata = (hdlhashtable) (**(**hp).hprofiledata).variabledata;
	
	if (hprofiledata == nil)
		return (false);
	
	if ((*pe).errorcallback == nil
		 || !(*(*pe).errorcallback) ((*pe).errorrefcon, 0, 0, &htable, bsname)) {
	
		langgetstringlist (anomynousthreadstring, bspath);
		}
	else {
		if (htable == nil)
			copystring (bsname, bspath);
		else
			langexternalgetfullpath (htable, bsname, bspath, nil);
		}
	
	if (hashtablelookupnode (hprofiledata, bspath, &hnode)) {
		
		(**hnode).val.data.longvalue += (*pe).profiletotal;
		}
	else {
		setlongvalue ((*pe).profiletotal, &val);
		
		hashtableassign (hprofiledata, bspath, val);
		}
		
	return (true);
	} /*langtracktimeslice*/


boolean langstartprofiling (void) {
	
	/*
	5.0.2b15 dmb: initialize profiling stats for current source frame
	*/
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	register hdlprocessrecord hp = currentprocess;
	//short ix, ixtop;
	tyerrorrecord *pe;
	unsigned long ticksnow = gettickcount ();
	
	if ((hs == nil) || ((**hs).toperror <= 0))
		return (false);
	
	if (hp && (**hp).flprofiling) { // should always be true
		
		/*
		ixtop = (**hs).toperror;
		
		for (ix = 0; ix < ixtop; ++ix) {
			
			pe = &(**hs).stack [ix];
			
			(*pe).profilebase = ticksnow;
			
			(*pe).profiletotal = 0;
			}
		*/
		pe = &(**hs).stack [(**hs).toperror - 1];
		
		(*pe).profilebase = ticksnow;
		
		(*pe).profiletotal = 0;
		}
	
	return (true);
	} /*langstartprofiling*/


boolean langstopprofiling (void) {
	
	/*
	5.0.2b15 dmb: grab profiling stats for current source frame
	*/
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	register hdlprocessrecord hp = currentprocess;
	short ix;
	tyerrorrecord item;
	unsigned long ticksnow = gettickcount ();
	
	if ((hs == nil) || ((**hs).toperror <= 0))
		return (false);
	
	if (hp && (**hp).flprofiling) {
		
		for (ix = (**hs).toperror - 1; ix >= 0; --ix) {
			
			item = (**hs).stack [ix];
			
			if (item.profilebase != 0) { // included in profile
			
				item.profiletotal += ticksnow - item.profilebase;
				
				langtracktimeslice (&item);
				}
			}
		}
	
	return (true);
	} /*langstopprofiling*/


boolean langpusherrorcallback (langerrorcallback errorroutine, long errorrefcon) {
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	register hdlprocessrecord hp = currentprocess;
	tyerrorrecord *pe;
	tyerrorrecord item;
	
	if (hs == nil)
		return (false);
	
	if (!langcheckstacklimit (iderrorcallbackstack, (**hs).toperror, cterrorcallbacks)) /*overflow!*/
		return (false);
	
	item.errorcallback = errorroutine;
	
	item.errorline = ctscanlines;
	
	item.errorchar = ctscanchars;	
	
	item.errorrefcon = errorrefcon;
	
	#ifdef flnewfeatures
	
	item.profilebase = 0;
	
	if (hp && (**hp).flprofiling) {
		
		item.profilebase = gettickcount ();
		
		item.profiletotal = 0;
	
		if ((**hp).flprofilesliced && ((**hs).toperror > 0)) { // add sliced time to previous function
			
			pe = &(**hs).stack [(**hs).toperror - 1];
			
			(*pe).profiletotal += item.profilebase - (*pe).profilebase;
			}
		}
	
	#endif
	
	(**hs).stack [(**hs).toperror++] = item;
	
	return (true);
	} /*langpusherrorcallback*/


boolean langpoperrorcallback (void) {
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	register hdlprocessrecord hp = currentprocess;
	tyerrorrecord *pe;
	short ixtop;
	
	if (hs == nil)
		return (false);
	
	if ((**hs).toperror <= 0)
		return (false);
	
	ixtop = --(**hs).toperror;
	
	#ifdef flnewfeatures
	
	if (hp && (**hp).flprofiling) {
	
		tyerrorrecord item;
		boolean flsliced = (**hp).flprofilesliced;
		
		item = (**hs).stack [ixtop]; // the one we just popped
		
		item.profiletotal += gettickcount () - item.profilebase;
		
		pe = &(**hs).stack [ixtop - 1]; // new new current top item
		
		if (flsliced || ixtop == 0 || (*pe).errorrefcon != item.errorrefcon)
			langtracktimeslice (&item);
		
		if ((**hp).flprofilesliced && (ixtop > 0)) // restart previous function timer
			(*pe).profilebase = gettickcount ();
		}
	
	#endif
	
	return (true);
	} /*langpoperrorcallback*/


#ifdef flnewfeatures	// flstacktrace

boolean langseterrorcallbackline (void) {
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	tyerrorrecord *pe;
	
	if (hs == nil)
		return (false);
	
	if ((**hs).toperror <= 0)
		return (false);
	
	pe = &(**hs).stack [(**hs).toperror - 1];
	
	(*pe).errorline = ctscanlines;
	
	(*pe).errorchar = ctscanchars;
	
	return (true);
	} /*langpoperrorcallback*/

#endif


unsigned long langgetsourceoffset (unsigned long lnum, unsigned short charnum) {

	return (parsegetscanoffset (lnum, charnum));
	} /*langgetsourceoffset*/


void langsetsourceoffset (unsigned long offset) {

	parsesetscanoffset (offset);
	} /*langsetsourceoffset*/


boolean langfinderrorrefcon (long errorrefcon, langerrorcallback *errorcallback) {
	
	/*
	return true if the errorrefcon is found in the scripterror stack.
	
	this is used to determine if a piece of code is in use.
	
	2.1b3 dmb: return the callback associated with the refcon; scripts.c 
	uses this routine for tracking local subroutine calls
	*/
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	register short ix;
	register short ixtop;
	register long refcon = errorrefcon;
	
	if (hs == nil)
		return (false);
	
	ixtop = (**hs).toperror;
	
	for (ix = 0; ix < ixtop; ++ix) {
		
		if ((**hs).stack [ix].errorrefcon == refcon) { /*found it*/
			
			*errorcallback = (**hs).stack [ix].errorcallback;
			
			return (true);
			}
		}
	
	return (false);
	} /*langfinderrorrefcon*/


static void langprescript (void) {
	
	/*
	run this code once each time you are about to run or compile a script.
	
	1/9/91 dmb: used to call cleartmpstack from here.  presumably, the intention 
	was to start with a clean slate, and to avoid accumlating stuff in the 
	root table's temp stack.  however, this routine can be called pretty much 
	asynchonously with repect to language execution, so we don't know which 
	hash table is current, let alone whether stuff in the temp stack is still 
	needed.  therefore, it seems that the right place to make this call is 
	in langpopscopechain (a recently-added routine), when popping the last chain.
	
	1/25/91 dmb: also need to clear return & break globals here, or else 
	script may not run properly
	*/
	
	fllangerror = false; /*the user hasn't seen an error dialog yet*/
	
	flreturn = false;
	
	flbreak = false;
	
	flcontinue = false;
	} /*langprescript*/


static void langpostscript (void) {
	
	/*
	call this after you've finished evaluating a tree or evaluating a list,
	but are not recursively inside the tree interpreter.
	*/
	
	flreturn = false;
	
	flbreak = false;
	
	flcontinue = false;
	} /*langpostscript*/


static bigstring bsparsererror;

static long lnumerror, cnumerror;

static ptrvoid refconerror;


static boolean langcompileerror (bigstring bs, ptrvoid refcon) {

	/*
	5.1.2 dmb: added refconerror to capture refcon value for orig callback
	*/

	if (isemptystring (bsparsererror)) { /*1st error reported*/
		
		copystring (bs, bsparsererror); 
		
		lnumerror = ctscanlines;
		
		cnumerror = ctscanchars;

		refconerror = refcon;
		}
	
	return (true);
	} /*langcompileerror*/


boolean langcompiletext (Handle htext, boolean fllinebased, hdltreenode *hcode) {
	
	/*
	parse the program text in htext and return true if it was free of 
	syntax errors.
	
	in either case, we dispose of the text, assuming that the caller has 
	no more need of it.
	
	12/26/91 dmb: if yyparse return val is 2, don't dispose yy vals
	
	6/23/92 dmb: moved parseerror here from langerror.c; now just set 
	global string & check it after yyparse completes.  this allows error 
	handler to not have to worry about parser reentrancy (not!)
	
	5/7/93 dmb: added fllinebased parameter. when false, ctscanlines/ctscanchars 
	are really the high & low words of an absolute offset.
	*/
	
	register hdltreenode h;
	int yyresult;
	langerrormessagecallback savecallback;
	
	*hcode = nil; /*default, no code generated*/
	
	parsesetscanstring (htext, fllinebased);  /*the scanner sets its globals with this text*/
	
	setemptystring (bsparsererror);
	
	langstarttrace (); /*prepare the trace dialog, if it's active*/
	
	savecallback = langcallbacks.errormessagecallback;
	
	langcallbacks.errormessagecallback = &langcompileerror;
	
	yyresult = yyparse (); /*parse the text, producing parsetree -- if no syntax errors*/
	
	langcallbacks.errormessagecallback = savecallback;
	
	langendtrace (); /*close the tracing dialog box*/
	
	disposehandle (htext); 
	
	h = yyval; /*copy into register*/ /*parseresult.parsetree*/
	
	if (yyresult != 0) { /*an error occurred*/
		
		if (yyresult != 2) { /*not an out-of-mem error*/
			
			if (h && *h) //Win safety
				langdisposetree (h); /*dispose whatever was built...*/
			
			langdisposetree (yylval); /*...including this piece*/
			}
		
		if (!isemptystring (bsparsererror)) {
			
			ctscanlines = lnumerror;
			
			ctscanchars = cnumerror;
			
			(*savecallback) (bsparsererror, refconerror);
			}
		
		return (false);
		}
	
	if (h == nil) /*syntax error, or system error*/
		return (false);
	
	*hcode = h; 
	
	return (true);
	} /*langcompiletext*/


boolean langbuildtree (Handle htext, boolean fllinebased, hdltreenode *hcode) {
	
	/*
	wrapper around langcompiletext, resets lang globals, clears error window
	*/
	
	langprescript (); /*initialize lang globals, about to compile a script*/
	
	return (langcompiletext (htext, fllinebased, hcode)); /*disposes htext*/
	} /*langbuildtree*/


boolean langdefaultpushtable (hdlhashtable *htable) {
	
	register hdlhashtable ht;
	
	if (!newhashtable (htable))
		return (false);
		
	ht = *htable; /*copy into register*/
	
	(**ht).fllocaltable = true;
	
	chainhashtable (ht); /*link it into the runtime stack*/
	
	return (true);
	} /*langdefaultpushtable*/
	
	
boolean langdefaultpoptable (hdlhashtable htable) {
	
	register hdlhashtable ht = currenthashtable;
	
	unchainhashtable ();
	
	disposehashtable (ht, false);
	
	return (true);
	} /*langdefaultpoptable*/


static boolean langerrorhook (bigstring bs) {
	
	langerrormessage (bs);
	
	return (false); /*consume the error*/
	} /*langerrorhook*/


void langhookerrors (void) {

	/*
	5.1.3 dmb: export this functionality: hook shell errors into lang error system
	*/
	
	shellpusherrorhook (&langerrorhook);
	} /*langhookerrors*/


void langunhookerrors (void) {

	shellpoperrorhook ();
	} /*langunhookerrors*/


static boolean langtraperrormessage (bigstring bsmsg, ptrstring perrorstring) {
	
	/*
	5.0a25 dmb: trap errors so caller can 
	make a better message that includes the more information
	*/
	
	copystring (bsmsg, perrorstring);
	
	return (true);
	} /*langtraperrormessage*/


void langtraperrors (bigstring bserror, langerrormessagecallback *savecallback, ptrvoid *saverefcon) {
	
	/*
	hook errors into bserror so caller can embellish
	*/

	setemptystring (bserror);
	
	*saverefcon = langcallbacks.errormessagerefcon;
	
	*savecallback = langcallbacks.errormessagecallback;
	
	langcallbacks.errormessagerefcon = bserror;
	
	langcallbacks.errormessagecallback = (langerrormessagecallback) &langtraperrormessage;
	} /*langtraperrors*/


void languntraperrors (langerrormessagecallback savecallback, ptrvoid saverefcon, boolean flerror) {

	/*
	unhook errors. if flerror is true, then the caller wants some sort of error 
	message in the old error string (that we have set as the errormessagerefcon)
	*/
	
	ptrstring bserror = (ptrstring) langcallbacks.errormessagerefcon;
	
	if (flerror) {
		
		if (isemptystring (bserror))
			if (getoserror () != noErr)
				getsystemerrorstring (getoserror (), bserror);
			else
				getstringlist (langerrorlist, undefinederror, bserror);
		}
	
	langcallbacks.errormessagecallback = savecallback;
	
	langcallbacks.errormessagerefcon = saverefcon;
	} /*languntraperrors*/


void langpushscopechain (void) {
	
	/*
	1/8/91 dmb: since language execution threads can be stacked (i.e. 
	one process can run through the background task while another is 
	evaluating), we must always make sure to start a new hash table 
	chain for variable scoping.
	
	we push the nil table so we don't pick up the local chain of something 
	that's already running
	
	we call chainhashtable for the global table (root) so that its flchained 
	flag will be set to protect it from untimely disposal
	
	note that we no longer attempt to set a global "current" hash table from the 
	upper layers (cancoon.c, tablestructure.c), so when nothing is pushed, the 
	current hash table is nil.
	*/
	
	pushhashtable (nil); /*clean slate*/
	
	chainhashtable (roottable); /*establishes new chain; sets flchained bit*/
	
	++ctrootchains;
	} /*langpushscopechain*/


void langpopscopechain (void) {
	
	register hdlhashtable ht = currenthashtable;
	
	unchainhashtable ();
	
	pophashtable (); /*restore original*/
	
	--ctrootchains;
	
	if (ctrootchains == 0) { /*we've popped the last scope chain*/
		
		if ((**ht).fldisposewhenunchained) /*file was probably closed*/
			disposehashtable (ht, false);
		
		else {
			pushhashtable (ht);
			
			cleartmpstack (); /*see comment in langprescript*/
			
			pophashtable ();
			}
		}
	else
		(**ht).flchained = true; /*this guy is still at the end of another chain*/
	} /*langpopscopechain*/


static boolean renumberlinesvisit (hdltreenode hnode, ptrvoid refcon) {
	
	/*
	when langrun executes code from within a script, it tries to preserve 
	the ability to locate errors and step in the debugger.  from the 
	point of view of the running script, the code about to be executed 
	is on the line that triggered the call to langrun.  our job is to 
	patch the code tree so that every node points to the current line and 
	character position
	*/
	
	(**hnode).lnum = ctscanlines;
	
	(**hnode).charnum = ctscanchars;
	
	return (true);
	} /*renumberlinesvisit*/


boolean langrun (Handle htext, tyvaluerecord *val) {
	
	/*
	compile and run the program source in htext.  if the syntax checks, and
	the program runs without error, return the result as a value record.
	
	we consume the text handle -- it is disposed before we return.
	
	1/3/90 dmb: temporarily patch out most langcallbacks, both for speed 
	and to avoid interference with the current process, which may be 
	being debugged.
	
	1/8/91 dmb: see comment in pushscopechain
	
	1/14/91 dmb: make sure that the value is maintained in a temp stack if 
	one is available.  if not, caller is reponsible for disposal
	
	10/23/91 dmb: reworked handling of flscriptalreadyrunning case.  we now 
	try to preserve the execution environment as much as possible so that 
	mechanisms in place in the parent script continue to function, like 
	error reporting and debugging.
	
	11/5/91 dmb: don't clear langerror window if script is being run with 
	errors disabled.
	
	3/30/93 dmb: process.c no longer keeps it's callbacks set up in the main 
	thread, so when a script isn't already running we don't have to undo them.
	
	also, if a table is already pushed when a script isn't running, we assume 
	our caller is in control and doesn't want us to push a scope chain.
	
	3.0.2 dmb: if a script is already running, pass a refcon of -1 along with 
	nil so that a "with" statement in the evaluated text won't carry through 
	to subroutine calls

	4.1b4 dmb: if a script is not already running, push a normal nil callback;
	initlang in langstartup.c no longer does this
	
	4.1b11 dmb: use new flevaluateglobalwith flag to avoid passing this as 
	a parameter everywhere. Running card scripts needs it.
	*/
	
	register boolean fl;
	hdltreenode hcode;
	/*tylangcallbacks savecallbacks;*/
	boolean fltmpval;
	register boolean flscriptalreadyrunning = flscriptrunning;
	register boolean fltablealreadypushed = false;
	unsigned short savelines = ctscanlines;
	unsigned short savechars = ctscanchars;
	
#ifdef WIN95VERSION
checkthreadglobals ();
#endif

	setbooleanvalue (false, val);
	
	flscriptrunning = true;
	
	if (flscriptalreadyrunning && !flevaluateglobalwith) /*4.1b12 dmb*/
		langpusherrorcallback (nil, -1L);
	else
		langpusherrorcallback (nil, 0L);
	
	flevaluateglobalwith = false; /*reset every time*/
	
	fl = langbuildtree (htext, true, &hcode);
	
	if (!fl)
		goto exit;
	
	if (flscriptalreadyrunning) { /*make nodes of this code tree point to current position*/
		
		ctscanlines = savelines;
		
		ctscanchars = savechars;
		
		langvisitcodetree (hcode, &renumberlinesvisit, nil);
		}
	
	else {
		
		fltablealreadypushed = currenthashtable != nil;
		
		if (!fltablealreadypushed)
			langpushscopechain ();
		
		if (langerrorenabled ()) /*compilation produced no error, be sure error window is empty*/
			langerrorclear ();
		
		langhookerrors ();
		}
	
	fl = evaluatelist ((**hcode).param1, val);
	
	if (!flscriptalreadyrunning) {
		
		langunhookerrors ();
		
		if (!fltablealreadypushed) {
		
			fltmpval = exemptfromtmpstack (val); /*must survive disposing of local scope chain*/
			
			langpopscopechain (); /*pop the runtime stack*/
			
			if (fltmpval) /*insert into the next-most-global tmpstack, if one exists*/
				pushvalueontmpstack (val);
			}
		}
	
	langpostscript ();
	
	langdisposetree (hcode);
	
	exit:
	
	/*
	if (flscriptalreadyrunning)
	*/
		langpoperrorcallback ();
	
	flscriptrunning = flscriptalreadyrunning;
	
	return (fl);
	} /*langrun*/


boolean langrunhandle (Handle htext, bigstring bsresult) {
	
	/*
	compile and run the program source in bsprogram.  if the syntax checks, and
	the program runs without error, return the result as a string.
	
	we consume the text handle -- it is disposed by langrun
	*/
	
	register boolean fl;
	register boolean flpushpop = !flscriptrunning;
	tyvaluerecord val;
	boolean flpushroot;
	
	setemptystring (bsresult);
	
	if (flpushpop)
		flpushpop = pushprocess (nil);
	
	fl = langrun (htext, &val);
	
	if (flpushpop)
		popprocess ();
	
	if (!fl)
		return (false);
	
	fl = false;
	
	flpushroot = currenthashtable == nil;
	
	if (flpushroot) {
		
		pushhashtable (roottable); /*need temp stack services*/
		
		pushvalueontmpstack (&val);
		}
	
	if (coercetostring (&val)) {
		
		texthandletostring (val.data.stringvalue, bsresult);
		
		fl = true; /*it worked, we'll return true*/
		}
	
	cleartmpstack ();
	
	if (flpushroot)
		pophashtable ();
	
	return (fl);
	} /*langrunhandle*/


static boolean langtraperror (bigstring bsmsg, ptrstring perrorstring) {
	
	/*
	6/25/92 dmb: when an error occurs during a try block, we stash it in 
	the tryerror handle.  it is later placed in the stack frame of the 
	else statement, if it exists, by evaluatelist
	*/
	
	assert (perrorstring != nil);
	
	copystring (bsmsg, perrorstring);
	
	return (true);
	} /*langtraperror*/


boolean langruntraperror (Handle htext, tyvaluerecord *v, bigstring bserror) {
	
	/*
	for langhtml.c: closely an evaluate under a try statement
	*/
	
	boolean fl;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	
	savecallback = langcallbacks.errormessagecallback;
	
	saverefcon = langcallbacks.errormessagerefcon;
	
	langcallbacks.errormessagecallback = (langerrormessagecallback) &langtraperror;
	
	langcallbacks.errormessagerefcon = bserror;
	
	fl = langrun (htext, v);
	
	langcallbacks.errormessagecallback = savecallback;
	
	langcallbacks.errormessagerefcon = saverefcon;
	
	fllangerror = false;
	
	return (fl);
	} /*langrunhandletraperror*/


boolean langrunhandletraperror (Handle htext, bigstring bsresult, bigstring bserror) {
	
	/*
	for iowascript.c: more closely emulate the effect of doing 
	an OSA DoScript.
	*/
#ifdef MACVERSION	
	boolean fl;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	GrafPtr saveport;
	//Code change by Timothy Paustian Wednesday, June 14, 2000 4:32:31 PM
	//Changed to Opaque call for Carbon
	#if TARGET_API_MAC_CARBON == 1
	saveport = GetQDGlobalsThePort();
	#else
	saveport = quickdrawglobal (thePort);
	#endif
	
	savecallback = langcallbacks.errormessagecallback;
	
	saverefcon = langcallbacks.errormessagerefcon;
	
	langcallbacks.errormessagecallback = (langerrormessagecallback) &langtraperror;
	
	langcallbacks.errormessagerefcon = bserror;
	
	flevaluateglobalwith = true;
	
	fl = langrunhandle (htext, bsresult);
	
	langcallbacks.errormessagecallback = savecallback;
	
	langcallbacks.errormessagerefcon = saverefcon;
	
	fllangerror = false;
	
	//Code change by Timothy Paustian Wednesday, June 14, 2000 4:35:24 PM
	//Changed to Opaque call for Carbon
	{
	GrafPtr thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetQDGlobalsThePort();
	#else
	thePort = quickdrawglobal (thePort);
	#endif
	
	if (thePort != saveport)
		SetPort (saveport);
	}
	return (fl);
#endif

#ifdef WIN95VERSION
	return (false);
#endif
	} /*langrunhandletraperror*/


boolean langrunstring (const bigstring bsprogram, bigstring bsresult) {
	
	/*
	compile and run the program source in bsprogram.  if the syntax checks, and
	the program runs without error, return the result as a string.
	*/
	
	Handle htext;
	register boolean fl;
	
	if (!newtexthandle (bsprogram, &htext))
		return (false);
		
	fl = langrunhandle (htext, bsresult);
	
	return (fl);
	} /*langrunstring*/


boolean langrunstringnoerror (const bigstring bsprogram, bigstring bsresult) {
	
	/*
	use this entrypoint when evaluating something that appears inside a document,
	e.g. a wp variable or a pict expression.  an error message would be very hard
	to relate to something in the document, we think.
	
	3/5/97 dmb: instead of disabling langerror, try errors with a noop. this 
	allows try/else blocks to execute normally
	*/
	
	langerrormessagecallback saveerrormessage;
	langvoidcallback saveerrorclear;
	boolean fl;
	
	// 3/5/97 dmb: disablelangerror ();
	
		saveerrormessage = langcallbacks.errormessagecallback;
		
		saveerrorclear = langcallbacks.clearerrorcallback;
		
		langcallbacks.errormessagecallback = (langerrormessagecallback) &truenoop;
		
		langcallbacks.clearerrorcallback = &truenoop;
		
		++fldisableyield;
		
	fl = langrunstring (bsprogram, bsresult);
		
	// 3/5/97 dmb: enablelangerror ();
	
		--fldisableyield;
		
		langcallbacks.errormessagecallback = saveerrormessage;
		
		langcallbacks.clearerrorcallback = saveerrorclear;
		
		fllangerror = false;
	
	return (fl);
	} /*langrunstringnoerror*/


boolean langreduceformula (bigstring bs) {
	
	/*
	returns true if bs changed -- it was a formula and was correctly reduced.
	
	6/1/93 dmb: agentsdisable is defunct.
	*/
	
	bigstring bscopy;
	register boolean fl;
	bigstring bsresult;
	
	if (isemptystring (bs)) /*not a formula*/
		return (false);
	
	if (bs [1] != '=') /*not a formula*/
		return (false);
	
	copystring (bs, bscopy); /*we work with a copy*/
	
	deletestring (bscopy, 1, 1); /*pop off the = sign*/
	
	if (isemptystring (bscopy)) /*nothing but an = sign, not a formula*/
		return (false);
	
	//agentsdisable (true); /*temporarily disable agents*/
	
	fl = langrunstringnoerror (bscopy, bsresult);
	
	//agentsdisable (false); /*restore*/
	
	if (!fl) /*syntax or runtime error*/
		return (false);
	
	copystring (bsresult, bs); /*result replaces the input string*/
	
	return (true);
	} /*langreduceformula*/


boolean langruncode (hdltreenode htree, hdlhashtable hcontext, tyvaluerecord *vreturned) {
	
	/*
	1/8/91 dmb: since the current hash table is the base of the 
	locals chain during execution, it must be protected from disposal 
	with the flchained flag.
	
	5/21/91 dmb: now return result value to caller
	
	5/20/93 dmb: preserve flscriptrunning; nesting can occur when sending an 
	event to self that is handled by a trap script.
	
	2.1b4 dmb: if hcontext is already chained, just push it instead of 
	overriding its current chain. (our own scope chain becomes superflous.)
	*/
	
	register boolean flscriptwasrunning = flscriptrunning;
	register hdlhashtable ht = hcontext;
	register boolean fl;
	register boolean flchained = false;
	
	if (htree == nil) /*defensive driving*/
		return (false);
	
	langprescript (); /*initialize lang globals, about to run a script*/
	
	langpushscopechain ();
	
	if (ht != nil) {
		
		flchained = (**ht).flchained;
		
		if (flchained)
			pushhashtable (ht);
		else
			chainhashtable (ht); /*establishes outer local context*/
		}
	
	langhookerrors ();
	
	flscriptrunning = true; /*this routine is not nestable*/
	
	fl = evaluatelist ((**htree).param1, vreturned);
	
	flscriptrunning = flscriptwasrunning;
	
	exemptfromtmpstack (vreturned);
	
	langunhookerrors ();
	
	if (ht != nil) {
		
		if (flchained)
			pophashtable ();
		else
			unchainhashtable ();
		}
	
	langpopscopechain ();
	
	langpostscript ();
	
	return (fl);
	} /*langruncode*/


boolean langopruncallbackscripts (short idscript) {

	/*
	7.0d5 AR: Generalized from langopstruct2click and langopreturnkey.

	7.0b10 PBS: set and restore outlinedata, in case callback destroys it.
	*/

	boolean fl, flresult = false;
	bigstring bsscript, bsresult;

	if (getsystemtablescript (idscript, bsscript)) {

		grabthreadglobals ();

		oppushoutline (outlinedata); /*7.0b10 PBS: make sure the current outline gets saved.*/

		fl = langrunstringnoerror (bsscript, bsresult);

		oppopoutline (); /*7.0b10 PBS: restore the current outline*/

		releasethreadglobals ();

		if (fl)
			stringisboolean (bsresult, &flresult);
		}

	return (flresult);
	} /*langopruncallbackscripts */


boolean langzoomobject (const bigstring bsobject) {
	
	/*
	locate the indicated database object, and open an editing window to 
	display it.
	
	9/11/91 dmb: call control2click () when control key is down
	
	7/8/92 dmb: call option2click () when the option key is down
	*/
	
	bigstring bsscript, bsresult;
	short idscript;
	boolean fl;
	
	if (!keyboardstatus.flcmdkey && !keyboardstatus.flcontrolkey && !keyboardstatus.floptionkey)
		return (false);
	
	if (keyboardstatus.flcontrolkey)
		idscript = idcontrol2clickscript;
	
	else if (keyboardstatus.flcmdkey)
		idscript = idcommand2clickscript;
	
	else
		idscript = idoption2clickscript;
	
	fl = getsystemtablescript (idscript, bsscript);
	
	if (fl) {
		
		parsedialogstring (bsscript, (ptrstring) bsobject, nil, nil, nil, bsscript);
		
		grabthreadglobals ();

		fl = langrunstringnoerror (bsscript, bsresult);

		releasethreadglobals ();
		}
	
	if (!fl) /*couldn't find it*/
		sysbeep ();
	
	return (fl);
	} /*langzoomobject*/


static boolean langbuildnamedparamlist (tyvaluerecord * listval, hdltreenode *hparams) {
	
	/*
	take all of the parameters in the incoming verb hverb and build a code 
	tree for the corresponding lang paramter list
	
	7.0b41 PBS: new routine, respects the named items in the record, matches to named
	params in the script.
	*/
	
	register short i;
	long ctparams;
	hdltreenode hlist = nil;
	tyvaluerecord val, vkey;
	hdltreenode hparam, hname;
	boolean flpushedroot;
	register boolean fl = false;
	
	if (currenthashtable == nil)
		flpushedroot = pushhashtable (roottable);
	else
		flpushedroot = false;
	
	if (!langgetlistsize (listval, &ctparams))
		goto exit;
	
	for (i = 1; i <= ctparams; i++) {

		bigstring bskey;

		if (!langgetlistitem (listval, i, bskey, &val))
			goto exit;

		exemptfromtmpstack (&val); /*its data is about to belong to code tree*/
		
		if (!newconstnode (val, &hparam))
			goto exit;

		if (!setstringvalue (bskey, &vkey) || !newidnode (vkey, &hname)) {

			langdisposetree (hparam);

			goto exit;
			}

		exemptfromtmpstack (&vkey);

		if (!pushbinaryoperation (fieldop, hname, hparam, &hparam))
			goto exit;
		
		if (hlist == nil)
			hlist = hparam;
		else
			pushlastlink (hparam, hlist);
		} /*for*/
	
	fl = true;
	
	exit:
	
	if (flpushedroot)
		pophashtable ();
	
	if (fl)
		*hparams = hlist; /*nil if there weren't any params*/
	else
		langdisposetree (hlist); /*checks for nil*/
	
	return (fl);
	} /*langbuildnamedparamlist*/


boolean langbuildparamlist (tyvaluerecord * listval, hdltreenode *hparams) {
	
	/*
	take all of the parameters in the incoming verb hverb and build a code 
	tree for the corresponding lang paramter list
	
	2.1b5 dmb: special case for subroutine events
	
	2.1b12 dmb: push the root table to make sure address values will work
	
	3.0.1b2 dmb: for subroutine events, the direct parameter is optional
	
	5.0d14 dmb: take hcode parameter, so we can see of trap script takes 
	parameters by name. the first (direct) parameter can have any name. if
	all others are 4 characters long, and appear in the event, we use names.
	*/
	
	register short i;
	long ctparams;
	hdltreenode hlist = nil;
	tyvaluerecord val;
	hdltreenode hparam;
	boolean flpushedroot;
	register boolean fl = false;
	
	if ((*listval).valuetype == recordvaluetype) /*7.0b41 PBS: build named param list if a record*/
		return (langbuildnamedparamlist (listval, hparams));
	
	if (currenthashtable == nil)
		flpushedroot = pushhashtable (roottable);
	else
		flpushedroot = false;
	
	if (!langgetlistsize (listval, &ctparams))
		goto exit;
	
	for (i = 1; i <= ctparams; i++) {

		if (!langgetlistitem (listval, i, NULL, &val))
			goto exit;

		exemptfromtmpstack (&val); /*its data is about to belong to code tree*/
		
		if (!newconstnode (val, &hparam))
			goto exit;
		
		if (hlist == nil)
			hlist = hparam;
		else
			pushlastlink (hparam, hlist);
		} /*for*/
	
	fl = true;
	
	exit:
	
	if (flpushedroot)
		pophashtable ();
	
	if (fl)
		*hparams = hlist; /*nil if there weren't any params*/
	else
		langdisposetree (hlist); /*checks for nil*/
	
	return (fl);
	} /*langbuildparamlist*/


boolean langrunscriptcode (hdlhashtable htable, bigstring bsverb, hdltreenode hcode, tyvaluerecord *vparams, hdlhashtable hcontext, tyvaluerecord *vreturned) {
	
	/*
	02/04/02 dmb: the guts of langrunscript
	*/
	
	boolean fl = false;
	boolean flchained;
	tyvaluerecord val;
	hdltreenode hfunctioncall;
	hdltreenode hparamlist;
	boolean fltmpval;
	
	if (!setaddressvalue (htable, bsverb, &val))
		goto exit;
	
	if (!pushfunctionreference (val, &hfunctioncall))
		goto exit;
	
/*	if (hcontext != nil)
		pushhashtable (hcontext);
*/
	if (hcontext != nil) {
		
		flchained = (**hcontext).flchained;
		
		if (flchained)
			pushhashtable (hcontext);
		else
			chainhashtable (hcontext); /*establishes outer local context*/
		}
	
	fl = langbuildparamlist (vparams, &hparamlist);
	
	if (hcontext != nil) {
		
		if (flchained)
			pophashtable ();
		else
			unchainhashtable ();
		}
	
	if (!fl) {
		
		langdisposetree (hfunctioncall);
		
		goto exit;
		}
	
	if (!pushfunctioncall (hfunctioncall, hparamlist, &hcode)) /*consumes input parameters*/
		goto exit;
		
	if (hcontext != nil) {

		if (flchained)
			pushhashtable (hcontext);
		else
			chainhashtable (hcontext); /*establishes outer local context*/
		}
	
	fl = evaluatelist (hcode, vreturned);
	
	fltmpval = exemptfromtmpstack (vreturned); /*must survive disposing of local scope chain*/
	
	if (hcontext != nil) {

		if (flchained)
			pophashtable ();
		else
			unchainhashtable ();
		}
	
	if (fltmpval) /*insert into the next-most-global tmpstack, if one exists*/
		pushvalueontmpstack (vreturned);

	langdisposetree (hcode);

	exit:
	
	return (fl);
	} /*langrunscriptcode*/


boolean langrunscript (bigstring bsscriptname, tyvaluerecord *vparams, hdlhashtable hcontext, tyvaluerecord *vreturned) {
	
	/*
	5.0.2b6 rab/dmb: new verb
	
	5.0.2b7 dmb: preserve errormessagecallback through the call
	
	6.0a9 dmb: was langipcrunscript, moved here and don't create new process

	6.0a14 dmb: fixed potential memory leak in error case.

	7.0b41 pbs: if passed a record instead of a list, call the script with named parameters.

	8.0.4 dmb: handle running code values
	*/
	
	bigstring bsverb;
	boolean fl = false;
	hdltreenode hcode;
	hdlhashtable htable;
	tyvaluerecord vhandler;
	hdlhashnode handlernode;
	
	pushhashtable (roottable);
	
	fl = langexpandtodotparams (bsscriptname, &htable, bsverb);

	if (fl) {
	
		if (htable == nil)
			langsearchpathlookup (bsverb, &htable);
		}
	
	pophashtable();
	
	if (!fl)
		return (false);
	
	if (!hashtablelookupnode (htable, bsverb, &handlernode)) {
		
		langparamerror (unknownfunctionerror, bsverb);
		
		return (false);
		}
	
	vhandler = (**handlernode).val;
	
	/*build a code tree and call the handler, with our error hook in place*/
	
	hcode = nil;
	
	if (vhandler.valuetype == codevaluetype) {

		hcode = vhandler.data.codevalue;
	}
	else if ((**htable).valueroutine == nil) { /*not a kernel table*/
		
		if (!langexternalvaltocode (vhandler, &hcode)) {

			langparamerror (notfunctionerror, bsverb);

			return (false);
			}
		
		if (hcode == nil) { /*needs compilation*/
			
			if (!langcompilescript (handlernode, &hcode))
				return (false);
			}
		}
	
	return (langrunscriptcode (htable, bsverb, hcode, vparams, hcontext, vreturned));
	} /*langrunscript*/

/*
	boolean flchained;
	tyvaluerecord val;
	hdltreenode hfunctioncall;
	hdltreenode hparamlist;
	boolean fltmpval;

	if (!setaddressvalue (htable, bsverb, &val))
		goto exit;
	
	if (!pushfunctionreference (val, &hfunctioncall))
		goto exit;
	
	if (hcontext != nil) {
		
		flchained = (**hcontext).flchained;
		
		if (flchained)
			pushhashtable (hcontext);
		else
			chainhashtable (hcontext); //establishes outer local context
		}
	
	fl = langbuildparamlist (vparams, &hparamlist);
	
	if (hcontext != nil) {
		
		if (flchained)
			pophashtable ();
		else
			unchainhashtable ();
		}
	
	if (!fl) {
		
		langdisposetree (hfunctioncall);
		
		goto exit;
		}
	
	if (!pushfunctioncall (hfunctioncall, hparamlist, &hcode)) //consumes input parameters
		goto exit;
		
	if (hcontext != nil) {

		if (flchained)
			pushhashtable (hcontext);
		else
			chainhashtable (hcontext); //establishes outer local context
		}
	
	fl = evaluatelist (hcode, vreturned);
	
	fltmpval = exemptfromtmpstack (vreturned); //must survive disposing of local scope chain
	
	if (hcontext != nil) {

		if (flchained)
			pophashtable ();
		else
			unchainhashtable ();
		}
	
	if (fltmpval) //insert into the next-most-global tmpstack, if one exists
		pushvalueontmpstack (vreturned);

	langdisposetree (hcode);

	exit:
	
	return (fl);
	} /%langrunscript%/
*/

