
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

#include "PAIGE.H"
#include "PGTRAPS.H"
#include "PGEXCEPS.H"
#include "DEFPROCS.H"
#include "MACHINE.H"
#include "pgHLevel.h"

#include "standard.h"

#include "bitmaps.h"
#include "cursor.h"
#include "error.h"
#include "font.h"
#include "kb.h"
#include "memory.h"
#include "mouse.h"
#include "quickdraw.h"
#include "strings.h"
#include "ops.h"
#include "file.h"
#include "resources.h"
#include "scrap.h"
#include "search.h"
#include "timedate.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "shellprint.h"
#include "shellundo.h"
#include "wpengine.h"
#include "wpinternal.h"
#include "op.h"
#include "opinternal.h"


#define flrulers false /*set to false to remove all rulers add-on package calls*/

#define lenfontname 32 /*number of chars reserved for font name*/

#define scrollquantum 16

#define i2fix(i) FixRatio (i, 1)

#define fix2i(f) FixRound (f)

#define defaultpgflags (NO_SMART_CUT_BIT | NO_LF_BIT | NO_HIDDEN_TEXT_BIT)

#define wpdisplay_way (wpdisplayenabled ()? best_way : draw_none)


WindowPtr wpwindow = nil;

hdlwprecord wpdata = nil;

hdlwindowinfo wpwindowinfo = nil;

pg_ref wpbuffer = MEM_NULL;

static hdlregion wpselectionrgn = nil;

static pgm_globals mem_globals;

static pg_globals ws_globals;

// static wpeditingnow = 0;

// static Handle hlastundodata = nil;

#ifdef WIN95VERSION
	static boolean fldestroycaret = false; /*7.0b16 PBS: global for destroying caret in main thread on Windows.*/
#endif


#define ctwpstack 5 /*we can remember wp contexts up to 5 levels deep*/

static short topwpstack = 0;

static hdlwprecord wpstack [ctwpstack];


typedef struct tywpheader { /*format of text item header stored on disk*/
	
	short versionnumber; /*important, this structure is saved on disk*/
	
	long timecreated, timelastsave;
	
	long ctsaves; /*the number of times the structure has been saved on disk*/
	
	char fontname [lenfontname]; /*the default font for the text*/
	
	short fontsize, fontstyle;
	
	long maxpos;
	
	short unused [4]; /*unused stuff*/
	
	short flags;
	
	diskrect windowrect; /*the size of the window that holds the edit record*/
	
	long varlistsize; /*variable list*/
	
	long buffersize; /*number of chars in pg_ref, stored after this header record*/
	
	char waste [52]; /*room to grow*/
	} tywpheader;

#ifdef MACVERSION
	#define floneline_mask	0x8000 /*set if this is a single-line, false otherwise*/
	#define flruleron_mask	0x4000 /*set if the ruler should be displayed*/
	#define flexpandvariables_mask	0x2000 /*expand variables into text?*/
	#define flhilitevariables_mask	0x1000 /*show visual indication of variable text?*/
#endif

#ifdef WIN95VERSION
	#define floneline_mask	0x0080 /*set if this is a single-line, false otherwise*/
	#define flruleron_mask	0x0040 /*set if the ruler should be displayed*/
	#define flexpandvariables_mask	0x0020 /*expand variables into text?*/
	#define flhilitevariables_mask	0x0010 /*show visual indication of variable text?*/
#endif


typedef struct tyOLD42wpheader { /*format of text item header stored on disk*/
	
	short versionnumber; /*important, this structure is saved on disk*/
	
	long timecreated, timelastsave;
	
	long ctsaves; /*the number of times the structure has been saved on disk*/
	
	char fontname [lenfontname]; /*the default font for the text*/
	
	short fontsize, fontstyle;
	
	long maxpos;
	
	short unused [4]; /*unused stuff*/
	
	boolean floneline: 1; /*true if this is a single-line, false otherwise*/
	
	boolean flruleron: 1; /*true if the ruler should be displayed*/
	
	boolean flexpandvariables: 1; /*expand variables into text?*/
	
	boolean flhilitevariables: 1; /*show visual indication of variable text?*/
	
	diskrect windowrect; /*the size of the window that holds the edit record*/
	
	long varlistsize; /*variable list*/
	
	long buffersize; /*number of chars in pg_ref, stored after this header record*/
	
	char waste [52]; /*room to grow*/
	} tyOLD42wpheader;


typedef struct tyselectinfo { /*for undo*/
	
	long startsel, endsel; /*beginning and end of selection*/
	
	long origsel; /*needed for forward delete in undoinsert*/
	
	boolean flreselect: 1; /*re-select text after un-deleting?*/
	
	boolean flrecalc: 1; /*recalc after undoing?*/
	
	hdlwprecord hwp; /*the wp record that owns this undo*/
	
	union {
		
		Handle hseldata;
		
		pg_ref hwpsel;
		
	//	rulerHandle hrulers;
		
	//	FormatHandle hformat;
		} u;
	} tyselectinfo, *ptrselectinfo, **hdlselectinfo;


static boolean wpinserttexthandle (pg_ref, Handle, short, boolean);


#ifdef MACVERSION
#pragma mark === low-level ===
#endif

static PG_PASCAL (long) wpcharinfoproc (
		paige_rec_ptr pg, style_walk_ptr style_walker,
		pg_char_ptr data, long block_offset, long offset_begin, 
		long offset_end, long char_offset, long mask_bits) {
	
	/*
	if (result & WORD_SEL_BIT)
		...
		result = ((result | !WORD_SEL_BIT) & INCLUDE_SEL_BIT)
	*/
	
	long result;
	
	result = pgCharInfoProc (pg, style_walker, data, block_offset, offset_begin, offset_end, 
				char_offset, mask_bits);
	
	if (mask_bits & (WORD_SEL_BIT | INCLUDE_SEL_BIT)) {
		
		switch (data [char_offset]) {
			
			case '.':
				if ((char_offset > 0) && isalnum (data [char_offset + 1])) {
				
					result &= ~WORD_SEL_BIT;
					result |= INCLUDE_SEL_BIT;
					}
				
				break;
			
			case '^':
			case '{': case '}':
			case '[': case ']':
				result &= ~INCLUDE_SEL_BIT;
				result |= WORD_SEL_BIT;
				
				break;
			}
		}
	
	return (result);
	} /*wpcharinfoproc*/


boolean wpinit (void) {
	
	pgMemStartup (&mem_globals, 0);
	
	pgInit (&ws_globals, &mem_globals);
	
//	pgInitEmbedProcs (&ws_globals, NULL, NULL);
	
	ws_globals.def_style.procs.char_info = (char_info_proc) wpcharinfoproc;

	wpselectionrgn = NewRgn ();
	
	return (true);
	} /*wpinit*/


void wpshutdown (void) {
	
	pgShutdown (&ws_globals);
	
	pgMemShutdown (&mem_globals);
	} /*wpshutdown*/


static void setdefaultstyles (boolean flprinting) {
#ifdef MACVERSION
#	pragma unused (flprinting)
#endif

	short font, size, style;
	bigstring bsfont;
	
	getfontsizestyle (&font, &size, &style);
	
	fontgetname (font, bsfont);

	pgFillBlock(ws_globals.def_font.name, FONT_SIZE * sizeof(pg_char), 0);
	pgBlockMove (bsfont, ws_globals.def_font.name, stringsize (bsfont));
	
	#ifdef MACVERSION
		ws_globals.def_style.point = (long) size << 16;     // Fixed data type
	#endif
	#ifdef WIN95VERSION
		// 6.0b3: scale pointsize when printing
		if (flprinting)
			ws_globals.def_style.point = MulDiv(size, shellprintinfo.scaleMult, shellprintinfo.scaleDiv) << 16;
		else
			ws_globals.def_style.point = (long) size << 16; // Fixed data type
	#endif

	ws_globals.def_style.char_width = 0;
	
	QDStyleToPaige (style, &ws_globals.def_style);
	} /*setdefaultstyles*/


boolean wpsetglobals (void) {
	
	register hdlwprecord hwp = wpdata;
	register pg_ref hbuf;
	
	if (hwp == nil)
		return (false);
	
	hbuf = (pg_ref) (**hwp).wpbuffer;
	
	wpbuffer = hbuf; /*set global*/
	
	return (hbuf != nil);
	} /*wpsetglobals*/


static boolean wpdisplayenabled (void) {
	
	register hdlwprecord hwp = wpdata;
	
	if (wpdata == NULL)
		return (false);

	return ((wpwindow != nil) && (**hwp).flwindowopen && !(**hwp).flinhibitdisplay);
	} /*wpdisplayenabled*/


static undo_ref getundo (void) {

	return ((undo_ref) (**wpdata).hlastundodata);
	} /*getundo*/


static void setundo (undo_ref undo) {
	
	/*
	12/10/96 dmb: we must maintain a flag in the undo itself, saying that 
	we own it. this will prevent it from being disposed.
	*/
	
	(**wpdata).hlastundodata = (void *) undo;
	} /*setundo*/


static void wpdisposeundo (hdlwprecord hwp) {
	
	/*
	divorce the wprecord from its lastinsertundo data.  one of them is 
	about to be disposed, and lingering attraction can lead to heap crash
	*/
	
	undo_ref hundo;
	
	if (hwp != nil) {
		
		hundo = (undo_ref) (**hwp).hlastundodata;
		
		if (hundo != nil) { /*there was a linked undo*/
			
			pgDisposeUndo (hundo);
			
			(**hwp).hlastundodata = nil;
			}
		}
	} /*wpdisposeundo*/


void wpresettyping (void) {
	
	/*
	call this routine when subsequent typing should be considered a new 
	operation, seperate from typing changes that have already occured.  this 
	should happen any time the user moves the insertion point with the mouse
	or cursor keys, or after certain other occurances.
	
	our task is to "close the book" on the undo we've been building by noting 
	the final insertion point.  we then remove our link to the undo stack 
	frame by clearing hlastinsertundodata
	
	12/6/96 dmb: paige implementation
	*/
	
	(**wpdata).flstartedtyping = false;
	} /*wpresettyping*/


static boolean wpownwindowinfo (void) {
	
	/*
	return true if the edit environment is "owned" by the word processor.
	*/
	
	if (wpwindowinfo == nil)
		return (false);
	
	return ((**wpwindowinfo).configresnum == idwpconfig);
	} /*wpownwindowinfo*/


static void wpdirty (boolean flchangedtext) {
	
	/*
	9/14/91 dmb: added flchangedcontent parameter
	
	12/4/91 dmb: don't dirty windowinfo if it's not our own
	
	2.1b4 dmb: update timelastsave on each change
	*/
	
	register hdlwprecord hwp = wpdata;
	
	if (!flchangedtext)
		(**hwp).fldirtyview = true;
	
	else {
		
		(**hwp).fldirty = true; /*the structure is dirty*/
		
		(**hwp).timelastsave = timenow (); /*modification time until saved*/
		
		if (wpownwindowinfo ()) /*the window is dirty*/
			windowsetchanges (wpwindow, true);
		
		(*(**hwp).dirtyroutine) ();
		}
	} /*wpdirty*/


static void wpdirtyselectioninfo (void) {
	
	/*
	make sure the ruler & menus get updated to reflect a formatting change.
	*/
	
	if ((wpwindowinfo != nil) && wpdisplayenabled ())
		(**wpwindowinfo).selectioninfo.fldirty = true;
	} /*wpdirtyselectioninfo*/


boolean wppushdata (hdlwprecord hwp) {
	
	/*
	when you want to temporarily work with a different wprecord, call this
	routine, do your stuff and then call wppopdata.
	*/
	
	if (topwpstack >= ctwpstack) {
		
		shellinternalerror (idwpstackfull, STR_wp_stack_overflow);
		
		return (false);
		}
	
	wpstack [topwpstack++] = wpdata;
	
	wpdata = hwp;
	
	return (true);
	} /*wppushdata*/
		

boolean wppopdata (void) {
	
	if (topwpstack <= 0)
		return (false);
	
	wpdata = wpstack [--topwpstack];
	
	return (true);
	} /*wppopdata*/


#ifdef MACVERSION
#pragma mark === display ===
#endif


static shape_ref newshape (const Rect *r) {
	
	rectangle bounds;
	
	RectToRectangle ((Rect *) r, &bounds);
	
	return (pgRectToShape (&mem_globals, &bounds));
	} /*newshape*/

static void wppushdraw (void) {
	
	/*
	7/26/91 dmb: make sure that now drawing occurs to the right of tBounds
	*/
	
//	Rect rclip;

	if (wpdisplayenabled ()) {
		
		//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
		//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	
		#ifdef MACVERSION

			CGrafPtr thePort;

			#if TARGET_API_MAC_CARBON == 1
			thePort = GetWindowPort(wpwindow);
			#else
			thePort = (CGrafPtr)wpwindow;
			#endif

			pushport (thePort);

		#endif

		#ifdef WIN95VERSION

			pushport (wpwindow);

		#endif

		
	/*
		rclip = (**wpdata).wprect;
		
		rclip.right = min (rclip.right, (**wpbuffer).tBounds.right);
		
		pushclip (rclip);
	*/
		}
	} /*wppushdraw*/


static void wppopdraw (void) {
	
	if (wpdisplayenabled ()) {
		
	/*
		popclip ();
		
		#ifdef flwpcolor
		
		RGBForeColor (&blackcolor);
		
		#endif
	*/
		
		popport ();
		}
	} /*wppopdraw*/


short wpavailwidth (void) {
	
	/*
	return the available width for wrapping text on the printed page
	
	for now, we just assume 1-inch of margins (total)
	*/
	
	return (shellprintinfo.paperrect.right - shellprintinfo.paperrect.left);
	} /*wpavailwidth*/


static void wpsetautoscroll (pg_ref hbuf, boolean flauto) {
	
	paige_rec_ptr pg = (paige_rec_ptr) UseMemory (hbuf);
	
	if (flauto)
		(*pg).flags2 &= ~(NO_HAUTOSCROLL | NO_VAUTOSCROLL);
	else
		(*pg).flags2 |= (NO_HAUTOSCROLL | NO_VAUTOSCROLL);
	
	UnuseMemory (hbuf);
	} /*wpsetautoscroll*/


static void wpvisicursor (void) {
	
	/*
	7/9/92 dmb: when selection isn't empty, visi endsel
	
	12/6/96 dmb: paige
	
	5.0a3 dmb: reset scrollbars
	*/
	
	register hdlwprecord hwp = wpdata;
	
	if ((**hwp).flneverscroll)
		return;
	
	if (!wpdisplayenabled ()) {
		
		(**hwp).flcheckvisi = true; /*defer*/
		
		return;
		}
	
	wpsetautoscroll (wpbuffer, true);

	pgScrollToView (wpbuffer, CURRENT_POSITION, 0, 0, TRUE, wpdisplay_way);
	
	wpsetautoscroll (wpbuffer, false);

	wpresetscrollbars ();
	
	(**hwp).flcheckvisi = false;
	} /*wpvisicursor*/


static boolean wpisactive () {
	
	/*
	wpsetglobals must have already been called
	*/
	
	short front_back_state, perm_state;
	
	pgGetHiliteStates (wpbuffer, &front_back_state, &perm_state);
	
	return (front_back_state == activate_verb);
	} /*wpisactive*/


