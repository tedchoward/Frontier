
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

#include <uisharing.h>
#include <land.h>
#include "dialogs.h"
#include "strings.h"
#include "osainternal.h"
#include "osadroplet.h"

#ifdef dropletcomponent /*entire file from here*/

#include <SetUpA5.h>


pascal OSErr PlotIconID (Rect *r, short align, short transform, short resid) = {0x303C,0x0500,0xABC9};


#define kDropletRunAppSelect 1


#define iddropletstring 3000
#define iddropletdescriptionstring 3001


#define mainwindowid 3000
#define runitem 1
#define cancelitem 2
#define msgitem 3
#define edititem 4
#define iconitem 5
#define titleitem 6

#define aboutalertid 3001

#define erroralertid 3002

#define applemenu 3000
#define aboutitem 1

#define filemenu 3001
#define quititem 1


#define dropletglobals ((**hcg).dropletglobals)




static Handle getResourceAndDetach (OSType restype, short resid) { /*new in Frontier SDK 2.1*/

	Handle h;
	
	h = GetResource (restype, resid);
	
	if (h != nil)
		DetachResource (h);
		
	return (h);
	} /*getResourceAndDetach*/


static OSErr OSAnewverb (OSType receiver, OSType vclass, OSType vtoken, AppleEvent *event) {
	
	/*
	events we create will not be sent via the AE Manager; they'll be 
	passed to OSAExecuteEvent. So we use a null address & other dummy parameters
	*/
	
	AEDesc adr;
	OSErr ec;
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
		newdescnull (&adr, typeNull);
	
	#else
	
		adr.dataHandle = nil;
	
		adr.descriptorType = typeNull;
	
	#endif
	
	if (receiver != 0)
		ec = AECreateDesc (typeApplSignature, (Ptr) &receiver, sizeof (receiver), &adr);
	
	if (ec != noErr)
		return (ec);
	
	ec = AECreateAppleEvent (vclass, vtoken, &adr, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	AEDisposeDesc (&adr);
	
	return (ec);
	} /*OSAnewverb*/


static Boolean frontmostapp () {
	
	ProcessSerialNumber currentprocess, frontprocess;
	Boolean fl;
	
	GetCurrentProcess (&currentprocess);
	
	GetFrontProcess (&frontprocess);
	
	SameProcess (&currentprocess, &frontprocess, &fl);
	
	return (fl);
	} /*frontmostapp*/
	
	
static void cometofront (void) {

	if (!frontmostapp ()) {
		
		register short i;
		ProcessSerialNumber psn;
		EventRecord ev;
		
		GetCurrentProcess (&psn);
		
		SetFrontProcess (&psn);
		
		for (i = 1; i <= 3; i++)
			WaitNextEvent (nullEvent, &ev, 1, nil);	
		}
	} /*cometofront*/


static pascal Boolean alertuser (Str255 s) {
	
	cometofront ();
	
	ParamText (s, nil, nil, nil); 
	
	Alert (erroralertid, nil);
	
	return (true);
	} /*alertuser*/


static void doclosedown (void) {
	
	AppleEvent event, reply;
	
	if (OSAnewverb (0, 'aevt', 'quit', &event) == noErr)
		;
	} /*doclosedown*/


/*
static Boolean doinstall (void) {
	
	AEDesc desc;
	OSErr ec;
	Boolean flhavereply = false;
	AppleEvent event, reply;
	Str255 bserror;
	
	IACglobals.event = &event;
	
	desc.descriptorType = 'scpt';
	
	desc.dataHandle = getResourceAndDetach ('scpt', 128);
	
	if (ec != noErr)
		return (false);
	
	desc.descriptorType = 'mbar';
	
	desc.dataHandle = getResourceAndDetach ('mbar', 128);
	
	ec = AEPutParamDesc (&event, (AEKeyword) 'prm2', &desc);
	
	AEDisposeDesc (&desc);
	
	return (true);
	
	return (false);
	} /%doinstall%/
*/
	

static boolean getprocessfilespec (ProcessSerialNumber psn, FSSpec *fs) {
	
	ProcessInfoRec info;
	
	info.processInfoLength = (long) sizeof (info);
	
	info.processName = nil;
	
	info.processAppSpec = fs;
	
	return (GetProcessInformation (&psn, &info) == noErr);
	} /*getprocessfilespec*/


static Boolean dropletedit (void) {
	
	AEDesc desc;
	AppleEvent event, reply;
	ProcessSerialNumber psn;
	FSSpec fs;
	AliasHandle halias;
	OSErr ec;
	
	ec = OSAnewverb ('LAND', kCoreEventClass, kAEOpenDocuments, &event);
	
	if (ec != noErr)
		return (false);	
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		newdescnull (&reply, typeNull);
	
	#else
		
		reply.descriptorType = typeNull;

		reply.dataHandle = nil;
	
	#endif
	
	psn.highLongOfPSN = 0;
	
	psn.lowLongOfPSN = kCurrentProcess;
	
	getprocessfilespec (psn, &fs);
	
	ec = NewAliasMinimal (&fs, &halias);
	
	if (ec != noErr)
		goto exit;	
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		newdescwithhandle (&desc, typeAlias, (Handle) halias);
	
	#else
		
		desc.descriptorType = typeAlias;

		desc.dataHandle = (Handle) halias;
	
	#endif
	
	ec = AEPutParamDesc (&event, keyDirectObject, &desc); /*put alias on the event*/
	
	if (ec == noErr)
		ec = AESend (&event, &reply, kAENoReply + kAEAlwaysInteract + kAECanSwitchLayer, kAENormalPriority, (long) kNoTimeOut, nil, nil);
	
	exit:
	
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);
	
	return (ec == noErr);
	} /*dropletedit*/


