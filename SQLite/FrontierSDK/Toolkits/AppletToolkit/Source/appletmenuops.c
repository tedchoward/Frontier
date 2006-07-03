
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletstrings.h"
#include "appletdynamicarrays.h"
#include "appletmenuops.h"


tydynamicarray menuarray = {longsizeof (MenuHandle), nil};


boolean pushmenuitem (MenuHandle hmenu, bigstring bs, boolean flenabled) {
	
	short item;
	bigstring bstemp;
	
	item = CountMItems (hmenu) + 1; /*new item will be after current end*/
	
	if (equalstrings (bs, "\p(-")) /*take disabled seperator as is*/
		AppendMenu (hmenu, bs);
	
	else { /*to allow meta-characters in bs, append blank item, then set item text*/
		
		setstringwithchar (chspace, bstemp);
		
		AppendMenu (hmenu, bstemp);
		
		copystring (bs, bstemp); /*work with a copy*/
		
		if (equalstrings (bstemp, "\p-")) /*this is one meta-character that it won't ignore*/
			copystring ("\p\0-", bstemp);
		
		if (!isemptystring (bs))
			SetItem (hmenu, item, bstemp);
		}
	
	if (!flenabled)
		DisableItem (hmenu, item);

	return (true);
	} /*pushmenuitem*/
	
	
void deletemenuitem (MenuHandle hmenu, short ixmenu) {
	
	DelMenuItem (hmenu, ixmenu);
	} /*deletemenuitem*/
	

void deleteallmenuitems (MenuHandle hmenu) {
	
	short ct, i;
	
	assert (hmenu != nil);
	
	ct = countmenuitems (hmenu);
	
	for (i = ct; i >= 1; i--)
		deletemenuitem (hmenu, i);
	} /*deleteallmenuitems*/
	
	
boolean getmenuitem (MenuHandle hmenu, short ixmenu, bigstring bs) {
	
	if (ixmenu <= 0)
		return (false);
		
	GetItem (hmenu, ixmenu, bs);
		
	return (true);
	} /*getmenuitem*/


boolean setmenuitem (MenuHandle hmenu, short ixmenu, bigstring bs) {
	
	SetItem (hmenu, ixmenu, bs);
	
	return (true);
	} /*setmenuitem*/
	
	
void checkmenuitem (MenuHandle hmenu, short ixmenu, boolean fl) {
		
	CheckItem (hmenu, ixmenu, fl);
	} /*checkmenuitem*/
	

void setmenuitemmark (MenuHandle hmenu, short ixmenu, char ch) {
	
	/*
	set the "mark character" for the menu, 0 to remove any mark
	*/
	
	SetItemMark (hmenu, ixmenu, ch);
	} /*setmenuitemmark*/
	

boolean visitonemenu (short idmenu, tymenuvisitcallback visitproc) {
	
	MenuHandle hmenu;
	short i;
	short lastitem;
	
	hmenu = GetMHandle (idmenu);
	
	if (!hmenu)
		return (false);
	
	lastitem = countmenuitems (hmenu);
	
	for (i = 1; i <= lastitem; i++) {
		
		if (!(*visitproc) (hmenu, i))
			return (false);
		} /*for*/

	return (true);
	} /*visitonemenu*/
	
	
boolean visitallmenus (tymenuvisitcallback visitproc) {
	
	short ct, i;
	MenuHandle hmenu;
	
	ct = sizedynamicarray (&menuarray);
	
	for (i = 0; i < ct; i++) {
		
		indexdynamicarray (i, &menuarray, &hmenu);
		
		if (hmenu != nil) {
		
			if (!(*visitproc) (hmenu, 0))
				return (false);
			}
		} /*for*/
	
	return (true);
	} /*visitallmenus*/
	

static boolean disablemenucallback (MenuHandle hmenu, short num) {
	
	DisableItem (hmenu, 0);
	
	return (true);
	} /*disablemenucallback*/
	
	
void disableallmenus (void) {
	
	HiliteMenu (0); /*make sure no menu is highlighted*/
	
	visitallmenus (&disablemenucallback);
	} /*disableallmenus*/


static boolean enablemenucallback (MenuHandle hmenu, short num) {
	
	EnableItem (hmenu, 0);
	
	return (true);
	} /*enablemenucallback*/
	
	
void enableallmenus (void) {
	
	visitallmenus (&enablemenucallback);
	} /*enableallmenus*/


void stylemenuitem (MenuHandle hmenu, short ixmenu, short style) {
	
	SetItemStyle (hmenu, ixmenu, style);
	} /*stylemenuitem*/
	
	
