
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
#include "bitmaps.h"
#include "cursor.h"
#include "file.h"
#include "font.h"
#include "icon.h"
#include "kb.h"
#include "mouse.h"
#include "ops.h"
#include "popup.h"
#include "quickdraw.h"
#include "search.h"
#include "smallicon.h"
#include "sounds.h"
#include "frontier_strings.h"
#include "textedit.h"
#include "frontierwindows.h"
#include "windowlayout.h"
#include "zoom.h"
#include "shell.h"
#include "shellprivate.h"
#include "shellbuttons.h"
#include "cancoon.h"
#include "cancooninternal.h"
#include "tableformats.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "command.h"

/*
implements the "home" window in CanCoon -- the topmost window, the one with a flag to
lift the hood, and buttons that zoom you to neat places.
*/



static boolean ccrun (short idscript) {
	
	bigstring bsscript;
	bigstring bsresult;
	
	if (!getsystemtablescript (idscript, bsscript)) {
		
		sysbeep ();
		
		return (false);
		}
	
	return (langrunstringnoerror (bsscript, bsresult));
	} /*ccrun*/


boolean cchelpcommand (void) {
	
	return (ccrun (idtechsupportscript));
	} /*cchelpcommand*/


boolean ccflipflag (void) {
	
	return (false);
	} /*ccflipflag*/

/*
	ccwindowsetglobals is the site of a crashing bug.

	http://linux2.userland.com/users/admin/static/drWatsonLogs/SUPERHONKER/24593835.txt
	http://linux2.userland.com/users/admin/static/drWatsonLogs/SUPERHONKER/24629842.txt
	http://linux2.userland.com/users/admin/static/drWatsonLogs/SUPERHONKER/24817036.txt

	function: ccwindowstart (actually: ccwindowsetglobals)
        00405d00 7459             jz      ccwindowstart+0x1e9 (00405d5b)

		00405d02 8b4dfc           mov     ecx,[ebp-0x4]          ss:011785be=????????
        00405d05 8b11             mov     edx,[ecx]              ds:022df948=022df950
        00405d07 83baf600000000   cmp   dword ptr [edx+0xf6],0x0 ds:95e000f6=00f6ba83
        00405d0e 744b             jz      ccwindowstart+0x1e9 (00405d5b)
        
		00405d10 8b45fc           mov     eax,[ebp-0x4]          ss:011785be=????????
        00405d13 8b08             mov     ecx,[eax]              ds:95e00000=????????
        00405d15 8b91f6000000     mov     edx,[ecx+0xf6]         ds:022dfa3e=95e00000
        00405d1b 891524405300   mov [cancoondata (00534024)],edx ds:00534024=95e00000

        00405d21 a124405300     mov eax,[cancoondata (00534024)] ds:00534024=95e00000
FAULT ->00405d26 8b08             mov     ecx,[eax]              ds:95e00000=518b088b
        00405d28 8b5104           mov     edx,[ecx+0x4]          ds:0332e34e=????????
        00405d2b 8b02             mov     eax,[edx]              ds:95e00000=????????
        00405d2d 8b0d9c595300                                    ds:0053599c=022df8e0
                                  mov     ecx,[tableformatsdata (0053599c)]
        00405d33 3b4842           cmp     ecx,[eax+0x42]         ds:96e4ea06=7542483b
        00405d36 7509             jnz     ccwindowstart+0x1cf (00405d41)

        00405d38 c745f800000000   mov    dword ptr [ebp-0x8],0x0 ss:011785be=????????
        00405d3f eb1a             jmp     ccwindowstart+0x1e9 (00405d5b)

		00405d41 6a52             push    0x52
        00405d43 68ac735200       push    0x5273ac
        00405d48 68dc735200       push    0x5273dc
        00405d4d e82ecfffff       call    __assert (00402c80)


		The crash occurs when we dereference (*cancoondata) in the assert.


		Theory 1: cancoondata was disposed somewhere but not set to nil.

		Theory 2: cancoondata was set to an uninitialized value.


		Note: cancoondata = (**tableformatsdata).refcon

		(**shellwindowinfo).hdata == tableformatsdata (?)

		cancooncallbacks.dataholder = &tableformatsdata

		tablecallbacks.dataholder = &tableformatsdata

		See tablepushformats, tablepopformats in tableformats.c:
			only used in tablepacktable in tablepack.c [not a likely candiate]
		
		See tablenewformatsrecord in tableformats.c
			--> tableverbsetupdisplay in tableexternal.c
				--> ccwindowsetuptableview in cancoonwindow.c [cancoondata is linked to (**hf).refcon]
				--> tableedit in tableexternal.c [site of cmd-enter-period crash on Mac!!!]
*/

