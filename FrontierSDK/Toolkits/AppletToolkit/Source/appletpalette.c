
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"


#define drawinset 3
#define ctperline 2
#define heightpaletteicon (heightsmallicon + 6)
#define widthpaletteicon (widthsmallicon + 6)
#define breakpixels 4



static void geticonrect (hdlpaletterecord hpal, short iconnum, Rect *ricon) {
	
	/*
	iconnum is 0-based.
	*/
	
	Rect rpal = (**hpal).r;
	short extrapixels = 0;
	Rect r;
	short lnum, ixline;
	short i;
	
	lnum = 0;
	
	ixline = 0;
	
	for (i = 0; i < iconnum; i++) {
	
		if ((**hpal).item [i].breakafter) {
		
			extrapixels += breakpixels;
			
			ixline = 0;
			
			lnum++;
			}
		else {
			ixline++;
			
			if (ixline > (ctperline - 1)) {
				
				ixline = 0;
				
				lnum++;
				}
			}
		} /*for*/
	
	r.top = rpal.top + drawinset + (lnum * (heightpaletteicon - 1)) + extrapixels;
	
	r.bottom = r.top + heightpaletteicon;
	
	r.left = rpal.left + drawinset;
	
	if (ixline % 2)
		r.left += widthpaletteicon - 1;
	
	r.right = r.left + widthpaletteicon;
	
	*ricon = r;
	} /*geticonrect*/
	
	
void getpalettesize (hdlpaletterecord hpal, short *height, short *width) {
	
	/*
	called when zooming windows that contain palettes.
	*/
	
	Rect rpal = (**hpal).r;
	Rect r;
	
	geticonrect (hpal, (**hpal).cticons, &r);
	
	*height = r.bottom - rpal.top + (4 * drawinset);
	
	*width = r.right - drawinset;
	} /*getpalettesize*/
	

static void oldgeticonrect (hdlpaletterecord hpal, short iconnum, Rect *ricon) {
	
	/*
	iconnum is 0-based.
	*/
	
	Rect rpal = (**hpal).r;
	short extrapixels = 0;
	Rect r;
	short lnum;
	short i;
	
	for (i = 0; i < iconnum; i++) {
	
		if ((**hpal).item [i].breakafter)
			extrapixels += breakpixels;
		} /*for*/
	
	lnum = (iconnum / ctperline);
	
	r.top = rpal.top + drawinset + (lnum * (heightpaletteicon - 1)) + extrapixels;
	
	r.bottom = r.top + heightpaletteicon;
	
	r.left = rpal.left + drawinset;
	
	if (iconnum % 2)
		r.left += widthpaletteicon - 1;
	
	r.right = r.left + widthpaletteicon;
	
	*ricon = r;
	} /*geticonrect*/
	
	
hdlpaletterecord newpalette (WindowPtr macwindow, short cticons) {

	hdlpaletterecord hpal;
	
	if (!newclearhandle (longsizeof (typaletterecord), (Handle *) &hpal))
		return (nil);
	
	(**hpal).macwindow = macwindow;
	
	(**hpal).cticons = cticons;
	
	(**hpal).palettewidth = (2 * drawinset) + (2 * widthpaletteicon);
	
	return (hpal);
	} /*newpalette*/
	
	
void disposepalette (hdlpaletterecord hpal) {
	
	disposehandle ((Handle) hpal);
	} /*disposepalette*/
	
	
static void centersmallicon (Rect r, short iconlist, short iconnum, boolean flinvert) {
	
	r.top += (r.bottom - r.top - heightsmallicon) / 2;
	
	r.left += (r.right - r.left - widthsmallicon) / 2;
	
	plotsmallicon (r, iconlist, iconnum, flinvert);
	} /*centersmallicon*/
	
	
void paletteupdate (hdlpaletterecord hpal) {

	boolean flbitmap = false;
	Rect r = (**hpal).r;
	short i;
	
	/*flbitmap = openbitmap (r, (**hpal).macwindow);*/
	
	pushbackcolor (&lightbluecolor);
		
	EraseRect (&r);
	
	MoveTo (r.right - 1, r.top);
	
	LineTo (r.right - 1, r.bottom);
	
	for (i = 0; i < (**hpal).cticons; i++) {
		
		geticonrect (hpal, i, &r);
		
		pushbackcolor (&whitecolor);
		
		EraseRect (&r);
		
		centersmallicon (r, (**hpal).sicnresource, i, false);
		
		popbackcolor ();
		
		FrameRect (&r);
		
		if ((**hpal).item [i].selected) {
			
			InsetRect (&r, 2, 2);
			
			InvertRect (&r);
			}
		} /*for*/
	
	popbackcolor ();
		
	if (flbitmap)
		closebitmap ((**hpal).macwindow);
	} /*updatepalette*/
	
	
void invalpalette (hdlpaletterecord hpal) {
	
	InvalRect (&(**hpal).r);
	} /*invalpalette*/
	
	
void invalpaletteitem (hdlpaletterecord hpal, short itemnum) {
	
	Rect r;
	
	geticonrect (hpal, itemnum, &r);
		
	InvalRect (&r);
	} /*invalpaletteitem*/
	
	
void palettemousedown (hdlpaletterecord hpal) {
	
	Rect rpalette = (**hpal).r;
	short i;
	
	for (i = 0; i < (**hpal).cticons; i++) {
	
		Rect r;
		
		geticonrect (hpal, i, &r);
		
		if (PtInRect (mousestatus.localpt, &r)) {
			
			(**hpal).itemselected = i;
			
			(*(**hpal).itemhitcallback) ();
			}
		} /*for*/
	
	/*invalpalette (hpal);*/
	} /*palettemousedown*/
	
	
void paletteactivate (hdlpaletterecord hpal, boolean flactivate) {
	} /*paletteactivate*/
	

void paletteselectitem (hdlpaletterecord hpal, short itemnum) {
	
	short i;
	
	for (i = 0; i < maxpaletteitems; i++) {
		
		if ((**hpal).item [i].selected)
			invalpaletteitem (hpal, i);
			
		(**hpal).item [i].selected = false;
		} /*for*/
		
	if (!(**hpal).item [itemnum].selected)
		invalpaletteitem (hpal, itemnum);
		
	(**hpal).item [itemnum].selected = true;
	
	/*invalpalette (hpal);*/
	} /*paletteselectitem*/
	
	
	