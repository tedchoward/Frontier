
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

/*
about.c -- a generic "about" box for any Macintosh program written in Lightspeed C.

Make sure you have a STR# resource number 128, with exactly six strings in it.

Not too big, kind of sexy, and certainly better than nothing!
*/ 

#include "frontier.h"
#include "standard.h"

#include "versions.h" /*9.1b3 JES*/
#include "quickdraw.h"
#include "strings.h"
#include "bitmaps.h"
#include "cursor.h"
#include "dialogs.h"
#include "icon.h"
#include "file.h"
#include "font.h"
#include "kb.h"
#include "memory.h"
#include "mouse.h"
#include "ops.h"
#include "popup.h"
#include "resources.h"
#include "scrollbar.h"
#include "smallicon.h"
#include "textedit.h"
#include "frontierwindows.h"
#include "windowlayout.h"
#include "zoom.h"
#include "shell.h"
#include "shellprivate.h"
#include "shellhooks.h"
#include "about.h"
#include "tablestructure.h"
#include "cancoon.h"
#include "cancooninternal.h"
#include "process.h"
#include "processinternal.h"
#include "launch.h"

#ifdef WIN95VERSION
	#include "Winland.h"
	#define idfrontiericon	IDB_FRONTIER_BITMAP
#endif

#ifdef MACVERSION
	#define idfrontiericon	128
#endif



static long aboutopenticks; /*so we can tell how long it's been up*/


typedef struct tyaboutrecord {
	
	Rect messagearea;
	
	Rect aboutarea;
	
	boolean flbootsplash;
	
	boolean flbigwindow;
	
	boolean flextrastats;

	long refcon;
	} tyaboutrecord, *ptraboutrecord, **hdlaboutrecord;


static hdlaboutrecord aboutdata = nil;

static WindowPtr aboutwindow = nil;

static hdlwindowinfo aboutwindowinfo;

static boolean flsessionstats = false;


static hdlaboutrecord displayedaboutdata = nil;

static CGrafPtr aboutport = nil;

static boolean flhavemiscrect = false;

static Rect miscinforect;


static bigstring bstheadinfo = "";

static bigstring bsmiscinfo = "";


#define agentpopupwidth (popuparrowwidth + 4)

#define msgtopinset 3

#define msgbottominset 3

#define minmsgheight (heightsmallicon + 4)

#define msgborderpix 5

#define msgvertgap 4

#define aboutlineheight 14

#define aboutlinewidth 300

#define aboutvertstart 0

#define aboutvertinset 6

#define aboutvertgap 0

#define abouthorizgap 20

#define abouthorizinset 12

#define aboutrowsStats 8

#define aboutrectheightStats (aboutvertinset * 2 + aboutvertstart + aboutrowsStats * aboutlineheight + aboutvertgap)

#define aboutrowsNoStats 3

#define aboutrectheightNoStats (aboutvertinset * 2 + aboutvertstart + aboutrowsNoStats * aboutlineheight + aboutvertgap)

#define abouticonsize 32

#define versionwidth 56

#define minaboutwidth  (aboutlinewidth + abouthorizgap + 2 * abouthorizinset)

#define agentmenuhorizgap 10

static byte * aboutstrings [] = {

#ifdef PIKE
	"\x2b" "The power of Web publishing on your desktop", /*7.0b1 PBS*/
#else
	"\x2e" "Powerful cross-platform web content management", /*6.1 AR*/
#endif
		
    /*"\x25" "Powerful cross-platform web scripting",*/
	
	"\x23" "© 1992-" copyright_year_string " UserLand Software, Inc.",

#ifdef PIKE
	"\x1a" "http://radio.userland.com/", /*7.0d8 PBS*/
#else
	"\x1d" "http://frontier.userland.com/",
#endif
	
	"\x02" "^0",
	
	"\x12" "Scripts Running:  ",
	
	"\x11" "Current Thread:  ",
	
	#ifdef MACVERSION
		"\x13" "Available Memory:  ",
	#endif
	#ifdef WIN95VERSION
		"\x14" "Handles Allocated:  ",
	#endif

	"",

	"\x10" "Visible Agent:  ",
	
	"\x0f" "Current Time:  ",
	
#ifdef PIKE
	#ifdef MACVERSION
		"\x0f" "Radio UserLand\xaa", /*7.0d8 PBS*/
	#endif
	#ifdef WIN95VERSION
		"\x0f" "Radio UserLand\x99", /*7.0d8 PBS*/
	#endif
	
	"\x14" "About Radio UserLand", /*7.0d8 PBS*/
#else
	#ifdef MACVERSION
		"\x12" "UserLand Frontier\xaa",
	#endif
	#ifdef WIN95VERSION
		"\x12" "UserLand Frontier\x99",
	#endif
	
	"\x0e" "About Frontier",
#endif
	
	"\x02" "^0",
	};


enum { /*main window items*/
	
	sloganitem,
	
	copyrightitem,
	
	urlitem,
	
	isaitem,
	
	threadsitem,
	
	thisthreaditem,
	
	memoryitem,
	
	miscinfoitem,

	agentpopupitem,
	
	datetimeitem,
	
	frontieritem,
	
	titleitem,
	
	versionitem
	};


void aboutsegment (void) {
	
	} /*aboutsegment*/


static boolean findaboutwindow (hdlwindowinfo *hinfo) {

	WindowPtr w;
	Handle hdata;
	
	return (shellfindwindow (idaboutconfig, &w, hinfo, &hdata));
	} /*findaboutwindow*/


boolean aboutstatsshowing (void) {
	
	return (displayedaboutdata && (**displayedaboutdata).flbigwindow && (**displayedaboutdata).flextrastats);
	} /*aboutstatsshowing*/


static long aboutrectheight () {
	if (aboutstatsshowing())
		return (aboutrectheightStats);

	return(aboutrectheightNoStats);
	} /*aboutrectheight*/


