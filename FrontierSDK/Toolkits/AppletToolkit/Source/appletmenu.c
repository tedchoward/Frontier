
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <iac.h>
#include "appletinternal.h"
#include "appletcardops.h"
#include "appletops.h"
#include "appletmenuops.h"
#include "appletmenu.h"
#include "appletwindowmenu.h"
#include "appletmain.h"

#ifndef cutCmd
#define	cutCmd		3
#define	copyCmd		4
#define	pasteCmd	5
#define clearCmd	6
#endif

MenuHandle hdlapplemenu, hdlfilemenu, hdleditmenu;

MenuHandle hdlfontmenu, hdlsizemenu, hdlstylemenu, hdljustifymenu;



void dialogmenuenabler (boolean flbeforedialog) {
	
	/*
	call before puting up a dialog. we disable all menus except the
	Edit menu.
	
	then call after the dialog is finished, and we re-enable all the
	menus.
	*/
	
	if (flbeforedialog) {
		
		disableallmenus ();
		
		enablemenuitem (hdleditmenu, 0);
		
		disableallmenuitems (hdleditmenu);
		
		enablemenuitem (hdleditmenu, cutitem);
		
		enablemenuitem (hdleditmenu, copyitem);
		
		enablemenuitem (hdleditmenu, pasteitem);
		
		enablemenuitem (hdleditmenu, clearitem);
		}
	else 
		enableallmenus ();
		
	DrawMenuBar ();
	} /*dialogmenuenabler*/
	
	
static boolean fontmenuchecker (MenuHandle hmenu, short itemnumber) {

	boolean fl;
	bigstring bs;
	short fontnum;
	
	getmenuitem (hmenu, itemnumber, bs);
	
	GetFNum (bs, &fontnum);
	
	fl = (fontnum == (**app.appwindow).selectioninfo.fontnum);
	
	checkmenuitem (hmenu, itemnumber, fl);
	
	enablemenuitem (hmenu, itemnumber);
		
	return (true);
	} /*fontmenuchecker*/
	
	
static boolean sizemenuchecker (MenuHandle hmenu, short itemnumber) {
	
	short checkeditem;
	short fontsize;
	short itemsize;
	short style;
	short fontnum;
	
	fontsize = (**app.appwindow).selectioninfo.fontsize;
	
	switch (fontsize) {
		
		case -1: /*no consistent size across selection*/
			checkmenuitem (hmenu, itemnumber, false);
			
			goto L1; /*skip to determining if it's a real font or not*/
		
		case 9:
			checkeditem = point9item;
			
			break;
			
		case 10:
			checkeditem = point10item;
			
			break;
			
		case 12:
			checkeditem = point12item;
			
			break;
			
		case 14:
			checkeditem = point14item;
			
			break;
			
		case 18:
			checkeditem = point18item;
			
			break;
			
		case 24:
			checkeditem = point24item;
			
			break;
			
		case 36:
			checkeditem = point36item;
			
			break;
			
		case 48:
			checkeditem = point48item;
			
			break;
			
		case 72:
			checkeditem = point72item;
			
			break;
			
		default:
			checkeditem = pointcustomitem;
			
			break;
		} /*switch*/
	
	checkmenuitem (hmenu, itemnumber, itemnumber == checkeditem);
	
	L1:
	
	fontnum = (**app.appwindow).selectioninfo.fontnum;
	
	if (fontnum == -1) { /*no consistent font across selection*/
		
		stylemenuitem (hmenu, itemnumber, 0); /*plain*/
		
		return (true);
		}
	
	itemsize = -1;
	
	switch (itemnumber) {
		
		case point9item:
			itemsize = 9;
			
			break;
			
		case point10item:
			itemsize = 10;
			
			break;
			
		case point12item:
			itemsize = 12;
			
			break;
			
		case point14item:
			itemsize = 14;
			
			break;
			
		case point18item:
			itemsize = 18;
			
			break;
			
		case point24item:
			itemsize = 24;
			
			break;
			
		case point36item:
			itemsize = 36;
			
			break;
			
		case point48item:
			itemsize = 48;
			
			break;
			
		case point72item:
			itemsize = 72;
			
			break;
			
		} /*switch*/
	
	style = 0;
	
	if (itemsize != -1)
		if (RealFont (fontnum, itemsize))
			style = outline;
		
	stylemenuitem (hmenu, itemnumber, style);
		
	return (true);
	} /*sizemenuchecker*/
	
	
