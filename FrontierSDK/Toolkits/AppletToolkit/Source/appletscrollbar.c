
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletinternal.h"
#include "appletops.h"
#include "appletquickdraw.h"
#include "appletmain.h"
#include "appletmsg.h"
#include "appletscrollbar.h"



#define idvertbar 256
#define idhorizbar 257


/*
7/25/94 dmb: updated for Univeral Headers, PowerPC
*/


static boolean scrollbarpushclip (hdlscrollbar hscrollbar) {
	
	/*
	11/19/90 DW: patch things up for the table displayer, and perhaps others in
	the future.  if the scrollbar is of trivial height, we disable the drawing
	that's about to happen.  we were getting one-pixel high scrollbars being
	drawn.
	*/
	
	Rect r = (**hscrollbar).contrlRect;
	
	if ((r.bottom - r.top) == 1)
		r.bottom = r.top;
		
	return (pushclip (r));
	} /*scrollbarpushclip*/
	
	
void validscrollbar (hdlscrollbar hscrollbar) {

	hdlscrollbar h = hscrollbar;
	
	if (h != nil) 
		validrect ((**h).contrlRect);
	} /*validscrollbar*/
	
	
void invalscrollbar (hdlscrollbar hscrollbar) {

	hdlscrollbar h = hscrollbar;
	
	if (h != nil) 
		invalrect ((**h).contrlRect);
	} /*invalscrollbar*/
	
	
boolean pointinscrollbar (Point pt, hdlscrollbar hscrollbar) {

	hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	return (pointinrect (pt, (**h).contrlRect));
	} /*pointinscrollbar*/
	
	
void disablescrollbar (hdlscrollbar hscrollbar) {
	
	hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return;
		
	scrollbarpushclip (h);
		
	HiliteControl (h, -1);
	
	/*validscrollbar (h);*/
	
	popclip ();
	} /*disablescrollbar*/
	
	
void enablescrollbar (hdlscrollbar hscrollbar) {
	
	hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return;
		
	if (!(**h).contrlRfCon) { /*the window containing the scrollbar is inactive*/
		
		disablescrollbar (h);
		
		return;
		}
		
	scrollbarpushclip (h);
		
	HiliteControl (h, 0);
	
	/*validscrollbar (h);*/
	
	popclip ();
	} /*enablescrollbar*/
	
	
boolean scrollbarenabled (hdlscrollbar hscrollbar) {
	
	return ((**hscrollbar).contrlHilite != 255);
	} /*scrollbarenabled*/
	
	
void activatescrollbar (hdlscrollbar hscrollbar, boolean flactivate) {
	
	/*
	8/25/92 DW: use the contrlRfCon field to indicate whether the 
	scrollbar is active or not. this flag is respected by subsequent calls
	to enablescrollbar.
	*/
	
	hdlscrollbar h = hscrollbar;
	
	(**h).contrlRfCon = flactivate;
	
	if (flactivate)
		enablescrollbar (h);
	else
		disablescrollbar (h);
	} /*activatescrollbar*/
	
	
void getscrollbarinfo (hscrollbar, minscroll, maxscroll, current) hdlscrollbar hscrollbar; short *minscroll, *maxscroll, *current; {
	
	hdlscrollbar h = hscrollbar;
	
	if (h == nil) { /*defensive driving*/
	
		*minscroll = *maxscroll = *current = 0;
		
		return;
		}
	
	*minscroll = GetCtlMin (h);
		
	*maxscroll = GetCtlMax (h);
		
	*current = GetCtlValue (h);
	} /*getscrollbarinfo*/
	
	
short getscrollbarcurrent (hdlscrollbar hscrollbar) {
	
	short minscroll, maxscroll, current;
	
	getscrollbarinfo (hscrollbar, &minscroll, &maxscroll, &current);
	
	return (current);
	} /*getscrollbarcurrent*/
	
	
