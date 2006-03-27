
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

 
/*
quickdraw.c -- code which builds on top of basic quickdraw operations.
*/

#include "frontier.h"
#include "standard.h"

#ifdef MACVERSION
	#include "mac.h"
#endif

#include "font.h"
#include "memory.h"
#include "ops.h"
#include "strings.h"
#include "quickdraw.h"
#include "frontierwindows.h"
#include "shell.h"


#ifdef WIN95VERSION
	HWND getcurrentwindow();
	HBRUSH getcurrentbrush();
	extern HINSTANCE hInst;
	short gPenPositionX;
	short gPenPositionY;
#endif

#ifdef MACVERSION
	#define stockgray gray
	#define stockblack black
	#define getstockpattern(x) qd.x
#endif

#ifdef WIN95VERSION
	#define stockgray GRAY_BRUSH
	#define stockblack BLACK_BRUSH
	#define getstockpattern(x) GetStockObject(x)
	#define WINRGB(rgb) RGB((rgb).red >> 8,(rgb).green >> 8, (rgb).blue >> 8)
#endif

RGBColor whitecolor = {65535, 65535, 65535};

RGBColor blackcolor = {0, 0, 0};

RGBColor greencolor = {0, 32767, 0};

RGBColor lightbluecolor = {52428, 52428, 65535};

RGBColor darkbluecolor = {0, 0, 32767};

RGBColor darkgraycolor = {10922, 10922, 10922};

RGBColor graycolor = {32767, 32767, 32767};

RGBColor mediumgraycolor = {52428, 52428, 52428};

RGBColor lightgraycolor = {61166, 61166, 61166};

RGBColor lightyellowcolor = {65535, 65535, 52428};

RGBColor lightgreencolor = {52428, 65535, 52428};

RGBColor lightpurplecolor = {65535, 52428, 65535};

RGBColor darkpurplecolor = {16384, 0, 16384};

RGBColor lightredcolor = {65535, 52428, 52428};

RGBColor darkredcolor = {32767, 0, 0};

RGBColor darkgreencolor = {0, 32767, 0};


#define qdfillrect(r, p) FillRect (r, p)

#ifdef fldebug

	#define checkdepth(top, maxtop) maxtop = (top > maxtop? top : maxtop)
	
	static short maxpen = 0, maxport = 0, maxclip = 0, maxstyle = 0;

#else

	#define checkdepth(top, maxtop)

#endif


typedef struct tystylerecord  {
	
	short fnum, fsize, fstyle;
	} tystylerecord;


#define ctports 20 /*we can remember ports up to 5 levels deep*/
#define ctclip 20 /*we can remember clips up to 5 levels deep*/
#define ctpens 4 /*we can remember pens up to 5 levels deep*/
#define ctstyle 5 /*we can remember font/size/styles up to 5 levels deep*/
#define ctforecolors 5
#define ctbackcolors 5

static short topport = 0;
static short topclip = 0;
static short toppen = 0;
static short topstyle = 0;
static short topforecolor = 0;
static short topbackcolor = 0;

static CGrafPtr portstack [ctports];
static hdlregion clipstack [ctclip];
static tystylerecord stylestack [ctstyle];

#ifdef MACVERSION
	static PenState penstack [ctpens];
	static RGBColor forecolorstack [ctforecolors];
	static RGBColor backcolorstack [ctbackcolors];
#endif

#ifdef WIN95VERSION
	static HPEN penstack[ctpens];
	static COLORREF forecolor;
	static COLORREF backcolor = (COLORREF) (COLOR_BTNFACE + 1); //should agree with class back brush in FrontierWinMain
	static COLORREF forecolorstack [ctforecolors];
	static COLORREF backcolorstack [ctbackcolors];
#endif


static hdlregion scratchrgn;

#ifdef MACVERSION
	static CGrafPtr scratchport;
	//Code change by Timothy Paustian Friday, May 5, 2000 9:50:05 PM
	//Changed to Opaque call for Carbon
	//I had to change this because carbon will not allow
	//GrafPorts (revealing the structure) you have to use a 
	//CGrafPtr. This may be deadly. I will test this.
	#define inscratchport() (getcurrentDC() == scratchport)

#endif

#ifdef WIN95VERSION
	static HDC scratchport;
	HWND currentport;
	HDC currentportDC;
	static short winportpushed = 0;
	HBRUSH currentwindowbrush;
	static BOOL winclipstack [ctclip]; // return values from GetClipRgn
	//Code change by Timothy Paustian Friday, May 5, 2000 9:50:05 PM
	//Changed to Opaque call for Carbon
	//I had to change this because carbon will not allow
	//GrafPorts (revealing the structure) you have to use a 
	//GrafPtr. I don't think I did anything here.
	#define inscratchport() (getcurrentDC() == scratchport) //07/06/2000 AR: getcurrentDC returns a HDC, so compare with scratchport instead of &scratchpart

#endif


/*Forward*/

static short maxdepth (Rect *r);


#ifdef WIN95VERSION

HBRUSH getcurrentbrush() {

	return (currentwindowbrush);
	} /*getcurrentbrush*/


HDC getcurrentDC (void) {
	
	return (currentportDC);
	} /*getcurrentDC*/


boolean winpushport (WindowPtr w, HDC hdc) {
	
	if (topport >= ctports) {
		
		shellinternalerror (idportstackfull, "\x13" "port stack overflow");
		
		return (false);
		}
	
	assert (winportpushed == 0);

	portstack [topport++] = currentport;
	
	if (currentport != NULL)
		ReleaseDC (currentport, currentportDC);
	
	currentport = w;
	
	currentportDC = hdc;
	
	++winportpushed;
	
	checkdepth (topport, maxport);
	
	return (true);
	} /*winpushport*/


boolean winpopport () {
	
	currentport = NULL;
	
	currentportDC = NULL;
	
	--winportpushed;
	
	return (popport ());
	} /*winpopport*/

#endif


short getmenubarheight (void) {
#ifdef MACVERSION	
	return (GetMBarHeight ()); /*call Script Manager routine*/
#endif

#ifdef WIN95VERSION
	return (GetSystemMetrics (SM_CYMENUSIZE));
#endif
	} /*getmenubarheight*/


GrafPtr getport (void) {

	#ifdef MACVERSION	
		//Code change by Timothy Paustian Friday, June 9, 2000 9:57:48 PM
		//Changed to Opaque call for Carbon
		//This is weird, Shouldn't this be calling GetPort()?
		#if TARGET_API_MAC_CARBON == 1
		return GetQDGlobalsThePort();
		#else
		return (quickdrawglobal (thePort));
		#endif
	#endif

	#ifdef WIN95VERSION
		return (currentport);
	#endif
	} /*getport*/


void setport (GrafPtr port) {

	#ifdef MACVERSION	
		SetPort (port);
	#endif

	#ifdef WIN95VERSION
		if (winportpushed)
			return;

		if (port != currentport) {
			
			if (currentport != NULL) {
				
				ReleaseDC (currentport, currentportDC);
				
				currentport = NULL;
				
				currentportDC = NULL;
				}
			
			if (port != NULL) {
				
				currentport = port;
				
				currentportDC = GetDC (port);
				}
			}
	#endif
	} /*setport*/
		

boolean pushport (CGrafPtr p) {
	/*
	5.0.2b4 dmb: must reset global fontinfo when port changes
	*/
	
	if (topport >= ctports) {
		
		shellinternalerror (idportstackfull, "\x13" "port stack overflow");
		
		return (false);
		}
	
	#ifdef MACVERSION
		//Code change by Timothy Paustian Friday, June 9, 2000 9:58:19 PM
		//Changed to Opaque call for Carbon
		//Comment by Timothy Paustian Wednesday, July 12, 2000 12:57:04 PM
		//When a dialog comes up, the globalport becomes invalid. I need 
		//to find out why.
		{
		CGrafPtr	theGlobalPort = nil;
		#if TARGET_API_MAC_CARBON == 1
		theGlobalPort = GetQDGlobalsThePort();
		#else
		theGlobalPort = quickdrawglobal (thePort);
		#endif
		
		portstack [topport++] = theGlobalPort;
		
		if ((p != nil) && (p != theGlobalPort)) {
			
			SetPort (p);
			GetFontInfo (&globalfontinfo);
			}
		}
	#endif

	#ifdef WIN95VERSION
		portstack [topport++] = currentport;
		setport (p);
	#endif
	
	checkdepth (topport, maxport);

	return (true);
	} /*pushport*/
		

boolean popport (void) {

	/*
	5.0.2b4 dmb: must reset global fontinfo when port changes
	*/
	
	register CGrafPtr p;
	
	if (topport <= 0) {
		
		shellinternalerror (idportstackempty, "\x11" "too many popports");
		
		return (false);
		}
	
	p = portstack [--topport];
	
	#ifdef MACVERSION
	{
		//Code change by Timothy Paustian Friday, June 9, 2000 10:01:35 PM
		//Changed to Opaque call for Carbon
		CGrafPtr	theQDPort = 
		#if TARGET_API_MAC_CARBON == 1
		GetQDGlobalsThePort();
		#else
		quickdrawglobal (thePort);
		#endif
		
		if ((p != nil) && (p != theQDPort)) {
			
			SetPort (p);
			
			GetFontInfo (&globalfontinfo);
			}
	}
	#endif
	
	#ifdef WIN95VERSION
		setport (p);
	#endif
	
	return (true);
	} /*popport*/


