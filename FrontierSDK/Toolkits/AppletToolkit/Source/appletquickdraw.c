
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <GestaltEqu.h>
#include <Palettes.h>
#include <QDOffscreen.h>
#include "appletdefs.h"
#include "appletfont.h"
#include "appletmemory.h"
#include "appletquickdraw.h"

/*
7/25/94 dmb: updated old-style function declarations
*/

RGBColor whitecolor = {65535, 65535, 65535};

RGBColor blackcolor = {0, 0, 0};

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

typedef char smalliconbits [32];

typedef smalliconbits *ptrsmalliconbits, **hdlsmalliconbits;


#define useoffscreenworlds


#ifdef useoffscreenworlds

	static boolean flhas32bitqd = false; /*32-bit quickdraw implemented?*/
	
	typedef struct tyoffscreen {
		
		boolean flbitmapopen;
		
		CGrafPtr psavedport;
		
		GDHandle hsavedgd;
		
		GWorldPtr poffscreenworld;
		
		Rect offscreenrect;
		
		Handle bitmapbasehandle;
		
		BitMap offscreenbitmap, savedbitmap;
		} tyoffscreen;
	
	static tyoffscreen bitoffscreen = {false, nil, nil, nil, {0, 0, 0, 0}, nil};

#else

	typedef struct tyoffscreen {
	
		GDHandle origdevice;
		
		GrafPtr offptr;
		
		Rect offrect;
		
		boolean flsuccess;
		} tyoffscreen;
	
	static tyoffscreen bitoffscreen;
	
	typedef BitMap *typtrbitmap;

#endif


#define ctpens 5 /*we can remember pens up to 5 levels deep*/

short toppens = 0;

PenState penstack [ctpens];


#define ctports 25 /*we can remember ports up to 25 levels deep*/

short topport = 0;

GrafPtr portstack [ctports];


#define ctclip 5 /*we can remember clips up to 5 levels deep*/

short topclip = 0;

Rect clipstack [ctclip];


#define ctstyle 5 /*we can remember font/size/styles up to 5 levels deep*/

short topstyle = 0;

struct {short fnum, fsize, fstyle;} stylestack [ctstyle];


#define ctforecolors 5

short topforecolor = 0;

RGBColor forecolorstack [ctforecolors];


#define ctbackcolors 5

short topbackcolor = 0;

RGBColor backcolorstack [ctbackcolors];





boolean pushpen () {
	
	if (toppens >= ctpens)
		return (false);
		
	GetPenState (&penstack [toppens++]);
	
	return (true);
	} /*pushpen*/
	

boolean poppen () {
	
	if (toppens <= 0)
		return (false);
		
	SetPenState (&penstack [--toppens]);
	
	return (true);
	} /*poppen*/
	

boolean pushmacport (GrafPtr p) {
	
	if (topport >= ctports) {
		
		DebugStr ((ConstStr255Param)"\ppushmacport: no room on stack");
		
		return (false);
		}

	portstack [topport++] = quickdrawglobal (thePort);
	
	if (p != nil)
		SetPort (p);
		
	return (true);
	} /*pushmacport*/
		

boolean popmacport () {
	
	if (topport <= 0) 
		return (false);
	
	SetPort (portstack [--topport]);
	
	return (true);
	} /*popmacport*/


boolean pushclip (Rect r) {
	
	RgnHandle rgn;
	
	if (topclip >= ctclip)
		return (false);
	
	GetClip (rgn = NewRgn ());
	
	clipstack [topclip++] = (**rgn).rgnBBox;
	
	DisposeRgn (rgn);
	
	ClipRect (&r);
	
	return (true);
	} /*pushclip*/
		

boolean popclip () {
	
	if (topclip <= 0)
		return (false);
	
	ClipRect (&clipstack [--topclip]);
	
	return (true);
	} /*popclip*/


boolean pushstyle (short fnum, short fsize, short fstyle) {
	
	if (topstyle >= ctstyle)
		return (false);
		
	getfontsizestyle (
		&stylestack [topstyle].fnum, 
		
		&stylestack [topstyle].fsize, 
		
		&stylestack [topstyle].fstyle);
	
	topstyle++;
	
	setglobalfontsizestyle (fnum, fsize, fstyle);
	
	return (true);
	} /*pushstyle*/
		