void showscrollbar (hdlscrollbar hscrollbar) {
	
	hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return;
		
	scrollbarpushclip (h);
		
	ShowControl (h); /*no effect if scrollbar is already visible, according to IM-1*/
	
	popclip ();
	
	/*don't validate the scrollbar rect, ShowControl might not draw it...*/
	} /*showscrollbar*/
	
	
void hidescrollbar (hdlscrollbar hscrollbar) {
	
	hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return;
		
	scrollbarpushclip (h);
		
	HideControl (h);
	
	popclip ();
	
	invalrect ((**h).contrlRect);
	} /*hidescrollbar*/
	

void drawscrollbar (hdlscrollbar hscrollbar) {
	
	hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return;
		
	scrollbarpushclip (h);
	
	Draw1Control (h);
	
	popclip ();
	
	validscrollbar (h);
	} /*drawscrollbar*/
	
	
void displayscrollbar (hdlscrollbar hscrollbar) {
	
	/*
	the caller is saying that we should enable it if there is enough 
	material to enable scrolling.
	*/
	
	hdlscrollbar h = hscrollbar;
	short minscroll, maxscroll, current;
	
	if (h == nil) /*defensive driving*/
		return;
		
	getscrollbarinfo (h, &minscroll, &maxscroll, &current);
	
	if (minscroll <= maxscroll) /*there's something to scroll to*/
		enablescrollbar (h);
	else
		disablescrollbar (h);
		
	drawscrollbar (h);
	} /*displayscrollbar*/
	
	
void setscrollbarinfo (hdlscrollbar hscrollbar, short minscroll, short maxscroll, short current) {
	
	/*
	set the bounds of the scrollbar, and its current value.
	
	if the current info agrees with the parameters we do nothing, 
	avoiding unpleasant flicker.
	
	disables drawing while setting the values, and draws it when 
	all three values have been set.  avoids inconsistent displays.
	*/
	
	hdlscrollbar h = hscrollbar;
	short curmin, curmax, curval;
	
	if (h == nil) /*defensive driving*/
		return;
		
	if (maxscroll < minscroll) /*DW 10/3/94*/
		maxscroll = minscroll;
	
	getscrollbarinfo (h, &curmin, &curmax, &curval);
	
	if ((minscroll == curmin) && (maxscroll == curmax) && (current == curval))
		return; /*nothing to do*/
	
	pushemptyclip (); /*disable drawing*/
	
	SetCtlMax (h, maxscroll);
	
	SetCtlMin (h, minscroll);
	
	SetCtlValue (h, current);
	
	popclip ();
	
	displayscrollbar (h);
	} /*setscrollbarinfo*/
	
	
void setscrollbarminmax (hdlscrollbar hscrollbar, short minscroll, short maxscroll) {
	
	hdlscrollbar h = hscrollbar;
	short curmin, curmax, curval;
	
	if (h == nil) /*defensive driving*/
		return;
	
	getscrollbarinfo (h, &curmin, &curmax, &curval);
	
	if ((minscroll != curmin) || (maxscroll != curmax)) {
	
		setscrollbarinfo (h, minscroll, maxscroll, curval);
		}
	} /*setscrollbarminmax*/
	
	
void setscrollbarcurrent (hdlscrollbar hscrollbar, short current) {

	hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return;
	
	scrollbarpushclip (h);
	
	SetCtlValue (h, current);
	
	popclip ();
	} /*setscrollbarcurrent*/


short getscrollbarwidth (void) {
	
	return (16); /*standard Macintosh scrollbars*/
	} /*getscrollbarwidth*/


boolean newscrollbar (WindowPtr w, boolean flvert, hdlscrollbar *hscrollbar) {
	
	/*
	create a new scroll bar linked into the control list of the indicated
	window.  
	
	we need to know if it is a vertical or horizontal scrollbar so we can
	choose the proper resource template.  we use a CDEF that has different
	specs for horiz and vertical scrollbars.
	*/
	
	hdlscrollbar h;
	short resnum;
	
	if (flvert)
		resnum = idvertbar;
	else
		resnum = idhorizbar;
	
	*hscrollbar = h = GetNewControl (resnum, w);
	
	return (h != nil);
	} /*newscrollbar*/
	
	