static boolean ccwindowsetglobals (void) {
	
	register hdltableformats hf = tableformatsdata;
	
	if (hf && (**hf).refcon) { // cancoon record has been linked
	
		cancoondata = (hdlcancoonrecord) (**hf).refcon;
		
		assert (tableformatsdata == (**(**cancoondata).hroottable).hashtableformats);
		}
	
	tableformatswindow = cancoonwindow;
	
	tableformatswindowinfo = cancoonwindowinfo;
	
	return (tableverbsetglobals ());
	
	} /*ccwindowsetglobals*/


void ccwindowsetup (boolean flbigwindow, boolean floldversion) {
#pragma unused (flbigwindow)

	/*
	called when cc is booting, the window hasn't shown up on the screen yet,
	we get a chance to change the size of the window.  we don't remember whether
	the flag was up or down, the window always comes up with the flag up.
	
	1/21/91 dmb: routine now works for new files.  someone didn't do enough 
	testing...
	
	9/10/91 dmb: don't reset the height of the window; we save the state of 
	flbigwindow now.
	
	12/20/91 dmb: to support new files, we set the bottom again, but now take 
	flbigwindow parameter
	*/
	
	(**cancoondata).fldirty = floldversion;
	} /*ccwindowsetup*/


boolean cctoggleflag (void) {
	
	register hdlcancoonrecord hc = cancoondata;
	
	(**hc).flflagdisabled = !(**hc).flflagdisabled;
	
	windowinval (cancoonwindow);
	
	return (true);
	} /*cctoggleflag*/


boolean cctoggleagentspopup (void) {
	
	register hdlcancoonrecord hc = cancoondata;
	
	(**hc).flpopupdisabled = !(**hc).flpopupdisabled;
	
	windowinval (cancoonwindow);
	
	return (true);
	} /*cctoggleagentspopup*/


static boolean ccwindowpreclose (WindowPtr w) {
	
	/*
	5.0a23 dmb: if we're a conditions are right, just hide the root window
	
	6.2b16 AR: No longer treat guest roots any different from the main root
	*/
	
	if (!keyboardstatus.flshiftkey	// shift disables root hiding
		&& (w == cancoonwindow) 		// a root window
		&& !flshellclosingall			// not quitting
		/*&& !(**cancoondata).flguestroot*/) {	// not a guest root
		
		assert (w == shellwindow);
		
		opcloseoutline (); //prepare for dormancy, not in a window anymore
		
		shellhidewindow (shellwindowinfo);
		
		return (false);
		}
	
	return (ccpreclose (w));
	} /*ccpreclose*/


static boolean ccwindowclose (void) {
	
	/*
	5.0d16 dmb: if we're a beginner and Frontier isn't quitting, 
	just hide the window
	*/
	
	if (!tableverbclose ())
		return (false);
	
	return (ccclose ());
	} /*ccwindowclose*/


