
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

#include "quickdraw.h"
#include "font.h"
#include "cursor.h"
#include "mouse.h"
#include "ops.h"
#include "icon.h"

#include "shellprint.h"
#include "strings.h" /*7.0b9 PBS*/
#include "file.h" /*7.0b9 PBS*/
#include "resources.h" /*7.0b9 PBS*/
#include "launch.h" /*7.0b9 PBS*/

/*
#define macicon 128
#define bugicon 129
#define diskicon 130
#define mruserlandicon 131
#define depressedmruserlandicon 132
#define invertedmenubaricon 136

#define firsthandicon 140
#define lasthandicon 147

#define systemerroricon macicon
#define langerroricon mruserlandicon
#define fileerroricon diskicon
*/

#define shrunkenwindoidicon 133
#define menubaricon 134
#define depressedshrunkenwindoidicon 135
#define windoidwithtexticon 137
#define windoidwithnotexticon 138



boolean ploticonresource (const Rect *r, short align, short transform, short resid) {
	
	/*
	1.0b20 dmb: try plotting cicn if icon family isn't found. This 
	allows all of the stand system icons to be used.
	
	1.0b21 dmb: try geting normal b&w icon if all else fails

	5.0a8 dmb: use srccopy, not srcand for win blits
	*/
	
#ifdef MACVERSION
	OSErr ec;
	CIconHandle hcicon;
	Handle hicon;
	
	ec = PlotIconID (r, align, transform, resid);
	
	if (ec == noErr)
		return (true);
	
	hcicon = GetCIcon (resid);
	
	if (hcicon != nil) {
		
		PlotCIcon (r, hcicon);
		
		DisposeCIcon (hcicon);
		
		return (true);
		}
	
	hicon = GetIcon (resid);
	
	if (hicon != nil) {
		
		PlotIcon (r, hicon);
		
		/*ReleaseResource (hicon);*/ /*dmb 1.0b21 - don't need to*/
		
		return (true);
		}
	
	return (false);
#endif

#ifdef WIN95VERSION
	HBITMAP hbm, oldbm;
	BITMAP bm;
	HDC hdcsrc, hdc;
	boolean flprinting;
	HDC hdcmask;
	HBITMAP hbmmask, oldmaskbm;
	COLORREF oldclr, oldclr2;


	hbm = LoadBitmap (shellinstance, MAKEINTRESOURCE (resid));

	if (hbm)
		{
		hdc = getcurrentDC();

		flprinting = iscurrentportprintport ();

		if (hdc)
			{
			hdcsrc = CreateCompatibleDC (hdc);

			if (hdcsrc)
				{
				GetObject (hbm, sizeof (BITMAP), &bm);

				oldbm = (HBITMAP) SelectObject (hdcsrc, hbm);
				
				if (flprinting) {
				//	StretchBlt (hdc, r->left, r->top, r->right-r->left, r->bottom - r->top, hdcsrc, 0,0, bm.bmWidth, bm.bmHeight, SRCCOPY);
					}
				else {
					hdcmask = CreateCompatibleDC (hdc);
					hbmmask = CreateBitmap (bm.bmWidth, bm.bmHeight, 1, 1, NULL);

					if (hdcmask && hbmmask) {
						oldmaskbm = (HBITMAP) SelectObject (hdcmask, hbmmask);

						oldclr = SetBkColor (hdcsrc, RGB(255,255,255));

						BitBlt (hdcmask, 0,0,bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCCOPY);

						SetBkColor (hdcsrc,oldclr);

						eraserect (*r);

						oldclr = SetBkColor (hdc, RGB(255,255,255));
						oldclr2 = SetTextColor (hdc, RGB(0,0,0));

						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCINVERT);
						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcmask, 0,0, SRCAND);
						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCINVERT);

						SetBkColor (hdc,oldclr);
						SetTextColor (hdc,oldclr2);

						SelectObject (hdcmask, oldmaskbm);

	//					BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCAND);
						}

					DeleteObject (hbmmask);
					DeleteDC (hdcmask);
					}

				SelectObject (hdcsrc, oldbm);
				DeleteDC (hdcsrc);
				}
			}

		DeleteObject (hbm);
		}
	
	return (true);	