static void getmessagecontentrect (Rect *rcontent) {

	if (aboutdata == nil)
		zerorect (rcontent);
	else
		*rcontent = (**aboutdata).messagearea;
	} /*getmessagecontentrect*/


static void ccdrawfrontiericon (Rect rcicn, boolean flpressed) {
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 7.0b53: draw an OS X style icon*/
	
		IconRef iconref;
		tyfilespec programfspec;
		short label;
		OSErr ec;
		Rect r = rcicn;
		
		getapplicationfilespec (nil, &programfspec);

		if (GetIconRefFromFile (&programfspec, &iconref, &label) == noErr) {
			
			ec = PlotIconRef (&r, atVerticalCenter + atHorizontalCenter, flpressed? kTransformSelected : 0, kIconServicesNormalUsageFlag, iconref);			
			
			ReleaseIconRef (iconref);
			} /*if*/
		
		if (ec == noErr)
			return;
	#endif
	
	ploticonresource (&rcicn, atVerticalCenter + atHorizontalCenter, flpressed? kTransformSelected : 0, idfrontiericon);
	
	} /*ccdrawfrontiericon*/


static boolean ccgettextitemrect (short item, Rect *r) {
	
	short linewidth = aboutlinewidth;
	
	if (item == versionitem) {
		
		*r = (**aboutwindowinfo).contentrect;
		
	//	insetrect (r, msgborderpix, msgborderpix);
		
		(*r).right -= msgborderpix + getscrollbarwidth (); // leave room for grow icon
		
		(*r).top = (*r).bottom - aboutlineheight;
		
		return (true);
		}
	
	*r = (**aboutdata).aboutarea;
	
	linewidth = min (linewidth, ((*r).right - (*r).left - abouthorizgap) / 2);
	
	(*r).top += aboutvertstart + item * aboutlineheight;
	
	/*if (false && item > urlitem) {
	
		if (odd (item))
			(*r).left = (*r).right - aboutlinewidth;
		
		else
			(*r).right = (*r).left + aboutlinewidth;
		}
	else
	*/
	if (item != miscinfoitem)
		(*r).left += 4 * abouticonsize + abouthorizinset;
	
	if ((*r).top > abouticonsize)
		(*r).top += aboutvertgap;
	
	(*r).bottom = (*r).top + aboutlineheight - 1;
	
	return ((*r).bottom <= (**aboutdata).aboutarea.bottom);
	} /*ccgettextitemrect*/


static boolean ccgetagentpopuprect (Rect *rpopup) {
	
	Rect r = (**aboutdata).messagearea;
	
	r.left += msgborderpix;
	
	r.right = r.left + widthsmallicon;
	
	// r.top += msgborderpix + msgtopinset;
	r.top = (r.bottom + r.top - heightsmallicon) / 2;
	
	r.bottom = r.top + heightsmallicon;
	
	*rpopup = r;
	
	return (true);
	} /*ccgetagentpopuprect*/


static void ccdrawmainwindowtext (short item, short style, ptrstring bs, short just) {
	
	Rect r;
	bigstring bsitem;
	
	if (!ccgettextitemrect (item, &r))
		return;
	
	if (bs == nil) { /*get from string list*/
		
		copystring (aboutstrings [item], bsitem);
		
		bs = bsitem;
		}
	
	setglobalfontsizestyle (geneva, 9, style);
	
	switch (just) {
		
		case leftjustified:
			eraserect (r);
			
			movepento (r.left, r.top + globalfontinfo.ascent);
			
			break;
		
		case rightjustified:
			movepento (r.right - stringpixels (bs), r.top + globalfontinfo.ascent);
			
			break;
		
		case fulljustified:
			break;
		}
	
	pendrawstring (bs);
	} /*ccdrawmainwindowtext*/


static void pushaboutstyle (void) {

	register hdlwindowinfo hw = aboutwindowinfo;
	
	if ((**aboutdata).flbigwindow)
		pushstyle (geneva, 9, normal);
	else
		pushstyle ((**hw).defaultfont, (**hw).defaultsize, (**hw).defaultstyle);
	} /*pushaboutstyle*/


static short ccgetneededheight (boolean flbigwindow) {

	register short x;
	
	if ((**aboutdata).flbootsplash)
		x = 0;
	
	else {
		
		pushaboutstyle ();
		
		x = globalfontinfo.ascent + globalfontinfo.descent;
		
		popstyle ();
		
		x = max (x, minmsgheight);
		
		x += msgtopinset + msgbottominset;
		}
	
	if (flbigwindow)
		x += 2 * msgborderpix + aboutrectheight() + msgvertgap;
	
	return (x);
	} /*ccgetneededheight*/


#if 0

static boolean ccfindagentvisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
	
	hdltreenode hcode;
	
	if (!langexternalvaltocode (val, &hcode)) /*not a scipt, or no code*/
		return (false);
	
	if (hcode == (**cancoonglobals).hprimaryagent)
		return (true);
	
	return (false);
	} /*ccfindagentvisit*/

#endif


static void ccdrawagentpopup (void) {
	
	Rect r;
	
	if (ccgetagentpopuprect (&r)) {
		
		if (cancoonglobals == nil)
			eraserect (r);
		else {
			ccupdateagentpopup (r);
			/*
			Point pt;
			bigstring bs;
			
			ccdrawmainwindowtext (agentpopupitem, bold, nil, leftjustified);
			
			getpenpoint (&pt);
			
			r.left = pt.h + agentmenuhorizgap;
			
			r.bottom = r.top + popupheight;
			
			if (!hashinversesearch (agentstable, &ccfindagentvisit, nil, bs))
				setemptystring (bs);
			
			insertstring ("\x03   ", bs);
			
			drawpopup (r, bs, true);
			*/
			}
		}
	} /*ccdrawagentpopup*/