static boolean stylemenuchecker (MenuHandle hmenu, short itemnumber) {
	
	boolean flchecked = false;
	tyselectioninfo x = (**app.appwindow).selectioninfo;
	
	switch (itemnumber) {
		
		case plainitem:
			flchecked = x.flplain;
			
			break;
			
		case bolditem:
			flchecked = x.flbold;
			
			break;
			
		case italicitem:
			flchecked = x.flitalic;
			
			break;
			
		case underlineitem:
			flchecked = x.flunderline;
			
			break;
			
		case outlineitem:
			flchecked = x.floutline;
			
			break;
			
		case shadowitem:
			flchecked = x.flshadow;
			
			break;

		} /*switch*/
	
	checkmenuitem (hmenu, itemnumber, flchecked);
	
	return (true);
	} /*stylemenuchecker*/
	

static boolean justifymenuchecker (MenuHandle hmenu, short itemnumber) {
	
	tyjustification justification = (**app.appwindow).selectioninfo.justification;
	short checkeditem = -1;
	
	switch (justification) {
		
		case leftjustified:
			checkeditem = leftjustifyitem;
			
			break;
			
		case centerjustified:
			checkeditem = centerjustifyitem;
			
			break;
			
		case rightjustified:
			checkeditem = rightjustifyitem;
			
			break;
			
		case fulljustified:
			checkeditem = fulljustifyitem;
			
			break;
			
		case unknownjustification:
			checkmenuitem (hmenu, itemnumber, false);
			
			return (true);
			
		} /*switch*/
	
	checkmenuitem (hmenu, itemnumber, itemnumber == checkeditem);
	
	return (true);
	} /*justifymenuchecker*/


static void checkfontsizestyle (void) {
	
	hdlappwindow hw = app.appwindow;
	tyselectioninfo x;
	
	if (hw == nil) /*no windows open*/
		return;
	
	x = (**hw).selectioninfo;
	
	if (!x.fldirty) /*nothing to do*/
		return;
	
	appprecallback ();
	
	(*app.setselectioninfocallback) ();
	
	apppostcallback ();
	
	x = (**hw).selectioninfo; /*get updated flags*/
	
	if (x.flcansetfont)
		visitonemenu (fontmenu, &fontmenuchecker);
	
	if (x.flcansetsize)
		visitonemenu (sizemenu, &sizemenuchecker);
	
	if (x.flcansetstyle)
		visitonemenu (stylemenu, &stylemenuchecker);
		
	if (x.flcansetjust)
		visitonemenu (justifymenu, &justifymenuchecker);
	} /*checkfontsizestyle*/


