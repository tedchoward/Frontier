
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
	7/31/96 dmb: fixed debugobjectcommandProcInfo
*/

#include "frontier.h"
#include "standard.h"

#include "ioa.h"
  
#ifdef THINK_C
	
	#include <SetUpA4.h>
	#define EnterCodeResource() do {RememberA0 ();SetUpA4();} while (0)
	#define ExitCodeResource()	RestoreA4()

#endif

#ifdef IOAinsideApp

	#include <appletmemory.h>
		
#endif


typedef pascal ComponentResult (*ComponentRoutine) (ComponentParameters *, Handle); // dmb 1/23/96

typedef void (*setupcallback) (tyioaconfigrecord *);
	
static ComponentInstance selfinstance = 0;

static short resfilenum = 0;

static hdlcard hcard = nil;

static tyioaconfigrecord config;



void IOAfillchar (void *pfill, long ctfill, char chfill) {
	
	/*
	do a mass memory fill -- copy ctfill chfills at pfill.
	*/
	
	char *p = pfill;
	long ct = ctfill;
	char ch = chfill;
	
	while (ct--) *p++ = (char) ch; /*tight loop*/
	} /*IOAfillchar*/
	

void IOAclearbytes (void *pclear, long ctclear) {
	
	/*
	fill memory with 0's.
	*/
	
	IOAfillchar (pclear, ctclear, (char) 0);
	} /*IOAclearbytes*/
	

short IOAmakemultiple (short val, short factor) {
	
	/*
	return a multiple of factor, round up in all cases -- that is, if
	it's even one more than the nearest multiple, return the next one.
	*/
	
	if ((val % factor) == 0) /*it's already a multiple, nothing to do*/
		return (val);
		
	val = ((val / factor) + 1) * factor;
	
	if (val == 0)
		val = factor;
		
	return (val);
	} /*IOAmakemultiple*/
	
	
short IOAclosestmultiple (short val, short factor) {
	
	/*
	return a multiple of factor, round to the nearest multiple.
	*/
	
	short x, diff;
	
	if ((val % factor) == 0) /*it's already a multiple, nothing to do*/
		return (val);
	
	x = (val / factor) * factor;
	
	diff = val - x;
	
	if (diff >= (factor / 2)) /*move to the next stop up*/
		x += factor;
		
	return (x);
	} /*IOAclosestmultiple*/
	

boolean IOAvisitobjects (hdlobject listhead, tyobjectvisitroutine visit) {
	
	/*
	visits all terminal nodes at all levels. we don't call the visit routine
	on group nodes.
	*/
	
	hdlobject nomad = listhead;
	hdlobject nextnomad;
	
	while (nomad != nil) {
		
		nextnomad = (**nomad).nextobject;
		
		if ((**nomad).objecttype == grouptype) {
		
			if (!IOAvisitobjects ((**nomad).childobjectlist, visit))
				return (false);
			}
			
		if (!(*visit) (nomad))
			return (false);
			
		nomad = nextnomad;
		} /*while*/
	
	return (true);
	} /*IOAvisitobjects*/
	
	
