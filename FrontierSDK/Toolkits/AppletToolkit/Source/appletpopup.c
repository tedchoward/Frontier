
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletmemory.h"
#include "appletstrings.h"
#include "appletfont.h"
#include "appletmouse.h"
#include "appletquickdraw.h"
#include "appletops.h"
#include "appletcursor.h"
#include "appletmenuops.h"
#include "appletpopup.h"


#define defaultpopupfontnum geneva
#define defaultpopupfontsize 9
#define defaultpopupfontstyle 0

	
static short trackericonnum;

static Rect trackerrect;



boolean initpopuprecord (typopuprecord *popup) {
	
	/*
	11/11/92 DW: call this when you're initializing a popup record. it doesn't
	allocate anything, it just sets up the record so that default values are in
	place. this may be important as the applet toolkit popup support evolves.
	*/
	
	clearbytes (popup, longsizeof (typopuprecord));
	
	(*popup).fontnum = defaultpopupfontnum;
		
	(*popup).fontsize = defaultpopupfontsize;
		
	(*popup).fontstyle = defaultpopupfontstyle;
		
	return (true);
	} /*initpopuprecord*/
	

boolean pushpopupitem (typopuprecord *popup, bigstring bs, boolean flenabled) {
	
	MenuHandle hmenu = (*popup).hmenu;
	bigstring bsspace;
	
	if (equalstrings (bs, "\p(-")) /*take disabled seperator as is*/
		AppendMenu (hmenu, bs);
	
	else { /*to allow meta-characters in bs, append blank item, then set item text*/
		
		setstringwithchar (chspace, bsspace);
		
		AppendMenu (hmenu, bsspace);
		
		if (!isemptystring (bs))
			SetItem (hmenu, CountMItems (hmenu), bs);
		}
	
	if (!flenabled)
		DisableItem (hmenu, CountMItems (hmenu));

	return (true);
	} /*pushpopupitem*/
	
	
static void displaypopupicon (Rect r, boolean flenabled) {
	
	short iconnum;
	
	if (flenabled)
		iconnum = 0;
	else
		iconnum = 1;
	
	#ifdef clayBasket /*6/24/93 DW: conflict between SICN resources*/
		
		plotsmallicon (r, 133, iconnum, false);
		
	#else
	
		plotsmallicon (r, 129, iconnum, false);
	
	#endif
	} /*displaypopupicon*/
	
	
void idealpopupsize (typopuprecord *popup, short *height, short *width) {
	
	/*
	11/11/92 DW: based on the string and font/size/style and other attributes
	of the popup record, determine the ideal height and width for the rect used
	to display the popup. this is needed by IOWA in it's Clean Object command.
	*/
	
	short h, w;
	short maxwidth;
	
	pushstyle ((*popup).fontnum, (*popup).fontsize, (*popup).fontstyle);
	
	h = globalfontinfo.ascent + globalfontinfo.descent + 4;
	
	maxwidth = StringWidth ((*popup).bs);
	
	(*popup).fldisposemenu = true; /*assume the menu was created dynamically, not loaded from resource*/
	
	if ((*popup).getmenucallback ()) { /*use the strings in the menu to determine width*/
		
		MenuHandle hmenu = (*popup).hmenu;
		short i;
		short sw;
		short lastitem;
		bigstring bs;
		
		lastitem = countmenuitems (hmenu);
		
		for (i = 1; i <= lastitem; i++) {
			
			getmenuitem (hmenu, i, bs);
			
			sw = StringWidth (bs);
			
			if (sw > maxwidth)
				maxwidth = sw;
			} /*for*/
		
		if ((*popup).fldisposemenu)
			DisposeMenu (hmenu);
		}
		
	w = maxwidth + popuparrowwidth + popupdrawinset + 8;
	
	popstyle ();
	
	*height = h;
	
	*width = w;
	} /*idealpopupsize*/
	
	
void drawpopup (typopuprecord *popup) {
	
	Rect rpopup = (*popup).popuprect;
	bigstring bs;
	Rect r;
	
	if (emptyrect (rpopup))
		return;
	
	pushbackcolor (&whitecolor);
	
	r = rpopup;
	
	EraseRect (&r);
	
	InsetRect (&r, 1, 1);
	
	r.left = r.right - popuparrowwidth;
	
	displaypopupicon (r, true);
	
	r = rpopup;
	
	FrameRect (&r);
	
	MoveTo (r.left + 1, r.bottom);
	
	LineTo (r.right, r.bottom);
	
	LineTo (r.right, r.top + 1);
	
	r.right -= popuparrowwidth;
	
	InsetRect (&r, 4, 2);

	pushstyle ((*popup).fontnum, (*popup).fontsize, (*popup).fontstyle);
	
	if ((*popup).drawcontentscallback != nil) /*DW 7/2/93*/
		(*popup).drawcontentscallback (r);
	else {
	
		MoveTo (r.left, r.bottom - globalfontinfo.descent - 1);
	
		copystring ((*popup).bs, bs);
	
		ellipsize (bs, r.right - r.left);
	
		DrawString (bs);
		}
	
	popstyle ();
	
	zerorect (&(*popup).leftarrowrect);
	
	if ((*popup).flarrows || (*popup).flleftarrow) {
	
		r = rpopup;
		
		r.right = r.left - popupdrawinset;
		
		r.left = r.right - widthsmallicon;
		
		OffsetRect (&r, 2, 1); /*move it right by two pixels, down by one pixel*/
		
		(*popup).leftarrowrect = r;
		
		plotsmallicon (r, 128, 1, false); 
		}
	
	zerorect (&(*popup).rightarrowrect);
	
	if ((*popup).flarrows || (*popup).flrightarrow) {
	
		r = rpopup;
		
		r.left = r.right + popupdrawinset;
		
		r.right = r.left + widthsmallicon;
		
		OffsetRect (&r, -2, 1); /*move it left by two pixels, down by one pixel*/
		
		(*popup).rightarrowrect = r;
		
		plotsmallicon (r, 128, 0, false); 
		}
	
	popbackcolor ();
	} /*drawpopup*/
	

