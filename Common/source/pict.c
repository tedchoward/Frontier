
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

#include "bitmaps.h"
#include "file.h"
#include "quickdraw.h"
#include "memory.h"
#include "mouse.h"
#include "ops.h"
#include "resources.h"
#include "scrollbar.h"
#include "strings.h"
#include "frontierwindows.h"
#include "zoom.h"
#include "shell.h"
#include "process.h"
#include "pict.h"
#include "timedate.h"
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */



#define pictstartrealdata 512L /*this is where the PICT is stored in a PICT file*/


typedef struct tydiskpictrecord {
	
	short versionnumber; /*this structure is stored on disk*/
	
	diskrect windowrect; 
	
	long timecreated, timelastsave; /*maybe we'll use these at some later date?*/
	
	long ctsaves; /*the number of times this structure has been saved*/
	
	long pictbytes; /*number of bytes allocated for PICT*/
	
	short updateticks; /*how many ticks between updates when window is in front*/
	
	short flags;
	
	byte waste [20]; /*room for growth*/
	
	/*remaining bytes hold the PICT data*/
	} tydiskpictrecord, *ptrdiskpictrecord, **hdldiskpictrecord;

#ifdef MACVERSION
	#define flbitmapupdate_mask 0x8000 /*if set, use offscreen bitmaps when updating*/
	#define flevalexpressions_mask 0x4000 /*if set, parse all text that begins with an = sign*/
	#define flscaletofitwindow_mask 0x2000 /*if set, scale window down to fit inside window*/
#endif

#ifdef WIN95VERSION
	#define flbitmapupdate_mask 0x0080 /*if set, use offscreen bitmaps when updating*/
	#define flevalexpressions_mask 0x0040 /*if set, parse all text that begins with an = sign*/
	#define flscaletofitwindow_mask 0x0020 /*if set, scale window down to fit inside window*/
#endif


typedef struct tyOLD42diskpictrecord {
	
	short versionnumber; /*this structure is stored on disk*/
	
	diskrect windowrect; 
	
	long timecreated, timelastsave; /*maybe we'll use these at some later date?*/
	
	long ctsaves; /*the number of times this structure has been saved*/
	
	long pictbytes; /*number of bytes allocated for PICT*/
	
	short updateticks; /*how many ticks between updates when window is in front*/
	
	boolean flbitmapupdate: 1; /*if true, use offscreen bitmaps when updating*/
	
	boolean flevalexpressions: 1; /*if true, parse all text that begins with an = sign*/
	
	boolean flscaletofitwindow: 1; /*if true, scale window down to fit inside window*/
	
	char waste [21]; /*room for growth*/
	
	/*remaining bytes hold the PICT data*/
	} tyOLD42diskpictrecord, *ptrOLD42diskpictrecord, **hdlOLD42diskpictrecord;


WindowPtr pictwindow = nil;

hdlwindowinfo pictwindowinfo = nil;

hdlpictrecord pictdata = nil;


#ifdef MACVERSION
	static CQDProcs procs; /*must be statically allocated & have room for color procs*/

	static CQDProcs *porigprocs; /*need to save, call & restore*/
#endif



boolean pictgetframerect (hdlpictrecord hpict, Rect *rframe) {
	
#ifdef MACVERSION
	PicHandle macpicture = (**hpict).macpicture;
	
	if (macpicture == nil)
		return (false);
	
	*rframe = (**macpicture).picFrame;
#endif

#ifdef WIN95VERSION
	#pragma message ("Win95: pictgetframerect not implemented")
#endif

	return (true);
	} /*pictgetframerect*/