boolean popstyle (void) {
	
	if (topstyle <= 0)
		return (false);
	
	topstyle--;
	
	setfontsizestyle (
		stylestack [topstyle].fnum, 
		
		stylestack [topstyle].fsize, 
		
		stylestack [topstyle].fstyle);
		
	return (true);
	} /*popstyle*/


short getmenubarheight (void) {
	
	return (GetMBarHeight ()); /*call Script Manager routine*/
	} /*getmenubarheight*/


void setrect (Rect *rset, short top, short left, short bottom, short right) {
	
	Rect *r = rset;
	
	(*r).top = top;
	
	(*r).left = left;
	
	(*r).bottom = bottom;
	
	(*r).right = right;
	} /*setrect*/
	
	
void localtoglobalrect (Rect *rchanged) {

	Point p1, p2;
	Rect r;
	
	r = *rchanged;
	
	p1.h = r.left;
	
	p1.v = r.top;
	
	LocalToGlobal (&p1);
	
	p2.h = r.right;
	
	p2.v = r.bottom;
	
	LocalToGlobal (&p2);
	
	Pt2Rect (p1, p2, &r);
	
	*rchanged = r;
	} /*localtoglobalrect*/
	
	
void eraserect (Rect r) {
	
	EraseRect (&r);
	} /*eraserect*/
	
	
void movepento (short h, short v) {
	
	MoveTo (h, v);
	} /*movepento*/
	
	
void pendrawline (short h, short v) {
	
	LineTo (h, v);
	} /*pendrawline*/
	
	
void invalrect (Rect r) {
	
	if (r.left >= r.right)
		return;
		
	if (r.top >= r.bottom)
		return;
		
	InvalRect (&r);
	} /*invalrect*/
	

void smashrect (Rect r) {
	
	eraserect (r);
	
	invalrect (r);
	} /*smashrect*/
	

void insetrect (Rect *r, short dh, short dv) {
	
	InsetRect (r, dh, dv);
	} /*insetrect*/
	
	
boolean equalpoints (Point p1, Point p2) {
	
	return (EqualPt (p1, p2));
	} /*equalpoints*/
	
	
boolean emptyrect (Rect r) {
	
	/*
	return false if there are any pixels contained inside the rectangle.
	*/
	
	return ((r.top >= r.bottom) || (r.left >= r.right));
	} /*emptyrect*/
	
	
boolean rectlessthan (Rect r1, Rect r2) {
	
	/*
	return true if r1 is "less than" r2.
	
	we look only at the top-left corners of the rectangles. a rectangle is
	less than the other rectangle if it's above the other guy vertically.
	
	if they're both at the same vertical position, then the guy to the left
	of the other is less than.
	
	this helps us implement a rational sort of tabbing thru objects, we go
	from top to bottom and left to right.
	*/
	
	if (r1.top != r2.top) { /*comparison is based on tops*/
		
		return (r1.top < r2.top);
		}
	
	return (r1.left < r2.left);
	} /*rectlessthan*/
	
	
static short absint (short x) {
	
	if (x < 0)
		x = -x;
		
	return (x);
	} /*absint*/
	
	
short pointdist (Point pt1, Point pt2) {
	
	return (absint (pt1.h - pt2.h) + absint (pt1.v - pt2.v));
	} /*pointdist*/


