
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


/*
11/14/93 DW: last minute pre-3.0 change -- the Run button is always
enabled. we were making it conditional on whether any files were dropped
onto the app. the Upgrader wants to become a droplet, and it doesn't
get anything dropped on it. seems that it was hasty to assume that droplets
might want to run even if nothing was dropped on it. why not use a desktop
script you ask? droplets do a lot more for you!

7/19/94 dmb: Updated for Univeral Headers
*/

#include <AppleEvents.h>
#include <Aliases.h>
#include <GestaltEqu.h>
#include <Folders.h>
#include <Icons.h>
#include <iac.h>
#include <menusharing.h>


static DialogPtr mainwindow = nil;


#define modelessid 130
#define runitem 1
#define cancelitem 2
#define msgitem 3
#define edititem 4
#define iconitem 5
#define titleitem 6
	
#define applemenu 128
#define aboutitem 1

#define filemenu 129
#define quititem 1

static MenuHandle happlemenu, hfilemenu;

Boolean flexitmainloop = false;


typedef unsigned char byte;

static Boolean faceless = false; /*Droplet 3.0 -- 10/1/93 DW*/

static Str255 apppath; /*the full path to the droplet application*/

static Str255 appname; /*just the name of the file, no path*/

static Boolean flopenhandled = false; /*when true we fall thru the main event loop*/

static AEDesc filelist; /*holds the list of files to be processed*/

static Boolean fl2click = true; /*true if we're launched by 2clicking*/
	
static Str255 titlestring; /*The "XXX" Droplet*/

static Handle helptext; /*text that's displayed inside the box*/

static Boolean flcurrentlyactive = true; /*are we the active process, based on OS events*/

static Boolean flinstalled = false; /*true, once we've installed the table and menu in Frontier*/




static void initmenus (void) {
	
	/*
	set up our apple and file menus.  nothing fancy.
	*/
	
	happlemenu = GetMenu (applemenu); 
	
	AddResMenu (happlemenu, 'DRVR'); 
	
	InsertMenu (happlemenu, 0); 
	
	hfilemenu = GetMenu (filemenu); 
	
	InsertMenu (hfilemenu, 0);
	
	DrawMenuBar ();
	} /*initmenus*/
	
	
static void boldenbutton (DialogPtr pdialog, short itemnumber) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	PenState savedpen;
	
	GetPenState (&savedpen); /*save the old pen state*/
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the item’s rect*/
	
	InsetRect (&itemrect, -4, -4);
	
	PenSize (3, 3); /*make the pen fatter*/
	
	FrameRoundRect (&itemrect, 16, 16); /*draw the ring*/

	SetPenState (&savedpen); /*restore the pen state*/
	} /*boldenbutton*/
	
	
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
	
	
static void notifyuser (Str255 s) {
	
	DialogPtr pdialog;
	short itemnumber;
	
	cometofront ();
	
	ParamText (s, "\p", "\p", "\p"); 
	
	pdialog = GetNewDialog (129, nil, (DialogPtr) -1L);
	
	ShowWindow (pdialog);	
	
	SetPort (pdialog);
	
	boldenbutton (pdialog, 1);

	SysBeep (1); /*multimedia!*/
	
	ModalDialog (nil, &itemnumber);

	DisposDialog (pdialog);
	} /*notifyuser*/


static Boolean getFaceless (void) {
	
	/*
	return true if this is a faceless droplet
	*/
	
	StringHandle hstring;
	
	hstring = GetString (129); 
	
	return (hstring != nil);
	} /*getFaceless*/
	
	
static Handle getResourceAndDetach (OSType restype, short resid, Boolean errorifnil) { 

	Handle h;
	
	h = GetResource (restype, resid);
	
	if (h != nil)
		DetachResource (h);
	else {
		if (errorifnil) {
		
			notifyuser ("\pError loading a resource. Suggestion: increase memory for the droplet application using the Finder and try again.");
		
			ExitToShell ();
			}
		}
		
	return (h);
	} /*getResourceAndDetach*/
	
	