static boolean ccwindowsetuptableview (void) {
	
	/*
	in order to function correctly as a table window, the window info record 
	must be linked to a tableformats record, not a cancoon record. we're called 
	after initially loading a cancoon record. we set up the table formats and 
	make the cancoon record hang off of it (as well as vice-versa), so the 
	tableformatsrecord can be the master, windowinfo-wise

	5.0a5 dmb: move and reize the window according to the tableformats windowrect
	(the Mac version was relying on the shell's window resource, which we should 
	omit now, since it's not xplat. must save and restore colwidths, since the 
	window will be resized semi-randomly before the correct size.
	
	5.0a20: finishing touches (?) of guest databases, make them children of the host
	*/
	
	register hdlcancoonrecord hc = cancoondata;
	hdlhashtable ht;
	hdltableformats hf;
	short colwidths [maxtablecols];
	
	ht = (**hc).hroottable;

	hf = (**ht).hashtableformats;
	
	if (hf == nil)
		colwidths [0] = 0;
	else
		moveleft ((**hf).colwidths, colwidths, maxtablecols * sizeof (short));
	
	tableverbsetupdisplay (ht, cancoonwindowinfo);
	
	if (hf == nil) // just created
		hf = (**ht).hashtableformats;
	else
		moveleft (colwidths, (**hf).colwidths, maxtablecols * sizeof (short));
	
	(**hf).refcon = (long) hc; /*link cancoonglobals to tableformats*/
	
	/*
	(**cancoonwindowinfo).hdata = (Handle) hf; //link tableformats to windowinfo
	
	shellsetglobals (cancoonwindow);
	
	tableresetformatsrects ();
	*/
	
	(**outlinedata).flwindowopen = true;
	
	(**ht).flwindowopen = true;
	
	shellmoveandsizewindow (cancoonwindowinfo, (**hf).windowrect);

	if ((**hc).fldirty)
		shellzoomwindow (cancoonwindowinfo, true);
	
	if ((**hc).flguestroot) {
		
		/*
		hdlwindowinfo hroot;
		
		if (!ccfindrootwindow (&hroot))
			return (false);
		
		(**cancoonwindowinfo).parentwindow = hroot;
		*/
		
		langexternalregisterwindow ((hdlexternalvariable) (**hc).hrootvariable);
		}
	
	return (true);
	} /*ccwindowsetuptableview*/


static boolean ccwindowloadfile (hdlfilenum fnum, short rnum) {
	
	if (!ccloadfile (fnum, rnum))
		return (false);
	
	return (ccwindowsetuptableview ());
	} /*ccwindowloadfile*/


static boolean ccwindownewrecord (void) {
	
	if (!ccnewrecord ())
		return (false);
	
	return (ccwindowsetuptableview ());
	} /*ccwindownewrecord*/


static boolean ccwindowdisposerecord (void) {
	
	/*
	5.1.5 dmb: call new purgefrommenubarlist to get rid of any installed menus
	*/
	register hdlcancoonrecord hc = cancoondata;

	if (hc == NULL)
		return (true);
	
	if ((**hc).flguestroot) {
		
		langexternalunregisterwindow (cancoonwindowinfo);
		
		purgefrommenubarlist ((long) (**hc).hdatabase);
		}

	ccdisposerecord ();

	//shellexitmaineventloop ();

	return (true);
	} /*ccwindowdisposerecord*/


static boolean findrootvisit (WindowPtr w, ptrvoid refcon) {
	
	hdlwindowinfo hinfo;
	
	if (getwindowinfo (w, &hinfo)) {
		
		hdltableformats hf = (hdltableformats) (**hinfo).hdata;
		
		if (hf && ((**hf).refcon == (long) cancoonglobals)) { // got it

			WindowPtr *pw = (WindowPtr *) refcon;
		
			*pw = w;
			
			return (false);
			}
		}
	
	return (true); // keep visiting
	} /*findrootvisit*/


boolean ccfindrootwindow (hdlwindowinfo *hinfo) {
	
	WindowPtr w;
	
	if (hinfo == nil) // a convenience for our callers, handle nil for don't care
		return (true);
	
	*hinfo = nil;
	
	if (shellvisittypedwindows (idcancoonconfig, findrootvisit, &w))
		return (false);
	
	return (getwindowinfo (w, hinfo));
	} /*ccfindrootwindow*/


hdldatabaserecord ccwindowgetdatabase (WindowPtr w) {
	
	/*
	5.1.5b15 dmb: a useful piece of abstraction, used for updating in tableexternal.c
	*/

	hdlwindowinfo hinfo;
	hdltableformats hf;

	if (getwindowinfo (w, &hinfo)) {
		
		hf = (hdltableformats) (**hinfo).hdata;
		
		if (hf != nil)
			return (tablegetdatabase ((**hf).htable));
		}

	return (nil);
	} /*ccwindowgetdatabase*/