static Boolean dropletrun (hdlcomponentglobals hcg) {
	
	AppleEvent event, reply;
	AEDesc scriptdesc;
	ComponentInstance comp;
	OSAID idscript;
	OSAError err;
	
	scriptdesc.descriptorType = 'scpt';
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle h;
		
		h = getResourceAndDetach ('scpt', 128);
		
		newdescwithhandle (h, 'scpt', &scriptdesc);
		
		disposehandle (h);
		
		if (AEGetDescDataSize (&scriptdesc) < 1)
		}
	
	#else
	
		scriptdesc.dataHandle = getResourceAndDetach ('scpt', 128);
	
		if (scriptdesc.dataHandle == nil) {
	
	#endif
		
		alertuser ("\pScript resource not found.");
		
		return (false);
		}
	
	comp = OpenDefaultComponent (kOSAComponentType, kOSAGenericScriptingComponentSubtype);
	
	if (comp == nil) {
		
		alertuser ("\pCouldnÕt open the scripting component");
		
		return (false);
		}
	
	err = OSALoad (comp, &scriptdesc, kOSANullMode, &idscript);
	
	AEDisposeDesc (&scriptdesc);
	
	if (err != noErr) {
		
		alertuser ("\pError opening the script");
		
		return (false);
		}
	
	event = dropletglobals.initialevent;
	
	err = OSADoEvent (comp, &event, idscript, kOSANullMode, &reply);
	
	doclosedown ();
	
	return (true);
	} /*dropletrun*/


#if 0

static Boolean appRunning (OSType appid) {
	
	/*
	return true if the server application is running. 
	*/
	
	ProcessInfoRec info;
	ProcessSerialNumber psn;
	Str255 bsname;
	FSSpec fss;
	
	info.processInfoLength = sizeof (info);
	
	info.processName = bsname; /*place to store process name*/
	
	info.processAppSpec = &fss; /*place to store process filespec*/
	
	psn.highLongOfPSN = kNoProcess;
	
	psn.lowLongOfPSN = kNoProcess;
	
	while (GetNextProcess (&psn) == noErr) {
		
	 	info.processInfoLength = sizeof (ProcessInfoRec);
	 	
		if (GetProcessInformation (&psn, &info) != noErr)
			continue; /*keep going -- ignore error*/
		
		if (info.processSignature == appid)
			return (true);
		} /*while*/
	
	return (false); /*loop completed, no server*/
	} /*appRunning*/

static Boolean launchApp (FSSpec *fs) {
	
	LaunchParamBlockRec pb;
	OSErr errcode;
	
	clearbytes (&pb, (long) sizeof (pb));
	
	pb.launchAppSpec = fs;
	
	pb.launchBlockID = extendedBlock;
	
	pb.launchEPBLength = extendedBlockLen;
	
	pb.launchControlFlags = launchContinue | launchNoFileFlags;
	
	return (LaunchApplication (&pb) == noErr);
	} /*launchApp*/