void IOAcopystring (bssource, bsdest) void *bssource, *bsdest; {

	/*
	create a copy of bssource in bsdest.  copy the length byte and
	all the characters in the source string.

	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/

	short i, len;
	
	len = (short) ((char *) bssource) [0];
	
	for (i = 0; i <= len; i++) 
		((char *) bsdest) [i] = ((char *) bssource) [i];
	} /*IOAcopystring*/


boolean IOApushstring (bssource, bsdest) bigstring bssource, bsdest; {

	/*
	insert the source string at the end of the destination string.
	
	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	unsigned short lensource = stringlength (bssource);
	unsigned short lendest = stringlength (bsdest);
	char *psource, *pdest;
	
	if ((lensource + lendest) > lenbigstring)
		return (false);
		
	pdest = (ptrchar) bsdest + (unsigned char) lendest + 1;
	
	psource = (ptrchar) bssource + 1;
	
	bsdest [0] += (unsigned char) lensource;
	
	while (lensource--) *pdest++ = *psource++;
	
	return (true);
	} /*IOApushstring*/
	
	
void IOAtexthandletostring (Handle htext, bigstring bs) {
	
	long len;
	
	if (htext == nil) {
		
		setstringlength (bs, 0);
		
		return;
		}
	
	len = GetHandleSize (htext);
	
	if (len > lenbigstring)
		len = lenbigstring;
	
	setstringlength (bs, len);
	
	BlockMove (*htext, &bs [1], len);
	} /*IOAtexthandletostring*/
	
	
boolean IOAnewtexthandle (bigstring bs, Handle *htext) {
	
	/*
	create a new handle to hold the text of the string.
	
	if the string is "\pABC" -- you get a handle of size 3.
	*/
	
	long len;
	Handle h;
	
	h = NewHandle (len = stringlength (bs));
	
	if (h == nil)
		return (false);
	
	if (len > 0)
		BlockMove (&bs [1], *h, len);
	
	*htext = h; /*pass handle back to caller*/
	
	return (true);
	} /*IOAnewtexthandle*/


void IOAellipsize (Str255 bs, short width) {

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
	} /*IOAellipsize*/
	

void IOAcenterstring (Rect r, bigstring bs) {
	
	/*
	draw the string in the current font, size and style, centered inside
	the indicated rectangle.
	*/
	
	short lh; 
	short rh = r.bottom - r.top;
	short rw = r.right - r.left;
	short h, v;
	RgnHandle rgn;
	Rect rorigclip;
	FontInfo fi;
	
	GetFontInfo (&fi);
	
	lh = fi.ascent + fi.descent;
	
	IOAellipsize (bs, rw); /*make sure it fits inside the rectangle, width-wise*/
	
	h = r.left + ((rw - StringWidth (bs)) / 2);
	
	v = r.top + ((rh - lh) / 2) + fi.ascent;
	
	MoveTo (h, v);
	
	GetClip (rgn = NewRgn ());
	//Code change by Timothy Paustian Sunday, May 7, 2000 9:11:46 PM
	//Changed to Opaque call for Carbon
	//This is a simple one, but untested.
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	GetRegionBounds(rgn, &rorigclip);
	#else
	rorigclip = (**rgn).rgnBBox;
	#endif
	DisposeRgn (rgn);

	ClipRect (&r);
	
	DrawString (bs);
	
	ClipRect (&rorigclip);
	} /*IOAcenterstring*/


void IOAcenterrect (Rect *rcentered, Rect rcontains) {
	
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
	} /*IOAcenterrect*/


void IOAgetobjectsize (hdlobject h, short *height, short *width) {
	
	(*(**hcard).IOAgetobjectsizeCallback) (h, height, width);
	} /*IOAgetobjectsize*/
	

boolean IOAgetstringvalue (hdlobject h, Handle *hvalue) {
	
	return ((*(**hcard).IOAgetstringvalueCallback) (h, hvalue));
	} /*IOAgetstringvalue*/


boolean IOApushforecolor (RGBColor *rgb) {
	
	return ((*(**hcard).IOApushforecolorCallback) (rgb));
	} /*IOApushforecolor*/


boolean IOApopforecolor (void) {
	
	return ((*(**hcard).IOApopforecolorCallback) ());
	} /*IOApopforecolor*/


boolean IOApushbackcolor (RGBColor *rgb) {
	
	return ((*(**hcard).IOApushbackcolorCallback) (rgb));
	} /*IOApushbackcolor*/


boolean IOApopbackcolor (void) {
	
	return ((*(**hcard).IOApopbackcolorCallback) ());
	} /*IOApopbackcolor*/


boolean IOArunbuttonscript (hdlobject h) {
	
	return ((*(**hcard).IOArunbuttonscriptCallback) (h));
	} /*IOArunbuttonscript*/
	

boolean IOAgetbooleanvalue (hdlobject h, Handle *hvalue) {
	
	return ((*(**hcard).IOAgetbooleanvalueCallback) (h, hvalue));
	} /*IOAgetbooleanvalue*/
	

boolean IOAsetbooleanvalue (hdlobject h, Handle hvalue) {
	
	return ((*(**hcard).IOAsetbooleanvalueCallback) (h, hvalue));
	} /*IOAsetbooleanvalue*/
	

void IOAeditdrawtexthandle (Handle htext, Rect r, tyjustification j) {
	
	(*(**hcard).IOAeditdrawtexthandleCallback) (htext, r, j);
	} /*IOAeditdrawtexthandle*/
	

boolean IOArecalcobjectvalue (hdlobject h) {
	
	return ((*(**hcard).IOArecalcobjectvalueCallback) (h));
	} /*IOArecalcobjectvalue*/
	

boolean IOAevalscript (hdlobject hself, Handle hscript, Handle *hvalue, OSType idlanguage, bigstring errorstring) {
	
	return ((*(**hcard).IOAevalscriptCallback) (hself, hscript, hvalue, idlanguage, errorstring));
	} /*IOAevalscript*/
	

boolean IOAsetobjectvalue (hdlobject h, Handle hvalue) {
	
	return ((*(**hcard).IOAsetobjectvalueCallback) (h, hvalue));
	} /*IOAsetobjectvalue*/
	
	
boolean IOAgetnthstring (hdlobject h, short n, bigstring bs) {

	return ((*(**hcard).IOAgetnthstringCallback) (h, n, bs));
	} /*IOAgetnthstring*/
	

void IOAinvalobject (hdlobject h) {
	
	(*(**hcard).IOAinvalobjectCallback) (h);
	} /*IOAinvalobject*/
	

boolean IOAclearactivetextobject (void) {
	
	return ((*(**hcard).IOAclearactivetextobjectCallback) ());
	} /*IOAclearactivetextobject*/
	

boolean IOAsetactivetextobject (hdlobject h) {
	
	return ((*(**hcard).IOAsetactivetextobjectCallback) (h));
	} /*IOAsetactivetextobject*/
	

boolean IOAeditclick (Point pt, boolean fl, Handle h) {
	
	return ((*(**hcard).IOAeditclickCallback) (pt, fl, h));
	} /*IOAeditclick*/
	

boolean IOAeditselectall (Handle h) {
	
	return ((*(**hcard).IOAeditselectallCallback) (h));
	} /*IOAeditselectall*/
	

boolean IOAeditidle (Handle h) {
	
	return ((*(**hcard).IOAeditidleCallback) (h));
	} /*IOAeditidle*/
	

boolean IOAeditkeystroke (char ch, Handle h) {
	
	return ((*(**hcard).IOAeditkeystrokeCallback) (ch, h));
	} /*IOAeditkeystroke*/
	

Handle IOAgetactiveeditrecord (void) {
	
	return ((*(**hcard).IOAgetactiveeditrecordCallback) ());
	} /*IOAgetactiveeditrecord*/
	
	
boolean IOAincolorwindow (hdlobject h) {
	
	return (true); /*need to fill this in*/
	} /*IOAincolorwindow*/
	

boolean IOAopenresfile (void) {

	resfilenum = OpenComponentResFile ((Component) selfinstance);
	
	return (resfilenum != 0);
	} /*IOAopenresfile*/
	
	
void IOAcloseresfile (void) {
	
	if (resfilenum != 0)
		CloseComponentResFile (resfilenum);
	
	resfilenum = 0;
	} /*IOAcloseresfile*/
	

static boolean defaultGetObjectInvalRectCallback (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	return (true);
	} /*defaultGetObjectInvalRectCallback*/
	

static boolean defaultDebugObjectCallback (hdlobject h, bigstring errorstring) {
	
	IOAcopystring ("\pbad object type", errorstring);
	
	return (false);
	} /*defaultDebugObjectCallback*/
	
	
static boolean defaultRecalcObjectCallback (hdlobject h, boolean flmajorrecalc) {
	
	return (true);
	} /*defaultRecalcObjectCallback*/
	

static boolean defaultClickObjectCallback (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	return (false); /*don't do a minor recalc*/
	} /*defaultClickObjectCallback*/
	

static boolean defaultSetObjectCursorCallback (hdlobject h, Point pt) {
	
	return (false); /*we accept the default cursor*/
	} /*defaultSetObjectCursorCallback*/
	
	
static boolean defaultCatchReturnCallback (hdlobject h) {
	
	return (false); /*we don't want to intercept the Return key*/
	} /*defaultCatchReturnCallback*/
	
	
static boolean defaultUnpackDataCallback (hdlobject h) {
	
	return (true); /*do nothing*/
	} /*defaultUnpackDataCallback*/
	
	
static boolean defaultAppleEventCallback (hdlobject h, AppleEvent *event) {
	
	return (true); /*do nothing*/
	} /*defaultAppleEventCallback*/
	
	
static boolean defaultPackDataCallback (hdlobject h, Handle *hpackeddata) {
	
	*hpackeddata = (**h).objectdata; /*return a copy of the objectdata handle*/
	
	if (*hpackeddata != nil)
		HandToHand (hpackeddata);
	
	return (true);
	} /*defaultPackDataCallback*/
	
	
static boolean defaultDisposeDataCallback (hdlobject h) {

	return (true);
	} /*defaultDisposeDataCallback*/
	
	
static void callsetuproutine (setupcallback callback, tyioaconfigrecord *config) {
	
	IOAclearbytes (config, longsizeof (tyioaconfigrecord));
	
	(*config).setValueFromScriptCallback = IOAsetobjectvalue;
	
	(*config).getObjectInvalRectCallback = defaultGetObjectInvalRectCallback;
	
	(*config).recalcObjectCallback = defaultRecalcObjectCallback;
	
	(*config).debugObjectCallback = defaultDebugObjectCallback;
	
	(*config).clickObjectCallback = defaultClickObjectCallback;
	
	(*config).setObjectCursorCallback = defaultSetObjectCursorCallback;
	
	(*config).catchReturnCallback = defaultCatchReturnCallback;
	
	(*config).getAttributesCallback = defaultAppleEventCallback;
	
	(*config).setAttributesCallback = defaultAppleEventCallback;
	
	(*config).packDataCallback = defaultPackDataCallback;
	
	(*config).unpackDataCallback = defaultUnpackDataCallback;
	
	(*config).disposeDataCallback = defaultDisposeDataCallback;
	
	(*config).handlesMouseTrack = false;
	
	(*config).editableInRunMode = false;
	
	(*config).isFontAware = true;
	
	(*callback) (config);
	} /*callsetuproutine*/


void name_of_IOA_setup_routine (tyioaconfigrecord *); /*forward declaration*/
	
	
static pascal ComponentResult IOAopen (ComponentInstance self) {
	
	selfinstance = self; /*copy into global*/
	
	#ifndef IOAinsideApp
	
		callsetuproutine (name_of_IOA_setup_routine, &config);
		
	#endif
	
	return (noErr);
	} /*IOAopen*/


static pascal ComponentResult IOAclose (ComponentInstance self) {
	
	return (noErr);
	} /*IOAclose*/


static pascal ComponentResult IOAcando (short selector) {

	switch (selector) {
	
		case kComponentOpenSelect: 
		case kComponentCloseSelect:
		case kComponentCanDoSelect: 
		case kComponentVersionSelect:
		
		case IOAgetconfigcommand:
		case IOAinitobjectcommand:
		case IOAdrawobjectcommand:
		case IOAeditobjectcommand:
		case IOAkeystrokecommand:
		case IOAidlecommand:
		case IOAclickcommand:
		case IOAcleanupcommand:
		case IOArecalccommand:
		case IOAcanreplicatcommand:
		case IOAcatchreturncommand:
		case IOAgetobjectinvalrectcommand:
		case IOAgetobjecteditrectcommand:
		case IOAgetvaluecommand:
		case IOAsetvaluecommand:
		case IOAsetcursorcommand:
		case IOAunpackdatacommand:
		case IOAdebugobjectcommand:
		case IOAgetattributescommand:
		case IOAsetattributescommand:
		case IOAgeticoncommand:
		case IOAgetcardcommand:
		case IOApackdatacommand:
		case IOAdisposedatacommand:
		
			return (true);
		
		} /*switch*/
		
	return (false);
	} /*IOAcando*/


static pascal ComponentResult IOAversion (void) {

	return (1);
	} /*IOAversion*/


static pascal ComponentResult getconfigcommand (tyioaconfigrecord *stackedconfig) {
	
	BlockMove (&config, stackedconfig, longsizeof (tyioaconfigrecord));

	return (true);
	} /*getconfigcommand*/


static pascal ComponentResult initobjectcommand (tyobject *obj) {
	
	return ((*config.initObjectCallback) (obj));
	} /*initobjectcommand*/


static pascal ComponentResult drawobjectcommand (hdlobject h) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.drawObjectCallback) (h));
	} /*drawobjectcommand*/