boolean ccwindowstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks cancooncallbacks;
	register ptrcallbacks cb;
	short ixtableconfig;
	
	ccinitverbs ();
	
	/*
	langcallbacks.msgverbcallback = (callback) &ccmsg;
	*/
	
	langcallbacks.codereplacedcallback = &cccodereplaced;
	
	shellnewcallbacks (&cancooncallbacks);
	
	cb = cancooncallbacks; /*copy into register*/
	
	if (!shellfindcallbacks (idtableconfig, &ixtableconfig))
		shellerrormessage (BIGSTRING ("\x24" "must initialize table before cancoon"));
	 
	*cb = globalsarray [ixtableconfig]; // start with table callbacks
	
	loadconfigresource (idcancoonconfig, &(*cb).config);
	
	(*cb).configresnum = idcancoonconfig;
	
	(*cb).windowholder = &cancoonwindow;
	
	(*cb).dataholder = (Handle *) &tableformatsdata;
	
	(*cb).infoholder = &cancoonwindowinfo;
	
	(*cb).loadroutine = &ccwindowloadfile;
	
	(*cb).loadspecialroutine = &ccloadspecialfile;
	
	(*cb).saveroutine = &ccsavefile;
	
	(*cb).newrecordroutine = &ccwindownewrecord;
	
	(*cb).disposerecordroutine = &ccwindowdisposerecord;
	
	(*cb).setdatabaseroutine = &ccsetdatabase;
	
	(*cb).getdatabaseroutine = &ccgetdatabase;
	
	(*cb).setsuperglobalsroutine = &ccsetsuperglobals;
	
	(*cb).backgroundroutine = &ccbackground;
	
	(*cb).fnumchangedroutine = &ccfnumchanged;
	
	(*cb).findusedblocksroutine = &ccfindusedblocks;
	
	(*cb).precloseroutine = &ccwindowpreclose;
	
	(*cb).closeroutine = &ccwindowclose;
	
	(*cb).childcloseroutine = &ccchildclose;
		
	(*cb).setglobalsroutine = &ccwindowsetglobals;
	
	(*cb).pushroutine = &oppushglobals;

	(*cb).poproutine = &oppopglobals;
	
	/*
	(*cb).getvariableroutine = &tablegetvariableroutine;
	
	(*cb).updateroutine = &tableverbupdate;
	
	(*cb).activateroutine = &tableverbactivate;
	
	(*cb).getcontentsizeroutine = &tableverbgetcontentsize;
	
	(*cb).resetrectsroutine = &tableverbresetrects;
	
	(*cb).resizeroutine = &tableverbresize;
	
	(*cb).scrollroutine = &opscroll;
	
	(*cb).setscrollbarroutine = &opresetscrollbars;
	
	(*cb).mouseroutine = &tableverbmousedown;
	
	(*cb).keystrokeroutine = &tableverbkeystroke;
	
	(*cb).titleclickroutine = &tableverbtitleclick;
	
	(*cb).cutroutine = &opcut;
	
	(*cb).copyroutine = &opcopy;
	
	(*cb).pasteroutine = &tableverbpaste;
	
	(*cb).clearroutine = &opclear;
	
	(*cb).selectallroutine = &opselectall;
	
	(*cb).getundoglobalsroutine = &tableeditgetundoglobals;
	
	(*cb).setundoglobalsroutine = &tableeditsetundoglobals;
	
	(*cb).idleroutine = &tableverbidle;
	
	(*cb).adjustcursorroutine = &tableverbadjustcursor;
	
	(*cb).gettargetdataroutine = &tableverbgettargetdata;
	
	(*cb).settextmoderoutine = &opsettextmode;
	
	(*cb).fontroutine = &tableverbsetfont;
	
	(*cb).sizeroutine = &tableverbsetsize;
	
	(*cb).setselectioninforoutine = &tablesetselectioninfo;
	
	(*cb).searchroutine = &tableverbsearch;
	
	(*cb).executeroutine = &tableverbruncursor;
	
	(*cb).setprintinfoproutine = &opsetprintinfo;
	
	(*cb).printroutine = &opprint;
	*/
	
	return (true);
	} /*ccwindowstart*/


