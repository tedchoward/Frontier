
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/

#include "frontier.h"
#include "standard.h"

#include "shelltypes.h"
#include "appletdefs.h"
#include "appletmemory.h"
#include "appletfont.h"
#include "appletstrings.h"
#include "appletquickdraw.h"
#include "applettextedit.h"



typedef struct tyscrolldata {
	
	Rect origdestrect;
	
	short origlen;
	} tyscrolldata;
	
tyscrolldata scrolldata;




static boolean getmacbuffer (hdleditrecord hbuffer, TEHandle *macbuffer) {

	if (hbuffer == nil)
		return (false);
		
	*macbuffer = (**hbuffer).macbuffer;
	
	return (*macbuffer != nil);
	} /*getmacbuffer*/
	

static void editprescroll (hdleditrecord hbuffer) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;

	scrolldata.origdestrect = (**macbuffer).destRect;
	
	scrolldata.origlen = (**macbuffer).teLength;
	} /*editprescroll*/
	
	
static void editsetscrollvalues (hdleditrecord hbuffer) {

	TEHandle macbuffer;
	Rect destrect, viewrect;
	boolean flscrolled;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
	
	flscrolled = scrolldata.origlen != (**macbuffer).teLength;
	
	if (!flscrolled)
		flscrolled = !equalrects (scrolldata.origdestrect, (**macbuffer).destRect);
	
	if (!(**hbuffer).flscrolled) /*DW 12/4/94*/
		(**hbuffer).flscrolled = flscrolled;
	
	destrect = (**macbuffer).destRect;
	
	viewrect = (**macbuffer).viewRect;
	
	(**hbuffer).vertcurrent = viewrect.top - destrect.top;
	
	(**hbuffer).horizcurrent = viewrect.left - destrect.left;
	} /*editsetscrollvalues*/
	
	
static short getTEjustification (tyjustification justification) {
	
	switch (justification) {
		
		case leftjustified:
			return (teJustLeft);
			
		case centerjustified:
			return (teJustCenter);
			
		case rightjustified:
			return (teJustRight);
		
		default:
			return (teJustLeft);
		} /*switch*/
	} /*getTEjustification*/
	

boolean editnewbuffer (Rect r, boolean flwordwrap, bigstring bs, hdleditrecord *hrecord) {
	
	hdleditrecord h;
	TEHandle macbuffer;
	
	if (!newclearhandle (longsizeof (tyeditrecord), (Handle *) hrecord))
		return (false);
		
	h = *hrecord; /*copy into register*/
	
	InsetRect (&r, texthorizinset, textvertinset);
		
	macbuffer = TENew (&r, &r);
	
	if (macbuffer == nil) { /*memory error*/
		
		disposehandle ((Handle) h);
		
		*hrecord = nil;
		
		return (false);
		}
		
	if (flwordwrap)
		(**macbuffer).crOnly = 1;
	else
		(**macbuffer).crOnly = -1;

	TESetText (&bs [1], (long) stringlength (bs), macbuffer);
	
	TEActivate (macbuffer);
	
	(**h).macbuffer = macbuffer;
	
	return (true);
	} /*editnewbuffer*/
	
	
static boolean rectneedsupdate (Rect r) {
	
	boolean retVal;
	//Code change by Timothy Paustian Sunday, May 7, 2000 9:11:35 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	{
		CGrafPtr thePort = GetQDGlobalsThePort();
		RgnHandle	visRgn = NewRgn();
		GetPortVisibleRegion(thePort, visRgn);
		retVal = RectInRgn(&r, visRgn);
		DisposeRgn(visRgn);
		return retVal;
	}
	#else
	WindowPtr w = (WindowPtr) quickdrawglobal (thePort);
	retVal = RectInRgn (&r, (*w).visRgn);
	return retVal;
	#endif
	} /*rectneedsupdate*/
	
	