void disposescrollbar (hdlscrollbar hscrollbar) {
	
	hdlscrollbar h = hscrollbar;
	
	if (h != nil)
		DisposeControl (h);
	} /*disposescrollbar*/
	
	
void getscrollbarrect (hdlscrollbar hscrollbar, Rect *r) {

	hdlscrollbar h = hscrollbar;
	
	if (h != nil) 
		*r = (**h).contrlRect;
	else
		zerorect (r);
	} /*getscrollbarrect*/
	
	
void setscrollbarrect (hdlscrollbar hscrollbar, Rect r) {

	hdlscrollbar h = hscrollbar;
	
	if (h != nil) {
		
		HideControl (h);
		
		SizeControl (h, r.right - r.left, r.bottom - r.top);
		
		MoveControl (h, r.left, r.top);
		}
	} /*setscrollbarrect*/
	
	
void scrollbarflushright (Rect r, hdlscrollbar hscrollbar) {
	
	hdlscrollbar h = hscrollbar;
	short width;
	
	if (h == nil) /*defensive driving*/
		return;
		
	width = getscrollbarwidth ();
	
	HideControl (h); /*also invals the rectangle that the scrollbar currently occupies*/
	
	SizeControl (h, width, r.bottom - r.top + 2);
	
	MoveControl (h, r.right - width + 1, r.top - 1);
	} /*scrollbarflushright*/
	
	
void scrollbarflushbottom (Rect r, hdlscrollbar hscrollbar) {
	
	hdlscrollbar h = hscrollbar;
	short width;
	
	if (h == nil) /*defensive driving*/
		return;
		
	width = getscrollbarwidth ();
		
	HideControl (h);
	
	SizeControl (hscrollbar, r.right - r.left, width);
		
	MoveControl (hscrollbar, r.left, r.bottom - width + 1);
	} /*scrollbarflushbottom*/
	
	
boolean findscrollbar (Point pt, WindowPtr w, hdlscrollbar *hscrollbar, short *scrollbarpart) {
	
	*scrollbarpart = FindControl (pt, w, hscrollbar);
	
	return (*hscrollbar != nil);
	} /*findscrollbar*/
	
	
boolean scrollbarhit (hdlscrollbar hscrollbar, short part, boolean *flup, boolean *flpage) {
	
	/*
	can be called by a routine that tracks a mouse hit in a scroll bar.
	
	return true if it represents a valid scroll action, according to the info
	stored in the scrollbar handle.
	
	return false if the indicated action would move beyond the min or max for
	the scrollbar.
	
	set flup and flpage according to the indicated scrollbar part.
	*/
	
	hdlscrollbar h = hscrollbar;
	short minscroll, maxscroll, current;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	getscrollbarinfo (h, &minscroll, &maxscroll, &current);
	
	switch (part) {
	
		case inUpButton:
			*flup = false; /*scroll text down*/
			
			*flpage = false;
			
			return (current > minscroll);
			
		case inDownButton:
			*flup = true; /*scroll text up*/
			
			*flpage = false;
			
			return (current < maxscroll);
			
		case inPageUp:
			*flup = false; /*scroll text down*/
			
			*flpage = true; 
			
			return (current > minscroll);
			
		case inPageDown:
			*flup = true; /*scroll text up*/
			
			*flpage = true;
			
			return (current < maxscroll);
		} /*switch*/
		
	return (false); /*fell through the switch statement*/
	} /*scrollbarhit*/
	

