
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

#include "tablestructure.h"

#include "byteorder.h"

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

OSStatus loadicondatafromodb(bigstring bsadricon, bigstring bsicondatatype, Handle *hicon);

boolean ploticonfromodb (const Rect *r, short align, short transform, bigstring bsadricon) {
	//bigstring bsadricon = "\psystem.verbs.builtins.Frontier.tools.data.nodeTypes.link.icon.mac";
	

	IconRef iconRef;
	IconFamilyHandle iconHand;
	SInt32 theSize;
	OSStatus theErr;
	Handle hicon;
	bigstring bsadriconpart;
	
	theErr = noErr;
	theSize = sizeof(OSType) + sizeof(OSType);
	
	newhandle(theSize, (Handle*) &iconHand);

	//iconHand = (IconFamilyHandle) getnewhandle(theSize, false);
	
	if (iconHand == NULL) theErr = memFullErr;
	
	if (theErr == noErr) {
		(*iconHand)->resourceType = EndianU32_NtoB(kIconFamilyType);
		(*iconHand)->resourceSize = EndianU32_NtoB(theSize);
	}
	
	if (theErr == noErr) {
		setemptystring(bsadriconpart);
		copystring(bsadricon, bsadriconpart);
		pushstring(BIGSTRING("\x05" ".ics4"), bsadriconpart);
		theErr = loadicondatafromodb(bsadriconpart, BIGSTRING("\x04" "ics4"), &hicon);
		
		if (theErr == noErr) {
			theErr = SetIconFamilyData(iconHand, kSmall4BitData, hicon);
			disposehandle(hicon);
		}
	}
	
	if (theErr == noErr) {
		setemptystring(bsadriconpart);
		copystring(bsadricon, bsadriconpart);
		pushstring(BIGSTRING("\x05" ".ics8"), bsadriconpart);
		theErr = loadicondatafromodb(bsadriconpart, BIGSTRING("\x04" "ics8"), &hicon);
		
		if (theErr == noErr) {
			theErr = SetIconFamilyData(iconHand, kSmall8BitData, hicon);
			disposehandle(hicon);
		}
	}
	
	if (theErr == noErr) {
		setemptystring(bsadriconpart);
		copystring(bsadricon, bsadriconpart);
		pushstring(BIGSTRING("\x09" ".icspound"), bsadriconpart);
		theErr = loadicondatafromodb(bsadriconpart, BIGSTRING("\x08" "icspound"), &hicon);
		
		if (theErr == noErr) {
			theErr = SetIconFamilyData(iconHand, kSmall1BitMask, hicon);
			disposehandle(hicon);
		}
	}
	
	if (theErr == noErr) {
		theErr = GetIconRefFromIconFamilyPtr(*iconHand, GetHandleSize((Handle) iconHand), &iconRef);
	}
	
	if (theErr == noErr) {
		theErr = PlotIconRef(r, align, transform, kIconServicesNormalUsageFlag, iconRef);
	}
	
	setemptystring(bsadriconpart);
	ReleaseIconRef(iconRef);
	disposehandle((Handle) iconHand);
	
	return theErr == noErr;
}

OSStatus loadicondatafromodb(bigstring bsadricon, bigstring bsicondatatype, Handle *hicon) {
	bigstring bsname;
	hdlhashtable ht;
	hdlhashnode hn;
	tyvaluerecord iconvalue;
	//bigstring bsiconname = "\pics4";
	boolean flexpanded = false;
	boolean fllookup = false;
	
	pushhashtable (roottable);
	
	disablelangerror ();
	
	flexpanded = langexpandtodotparams (bsadricon, &ht, bsname);
	enablelangerror ();
	pophashtable ();
    
    if (!flexpanded) {
        return resNotFound;
    }
	
	fllookup = hashtablelookup (ht, bsicondatatype, &iconvalue, &hn);
	copyhandle (iconvalue.data.binaryvalue, hicon);
	stripbinarytypeid (*hicon);
	
	return noErr;
}