boolean pushscratchport (void) {
	
	/*
	2.1b4 dmb: call this when a port is needed, but no window is available
	*/
	#ifdef MACVERSION
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = scratchport; /* hra: scratchport is already a CGrafPtr, not a WindowPtr */
	#else
	thePort = (CGrafPtr)scratchport;
	#endif
		
	return pushport (thePort);
	#endif
	#ifdef WIN95VERSION
		return (pushport (currentport));
	#endif
	} /*pushscratchport*/


#if !flruntime

boolean pushcliprgn (hdlregion rgnclip, boolean flomit) {

	#ifdef MACVERSION	
	register hdlregion rgn;
	#endif
	
	if (topclip >= ctclip) {
		
		shellinternalerror (idregionstackfull, "\x15" "region stack overflow");
		
		return (false);
		}
	
	#ifdef MACVERSION	
		rgn = clipstack [topclip++];
		
		GetClip (rgn);

		if (flomit)
			DiffRgn (rgn, rgnclip, scratchrgn);
		else
			SectRgn (rgn, rgnclip, scratchrgn);
		
		SetClip (scratchrgn);
	#endif
	#ifdef WIN95VERSION
		winclipstack [topclip] = GetClipRgn (getcurrentDC(), clipstack [topclip]);
		
		topclip++;
		
		ExtSelectClipRgn (getcurrentDC(), rgnclip, flomit?RGN_DIFF:RGN_AND); // 5.13.97 dmb: was OR
	#endif	
	
	checkdepth (topclip, maxclip);

	return (true);
	} /*pushcliprgn*/


boolean pushclip (Rect r) {

#ifdef MACVERSION	
	RectRgn (scratchrgn, &r);
	
	return (pushcliprgn (scratchrgn, false));
#endif

#ifdef WIN95VERSION
	hdlregion rgn;
	boolean res;

	rgn = CreateRectRgn (r.left, r.top, r.right, r.bottom);
	res = pushcliprgn (rgn, false);
	DeleteObject (rgn);
	return (res);
#endif
	} /*pushclip*/


boolean superpushclip (Rect r) {
	
	/*
	pushclip is scoped, that is if you do a pushclip
	while one is already outstanding, you can't draw outside of the
	earlier rectangle...  superpushclip is the escape valve, very
	rarely called. but it allows you to break this rule.
	
	we put it in early so that we could start calling it.
	
	2/27/91 dmb: pushclip now provides true clip nesting, so this procedure 
	may be needed for real
	*/

	if (topclip >= ctclip) {
		
		shellinternalerror (idclipstackfull, "\x13" "clip stack overflow");
		
		return (false);
		}

#ifdef MACVERSION	
	GetClip (clipstack [topclip++]);

	ClipRect (&r);
#endif	
#ifdef WIN95VERSION
	{
	hdlregion rgn;

	winclipstack [topclip] = GetClipRgn (getcurrentDC(), clipstack [topclip]);
	
	topclip++;
	
	rgn = CreateRectRgn (r.left, r.top, r.right, r.bottom);
	
	SelectClipRgn (getcurrentDC(), rgn);
	
	DeleteObject (rgn);
	}
#endif

	checkdepth (topclip, maxclip);

	return (true);
	} /*superpushclip*/


boolean pushvalidrgnclip (void) {
	
	/*
	set up the clip region so that it excludes any part of the window 
	that has a pending update; i.e. clip to the valid part of the window.
	*/
	
#ifdef MACVERSION
	//Code change by Timothy Paustian Monday, May 1, 2000 9:47:33 PM
	//Changed to Opaque call for Carbon
	//we are assuming that scratchrgn has already been allocated.
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	WindowRef	theWindow;
	CGrafPtr	thePort = GetQDGlobalsThePort();
	theWindow = GetWindowFromPort(thePort);
	//This probably needs to change after the bug in kWindowUpdateRgn is fixed
	//check the carbon release notes I seem to rememebr something about
	//this not working.
	GetWindowRegion(theWindow, kWindowContentRgn, scratchrgn);
	#else
	//old code
	CopyRgn ((*(WindowPeek) quickdrawglobal (thePort)).updateRgn, scratchrgn);
	#endif
	globaltolocalrgn (scratchrgn);
	
	return (pushcliprgn (scratchrgn, true));
#endif

#ifdef WIN95VERSION
	hdlregion rgn;
	boolean res;

	rgn = CreateRectRgn (0,0,1,1);
	GetUpdateRgn (getcurrentwindow(), rgn, false);
	res = pushcliprgn (rgn, true);
	DeleteObject (rgn);
	return (res);
#endif
	} /*pushvalidrgnclip*/


boolean pushemptyclip (void) {
	
	/*
	set up the clip region so that no display will occur
	*/
#ifdef MACVERSION	
	SetEmptyRgn (scratchrgn);
	
	return (pushcliprgn (scratchrgn, false));
#endif

#ifdef WIN95VERSION
	hdlregion rgn;
	boolean res;

	rgn = CreateRectRgn (0,0,1,1);
	CombineRgn (rgn, rgn, rgn, RGN_XOR);
	res = pushcliprgn (rgn, false);
	DeleteObject (rgn);
	return (res);
#endif
	} /*pushemptyclip*/


boolean popclip (void) {

	if (topclip <= 0) {
		
		shellinternalerror (idclipstackempty, "\x11" "too many popclips");
		
		return (false);
		}
	
#ifdef MACVERSION
	SetClip (clipstack [--topclip]);
#endif	
#ifdef WIN95VERSION
	if (winclipstack [--topclip])
		SelectClipRgn (getcurrentDC(), clipstack [topclip]);
	else
		SelectClipRgn (getcurrentDC(), NULL);
#endif	
	return (true);
	} /*popclip*/


boolean pushstyle (short fnum, short fsize, short fstyle) {
	
	register tystylerecord *pstyle;
	
	if (topstyle >= ctstyle) {
		
		shellinternalerror (idstylestackfull, "\x14" "style stack overflow");
		
		return (false);
		}
	
	pstyle = &stylestack [topstyle];
	
	getfontsizestyle (&(*pstyle).fnum, &(*pstyle).fsize, &(*pstyle).fstyle);
	
	topstyle++;
	
	//if ((pstyle->fnum != fnum) || (pstyle->fsize != fsize) || (pstyle->fstyle != fstyle))
		setglobalfontsizestyle (fnum, fsize, fstyle);
	
	checkdepth (topstyle, maxstyle);
	
	return (true);
	} /*pushstyle*/
		

boolean popstyle (void) {
	short fnum, fsize, fstyle;

	if (topstyle <= 0) {
		
		shellinternalerror (idstylestackempty, "\x12" "too many popstyles");
		
		return (false);
		}
	
	topstyle--;

	getfontsizestyle (&fnum, &fsize, &fstyle);
	
	//if ((stylestack[topstyle].fnum != fnum) || (stylestack [topstyle].fsize != fsize) || (stylestack [topstyle].fstyle != fstyle))
		setglobalfontsizestyle (stylestack [topstyle].fnum, stylestack [topstyle].fsize, 
								stylestack [topstyle].fstyle);
		
	return (true);
	} /*popstyle*/


#ifdef WIN95VERSION

static boolean equalcolors (const RGBColor *rgb1, const RGBColor *rgb2) {
	
	return (memcmp (rgb1, rgb2, sizeof (RGBColor)) == 0);
	} /*equalcolors*/


static COLORREF wincolorref (const RGBColor *rgb) {
	
	/*
	5.0b7 dmb: map the stock quickdraw.c color values to the system
	color values, if there's a match, or just a win rgb
	*/
	
	if (equalcolors (rgb, &whitecolor))
		return GetSysColor (COLOR_WINDOW);
	
	if (equalcolors (rgb, &blackcolor))
		return GetSysColor (COLOR_WINDOWTEXT);
	
	if (equalcolors (rgb, &graycolor))
		return GetSysColor (COLOR_BTNFACE);
	
	if (equalcolors (rgb, &darkgraycolor))
		return GetSysColor (COLOR_3DDKSHADOW);

	/*
	if (equalcolors (rgb, &mediumgraycolor))
		return GetSysColor(COLOR_WINDOW);
	*/
	
	if (equalcolors (rgb, &lightgraycolor))
		return GetSysColor (COLOR_BTNHIGHLIGHT);

	return (WINRGB (*rgb));
	} /*wincolorref*/

#endif


boolean pushforecolor (const RGBColor *rgb) {
		
	if (colorenabled ()) {
	
		if (topforecolor >= ctforecolors) {
			
			DebugStr ("\x1f" "pushforecolor: no room on stack");
			
			return (false);
			}
	
		#ifdef MACVERSION			
			GetForeColor (&forecolorstack [topforecolor++]);
					
			/*
			3/9/93 DW: copy the rgb record so the caller can pass us the 
			address of a field of a heap-allocated record.
			*/
				{
				RGBColor rgbcopy = *rgb;
				RGBForeColor (&rgbcopy);
				}
		#endif

		#ifdef WIN95VERSION
			{
			HDC hdc = getcurrentDC ();
			HPEN colorpen = CreatePen (PS_SOLID, 1, wincolorref (rgb));
			
			penstack[toppen++] = SelectObject (hdc, colorpen);
			
			forecolorstack [topforecolor++] = SetTextColor (hdc, wincolorref (rgb));
			}
		#endif
		}
		
	return (true);
	} /*pushforecolor*/
		

boolean popforecolor (void) {
	
	if (colorenabled ()) {
	
		if (topforecolor <= 0) {
			
			DebugStr ( "\x1e" "popforecolor: nothing on stack");
			
			return (false);
			}
		
		#ifdef MACVERSION		
			RGBForeColor (&forecolorstack [--topforecolor]);
		#endif
		
		#ifdef WIN95VERSION
			SetTextColor (getcurrentDC(), forecolorstack [--topforecolor]);
			
			poppen ();
		#endif
		}
	
	return (true);
	} /*popforecolor*/
	