boolean pictpack (hdlpictrecord hpict, Handle *hpacked) {
	
	/*
	create a packed, contiguous version of the indicated pict record.
	
	if hpacked comes in non-nil, we just append our stuff to the end of the handle, 
	we don't allocate a new one.
	*/
	
	register hdlpictrecord hp = hpict;
	tydiskpictrecord header;
	
	clearbytes (&header, longsizeof (header));
	
	header.versionnumber = conditionalshortswap (1);
	
	recttodiskrect (&(**hpict).windowrect, &header.windowrect);
	
	header.timecreated = conditionallongswap ((**hp).timecreated);
	
	header.timelastsave = conditionallongswap ((**hp).timelastsave);
	
	/*timestamp (&header.timelastsave);*/ /*dmb 4.1b13: don't stamp it; pictdirty sets it as true mode date*/
	++((**hp).ctsaves);

	header.ctsaves = conditionallongswap((**hp).ctsaves);
	
	header.updateticks = conditionalshortswap((**hp).updateticks);
	
	if ((**hp).flbitmapupdate)
		header.flags |= flbitmapupdate_mask;
	
	if ((**hp).flevalexpressions)
		header.flags |= flevalexpressions_mask;
	
	if ((**hp).flscaletofitwindow)
		header.flags |= flscaletofitwindow_mask;
	
	header.pictbytes = gethandlesize ((Handle) (**hp).macpicture);

	memtodisklong (header.pictbytes);
	
	if (*hpacked == nil) { /*must allocate a new one*/
		
		if (!newfilledhandle (&header, longsizeof (header), hpacked))
			return (false);
		}
	else {
		
		if (!enlargehandle (*hpacked, longsizeof (header), &header))
			return (false);
		}
	
	return (pushhandle ((Handle) (**hpict).macpicture, *hpacked));
	} /*pictverbpack*/
	
	
boolean pictunpack (Handle hpacked, long *ixload, hdlpictrecord *hpict) {
	
	/*
	unpack the pict record from the handle at the indicated index.  bump the index
	to point past the pict record.  return true if it worked.
	*/
	
	tydiskpictrecord header;
	PicHandle macpicture;
	register hdlpictrecord hp;

	assert (sizeof (tyOLD42diskpictrecord) == sizeof (tydiskpictrecord));
	
#if 0 // def MACVERSION
	short version;
	long ix;
	tyOLD42diskpictrecord oldheader;

	ix = *ixload;

	if (!loadfromhandle (hpacked, &ix, longsizeof(version), &version))
		return (false);

	if (version == 1) {
		if (!loadfromhandle (hpacked, ixload, longsizeof (oldheader), &oldheader))
			return (false);
		
		if (!loadfromhandletohandle (hpacked, ixload, oldheader.pictbytes, false, (Handle *) &macpicture))
			return (false);
		
		if (!newclearhandle (longsizeof (typictrecord), (Handle *) hpict)) {
			
			disposehandle ((Handle) macpicture);
			
			return (false);
			}
		
		hp = *hpict; /*copy into register*/
		
		recttodiskrect (&(**hp).windowrect, &oldheader.windowrect);
		
		(**hp).timecreated = oldheader.timecreated;
		
		(**hp).timelastsave = oldheader.timelastsave;
		
		(**hp).ctsaves = oldheader.ctsaves;
		
		(**hp).updateticks = oldheader.updateticks;
		
		(**hp).flbitmapupdate = oldheader.flbitmapupdate;
		
		(**hp).flevalexpressions = oldheader.flevalexpressions;
		
		(**hp).flscaletofitwindow = oldheader.flscaletofitwindow;
		
		(**hp).macpicture = macpicture;
		
		return (true);
		}
#endif

	if (!loadfromhandle (hpacked, ixload, longsizeof (header), &header)) {
	
		shellerrormessage (BIGSTRING ("\x3d" "Can't unpack picture because unexpected data was encountered."));
		
		return (false);
		}
	
	disktomemlong (header.pictbytes);
	
	if (!loadfromhandletohandle (hpacked, ixload, header.pictbytes, false, (Handle *) &macpicture))
		return (false);
	
	if (!newclearhandle (longsizeof (typictrecord), (Handle *) hpict)) {
		
		disposehandle ((Handle) macpicture);
		
		return (false);
		}
	
	hp = *hpict; /*copy into register*/
	
	diskrecttorect (&header.windowrect, &(**hp).windowrect);
	
	(**hp).timecreated = conditionallongswap (header.timecreated);
	
	(**hp).timelastsave = conditionallongswap (header.timelastsave);
	
	(**hp).ctsaves = conditionallongswap (header.ctsaves);
	
	(**hp).updateticks = conditionalshortswap (header.updateticks);
	
	(**hp).flbitmapupdate = (header.flags & flbitmapupdate_mask) != 0;
	
	(**hp).flevalexpressions = (header.flags & flevalexpressions_mask) != 0;
	
	(**hp).flscaletofitwindow = (header.flags & flscaletofitwindow_mask) != 0;
	
	(**hp).macpicture = macpicture;
	
	return (true);
	} /*pictunpack*/
	
	
