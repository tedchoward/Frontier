
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

 

#ifdef MACVERSION
#include <standard.h>
#endif

#ifdef WIN95VERSION
#include "standard.h"
#endif

#include "strings.h"
#include "lang.h"
#include "langinternal.h"
#include "shell.h"
#include "memory.h"


/*
jackets around routines that are implemented as callbacks.
*/


boolean flerrorscriptrunning = false;


boolean langpushlocalchain (hdlhashtable *htable) {
	
	/*
	9/23/91 dmb: to clean up langfunccall, we've distributed magictable 
	handling around so that a locals table can be created and populated 
	without actually pushing it into the local chain.  our task here is 
	simply to make sure that it's consumed, whether or not an error occurs.
	*/
	
	register boolean fl;
	
	fl = (*langcallbacks.pushtablecallback) (htable);
	
	/*stacktracer (hashgetstackdepth ());*/ /*debugging code*/
	
	if (hmagictable != nil) { /*make sure we've consumed any magic, even on error*/
		
		disposehashtable (hmagictable, false);
		
		hmagictable = nil;
		}
	
	return (fl);
	} /*langpushlocalchain*/
	
	
boolean langpoplocalchain (hdlhashtable hcheck) {
	
	register hdlhashtable ht = currenthashtable;
	register boolean fl;
	
	assert (hcheck == ht);
	
	fl = (*langcallbacks.poptablecallback) (ht);
	
	/*stacktracer (hashgetstackdepth ());*/ /*debugging code*/
	
	return (fl);
	} /*langpoplocalchain*/
	

boolean langbackgroundtask (boolean flresting) {
	
	/*
	a jacket around the running of background tasks -- we must save and restore
	some globals on the C stack, otherwise things break!
	
	1/18/93 dmb: added flresting parameter
	*/
	
	boolean fl;
	
	#ifdef fldebug
	
	boolean flsavedbreak;
	
	flsavedbreak = flbreak;
	
	#endif
	
	fl = (*langcallbacks.backgroundtaskcallback) (flresting);
	
	assert (flbreak == flsavedbreak); /*restore globals*/
	
	return (fl);
	} /*langbackgroundtask*/


boolean languserescaped (boolean flchecknow) {
	
	/*
	9/27/92 dmb: added flichecknow parameter
	*/
	
	return ((*langcallbacks.scriptkilledcallback) (flchecknow));
	} /*languserescaped*/


boolean langdebuggercall (hdltreenode hnode) {
	
	/*
	if we return false, it's a signal to the caller to exit without an error
	dialog.  the user has decided to stop running the current script.
	
	there didn't seem to be a better place to explain why we don't break at
	compound lines -- like the ones which contain loop, if, fileloop, etc.
	
	this is why: the compiler doesn't generate an accurate line number for
	these.  the line number is the first one after the closing right curly
	bracket.  if the debugger stopped on an if, it would put the cursor on
	the first statement after the if!  this is fine for error reporting,
	but is not too useful for debugging.  so we have a rule, the cursor
	only stops on "meaty" lines, ones which actually do some command or
	arithmetic.  not controlling the flow of the program.  of course it's
	easy to discern the flow from where the cursor ends up.
	
	1/18/91 dmb: parser modifications now generate accurate line numbers 
	for everything except modules.
	
	2/13/91 dmb: don't debug noops, localops, bundleops
	
	7/29/91 dmb: save and restore flreturn & flbreak, just like backgroundtask
	
	5/20/92 dmb: leave it up to the callback to decide which lines are "meaty".  
	pass the node itself to the callback instead of the line & character.
	*/
	
	register hdltreenode h = hnode;
	boolean fl;
	boolean flsavedcontinue;
	
	flsavedcontinue = flcontinue;
	
	fl = (*langcallbacks.debuggercallback) (h);
	
	assert (flcontinue == flsavedcontinue); /****should be able to nuke this stuff*/
	
	return (fl);
	} /*langdebuggercall*/


boolean langsaveglobals (void) {
	
	return ((*langcallbacks.saveglobalscallback) ());
	} /*langsaveglobals*/