static void copystring (void *source, void *dest) {

	/*
	create a copy of source in dest.  copy the length byte and
	all the characters in the source string.

	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	register short n;
	register byte *s = source;
	register byte *d = dest;
	
	n = (short) s [0];
	
	while (n-- >= 0)
		*d++ = *s++;
	} /*copystring*/


static Boolean pushstring (Str255 bssource, Str255 bsdest) {

	register short lensource = bssource [0];
	register short lendest = bsdest [0];
	register byte *psource, *pdest;
	
	if ((lensource + lendest) > 255)
		return (false);
	
	pdest = (byte *) bsdest + (byte) lendest + 1;
	
	psource = (byte *) bssource + 1;
	
	bsdest [0] += (byte) lensource;
	
	while (--lensource >= 0)
		*pdest++ = *psource++;
	
	return (true);
	} /*pushstring*/
	
	
static Boolean pushlong (long num, Str255 bsdest) {

	Str255 bsint;
	
	NumToString (num, bsint);
	
	return (pushstring (bsint, bsdest));
	} /*pushlong*/
	

static void clearbytes (void *pclear, long ctclear) {
	
	/*
	do a mass memory clear
	*/
	
	register byte *p = pclear;
	register long ct = ctclear;
	
	while (--ct >= 0)
		*p++ = (byte) 0; /*tight loop*/
	} /*clearbytes*/


typedef struct versionRecord {

	unsigned short majorRev: 8; 
	
	unsigned short minorRev: 4;	
	
	unsigned short bugFixRev: 4; 
	
	unsigned short reserved: 15;
	
	unsigned short flFrontier: 1; /*true if it's Frontier, false if it's Runtime*/
	} versionRecord;


static Boolean getFrontierVersion (short *majorRev, short *minorRev, short *bugFixRev, Boolean *flRuntime) {
	
	/*
	if Frontier isn't running, return false.
	
	if it is, return true with information about the version of Frontier that's running.
	
	About the Apple Event we send...
	
	We call a system event handler, so it's very fast. 
	
	It takes no parameters, and returns a long value. The high word of the long is the version 
	number, packed the same way as the system version is packed into the SysEnvirons record. 
	(8 bits major version, 4 bits minor version, 4 bits revision. The version 2.1.1 would 
	be 0x0211.) The low word contains attributes of the server program. At this point only 
	a single bit is defined: the low order bit is set if Frontier is the server; otherwise, 
	Runtime is the server.
	*/
	
	AppleEvent event, reply;
	Boolean flhavereply = false;
	versionRecord x;
	
	if (!IAChandlerinstalled ('LAND', 'who?', true)) { /*it's Frontier 1.0, not Runtime*/
		
		*majorRev = 1;
		
		*minorRev = 0;
		
		*bugFixRev = 0;
		
		*flRuntime = false;
		
		return (true);
		}

	if (!IACnewsystemverb ('LAND', 'who?', &event))
		return (false);
	
	if (!IACsendverb (&event, &reply))
		goto error;
	
	flhavereply = true;
	
	IACglobals.reply = &reply;
	
	IACglobals.event = &reply; /*get the string from the reply record*/
		
	if (!IACgetlongparam ('----', (long *) &x))
		goto error;
	
	*majorRev = x.majorRev;
	
	*minorRev = x.minorRev;
	
	*bugFixRev = x.bugFixRev;
	
	*flRuntime = !x.flFrontier;
	
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	if (flhavereply)
		AEDisposeDesc (&reply);
	
	return (false);
	} /*getFrontierVersion*/


static Boolean IACstringtotext (Str255 s, Handle *htext) {
	
	register long len;
	register Handle h;
	
	len = s [0];
	
	h = NewHandle (len);
	
	if (h == nil)
		return (false);
		
	BlockMove (&s [1], *h, len);
	
	*htext = h;
	
	return (true);
	} /*IACstringtotext*/
	
	
static Boolean IACcheckerror (Str255 errorstring) {
	
	if (IACiserrorreply (errorstring)) {
		
		notifyuser (errorstring);
		
		return (true);
		}
		
	return (false);
	} /*IACcheckerror*/
	
	
static Boolean isfolder (Str255 fullpath) {
	
	CInfoPBRec pb;
	OSErr ec;
	
	clearbytes (&pb, (long) sizeof (pb)); /*init all fields to zero*/
	
	pb.hFileInfo.ioNamePtr = fullpath;

	pb.hFileInfo.ioVRefNum = 0;
	
	ec = PBGetCatInfo (&pb, false);
	
	if (ec != noErr)
		return (false);
	
	return (BitTst (&pb.dirInfo.ioFlAttrib, 3));
	} /*isfolder*/


static Boolean directorytopath (long dirid, short vnum, Str255 path) {
	
	CInfoPBRec block;
	Str255 bsdirectory;
	OSErr errcode;
	
	path [0] = (byte) 0; /*0-length string*/
	
	clearbytes (&block, (long) sizeof (block));
	
	block.dirInfo.ioNamePtr = bsdirectory;
	
	block.dirInfo.ioDrParID = dirid;
	
	do {
		block.dirInfo.ioVRefNum = vnum;
		
		block.dirInfo.ioFDirIndex = -1;
		
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		
		errcode = PBGetCatInfo (&block,false);
		
		if (errcode != noErr)
			return (false);
			
		if (!pushstring ("\p:", bsdirectory))
			return (false);
		
		if (!pushstring (path, bsdirectory))
			return (false);
		
		copystring (bsdirectory, path);
		} while (block.dirInfo.ioDrDirID != fsRtDirID);
	
	return (true);
	} /*directorytopath*/


static Boolean getfullpath (FSSpec *fs, Str255 fullpath) {

	if ((*fs).parID == 1) { /*it's a volume*/
		
		copystring ((*fs).name, fullpath);
		
		pushstring ("\p:", fullpath);
		}
	else {
		if (!directorytopath ((*fs).parID, (*fs).vRefNum, fullpath)) 
			return (false);
		
		pushstring ((*fs).name, fullpath);
		
		if (isfolder (fullpath))
			pushstring ("\p:", fullpath);		
		}
		
	return (true);
	} /*getfullpath*/
	

static OSType GetProcessFullPath (void) {
	
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	FSSpec fss;
	
	GetCurrentProcess (&psn);
	
	info.processInfoLength = (long) sizeof (info);
	
	info.processName = appname; /*place to store process name*/
	
	info.processAppSpec = &fss; /*place to store process filespec*/
	
	GetProcessInformation (&psn, &info);
	
	return (getfullpath (&fss, apppath));
	} /*GetProcessFullPath*/
	
	
static Boolean processfile (Str255 fullpath) {

	Boolean flhavereply = false;
	AppleEvent event, reply;
	Str255 bserror;
	
	if (!IACnewverb ('LAND', 'OHIO', 'proc', &event))
		return (false);
	
	IACglobals.event = &event;
	
	if (!IACpushstringparam (fullpath, '----'))
		return (false);
		
	if (!IACsendverb (&event, &reply))
		goto error;
	
	flhavereply = true;
	
	IACglobals.reply = &reply;
	
	if (IACcheckerror (bserror)) 
		goto error;
		
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	if (flhavereply)
		AEDisposeDesc (&reply);
	
	return (false);
	} /*processfile*/
	
	
static void doclosedown (void) {
	
	AppleEvent event, reply;
	
	if (IACnewverb ('LAND', 'OHIO', 'clos', &event))	
		IACsendverb (&event, &reply);
	} /*doclosedown*/


static Boolean doinstall (void) {

	AEDesc desc;
	OSErr ec;
	Boolean flhavereply = false;
	AppleEvent event, reply;
	Str255 bserror;
		
	if (!IACnewverb ('LAND', 'OHIO', 'init', &event))
		return (false);
		
	IACglobals.event = &event;
	
	desc.descriptorType = 'scpt';
	
	desc.dataHandle = getResourceAndDetach ('scpt', 128, true);
	
	ec = AEPutParamDesc (&event, (AEKeyword) 'prm1', &desc);
	
	AEDisposeDesc (&desc);
	
	if (ec != noErr)
		return (false);
	
	desc.descriptorType = 'mbar';
	
	desc.dataHandle = getResourceAndDetach ('mbar', 128, false);
	
	ec = AEPutParamDesc (&event, (AEKeyword) 'prm2', &desc);
	
	AEDisposeDesc (&desc);
	
	if (ec != noErr)
		return (false);
	
	if (!IACpushstringparam (apppath, 'prm3'))
		return (false);
		
	if (!IACsendverb (&event, &reply))
		goto error;
	
	flhavereply = true;
	
	IACglobals.reply = &reply;
	
	if (IACcheckerror (bserror)) /*syntax error or runtime error*/
		goto error;
		
	IACglobals.event = &reply; /*get the string from the reply record*/
		
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	if (flhavereply)
		AEDisposeDesc (&reply);
	
	return (false);
	} /*doinstall*/
	
	
static Boolean dohelptext (void) {

	AEDesc desc;
	OSErr ec;
	Boolean flhavereply = false;
	AppleEvent event, reply;
	Str255 bserror;
	
	if (!IACnewverb ('LAND', 'OHIO', 'help', &event))
		return (false);
		
	IACglobals.event = &event;
	
	desc.descriptorType = 'wptx';
	
	desc.dataHandle = getResourceAndDetach ('wptx', 128, false);
	
	ec = AEPutParamDesc (&event, (AEKeyword) '----', &desc);
	
	AEDisposeDesc (&desc);
	
	if (ec != noErr)
		return (false);
		
	desc.descriptorType = 'TEXT'; /*DW 7/21/92 -- for Runtime, push plain text on the AE*/
	
	desc.dataHandle = getResourceAndDetach ('TEXT', 128, true);
	
	ec = AEPutParamDesc (&event, (AEKeyword) 'RUNT', &desc);
	
	AEDisposeDesc (&desc);
	
	if (ec != noErr)
		return (false);
		
	if (!IACsendverb (&event, &reply))
		goto error;
	
	flhavereply = true;
	
	IACglobals.reply = &reply;
	
	if (IACcheckerror (bserror)) /*syntax error or runtime error*/
		goto error;
		
	IACglobals.event = &reply; /*get the string from the reply record*/
		
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	if (flhavereply)
		AEDisposeDesc (&reply);
	
	return (false);
	} /*dohelptext*/
	
	
static Boolean doedit (void) {

	Boolean flhavereply = false;
	AppleEvent event, reply;
	Str255 bserror;
	
	if (!IACnewverb ('LAND', 'OHIO', 'edit', &event))
		return (false);
		
	if (!IACsendverb (&event, &reply))
		goto error;
	
	flhavereply = true;
	
	IACglobals.reply = &reply;
	
	if (IACcheckerror (bserror)) /*syntax error or runtime error*/
		goto error;
		
	IACglobals.event = &reply; /*get the string from the reply record*/
		
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);
	
	flexitmainloop = true;
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	if (flhavereply)
		AEDisposeDesc (&reply);
	
	return (false);
	} /*doedit*/
	
	
