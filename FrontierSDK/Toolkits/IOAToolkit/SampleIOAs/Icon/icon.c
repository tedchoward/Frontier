
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <iac.h>
#include <ioa.h>
#include <appleticons.h>
#include "ioaicon.h"


#define iconheight 32
#define iconwidth 32

#define iconvertinset 3

#define textvertinset 2


typedef struct tyversion1icondata {
	
	short versionnumber;
	
	short resnum;
	
	boolean haslabel;
	
	unsigned short align: 4; /*DW 8/25/93*/
	
	unsigned short ixlabel: 3; /*DW 8/25/93*/
	
	/*1 more bit is available here*/
	
	Handle iconsuite; /*DW 8/25/93 -- if non-nil, it overrides resnum*/
	
	char waste [2];
	} tyversion1icondata, **hdlversion1icondata;




static boolean cleanicon (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	short gridunits = (**hc).gridunits;
	
	if ((**hdata).haslabel) {
	
		width = IOAmakemultiple (width, gridunits);
		
		if (width < iconwidth)
			width = iconwidth;
		
		height += iconvertinset + iconheight;
		}
	else {
		width = iconwidth;
		
		height = iconheight;
		}
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + height;
	
	return (true);
	} /*cleanicon*/
	

static boolean canreplicateicon (hdlobject h) {
	
	return (true); /*it can be replicated*/
	} /*canreplicateicon*/
	

static boolean geticonvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*geticonvalue*/
	