#if 0

static void cceraseagentpopup (void) {
	
	Rect r;
	
	if (ccgetagentpopuprect (&r))
		eraserect (r);
	} /*cceraseagentpopup*/

#endif


static void ccgetmsgrect (Rect *rmsg) {
	
	/*
	5.0.2b19 dmb: leave correct room for version string
	*/
	
	Rect r;
	short lineheight = globalfontinfo.ascent + globalfontinfo.descent;
	
	getmessagecontentrect (rmsg);
	
	// if ((**aboutdata).flbigwindow)
	//		insetrect (rmsg, 6, 6);
	
	ccgetagentpopuprect (&r);
	
//	(*rmsg).top = r.top + 1;
	
//	(*rmsg).bottom = (*rmsg).top + globalfontinfo.ascent + globalfontinfo.descent;

	(*rmsg).top = ((*rmsg).bottom + (*rmsg).top - lineheight) / 2;
	
	(*rmsg).bottom = (*rmsg).top + lineheight;
	
	(*rmsg).left = r.right + 8;
	
	if (ccgettextitemrect (versionitem, &r) && (*rmsg).bottom > r.top) // overlap
		(*rmsg).right -= versionwidth; // leave root for version
	} /*ccgetmsgrect*/


static boolean ccdrawmsg (void) {
	
	/*
	2.1b5 dmb: don't need flbitmapactive logic; openbitmap will return 
	false if one's already open.
	*/
	
	register hdlcancoonrecord hc = cancoonglobals;
	//register hdlwindowinfo hw = aboutwindowinfo;
	register hdlstring hstring;
	register boolean flbitmap = false;
	bigstring bs;
	Rect r;
	
	if (hc == nil || aboutdata == nil)
		return (false);
	
	pushaboutstyle ();
	
	ccgetmsgrect (&r);
	
	flbitmap = openbitmap (r, aboutwindow);
	
	if (!flbitmap)
		pushclip (r);

	eraserect (r);
	
	/*
	grayframerect (r);
	*/
	
	hstring = (**hc).hprimarymsg;
	
	/*
	if (hstring == nil) /%no primary message, display file name%/
		hstring = (**hw).hfilename;
	*/
	
	copyheapstring (hstring, bs);
	
//	centerstring (r, bs);
	
	movepento (r.left, r.top + globalfontinfo.ascent);
	
	pendrawstring (bs);
	
	popstyle ();
	
	if (flbitmap)
		closebitmap (aboutwindow);
	else
		popclip ();
	
	return (true);
	} /*ccdrawmsg*/


static void ccdrawtextitem (short item, ptrstring param, short style, short just) {
	
	bigstring bsitem;
	
	parsedialogstring (aboutstrings [item], param, nil, nil, nil, bsitem);
	
	ccdrawmainwindowtext (item, style, bsitem, just);
	} /*ccdrawtextitem*/


static boolean ccrunurlscript () {

	bigstring bsscript;
	
	if (!getsystemtablescript (idopenurlscript, bsscript))
		return (false);
	
	parsedialogstring (bsscript, aboutstrings [urlitem], nil, nil, nil, bsscript);
	
	return (processrunstring (bsscript));
	} /*ccrunurlscript*/


static void ccdrawurlitem (boolean flpressed) {
	
	RGBColor rgb = {0, 0, 0};
	
	if (flpressed)
		rgb.red = 0xA000;	
	else
		rgb.blue = 0xA000;
	
	pushforecolor (&rgb);
	
	ccdrawtextitem (urlitem, nil, underline, leftjustified);
//	ccdrawmainwindowtext (urlitem, underline, nil, leftjustified);
	
	popforecolor ();
	} /*ccdrawurlitem*/


static void ccdrawstatistic (short item, ptrstring value, boolean flbitmap) {

	Rect r;
	
	if (flbitmap) {
	
		if (!ccgettextitemrect (item, &r))
			return;
		
		flbitmap = openbitmap (r, aboutwindow);
		}
	
	ccdrawmainwindowtext (item, bold, nil, leftjustified);
	
	ccdrawmainwindowtext (item, normal, value, rightjustified);
	
	if (flbitmap)
		closebitmap (aboutwindow);
	} /*ccdrawstatistic*/

	
static void ccupdatestatistics (boolean flbitmap) {
	
	/*
	1/20/93 dmb: if fldisableyield is set, we got here in strange circumstances.
	most likely, it indicates that a script running as a component or fast script 
	put up a dialog. to avoid a system crash, we must not call langrunstringnoerror.
	*/
	
	bigstring bs;
	short threadcount;
	
//	timedatestring (timenow (), bs);
//	ccdrawstatistic (datetimeitem, bs, flbitmap);
	
	if (aboutstatsshowing ()) {
	
	#ifdef WIN95VERSION
		extern long handlecounter;
		
		numbertostring (handlecounter, bs);

	#endif

	#ifdef MACVERSION
		long freemem = FreeMem () / 1024;
		
		numbertostring (freemem, bs);
		
		pushchar ('K', bs);
	#endif
		
		ccdrawstatistic (memoryitem, bs, flbitmap);
		
		threadcount = processthreadcount ();
		
		numbertostring (threadcount, bs);
		
		ccdrawstatistic (threadsitem, bs, flbitmap);
		
		ccdrawstatistic (thisthreaditem, bstheadinfo, flbitmap);
		
		ccdrawmainwindowtext (miscinfoitem, normal, bsmiscinfo, leftjustified);
		}
	
	/*
	if ((roottable != nil) && (!fldisableyield)) {
		
		if (langrunstringnoerror ("\puser.name", bs)) {
			
			ccdrawstatistic (usernameitem, bs, flbitmap);
			}
		}
	*/
	} /*ccupdatestatistics*/