#endif
	} /*ploticonresource*/


#ifdef MACVERSION


struct tycustomicontypeinfo icontypes [maxcustomicontypes]; /*array*/

static short ixnexticon = 0; /*keep track of which is next to load*/


boolean customicongetrnum (bigstring bstype, short *rnum) {
	
	/*
	7.0b9 PBS: get the rnum of a file containing custom icon.
	*/
	
	short i = 0;
	
	alllower (bstype);
	
	while (i < ixnexticon) {
		
		if (equalstrings (bstype, icontypes [i].bstype)) {
		
			*rnum = icontypes [i].rnum;
			
			return (true);
			} /*if*/
		
		i++;
		
		if (i == maxcustomicontypes)
		
			break;		
		} /*while*/
	
	return (false);	
	} /*customicongetresid*/


static boolean customiconload (bigstring bsiconname, short *rnum) {
	
	/*
	7.0b9 PBS: Open a resource file just once, store info about it,
	so it doesn't have to be opened for each rendering.
	*/
	
	bigstring bsappearancefolder = "\pAppearance";
	bigstring bsiconsfolder = "\pIcons";
	OSErr err;
	CInfoPBRec pb;
	long dirid;
	FSSpec programfilespec;
	FSSpec appearancefolder, iconsfolder, iconfilespec;
	short r, ixcurricon;
	
	if (ixnexticon >= maxcustomicontypes)
	
		return (false); /*limit reached*/
	
	/*Get app file spec*/
	
	getapplicationfilespec (nil, &programfilespec);
	
	dirid = programfilespec.parID;
	
	/*Get Appearances folder*/
	
	err = FSMakeFSSpec (programfilespec.vRefNum, dirid, bsappearancefolder, &appearancefolder);
	
	if (!getmacfileinfo (&appearancefolder, &pb))
	
		return (false);

	dirid = pb.dirInfo.ioDrDirID;
	
	/*Get Icons folder*/
	
	err = FSMakeFSSpec (appearancefolder.vRefNum, dirid, bsiconsfolder, &iconsfolder);
	
	if (err != noErr)
	
		return (false);
	
	if (!getmacfileinfo (&iconsfolder, &pb))
	
		return (false);
		
	dirid = pb.dirInfo.ioDrDirID;

	/*Get icon file*/
	
	err = FSMakeFSSpec (iconsfolder.vRefNum, dirid, bsiconname, &iconfilespec);
	
	if (err != noErr)
	
		return (false);
	
	r = FSpOpenResFile (&iconfilespec, fsRdPerm);
	
	if (r == -1)
	
		return (false);
	
	*rnum = r;
	
	ixcurricon = ixnexticon;
	
	ixnexticon++;
	
	alllower (bsiconname);
	
	copystring (bsiconname, icontypes [ixcurricon].bstype);
	
	icontypes [ixcurricon].rnum = r;
	
	return (true);	
	} /*customiconload*/

#endif