void resizeappscrollbars (appwindow) hdlappwindow appwindow; {
	
	/*
	DW 6/27/93: allow for message area, takes space from the 
	horiz scrollbar.
	
	DW 10/27/93: inval the message area for window resizing.
	*/
	
	hdlappwindow ha = appwindow;
	
	if (app.hasmessagearea) {
		
		invalrect ((**ha).messagerect);
			
		appsetmessagerect (ha);
		}
	
	if (app.horizscroll || app.vertscroll) {
		
		short scrollbarwidth;
		Rect rwholewindow = (**ha).windowrect;
		Rect r;
		
		scrollbarwidth = getscrollbarwidth ();
	
		if (app.vertscroll) {
	
			r = rwholewindow; 
		
			r.bottom -= scrollbarwidth - 1;
			
			r.top += (**ha).statuspixels; /*leave room for status bar at top of window*/
			
			r.top = (**ha).contentrect.top;
		
			scrollbarflushright (r, (**ha).vertbar);
			}
		
		if (app.horizscroll) {
		
			r = rwholewindow; 
		
			r.right -= scrollbarwidth - 2;
			
			if (app.hasmessagearea)
				r.left = (**ha).messagerect.right - 1;
			else
				r.left -= 1; 
			
			scrollbarflushbottom (r, (**ha).horizbar);
			}
		}
	} /*resizeappscrollbars*/
	
	
void showappscrollbars (appwindow) hdlappwindow appwindow; {
	
	hdlappwindow ha = appwindow;
	
	if (app.vertscroll)
		showscrollbar ((**ha).vertbar);
	
	if (app.horizscroll)
		showscrollbar ((**ha).horizbar);
	} /*showappscrollbars*/
	
	
void updateappscrollbars (appwindow) hdlappwindow appwindow; {
	
	hdlappwindow ha = appwindow;
		
	if (app.vertscroll) {
		
		showscrollbar ((**ha).vertbar);
		
		drawscrollbar ((**ha).vertbar);
		}
		
	if (app.horizscroll) {
		
		showscrollbar ((**ha).horizbar);
		
		drawscrollbar ((**ha).horizbar);
		}
	} /*updateappscrollbars*/
	
	
void activateappscrollbars (hdlappwindow appwindow, boolean flactivate) {
	
	hdlappwindow ha = appwindow;
	
	if (app.vertscroll) 	
		activatescrollbar ((**ha).vertbar, flactivate);
		
	if (app.horizscroll) 	
		activatescrollbar ((**ha).horizbar, flactivate);
	} /*activateappscrollbars*/
	
	
tydirection scrolldirection (boolean flvert, boolean flup) {

	tydirection dir;
	
	if (flvert) {
		if (flup)
			dir = up;
		else
			dir = down;
		}
	else {
		if (flup)
			dir = left;
		else
			dir = right; 
		}
		
	return (dir);
	} /*scrolldirection*/ 


static pascal void apphorizscroll (hdlscrollbar ctrl, short part) {
	
	tydirection dir;
	boolean flleft, flpage;
	
	if (!scrollbarhit (ctrl, part, &flleft, &flpage))
		return;
		
	dir = scrolldirection (false, flleft);
		
	scrollappwindow (dir, flpage, 1);
	} /*apphorizscroll*/


static pascal void appvertscroll (hdlscrollbar sb, short part) {
	
	tydirection dir;
	boolean flup, flpage;
	
	if (!scrollbarhit (sb, part, &flup, &flpage))
		return;
	
	dir = scrolldirection (true, flup);
		
	scrollappwindow (dir, flpage, 1);
	} /*appvertscroll*/

#if USESROUTINEDESCRIPTORS

	static RoutineDescriptor apphorizscrollDesc = BUILD_ROUTINE_DESCRIPTOR (uppControlActionProcInfo, apphorizscroll);
	static RoutineDescriptor appvertscrollDesc = BUILD_ROUTINE_DESCRIPTOR (uppControlActionProcInfo, appvertscroll);
	
	#define apphorizscrollUPP &apphorizscrollDesc
	#define appvertscrollUPP &appvertscrollDesc
	