void adjustmenus (void) {
	
	hdlappwindow ha = app.appwindow;
	boolean flappwindow = ha != nil;
	boolean flanywindow = FrontWindow () != nil;
	MenuHandle hmenu;
	boolean flchanges = false;
	boolean flsavable = true; 
	tyselectioninfo x;
	bigstring bsundo;
	boolean flenabled;
	
	if (app.haswindowmenu) 
		adjustwindowmenu ();
	
	if (flappwindow) {
	
		flsavable = !(**ha).notsaveable;
		
		flchanges = (**ha).flmadechanges;
		}
	
	hmenu = hdlfilemenu;
	
	if (hmenu != nil) {
	
		setmenuitemenable (hmenu, newitem, flsavable);
		
		setmenuitemenable (hmenu, openitem, true);
		
		setmenuitemenable (hmenu, closeitem, flanywindow);
		
		setmenuitemenable (hmenu, saveitem, flchanges && flsavable);
		
		setmenuitemenable (hmenu, saveasitem, flappwindow && flsavable);
		
		setmenuitemenable (hmenu, revertitem, flchanges && flsavable && ((**ha).fnum != 0));
		
		enablemenuitem (hmenu, pagesetupitem);
		
		setmenuitemenable (hmenu, printitem, flappwindow);
		
		enablemenuitem (hmenu, quititem);
		}
	
	hmenu = hdleditmenu;
	
	if (hmenu != nil) {
		
		if (getundomenuitem (bsundo, &flenabled)) {
			
			setmenuitem (hmenu, undoitem, bsundo);
			
			setmenuitemenable (hmenu, undoitem, flenabled);
			}
		
		if (flappwindow) {
			 
			checkfontsizestyle ();
			
			x = (**app.appwindow).selectioninfo;
			}
		else
			clearbytes (&x, longsizeof (x)); /*set all flags to 0*/
		
		setmenuitemenable (hmenu, fontitem, x.flcansetfont);
		
		setmenuitemenable (hmenu, sizeitem, x.flcansetsize);
		
		setmenuitemenable (hmenu, styleitem, x.flcansetstyle);
		
		setmenuitemenable (hmenu, justifyitem, x.flcansetjust);
	
		if (x.flcansetsize) {
		
			hmenu = hdlsizemenu;
			
			enablemenuitem (hmenu, point9item);
			
			enablemenuitem (hmenu, point10item);
			
			enablemenuitem (hmenu, point12item);
			
			enablemenuitem (hmenu, point14item);
			
			enablemenuitem (hmenu, point18item);
			
			enablemenuitem (hmenu, point24item);
			
			enablemenuitem (hmenu, point36item);
			
			enablemenuitem (hmenu, point48item);
			
			enablemenuitem (hmenu, point72item);
			
			/*
			enablemenuitem (hmenu, pointupitem);
			
			enablemenuitem (hmenu, pointdownitem);
			*/
			
			enablemenuitem (hmenu, pointcustomitem);
			}
		
		if (x.flcansetstyle) {
	
			hmenu = hdlstylemenu;
			
			enablemenuitem (hmenu, plainitem);
			
			enablemenuitem (hmenu, bolditem);
			
			enablemenuitem (hmenu, italicitem);
			
			enablemenuitem (hmenu, underlineitem);
			
			enablemenuitem (hmenu, outlineitem);
			
			enablemenuitem (hmenu, shadowitem);
			}
			
		if (x.flcansetjust) {
		
			hmenu = hdljustifymenu;
			
			enablemenuitem (hmenu, leftjustifyitem);
		
			enablemenuitem (hmenu, centerjustifyitem);
		
			enablemenuitem (hmenu, rightjustifyitem);
		
			enablemenuitem (hmenu, fulljustifyitem);
			}
		} /*Edit menu*/
	} /*adjustmenus*/


static hdlcard aboutcard = nil;


static void aboutfilter (EventRecord *ev) {
	
	#ifdef appRunsCards
		
		switch ((*ev).what) {
			
			case iowaInitEvent: 
				aboutcard = (hdlcard) (*ev).message;
				
				break;
				
			case iowaCloseEvent:
				currentcard = (hdlcard) (*ev).message; 
				
				appsavedialoginfo (129); /*remember window position*/
				
				aboutcard = nil;
					
				break;
			
			} /*switch*/
		
	#endif
	} /*aboutfilter*/
	

static void openaboutwindow (void) {
	
	#ifdef appRunsCards
		
		appprecallback ();
		
		if (aboutcard == nil)
			appopencardresource (129, aboutfilter);
		else
			appcardtofront (aboutcard); 

		apppostcallback ();
		
	#endif
	} /*openaboutwindow*/


static void undocommand (void) {
	
	short ixaction;
	
	appprecallback (); 
	
	if (getredoaction (&ixaction))
		redolastaction (true);
	else
		undolastaction (true);
	
	apppostcallback ();
	} /*undocommand*/


static boolean pushscrap (Handle h, OSType type) {

	OSErr ec;
	
	if (h == nil)
		return (true);
		
	lockhandle (h);
	
	ec = PutScrap (GetHandleSize (h), type, *h);
	
	unlockhandle (h);
	
	if (ec != noErr) {
	
		alertdialog ("\pError copying to clipboard.");
		
		return (false);
		}
		
	return (true);
	} /*pushscrap*/
	
	
void appcopycommand (void) {
	
	/*
	DW 9/1/93: changed name, it's called from Clay Basket, to implement the
	Copy Alias command.
	*/

	Handle appscrap = nil, textscrap = nil, pictscrap = nil;
	boolean flhavescrap = false;
	boolean fl;
	
	#ifdef appRunsCards
	
		if (cardEditCommand (copyCmd)) /*command was consumed by a shared window*/
			return;
	
	#endif
	
	appprecallback ();
	
	fl = (*app.copycallback) (&appscrap);
	
	apppostcallback ();
	
	if (fl) {
		
		if (app.commanderror)
			return;
			
		flhavescrap = true;
		}
	
	appprecallback ();
	
	(*app.gettextcallback) (&textscrap); /*textscrap is nil if it failed*/
	
	apppostcallback ();
	
	if (textscrap != nil)
		flhavescrap = true;

	appprecallback ();
	
	(*app.getpictcallback) (&pictscrap); /*pictscrap is nil if it failed*/
	
	apppostcallback ();
	
	if (pictscrap != nil)
		flhavescrap = true;

	if (!flhavescrap) {
		
		alertdialog ("\pNothing to copy.");
		
		return;
		}
		
	ZeroScrap ();
	
	if (!pushscrap (appscrap, app.creator)) /*error message already set/displayed*/
		return;
	
	if (!pushscrap (textscrap, 'TEXT')) 
		return;
	
	pushscrap (pictscrap, 'PICT');
	} /*appcopycommand*/
	
	