static void	ccdrawabout (void) {
	
	/*
	5.0b9 dmb: user Arial, but just for the frontier(tm) item
	*/

	Rect rabout = (**aboutdata).aboutarea;
	bigstring bs;
	
	/*draw the icon*/ {
		
		#ifdef WIN95VERSION
			short tmfont;

			fontgetnumber ("\x05" "Arial", &tmfont);

			if (tmfont != 0)
				setglobalfontsizestyle (tmfont, 9, bold);
			else
		#endif
		
		setglobalfontsizestyle (geneva, 9, bold);
	
		movepento (rabout.left, rabout.top + abouticonsize + globalfontinfo.ascent);
		
		pendrawstring (aboutstrings [frontieritem]);
		
		#ifdef WIN95VERSION
			if (tmfont != 0)
				setglobalfontsizestyle (geneva, 9, bold);
		#endif
		

		rabout.left += abouticonsize;
		
		rabout.right = rabout.left + abouticonsize;
		
		rabout.bottom = rabout.top + abouticonsize;
		
		ccdrawfrontiericon (rabout, false);
		}
	
	ccdrawtextitem (copyrightitem, nil, normal, leftjustified);
	
	ccdrawurlitem (false);
	
	#if __powerc
	
	//	ccdrawtextitem (isaitem, "\pPowerPC", normal);
		parsedialogstring (aboutstrings [isaitem], "\x07" "PowerPC", nil, nil, nil, bs);
	
	#else
	
		parsedialogstring (aboutstrings [isaitem], "\x05" "680x0", nil, nil, nil, bs);
	
	#endif
	
//	ccdrawmainwindowtext (isaitem, normal, bs, leftjustified);
	
//	ccdrawtextitem (frontieritem, nil, bold);
	
	ccdrawtextitem (sloganitem, nil, italic, leftjustified);
	
	filegetprogramversion (bs);
	
	#ifdef fltrialsize
	
		insertstring ("\x06" "Trial ", bs);
		
	#endif
	
	ccdrawmainwindowtext (versionitem, normal, bs, rightjustified);
	
	ccupdatestatistics (false);
	} /*ccdrawabout*/


static void ccunblockmsg (void) {
	
	/*
	1/9/91 dmb: must set secondary message to nil when it's moved into 
	the primary message
	*/
	
	register hdlcancoonrecord hc = cancoonglobals;
	register hdlstring hs;
	
	if (hc && (**hc).flbackgroundmsgblocked) {
	
		(**hc).flbackgroundmsgblocked = false; 
		
		hs = (**hc).hsecondarymsg;
		
		if (hs != nil) {
		
			disposehandle ((Handle) (**hc).hprimarymsg);
			
			(**hc).hprimarymsg = hs;
			
			(**hc).hsecondarymsg = nil;
			
			ccdrawmsg ();
			}
		}
	} /*ccunblockmsg*/


boolean aboutsetthreadstring (hdlprocessthread hp, boolean flin) {
	
	register hdlcancoonrecord hc = cancoonglobals;
	hdlthreadglobals hg = (hdlthreadglobals) hp;
	boolean fl = false;
	hdlwindowinfo hinfo;
	
	if (hc == nil)
		return (false);
	
	if (!aboutstatsshowing ())
		return (false);
	
	if (!findaboutwindow (&hinfo) || !shellpushglobals ((**hinfo).macwindow))
		return (false);
	
	if (flin) {

		numbertostring ((long) (**hg).idthread, bstheadinfo);

		if ((**hg).hprocess) {
			
			pushstring ("\x02" " [", bstheadinfo);

			pushstring ((**(**hg).hprocess).bsname, bstheadinfo);

			pushchar (']', bstheadinfo);
			}
		}
	else
		insertstring ("\x06" "(out) ", bstheadinfo);
	
	if (aboutdata != nil)
		ccupdatestatistics (false);
	
	shellpopglobals ();
		
	return (fl);
	} /*aboutsetthreadstring*/


boolean aboutsetmiscstring (bigstring bsmisc) {
	
	#ifdef WIN95VERSION
	extern 	DWORD ixthreadglobalsgrabcount;			// Tls index of counter for nest globals grabbing

	long grabcount = (long) TlsGetValue (ixthreadglobalsgrabcount);
	#endif

//	register hdlcancoonrecord hc = cancoonglobals;
//	hdlwindowinfo hinfo;

	copystring (bsmisc, bsmiscinfo);

//	if (!findaboutwindow (&hinfo) || !shellpushglobals ((**hinfo).macwindow))
//		return (false);
	
	#ifdef WIN95VERSION
	if (grabcount > 0)
	#endif
	if (aboutport != nil && flhavemiscrect) {
		//Code change by Timothy Paustian Monday, August 21, 2000 4:17:36 PM
		//We cannot just pass a window or dialog ptr to pushport. It's doing
		//an implicit cast. This will not work on OS X
		CGrafPtr	thePort;
		#if TARGET_API_MAC_CARBON == 1
		thePort = GetWindowPort((WindowRef) aboutport);
		#else
		thePort = (CGrafPtr)aboutport;		
		#endif
		pushport(thePort);
			
			pushclip (miscinforect);
			
			eraserect (miscinforect);
			
			movepento (miscinforect.left, miscinforect.top + globalfontinfo.ascent);
			
			pendrawstring (bsmisc);
			
			popclip ();

		popport ();
		}
	
//	shellpopglobals ();

	return (true);
	} /*aboutsetmiscstring*/