boolean pushbackcolor (const RGBColor *rgb) {
	
	/*
	5.0b8 dmb: maintain our own backcolor value. fixed problems, with 
	the initial level of push/pop, at least.
	*/

	if (colorenabled ()) {
		
		if (topbackcolor >= ctbackcolors) {
			
			DebugStr ("\x1f" "pushbackcolor: no room on stack");
			
			return (false);
			}
		
		#ifdef MACVERSION
			GetBackColor (&backcolorstack [topbackcolor++]);
			
			/*
			3/9/93 DW: copy the rgb record so the caller can pass us the 
			address of a field of a heap-allocated record.
			*/
				{
				RGBColor rgbcopy = *rgb;
				
				RGBBackColor (&rgbcopy);
				}
		#endif

		#ifdef WIN95VERSION
			backcolor = wincolorref (rgb);
			
			backcolorstack [topbackcolor++] = SetBkColor (getcurrentDC (), backcolor);
		#endif
		}
		
	return (true);
	} /*pushbackcolor*/
		

boolean popbackcolor (void) {
	
	if (colorenabled ()) {
	
		if (topbackcolor <= 0) {
			
			DebugStr ("\x1e" "popbackcolor: nothing on stack");
			
			return (false);
			}
		#ifdef MACVERSION		
			RGBBackColor (&backcolorstack [--topbackcolor]);
		#endif

		#ifdef WIN95VERSION
			backcolor = backcolorstack [--topbackcolor];
			
			SetBkColor (getcurrentDC(), backcolor);
		#endif
		}
	
	return (true);
	} /*popbackcolor*/


boolean pushcolors (const RGBColor *forecolor, const RGBColor *backcolor) {
	
	pushforecolor (forecolor);
	
	return (pushbackcolor (backcolor));
	} /*pushcolors*/


boolean popcolors (void) {
//	char 	s[256];
	popforecolor ();
	return (popbackcolor ());
	} /*popcolors*/


boolean pushpen (void) {
	
	if (toppen >= ctpens)
		return (false);
	
	#ifdef MACVERSION	
		GetPenState (&penstack [toppen++]);
	#endif
		
	#ifdef WIN95VERSION
		penstack[toppen++] = SelectObject (getcurrentDC(), GetStockObject (NULL_PEN));
	#endif

	checkdepth (toppen, maxpen);
	
	return (true);
	} /*pushpen*/
	

boolean poppen (void) {
	
	/*
	5.0a24 dmb: delete the current pen after it's swapped out. 
	shouldn't hurt if it as the null pen that pushpen sets, but
	is necessary if pushforecolor pushed the pen
	*/

	if (toppen <= 0) {
		
		shellinternalerror (idpenstackempty, "\x10" "too many poppens");
		
		return (false);
		}

	#ifdef MACVERSION	
		SetPenState (&penstack [--toppen]);
	#endif

	#ifdef WIN95VERSION
		DeleteObject (SelectObject (getcurrentDC(), penstack [--toppen]));
	#endif

	return (true);
	} /*poppen*/
	
	
void getpenpoint (Point *pt) {
	
#ifdef MACVERSION	
	GetPen (pt);
#endif

#ifdef WIN95VERSION
	(*pt).h = gPenPositionX;

	(*pt).v = gPenPositionY;
#endif
	} /*getpenpoint*/
	
	
boolean equalpoints (Point pt1, Point pt2) {
#ifdef MACVERSION	
	return (EqualPt (pt1, pt2));
#endif

#ifdef WIN95VERSION
	return ((pt1.v == pt2.v) && (pt1.h == pt2.h));
#endif
	} /*equalpoints*/


short pointdist (Point pt1, Point pt2) {
	
	return (absint (pt1.h - pt2.h) + absint (pt1.v - pt2.v));
	} /*pointdist*/


void movepento (short h, short v) {
	
	/*
	move the pen to the indicated position.
	*/
	
#ifdef MACVERSION
	MoveTo (h, v);
#endif

#ifdef WIN95VERSION
	MoveToEx (getcurrentDC(), h, v, NULL);
	gPenPositionX = h;
	gPenPositionY = v;
#endif
	} /*movepento*/
	
	
void pendrawline (short h, short v) {
	
	/*
	draw a line from the current pen position to the indicated position.  side-
	effect, the pen moves to the endpoint of the line.
	*/
	
#ifdef MACVERSION
	LineTo (h, v);
#endif

#ifdef WIN95VERSION
	LineTo (getcurrentDC(), h, v);
	gPenPositionX = h;
	gPenPositionY = v;
#endif
	} /*pendrawline*/
	
//Code change by Timothy Paustian Saturday, April 29, 2000 9:32:15 PM
//Changed for UH 3.3.1 conclicts with drawstring in QuickDrawText.h
static void Drawstring (bigstring bs, boolean fldisabled) {

#ifdef MACVERSION
	if (fldisabled)
		TextMode (grayishTextOr);
	
	DrawString (bs);
	
	if (fldisabled)
		TextMode (srcOr);
#endif

#ifdef WIN95VERSION
	RECT r;
	HDC hdc = getcurrentDC();

	r.top = gPenPositionY - globalfontinfo.ascent;
	r.left = gPenPositionX;
	r.bottom = gPenPositionY + globalfontinfo.descent + globalfontinfo.leading;
	r.right = r.left + 2048;

	setWindowsFont();
	
	if (stringlength (bs) > 0) {
		
		int savebkmode = GetBkMode (hdc);
		
		SetBkMode (hdc, TRANSPARENT);

		DrawText (hdc, stringbaseaddress (bs), stringlength (bs), &r, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
		
		if (fldisabled)
			GrayString (hdc, GetStockObject(BLACK_BRUSH) /*NULL*/, NULL, (LPARAM) stringbaseaddress(bs), stringlength (bs), 
				r.left, r.top, r.right - r.left, r.bottom - r.top);
		else
			DrawText (hdc, stringbaseaddress (bs), stringlength (bs), &r, DT_NOCLIP | DT_NOPREFIX);
		
		SetBkMode (hdc, savebkmode);

		gPenPositionX = r.right;
		}
	
	clearWindowsFont();
#endif
	} /*drawstring*/


void pendrawstring (bigstring bs) {
	
	/*
	draw a string at the current pen position.  side-effect, the pen moves
	to point just after the last character drawn.
	*/
#ifdef MACVERSION	
	DrawString (bs);
#endif

#ifdef WIN95VERSION
	Drawstring (bs, false);
#endif
	} /*pendrawstring*/


void graydrawstring (bigstring bs) {
	
	Drawstring (bs, true);
	} /*graydrawstring*/


short stringpixels (bigstring bs) {
	
	/*
	return the number of pixels the string would occupy if it were drawn in
	the current font, size and style.

	5.0a17 dmb: use DrawText to more closely reflect the width of drawn text.
	*/

#ifdef MACVERSION
	return (StringWidth (bs));
#endif

#ifdef WIN95VERSION
	RECT r;
	
	r.top = 0;
	r.left = 0;
	r.bottom = globalfontinfo.ascent + globalfontinfo.descent + globalfontinfo.leading;
	r.right = 0;
	
	setWindowsFont();
	// GetTextExtentPoint32 (getcurrentDC(), stringbaseaddress(bs), stringlength(bs), &stringSize); 
	DrawText (getcurrentDC(), stringbaseaddress (bs), stringlength (bs), &r, DT_SINGLELINE | DT_CALCRECT | DT_NOPREFIX);
	clearWindowsFont();
	return ((short)r.right);
#endif
	} /*stringpixels*/


void centerrect (Rect *rcentered, Rect rcontains) {
	
	/*
	center the first rectangle within the second rectangle.
	*/
	
	register short height, width;
	Rect r;
	
	r = *rcentered;
	
	width = r.right - r.left;
	
	r.left = rcontains.left + ((rcontains.right - rcontains.left - width) / 2);
	
	r.right = r.left + width;
	
	height = r.bottom - r.top;
	
	r.top = rcontains.top + ((rcontains.bottom - rcontains.top - height) / 2);
	
	r.bottom = r.top + height;
	
	*rcentered = r;
	} /*centerrect*/


boolean havecolorquickdraw (void) {
#ifdef MACVERSION	
	//Code change by Timothy Paustian Friday, June 9, 2000 2:36:02 PM
	//Changed because using SysEnvisons and SysEnvRec is like Really old style
	//This was changed to Gestalt calls with two new globals see mac.c initmacintosh
	return gHasColorQD;
#endif

#ifdef WIN95VERSION
	return (true);  
#endif
	} /*havecolorquickdraw*/


void getcurrentscreenbounds (Rect *r) {
#ifdef MACVERSION	
	/*
	instead of using screenBits.bounds, call this routine to find out 
	the bounding global rectangle for the current monitor.
	
	we do this very heuristically -- we return the global rectangle that
	encloses the monitor that the mouse is in.
	*/
	
	register WindowPtr w = FrontWindow ();
	register GDHandle hdevice;
	Point mousepoint;
	boolean flmenubar = true;
	//Code change by Timothy Paustian Friday, June 9, 2000 10:05:41 PM
	//Changed to Opaque call for Carbon
	BitMap	screenBits;
	#if TARGET_API_MAC_CARBON == 1
	GetQDGlobalsScreenBits(&screenBits);
	#else
	screenBits = qd.screenBits;
	#endif
	
	*r = screenBits.bounds; /*default: use the old standby...*/
	
	if (!havecolorquickdraw () || (w == nil))
		goto exit;
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
	
	pushport (thePort);
	}
	
	GetMouse (&mousepoint);
	
	LocalToGlobal (&mousepoint);
	
	popport ();
	
	hdevice = GetDeviceList (); /*start with the first graphics device*/
	
	while (true) { /*step through the graphics device list*/
		
		if (hdevice == nil) /*no more devices to look at, didn't find mouse loc*/
			goto exit;
		
		*r = (**hdevice).gdRect;
		
		if (pointinrect (mousepoint, *r)) {
			
			flmenubar = hdevice == GetMainDevice ();
			
			goto exit;
			}
		
		hdevice = GetNextDevice (hdevice); /*advance to next device in list*/
		} /*while*/
	
	exit:
	
	if (flmenubar)
		(*r).top += getmenubarheight ();
#endif

#ifdef WIN95VERSION
/*
//Use GetSystemMetrics
	r->top = 0;
	r->left = 0;
	r->right = GetSystemMetrics (SM_CXFULLSCREEN);
	r->bottom = GetSystemMetrics (SM_CYFULLSCREEN);
*/
	getdesktopbounds (r);
#endif
	} /*getcurrentscreenbounds*/


