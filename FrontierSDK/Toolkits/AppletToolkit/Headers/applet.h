
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef appletinclude
#define appletinclude /*so other modules can tell if we've been included*/


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif


#ifndef opsinclude

	#include "appletops.h"
	
#endif


#ifndef cursorinclude

	#include "appletcursor.h"
	
#endif


#ifndef mouseinclude

	#include "appletmouse.h"
	
#endif


#ifndef kbinclude

	#include "appletkb.h"

#endif


#ifndef memoryinclude

	#include "appletmemory.h"

#endif


#ifndef appletfilesinclude

	#include "appletfiles.h"
	
#endif


#ifndef quickdrawinclude

	#include "appletquickdraw.h"

#endif


#ifndef stringsinclude

	#include "appletstrings.h"

#endif


#ifndef fontinclude

	#include "appletfont.h"

#endif


#ifndef undoinclude

	#include "appletundo.h"

#endif


#ifndef frontierinclude

	#include "appletfrontier.h"

#endif


#ifndef paletteinclude

	#include "appletpalette.h"

#endif


#ifndef appletwiresinclude

	#include "appletwires.h"
	
#endif


#ifndef __IAC__

	#include <iac.h>
	
#endif


#ifndef appletcardinclude

	#include "appletcard.h"

#endif
	





extern EventRecord appletevent; /*the last event received by appletmain.c*/


typedef struct tyselectioninfo {
	
	boolean fldirty; /*something changed, menus need updating*/
	
	boolean flcansetfont; /*can we set font in current context?*/
	
	boolean flcansetsize; /*can we set size?*/
	
	boolean flcansetstyle; /*can we set style?*/
	
	boolean flcansetjust; /*can we set justification?*/
	
	boolean flcansetleading; /*can we set leading?*/
	
	short fontnum;
	
	short fontsize;
	
	short leading;
	
	boolean flplain;
	
	boolean flshadow;
	
	boolean floutline;
	
	boolean flunderline;
	
	boolean flitalic;
	
	boolean flbold;
	
	boolean flsubscript;
	
	boolean flsuperscript;
	
	boolean flextended; 
	
	boolean flcondensed;
			
	tyjustification justification;
	
	short fontstyle; /*see setselectionstyleinfo*/
	} tyselectioninfo;
	
	
typedef boolean (*tysystemidlecallback) (long);

typedef boolean (*tyappcallback) (void);


typedef struct tyappwindow {
	
	tyfilespec filespec; /*the file being edited in the window*/
	
	short fnum; /*the file remains open while editing, this is the system file number*/
	
	WindowPtr macwindow; /*the Macintosh window pointer for the window*/
		
	short defaultfont, defaultsize, defaultstyle;
	
	tyjustification defaultjustification;
	
	short fontheight;
	
	Handle appdata;
	
	Rect windowrect;
	
	Rect contentrect, oldcontentrect;
	
	Rect statusrect; /*the rectangle covering the status portion of window*/
	
	Rect updaterect; /*when an update event is in process, this is the dirty part of the window*/
	
	hdlscrollbar vertbar, horizbar; /*the window's scrollbars*/
	
	boolean flactive; /*if true it's the frontmost/active window*/
	
	boolean flmadechanges;
	
	boolean flresetscrollbars;
	
	boolean flcolorwindow;
	
	boolean flzoomed;
	
	boolean flprinting;
	
	boolean fliacdata; /*if true appdata came via iac, not a file*/
	
	OSType iacdataowner; /*if fliacdata, this was the sender of the packed data*/
	
	AEDesc iacdatarefcon; /*if fliacdata, this is the owner's reference info for the data*/
	
	short zoomheight, zoomwidth; /*set by getcontentsizecallback*/
	
	Point scrollorigin;
	
	tyselectioninfo selectioninfo;
	
	RGBColor forecolor, backcolor; 
	
	short originpushdepth; /*see apppushorigin*/
	
	hdlpaletterecord hpalette; /*info about the palette for this window, nil if it has no palette*/

	Handle undostack, redostack;
	
	short windowvertpixels, windowhorizpixels;

	Handle hmessage; /*the string that's displayed in the message area, nil if no message*/	
	
	Rect messagerect; /*this is where we draw the window's message*/

	long filetype, creator; /*file type and creator for file saving*/
	
	tysystemidlecallback systemidlecallback; /*for interruptable idle-time processing, for clay basket*/
	
	long systemidlerefcon; /*passed to the callback routine*/
	
	long systemidleticks; /*how much of an idle wait to we need to see before calling the callback?*/
	
	long lastsystemidle; /*needed if the system is idle for more than systemidleticks ticks*/
	
	boolean systemidlecomplete; /*the idle handler sets this if he doesn't want to be called until after the user does something*/
	
	unsigned long idleatreset; /*number of idle ticks when the window was reset*/
	
	boolean notsaveable; /*some apps need to determine save-ability on a window level, not a global level*/
	
	short statuspixels; /*allow apps to have status bars for some windows and not for others*/
	
	tyappcallback msgclickcallback; /*handles clicks in the message part of the window*/
	
	short idmenu; /*this menu is inserted when the window is in front*/
	
	boolean flcloseonidle; /*if true, we close the window next time thru the main event loop*/
	
	boolean flzoombeforeopen; /*if true, we zoom-out the window before making it visible*/
	
	boolean touchparentsonsave; /*if true, we set the mod date of all containing folders when we save*/
	} tyappwindow, *ptrappwindow, **hdlappwindow;