boolean ccmsg (bigstring bs, boolean flbackgroundmsg) {
	
	/*
	can be called from a script, not as part of a callback sequence.
	
	if flbackgroundmsg is true the message is coming from an agent.  we avoid 
	letting such messages overwrite the non-background message until the user
	has done something.
	
	1/9/91: see comment in ccbringtofront
	
	12/28/91 dmb: always force code match for background message, event if hcode is nil
	*/
	
	register hdlcancoonrecord hc = cancoonglobals;
	boolean fl = false;
	hdlwindowinfo hinfo;
	hdlstring hstring;
	
	if (hc == nil)
		return (false);
	
	if (!findaboutwindow (&hinfo) || !shellpushglobals ((**hinfo).macwindow))
		hinfo = nil;
	
	if (!flbackgroundmsg && isemptystring (bs)) { /*empty string unblocks background messages*/
		
		ccunblockmsg ();
		
		goto pop;
		}
	
	if (flbackgroundmsg) {
		
		/*
		coming from an agent.  record the message text in the process record, so 
		we can use the popup to switch between the last message left by any agent.
		
		the make sure it's the currently-selected agent, or don't display it.
		*/
		
		register hdltreenode hcode = (**hc).hprimaryagent;
		register hdlprocessrecord hp = currentprocess;
		
		if (hp == nil) /*only agents are allowed to send background messages!*/
			goto pop;
		
		copystring (bs, (**hp).bsmsg); /*store in process record*/
		
		if (/*(hcode != nil) &&*/ ((**hp).hcode != hcode))
			goto pop;
		}
	
	if (!newheapstring (bs, &hstring))
		goto pop;
	
	if (flbackgroundmsg && (**hc).flbackgroundmsgblocked) {
		
		disposehandle ((Handle) (**hc).hsecondarymsg);
		
		(**hc).hsecondarymsg = hstring;
		
		goto pop;
		}
	
	disposehandle ((Handle) (**hc).hprimarymsg);
	
	(**hc).hprimarymsg = hstring;
	
	if (hinfo != nil)
		fl = ccdrawmsg ();
	
	if (!flbackgroundmsg)
		(**hc).flbackgroundmsgblocked = true;
	
	pop: 
	
	if (hinfo != nil)
		shellpopglobals ();
		
	return (fl);
	} /*ccmsg*/
	

static void aboutupdate (void) {
	
	/*
	1/22/91 dmb: openbitmap moves memory; its result cannot be assigned 
	into a double-dereferenced handle.  (ahem!)
	
	2.1b5 dmb: special case for nil cancoondata (during revert). also, removed 
	unneeded flbitmapactive logic
	*/
	
	register hdlwindowinfo hw = aboutwindowinfo;
	Rect r;
	
	displayedaboutdata = aboutdata;

	#if TARGET_API_MAC_CARBON
		aboutport = GetWindowPort(aboutwindow);
	#else
		aboutport = (CGrafPtr)aboutwindow;	
	#endif
	
	flhavemiscrect = false;

	if (aboutdata == nil) /*in the middle of a revert*/
		return;
	
	r = (**hw).contentrect;
	
	eraserect (r);
	
	shelldrawgrowicon (hw);
	
	if ((**aboutdata).flbigwindow) {
		
		ccdrawabout ();
		
	//	ccgetmsgrect (&r);
		getmessagecontentrect (&r);
		
		insetrect (&r, -1, -1);
		
		#if TARGET_API_MAC_CARBON == 1
			
			insetrect (&r, 0, -3);
			
			r.bottom = r.top + 2;
		
			DrawThemeSeparator (&r, kThemeStateActive);
		
		#else

		//	grayframerect (r);
			movepento (r.left, r.top);
			
			pushpen ();
			
			setgraypen ();
			
			pendrawline (r.right, r.top);
			
			poppen ();

		#endif
		
		flhavemiscrect = ccgettextitemrect (miscinfoitem, &miscinforect);
		}
	
	if (!(**aboutdata).flbootsplash) {
		
		ccdrawagentpopup ();
		
		ccdrawmsg ();
		}
	} /*aboutupdate*/


static boolean aboutgettargetdata (short id) {
	
	return (id == -1); /*true if target type is generic -- a shell verb*/
	} /*aboutgettargetdata*/


static boolean aboutresetrects (hdlwindowinfo hinfo) {
	
	/*
	12/28/90 dmb: the resetrects routine is called while the windowinfo is 
	being created, before any data has been created for it.  we can't assume that 
	cancoondata or aboutwindowinfo are not nil
	*/
	
	register hdlwindowinfo hw = hinfo;
	hdlaboutrecord ha = (hdlaboutrecord) (**hw).hdata;
	Rect rmessage, rabout;
	
	if (ha == nil)
		return (false);
	
	rmessage = (**hw).contentrect;
	
	shellcalcgrowiconrect (rmessage, hw);
	
	zerorect (&rabout);
	
	if ((**aboutdata).flbigwindow) { /*divide window into message and about*/
		
		rabout = rmessage; /*set left, right, bottom*/
		
		rabout.bottom = rabout.top + aboutrectheight();
		
		rmessage.top = rabout.bottom + msgvertgap;
		
	//	rmessage.bottom -= aboutlineheight;
		
		insetrect (&rabout, abouthorizinset, aboutvertinset);
		
		insetrect (&rmessage, abouthorizinset, aboutvertinset);
		}
	
	(**aboutdata).messagearea = rmessage;
	
	(**aboutdata).aboutarea = rabout;
	
	return (true);
	} /*aboutresetrects*/
	

static boolean attachabout (void) {
	
	hdlwindowinfo hroot;
	
	if (aboutdata == nil)
		return (false);

	if (!ccfindrootwindow (&hroot)) {
		
		(**aboutwindowinfo).parentwindow = nil;
		
		return (false);
		}
	
	(**aboutwindowinfo).parentwindow = hroot;
	
	return (true);
	} /*attachabout*/


static void detachabout (void) {
	
	(**aboutwindowinfo).parentwindow = nil;
	} /*detachabout*/