/*
static void getlongbounds (register Rect *r, register rectangle *bounds) {
	
	(*bounds).top = (*r).top;
	
	(*bounds).bottom = (*r).top + (shellprintinfo.paperrect.bottom - shellprintinfo.paperrect.top);
	
	(*bounds).left = (*r).left;
	
	(*bounds).right = (*r).left + wpavailwidth ();
	} /%getlongbounds%/
*/


boolean wpgetdisplay (void) { /*unused 8/29/91*/
	
	/*
	return true if WS-Engine's display is enabled.
	*/
	
	register hdlwprecord hwp;
	
	if (!wpsetglobals ())
		return (false);
	
	hwp = wpdata; /*move into register*/
	
	return ((**hwp).flwindowopen && !(**hwp).flinhibitdisplay);
	} /*wpgetdisplay*/


boolean wpsetdisplay (boolean fldisplay) {
	
	/*
	enable or display wp display.  return true if state is changed.
	*/
	
	if (!wpsetglobals ())
		return (false);
	
	if ((**wpdata).flinhibitdisplay != fldisplay) /*nothing to do, no change*/
		return (false);
	
	(**wpdata).flinhibitdisplay = !fldisplay;
	
	if (fldisplay)
		wptotalrecalc ();
	
	return (true);
	} /*wpsetdisplay*/


boolean wpsetruler (register boolean flruler) {
	
	/*
	make sure the ruler is turned on if flruler is true, make sure it is turned
	off if flruler is false.
	
	return true if the state of the ruler changed.
	
	6/25/92 dmb: return false if flonelist is set
	*/
	
	register hdlwprecord hwp = wpdata;
	
	if (!wpsetglobals ())
		return (false);
	
	if ((**hwp).floneline)
		return (false);
	
	if ((**hwp).flshowruler == flruler)
		return (false);
	
	(**hwp).flshowruler = flruler;
	
	#if flrulers
	
	wpdirty (false);
	
	#endif
	
	return (true);
	} /*wpsetruler*/


boolean wpgetruler (void) {
	
	/*
	return true if WS-Engine's ruler is on.
	*/
	
	if (!wpsetglobals ())	// check for nil
		return (false);
	
	return ((**wpdata).flshowruler);
	} /*wpgetruler*/


static short wpgetrulerheight (void) {
	
	#if flrulers
	
	// ***
		
	#endif
	
	return (0);
	} /*wpgetrulerheight*/


static void wpsetavailbounds (pg_ref pg) {
	
	/*
	5.0a2 dmb: implemented correctly

	5.0b9 dmb: take pg as parameter instead of using global
	*/
	
	rectangle pagebounds;
	
	pgAreaBounds (pg, &pagebounds, NULL);
	
	pagebounds.bot_right.h = pagebounds.top_left.h + wpavailwidth ();
	
	pgSetAreaBounds (pg, &pagebounds, NULL);
	} /*wpsetavailbounds*/


void wpsetupwindow (void) {
	
	graf_device	port;
	
	if (wpsetglobals () && wpwindow) {

		#ifdef MACVERSION
			#if TARGET_API_MAC_CARBON
			CGrafPtr thePort = GetWindowPort(wpwindow);
			pgInitDevice (&ws_globals, thePort, 0, &port);
			#else
			pgInitDevice (&ws_globals, wpwindow, 0, &port);
			#endif
		#endif
	
		#ifdef WIN95VERSION
			pgInitDevice (&ws_globals, (generic_var) wpwindow, MEM_NULL, &port);
		#endif
	
		pgSetDefaultDevice (wpbuffer, &port);
		}
	} /*wpsetupwindow*/


static boolean wpadjustboundstofit (void) {

	/*
	if we're editing a single line, and scrolling is allowed, sise
	the bounds to the text width. return true if we adjust it.
	*/

	register hdlwprecord hwp = wpdata;

	if ((**hwp).flalwaysmeasuretext) { // set bounds to fit text
		
		rectangle linebounds, curbounds, visbounds;
		long width, height;
		
		pgMaxTextBounds (wpbuffer, &linebounds, false);
		
		pgAreaBounds (wpbuffer, &curbounds, &visbounds);
		
		width = linebounds.bot_right.h - linebounds.top_left.h;
		
		height = linebounds.bot_right.v - linebounds.top_left.v;
		
		(**hwp).vpixels = height;
		
		(**hwp).hpixels = width;
		
		if ((**hwp).floneline) {
			
			width = max (width, visbounds.bot_right.h - visbounds.top_left.h);
			
			if (width != curbounds.bot_right.h - curbounds.top_left.h) {
				
				curbounds.bot_right.h = curbounds.top_left.h + width;
				
				pgSetAreaBounds (wpbuffer, &curbounds, NULL);

				return (true);
				}
			}
		else {
				
			if (height != curbounds.bot_right.v - curbounds.top_left.v) {
				
				curbounds.bot_right.v = curbounds.top_left.v + height;
				
				pgSetAreaBounds (wpbuffer, &curbounds, NULL);

				return (true);
				}
			}
		}
	
	return (false);
	} /*wpajustboundstofit*/


boolean wpsetbufferrect (Rect rclip, Rect r) {
	
	/*
	5.0.1 dmb: adjust bounds to fit text here, so we can reajust 
	in posstedit, not preedit
	*/

	rectangle bounds;
	shape_ref cliprgn;
	shape_ref boundsrgn;
	
	if (!wpsetglobals ())
		return (false);
		
	r.top += wpgetrulerheight ();
	
	RectToRectangle (&r, &bounds);
	
	boundsrgn = newshape (&r);
	
	cliprgn = newshape (&rclip);
	
	pgInsertPendingKeys (wpbuffer);
	
	pgSetCursorState (wpbuffer, hide_cursor);
	
	pgSetAreas (wpbuffer, cliprgn, boundsrgn, MEM_NULL);
	
	pgDisposeShape (boundsrgn);
	
	pgDisposeShape (cliprgn);
	
	pgPaginateNow (wpbuffer, CURRENT_POSITION, FALSE);
	
	(**wpdata).wprect = rclip;
	
	wpadjustboundstofit ();

	wpresetscrollbars (); // 5.0a3 dmb
	
	return (true);
	} /*wpsetbufferrect*/


boolean wphidecursor (void) {

	/*
	make sure the cursor is hidden, probably in preparation for a 
	non-paige display operation
	*/

	if (!wpsetglobals ())
		return (false);

	pgSetCursorState (wpbuffer, hide_cursor);

	return (true);
	} /*wphidecursor*/


static void wpframedisplay (void) {
	
	} /*wpframedisplay*/


static void wpframeselection (void) {

	if (wpdisplayenabled ()) {
		
	//	#ifdef MACVERSION
		
		rectangle bounds;
		shape_ref rgn;
		Rect r;
		
		if (pgCaretPosition (wpbuffer, CURRENT_POSITION, &bounds)) {
			
			RectangleToRect (&bounds, NULL, &r);
			
			#ifdef MACVERSION
				RectRgn (wpselectionrgn, &r);
			#endif
			#ifdef WIN95VERSION
				wpselectionrgn = CreateRectRgn (r.left, r.top, r.right, r.bottom);
			#endif
			}
		else {
		
			rgn = pgRectToShape (&mem_globals, MEM_NULL);
			
			pgGetHiliteRgn (wpbuffer, MEM_NULL, MEM_NULL, rgn);
			
			ShapeToRgn (rgn, 0, 0, NULL, 0, NULL, wpselectionrgn);
			
			pgDisposeShape (rgn);
			}
		
		grayframerrgn (wpselectionrgn);
		
	//	#endif
		}
	} /*wpframeselection*/


/*
static short wpgetcurrentscreenlines (short line1) {
	
	register hdlwprecord hwp = wpdata;
	co_ordinate pt;
	long offset, lastline;

	pt.h = (**hwp).wprect.left;
	
	pt.v = (**hwp).wprect.bottom - 1;

	offset = pgPtToChar (wpbuffer, &pt, NULL);
	
	lastline = pgOffsetToLineNum (wpbuffer, offset, true);
	
	return (lastline - line1 + 1);
	} /%wpgetcurrentscreenlines%/
*/


static boolean wpgetscrollbarinfo (void) {
	
	register hdlwprecord hwp = wpdata;
	tyscrollinfo vertinfo, horizinfo;
	rectangle curbounds;
	short horizcur, vertcur, horizmax, vertmax;
	short unit_h, unit_v, append_h, append_v;
	
	if (!wpsetglobals ())
		return (false);
	
	if (!pgGetScrollValues (wpbuffer, &horizcur, &vertcur, &horizmax, &vertmax))
		return (false);
	
	pgGetScrollParams (wpbuffer, &unit_h, &unit_v, &append_h, &append_v);
	
	horizinfo.max = horizmax;
	horizinfo.cur = horizcur;
	
	vertinfo.max = vertmax;
	vertinfo.cur = vertcur;
	
	horizinfo.min = vertinfo.min = 0;

	if ((**hwp).floneline)
		vertinfo.max = 0;
	
	pgAreaBounds (wpbuffer, &curbounds, NULL);
	
	horizinfo.pag = curbounds.bot_right.h - curbounds.top_left.h;

	if (unit_h > 0)
		horizinfo.pag /= unit_h;
	
	vertinfo.pag = curbounds.bot_right.v - curbounds.top_left.v;
	
	if (unit_v > 0)
		vertinfo.pag /= unit_v;
	//	vertinfo.pag = wpgetcurrentscreenlines (vertinfo.cur);
	
	(**hwp).horizscrollinfo = horizinfo;
	
	(**hwp).vertscrollinfo = vertinfo;
	
	return (true);
	} /*wpgetscrollbarinfo*/


static void wpcheckscrollpos (void) {
	
	/*
	called after an edit, we're making sure that, in case a lot of text 
	was deleted, the whole document isn't scrolled up above the screen
	
	normal visiing won't account for this, since our internal scroll position 
	is based on absolute values
	
	12/6/96 dmb: paige implementation is easy!
	*/
	
	pgAdjustScrollMax (wpbuffer, wpdisplay_way);
	} /*wpcheckscrollpos*/


static void wpredrawscrollbars (void) {
	
	(*(**wpdata).setscrollbarsroutine) ();
	} /*wpredrawscrollbars*/


void wpresetscrollbars (void) {
	
	if (!wpsetglobals ())
		return;
	
	if (!(**wpdata).flneverscroll) {
		
		wpgetscrollbarinfo ();
		
		wpredrawscrollbars ();
		}
	} /*wpresetscrollbars*/


boolean wpgetcontentsize (long *width, long *height) {
	
	/*
	12/13/91 dmb: account for ruler height
	*/
	
	rectangle bounds;
	
	if (!wpsetglobals ())
		return (false);
	
	pgMaxTextBounds (wpbuffer, &bounds, true);
	
	*width = bounds.bot_right.h - bounds.top_left.h + 10; /*leave some gray*/
	
	*height = bounds.bot_right.v - bounds.top_left.v + wpgetrulerheight ();
	
	return (true);
	} /*wpgetcontentsize*/


static pg_ref wpnewpg (Handle htext, const Rect *rclip, const Rect *rbounds, tywpflags wpflags, boolean fldisplay) {
	
	/*
	create a new pg rec containing the given text

	7.1b41 PBS: Fix bug with large fonts settings on Windows. Don't use USE_NO_DEVICE unless
	currentport is nil. Use old code on Macs (to be conservative, since there's no bug there.
	*/
	
	long flags = defaultpgflags | NO_EDIT_BIT;
	shape_ref visarea, pagearea;
	pg_ref pg = MEM_NULL;
	short draw_mode;
	generic_var device;
	
	#ifdef WIN95VERSION /*PBS 7.1b41: fix large fonts bug.*/

		device = (generic_var) getport ();

		if (fldisplay)		
			draw_mode = best_way;
			
		else {

			if (device == MEM_NULL)
				device = USE_NO_DEVICE;

			draw_mode = draw_none;
			} /*else*/

	#endif

	#ifdef MACVERSION /*PBS 7.1b41: old code, since there's no bug on Macs.*/

		if (fldisplay) {
	
			device = (generic_var) getport ();
			
			draw_mode = best_way;
			} /*if*/
			
		else {

			device = USE_NO_DEVICE;

			draw_mode = draw_none;
			} /*else*/

	#endif


	if ((wpflags & wponeline) != 0)
		flags |= NO_WRAP_BIT; // | EXTERNAL_SCROLL_BIT));
	
	visarea = newshape (rclip);
	
	pagearea = newshape (rbounds);

	setdefaultstyles ((wpflags & wpprinting) != 0);
	
	PG_TRY (&mem_globals) {
	
		pg = pgNew (&ws_globals, device, visarea, pagearea, MEM_NULL, flags);
		}
	PG_CATCH {
	
		pgFailureDispose (pg);
		
		pg = MEM_NULL;
		
		memoryerror ();
		}
	PG_ENDTRY;
	
	pgDisposeShape (visarea);
	
	pgDisposeShape (pagearea);
	
	if ((pg != MEM_NULL) && (htext != nil)) {
	
		if (!wpinserttexthandle (pg, htext, draw_mode, false)) {
			
			pgDispose (pg);
			
			pg = MEM_NULL;
			}
		}
	
	return (pg);
	} /*wpnewpg*/