static void clearcommand (void) {
	
	#ifdef appRunsCards
	
		if (cardEditCommand (clearCmd)) /*command was consumed by a shared window*/
			return;
			
	#endif
		
	appprecallback ();
	
	(*app.clearcallback) ();
	
	apppostcallback ();
	} /*clearcommand*/
	
	
static void pastecommand (void) {
	
	Handle hscrap;
	long offset, ctbytes;
	
	#ifdef appRunsCards
		
		if (cardEditCommand (pasteCmd)) /*command was consumed by a shared window*/
			return;
	
	#endif
	
	hscrap = appnewhandle (0);
	
	ctbytes = GetScrap (hscrap, app.creator, &offset);
	
	if (ctbytes > 0) {
		
		boolean fl;
		
		appprecallback ();
		
		fl = (*app.pastecallback) (hscrap);
		
		apppostcallback ();
	
		if (fl) {
			
			DisposeHandle (hscrap);
			
			return;
			}
			
		DisposeHandle (hscrap);
			
		hscrap = appnewhandle (0);
		}
	
	ctbytes = GetScrap (hscrap, 'PICT', &offset);
	
	if (ctbytes > 0) {
		
		boolean fl;
		
		appprecallback ();
		
		fl = (*app.putpictcallback) (hscrap);
		
		apppostcallback ();
		
		if (fl)
			return;
			
		DisposeHandle (hscrap); /*app didn't want the PICT*/
		
		hscrap = appnewhandle (0);
		}
		
	ctbytes = GetScrap (hscrap, 'TEXT', &offset);
	
	if (ctbytes > 0) {
	
		boolean fl;
		
		appprecallback ();
		
		fl = (*app.puttextcallback) (hscrap);
		
		apppostcallback ();
		
		if (fl)
			return;
			
		DisposeHandle (hscrap);
			
		hscrap = appnewhandle (0);
		}
	
	if (hscrap != nil)
		DisposeHandle (hscrap);	
	} /*pastecommand*/
	
	
static void cutcommand (void) {
	
	#ifdef appRunsCards
	
		if (cardEditCommand (cutCmd)) /*command was consumed by a shared window*/
			return;
	
	#endif
		
	appcopycommand ();
	
	if (app.commanderror)
		return;

	clearcommand ();
	} /*cutcommand*/
	
	
static void UpdateSharedMenus (void) {

	AppleEvent event, reply;
	AEAddressDesc adr; 
	ProcessSerialNumber psn;

	psn.highLongOfPSN = 0;

	psn.lowLongOfPSN = kCurrentProcess;

	AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, sizeof (psn), &adr);
	
	AECreateAppleEvent ('BARC', 'updm', &adr, kAutoGenerateReturnID, kAnyTransactionID, &event);
	
	AEDisposeDesc (&adr);

	AESend (&event, &reply, kAEWaitReply, kAENormalPriority, kNoTimeOut, (AEIdleUPP) nil, nil);
	
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);
	} /*UpdateSharedMenus*/


static void testmenus (void) {
	
	deletemenu (applemenu);
	
	deletemenu (filemenu);
	
	deletemenu (editmenu);
	
	deletemenu (fontmenu);
	
	deletemenu (sizemenu);
	
	deletemenu (stylemenu);
	
	deletemenu (justifymenu);
	
	deletemenu (windowmenu);
	
	DisposeSharedMenus ();
	
	initmenus ();
	
	UpdateSharedMenus ();
	} /*testmenus*/
	