void editupdate (hdleditrecord hbuffer) {
	
	TEHandle macbuffer;
	Rect r, rerase;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
		
	r = (**macbuffer).viewRect;
	
	if ((**macbuffer).teLength == 0)
		EraseRect (&r);
	
	TEUpdate (&r, macbuffer);
	
	InsetRect (&r, -(texthorizinset - 1), -(textvertinset - 1));
	
	/*erase the top sliver*/ {
	
		rerase = r; 
	
		rerase.bottom = rerase.top + textvertinset;
	
		if (rectneedsupdate (rerase)) 
			eraserect (rerase);
		}
	
	/*erase the bottom sliver*/ {
	
		rerase = r; 
	
		rerase.top = rerase.bottom - textvertinset;
	
		if (rectneedsupdate (rerase)) 
			eraserect (rerase);
		}
	
	/*erase the left sliver*/ {
	
		rerase = r; 
	
		rerase.right = rerase.left + texthorizinset;
		
		if (rectneedsupdate (rerase)) 
			eraserect (rerase);
		}
	
	/*erase the right sliver*/ {
	
		rerase = r; 
	
		rerase.left = rerase.right - texthorizinset;
		
		#if false /*DW 2/15/95 live with the schmutz for now -- it's breaking other things*/
		
			/*DW 1/28/95 hack for LSE -- noticed schmutz on the right edge when you resize to make the window narrower*/
			
			/*DW 1/6/96 -- LSE is no longer supported, no app record in MacBird Runtime
			
			if (app.creator == 'CAML')
				rerase.right++; 
			*/
		#endif
	
		if (rectneedsupdate (rerase)) 
			eraserect (rerase);
		}
	} /*editupdate*/
	
	
void editupdateport (hdleditrecord hbuffer, Rect rport, WindowPtr w) {
	
	/*
	8/26/92 DW: update the TE handle, but do it in a port different from the
	one it lives inside of. need this to support printing in MinApp.
	
	9/3/92 DW: add "rport" parameter, to allow the caller to determine the
	temporary viewRect for the TE buffer.
	*/
	
	TEHandle macbuffer;
	Rect oldview, olddest;
	WindowPtr oldport;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
		
	oldview = (**macbuffer).viewRect; 
	
	olddest = (**macbuffer).destRect; 
	
	oldport = (WindowPtr) (**macbuffer).inPort;	/* inPort is a GrafPtr */
	
	(**macbuffer).viewRect = rport;
	
	(**macbuffer).destRect = rport;
	
	(**macbuffer).inPort = (GrafPtr) w;
	
	TEUpdate (&rport, macbuffer);
	
	(**macbuffer).viewRect = oldview;
	
	(**macbuffer).destRect = olddest;
	
	(**macbuffer).inPort = (GrafPtr) oldport;
	} /*editupdateport*/
	
	
void editactivate (hdleditrecord hbuffer, boolean flactive) {
	
	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
		
	if (flactive)
		TEActivate (macbuffer);
	else
		TEDeactivate (macbuffer);
	} /*editactivate*/
	
	
void editsetrect (hdleditrecord hbuffer, Rect r) {

	TEHandle macbuffer;

	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
		
	InsetRect (&r, texthorizinset, textvertinset);
			
	(**macbuffer).viewRect = r;
	
	/*if (!(**hbuffer).flwindowbased) -- commented DW 1/16/95 -- for Little Script Editor*/
		(**macbuffer).destRect = r;
	
	TECalText (macbuffer);
	
	/*editupdate (hbuffer);*/
	} /*editsetrect*/
	
	
void editsetfont (hdleditrecord hbuffer, short fontnum, short fontsize) {

	TEHandle macbuffer;

	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
		
	editprescroll (hbuffer);
	
	(**macbuffer).txFont = fontnum;
	
	(**macbuffer).txSize = fontsize;
	
	pushstyle (fontnum, fontsize, (**macbuffer).txFace);
	
	(**macbuffer).fontAscent = globalfontinfo.ascent;
	
	(**macbuffer).lineHeight = globalfontinfo.ascent + globalfontinfo.descent + globalfontinfo.leading;
	
	popstyle ();
		
	TECalText (macbuffer);
	
	editsetscrollvalues (hbuffer);
	
	(**hbuffer).flscrolled = true;
	} /*editsetfont*/
	
	
