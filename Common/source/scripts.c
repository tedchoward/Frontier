
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

#include "memory.h"
#include "cursor.h"
#include "dialogs.h"
#include "file.h"
#include "font.h"
#include "kb.h"
#include "mouse.h"
#include "ops.h"
#include "popup.h"
#include "quickdraw.h"
#include "scrap.h"
#include "search.h"
#include "strings.h"
#include "frontierwindows.h"
#include "db.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "shellbuttons.h"
#include "shellhooks.h"
#include "shellprivate.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "op.h"
#include "opinternal.h"
#include "opverbs.h"
#include "menueditor.h"
#include "meprograms.h"
#include "process.h"
#include "processinternal.h"
#include "tablestructure.h"
#include "tableinternal.h"
#include "tableverbs.h"
#include "scripts.h"
#ifdef flcomponent
#include "osacomponent.h"
#endif
#include "error.h"

static boolean scriptdebuggereventloop (void);

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h"
#endif


enum {
	runbutton = 1,
	
	debugbutton,
	
	recordbutton,

	stepbutton,
	
	inbutton,
	
	outbutton,
	
	followbutton,
	
	gobutton,
	
	stopbutton,
	
	killbutton,
	
	localsbutton,
	
	installbutton
	};



//#define maxchainedlocals 80 /*limit for local table nesting in debugged process*/

#define maxchainedlocals 200 /*6.1d19 AR*/

//#define maxnestedsources 40 /*local handlers count as nested sources*/

#define maxnestedsources 200 /*6.1d19 AR*/


typedef struct tysourcerecord {
	
	WindowPtr pwindow; /*window containing running script; nil if closed or unknown*/
	
	hdlhashnode hnode; /*hash node containing script variable*/
	
	hdloutlinerecord houtline; /*script outline itself*/
	} tysourcerecord;


typedef struct tydebuggerrecord {
	
	hdlprocessrecord scriptprocess;
	
	tysourcerecord scriptsourcestack [maxnestedsources]; /*for stepping other scripts*/
	
	short topscriptsource; /*source stack pointer*/
	
	short sourcesteplevel; /*level in source stack at which we suspend after step/in/out*/
	
	tydirection stepdir; /*left,right,down & flatdown for out,in,step & trace*/
	
	long lastlnum; /*the last line number stepped onto*/
	
	hdlhashtable hlocaltable; /*the most local table in our process*/
	
	hdlhashnode localtablestack [maxchainedlocals]; /*nodes of tables in the runtimestack*/
	
	//Handle localtableformatsstack [maxchainedlocals]; /*nodes of tables in the runtimestack*/
	
	short toplocaltable; /*table stack pointer*/
	
	//short deferredbuttonnum; /*see scriptdeferbutton*/
	
	//WindowPtr deferredwindow; /*ditto*/
	
	hdlheadrecord hbarcursor; /*the headline we've most recently shown*/
	
#ifdef flcomponent
	
	ComponentInstance servercomp; /*component that we have open for this script*/
	
	OSType servertype; /*the component's type*/
	
	OSAID idscript; /*while recording, this is the id of the script*/
	
#endif
	
	short lastindent;
	
	boolean flscriptsuspended: 1; /*user is debugging a script*/
	
	boolean flstepping: 1; /*single-stepping thru scripts*/
	
	boolean flfollowing: 1; /*does the bar cursor follow the interpreter?*/
	
	boolean flscriptkilled: 1; /*true if the script has been killed*/
	
	boolean flscriptrunning: 1; /*true when the script is running*/
	
	boolean flwindowclosed: 1; /*false if script's window is open*/
	
	boolean fllangerror: 1; /*are we stopped with an error?*/
	
	boolean flrecording: 1; /*are we recording into this script?*/
	
	boolean flcontextlocked: 1; /*is out context being operated on externally?*/
	
	long scriptrefcon; /*copied from outline refcon*/
	} tydebuggerrecord, *ptrdebuggerrecord, **hdldebuggerrecord;


static hdldebuggerrecord debuggerdata = nil;

//static boolean flenablemenus = false;

//static short scriptbackgrounddisable = 0; /*it's possible to temporarily disable backgrounding*/

//static long scriptbackgroundtime = 0;

static hdloutlinerecord scriptdataholder = nil;


static boolean scriptsetglobals (void) {
	
	/*
	5.1.5b7 dmb: added this layer so we can trap the setting of outlinedata
	*/
	
	opsetoutline (scriptdataholder);
	
	return (opeditsetglobals ());
	} /*scriptsetglobals*/


/*
disablescriptbackground (void) {
	
	++scriptbackgrounddisable;
	} /%disablescriptbackground%/


enablescriptbackground (void) {
	
	--scriptbackgrounddisable;
	} /%enablescriptbackground%/
*/


static boolean scriptbackgroundenabled (void) {
	
	/*
	return (scriptbackgrounddisable == 0);
	*/
	
	return (shellwindow == nil); /*don't background while window globals are pushed*/
	} /*scriptbackgroundenabled*/


/*static void scriptresetbackgroundtime (void) {
	
	scriptbackgroundtime = gettickcount ();
	} /%scriptresetbackgroundtime%/
*/

static boolean isheadrecordhandle (long refcon) {
	
	/*
	1/14/91 dmb: there are a few places in this file where we need to 
	distinguish between a hdlheadrecord and a hdlhashnode stored as a 
	refcon.  ugly as it may be, testing the handle size seems fairly 
	foolproof.  if we wanted to, we could look into the record itself 
	for other evidence, but it doesn't seem neccessary
	
	6/4/92 dmb: now that we place the headstring and hashkey strings at 
	the end of their respective records, the old check no longer works.
	now we make sure that the size of the record is inconsistent with assuming 
	that it is a hashnode, which is reasonable safe so long as the hashkey field 
	lines up with the beginning of the headlevel field, which we check for.  to 
	add extra assurance, we do two checks; first for a hashnode (which is shorter), 
	second for a headrecord.  note that hdlexternalvariables are also passed to 
	this routine
	*/
	
	register Handle h = (Handle) refcon;
	register long size = gethandlesize (h);
	
	if (odd (refcon) || refcon == 0)
		return (false);
	
//	assert ((long) &(**(hdlheadrecord) h).headlevel == (long) &(**(hdlhashnode) h).hashkey); /*extra assurance*/
	
	if ((unsigned) size == sizeof (tyhashnode) + stringsize ((**(hdlhashnode) h).hashkey))
		return (false);
	
	if ((unsigned) size == sizeof (tyheadrecord))
		return (true);
	
	return (false);
	} /*isheadrecordhandle*/


boolean scriptbuildtree (Handle htext, long signature, hdltreenode *hcode) {
	
	/*
	2.1b11 dmb: don't compile osa scripts execute-only, because some 
	scripting systems (e.g. AppleScript) needs to be able to get the 
	source in order to report runtime error offsets
	*/
	
	hdltreenode hstub;
	tyvaluerecord codeval;
	boolean fl;
	
	if (signature == typeLAND)
		return (langbuildtree (htext, true, hcode));
	
#ifdef flcomponent
	fl = osagetcode (htext, signature, false, &codeval);
#else
	fl = false;
#endif
	disposehandle (htext);
	
	if (!fl)
		return (false);
	
	exemptfromtmpstack (&codeval);
	
	if (!newconstnode (codeval, &hstub))
		return (false);
	
	(**hstub).nodetype = osascriptop;
	
	/*
	*hcode = hstub;
	
	return (true);
	*/
	
	return (pushbinaryoperation (moduleop, hstub, nil, hcode));
	} /*scriptbuildtree*/


static boolean scriptgetcode (hdlhashnode hnode, hdltreenode *hcode) {
	
	/*
	give me a node, I'll get you the code.
	
	5/19/92 dmb: stashes a link to the source node in the nodeval at the top of 
	the code tree.  see comment in opverbdisposecode.
	*/
	
	tyvaluerecord val;
	register hdlexternalvariable hv;
	Handle htext;
	long signature;
	register boolean fl;
	
	*hcode = nil; /*default return*/
	
	val = (**hnode).val;
	
	if (val.valuetype != externalvaluetype) /*not a script*/
		return (false);
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if ((**hv).id != idscriptprocessor) /*not a script*/
		return (false);
	
	if (!opverbgetlangtext (hv, false, &htext, &signature))
		return (false);
	
	fl = scriptbuildtree (htext, signature, hcode);
	
	
	/*7/9/90 DW: langbuildtree disposes of htext, not a memory leak*/
	
	if (fl) {
		
		assert ((***hcode).nodeval.data.longvalue == 0);
		
		(***hcode).nodeval.data.longvalue = (long) hnode; /*5/19/92 dmb: point back at node*/
		}
	
	return (fl);
	} /*scriptgetcode*/


static boolean scriptzoomscript (hdlhashnode hnode, WindowPtr *w) {
	
	/*
	hnode is a hash table node containing a script variable.  zoom its 
	window and return the windowptr in w
	
	7/4/91 dmb: langexternalzoom now take table parameter.  we don't know 
	what table the node is in, so we pass nil.
	*/
	
	register hdlhashnode h = hnode;
	bigstring bs;
	hdlwindowinfo hinfo;
	
	gethashkey (h, bs);
	
	if (!langexternalzoom ((**h).val, nil, bs))	
		return (false);
	
	if (!langexternalwindowopen ((**h).val, &hinfo))
		return (false);
	
	*w = (**hinfo).macwindow;
	
	return (true);
	} /*scriptzoomscript*/


static boolean systemscripterrorroutine (long scripterrorrefcon, long lnum, short charnum, hdlhashtable *htable, bigstring bsname) {
	
	/*
	display an error for a system script -- a handler, agent or startup script.
	
	called back from langerror.c.  we receive a handle to the table node that
	made the error, and we zoom out a window that displays the attached script,
	in text mode, with the character cursor pointing to line number lnum at
	offset charnum -- the exact spot where the error occured, we hope...
	
	2/4/91 dmb: allow hdlheadrecords to end up here too
	
	4.0b8 dmb: if caller provides non-null htable, don't show error, just return 
	its location. bsname better be non-null too!

	4/7/97 dmb: handle standalone scripts

	5.0.2b21 dmb: for Win, nil check must cover dereferenced handle too
	*/
	
	register hdlhashnode h = (hdlhashnode) scripterrorrefcon;
	hdlwindowinfo hinfo;
	hdlexternalvariable hv;
	WindowPtr w;
	register boolean fl;
	
	if (h == nil || *h == nil) /*defensive driving*/
		return (false);
	
	if (isheadrecordhandle ((long) h))
		return (mescripterrorroutine ((long) h, lnum, charnum, htable, bsname));
	
	if (shellfinddatawindow ((Handle) h, &hinfo)) { // 4/7/97 dmb: standalone

		if (htable != nil) { /*caller wants table, name*/
			
			if (shellgetexternaldata (hinfo, &hv))
				return (langexternalfindvariable (hv, htable, bsname));
			
			return (false);
			}
		
		shellbringtofront (hinfo);
		
		w = (**hinfo).macwindow;

		goto exit;
		}
	
	if (htable != nil) { /*caller wants table, name*/
		
		hv = (hdlexternalvariable) (**h).val.data.externalvalue;
		
		return (langexternalfindvariable (hv, htable, bsname));
		}
	
	if (!scriptzoomscript (h, &w))
		return (false);
	
exit:

	shellpushglobals (w);
	
	fl = opshowerror (lnum, charnum);
	
	shellpopglobals ();
	
	return (fl);
	} /*systemscripterrorroutine*/


static boolean scriptcompiler (hdlhashnode hnode, hdltreenode *hcode) {
	
	/*
	7/1/91 dmb: don't return true on failure to continue visit; we now have 
	a wrapper for visiting.  also, langternalvaltocode returns false only 
	when val isn't a script, and may return true with holdcode nil if there's 
	no existing code.
	
	7/3/91 dmb: let processreplacecode manage old code, in case it's running now.
	
	5/14/93 dmb: added hcode parameter; caller often wants to grab it.
	*/
	
	hdltreenode hnewcode;
	hdltreenode holdcode;
	register hdlexternalvariable hv;
	register boolean fl;
	
	if (!langexternalvaltocode ((**hnode).val, &holdcode))
		return (false);
	
	langpusherrorcallback (&systemscripterrorroutine, (long) hnode);
	
	fl = scriptgetcode (hnode, &hnewcode);
	
	langpoperrorcallback ();
	
	if (!fl)
		return (false);
	
	hv = (hdlexternalvariable) (**hnode).val.data.externalvalue;
	
	opverblinkcode (hv, (Handle) hnewcode); /*link code into the outline variable record*/
	
	/*
	if (hashnodeintable (hnode, agentstable)) { /%special stuff for agents%/
		
		if (!processreplacecode (holdcode, hnewcode)) { /%no replacement, add new process%/
			
			langdisposetree (holdcode);
			
			addnewprocess (hnewcode, false, &systemscripterrorroutine, (long) hnode);
			}
		}
	else
		langdisposetree (holdcode);
	*/
	
	if (processreplacecode (holdcode, hnewcode))
		return (true);
	
	processcodedisposed ((long) hnode);
	
	langdisposetree (holdcode);
	
	if (hashnodeintable (hnode, agentstable)) /*special stuff for agents*/
		addnewprocess (hnewcode, false, &systemscripterrorroutine, (long) hnode);
	
	*hcode = hnewcode;
	
	return (true);
	} /*scriptcompiler*/


static boolean installscriptvisit (hdlhashnode hnode, ptrvoid refcon) {
#pragma unused (refcon)

	hdltreenode hcode;
	
	scriptcompiler (hnode, &hcode);
	
	return (true); /*continue visit unconditionally*/
	} /*installscriptvisit*/


static boolean scriptinstallscripts (hdlhashtable htable) {
	
	if (htable == nil) /*defensive driving*/
		return (false);
	
	return (hashtablevisit (htable, &installscriptvisit, nil));
	} /*scriptinstallscripts*/


boolean scriptinstallagent (hdlhashnode hnode) {
	
	hdltreenode hcode;
	
	return (scriptcompiler (hnode, &hcode));
	} /*scriptinstallagent*/