static Boolean dofilelist (void) {
	
	long ctfiles;
	DescType actualtype;
	long actualsize;
	AEKeyword actualkeyword;
	FSSpec fs;
	long i;
	OSErr ec;
	Str255 fullpath;
	
	flopenhandled = true;
	
	fl2click = false; /*it was a drag-and-drop*/
	
	ec = AECountItems (&filelist, &ctfiles);
	
	if (ec != noErr) 
		return (ec);
	
	if (ctfiles <= 0)
		return (noErr);
	
	for (i = 1; i <= ctfiles; i ++) {
		
		ec = AEGetNthPtr (
			&filelist, i, typeFSS, &actualkeyword, &actualtype, 
			
			(Ptr) &fs, sizeof (fs), &actualsize);
		
		getfullpath (&fs, fullpath);
	
		if (!processfile (fullpath)) {
			
			doclosedown ();
			
			return (-1);
			}
		} /*for*/
	
	doclosedown ();
	
	return (noErr);
	} /*dofilelist*/


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


static Boolean hasdesktopmanager (short vnum) {
	
	GetVolParmsInfoBuffer volparms;
	HParamBlockRec pb;
	
	pb.volumeParam.ioVRefNum = vnum;
	
	pb.volumeParam.ioNamePtr = nil;
	
	pb.ioParam.ioBuffer = (Ptr) &volparms;
	
	pb.ioParam.ioReqCount = sizeof (volparms);
	
	if (PBHGetVolParmsSync (&pb) != noErr)
		return (false);
	
	return ((volparms.vMAttrib & (1 << bHasDesktopMgr)) != 0);
	} /*hasdesktopmanager*/