void getwindowscreenbounds (const Rect *rwindow, Rect *r) {
#ifdef MACVERSION	
	/*
	instead of using screenBits.bounds, call this routine to find out 
	the bounding global rectangle for the monitor containing the 
	given window.
	
	if there is a window open, we determine which monitor it mostly
	resides on (windows can span two monitors) and return the global
	rectangle that encloses that window.
	
	if no window is given, or color quickdraw isn't around, we use 
	screenbits.bounds.
	
	12/15/91 dmb: don't look at inactive screens
	
	2.1b2 dmb: take desired rwindow directly instead of using a window's portrect
	*/
	
	register GDHandle hdevice;
	register GDHandle hmost = nil;
	register long ctpixelsmost = 0;
	register long ctpixels;
	Rect rsect;
	boolean flmenubar = true;
	//Code change by Timothy Paustian Friday, June 9, 2000 10:05:50 PM
	//Changed to Opaque call for Carbon
	BitMap	screenBits;
	#if TARGET_API_MAC_CARBON == 1
	GetQDGlobalsScreenBits(&screenBits);
	#else
	screenBits = qd.screenBits;
	#endif
	
	*r = screenBits.bounds; /*default return value, if no window open, or error*/
	
	/* commenting this out because we don't run on systems without color quickdraw anyway */
	/*
	if (!havecolorquickdraw ())
		goto exit;
	*/
	
	hdevice = GetDeviceList (); /*start with the first graphics device*/
	
	while (true) { /*step through the graphics device list*/
		
		if (hdevice == nil) { /*no more devices to look at*/
			
			if (hmost != nil) {
				
				/*
					2006-03-27 SMD - let the OS tell us about the available bounds
					this handles the menubar, the dock, and anything else the system knows about
				*/
				GetAvailableWindowPositioningBounds (hmost, r);
				}
			
			break;  /* exit the while loop */
			}
		
		if (((**hdevice).gdFlags | (1 << screenActive)) != 0) { /*screen is active*/
				
			if (SectRect (rwindow, &(**hdevice).gdRect, &rsect)) { /*they intersect*/
				
				ctpixels = abs ((long) (rsect.right - rsect.left) * (long) (rsect.bottom - rsect.top));
				
				if (ctpixels > ctpixelsmost) { /*more than last device that intersects*/
					
					ctpixelsmost = ctpixels;
					
					hmost = hdevice;
					}
				}
			}
		
		hdevice = GetNextDevice (hdevice); /*advance to next device in list*/
		} /*while*/
	
#endif

#ifdef WIN95VERSION
/*
//Use GetSystemMetrics
	r->top = 0;
	r->left = 0;
	r->right = GetSystemMetrics (SM_CXFULLSCREEN);
	r->bottom = GetSystemMetrics (SM_CYFULLSCREEN);
*/
	getdesktopbounds (r);
#endif
	} /*getwindowscreenbounds*/


void centerrectondesktop (Rect *r) {
	
	Rect rscreen;
	
	getcurrentscreenbounds (&rscreen);
	
	centerrect (r, rscreen);
	} /*centerrectondesktop*/


void centerbuttonstring (const Rect *r, bigstring bs, boolean fldisabled) {
	
	/*
	draw the string in the current font, size and style, centered inside
	the indicated rectangle.
	
	11/10/89 DW: workaround wierdnesses in geneva 9 by subtracting 1 from v.
	
	9/10/91 dmb: geneva 9 workaround can mess up other fonts; added check to 
	ensure top of characters aren't cut off

	6.26.97 dmb: added fldisabled, renamed as centerbuttonstring
	*/
	
	register short lh = globalfontinfo.ascent + globalfontinfo.descent; /*line height*/
	register short rh = (*r).bottom - (*r).top;
	register short rw = (*r).right - (*r).left;
	register short h, v;
	
	ellipsize (bs, rw); /*make sure it fits inside the rectangle, width-wise*/
	
	h = (*r).left + ((rw - stringpixels (bs)) / 2);
	
	v = (*r).top + ((rh - lh) / 2);
	
	if (v > (*r).top) /*9/10/91*/
		v--;
	
	#if TARGET_API_MAC_CARBON
	
		v--;
		
	#endif
	
	movepento (h, v + globalfontinfo.ascent);
	
	pushclip (*r);
	
	Drawstring (bs, fldisabled);
	
	popclip ();
	} /*centerbuttonstring*/


void grayrect (Rect r) {
#ifdef MACVERSION		
	pushpen ();
	
	PenMode (patBic);
	{
	#if TARGET_API_MAC_CARBON == 1
	
	Pattern	gray;
	GetQDGlobalsGray(&gray);
	PenPat(&gray);
	#else
	PenPat (&qd.gray);
	
	#endif
	}
	PaintRect (&r);
	
	poppen ();
#endif

#ifdef WIN95VERSION
	/*
	HDC hdc = getcurrentDC ();
	int oldROP2;
	HPEN oldpen, graypen;
	
	oldROP2 = SetROP2 (hdc, R2_BLACK);
	
	graypen = CreatePen (PS_DOT, 0, 0);
	
	oldpen = SelectObject (hdc, graypen);

	fillrect (r, getstockpattern (stockgray));
	
	SelectObject (hdc, oldpen);
	
	SetROP2 (hdc, oldROP2);
	
	DeleteObject (graypen);
	*/
#endif
	} /*grayrect*/
	
	
/*
void evenrectangle (Rect *rsource) {
	
	register Rect *r = rsource;
	
	if ((*r).top % 2) {
		
		(*r).top++;
		
		(*r).bottom++;
		}
		
	if ((*r).left % 2) {
		
		(*r).left++;
		
		(*r).right++;
		}
	} %*evenrectangle*/
	

/*
void oddrectangle (Rect *rsource) {
	
	register Rect *r = rsource;
	
	if (((*r).top % 2) == 0) {
		
		(*r).top++;
		
		(*r).bottom++;
		}
		
	if (((*r).left % 2) == 0) {
		
		(*r).left++;
		
		(*r).right++;
		}
	} %*oddrectangle*/
	

void zerorect (Rect *rzero) {
	
	register Rect *r = rzero;
	
	(*r).top = (*r).left = (*r).bottom = (*r).right = 0;
	} /*zerorect*/
	

boolean equalrects (Rect r1, Rect r2) {
	
	return (
		(r1.top == r2.top) && (r1.left == r2.left) && 
		
		(r1.bottom == r2.bottom) && (r1.right == r2.right));
	} /*equalrects*/
	

boolean issubrect (Rect r1, Rect r2) {
	
	/*
	return true if r1 is completely contained within r2.
	*/
	
	Rect runion;
	
	unionrect (r1, r2, &runion);
	
	return (equalrects (r2, runion));
	} /*issubrect*/

#endif


void globaltolocalrgn (hdlregion rgn) {
#ifdef MACVERSION	
	Point pt;
	
	SetPt (&pt, 0, 0);
	
	GlobalToLocal (&pt);
	
	OffsetRgn (rgn, pt.h, pt.v);
#endif
	} /*globaltolocalrgn*/
	
	
void localtoglobalrect (WindowPtr w, Rect *r) {
	
	/*
	convert the rectangle inside w to global coordinates on the desktop.
	*/
#ifdef MACVERSION	
	register Rect *x = r;
	Point p1, p2;
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
		
	pushport (thePort);
	
	p1 = topLeft (*x);
	
	p2 = botRight (*x);
	
	LocalToGlobal (&p1);
	
	LocalToGlobal (&p2);
	
	Pt2Rect (p1, p2, x);
	
	popport ();
#endif

#ifdef WIN95VERSION
	/*
	for points, global means screen coordinates.
	for rects, global means frame-relative coordinates
	*/

	POINT winpt;
	
	winpt.x = (*r).left;
	winpt.y = (*r).top;
	
//	ClientToScreen (w, &winpt);
	MapWindowPoints (w, shellframewindow, &winpt, 1);
	
	offsetrect (r, winpt.x - (*r).left, winpt.y - (*r).top);
#endif
	} /*localtoglobalrect*/
	
	
