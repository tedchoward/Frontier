
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

#include <iac.h>
#include <ioa.h>
#include "ioapopup.h"
#include "appletmemory.h"


#define idpopupmenu 25000

#define popuparrowwidth 16 

#define popupdrawinset 5

#define horizinset 3 


typedef struct typackedpopupdata {
	
	short versionnumber;
	
	unsigned short haslabel;
	
	bigstring bspopup;
	
	short checkeditem;
	
	short popupwidth;
	
	long lenmenuitems;

	char waste [10];
	} typackedpopupdata;
	

typedef struct typackedB14popupdata { /*layout of a 1.0b14 popup record*/
	
	bigstring bs;
	
	Rect popuprect;
	
	unsigned short flarrows;
	
	short fontnum, fontsize, fontstyle;
	
	Rect leftarrowrect;
	
	Rect rightarrowrect;
	
	short checkeditem;
	
	short itemselected;
	
	short idmenu;
	
	unsigned short fldisposemenu;
	
	MenuHandle hmenu;
	
	ProcPtr getmenucallback;
	
	ProcPtr leftarrowcallback;
	
	ProcPtr rightarrowcallback;
	
	ProcPtr menuselectcallback;
	} typackedB14popupdata;


typedef char smalliconbits [32];

typedef smalliconbits *ptrsmalliconbits, **hdlsmalliconbits;


static boolean plotsmallicon (Rect r, short iconlist, short iconnum, boolean flinvert) {

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
		mode = srcOr; 
	
	//Code change by Timothy Paustian Saturday, May 20, 2000 10:06:20 PM
	//Changed to Opaque call for Carbon
	//I have not tested this yet.	
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	CopyBits(&bmap, GetPortBitMapForCopyBits(w), &bmap.bounds, &r, mode, nil);
	#else
	CopyBits (&bmap, &(*w).portBits, &bmap.bounds, &r, mode, nil);
	#endif
	return (true);
	} /*plotsmallicon*/
	
	
static long texthandlewidth (Handle htext) {

	return (TextWidth (*htext, 0, GetHandleSize (htext)));
	} /*texthandlewidth*/
	
	
static void disposehandle (h) Handle h; {
	
	if (h != nil) DisposeHandle (h);
	} /*disposehandle*/
	
	
static boolean pushchar (byte ch, bigstring bs) {
	
	short newlen = bs [0] + 1;
	
	if (newlen > lenbigstring)
		return (false);
	
	bs [newlen] = ch;
	
	bs [0] = newlen;
	
	return (true);
	} /*pushchar*/
	
	
static boolean getmenuitem (MenuHandle hmenu, short ixmenu, bigstring bs) {
	
	if (ixmenu <= 0)
		return (false);
		
	GetMenuItemText (hmenu, ixmenu, bs);
		
	return (true);
	} /*getmenuitem*/


static void uncheckallmenuitems (MenuHandle hmenu) {
	
	short i;
	short ct;
	//Code change by Timothy Paustian Sunday, June 25, 2000 10:50:50 PM
	//updated to modern functions.
	ct = CountMenuItems (hmenu);
	
	for (i = 1; i <= ct; i++)
		CheckMenuItem (hmenu, i, false);
	} /*uncheckallmenuitems*/
	
	
static boolean equalstrings (bs1, bs2) bigstring bs1, bs2; {

	/*
	return true if the two strings (pascal type, with length-byte) are
	equal.  return false otherwise.
	*/

	ptrchar p1 = (ptrchar) bs1, p2 = (ptrchar) bs2;
	char ct = *p1 + 1;
	
	while (ct--) 
		
		if (*p1++ != *p2++)
		
			return (false);
		
	return (true); /*loop terminated*/
	} /*equalstrings*/
	
	