static Boolean findApp (OSType appid, FSSpec *fs) {
	
	/*
	find the app whose creator id is appid. return true if we found it, 
	false otherwise.
	*/
	
	DTPBRec dt;
	ParamBlockRec pb;
	Str255 appfname;
	
	clearbytes (&pb, (long) sizeof (pb));
	
	while (true) {
		
		++pb.volumeParam.ioVolIndex;
		
		if (PBGetVInfoSync (&pb) != noErr)
			return (false);
		
		if (!hasdesktopmanager (pb.volumeParam.ioVRefNum))
			continue;
		
		dt.ioNamePtr = NULL;
		
		dt.ioVRefNum = pb.volumeParam.ioVRefNum;
		
		if (PBDTGetPath (&dt) != noErr)
			return (false);
		
		dt.ioNamePtr = (StringPtr) &appfname;
		
		dt.ioIndex = 0;
		
		dt.ioFileCreator = appid;
		
		if (PBDTGetAPPLSync (&dt) == noErr) {
		
			if (FSMakeFSSpec (pb.volumeParam.ioVRefNum, dt.ioAPPLParID, appfname, fs) == noErr)
				return (true);
			}
		} /*while*/
		
	return (false);
	} /*findApp*/


static Boolean launchApp (FSSpec *fs) {
	
	LaunchParamBlockRec pb;
	
	clearbytes (&pb, (long) sizeof (pb));
	
	pb.launchAppSpec = fs;
	
	pb.launchBlockID = extendedBlock;
	
	pb.launchEPBLength = extendedBlockLen;
	
	pb.launchControlFlags = launchContinue | launchNoFileFlags/* | launchDontSwitch*/;
	
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
		
	if (!findApp (serverid, &fs))
		return (false);
		
	if (!launchApp (&fs))
		return (false);
		
	while (!appRunning (serverid)) {
		
		EventRecord ev;
		
		EventAvail (everyEvent, &ev);
		} /*while*/
		
	return (true);
	} /*launchServer*/
	
	