static void aboutresizeafterfontchange (void) {
	
	register hdlwindowinfo hw = aboutwindowinfo;
	register short diff;
	Rect r;
	
//	r = (**hw).contentrect;
	
	shellgetglobalwindowrect (hw, &r); /*rwindow is in global coords*/
	
	diff = ccgetneededheight ((**aboutdata).flbigwindow) - (r.bottom - r.top);
	
	if (diff > 0) {
		
		r.bottom += diff;
		
		moveandsizewindow ((**hw).macwindow, r);
		
		shelladjustaftergrow ((**hw).macwindow);
		}
	
	windowinval (aboutwindow);
	
	if (attachabout ()) {
		
		cccopywindowinfo (hw, ixaboutinfo); /*update aboutdata*/
		
		detachabout ();
		}
	} /*aboutresizeafterfontchange*/

static void aboutsetconfigminimum (void) {
	
	short ixaboutconfig;
	Rect *rmin;
	
	if (shellfindcallbacks (idaboutconfig, &ixaboutconfig)) {
	
		rmin = &globalsarray [ixaboutconfig].config.rmin;
		
		if ((**aboutdata).flbigwindow) {
		
			(*rmin).bottom = aboutrectheight() + msgvertgap + minmsgheight + 2 * msgborderpix;
	
			(*rmin).right = minaboutwidth;
			}
		else {
		
			(*rmin).bottom = 26;
	
			(*rmin).right = 260;
			}
		}
	} /*aboutsetconfigminimum*/


static boolean aboutzoom (hdlwindowinfo hinfo, boolean flgrow) {
	

	hdlaboutrecord ha = aboutdata;
	register hdlwindowinfo hw = hinfo;
	Rect rwindow;
	
	flgrow = (**ha).flbigwindow = !(**ha).flbigwindow;
	
	shellgetglobalwindowrect (hw, &rwindow);
		
	/*
	if (flgrow)
		rwindow.bottom += aboutrectheight + msgvertgap;
	else
		rwindow.bottom -= aboutrectheight + msgvertgap;
	*/
	rwindow.bottom = rwindow.top + ccgetneededheight ((**aboutdata).flbigwindow);
	
	if (flgrow)
		rwindow.right = max (rwindow.right, rwindow.left + minaboutwidth);
	
	moveandsizewindow ((**hw).macwindow, rwindow);
	
	shellinvalcontent (hw);

	shelladjustaftergrow ((**hw).macwindow);

	if (cancoonglobals != nil) // remember setting with current root
		(**cancoonglobals).flbigwindow = (**ha).flbigwindow;
	
	aboutsetconfigminimum ();

	shellupdatewindow ((**hw).macwindow);

	return (true);
	} /*aboutzoom*/


static boolean aboutsetsize (void) {
	
	register hdlwindowinfo hw = aboutwindowinfo;

	(**hw).defaultsize = (**hw).selectioninfo.fontsize;
	
	(**hw).selectioninfo.fldirty = true;
	
	aboutresizeafterfontchange ();
	
	return (true);
	} /*aboutsetsize*/


static boolean aboutsetfont (void) {

	register hdlwindowinfo hw = aboutwindowinfo;

	(**hw).defaultfont = (**hw).selectioninfo.fontnum;
	
	(**hw).selectioninfo.fldirty = true;
	
	aboutresizeafterfontchange ();
	
	return (true);
	} /*aboutsetfont*/

	
static boolean aboutsetstyle (void) {

	register hdlwindowinfo hw = aboutwindowinfo;
	
	shellsetdefaultstyle (hw); /*sets the defaultstyle field based on selectioninfo*/
	
	(**hw).selectioninfo.fldirty = true;
	
	aboutresizeafterfontchange ();
	
	return (true);
	} /*aboutsetstyle*/


static boolean aboutmousedown (Point pt, tyclickflags flags) {
	
	/*
	1/24/91 dmb: to allow the home window to be dragged with a single click, 
	even when it's not already the front window, the dontconsumefrontclicks 
	flag is set in its config.  in order to avoid reacting to button clicks 
	or clicks in the close box or on the flag (which aren't even visiable when 
	the window is inactive), we test to make sure that the window has been 
	activated before considering taking any special actions.
	*/
	
	hdlaboutrecord hc = aboutdata;
	Rect r;
	
	ccunblockmsg (); /*any mouse click re-enables background messages*/
	
	if ((**hc).flbigwindow) {
		
		if (ccgettextitemrect (urlitem, &r) && pointinrect (pt, r)) {
			
			if (trackicon (r, &ccdrawurlitem))
				if (!ccrunurlscript ())
					sysbeep ();
			
			return (true);
			}
		}

	if (ccgetagentpopuprect (&r)) { /*see if click is in agents popup*/
		
		if (pointinrect (pt, r)) { /*click in agents popup*/
			
			--r.top;
			
			if (ccagentpopuphit (r, pt))
				ccdrawagentpopup ();
			
			return (true);
			}
		}
	
	return (true);
	} /*aboutmousedown*/


static boolean aboutkeystroke (void) {

	char chkb = keyboardstatus.chkb;

	if (chkb == chenter)
		aboutzoom (aboutwindowinfo, true);

	return (true);
	} /*aboutkeystroke*/


#ifndef version42orgreater

static void drawtextitem (WindowPtr w, short item, short font, short size, short style) {
	
	Rect ritem;
	bigstring bsitem;
	
	getdialogtext ((DialogPtr) w, item, bsitem);
	
	dialoggetobjectrect ((DialogPtr) w, item, &ritem);
	
	edittextbox (bsitem, ritem, font, size, style);
	} /*drawtextitem*/


static boolean runurlscript () {

	bigstring bsurl;
	bigstring bsscript;
	//this is a problem. aboutwindow is a window
	getdialogtext ((DialogPtr) aboutwindow, urlitem, bsurl);
	
	if (!getsystemtablescript (idopenurlscript, bsscript))
		return (false);
	
	parsedialogstring (bsscript, bsurl, nil, nil, nil, bsscript);
	
	return (processrunstring (bsscript));
	} /*runurlscript*/