void editautoscroll (hdleditrecord hbuffer) {

	TEHandle macbuffer;

	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
		
	TEAutoView (true, macbuffer);
	} /*editautoscroll*/
	
	
void editsetwordwrap (hdleditrecord hbuffer, boolean fl) {
	
	TEHandle macbuffer;
	short cronly;

	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
	
	if (fl)
		cronly = 1; /*word-wrap on*/
	else
		cronly = -1; /*no word-wrap*/
		
	(**macbuffer).crOnly = cronly; 
	} /*editsetwordwrap*/
	
	
void editrewrap (hdleditrecord hbuffer) {

	TEHandle macbuffer;

	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
		
	editprescroll (hbuffer);
	
	TECalText (macbuffer);
	
	editsetscrollvalues (hbuffer);
	
	(**hbuffer).flscrolled = true;
	} /*editrewrap*/
	

void editdispose (hdleditrecord hbuffer) {
	
	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
	
	TEDeactivate (macbuffer); /*remove the caret cursor, or unhighlight selection*/
	
	TEDispose (macbuffer);
		
	disposehandle ((Handle) hbuffer);
	} /*editdispose*/


void editdrawtext (bigstring bs, Rect r, tyjustification justification) {
	
	InsetRect (&r, texthorizinset, textvertinset);
	
	TETextBox (&bs [1], stringlength (bs), &r, getTEjustification (justification));
	} /*editdrawtext*/
	

void editdrawtexthandle (Handle htext, Rect r, tyjustification justification) {
	
	if (htext == nil)
		return;
		
	InsetRect (&r, texthorizinset, textvertinset);
	
	lockhandle (htext);
	
	TETextBox (*htext, GetHandleSize (htext), &r, getTEjustification (justification));
	
	unlockhandle (htext);
	} /*editdrawtexthandle*/
	

void editcut (hdleditrecord hbuffer) {
	
	#ifdef onlineOutliner
		Handle htext;
		TEHandle macbuffer;
		
		if (editgetselectedtexthandle (hbuffer, &htext)) {
		
			ZeroScrap ();
		
			PutScrap (GetHandleSize (htext), 'TEXT', *htext);
		
			DisposeHandle (htext);
					
			getmacbuffer (hbuffer, &macbuffer);
			
			if ((**macbuffer).selStart != (**macbuffer).selEnd)
				TEDelete (macbuffer);
			}
	#else
		TEHandle macbuffer;
		
		if (!getmacbuffer (hbuffer, &macbuffer))
			return;
	
		editprescroll (hbuffer);
		
		TECut (macbuffer);
		//Code change by Timothy Paustian Sunday, June 25, 2000 9:52:36 PM
		//Update to new scrap manager API
		#if TARGET_API_MAC_CARBON == 1
		ClearCurrentScrap();
		#else
		ZeroScrap (); /*dmb 1.0b24*/
		#endif
		
		
		TEToScrap (); /*dmb 1.0b24*/
		
		editsetscrollvalues (hbuffer);
	#endif
	} /*editcut*/


void editcopy (hdleditrecord hbuffer) {
	
	#ifdef onlineOutliner
		Handle htext;
		
		if (editgetselectedtexthandle (hbuffer, &htext)) {
			//Code change by Timothy Paustian Sunday, June 25, 2000 9:52:36 PM
		//Update to new scrap manager API
		#if TARGET_API_MAC_CARBON == 1
			ClearCurrentScrap();
			
			ScrapRef			theScrap;
			OSStatus			status;
		    ScrapFlavorType 	flavorType = (ScrapFlavorType) scraptype;
		    ScrapFlavorFlags 	flavorFlags = kScrapFlavorMaskNone;
			Size				flavorSize = GetHandleSize(hscrap);
			
			status = GetCurrentScrap(&theScrap);
			//I don't think this should be fatal, just beep to notify the user.
			if(status != noErr)
			{
				SysBeep(30);
				return;
			}
			HLock(hscrap);
			status = PutScrapFlavor (theScrap, 'TEXT', flavorFlags, flavorSize, *hscrap);
	    	HUnlock(hscrap);
			if(status != noErr)
			{
				SysBeep(30);
				return;
			}
			#else
			ZeroScrap ();
			PutScrap (GetHandleSize (htext), 'TEXT', *htext);
			#endif
			DisposeHandle (htext);
			}
	#else
		TEHandle macbuffer;
	
		if (!getmacbuffer (hbuffer, &macbuffer))
			return;

		TECopy (macbuffer);
		#if TARGET_API_MAC_CARBON == 1
		ClearCurrentScrap();
		#else	
		ZeroScrap (); /*dmb 1.0b24*/
		#endif
		TEToScrap (); /*dmb 1.0b24*/
		
	#endif
	} /*editcopy*/