static boolean wpapplyhtmlstyles (Handle htext, pg_ref pg, boolean flredraw) {

	/*
	7.0b28 PBS: Apply HTML styles to the selection. This routine understands bold, italic,
	underline, and links. Hide the HTML tags and format the enclosed tag as if in a
	web browser.

	If pg is nil, the caller just wants to know if this routine *would* apply HTML
	styles. In that case, don't actually apply any styles; return just before the first
	would-be application. If no styles would be applied, return false.
	*/

	long sizeopenstack = 0;
	long stackopenleft [256];
	long stackopenright [256];
	char stackopentag [256];
	char tagvalue, lasttagvalue = 0;
	char ch;
	long i = 0;
	long lentext;
	boolean flintag = false;
	boolean flokaytag = false;
	long ixleft, ixright;
	select_pair range, deleterange;
	boolean flstyleapplied = false;
	RGBColor linkcolor;
	
	linkcolor = darkbluecolor;
	
	#ifdef WIN95VERSION
		linkcolor = darkgreencolor;
	#endif

	lentext = gethandlesize (htext);

	while (true) {
	
		ch = (*htext) [i];
	
		switch (ch) {

			case '<':

				if (i + 2 > lentext)
					break;

				if ((*htext) [i + 1] == '/') {
					
					if (i + 3 > lentext)
						break;
				
					tagvalue = (*htext) [i + 2];
				
					tagvalue = tolower (tagvalue);
				
					if ((tagvalue == 'a') || (tagvalue == 'b') || (tagvalue == 'i') || (tagvalue == 'u')) {
					
						if ((*htext) [i + 3] == '>') {
						
							if (sizeopenstack > 0) {
							
								if (tagvalue == stackopentag [sizeopenstack]) {
								
									flstyleapplied = true;
									
									if ((void *)pg == NULL) /*Caller wants to know if styles *would* be applied.*/
										goto exit;

									deleterange.begin = i;

									deleterange.end = i + 4;

									pgSetStyleBits (pg, 0x400, -1, &deleterange, false);

									ixleft = stackopenleft [sizeopenstack];
								
									ixright = stackopenright [sizeopenstack];

									deleterange.begin = ixleft;

									deleterange.end = ixright + 1;

									pgSetStyleBits (pg, 0x400, -1, &deleterange, false);
							
									sizeopenstack--;
								
									lentext = gethandlesize (htext);

									range.begin = ixright + 1;

									range.end = i;

									lasttagvalue = tagvalue;

									if (tagvalue == 'b')
		
										pgSetStyleBits (pg, bold, bold, &range, false);
									
									if (tagvalue == 'i')
										
										pgSetStyleBits (pg, italic, italic, &range, false);

									if (tagvalue == 'u')
										
										pgSetStyleBits (pg, underline, underline, &range, false);

									if (tagvalue == 'a') {

										pgSetStyleBits (pg, underline, underline, &range, false);

										pgSetTextColor (pg, &linkcolor, &range, false);
										} /*if*/
									} /*if*/
								} /*if*/
							} /*if*/
						} /*if*/
					} /*if*/
		
				else {
				
					flokaytag = false;

					tagvalue = (*htext) [i + 1];
				
					tagvalue = tolower (tagvalue);
				
					if ((tagvalue == 'a') || (tagvalue == 'b') || (tagvalue == 'i') || (tagvalue == 'u')) {
					
						if (tagvalue == 'a') {
						
							if (i + 6 > lentext)
								break;

							if ((*htext) [i + 2] == ' ') {
							
								if (tolower ((*htext) [i + 3]) == 'h')
									
									flokaytag = true;
								} /*if*/
							} /*if*/

						else {
						
							if ((*htext) [i + 2] == '>')
							
								flokaytag = true;
							} /*else*/

						if (flokaytag) {
							
							flintag = true;

							sizeopenstack++;

							stackopentag [sizeopenstack] = tagvalue;

							stackopenleft [sizeopenstack] = i;
							} /*if*/
						} /*if*/
					} /*else*/

				break;

			case '>':
		
				if (flintag) {
					
					flintag = false;
				
					stackopenright [sizeopenstack] = i;
					} /*if*/
			} /*switch*/
	
		i++;

		if (i >= lentext)

			break;
		} /*while*/

	if (flredraw) { /*defer drawing until the end.*/

		if (lasttagvalue == 'b') {

			pgSetStyleBits (pg, bold, bold, &range, true);

			pgSetStyleBits (pg, bold, bold, &range, true);
			}

		
		if (lasttagvalue == 'i') {
			
			pgSetStyleBits (pg, italic, italic, &range, true);

			pgSetStyleBits (pg, italic, italic, &range, true);
			}

		if (lasttagvalue == 'u') {

			pgSetStyleBits (pg, underline, underline, &range, true);

			pgSetStyleBits (pg, underline, underline, &range, true);
			}

		if (lasttagvalue == 'a') {

			pgSetStyleBits (pg, underline, underline, &range, true);
			pgSetStyleBits (pg, underline, underline, &range, true);

			pgSetTextColor (pg, &linkcolor, &range, true);
			} /*if*/
		} /*if*/

	exit:

	return (flstyleapplied);
	} /*wpapplyhtmlstyles*/


#if 0

static boolean wpfilterstyles (Handle htext, pg_ref pg, boolean redraw) {

	/*
	7.0b28: if this is an outline of some kind, and it's in HTML mode,
	call the appropriate script.
	*/

	if (outlinedata == NULL)
		return (false);

	if ((**outlinedata).flhtml)
		return (wpapplyhtmlstyles (htext, pg, redraw));

	return (false);
	} /*wpfilterstyles*/

#endif


boolean wpmeasuretext (Handle htext, Rect *r, tywpflags wpflags) {
	
	/*
	wrapping htext to fit the width of r, set the rectangle's height to fit.
	
	if htext is shorter than r's width, set the width to fit the (one) line.
	*/
	
	pg_ref pg;
	rectangle bounds;
	
	pg = wpnewpg (htext, r, r, wpflags, false);
	
	if (pg == MEM_NULL)
		return (false);
	
	pgMaxTextBounds (pg, &bounds, true);
	
	RectangleToRect (&bounds, 0, r);
	
	pgDispose (pg);
	
	return (true);
	} /*wpmeasuretext*/


boolean wpdrawtext (Handle htext, const Rect *rclip, const Rect *rtext, tywpflags wpflags) {

	/*
	6.0a14 dmb: if updating on Win, must use wpupdate's device  logic
	*/

	pg_ref pg;
	graf_device	port;
	boolean flupdating = false;
	boolean flprinting = false;

	if (wpflags == 0)
		wpflags = defaultpgflags;
		
	flprinting = (wpflags & wpprinting) != 0;

	#ifdef WIN95VERSION

		if (shellwindowinfo != nil)
			flupdating = (**shellwindowinfo).drawrgn != nil;
	#endif
	
	pg = wpnewpg (htext, rclip, rtext, wpflags,	!flupdating && !flprinting);
	
	if (pg == MEM_NULL)
		return (false);
	
	if (flprinting) {

		#ifdef MACVERSION
			pgInitDevice (&ws_globals, shellprintinfo.printport, 0, &port);
		#endif

		#ifdef WIN95VERSION
			pgInitDevice (&ws_globals, MEM_NULL, (long) shellprintinfo.printport, &port);
		#endif
		
		//pgSetDefaultDevice (pg, &port);
			
		PG_TRY (&mem_globals)
		{
			pgDisplay (pg, &port, MEM_NULL, MEM_NULL, NULL, direct_copy);
		}
		PG_CATCH
		{
		}
		PG_ENDTRY;

	//	pgPrintToPage (pg, &port, 0, &r, best_way);
		
		pgCloseDevice (&ws_globals, &port);
		}
	else if (flupdating) {

		#ifdef WIN95VERSION
			pgInitDevice (&ws_globals, (generic_var) getcurrentDC (), MEM_NULL, &port);
			port.machine_ref3 = port.machine_var; // default place to get DC
			port.machine_var = 0; //can't GetDC from a HDC
			
			PG_TRY (&mem_globals)
			{
				pgDisplay (pg, &port, MEM_NULL, MEM_NULL, NULL, direct_copy);
			}
			PG_CATCH
			{
			}
			PG_ENDTRY;
			
			pgCloseDevice (&ws_globals, &port);
		#endif
		}
	
	pgDispose (pg);
	
	return (true);
	} /*wpdrawtext*/


boolean wpgetselrect (Rect *r) {
	
	/*
	set selrect to be the rectangle enclosing the current caret position.  
	intended to be used for autoscrolling

	dmb 12/5/96: paige
	*/
	
	shape_ref rgn;
	rectangle bounds;
	
	if (!wpsetglobals ())
		return (false);
	
	if (!pgCaretPosition (wpbuffer, CURRENT_POSITION, &bounds)) {
		
		rgn = pgRectToShape (&mem_globals, MEM_NULL);

		pgGetHiliteRgn (wpbuffer, MEM_NULL, MEM_NULL, rgn);

		pgShapeBounds (rgn, &bounds);
		
		pgDisposeShape (rgn);
		}
	
	RectangleToRect (&bounds, 0, r);
	
	return (true);
	} /*wpgetselrect*/


boolean wpgetselpoint (Point *pt) {
	
	/*
	return in pt.h the pixel offset of the left edge of the caret.  intended 
	to be passed to wpsetselpoint to restore the same position when 
	cursoring up or down through headlines

	dmb 12/5/96: paige
	*/
	
	long startsel, endsel;
	rectangle r;
	
	if (!wpsetglobals ())
		return (false);
	
	pgGetSelection (wpbuffer, &startsel, &endsel);
	
	pgCharacterRect (wpbuffer, endsel, TRUE, TRUE, &r);
	
	(*pt).v = r.top_left.v;
	
	(*pt).h = r.top_left.h;
	
	return (true);
	} /*wpgetselpoint*/


boolean wpsetselpoint (Point pt) {
	
	/*
	re-establish pt.h as the horizontal pixel offset of the cursor.  (probably 
	after cursoring up or down into a new headline

	dmb 12/5/96: paige

	5.0b8 dmb: make it work with non-oneline wps
	*/
	
	long startsel, endsel;
	long offset;
	rectangle r;
	co_ordinate point;
	
	if (!wpsetglobals ())
		return (false);
	
	point.h = pt.h;

	point.v = pt.v;
	
//	if ((**wpdata).floneline) {
	
		pgGetSelection (wpbuffer, &startsel, &endsel);
		
		pgCharacterRect (wpbuffer, endsel, TRUE, TRUE, &r);
		
		point.v = r.top_left.v + 1;
//		}
	
	offset = pgPtToChar (wpbuffer, &point, NULL);
	
	pgSetSelection (wpbuffer, offset, offset, 0, wpdisplayenabled ());
	
	return (true);
	} /*wpsetselpoint*/


boolean wpscroll (register tydirection dir, boolean flpage, long ctscroll) {
	
	/*
	12/27/91 dmb: added call to wpcheckscrollpos to make sure we correct 
	WS-Engine oddities

	5.0.1 dmb: handle zero, negative scroll amounts
	*/
	
	hdlwprecord hwp = wpdata;
	long dh = scroll_none;
	long dv = scroll_none;
	long absh = (**hwp).horizscrollinfo.cur;
	long absv = (**hwp).vertscrollinfo.cur;
	boolean fltoend = ctscroll == longinfinity;
	
	if (!wpsetglobals ()) 
		return (false);
	
	if (ctscroll == 0)
		return (true);

	if (ctscroll < 0) {
		
		ctscroll = -ctscroll;

		dir = oppositdirection (dir);
		}

	switch (dir) {
		
		case up:
			if (flpage)
				dv = -scroll_page;
			else if (fltoend)
				absv = (**hwp).vertscrollinfo.max;
			else if (ctscroll > 1)
				absv = absv + ctscroll;
			else
				dv = -scroll_unit;
			
			break;
		
		case down:
			if (flpage)
				dv = scroll_page;
			else if (fltoend)
				absv = (**hwp).vertscrollinfo.min;
			else if (ctscroll > 1)
				absv = absv - ctscroll;
			else
				dv = scroll_unit;
			
			break;
		
		case left:
			if (flpage)
				dh = -scroll_page;
			else if (fltoend)
				absh = (**hwp).horizscrollinfo.max;
			else if (ctscroll > 1)
				absh = absh + ctscroll;
			else
				dh = -scroll_unit;
			
			break;
		
		case right:
			if (flpage)
				dh = scroll_page;
			else if (fltoend)
				absh = (**hwp).horizscrollinfo.min;
			else if (ctscroll > 1)
				absh = absh - ctscroll;
			else
				dh = scroll_unit;
			
			break;
		
		default:
			/* nothing to do */
			break;
		}
	
	if ((dv == scroll_none) && (dh == scroll_none))
		pgSetScrollValues (wpbuffer, absh, absv, false, wpdisplay_way);
	else
		pgScroll (wpbuffer, dh, dv, wpdisplay_way);
	
	#if flrulers
	
	if (dh && (**hwp).flshowruler) {
		
		// *** redraw ruler
		}
	
	#endif
	
	wpresetscrollbars ();
	
	wpcheckscrollpos ();
	
	if (wpownwindowinfo ()) {

		shellupdatescrollbars (wpwindowinfo);

		pushclip ((**wpwindowinfo).contentrect);
		
		wpframedisplay ();
		
		popclip ();
		}
	
	return (true);
	} /*wpscroll*/


void wpactivate (boolean flactivate) {
	
	/*
	5.0d16 dmb: if flactivate is -1, don't outline selection; caller 
	is about to dispose wp, and doesn't want selection to show

	5.1b23 dmb: reestablish globals after destroying caret in main thread
	
	5.1.5b7 dmb: ...no, do it last; wpdata could be gone

	7.0b16 PBS: set fldestroycaret global when it should be destroyed. (Windows).
	Destroy caret from main thread, in wpidle. Because, on Windows, the caret
	can only be destroyed from the main thread.
	*/

	boolean flisactive;
//	boolean fldestroycaret;
	
	if (!wpsetglobals ())
		return;
	
	if (!wpdisplayenabled ())
		return;
	
	flisactive = wpisactive ();
	
	wppushdraw ();
	
	pushvalidrgnclip ();
	
	if (flactivate && flactivate != (boolean) -1) {
		
		if (!flisactive)
			wpframeselection ();
		
		pgSetHiliteStates (wpbuffer, activate_verb, no_change_verb, TRUE);

		#ifdef WIN95VERSION

			fldestroycaret = false; /*7.0b16 PBS: make sure the caret doesn't get destroyed*/

		#endif

		}
	else {
		pgSetHiliteStates (wpbuffer, deactivate_verb, no_change_verb, TRUE);
		
		#ifdef MACVERSION
		
			pgSetCursorState (wpbuffer, hide_cursor);

		#endif
		
		#ifdef WIN95VERSION
		
			/*
			7.0b16 PBS: destroy caret in main thread, in wpidle.
			On Windows, you can only destroy carets in the main thread.
			*/

			fldestroycaret = true;

		#endif
		
		if (flisactive)
			if (flactivate != (boolean) -1)
				wpframeselection ();
		}
	
	popclip ();
	
	if (flactivate && (**wpdata).flcheckvisi)
		wpvisicursor ();
	
	wppopdraw ();
	
	#ifdef WIN95VERSION
	//	if (fldestroycaret)
		//	shelldestoycaretinmainthread();
	#endif
	} /*wpactivate*/


void wpupdate (void) {
	
	/*
	5.0b19 dmb: I can't for the life of me figure out what's going on, but Paige 
	somehow doesn't respect the clipping region during updates. so it can redraw
	text where we can't frame the selection. to keep things reasonably in sync, 
	the best thing we can do is _not_ frame the selection during updates.
	obviously, this only affects inactive windows.

	6.0a10 dmb: ugly graf_device hacking to get Paige to use the currentportDC 
	during updates, which is set up by BeginUpdate and is not connected to a HWND
	*/

	if (wpsetglobals ()) {
		
		#ifdef gray3Dlook
			Rect rcontent = (**wpdata).wprect;
		#endif
		graf_device_ptr updateport = NULL;
		hdlregion updatergn = nil;
		
		#ifdef WIN95VERSION
		graf_device	port;
		
		updatergn = (**shellwindowinfo).drawrgn;
		
		if (updatergn != nil) {
			pgInitDevice (&ws_globals, (generic_var) getcurrentDC (), MEM_NULL, &port);
			port.machine_ref3 = port.machine_var; // default place to get DC
			port.machine_var = 0; //can't GetDC from a HDC
			updateport = &port;
			}
		#endif

		#if flrulers
		
			if (wpgetruler ())
				; // *** 
		
		#endif

		#ifdef gray3Dlook
			pushbackcolor (&whitecolor);
		
			eraserect (rcontent);
		#endif

		if (updatergn == nil)
			pgErasePageArea (wpbuffer, MEM_NULL);

		PG_TRY (&mem_globals)
		{
			pgDisplay (wpbuffer, updateport, MEM_NULL, MEM_NULL, NULL, direct_copy); /*direct_or*/
		}
		PG_CATCH
		{
		}
		PG_ENDTRY;
		
		#ifdef gray3Dlook
			popbackcolor ();
		#endif
		
		if (!wpisactive ())
			wpframeselection ();
		
		wpframedisplay ();
		
		#ifdef WIN95VERSION
			if (updatergn != nil)
				pgCloseDevice (&ws_globals, &port);
		#endif
		}
	} /*wpupdate*/