#else

	#define apphorizscrollUPP &apphorizscroll
	#define appvertscrollUPP &appvertscroll

#endif

void handlescrollbar (boolean flvert, hdlscrollbar sb, short part, Point pt) {
	
	short oldscrollbarcurrent;
	
	if (part == inThumb) {
		
		oldscrollbarcurrent = getscrollbarcurrent (sb);
		
		scrollbarpushclip (sb);
		
		TrackControl (sb, pt, nil);
			
		popclip ();
		
		scrolltoappwindow ();
		
		/*
		ctscroll = getscrollbarcurrent (sb) - oldscrollbarcurrent;
		
		dir = scrolldirection (flvert, ctscroll > 0);
		
		(*app.scrollcallback) (dir, false, abs (ctscroll));
		*/
		}
	else {
		scrollbarpushclip (sb);
		
		if (flvert)
			TrackControl (sb, pt,appvertscrollUPP);
		else
			TrackControl (sb, pt, apphorizscrollUPP);
		
		invalscrollbar (sb); /*DW 8/19/94*/
		
		popclip ();
		}
	} /*handlescrollbar*/
	
	
boolean appdefaultresetscroll (void) {
		
	hdlappwindow ha = app.appwindow;
	Rect rcontent;
	short vertmin, vertmax, vertcurrent;	
	short horizmin, horizmax, horizcurrent;
	
	appprecallback ();
		
	(*app.getcontentsizecallback) ();
	
	apppostcallback ();
	
	getscrollbarinfo ((**ha).vertbar, &vertmin, &vertmax, &vertcurrent);
	
	getscrollbarinfo ((**ha).horizbar, &horizmin, &horizmax, &horizcurrent);
	
	rcontent = (**ha).contentrect;
	
	/*reset the vertical scrollbar*/ {
		
		short needpixels = (**ha).zoomheight;
		short havepixels = rcontent.bottom - rcontent.top;
		hdlscrollbar bar = (**ha).vertbar;
		
		if (havepixels >= needpixels) {
			
			disablescrollbar (bar);
			
			/*setscrollbarinfo (bar, 0, 0, 0);*/
			}
		else {
			short x = 0;
			
			enablescrollbar (bar);
			
			vertmin = 0;
			
			vertmax = needpixels - havepixels;
			}
		}
		
	/*reset the horiz scrollbar*/ {
		
		short needpixels = (**ha).zoomwidth;
		short havepixels = rcontent.right - rcontent.left;
		hdlscrollbar bar = (**ha).horizbar;
		
		if (havepixels >= needpixels) {
			
			disablescrollbar (bar);
			
			/*setscrollbarinfo (bar, 0, 0, 0);*/
			}
		else {
			short x = 0;
			
			enablescrollbar (bar);
			
			horizmin = 0;
			
			horizmax = needpixels - havepixels;
			}
		}
	
	setscrollbarinfo ((**ha).vertbar, vertmin, vertmax, vertcurrent);
	
	setscrollbarinfo ((**ha).horizbar, horizmin, horizmax, horizcurrent);
	
	return (true);
	} /*appdefaultresetscroll*/
	