boolean scriptremoveagent (hdlhashnode hnode) {
	
	hdltreenode hcode;
	
	if (!langexternalvaltocode ((**hnode).val, &hcode))
		return (false);
	
	if (hcode != nil)
		processdisposecode (hcode);
	
	return (true);
	} /*scriptremoveagent*/


static boolean scriptloadagents (void) {
	
	return (scriptinstallscripts (agentstable));
	} /*scriptloadagents*/
	

static long ctspecialprocessesrunning = 0;


static boolean specialscriptkilled (void) {
	
	--ctspecialprocessesrunning;
	
	return (true);
	} /*specialscriptkilled*/


static boolean newprocessvisit (hdlhashnode hnode, ptrvoid refcon) {
#pragma unused (refcon)

	hdltreenode hcode;
	hdlprocessrecord hprocess;
	boolean fl;
	
	langpusherrorcallback (&systemscripterrorroutine, (long) hnode);
	
	fl = scriptgetcode (hnode, &hcode);
	
	langpoperrorcallback ();
	
	if (!fl)
		return (true);
	
	#ifdef version5orgreater
		if (!newprocess (hcode, true, &systemscripterrorroutine, (long) hnode, &hprocess))
			return (false);
		
		(**hprocess).processkilledroutine = &specialscriptkilled;
		
		addprocess (hprocess);
		
		++ctspecialprocessesrunning;
		
		return (true);
	#else
		return (addnewprocess (hcode, true, &systemscripterrorroutine, (long) hnode));
	#endif
	} /*newprocessvisit*/


static boolean scriptrunspecialscripts (bigstring bsspecialtable) {
	
	/*
	run all the scripts in the startup table.
	*/
	
	hdlhashtable htable;
	
	ctspecialprocessesrunning = 0;
	
	if (!findnamedtable (systemtable, bsspecialtable, &htable))
		return (true);
	
	return (hashtablevisit (htable, &newprocessvisit, nil));
	} /*scriptrunspecialscripts*/


boolean scriptrunstartupscripts (void) {
	
	/*
	run all the scripts in the startup table.
	*/
	
	return (scriptrunspecialscripts (namestartuptable));
	} /*scriptrunstartupscripts*/


boolean scriptrunsuspendscripts (void) {
	
	/*
	run all the scripts in the suspend table.
	
	5.0a3 dmb: no; just run the single system.callbacks.suspend script
	*/
	
	// return (scriptrunspecialscripts (namesuspendtable));
	
	bigstring bsscript;
	
	getsystemtablescript (idsuspendscript, bsscript);
	
	return (processrunstringnoerrorclear (bsscript));
	} /*scriptrunsuspendscripts*/


long specialoneshotscriptsrunning (void) {
	
	return (ctspecialprocessesrunning);
	} /*specialoneshotscriptsrunning*/


boolean scriptrunresumescripts (void) {
	
	/*
	run all the scripts in the resume table.
	
	5.0a3 dmb: no; just run the single system.callbacks.suspend script
	*/
	
	// return (scriptrunspecialscripts (nameresumetable));
	
	bigstring bsscript;
	
	getsystemtablescript (idresumescript, bsscript);
	
	return (processrunstringnoerrorclear (bsscript));
	} /*scriptrunresumescripts*/


boolean loadsystemscripts (void) {
	
	/*
	6/19/92 dmb: load agents last, so that a startup script can 
	disable them effectively
	
	2.1b4 dmb: disabled calls to preload stuff in builtins, apps & traps. 
	it this stuff isn't used, why force it into memory?
	*/
	
	scriptrunstartupscripts (); /*run all scripts in startup table*/
	
	scriptloadagents (); /*load all scripts in agents table*/
	
	return (true);
	} /*loadsystemscripts*/


static boolean runprocessvisit (hdlhashnode hnode, ptrvoid refcon) {
#pragma unused (refcon)

	hdltreenode hcode;
	hdlprocessrecord hprocess;
	tyvaluerecord val;
	boolean fl;
	
	langpusherrorcallback (&systemscripterrorroutine, (long) hnode);
	
	fl = scriptgetcode (hnode, &hcode);
	
	langpoperrorcallback ();
	
	if (!fl)
		return (true);
	
	if (!newprocess (hcode, true, nil, 0L, &hprocess)) {
		
		langdisposetree (hcode);
		
		return (false);
		}
	
	fl = processruncode (hprocess, &val);
	
	disposeprocess (hprocess);
	
	if (fl)
		disposetmpvalue (&val);
	
	return (true);
	} /*runprocessvisit*/


boolean runshutdownscripts (void) {
	
	/*
	run all the scripts in the shutdown table.
	*/
	
	hdlhashtable htable;
	
	if (!findnamedtable (systemtable, nameshutdowntable, &htable))
		return (true);
	
	return (hashtablevisit (htable, &runprocessvisit, nil));
	} /*runshutdownscripts*/


static void scriptgetpopuprect (Rect *rpopup) {
	
	register hdlwindowinfo hw = outlinewindowinfo;
	register Rect *r = rpopup;
	
	*r = (**hw).messagerect;
	
	(*r).left = (*r).right - 1;
	
	(*r).right = (*r).left + 73;
	
	/*
	(*r).right = (*r).left + 1;
	
	(*r).left = (**hw).contentrect.left - 1;
	*/
	
	} /*scriptgetpopuprect*/


static boolean scriptinvalbuttonsvisit (WindowPtr w, ptrvoid refcon) {
#pragma unused (refcon)

	/*
	7.0b42 PBS: If Mac OS X, re-draw the script buttons right away, because
	otherwise the window doesn't get updated until it's activated.
	*/

	Rect r;
	
	shellpushglobals (w);
	
	shellinvalbuttons (); 
	
	scriptgetpopuprect (&r);
	
	invalwindowrect (shellwindow, r);
	
	#if TARGET_API_MAC_CARBON == 1
	
		GetPortBounds (GetWindowPort (w), &r);	
	
		pushclip (r);
	
		shelldrawbuttons ();
		
		popclip ();
		
	#endif
		
	shellpopglobals ();
	
	return (true);
	} /*scriptinvalbuttonsvisit*/


static void scriptinvalbuttons (void) {
	
	shellvisittypedwindows (idscriptconfig, &scriptinvalbuttonsvisit, nil);
	} /*scriptinvalbuttons*/


static boolean scriptfindhashnode (hdlexternalvariable hvariable, hdlhashnode *hnode) {
	
	hdlhashtable htable;
	bigstring bsname;
	boolean fl;
	
	if (!langexternalfindvariable (hvariable, &htable, bsname))
		return (false);
	
	pushhashtable (htable);
	
	fl = hashlookupnode (bsname, hnode);
	
	pophashtable ();
	
	return (fl);
	} /*scriptfindhashnode*/


boolean scriptinmenubar (void) {
	
	/*
	12/31/90 dmb: for menubar scripts, we now stick the hdlheadrecord of 
	owning headline in the refcon of the script outline.  sadly, we have to 
	rely on the handle size to tell what kind of handle it is
	
	7/16/91 dmb: removed staticness for "temporary" call from shellwindowverbs.c
	*/
	
//	return (isheadrecordhandle ((**outlinedata).outlinerefcon));
	return ((**outlinedata).outlinetype == outlineismenubarscript);
	} /*scriptinmenubar*/


static boolean scriptindatabase (void) {
	
	/*
	12/31/90 dmb: for menubar scripts, we now stick the hdlheadrecord of 
	owning headline in the refcon of the script outline.  sadly, we have to 
	rely on the handle size to tell what kind of handle it is
	
	7/16/91 dmb: removed staticness for "temporary" call from shellwindowverbs.c
	*/
	
	return ((**outlinedata).outlinetype == outlineisdatabasescript);
	} /*scriptindatabase*/


static boolean scriptinruntimestack (void) {
	
	/*
	is the current script part of the current process?
	
	6/24/92 dmb: compare outlines in loop, not windows; windows may not yet 
	be assigned into stack (if opened manaually or by Error Info)
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register short ix;
	
	for (ix = 0; ix < (**hd).topscriptsource; ix++) { /*visit every window in stack*/
		
		/*
		if (outlinewindow == (**hd).scriptsourcestack [ix].pwindow) {
		*/
		
		if (outlinedata == (**hd).scriptsourcestack [ix].houtline) {
			
			if (ix == 0) /*main source window.  make sure same script is displayed*/
			
				return ((**hd).scriptrefcon == (**outlinedata).outlinerefcon);
			
			return (true);
			}
		}
	
	return (false);
	} /*scriptinruntimestack*/


static boolean scriptinstallable (void) {
	
	/*
	8/26/90 DW:
	
	this is separated out into a separate function so that our method is easily
	visible, and can easily be changed.
	
	we want to know if the script lives in the menubar structure or in the symbol
	table structure.  we observe that the outline record's refcon field will point
	to a variable record if it lives in the symbol table -- and it's nil if it 
	lives in the menubar.
	
	if you allocate a refcon for menubar scripts all of a sudden they will appear
	to be installable.
	
	11/19/90 DW: no longer depend on the outline being dirty -- allow the user
	to install the script, even when it isn't dirty.
	
	12/31/90 dmb: refcon is no longer nil for menubar scripts -- see commment 
	in scriptinmenubar
	*/
	
	return (scriptindatabase ());
	} /*scriptinstallable*/


#ifdef flcomponent

static boolean scriptrecordable (void) {
	
	register ComponentInstance comp;
	
	if (!havecomponentmanager ())
		return (false);
	
	comp = getosaserver ((**outlinedata).outlinesignature);
	
	if (comp == nil)
		return (false);
	
	return (ComponentFunctionImplemented (comp, kOSASelectStartRecording));
	} /*scriptrecordable*/

#endif

static boolean scriptpushsourcerecord (tysourcerecord source) {
	
	register hdldebuggerrecord hd = debuggerdata;
	
	if (!langcheckstacklimit (idsourcestack, (**hd).topscriptsource, maxnestedsources)) /*overflow!*/
		return (false);
	
	(**hd).scriptsourcestack [(**hd).topscriptsource++] = source;
	
	return (true);
	} /*scriptpushsourcerecord*/


static boolean scriptpopsourcerecord (tysourcerecord *source) {
	
	register hdldebuggerrecord hd = debuggerdata;
	
	*source = (**hd).scriptsourcestack [--(**hd).topscriptsource];
	
	return (true);
	} /*scriptpopsourcerecord*/


static boolean scriptgetsourcerecord (hdlhashnode hnode, tysourcerecord *source) {
	
	/*
	look for a source code record for the indicated hashnode.
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register short ix;
	
	for (ix = 0; ix < (**hd).topscriptsource; ix++) { /*visit every window in stack*/
		
		if (hnode == (**hd).scriptsourcestack [ix].hnode) {
			
			*source = (**hd).scriptsourcestack [ix];
			
			return (true);
			}
		}
	
	return (false);
	} /*scriptgetsourcerecord*/


static boolean scriptprocessstarted (void) {
	
	//scriptresetbackgroundtime ();
	
	return (true);
	} /*scriptprocessstarted*/


static boolean scriptprocesskilled (void) {
	
	/*
	12/31/90 dmb: don't clear all debugger globals -- may be needed for error
	
	10/29/91 dmb: must clear flscriptsuspended flag to ensure proper button handling
	
	8/12/92 dmb: set scriptprocess to nil to as not to confuse scriptgettargetdata
	
	5.0d19 dmb: now that it's all in one window, close the runtime stack table.

	5.0.1 dmb: use tablefinddatawindow to find runtimestacktable. avoids rare crash.
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
	(**hd).scriptprocess = nil;
	
	(**hd).flscriptrunning = false;
	
	(**hd).flscriptsuspended = false;
	
	(**hd).topscriptsource = 0; /*cheap pop of original source record*/
	
	scriptinvalbuttons ();
	
	/*close the runtime stack window*/ {
		
		hdlwindowinfo hinfo;
		
		if (tablefinddatawindow (runtimestacktable, &hinfo))
			shellclosewindow ((**hinfo).macwindow);
		}
	
	return (true);
	} /*scriptprocesskilled*/


static boolean scriptmainexists (hdltreenode hcode, bigstring bsmodule) {
	
	/*
	skip over local subroutines and declarations and verify that at 
	least one line of executable code exists in the code tree
	*/
	
	register hdltreenode hp1 = (**(hcode)).param1;
	
	setemptystring (bsmodule);
	
	while (hp1) {
		
		switch ((**hp1).nodetype) {
			
			case noop:
			case localop:
				break;
			
			case moduleop: { /*grab name*/
				register hdltreenode hp2 = (**hp1).param2; /*should be procop*/
				
				assert (hp2 && ((**hp2).nodetype == procop));
				
				hp2 = (**hp2).param1;
				
				if (!langgetidentifier (hp2, bsmodule))
					return (false);

				/*
				if ((**hp2).nodetype == identifierop))
					pullstringvalue (&(**(**hp2).param1).nodeval, bsmodule);
				*/
				
				break;
				}
			
			default:
				return (true);
			}
		
		hp1 = (**hp1).link;
		}
	
	return (false); /*no meaty lines found*/
	} /*scriptmainexists*/


static boolean scriptdebugerrormessage (bigstring bsmsg, ptrvoid refcon) {
	
	/*
	6/23/92 dmb: new feature: we invoke the debugger at the current line 
	when an error is encountered, before allowing execution thread to 
	unwind.  this allows stack values to be examined.
	*/
	
	langerrordialog (bsmsg, refcon);
	
	(**currentprocess).errormessagecallback = &langerrordialog; /*prevent reentrance*/
	
	scriptdebugger (herrornode);
	
	return (true);
	} /*scriptdebugerrormessage*/