boolean pictnewrecord (void) {
	
	/*
	4.0b7 dmb: initialize timelastsave to creation time; it's really a modification date.
	*/
	
	register hdlpictrecord hp;
	
	if (!newclearhandle (longsizeof (typictrecord), (Handle *) &pictdata)) 
		return (false);
		
	hp = pictdata; /*copy into register*/
	
	(**hp).timecreated = (**hp).timelastsave = timenow ();
	
	(**hp).windowrect.top = -1; /*accept default, unless someone resets this*/
	
	return (true);
	} /*pictnewrecord*/
	
	
boolean pictdisposerecord (hdlpictrecord hpict) {
	
	disposehandle ((Handle) (**hpict).macpicture);
	
	disposehandle ((Handle) hpict);
	
	return (true);
	} /*pictdisposerecord*/


#if 0

static boolean pictdebug (PicHandle macpicture) { 

	Rect r, rbounds;
	WindowPtr w;
	
	r = (**macpicture).picFrame;
	
	if (!zoomtempwindow (true, r.bottom - r.top, r.right - r.left, &w))
		return (false);
	
	DrawPicture (macpicture, &r);
	
	waitmousebutton (true);
	
	closetempwindow (true, w);
	
	return (true);
	} /*pictdebug*/
	

boolean pictreadfile (bigstring fname, PicHandle *macpicture) {
	
	register PicHandle hp = nil;
	register long ctbytes;
	short fnum;
	register boolean fl;
	
	if (!openfile (fname, 0, &fnum))
		return (false);
	
	ctbytes = filegetsize (fnum) - pictstartrealdata;
	
	if (ctbytes <= 0) /*error, this can't be a PICT file*/
		goto error;
	
	if (!filesetposition (fnum, pictstartrealdata))
		goto error;
		
	if (!newhandle (ctbytes, (Handle *) macpicture))
		goto error;
	
	hp = *macpicture; /*copy into register*/
	
	lockhandle ((Handle) hp);
	
	fl = fileread (fnum, ctbytes, *hp);
	
	unlockhandle ((Handle) hp);
	
	if (!fl)
		goto error;
	
	closefile (fnum);
	
	/*pictdebug (*macpicture);*/
	
	return (true);
		
	error:
	
	disposehandle ((Handle) hp); 
	
	closefile (fnum);
	
	*macpicture = nil;
	
	return (false);
	} /*pictreadfile*/
	
#endif


void pictresetscrollbars (void) {
	
	register hdlpictrecord hp = pictdata;
	register hdlwindowinfo hw = pictwindowinfo;
	tyscrollinfo vertinfo, horizinfo;
	Rect rcontent, rpict;
	
	rcontent = (**hw).contentrect;
	
	if (!pictgetframerect (hp, &rpict))
		rpict = rcontent;
	
	vertinfo.min = 0;
	
	vertinfo.max = (rpict.bottom - rpict.top) - (rcontent.bottom - rcontent.top);
	
	horizinfo.min = 0;
	
	horizinfo.max = (rpict.right - rpict.left) - (rcontent.right - rcontent.left);
	
	vertinfo.cur = (**hw).vertscrollinfo.cur;
	
	horizinfo.cur = (**hw).horizscrollinfo.cur;
	
	vertinfo.max = max (vertinfo.min, vertinfo.max);
	
	horizinfo.max = max (horizinfo.min, horizinfo.max);
	
	vertinfo.cur = max (vertinfo.min, vertinfo.cur);
	
	vertinfo.cur = min (vertinfo.max, vertinfo.cur);
	
	horizinfo.cur = max (horizinfo.min, horizinfo.cur);
	
	horizinfo.cur = min (horizinfo.max, horizinfo.cur);
	
	horizinfo.pag = rcontent.right - rcontent.left;

	vertinfo.pag = rcontent.bottom - rcontent.top;

	(**hw).vertscrollinfo = vertinfo;
	
	(**hw).horizscrollinfo = horizinfo;
	
	(**hw).fldirtyscrollbars = true; /*force an update*/
	} /*pictresetscrollbars*/