#define app1class 'app1'

typedef boolean (*tyapphandleptrcallback) (Handle *);

typedef boolean (*tyapphandlecallback) (Handle);

typedef void (*tyapprectcallback) (Rect);

typedef boolean (*tyappbooleancallback) (boolean);

typedef boolean (*tyappshortcallback) (short);

typedef boolean (*tyappsscrollcallback) (tydirection, boolean, short);

typedef boolean (*tymenucallback) (short, short);

typedef boolean (*tystringcallback) (bigstring);

typedef boolean (*tyfilespeccallback) (ptrfilespec);

typedef boolean (*tyappgetundoglobalscallback) (long *);

typedef boolean (*tyappsetundoglobalscallback) (long, boolean);




typedef struct typrintinfo {
	
	Rect paperrect;
	
	short vpagepixels; /*number of pixels on a page, vertically*/
	
	short ctpages; /*the number of pages in the document being printed*/
	
	Handle printhandle; /*a handle to the machine's print record*/
	} typrintinfo;


typedef struct tyappletrecord {
	
	hdlappwindow appwindow; /*the frontmost applet window, callbacks work on this*/

	Handle appdata; /*a handle allocated by the applet to hold its data*/
	
	long filetype, creator; /*file type and creator for file saving*/
	
	sfcallback filefiltercallback; /*determines if files are displayed in stanard file dialog*/
	
	typrintinfo printinfo; /*for coordinating printing between applet and the shell*/
	
	short defaultfont, defaultsize, defaultstyle;
	
	boolean resizeable; /*if true the app's windows can be resized*/
	
	boolean eraseonresize; /*if true, the toolkit erases the window when it's resized*/
	
	boolean horizscroll; /*if true, the applet implements horiz scrolling*/
	
	boolean vertscroll; /*if true, the applet implements vert scrolling*/
	
	boolean haswindowmenu; /*if true, the applet wants a Window menu*/
	
	boolean haspalette; /*if true, the applet has an icon palette*/
	
	short ctpaletteicons; /*the number of icons in the palette*/
	
	short minwindowwidth, minwindowheight; /*constraints when resizing a window*/
	
	boolean notsaveable; /*if true, the applet doesn't implement saving/opening files*/
	
	short statuspixels; /*height of status bar at top of window*/
	
	boolean usecolor; /*if true, we allocate a color window on color machines*/
	
	boolean exactcolors; /*if true, we use the palette manager to be sure you get exact colors*/
	
	boolean appmanagesfontmenus; /*if true, the toolkit doesn't insert font/size/style sub-menus*/
	
	boolean insyseventhandler; /*true if the applet code is running in a call to a system event handler*/
	
	boolean dontpostcallback; /*see apppostcallback, DW 3/2/93*/
	
	boolean hasmessagearea; /*if true, we allocate 1/3 of the horiz scroll bar for a string msg*/
	
	boolean usetempmemory; /*if true, appletmemory uses temp memory instead of the application heap*/
	
	long systemidleticks; /*the number of ticks until system idle routine is called, 0, disables feature*/
	
	boolean openingfile; /*so newrecord routine can tell that a file is being opened -- hack!*/
	
	boolean commanderror; /*a serious error raised by the app, the toolkit stops what it's doing now*/
	
	tyappcallback newrecordcallback; /*allocate and initialize a new data handle*/
	
	tyappcallback disposerecordcallback; /*dealloc a data handle*/
	
	tyappcallback idlecallback; /*set mouse cursor, other "idle" stuff*/
	
	tyappbooleancallback activatecallback; /*activate or deactivate the window data*/
	
	tyappbooleancallback switchcallback; /*the app is being switched in or out*/
	
	tyappcallback updatecallback; /*update the display of the window data*/
	
	tyappcallback preupdatecallback; /*inval anything that needs inval'ing before an update*/
	
	tyappcallback windowresizecallback; /*recalc display data, adjust to new window size*/
	
	tyappcallback iacmessagecallback; /*process an IAC message*/
	
	tyappcallback iacfastmessagecallback; /*process a system event handler message*/
	
	tyapphandleptrcallback packcallback; /*pack memory structure into a disk handle*/
	
	tyapphandlecallback unpackcallback; /*unpack disk handle into memory structure*/
	
	tyapphandleptrcallback gettextcallback; /*return a handle of unformatted ASCII text*/
	
	tyapphandleptrcallback getpictcallback; /*return a Macintosh PicHandle of selected items*/
	
	tyappsscrollcallback scrollcallback;
	
	tyappcallback scrolltocallback;
	
	tyappcallback resetscrollcallback;
	
	tyappcallback pagesetupcallback;
	
	tyappcallback openprintcallback;
	
	tyappshortcallback printpagecallback;
	
	tyappcallback closeprintcallback;
	
	tyapphandlecallback puttextcallback;
	
	tyapphandlecallback putpictcallback;
	
	tyappcallback haveselectioncallback;
	
	tyappcallback selectallcallback;
	
	tyappcallback keystrokecallback;
	
	tyappcallback mousecallback;
	
	tyappcallback mouseinstatuscallback;
	
	tyappcallback updatestatuscallback;
	
	tymenucallback menucallback;
	
	tyappcallback insertmenucallback;
	
	tyappcallback setselectioninfocallback;
	
	tyappcallback setfontcallback;
	
	tyappcallback setsizecallback;
	
	tyappcallback setstylecallback;
	
	tyappcallback setjustifycallback;
	
	tyapphandleptrcallback copycallback; 
	
	tyappcallback clearcallback;
	
	tyapphandlecallback pastecallback;
	
	tyappcallback initmacintoshcallback;
	
	tyappbooleancallback resumecallback; /*suspend or resume the window*/
	
	tyapprectcallback eraserectcallback; /*color apps may want to override the default routine*/
	
	tyappcallback getcontentsizecallback; /*for apps that want to be smart about zooming*/
	
	tyappcallback openappcallback; /*for apps that want to open untitled window if no docs launched*/
	
	tyfilespeccallback opendoccallback; /*allow app to override 'odoc' Apple Event*/
	
	tyappcallback setglobalscallback; /*app can copy from app.appdata to its own globals*/
	
	tyapphandleptrcallback getoptionscallback;
	
	tyapphandlecallback putoptionscallback;
	
	tyappgetundoglobalscallback getundoglobalscallback;
	
	tyappsetundoglobalscallback setundoglobalscallback;
	
	tyappcallback afteropenwindowcallback;
	
	tyappcallback beforeclosewindowcallback;
	
	tyappcallback aftersavewindowcallback;
	
	tyappcallback beforequitcallback;
	
	tystringcallback alertcallback; 
	
	tyappshortcallback diskinsertedcallback;
	
	tyfilespeccallback interpretfilecallback;
	} tyappletrecord, *ptrappletrecord, **hdlappletrecord;
	
	