static boolean scriptdebugtraperror (hdltreenode hnode) {
	
	/*
	6.0a9 dmb: short form of scriptbugger, we unconditionally stop on the 
	error line, but don't flag it as an error
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	short topscriptsource;
	tysourcerecord source;
	long lnum = (**hnode).lnum;
	
	topscriptsource = (**hd).topscriptsource; /*move into local*/
	
	source = (**hd).scriptsourcestack [topscriptsource - 1]; /*move into local*/
	
	if (source.houtline == nil) /*an error occured when source was pushed*/
		return (true);
	
	if (source.pwindow == nil) { /*closed, or never opened*/
	
		if (!systemscripterrorroutine ((long) source.hnode, lnum, -1, nil, nil))
			return (true);
		
		source.pwindow = getfrontwindow ();
		
		/*put updated version on stack*/
		
		(**hd).scriptsourcestack [topscriptsource - 1].pwindow = source.pwindow;
		}
	
	windowbringtofront (source.pwindow);
	
	shellpushglobals (source.pwindow);
	
	opshowerror (lnum, -1);
	
	(**hd).lastlnum = lnum;
	
	(**hd).hbarcursor = (**outlinedata).hbarcursor;
	
	shellpopglobals ();
	
	(**hd).flscriptsuspended = true;
	
	(**hd).flstepping = true;
	
	(**hd).flfollowing = false;
	
	scriptinvalbuttons ();
	
	return (true);
	} /*scriptdebugtraperror*/


static boolean scriptdebugerrordebugger (bigstring bsmsg, ptrvoid refcon) {
	
	/*
	6/23/92 dmb: new feature: we invoke the debugger at the current line 
	when an error is encountered, before allowing execution thread to 
	unwind.  this allows stack values to be examined.
	*/
	
	hdlhashtable hdebugprefs;
	hdltreenode hnode = herrornode;
	tyvaluerecord val;
	bigstring bstrap;
	boolean fl;
	//static boolean scriptdebuggereventloop (void); /* 2006-02-04 aradke: declare at top of file */
	hdlhashnode hhashnode;
	#define str_userdebugprefs ("\x10" "user.prefs.debug")
	
	if (langcallbacks.errormessagecallback == scriptdebugerrormessage) //errors aren't being trapped
		return (true);
	
	if (hnode == nil)
		return (true);
	
	disablelangerror ();
	
	fl = langfastaddresstotable (roottable, str_userdebugprefs, &hdebugprefs) &&
		
		hashtablelookup (hdebugprefs, "\x0a" "trapErrors", &val, &hhashnode) &&
		
		copyvaluerecord (val, &val) && 
		
		coercetoboolean (&val) &&
		
		val.data.flvalue;
	
	enablelangerror ();
	
	if (!fl) //we're not trapping errors
		return (true);
	
	disablelangerror ();
	
	if (hashtablelookup (hdebugprefs, "\x0f" "trapErrorString", &val, &hhashnode) &&
		
		val.valuetype == stringvaluetype) {
		
		pullstringvalue (&val, bstrap);
		
		fl = isemptystring (bstrap) || patternmatch (bstrap, bsmsg);
		}
	
	enablelangerror ();
	
	if (!fl) //we're not trapping errors
		return (true);
	
	if (!scriptdebugtraperror (hnode))
		return (true);
	
	langerrordialog (bsmsg, refcon);
	
	scriptdebuggereventloop ();
	
	fllangerror = true; //in case it's been cleared by our actions
	
	return (!(**debuggerdata).flscriptkilled);
	} /*scriptdebugerrordebugger*/


static long getscriptparentrefcon (hdloutlinerecord ho) {

	/*
	4/7/97 dmb: there are now three possiblities for the source 
	of a script being run:

	1 - it's a menubar script. ho.refcon is the hdlheadrecord of menubar item
	2 - it's an odb script. ho.refcon is the hdlhashnode of the table value
	3 - it's a starndalone script. ho.refcon is zero, ho it it's own parent
	*/

	long refcon = (**ho).outlinerefcon; /*default: use outline's refcon*/

	switch ((**ho).outlinetype) {

		case outlineisdatabasescript: { /*script comes from a table*/
			
			hdlexternalvariable hvariable = (hdlexternalvariable) refcon;
			hdlhashnode hnode;
			
			assert (gethandlesize ((Handle) hvariable) == (sizeof (tyexternalvariable) + 4));
			
			if (!scriptfindhashnode (hvariable, &hnode))
				return (false);
			
			return ((long) hnode);
			}
		
		case outlineisstandalonescript:
			return ((long) ho);
		
		case outlineismenubarscript:
		default:
			return (refcon);
		}
	} /*getscriptparentrefcon*/


static boolean scriptnewprocess (short buttonnum) {
	
	/*
	2/5/91 dmb, on refcons: as discussed in scriptinmenubar, a outline's 
	refcon can be either a hdlexternalvariable or a hdlheadrecord.  while 
	debugging, we want to identify source by a hdlheadrecord or a 
	hdlhashnode.  we do the conversion from hdlexternalvariable to 
	hdlhashnode here so that no one else has to.
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	hdloutlinerecord ho = outlinedata;
	Handle htext;
	hdltreenode hcode;
	hdlprocessrecord hprocess;
	register hdlprocessrecord hp;
	tysourcerecord source;
	register boolean fl;
	register long refcon;
	bigstring bsmodule;
	
	setcursortype (cursoriswatch);
	
	if (!opgetlangtext (ho, false, &htext))
		return (false);
	
	refcon = getscriptparentrefcon (ho);

	langpusherrorcallback (&systemscripterrorroutine, refcon);
	
	fl = scriptbuildtree (htext, (**ho).outlinesignature, &hcode);
	
	langpoperrorcallback ();
	
	if (!fl) /*syntax error detected by compiler*/
		return (false);
	
	langerrorclear (); /*compilation produced no error, be sure error window is empty*/
	
	if (!scriptmainexists (hcode, bsmodule)) {
		bigstring bs;
		byte bsbutton [16];
		
		langdisposetree (hcode);
		
		shellgetbuttonstring (buttonnum, bsbutton);
		
		alllower (bsbutton);
		
		if (isemptystring (bsmodule))
			langgetstringlist (nomaintorunstring, bs);
		else
			langgetstringlist (needmodulecallstring, bs);
		
		parsedialogstring (bs, bsbutton, bsmodule, nil, nil, bs);
		
		shellerrormessage (bs);
		
		return (false);
		}
	
	if (!newprocess (hcode, true, &systemscripterrorroutine, refcon, &hprocess))
		return (false);
	
	hp = hprocess; /*copy into register*/
	
	(**hp).processstartedroutine = &scriptprocessstarted;
	
	(**hp).processkilledroutine = &scriptprocesskilled;
	
	addprocess (hp); /*insert in process list*/
	
	if (buttonnum == debugbutton) {
		
		(**hp).fldebugging = true;
	
		(**hp).errormessagecallback = &scriptdebugerrormessage;
	
		(**hp).debugerrormessagecallback = &scriptdebugerrordebugger;
		}
	
	clearhandle ((Handle) hd);
	
	bundle { /*initial debugger fields*/
		
		register ptrdebuggerrecord pd = *debuggerdata;
		
		(*pd).scriptprocess = hp;
		
		(*pd).flscriptrunning = true;
		
		(*pd).scriptrefcon = (**outlinedata).outlinerefcon; /*for later identification*/
		
		/* clearhandle gets all these
		
		(*pd).flscriptkilled = false;
		
		(*pd).flscriptsuspended = false;
		
		(*pd).flstepping = false;
		
		(*pd).flfollowing = false;
		
		(*pd).flwindowclosed = false;
		
		(*pd).fllangerror = false;
		
		(*pd).topscriptsource = 0;
		
		(*pd).sourcesteplevel = 0;
		
		(*pd).stepdir = nodirection;
		
		(*pd).lastlnum = 0;
		
		(*pd).hlocaltable = nil;
		
		(*pd).toplocaltable = 0;
		
		(*pd).hbarcursor = nil;
		*/
		}
	
	source.pwindow = outlinewindow; /*the main source window*/
	
	source.hnode = (hdlhashnode) refcon;
	
	source.houtline = outlinedata;
	
	scriptinvalbuttons ();
	
	return (scriptpushsourcerecord (source));
	} /*scriptnewprocess*/


static void scriptstepbutton (tydirection dir) {
	
	/*
	resume script execution until the source stack level changes 
	by the indicated amount.
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
	(**hd).flscriptsuspended = false; /*allow the script to resume*/
	
	(**hd).flstepping = true;
	
	(**hd).flfollowing = keyboardstatus.floptionkey;
	
	(**hd).sourcesteplevel = (**hd).topscriptsource;
	
	(**hd).stepdir = dir;
	} /*scriptstepbutton*/


static void scriptgobutton (void) {
	
	register hdldebuggerrecord hd = debuggerdata;
	
	(**hd).flscriptsuspended = false; /*allow the script to resume*/
	
	(**hd).flstepping = false; /*don't halt after every line of code*/
	
	(**hd).flfollowing = false; 
	
	(**hd).sourcesteplevel = (**hd).topscriptsource;
	} /*scriptgobutton*/


static void scriptkillbutton (void) {
	
	/*
	11/5/91 dmb: use new scriptprocess field to make kill more explicit
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
	processkill ((**hd).scriptprocess);
	
	(**hd).flscriptkilled = true;
	
	(**hd).flscriptsuspended = false; /*allow script to resume, so it can die*/
	} /*scriptkillbutton*/


#ifdef flcomponent

static pascal OSErr handlerecordedtext (const AppleEvent *event, AppleEvent *reply, SInt32 refcon) {
#pragma unused (reply, refcon)

	/*
	2.1b3 dmb: ignore extra syntax/lines needed for text-based recording
	
	2.1b4 dmb: special case leading comment character
	
	2.1b5 dmb: use opinserthandle logic to add text from other scripting systems.
	this handles multiple lines, which QuicKeys dishes out.
	
	2.1b8 dmb: strip nul characters from recorded text (QuicKeys again)
	
	2.1b9 dmb: don't call opinserthandle for non-outline text, or leading 
	tabs won't be stripped.
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register WindowPtr w = (**hd).scriptsourcestack [0].pwindow;
	AEDesc desc;
	boolean flfrontierserver;
	boolean floutline;
	tydirection dir;
	bigstring bs;
	Handle hstring;
	short indent;
	short diff;
	boolean flcomment = false;
	OSErr err;
	
	err = AEGetParamDesc (event, keyDirectObject, typeChar, &desc);
	
	if (err != noErr)
		return (err);
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		datahandletostring (&desc, bs);
	
	#else
	
		texthandletostring (desc.dataHandle, bs);
	
	#endif
	
	stringdeletechars (bs, chnul);
	
	if (isemptystring (bs))
		goto exit;
	
	indent = popleadingchars (bs, chtab);
	
	flfrontierserver = (**hd).servertype == typeLAND;
	
	if (flfrontierserver) { /*strip text-based syntax*/
		
		floutline = false; /*we'll strip trailing return*/
		
		if (bs [1] == chcomment) {
			
			deletefirstchar (bs);
			
			flcomment = true;
			}
		else {
			short len = stringlength (bs);
			
			if (lastchar (bs) == chreturn) /*strip trailing return*/
				len -= 1;
			
			switch (bs [len]) { /*look at the last char*/
				
				case '{':
					len -= 2;
					
					break;
				
				case ';':
					len -= 1;
					
					break;
				}
			
			if (bs [len] == '}')
				goto exit;
			
			setstringlength (bs, len);
			}
		}
	else
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			{
			Handle hcopy;
			
			copydatahandle (&desc, &hcopy);
			
			floutline = isoutlinetext (hcopy);
			
			disposehandle (hcopy);
			}
		
		#else
		
			floutline = isoutlinetext (desc.dataHandle);
	
		#endif
		
	diff = indent - (**hd).lastindent;
	
	if (shellpushglobals (w)) {
		
		if (diff > 0)
			dir = right;
		else {
			
			if (diff < 0)
				opmotionkey (left, -diff, false);
			
			dir = down;
			}
		
		if (floutline)
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				{
				Handle hcopy;
				
				copydatahandle (&desc, &hcopy);
				
				opinserthandle (hcopy, dir);
				}
			
			#else
			
				opinserthandle (desc.dataHandle, dir);
		
			#endif
			
		else {
			
			if (newtexthandle (bs, &hstring))
				if (opinsertheadline (hstring, dir, flcomment))
					shellcheckdirtyscrollbars ();
			}
		
		shellpopglobals ();
		}
	
	(**hd).lastindent = indent;
	
	exit:
	
	AEDisposeDesc (&desc);
	
	return (noErr);
	} /*handlerecordedtext*/


static boolean openservercomp (void) {

	register hdldebuggerrecord hd = debuggerdata;
	ComponentInstance comp = (**hd).servercomp;
	OSType servertype;
	
	servertype = (**outlinedata).outlinesignature;
	
	comp = getosaserver (servertype);
	
	if (comp == nil)
		return (false);
	
	(**hd).servercomp = comp;
	
	(**hd).servertype = servertype;
	
	return (true);
	} /*openservercomp*/


static void scriptrecordbutton (void) {
	
	register hdldebuggerrecord hd = debuggerdata;
	OSAID idscript = kOSANullScript;
	tysourcerecord source;
	
	if (!openservercomp ())
		return;
	
	setcursortype (cursoriswatch);
	
	if (oserror (OSAStartRecording ((**hd).servercomp, &idscript)))
		return;
	
	(**hd).idscript = idscript;
	
	(**hd).flrecording = true;
	
	assert ((**hd).topscriptsource == 0);
	
	source.pwindow = outlinewindow; /*the main source window*/
	
	source.hnode = nil;
	
	source.houtline = outlinedata;
	
	scriptpushsourcerecord (source);
	} /*scriptrecordbutton*/