static pascal ComponentResult editobjectcommand (hdlobject h, boolean flgoin) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.editObjectCallback) (h, flgoin));
	} /*editobjectcommand*/


static pascal ComponentResult keystrokecommand (hdlobject h, short ch) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.keystrokeObjectCallback) (h, (char) ch));
	} /*keystrokecommand*/


static pascal ComponentResult idleobjectcommand (hdlobject h) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.idleObjectCallback) (h));
	} /*idleobjectcommand*/


static pascal ComponentResult clickobjectcommand (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.clickObjectCallback) (listhead, h, pt, flshiftkey, fl2click));
	} /*clickobjectcommand*/


static pascal ComponentResult cleanupobjectcommand (hdlobject h, short height, short width, Rect *r) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.cleanupObjectCallback) (h, height, width, r));
	} /*cleanupobjectcommand*/


static pascal ComponentResult recalcobjectcommand (hdlobject h, boolean flmajorrecalc) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.recalcObjectCallback) (h, flmajorrecalc));
	} /*recalcobjectcommand*/


static pascal ComponentResult canreplicateobjectcommand (hdlobject h) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.canReplicateObjectCallback) (h));
	} /*canreplicateobjectcommand*/


static pascal ComponentResult catchreturncommand (hdlobject h) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.catchReturnCallback) (h));
	} /*catchreturncommand*/


static pascal ComponentResult getinvalrectcommand (hdlobject h, Rect *r) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.getObjectInvalRectCallback) (h, r));
	} /*getinvalrectcommand*/


static pascal ComponentResult geteditrectcommand (hdlobject h, Rect *r) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.getObjectEditRectCallback) (h, r));
	} /*geteditrectcommand*/


static pascal ComponentResult getvaluecommand (hdlobject h, Handle *hvalue) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.getValueForScriptCallback) (h, hvalue));
	} /*getvaluecommand*/


static pascal ComponentResult setvaluecommand (hdlobject h, Handle hvalue) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.setValueFromScriptCallback) (h, hvalue));
	} /*setvaluecommand*/


static pascal ComponentResult setcursorcommand (hdlobject h, Point pt) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.setObjectCursorCallback) (h, pt));
	} /*setcursorcommand*/


static pascal ComponentResult unpackdatacommand (hdlobject h) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.unpackDataCallback) (h));
	} /*unpackdatacommand*/


static pascal ComponentResult disposedatacommand (hdlobject h) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.disposeDataCallback) (h));
	} /*disposedatacommand*/


static pascal ComponentResult getattributescommand (hdlobject h, AppleEvent *event) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.getAttributesCallback) (h, event));
	} /*getattributescommand*/


static pascal ComponentResult setattributescommand (hdlobject h, AppleEvent *event) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.setAttributesCallback) (h, event));
	} /*setattributescommand*/