void centerrect (Rect *rcentered, Rect rcontains) {
	
	/*
	center the first rectangle within the second rectangle.
	*/
	
	short height, width;
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


void dropshadowrect (Rect rorig, short width, boolean flerase) {
	
	/*
	draw a drop-shadow to the right and below the indicated rectangle.  if flerase,
	then we fill the drop shadow area with white.
	*/
	
	Rect r = rorig;
	short h, v;
	short i;
	
	if (flerase) {
		
		pushpen ();
		
		PenMode (patBic);
		}
		
	for (i = 0; i < width; i++) {
		
		h = r.right + i;
		
		MoveTo (h, r.top + i + 1);
		
		LineTo (h, r.bottom + width - 1);
		
		v = r.bottom + i;
		
		MoveTo (r.left + i + 1, v);
		
		LineTo (r.right + width - 1, v);
		} /*for*/
		
	if (flerase)
		poppen ();
	} /*dropshadowrect*/ 
	
	
void invaldropshadow (Rect r, short width) {
	
	/*
	inval the rectangle and it's drop shadow.
	
	5/6/92 DW: work around IOWA problem when the drop shadow shrinks, there's
	no convenient way to inval the old drop shadow. if you ever decrease the
	drop shadow by more than 5 pixels you'll have to do a complete update.
	*/
	
	r.right += width + 5;
	
	r.bottom += width + 5;
	
	InvalRect (&r);
	} /*invaldropshadow*/
	

#define gestalttrap 		0xA1AD
#define unimplementedtrap	0xA09F


static boolean gestaltavailable (void) {

	UniversalProcPtr gestaltaddr;
	UniversalProcPtr unimplementedaddr;

	gestaltaddr = NGetTrapAddress (gestalttrap, ToolTrap);
	
	unimplementedaddr = NGetTrapAddress (unimplementedtrap, ToolTrap);
	
	return (unimplementedaddr != gestaltaddr);	
	} /*gestaltavailable*/
	
	
static boolean getgestaltattr (OSType selector, long *response) {
	
	OSErr errcode;
	
	if (!gestaltavailable ())
		return (false);
		
	errcode = Gestalt (selector, response);
	
	return (errcode == noErr);
	} /*getgestaltattr*/
	
	
boolean systemhascolor (void) {
	
	static boolean flgotit = false;
	static boolean flhavecolor = false;
	
	if (!flgotit) {
	
		long templong;
	
		if (!getgestaltattr (gestaltQuickdrawVersion, &templong))
			flhavecolor = false;
		else
			flhavecolor = templong >= gestalt8BitQD;
			
		flgotit = true;
		}
	
	return (flhavecolor);
	} /*systemhascolor*/


static short maxdepth (void) {

	GDHandle hdldevice;

	if (!systemhascolor ())
		return (1);
		
	hdldevice = GetMaxDevice (&quickdrawglobal (screenBits).bounds);

	if (hdldevice == nil)
		return (1);
		
	return ((**(**hdldevice).gdPMap).pixelSize);
	} /*maxdepth*/
	
	
boolean colorenabled (void) {

	if (!systemhascolor ())
		return (false);
		
	return (maxdepth () > 1);
	} /*colorenabled*/
	
	
static short iscolorport (GrafPtr pport) {

	return ((*(CGrafPtr) pport).portVersion < 0);
	} /*iscolorport*/


static boolean getclut (short resid, CTabHandle *hdlctab) {
	
	Handle hdata;
	
	*hdlctab = nil;
	
	hdata = GetResource ('clut', resid);
	
	if (ResError () != noErr)
		return (false);
	
	*hdlctab = (CTabHandle) hdata;
	
	return (true);
	} /*getclut*/


boolean oldclutconverter (short index, RGBColor *rgb) {

	CTabHandle hdlctab;

	getclut (128, &hdlctab);
	
	*rgb = (**hdlctab).ctTable [index].rgb;

	return (true);
	} /*oldclutconverter*/
	
	
boolean pushforecolor (RGBColor *rgb) {
		
	if (colorenabled ()) {
	
		if (topforecolor >= ctforecolors) {
			
			DebugStr ((ConstStr255Param) "\ppushforecolor: no room on stack");
			
			return (false);
			}
	
		GetForeColor (&forecolorstack [topforecolor++]);
				
		/*
		3/9/93 DW: copy the rgb record so the caller can pass us the 
		address of a field of a heap-allocated record.
		*/
			{
			RGBColor rgbcopy = *rgb;
			
			RGBForeColor (&rgbcopy);
			}
		}
		
	return (true);
	} /*pushforecolor*/
		

boolean popforecolor (void) {
	
	if (colorenabled ()) {
	
		if (topforecolor <= 0) {
			
			DebugStr ((ConstStr255Param) "\ppopforecolor: nothing on stack");
			
			return (false);
			}
		
		RGBForeColor (&forecolorstack [--topforecolor]);
		}
	
	return (true);
	} /*popforecolor*/
	

boolean pushbackcolor (RGBColor *rgb) {
		
	if (colorenabled ()) {
		
		if (topbackcolor >= ctbackcolors) {
			
			DebugStr ((ConstStr255Param) "\ppushbackcolor: no room on stack");
			
			return (false);
			}
		
		GetBackColor (&backcolorstack [topbackcolor++]);
		
		/*
		3/9/93 DW: copy the rgb record so the caller can pass us the 
		address of a field of a heap-allocated record.
		*/
			{
			RGBColor rgbcopy = *rgb;
			
			RGBBackColor (&rgbcopy);
			}
		}
		
	return (true);
	} /*pushbackcolor*/
		

boolean popbackcolor (void) {
	
	if (colorenabled ()) {
	
		if (topbackcolor <= 0) {
			
			DebugStr ((ConstStr255Param) "\ppopbackcolor: nothing on stack");
			
			return (false);
			}
		
		RGBBackColor (&backcolorstack [--topbackcolor]);
		}
	
	return (true);
	} /*popbackcolor*/
	
	
boolean pushcolors (RGBColor *forecolor, RGBColor *backcolor) {
	
	pushforecolor (forecolor);
	
	return (pushbackcolor (backcolor));
	} /*pushcolors*/
	
	
boolean popcolors (void) {
	
	popforecolor ();
	
	return (popbackcolor ());
	} /*popcolors*/
	
	
boolean equalcolors (RGBColor *rgb1, RGBColor *rgb2) {
	
	return (	
		((*rgb1).red == (*rgb2).red) && 
		
		((*rgb1).green == (*rgb2).green) && 
		
		((*rgb1).blue == (*rgb2).blue));
	} /*equalcolors*/
	
	
boolean pushgridcolor (RGBColor *rgbbase) {
	
	/*
	this color is used in drawing the grid in Iowa. we choose something
	that we think looks nice when superimposed on the base color. for dark
	colors it's brighter, for bright colors its darker. avoid jarring
	contrasts.
	*/
	
	short diff = 10000;
	short maxval = whitecolor.red - diff;
	RGBColor rgb = *rgbbase;
	
	if (equalcolors (&rgb, &whitecolor))
		return (pushforecolor (&lightbluecolor));
		
	if (rgb.red > maxval)
		rgb.red -= diff;
	else
		rgb.red += diff;
	
	if (rgb.green > maxval)
		rgb.green -= diff;
	else
		rgb.green += diff;
	
	if (rgb.blue > maxval)
		rgb.blue -= diff;
	else
		rgb.blue += diff;
	
	return (pushforecolor (&rgb));
	} /*pushgridcolor*/
	
	
boolean pushhighlightcolor (RGBColor *rgbbase) {
	
	/*
	the user is dragging to select a rectangle on the base color.
	
	we are looking for high contrast between the two colors.
	
	unfortunately, it doesn't appear to effect the color when
	we call this routine. must be something I don't understand about
	Color QuickDraw. DW 2/9/93.
	*/
	
	unsigned long midpoint = (3 * whitecolor.red) / 2;
	RGBColor rgb = *rgbbase;
	long sum;
	
	sum = rgb.red + rgb.blue + rgb.green;
	
	if (sum > midpoint)
		rgb = blackcolor;
	else
		rgb = whitecolor;
		
	return (pushforecolor (&rgb));
	} /*pushhighlightcolor*/
	
	
boolean getcolorpalette (WindowPtr w) {

	CTabHandle hclut;
	PaletteHandle hpalette;

	getclut (128, &hclut); 
			
	hpalette = GetPalette (w);
	
	if (hpalette == nil)
		hpalette = NewPalette (32, hclut, pmTolerant, 0x0000);
	else
		CTab2Palette (hclut, hpalette, pmTolerant, 0x0000);

	SetPalette (w, hpalette, true);
	
	ActivatePalette (w);
	
	return (true);
	} /*getcolorpalette*/


#ifdef useoffscreenworlds

static boolean openworld (Rect r, WindowPtr w) {
	
	/*
	9/22/93 dmb: use temp mem for offscreens
	*/
	
	short fontnum, fontsize, fontstyle;
	PenState pen;
	
	GetGWorld (&bitoffscreen.psavedport, &bitoffscreen.hsavedgd);
	
	bitoffscreen.offscreenrect = r;
	
	localtoglobalrect (&r);
	
	if (bitoffscreen.poffscreenworld) {
		
		if (UpdateGWorld (&bitoffscreen.poffscreenworld, 0, &r, nil, nil, useTempMem) & gwFlagErr)
			return (false);
		}
	else {
		OSErr ec;
		
		ec = NewGWorld (&bitoffscreen.poffscreenworld, 0, &r, nil, nil, useTempMem); /*doug says turn last param to 0*/
		
		if (ec != noErr)
			return (false);
		}
	
	getfontsizestyle (&fontnum, &fontsize, &fontstyle);
	
	GetPenState (&pen);
	
	SetGWorld (bitoffscreen.poffscreenworld, nil);
	
	LockPixels (bitoffscreen.poffscreenworld->portPixMap);
		
	setfontsizestyle (fontnum, fontsize, fontstyle);
	
	SetPenState (&pen);
	
	SetOrigin (bitoffscreen.offscreenrect.left, bitoffscreen.offscreenrect.top);
	
	ClipRect (&bitoffscreen.poffscreenworld->portRect); /*recommended by Apple DTS*/
	
	CopyBits (
		&((GrafPtr) bitoffscreen.psavedport)->portBits,
		&((GrafPtr) bitoffscreen.poffscreenworld)->portBits,
		&bitoffscreen.poffscreenworld->portRect,
		&bitoffscreen.offscreenrect, srcCopy, nil);
		
	return (true);
	} /*openworld*/


static void closeworld (WindowPtr w) {
	
	/*
	2/8/91 dmb: experiments show that we get better performance tossing 
	the gworld every time than we do using updategworld on the same one.
	*/
	
	SetGWorld (bitoffscreen.psavedport, bitoffscreen.hsavedgd);
	
	pushforecolor (&blackcolor);	/* 2.2.96 dmb: need b&w to preserve colors correctly*/
	pushbackcolor (&whitecolor);
	
	CopyBits (
		&((GrafPtr) bitoffscreen.poffscreenworld)->portBits, 
		&((GrafPtr) bitoffscreen.psavedport)->portBits,
		&bitoffscreen.poffscreenworld->portRect, &bitoffscreen.offscreenrect, srcCopy, nil);
	
	popforecolor ();
	popbackcolor ();
	
	/*
	#ifdef coderesource
		
		DisposeGWorld (bitoffscreen.poffscreenworld);
		
		bitoffscreen.poffscreenworld = nil;
	
	#else
	
		UnlockPixels (bitoffscreen.poffscreenworld->portPixMap);
		
	#endif
	*/
	
	DisposeGWorld (bitoffscreen.poffscreenworld);
		
	bitoffscreen.poffscreenworld = nil;
	} /*closeworld*/


static boolean openmono (Rect r, WindowPtr w) {
	
	short nrowbytes;
	long nboxbytes;
	
   	nrowbytes = (r.right - r.left + 7) / 8;
	
   	if (odd (nrowbytes)) 
      	nrowbytes++;
	
	nboxbytes = (r.bottom - r.top) * nrowbytes;
	
	bitoffscreen.bitmapbasehandle = appnewhandle (nboxbytes);
	
	if (bitoffscreen.bitmapbasehandle == nil)
      	return (false);
	
	HLock (bitoffscreen.bitmapbasehandle);
	
	bitoffscreen.offscreenbitmap.baseAddr = *bitoffscreen.bitmapbasehandle;
	
	bitoffscreen.offscreenbitmap.rowBytes = nrowbytes;
	
	bitoffscreen.offscreenbitmap.bounds = r;
	
	bitoffscreen.savedbitmap = (*w).portBits;
	
	SetPortBits (&bitoffscreen.offscreenbitmap);
	
	return (true);
	} /*openmono*/


static void closemono (WindowPtr w) {
	
	SetPortBits (&bitoffscreen.savedbitmap);
	
	CopyBits (
   		&bitoffscreen.offscreenbitmap, &w->portBits, &bitoffscreen.offscreenbitmap.bounds, 
   	
   		&bitoffscreen.offscreenbitmap.bounds, srcCopy, nil);
   
	HUnlock (bitoffscreen.bitmapbasehandle);
	
	DisposeHandle (bitoffscreen.bitmapbasehandle);
	} /*closemono*/


static boolean systemhas32bitqd (void) {
	
	long qdversion;
	
	if (Gestalt (gestaltQuickdrawVersion, &qdversion) == noErr)
		flhas32bitqd = qdversion >= gestalt32BitQD;
	else
		flhas32bitqd = false;
		
	return (flhas32bitqd);
	} /*systemhas32bitqd*/


boolean openbitmap (Rect r, WindowPtr w) {
	
	boolean fl;
	
	if (bitoffscreen.flbitmapopen) /*can't nest offscreens*/
		return (false);
	
	if (systemhas32bitqd ())
		fl = openworld (r, w);
	else
		fl = openmono (r, w);
	
	bitoffscreen.flbitmapopen = fl;
	
	return (fl);
	} /*openbitmap*/


void closebitmap (WindowPtr w) {
	
	if (bitoffscreen.flbitmapopen) {
		
		bitoffscreen.flbitmapopen = false;
		
		if (flhas32bitqd)
			closeworld (w);
		else
			closemono (w);
		}
	} /*closebitmap*/

#else

static void calcrowbytes (boolean flcolor, CGrafPtr offptr, Rect offrect, long *rowbytes) {

	short pixelsize;
	short numpix;
	long offbits;

	if (flcolor)
		pixelsize = (**(*offptr).portPixMap).pixelSize;
	else
		pixelsize = 1;
	
	numpix = offrect.right - offrect.left;
	
	offbits = pixelsize * numpix;

	*rowbytes = ((offbits + 15) / 16) * 2;
	} /*calcrowbytes*/
	
	
static void optimizerect (Rect *r) {

	(*r).left = ((*r).left / 32) * 32;
	
	(*r).right = (((*r).right + 31) / 32) * 32;
	} /*optimizerect*/
	

static boolean openpixmap (WindowPtr w, Rect offrect,  Boolean flcopy, tyoffscreen *offscreen) {

	Rect rectcopy;
	GDHandle maxdevice;
	long offrowbytes;
	long offsize;
	PixMapHandle temppixmap;
	Ptr tempptr;
	CTabHandle temppmtable;
	short err;
	short i;
	Handle temphandle;
	boolean flcolor;
	typtrbitmap destptr;

	(*offscreen).flsuccess = false;

	if (!SectRect (&(*w).portRect, &offrect, &rectcopy))
		return (false);

	SetPort (w);
				
	if (flcopy)
		optimizerect (&offrect);

	(*offscreen).offrect = offrect;
	
	flcolor = iscolorport (w);
	
	if (flcolor) {
	
		localtoglobalrect (&rectcopy);
	
		maxdevice = GetMaxDevice (&rectcopy);
		
		if (maxdevice == nil)
			return (false);
	
		(*offscreen).origdevice = GetGDevice ();
		
		SetGDevice (maxdevice);
		
		(*offscreen).offptr = (GrafPtr) NewPtr (sizeof (CGrafPort));
		}
	else {
		(*offscreen).offptr = (GrafPtr) NewPtr (sizeof (GrafPort));
		}
	
	if (MemError () != 0) {
	
		/*DebugStr ((ConstStr255Param) "\pMemErr in openpixmap");*/
		
		return (false);
		}
	
	if (flcolor)
		OpenCPort ((CGrafPtr) (*offscreen).offptr);
	else
		OpenPort ((*offscreen).offptr);
	
	calcrowbytes (flcolor, (CGrafPtr) (*offscreen).offptr, offrect, &offrowbytes);
	
	offsize = (long) (offrect.bottom - offrect.top) * offrowbytes;
	
	tempptr = NewPtr (offsize);
	
	if (MemError () != 0) {

		/*DebugStr ("\pMemErr in openpixmap");*/
		
		SetPort (w);
		
		if (flcolor)
			CloseCPort ((CGrafPtr) (*offscreen).offptr);
		else
			ClosePort ((*offscreen).offptr);

		DisposPtr ((Ptr) (*offscreen).offptr);
		
		return (false);
		}
	
	if (flcolor) {
	
		temphandle = (Handle) (**(**maxdevice).gdPMap).pmTable;
		
		err = HandToHand (&temphandle);
		
		if (err != noErr) {
			
			/*DebugStr ("\pMemErr in openpixmap");*/
		
			SetPort (w);
			
			CloseCPort ((CGrafPtr) (*offscreen).offptr);
		
			DisposPtr ((Ptr) (*offscreen).offptr);
			
			DisposPtr (tempptr);
			
			return (false);
			}
			
		temppmtable = (CTabHandle) temphandle;
	
		temppixmap = (*(CGrafPtr) (*offscreen).offptr).portPixMap;
	
		(**temppixmap).baseAddr = tempptr;
		
		(**temppixmap).rowBytes = offrowbytes + 0x8000;
		
		(**temppixmap).bounds = offrect;
		
		for (i = 0; i <= (**temppmtable).ctSize; i++)
			(**temppmtable).ctTable[i].value = i;
			
		(**temppmtable).ctFlags = (**temppmtable).ctFlags & 0x7FFF;
	
		(**temppixmap).pmTable = temppmtable;
		}
	else {
		(*(*offscreen).offptr).portBits.baseAddr = tempptr;
		
		(*(*offscreen).offptr).portBits.rowBytes = offrowbytes;

		(*(*offscreen).offptr).portBits.bounds = offrect;
		}
	
	SetPort ((GrafPtr) (*offscreen).offptr);
	
	if (flcopy) {
	
		if (flcolor)
			destptr = (typtrbitmap) (*(*(CGrafPtr) (*offscreen).offptr).portPixMap);
		else
			destptr = &(*(*offscreen).offptr).portBits;
			
		CopyBits (&(*w).portBits, destptr, &offrect, &offrect, 0, nil);
		}
		
	(*offscreen).flsuccess = true;
	
	return (true);
	} /*openpixmap*/


static void closepixmap (WindowPtr w, tyoffscreen offscreen) {

	PixMapHandle temppixmap;
	boolean flcolor;
	typtrbitmap sourceptr;
	
	if (!offscreen.flsuccess)
		return;

	SetPort (w);
	
	flcolor = iscolorport (w);
	
	if (flcolor) {
	
		SetGDevice (offscreen.origdevice);
		
		sourceptr = (typtrbitmap) (*(*(CGrafPtr) offscreen.offptr).portPixMap);
		}
	else
		sourceptr = &(*offscreen.offptr).portBits;

	pushforecolor (&blackcolor);	/* 2.2.96 dmb: need b&w to preserve colors correctly*/
	pushbackcolor (&whitecolor);
	
	CopyBits (sourceptr, &(*w).portBits, &offscreen.offrect, &offscreen.offrect, 0, nil);
	
	popforecolor ();
	popbackcolor ();
	
	if (flcolor) {
	
		temppixmap = (*(CGrafPtr) offscreen.offptr).portPixMap;
	
		DisposHandle ((Handle) (**temppixmap).pmTable);
		
		DisposPtr ((Ptr) (**temppixmap).baseAddr);
	
		CloseCPort ((CGrafPtr) offscreen.offptr);
		}
	else {
		DisposPtr ((Ptr) (*offscreen.offptr).portBits.baseAddr);
		
		ClosePort (offscreen.offptr);
		}
	
	DisposPtr ((Ptr) offscreen.offptr);
	} /*closepixmap*/
	
	
boolean openbitmap (Rect offrect, WindowPtr w) {
	
	return (openpixmap (w, offrect, true, &bitoffscreen));
	} /*openbitmap*/
	
	
void closebitmap (WindowPtr w) {
	
	closepixmap (w, bitoffscreen);
	} /*closebitmap*/

#endif

	
boolean plotsmallicon (Rect r, short iconlist, short iconnum, boolean flinvert) {

	hdlsmalliconbits hbits;
	short mode;
	BitMap bmap;
	WindowPtr w;
	
	GetPort (&w);
	
	hbits = (hdlsmalliconbits) GetResource ('SICN', iconlist);
	
	if (hbits == nil) /*failed to load the resource*/  
		return (false);
		
	r.right = r.left + widthsmallicon; /*we only pay attention to the top, left fields of rectangle*/
	
	r.bottom = r.top + heightsmallicon;
		
	bmap.baseAddr = (Ptr) &(*hbits) [iconnum];
	
	bmap.rowBytes = 2;
	
	bmap.bounds.top = bmap.bounds.left = 0; 
	
	bmap.bounds.bottom = r.bottom - r.top; 
	
	bmap.bounds.right = r.right - r.left;
	
	if (flinvert)
		mode = notSrcCopy;
	else 
		mode = srcOr; /*was srcCopy*/
		
	CopyBits (&bmap, &(*w).portBits, &bmap.bounds, &r, mode, nil);
	
	return (true);
	} /*plotsmallicon*/
	
	
void drawicon (Rect r, short bitdepth, Handle hbits) {
	
	if (bitdepth == 1) {
		
		PlotIcon (&r, hbits);
		
		return;
		}
	} /*drawicon*/


static void constraintorect (Rect *rconstrained, Rect rcontains, boolean flcenter) {
	
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
	
	OffsetRect (&r, dh, dv);
	
	*rconstrained = r; /*copy back to parameter*/
	} /*constraintorect*/


void constraintodesktop (Rect *rconstrained) {
	
	Rect rdesktop = (**LMGetGrayRgn ()).rgnBBox;
	
	InsetRect (&rdesktop, 10, 10);
	
	rdesktop.top += doctitlebarheight; /*leave room for a title bar at top of rectangle*/
	
	constraintorect (rconstrained, rdesktop, false);
	} /*constraintodesktop*/
	
		
void ellipsize (Str255 bs, short width) {

	/*
	if the string fits inside the given number of pixels, fine -- do nothing
	and return.
	
	if not, return a string that does fit, with ellipses representing the 
	deleted characters.  ellipses are generated by pressing option-semicolon.
	*/
	
	char len;
	short newwidth;
	
	if ((newwidth = StringWidth (bs)) <= width) /*nothing to do, the string fits*/
		return;
	
	len = bs [0]; /* current length in characters*/
	
	width -= CharWidth ('É'); /* subtract width of ellipses*/
		
	do { /*until it fits (or we run out of characters)*/
	
		newwidth -= CharWidth (bs [len]);
		
		--len;
	} while ((newwidth > width) && (len != 0));
	
	++len; /*make room for the ellipses*/
	
	bs [len] = 'É'; 
	
	bs [0] = (char) len;
	} /*ellipsize*/
	

void centerstring (Rect r, bigstring bs) {
	
	/*
	draw the string in the current font, size and style, centered inside
	the indicated rectangle.
	*/
	
	short lh = globalfontinfo.ascent + globalfontinfo.descent; /*line height*/
	short rh = r.bottom - r.top;
	short rw = r.right - r.left;
	short h, v;
	
	ellipsize (bs, rw); /*make sure it fits inside the rectangle, width-wise*/
	
	h = r.left + ((rw - StringWidth (bs)) / 2);
	
	v = r.top + ((rh - lh) / 2) + globalfontinfo.ascent;
	
	MoveTo (h, v);
	
	pushclip (r);
	
	DrawString (bs);
	
	popclip ();
	} /*centerstring*/


void centerwindow (WindowPtr w, Rect rscreen) {

	short h, v;
	Rect r;
	short minv;
	
	r = (*w).portRect;
	
	h = rscreen.left + (((rscreen.right - rscreen.left) - (r.right - r.left)) / 2);
	
	v = rscreen.top + (((rscreen.bottom - rscreen.top) - (r.bottom - r.top)) / 6);
	
	minv = getmenubarheight () + doctitlebarheight + 10;
	
	if (v < minv)
		v = minv;
	
	MoveWindow (w, h, v, false);
	} /*centerwindow*/


void grayrect (Rect r) {

	pushpen ();
	
	PenMode (patBic);
	
	PenPat (&quickdrawglobal (gray));
	
	PaintRect (&r);
	
	poppen ();
	} /*grayrect*/
	

boolean pointinrect (Point pt, Rect r) {
	
	return (PtInRect (pt, &r));
	} /*pointinrect*/


void validrect (Rect r) {
	
	ValidRect (&r);
	} /*validrect*/
	
	
void zerorect (Rect *rzero) {
	
	Rect *r = rzero;
	
	(*r).top = (*r).left = (*r).bottom = (*r).right = 0;
	} /*zerorect*/

	
boolean pushemptyclip (void) {
	
	/*
	set up the clip region so that no display will occur
	*/
	
	Rect r;
	
	zerorect (&r);
	
	return (pushclip (r));
	} /*pushemptyclip*/


void globaltolocalpoint (WindowPtr w, Point *pt) {
	
	pushmacport (w);
	
	GlobalToLocal (pt);
	
	popmacport ();
	} /*globaltolocalpoint*/
	
	
void localtoglobalpoint (WindowPtr w, Point *pt) {
	
	if (w == nil)
		w = quickdrawglobal (thePort);
		
	pushmacport (w);
	
	LocalToGlobal (pt);
	
	popmacport ();
	} /*localtoglobalpoint*/
	
	
void scrollrect (Rect r, short dh, short dv) {
	
	/*
	a front end for the Macintosh routine that scrolls a rectangle of pixels.
	*/
	
	RgnHandle rgn;
	
	rgn = NewRgn ();
	
	ScrollRect (&r, dh, dv, rgn);
	
	InvalRgn (rgn);
	
	/*FillRgn (rgn, gray);*/
	
	DisposeRgn (rgn);
	} /*scrollrect*/


boolean equalrects (Rect r1, Rect r2) {
	
	return (
		(r1.top == r2.top) && (r1.left == r2.left) && 
		
		(r1.bottom == r2.bottom) && (r1.right == r2.right));
	} /*equalrects*/