static Boolean launchServer (OSType serverid) {
	
	/*
	if the application whose creator id is serverid is running, return true.
	
	if not, we look for the application and try to launch it. we wait until it's 
	actually running and ready to receive Apple Events.
	*/
	
	FSSpec fs;
	
	if (appRunning (serverid))
		return (true);
		
	if (!findapplication (serverid, &fs))
		return (false);
		
	if (!launchApp (&fs))
		return (false);
		
	while (!appRunning (serverid)) {
		
		EventRecord ev;
		
		EventAvail (everyEvent, &ev);
		} /*while*/
		
	return (true);
	} /*launchServer*/
	
#endif

static pascal void drawmessageitem (DialogPtr pdialog, short itemnumber) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	short savedfont, savedsize;
	Rect r;
	PenState savedpen;
	short fnum;
	Handle htext;
	
	long curA5 = SetUpAppA5 ();
	
	htext = getResourceAndDetach ('TEXT', 128);
	
	RestoreA5 (curA5);
	
	savedfont = (*thePort).txFont;
	
	savedsize = (*thePort).txSize;
	
	GetFNum ("\pPalatino", &fnum);
	
	TextFont (fnum);
	
	TextSize (12);
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the itemÕs rect*/
	
	r = itemrect;
	
	InsetRect (&r, 3, 3);
	
	TextBox (*htext, GetHandleSize (htext), &r, 0);
	
	TextFont (savedfont);
	
	TextSize (savedsize);
	
	GetPenState (&savedpen); /*save the old pen state*/
	
	PenPat (gray);
	
	FrameRect (&itemrect);
	
	SetPenState (&savedpen);
	} /*drawmessageitem*/
	

static void drawitemstring (DialogPtr pdialog, short itemnumber, Str255 s) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	short savedfont, savedsize, savedstyle;
	Rect r;
	PenState savedpen;
	FontInfo info;
	RgnHandle rgn;
	
	savedfont = (*thePort).txFont;
	
	savedsize = (*thePort).txSize;
	
	savedstyle = (*thePort).txFace;
	
	TextFont (helvetica);
	
	TextSize (18);
	
	TextFace (bold);
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the itemÕs rect*/
	
	r = itemrect;
	
	GetFontInfo (&info);
	
	MoveTo (r.left, r.bottom - info.descent);
	
	GetClip (rgn = NewRgn ());
	
	ClipRect (&r);
	
	DrawString (s);
	
	SetClip (rgn);
	
	DisposeRgn (rgn);
	
	TextFont (savedfont);
	
	TextSize (savedsize);
	
	TextFace (savedstyle);
	} /*drawitemstring*/


static pascal void drawtitleitem (DialogPtr pdialog, short itemnumber) {
	
	Str255 titlestring;
	long appA5;
	
	GetWTitle (pdialog, titlestring);
	
	curA5 = SetUpAppA5 ();
	
	insertstring ("\pThe Ò", titlestring);
	
	pushstring ("\pÓ Droplet", titlestring);
	
	drawitemstring (pdialog, itemnumber, titlestring);
	
	RestoreA5 (curA5);
	} /*drawtitleitem*/


static pascal void drawiconitem (DialogPtr pdialog, short itemnumber) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the itemÕs rect*/
	
	PlotIconID (&itemrect, 0, 0, 128);
	} /*drawiconitem*/


static void handledrag (EventRecord *ev, WindowPtr w) {
	
	Rect r;

	r = qd.screenBits.bounds; 
	
	r.top = r.top + GetMBarHeight (); 
	
	InsetRect (&r, 4, 4);
	
	DragWindow (w, (*ev).where, &r);
	} /*handledrag*/