static pascal ComponentResult geticoncommand (Handle *hicon) {
	
	Handle h;
	
	#ifdef IOAinsideApp
	
		h = GetResource ('SICN', 512 + config.objectTypeID);
		
		DetachResource (h);
	
	#else
	
		if (!IOAopenresfile ())
			return (false);
	
		h = GetResource ('SICN', 130);
		
		DetachResource (h);
	
		IOAcloseresfile ();
	
	#endif
	
	*hicon = h;
	
	return (true);
	} /*geticoncommand*/
	


static pascal ComponentResult getcardcommand (Handle *hpackedcard) {
	
	Handle h;
	
	#ifdef IOAinsideApp
		h = GetResource ('CARD', 512 + config.objectTypeID);
		
		DetachResource (h);
	#else
		if (!IOAopenresfile ())
			return (false);
	
		h = GetResource ('CARD', 128);
		
		DetachResource (h);
	
		IOAcloseresfile ();
	#endif

	*hpackedcard = h;
	
	return (true);
	} /*getcardcommand*/


static pascal ComponentResult packdatacommand (hdlobject h, Handle *hpackeddata) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.packDataCallback) (h, hpackeddata));
	} /*packdatacommand*/


static pascal ComponentResult debugobjectcommand (hdlobject h, Str255 errorstring) {
	
	hcard = (**h).owningcard; /*set global for callbacks*/
	
	return ((*config.debugObjectCallback) (h, errorstring));
	} /*debugobjectcommand*/

//Code change by Timothy Paustian Sunday, June 25, 2000 10:29:27 PM
//watch out, will this work?, I may have to check this.
#if 0
	
	// UPPs are just the routine addresses
	#define IOAopenUPP ((ComponentFunctionUPP) &IOAopen)
	#define IOAcloseUPP ((ComponentFunctionUPP) &IOAclose)
	#define IOAcandoUPP ((ComponentFunctionUPP) &IOAcando)
	#define IOAversionUPP ((ComponentFunctionUPP) &IOAversion)
	#define getconfigcommandUPP ((ComponentFunctionUPP) &getconfigcommand)
	#define initobjectcommandUPP ((ComponentFunctionUPP) &initobjectcommand)
	#define drawobjectcommandUPP ((ComponentFunctionUPP) &drawobjectcommand)
	#define editobjectcommandUPP ((ComponentFunctionUPP) &editobjectcommand)
	#define keystrokecommandUPP ((ComponentFunctionUPP) &keystrokecommand)
	#define idleobjectcommandUPP ((ComponentFunctionUPP) &idleobjectcommand)
	#define clickobjectcommandUPP ((ComponentFunctionUPP) &clickobjectcommand)
	#define cleanupobjectcommandUPP ((ComponentFunctionUPP) &cleanupobjectcommand)
	#define recalcobjectcommandUPP ((ComponentFunctionUPP) &recalcobjectcommand)
	#define canreplicateobjectcommandUPP ((ComponentFunctionUPP) &canreplicateobjectcommand)
	#define catchreturncommandUPP ((ComponentFunctionUPP) &catchreturncommand)
	#define getinvalrectcommandUPP ((ComponentFunctionUPP) &getinvalrectcommand)
	#define geteditrectcommandUPP ((ComponentFunctionUPP) &geteditrectcommand)
	#define getvaluecommandUPP ((ComponentFunctionUPP) &getvaluecommand)
	#define setvaluecommandUPP ((ComponentFunctionUPP) &setvaluecommand)
	#define setcursorcommandUPP ((ComponentFunctionUPP) &setcursorcommand)
	#define unpackdatacommandUPP ((ComponentFunctionUPP) &unpackdatacommand)
	#define disposedatacommandUPP ((ComponentFunctionUPP) &disposedatacommand)
	#define getattributescommandUPP ((ComponentFunctionUPP) &getattributescommand)
	#define setattributescommandUPP ((ComponentFunctionUPP) &setattributescommand)
	#define geticoncommandUPP ((ComponentFunctionUPP) &geticoncommand)
	#define getcardcommandUPP ((ComponentFunctionUPP) &getcardcommand)
	#define packdatacommandUPP ((ComponentFunctionUPP) &packdatacommand)
	#define debugobjectcommandUPP ((ComponentFunctionUPP) &debugobjectcommand)

