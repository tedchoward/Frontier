
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <iac.h>
#include <ioa.h>



typedef struct tyscrollbardata {
	
	short versionnumber;
	
	short controlresourceid;
	
	short controlmin;
	
	short controlmax;
	
	short controlinitialvalue;
	
	ControlHandle hcontrol;
	
	char waste [16];
	} tyscrollbardata, **hdlscrollbardata;


static boolean checkcontrolexists (hdlobject h) {

	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	hdlcard hcard = (**h).owningcard;
	ControlHandle hcontrol = (**hdata).hcontrol;
	
	if (hcontrol != nil)
		return (true);
		
	hcontrol = GetNewControl ((**hdata).controlresourceid, (**hcard).drawwindow);
		
	(**hdata).hcontrol = hcontrol;
	
	(**hcontrol).contrlMin = (**hdata).controlmin;
	
	(**hcontrol).contrlMax = (**hdata).controlmax;
	
	(**hcontrol).contrlValue = (**hdata).controlinitialvalue;
	
	return (hcontrol != nil);
	} /*checkcontrolexists*/
	

static boolean cleanscrollbar (hdlobject h, short textheight, short textwidth, Rect *r) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	short height, width;
	
	if (!checkcontrolexists (h))
		return (false);
		
	height = (*r).bottom - (*r).top;
	
	width = (*r).right - (*r).left;
	
	if (height > width) { /*it's a vertical scrollbar*/
		
		width = IOAclosestmultiple (width, 16);
		
		(*r).right = (*r).left + width;
		}
	else { /*it's a horizontal scrollbar*/
		
		height = IOAclosestmultiple (height, 16);
		
		(*r).bottom = (*r).top + height;
		}
		
	return (true);
	} /*cleanscrollbar*/
	

static pascal void trackscrollarrow (hdlscrollbar sb, short part) {
	
	short val = (**sb).contrlValue;
	short minval = (**sb).contrlMin;
	short maxval = (**sb).contrlMax;
	short pageamount = (maxval - minval) / 10;
	
	switch (part) {
	
		case inUpButton:
			val--;
			
			break;
			
		case inDownButton:
			val++;
			
			break;
			
		case inPageUp:
			val -= pageamount;
			
			break;
						
		case inPageDown:
			val += pageamount;
			
			break;
		} /*switch*/
	
	if (val < minval)
		val = minval;
		
	if (val > maxval)
		val = maxval;
		
	SetCtlValue (sb, val);
	} /*trackscrollarrow*/


#if GENERATINGCFM
	
	static RoutineDescriptor trackscrollarrowDesc = BUILD_ROUTINE_DESCRIPTOR (uppControlActionProcInfo, trackscrollarrow);
	
	#define trackscrollarrowUPP (&trackscrollarrowDesc)
	
#else

	#define trackscrollarrowUPP (&trackscrollarrow)
	
#endif


static boolean clickscrollbar (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	hdlcard hc = (**h).owningcard;
	
	if (!checkcontrolexists (h))
		return (false);
			
	(**hc).runmodechanges = true; /*indicate that one of the objects in the card changed*/
	
	(**hc).recalcneeded = true; /*force recalc of all objects that calc when another changes*/
	
	(**hc).recalctickcount = TickCount (); /*do it asap*/
	
	/*do the actual tracking*/ {
		
		short scrollbarpart;
		ControlHandle sb;
		
		scrollbarpart = FindControl (pt, (**hc).drawwindow, &sb);
		
		assert (sb == (**hdata).hcontrol);
		
		if (scrollbarpart == inThumb) 
			TrackControl (sb, pt, nil);
		else
			TrackControl (sb, pt, trackscrollarrowUPP);
		}
	
	return (true);
	} /*clickscrollbar*/


static boolean drawscrollbar (hdlobject h) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	ControlHandle hcontrol;
	Rect r = (**h).objectrect;
	
	if (!checkcontrolexists (h))
		return (false);
		
	hcontrol = (**hdata).hcontrol;
		
	if (hcontrol == nil) {
		
		FillRect (&r, &quickdrawglobal(gray));
		
		return (true);
		}
	
	(**hcontrol).contrlRect = r;
	
	if (!(**hcontrol).contrlVis)
		ShowControl (hcontrol);
	else
		Draw1Control (hcontrol);
		
	return (true);
	} /*drawscrollbar*/
	
	
static boolean newscrollbardata (hdlscrollbardata *hdata) {

	hdlscrollbardata h;
	
	h = (hdlscrollbardata) NewHandleClear (longsizeof (tyscrollbardata));
	
	if (h == nil)
		return (false);
	
	(**h).versionnumber = 1;
	
	(**h).controlresourceid = 1024;
	
	(**h).controlmin = 0;
	
	(**h).controlinitialvalue = 0;
	
	(**h).controlmax = 99;
	
	*hdata = h;
	
	return (true);
	} /*newscrollbardata*/
	
	