static void handlemenu (hdlcomponentglobals hcg, long codeword) {
	
	register short idmenu, iditem;
	
	iditem = LoWord (codeword);
	
	idmenu = HiWord (codeword);
	
	if (uisSharedMenuHit (idmenu, iditem)) /*See Step #3*/	
		goto exit;
	
	switch (idmenu) {
	
		case applemenu: 
			switch (iditem) {
				
				case aboutitem:
					Alert (aboutalertid, nil);
					
					break;
				
				default: {
				
					Str255 s;
					
					GetItem (dropletglobals.happlemenu, iditem, s);
					
					OpenDeskAcc (s);
					
					break;
					}
				} /*switch*/
			
			break; /*apple menu*/

		case filemenu: 
			switch (iditem) {
				
				case quititem:
				
					dropletglobals.flexitmainloop = true;
					
					break;
				} /*switch*/
			
			break; /*file menu*/
			
		} /*switching on which menu was invoked*/
		
	exit:
	
	HiliteMenu (0);
	} /*handlemenu*/


static void handlemouse (hdlcomponentglobals hcg, EventRecord *ev) {

	register short part;
	WindowPtr w;
	
	part = FindWindow ((*ev).where, &w);
	
	if (w != nil) 
	
		if (w != FrontWindow ()) { /*just like all other Mac programs*/
			
			SelectWindow (w);
							
			return; /*the mouse click is consumed by the bringtofront operation*/
			}
	
	switch (part) {
	
		case inMenuBar: 
			handlemenu (hcg, MenuSelect ((*ev).where)); 
			
			break;
		
		case inSysWindow:
			SystemClick (ev, w); 
			
			break;
		
		case inDrag:
			handledrag (ev, w);
			
			break;
			
		} /*switch*/
	} /*handlemouse*/

#if 0
	
static void highlightbutton (short itemnumber, Boolean flon) {
	
	short x;
	short itemtype;
	Handle itemhandle;
	Rect itembox;
	
	GetDItem (w, itemnumber, &itemtype, &itemhandle, &itembox);
	
	if (flon)
		x = inButton;
	else
		x = 0;
	
	HiliteControl ((ControlHandle) itemhandle, x); 
	} /*highlightbutton*/

	
static void simulatehit (short item) {
	
	highlightbutton (item, true);
	
	delayticks (8);
	
	highlightbutton (item, false);
	} /*simulatehit*/

#endif