#else

	// define all of the ProcInfos
	enum {
		IOAopenProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ComponentInstance)))
		};
	
	enum {
		IOAcloseProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ComponentInstance)))
		};
	
	enum {
		IOAcandoProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		};
	
	enum {
		IOAversionProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		};
	
	enum {
		getconfigcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(tyioaconfigrecord *)))
		};
	
	enum {
		initobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(tyobject *)))
		};
	
	enum {
		drawobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
		};
	
	enum {
		editobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(boolean)))
		};
	
	enum {
		keystrokecommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		};
	
	enum {
		idleobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
		};
	
	enum {
		clickobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Point)))
			| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(boolean)))
			| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(boolean)))
		};
	
	enum {
		cleanupobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
			| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Rect *)))
		};
	
	enum {
		recalcobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(boolean)))
		};
	
	enum {
		canreplicateobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
		};
	
	enum {
		catchreturncommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
		};
	
	enum {
		getinvalrectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect *)))
		};
	
	enum {
		geteditrectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect *)))
		};
	
	enum {
		getvaluecommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Handle *)))
		};
	
	enum {
		setvaluecommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Handle)))
		};
	
	enum {
		setcursorcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Point)))
		};
	
	enum {
		unpackdatacommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
		};
	
	enum {
		disposedatacommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
		};
	
	enum {
		getattributescommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AppleEvent *)))
		};
	
	enum {
		setattributescommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AppleEvent *)))
		};
	
	enum {
		geticoncommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle *)))
		};
	
	enum {
		getcardcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle *)))
		};
	
	enum {
		packdatacommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Handle *)))
		};
	
	enum {
		debugobjectcommandProcInfo = kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlobject)))
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(StringPtr)))
		};
	
		
	#if !TARGET_API_MAC_CARBON
	static RoutineDescriptor IOAopenDesc = BUILD_ROUTINE_DESCRIPTOR (IOAopenProcInfo, IOAopen);
	static RoutineDescriptor IOAcloseDesc = BUILD_ROUTINE_DESCRIPTOR (IOAcloseProcInfo, IOAclose);
	static RoutineDescriptor IOAcandoDesc = BUILD_ROUTINE_DESCRIPTOR (IOAcandoProcInfo, IOAcando);
	static RoutineDescriptor IOAversionDesc = BUILD_ROUTINE_DESCRIPTOR (IOAversionProcInfo, IOAversion);
	static RoutineDescriptor getconfigcommandDesc = BUILD_ROUTINE_DESCRIPTOR (getconfigcommandProcInfo, getconfigcommand);
	static RoutineDescriptor initobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (initobjectcommandProcInfo, initobjectcommand);
	static RoutineDescriptor drawobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (drawobjectcommandProcInfo, drawobjectcommand);
	static RoutineDescriptor editobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (editobjectcommandProcInfo, editobjectcommand);
	static RoutineDescriptor keystrokecommandDesc = BUILD_ROUTINE_DESCRIPTOR (keystrokecommandProcInfo, keystrokecommand);
	static RoutineDescriptor idleobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (idleobjectcommandProcInfo, idleobjectcommand);
	static RoutineDescriptor clickobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (clickobjectcommandProcInfo, clickobjectcommand);
	static RoutineDescriptor cleanupobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (cleanupobjectcommandProcInfo, cleanupobjectcommand);
	static RoutineDescriptor recalcobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (recalcobjectcommandProcInfo, recalcobjectcommand);
	static RoutineDescriptor canreplicateobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (canreplicateobjectcommandProcInfo, canreplicateobjectcommand);
	static RoutineDescriptor catchreturncommandDesc = BUILD_ROUTINE_DESCRIPTOR (catchreturncommandProcInfo, catchreturncommand);
	static RoutineDescriptor getinvalrectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (getinvalrectcommandProcInfo, getinvalrectcommand);
	static RoutineDescriptor geteditrectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (geteditrectcommandProcInfo, geteditrectcommand);
	static RoutineDescriptor getvaluecommandDesc = BUILD_ROUTINE_DESCRIPTOR (getvaluecommandProcInfo, getvaluecommand);
	static RoutineDescriptor setvaluecommandDesc = BUILD_ROUTINE_DESCRIPTOR (setvaluecommandProcInfo, setvaluecommand);
	static RoutineDescriptor setcursorcommandDesc = BUILD_ROUTINE_DESCRIPTOR (setcursorcommandProcInfo, setcursorcommand);
	static RoutineDescriptor unpackdatacommandDesc = BUILD_ROUTINE_DESCRIPTOR (unpackdatacommandProcInfo, unpackdatacommand);
	static RoutineDescriptor disposedatacommandDesc = BUILD_ROUTINE_DESCRIPTOR (disposedatacommandProcInfo, disposedatacommand);
	static RoutineDescriptor getattributescommandDesc = BUILD_ROUTINE_DESCRIPTOR (getattributescommandProcInfo, getattributescommand);
	static RoutineDescriptor setattributescommandDesc = BUILD_ROUTINE_DESCRIPTOR (setattributescommandProcInfo, setattributescommand);
	static RoutineDescriptor geticoncommandDesc = BUILD_ROUTINE_DESCRIPTOR (geticoncommandProcInfo, geticoncommand);
	static RoutineDescriptor getcardcommandDesc = BUILD_ROUTINE_DESCRIPTOR (getcardcommandProcInfo, getcardcommand);
	static RoutineDescriptor packdatacommandDesc = BUILD_ROUTINE_DESCRIPTOR (packdatacommandProcInfo, packdatacommand);
	static RoutineDescriptor debugobjectcommandDesc = BUILD_ROUTINE_DESCRIPTOR (debugobjectcommandProcInfo, debugobjectcommand);
	
	// UPPs are descriptor addresses
	#define IOAopenUPP (&IOAopenDesc)
	#define IOAcloseUPP (&IOAcloseDesc)
	#define IOAcandoUPP (&IOAcandoDesc)
	#define IOAversionUPP (&IOAversionDesc)
	#define getconfigcommandUPP (&getconfigcommandDesc)
	#define initobjectcommandUPP (&initobjectcommandDesc)
	#define drawobjectcommandUPP (&drawobjectcommandDesc)
	#define editobjectcommandUPP (&editobjectcommandDesc)
	#define keystrokecommandUPP (&keystrokecommandDesc)
	#define idleobjectcommandUPP (&idleobjectcommandDesc)
	#define clickobjectcommandUPP (&clickobjectcommandDesc)
	#define cleanupobjectcommandUPP (&cleanupobjectcommandDesc)
	#define recalcobjectcommandUPP (&recalcobjectcommandDesc)
	#define canreplicateobjectcommandUPP (&canreplicateobjectcommandDesc)
	#define catchreturncommandUPP (&catchreturncommandDesc)
	#define getinvalrectcommandUPP (&getinvalrectcommandDesc)
	#define geteditrectcommandUPP (&geteditrectcommandDesc)
	#define getvaluecommandUPP (&getvaluecommandDesc)
	#define setvaluecommandUPP (&setvaluecommandDesc)
	#define setcursorcommandUPP (&setcursorcommandDesc)
	#define unpackdatacommandUPP (&unpackdatacommandDesc)
	#define disposedatacommandUPP (&disposedatacommandDesc)
	#define getattributescommandUPP (&getattributescommandDesc)
	#define setattributescommandUPP (&setattributescommandDesc)
	#define geticoncommandUPP (&geticoncommandDesc)
	#define getcardcommandUPP (&getcardcommandDesc)
	#define packdatacommandUPP (&packdatacommandDesc)
	#define debugobjectcommandUPP (&debugobjectcommandDesc)
	
	#else //TARGET_API_MAC_CARBON
	/*ComponentRoutineUPP	IOAopenDesc;
	ComponentRoutineUPP IOAcloseDesc;
	ComponentRoutineUPP IOAcandoDesc;
	ComponentRoutineUPP IOAversionDesc;
	ComponentRoutineUPP getconfigcommandDesc;
	ComponentRoutineUPP initobjectcommandDesc;
	ComponentRoutineUPP drawobjectcommandDesc;
	ComponentRoutineUPP editobjectcommandDesc;
	ComponentRoutineUPP keystrokecommandDesc;
	ComponentRoutineUPP idleobjectcommandDesc;
	ComponentRoutineUPP clickobjectcommandDesc;
	ComponentRoutineUPP cleanupobjectcommandDesc;
	ComponentRoutineUPP recalcobjectcommandDesc;
	ComponentRoutineUPP canreplicateobjectcommandDesc;
	ComponentRoutineUPP catchreturncommandDesc;
	ComponentRoutineUPP getinvalrectcommandDesc;
	ComponentRoutineUPP geteditrectcommandDesc;
	ComponentRoutineUPP getvaluecommandDesc;
	ComponentRoutineUPP setvaluecommandDesc;
	ComponentRoutineUPP setcursorcommandDesc;
	ComponentRoutineUPP unpackdatacommandDesc;
	ComponentRoutineUPP disposedatacommandDesc;
	ComponentRoutineUPP getattributescommandDesc;
	ComponentRoutineUPP setattributescommandDesc;
	ComponentRoutineUPP geticoncommandDesc;
	ComponentRoutineUPP getcardcommandDesc;
	ComponentRoutineUPP packdatacommandDesc;
	ComponentRoutineUPP debugobjectcommandDesc;
	
	// UPPs are descriptor addresses
	#define IOAopenUPP (IOAopenDesc)
	#define IOAcloseUPP (IOAcloseDesc)
	#define IOAcandoUPP (IOAcandoDesc)
	#define IOAversionUPP (IOAversionDesc)
	#define getconfigcommandUPP (getconfigcommandDesc)
	#define initobjectcommandUPP (initobjectcommandDesc)
	#define drawobjectcommandUPP (drawobjectcommandDesc)
	#define editobjectcommandUPP (editobjectcommandDesc)
	#define keystrokecommandUPP (keystrokecommandDesc)
	#define idleobjectcommandUPP (idleobjectcommandDesc)
	#define clickobjectcommandUPP (clickobjectcommandDesc)
	#define cleanupobjectcommandUPP (cleanupobjectcommandDesc)
	#define recalcobjectcommandUPP (recalcobjectcommandDesc)
	#define canreplicateobjectcommandUPP (canreplicateobjectcommandDesc)
	#define catchreturncommandUPP (catchreturncommandDesc)
	#define getinvalrectcommandUPP (getinvalrectcommandDesc)
	#define geteditrectcommandUPP (geteditrectcommandDesc)
	#define getvaluecommandUPP (getvaluecommandDesc)
	#define setvaluecommandUPP (setvaluecommandDesc)
	#define setcursorcommandUPP (setcursorcommandDesc)
	#define unpackdatacommandUPP (unpackdatacommandDesc)
	#define disposedatacommandUPP (disposedatacommandDesc)
	#define getattributescommandUPP (getattributescommandDesc)
	#define setattributescommandUPP (setattributescommandDesc)
	#define geticoncommandUPP (geticoncommandDesc)
	#define getcardcommandUPP (getcardcommandDesc)
	#define packdatacommandUPP (packdatacommandDesc)
	#define debugobjectcommandUPP (debugobjectcommandDesc)*/
	#endif
	