boolean ploticoncustom (const Rect *r, short align, short transform, bigstring bsiconname) {
	
	/*
	7.0b9 PBS: plot a custom icon.
	*/
	
#ifdef MACVERSION

	short rnum;
	short resid = 128; /*Always 128 for custom icons*/
	short saveresfile;

	/*Get the resource reference for the file containing this icon.*/
	
	if (!customicongetrnum (bsiconname, &rnum)) { /*Already loaded?*/
	
		if (!customiconload (bsiconname, &rnum)) { /*Try to load it.*/
		
			return (false);
			} /*if*/
		} /*if*/
	
	saveresfile = CurResFile ();
	
	UseResFile (rnum);
	
	ploticonresource (r, align, transform, resid);
	
	UseResFile (saveresfile);
	
	return (true);

#endif

#ifdef WIN95VERSION
	HBITMAP hbm, oldbm;
	BITMAP bm;
	HDC hdcsrc, hdc;
	boolean flprinting;
	HDC hdcmask;
	HBITMAP hbmmask, oldmaskbm;
	COLORREF oldclr, oldclr2;
	bigstring bsfilepath = "\x11" "Appearance\\Icons\\";
	char cfilepath [256];
	
	pushstring (bsiconname, bsfilepath); /*add file name to folder path*/

	pushstring ("\x04" ".bmp", bsfilepath); /*add .bmp file extension*/

	copyptocstring (bsfilepath, cfilepath);

	/*Load the image from a file.*/

	hbm = LoadImage (shellinstance, cfilepath, IMAGE_BITMAP, 16, 16, LR_LOADFROMFILE);

	if (hbm == NULL) /*Load failed, return false, use default icon.*/

		return (false);

	if (hbm)
		{
		hdc = getcurrentDC();

		flprinting = iscurrentportprintport ();

		if (hdc)
			{
			hdcsrc = CreateCompatibleDC (hdc);

			if (hdcsrc)
				{
				GetObject (hbm, sizeof (BITMAP), &bm);

				oldbm = (HBITMAP) SelectObject (hdcsrc, hbm);
				
				if (flprinting) {
				//	StretchBlt (hdc, r->left, r->top, r->right-r->left, r->bottom - r->top, hdcsrc, 0,0, bm.bmWidth, bm.bmHeight, SRCCOPY);
					}
				else {
					hdcmask = CreateCompatibleDC (hdc);
					hbmmask = CreateBitmap (bm.bmWidth, bm.bmHeight, 1, 1, NULL);

					if (hdcmask && hbmmask) {
						oldmaskbm = (HBITMAP) SelectObject (hdcmask, hbmmask);

						oldclr = SetBkColor (hdcsrc, RGB(255,255,255));

						BitBlt (hdcmask, 0,0,bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCCOPY);

						SetBkColor (hdcsrc,oldclr);

						eraserect (*r);

						oldclr = SetBkColor (hdc, RGB(255,255,255));
						oldclr2 = SetTextColor (hdc, RGB(0,0,0));

						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCINVERT);
						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcmask, 0,0, SRCAND);
						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCINVERT);

						SetBkColor (hdc,oldclr);
						SetTextColor (hdc,oldclr2);

						SelectObject (hdcmask, oldmaskbm);

	//					BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCAND);
						}

					DeleteObject (hbmmask);
					DeleteDC (hdcmask);
					}

				SelectObject (hdcsrc, oldbm);
				DeleteDC (hdcsrc);
				}
			}

		DeleteObject (hbm);
		}

	return (true);	
#endif
	} /*ploticoncustom*/


#if 0

/*7.0b9 PBS: saved version that had test code for getting a bitmap from the odb.*/