static void icontracker (boolean flpressed) {
	
	/*
	callback routine for the mouse tracker.
	*/
	
	short iconnum = trackericonnum;
	
	if (flpressed)
		iconnum += 2;
		
	pushbackcolor (&lightbluecolor);
	
	EraseRect (&trackerrect);
	
	plotsmallicon (trackerrect, 128, iconnum, false);
	
	popbackcolor ();
	} /*icontracker*/
	
	
static boolean arrowtracker (Rect r, popupcallback arrowcallback) {

	trackerrect = r;
	
	icontracker (true); /*display button in its pressed state*/
	
	if (mousetrack (r, &icontracker)) {
	
		icontracker (false); /*display button in its un-pressed state*/
	
		(*arrowcallback) ();
		}
	
	return (true);
	} /*arrowtracker*/


boolean popupclick (typopuprecord *popup) {
	
	/*
	10/22/92 DW: added fldisposemenu field to a popup record. if the menu comes from
	the resource fork of the application, set this to false. if it gets disposed, the
	resource manager will crash. needed in the calendar program.
	*/

	if (PtInRect (mousestatus.localpt, &(*popup).leftarrowrect)) {
		
		trackericonnum = 1;
		
		if ((*popup).trackleftarrow) /*the caller handles tracking*/
			(*(*popup).leftarrowcallback) ();
		else
			arrowtracker ((*popup).leftarrowrect, (*popup).leftarrowcallback);
		
		return (true);
		}

	if (PtInRect (mousestatus.localpt, &(*popup).rightarrowrect)) {
		
		trackericonnum = 0;
	
		if ((*popup).trackrightarrow) /*the caller handles tracking*/
			(*(*popup).rightarrowcallback) ();
		else
			arrowtracker ((*popup).rightarrowrect, (*popup).rightarrowcallback);
		
		return (true);
		}
		
	if (PtInRect (mousestatus.localpt, &(*popup).popuprect)) {
		
		MenuHandle hmenu;
		Point pt;
		boolean flcommandselected = false;
		
		(*popup).fldisposemenu = true; /*assume the menu was created dynamically, not loaded from resource*/
		
		if (!(*popup).getmenucallback ()) /*error, but consume the mouse click*/
			return (true);
		
		hmenu = (*popup).hmenu;
		
		uncheckallmenuitems (hmenu);
		
		CheckItem (hmenu, (*popup).checkeditem, true);
		
		InsertMenu (hmenu, -1); /*see IM-V, p 236*/
		
		setcursortype (cursorisarrow); /*arrow cursor is more appropriate here*/
		
		pt.v = (*popup).popuprect.top;
		
		pt.h = (*popup).popuprect.left; 
		
		if ((*popup).checkeditem == 0) { /*no item selected*/
			
			CalcMenuSize (hmenu); 
		
			pt.v -= (**hmenu).menuHeight / 2;
			}
			
		LocalToGlobal (&pt); 
		
		pushstyle (geneva, 9, 0);
		
		/*jump into the roms*/ {
			
			long result;
			short lo, hi;
			
			result = PopUpMenuSelect (hmenu, pt.v, pt.h, (*popup).checkeditem);
			
			lo = LoWord (result);
			
			hi = HiWord (result);
			
			flcommandselected = hi > 0; /*something was selected*/
			
			(*popup).itemselected = lo;
			}
		
		popstyle ();
		
		if (flcommandselected)
			(*popup).menuselectcallback ();
		
		DeleteMenu ((*popup).idmenu);
		
		if ((*popup).fldisposemenu)
			DisposeMenu (hmenu);
			
		InvalRect (&(*popup).popuprect);
		
		return (true);
		}
	
	return (false);
	} /*popupclick*/
	

boolean adjustpopupcursor (typopuprecord *popup) {
	
	if (!PtInRect (mousestatus.localpt, &(*popup).popuprect)) 
		return (false);
	
	setcursortype (cursorispopup);
	
	return (true);
	} /*adjustpopupcursor*/