#endif

#ifndef IOAinsideApp

	#if __powerc
	
	/*
		on the powerPC, our main entry point is a actually a routine descriptor, 
		pointing to our native main code. the global __procinfo is used by the 
		MWC startup code to build the descriptor.
	*/
		ProcInfoType __procinfo = uppComponentRoutineProcInfo;
		
		void __sinit (void);
	
	#endif
	
	pascal ComponentResult main (ComponentParameters *params, Handle hstorage) {

#else

	//Code change by Timothy Paustian Saturday, July 8, 2000 8:23:29 AM
	//I changed this to plain pascal instead of static. Will this shoot things?
static	pascal ComponentResult IOAmain (ComponentParameters *params, Handle hstorage) {
	
#endif
	
	ComponentResult result;
	
	#ifndef IOAinsideApp
	
		#if _powerc
		
			__sinit ();		/*initialize static data*/
		
		#endif
	
		EnterCodeResource ();
	
	#endif
	
	result = noErr;
	
	switch ((*params).what) {
		
		case kComponentOpenSelect:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP IOAopenUPP = NewComponentFunctionUPP(IOAopen, IOAopenProcInfo);
			result = CallComponentFunction(params, IOAopenUPP);
			DisposeComponentFunctionUPP(IOAopenUPP);
			}
			#else
			result = CallComponentFunction(params, IOAopenUPP);
			#endif
			break;
			
		case kComponentCloseSelect:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP IOAcloseUPP  = NewComponentFunctionUPP(IOAclose, IOAcloseProcInfo);
			result = CallComponentFunction(params, IOAcloseUPP);
			DisposeComponentFunctionUPP(IOAcloseUPP);
			}
			#else
			result = CallComponentFunction(params, IOAcloseUPP);
			#endif
			
			break;
			
		case kComponentCanDoSelect:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP IOAcandoUPP  = NewComponentFunctionUPP(IOAcando, IOAcandoProcInfo);
			result = CallComponentFunction(params, IOAcandoUPP);
			DisposeComponentFunctionUPP(IOAcandoUPP);
			}
			#else
			result = CallComponentFunction(params, IOAcandoUPP);
			#endif
			break;
			
		case kComponentVersionSelect:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP IOAversionUPP  = NewComponentFunctionUPP(IOAversion, IOAversionProcInfo);
			result = CallComponentFunction(params, IOAversionUPP);
			DisposeComponentFunctionUPP(IOAversionUPP);
			}
			#else
			result = CallComponentFunction(params, IOAversionUPP);
			#endif
			break;
			
		case IOAgetconfigcommand:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP getconfigcommandUPP  = NewComponentFunctionUPP(getconfigcommand, getconfigcommandProcInfo);
			result = CallComponentFunction(params, getconfigcommandUPP);
			DisposeComponentFunctionUPP(getconfigcommandUPP);
			}
			#else
			result = CallComponentFunction(params, getconfigcommandUPP);
			#endif
			break;
			
		case IOAinitobjectcommand:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP initobjectcommandUPP  = NewComponentFunctionUPP(initobjectcommand, initobjectcommandProcInfo);
			result = CallComponentFunction(params, initobjectcommandUPP);
			DisposeComponentFunctionUPP(initobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, initobjectcommandUPP);
			#endif
			break;
			
		case IOAdrawobjectcommand:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP drawobjectcommandUPP  = NewComponentFunctionUPP(drawobjectcommand, drawobjectcommandProcInfo);
			result = CallComponentFunction(params, drawobjectcommandUPP);
			DisposeComponentFunctionUPP(drawobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, drawobjectcommandUPP);
			#endif
			break;
			
		case IOAeditobjectcommand:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP editobjectcommandUPP  = NewComponentFunctionUPP(editobjectcommand, editobjectcommandProcInfo);
			result = CallComponentFunction(params, editobjectcommandUPP);
			DisposeComponentFunctionUPP(editobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, editobjectcommandUPP);
			#endif
			break;
			
		case IOAkeystrokecommand:
			
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP keystrokecommandUPP  = NewComponentFunctionUPP(keystrokecommand, keystrokecommandProcInfo);
			result = CallComponentFunction(params, keystrokecommandUPP);
			DisposeComponentFunctionUPP(keystrokecommandUPP);
			}
			#else
			result = CallComponentFunction(params, keystrokecommandUPP);
			#endif
			break;
			
		case IOAidlecommand:
		
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP idleobjectcommandUPP  = NewComponentFunctionUPP(idleobjectcommand, idleobjectcommandProcInfo);
			result = CallComponentFunction(params, idleobjectcommandUPP);
			DisposeComponentFunctionUPP(idleobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, idleobjectcommandUPP);
			#endif
			break;
			
		case IOAclickcommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP clickobjectcommandUPP  = NewComponentFunctionUPP(clickobjectcommand, clickobjectcommandProcInfo);
			result = CallComponentFunction(params, clickobjectcommandUPP);
			DisposeComponentFunctionUPP(clickobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, clickobjectcommandUPP);
			#endif
			break;
			
		case IOAcleanupcommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP cleanupobjectcommandUPP  = NewComponentFunctionUPP(cleanupobjectcommand, cleanupobjectcommandProcInfo);
			result = CallComponentFunction(params, cleanupobjectcommandUPP);
			DisposeComponentFunctionUPP(cleanupobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, cleanupobjectcommandUPP);
			#endif
			break;
			
		case IOArecalccommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP recalcobjectcommandUPP  = NewComponentFunctionUPP(recalcobjectcommand, recalcobjectcommandProcInfo);
			result = CallComponentFunction(params, recalcobjectcommandUPP);
			DisposeComponentFunctionUPP(recalcobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, recalcobjectcommandUPP);
			#endif
			break;
			
		case IOAcanreplicatcommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP canreplicateobjectcommandUPP  = NewComponentFunctionUPP(canreplicateobjectcommand, canreplicateobjectcommandProcInfo);
			result = CallComponentFunction(params, canreplicateobjectcommandUPP);
			DisposeComponentFunctionUPP(canreplicateobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, canreplicateobjectcommandUPP);
			#endif
			break;
			
		case IOAcatchreturncommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP catchreturncommandUPP  = NewComponentFunctionUPP(catchreturncommand, catchreturncommandProcInfo);
			result = CallComponentFunction(params, catchreturncommandUPP);
			DisposeComponentFunctionUPP(catchreturncommandUPP);
			}
			#else
			result = CallComponentFunction(params, catchreturncommandUPP);
			#endif
			break;
			
		case IOAgetobjectinvalrectcommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP getinvalrectcommandUPP  = NewComponentFunctionUPP(getinvalrectcommand, getinvalrectcommandProcInfo);
			result = CallComponentFunction(params, getinvalrectcommandUPP);
			DisposeComponentFunctionUPP(getinvalrectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, getinvalrectcommandUPP);
			#endif
			break;
			
		case IOAgetobjecteditrectcommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP geteditrectcommandUPP  = NewComponentFunctionUPP(geteditrectcommand, geteditrectcommandProcInfo);
			result = CallComponentFunction(params, geteditrectcommandUPP);
			DisposeComponentFunctionUPP(geteditrectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, geteditrectcommandUPP);
			#endif
			break;
			
		case IOAgetvaluecommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP getvaluecommandUPP  = NewComponentFunctionUPP(getvaluecommand, getvaluecommandProcInfo);
			result = CallComponentFunction(params, getvaluecommandUPP);
			DisposeComponentFunctionUPP(getvaluecommandUPP);
			}
			#else
			result = CallComponentFunction(params, getvaluecommandUPP);
			#endif
			break;
			
		case IOAsetvaluecommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP setvaluecommandUPP  = NewComponentFunctionUPP(setvaluecommand, setvaluecommandProcInfo);
			result = CallComponentFunction(params, setvaluecommandUPP);
			DisposeComponentFunctionUPP(setvaluecommandUPP);
			}
			#else
			result = CallComponentFunction(params, setvaluecommandUPP);
			#endif
			break;
			
		case IOAsetcursorcommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP setcursorcommandUPP  = NewComponentFunctionUPP(setcursorcommand, setcursorcommandProcInfo);
			result = CallComponentFunction(params, setcursorcommandUPP);
			DisposeComponentFunctionUPP(setcursorcommandUPP);
			}
			#else
			result = CallComponentFunction(params, setcursorcommandUPP);
			#endif
			break;
			
		case IOAunpackdatacommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP unpackdatacommandUPP  = NewComponentFunctionUPP(unpackdatacommand, unpackdatacommandProcInfo);
			result = CallComponentFunction(params, unpackdatacommandUPP);
			DisposeComponentFunctionUPP(unpackdatacommandUPP);
			}
			#else
			result = CallComponentFunction(params, unpackdatacommandUPP);
			#endif
			break;
			
		case IOAdebugobjectcommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP debugobjectcommandUPP  = NewComponentFunctionUPP(debugobjectcommand, debugobjectcommandProcInfo);
			result = CallComponentFunction(params, debugobjectcommandUPP);
			DisposeComponentFunctionUPP(debugobjectcommandUPP);
			}
			#else
			result = CallComponentFunction(params, debugobjectcommandUPP);
			#endif
			break;
			
		case IOAgetattributescommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP getattributescommandUPP  = NewComponentFunctionUPP(getattributescommand, getattributescommandProcInfo);
			result = CallComponentFunction(params, getattributescommandUPP);
			DisposeComponentFunctionUPP(getattributescommandUPP);
			}
			#else
			result = CallComponentFunction(params, getattributescommandUPP);
			#endif
			break;
			
		case IOAsetattributescommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP setattributescommandUPP  = NewComponentFunctionUPP(setattributescommand, setattributescommandProcInfo);
			result = CallComponentFunction(params, setattributescommandUPP);
			DisposeComponentFunctionUPP(setattributescommandUPP);
			}
			#else
			result = CallComponentFunction(params, setattributescommandUPP);
			#endif
			break;
			
		case IOAgeticoncommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP geticoncommandUPP  = NewComponentFunctionUPP(geticoncommand, geticoncommandProcInfo);
			result = CallComponentFunction(params, geticoncommandUPP);
			DisposeComponentFunctionUPP(geticoncommandUPP);
			}
			#else
			result = CallComponentFunction(params, geticoncommandUPP);
			#endif
			break;
			
		case IOAgetcardcommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP getcardcommandUPP  = NewComponentFunctionUPP(getcardcommand, getcardcommandProcInfo);
			result = CallComponentFunction(params, getcardcommandUPP);
			DisposeComponentFunctionUPP(getcardcommandUPP);
			}
			#else
			result = CallComponentFunction(params, getcardcommandUPP);
			#endif
			break;
			
		case IOApackdatacommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP packdatacommandUPP  = NewComponentFunctionUPP(packdatacommand, packdatacommandProcInfo);
			result = CallComponentFunction(params, packdatacommandUPP);
			DisposeComponentFunctionUPP(packdatacommandUPP);
			}
			#else
			result = CallComponentFunction(params, packdatacommandUPP);
			#endif
			break;
			
		case IOAdisposedatacommand:
			#if TARGET_API_MAC_CARBON
			{
			ComponentFunctionUPP disposedatacommandUPP  = NewComponentFunctionUPP(disposedatacommand, disposedatacommandProcInfo);
			result = CallComponentFunction(params, disposedatacommandUPP);
			DisposeComponentFunctionUPP(disposedatacommandUPP);
			}
			#else
			result = CallComponentFunction(params, disposedatacommandUPP);
			#endif
			break;
			
		default:
			result = paramErr;			
			break;
		} /*switch*/
		
	#ifndef IOAinsideApp

		ExitCodeResource ();
		
	#endif
		
	return (result);
	} /*IOAmain*/
	
	