boolean langrestoreglobals (void) {
	
	return ((*langcallbacks.restoreglobalscallback) ());
	} /*langrestoreglobals*/


boolean langpushsourcecode (hdlhashtable htable, hdlhashnode hnode, bigstring bs) {

	return ((*langcallbacks.pushsourcecodecallback) (htable, hnode, bs));
	} /*langpushsourcecode*/


boolean langpopsourcecode (void) {

	return ((*langcallbacks.popsourcecodecallback) ());
	} /*langpopsourcecode*/


#if 0

boolean langgetsourcecodeaddress (hdlhashtable *htable, bigstring bsname) {
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	tyerrorrecord errorrecord = (**hs).stack [(**hs).toperror - 1];
	
	return ((*errorrecord.errorcallback) (errorrecord.errorrefcon, 0, 0, htable, bsname));
	} /*langgetsourcecodeaddress*/

#endif

boolean langerrormessage (bigstring bs) {
	
	/*
	10/31/91 dmb: experimented with calling local "error" routine on all 
	errors.  it's quickly becoming apparant that this is not a generally safe 
	thing to do, and that certain classes of errors must not trigger this 
	call (compiler errors, for example).  If we want to support this in the 
	future, a more formal mechanism should be developed.
	
	4.1b3 dmb: added call to new langseterrorcallbackline for stack tracing (on error)
	*/
	
	if (!langerrorenabled ())
		return (true);
	
	if (fllangerror) /*one message per script*/
		return (true);
	
	fllangerror = true; /*only display once for each script*/
	
	#ifdef flnewfeatures	// flstacktrace
	
	langseterrorcallbackline ();
	
	#endif
	
	if (!(*langcallbacks.debugerrormessagecallback) (bs, langcallbacks.errormessagerefcon))
		return (false);
	
	return ((*langcallbacks.errormessagecallback) (bs, langcallbacks.errormessagerefcon));
	} /*langerrormessage*/


boolean langerrorclear (void) {
	
	fllangerror = false;
	
	return ((*langcallbacks.clearerrorcallback) ());
	} /*langerrorclear*/


boolean langcompilescript (hdlhashnode hnode, hdltreenode *hcode) {
	
	return ((*langcallbacks.scriptcompilecallback) (hnode, hcode));
	} /*langcompilescript*/


void langsymbolchanged (hdlhashtable htable, const bigstring bs, hdlhashnode hnode, boolean flvalue) {
	
	/*
	5.1.5b15 dmb: call callback first, so it can tell if table was already dirty
	*/

#ifdef fltracklocaladdresses

	if (flvalue) {

		hdlhashnode hn = hnode;
		
		if ((hn != nil && hn != HNoNode) || hashtablelookupnode (htable, bs, &hn)) {

			hashunregisteraddressnode (hn);

			hashregisteraddressnode (htable, hn);
			}
		}

#endif

	(*langcallbacks.symbolchangedcallback) (htable, bs, hnode, flvalue);
	
	dirtyhashtable (htable);
	} /*langsymbolchanged*/

/*
void langsymbolprechange (hdlhashtable htable, const bigstring bs, hdlhashnode hnode, boolean flvalue) {
	
	(*langcallbacks.symbolprechangecallback) (htable, bs, hnode, flvalue);

	}*/ /*langsymbolprechange*/


void langsymbolinserted (hdlhashtable htable, const bigstring bsname, hdlhashnode hnode) {

#ifdef fltracklocaladdresses	
	hashregisteraddressnode (htable, hnode);
#endif

	(*langcallbacks.symbolinsertedcallback) (htable, bsname, hnode);

	} /*langsymbolinserted*/


void langsymbolunlinking (hdlhashtable htable, hdlhashnode hnode) {
	
	(*langcallbacks.symbolunlinkingcallback) (htable, hnode);

	} /*langsymbolunlinking*/


void langsymboldeleted (hdlhashtable htable, const bigstring bsname) {

	(*langcallbacks.symboldeletedcallback) (htable, bsname);

	} /*langsymboldeleted*/


boolean langpartialeventloop (short desiredevents) {
	
	return ((*langcallbacks.partialeventloopcallback) (desiredevents));
	} /*langpartialeventloop*/