void globaltolocalpoint (WindowPtr w, Point *pt) {
#ifdef MACVERSION	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
		
	pushport (thePort);
	
	GlobalToLocal (pt);
	
	popport ();
#endif
#ifdef WIN95VERSION
	/*
	for points, global means screen coordinates.
	for rects, global means frame-relative coordinates
	*/

	POINT winpt;
	
	winpt.x = (*pt).h;
	winpt.y = (*pt).v;
	
	ScreenToClient (w, &winpt);
//	MapWindowPoints (shellframewindow, w, &winpt, 1);
	
	(*pt).h = (short)winpt.x;
	(*pt).v = (short)winpt.y;
#endif
	} /*globaltolocalpoint*/
	
	
void localtoglobalpoint (WindowPtr w, Point *pt) {
#ifdef MACVERSION	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
		
	pushport (thePort);
	
	LocalToGlobal (pt);
	
	popport ();
#endif

#ifdef WIN95VERSION
	/*
	for points, global means screen coordinates. we're probably
	talking about the mouse position

	for rects, global means frame-relative coordinates. we're probably
	talking about a window position
	*/

	POINT winpt;
	
	winpt.x = (*pt).h;
	winpt.y = (*pt).v;
	
	ClientToScreen (w, &winpt);
//	MapWindowPoints (w, shellframewindow, &winpt, 1);
	
	(*pt).h = (short)winpt.x;
	(*pt).v = (short)winpt.y;
#endif
	} /*localtoglobalpoint*/
	

#if !flruntime
	
void flashrect (Rect r, short ctflashes) {
	
	/*
	invert and uninvert the rectangle several times, with delays, to simulate
	a menu selection type animation.
	*/
	
	register short ct = ctflashes;
	register short i;
	
	pushclip (r);
	
	for (i = 1; i <= ct; i++) {
		
		if (i > 1)
			delayticks (1);
		
		invertrect (r);
		
		delayticks (2);
		
		invertrect (r);
		} /*for*/
		
	popclip ();
	} /*flashrect*/
	
	
void dropshadowrect (Rect r, short width, boolean flerase) {
	
	/*
	draw a drop-shadow to the right and below the indicated rectangle.  if flerase,
	then we fill the drop shadow area with gray.  it's designed to manage buttons
	displayed on a field of gray.
	*/
	
	Rect rfill;
	
	rfill = r;
	
	rfill.top++;
	
	rfill.left = rfill.right;
	
	rfill.right = rfill.left + width;
	
	rfill.bottom += width;
	
	if (flerase) {
		//Code change by Timothy Paustian Friday, June 9, 2000 10:10:25 PM
		//Changed to Opaque call for Carbon
		#if TARGET_API_MAC_CARBON == 1
			Pattern gray;
			
			GetQDGlobalsGray(&gray);
			
			fillrect (rfill, gray);
		#else
			fillrect (rfill, getstockpattern (stockgray));
		#endif
		}
	else {
		#if TARGET_API_MAC_CARBON == 1
			Pattern black;

			GetQDGlobalsBlack(&black);
			
			fillrect (rfill, black);
		#else
			fillrect (rfill, getstockpattern (stockblack));
		#endif
		}

							
	rfill = r;
	
	rfill.top = rfill.bottom;
	
	rfill.bottom = rfill.top + width;
	
	rfill.left++;
	
	rfill.right += width;
	
	if (flerase) {
		//Code change by Timothy Paustian Friday, June 9, 2000 10:10:25 PM
		//Changed to Opaque call for Carbon
		#if TARGET_API_MAC_CARBON == 1
			Pattern gray;

			GetQDGlobalsGray(&gray);
			
			fillrect (rfill, gray);
		#else
			fillrect (rfill, getstockpattern (stockgray));
		#endif
		}
	else {
		#if TARGET_API_MAC_CARBON == 1
			Pattern black;

			GetQDGlobalsBlack(&black);
			
			fillrect (rfill, black);
		#else
			fillrect (rfill, getstockpattern (stockblack));
		#endif
		}
	} /*dropshadowrect*/


void smashrect (Rect r) {
#ifdef MACVERSION		
	EraseRect (&r);
	//Code change by Timothy Paustian Friday, June 9, 2000 10:13:24 PM
	//Changed to Opaque call for Carbon
	#if TARGET_API_MAC_CARBON == 1
	//Will GetFrontWindowOfClass do what we want. I hope so.
	{
	WindowRef w;	
	CGrafPtr thePort = GetQDGlobalsThePort();
	
	w = GetWindowFromPort (thePort);
		
	InvalWindowRect(w, &r);
	}
	#else
	InvalRect (&r);
	#endif
#endif
#ifdef WIN95VERSION
	RECT winrect;
	recttowinrect (&r, &winrect);
	InvalidateRect (getcurrentwindow(), &winrect, true);

#endif
	} /*smashrect*/
	

void invalrect (Rect r) {
#ifdef MACVERSION	
	//Code change by Timothy Paustian Friday, June 9, 2000 10:13:24 PM
	//Changed to Opaque call for Carbon
	#if TARGET_API_MAC_CARBON == 1
	//Will GetFrontWindowOfClass do what we want. I hope so.
	//InvalWindowRect(GetFrontWindowOfClass(kAllWindowClasses, false), &r);
	{
	WindowRef w;	
	CGrafPtr thePort = GetQDGlobalsThePort();
	
	w = GetWindowFromPort (thePort);
		
	InvalWindowRect(w, &r);
	}
	#else
	InvalRect (&r);
	#endif
#endif
#ifdef WIN95VERSION
	RECT winrect;
	recttowinrect (&r, &winrect);
	InvalidateRect (getcurrentwindow(), &winrect, false);

#endif
	} /*invalrect*/
	
	
void invalwindowrect (WindowPtr w, Rect r) {

	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
		
	pushport (thePort);
	
	invalrect (r);
	
	popport ();
	} /*invalwindowrect*/


void validrect (Rect r) {
#ifdef MACVERSION	
	#if TARGET_API_MAC_CARBON == 1
	//ValidWindowRect(GetFrontWindowOfClass(kAllWindowClasses, false), &r);
	ValidWindowRect (shellwindow, &r);
	#else
	ValidRect (&r);
	#endif
#endif
#ifdef WIN95VERSION
	RECT winrect;
	recttowinrect (&r, &winrect);
	ValidateRect (getcurrentwindow(), &winrect);
#endif
	} /*validrect*/


void eraserect (Rect r) {

#ifdef MACVERSION	
	EraseRect (&r);
#endif
#ifdef WIN95VERSION
	RECT winrect;
	recttowinrect (&r, &winrect);

	if (!isemptyrect (r)) {
		
		if (topbackcolor > 0) {

			HBRUSH brush = CreateSolidBrush (GetBkColor (getcurrentDC()));
			FillRect (getcurrentDC(), &winrect, brush);
			DeleteObject (brush);
			}
		else
			FillRect (getcurrentDC(), &winrect, (HBRUSH) (COLOR_BTNFACE + 1));	// );GetStockObject(WHITE_BRUSH)
		
	//	FillRect (getcurrentDC(), &winrect, (HBRUSH) backcolor);
		}
#endif
	} /*eraserect*/
	

void framerect (Rect r) {
	
	#ifdef gray3Dlook
		boolean fllargeobject;

		if (isemptyrect (r))
			return;

		fllargeobject = r.bottom - r.top > 18;

		--r.right;
		--r.bottom;
		movepento (r.left, r.bottom);
		
		if (fllargeobject)
			pushforecolor (&darkgraycolor);
		else
			pushforecolor (&lightgraycolor);

		pendrawline (r.left, r.top);
		pendrawline (r.right, r.top);
		popforecolor ();
		
		if (fllargeobject)
			pushforecolor (&lightgraycolor);
		else
			pushforecolor (&darkgraycolor);

		pendrawline (r.right, r.bottom);
		pendrawline (r.left, r.bottom);
		popforecolor ();
	#else
		if (isemptyrect (r))
			return;

		#ifdef MACVERSION	
			FrameRect (&r);
		#endif

		#ifdef WIN95VERSION
		{
			RECT winrect;
			recttowinrect (&r, &winrect);
			FrameRect (getcurrentDC(), &winrect, getcurrentbrush());
		}
		#endif
	#endif
	} /*framerect*/


void drawthemeborder (Rect r, Rect rcontent) {
	
	/*
	7.0b52 PBS: draw the "scan line" theme border on OS X.
	*/
	
	#if TARGET_API_MAC_CARBON == 1
	
		//register hdlminirecord hm = minidata;
		//register hdlwindowinfo hw = miniwindowinfo;
		//WindowPtr w = (**hw).macwindow;
		Rect /*r, rcontent, */ rbackground;

		/*r = (**hm).textrects [0];

		rcontent = (**hw).contentrect;*/
		
		pushpen ();
		
		setthemepen (kThemeBrushDialogBackgroundActive, rcontent, true);
		
		rbackground.top = 0;
		rbackground.left = 0;
		rbackground.bottom = rcontent.bottom;
		rbackground.right = r.left;
		
		paintrect (rbackground);
		
		rbackground.right = rcontent.right;
		rbackground.bottom = r.top;
		
		paintrect (rbackground);
		
		rbackground.left = r.right;
		rbackground.right = rcontent.right;
		rbackground.bottom = rcontent.bottom;
		
		paintrect (rbackground);
		
		rbackground.left = 0;
		rbackground.top = r.bottom;
		
		paintrect (rbackground);
		
		poppen ();

	#endif
	
	}