boolean wppagesetup (void) {
	
	if (!wpsetglobals ())
		return (false);
	
	wpsetavailbounds (wpbuffer);
	
	shellinvalcontent (wpwindowinfo);
	
	return (true);
	} /*wppagesetup*/


boolean wpsetprintinfo (void) {
	
	/*
	5.0fc3 dmb: implemented, but only by returning infinity.
	otherwise, we'd have to set up our bounding rectangle to 
	the print page.,but we really don't need a real number
	here; we'll just return false from wpprint when we're done.
	*/
	
	rectangle r;
	
	if (!wpsetglobals ())
		return (false);
	
	RectToRectangle (&shellprintinfo.paperrect, &r);
	
	shellprintinfo.ctpages = infinity; //pgNumPages (wpbuffer, &r);
	
	return (true);
	} /*wpsetprintinfo*/


static graf_device wpprintdevice;


boolean wpbeginprint (void) {

	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, June 19, 2000 2:21:51 PM
		//Changed to Opaque call for Carbon
		//we have trouble here. Paige expects a GrafPort, I will have to 
		//decide how to fix this when I get to the engine.
	return(false);
	//	pgInitDevice (&ws_globals, shellprintinfo.printport, 0, &wpprintdevice);
	#endif

	#ifdef WIN95VERSION
		pgInitDevice (&ws_globals, MEM_NULL, (long) shellprintinfo.printport, &wpprintdevice);
	#endif
	
	if (wpdata != nil)
		(**wpdata).printpos = 0;

	return (true);
	} /*wpbeginprint*/


boolean wpendprint (void) {
	
	pgCloseDevice (&ws_globals, &wpprintdevice);

	return (true);
	} /*wpendprint*/


boolean wpprint (short pagenumber) {
	
	/*
	4.1b6 dmb: call SetFractEnable(true) so tab columns will line up
	*/
	
	/* kw - 2005-12-05 remove this after print error correction */
#if TARGET_API_MAC_CARBON == 1
	return (false);
#else
	register hdlwprecord hwp = wpdata;
	long nextpos = 0;
	rectangle r;
	
	if (!wpsetglobals ())
		return (false);
	
	RectToRectangle (&shellprintinfo.paperrect, &r);
	
	wpresettyping ();
	
	(**hwp).flprinting = true;
	
	nextpos = pgPrintToPage (wpbuffer, &wpprintdevice, (**hwp).printpos, &r, best_way);
	
	(**hwp).flprinting = false;
	
	(**hwp).printpos = nextpos; /*copy from local, used to avoid locking handle*/
	
	return (nextpos > 0);
#endif
	} /*wpprint*/


#ifdef MACVERSION
#pragma mark === read / write ===
#endif


static 
pascal void wptrackclick (hdlwprecord wp, Point pt) {
#pragma unused(wp)

	/*
	12/16/91 dmb: update scrollbars every time.  (don't need to check dirtyness, 
	because no drawing occurs if unchanged.)
	*/
	
	rectangle visbounds;
	tydirection dir;
	Rect r;
	static unsigned long lastautoscroll = 0;
	#define autoscrollticks 5
	
	if (gettickcount () - lastautoscroll < autoscrollticks)
		return;
	
	lastautoscroll = gettickcount ();
	
	pgAreaBounds (wpbuffer, NULL, &visbounds);
	
	RectangleToRect (&visbounds, NULL, &r);
	
	if (mousecheckautoscroll (pt, r, true, &dir)) // horizontal?
		wpscroll (dir, false, 1);
	
	if (mousecheckautoscroll (pt, r, false, &dir)) // vertical?
		wpscroll (dir, false, 1);
	
	if (wpownwindowinfo ())
		shellupdatescrollbars (wpwindowinfo);
	} /*wptrackclick*/


//Code change by Timothy Paustian Wednesday, August 2, 2000 9:45:12 PM
//I had a crash with wptrackclickDesc. I found I can get away with a proc ptr.
#if TARGET_RT_MAC_CFM
	#if TARGET_API_MAC_CARBON == 1
	//we can get away with a straight proc ptr here.
		#define wptrackclickUPP (wptrackclick)
	#else	
		static RoutineDescriptor wptrackclickDesc = BUILD_ROUTINE_DESCRIPTOR (uppTrackClickProcInfo, wptrackclick);
		#define wptrackclickUPP (&wptrackclickDesc)

	#endif
#else

	#define wptrackclickUPP (&wptrackclick)
#endif




#ifdef MACVERSION
	#include <WSE.h>
#endif

#ifdef WIN95VERSION
	typedef void * UniversalProcPtr;
	#define FixRound(x) (x.whole)
	#include "WSE.h"
#endif

#define rulerheaderlen (sizeof (rulerRecord) + sizeof (rulerInfo) + 2 * sizeof (Handle))


static shape_ref newlongshape (const LongRect *r) { // r is in disk byte format
	
	rectangle bounds;
	
	bounds.top_left.h = conditionallongswap ((*r).left);
	
	bounds.top_left.v = conditionallongswap ((*r).top);
	
	bounds.bot_right.h = conditionallongswap ((*r).right);
	
	bounds.bot_right.v = conditionallongswap ((*r).bottom);
	
	return (pgRectToShape (&mem_globals, &bounds));
	} /*newlongshape*/


static boolean unpackwordsolutionenginefile (Handle hpacked, pg_ref *pg) {

	/*
	1/2/97 dmb: looking at the wse asm source, it appears that the structure 
	of a packed handle is this:
	
		[WSRec]
			ws.tHandle is _size_ of text block array
			ws.tFormat is _size_ of format rec array
		[TextStuff array]
			tx.TBLines is _size_ of line array
			tx.TBHandle is _size_ of text
			tx.TBCtl is _size of control char run
		[FormatHandle array]
		[raw data]
			the text / lines / crs pointed to by each text block, sequentially

	5.0a10 dmb: omit trailing nulls in each text block. (two in final block)

	5.0b18 dmb: only the final text block has the trailing nulls.

	5.0b18 dmb: maintain font size; shift 16 bit fixed 8 bits to become 32 bit
	*/
	
	WSRec *ws;
	TextBlock *block;
	FormatRec *format;
	char *pdata;
	long ctblocks, ctformats, ctbytes, ctchars;
	shape_ref visarea, pagearea;
	Rect r;
	
	// start by pointing to all of the relevant structures
	lockhandle (hpacked);
	
	ws = (WSRec *) *hpacked;
	
	block = (TextBlock *) ((char *) ws + sizeof (WSRec));
	
	format = (FormatRec *) ((char *) block + conditionallongswap ((long) (*ws).tHandle));
	
	pdata = (char *) format + conditionallongswap ((long) (*ws).tFormat);
	
	// create paige record
	
	diskrecttorect (&(*ws).tRect, &r);

	visarea = newshape (&r);
	
	pagearea = newlongshape (&(*ws).tBounds);
	
	PG_TRY (&mem_globals) {
	
		*pg = pgNew (&ws_globals, USE_NO_DEVICE, visarea, pagearea, MEM_NULL, defaultpgflags);
		}
	PG_CATCH {
	
		pgFailureDispose (*pg);
		
		memoryerror ();
		}
	PG_ENDTRY;
	
	pgDisposeShape (visarea);
	
	pgDisposeShape (pagearea);
	
	// loop through text blocks, retaining character data, skipping the rest
	
	ctblocks = conditionallongswap ((long) (*ws).tHandle) / sizeof (TextBlock);
	
	for  ( ; --ctblocks >= 0; ++block) {
		
		ctbytes = conditionallongswap ((long) (*block).TBHandle);
		
		ctchars = ctbytes;

		if (ctblocks == 0)
			ctchars -= 2;

		pgInsert (*pg, (pg_byte_ptr) pdata, ctchars, CURRENT_POSITION, data_insert_mode, 0, draw_none);
		
		pdata += ctbytes;
		
		pdata += conditionallongswap ((long) (*block).TBLines);
		
		pdata += conditionallongswap ((long) (*block).TBCtl);
		}
	
	// loop through formats
	
	ctformats = conditionallongswap ((long) (*ws).tFormat) / sizeof (FormatRec);
	
	for ( ; --ctformats >= 0; ++format) {
		font_info	font, fontmask;
		style_info	style, stylemask;
		select_pair range;

		pgFillBlock (&font, sizeof(font_info), 0);
		pgFillBlock (&fontmask, sizeof(font_info), SET_MASK_BITS);
		
		pgBlockMove ((*format).fName, font.name, stringsize ((*format).fName));
		
		pgFillBlock (&stylemask, sizeof (style_info), 0);
		stylemask.point = SET_MASK_BITS;
		style.point = FixRound ((*format).fPoint);
		style.point <<= 8;
		
		pgFillBlock(stylemask.styles, (short) condense_var * sizeof(short), SET_MASK_BITS);
		QDStyleToPaige(conditionalshortswap ((*format).fStyle), &style);
		
		range.begin = conditionallongswap ((*format).Pos);
		
		if (ctformats > 0)
			range.end = conditionallongswap ((*(format + 1)).Pos);
		else
			range.end = min (pgTextSize (*pg), conditionallongswap ((*ws).tLength) - 1);
		
		pgSetStyleAndFont (*pg, &range, &style, &stylemask, &font, &fontmask, draw_none);
		}
	
	if (true) {
		
		rulerRecord *ruler;
		long rulerinfolen, ctrulers;
		
		if (odd ((long) pdata))
			++pdata;
		
		rulerinfolen = gethandlesize (hpacked) - (pdata - (char *) ws);
		
		ruler = (rulerRecord *) (pdata + rulerheaderlen);
		
		ctrulers = (rulerinfolen - rulerheaderlen) / sizeof (rulerRecord);
		
		for ( ; --ctrulers >= 0; ++ruler) {
			par_info info, mask;
			select_pair range;
			
			pgFillBlock(&info, sizeof(par_info), 0);
			pgInitParMask(&mask, 0);
			
			info.justification = conditionalshortswap ((*ruler).just);
			mask.justification = SET_MASK_BITS;
			
		//	info.leading_extra = (*ruler).spacing;
		//	mask.leading_extra = SET_MASK_BITS;
			
		//	can also do left/right margins, para indent, tabs, etc.
		
			range.begin = conditionallongswap ((*ruler).rBegin);
			
			if (ctrulers > 0)
				range.end = conditionallongswap ((*(ruler + 1)).rBegin);
			else
				range.end = conditionallongswap ((*ws).tLength) - 1;
			
			pgSetParInfo (*pg, NULL, &info, &mask, draw_none);
			}
		}
	
	unlockhandle (hpacked);
	
	return (true);
	} /*unpackwordsolutionenginefile*/


static boolean wpunpacktext (Handle hpacked, pg_ref *pg) {
	
	/*
	12/26/91 dmb: call memoryerror on HandleToWS failure
	*/
	
	file_ref filemap;
	long position;
	pg_byte	*ptext;
	long len = gethandlesize (hpacked);
	pg_error ec;
	
	filemap = MemoryAlloc (&mem_globals, sizeof (pg_byte), len, 0);
	
	ptext = (pg_byte *) UseMemory (filemap);
	
	pgBlockMove (*hpacked, ptext, len);
	
	UnuseMemory (filemap);
	
	position = 0;
	
	ec = pgVerifyFile (filemap, pgScrapMemoryRead, position);
	
	if (ec != 0) {
		
		if (unpackwordsolutionenginefile (hpacked, pg))
			ec = noErr;
		}
	else {
		
		PG_TRY (&mem_globals) {
			
			*pg = pgNewShell (&ws_globals);
			
			ec = pgReadDoc (*pg, &position, NULL, 0, pgScrapMemoryRead, filemap);
			
			if (ec == noErr)
				pgSetHiliteStates (*pg, deactivate_verb, no_change_verb, false);
			}
		PG_CATCH {
			pgFailureDispose (*pg);
			
			memoryerror ();
		}
		PG_ENDTRY;
		}
	
	DisposeMemory (filemap);
		
	return (ec == noErr);
	} /*wpunpacktext*/


static boolean wppackheader (long buffersize, Handle *hpacked) {
	
	tywpheader header;
	register Handle h;
	register hdlwprecord hwp = wpdata;
	
	*hpacked = nil; /*default returned value*/
	
	if (!newclearhandle (sizeof (tywpheader), hpacked))
		return (false);
	
	clearbytes (&header, sizeof (header)); /*assure all bits set to 0*/
	
	header.versionnumber = conditionalshortswap (1);
	
	/*timestamp (&header.timelastsave);*/ /*dmb 4.1b13: don't stamp it; wpdirty sets it as true mode date*/
	
	header.timecreated = conditionallongswap ((**hwp).timecreated);
	
	header.timelastsave = conditionallongswap ((**hwp).timelastsave);
	
	header.ctsaves = ++(**hwp).ctsaves;
	
	wpgetmaxpos (&header.maxpos);
	
	/* can not use other macros while using ++ or as a parameter to a function*/
	memtodisklong (header.ctsaves);
	memtodisklong (header.maxpos);
	
	header.flags |= floneline_mask * (**hwp).floneline;
	
	header.flags |= flruleron_mask * (**hwp).flshowruler;
	
	/*
	header.flexpandvariables = (**hwp).flexpandvariables;
	
	header.flhilitevariables = (**hwp).flhilitevariables;
	*/
	
	recttodiskrect (&(**hwp).windowrect, &header.windowrect);
	
	header.buffersize = conditionallongswap (buffersize);
	
	header.varlistsize = 0;
	
	h = *hpacked; /*copy into register*/
	
	moveleft (&header, *h, sizeof (header));
	
	return (true);
	} /*wppackheader*/


static boolean wppacktext (pg_ref pg, Handle *hpacked) {
	
	/*
	12/26/91 dmb: call memoryerror on WSToHandle failure
	
	4.16.97 dmb: was passing wpbuffer to pgSaveDoc instead of pg parameter.

	5.0a10 dmb: call pgTerminateFile, so we're not crashed by extra, trailing data
	*/
	
	file_ref filemap;
	long pos;
	pg_byte	*ptext;
	long len;
	boolean fl;
	
	filemap = (file_ref) MemoryAlloc (&mem_globals, sizeof (pg_byte), 0, 0);
	
	pos = 0;
	
	fl = (pgSaveDoc (pg, &pos, NULL, 0, pgScrapMemoryWrite, filemap, 0) == 0);
	
	// dmb: for future: pgSaveAllEmbedRefs (pg, pgScrapMemoryWrite, NULL, &pos, filemap);
	
	if (fl)
		fl = (pgTerminateFile (pg, &pos, pgScrapMemoryWrite, filemap) == 0);

	if (fl) {
	
		ptext = (pg_byte *) UseMemory(filemap);
		
		len = GetMemorySize (filemap);
		
		fl = newfilledhandle (ptext, len, hpacked);
		}
	
	UnuseAndDispose ((memory_ref) filemap);

	if (!fl)
		memoryerror ();
	
	return (fl);
	} /*wppacktext*/