void editpaste (hdleditrecord hbuffer) {
	
	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;

	editprescroll (hbuffer);
	
	TEFromScrap (); /*DW 4/8/95*/
		
	TEPaste (macbuffer);
	
	editsetscrollvalues (hbuffer);
	} /*editpaste*/
	
	
boolean editgettext (hdleditrecord hbuffer, bigstring bs) {

	TEHandle macbuffer;
	short len;
	
	setstringlength (bs, 0);
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);

	len = (**macbuffer).teLength;
	
	if (len > lenbigstring) /*8/17/93 DW*/
		len = lenbigstring;
		
	setstringlength (bs, len);
			
	moveleft (*(**macbuffer).hText, &bs [1], (long) len);
	
	return (true);
	} /*editgettext*/


void editkeystroke (char chkb, hdleditrecord hbuffer) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
	
	editprescroll (hbuffer);
	
	TEKey (chkb, macbuffer);
	
	editsetscrollvalues (hbuffer);
	} /*editkeystroke*/
	
	
void editclick (Point pt, boolean flshiftkey, hdleditrecord hbuffer) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
		
	editprescroll (hbuffer);

	pushclip ((**macbuffer).viewRect);
	
	TEClick (pt, flshiftkey, macbuffer);
	
	popclip ();
	
	editsetscrollvalues (hbuffer);
	} /*editclick*/
	

boolean editsetselection (hdleditrecord hbuffer, short selstart, short selend) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
		
	editprescroll (hbuffer);

	TESetSelect (selstart, selend, macbuffer);
	
	editsetscrollvalues (hbuffer);
		
	return (true);
	} /*editsetselection*/
	
	
boolean editvisiselection (hdleditrecord hbuffer) { /*DW 1/16/95 -- new feature*/

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
		
	editprescroll (hbuffer);

	TESelView (macbuffer);
	
	editsetscrollvalues (hbuffer);
		
	return (true);
	} /*editvisiselection*/


void editselectall (hdleditrecord hbuffer) {
	
	editsetselection (hbuffer, 0, 32767);
	} /*editselectall*/
	

void editidle (hdleditrecord hbuffer) {
	
	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
	
	if ((**hbuffer).flselectall) {
		
		editselectall (hbuffer);
		
		(**hbuffer).flselectall = false;
		}
	
	TEIdle (macbuffer);
	} /*editidle*/
	
	
boolean editgetselection (hdleditrecord hbuffer, short *selstart, short *selend) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
		
	*selstart = (**macbuffer).selStart;
	
	*selend = (**macbuffer).selEnd;
	
	return (true);
	} /*editgetselection*/


boolean editpointinrect (Point pt, hdleditrecord hbuffer) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);

	return (PtInRect (pt, &(**macbuffer).viewRect));
	} /*editpointinrect*/
	

boolean editreplace (bigstring bs, hdleditrecord hbuffer) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
	
	editprescroll (hbuffer);
	
	if ((**macbuffer).selStart != (**macbuffer).selEnd)
		TEDelete (macbuffer);
	
	if (stringlength (bs) > 0)
		TEInsert (&bs [1], stringlength (bs), macbuffer);
	
	editsetscrollvalues (hbuffer);
	
	return (true); /*TE gives no way to return an error*/
	} /*editreplace*/
	
	