void setgraypen (void) {
#ifdef MACVERSION	
	//Code change by Timothy Paustian Friday, June 9, 2000 10:22:55 PM
	//Changed to Opaque call for Carbon
	#if TARGET_API_MAC_CARBON == 1
	Pattern gray;
	PenPat(GetQDGlobalsGray(&gray));
	#else
	PenPat (&qd.gray);
	#endif
#endif

#ifdef WIN95VERSION
	HPEN graypen;

	currentwindowbrush = GetStockObject (GRAY_BRUSH);
	
	graypen = CreatePen (PS_SOLID, 0, RGB (0x7f, 0x7f, 0x7f));

	SelectObject (getcurrentDC(), graypen);

	assert (toppen > 0); // assume caller has pushed a pen
#endif
	} /*setgraypen*/


void setthemepen (const short brush, Rect r, boolean flupdate) {

	#if TARGET_API_MAC_CARBON == 1
	
		SetThemePen (brush, maxdepth (&r), flupdate);
				
	#endif
	} /*setthemepen*/


void graydrawline (short h, short v) {

#ifdef MACVERSION		
	pushpen ();
	
	setgraypen (); 
	
	PenMode (patCopy);// (patXor);
	
	LineTo (h, v);
	
	poppen ();
#endif

#ifdef WIN95VERSION
	HDC hdc = getcurrentDC ();
	int oldROP2;
	HPEN oldpen, graypen;
	
	oldROP2 = SetROP2 (hdc, R2_XORPEN);
	
	graypen = CreatePen (PS_DOT, 0, 0);
	
	oldpen = SelectObject (hdc, graypen);

	LineTo (hdc, h, v);
	
	SelectObject (hdc, oldpen);
	
	SetROP2 (hdc, oldROP2);
	
	DeleteObject (graypen);

	gPenPositionX = h;
	gPenPositionY = v;
#endif
	} /*graydrawline*/


void grayframerect (Rect r) {
	#ifdef MACVERSION		
		pushpen ();
		
		setgraypen ();
		
		FrameRect (&r);
		
		poppen ();
	#endif

	#ifdef WIN95VERSION
		RECT winrect;
		HGDIOBJ brush;

		recttowinrect (&r, &winrect);

		brush = GetStockObject (GRAY_BRUSH);

		FrameRect (getcurrentDC(), &winrect, brush);

		DeleteObject (brush); //Not needed, but not harmful - keep code complete
	#endif
	} /*grayframerect*/


void grayframerrgn (hdlregion rgn) {
	
	/*
	5.0b8 dmb: use xor rop2 so wp selrgn can be inverted
	*/
	
	#ifdef MACVERSION	
		pushpen ();
		
		setgraypen ();
		
		PenMode (patCopy);// (patXor);
		
		FrameRgn (rgn);
		
		poppen ();
	#endif

	#ifdef WIN95VERSION
		HDC hdc = getcurrentDC();
		HGDIOBJ brush;
		int oldROP2;

		brush = GetStockObject (GRAY_BRUSH);
		
		oldROP2 = SetROP2 (hdc, R2_XORPEN);

		FrameRgn (hdc, rgn, brush, 1, 1);

		SetROP2 (hdc, oldROP2);

		DeleteObject (brush); //Not needed, but not harmful - keep code complete
	#endif
	} /*grayframerrgn*/


void fillrect (Rect r, xppattern pat) {
	#ifdef MACVERSION
		qdfillrect (&r, &pat);
	#endif

	#ifdef WIN95VERSION
		RECT winrect;
		recttowinrect (&r, &winrect);
		FillRect (getcurrentDC(), &winrect, pat);
	#endif
	} /*fillrect*/


#ifdef MACVERSION
void paintrect (Rect r) {
	
	PaintRect (&r);
	} /*paintrect*/
#endif	


void frame3sides (Rect r) {
	
	/*
	draw a border, leaving out the right side of the rect.  makes it prettier
	if you're displaying a scrollbar to the right of the rect, for example.

	5.0a25 dmb: for Win vesion we now use this routine to frame the content
	areas
	*/
	
	#ifdef MACVERSION
	--r.bottom;
	
	movepento (r.right, r.top); 
	
	pendrawline (r.left, r.top); /*draw top of box*/
	
	pendrawline (r.left, r.bottom); /*draw left side of box*/
	
	pendrawline (r.right, r.bottom); /*draw bottom of box*/
	
	#if TARGET_API_MAC_CARBON == 1
	
		{
		Rect rwindow;
		Rect rbackground;
		short depth;
		
		shellgetwindowrect (shellwindowinfo, &rwindow);
		
		depth = maxdepth (&rwindow);
				
		pushpen ();
		
		setthemepen (kThemeBrushDialogBackgroundActive, rwindow, true);
		
		rbackground.top = 0;
		rbackground.left = 0;
		rbackground.bottom = r.top - 1;
		rbackground.right = rwindow.right;
		
		paintrect (rbackground);
		
		rbackground.right = r.left - 1;
		rbackground.bottom = rwindow.bottom;
		
		paintrect (rbackground);
		
		rbackground.left = r.right;
		rbackground.right = rwindow.right;
		
		paintrect (rbackground);
		
		rbackground.left = 0;
		rbackground.top = r.bottom + 1;
		rbackground.bottom = rwindow.bottom;
		
		paintrect (rbackground);
		
		poppen ();
		}
	
	
	#endif
	
	#endif

	#ifdef WIN95VERSION
	--r.right;

	--r.bottom;

	movepento (r.right, r.top); 
	
	pendrawline (r.left, r.top); /*draw top of box*/
	
	pendrawline (r.left, r.bottom); /*draw left side of box*/
	
	pendrawline (r.right, r.bottom); /*draw bottom of box*/
	
	pendrawline (r.right, r.top); /*draw right of box*/
	#endif
	} /*frame3sides*/
	

void eraseandframerect (Rect r) {

	eraserect (r);

	framerect (r);
	} /*eraseandframerect*/


void invertrect (Rect r) {
#ifdef MACVERSION	
	InvertRect (&r);
#endif

#ifdef WIN95VERSION
	RECT winrect;

	recttowinrect (&r, &winrect);

	InvertRect (getcurrentDC(), &winrect);
#endif
	} /*invertrect*/	
	
#endif


void setrect (Rect *rset, short top, short left, short bottom, short right) {
	
	register Rect *r = rset;
	
	(*r).top = top;
	
	(*r).left = left;
	
	(*r).bottom = bottom;
	
	(*r).right = right;
	} /*setrect*/
	

void insetrect (Rect *r, short dh, short dv) {
#ifdef MACVERSION	
	InsetRect (r, dh, dv);
#endif
#ifdef WIN95VERSION
	RECT winrect;

	recttowinrect (r, &winrect);

	InflateRect (&winrect, dh*-1, dv*-1);

	winrecttorect (&winrect, r);
#endif
	} /*insetrect*/
	

void offsetrect (Rect *r, short dh, short dv) {
#ifdef MACVERSION	
	OffsetRect (r, dh, dv);
#endif
#ifdef WIN95VERSION
	RECT winrect;

	recttowinrect (r, &winrect);

	OffsetRect (&winrect, dh, dv);

	winrecttorect (&winrect, r);
#endif
	} /*offsetrect*/
	

boolean pointinrect (Point pt, Rect r) {

	#ifdef MACVERSION	
		return (PtInRect (pt, &r));
	#endif
	#ifdef WIN95VERSION
		RECT winrect;
		POINT winpoint;

		recttowinrect (&r, &winrect);
		winpoint.x = pt.h;
		winpoint.y = pt.v;
		return (PtInRect (&winrect, winpoint));
	#endif
	} /*pointinrect*/

#if !flruntime

void scrollrect (Rect r, short dh, short dv) {
	
	/*
	a front end for the Macintosh routine that scrolls a rectangle of pixels.

	5.0b9 dmb: use ScrollDC's region to handle window's true vis rgn
	*/
	
	register hdlregion rgn;
	
#ifdef MACVERSION
	rgn = NewRgn ();
	
	/*7.1b19 PBS: Call QDerror: you're supposed to check. Plus Eric Soroos is getting crashes in ScrollRect I can't explain.*/

	if (QDError () != noErr) 
		return;
		
	ScrollRect (&r, dh, dv, rgn);
	
	#if TARGET_API_MAC_CARBON == 1
	//InvalWindowRgn(GetFrontWindowOfClass(kAllWindowClasses, false), rgn);
	{
	WindowRef w;	
	CGrafPtr thePort = GetQDGlobalsThePort();
	
	w = GetWindowFromPort (thePort);
		
	InvalWindowRgn (w, rgn);
	}
	
	#else
	InvalRgn (rgn);
	#endif
	DisposeRgn (rgn);
#endif
	
#ifdef WIN95VERSION
	RECT winrect, updaterect, cliprect, smashedrect;
	
	recttowinrect (&r, &winrect);
	smashedrect = winrect;
	
	if (dv > 0) {
		winrect.bottom -= dv;
		
		smashedrect.bottom = r.top + dv;
		}
	else {
		winrect.top -= dv;
		
		if (dv)
			smashedrect.top = r.bottom + dv;
		}

	if (dh > 0) {
		winrect.right -= dh;

		smashedrect.right = r.left + dh;
		}
	else {
		winrect.left -= dh;

		if (dh)
			smashedrect.left = r.right + dh;
		}
	
	rgn = CreateRectRgn (r.left, r.top, r.right, r.bottom); // default update region
	
	if (winrect.bottom > winrect.top && winrect.right > winrect.left) {
		
		GetClipBox (currentportDC, &cliprect);
		
		ScrollDC (currentportDC, dh, dv, &winrect, &cliprect, rgn, &updaterect);
		}
	
	InvalidateRgn (currentport, rgn, false); // region calc'ed by ScrollDC
	
	DeleteObject (rgn);

	InvalidateRect (currentport, &smashedrect, false); // region we know to need painting
#endif

	} /*scrollrect*/