static boolean pushpopupitem (hdlobject h, bigstring bs, boolean flenabled) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	MenuHandle hmenu = (**hdata).hmenu;
	bigstring bsspace;
	
	if (equalstrings (bs, "\p(-")) /*take disabled seperator as is*/
		AppendMenu (hmenu, bs);
	
	else { /*to allow meta-characters in bs, append blank item, then set item text*/
		
		setstringwithchar (chspace, bsspace);
		
		AppendMenu (hmenu, bsspace);
		//Code change by Timothy Paustian Sunday, June 25, 2000 10:51:36 PM
		//Updated to modern calls
		if (!isemptystring (bs))
			SetMenuItemText (hmenu, CountMenuItems (hmenu), bs);
		}
	//Code change by Timothy Paustian Sunday, June 25, 2000 10:52:07 PM
	//updated
	if (!flenabled) {
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		DisableMenuItem (hmenu, CountMenuItems (hmenu));
		#else
		DisableItem (hmenu, CountMenuItems (hmenu));
		#endif
		}

	return (true);
	} /*pushpopupitem*/
	
	
boolean getnthpopupitem (hdlobject h, short n, bigstring bs) {
	
	/*
	get the nth string from the popup menu object. it's a bit slower to load
	up the menu using calls to this guy, but it's simpler, and it's nice to
	have this mess confined to one routine.
	
	n is 1-based.
	*/
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	Handle htext = (**hdata).hmenuitems;
	short i;
	short ctchars;
	short itemnum = 1;
	
	ctchars = GetHandleSize (htext);
	
	setstringlength (bs, 0);
		
	for (i = 0; i < ctchars; i++) {
		
		char ch = (*htext) [i];
		
		if (ch == ';') {
			
			if (itemnum == n)
				return (true);
				
			itemnum++;
			
			setstringlength (bs, 0);
			}
		else
			pushchar (ch, bs);
		} /*for*/
	
	return ((itemnum == n) && (stringlength (bs) > 0));
	} /*getnthpopupitem*/
	
		
static boolean getpopupmenu (hdlobject h) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	MenuHandle hmenu;
	bigstring bs;
	short n;
	
	hmenu = NewMenu (idpopupmenu, "\px");
	
	(**hdata).hmenu = hmenu;
		
	if (hmenu == nil)
		return (false);

	n = 1;
	
	while (getnthpopupitem (h, n++, bs)) {
	
		if (!pushpopupitem (h, bs, true))
			return (false);
		} /*while*/
	
	return (true);
	} /*getpopupmenu*/
	
	
static void idealpopupsize (hdlobject hobject, short *height, short *width) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**hobject).objectdata;
	short h, w;
	short maxwidth;
	FontInfo fi;
	
	GetFontInfo (&fi);
	
	h = fi.ascent + fi.descent + 4;
	
	maxwidth = StringWidth ((**hdata).bspopup);
	
	if (getpopupmenu (hobject)) { /*use the strings in the menu to determine width*/
		
		MenuHandle hmenu = (**hdata).hmenu;
		short i;
		short sw;
		short lastitem;
		bigstring bs;
		//Code change by Timothy Paustian Sunday, June 25, 2000 10:52:18 PM
		//updated
		lastitem = CountMenuItems (hmenu);
		
		for (i = 1; i <= lastitem; i++) {
			
			getmenuitem (hmenu, i, bs);
			
			sw = StringWidth (bs);
			
			if (sw > maxwidth)
				maxwidth = sw;
			} /*for*/
		
		DisposeMenu (hmenu);
		}
		
	w = maxwidth + popuparrowwidth + popupdrawinset + 8;
	
	*height = h;
	
	*width = w;
	} /*idealpopupsize*/
	
	
static boolean cleanpopup (hdlobject h, short height, short width, Rect *r) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	short gridunits = (**(**h).owningcard).gridunits;
	short popupheight, popupwidth;
	
	idealpopupsize (h, &popupheight, &popupwidth);
	
	popupwidth = IOAmakemultiple (popupwidth, gridunits);
	
	if ((**hdata).haslabel) {
	
		width += horizinset + popupwidth;
		
		width = IOAmakemultiple (width, gridunits);
		}
	else
		width = popupwidth;
	
	(*r).right = (*r).left + width; 

	(*r).bottom = (*r).top + height;
	
	/*side-effect -- we use the popup width when drawing the object*/ {
		
		hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
		
		(**hdata).popupwidth = popupwidth;
		}
	
	return (true);
	} /*cleanpopup*/
	