static Boolean handlekeystroke (hdlcomponentglobals hcg, EventRecord *ev) { 

	char ch = (*ev).message & charCodeMask;
	Boolean flcmdperiod = false;
	
	#if 0
	
	if (SharedScriptRunning ()) { /*cmd-period terminates the script*/
	
		if (((*ev).modifiers & cmdKey) && (ch == '.')) { 
			
			flcmdperiod = true;
			
			CancelSharedScript (); /*cancel the shared menu script, if one is running*/
			
			return (true);
			}
		}
	
	if ((ch == (char) 13) || (ch == (char) 3) || (ch == 'r') || (ch == 'R')) { 
		
		/*user hit Return, Enter, r or R*/
		
		if (false /*fl2click****/) /*Run button not enabled*/
			return (true);
		
		simulatehit (runitem);
	
		DisposDialog (w);
	
		w = nil;
		
		dropletrun ();
		
		return (false);
		}
		
	if ((flcmdperiod) || ((ch == 'q') || (ch == 'Q'))) { /*close the window and exit*/
	
		simulatehit (cancelitem);
		
		return (false);
		}
		
	if ((ch == 'e') || (ch == 'E')) {
		
		simulatehit (edititem);
		
		if (dropletedit ();
		
		return (true);
		}
	
	#endif
	
	if ((*ev).modifiers & cmdKey)
		handlemenu (hcg, MenuKey (ch)); 
	
	return (true);
	} /*handlekeystroke*/


static void initdropletmenus (hdlcomponentglobals hcg) {
	
	/*
	set up our apple and file menus.  nothing fancy.
	*/
	
	register MenuHandle hmenu;
	
	long appA5 = SetUpCurA5 ();
	
	hmenu = GetMenu (applemenu);
	
	if (hmenu != nil) {
		
		dropletglobals.happlemenu = hmenu;
		
		AddResMenu (hmenu, 'DRVR'); 
		
		InsertMenu (hmenu, 0); 
		}
	
	hmenu = GetMenu (filemenu);
	
	if (hmenu != nil) {
		
		dropletglobals.hfilemenu = hmenu;
		
		InsertMenu (hmenu, 0);
		}
	
	DrawMenuBar ();
	
	RestoreA5 (appA5);
	} /*initdropletmenus*/


static boolean initdropletwindow (hdlcomponentglobals hcg, boolean fl2click) {
	
	ProcessSerialNumber psn;
	Str255 appname;
	DialogPtr w;
	boolean flignore;
	long appA5;
	
	psn.highLongOfPSN = 0;
	
	psn.lowLongOfPSN = kCurrentProcess;
	
	getprocessname (psn, appname, &flignore);
	
	appA5 = SetUpCurA5 ();
	
	w = GetNewDialog (mainwindowid, nil, (WindowPtr) -1);
	
	RestoreA5 (appA5);
	
	dropletglobals.pmainwindow = w;
	
	//Code change by Timothy Paustian Wednesday, August 23, 2000 9:21:06 PM
	//
	{
	#if TARGET_API_MAC_CARBON
	CGrafPtr	thePort = GetWindowPort(w);
	#else
	GrafPtr	thePort = (GrafPtr)w;
	#endif

	SetPort (thePort);
	}
	
	SetWTitle (w, appname);
	
	if (fl2click) 					
		disabledialogitem (w, runitem);
	else
		setdefaultitem (w, runitem);
	
	#if flruntime
	
	disabledialogitem (w, edititem); /*disable Edit button if we're Runtime*/
	
	#endif
	
	setuseritemdrawroutine (w, iconitem, &drawiconitem);
	
	setuseritemdrawroutine (w, titleitem, &drawtitleitem);
	
	setuseritemdrawroutine (w, msgitem, &drawmessageitem);
	
	ShowWindow (w);
	
	return (true);
	} /*initdropletwindow*/


static pascal OSErr droplethandleopendoc (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	register hdlcomponentglobals hcg = (hdlcomponentglobals) refcon;
	AEDesc desc;
	OSErr ec;
	
	ec = AEDuplicateDesc (event, &desc);
	
	dropletglobals.initialevent = desc;
	
	initdropletwindow (hcg, false); /*it was a drag-and-drop*/
	
	return (ec);
	} /*droplethandleopendoc*/


static pascal OSErr droplethandleopenapp (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	register hdlcomponentglobals hcg = (hdlcomponentglobals) refcon;
	AEDesc desc;
	OSErr ec;
	
	ec = AEDuplicateDesc (event, &desc);
	
	dropletglobals.initialevent = desc;
	
	initdropletwindow (hcg, true); /*launched by 2clicking*/
	
	return (ec);
	} /*droplethandleopenapp*/


static pascal OSErr droplethandlewildcard (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	register hdlcomponentglobals hcg = (hdlcomponentglobals) refcon;
	
	initdropletwindow (hcg, true); /*launched by 2clicking*/
	
	return (noErr);
	} /*droplethandleopenapp*/


static OSErr dropletmaineventloop (hdlcomponentglobals hglobals) {
	
	register hdlcomponentglobals hcg = hglobals;
	DialogPtr w = dropletglobals.pmainwindow;
	EventRecord ev;
	short itemhit;
	boolean fl;
	boolean flinstalled = false;
	Boolean fltoss;
	long appA5;
	
	while (!dropletglobals.flexitmainloop) {
		
		SetCursor (&arrow);
		
//		osapreclientcallback (hcg);
		
		appA5 = SetUpCurA5 ();
		
		fl = WaitNextEvent (everyEvent, &ev, 30, nil);
		
		RestoreA5 (appA5z);
		
//		osapostclientcallback (hcg);
		
		if (fl) {
			
			if (dialogevent (&ev, w, &itemhit)) {
				
				switch (itemhit) {
					
					case runitem:
						DisposDialog (w);
						
						w = nil;
						
						dropletrun (hcg);
						
						goto endloop;
					
					case cancelitem:
						goto endloop;
					
					case edititem:
						if (dropletedit ())
							goto endloop;
						
						break;
					} /*switch*/
				} /*dialog event*/
			
			else { /*not a dialog event*/
				
				switch (ev.what) {	

					case mouseDown:
						handlemouse (hcg, &ev);
						
						break;
					
					case kHighLevelEvent:
						AEProcessAppleEvent (&ev);
						
						break;
					} /*switch*/
				}
			} /*WNE returned true*/
		else {
			/*		
			if (!flinstalled) {
				
				flinstalled = true;
				
				doinstall ();
				}
			*/
			
			uisHandleEvent (&ev, &fltoss);
			}
		} /*while*/
	
	endloop:
	
	if (w != nil)
		DisposDialog (w);
	
	return (noErr);
	} /*dropletmaineventloop*/


static pascal ComponentResult rundropletcommand (hdlcomponentglobals hglobals) {
	
	OSErr err;
	
	long appA5 = SetUpCurA5 ();
	
	err = AEInstallEventHandler (kCoreEventClass, kAEOpenApplication, (ProcPtr) &droplethandleopenapp, (long) hglobals, false);
	
	if (err == noErr)
		err = AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments, (ProcPtr) &droplethandleopendoc, (long) hglobals, false);
	
	if (err == noErr)
		err = AEInstallEventHandler (typeWildCard, typeWildCard, (ProcPtr) &droplethandlewildcard, (long) hglobals, false);
	
	RestoreA5 (appA5);
	
	if (err != noErr)
		return (err);
	
	initdropletmenus (hglobals);
	
	uisInit (&alertuser, 140, 0, noWindowSharing);
	
	err = dropletmaineventloop (hglobals);
	
	uisClose ();
	
	return (err);
	} /*rundropletcommand*/