static boolean debugicon (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugicon*/
	

static short icontextheight (void) {
	
	FontInfo fi;
	short lh;
	
	GetFontInfo (&fi);
	
	lh = fi.ascent + fi.descent + (2 * textvertinset);
	
	if ((lh % 2) == 1) /*round up*/
		lh++;
	
	return (lh);
	} /*icontextheight*/


static boolean geticoneditrect (hdlobject h, Rect *redit) {
	
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	Rect r;
	
	if (!(**hdata).haslabel) /*can't be edited*/
		return (false);
	
	r = (**h).objectrect;
	
	r.top = r.bottom - icontextheight ();
	
	*redit = r;
	
	return (true); /*it can be edited*/
	} /*geticoneditrect*/
	

static boolean drawiconobject (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	boolean haslabel = (**hdata).haslabel;
	Rect r, ricon, rcontains, rtext;
	boolean flhot = false;
	short transform = 0;
	
	r = (**h).objectrect;
	
	rcontains = r;
	
	rcontains.bottom -= icontextheight () + iconvertinset;
	
	ricon = r;
	
	ricon.bottom = ricon.top + iconheight;
	
	ricon.right = ricon.left + iconwidth;
	
	IOAcenterrect (&ricon, rcontains);
	
	if (ricon.top < r.top)
		ricon.top = r.top;
		
	if (!haslabel) /*DW 6/15/93*/
		ricon = r;
		
	if ((**hc).tracking && (**hc).trackerpressed) {
	
		transform = 0x4000; /*display it selected*/
		
		flhot = true;
		}
		
	transform += ttLabel [(**hdata).ixlabel]; /*color it according to the label*/
	
	if ((**hdata).iconsuite != nil)
		ploticonsuite (&ricon, (**hdata).align, transform, (**hdata).iconsuite);
	else
		ploticonresource (&ricon, (**hdata).align, transform, (**hdata).resnum);
	
	if (haslabel) {
	
		geticoneditrect (h, &rtext);
	
		IOAeditdrawtexthandle ((**h).objectvalue, rtext, (**h).objectjustification);
	
		if (flhot) {
		
			InsetRect (&r, 2, 2);
		
			InvertRect (&rtext);
			}
		}
		
	return (true);
	} /*drawiconobject*/
	
	
static boolean recalciconobject (hdlobject h, boolean flmajorrecalc) {
	
	return (IOArecalcobjectvalue (h));
	} /*recalciconobject*/
	
	
static boolean newicondata (hdlicondata *hdata) {

	hdlicondata h;
	
	h = (hdlicondata) NewHandleClear (longsizeof (tyicondata));
	
	if (h == nil)
		return (false);
	
	(**h).versionnumber = 2;
	
	(**h).resnum = 128;
	
	(**h).haslabel = true;
	
	(**h).align = atVerticalCenter + atHorizontalCenter;
	
	*hdata = h;
	
	return (true);
	} /*newicondata*/
	
	
static boolean initicon (tyobject *obj) {
	
	(*obj).objectautosize = true; /*it's kind of senseless to have wierd sized icons*/
	
	(*obj).objectfont = geneva;

	(*obj).objectfontsize = 9;

	(*obj).objectjustification = centerjustified;
	
	newicondata ((hdlicondata *) &(*obj).objectdata);
	
	return (true); /*we do want to edit it*/
	} /*initicon*/
	
	
static boolean geticonattributes (hdlobject h, AppleEvent *event) {
	
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	
	IACglobals.event = event; 
	
	if (!IACpushshortparam ((**hdata).resnum, 'rsnm'))
		return (false);
		
	if (!IACpushbooleanparam ((**hdata).haslabel, 'hlbl'))
		return (false);
		
	return (true);
	} /*geticonattributes*/
	
	
static boolean seticonattributes (hdlobject h, AppleEvent *event) {
	
	hdlicondata hdata = (hdlicondata) (**h).objectdata;	
	Boolean haslabel;
	short resnum;
	
	IACglobals.event = event; 
	
	IACglobals.nextparamoptional = true;

	if (IACgetbooleanparam ('hlbl', &haslabel)) 
		(**hdata).haslabel = haslabel; 
	
	IACglobals.nextparamoptional = true;
	
	if (IACgetshortparam ('rsnm', &resnum))
		(**hdata).resnum = resnum;
	
	return (true);
	} /*seticonattributes*/
	
	
static boolean unpackicondata (hdlobject h) {

	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	
	if (hdata == nil) {
		
		newicondata (&hdata);
		
		(**h).objectdata = (Handle) hdata;
		
		return (hdata != nil);
		}
	
	if ((**hdata).versionnumber == 1) { /*convert from version 1 to version 2*/
		
		tyversion1icondata x;
		
		BlockMove (*hdata, &x, longsizeof (tyversion1icondata));
		
		DisposHandle ((Handle) hdata);
		
		hdata = (hdlicondata) NewHandleClear (longsizeof (tyicondata));
		
		BlockMove (&x, *hdata, longsizeof (tyversion1icondata));
		
		(**hdata).versionnumber = 2;
		
		(**h).objectdata = (Handle) hdata;
		}
		
	return (true);
	} /*unpackicondata*/
	
	
static boolean clickicon (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	tyiconclickcallback callback = (**hdata).iconclick;
	
	if (callback != nil)
		return ((*callback) (h, fl2click));
	
	if (fl2click)
		IOArunbuttonscript (h);
	
	return (true); /*do a minor recalc*/
	} /*clickicon*/
	

void setupicon (tyconfigrecord *);

	
void setupicon (tyconfigrecord *config) {

	IOAcopystring ("\pIcon", (*config).objectTypeName);
	
	(*config).objectTypeID = icontype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).isFontAware = true;
	
	(*config).hasSpecialCard = true;
	
	(*config).initObjectCallback = initicon;
	
	(*config).drawObjectCallback = drawiconobject;
	
	(*config).clickObjectCallback = clickicon;
	
	(*config).recalcObjectCallback = recalciconobject;
	
	(*config).cleanupObjectCallback = cleanicon;
	
	(*config).canReplicateObjectCallback = canreplicateicon;
	
	(*config).getObjectEditRectCallback = geticoneditrect;
	
	(*config).getValueForScriptCallback = geticonvalue;
	
	(*config).debugObjectCallback = debugicon;
	
	(*config).getAttributesCallback = geticonattributes;
	
	(*config).setAttributesCallback = seticonattributes;
		
	(*config).unpackDataCallback = unpackicondata;
	} /*setupicon*/
	
	