boolean pictscroll (tydirection dir, boolean flpage, long ctscroll) {
	
	/*
	from DW to anyone -- don't factor this shit down, I like it this way.  if you
	factor it, you may save one or two bytes, but you make it unreadable.  if you
	don't believe me, have a look at wpscroll.
	
	9/22/92 dmb: don't ctscroll parameter -- it's important!
	*/
	
	register hdlwindowinfo hw = pictwindowinfo;
	register long dh = 0, dv = 0;
	Rect r;
	
	r = (**hw).contentrect;
	
	if (!flpage)
		ctscroll = max (ctscroll, 4);
	
	switch (dir) {
		
		case down:
			if (flpage)
				dv = -(r.bottom - r.top);
			else
				dv = -ctscroll;
			
			break;
			
		case up:
			if (flpage)
				dv = r.bottom - r.top;
			else
				dv = ctscroll;
			
			break;
		
		case right:
			if (flpage)
				dh = -(r.right - r.left);
			else
				dh = -ctscroll;
				
			break;
			
		case left:
			if (flpage)
				dh = r.right - r.left;
			else
				dh = ctscroll;
				
			break;
			
		default:
			return (false);
		} /*switch*/
	
	if (dv != 0) {
		
		register long x;
		
		x = (**hw).vertscrollinfo.cur + dv;
	
		x = max (x, (**hw).vertscrollinfo.min);
	
		x = min (x, (**hw).vertscrollinfo.max);
		
		dv = (**hw).vertscrollinfo.cur - x;
		
		(**hw).vertscrollinfo.cur = x;
		}
	
	if (dh != 0) {
		
		register long x;
		
		x = (**hw).horizscrollinfo.cur + dh;
	
		x = max (x, (**hw).horizscrollinfo.min);
	
		x = min (x, (**hw).horizscrollinfo.max);
		
		dh = (**hw).horizscrollinfo.cur - x;
		
		(**hw).horizscrollinfo.cur = x;
		}
	
	if ((dh != 0) || (dv != 0)) {
	
		scrollrect (r, dh, dv);
	
		shellupdatenow (pictwindow);
		}
		
	return (true);
	} /*pictscroll*/


static boolean picttextparse (Ptr textbuf, short ctbytes, bigstring bs) {
	
	/*
	6/1/93 dmb: agentsdisable is defunct.
	*/
	
	bigstring bsresult;
	boolean fl;
	
	setstringlength (bs, ctbytes);
	
	if (ctbytes == 0)
		return (true);
		
	moveleft (textbuf, &bs [1], (long) ctbytes);
	
	if (bs [1] != '=') /*no computation desired*/
		return (true);
		
	deletestring (bs, 1, 1); /*pop off the = sign*/
	
	if (stringlength (bs) == 0) /*nothing but an = sign*/
		return (true);
	
	/*
	agentsdisable (true);
	*/
	
	fl = langrunstringnoerror (bs, bsresult);
	
	/*
	agentsdisable (false);
	*/
	
	if (fl) {
	
		copystring (bsresult, bs);
		
		return (true);
		}
	
	getstringlist (pictstringlist, picterrorstring, bs);
	
	return (false);
	} /*picttextparse*/