static boolean initscrollbar (tyobject *obj) {
	
	hdlscrollbardata hdata;
	
	if (!newscrollbardata (&hdata))
		return (false);
	
	(*obj).objectdata = (Handle) hdata;
	
	return (false); /*we don't want to edit it*/
	} /*initscrollbar*/
	
	
static boolean recalcscrollbar (hdlobject h, boolean flmajorrecalc) {
	
	return (IOArecalcobjectvalue (h));
	} /*recalcscrollbar*/
	

static boolean getscrollbarinvalrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	return (true);
	} /*getscrollbarinvalrect*/
	
	
static boolean canreplicatescrollbar (hdlobject h) {
	
	return (true); 
	} /*canreplicatescrollbar*/
	

static boolean getscrollbareditrect (hdlobject h, Rect *r) {
	
	return (false); /*can't be edited*/
	} /*getscrollbareditrect*/
	

static boolean getscrollbarvalue (hdlobject h, Handle *hvalue) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	bigstring bs, bscontrolvalue;
	
	if (!checkcontrolexists (h))
		return (false);
		
	IOAcopystring ("\p\"", bs);

	NumToString (GetCtlValue ((**hdata).hcontrol), bscontrolvalue);
	
	IOApushstring (bscontrolvalue, bs);
	
	IOApushstring ("\p\"", bs);
	
	return (IOAnewtexthandle (bs, hvalue));
	} /*getscrollbarvalue*/
	

static boolean debugscrollbar (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugscrollbar*/
	

static boolean getscrollbarattributes (hdlobject h, AppleEvent *event) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	
	IACglobals.event = event; 
	
	if (!IACpushshortparam ((**hdata).controlresourceid, 'sbid'))
		return (false);
	
	if (!IACpushshortparam ((**hdata).controlmin, 'sbmn'))
		return (false);
	
	if (!IACpushshortparam ((**hdata).controlmax, 'sbmx'))
		return (false);
	
	if (!IACpushshortparam ((**hdata).controlinitialvalue, 'sbin'))
		return (false);
	
	return (true);
	} /*getscrollbarattributes*/
	
	
static boolean setscrollbarattributes (hdlobject h, AppleEvent *event) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	tyscrollbardata info = **hdata;
	
	IACglobals.event = event; 
	
	IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sbid', &info.controlresourceid); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sbmn', &info.controlmin); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sbmx', &info.controlmax); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sbin', &info.controlinitialvalue); IACglobals.nextparamoptional = true;
	
	**hdata = info;
	
	/*update the control handle to reflect new numbers*/ {
		
		ControlHandle hcontrol = (**hdata).hcontrol;
		
		(**hcontrol).contrlMin = info.controlmin;
	
		(**hcontrol).contrlMax = info.controlmax;
	
		(**hcontrol).contrlValue = info.controlinitialvalue;
		}
	
	return (true);
	} /*setscrollbarattributes*/
	
	
static boolean disposescrollbardata (hdlobject h) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	
	if ((**hdata).hcontrol != nil)
		DisposeControl ((**hdata).hcontrol);
	
	return (true);
	} /*disposescrollbardata*/
	
	
static boolean unpackscrollbardata (hdlobject h) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	
	(**hdata).hcontrol = nil;
	
	return (true);
	} /*unpackscrollbardata*/
	
	
void setupscrollbar (tyconfigrecord *);


void setupscrollbar (tyconfigrecord *config) {

	IOAcopystring ("\pScrollbar", (*config).objectTypeName);
	
	(*config).objectTypeID = scrollbartype;
	
	(*config).frameWhenEditing = false;
	
	(*config).canEditValue = false;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).handlesMouseTrack = true; /*allow the control manager to track the mouse*/
	
	(*config).hasSpecialCard = true; 
	
	(*config).cantDrawInBitmap = true; /*can't use offscreen bitmap to draw a control*/

	(*config).initObjectCallback = initscrollbar;
	
	(*config).drawObjectCallback = drawscrollbar;
	
	(*config).clickObjectCallback = clickscrollbar;
	
	(*config).recalcObjectCallback = recalcscrollbar;
	
	(*config).cleanupObjectCallback = cleanscrollbar;
	
	(*config).canReplicateObjectCallback = canreplicatescrollbar;
	
	(*config).getObjectEditRectCallback = getscrollbareditrect;
	
	(*config).getValueForScriptCallback = getscrollbarvalue;
	
	(*config).debugObjectCallback = debugscrollbar;	
	
	(*config).getObjectInvalRectCallback = getscrollbarinvalrect;
	
	(*config).getAttributesCallback = getscrollbarattributes;
	
	(*config).setAttributesCallback = setscrollbarattributes;
	
	(*config).unpackDataCallback = unpackscrollbardata;
	
	(*config).disposeDataCallback = disposescrollbardata;
	} /*setupscrollbar*/
	
	