static void getpopuprects (hdlobject h, Rect *rtext, Rect *rpopup) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	Rect robject = (**h).objectrect;
	
	if ((**hdata).haslabel) {
	
		*rpopup = robject; /*set right, top, bottom*/
	
		(*rpopup).left = (*rpopup).right - (**hdata).popupwidth;
	
		*rtext = robject; /*set left, top, bottom*/
	
		(*rtext).right = (*rpopup).left - horizinset;
		}
	else {
		*rpopup = robject; /*the popup takes up the whole rectangle*/
		
		IOAclearbytes (rtext, longsizeof (Rect));
		}
	} /*getpopuprects*/
	
	
static boolean clickpopup (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	hdlcard hc = (**h).owningcard;
	MenuHandle hmenu;
	
	(**hc).runmodechanges = true; /*indicate that one of the objects in the card changed*/
	
	if (!getpopupmenu (h)) /*error, but consume the mouse click*/
		return (true);
		
	hmenu = (**hdata).hmenu;
	
	uncheckallmenuitems (hmenu);
	//Code change by Timothy Paustian Sunday, June 25, 2000 10:51:50 PM
	//updated to modern call
	CheckMenuItem (hmenu, (**hdata).checkeditem, true);
	
	InsertMenu (hmenu, -1); /*see IM-V, p 236*/
	
	/*setcursortype (cursorisarrow);*/ /*arrow cursor is more appropriate here*/
	
	/*set pt to point at the upper left corner of the popup rect*/ {
		
		Rect r = (**h).objectrect;
		int			menuHeight;
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		menuHeight = GetMenuHeight(hmenu);
		#else
		menuHeight = (**hmenu).menuHeight;
		#endif
		
		pt.v = r.top;
		
		pt.h = r.right - (**hdata).popupwidth;
	
		if ((**hdata).checkeditem == 0) { /*no item selected*/
			
			CalcMenuSize (hmenu); 
		
			pt.v -= menuHeight / 2;
			}
			
		LocalToGlobal (&pt); 
		}
		
	/*standard UI -- invert the label text*/ {
		
		if ((**hdata).haslabel) {
		
			Rect rtext, rpopup;
		
			getpopuprects (h, &rtext, &rpopup);
		
			InvertRect (&rtext);
			}
		}
	
	/*jump into the roms*/ {
		
		long result;
		short lo, hi;
		
		result = PopUpMenuSelect (hmenu, pt.v, pt.h, (**hdata).checkeditem);
		
		lo = LoWord (result);
		
		hi = HiWord (result);
		
		if (hi > 0) { /*something was selected*/
			
			bigstring bs;
			
			(**hdata).checkeditem = lo;
			
			getmenuitem (hmenu, lo, bs);
			
			IOAcopystring (bs, (**hdata).bspopup);
			
			IOArunbuttonscript (h); /*1.0b15 -- run the action script*/
			}
		}
	
	DeleteMenu (idpopupmenu);
	
	DisposeMenu (hmenu);
		
	IOAinvalobject (h);
	
	(**(**h).owningcard).runtimevaluechanged = true; /*DW 10/15/95*/
	
	return (true);
	} /*clickpopup*/
	

#if TARGET_API_MAC_CARBON == 1

	static void MyThemeButtonDrawCallback (const Rect *bounds, ThemeButtonKind kind, const ThemeButtonDrawInfo *info,
		bigstring bs, SInt16 depth, Boolean isColorDev) {
		
		/*
		7.0b48 PBS: draw the label for a popup menu.
		*/
		pushstyle (geneva, 10, 0);

		movepento ((*bounds).left, (*bounds).top + 12);
		
		pendrawstring (bs);

		popstyle ();
		} /*MyThemeButtonDrawCallback*/

#endif