void editsetjustification (tyjustification justification, hdleditrecord hbuffer) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
	
	TESetAlignment (getTEjustification (justification), macbuffer);
	} /*editsetjustification*/


void editgetsize (Handle htext, short *height, short *width) {
	
	/*
	return the size of a rectangle that could contain the indicated string
	observing the defaults for inset both vertically and horizontally, and 
	a minimum width for text display.
	
	assumes that the font/size/style info are correct in globalfontinfo.
	
	8/8/92 DW: upgrade to not use a bigstring to hold the text to be measured.
	we were encountering lines that were longer than 255 chars.
	
	12/30/92 DW: if you just deleted the last char of the buffer and it
	was a carriage return, we'd tell you that you have one more line than
	you actually have. I was tempted to fix the i <= textlength thing in 
	the for loop, but it broke other things. the klooginess is contained
	within this routine.
	*/
	
	long textlength;
	long i;
	short ctlines = 1;
	short maxwidth = 0;
	short vertpixels;
	short ixlinestart = 0, linelength = 0;
	
	textlength = GetHandleSize (htext);
		
	for (i = 0; i <= textlength; i++) {
		
		char ch = (*htext) [i];
			
		if ((ch == chreturn) || (i == textlength)) {
			
			short w;
			
			HLock(htext);
			w = TextWidth (*htext, ixlinestart, linelength);
			HUnlock(htext);
			if (w > maxwidth)
				maxwidth = w;
			
			if (i != textlength) { /*don't count the char after the last valid char*/

				if (ch == chreturn)
					ctlines++;
				}
			
			ixlinestart += linelength + 1;
			
			linelength = 0;
			}
		else
			linelength++;
		} /*for*/

	maxwidth = max (maxwidth, mintextwidth);
	
	maxwidth += 2 * texthorizinset;
	
	maxwidth++; /*ask for one extra pixel, work around TE display anomaly*/
	
	*width = maxwidth;
	
	vertpixels = ctlines * (globalfontinfo.ascent + globalfontinfo.descent + globalfontinfo.leading);
	
	*height = vertpixels + (2 * textvertinset);
	} /*editgetsize*/
	
	
void editgetbuffersize (hdleditrecord hbuffer, short *height, short *width) {
	
	/*
	8/8/92 DW for Snuffy: can't rely on the value for height returned by editgetsize, 
	since it doesn't take word-wrapping into account.
	*/
	
	TEHandle macbuffer;
	char lastchar;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
	
	editgetsize ((**macbuffer).hText, height, width);
	
	if ((**hbuffer).flwindowbased) {
	
		*height = ((**macbuffer).nLines * (**macbuffer).lineHeight) + globalfontinfo.descent;
		
		lastchar = (*(**macbuffer).hText) [(**macbuffer).teLength - 1];
		
		if (lastchar != chreturn) /*a quirk of TE*/
			*height += (**macbuffer).lineHeight;
		
		if ((**macbuffer).crOnly == 1) { /*word-wrapping turned on*/
		
			Rect r = (**macbuffer).destRect;
		
			*width = r.right - r.left;
			}
		}
		
	*width += 2 * texthorizinset;
	
	*height += 2 * textvertinset;
	} /*editgetbuffersize*/
	
	
boolean editgettexthandle (hdleditrecord hbuffer, Handle *htext) {
	
	/*
	return the text handle from the edit buffer. do not dispose the handle or
	alter the length of the text.
	*/

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
		
	*htext = (**macbuffer).hText;
	
	return (true);
	} /*editgettexthandle*/


boolean editnewbufferfromhandle (Rect r, boolean flwordwrap, Handle htext, hdleditrecord *hrecord) {
	
	hdleditrecord h;
	TEHandle macbuffer;
	
	if (!newclearhandle (longsizeof (tyeditrecord), (Handle *) hrecord))
		return (false);
		
	h = *hrecord; /*copy into register*/
	
	InsetRect (&r, texthorizinset, textvertinset);
		
	macbuffer = TENew (&r, &r);
	
	if (macbuffer == nil) { /*memory error*/
		
		disposehandle ((Handle) h);
		
		*hrecord = nil;
		
		return (false);
		}
		
	if (flwordwrap)
		(**macbuffer).crOnly = 1;
	else
		(**macbuffer).crOnly = -1;

	lockhandle (htext);
	
	TESetText (*htext, GetHandleSize (htext), macbuffer);
	
	unlockhandle (htext);
	
	TEActivate (macbuffer);
	
	(**h).macbuffer = macbuffer;
	
	return (true);
	} /*editnewbufferfromhandle*/
	
	