void handlemenu (long codeword) {
	
	hdlappwindow ha = app.appwindow;
	short idmenu, iditem;
	boolean fl;
	
	/*
	if (shiftkeydown ()) {
		
		testmenus ();
		
		return;
		}
	*/
	iditem = LoWord (codeword);

	if (iditem <= 0) 	
		goto exit;
     
	idmenu = HiWord (codeword); 
	
	if (SharedMenuHit (idmenu, iditem)) 
		goto exit;
	
	appprecallback (); 
	
	fl = (*app.menucallback) (idmenu, iditem);
	
	apppostcallback ();
	
	if (fl) /*consumed by the app*/
		goto exit;
	
	switch (idmenu) {
   
		case applemenu: 
			switch (iditem) {
			
				case aboutitem:
					openaboutwindow ();
					
					break;
					
				default: {
				
					bigstring bs;
					
					GetItem (hdlapplemenu, iditem, bs);
          
					OpenDeskAcc (bs);
					
					break;
					}
				} /*switch*/
			
			break; /*apple menu*/

		case filemenu: 
			switch (iditem) {
			
				case newitem: 
					newuntitledappwindow (true);
					
					break;
					
				case openitem:
					openfrontwindow ();
					
					break;
				
				case closeitem:
					if (optionkeydown ())
						closeallwindows ();
					else 
						closefrontwindow ();
					
					break;
				
				case saveitem: 
					savefrontwindow ();
					
					break;
					
				case saveasitem:
					saveasfrontwindow ();
					
					break;
					
				case revertitem:
					revertfrontwindow ();
					
					break;
					
				case pagesetupitem: 
					pagesetup ();
					
					break;
				
				case printitem:
					printappwindow (ha, true);
					
					break;
					
				case quititem:
					exitmainloop ();
					
					break;
				} /*switch*/
            
			break; /*file menu*/ 
			
		case editmenu: 
			switch (iditem) {
				
				case undoitem:
					undocommand ();
					
					break;
					
				case cutitem:
					cutcommand ();
					
					break;
					
				case copyitem:
					appcopycommand ();
					
					break;
					
				case pasteitem:
					pastecommand ();
					
					break;
					
				case clearitem:
					clearcommand ();
					
					break;
					
				case selectallitem:
					selectallcommand ();
					
					break;
				} /*switch*/
				
			break; /*edit menu*/
			
		case windowmenu:
			selectwindowmenu (iditem);

			break; /*window menu*/
			
		case fontmenu: {
			bigstring bs;
			
			getmenuitem (hdlfontmenu, iditem, bs);
			
			appsetfont (bs);
			
			break;
			} /*font menu*/
			
		case sizemenu: {
			short size = (**ha).selectioninfo.fontsize;
			
			switch (iditem) {
				
				case point9item:
					size = 9; break;
					
				case point10item:
					size = 10; break;
					
				case point12item:
					size = 12; break;
					
				case point14item:
					size = 14; break;
					
				case point18item:
					size = 18; break;
					
				case point24item:
					size = 24; break;
					
				case point36item:
					size = 36; break;
					
				case point48item:
					size = 48; break;
					
				case point72item:
					size = 72; break;
					
				case pointupitem:
					size++;
					
					break;
					
				case pointdownitem:
					size--;
					
					if (size < 1)
						size = 1;
						
					break;
					
				case pointcustomitem: {
					bigstring bs;
					long num;
					
					NumToString (size, bs);
					
					if (!askdialog ("\pCustom font size:", bs))
						goto exit;
					
					StringToNum (bs, &num); /*no protection against non-numeric chars*/
					
					size = (short) num;
					
					break;
					}
				} /*switch*/
				
			if (size != 0) 
				appsetfontsize (size);
								
			break;
			} /*size menu*/
			
		case stylemenu: {
			tyselectioninfo x = (**ha).selectioninfo;
			
			clearbytes (&x, longsizeof (x)); /*init all fields to zero*/
			
			x.flplain = false; /*it's off unless plain was selected*/
			
			switch (iditem) {
				
				case plainitem:
					x.flplain = true;
					
					break;
					
				case bolditem:
					x.flbold = true;
					
					break;
					
				case italicitem:
					x.flitalic = true;
					
					break;
					
				case underlineitem:
					x.flunderline = true;
					
					break;
					
				case outlineitem:
					x.floutline = true;
					
					break;
					
				case shadowitem:
					x.flshadow = true;
					
					break;
				} /*switch*/
			
			setselectionstyleinfo (&x, true); /*pack into bits of x.fontstyle*/
	
			(**ha).selectioninfo = x;
			
			appprecallback (); 
			
			(*app.setstylecallback) ();
			
			apppostcallback ();
			
			(**ha).selectioninfo.fldirty = true;
			
			break;
			} /*style menu*/
			
		case justifymenu: {
			tyjustification justification = unknownjustification;
			
			switch (iditem) {
				
				case leftjustifyitem:
					justification = leftjustified;
					
					break;
					
				case centerjustifyitem:
					justification = centerjustified;
					
					break;
					
				case rightjustifyitem:
					justification = rightjustified;
					
					break;
					
				case fulljustifyitem:
					justification = fulljustified;
					
					break;
					
				} /*switch*/
				
			if (justification != unknownjustification) {
				
				(**ha).selectioninfo.justification = justification;
				
				appprecallback (); 
				
				(*app.setjustifycallback) ();
				
				apppostcallback ();
				
				(**ha).selectioninfo.fldirty = true;
				}
				
			break;
			} /*justify menu*/
		
		case extendedmenu: { /*special key on the extended keyboard*/
			switch (iditem) {
				
				case helpitem:
					openaboutwindow ();
					
					break;
				
				case homeitem:
					appprecallback (); (*app.scrollcallback) (down, false, infinity); apppostcallback ();
					
					break;
				
				case enditem:
					appprecallback (); (*app.scrollcallback) (up, false, infinity); apppostcallback ();
					
					break;
				
				case pageupitem:
					appprecallback (); (*app.scrollcallback) (down, true, 1); apppostcallback ();
					
					break;
				
				case pagedownitem:
					appprecallback (); (*app.scrollcallback) (up, true, 1); apppostcallback ();
					
					break;
				} /*switch*/
			} /*extended menu*/
		
		} /*switching on which menu was invoked*/
        
	exit:
	
	HiliteMenu (0);
	} /*handlemenu*/
	
	