boolean ploticonresource (const Rect *r, short align, short transform, short resid) {
	
	/*
	1.0b20 dmb: try plotting cicn if icon family isn't found. This 
	allows all of the stand system icons to be used.
	
	1.0b21 dmb: try geting normal b&w icon if all else fails

	5.0a8 dmb: use srccopy, not srcand for win blits
	 
	Note that the Windows version does NOT use the transform parameter
	*/
	
	OSErr ec;
	CIconHandle hcicon;
	Handle hicon;
	Rect rlocal = *r;
	
#ifdef SWAP_BYTE_ORDER
	/* For some unknown reason the Intel OS X builds shift the icon displays */
	rlocal.top		+= 3;
	rlocal.bottom	+= 3;
	rlocal.left	+= 6;
	rlocal.right += 6;
#endif
	
	ec = PlotIconID (&rlocal, align, transform, resid);
	
	if (ec == noErr)
		return (true);
	
	hcicon = GetCIcon (resid);
	
	if (hcicon != nil) {
		
		PlotCIcon (&rlocal, hcicon);
		
		DisposeCIcon (hcicon);
		
		return (true);
		}
	
	hicon = GetIcon (resid);
	
	if (hicon != nil) {
		
		PlotIcon (&rlocal, hicon);
		
		/*ReleaseResource (hicon);*/ /*dmb 1.0b21 - don't need to*/
		
		return (true);
		}
	
	return (false);

	} /*ploticonresource*/



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
	
	//
	// 2006-06-18 creedon: FSRef-ized
	//
	// 7.0b9 PBS: Open a resource file just once, store info about it, so it doesn't have to be opened for each rendering.
	//
	
	bigstring bsappearancefolder = "\x0a" "Appearance";
	bigstring bsiconsfolder = "\x05" "Icons";
	tyfilespec programfilespec, appfolderfilespec, appearancefolder, iconsfolder, iconfilespec;
	short r, ixcurricon;
	
	if (ixnexticon >= maxcustomicontypes)
		return (false); // limit reached
	
	// get app filespec
	
	getapplicationfilespec (nil, &programfilespec);
	macgetfilespecparent(&programfilespec, &appfolderfilespec); //PBS 8 Sep 2010: need parent folder of app before getting Appearance folder
	
	// get Appearances folder
	
	if (macgetfilespecchildfrombigstring (&appfolderfilespec, bsappearancefolder, &appearancefolder) != noErr)
		return (false);
	
	// get Icons folder
	
	if (macgetfilespecchildfrombigstring (&appearancefolder, bsiconsfolder, &iconsfolder) != noErr)
		return (false);
	
	// get icon file
	
	if (macgetfilespecchildfrombigstring (&iconsfolder, bsiconname, &iconfilespec) != noErr)
		return (false);
	
	if (!openresourcefile (&iconfilespec, &r, resourcefork))
		return (false);
	
	if (r == -1)
		return (false);
	
	*rnum = r;
	
	ixcurricon = ixnexticon;
	
	ixnexticon++;
	
	alllower (bsiconname);
	
	copystring (bsiconname, icontypes [ixcurricon].bstype);
	
	icontypes [ixcurricon].rnum = r;
	
	return (true);	
	
} // customiconload



boolean ploticoncustom (const Rect *r, short align, short transform, bigstring bsiconname) {
	
	/*
	7.0b9 PBS: plot a custom icon.
	*/
	

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


	} /*ploticoncustom*/


#if 0

/*7.0b9 PBS: saved version that had test code for getting a bitmap from the odb.*/

boolean ploticoncustom (const Rect *r, short align, short transform) {
	
	/*
	7.0b9 PBS: plot a custom icon.
	*/
	

	} /*ploticoncustom*/

#endif


boolean ploticon (const Rect *r, short id) {
	
	/*
	plot the icon with the given id in rect r.  if a color icon is available, 
	and the machine supports color, use the color version
	*/
	
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

	} /*ploticon*/



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


void drawlabeledwindoidicon (Rect r, bigstring bslabel, boolean flhastext, boolean flpressed) {
	
	/*
	draw the labeled windoid icon in the appropriate state
	*/
	
	
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