static void scriptstoprecordbutton (void) {
	
	/*
	2.1b5 dmb: don't forget to dispose the recorded script id
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	ComponentInstance comp = (**hd).servercomp;
	OSAID idscript = (**hd).idscript;
	tysourcerecord source;
	OSAError err;
	
	setcursortype (cursoriswatch);
	
	err = OSAStopRecording (comp, idscript);
	
	if ((err != badComponentInstance) && oserror (err))
		return;
	
	scriptpopsourcerecord (&source);
	
	OSADispose (comp, idscript);
	
	(**hd).idscript = kOSANullScript;
	
	(**hd).flrecording = false;
	} /*scriptstoprecordbutton*/


static pascal OSErr handlestoprecording (const AppleEvent *event, AppleEvent *reply, SInt32 refcon) {
#pragma unused (event, reply, refcon)

	/*
	2.1b5 dmb: special event handler for QuicKeys recording panel
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
	if ((**hd).flrecording) {
		
		scriptstoprecordbutton ();
		
		scriptinvalbuttons ();
		}
	
	return (noErr);
	} /*handlestoprecording*/

#endif	// flcomponent

static boolean scriptstringlookup (bigstring bs, hdlhashtable *htable, bigstring bsname) {
	
	/*
	if the text in bs (a text selection) is something identifiable as 
	a language element, try to display the corresponding table entry
	*/
	
	Handle htext;
	hdltreenode hmodule;
	register boolean fl = false;
	register hdltreenode h;
	
	copystring (bs, bsname); /*start with literal interpretation*/
	
	if (langtablelookup (hkeywordtable, bsname, htable)) /*it's a keyword*/
		return (true);
	
	if (langtablelookup (hbuiltinfunctions, bsname, htable)) /*it's a build-in*/
		return (true);
	
	if (langtablelookup (hconsttable, bsname, htable)) /*it's a constant*/
		return (true);
	
	/*try to compile it for interpretation*/
	
	if (!newtexthandle (bs, &htext))
		return (false);
	
	if (!langcompiletext (htext, false, &hmodule)) /*always disposes htext*/
		return (false);
	
	h = (**hmodule).param1;
	
	fl = langgetdotparams (h, htable, bsname);
	
	langdisposetree (hmodule);
	
	if (!fl)
		return (false);
	
	if (*htable == nil)
		return (langsearchpathlookup (bsname, htable));
	
	return (langtablelookup (*htable, bsname, htable)); /*must already exist*/
	} /*scriptstringlookup*/


static boolean scriptzoomdotparams (bigstring bs) {
	
	/*
	bs should be an expression that resolves to an identifier or 
	dotparam pair.  if it is, locate the indicated hash node in its 
	table.  otherwise, beep
	
	9/1/92 dmb: push the top locals table here so we don't rely on being 
	called withing the script's context. allows us to get rid of the 
	deferredbutton stuff.
	
	9/11/92 dmb: for global external values, open the value's window rather 
	than selecting the value in its parent window. this is what is normally 
	desired, and allows scriptdoubleclick to use this w/out special-case code 
	*/
	
	register hdlhashtable ht = (**debuggerdata).hlocaltable;
	bigstring bsname;
	hdlhashtable htable;
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;
	
	disablelangerror ();
	
	if (ht != nil)
		pushhashtable (ht);
	
	fl = scriptstringlookup (bs, &htable, bsname);
	
	if (ht != nil)
		pophashtable ();
	
	enablelangerror ();
	
	if (!fl)
		return (false);
	
	/*
	if (flzoomlocalonly && !(**htable).fllocaltable)
		return (false);
	*/
	
	if ((**htable).fllocaltable) {	/*local value -- open stack window*/
 	
		if (!tablezoomfromtable (runtimestacktable))
			return (false);
		}
	else {							/*global value -- open window of externals*/
		
		hashtablelookup (htable, bsname, &val, &hnode);
		
		if (val.valuetype == externalvaluetype)
			return (langexternalzoom (val, htable, bsname));
		}
	
 	if (!tablezoomtoname (htable, bsname))
 		return (false);
 	
	return (true);
	} /*scriptzoomdotparams*/


static boolean scriptlocalsbutton (void) {
	
	/*
	5.0d18 dmb: we can now show whatever we zoom within the 
	context of the whole stack table, by zooming the latter first
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	hdlhashtable ht;
	bigstring bs;
	
	if ((**outlinedata).fltextmode) {
		
		opeditgetseltext (bs);
		
		if (!isemptystring (bs))
			return (scriptzoomdotparams (bs));
		}
	
	if (!tablezoomfromtable (runtimestacktable))
		return (false);
	
	if ((**hd).toplocaltable > 0 && // some locals are pushed
		findinparenttable ((**hd).hlocaltable, &ht, bs)) { // looked up ok
		
		if (!tablezoomtoname (ht, bs))
			return (false);
		
		shellpushfrontglobals ();
		
		if (outlinedata) // should be there
			opexpand ((**outlinedata).hbarcursor, 1, false);
		
		shellpopglobals ();
		}
		
		//	return (tablezoomfromtable ((**hd).hlocaltable));
	
//	return (tablezoomfromtable (runtimestacktable));

	return (true);
	} /*scriptlocalsbutton*/


static boolean scriptinstallbutton (void) {
	
	hdlexternalvariable hvariable;
	hdlhashnode hnode;
	hdltreenode hcode;
	
	hvariable = (hdlexternalvariable) (**outlinedata).outlinerefcon;
	
	if (!scriptfindhashnode (hvariable, &hnode))
		return (false);
	
	langerrorclear (); /*be sure error window is empty*/
	
	/*
	if (!processyieldtoagents ()) /%file closed while agents were finishing up%/
		return (false);
	*/
	
	if (!scriptcompiler (hnode, &hcode))
		return (false);
	
	scriptinvalbuttons (); // 5.28.97 dmb: compile button will dim now
		
	return (true);
	} /*scriptinstallbutton*/


static boolean scriptbutton (short buttonnum) {
	
	/*
	1/14/91 dmb: for the locals button, we act on the text selection, 
	if one exists.  for all other buttons, exit edit mode before 
	proceeding
	
	1/29/91 dmb: some button hits (Step, Locals) can't be handled reliably 
	when a background process is executing.  if the main process is 
	suspended (i.e. this is a background event) but isn't running now, 
	defer the button hit until control is returned to main process.  
	scriptdebuggereventloop watches for this condition
	
	6/23/92 dmb: don't defer install button hits; it never applies to the 
	currently-running script.  handling the install button in the debugged 
	script's thread is a strain on stack space, and can cause reentrancy if 
	an error is being handled and a compilation error occurs.
	
	9/1/92 dmb: as it turns out, the process/thread architecture has changed so 
	much since 1/29/91 that we really don't need the deferred button stuff at 
	all, which does indeed stress stack space -- especially when debugging after 
	a stack overflow error! the one thing we do have to do is push the current 
	locals table while handling the locals button in scriptzoomdotparams
	
	2.1b1 dmb: added recording logic
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
	if (buttonnum != localsbutton) /*exit edit mode on any button hit except locals*/
		opsettextmode (false);
	
	/*
	if ((**hd).flscriptsuspended && !debuggingcurrentprocess ()) { /%defer background hits%/
		
		if (buttonnum != installbutton) {
			
			(**hd).deferredbuttonnum = buttonnum;
			
			(**hd).deferredwindow = shellwindow;
			
			return (true);
			}
		}
	*/
	
	switch (buttonnum) {
		
	#ifdef flcomponent
		case recordbutton:
			scriptrecordbutton ();
			
			scriptinvalbuttons ();
			
			return (true);
	#endif		
		case runbutton:
			if (scriptnewprocess (runbutton))
				shellupdatenow (outlinewindow);
			
			return (true);
		
		case debugbutton:
			if (scriptnewprocess (debugbutton))
				scriptstepbutton (down); 
			
			return (true);
		
		case gobutton:
			scriptgobutton ();
			
			scriptinvalbuttons ();
			
			return (true);
		
		case followbutton:
			scriptstepbutton (nodirection); /*proceed as if stepping...*/
			
			(**hd).flfollowing = true; /*...but the cursor follows the interpreter*/
			
			scriptinvalbuttons ();
			
			return (true);
		
		case stepbutton:
			scriptstepbutton (down);
			
			return (true);
		
		case inbutton:
			scriptstepbutton (right);
			
			return (true);
		
		case outbutton:
			scriptstepbutton (left);
			
			return (true);
		
		case stopbutton:
	#ifdef flcomponent
			if ((**hd).flrecording)
				scriptstoprecordbutton ();
			else
	#endif
			 {
				
				scriptstepbutton (down); /*we're going to single-step thru scripts*/
				
				(**hd).flfollowing = false; 
				}
			
			scriptinvalbuttons ();
			
			return (true);
			
		case killbutton:
			scriptkillbutton ();
			
			return (true);
		
		case localsbutton:
			if (!scriptlocalsbutton ())
				sysbeep ();
			
			return (true);
		
		case installbutton:
			scriptinstallbutton (); 
			
			return (true);
		} /*switch*/
	
	return (true);
	} /*scriptbutton*/


static boolean scripthascleancode () {

	register hdloutlinerecord ho = outlinedata;
	hdlexternalvariable hvariable;
	hdlhashnode hnode;
	hdltreenode hcode;
	
	if ((**ho).flrecentlychanged)
		return (false);
	
	hvariable = (hdlexternalvariable) (**ho).outlinerefcon;
	
	if (!scriptfindhashnode (hvariable, &hnode))
		return (false);
	
	if (!langexternalvaltocode ((**hnode).val, &hcode)) /*doesn't contain code*/
		return (false);
	
	return (hcode != nil);
	} /*scripthasdirtycode*/


static boolean scriptbuttonenabled (short buttonnum) {
	
	/*
	return true if the button is enabled.
	
	1. if no script is running, then only the debug button is enabled.
	
	2. the "step button klooge" -- when the user presses the step button
	the first thing that happens is the button gets redrawn.  naturally,
	we're asked if it should be enabled.  by all logic, it should not be,
	since the program is not suspended, stepping isn't really a possibility.
	but, we know that the very next time around thru the debugger (which
	shouldn't be too long from now), it will make sense.  so to avoid an
	intermediate and distracting graying of the step button, we say 
	that it's enabled if the flag is set.
	
	8/26/90 DW: the klooge prevents the Install button from being properly
	enabled.  so we klooge around the klooge.
	
	1/2/91 dmb: rule 3. if a script is running, but isn't the current script, 
	don't enable anything except the install and kill buttons
	
	1/3/91 dmb: don't allow a script to be run when there's a one-shot 
	processing already running
	
	2/5/91 dmb: disable all buttons when script is blocked, even install & kill
	
	8/14/91 dmb: disable some buttons when script isn't suspended
	
	2.1b1 dmb: added recording logic
	
	2.1b4 dmb: can't do anything if context is locked
	*/
	
	register short x = buttonnum;
	register hdldebuggerrecord hd = debuggerdata;
	register boolean lflscriptrunning = (**hd).flscriptrunning;
	register boolean flscriptsuspended = (**hd).flscriptsuspended;
	register boolean flrunningthisscript;
	
	flrunningthisscript = lflscriptrunning && scriptinruntimestack ();
	
	if (lflscriptrunning) {
		
		if ((x != installbutton) && (x != killbutton)) /*rule 3*/
			if (!flrunningthisscript)
				return (false);
		}
	
	if ((**hd).flrecording)
		return (x == stopbutton);
	
	if ((**hd).flcontextlocked)
		return (false);
	
	/*
	else {
		
		if ((x != runbutton) && (x != debugbutton) && (x != installbutton)) /%rule 1%/
			return (false);
		}
	*/
	
	switch (x) {
		
		case recordbutton:
		#ifdef flcomponent
			return (!lflscriptrunning && scriptrecordable ());
		#else
			return (false);
		#endif
		
		case runbutton:
			return (!lflscriptrunning);
		
		case debugbutton:
			return (!lflscriptrunning && ((**outlinedata).outlinesignature == typeLAND));
		
		case stopbutton:
			return (!flscriptsuspended);
		
		case gobutton:
			return ((flscriptsuspended || (**hd).flfollowing) && !(**hd).fllangerror);
		
		case followbutton:
		case stepbutton:
		case inbutton:
		case outbutton:
			return (lflscriptrunning && flscriptsuspended && !(**hd).fllangerror);
		
		case localsbutton:
			return (lflscriptrunning && flscriptsuspended); /*(**hd).flstepping && (!(**hd).flfollowing))*/
		
		case killbutton:
			return (lflscriptrunning);
		
		case installbutton:
			return (!flrunningthisscript && scriptinstallable () && !scripthascleancode ());
		} /*switch*/
	
	return (true);
	} /*scriptbuttonenabled*/


static boolean scriptbuttondisplayed (short buttonnum) {
	
	/*
	return true if the button should be displayed.
	
	1. if no script is running, then display the run button, and 
	maybe the install button
	
	2. if a script is running, don't show the run button or the 
	install button, but show everything else
	
	1/2/91 dmb: if this script isn't in the run-time stack, we're not
	"running"
	
	1/14/91 dmb: it's deathly to install a script when it's running, 
	since the currently-running code tree will be disposed.  modified 
	rule 2 above accordingly
	
	2/1/91 dmb: if a script is running, show the kill button in all script 
	windows (not just those in the run-time stack) and never show the run
	button
	
	2.1b1 dmb: added recording logic
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register hdlprocessrecord hp = (**hd).scriptprocess;
	register boolean lflscriptrunning = (**hd).flscriptrunning;
	register boolean flrunningthisscript;
	register boolean fldebuggingthisscript;
	register short x = buttonnum;
	
	flrunningthisscript = lflscriptrunning && scriptinruntimestack (); /*1/2/91*/
	
	fldebuggingthisscript = flrunningthisscript && (**hp).fldebugging;
	
#ifdef flcomponent
	if ((**hd).flrecording)
		return ((x == recordbutton) || (x == stopbutton));
#endif
	
	switch (x) {
		
		case recordbutton:
		#ifndef flcomponent
			return (false);
		#endif
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 7.0b43: no record button in OS X*/
		
			return (false);
		
		#endif
		
		case runbutton:
		case debugbutton:
			return (!lflscriptrunning);
		
		case installbutton:
			return (!fldebuggingthisscript && scriptinstallable ());
		
		case stopbutton:
			return (fldebuggingthisscript && !(**hd).flscriptsuspended);
		
		case gobutton:
			return (fldebuggingthisscript && (**hd).flscriptsuspended);
		
		case followbutton:
		case stepbutton:
		case inbutton:
		case outbutton:
		case localsbutton:
			return (fldebuggingthisscript);
		
		case killbutton:
			return (lflscriptrunning);
		} /*switch*/
	
	return (true);
	} /*scriptbuttondisplayed*/


static boolean scriptbuttonstatus (short buttonnum, tybuttonstatus *status) {
	
	/*
	boolean fldisplay = scriptbuttondisplayed (buttonnum);
	
	(*status).fldisplay = fldisplay;
	
	(*status).flenabled = fldisplay && scriptbuttonenabled (buttonnum);
	*/
	if (outlinedata == NULL)
		return (false);

	(*status).flenabled = scriptbuttonenabled (buttonnum);
	
	(*status).fldisplay = scriptbuttondisplayed (buttonnum);
	
	(*status).flbold = false; /*our buttons are never bold*/
	
	return (true);
	} /*scriptbuttonstatus*/


#if 0

boolean scriptkilled (void) {
	
	/*
	returns true if the current running script should be killed by the
	interpreter.  it's called back from within the interpreter.
	
	7/2/91 dmb: no longer check the scriptkilled debuggerdata flag here; 
	it's handled by returning false from the debugger routine.  checking 
	it here will wrongly terminate some other process
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
	/*
	if ((**hd).flscriptkilled) {
		
		if (processisoneshot (false)) { /%we're running a one-shot process%/
			
			(**hd).flscriptkilled = false; /%consume it%/
			
			scriptprocesskilled ();
			
			return (true);
			}
		}
	*/
	
	if (keyboardescape ()) { /*user pressed cmd-period or something like that*/
		
		if (processisoneshot (true)) { /*cmd-period doesn't kill background tasks*/
			
			keyboardclearescape (); /*consume it*/
			
			return (true);
			}
		}
		
	return (false);
	} /*scriptkilled*/

#endif


static boolean scriptdebuggercallback (void) {
	
	register hdldebuggerrecord hd = debuggerdata;
	
	if (keyboardescape ()) /*user pressed cmd-period*/
		scriptkillbutton ();
	
	if (!(**hd).flscriptsuspended) /*get ready to resume running the script*/
		return (false);
	
	assert (!(**hd).flwindowclosed); /*if it was closed, script shouldn't be suspended*/
	
	/*
	if ((**hd).deferredbuttonnum > 0) /%a script button hit was deferred%/
		return (false);
	*/
	
	return (true); /*keep getting events from the user*/
	} /*scriptdebuggercallback*/


static boolean scriptdebuggereventloop (void) {
	
	/*
	8/12/91 dmb: make sure menus aren't dimmed
	
	2.1b3 dmb: handle not having threads

	5.0a19 dmb: sleep instead of yielding
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
	processnotbusy (); /*un-dim menus*/
	
	while ((**hd).flscriptsuspended) {
		
		if (flcanusethreads) {
			
			while (scriptdebuggercallback ())
				if (!processsleep (getcurrentthreadglobals (), 10))
					return (false);
			}
		else
			shelleventloop (&scriptdebuggercallback);
		
		/*
		if ((**hd).deferredbuttonnum > 0) { /%a background button click was deferred%/
			
			shellpushglobals ((**hd).deferredwindow);
			
			scriptbutton ((**hd).deferredbuttonnum);
			
			shellpopglobals ();
			
			(**hd).deferredbuttonnum = 0; /%consume it%/
			}
		*/
		}
	
	return (true);
	} /*scriptdebuggereventloop*/


static boolean scriptwindowmessage (bigstring bs) {
	
	/*
	display the message in the active script source window.
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register WindowPtr w = (**hd).scriptsourcestack [(**hd).topscriptsource - 1].pwindow;
	
	if (w == nil)
		return (false);
	
	if (!shellpushglobals (w))
		return (false);
	
	shellwindowmessage (bs);
	
	shellpopglobals ();
	
	return (true);
	} /*scriptwindowmessage*/


boolean scriptdebugger (hdltreenode hnode) {
	
	/*
	allow the user to sniff around while a script is running.
	
	the interpreter is running the script linked into the indicated headrecord
	at the indicated line number.
	
	we won't automatically zoom the script window open, assume if it's closed
	the user isn't interested in debugging.
	
	returns false if the user pressed the Kill button, a signal to the
	interpreter to shut down the script.
	
	1/14/91 dmb: no longer bring script windows to front when following.  added 
	support for breakpoints in handlers.
	
	1/22/91 dmb: don't call shellshorteventloop at beginning.  we're already 
	doing enough background tasking, and we need to save and restore globals 
	around such calls (as is now done at the end of this routine).
	
	8/14/91 dmb: attempt to prevent multiple steps on single line when there's 
	more than one statement on it.
	
	5/20/92 dmb: now take hdltreenode parameter instead of its lnum, cnum pair.
	
	6/23/92 dmb: treat fllangerror as unconditional stop; this new case 
	can occur with the addition of scriptdebugerror routine
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register hdltreenode hn = hnode;
	register tytreetype op;
	register boolean flshowline = false;
	register boolean lflcontinue = true;
	short topscriptsource;
	tysourcerecord source;
	boolean flbreakpoint = false;
	long lastlnum = (**hd).lastlnum;
	long lnum = (**hn).lnum;
	short diff = 0;
	
	//shellshorteventloop (); /*suck up and process any waiting events*/
	
	if (languserescaped (true)) /*stop running the script immediately*/
		return (false);
	
	if ((**hd).flscriptkilled) /*user pressed kill button*/
		return (false);
	
	if ((**hd).flwindowclosed)
		return (true);
	
	op = (**hn).nodetype; /*test for "meaty" op*/
	
	if ((op == moduleop) || (op == noop) || (op == bundleop) || (op == localop)) /*never stop on these*/
		return (true);
	
	topscriptsource = (**hd).topscriptsource; /*move into local*/
	
	source = (**hd).scriptsourcestack [topscriptsource - 1]; /*move into local*/
	
	if (source.houtline == nil) /*an error occured when source was pushed*/
		return (true);
	
	if (fllangerror) {
		
		(**hd).fllangerror = true; /*for button updating*/
		
		flshowline = true;
		
		lflcontinue = false;
		}
	else {
		
		diff = topscriptsource - (**hd).sourcesteplevel;
		
		if ((diff == 0) && (lnum == lastlnum)) /*we haven't moved off of previous line*/
			return (scriptbackgroundtask (false));
		}
	
	if (diff <= 0)
		(**hd).lastlnum = lnum;
	
	if ((**hd).flstepping && !flshowline) { /*the debugger is turned on*/
		
		/*
		short diff = topscriptsource - (**hd).sourcesteplevel;
		*/
		
		switch ((**hd).stepdir) {
				
			case down:
				if (diff == 0)
					flshowline = lnum != lastlnum;
				else
					flshowline = diff < 0;
				break;
			
			case right:
				flshowline = diff != 0; /*if less then zero, In step fails and stops*/
				break;
			
			case left:
				flshowline = diff < 0;
				break;
			
			case nodirection: /*following*/
				flshowline = false;
				break;
			
			default:
				/* do nothing for up, flatup, flatdown, sorted, pageup, pagedown, pageleft, pageright */
				break;
			}
		
		lflcontinue = !flshowline; /*continue if we aren't going to show this line*/
		
		if ((**hd).flfollowing)
			flshowline = true; /*show every line when following*/
		}
	
	if (lflcontinue) { /*so far, nothing is suspending the processor*/
		
		hdlheadrecord hnoderec;
		
		oppushoutline (source.houtline); /*set globals for breakpoint check*/
		
		if (opgetnthnode (lnum, &hnoderec) && (**hnoderec).flbreakpoint) /*breakpoint*/
			
			if (!opnestedincomment (hnoderec)) {
				
				flbreakpoint = true;
				
				flshowline = true;
				
				lflcontinue = false;
				}
		
		oppopoutline ();
		}
	
	if (flshowline) {
		
		if (source.pwindow == nil) { /*closed, or never opened*/
			
			if (lflcontinue || fllangerror) { /*show line if window is open; don't bring to front*/
				
				hdlwindowinfo hinfo;
				
				/*
				if (!langexternalwindowopen ((**source.hnode).val))
				*/
				if (!(**source.houtline).flwindowopen)
					goto exit;
				
				if (!shellfinddatawindow ((Handle) source.houtline, &hinfo))
					goto exit;
				
				source.pwindow = (**hinfo).macwindow;
				}
			else { /*we're stopping, so open window and bring to front*/
				/*	
				if (!scriptzoomscript (source.hnode, &source.pwindow))
					return (true);
				*/
				
				if (!systemscripterrorroutine ((long) source.hnode, lnum, -1, nil, nil))
					return (true);
				
				source.pwindow = getfrontwindow ();
				}
			
			/*put updated version on stack*/
			
			(**hd).scriptsourcestack [topscriptsource - 1].pwindow = source.pwindow;
			}
		
		if (!lflcontinue && !fllangerror)
			windowbringtofront (source.pwindow);
		
		shellpushglobals (source.pwindow);
		
		opshowerror (lnum, -1);
		
		(**hd).lastlnum = lnum;
		
		(**hd).hbarcursor = (**outlinedata).hbarcursor;
		
		shellpopglobals ();
		}
	
	exit:
	
	if (lflcontinue) /*don't drop into event loop, continue running script*/
		return (scriptbackgroundtask (false));
	
	(**hd).flscriptsuspended = true;
	
	(**hd).flstepping = true;
	
	(**hd).flfollowing = false;
	
	scriptinvalbuttons ();
	
	if (flbreakpoint) {
		bigstring bs;
		
		langgetmiscstring (breakpointstring, bs);
		
		scriptwindowmessage (bs);
		}
	
	scriptdebuggereventloop ();
	
	if (flbreakpoint)
		scriptwindowmessage ((ptrstring) zerostring);
	
	return (!(**hd).flscriptkilled);
	} /*scriptdebugger*/


boolean scriptpushsourcecode (hdlhashtable htable, hdlhashnode hnode, bigstring bsname) {
#pragma unused (bsname)

	/*
	code execution is about to move into a new script.  hnode should be the 
	node in htable containing the script (external variable).
	
	2/4/91 dmb: hnode can now contain code -- a local handler.  we need to 
	use htable and the localtablestack to figure out which source the 
	code came from.  kind of ugly.
	
	2/5/91 dmb: too ugly, and incomplete.  yesterday's hack only worked 
	under the script debugger; we need to keep the langerrorcallbacks 
	consistant at all times.  new method: langaddhandler stuffs the error
	refcon into the code tree's value field, which is otherwise unused.
	
	4/8/93 dmb: make sure htable is local before assuming codevalue is local
	
	2.1b3 dmb: when synthesizing an errorrefcon for a code module, lookup 
	the associated error routine.
	*/
	
	register hdlhashnode h = hnode;
	langerrorcallback errorcallback;
	
	errorcallback = &systemscripterrorroutine;
	
	if (h != nil)
		
		if (((**h).val.valuetype == codevaluetype) && (**htable).fllocaltable) { /*a local handler*/
			
			register hdltreenode hmodule = (**h).val.data.codevalue;
			
			h = (hdlhashnode) (**hmodule).nodeval.data.longvalue;
			
			langfinderrorrefcon ((long) h, &errorcallback);
			}
	
	if (debuggingcurrentprocess ()) {
		
		tysourcerecord source;
		boolean fljustloaded; /***needs memory management*/
		hdlwindowinfo hinfo;
		
		if (!scriptgetsourcerecord (h, &source)) {
			
			source.hnode = h;

			source.pwindow = nil; /*no window that we know about yet*/
			
			source.houtline = nil;
			
			if (h != nil) {
				
				if (shellfinddatawindow ((Handle) h, &hinfo)) {

					source.pwindow = (**hinfo).macwindow;

					source.houtline = (hdloutlinerecord) h;
					}
				else {
				
					if (isheadrecordhandle ((long) h))
						meloadscriptoutline (menudata, (hdlheadrecord) h, &source.houtline, &fljustloaded);
					else
						opvaltoscript ((**h).val, &source.houtline);
					}
				}
			}
		
		if (!scriptpushsourcerecord (source))
			return (false);
		}
	
	return (langpusherrorcallback (errorcallback, (long) h));
	} /*scriptpushsourcecode*/


boolean scriptpopsourcecode (void) {
	
	tysourcerecord source;
	
	if (debuggingcurrentprocess ()) {
		
		scriptpopsourcerecord (&source);
		
		if (source.pwindow != nil)
			scriptinvalbuttonsvisit (source.pwindow, nil);
		}
	
	return (langpoperrorcallback ());
	} /*scriptpopsourcecode*/


static void scriptgetlevelname (bigstring bs) {
	
	/*
	1/8/91 dmb: if we're going to add the node name onto the table name, 
	we need to keep a handle to the node itself (or something) around so 
	that we can reliably delete the table, even after the code node is gone 
	and we can't regenerate the name.
	
	10/6/91 dmb make sure level names have two digits so they sort nicely.
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	hdlhashnode h;
	short level = hashgetstackdepth ();
	hdlwindowinfo hinfo;
	bigstring title;
	
	langgetmiscstring (levelstring, bs);
	
	if (level <= 9) /*single digit; pad with 0*/
		pushchar ('0', bs);
	
	pushint (level, bs);
	
	pushstring ("\x02" ": ", bs); /*prepare to append script name*/
	
	h = (**hd).scriptsourcestack [(**hd).topscriptsource - 1].hnode;
	
	if (isheadrecordhandle ((long) h)) {
		
		opgetheadstring ((hdlheadrecord) h, title);
		
		pushstring (title, bs);
		}
	else if (shellfinddatawindow ((Handle) h, &hinfo)) {

		shellgetwindowtitle (hinfo, title); // 7.24.97 dmb: was windowgettitle

		pushstring (title, bs);
		}
	else if (h != nil)
		pushstring ((**h).hashkey, bs);
	else
		langgetmiscstring (unknownstring, bs);
	} /*scriptgetlevelname*/


boolean scriptpushtable (hdlhashtable *htable) {
	
	/*
	1/23/91 dmb: use tablenewsystemtable instead of tablenewsubtable.  
	we want the dontsave flag set, and protection from user deletion
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register hdlhashtable ht;
	register boolean fl;
	bigstring bs;
	hdlhashnode hnode;
	
	if (!langcheckstacklimit (idlocalsstack, (**hd).toplocaltable, maxchainedlocals)) /*overflow!*/
		return (false);
	
	scriptgetlevelname (bs);
	
	pushhashtable (runtimestacktable);
	
	assert (!hashsymbolexists (bs));
	
	fl = tablenewsystemtable (runtimestacktable, bs, htable);
	
	if (fl)
		fl = hashlookupnode (bs, &hnode); /*should never fail if last call succeeded*/
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	ht = *htable; /*copy into register*/
	
	(**ht).fllocaltable = true;
	
	chainhashtable (ht); /*link it into the runtime stack*/
	
	(**hd).hlocaltable = ht;
	
	/*
	(**ht).hashtableformats = (**hd).localtableformatsstack [(**hd).toplocaltable];
	*/
	
	(**hd).localtablestack [(**hd).toplocaltable++] = hnode;
	
	return (true);
	} /*scriptpushtable*/


boolean scriptpoptable (hdlhashtable htable) {
#pragma unused (htable)

	/*
	1/9/91 dmb: see comment in scriptgetlevelname...
	
	decided to keep a handle to the hash node key stored in a
	stack in debuggerdata.  this should be pretty fast, but the down side 
	is that the stack has an upper limit (not enforced -- potential crash), 
	while the locals chain doesn't.  there are at least three ways to avoid 
	this limit:
		
		1.  make the local name stack dynamically-sized.  since speed is
			important, handle resizing is best avoided.  a hassle.
		
		2.  use hashtablevisit to locate the value in runtimestacktable
			containing currenthashtable, and delete the node using its key.
		
		3.  use hashinversesearch to locate the node whose name begins 
			with "level xx", ignoring the rest of the string
	
	the current implementation is the easiest, and perhaps the fastest
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register hdlhashnode hnode;
	bigstring bs;
	
	hnode = (**hd).localtablestack [--(**hd).toplocaltable];
	
	/*
	(**hd).localtableformatsstack [(**hd).toplocaltable] = (**htable).hashtableformats;
	*/
	
	gethashkey (hnode, bs);
	
	/*
	hashtablevisit (runtimestacktable, &scriptpoptablevisit);
	*/
	
	unchainhashtable ();
	
	pushhashtable (runtimestacktable);
	
	if (!hashdelete (bs, true, false)) /*also disposes of the hashtable*/
		shellinternalerror (idscriptpoptablefailed, "\x15" "scriptpoptable failed");
	
	pophashtable ();
	
	(**hd).hlocaltable = currenthashtable;
	
	return (true);
	} /*scriptpoptable*/


static void scriptgetwindowtitle (hdlheadrecord hnode, bigstring bs) {
	
	bigstring bshead;
	
	getheadstring (hnode, bshead);
	
	setparseparams (bshead, nil, nil, nil);
		
	shellgetstring (scriptforstring, bs);
	
	parseparamstring (bs); /*insert script name into window title string*/
	} /*scriptgetwindowtitle*/ 


boolean scriptgetdebuggingcontext (hdlhashtable *hcontext) {
	
	/*
	2.1b4: make our context available, if a script is suspended in the 
	debugger. it becomes locked until freed by the caller.  (this may 
	not be strictly necessary, but it seems prudent
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
	if (!(**hd).flscriptsuspended || !(**hd).flscriptrunning)
		return (false);
	
	*hcontext = (**hd).hlocaltable;
	
	if (*hcontext == nil)
		return (false);
	
	(**hd).flcontextlocked = true;
	
	scriptinvalbuttons ();
	
	return (true);
	} /*scriptgetdebuggingcontext*/


void scriptunlockdebuggingcontext (void) {
	
	register hdldebuggerrecord hd = debuggerdata;
	
	(**hd).flcontextlocked = false;
	
	scriptinvalbuttons ();
	
	} /*scriptunlockdebuggingcontext*/


boolean scriptzoomwindow (Rect rwindow, Rect rzoom, hdlheadrecord hcursor, WindowPtr *w) {
	
	/*
	5.0a10 dmb: don't zoom now. window isn't all the way set up.
	under windows, it matters.
	*/
	
	bigstring bs; 
	
	scriptgetwindowtitle (hcursor, bs);
	
	if (!newchildwindow (idscriptconfig, menuwindowinfo, &rwindow, &rzoom, bs, w))
		return (false);
	
	return (true); //windowzoom (*w));
	} /*scriptzoomwindow*/


#if 0

static boolean scripthaslinkedtextroutine (hdlheadrecord hnode) {
	
	return (false); /*no linked text for script lines*/
	} /*scripthaslinkedtextroutine*/

#endif


static boolean optogglebreakpoint (hdlheadrecord hnode) {
	
	/*
	2.1b2 dmb: return new state
	
	5.0a25 dmb: dirty the outline...

	5.0.1 dmb: ...but clear the recently changed bit to avoid recompilation
	
	5.0.2 dmb: don't clear the recently changed bit; restore it.
	*/
	
	boolean lflbreak = !(**hnode).flbreakpoint;
	boolean flrecentlychanged = (**outlinedata).flrecentlychanged;
	
	(**hnode).flbreakpoint = lflbreak;
	
	opinvalnode (hnode); /*inval the dirty line*/
	
	opdirtyoutline ();
	
	(**outlinedata).flrecentlychanged = flrecentlychanged;
	
	opupdatenow ();
	
	return (lflbreak);
	} /*optogglebreakpoint*/


static boolean scriptcmdclick (hdlheadrecord hnode) {
	
	/*
	5.0a25 dmb: shift key will Go if enabled. return true if we consume
	the click
	
	5.0.2b19 dmb: preserve outline's text mode
	*/
	
	hdloutlinerecord ho = outlinedata;
	
	if (keyboardstatus.floptionkey) {
	
		(**ho).fltextmode = (**ho).hbuffer != nil; //preserve text mode
		
		if (optogglebreakpoint (hnode)) { /*breakpoint set*/
			
			if (keyboardstatus.flshiftkey && scriptbuttonenabled (gobutton))
				scriptgobutton ();
			}
		
		return (true);
		}
	
	return (false);
	} /*scriptcmdclick*/


static boolean scriptdoubleclick (void) {
	
	/*
	12/9/91 dmb: new hook to make cmd-doubleclick behave like locals button
	*/
	
	if (keyboardstatus.flcmdkey && !keyboardstatus.flcontrolkey)
		
		if (scriptbuttonenabled (localsbutton)) {
			
			register hdlhashtable ht = (**debuggerdata).hlocaltable;
			register boolean fl;
			
			if (ht != nil) {
				
				/*
				flzoomlocalonly = true;
				
				pushhashtable (ht);
				*/
				
				fl = scriptlocalsbutton ();
				
				/*
				pophashtable ();
				
				flzoomlocalonly = false;
				*/
				
				if (fl)
					return (false); /*don't process click any further*/
				}
			}
	
	return (true); /*process click normally*/
	} /*scriptdoubleclick*/


static void scriptdisposescrap (hdloutlinerecord houtline) {
	
	opdisposeoutline (houtline, false);
	} /*scriptdisposescrap*/


static boolean scriptexportscrap (hdloutlinerecord houtline, tyscraptype totype, Handle *hexport, boolean *fltempscrap) {
	
	*fltempscrap = true; /*usually the case*/
	
	switch (totype) {
		
		case scriptscraptype: /*export flat version for system scrap*/
			return (oppackoutline (houtline, hexport));
			
		case opscraptype:
			*hexport = (Handle) houtline; /*op and script scraps are the same*/
			
			*fltempscrap = false; /*it's the original, not a copy*/
			
			return (true);
		
		case textscraptype:
			/*
			return (opoutlinetonewtextscrap (houtline, hexport));
			*/
			return (opgetlangtext (houtline, true, hexport));
			
		default:
			return (false);
		}
	} /*scriptexportscrap*/


static boolean scriptsetscraproutine (hdloutlinerecord houtline) {
	
	return (shellsetscrap ((Handle) houtline, scriptscraptype, 
				(shelldisposescrapcallback) &scriptdisposescrap,
				(shellexportscrapcallback) &scriptexportscrap));
	} /*scriptsetscraproutine*/


static boolean scriptgetscraproutine (hdloutlinerecord *houtline, boolean *fltempscrap) {
	
	Handle hscrap;
	tyscraptype scraptype;
	
	if (!shellgetscrap (&hscrap, &scraptype))
		return (false);
	
	if (scraptype == scriptscraptype) {
		
		*houtline = (hdloutlinerecord) hscrap;
		
		*fltempscrap = false; /*we're returning a handle to the actual scrap*/
		
		return (true);
		}
	
	return (shellconvertscrap (opscraptype, (Handle *) houtline, fltempscrap));
	} /*scriptgetscraproutine*/


static boolean scriptcommentvisit (hdlheadrecord hnode, void *refcon) {
#pragma unused (refcon)

	register hdlheadrecord h = hnode;
	bigstring bs;
	
	getheadstring (h, bs);
	
	if (!isemptystring (bs) && bs [1] == chcomment) {
		
		pullfromhandle ((**h).headstring, 0, 1, nil); /*don't use opsetheadstring; don't want callbacks invoked*/
		
		if (!opnestedincomment (h))
			(**h).flcomment = true;
		}
	
	return (true);
	} /*scriptcommentvisit*/


static boolean scripttexttooutlineroutine (hdloutlinerecord houtline, Handle hscrap, hdlheadrecord *hnode) {
	
	/*
	2.1b4 dmb: new feature: we strip braces and semicolons out of pasted (or inserted) 
	text automatically using langstriptextsyntax. then we convert comment lines to 
	true comments.
	*/
	
	boolean flusertalk = (**houtline).outlinesignature == typeLAND;
	
	if (flusertalk)
		langstriptextsyntax (hscrap);
	
	if (!optextscraptooutline (houtline, hscrap, hnode))
		return (false);
	
	if (flusertalk)
		opsiblingvisiter (*hnode, false, &scriptcommentvisit, nil);
	
	return (true);
	} /*scripttexttooutlineroutine*/


static boolean scriptscraphook (Handle hscrap) {
	
	/*
	if our private type is on the external clipboard, set the internal 
	scrap to it.
	*/
	
	if (getscrap (scriptscraptype, hscrap)) {
		
		hdloutlinerecord houtline;
		
		if (opunpackoutline (hscrap, &houtline))
			scriptsetscraproutine (houtline);
		
		return (false); /*don't call any more hooks*/
		}
	
	return (true); /*keep going*/
	} /*scriptscraphook*/


void scriptsetcallbacks (hdloutlinerecord houtline) {
	
	/*
	1/31/97 dmb: don't need to set setscrollbarsroutine anymore
	*/
	
	register hdloutlinerecord ho = houtline;
	
	/*
	(**ho).setscrollbarsroutine = (callback) &scriptsetscrollbarsroutine;
	
	(**ho).drawlinecallback = &scriptdrawline;
	
	(**ho).postdrawlinecallback = &truenoop;
	
	(**ho).haslinkedtextcallback = &scripthaslinkedtextroutine;
	*/
	
	(**ho).cmdclickcallback = &scriptcmdclick;
	
	(**ho).doubleclickcallback = &scriptdoubleclick;
	
	(**ho).setscrapcallback = (opsetscrapcallback) &scriptsetscraproutine;
	
	(**ho).getscrapcallback = (opgetscrapcallback) &scriptgetscraproutine;
	
	(**ho).texttooutlinecallback = (optexttooutlinecallback) &scripttexttooutlineroutine;

	} /*scriptsetcallbacks*/


boolean scriptsetdata (WindowPtr w, hdlheadrecord hnode, hdloutlinerecord houtline) {
	
	/*
	called by the menu editor to establish script content
	
	2/28/91 dmb: outline must inherit activeness from windowinfo
	*/
	
	register hdloutlinerecord ho = houtline;
	hdlwindowinfo hinfo;
	bigstring bs;
	
	getwindowinfo (w, &hinfo);
	
	(**hinfo).hdata = (Handle) ho;
	
	(**ho).outlinerect = (**hinfo).contentrect;
	
	(**ho).flactive = (**hinfo).flwindowactive;
	
	scriptsetcallbacks (ho);
	
	shellpushglobals (w);
	
	pushemptyclip ();
	
	/*
	opgetdisplayinfo ();
	*/
	
	opdirtymeasurements (); //6.0a14 dmb
	
	opsetctexpanded (ho); //6.0a14 dmb
	
	opredrawscrollbars (); 	// 1/31/97 dmb - was: scriptsetscrollbarsroutine ();
	
	opschedulevisi ();
	
	opactivate ((**hinfo).flwindowactive);
	
	popclip ();
	
	invalrect ((**ho).outlinerect);
	
	eraserect ((**ho).outlinerect);
	
	shellpopglobals ();
	
	scriptgetwindowtitle (hnode, bs);
	
	shellsetwindowtitle (hinfo, bs); // 7.24.97 dmb: was windowsettitle
	
	return (true);
	} /*scriptsetdata*/


static void scriptresize (void) {
	
	opresize ((**outlinewindowinfo).contentrect);
	
	#ifdef WIN95VERSION
		opupdatenow ();
	#endif
	} /*scriptresize*/


static boolean scriptgetvariablepath (hdlexternalvariable hvariable, bigstring bspath) {
	
	/*
	return the full path to the given variable
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	
	if (!langexternalfindvariable (hvariable, &htable, bsname))
		return (false);
	
	return (langexternalgetquotedpath (htable, bsname, bspath));
	} /*scriptgetvariablepath*/


static boolean scriptverifycompilation () {
	
	/*
	9/25/91 dmb: new feature -- when a script window is closed, we give the 
	call a special script in the database.  if the script is there and 
	returns true, we'll try to recompile the script.
	
	if compilation is attempted but fails, return false; otherwise return true.
	
	10/1/91 dmb: don't need to do anything if script hasn't yet been compiled.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	hdlexternalvariable hvariable;
	hdlhashnode hnode;
	hdltreenode hcode;
	bigstring bspath;
	bigstring bsscript;
	bigstring bsresult;
	
	if (flinhibitclosedialogs) /*not allowed to do verification*/
		return (true);
	
	if (!(**ho).flrecentlychanged) /*no changes to compile*/
		return (true);
	
	hvariable = (hdlexternalvariable) (**ho).outlinerefcon;
	
	if (!scriptfindhashnode (hvariable, &hnode))
		return (true);
	
	if (!langexternalvaltocode ((**hnode).val, &hcode)) /*doesn't contain code*/
		return (true);
	
	if (hcode == nil) /*never been compiled; don't need to re-compile*/
		return (true);
	
	if (!scriptgetvariablepath (hvariable, bspath))
		return (true);
	
	/*
	parsedialogstring (bsclosescript, bspath, nil, nil, nil, bsscript);
	*/
	if (!getsystemtablescript (idcompilewindowscript, bsscript))
		return (true);
	
	parsedialogstring (bsscript, bspath, nil, nil, nil, bsscript);
	
	if (!langrunstringnoerror (bsscript, bsresult)) /*couldn't find/run the script*/
		return (true);
	
	if (equalstrings (bsresult, "\x01" "3")) /*yes/no/cancel was cancelled*/
		return (false);
	
	if (!equalstrings (bsresult, bstrue) && !equalstrings (bsresult, "\x01" "1")) /*script didn't ask for compilation*/
		return (true);
	
	langerrorclear (); /*be sure error window is empty*/
	
	/*
	if (!processyieldtoagents ()) /%file closed while agents were finishing up%/
		return (false);
	*/
	
	return (scriptcompiler (hnode, &hcode)); /*try to recompile*/
	} /*scriptverifycompilation*/


static boolean scriptclose (void) {
	
	/*
	12/28/90 dmb: new logic includes support for script source stack:
	
	if the main script window is closed, and we're suspended, we kill 
	the script.  if we're not suspended, we keep going but don't follow
	
	if any other script window in the current stack is closed, we keep 
	going.
	
	6/20/92 dmb: search scriptsourcestack for the outline, not the window; 
	houtline is set in every frame & must be checked; pwindow may still be nil.
	
	2.1b1 dmb: make sure we stop recording as necessary

	5.0b11 dmb: don't call mescriptwindowclosed from here. the globals 
	aren't set.
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register short ix;
	register boolean flinmenubar;
	
	flinmenubar = scriptinmenubar ();
	
	if (scriptinstallable ()) {
		
		if (!scriptverifycompilation ())
			return (false);
		}
	
	for (ix = 0; ix < (**hd).topscriptsource; ix++) { /*see if it's any window in stack*/
		
		if ((**hd).scriptsourcestack [ix].houtline == outlinedata) { /*yup*/
			
			(**hd).scriptsourcestack [ix].pwindow = nil; /*clear it*/
			
			if (!(**outlinedata).fldirty) /*outline is about to be disposed*/
				(**hd).scriptsourcestack [ix].houtline = nil;
			
			if (ix == 0) { /*user is closing main source window*/
				
				if ((**hd).flscriptsuspended)
					scriptkillbutton ();
				else
					scriptgobutton ();
				
			#ifdef flcomponent
				if ((**hd).flrecording)
					scriptstoprecordbutton ();
			#endif
			
				(**hd).flwindowclosed = true;
				}
			}
		}
	
	if (flinmenubar)
		return (true);
	else
		return (opverbclose ());
	} /*scriptclose*/


static boolean scriptgetundoglobals (long *globals) {
	
	return (opeditgetundoglobals (globals));
	} /*scriptgetundoglobals*/


static boolean scriptsetundoglobals (long globals, boolean flundo) {
	
	return (opeditsetundoglobals (globals, flundo));
	} /*scriptsetundoglobals*/


static void scriptidle (void) {
	
	opidle ();
	} /*scriptidle*/


static boolean scriptadjustcursor (Point pt) {
	
	return (opsetcursor (pt));
	} /*scriptadjustcursor*/


static boolean scriptsetfont (void) {
	
	return (opsetfont ((**outlinewindowinfo).selectioninfo.fontnum));
	} /*scriptsetfont*/
	

static boolean scriptsetsize (void) {
	
	return (opsetsize ((**outlinewindowinfo).selectioninfo.fontsize));
	} /*scriptsetsize*/


static boolean scriptsetselectioninfo (void) {
	
	return (opsetselectioninfo ());
	} /*scriptsetselectioninfo*/


static boolean scriptsearch (void) {
	
	long refcon = (**outlinedata).outlinerefcon;
	
	startingtosearch (refcon);
	
	if (opflatfind (false, searchshouldwrap (refcon)))
		return (true);
	
	if (!searchshouldcontinue (refcon))
		return (false);
	
	if (scriptindatabase ()) /*lives in symbol table -- see scriptinstallable*/
		return (langexternalcontinuesearch ((hdlexternalvariable) refcon));
	
	if (scriptinmenubar ())
		return (mecontinuesearch ((**outlinewindowinfo).parentwindow, (hdlheadrecord) refcon));

	return (false);
	} /*scriptsearch*/


static boolean scriptruncursor (void) {
	
	return (opverbruncursor ());
	} /*scriptruncursor*/


static boolean scriptgetoutinesize (long *width, long *height) {
	
	short buttonswidth;
	
	opgetoutinesize (width, height);
	
	shellbuttongetoptimalwidth (&buttonswidth);
	
	*width = max (*width, buttonswidth);
	
	return (true);
	} /*scriptgetoutinesize*/


static boolean scriptkeystroke (void) {
	
	/*
	4/7/97 dmb: handle standalone scripts
	*/

	register hdloutlinerecord ho = outlinedata;
	
	if (scriptindatabase ()) { /*lives in symbol table -- see scriptinstallable*/
		
		if (langexternalsurfacekey ((hdlexternalvariable) (**ho).outlinerefcon))
			return (true);
		}

	if (scriptinmenubar ()) { /*lives in menubar*/
		
		#ifdef MACVERSION
		if (keyboardstatus.chkb == chenter)
			
			if (keyboardstatus.flcmdkey && keyboardstatus.flshiftkey) {
		#endif
		#ifdef WIN95VERSION
		if (keyboardstatus.chkb == chbackspace)
			
			if (keyboardstatus.flshiftkey) {
		#endif
				shellbringtofront ((**outlinewindowinfo).parentwindow);
				
				if (keyboardstatus.floptionkey)
					shellclosewindow (outlinewindow);
				
				return (true);
				}
		}
	
	return (opkeystroke ()); /*this operation may dirty the outlinerecord*/
	} /*scriptkeystroke*/


static boolean scripttitleclick (Point pt) {
	
	if (scriptindatabase ()) /*lives in symbol table -- see scriptinstallable*/
		return (langexternaltitleclick (pt, (hdlexternalvariable) (**outlinedata).outlinerefcon));
	
	if ((**outlinewindowinfo).parentwindow == nil)
		return (false);

	if (scriptinmenubar ()) { /*do same popup as parent window would*/
		
		if (cmdkeydown () || ismouserightclick()) {
		
			if (shellpushglobals ((**(**outlinewindowinfo).parentwindow).macwindow)) {
				
				hdlexternalvariable hv = (hdlexternalvariable) (**menudata).menurefcon;
				
				shellpopglobals ();
				
				pt.v += 50; /*hack: this signals special case*/
				
				tableclienttitlepopuphit (pt, hv);
				
				return (true);
				}
			}
		}
	
	if (!mousedoubleclick ())
		return (false);
	
	shellbringtofront ((**outlinewindowinfo).parentwindow);
	
	if (keyboardstatus.floptionkey)
		shellclosewindow (outlinewindow);
	
	return (true); /*consumed*/
	} /*scripttitleclick*/


#ifdef flcomponent

typedef boolean (*tyscriptvisitcallback) (OSType, bigstring, long);


static boolean scriptvisitservers (tyscriptvisitcallback visit, long refcon) {
	
	ComponentDescription desc, info;
	Component comp = nil;
	Handle hname;
	bigstring bsname;
	
	if (!havecomponentmanager ())
		return (false);
	
	if (!newemptyhandle (&hname))
		return (false);
	
	clearbytes (&desc, longsizeof (desc));
	
	desc.componentType = kOSAComponentType;
	
	desc.componentFlags = kOSASupportsCompiling;
	
	desc.componentFlagsMask = kOSASupportsCompiling;
	
	while (true) {
		
		comp = FindNextComponent (comp, &desc);
		
		if (comp == nil)
			break;
		
		if (GetComponentInfo (comp, &info, hname, nil, nil) != noErr)
			continue;
		
		if (info.componentSubType == kOSAGenericScriptingComponentSubtype)
			continue;
		
		copyheapstring ((hdlstring) hname, bsname);
		
		if (!(*visit) (info.componentSubType, bsname, refcon)) /*terminate visit*/
			break;
		}
	
	disposehandle (hname);
	
	return (comp == nil); /*true if all components visited*/
	} /*scriptvisitservers*/


static long *serverarray;


typedef struct tyfillservercallbackdata {
	
	hdlmenu hmenu;
	
	short checkeditem;
	
	long ctmenus;
	
	long signature;
	} tyfillservercallbackdata;


static boolean scriptfillservervisit (OSType subtype, bigstring bsname, long refcon) {
	
	tyfillservercallbackdata *cbd = (tyfillservercallbackdata *) refcon;

	if (!pushpopupitem ((*cbd).hmenu, bsname, true, 0)) /*terminate visit on error*/
		return (false);
	
	serverarray [(*cbd).ctmenus++] = subtype;
	
	if (subtype == (OSType)(*cbd).signature)
		(*cbd).checkeditem = (*cbd).ctmenus;
	
	return (true);
	} /*scriptfillservervisit*/


static boolean scriptserverpopupselect (hdlmenu hmenu, short itemselected) {
#pragma unused (hmenu)

	/*
	2.1b11 dmb: dirty the outline
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register OSType signature = serverarray [itemselected - 1];
	
	if ((**ho).outlinesignature != signature) {
		
		(**ho).outlinesignature = signature;
		
		opdirtyoutline ();
		}
	
	return (true);
	} /*scriptserverpopupselect*/


static boolean scriptfillserverpopup (hdlmenu hmenu, short *checkeditem) {
	
	tyfillservercallbackdata cbd;
	
	cbd.hmenu = hmenu;
	
	cbd.checkeditem = -1; /*default*/
	
	cbd.ctmenus = 0;
	
	cbd.signature = (**outlinedata).outlinesignature;
	
	if (!scriptvisitservers (&scriptfillservervisit, (long) &cbd))
		return (false);
	
	*checkeditem = cbd.checkeditem;
	
	return (true);
	} /*scriptfillserverpopup*/

#endif

#define flstacktrace 0

#if flstacktrace

static boolean scripterrorpopupselect (hdlmenu hmenu, short itemselected) {
	
	/*
	register hdlerrorstack hs = langerrorgetstack ();
	*/
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	register short ix = itemselected - 1;
	tyerrorrecord *pe;
	
	if (hs == nil)
		return (false);
	
	pe = &(**hs).stack [ix];
	
	return ((*(*pe).errorcallback) ((*pe).errorrefcon, 0, 0, nil, nil));
	} /*scripterrorpopupselect*/


static boolean scriptfillerrorpopup (hdlmenu hmenu, short *checkeditem) {
	
	/*
	register hdlerrorstack hs = langerrorgetstack ();
	*/
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	register short ix;
	register short ixtop;
	hdlhashtable htable;
	bigstring bsname;
	
	if (hs == nil)
		return (false);
	
	ixtop = (**hs).toperror;
	
	for (ix = 0; ix < ixtop; ++ix) {
		
		tyerrorrecord *pe = &(**hs).stack [ix];
		
		if ((*pe).errorcallback == nil)
			copystring ("\x02" "(-", bsname);
		else
			if (!(*(*pe).errorcallback) ((*pe).errorrefcon, 0, 0, &htable, bsname))
				return (false);
		
		if (!pushpopupitem (hmenu, bsname, true)) /*terminate visit on error*/
			return (false);
		}
	
	*checkeditem = ixtop;
	
	return (true);
	} /*scriptfillerrorpopup*/
#endif


static void scriptupdateserverpopup (void) {
	
	Rect r;
	bigstring bs;
	long signature;
	
	scriptgetpopuprect (&r);
	
	framerect (r);
	
	insetrect (&r, 1, 1);
	
	eraserect (r);
	
	#if flstacktrace
	//hs = langcallbacks.scripterrorstack; /*langerrorgetstack ();*/
	
	if (scriptinruntimestack () /*hs != nil*/ ) /*popup is stack, not language*/
		copystring ("\x06" "Stack:", bs);
	else {
	#endif
	
	signature = (**outlinedata).outlinesignature;
	
	if (!scriptgettypename (signature, bs)) { /*didn't find it*/
		
		if (signature == typeLAND)
			langgetstringlist (usertalkcomponentstring, bs);
		else
			langgetstringlist (unavailablestring, bs);
		}
	
	pushstyle (geneva, 9, 0);
	
	centerstring (r, bs);
	
	popstyle ();
	} /*scriptupdateserverpopup*/


static boolean scriptmousedown (Point pt, tyclickflags flags) {
	
	/*
	12/12/91 dmb: click in message area restores PC in debugger.  for now, 
	we don't bother making sure that the current script window is the one 
	that the PC is in, since we make sure that the node is safe before trying 
	to move to it.  a nice addition to this feature would be to bring the PC 
	window to the front and select the cursor.
	
	2.1b8 dmb: use opexpandto, not opmoveto when user clicks in message area.
	
	5.0b10 dmb: update server popup display after its clicked
	*/
	
	Rect rmsg;
	boolean flgeneva9 = false;
	
	#if TARGET_API_MAC_CARBON == 1
	
		flgeneva9 = true;
		
	#endif
	
	scriptgetpopuprect (&rmsg);
	
	if (pointinrect (pt, rmsg)) { /*click in message area*/
		
		register hdldebuggerrecord hd = debuggerdata;
		
		if ((**hd).flscriptsuspended) { /*we're in debugger waiting for input*/
			
			if (opnodeinoutline ((**hd).hbarcursor)) { /*very safely determined that node is valid*/
				
				opexpandto ((**hd).hbarcursor);
				
				opsettextmode (false);
				}
			}
		
		bundle {
			
		#if flstacktrace
			if (scriptinruntimestack ()) /*popup is of stack, not language*/
							
				popupmenuhit (rmsg, flgeneva9, &scriptfillerrorpopup, &scripterrorpopupselect);
				
			else {
		#endif
		
			#ifdef flcomponent
				serverarray = (long *) NewPtr (sizeof (long) * 100);
				
				if (popupmenuhit (rmsg, flgeneva9, &scriptfillserverpopup, &scriptserverpopupselect)) {
					
					scriptinvalbuttons ();
					
					scriptupdateserverpopup (); 
					}
				
				DisposePtr ((Ptr) serverarray);
			#endif
				}
		#if flstacktrace
			}
		#endif
		
		return (true);
		}
	
	opmousedown (pt, flags);
	
	return (true);
	} /*scriptmousedown*/
	

static boolean scriptcmdkeyfilter (char chkb) {

	return (opcmdkeyfilter (chkb)); /*this operation may dirty the outlinerecord*/
	} /*scriptcmdkeyfilter*/
	
	
static boolean scriptcut (void) {

	return (opcut ()); /*this operation may dirty the outlinerecord*/
	} /*scriptcut*/
	
	
static boolean scriptpaste (void) {

	return (oppaste ()); /*this operation may dirty the outlinerecord*/
	} /*scriptpaste*/
	
	
static boolean scriptclear (void) {

	return (opclear ()); /*this operation may dirty the outlinerecord*/
	} /*scriptclear*/


static boolean scriptgetvariable (hdlexternalvariable *hvariable) {
	
	/*
	if (!scriptindatabase ())
		return (false);
	*/
	
	return (opverbgetvariable (hvariable));
	} /*scriptgetvariable*/


static boolean scriptgettargetdata (short id) {
	
	/*
	don't allow verbs that are being executed by the debugger script being 
	debugged to act on itself
	
	12/13/91 dmb: updated to handle case where Run button has been hit.  we 
	still want to exclude ourselves from being the target
	*/
	
	/*
	if (debuggingcurrentprocess () && scriptinruntimestack())
		return (false);
	*/
	
	if (processisoneshot (false) && (currentprocess == (**debuggerdata).scriptprocess))
		if (scriptinruntimestack ())
			return (false);
	
	return (opverbgettargetdata (id));
	} /*scriptgettargetdata*/


boolean scriptbackgroundtask (boolean flresting) {
	
	/*
	1/22/91 dmb: a new approach to dealing with massive context changes in 
	background tasks.  we try to save and restore context globals (via 
	langcallbacks) around the backgrounding calls.  so even if another 
	file is opened or brought to the front, the current process will retain 
	its context.  if the context can't be restored (i.e. the file was closed), 
	we return false and the execution thread will return quickly.

	5.1.5 dmb: yield if a new thread is waiting
	*/
	
	boolean fldialogrunning;
	boolean fl = true;
	
	if (scriptbackgroundenabled () && (flresting || processtimesliceelapsed () || (threadiswaiting () && processthreadcount () < 7))) {
		
		fldialogrunning = langdialogrunning ();
		
		if (fldialogrunning) /*modal dialogs need to get all events*/
			shellblockevents ();
		
		flscriptresting = flresting;
		
		fl = processyield ();
		
		flscriptresting = false;
		
		if (fldialogrunning)
			shellpopevents ();
		
	//	scriptresetbackgroundtime ();
		}
	
	return (fl);
	} /*scriptbackgroundtask*/


static boolean scriptresetrects (hdlwindowinfo hinfo) {
	
	register hdlwindowinfo hw = hinfo;
	Rect rmsg;
	short width;
	
	shellresetwindowrects (hw);
	
	rmsg = (**hw).messagerect;
	
	width = rmsg.right - rmsg.left;
	
	if (width > 100)
		/*
		(**hw).messagerect.left += 72;
		*/
		(**hw).messagerect.right -= 72;
		
	return (true);
	} /*scriptresetrects*/


typedef struct tyfindservercallbackdata {
	
	ptrstring pname;
	
	long signature;
	} tyfindservercallbackdata;


static boolean scriptfindsubtypevisit (OSType subtype, bigstring bsname, long refcon) {
	
	tyfindservercallbackdata *cbd = (tyfindservercallbackdata *) refcon;
	
	if (subtype == (OSType) (*cbd).signature) {
		
		copystring (bsname, (*cbd).pname);
		
		return (false); /*terminate visit*/
		}
	
	return (true); /*keep visiting*/
	} /*scriptfindsubtypevisit*/


boolean scriptgettypename (long signature, bigstring bsname) {
	
	/*
	2006-02-22 creedon: if flcomponent is false then fallback to hard coded
				values, useful on Windows

	visit all OSA servers and find the name of the one who's subtype 
	matches the given signature
	*/
	
	tyfindservercallbackdata cbd;
	
	setemptystring (bsname);
	
	cbd.pname = bsname;
	
	cbd.signature = signature;
	
#ifdef flcomponent
	scriptvisitservers (&scriptfindsubtypevisit, (long) &cbd);
#else
	switch (signature) {
		case typeLAND:
			copystring ("\x08" "UserTalk", bsname);
			break;

		case 'ascr':
			copystring ("\x0B" "AppleScript", bsname);
			break;
		} /* switch */

	
#endif
	
	return (!isemptystring (bsname));
	} /*scriptgettypename*/


static boolean scriptfindnamevisit (OSType subtype, bigstring bsname, long refcon) {
	
	tyfindservercallbackdata *cbd = (tyfindservercallbackdata *) refcon;
	
	if (equalidentifiers (bsname, (*cbd).pname)) {
		
		(*cbd).signature = subtype;
		
		return (false); /*terminate visit*/
		}
	
	return (true); /*keep visiting*/
	} /*scriptfindnamevisit*/


boolean scriptgetnametype (bigstring bsname, long *signature) {
	
	/*
	2006-02-22 creedon: if flcomponent is false then fallback to hard coded
				values, useful on Windows

	visit all OSA servers and find the subtype of the one who's name
	matches bsname
	*/
	
#ifdef flcomponent

	tyfindservercallbackdata cbd;
	
	cbd.pname = bsname;
	
	cbd.signature = 0;
	
	scriptvisitservers (&scriptfindnamevisit, (long) &cbd);
	
	*signature = cbd.signature;
	
	return (cbd.signature != 0);
#else
	if (comparestrings (bsname, "\x08" "UserTalk") == 0) {
		*signature = typeLAND;
		return (true);
	}
	else if (comparestrings (bsname, "\x0B" "AppleScript") == 0) {
		*signature = 'ascr';
		return (true);
	}
	else
		return (false);
#endif
	} /*scriptgetnametype*/


static void scriptupdate (void) {

	/*
	7.0b26 PBS: update the script language popup on Macs only -- since
	it doesn't exist on Windows. This was the site of a display glitch on Windows.
	*/

	#ifdef MACVERSION
	
		scriptupdateserverpopup ();
	
	#endif
	
	opupdate ();
	} /*scriptupdate*/


static boolean scriptdirtyhook (void) {
	
	/*
	this is where we hook into events that dirty things.  if a langerror 
	window is open, we gray out its script button.  the theory is that we
	can't handle the world changing **that** much, and as soon as the user
	does some editing or cursor moving, we don't want to take any chances
	on the callback routine and the data linked into the langerror window.
	
	7/20/90 DW: it's important to keep the work to an absolute minimum 
	when the window is open because the system really bogs down if you
	constantly update the script icon in the langerror window.
	*/
	
	register hdloutlinerecord ho = outlinedata;

	if ((ho != nil) && (outlinewindow == shellwindow))
		if (scriptinstallable ())
			if (!(**ho).flrecentlychanged) {
				
				(**ho).flrecentlychanged = true; // set it ourselves now

				shellinvalbuttons (); // calls us immediately to get status
				}
	
	return (true);
	} /*scriptdirtyhook*/


#if 0

static boolean opwinnewrecord (void) {
	
	tyvaluerecord val;
	hdlexternalvariable hv;
	hdloutlinerecord ho;
	
	if (!langexternalnewvalue (idscriptprocessor, nil, &val))
		return (false);
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	(**outlinewindowinfo).hdata = (Handle) ho;
	
	(**ho).outlinetype = outlineisstandalonescript;

	(**ho).flwindowopen = true;
	
	(**ho).fldirty = true; /*needs to be saved*/
	
	scriptsetcallbacks (ho);
	
	return (true);
	} /*opwinnewrecord*/


static boolean opwindisposerecord (void) {

	hdloutlinerecord ho = outlinedata; /*copy into local*/
	
	if ((ho != NULL) && ((**ho).outlinetype == outlineisstandalonescript)) {
		
		opdisposeoutline (ho, true);
		
		opsetoutline (nil);
		
		(**outlinewindowinfo).hdata = nil;
		}

	return true;
	} /*opwindisposerecord*/

	
static boolean opwinloadfile (hdlfilenum fnum, short rnum) {
	
	Handle hpackedop;
	hdloutlinerecord ho;
	boolean fl;
	long ixload = 0;
	
	if (!filereadhandle (fnum, &hpackedop))
		return (false);
	
	fl = opunpack (hpackedop, &ixload, &ho);
	
	disposehandle (hpackedop);
	
	if (!fl)
		return (false);
	
	(**outlinewindowinfo).hdata = (Handle) ho;
	
	(**ho).outlinetype = outlineisstandalonescript;

	(**ho).flwindowopen = true;
	
	(**ho).fldirty = true; /*never been saved, can't be clean*/
	
	scriptsetcallbacks (ho);
	
	return (true);
	} /*opwinloadfile*/


static boolean opwinsavefile (hdlfilenum fnum, short rnum, boolean flsaveas) {
	
	Handle hpackedop = nil;
	boolean fl;
	
	if (!oppack (&hpackedop))
		return (false);
	
	fl = 
		fileseteof (fnum, 0) &&
		
		filesetposition (fnum, 0) &&
		
		filewritehandle (fnum, hpackedop);
	
	disposehandle (hpackedop);
	
	return (fl);
	} /*opwinsavefile*/

#endif


boolean scriptstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's  
	data structure.
	*/
	
	ptrcallbacks scriptcallbacks;
	register ptrcallbacks cb;
	
	shellpushscraphook (&scriptscraphook);
	
	shellpushdirtyhook (&scriptdirtyhook);
	
	langcallbacks.scriptcompilecallback = &scriptcompiler;
	
	shellnewcallbacks (&scriptcallbacks);
	
	cb = scriptcallbacks; /*copy into register*/
	
	loadconfigresource (idscriptconfig, &(*cb).config);
	
	(*cb).config.flnewonlaunch = true; // *** need to update resource

	(*cb).configresnum = idscriptconfig;
	
	(*cb).windowholder = &outlinewindow;
	
	(*cb).dataholder = (Handle *) &scriptdataholder;
	
	(*cb).infoholder = &outlinewindowinfo;
	
	(*cb).setglobalsroutine = &scriptsetglobals;
	
	(*cb).pushroutine = &oppushglobals;

	(*cb).poproutine = &oppopglobals;
	
#ifdef version42orgreater
	
	(*cb).saveroutine = ccsavespecialfile;
	
	(*cb).disposerecordroutine = ccdisposefilerecord;
	
	/*
	(*cb).newrecordroutine = opwinnewrecord;
	
	(*cb).disposerecordroutine = opwindisposerecord;
	
	(*cb).loadroutine = opwinloadfile;
	
	(*cb).backgroundroutine = &ccbackground;

	(*cb).childcloseroutine = &ccchildclose;
	*/

#endif
	
	(*cb).resetrectsroutine = &scriptresetrects;
	
	(*cb).updateroutine = &scriptupdate;
	
	(*cb).activateroutine = &opactivate;
	
	(*cb).getcontentsizeroutine = &scriptgetoutinesize;
	
	(*cb).resizeroutine = &scriptresize;
	
	(*cb).scrollroutine = &opscroll;
	
	(*cb).setscrollbarroutine = &opresetscrollbars;
	
	(*cb).buttonroutine = &scriptbutton;
	
	(*cb).buttonstatusroutine = &scriptbuttonstatus;
	
	(*cb).mouseroutine = &scriptmousedown;

	(*cb).rmouseroutine = &oprmousedown; /*7.0b13 PBS: right-click in scripts*/
	
	(*cb).keystrokeroutine = &scriptkeystroke;
	
	(*cb).titleclickroutine = &scripttitleclick;
	
	(*cb).cmdkeyfilterroutine = &scriptcmdkeyfilter;
	
	(*cb).cutroutine = &scriptcut;
	
	(*cb).copyroutine = &opcopy; /*sic -- no need for a script layer here... yet*/
	
	(*cb).pasteroutine = &scriptpaste;

	(*cb).clearroutine = &scriptclear;
	
	(*cb).selectallroutine = &opselectall;
	
	(*cb).closeroutine = &scriptclose;
	
	(*cb).getundoglobalsroutine = &scriptgetundoglobals;
	
	(*cb).setundoglobalsroutine = &scriptsetundoglobals;
	
	(*cb).idleroutine = &scriptidle;
	
	(*cb).adjustcursorroutine = &scriptadjustcursor;
	
	(*cb).fontroutine = &scriptsetfont;
	
	(*cb).sizeroutine = &scriptsetsize;
	
	(*cb).setselectioninforoutine = &scriptsetselectioninfo;
	
	(*cb).searchroutine = &scriptsearch;
	
	(*cb).executeroutine = &scriptruncursor;
	
	(*cb).gettargetdataroutine = &scriptgettargetdata;
	
	(*cb).getvariableroutine = (shellgetvariablecallback) &scriptgetvariable;
	
	(*cb).settextmoderoutine = &opsettextmode;
	
	(*cb).setprintinfoproutine = &opsetprintinfo;
	
	(*cb).printroutine = &opprint;
	
	(*cb).beginprintroutine = &opbeginprint;
	
	(*cb).endprintroutine = &opendprint;
	
	return (true);
	} /*scriptstart*/


boolean initscripts (void) {
	
	if (!newclearhandle (longsizeof (tydebuggerrecord), (Handle *) &debuggerdata)) /*all fields are cool at 0*/
		return (false);
	
#ifdef flcomponent

	#if TARGET_API_MAC_CARBON == 1
	
		AEInstallEventHandler (kOSASuite, kOSARecordedText, NewAEEventHandlerUPP (handlerecordedtext), 0, false);
		
		AEInstallEventHandler ('ToyS', kAENotifyStopRecording, NewAEEventHandlerUPP (handlestoprecording), 0, false);
	
	#else

		AEInstallEventHandler (kOSASuite, kOSARecordedText, NewAEEventHandlerProc (handlerecordedtext), 0, false);
		
		AEInstallEventHandler ('ToyS', kAENotifyStopRecording, NewAEEventHandlerProc (handlestoprecording), 0, false);
	
	#endif
	
#endif
	
	return (true);
	} /*initscripts*/