boolean flipcustommenu (hdlappwindow appwindow, boolean flactivate) {
	
	/*
	DW 8/19/93: add call with appwindow == nil, flactivate == false.
	
	DW 10/8/93: insert menu after the Window menu.
	*/
	
	static MenuHandle hdlcustommenu = nil;
	static short idcustommenu;
	boolean fldraw = false;
	
	if (hdlcustommenu != nil) {
		
		deletemenu (idcustommenu);
		
		hdlcustommenu = nil;
		
		fldraw = true;
		}
		
	if (flactivate) {
		
		idcustommenu = (**appwindow).idmenu;
		
		if (idcustommenu != 0) {
			
			installmenubefore (idcustommenu, windowmenu, &hdlcustommenu);
			
			fldraw = true;
			}
		}
		
	if (fldraw)
		DrawMenuBar ();
	
	return (true);
	} /*flipcustommenu*/
	
	
void disableeditmenuitems (void) { /*DW 11/17/95*/ 
	
	disablemenuitem (hdleditmenu, undoitem);
	
	disablemenuitem (hdleditmenu, cutitem);
	
	disablemenuitem (hdleditmenu, copyitem);
	
	disablemenuitem (hdleditmenu, pasteitem);
	
	disablemenuitem (hdleditmenu, clearitem);
	
	disablemenuitem (hdleditmenu, selectallitem);
	
	disablemenuitem (hdleditmenu, fontitem);
	
	disablemenuitem (hdleditmenu, sizeitem);
	
	disablemenuitem (hdleditmenu, styleitem);
	
	disablemenuitem (hdleditmenu, justifyitem);
	} /*disableeditmenuitems*/


void initmenus (void) {

	installmenu (applemenu, &hdlapplemenu);
	
	if (hdlapplemenu != nil)
		AddResMenu (hdlapplemenu, 'DRVR');
	
	installmenu (filemenu, &hdlfilemenu);
	
	installmenu (editmenu, &hdleditmenu);
	
	(*app.insertmenucallback) ();
	
	installhierarchicmenu (fontmenu, &hdlfontmenu);
	
	if (hdlfontmenu != nil)
		AddResMenu (hdlfontmenu, 'FONT');
	
	installhierarchicmenu (sizemenu, &hdlsizemenu);
	
	installhierarchicmenu (stylemenu, &hdlstylemenu);
	
	installhierarchicmenu (justifymenu, &hdljustifymenu);
	
	initwindowmenu (); /*window menu is last menu*/
	
	DrawMenuBar ();
	} /*initmenus*/
	
	
	