#ifdef MACVERSION

	static pascal void update_stdtext (short ctbytes, Ptr textbuf, Point numer, Point denom)  {
	
		bigstring bs;
		
		picttextparse (textbuf, ctbytes, bs);
		
		if (porigprocs == nil)
			StdText (stringlength (bs), &bs [1], numer, denom);
		else
		
			#if TARGET_API_MAC_CARBON == 1
			
				InvokeQDTextUPP ((short) stringlength (bs), (Ptr) &bs [1], numer, denom, (*porigprocs).textProc);
			
			#else
		
				CallQDTextProc ((*porigprocs).textProc, (short) stringlength (bs), (Ptr) &bs [1], numer, denom);
			
			#endif
		} /*update_stdtext*/
	
	
	static pascal short update_stdtxmeas (short ctbytes, Ptr textbuf, Point *numer, Point *denom, FontInfo *info) {
		
		bigstring bs;
		
		picttextparse (textbuf, ctbytes, bs);
		
		if (porigprocs == nil)
			return (StdTxMeas (stringlength (bs), &bs [1], numer, denom, info));
	
		#if TARGET_API_MAC_CARBON == 1

			return (InvokeQDTxMeasUPP ((short) stringlength (bs), (Ptr) &bs [1], numer, denom, info, (*porigprocs).txMeasProc));
			
		#else
			
			return (CallQDTxMeasProc ((*porigprocs).txMeasProc, (short) stringlength (bs), (Ptr) &bs [1], numer, denom, info));
		
		#endif
		} /*update_stdtxmeas*/
	
	#if TARGET_API_MAC_CARBON == 1
	#define update_stdtextUPP ((QDTextUPP) update_stdtext)
		
	#define update_stdtxmeasUPP ((QDTxMeasUPP) update_stdtxmeas)
	#else
	#if GENERATINGCFM
		
		static RoutineDescriptor update_stdtextDesc = BUILD_ROUTINE_DESCRIPTOR (uppQDTextProcInfo, update_stdtext);
		
		static RoutineDescriptor update_stdtxmeasDesc = BUILD_ROUTINE_DESCRIPTOR (uppQDTxMeasProcInfo, update_stdtxmeas);
		
		#define update_stdtextUPP (&update_stdtextDesc)
		
		#define update_stdtxmeasUPP (&update_stdtxmeasDesc)
		
	#else
		
		#define update_stdtextUPP ((QDTextUPP) update_stdtext)
		
		#define update_stdtxmeasUPP ((QDTxMeasUPP) update_stdtxmeas)
		
	#endif
#endif//TARGET_API_MAC_CARBON
		
	
#endif


void pictupdatepatcher (void) {
	
	/*
	2.1b5 dmb: use CProcs to handle color windows
	*/
	
#ifdef MACVERSION
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:24:15 PM
	//Changed to Opaque call for Carbon
	//I think we can just use a CGrafPort instead of casting to a WindowPtr
	//as in the old code
	//This needs to be tested.
	register CGrafPtr	thePort;
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	thePort = GetQDGlobalsThePort();
	porigprocs = (CQDProcsPtr)GetPortGrafProcs(thePort);
	#else
	//old code
	register WindowPtr w = getport ();
	thePort = (CGrafPtr) w;
	porigprocs = (CQDProcs *) (*w).grafProcs; /*must chain if not nil*/
	#endif
	if (porigprocs == nil) {
		
		if (iscolorport (thePort))
			SetStdCProcs (&procs); /*QD fills in the record*/
		else
			SetStdProcs ((QDProcs *) &procs);
		}
	else
		procs = *porigprocs; /*start with current state*/
	
	#if 0 //def THINK_C
	
		procs.textProc = (QDPtr) update_stdtext; /*patch in our own procs*/
		
		procs.txMeasProc = (QDPtr) update_stdtxmeas;
		
	#else
	
		procs.textProc = update_stdtextUPP; /*patch in our own procs*/
		
		procs.txMeasProc = update_stdtxmeasUPP;
	
	#endif
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:28:13 PM
	//Changed to Opaque call for Carbon
	//This needs to be tested.
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	SetPortGrafProcs(thePort, &procs);
	#else
	//old code
	(*w).grafProcs = (QDProcs *) &procs;
	#endif
#endif
	} /*pictupdatepatcher*/


void pictdepatcher (void) {
	
#ifdef MACVERSION
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:32:15 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	CGrafPtr thePort = GetQDGlobalsThePort();
	SetPortGrafProcs(thePort, porigprocs);
	#else
	//old code
	(*getport ()).grafProcs = (QDProcs *) porigprocs;
	#endif
#endif
	} /*pictdepatcher*/