boolean wppack (Handle *hpacked) {
	
	/*
	dmb 9/6/90:  "header" is now really a "trailer".  since the header record 
	is never very large, while the packed wp can be huge, sticking the 
	header on the end is less memory intensive
	*/
	
	Handle hpackedtext, hpackedheader;
	register boolean fl = false;
	long buffersize;
	
	*hpacked = nil;
	
	if (!wpsetglobals ())
		return (false);
	
	if (!wppacktext (wpbuffer, &hpackedtext))
		return (false);
	
	buffersize = gethandlesize (hpackedtext);
	
	if (!wppackheader (buffersize, &hpackedheader)) {
		
		disposehandle (hpackedtext);
		
		return (false);
		}
	
	fl = pushhandle (hpackedheader, hpackedtext);
	
	disposehandle (hpackedheader);
	
	*hpacked = hpackedtext;
	
	return (fl);
	} /*wppack*/


boolean wpunpack (Handle hpacked, hdlwprecord *hwp) {
	
	/*
	unpack the packed edit record stored in the handle into wpdata.
	
	we fit the text to the current window (wpwindow) and its associated
	information record (wpwindowinfo).
	
	6/6/90 DW: we no longer dispose of the packed handle.  should we dispose
	of it if there was an error?  no, it should be up to the caller to get
	rid of the packed handle.  it means our memory "high water mark" is a little
	higher -- but confusion over this cost us three days of debugging (gotta 
	admit I was a little hazy, shouldn't have taken that long.  anyway...)

	5.0b9 dmb: don't smash wpbuffer global
	*/
	
	register ptrwprecord pwp;
	tywpheader header;
	bigstring bsname;
	Rect r;
	tywpflags flags;
	
	/*remove "header" from end of packed wsdata, so we can pass remainder to wpnew*/

	if (!popfromhandle (hpacked, sizeof (header), &header))
		goto error;

	disktomemlong (header.buffersize);

	sethandlesize (hpacked, header.buffersize);
	
	setemptystring (bsname); /*zero-length name*/
	
//	setrect (&rclip, -infinity, -infinity, infinity, infinity); /*wide open clip*/
	
	diskrecttorect (&header.windowrect, &r);
	
	flags = wpnoflags;
	
	if ((header.flags & floneline_mask) != 0)
		flags |= wponeline;
	
	*hwp = wpnewbuffer (hpacked, &r, &shellprintinfo.paperrect, flags, false);
	
	if (*hwp == nil)
		return (false);
	
	pwp = **hwp; /*move into register*/
	
	wpsetavailbounds ((pg_ref) (*pwp).wpbuffer); /*adjust bounds for current page setup*/
	
	(*pwp).timecreated = conditionallongswap (header.timecreated);
	
	(*pwp).timelastsave = conditionallongswap (header.timelastsave);
	
	(*pwp).ctsaves = conditionallongswap (header.ctsaves);
	
	assert ((*pwp).floneline == ((header.flags & floneline_mask) != 0));
	
	(*pwp).flshowruler = (header.flags & flruleron_mask) != 0;
	
	/*
	(*pwp).flexpandvariables = header.flexpandvariables;
	
	(*pwp).flhilitevariables = header.flhilitevariables;
	*/
	
	diskrecttorect (&header.windowrect, &(*pwp).windowrect);
	
	return (true);
	
	error:
		shellerrormessage ("\x43" "Can't unpack wptext object because unexpected data was encountered.");

		return (false);
	} /*wpunpack*/


boolean wpgetpackedmaxpos (Handle hpacked, long *maxpos) {
	
	/*
	extract the saved maxpos from the packed wp header
	*/
	
	register Handle h = hpacked;
	long ix;
	tywpheader header;
	
	ix = gethandlesize (h) - sizeof (header);
	
	if (!loadfromhandle (h, &ix, sizeof (header), &header))
		return (false);
	
	assert (sizeof (tyOLD42wpheader) == sizeof (tywpheader));
	
	*maxpos = conditionallongswap (header.maxpos);
	
	return (true);
	} /*wpgetpackedmaxpos*/


static long gettextlength (pg_ref pg) {

	return (pgTextSize (pg));
	} /*gettextlength*/


static boolean wpnormalizelineendings (Handle htext) {
	
	/*
	5.1.4 dmb: convert any lf-only endings to cr-lf
	*/
	
	handlestream s;
	byte ch, chlast = chnul;
	byte bscr[] = "\x01\r";
	
	openhandlestream (htext, &s);
	
	while (!athandlestreameof (&s)) {
		
		ch = nexthandlestreamcharacter (&s);
		
		if ((ch == chlinefeed) && (chlast != chreturn)) {
		
			if (!mergehandlestreamstring (&s, 0, bscr))
				return (false);
			}
		
		chlast = ch;
		
		++s.pos;
		}
	
	closehandlestream (&s);
	
	return (true);
	} /*wpnormalizelineendings*/


boolean wpgettexthandle (Handle *htext) {
	
	/*
	return in htext a handle containing the entire text of the current wp.
	
	12/26/91 dmb: call memoryerror on WSGetText failure
	
	dmb 12/5/96: paige
	*/
	
	Handle			h;
	paige_rec_ptr	pg_rec;
	pg_byte_ptr		text;
	text_block_ptr	block;
	long			num_blocks;
	long			text_size;
	long			total_bytes = 0;
	boolean			fl = true;	
	
	if (!wpsetglobals ()) 
		return (false);
	
	if (!newemptyhandle (&h))
		return (false);

	pg_rec = (paige_rec_ptr) UseMemory (wpbuffer);

	block = (text_block_ptr) UseMemory (pg_rec->t_blocks);
	
	for (num_blocks = GetMemorySize (pg_rec->t_blocks); fl && (--num_blocks >= 0); ) {

		text_size = GetMemorySize (block->text);

		text = (pg_byte_ptr) UseMemory (block->text);
		
		total_bytes += text_size;

		fl = enlargehandle (h, text_size, text);
		
		UnuseMemory (block->text);

		++block;
		}
	
	UnuseMemory (pg_rec->t_blocks);
	
	UnuseMemory (wpbuffer);
	
	#ifdef xxxWIN95VERSION
		if (fl)
			fl = wpcr2crlf (h);
	#endif
	
	if (fl)
		*htext = h;
	else	
		disposehandle (h);
	
	return (fl);
	} /*wpgettexthandle*/


boolean wpgettext (bigstring bs, long *cttotal) {
	
	/*
	return the text as a string, truncated if necessary.  in cttotal, return 
	the total number of characters in the text.
	*/
	
	Handle htext;
	
	setemptystring (bs); /*default, in case of error*/
	
	if (!wpgettexthandle (&htext))
		return (false);
	
	*cttotal = gethandlesize (htext);
	
	texthandletostring (htext, bs);
	
	disposehandle (htext);
	
	return (true);
	} /*wpgettext*/


boolean wpgetseltexthandle (Handle *htext) {
	
	/*
	9/27/91 dmb: return in htext a handle containing the currently selected text.
	
	dmb 12/5/96: paige

	5.0a16: return empty handle if pgCopyText returns null. For an empty selection
	Win does this, but the Mac does something else.
	*/
	
	text_ref text;

	if (!wpsetglobals ())
		return (false);
	
	text = pgCopyText (wpbuffer, NULL, all_visible_text_chars);
	
	if (text == MEM_NULL)
		newemptyhandle (htext);
	
	else {
	
		newfilledhandle (UseMemory (text), GetMemorySize (text), htext);
		
		UnuseAndDispose ((memory_ref) text);

		#ifdef xxxWIN95VERSION
			if (htext != nil)
				wpcr2crlf (*htext);
		#endif
		}
	
	return (*htext != nil);
	} /*wpgetseltexthandle*/


boolean wpgetseltext (bigstring bs) {
	
	Handle htext;
	
	setemptystring (bs); /*default return*/
	
	if (!wpgetseltexthandle (&htext))
		return (false);
	
	texthandletostring (htext, bs);
	
	disposehandle (htext);
	
	return (true);
	} /*wpgetseltext*/


boolean wppreedit (void);  //forward

void wppostruleredit (boolean, boolean); //forward

boolean wppushundo (long, void *); // forward


#define PGINSERT_BLOCKSIZE 2048

static boolean wpinserttexthandle (pg_ref pg, Handle htext, short draw_mode, boolean flsetupundo) {

	/*
	5.1.4 dmb: normalize the line endings. Note: we're modifying the input parameter
	*/
	
	handlestream s;
	memory_ref textbuf;
	byte *pbuf;

	if (!wpnormalizelineendings (htext))
		return (false);
	
	PG_TRY (&mem_globals)
	{
		openhandlestream (htext, &s);
		
		if (flsetupundo) {
			
			pushundoaction (undopastestring);
			
			wppushundo (undo_insert, (void *) &s.eof);
			}
		
		textbuf = MemoryAlloc (&mem_globals, sizeof (pg_byte), PGINSERT_BLOCKSIZE, 0);
		
		pbuf = (pg_byte *) UseMemory (textbuf);
		
		while (!athandlestreameof (&s)) {
			 
			long ctbytes = min (PGINSERT_BLOCKSIZE, s.eof - s.pos);

			boolean flhashtmlstyles = false;
			
			if (!readhandlestream (&s, pbuf, ctbytes))
				break;
			
			//#ifdef PIKE
				if (outlinedata != NULL)
					if ((**outlinedata).flhtml)
							flhashtmlstyles = wpapplyhtmlstyles (htext, (unsigned long)NULL, false);
			//#endif

			if (flhashtmlstyles) {
		
				pgInsert (pg, (pg_byte_ptr) pbuf, ctbytes, CURRENT_POSITION, data_insert_mode, 0, draw_none);
			
				wpapplyhtmlstyles (htext, pg, draw_mode);
				} /*if*/

			else {

				pgInsert (pg, (pg_byte_ptr) pbuf, ctbytes, CURRENT_POSITION, data_insert_mode, 0, draw_mode);

				} /*else*/

		//	pgInsertText (pg, (pg_byte_ptr) pbuf, ctbytes, NULL, NULL, NULL, draw_mode);



			} /*while*/
		UnuseAndDispose (textbuf);
		
		closehandlestream (&s);

		return (true);
		}
	PG_CATCH
	{
		#ifdef MACVERSION
			oserror (memFullErr);
		#endif
		#ifdef WIN95VERSION
			oserror (ERROR_OUTOFMEMORY);
		#endif
		
		return (false);
	}
	PG_ENDTRY;
	} /*wpinserttexthandle*/


boolean wpsettexthandle (Handle htext) {
	
	/*
	9/28/91 dmb: cut away all of the existing text before setting 
	the new so that all format runs go away.
	
	12/26/91 dmb: make sure undo is cleared
	
	dmb 12/5/96: paige
	
	5.0.2b17 dmb: added PG_TRY block
	*/
	
	select_pair range;
	boolean fl = false;
	
	if (!wppreedit ())
		return (false);
	
	if (wpownwindowinfo ())
		pushundoaction (0); /*can't undo this*/
	
	range.begin = 0; range.end = gettextlength (wpbuffer);
	
	pgDelete (wpbuffer, &range, draw_none);
	
	if (htext != nil)
		fl = wpinserttexthandle (wpbuffer, htext, wpdisplay_way, false);
	
	wpdirtyselectioninfo ();
	
	wppostruleredit (fl, fl);
	
	return (fl);
	} /*wpsettexthandle*/


boolean wpsettext (bigstring bs) {
	
	Handle htext;
	boolean fl;
	
	if (!newtexthandle (bs, &htext))
		return (false);
	
	fl = wpsettexthandle (htext);
	
	disposehandle (htext);
	
	return (fl);
	} /*wpsettext*/


static void wpnoop (void) {
	
	} /*wpnoop*/


hdlwprecord wpnewbuffer (Handle hpacked, const Rect *rclip, const Rect *rbounds, tywpflags wpflags, boolean flinwindow) {
	
	/*
	12/26/91 dmb: call memoryerror on WSNew failure
	
	4.0b7 dmb: initialize timelastsave to creation time; it's really a modification date.

	5.0b9 dmb: don't use (smash) wpbuffer global for what's really a local var
	*/
	
	Handle hrecord;
	register hdlwprecord hwp;
	long flags;
//	tyconfigrecord config;
	shape_ref visarea, pagearea;
//	generic_var device = flinwindow? MEM_NULL : USE_NO_DEVICE;
	generic_var device = USE_NO_DEVICE;
	pg_ref hbuf = nil;
	
	if (flinwindow)
	{
		#if TARGET_API_MAC_CARBON == 1
		device = (generic_var)GetWindowPort(wpwindow);
		#else
		device = (generic_var) wpwindow;
		#endif
	}
	
	if (!newclearhandle (sizeof (tywprecord), &hrecord))
		return (nil);
	
	hwp = (hdlwprecord) hrecord; /*copy into register*/
	
	(**hwp).floneline = (wpflags & wponeline) != 0;
	
	(**hwp).flalwaysmeasuretext = (wpflags & wpalwaysmeasure) != 0;
	
	(**hwp).wprect = *rclip;
	
	if (hpacked) /*we have a packed version to go with*/ {
		
		if (wpunpacktext (hpacked, &hbuf))
			;	// *** set area???
		}
	else {
		
		flags = defaultpgflags;
		
		if ((wpflags & wponeline) != 0)
			flags |= NO_WRAP_BIT; // | EXTERNAL_SCROLL_BIT));
		
		visarea = newshape (rclip);
		
		pagearea = newshape (rbounds);
	
		setdefaultstyles ((wpflags & wpprinting) != 0);
		
		PG_TRY (&mem_globals) {
		
			hbuf = pgNew (&ws_globals, device, visarea, pagearea, MEM_NULL, flags);
			}
		PG_CATCH {
		
			pgFailureDispose (hbuf);
			
			memoryerror ();
			}
		PG_ENDTRY;
		
		pgDisposeShape (visarea);
		
		pgDisposeShape (pagearea);
		}
	
	if (hbuf == nil) { /*memory error*/
			
		disposehandle ((Handle) hwp);
		
		return (nil);
		}
	
	pgSetHiliteStates (hbuf, activate_verb, activate_verb, false);
	
//	pgSetPointSize (hbuf, (**wpwindowinfo).selectioninfo.fontsize, NULL, false);
	
	/*don't let Paige autoscroll; it goes too fast and doesn't update scroll bars*/ {

		wpsetautoscroll (hbuf, false);
		}
	
	if ((wpflags & wpneverscroll) != 0)
		(**hwp).flneverscroll = true;
	else
		pgSetScrollParams (hbuf, 32, 0, 0, 0);
	
	(**hwp).wpbuffer = (Handle) hbuf;
	
	(**hwp).timecreated = (**hwp).timelastsave = timenow ();
	
	(**hwp).dirtyroutine = &wpnoop;
	
	//#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	//Code change by Timothy Paustian Sunday, May 7, 2000 10:52:06 PM
	//Changed to Opaque call for Carbon
	//in carbon you don't need UPPs so just install the callback
	//(**hwp).trackclickroutine = wptrackclick;
	//#else
	(**hwp).trackclickroutine = wptrackclickUPP; 
	//#endif
	(**hwp).preeditroutine = &truenoop;
	
	(**hwp).posteditroutine = &wpnoop;
	
	(**hwp).setscrollbarsroutine = &wpnoop;
	
	return (hwp);
	} /*wpnewbuffer*/