static boolean drawpopup (hdlobject h) {

	/*
	2002-10-13 AR: Reorganized variable declarations in draw popup code
	to eliminate compiler warnins about unused variables.
	*/
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	Rect rtext, rpopup;
	
	getpopuprects (h, &rtext, &rpopup);
	
	/*draw the popup menu*/ {
	
		#if TARGET_API_MAC_CARBON == 1
		
			Rect r = rpopup;
			ThemeButtonDrawUPP drawupp;
			ThemeButtonDrawInfo drawinfo;
			
			drawupp = NewThemeButtonDrawUPP (MyThemeButtonDrawCallback);
					
			drawinfo.state = kThemeButtonOn;
			
			drawinfo.value = kThemeStateActive;
			
			drawinfo.adornment = 0;
			
			//setrect (&r, rpopup.top, rpopup.left, rpopup.bottom + 1, rpopup.right + 1);
			
			//eraserect (r);
			
			DrawThemeButton (&r, kThemePopupButton, &drawinfo, nil, nil, drawupp, (unsigned long) (**hdata).bspopup);
			
			//DisposeThemeButtonDrawUPP (MyThemeButtonDrawCallback);
			DisposeThemeButtonDrawUPP (drawupp);
		
		#else
		
			Rect r = rpopup;
			FontInfo fi;
			bigstring bs;

			GetFontInfo (&fi);
			
			EraseRect (&r);
			
			InsetRect (&r, 1, 1);
			
			r.left = r.right - popuparrowwidth;
			
			plotsmallicon (r, 129, 0, false); /*display the down-pointing wedge inside the popup*/
		
			r = rpopup;
			
			FrameRect (&r);
			
			MoveTo (r.left + 1, r.bottom);
			
			LineTo (r.right, r.bottom);
			
			LineTo (r.right, r.top + 1);
			
			r.right -= popuparrowwidth;
			
			InsetRect (&r, 4, 2);
		
			MoveTo (r.left, r.bottom - fi.descent - 1);
			
			IOAcopystring ((**hdata).bspopup, bs);
			
			IOAellipsize (bs, r.right - r.left);
			
			DrawString (bs);
		
		#endif
		}
	
	/*draw the text*/ {
		
		if ((**hdata).haslabel) {
		
			if (!(**h).objecttransparent)
				EraseRect (&rtext);
			
			if (!(**(**h).owningcard).flskiptext) 
				IOAeditdrawtexthandle ((**h).objectvalue, rtext, (**h).objectjustification);
			}
		}
	
	return (true);
	} /*drawpopup*/
	
	
static boolean newpopupdata (hdlpopupdata *hdata) {

	hdlpopupdata h;
	
	h = (hdlpopupdata) NewHandleClear (longsizeof (typopupdata));
	
	if (h == nil)
		return (false);
	
	(**h).haslabel = true;
	
	(**h).checkeditem = 1;
	
	*hdata = h;
	
	return (true);
	} /*newpopupdata*/
	
	
static boolean initpopup (tyobject *obj) {
	
	hdlpopupdata hdata;
	Handle hmenuitems;
	
	if (!newpopupdata (&hdata))
		return (false);
	
	(*obj).objectdata = (Handle) hdata;
	
	IOAnewtexthandle ("\pRed;Green;Blue", &hmenuitems);
	
	(**hdata).hmenuitems = hmenuitems;
	
	(**hdata).checkeditem = 1;
	
	IOAcopystring ("\pRed", (**hdata).bspopup);
	
	return (true); /*we do want to edit it*/
	} /*initpopup*/
	
	
static boolean recalcpopup (hdlobject h, boolean flmajorrecalc) {
	
	bigstring errorstring;
	Handle htoss;
	
	if (!IOAevalscript (h, (**h).objectrecalcscript, &htoss, (**h).objectlanguage, errorstring))
		return (false);
	
	disposehandle (htoss);
	
	return (true); 
	} /*recalcpopup*/