static void drawurlitem (boolean flpressed) {
	
	RGBColor rgb = {0, 0, 0};
	
	if (flpressed)
		rgb.red = 0xA000;	
	else
		rgb.blue = 0xA000;
	
	pushforecolor (&rgb);
	
	drawtextitem (aboutwindow, urlitem, geneva, 9, underline);
	
	popforecolor ();
	} /*drawurlitem*/


#define aboutresnumber 128 /*the id of the various "about" resources*/

static void drawabout (WindowPtr w, boolean flliveurl) { 

	/*
	10/18/91 DW: color!
	
	10/18/91 DW: added PICT #128 to shell.¹.rsrc.
	
	12/19/91 dmb: use version resource instead of buildinfo routine
	*/
	
	/*draw the icon*/ {
		
		Rect rcicn;
		
		dialoggetobjectrect (w, iconitem, &rcicn);
		
		ploticon (&rcicn, aboutresnumber);
		}
	
	drawtextitem (w, userlanditem, systemFont, 12, 0);
	
	drawtextitem (w, sloganitem, helv, 9, 0);
	
	drawtextitem (w, copyrightitem, helv, 9, 0);
	
	if (flliveurl)
		drawurlitem (false);
	else
		drawtextitem (w, urlitem, helv, 9, 0);
	
	#if __powerc
	
		drawtextitem (w, nativeitem, helv, 9, 0);
	
	#else
	
		drawtextitem (w, emulateditem, helv, 9, 0);
	
	#endif
	
	/*draw the version string*/ {
	
		Rect ritem;
		
		bigstring bsbuildinfo;
		
		filegetprogramversion (bsbuildinfo);
		
		setfontsizestyle (geneva, 9, normal);
		
		dialoggetobjectrect (w, versionitem, &ritem);
		
		movepento (ritem.right - stringpixels (bsbuildinfo), ritem.bottom - 6);
	
		pendrawstring (bsbuildinfo);
		}
	} /*drawabout*/


static void updateabout (WindowPtr w, boolean flliveurl) {
	
	shellupdatenow (w);
	} /*updateabout*/


static boolean abouteventhook (EventRecord *ev, WindowPtr w) {
	
	if (w != aboutwindow) /*don't hook if not our dialog*/
		return (true);
	
	if ((*ev).what == updateEvt)
		updateabout (w, false);
	
	return (false); /*don't process this event any further*/
	} /*abouteventhook*/

#endif


static boolean aboutsave (ptrfilespec fs, hdlfilenum fnum, short rnum, boolean flsaveas, boolean flrunnable) {

	hdlwindowinfo hinfo;
	
	if (ccfindrootwindow (&hinfo)) {
		
		shellpushglobals ((**hinfo).macwindow);
		
		grayownedwindows (shellwindow);
		
		ccsavefile (fs, fnum, rnum, flsaveas, flrunnable);
		
		shellpopglobals ();
		}
	
	return (true);
	} /*aboutsave*/


static boolean aboutclose (void) {
	
	if (!(**aboutdata).flbootsplash && (cancoonglobals != nil)) {
		
		attachabout ();
		
		ccsubwindowclose (aboutwindowinfo, ixaboutinfo);
		
		detachabout ();
		}
	
	aboutwindow = nil;
	
	return (true);
	} /*aboutclose*/


static boolean aboutdisposerecord (void) {
	
	disposehandle ((Handle) aboutdata);
	
	aboutdata = nil;
	
	displayedaboutdata = nil;
	
	aboutport = nil;

	return (true);
	} /*aboutdisposerecord*/


static boolean aboutsetsuperglobals (void) {
	
	hdlwindowinfo hinfo;
	
	if (ccfindrootwindow (&hinfo)) {
		
		shellpushglobals ((**hinfo).macwindow);
		
		ccsetsuperglobals ();
		
		shellpopglobals ();
		}
	
	return (true);
	} /*aboutsetsuperglobals*/


static void aboutwindowsetup (void) {
	
	/*
	5.0a2 dmb: while we're grabbing out font settings from the root, 
	also steal the old flbigwindow flag
	*/
	
	register hdlwindowinfo hw = aboutwindowinfo;
	tycancoonwindowinfo windowinfo;
	
	// get bigwindow setting from current root
	(**aboutdata).flbigwindow = (cancoonglobals == nil) || (**cancoonglobals).flbigwindow;
	
	(**aboutdata).flextrastats = flsessionstats; //optionkeydown ();
	
	if (attachabout ()) {
		
		shellpushrootglobals (aboutwindow);

		ccgetwindowinfo (ixaboutinfo, &windowinfo);
		
		(**hw).defaultfont = windowinfo.fontnum;
		
		(**hw).defaultsize = windowinfo.fontsize;
		
		windowinfo.w = aboutwindow;
		
		ccsetwindowinfo (ixaboutinfo, windowinfo);
		
		shellpopglobals ();

		detachabout ();
		}
	} /*aboutwindowsetup*/