void installmenubefore (short idmenu, short idafter, MenuHandle *hmenu) {
	
	MenuHandle h;
	
	*hmenu = GetMenu (idmenu); 
	
	if (*hmenu == nil)
		return;
	
	InsertMenu (*hmenu, idafter);
	
	h = *hmenu;
	
	addtodynamicarray (&h, &menuarray);
	} /*installmenubefore*/
	
	
void installmenu (short idmenu, MenuHandle *hmenu) {
	
	installmenubefore (idmenu, 0, hmenu);
	} /*installmenu*/
	
	
void installhierarchicmenu (short idmenu, MenuHandle *hmenu) {

	installmenubefore (idmenu, -1, hmenu);
	} /*installhierarchicmenu*/
	
	
void deletemenu (short idmenu) {
	
	removefromdynamicarray ((long) GetMenu (idmenu), &menuarray);
	
	DeleteMenu (idmenu);
	} /*deletemenu*/
	
	
void setmenuitemenable (MenuHandle hmenu, short item, boolean fl) {
	
	/*
	enable or disable a menu or a menu item.  
	
	if item == 0 we enable or disable the entire menu.
	
	dmb 8/1/90:  check for dummy items (negative item numbers)
	
	DW 8/28/93: if item is 0, and the state didn't change, don't draw the 
	menubar. avoids flicker in the menu bar.
	*/
	
	boolean flwasenabled;
	
	assert (hmenu != nil); /*it happened once, it could happen again*/
	
	if (item < 0) /*this item has been dummied out -- do nothing*/
		return;
	
	flwasenabled = ((**hmenu).enableFlags >> item) & 1;
	
	if (fl)
		EnableItem (hmenu, item);
	else
		DisableItem (hmenu, item);
		
	if ((item == 0) && (fl != flwasenabled))
		DrawMenuBar ();
	} /*setmenuitemenable*/
	
	
void disablemenuitem (MenuHandle hmenu, short item) {
	
	setmenuitemenable (hmenu, item, false);
	} /*disablemenuitem*/
	

void enablemenuitem (MenuHandle hmenu, short item) {
	
	setmenuitemenable (hmenu, item, true);
	} /*enablemenuitem*/


short countmenuitems (MenuHandle hmenu) {
	
	return (CountMItems (hmenu));
	} /*countmenuitems*/
	
	
void uncheckallmenuitems (MenuHandle hmenu) {
	
	short i;
	short ct;
	
	ct = countmenuitems (hmenu);
	
	for (i = 1; i <= ct; i++)
		CheckItem (hmenu, i, false);
	} /*uncheckallmenuitems*/
	
	
void disableallmenuitems (MenuHandle hmenu) {
	
	short i;
	short ct;
	
	ct = countmenuitems (hmenu);
	
	for (i = 1; i <= ct; i++)
		setmenuitemenable (hmenu, i, false);
	
	setmenuitemenable (hmenu, 0, false);	// and the main item itself - LDR 10/29
	} /*disableallmenuitems*/
	
	
void enableallmenuitems (MenuHandle hmenu) {
	
	short i;
	short ct;
	
	ct = countmenuitems (hmenu);
	
	for (i = 1; i <= ct; i++)
		setmenuitemenable (hmenu, i, true);

	setmenuitemenable (hmenu, 0, true);	// and the main item itself - LDR 10/29
	} /*enableallmenuitems*/
	
	
void disableemptymenuitems (MenuHandle hmenu) {
	
	short i, j;
	short ctmenuitems;
	
	ctmenuitems = countmenuitems (hmenu);
	
	for (i = 1; i <= ctmenuitems; i++) {
		
		boolean flnotempty = false;
		bigstring bs;
		
		getmenuitem (hmenu, i, bs);
		
		for (j = 1; j <= stringlength (bs); j++) {
			
			if (bs [j] != ' ') {
				
				flnotempty = true;
				
				break;
				}
			} /*for*/
	
		setmenuitemenable (hmenu, i, flnotempty);
		} /*for*/
	} /*disableemptymenuitems*/
	
	
boolean findinmenu (MenuHandle hmenu, bigstring bs, short *item) {

	short lastitem, i;
	bigstring bsitem;
	
	lastitem = countmenuitems (hmenu);

	for (i = 1; i <= lastitem; i++) {
		
		getmenuitem (hmenu, i, bsitem);
		
		if (equalstrings (bsitem, bs)) {
			
			*item = i;
			
			return (true);
			}
		} /*for*/

	*item = 1;
	
	return (false);
	} /*findinmenu*/
	
	