static boolean xxxrecalcpopup (hdlobject h, boolean flmajorrecalc) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	Handle hmenuitems;
	
	if (!IOArecalcobjectvalue (h))
		return (false);
	
	if (!copyhandle ((**h).objectdata, &hmenuitems))
		return (false);
	
	disposehandle ((**hdata).hmenuitems); 
	
	(**hdata).hmenuitems = hmenuitems;
	
	(**hdata).checkeditem = 1;
	
	getnthpopupitem (h, (**hdata).checkeditem, (**hdata).bspopup);
	
	IOAinvalobject (h);
	
	return (true); 
	} /*xxxrecalcpopup*/


static boolean getpopupinvalrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	(*r).right += 3; /*4/2/93 DW: work around display bug in framework?*/
	
	(*r).bottom += 3;
	
	return (true);
	} /*getpopupinvalrect*/
	
	
static boolean canreplicatepopup (hdlobject h) {
	
	return (true); 
	} /*canreplicatepopup*/
	

static boolean getpopupeditrect (hdlobject h, Rect *redit) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	Rect rpopup;
	
	if ((**hdata).haslabel) {
	
		getpopuprects (h, redit, &rpopup);
	
		return (true); /*it can be edited*/
		}
	else
		return (false); /*it can't be edited*/
	} /*getpopupeditrect*/
	

static boolean getpopupvalue (hdlobject h, Handle *hvalue) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	bigstring bs;
	
	IOAcopystring ("\p\"", bs);

	IOApushstring ((**hdata).bspopup, bs);
	
	IOApushstring ("\p\"", bs);
	
	return (IOAnewtexthandle (bs, hvalue));
	} /*getpopupvalue*/
	

static boolean debugpopup (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugpopup*/
	

static boolean getpopupattributes (hdlobject h, AppleEvent *event) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	Handle hselectionstring, hsemicolonstring;
	
	IOAnewtexthandle ((**hdata).bspopup, &hselectionstring);

	hsemicolonstring = (**hdata).hmenuitems; HandToHand (&hsemicolonstring);
	
	IACglobals.event = event; 
	
	if (!IACpushbooleanparam ((**hdata).haslabel, 'hlbl'))
		return (false);
		
	if (!IACpushtextparam (hselectionstring, 'sels')) /*the selected string*/
		return (false);

	if (!IACpushshortparam ((**hdata).checkeditem, 'seln')) /*the selected number*/
		return (false);
	
	if (!IACpushtextparam (hsemicolonstring, 'mens')) /*the semi-colon separated string*/
		return (false);

	return (true);
	} /*getpopupattributes*/
	
	
static boolean setpopupattributes (hdlobject h, AppleEvent *event) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	Boolean haslabel;
	short checkeditem;
	Handle hmenuitems;
	
	IACglobals.event = event; 
	
	IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('hlbl', &haslabel)) 
		(**hdata).haslabel = haslabel; 
	
	IACglobals.nextparamoptional = true;
	
	if (IACgetshortparam ('seln', &checkeditem)) 
		(**hdata).checkeditem = checkeditem; 
		
	IACglobals.nextparamoptional = true;
	
	if (IACgettextparam ('mens', &hmenuitems)) {
		
		disposehandle ((**hdata).hmenuitems);
		
		(**hdata).hmenuitems = hmenuitems;
		}
	
	getnthpopupitem (h, (**hdata).checkeditem, (**hdata).bspopup);

	return (true);
	} /*setpopupattributes*/
	
	