static pascal OSErr handleopen (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	OSErr ec;
	
	flopenhandled = true;
	
	fl2click = false; /*it was a drag-and-drop*/
	
	ec = AEGetKeyDesc (event, keyDirectObject, typeAEList, &filelist);
	
	return (ec);
	} /*handleopen*/


static pascal OSErr handleopenapp (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	flopenhandled = true;
	
	fl2click = true; /*launched by 2clicking*/
	
	return (noErr);
	} /*handleopenapp*/
	

static void drawitemtext (DialogPtr pdialog, short itemnumber, Handle htext) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	short savedfont, savedsize;
	Rect r;
	PenState savedpen;
	short fnum;
	
	savedfont = (*qd.thePort).txFont;
	
	savedsize = (*qd.thePort).txSize;
	
	GetFNum ("\pPalatino", &fnum);
	
	TextFont (fnum);
	
	TextSize (12);
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the item’s rect*/
	
	r = itemrect;
	
	InsetRect (&r, 3, 3);
	
	TextBox (*htext, GetHandleSize (htext), &r, 0);
	
	TextFont (savedfont);
	
	TextSize (savedsize);
	
	GetPenState (&savedpen); /*save the old pen state*/
	
	PenPat (&qd.gray);
	
	FrameRect (&itemrect);
	
	SetPenState (&savedpen);
	} /*drawitemtext*/
	