static pascal ComponentResult dropletcandofunction (short selector) {
	
	switch (selector) {
		
		case kComponentOpenSelect:
		case kComponentCloseSelect:
		case kComponentCanDoSelect:
		case kComponentVersionSelect:
		
		case kDropletRunAppSelect:
			return (true);
		}
	
	return (false);
	} /*dropletcandofunction*/


static pascal ComponentResult dropletdispatch (register ComponentParameters *params, Handle storage) {
	
	ComponentResult result = noErr;
	short what = (*params).what;
	
	long curA5 = SetUpAppA5 ();
	
	switch (what) {
		
		case kComponentOpenSelect: { /*can't call subroutine because a5 isn't ours*/
			
			hdlcomponentglobals hglobals;
			Component self = (Component) (*params).params [0];
			long selfa5;
			long clienta5;
			
			/*
			selfa5 = GetComponentRefcon (self);
			
			asm {
				move.l	a5,clienta5
				move.l	a5,-(a7)
				move.l	selfa5,a5
				}
			
			SetComponentInstanceA5 ((ComponentInstance) self, selfa5);
			*/
			
			if (newcomponentglobals (self, clienta5, &hglobals))
				SetComponentInstanceStorage ((ComponentInstance) self, (Handle) hglobals);
			else
				result = memFullErr;
			
			/*
			asm {
				move.l	(a7)+,a5
				}
			*/
			
			break;
			}
		
		case kComponentCloseSelect:
			disposecomponentglobals ((hdlcomponentglobals) storage);
			
			result = noErr;
			
			break;
		
		case kComponentCanDoSelect:
			result = CallComponentFunction (params, &dropletcandofunction))
			break;
		
		case kComponentVersionSelect:
			result = 0x04100100;
			
			break;
		
		case kDropletRunAppSelect:
			result = callosafunction (storage, params, &rundropletcommand);
			
			break;
		
		default:
			result = badComponentSelector;
			
			break;
		}
	
	RestoreA5 (curA5);
	
	return (result);
	} /*dropletdispatch*/


boolean initdropletcomponent (void) {
	
	/*
	register the window sharing component.
	*/
	
	long result;
	ComponentDescription desc;
	bigstring bs;
	Handle hname, hdescription, hicon;
	Component comp;
	
	#if !TARGET_API_MAC_CARBON
	RememberA5 ();
	#endif
	
	desc.componentType = 'aplt';
	
	desc.componentSubType = 'LAND';
	
	desc.componentManufacturer = 'LAND';
	
	desc.componentFlags = 0;
	
	desc.componentFlagsMask = 0;
	
	hname = (Handle) GetString (iddropletstring);
	
	hdescription = (Handle) GetString (iddropletdescriptionstring);
	
	hicon = GetIcon (129); /****should be droplet icon*/
	
	comp = RegisterComponent (&desc, NewComponentRoutineProc (dropletdispatch), true, hname, hdescription, hicon);
	
	if (comp == nil)
		return (false);
	
	SetComponentRefcon (comp, (long) CurrentA5);
	
	return (true);
	} /*initdropletcomponent*/

#endif