static boolean packpopupdata (hdlobject hobject, Handle *hpackeddata) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**hobject).objectdata;
	Handle hmenuitems = (**hdata).hmenuitems;
	short lenmenuitems;
	typackedpopupdata info;
	Handle hpacked;
	Ptr p;
	
	*hpackeddata = nil;
	
	IOAclearbytes (&info, longsizeof (info));
	
	info.versionnumber = 1;
	
	info.haslabel = (**hdata).haslabel;
	
	IOAcopystring ((**hdata).bspopup, info.bspopup);
	
	info.checkeditem = (**hdata).checkeditem;
	
	info.popupwidth = (**hdata).popupwidth;
	
	if (hmenuitems == nil)
		lenmenuitems = 0;
	else
		lenmenuitems = GetHandleSize (hmenuitems);
		
	info.lenmenuitems = lenmenuitems;
		
	hpacked = NewHandle (longsizeof (info) + lenmenuitems);
	
	if (hpacked == nil)
		return (false);
	
	HLock (hpacked);
	
	p = *hpacked;
	
	BlockMove (&info, p, longsizeof (info));
	
	p += longsizeof (info);
	
	if (lenmenuitems > 0) 
		BlockMove (*hmenuitems, p, lenmenuitems);
	
	HUnlock (hpacked);
	
	*hpackeddata = hpacked;
	
	return (true);
	} /*packpopupdata*/
	
	
static boolean disposepopupdata (hdlobject h) {
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	
	disposehandle ((**hdata).hmenuitems);
	
	return (true);
	} /*disposepopupdata*/
	
	
static boolean unpackpopupdata (hdlobject h) {
	
	/*
	4/27/93 DW: has code to handle pre-1.0b15 popup records. after
	all are converted this code should be removed. look for "zzz" 
	comments.
	*/
	
	hdlpopupdata hdata;
	Handle hpackeddata;
	typackedpopupdata info;
	Handle hmenuitems;
	Ptr p;
		
	hpackeddata = (**h).objectdata; 
	
	if (!newpopupdata (&hdata)) 
		return (false);
		
	(**h).objectdata = (Handle) hdata;
	
	HLock (hpackeddata);
	
	p = *hpackeddata;
	
	BlockMove (p, &info, longsizeof (info));
	
	p += longsizeof (info);
	
	if (info.versionnumber != 1) { /*zzz*/
		
		Rect robject = (**h).objectrect;
		
		info.lenmenuitems = 0;
		
		info.haslabel = false;
		
		setstringlength (info.bspopup, 0);
		
		info.checkeditem = 1;
		
		info.popupwidth = robject.right - robject.left;
		}
	
	if (info.lenmenuitems > 0) 
		PtrToHand (p, &hmenuitems, info.lenmenuitems);
	else
		hmenuitems = nil;
		
	HUnlock (hpackeddata);
	
	(**hdata).hmenuitems = hmenuitems;
	
	(**hdata).haslabel = info.haslabel;

	IOAcopystring (info.bspopup, (**hdata).bspopup);

	(**hdata).checkeditem = info.checkeditem;

	(**hdata).popupwidth = info.popupwidth;
	
	disposehandle (hpackeddata);
	
	return (true);
	} /*unpackpopupdata*/
	
	
void setuppopup (tyioaconfigrecord *);


void setuppopup (tyioaconfigrecord *config) {

	IOAcopystring ("\pPopup menu", (*config).objectTypeName);
	
	(*config).objectTypeID = popuptype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = false;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).handlesMouseTrack = true; /*allow the popup manager to track the mouse*/
	
	(*config).hasSpecialCard = true;

	(*config).initObjectCallback = initpopup;
	
	(*config).drawObjectCallback = drawpopup;
	
	(*config).clickObjectCallback = clickpopup;
	
	(*config).recalcObjectCallback = recalcpopup;
	
	(*config).cleanupObjectCallback = cleanpopup;
	
	(*config).canReplicateObjectCallback = canreplicatepopup;
	
	(*config).getObjectEditRectCallback = getpopupeditrect;
	
	(*config).getValueForScriptCallback = getpopupvalue;
	
	(*config).debugObjectCallback = debugpopup;	
	
	(*config).getObjectInvalRectCallback = getpopupinvalrect;
	
	(*config).getAttributesCallback = getpopupattributes;
	
	(*config).setAttributesCallback = setpopupattributes;
	
	(*config).unpackDataCallback = unpackpopupdata;
	
	(*config).packDataCallback = packpopupdata;
	
	(*config).disposeDataCallback = disposepopupdata;
	} /*setuppopup*/
	
	