static boolean newaboutwindow (boolean flbootsplash) {
	
	/*
	5.0.2b20 dmb: don't reset window pos for negative values
	*/
	
	WindowPtr w;
	hdlwindowinfo hw;
	bigstring bstitle;
	Rect rzoom, rwindow;
	hdlaboutrecord hdata;
	short ixaboutconfig;
	
	if (!newclearhandle (sizeof (tyaboutrecord), (Handle *) &hdata))
		return (false);
	
	shellfindcallbacks (idaboutconfig, &ixaboutconfig);
	
	if (flbootsplash) {
		
		globalsarray [ixaboutconfig].config.templateresnum = 131;//config.fldialog
		
		(**hdata).flbootsplash = true;
		}
	else
		globalsarray [ixaboutconfig].config.templateresnum = 129; //restore
	
	// get bigwindow setting with current root
//	(**hdata).flbigwindow = (cancoonglobals == nil) || (**cancoonglobals).flbigwindow;
	
	ccgetwindowrect (ixaboutinfo, &rwindow);
	
//	if (rwindow.top <= 0 || rwindow.left <= 0)
//		rwindow.top = -1;
	
	getsystemoriginrect (&rzoom);
	
	//	getstringlist (aboutlistnumber, abouttitlestring, bstitle);
	copystring (aboutstrings [flbootsplash? frontieritem: titleitem], bstitle);
	
	if (!newchildwindow (idaboutconfig, nil, &rwindow, &rzoom, bstitle, &w)) {
		
		disposehandle ((Handle) hdata);
		
		return (false);
		}
	
	#if TARGET_API_MAC_CARBON == 1
	
		SetThemeWindowBackground (w, kThemeBrushModelessDialogBackgroundActive, false);
	
	#endif
	
	getwindowinfo (w, &hw);
	
	(**hw).hdata = (Handle) hdata;
	
	shellpushglobals (w);
	
	aboutwindowsetup ();
	
	aboutresetrects (hw);
	
	if (!flbootsplash) {
		
		aboutresizeafterfontchange ();
		
		aboutsetconfigminimum ();
		}
	
	shellpopglobals ();
	
	windowzoom (w);
	
	return (true);
	} /*newaboutwindow*/


boolean aboutcommand (void) {
	
	hdlwindowinfo hinfo;
	
	if (findaboutwindow (&hinfo)) {
		
		shellbringtofront (hinfo);
		
		return (true);
		}
	
	return (newaboutwindow (false));
	} /*aboutcommand*/


void aboutsetstatsflag (boolean fl) {
	hdlwindowinfo hinfo;
	WindowPtr w;
	flsessionstats = fl;

	if (findaboutwindow (&hinfo)) {
		
		aboutwindowinfo = hinfo;

		aboutdata = (hdlaboutrecord)(**aboutwindowinfo).hdata;

		if (aboutdata != nil)
			(**aboutdata).flextrastats = fl;

		w = (**hinfo).macwindow;
	
		aboutresetrects (hinfo);

		shellpushglobals (w);

		aboutsetsize();
	
		shellpopglobals ();
		}
		
	} /*aboutsetstatsflag*/


boolean aboutstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks aboutcallbacks;
	register ptrcallbacks cb;
	static boolean aboutstarted = false;
	
	if (aboutstarted)
		return (true);
	
	aboutstarted = true;
	
	shellnewcallbacks (&aboutcallbacks);
	
	cb = aboutcallbacks; /*copy into register*/
	
	loadconfigresource (idaboutconfig, &(*cb).config);
	
	(*cb).configresnum = idaboutconfig;
		
	(*cb).windowholder = &aboutwindow;
	
	(*cb).dataholder = (Handle *) &aboutdata;
	
	(*cb).infoholder = &aboutwindowinfo;
	
	(*cb).setglobalsroutine = &attachabout;
	
	(*cb).saveroutine = &aboutsave;
	
	(*cb).disposerecordroutine = &aboutdisposerecord;
	
	(*cb).closeroutine = &aboutclose;
	
	(*cb).gettargetdataroutine = &aboutgettargetdata;
	
	(*cb).updateroutine = &aboutupdate;
	
//	(*cb).activateroutine = &aboutactivate;
	
	(*cb).mouseroutine = &aboutmousedown;
	
	(*cb).keystrokeroutine = &aboutkeystroke;
	
//	(*cb).adjustcursorroutine = &aboutadjustcursor;
	
//	(*cb).msgroutine = &aboutmsg;
	
	(*cb).resetrectsroutine = &aboutresetrects;
	
	(*cb).fontroutine = &aboutsetfont;
	
	(*cb).sizeroutine = &aboutsetsize;
	
	(*cb).styleroutine = &aboutsetstyle;
	
	(*cb).zoomwindowroutine = &aboutzoom;
	
	(*cb).setsuperglobalsroutine = &aboutsetsuperglobals;
	
	return (true);
	} /*aboutstart*/


boolean openabout (boolean flzoom, long ctreservebytes) {
	
	/*
	2.1b5 dmb: added ctreservebytes parameter. of non-zero, caller wants us to 
	reserve space in the heap below the dialog record (during initialization)
	*/
	
	hdlwindowinfo hinfo;
	
#ifdef MACVERSION
	Ptr ptemp = nil;
	
	if (ctreservebytes > 0)
		ptemp = NewPtr (ctreservebytes); /*force about window to load high*/
#endif

	aboutstart ();
	
	shellpatchnilroutines ();
	
//	aboutwindow = newmodaldialog (128, -1);
	newaboutwindow (true);
	
	if (findaboutwindow (&hinfo))
	{
		shellupdatenow ((**hinfo).macwindow);
		//Code change by Timothy Paustian 10/5/00
		//We need to flush the about window to the screen
		//safe because this routine only gets call during init. 
		#if TARGET_API_MAC_CARBON == 1
		QDFlushPortBuffer(GetWindowPort((**hinfo).macwindow), nil);
		#endif
	}
#ifdef MACVERSION
	if (ptemp != nil)
		DisposePtr (ptemp); /*restore heap space for remaining code segments*/
#endif
	
	aboutopenticks = gettickcount ();
	
	return (true);
	} /*openabout*/


void closeabout (boolean flzoom, short minticks) {
	
	hdlwindowinfo hinfo;
	
	if (findaboutwindow (&hinfo)) {
	
		delayticks (aboutopenticks + minticks - gettickcount ());
		
		shellclose ((**hinfo).macwindow, false);
		}
	} /*closeabout*/