boolean wpnewrecord (Rect r, hdlwprecord *hwprecord) {
#pragma unused (r)

	/*
		3.0.4b8 dmb: set the scratchport
	*/
	
	bigstring bs;
	tyconfigrecord lconfig;
	register hdlwprecord hwp;
	Rect rclip;
	
	setemptystring (bs);
	
	shellgetconfig (idwpconfig, &lconfig);
	
	pushscratchport ();
	
	pushstyle (lconfig.defaultfont, lconfig.defaultsize, lconfig.defaultstyle);
	
	rclip = lconfig.defaultwindowrect;

	offsetrect (&rclip, -rclip.left, -rclip.top);

	hwp = wpnewbuffer (nil, &rclip, &shellprintinfo.paperrect, 0, false);
	
	popstyle ();
	
	popport ();
	
	if (hwp == nil)
		return (false);
	
	(**hwp).windowrect = lconfig.defaultwindowrect;
	
	(**hwp).fldirty = true;
	
	*hwprecord = hwp;
	
	return (true);
	} /*wpnewrecord*/


void wpdisposerecord (hdlwprecord hwp) {
	
	register pg_ref hbuf = (pg_ref) (**hwp).wpbuffer;
	
	if (hbuf != nil)
		pgDispose (hbuf);
	
	disposehandle ((Handle) (**hwp).hname);
	
	wpdisposeundo (hwp);
	
	disposehandle ((Handle) hwp);
	} /*wpdisposerecord*/


void wpdispose (void) {
	
	if (!wpsetglobals ())
		return;
	
	wpresettyping ();
	
	wpdisposerecord (wpdata);
	
	wpdata = nil;
	} /*wpdispose*/


#ifdef MACVERSION
#pragma mark === editing ===
#endif


boolean wppreedit (void) {
	
	/*
	5.0b8 dmb: resize area bounds here, now in postedit, so mouse
	operations benefit immmediately
	*/
	
	register hdlwprecord hwp = wpdata;
	
	if (!wpsetglobals ())
		return (false);
	
	if (!(*(**hwp).preeditroutine) ())
		return (false);
	
	wppushdraw ();

	(**hwp).fltextchanged = false; /*on last edit operation, text did not change, by default*/
	
	++(**hwp).fleditingnow;
	
	return (true);
	} /*wppreedit*/


void wppostruleredit (boolean flchangedtext, boolean flvisi) {
	
	/*
	high-resolution postedit, after using rulers add-on package calls that 
	return the index of the first line requiring recalc
	
	12/6/96 dmb: new code for paige

	5.0a5 dmb: fixed up new auto bounds expansion code. don't make bounds
	narrower than the visible area
	*/
	
	register boolean fldisplay = wpdisplayenabled ();
	register hdlwprecord hwp = wpdata;
	
	--(**hwp).fleditingnow;
	
	if (flchangedtext)
		(**hwp).fltextchanged = true;
	
	if (fldisplay) {
		
		wpadjustboundstofit ();
		
		wpresetscrollbars ();
		
		if (flvisi)
			wpcheckscrollpos (); /*make sure we have shrunk out of scroll range*/
		
		wpdirty (flchangedtext);
		
		if (wpwindowinfo != nil)
			(**wpwindowinfo).selectioninfo.fldirty = true;
		
		if (flvisi)
			wpvisicursor ();
		}
	else {
		
		if (flchangedtext)
			(**hwp).fldirty = true;
		else
			(**hwp).fldirtyview = true;
		}
	
	wppopdraw ();
	
	(*(**hwp).posteditroutine) ();
	} /*wppostruleredit*/


void wppostedit (boolean flrecalc) {
	
	/*
	low-resolution postedit, after using standard ws-engine calls that 
	only return true or false for recalc.
	*/
	
	wppostruleredit (flrecalc, true);
	} /*wppostedit*/


/*
static pascal boolean wperrorhandler (long ctbytesneeded) {
	
	return (false);
	} /%wperrorhandler%/
*/


static void setupundo (long undocode, void *insertref) {
	
	undo_ref undo = getundo ();
	
	switch (undocode) {
	
		case undo_none:	// probably a Copy
			pgDisposeUndo (undo);
			
			setundo (MEM_NULL);
			
			break;
		
		case undo_typing:
		case undo_fwd_delete:
		case undo_backspace:
			setundo (pgPrepareUndo (wpbuffer, undocode, (void *) undo));
			
			break;
		
		default:
			pgDisposeUndo (undo);
			
			setundo (pgPrepareUndo (wpbuffer, undocode, insertref));

			break;
		}
	} /*setupundo*/


static boolean wppushundostep (void); // forward;


static boolean wpundoroutine (Handle hdata, boolean flundo) {
	
	/*
	12/6/96 dmb: paige implementation
	*/
	
	undo_ref hundo = (undo_ref) hdata;
	
	if (flundo) {
		
		assert (getundo () == nil);
		
		setundo (hundo);
		
		wpundo ();
		
		wppushundostep ();
		}
	else {
		
		if (wpdata && (**wpdata).fleditingnow)
			setundo (hundo); // wp owns it now
		else
			pgDisposeUndo (hundo);
		}
	
	return (true);
	} /*wpundoroutine*/


static boolean wppushundostep (void) {
	
	/*
	12/6/96 dmb: paige implementation. we just need to be called 
	so we can ask paige to do the undo.
	*/
	
	if (!pushundostep (&wpundoroutine, (Handle) getundo ()))
		return (false);
	
	setundo (nil); // undo owns it now
	
	return (true);
	} /*wppushundostep*/


boolean wppushundo (long undocode, void *insertref) {
	
	setupundo (undocode, insertref);
	
	return (wppushundostep ());
	} /*wppushundo*/


static boolean wppushrulerundo (void) {
	
	return (wppushundo (undo_format, NULL));
	} /*wppushrulerundo*/


boolean wppushformatundo (void) {
	
	return (wppushrulerundo ());
	} /*wppushformatundo*/


static boolean wpsetcaretpos (pg_short_t caretverb) {
	
	/*
	5.0a16 dmb: fart around to get selection to work right when going home, 
	end-of-line, etc. Paige stops at the edge of the visible area otherwise.
	*/

	boolean flwordwise = (caretverb == next_word_caret || caretverb == previous_word_caret);
	boolean flextend = keyboardstatus.flshiftkey;
	long oldstartsel, oldendsel;
	long newstartsel, newendsel;

	if (flextend)
	{
		if (flwordwise)
			caretverb |= EXTEND_CARET_FLAG;
		else
			pgGetSelection (wpbuffer, &oldstartsel, &oldendsel);
	}

	pgSetCaretPosition (wpbuffer, caretverb, true);
	
	if (flextend && !flwordwise) {
		
		pgGetSelection (wpbuffer, &newstartsel, &newendsel);

		pgSetSelection (wpbuffer, oldstartsel, newendsel, 0, true);
		}
	
	return (true);
	} /*wpsetcaretpos*/


boolean wptraversehiddentext (tydirection dir, long *newstartsel) {
	
	boolean flmoved = false;
	long oldstartsel, oldendsel;
	long stylebits, consistentbits;

	//#ifdef PIKE
	
		pgGetSelection (wpbuffer, &oldstartsel, &oldendsel);

		if (oldstartsel < 0)
			return (false); /*didn't move*/

		if (dir == left)
			*newstartsel = oldstartsel - 1;
		else
			*newstartsel = oldstartsel;

		while (true) {

			wpsetselection (*newstartsel, *newstartsel + 1);

			pgGetStyleBits (wpbuffer, &stylebits, &consistentbits);

			if (stylebits & 0x400) {

				flmoved = true;

				if ((*newstartsel == 0) && (dir == left))

					break;

				if (dir == left)
					*newstartsel = *newstartsel - 1;
				else
					*newstartsel = *newstartsel + 1;
				}

			else {
				
				if (dir == left)
					*newstartsel = *newstartsel + 1;

				break;
				} /*else*/
			} /*while*/
		
		wpsetselection (*newstartsel, *newstartsel);

		if (!flmoved)
			wpsetselection (oldstartsel, oldendsel);

	//#endif
	
	return (flmoved);
	} /*wptraversehiddentext*/


boolean wptrimhiddentext (void) {
	
	
	boolean fl = false;
	long startsel, endsel;
	long newstartsel, newendsel;

	//#ifdef PIKE

		wpgetselection (&startsel, &endsel);

		newstartsel = startsel;

		newendsel = endsel;

		wptraversehiddentext (right, &newstartsel);

		wpsetselection (endsel, endsel);

		wptraversehiddentext (left, &newendsel);

		wpsetselection (newstartsel, newendsel);

		if (startsel != newstartsel)
			fl = true;

		if (endsel != newendsel)
			fl = true;
		
	//#endif
	
	return (fl); /*the selection was trimmed*/
	} /*wptrimhiddentext*/


/*boolean wptrimrighthiddentextfromselection (void) {
	
	#ifdef PIKE
	
		long startsel, endsel;
		long newendsel;

		wpgetselection (&startsel, &endsel);

		wpsetselection (endsel, endsel);

		if (wptraversehiddentext (left, &newendsel)) {

			wpsetselection (startsel, newendsel);

			return (true);
			}

		wpsetselection (startsel, endsel);
		
	#endif
	return (false);	
	} /%wptrimrighthiddentextfromselection%/
*/

boolean wpkeystroke (void) {
	
	/*
	4.1b7 dmb: for forward delete keys, new WS needs a force recalc
	in case they come too fast & get buffered

	5.0b8 dmb: Windows-standard home/end, page up/down
	*/
	
	pg_byte chkey = keyboardstatus.chkb;
	short modifiers = 0;
	boolean flinserting = true;	
	pg_short_t caretverb = (pg_short_t) -1;
	long undocode = undo_none;
	pg_short_t forcemotionverb = (pg_short_t) -1;
	long oldstartsel, oldendsel;
	long newstartsel, newendsel;
	#define null_caret ((pg_short_t) -1)
	boolean floptionkey = keyboardstatus.floptionkey;
	boolean flvisi = true;
	hdlwprecord pwpdata;
	
	#ifdef WIN95VERSION
		Point pt;
	#endif
	
	if (!wppreedit ())
		return (false);
	
	obscurecursor ();
	
	#ifdef WIN95VERSION
		if (keyboardstatus.flcmdkey) {
			
			floptionkey = true;

			keyboardstatus.flcmdkey = false;
			}
	#endif

	switch (chkey) {
	
		case chhome:
			#ifdef MACVERSION
				caretverb = home_caret;
			#else
				if (floptionkey)
					caretverb = home_caret;
				else
					caretverb = begin_line_caret;
			#endif
			break;
		
		case chend:
			#ifdef MACVERSION
				caretverb = doc_bottom_caret;
			#else
				if (floptionkey)
					caretverb = doc_bottom_caret;
				else
					caretverb = end_line_caret;
			#endif
			break;
		
		case chpageup:
			#ifdef WIN95VERSION
				wpvisicursor ();
		 		wpgetselpoint (&pt);
			#endif
			
			wpscroll (down, true, 1);
			
			#ifdef WIN95VERSION
		 		wpsetselpoint (pt);
			#endif
			
			flinserting = flvisi = false;
			break;
		
		case chpagedown:
			#ifdef WIN95VERSION
				wpvisicursor ();
		 		wpgetselpoint (&pt);
			#endif
			
			wpscroll (up, true, 1);
			
			#ifdef WIN95VERSION
		 		wpsetselpoint (pt);
			#endif
			
			flinserting = flvisi = false;
			break;
		
		case chuparrow:
			forcemotionverb = home_caret;
			
			if (floptionkey) {
				#ifdef WIN95VERSION
					wpscroll (down, false, 1);
					flinserting = flvisi = false;
				#else
					caretverb = home_caret;
				#endif
				}

			if (keyboardstatus.flcmdkey)
				caretverb = home_caret;
			break;
		
		case chdownarrow:
			forcemotionverb = doc_bottom_caret;
			
			if (floptionkey) {
				#ifdef WIN95VERSION
					wpscroll (up, false, 1);
					flinserting = flvisi = false;
				#else
					caretverb = doc_bottom_caret;
				#endif
				}
			
			if (keyboardstatus.flcmdkey)
				caretverb = doc_bottom_caret;
			break;
		
		case chleftarrow:
			if (floptionkey)
				caretverb = previous_word_caret;
			
			if (keyboardstatus.flcmdkey)
				caretverb = begin_line_caret;
			break;
		
		case chrightarrow:
			if (floptionkey)
				caretverb = next_word_caret;
			
			if (keyboardstatus.flcmdkey)
				caretverb = end_line_caret;
			break;
		
		case chdelete:
			undocode = undo_fwd_delete;
			break;
		
		case chbackspace: {

			long lnewstartsel;

			undocode = undo_backspace;

			wptraversehiddentext (left, &lnewstartsel);

			break;
			}
		
		default:
			flinserting = isprint (chkey) || (chkey == chtab) || (chkey == chreturn) || (chkey > 0x7F);
			
			if (flinserting)
				undocode = undo_typing;
			
			break;
	}
	
	if (caretverb != null_caret) {
		
		wpsetcaretpos (caretverb);
		}
	
	else if (flinserting) {
		pwpdata = wpdata;
		
		if (undocode != undo_none) {
		
			pushundoaction (undotypingstring);
			
			//why this is needed or why it fixes the crashing bug I don't know but it does
			//rab 3/29/99
			
			wpdata = pwpdata;
			
//			assert (pwpdata == wpdata);
			
			if (! ((*pwpdata)->flstartedtyping)) {
				
				wpdisposeundo (wpdata); // dispose any inherited undo after pushundoaction, before next statement

//				assert (pwpdata == wpdata);
				
				(*pwpdata)->flstartedtyping = true;
				}
			
//			assert (pwpdata == wpdata);

			setupundo (undocode, NULL);
			
			wppushundostep ();
			}
		
		if (keyboardstatus.flshiftkey)
			modifiers |= EXTEND_MOD_BIT;
		
		if (forcemotionverb != null_caret)
			pgGetSelection (wpbuffer, &oldstartsel, &oldendsel);
		
		pgInsert (wpbuffer, &chkey, sizeof (pg_byte), CURRENT_POSITION, key_insert_mode, modifiers, wpdisplay_way);
		
		if (forcemotionverb != null_caret) {
			
			pgGetSelection (wpbuffer, &newstartsel, &newendsel);
			
			if (oldstartsel == newstartsel && oldendsel == newendsel && oldstartsel == oldendsel) {
			
				wpsetcaretpos (forcemotionverb);
				}
			}
		}
	
	if (keyboardstatus.keydirection != nodirection)
		pgSetCursorState (wpbuffer, toggle_cursor);
	
	wppostruleredit (undocode != undo_none, flvisi);
	
	return (true);
	} /*wpkeystroke*/