static void drawitemstring (DialogPtr pdialog, short itemnumber, Str255 s) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	short savedfont, savedsize, savedstyle;
	Rect r;
	PenState savedpen;
	FontInfo info;
	RgnHandle rgn;
		
	savedfont = (*qd.thePort).txFont;
	
	savedsize = (*qd.thePort).txSize;
	
	savedstyle = (*qd.thePort).txFace;
	
	TextFont (helvetica);
	
	TextSize (18);
	
	TextFace (bold);
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the item’s rect*/
	
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
	
	GetPenState (&savedpen); /*save the old pen state*/
	
	PenPat (&qd.gray);
	
	SetPenState (&savedpen);
	} /*drawitemstring*/
	

static void drawitemicon (DialogPtr pdialog, short itemnumber, short resid) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
		
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the item’s rect*/
	
	PlotIconID (&itemrect, 0, 0, resid);
	} /*drawitemicon*/
	
	
static void grayrect (Rect r) {

	PenState savedpen;
	
	GetPenState (&savedpen); /*save the old pen state*/
	
	PenMode (patBic);
	
	PenPat (&qd.gray);
	
	PaintRect (&r);
	
	SetPenState (&savedpen);
	} /*grayrect*/
	
	
static void disabledialogitem (DialogPtr pdialog, short itemnumber) {
	
	/*
	new version of this routine, thanks to Mike Alexander of Ann Arbor, MI.
	
	see notes for Droplet Developer version 2.1b3.
	*/
	
	short itemtype;
	ControlHandle itemhandle;
	Rect itemrect;

	GetDItem (pdialog, itemnumber, &itemtype, (Handle *) &itemhandle, &itemrect);
	
	if ((**itemhandle).contrlHilite == 0) /*it is enabled, disable it*/
		HiliteControl (itemhandle, 0xFF);
	} /*disabledialogitem*/