boolean ploticoncustom (const Rect *r, short align, short transform) {
	
	/*
	7.0b9 PBS: plot a custom icon.
	*/
	

#ifdef WIN95VERSION
	HBITMAP hbm, oldbm;
	BITMAP bm;
	HDC hdcsrc, hdc;
	boolean flprinting;
	HDC hdcmask;
	HBITMAP hbmmask, oldmaskbm;
	COLORREF oldclr, oldclr2;

	bigstring bsadricon = "\x18" "user.playlist.icons.face";
	bigstring bsname;
	hdlhashtable ht;
	hdlhashnode hn;
	tyvaluerecord iconvalue;
	bigstring bsiconname = "\x04" "face";
	boolean flexpanded = false;
	boolean fllookup = false;
	Handle hicon;

	pushhashtable (roottable);

	disablelangerror ();

	flexpanded = langexpandtodotparams (bsadricon, &ht, bsname);
	enablelangerror ();
	pophashtable ();


	fllookup = hashtablelookup (ht, bsiconname, &iconvalue, &hn);
	copyhandle (iconvalue.data.binaryvalue, &hicon);
	stripbinarytypeid (hicon);

	hbm = CreateBitmap (16, 16, 1, 32, hicon);

//	hbm = LoadImage (shellinstance, "face.bmp", IMAGE_BITMAP, 16, 16, LR_LOADFROMFILE);

	if (hbm)
		{
		hdc = getcurrentDC();

		flprinting = iscurrentportprintport ();

		if (hdc)
			{
			hdcsrc = CreateCompatibleDC (hdc);

			if (hdcsrc)
				{
				//GetObject (hbm, sizeof (BITMAP), &bm);

				bm.bmType = 0;
				bm.bmWidth = 16;
				bm.bmHeight = 16;
				bm.bmWidthBytes = 32;
				bm.bmPlanes = 1;
				bm.bmBitsPixel = 16;
				bm.bmBits = hicon;

				oldbm = (HBITMAP) SelectObject (hdcsrc, hbm);
				
				if (flprinting) {
				//	StretchBlt (hdc, r->left, r->top, r->right-r->left, r->bottom - r->top, hdcsrc, 0,0, bm.bmWidth, bm.bmHeight, SRCCOPY);
					}
				else {
					hdcmask = CreateCompatibleDC (hdc);
					hbmmask = CreateBitmap (bm.bmWidth, bm.bmHeight, 1, 1, NULL);

					if (hdcmask && hbmmask) {
						oldmaskbm = (HBITMAP) SelectObject (hdcmask, hbmmask);

						oldclr = SetBkColor (hdcsrc, RGB(255,255,255));

						BitBlt (hdcmask, 0,0,bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCCOPY);

						SetBkColor (hdcsrc,oldclr);

						eraserect (*r);

						oldclr = SetBkColor (hdc, RGB(255,255,255));
						oldclr2 = SetTextColor (hdc, RGB(0,0,0));

						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCINVERT);
						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcmask, 0,0, SRCAND);
						BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCINVERT);

						SetBkColor (hdc,oldclr);
						SetTextColor (hdc,oldclr2);

						SelectObject (hdcmask, oldmaskbm);

	//					BitBlt (hdc, r->left, r->bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcsrc, 0,0, SRCAND);
						}

					DeleteObject (hbmmask);
					DeleteDC (hdcmask);
					}

				SelectObject (hdcsrc, oldbm);
				DeleteDC (hdcsrc);
				}
			}

		DeleteObject (hbm);
		}
	disposehandle (hicon);
	return (true);	
#endif
	} /*ploticoncustom*/

#endif


boolean ploticon (const Rect *r, short id) {
	
	/*
	plot the icon with the given id in rect r.  if a color icon is available, 
	and the machine supports color, use the color version
	*/
	
#ifdef MACVERSION
	Handle hicon;
	CIconHandle hcicn;
	
	if (systemhascolor ()) {
		
		hcicn = GetCIcon (id);
		
		if (hcicn != nil) {
			
			PlotCIcon (r, hcicn);
			
			DisposeCIcon (hcicn);
			
			return (true);
			}
		}
	
	hicon = GetResource ('ICON', id);
	
	if (hicon != nil) {
		
		PlotIcon (r, hicon);
		
		return (true);
		}
	
	return (false);
#endif

#ifdef WIN95VERSION
	return (ploticonresource (r, 0, 0, id));
#endif
	} /*ploticon*/


#if !flruntime

void drawlabeledicon (const Rect *r, short resnum, bigstring bslabel, boolean flinverted) {
	
	/*
	draw the indicated icon resource with the label below it.
	
	the label is horizontally centered flush with the bottom of the rectangle.
	
	the icon is vertically and horizontally centered in the rectangle minus the
	area where the label is drawn.
	
	if flinverted is true, the icon is drawn and then inverted.
	
	4/3/91 dmb: not safe to do a validrect here, unless we're willing to 
	calculate the intersection of the rect and the clip region.
	*/
	
	Rect rlabel, ricon, rcenter;
	
	/*
	validrect (r);
	*/
	
	pushstyle (geneva, 9, 0);
	
	rlabel = *r;
	
	rlabel.top = (*r).bottom - globalfontinfo.ascent - globalfontinfo.descent;
	
	centerstring (rlabel, bslabel);
	
	popstyle ();
	
	rcenter = *r;
	
	rcenter.bottom = rlabel.top; /*confine icon to area above the label*/
	
	ricon.top = ricon.left = 0;
	
	ricon.bottom = ricon.right = 32;
	
	centerrect (&ricon, rcenter);
	
	if (ploticon (&ricon, resnum)) {
			
		if (flinverted)
			invertrect (ricon);
		}
	} /*drawlabeledicon*/
	