boolean appdefaultscroll (tydirection dir, boolean flpage, short ctscroll) {
	
	/*
	if the applet didn't define a scrolling routine, this routine will
	be called to do the scrolling. it implements a very bare-bones way
	of scrolling, that often is just what you want... in order for this
	to work, the app must define a scrollto callback routine.
	*/
	
	hdlappwindow ha = app.appwindow;
	short ct = ctscroll;
	short dh = 0, dv = 0;
	Rect r;
	
	if ((ct == 1) && (!flpage)) { /*it's heuristic time!*/
		
		if (!optionkeydown ()) {
			
			ct = getfontheight (); /*scroll by this (reasonable) amount*/
			
			/*user holds down the option key to get a 2-pixel scroll*/
			}
		}
	
	if (ct % 2) /*it's an odd number*/
		ct++;
	
	r = (**ha).contentrect;
	
	switch (dir) {
		
		case down:
			if (flpage)
				dv = -(r.bottom - r.top);
			else
				dv = -ct;
			
			break;
			
		case up:
			if (flpage)
				dv = r.bottom - r.top;
			else
				dv = ct;
			
			break;
		
		case right:
			if (flpage)
				dh = -(r.right - r.left);
			else
				dh = -ct;
				
			break;
			
		case left:
			if (flpage)
				dh = r.right - r.left;
			else
				dh = ct;
				
			break;
			
		default:
			return (false);
		} /*switch*/
	
	if ((dv != 0) && scrollbarenabled ((**ha).vertbar)) {
		
		short x;
		short vertmin, vertmax, vertcurrent;
		
		getscrollbarinfo ((**ha).vertbar, &vertmin, &vertmax, &vertcurrent);
		
		x = vertcurrent + dv;
	
		x = max (x, vertmin);
	
		x = min (x, vertmax);
		
		dv = vertcurrent - x;
		
		vertcurrent = x;
		
		scrollbarpushclip ((**ha).vertbar);
		
		SetCtlValue ((**ha).vertbar, vertcurrent);
		
		Draw1Control ((**ha).vertbar);
		
		popclip ();
		
		/*setscrollbarinfo ((**ha).vertbar, vertmin, vertmax, vertcurrent);*/
		}
	
	if ((dh != 0) && scrollbarenabled ((**ha).horizbar)) {
		
		short x;
		short horizmin, horizmax, horizcurrent;
		
		getscrollbarinfo ((**ha).horizbar, &horizmin, &horizmax, &horizcurrent);
		
		x = horizcurrent + dh;
	
		x = max (x, horizmin);
	
		x = min (x, horizmax);
		
		dh = horizcurrent - x;
		
		horizcurrent = x;
		
		scrollbarpushclip ((**ha).horizbar);
		
		SetCtlValue ((**ha).horizbar, horizcurrent);
		
		Draw1Control ((**ha).horizbar);
		
		popclip ();
		
		/*setscrollbarinfo ((**ha).horizbar, horizmin, horizmax, horizcurrent);*/
		}
	
	if ((dh != 0) || (dv != 0)) {
		
		scrolltoappwindow ();
		
		/*
		scrollrect (r, dh, dv);
	
		updateappwindow (ha);
		*/
		}
		
	return (true);
	} /*appdefaultscroll*/
	
	
boolean scrollappwindow (tydirection dir, boolean flpage, short ctscroll) {
	
	boolean fl;
	
	appprecallback ();
		
	fl = (*app.scrollcallback) (dir, flpage, 1);
	
	apppostcallback ();
	
	return (fl);
	} /*scrollappwindow*/
	

boolean scrolltoappwindow (void) {

	boolean fl;
	
	appprecallback ();
	
	fl = (*app.scrolltocallback) ();
	
	apppostcallback ();
	
	return (fl);
	} /*scrolltoappwindow*/
	
		
void resetappscrollbars (hdlappwindow appwindow) {
	
	hdlappwindow savedwindow = app.appwindow;
	
	setappwindow (appwindow);
	
	appprecallback ();
	
	(*app.resetscrollcallback) ();
	
	apppostcallback ();
	
	setappwindow (savedwindow);
	} /*resetappscrollbars*/
	
	
void installscroll (void) {
	
	if ((app.horizscroll) || (app.vertscroll)) {
	
		if (app.scrollcallback == nil)
			app.scrollcallback = &appdefaultscroll;
			
		if (app.resetscrollcallback == nil)
			app.resetscrollcallback = &appdefaultresetscroll;
		}
	} /*installscroll*/
	
	