extern tyappletrecord app; /*global applet record, short name for easy access*/



#ifndef scrollbarinclude

	#include "appletscrollbar.h"

#endif



void appgetdesktoprect (hdlappwindow, Rect *);

boolean apppushwindow (hdlappwindow);

boolean apppopwindow (void);

boolean apppushorigin (void);

boolean apppoporigin (void);

boolean appopenbitmap (Rect, hdlappwindow);

boolean appclosebitmap (hdlappwindow);

boolean apppushclip (Rect r);

boolean apppopclip (void);

void appprecallback (void);

void apppostcallback (void);

boolean newappwindow (bigstring, boolean);

void invalappwindow (hdlappwindow, boolean);

void updateappwindow (hdlappwindow);

void moveappwindow (hdlappwindow, Rect);

boolean findbywindowtitle (bigstring, hdlappwindow *);

boolean findnthwindow (short, hdlappwindow *);

short countwindows (void);

boolean appletinitmanagers (void);

void runapplet (void);

boolean exitmainloop (void);

boolean closefrontwindow (void);

boolean initprint (void);

boolean pagesetup (void);

boolean printappwindow (hdlappwindow, boolean);

boolean getprintinfo (void);

pascal Boolean appscriptcomplete (void); /*call this when a script completes*/

boolean visitappwindows (boolean  (*) ());

boolean invalallwindows (void);

boolean eraseallwindows (void);

void setappwindow (hdlappwindow);

void apperaserect (Rect);

boolean apprectneedsupdate (Rect);

void appinvalstatusseparator (void);

void drawappgrowicon (hdlappwindow);

void setappwindowmadechanges (hdlappwindow);

#endif