/*
drawiconsequence (Rect r, short firsticon, short lasticon, bigstring bs) {
	
	register short i;
	
	for (i = firsticon; i <= lasticon; i++) {
		
		drawlabeledicon (r, i, bs, false);
		
		delayticks (1);
		}
	} /%drawiconsequence%/
*/


#if TARGET_API_MAC_CARBON == 1

static void
MyThemeButtonDrawCallback (
		const Rect					*bounds,
		ThemeButtonKind				 kind,
		const ThemeButtonDrawInfo	*info,
		UInt32						 refcon,
		SInt16						 depth,
		Boolean						 isColorDev)
{
#pragma unused (kind, info, refcon, depth, isColorDev)

		/*
		7.0b48 PBS: draw the arrow for a pushbutton. It will be centered.
		*/
		
		Rect rarrow;
		//ptrstring bsptr = (ptrstring) refcon;
		
		setrect (&rarrow, 0, 0, 9, 9);
		
		centerrect (&rarrow, *bounds);
		
		rarrow.left++; /*it appears to need it*/
				
		DrawThemePopupArrow (&rarrow, kThemeArrowRight, kThemeArrow9pt, kThemeStateActive, NULL, 0);

	} /*MyThemeButtonDrawCallback*/

#endif

void drawlabeledwindoidicon (Rect r, bigstring bslabel, boolean flhastext, boolean flpressed) {
	
	/*
	draw the labeled windoid icon in the appropriate state
	*/
	
	#if TARGET_API_MAC_CARBON == 1
	
		ThemeButtonDrawInfo drawinfo;
		ThemeButtonDrawUPP drawupp;
		Rect rbutton;
		
		drawupp = NewThemeButtonDrawUPP (MyThemeButtonDrawCallback);

		drawinfo.state = kThemeButtonOn;
		
		drawinfo.value = kThemeStateActive;

		if (flpressed)
			drawinfo.value = kThemeStatePressed;
		
		drawinfo.adornment = 0;
		
		rbutton = r;
		
		rbutton.bottom = rbutton.bottom - 12; /*leave room for label*/
		
		DrawThemeButton (&rbutton, kThemeLargeBevelButton, &drawinfo, nil, nil, drawupp, (unsigned long) bslabel); 
		
		//DisposeThemeButtonDrawUPP (MyThemeButtonDrawCallback);
		DisposeThemeButtonDrawUPP (drawupp);
		if (flhastext) {
			
			Rect rlabel;
			
			pushstyle (geneva, 9, 0);
	
			rlabel = r;
	
			rlabel.top = r.bottom - globalfontinfo.ascent - globalfontinfo.descent;
	
			centerstring (rlabel, bslabel);
	
			popstyle ();
			} /*if*/			
			
	#else

		register short resnum;
		
		if (flpressed)
			resnum = depressedshrunkenwindoidicon;
			
		else {
			if (flhastext)
				resnum = windoidwithtexticon;
			else
				resnum = windoidwithnotexticon;
			}
		
		drawlabeledicon (&r, resnum, bslabel, false);
	
	#endif
		
	} /*drawlabeledwindoidicon*/


boolean trackicon (Rect r, void (*displayiconcallback) (boolean)) {
	
	/*
	this is really more general than its name implies, but for now 
	it covers the common calling mousetracking conventions use by 
	all the windoid icons, and belongs here.
	
	12/12/90 dmb: if mousetrack returns false, icon has already been displayed 
	in off state, and shouldn't be redisplayed (in case callback uses simple 
	inversion)
	*/
	
	setcursortype (cursorisarrow); /*get rid of funny appendage*/
	
	(*displayiconcallback) (true); /*show icon in depressed state*/
	
	if (!mousetrack (r, displayiconcallback)) /*if false, mouse didn't stay in rectangle*/
		return (false);
	
	(*displayiconcallback) (false); /*show icon in un-depressed state*/
	
	return (true);
	} /*trackicon*/

#endif