boolean wpgo (tydirection dir, long dist) {
	
	/*
	simulate cursor motion in the indicated direction and distance.
	
	return true if any motion actually occurs.
	
	4.1b5 dmb: with WSE 2.2, WSKey can modify the event record. So we
	have to set it up each time through the loop
	
	4.1b6 dmb: test motion be comparing Pos and Carent fields of tBegin,
	not by seeing of pgGetSelection changes. Also, activate the text during
	move in case there's a selection. It won't update right if inacdtive.
	
	12/6/96 dmb: paige implementation
	
	5.1.5b7 dmb: handle ill-timed disposal of wpdata in wpactivate
	*/
	
	boolean flmoved = false;
	boolean fldidnotmove = false;
	boolean flactive;
	pg_byte chkey = 0;
	long startsel, endsel;
	long newsel;
	
	if (!wppreedit ())
		return (false);
	
	wpresettyping ();
	
	flactive = wpisactive ();
	
	if (!flactive) {
		
		wpactivate (true);
		
		if (wpdata == nil)
			return (false);
		}
	
	wpgetselection (&startsel, &endsel);
	
	switch (dir) {
		
		case up: case flatup:
			chkey = chuparrow;
			
			break;
		
		case down: case flatdown:
			chkey = chdownarrow;
			
			break;
		
		case left:
			chkey = chleftarrow;
		//	wpsetselection (startsel - dist, startsel - dist);
			
			break;
		
		case right:
			chkey = chrightarrow;
		//	wpsetselection (endsel + dist, endsel + dist);
			
			break;
		
		default:
			break;
		}
	
	if (chkey != 0) {
		
		while (--dist >= 0) {
		
			pgInsert (wpbuffer, &chkey, sizeof (pg_byte), CURRENT_POSITION, key_insert_mode, 0, wpdisplay_way);
			
			wpgetselection (&newsel, &newsel);
			
			if ((newsel == startsel) && (newsel == endsel)) {
			
				if (fldidnotmove)
					break;
			
				fldidnotmove = true;
				}
			else {
				startsel = endsel = newsel;
				
				fldidnotmove = false;
				
				flmoved = true;
				}
			
			pgSetCursorState (wpbuffer, compute_cursor);
			}
		}
	
	if (!flactive) {
	
		wpactivate (false);
		
		if (wpdata == nil)
			return (false);
		}
	
	wppostedit (false);
	
	return (flmoved);
	} /*wpgo*/


static boolean wprulerclick (Point pt) {
#pragma unused (pt)

	boolean flrecalc = false;
	
#if flrulers
	
	#endif
	
	return (flrecalc);
	} /*wprulerclick*/


void wpclick (Point pt, tyclickflags flags) {
	
	register boolean flrecalc;
	Rect r;
	Point ptnew;
	co_ordinate point;
	boolean flinview;
	
	r = (**wpwindowinfo).contentrect;
	
	if (!pointinrect (pt, r))
		return;
	
	if (!wppreedit ()) 
		return;
	
	if ((**wpdata).flshowruler  && false /*pointinruler*/) {
		
		flrecalc = wprulerclick (pt);
		
		if (flrecalc) /*formatting was changed, update ruler right away*/
			shellsetselectioninfo ();
		}
	else {
		
		wpresettyping ();
		
		point.h = pt.h;
		point.v = pt.v;
		
		pgDragSelect (wpbuffer, &point, mouse_down, (short) flags, 0, FALSE);
		
		while (mousestilldown ()) {
		
			getmousepoint (&ptnew);
			
			point.h = ptnew.h;
			point.v = ptnew.v;
			
			flinview = pgPtInView (wpbuffer, &point, NULL) & WITHIN_VIS_AREA;

			if ((!flinview) || (ptnew.h != pt.h) || (ptnew.v != pt.v)) {
			
				pgDragSelect (wpbuffer, &point, mouse_moved, (short) flags, 0, TRUE);
				
				pt = ptnew;
				}
			
			CallTrackClickProc ((**wpdata).trackclickroutine, wpdata, pt);
			}
		
		pgDragSelect (wpbuffer, &point, mouse_up, (short) flags, 0, FALSE);
		
		flrecalc = false;
		}
	
	wppostruleredit (flrecalc, false);
	} /*wpclick*/


static boolean wpinserttext (byte *ptext, long ctchars) {
	
	/*
	4.1b7 dmb: handle ctchars > maxint by giving it to WSE in short chunks
	
	5.0a9 dmb: fixed undo crashing bug
	
	5.0.2b17 dmb: added PG_TRY block
	
	5.0.2b18 dmb: always pgDelete to fix display bug
	*/
	
	boolean fl = false;
	
	if (wppreedit ()) {
		
		pushundoaction (undopastestring);
		
		wpresettyping ();
		
		wppushundo (undo_insert, (void *) &ctchars);
		
		PG_TRY (&mem_globals)
		{
			//if (ctchars == 0)
				pgDelete (wpbuffer, NULL, wpdisplay_way);
			
			//else
				pgInsert (wpbuffer, (pg_byte_ptr) ptext, ctchars, CURRENT_POSITION, data_insert_mode, 0, wpdisplay_way);
			
			fl = true;
		}
		PG_CATCH
		{
			#ifdef MACVERSION
				oserror (memFullErr);
			#endif
			#ifdef WIN95VERSION
				oserror (ERROR_OUTOFMEMORY);
			#endif
		}
		PG_ENDTRY;
		
		(**wpdata).flstartedtyping = true;
		
		wppostruleredit (fl, fl);
		}
	
	return (fl);
	} /*wpinserttext*/


boolean wpinserthandle (Handle htext) {

	/*
	PBS 7.0b22: Fixed display glitch by invalidating the rect before
	drawing. Now you don't get all the garbage characters when you
	choose a command that does a wp.insert.
	*/
	
	boolean fl = false;
	boolean flhtmloutline = false;
	boolean fldisplaywasenabled = wpdisplayenabled ();

	if (wppreedit ()) {

		invalrect ((**wpdata).wprect); /*PBS 7.0b22*/
		
		wpresettyping ();

		if (outlinedata != NULL)

			if ((**outlinedata).flhtml)

				flhtmloutline = true;

		if (flhtmloutline)

			wpsetdisplay (false);
		
		fl = wpinserttexthandle (wpbuffer, htext, wpdisplay_way, true);

		if (flhtmloutline) {

			long startsel, endsel;

			wpgetselection (&startsel, &endsel);

			opunloadeditbuffer ();

			oploadeditbuffer ();

			wpsetselection (startsel, endsel);

			if (fldisplaywasenabled)

				wpsetdisplay (true);
			} /*if*/
		
		(**wpdata).flstartedtyping = true;
		
		wppostruleredit (fl, fl);
		}
	
	return (fl);
	} /*wpinserthandle*/


boolean wpinsert (bigstring bs) {
	
	return (wpinserttext (bs + 1, stringlength (bs)));
	} /*wpinsert*/


boolean wpdelete (boolean flinserting) {
#pragma unused (flinserting)

	/*
	12/31/91 dmb: added flinserting parameter -- needed to deterinate whether 
	the undo of the deletion needs to redisplay or not
	*/
	
	long startsel, endsel;
	
	pgGetSelection (wpbuffer, &startsel, &endsel);
	
	if (startsel == endsel)
		return (false);
	
	wppushundo (undo_delete, NULL);
	
	pgDelete (wpbuffer, NULL, wpdisplay_way);
	
	return (true);
	} /*wpdelete*/


static void wpdisposescrap (void * hwp) {
	
	pgDispose ((pg_ref) hwp);
	} /*wpdisposescrap*/


static boolean wpexportscrap (void *scrap, tyscraptype totype, Handle *htext, boolean *fltempscrap) {

	/*
	if the requested type isn't text, return false.  otherwise, try to 
	convert the given scrap to text
	*/
	
	pg_ref hwp = (pg_ref) scrap;
	
	switch (totype) {
		
		case wpscraptype:
			if (!wppacktext (hwp, htext)) /*flat version for system scrap*/
				return (false);
			
			break;
		
		case textscraptype: {
			select_pair range;
			text_ref text;
			
			range.begin = 0, range.end = pgTextSize (hwp);
			
			text = pgCopyText(hwp, &range, all_text_chars);
			
			if (text == MEM_NULL)
				return (false);

			newfilledhandle (UseMemory (text), GetMemorySize (text), htext);
			
			UnuseAndDispose ((memory_ref) text);
			
			#ifdef xxxWIN95VERSION
				if (htext != nil)
					wpcr2crlf (*htext);
			#endif

			break;
			}
		
		case pictscraptype:
		//	SavePictToScrap ((pg_ref) data);
		
			return (false);
		
		default:
			return (false);
		}
	
	*fltempscrap = true;
			
	return (*htext != nil);
	} /*wpexportscrap*/


static boolean wpsetscrap (pg_ref hwp) {

	return (shellsetscrap ((Handle) hwp, wpscraptype, &wpdisposescrap, &wpexportscrap));
	} /*wpsetscrap*/


boolean wpscraphook (Handle hscrap) {
	
	/*
	if our private type is on the external clipboard, set the internal 
	scrap to it.
	*/
	
	pg_ref pg;
	
	if (getscrap (wpscraptype, hscrap)) {
		
		if (wpunpacktext (hscrap, &pg))
			wpsetscrap (pg);
		
		return (false); /*don't call any more hooks*/
		}
	
	return (true); /*keep going*/
	} /*wpscraphook*/


boolean wpcut (void) {
	
	if (!wppreedit ())
		return (false);
	
	pushundoaction (undocutstring);
	
	wppushundo (undo_delete, NULL);
	
	wpresettyping ();
	
	wpsetscrap (pgCut (wpbuffer, NULL, wpdisplay_way));
	
	wppostedit (true);
	
	return (true);
	} /*wpcut*/


boolean wpcopy (void) {

	if (!wpsetglobals ())
		return (false);
	
	return (wpsetscrap (pgCopy (wpbuffer, NULL)));
	} /*wpcopy*/


boolean wppaste (boolean flcanusestyle) {

	/*
	5.0b9 dmb: test display enabling for pg paste parameter
	*/

	pg_ref hwpscrap;
	Handle htext;
	long len;
	boolean fltempscrap;
	
	if (!wppreedit ())
		return (false);
	
	pushundoaction (undopastestring);
	
	wpresettyping ();
	
	//	paste native paige if present
	
	if (flcanusestyle && shellconvertscrap (wpscraptype, (Handle *) &hwpscrap, &fltempscrap)) {
	
		wppushundo (undo_paste, (void *) hwpscrap);
		
		PG_TRY (&mem_globals) {
			
			pgPaste (wpbuffer, hwpscrap, CURRENT_POSITION, FALSE, wpdisplay_way);
			}
		PG_CATCH {
		
			memoryerror ();
			}
		PG_ENDTRY;

		if (fltempscrap)
			pgDispose (hwpscrap);
			
		}
	
	// no paige; try TEXT scrap
	
	else if (shellconvertscrap (textscraptype, &htext, &fltempscrap)) {
	
		lockhandle (htext);
		
		PG_TRY (&mem_globals) {
		
			len = gethandlesize (htext);

			wppushundo (undo_insert, (void *) &len);
			
			pgDelete (wpbuffer, NULL, wpdisplay_way); // 5.0d14 dmb: fixes display bug
			
			pgInsert (wpbuffer, (pg_byte_ptr) *htext, len, CURRENT_POSITION, data_insert_mode, 0, wpdisplay_way);
			}
		PG_CATCH {
		
			memoryerror ();
			}
		PG_ENDTRY;
		
		unlockhandle (htext);
		
		if (fltempscrap)
			disposehandle (htext);
		}
	
	// no paige or text, try PICT scrap
	
	else if (shellconvertscrap (pictscraptype, &htext, &fltempscrap)) {
		
		// *** paste pict
		if (fltempscrap)
			disposehandle (htext);
		}
	
	(**wpdata).flstartedtyping = true;
	
	wppostedit (true);
	
	return (true);
	} /*wppaste*/


boolean wpclear (void) {
	
	boolean flrecalc;
	
	if (!wppreedit ())
		return (false);
	
	pushundoaction (undoclearstring);
	
	wpresettyping ();
	
	flrecalc = wpdelete (false);
	
	wppostedit (flrecalc);
	
	return (true);
	} /*wpclear*/


boolean wpselectall (void) {
	
	return (wpsetselection (0L, longinfinity));
	} /*wpselectall*/


boolean wpselectword (void) {
	
	long startsel, endsel;
	
	if (!wpsetglobals ())
		return (false);
	
	pgFindWord (wpbuffer, CURRENT_POSITION, &startsel, &endsel, TRUE, FALSE);
	
	return (wpsetselection (startsel, endsel));
	} /*wpselectword*/


boolean wpselectline (void) {
	
	long startsel, endsel;
	Rect rbefore, rafter;
	
	if (!wpsetglobals ())
		return (false);
	
	if ((**wpdata).floneline)
		return (wpselectall());
	
	wpgetselrect (&rbefore);
	
	pgFindLine (wpbuffer, CURRENT_POSITION, &startsel, &endsel);
	
	wpsetselection (startsel, endsel);
	
	wpgetselrect (&rafter);
	
	if (rafter.top >= rbefore.bottom) { // wrong line
		
		pgFindLine (wpbuffer, startsel - 1, &startsel, &endsel);
		
		wpsetselection (startsel, endsel);
		}
	
	return (true);
	} /*wpselectline*/


boolean wpselectpara (void) {
	
	long startsel, endsel;
	
	if (!wpsetglobals ())
		return (false);
	
	wpgetselection (&startsel, &endsel);

	pgFindPar (wpbuffer, endsel, &startsel, &endsel);
	
	return (wpsetselection (startsel, endsel));
	} /*wpselectpara*/


boolean wpsetname (bigstring bsname) {
	
	/*
	allows an external caller to set the name of wpdata.
	*/
	
	hdlstring hname;
	boolean fl;
	
	disposehandle ((Handle) (**wpdata).hname);
	
	fl = newheapstring (bsname, &hname);
	
	(**wpdata).hname = hname;
	
	wpdirty (true);
	
	return (fl);
	} /*wpsetname*/


void wpresize (void) {
	
	Rect oldrect, newrect;
	co_ordinate grow;
	
	if (!wpsetglobals ()) 
		return;
	
	newrect = (**wpwindowinfo).contentrect;
	
	oldrect = (**wpwindowinfo).oldcontentrect;
	
	grow.h = newrect.right - oldrect.right;
	
	grow.v = newrect.bottom - oldrect.bottom;
	
	pgGrowVisArea (wpbuffer, NULL, &grow);
	
	(**wpdata).wprect.right += grow.h;
	
	(**wpdata).wprect.bottom += grow.v;
	
	wpresetscrollbars ();
	
	wpdirty (false);
	} /*wpresize*/


boolean wptotalrecalc (void) {
	
	/*
	force a total recalc & redisplay
	*/
	
	if (!wppreedit ())
		return (false);
	
	pgPaginateNow (wpbuffer, 0x7fffffff, FALSE);
	
	wppostedit (true);
	
	return (true);
	} /*wptotalrecalc*/