boolean editsettexthandle (hdleditrecord hrecord, Handle htext, boolean flselectall) {
	
	/*
	if (!getmacbuffer (hrecord, &macbuffer))
		return (false);
	
	editprescroll (hrecord);
	
	lockhandle (htext);
	
	TESetText (*htext, GetHandleSize (htext), macbuffer);
	
	unlockhandle (htext);
	
	editsetscrollvalues (hrecord);
	*/
	
	/*
	1.0b20 dmb: new implementation. The old version would not update 
	active text.
	*/
	
	if (!editsetselection (hrecord, 0, infinity))
		return (false);
	
	if (!editreplacehandle (htext, hrecord))
		return (false);
	
	if (flselectall) 
		(**hrecord).flselectall = true;
	
	return (true);
	} /*editsettexthandle*/
	
	
boolean editgettexthandlecopy (hdleditrecord hbuffer, Handle *htext) {
	
	/*
	caller must dispose of the handle we allocate.
	
	7/8/92 DW: if there is no text, return true with htext == nil.
	*/

	TEHandle macbuffer;
	Handle h;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
	
	h = (**macbuffer).hText;
	
	if (GetHandleSize (h) == 0) {
	
		*htext = nil;
		
		return (true);
		}
	
	return (copyhandle (h, htext));
	} /*editgettexthandlecopy*/


boolean edithaveselection (hdleditrecord hbuffer) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
	
	return ((**macbuffer).selStart != (**macbuffer).selEnd);
	} /*edithaveselection*/
	
	
boolean editscroll (hdleditrecord hbuffer, short dh, short dv) {

	TEHandle macbuffer;
	
	if ((dh != 0) || (dv != 0)) {
	
		if (!getmacbuffer (hbuffer, &macbuffer))
			return (false);
			
		editprescroll (hbuffer);
		
		TEScroll (dh, dv, macbuffer);
		
		editsetscrollvalues (hbuffer);
		}
	
	return (true);
	} /*edithaveselection*/
	
	
void editscrollto (hdleditrecord hbuffer, short h, short v) {
		
	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return;
	
	editsetscrollvalues (hbuffer);
	
	editscroll (hbuffer, (**hbuffer).horizcurrent - h, (**hbuffer).vertcurrent - v);
	} /*editscrollto*/


boolean editreplacehandle (Handle htext, hdleditrecord hbuffer) {

	TEHandle macbuffer;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
	
	editprescroll (hbuffer);
	
	if ((**macbuffer).selStart != (**macbuffer).selEnd)
		TEDelete (macbuffer);
		
	lockhandle (htext);
	
	TEInsert (*htext, GetHandleSize (htext), macbuffer);
	
	unlockhandle (htext);
	
	editsetscrollvalues (hbuffer);
	
	return (true); /*TE gives no way to return an error*/
	} /*editreplacehandle*/
	
	
boolean editgetselectedtexthandle (hdleditrecord hbuffer, Handle *htext) {

	TEHandle macbuffer;
	short selstart, selend;
	short lensel;
	Handle h;
	boolean fl;
	
	if (!getmacbuffer (hbuffer, &macbuffer))
		return (false);
		
	selstart = (**macbuffer).selStart;
	
	selend = (**macbuffer).selEnd;
	
	lensel = selend - selstart;
	
	if (lensel < 0)
		lensel = 0;
		
	h = (**macbuffer).hText;
	
	lockhandle (h);
	
	fl = newfilledhandle (&((*h) [selstart]), (long) lensel, htext);
	
	unlockhandle (h);
	
	return (fl);
	} /*editgetselectedtexthandle*/



	
	