void scrollrectvert (Rect r, short dv) {
	
	scrollrect (r, 0, dv);
	} /*scrollrectvert*/
	

void scrollrecthoriz (Rect r, short dh) {
	
	scrollrect (r, dh, 0);
	} /*scrollrecthoriz*/


void unionrect (Rect r1, Rect r2, Rect *runion) {

	/*
	calculates the smallest rectangle which encloses both input rectangles. 
	It works correctly even if one of the source rectangles is also the 
	destination.
	*/

	#ifdef MACVERSION	
		UnionRect (&r1, &r2, runion);
	#endif
	
	#ifdef WIN95VERSION
		RECT winrect1, winrect2, windestrect;

		recttowinrect (&r1, &winrect1);
		recttowinrect (&r2, &winrect2);

		UnionRect (&windestrect, &winrect1, &winrect2);

		winrecttorect (&windestrect, runion);
	#endif
	} /*unionrect*/
	
	
boolean intersectrect (Rect r1, Rect r2, Rect *rintersection) {

	/*
	calculates the rectangle that is the intersection of the two input 
	rectangles, and returns true if they indeed intersect or false if they do not. 
	rectangles that touch at a line or a point are not considered intersecting, 
	because their intersection rectangle (really, in this case, an intersection 
	line or point) does not enclose any bits on the bitMap.	
	*/
#ifdef MACVERSION	
	return (SectRect (&r1, &r2, rintersection));
#endif
#ifdef WIN95VERSION
	RECT winrect1, winrect2, windestrect;
	boolean res;

	recttowinrect (&r1, &winrect1);
	recttowinrect (&r2, &winrect2);

	res = IntersectRect (&windestrect, &winrect1, &winrect2);

	winrecttorect (&windestrect, rintersection);

	return (res);
#endif
	} /*intersectrect*/


void getdesktopbounds (Rect *r) {
	
	/*
	the QuickDraw global GrayRgn encloses the entire desktop, the
	concatenation of all monitors attached to the system.
	
	call this routine when you want to limit a rectangle not to the
	current monitor, but to the entire desktop.
	*/
#ifdef MACVERSION
	//Code change by Timothy Paustian Monday, May 1, 2000 9:57:24 PM
	//Changed to Opaque call for Carbon
	//This is probably not the best way to do this, but it works.
	//I could probably remove the #if #else stuff this is an old call.
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	RgnHandle	grayRgn;
	grayRgn = GetGrayRgn();
	GetRegionBounds(grayRgn, r);
	#else
	//old code
	*r = (**LMGetGrayRgn ()).rgnBBox;
	#endif
#endif

#ifdef WIN95VERSION
/*
//Use GetSystemMetrics
	r->top = 0;
	r->left = 0;
	r->right = GetSystemMetrics (SM_CXFULLSCREEN);
	r->bottom = GetSystemMetrics (SM_CYFULLSCREEN);
*/
	getlocalwindowrect (shellframewindow, r);

	localtoglobalrect (shellframewindow, r);
#endif
	} /*getdesktopbounds*/


static boolean constraintorect (Rect *rconstrained, Rect rcontains, boolean flcenter) {
	
	/*
	9/15/91 dmb: make sure that rconstrained fits within rcontains.
	
	if flcenter is true, center the window along any axis that needs 
	constraint. otherwise, just move the rect enough to get it inside.
	*/
	
	Rect r;
	Rect rcentered;
	short dh = 0;
	short dv = 0;
	
	r = *rconstrained; /*copy into local*/
	
	/*first, limit size to size of screen*/
	
	r.right = min (r.right, r.left + (rcontains.right - rcontains.left));
	
	r.bottom = min (r.bottom, r.top + (rcontains.bottom - rcontains.top));
	
	/*now, if repositioning is necessary, center in that dimension*/
	
	rcentered = r;
	
	centerrect (&rcentered, rcontains);
	
	if (r.left < rcontains.left) {
		
		if (flcenter)
			dh = rcentered.left - r.left;
		else
			dh = rcontains.left - r.left;
		}
	else if (r.right > rcontains.right) {
		
		if (flcenter)
			dh = rcentered.right - r.right;
		else
			dh = rcontains.right - r.right;
		}
	
	if (r.top < rcontains.top) {
		
		if (flcenter)
			dv = rcentered.top - r.top;
		else
			dv = rcontains.top - r.top;
		}
	else if (r.bottom > rcontains.bottom) {
		
		if (flcenter)
			dv = rcentered.bottom - r.bottom;
		else
			dv = rcontains.bottom - r.bottom;
		}
	
	offsetrect (&r, dh, dv);
	
	/*
	r.left = max (r.left, rcontains.left);
	
	r.right = min (r.right, rcontains.right);
	
	r.top = max (r.top, rcontains.top);
	
	r.bottom = min (r.bottom, rcontains.bottom);
	
	r.bottom = max (r.bottom, r.top);
	*/
	
	if (equalrects (*rconstrained, r))
		return (false);
	
	*rconstrained = r; /*copy back to parameter*/
	
	return (true);
	} /*constraintorect*/


static void accountfortitlebar (WindowPtr w, Rect *rconstrain) {

	/*
	if w is a window with a titlebar, tighten the contraining rect to 
	account for the titlebar's height
	*/
	#ifdef MACVERSION	
		//Code change by Timothy Paustian Monday, May 1, 2000 10:10:10 PM
		//Changed to Opaque call for Carbon
		short variant = 0;
		//Code change by Timothy Paustian Wednesday, May 3, 2000 10:08:46 PM
		//This is sometimes called with nil, we have to check, for this.
		if(w != nil)
			variant = GetWVariant (w);
			
		if ((variant == documentProc) || (variant > altDBoxProc))
			(*rconstrain).top += doctitlebarheight;
		/*
		old code
		register WindowPeek wpeek = (WindowPeek) w;
		
		if (wpeek != nil) {
			
			short variant = GetWVariant (w);
			
			if ((variant == documentProc) || (variant > altDBoxProc))
				(*rconstrain).top += doctitlebarheight;
			
			%*
			titleheight = (**(*wpeek).contRgn).rgnBBox.top - (**(*wpeek).strucRgn).rgnBBox.top;
			
			(*rconstrain).top += titleheight;
			*/
			//}*/
	#endif

	#ifdef WIN95VERSION
		#define titlebarheight 20

		(*rconstrain).top += titlebarheight;
	#endif
	} /*accountfortitlebar*/


boolean constraintodesktop (WindowPtr w, Rect *rparam) {
	
	/*
	make sure the rect fits within the desktop of the machine we're running on.
	
	12/28/90 dmb: don't add titlebarheight to top.  make sure bottom doesn't 
	end up above top
	
	9/27/91 dmb: title bar height is now accounted for by taking windowptr 
	parameter and calling accountfortitlebar.
	*/
	
	Rect rdesktop;
	
	getdesktopbounds (&rdesktop);
	
	accountfortitlebar (w, &rdesktop);
	
	return (constraintorect (rparam, rdesktop, false));
	} /*constraintodesktop*/


boolean constraintoscreenbounds (WindowPtr w, boolean flcurrentscreen, Rect *rparam) {
	
	/*
	9/15/91 dmb: make sure the rect fits within the screen that is occupies
	
	2.1b6 dmb: added flcurrentscreen parameter. if true, constrain to bounds 
	of the screen the window is actually on, like we originally did. if false, 
	contstrain to the screen containing rparam, as we've been doing 
	more recently.
	*/
	
	Rect r;
	Rect rscreen;
	
	if (flcurrentscreen) { /*use its portrect*/
		
		getglobalwindowrect (w, &r);
		
		getwindowscreenbounds (&r, &rscreen);
		}
	else
		getwindowscreenbounds (rparam, &rscreen);
	
	accountfortitlebar (w, &rscreen);
	
	/***insetrect (&rscreen, 3, 3); %*an aesthetic border of desktop*/
	
	return (constraintorect (rparam, rscreen, false)); /*4.1b7 dmb: flcenter parm was true*/
	} /*constraintoscreenbounds*/

#ifdef MACVERSION
boolean pushdesktopport (CGrafPtr port) {
	
	/*
	call this when you want to draw directly to the whole desktop, regardless
	of how many monitors there are.
	
	it must be balanced with a call to popdesktopport.
	
	it returns false if it was unable to set up the port, in which case the
	caller should **not** call popdesktopport, and should assume that he cannot
	draw on the desktop.
	
	12/28/90 dmb: original version copied the GrayRgn to a local, and then
	assigned to the visRgn, without disposing the original (stranding a heap 
	block).  since OpenPort isn't preflighted, there's no point in creating an 
	intermediate copy of the GrayRgn anyway, so we copy directly to the visRgn
	*/
	
	//Code change by Timothy Paustian Monday, May 1, 2000 10:12:02 PM
	//Changed to Opaque call for Carbon
	//Using this whole routine may be a problem. Check to see if it is even used.
	register CGrafPtr p = port;
	hdlregion grayrgn;
	Rect		grayRect;
		
	pushport (nil); /*save current port, don't SetPort*/
	
	#if TARGET_API_MAC_CARBON == 1
	 p = CreateNewPort();
	#else
	OpenPort (p); /*also makes it the current port*/
	#endif
	grayrgn = GetGrayRgn ();
	
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	SetPortVisibleRegion(p, grayrgn);
	GetRegionBounds(grayrgn, &grayRect);
	SetPortBounds(p, &grayRect);
	#else
	#pragma unused(grayRect)
	//old code
	CopyRgn (grayrgn, (*p).visRgn); /*make visRgn cover entire desktop*/
	
	(*p).portRect = (**LMGetGrayRgn ()).rgnBBox;
	#endif
	
	return (true);
	} /*pushdesktopport*/