void pictupdate (void) {
	
	register hdlpictrecord hp = pictdata;
	register PicHandle macpicture = (**hp).macpicture;
	register hdlwindowinfo hw = pictwindowinfo;
	register boolean flbitmap = false;
	register boolean fleval;
	Rect rcontent;
	Rect rframe;
	Rect rintersection;
	boolean flintersect;
	short extrawidth;
	short extraheight;
	
	rcontent = (**hw).contentrect;
	
	if (macpicture == nil) {
	
		eraserect (rcontent);
		
		return;
		}
	
	pictgetframerect (hp, &rframe);
	
	offsetrect (&rframe, -rframe.left, -rframe.top); /*make zero-based*/
	
	extrawidth = max (0, (rcontent.right - rcontent.left) - rframe.right);
	
	extraheight = max (0, (rcontent.bottom - rcontent.top) - rframe.bottom);
	
	offsetrect (&rframe, extrawidth / 2, extraheight / 2);
	
	offsetrect (&rframe, -(**hw).horizscrollinfo.cur, -(**hw).vertscrollinfo.cur);
	
	flintersect = intersectrect (rcontent, rframe, &rintersection);
	
	if ((**hp).flbitmapupdate) {
		
		if (flintersect) 
			flbitmap = openbitmap (rintersection, pictwindow);
		}
	
	if (!flbitmap)
		pushclip (rintersection);
	
	fleval = (**hp).flevalexpressions;
	
	if (fleval) 
		pictupdatepatcher (); /*install our quickdraw patches*/
	
	DrawPicture (macpicture, &rframe);
	
	if (fleval) 
		pictdepatcher (); /*de-install 'em*/
	
	if (flbitmap)
		closebitmap (pictwindow);
	else
		popclip ();
	
	(**hp).timelastupdate = gettickcount ();
	} /*pictupdate*/
	
	
void pictidle (void) {
	
	register hdlpictrecord hp = pictdata;
	register short updateticks;
	
	updateticks = (**hp).updateticks;
	
	if (updateticks == 0) /*feature disabled for this pict*/
		return;
	
	if (gettickcount () > ((**hp).timelastupdate + updateticks)) 
		pictupdate ();
	} /*pictidle*/


void pictdirty (void) {
	
	/*
	2.1b4 dmb: update timelastsave on each change
	*/
	
	register hdlpictrecord hp = pictdata;
	
	(**hp).fldirty = true;
	
	(**hp).timelastsave = timenow (); /*modification time until saved*/
	} /*pictdirty*/


void pictscheduleupdate (short updateseconds) {
	
	(**pictdata).updateticks = updateseconds * 60;
	
	pictdirty ();
	} /*pictscheduleupdate*/
	
	
void pictsetbitmapupdate (boolean flbitmap) {
	
	(**pictdata).flbitmapupdate = flbitmap;
	
	pictdirty ();
	} /*pictsetbitmapupdate*/


void pictsetevaluate (boolean flevaluate) {
	
	(**pictdata).flevalexpressions = flevaluate;
	
	pictdirty ();
	
	windowsetchanges (pictwindow, true);
	
	windowinval (pictwindow);
	} /*pictsetevaluate*/


void pictgetnewwindowrect (hdlpictrecord hpict, Rect *rwindow) {
	
	/*
	a little bit of semi-dirty code that makes things nice for the user.
	
	if the pict hasn't been opened before, we custom-fit the window so that
	the picture fits perfectly inside it.
	
	it isn't really dirty because we follow all the rules of the shell, and
	it can handle any style scrollbars.
	*/
	
	Rect r;
	tyconfigrecord origconfig;
	register short scrollbarwidth;
	
	r = (**hpict).windowrect; 
	
	if (r.top != -1) { /*it's been opened before*/
		
		*rwindow = r;
		
		return;
		}
	
	if (!pictgetframerect (hpict, &r)) { /*nil picture*/
		
		*rwindow = r;
		
		return;
		}
	
	/*it's never been opened, fit window to picture*/
	
	origconfig = config; /*we preserve this*/
	
	shellgetconfig (idpictconfig, &config);
	
	scrollbarwidth = getscrollbarwidth ();
	
	config = origconfig; /*restore*/
	
	r.right += scrollbarwidth;
	
	r.bottom += scrollbarwidth;
	
	centerrectondesktop (&r);
	
	*rwindow = r;
	} /*pictgetnewwindowrect*/
	
	


	