boolean wpgetmaxpos (long *maxpos) {

	if (!wpsetglobals ())
		return (false);
	
	*maxpos = gettextlength (wpbuffer);
	
	return (true);
	} /*wpgetmaxpos*/


boolean wpgetselection (long *startsel, long *endsel) {

	long newstartsel;

	if (!wpsetglobals ())
		return (false);
	
	pgGetSelection (wpbuffer, startsel, endsel);

	if (wptraversehiddentext (left, &newstartsel)) {

		wpsetselection (newstartsel, *endsel);

		pgGetSelection (wpbuffer, startsel, endsel);
		} /*if*/
	
	return (true);
	} /*wpgetselection*/


boolean wpsetselection (long startsel, long endsel) {

	/*
	7/11/91 dmb: added call to resettyping

	12/5/96 dmb: paige
	
	5.0.2b18 dmb: call wpdirtyselectioninfo
	*/
	
	boolean flactive;
	
	if (!wpsetglobals ())
		return (false);
	
	wppushdraw ();
	
	endsel = min (endsel, gettextlength (wpbuffer));
	
	startsel = min (startsel, endsel);
	
	wpresettyping ();
	
	flactive = wpisactive();
	
	if (!flactive)
		wpframeselection ();
	
	pgSetSelection (wpbuffer, startsel, endsel, 0, wpdisplayenabled () /*&& flactive*/);
	
	if (!flactive)
		wpframeselection ();
	
	wppopdraw ();
	
	wpdirtyselectioninfo ();
	
	return (true);
	} /*wpsetselection*/


boolean wpundo (void) {
	
	undo_ref undo;
	
	if (!wppreedit ())
		return (false);
	
	undo = pgUndo (wpbuffer, getundo (), true, wpdisplay_way);
	
	wpdisposeundo (wpdata);
	
	setundo (undo);
	
	wppostedit (true);
	
	return (true);
	} /*wpundo*/


boolean wpsearch (boolean flfromtop, boolean flcase, boolean flwords, boolean flwrap) {
#pragma unused (flcase, flwords)

	/*
	9/12/91 dmb: supported flwrap parameter
	
	5.0a2 dmb: fixed loop bug in paige rewrite
	
	5.0.2b19 dmb: initialize flfound to false
	*/
	
	paige_rec_ptr pg_rec;
	long ixstart, lenmatch;
	long startsel, endsel;
	long ixblock, ctblocks;
	text_block_ptr textblocks, block;
	boolean flfound = false;
	
	if (!wpsetglobals ())
		return (false);
	
	pgGetSelection (wpbuffer, &startsel, &endsel);
	
	if (flfromtop)
		ixstart = 0;
	else
		ixstart = endsel;
	
	pg_rec = (paige_rec_ptr) UseMemory (wpbuffer);

	textblocks = (text_block_ptr) UseMemory (pg_rec->t_blocks);
	
	ctblocks = GetMemorySize (pg_rec->t_blocks);
	
	for (ixblock = 0, block = textblocks; ixblock < ctblocks; ) {
	
		pg_byte_ptr ptext;
		long lentext;
		
		if (ixstart) {
		
			if (ixstart >= (*block).end)
				goto nextblock; /*next buffer, please*/
			
			ixstart -= (*block).begin; /*convert to offset within buffer*/
			}
		
		lentext = GetMemorySize (block->text);
		
		ptext = (pg_byte_ptr) UseMemory (block->text);
		
		flfound = textsearch (ptext, lentext, &ixstart, &lenmatch);
		
		UnuseMemory (block->text);
		
		if (flfound)
			break;
		
		ixstart = 0; /*we've already searched the only partial buffer*/
		
		if (flwrap && (ixblock == ctblocks - 1)) { /*time to wrap*/
			
			ixblock = 0; /*restart from beginning*/
			
			block = textblocks;
			
			flwrap = false; /*only wrap once!*/
			
			continue; /*wrap around, don't increment block*/
			}
		
	nextblock:
	
		++ixblock;
		
		++block;
		}
	
	if (flfound) {
		
		boolean fldisplay = wpdisplayenabled () && wpisactive();
		long ixmatch;
		
		ixmatch = ixstart;
		
		ixmatch += (*block).begin;
		
		wpresettyping ();
		
		wpsetselection (ixmatch, ixmatch + lenmatch);
		
		if (fldisplay)
			wppushdraw ();
		
		wpvisicursor ();
		
		if (fldisplay)
			wppopdraw ();
		}
	
	UnuseMemory (pg_rec->t_blocks);
	
	UnuseMemory (wpbuffer);
	
	return (flfound);
	} /*wpsearch*/


boolean wpadjustcursor (Point pt) {
	
	co_ordinate point;
	
	if (!wpsetglobals ())
		return (false);
	
	point.h = pt.h;
	point.v = pt.v;
	
	if (pgPtInView (wpbuffer, &point, NULL) & WITHIN_VIS_AREA)
		setcursortype (cursorisibeam);
	else
		setcursortype (cursorisarrow);
	
	return (true);
	} /*wpadjustcursor*/


void wpidle (void) {

	/*
	7.0b16 PBS: On Windows, the caret can only be destroyed in the main thread.
	So wpactive, when asked to destroy the caret, sets fldestroycaret to true.
	Next time this routine is called from the main thread it destroys the caret.
	*/
	
	if (wpsetglobals ()) {
		
		wppushdraw ();
		
		pgIdle (wpbuffer);

		#ifdef WIN95VERSION

		if (fldestroycaret) { /*7.0b16 PBS: on Windows, caret must be destroyed in main thread.*/

				fldestroycaret = false; /*Reset*/

				pgSetCursorState (wpbuffer, hide_cursor); /*Call Paige to destroy the caret*/
				} /* if*/
		#endif
		
		wppopdraw ();
		}
	} /*wpidle*/


#ifdef MACVERSION
#pragma mark === formatting ===
#endif


boolean wpsetfont (void) {
	
	bigstring bsfont;
	
	if (!wppreedit ())
		return (false);
	
	pushundoaction (undoformatstring);
	
	wppushformatundo ();
	
	fontgetname ((**wpwindowinfo).selectioninfo.fontnum, bsfont);
	
	#ifdef WINDOWS_PLATFORM
		convertpstring (bsfont);
	#endif
	
	pgSetFontByName (wpbuffer, bsfont, NULL, wpdisplayenabled ());
	
	wppostedit (true);
	
	return (true);
	} /*wpsetfont*/


boolean wpsetsize (void) {
	
	if (!wppreedit ())
		return (false);
	
	pushundoaction (undoformatstring);
	
	wppushformatundo ();
	
	pgSetPointSize (wpbuffer, (**wpwindowinfo).selectioninfo.fontsize, NULL, wpdisplayenabled ());
	
	wppostedit (true);
	
	return (true);
	} /*wpsetsize*/


boolean wpsetstyle (void) {
	
	long	set_bits;
	long	style_bits = 0;
	
	if (!wppreedit ())
		return (false);
	
	pushundoaction (undoformatstring);
	
	wppushformatundo ();
	
	style_bits = (**wpwindowinfo).selectioninfo.fontstyle;
	
	if (style_bits == normal)
		set_bits = -1;
	else
		set_bits = style_bits;
	
	pgSetStyleBits (wpbuffer, style_bits, set_bits, NULL, wpdisplayenabled ());
	
	wppostedit (true);
	
	return (true);
	} /*wpsetstyle*/


boolean wpsetleading (void) {
	
	par_info info;
	par_info mask;
	
	if (!wppreedit ())
		return (false);
	
	pgFillBlock(&info, sizeof(par_info), 0);
	pgInitParMask(&mask, 0);
	
	info.leading_extra = (long) (**wpwindowinfo).selectioninfo.leading;
	mask.leading_extra = -1;
	
	pushundoaction (undoformatstring);
	
	wppushrulerundo ();
	
	pgSetParInfo (wpbuffer, NULL, &info, &mask, wpdisplay_way);
	
	wppostruleredit (1, false);
	
	return (true);
	} /*wpsetleading*/


boolean wpsetjustify (void) {
	
	par_info info;
	par_info mask;
	
	if (!wppreedit ())
		return (false);
	
	pgFillBlock (&info, sizeof (par_info), 0);
	
	pgInitParMask (&mask, 0);
	
	switch ((**wpwindowinfo).selectioninfo.justification) {
		
		case leftjustified:
			info.justification = justify_left;
			
			break;
			
		case centerjustified:
			info.justification = justify_center;
			
			break;
			
		case rightjustified:
			info.justification = justify_right;
			
			break;
			
		case fulljustified:
			info.justification = justify_full;
			
			break;
			
		default:
			return (false);
			
		} /*switch*/
	
	mask.justification = -1;
	
	pushundoaction (undoformatstring);
	
	wppushrulerundo ();
	
	pgSetParInfo (wpbuffer, NULL, &info, &mask, wpdisplay_way);
	
	wpdirtyselectioninfo ();
	
	wppostruleredit (1, false);
	
	return (true);
	} /*wpsetjustify*/


boolean wpsetselectioninfo (void) {
	
	tyselectioninfo x;
	long selstyle, selstylemask;
	tyjustification j;
	bigstring bsfont;
	short size;
	par_info info;
	par_info mask;
	
	if (!wpsetglobals ())
		return (false);
	
	if (!wpownwindowinfo ())
		return (false);
	
	clearbytes (&x, sizeof (x)); /*initialize all fields to zero*/
	
	x.flcansetfont = true;
	
	x.flcansetsize = true;
	
	x.flcansetstyle = true;
	
	if (pgGetFontByName (wpbuffer, (pg_font_name_ptr) bsfont)) {
	
		#ifdef WINDOWS_PLATFORM
			convertcstring (bsfont);
		#endif

		fontgetnumber (bsfont, &x.fontnum);
		}
	else
		x.fontnum = -1; /*no consistent font across selection*/
	
	if (pgGetPointsize (wpbuffer, &size))
		x.fontsize = size;
	else
		x.fontsize = -1;
	
	pgGetStyleBits (wpbuffer, &selstyle, &selstylemask);
	
	x.fontstyle = selstyle & selstylemask;
	
	x.flcansetjust = true;
	
	x.flcansetleading = true;
	
	pgGetParInfo (wpbuffer, NULL, FALSE, &info, &mask);
	
	x.leading = info.leading_extra;
	
	switch (info.justification) {
	
		case justify_left:
			j = leftjustified;
			
			break;
			
		case justify_center:
			j = centerjustified;
			
			break;
			
		case justify_right:
			j = rightjustified;
			
			break;
			
		case justify_full:
			j = fulljustified;
			
			break;
		
		default:
			j = unknownjustification;
			
			break;
		}
	
	x.justification = j;
	
//	if (wpgetruler ())
//		wpdisplayruler (wpbuffer, TRUE);
	
	(**wpwindowinfo).selectioninfo = x;
	
	return (true);
	} /*wpsetselectioninfo*/


static boolean wpsetverbindent (pg_indents *indents, pg_indents *mask) {
	
	if (!wppreedit ())
		return (false);
	
	pushundoaction (undoformatstring);
	
	wppushrulerundo ();
	
	pgSetIndents (wpbuffer, NULL, indents, mask, wpdisplay_way);
	
	wpdirtyselectioninfo (); /*force ruler update*/
	
	wppostruleredit (1, false);
	
	return (true);
	} /*wpsetverbindent*/
	
	
boolean wpgetindent (short *indent) {
	
	pg_indents indents, mask;
	
	pgGetIndents (wpbuffer, NULL, &indents, &mask, NULL, NULL);
	
	*indent = indents.first_indent;

	return (true);
	} /*wpgetindent*/


boolean wpsetindent (short indent) {
	
	pg_indents indents, mask = {0 ,0 ,-1};
	
	indents.first_indent = indent;

	return (wpsetverbindent (&indents, &mask));
	} /*wpsetindent*/
	
	
boolean wpgetleftmargin (short *leftmargin) {
	
	pg_indents indents, mask;
	
	pgGetIndents (wpbuffer, NULL, &indents, &mask, NULL, NULL);
	
	*leftmargin = indents.left_indent;

	return (true);
	} /*wpgetleftmargin*/


boolean wpsetleftmargin (short leftmargin) {
	
	pg_indents indents, mask = {-1, 0, 0};
	
	indents.left_indent = leftmargin;

	return (wpsetverbindent (&indents, &mask));
	} /*wpsetleftmargin*/


boolean wpgetrightmargin (short *rightmargin) {
	
	pg_indents indents, mask;
	
	pgGetIndents (wpbuffer, NULL, &indents, &mask, NULL, NULL);
	
	*rightmargin = indents.right_indent;

	return (true);
	} /*wpgetrightmargin*/


boolean wpsetrightmargin (short rightmargin) {
	
	pg_indents indents, mask = {0 ,-1 ,0};
	
	indents.right_indent = rightmargin;
	
	return (wpsetverbindent (&indents, &mask));
	} /*wpsetrightmargin*/


boolean wpsettab (short pos, short tabjust, byte chfill) {
	
	/*
	9/27/91 dmb: set a tab at the indicated position.
	
	tabjust should be:
		0 - left
		1 - center
		2 - right
		3 - decimal
	
	chfill is the tab leader.  pass zero or space for no leader
	*/
	
	tab_stop tab;
	
	if ((tabjust < 0) || (tabjust > 3))
		return (false);
	
	if (!wppreedit ())
		return (false);
	
	tab.tab_type = tabjust;
	
	tab.position = pos;
	
	if (chfill == chspace) /*if a space leader is specified, interpret as no leader*/
		chfill = chnul;
	
	tab.leader = chfill;
	
	pushundoaction (undoformatstring);
	
	wppushrulerundo ();
	
	pgSetTab (wpbuffer, NULL, &tab, wpdisplay_way);
	
	wpdirtyselectioninfo (); /*force ruler update*/
	
	wppostruleredit (1, false);
	
	return (true);
	} /*wpsettab*/


boolean wpcleartabs (void) {
	
	/*
	9/27/91 dmb: remove all tabs for the current selection
	*/
	
	long cttabs, ixtab;
	tab_ref tablist;
	tab_stop_ptr tab;
	long screen_offset;
	
	if (!wppreedit ())
		return (false);
	
	tablist = (tab_ref) MemoryAlloc (&mem_globals, sizeof (tab_stop), 0, 0);
	
	pgGetTabList (wpbuffer, NULL, tablist, MEM_NULL, &screen_offset);
	
	cttabs = GetMemorySize (tablist);
	
	if (cttabs > 0) {
		
		pushundoaction (undoformatstring);
		
		wppushrulerundo ();
		
		tab = (tab_stop_ptr) UseMemory (tablist);
		
		for (ixtab = 0; ixtab < cttabs; ++ixtab, ++tab) { /*traverse entire tab array*/
			
			(*tab).tab_type = no_tab;
			
			pgSetTab (wpbuffer, NULL, tab, wpdisplay_way);
			}
		
		wpdirtyselectioninfo (); /*force ruler update*/
		}
	
	wppostruleredit (1, false);
	
	return (cttabs > 0);
	} /*wpcleartabs*/