void popdesktopport (CGrafPtr port) {
	
	//Code change by Timothy Paustian Thursday, July 20, 2000 4:27:05 PM
	//pop this port off the stack, then dispose of it.
	popport ();
	//Code change by Timothy Paustian Friday, June 16, 2000 2:51:11 PM
	//Changed to Opaque call for Carbon
	#if TARGET_API_MAC_CARBON == 1
	DisposePort(port);
	#else	
	ClosePort (port);
	#endif
		
	
	} /*popdesktopport*/
#endif


void getmainscreenrect (Rect *r) {
	
	/*
	returns the rectangle on the user's main screen rect, minus the height
	of the menubar.
	*/
	
	getcurrentscreenbounds (r);
	
	/*
	(*r).top += getmenubarheight ();
	*/
	} /*getmainscreenrect*/
	

void getsystemoriginrect (Rect *r) {

	/*
	when you want to zoom something from the system menu, get the source
	rectangle from us.
	*/
	
	register short h, v;
	Rect rbounds;
	#ifdef WIN95VERSION
		RECT winrect;
	#endif
	
	getcurrentscreenbounds (&rbounds);
	
	h = rbounds.left;
	
	v = rbounds.top;
	
	/*
	v = getmenubarheight ();
	*/
	
	#ifdef MACVERSION
		SetRect (r, h, v, h, v); /*approximate location of Apple menu*/
	#endif
	#ifdef WIN95VERSION
		SetRect (&winrect, h, v, h, v);
		winrecttorect (&winrect, r);
	#endif
	} /*getsystemoriginrect*/


static short maxdepth (Rect *r) {
	
	/*
	5.0b8 dmb: fixed Windows vesion
	*/

	#ifdef MACVERSION	
		GDHandle hdldevice;
		
		if (!systemhascolor ())
			return (1);
		
		hdldevice = GetMaxDevice (r);
		
		if (hdldevice == nil)
			return (1);
		
		return ((**(**hdldevice).gdPMap).pixelSize);
	#endif

	#ifdef WIN95VERSION
		HDC hdc;
		short depth;
		
		hdc = GetDC(GetTopWindow(NULL));
		
		if ((GetDeviceCaps (hdc, RASTERCAPS) & RC_PALETTE) != 0)
			depth = GetDeviceCaps (hdc, COLORRES);
		else
			depth = 4;

		ReleaseDC (GetTopWindow(NULL), hdc);
		
		return (depth);
	#endif
	} /*maxdepth*/


boolean colorenabled (void) {
	
	/*
	2.1b6 dmb: take rect in which to test maxdepth
	*/
	
	if (!systemhascolor ())
		return (false);
	
	#ifdef MACVERSION
		//Code change by Timothy Paustian Friday, June 9, 2000 10:28:39 PM
		//Changed to Opaque call for Carbon
		#if TARGET_API_MAC_CARBON == 1
		{
		BitMap	screenBits;
		GetQDGlobalsScreenBits(&screenBits);
		return (maxdepth(&(screenBits).bounds) > 1);
		}
		#else
		return (maxdepth (&quickdrawglobal (screenBits).bounds) > 1);
		#endif
	#endif
	#ifdef WIN95VERSION
		return (maxdepth (NULL) > 1);
	#endif
	} /*colorenabled*/


short iscolorport (CGrafPtr pport) {

	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, May 1, 2000 10:17:59 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		return IsPortColor(pport);
		#else
		//old code
		return ((*(CGrafPtr) pport).portVersion < 0);
		#endif
	#endif
	#ifdef WIN95VERSION
		return (true);
	#endif
	} /*iscolorport*/


void fillcolorrect (Rect r, short idppat) {

	#ifdef MACVERSION	
		PixPatHandle hppat = GetPixPat (idppat);
		
		FillCRect (&r, hppat);
		
		DisposePixPat (hppat);
	#endif

	#ifdef WIN95VERSION
		RECT winrect;
		HBRUSH hBrush;
		HBITMAP hBitmap;

		hBitmap = LoadBitmap (hInst, MAKEINTRESOURCE(idppat));
		hBrush = CreatePatternBrush (hBitmap);
		recttowinrect (&r, &winrect);

		FillRect (getcurrentDC(), &winrect, hBrush);

		DeleteObject (hBrush);
		DeleteObject (hBitmap);
	#endif
	} /*fillcolorrect*/


/*
static boolean getclut (short resid, CTabHandle *hdlctab) {
	
	Handle hdata;
	
	*hdlctab = nil;
	
	hdata = GetResource ('clut', resid);
	
	if (hdata == nil)
		return (false);
	
	*hdlctab = (CTabHandle) hdata;
	
	return (true);
	} %*getclut*/

/*
static boolean getcolorfromindex (short index, RGBColor *rgb) {

	CTabHandle hctab;
	
	hctabl = GetCTable (128);
	
	if (hctab == nil)
		return (false);
		
	%*
	if (!getclut (128, &hctab))
		return (false);
	%/
	
	*rgb = (**hctab).ctTable [index].rgb;
	
	return (true);
	} %*getcolorfromindex*/

#endif /*flruntime*/


boolean systemhascolor (void) {

	#ifdef MACVERSION
		long templong;
		if (!gestalt (gestaltQuickdrawVersion, &templong))
			return (false);
		
		return (templong >= gestalt8BitQD);
	#endif
	#ifdef WIN95VERSION
		return (true);  //8 bit color required for windows...
	#endif
	} /*systemhascolor*/


boolean rectinregion (Rect r, hdlregion rgn) {

	#ifdef MACVERSION
		return (RectInRgn (&r, rgn));
	#endif

	#ifdef WIN95VERSION
		RECT winrect;

		recttowinrect (&r, &winrect);

		return (RectInRegion (rgn, &winrect));
	#endif
	} /*rectinregion*/


void initquickdraw (void) {
	
	/*
	by pre-allocating regions for the clip stack, and a scratch region, 
	we reduce heap management overhead for push/popclip operations
	
	2.1b4 dmb: added opening of a scratchport. since we always push & pop 
	ports, this ensures that when nothing is pushed, a valid port is still 
	established. avoids crashes. even better is to call "pushscratchport" 
	in places where a port is needed, and no window is appropriate.

	2002-11-11 AR: Added asserts to make sure the C compiler chose the
	proper byte alignment for the diskrect and diskrgb structs.
	If it did not, we would end up corrupting any database files we saved.
	*/
	
	register short ix;

	assert (sizeof(diskrect) == 8);
	
	assert (sizeof(diskrgb) == 6);
	
	#ifdef MACVERSION
		for (ix = 0; ix < ctclip; ix++)
			clipstack [ix] = NewRgn ();
		
		scratchrgn = NewRgn ();
		//Code change by Timothy Paustian Friday, May 5, 2000 9:55:41 PM
		//I need to allocate the memory for the scratch port since it is
		//now a pointer. Make sure this is cool with Andre
		//This is creating a memory leak but it only gets called once.
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		scratchport = CreateNewPort();
		#else
		scratchport = NewPtr(sizeof(CGrafPort));
		OpenPort (scratchport);
		#endif
	#endif

	#ifdef WIN95VERSION
		for (ix = 0; ix < ctclip; ix++)
			clipstack [ix] = CreateRectRgn (0,0,1,1);
		
		scratchrgn = CreateRectRgn (0,0,1,1);
		currentwindowbrush = GetStockObject (BLACK_BRUSH);
		gPenPositionX = 0;
		gPenPositionY = 0;
	#endif
	} /*initquickdraw*/


void diskrecttorect (diskrect *rdisk, Rect *r) {

	diskrect rcopy = *rdisk;

	disktomemshort (rcopy.left);
	disktomemshort(rcopy.top);
	disktomemshort(rcopy.right);
	disktomemshort(rcopy.bottom);

	(*r).left = rcopy.left;
	
	(*r).top = rcopy.top;
	
	(*r).right = rcopy.right;
	
	(*r).bottom = rcopy.bottom;
	} /*diskrecttorect*/


void recttodiskrect (Rect *r, diskrect *rdisk) {

	(*rdisk).left = (short) (*r).left;
	
	(*rdisk).top = (short) (*r).top;
	
	(*rdisk).right = (short) (*r).right;
	
	(*rdisk).bottom = (short) (*r).bottom;

	memtodiskshort (rdisk->left);
	memtodiskshort(rdisk->top);
	memtodiskshort(rdisk->right);
	memtodiskshort(rdisk->bottom);
	} /*recttodiskrect*/


void diskrgbtorgb (diskrgb *rgbdisk, RGBColor *r) {

	(*r).red = (*rgbdisk).red;
	
	(*r).green = (*rgbdisk).green;
	
	(*r).blue = (*rgbdisk).blue;

	#ifdef PACKFLIPPED
		shortswap(r->red);
		shortswap(r->green);
		shortswap(r->blue);
	#endif
	} /*diskrgbtorgb*/


void rgbtodiskrgb (RGBColor *r, diskrgb *rgbdisk) {

	(*rgbdisk).red = (short) (*r).red;
	
	(*rgbdisk).green = (short) (*r).green;
	
	(*rgbdisk).blue = (short) (*r).blue;

	#ifdef PACKFLIPPED
		shortswap(rgbdisk->red);
		shortswap(rgbdisk->green);
		shortswap(rgbdisk->blue);
	#endif
	} /*rgbtodiskrgb*/