#ifdef IOAinsideApp

	boolean IOAregistercomponents (void); /*prototype*/

	void IOAunregistercomponents (void); /*prototype*/
	
	void setupbutton (tyioaconfigrecord *); /*prototype*/
		
	void setupcolorpopup (tyioaconfigrecord *); /*prototype*/
		
	void setupcheckbox (tyioaconfigrecord *); /*prototype*/
		
	void setupedittext (tyioaconfigrecord *); /*prototype*/
		
	void setupframe (tyioaconfigrecord *); /*prototype*/
		
	void setupicon (tyioaconfigrecord *); /*prototype*/
		
	void setuppicture (tyioaconfigrecord *); /*prototype*/
		
	void setuppopup (tyioaconfigrecord *); /*prototype*/
		
	void setupradio (tyioaconfigrecord *); /*prototype*/
		
	void setuprect (tyioaconfigrecord *); /*prototype*/
		
	void setupstatic (tyioaconfigrecord *); /*prototype*/
		
	void setupscrollbar (tyioaconfigrecord *); /*prototype*/
	
	#define ioacount 13
		
	static tyioaconfigrecord configarray [ioacount];
	
	static Component componentarray [ioacount];
	
	
	
	static pascal ComponentResult buttonmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [1];
		
		return (IOAmain (params, hstorage));
		} /*buttonmain*/
		
	
	static pascal ComponentResult colorpopupmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [2];
		
		return (IOAmain (params, hstorage));
		} /*colorpopupmain*/
		
	
	static pascal ComponentResult checkboxmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [3];
		
		return (IOAmain (params, hstorage));
		} /*checkboxmain*/
		
	
	static pascal ComponentResult edittextmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [4];
		
		return (IOAmain (params, hstorage));
		} /*edittextmain*/
		
	
	static pascal ComponentResult framemain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [5];
		
		return (IOAmain (params, hstorage));
		} /*framemain*/
		
	
	static pascal ComponentResult iconmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [6];
		
		return (IOAmain (params, hstorage));
		} /*iconmain*/
		
	
	static pascal ComponentResult picturemain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [7];
		
		return (IOAmain (params, hstorage));
		} /*picturemain*/
		
	
	static pascal ComponentResult popupmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [8];
		
		return (IOAmain (params, hstorage));
		} /*popupmain*/
		
	
	static pascal ComponentResult radiomain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [9];
		
		return (IOAmain (params, hstorage));
		} /*radiomain*/
		
	
	static pascal ComponentResult rectmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [10];
		
		return (IOAmain (params, hstorage));
		} /*rectmain*/
		
	
	static pascal ComponentResult staticmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [11];
		
		return (IOAmain (params, hstorage));
		} /*staticmain*/
		
	
	static pascal ComponentResult scrollbarmain (ComponentParameters *params, Handle hstorage) {
		
		config = configarray [12];
		
		return (IOAmain (params, hstorage));
		} /*scrollbarmain*/
		
	
	static boolean IOAregister (short ixarray, OSType subtype, ComponentRoutine main, setupcallback callback) {
		
		ComponentDescription desc;
		Component comp;
		ComponentRoutineUPP routinePtr;
		
		callsetuproutine (callback, &configarray [ixarray]);
		
		IOAclearbytes (&desc, longsizeof (desc));
		
		desc.componentType = 'IOAb';
		
		desc.componentSubType = subtype;
		
		desc.componentManufacturer = 'LAND';
		//Code change by Timothy Paustian Saturday, July 8, 2000 9:42:17 PM
		//Just doing some testing. Split this out so I could look at it.
		
		#if TARGET_API_MAC_CARBON == 1
		
			routinePtr = NewComponentRoutineUPP (main);
		
		#else

			routinePtr = NewComponentRoutineProc (main);
				
		#endif
		
		comp = RegisterComponent (&desc, routinePtr, false, nil, nil, nil);
		
		componentarray [ixarray] = comp;
			
		return (comp != 0);
		} /*IOAregister*/
	

	boolean IOAregistercomponents (void) {
	
		IOAregister (1, 'bttn', &buttonmain, &setupbutton);
		
		IOAregister (2, 'cpop', &colorpopupmain, &setupcolorpopup);
		
		IOAregister (3, 'ckbx', &checkboxmain, &setupcheckbox);
		
		IOAregister (4, 'edtx', &edittextmain, &setupedittext);
		
		IOAregister (5, 'fram', &framemain, &setupframe);
		
		IOAregister (6, 'icon', &iconmain, &setupicon);
		
		IOAregister (7, 'pict', &picturemain, &setuppicture);
		
		IOAregister (8, 'popu', &popupmain, &setuppopup);
		
		IOAregister (9, 'rdio', &radiomain, &setupradio);
		
		IOAregister (10, 'rect', &rectmain, &setuprect);
		
		IOAregister (11, 'text', &staticmain, &setupstatic);
		
		IOAregister (12, 'scro', &scrollbarmain, &setupscrollbar);

		return (true);
		} /*IOAregistercomponents*/
		

	void IOAunregistercomponents (void) {
		
		short i;
		
		for (i = 1; i <= ioacount; i++)
			UnregisterComponent (componentarray [i]);
		
		} /*IOAunregistercomponents*/

#endif