static void highlightbutton (DialogPtr w, short itemnumber, Boolean flon) {
	
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


static void delayticks (short ct) {
	
	long tc;
	
	tc = TickCount () + ct;
	
	while (TickCount () < tc) {}
	} /*delayticks*/
	
	
static void simulatehit (DialogPtr w, short item) {
	
	highlightbutton (w, item, true);
	
	delayticks (8);
	
	highlightbutton (w, item, false);
	} /*simulatehit*/


static void handledrag (EventRecord *ev, WindowPtr w) {
	
	Rect r;

	r = qd.screenBits.bounds; 
	
	r.top = r.top + GetMBarHeight (); 
	
	InsetRect (&r, 4, 4);
	
	DragWindow (w, (*ev).where, &r);
	} /*handledrag*/


static void handlemenu (long codeword) {
	
	register short idmenu, iditem;
	
	iditem = LoWord (codeword);
	
	idmenu = HiWord (codeword);
	
	if (SharedMenuHit (idmenu, iditem)) 
		goto exit;
	
	switch (idmenu) {
	
		case applemenu: 
			switch (iditem) {
				
				case aboutitem:
					notifyuser (*GetString (130));
					
					break;
			
				default: {
					Str255 s;
					
					GetItem (happlemenu, iditem, s);
					
					OpenDeskAcc (s);
					
					break;
					}
				} /*switch*/
			
			break; /*apple menu*/

		case filemenu: 
			switch (iditem) {
				
				case quititem:
				
					flexitmainloop = true;
					
					break;
				} /*switch*/
			
			break; /*file menu*/
			
		} /*switching on which menu was invoked*/
		
	exit:
	
	HiliteMenu (0);
	} /*handlemenu*/


static void handlemouse (EventRecord *ev) {

	register short part;
	WindowPtr w;
	
	if (IsDialogEvent (ev)) {
		
		DialogPtr pdialog;
		short itemhit;

		if (DialogSelect (ev, &pdialog, &itemhit)) {
			
			switch (itemhit) {
				
				case runitem:
					DisposDialog (mainwindow);
					
					mainwindow = nil;
					
					dofilelist ();
					
					flexitmainloop = true;
					
					break;
				
				case cancelitem:
					flexitmainloop = true;
					
					break;
					
				case edititem:
					doedit ();
					
					break;
				} /*switch*/
			}
		
		return;
		}
	
	part = FindWindow ((*ev).where, &w);
	
	if (w != nil) 
	
		if (w != FrontWindow ()) { /*just like all other Mac programs*/
			
			SelectWindow (w);
							
			return; /*the mouse click is consumed by the bringtofront operation*/
			}
	
	switch (part) {
	
		case inMenuBar: 
			handlemenu (MenuSelect ((*ev).where)); 
			
			break;
		
		case inSysWindow:
			SystemClick (ev, w); 
			
			break;
		
		case inDrag:
			handledrag (ev, w);
			
			break;
		} /*switch*/
	} /*handlemouse*/
	
	
static Boolean handlekeystroke (EventRecord *ev) { 

	char ch = (*ev).message & charCodeMask;
	Boolean flcmdperiod = ((*ev).modifiers & cmdKey) && (ch == '.');
	
	if (SharedScriptRunning ()) { /*cmd-period terminates the script*/
	
		if (flcmdperiod) { 
			
			CancelSharedScript (); /*cancel the shared menu script, if one is running*/
		
			return (true);
			}
		}
	
	if ((ch == (char) 13) || (ch == (char) 3) || (ch == 'r') || (ch == 'R')) { 
		
		/*user hit Return, Enter, r or R*/
		
		if (fl2click) /*Run button not enabled*/
			return (true);
		
		simulatehit (mainwindow, runitem);
	
		DisposDialog (mainwindow);
		
		mainwindow = nil;
	
		dofilelist ();
	
		return (false);
		}
		
	if ((flcmdperiod) || ((ch == 'q') || (ch == 'Q'))) { /*close the window and exit*/
	
		simulatehit (mainwindow, cancelitem);
		
		return (false);
		}
		
	if ((ch == 'e') || (ch == 'E')) {
		
		simulatehit (mainwindow, edititem);
		
		doedit ();
		
		return (true);
		}
		
	handlemenu (MenuKey (ch)); 
	
	return (true);
	} /*handlekeystroke*/
	
	
static void handleupdate (EventRecord *ev) {
	
	WindowPtr w = (WindowPtr) (*ev).message;
	
	SetPort (w);
	
	BeginUpdate (w);
	
	DrawDialog (w);
	
	if (false) { /*11/14/93 DW -- always enable the Run button*/
	
		if (fl2click) 					
			disabledialogitem (w, runitem);
		else
			boldenbutton (w, runitem);
		}
	else
		boldenbutton (w, runitem);
	
	/*enable Edit button if it's not Runtime running*/ {
	
		short majorRev, minorRev, bugFixRev;
		Boolean flRuntime;

		getFrontierVersion (&majorRev, &minorRev, &bugFixRev, &flRuntime);
		
		if (flRuntime)
			disabledialogitem (w, edititem);
		}
	
	drawitemicon (w, iconitem, 128);
	
	drawitemstring (w, titleitem, titlestring);

	drawitemtext (w, msgitem, helptext);
	
	EndUpdate (w);
	} /*handleupdate*/


static Boolean handleevent (EventRecord *ev) {

	switch ((*ev).what) { 
	
		case kHighLevelEvent:
			AEProcessAppleEvent (ev);
			
			break;
			
		case osEvt:
			flcurrentlyactive = ((*ev).message & resumeFlag) != 0;
			
			break;
	
		case nullEvent: 
			if (flcurrentlyactive && flinstalled)
				CheckSharedMenus (140);
			
			break;

		case updateEvt:
			handleupdate (ev);
			
			break;
			
		case keyDown: case autoKey: 
			if (!handlekeystroke (ev))
				return (false);
			
			break;
				
		case mouseDown:
			if (!faceless)
				handlemouse (ev);
			
			break;			
		} /*switch*/
			
	return (true);
	} /*handleevent*/


static pascal void pascalalert (Str255 s) { /*DW 11/5/93*/
	
	notifyuser (s);
	} /*pascalalert*/
	
	
static pascal void pascaleventfilter (EventRecord *ev) { /*DW 11/5/93*/
	
	handleevent (ev);
	} /*pascaleventfilter*/
	

static void maineventloop (void) {
	
	EventRecord ev;
	
	if (!faceless) {
	
		InitSharedMenus (&pascalalert, &pascaleventfilter);
		
		initmenus ();
		
		helptext = getResourceAndDetach ('TEXT', 128, true);
		
		mainwindow = GetNewDialog (modelessid, nil, (WindowPtr) -1);
		
		SetPort (mainwindow);
		
		SetWTitle (mainwindow, appname);
		
		copystring ("\pThe “", titlestring);
		
		pushstring (appname, titlestring);
		
		pushstring ("\p” Droplet", titlestring);
		}
	
	while (!flexitmainloop) {
		
		if (!faceless)
			SetCursor (&qd.arrow);
		
		WaitNextEvent (everyEvent, &ev, 30, nil);
		
		if (!handleevent (&ev))
			goto endloop;
		
		if (!flinstalled) {
			
			flinstalled = true;
			
			doinstall ();
			}
		
		if (faceless) {
			
			if (flopenhandled) {
				
				dofilelist ();
				
				goto endloop;
				}
			}
		} /*while*/

	endloop:
	
	if (mainwindow != nil)
		DisposDialog (mainwindow);
	} /*maineventloop*/
	
	
void main (void) {
	
	long expiresat;
	
	InitGraf (&qd.thePort);
	
	InitFonts ();
	
	FlushEvents (everyEvent, 0);
	
	InitWindows ();
	
	InitMenus ();
	
	TEInit ();
	
	InitDialogs (0L);
	
	InitCursor ();
	
	if (!IAChaveappleevents ()) 
		return;
		
	if (!IACinstallhandler (kCoreEventClass, kAEOpenApplication, (ProcPtr) &handleopenapp))
		return;
		
	if (!IACinstallhandler (kCoreEventClass, kAEOpenDocuments, (ProcPtr) &handleopen))
		return;
		
	GetProcessFullPath ();
	
	if (!launchServer ('LAND')) { /*Frontier/Runtime isn't running and couldn't be launched*/
	
		Alert (128, nil);
		
		return;
		}
	
	faceless = getFaceless (); /*DW 10/1/93*/
	
	if (faceless) {
	
		expiresat = TickCount () + (1 * 60);
		
		while (!flopenhandled) { /*wait for the opendoc Apple Event to come in*/
			
			EventRecord ev;
	
			if (WaitNextEvent (everyEvent, &ev, 30, nil)) {
				
				if (ev.what == kHighLevelEvent) 
					AEProcessAppleEvent (&ev);
				}
			
			if (TickCount () > expiresat)
				break;
			} /*while*/
		}
	
	maineventloop ();
	} /*main*/


