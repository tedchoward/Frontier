
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Aliases.h>
#include <GestaltEqu.h>
#include <Folders.h>

#include <iac.h>

#ifndef macBirdRuntime /*it has no app.xxx record*/
	
	#include "applet.h"

#else

	#include "appletdefs.h"
	#include "appletstrings.h"
	#include "appletmemory.h"
	#include "appletsyserror.h"
	
	extern void alertdialog (bigstring);
	
#endif

#include "appletprocess.h"
#include "appletsyserror.h"
#include "appletfolder.h"



typedef AppParameters **AppParametersHandle;

#define kAEFinderEvents 'FNDR'
#define kAEFinderID 'MACS'
#define kAEShowClipboard 'shcl'
#define kAEPutAway 'sput'
#define keyAEFinderSelection 'fsel'
#define kAEDrag 'drag'
#define keyAEFinderMisc 'fmsc'
#define kAEEmptyTrash 'empt'
#define kAEOpenSelection 'sope'
#define kAEPrintSelection 'spri'


static struct {

	boolean launched;

	OSType apptype;

	bigstring errormessage;
	} helperappinfo;



boolean processloop (typrocessloopcallback callback, long refcon) {
	
	ProcessInfoRec sysinfo;
	typrocessinfo ourinfo;
	ProcessSerialNumber psn;
	
	sysinfo.processInfoLength = sizeof (sysinfo);
	
	sysinfo.processName = (StringPtr) ourinfo.name; /*place to store process name*/
	
	sysinfo.processAppSpec = &ourinfo.file; /*place to store process filespec*/
	
	psn.highLongOfPSN = kNoProcess;
	
	psn.lowLongOfPSN = kNoProcess;
	
	while (GetNextProcess (&psn) == noErr) {
		
		clearbytes (&ourinfo, longsizeof (ourinfo));
		
	 	sysinfo.processInfoLength = sizeof (sysinfo);
	 	
		if (GetProcessInformation (&psn, &sysinfo) != noErr)
			continue; /*keep going -- ignore error*/
				
		ourinfo.serialnumber = sysinfo.processNumber;
		
		ourinfo.launcherserialnumber = sysinfo.processLauncher;
		
		ourinfo.filetype = sysinfo.processType;
		
		ourinfo.filecreator = sysinfo.processSignature;
		
		ourinfo.sizerecourceflags = sysinfo.processMode;
		
		ourinfo.partition = sysinfo.processLocation;
		
		ourinfo.partitionsize = sysinfo.processSize;
		
		ourinfo.freebytesinheap = sysinfo.processFreeMem;
		
		ourinfo.launchticks = sysinfo.processLaunchDate;
		
		ourinfo.activeticks = sysinfo.processActiveTime;
		
		if (!(*callback) (&ourinfo, refcon))
			return (false);
		} /*while*/
		
	return (true); /*loop terminated, none of the visit routines returned false*/
	} /*processloop*/
	

static boolean findrunningappvisit (typrocessinfo *visitinfo, long refcon) {
	
	typrocessinfo *lookforinfo = (typrocessinfo *) refcon;
	
	if ((*visitinfo).filecreator == (*lookforinfo).filecreator) {
		
		moveleft (visitinfo, lookforinfo, longsizeof (typrocessinfo));

		return (false);
		}
		
	return (true);
	} /*findrunningappvisit*/
	

boolean findrunningapp (OSType id, typrocessinfo *info) {
	
	clearbytes (info, longsizeof (typrocessinfo));
	
	(*info).filecreator = id; /*this is the one we're looking for*/
	
	return (!processloop (&findrunningappvisit, (long) info));
	} /*findrunningapp*/
	

boolean bringapptofront (OSType id) {

	typrocessinfo info;
	
	if (!findrunningapp (id, &info))
		return (false);
	
	WakeUpProcess (&info.serialnumber);
	
	SetFrontProcess (&info.serialnumber);
	
	return (true); 
	} /*bringapptofront*/
	
	
static OSErr buildlaunchparamlist (FSSpec *fsdoc, AppParametersHandle *appparams) {
	
	AEDesc addrdesc;
	AEDesc launchdesc;
	AEDescList doclist;
	AppleEvent event;
	OSErr ec;
	AliasHandle halias;
	AEDesc docdesc;
	
	if (!testheapspace (0x0200)) /*512 bytes should be plenty to avoid lots of error checking*/
		return (memFullErr);
	
	ec = NewAliasMinimal (fsdoc, &halias);
	
	if (ec != noErr)
		return (ec);
	
	addrdesc.descriptorType = typeNull;
	
	addrdesc.dataHandle = nil;
	
	AECreateAppleEvent (kCoreEventClass, kAEOpenDocuments, &addrdesc, kAutoGenerateReturnID, kAnyTransactionID, &event);
	
	AECreateList (nil, 0, false, &doclist); /*create list for the fsspec*/
	
	docdesc.descriptorType = typeAlias;
	
	docdesc.dataHandle = (Handle) halias;
	
	AEPutDesc (&doclist, 0, &docdesc); /*put alias on the list*/
	
	AEDisposeDesc (&docdesc);
	
	AEPutParamDesc (&event, keyDirectObject, &doclist); /*put list on the event*/
	
	AEDisposeDesc (&doclist); /*dispose list*/
	
	ec = AECoerceDesc (&event, typeAppParameters, &launchdesc); /*this is what Launch wants*/
	
	AEDisposeDesc (&event); /*we're done with it*/
	
	if (ec != noErr) /*shouldn't fail, but...*/
		return (ec);
	
	*appparams = (AppParametersHandle) launchdesc.dataHandle;
	
	return (noErr);
	} /*buildlaunchparamlist*/


boolean launchappwithdoc (FSSpec *fsapp, FSSpec *fsdoc, boolean flbringtofront) {
	
	LaunchParamBlockRec pb;
	AppParametersHandle docparams = nil;
	OSErr ec;
	
	clearbytes (&pb, longsizeof (pb));
	
	pb.launchAppSpec = fsapp;
	
	pb.launchBlockID = extendedBlock;
	
	pb.launchEPBLength = extendedBlockLen;
	
	pb.launchControlFlags = launchContinue | launchNoFileFlags;
	
	if (!flbringtofront)
		pb.launchControlFlags = pb.launchControlFlags | launchDontSwitch;
	
	if (fsdoc != nil) { /*set up docparam with launch event*/
		
		ec = buildlaunchparamlist (fsdoc, &docparams);
		
		if (ec != noErr)
			return (false);
		
		lockhandle ((Handle) docparams);
		
		pb.launchAppParameters = *docparams;
		}
	
	ec = LaunchApplication (&pb);
	
	disposehandle ((Handle) docparams); /*checks for nil*/
	
	if (ec != noErr)
		return (false);
	
	/*wait for a few events, per DMB's advice*/ {
	
		short i;
		EventRecord ev;
		
		for (i = 1; i <= 5; i++)
			EventAvail (everyEvent, &ev);
		}
	
	return (true);
	} /*launchappwithdoc*/


static boolean pushaliaslist (FSSpec *fsdoc, OSType listkey, AppleEvent *event) {
	
	AEDescList doclist;
	OSErr ec;
	AliasHandle halias;
	AEDesc docdesc;
	
	ec = NewAliasMinimal (fsdoc, &halias);
	
	if (ec != noErr)
		return (false);
	
	ec = AECreateList (nil, 0, false, &doclist); /*create list for the fsspec*/
	
	if (ec != noErr)
		return (false);
		
	docdesc.descriptorType = typeAlias;
	
	docdesc.dataHandle = (Handle) halias;
	
	ec = AEPutDesc (&doclist, 0, &docdesc); /*put alias on the list*/
	
	if (ec != noErr)
		return (false);
		
	AEDisposeDesc (&docdesc);
	
	AEPutParamDesc (event, listkey, &doclist); /*put list on the event*/
	
	AEDisposeDesc (&doclist); /*dispose list*/
	
	return (true);
	} /*pushaliaslist*/


static boolean oldsendodoc (OSType appid, FSSpec *fsdoc) { 

	AppleEvent event, reply;
	boolean fl = false;
	
	if (!IACnewverb (appid, kCoreEventClass, kAEOpenDocuments, &event))
		return (false);
	
	if (!pushaliaslist (fsdoc, keyDirectObject, &event))
		goto exit;
	
	fl = IACsendverb (&event, &reply);
	
	AEDisposeDesc (&reply);
	
	exit:
	
	AEDisposeDesc (&event);	
	
	return (fl);
	} /*oldsendodoc*/
	

boolean sendodoc (OSType appid, FSSpec *fsdoc) {
	
	typrocessinfo info;
	AEAddressDesc adr; 
	OSErr ec;
	AppleEvent event, reply;
	boolean fl = false;
	
	if (!findrunningapp (appid, &info))
		return (false);
		
	AECreateDesc (typeProcessSerialNumber, (Ptr) &info.serialnumber, sizeof (info.serialnumber), &adr);
	
	ec = AECreateAppleEvent (kCoreEventClass, kAEOpenDocuments, &adr, kAutoGenerateReturnID, kAnyTransactionID, &event);
	
	AEDisposeDesc (&adr);
	
	if (ec != noErr)
		return (false);
	
	if (!pushaliaslist (fsdoc, keyDirectObject, &event))
		goto exit;
	
	fl = IACsendverb (&event, &reply);
	
	AEDisposeDesc (&reply);
	
	exit:
	
	AEDisposeDesc (&event);	
	
	return (fl);
	} /*sendodoc*/
	

static boolean xxxfinderevent (OSType idverb, FSSpec *fsdoc) {

	AppleEvent event, reply;
	boolean fl = false;
	OSErr ec;
	
	if (!IACnewverb ('MACS', 'FNDR', idverb, &event))
		return (false);
	
	if (!pushaliaslist (fsdoc, 'fsel', &event)) {
		
		AEDisposeDesc (&event);
		
		return (false);
		}
	
	ec = AESend (
		&event, &reply, kAENoReply + kAENeverInteract, 
		
		kAENormalPriority, kAEDefaultTimeout, nil, nil);
	
	AEDisposeDesc (&event);	
	
	return (ec == noErr);
	} /*finderevent*/
	
	
static boolean finderevent (OSType idverb, FSSpec *fsdoc) {
	
	/*
	this is crufty code donated by Jorg, stolen from Frontier.
	
	really needs to be cleaned out, I think. DW 7/19/93.
	*/
	
	static EventRecord er;
	short err;
	AliasHandle a1 = 0, a2 = 0;
	long hs1, hs2;
	FSSpec fs1, fs2 = *fsdoc;
	Handle message;
	CInfoPBRec	pb;
	long version;
	
	/* get some catalog information, which we will use later. */
	pb.dirInfo.ioCompletion=0;
	pb.dirInfo.ioNamePtr = fs2.name;
	pb.dirInfo.ioVRefNum = fs2.vRefNum;
	pb.dirInfo.ioFDirIndex = 0;
	pb.dirInfo.ioDrDirID = fs2.parID;
	
	err = PBGetCatInfoSync(&pb);
	
	if (err != noErr)
		goto exit;
	
	err = NewAliasMinimal(&fs2, &a2);
	
	if (err != noErr)
		goto exit;
	
	fs1 = fs2;
	
	if (Gestalt (gestaltAUXVersion, &version)) { /*running under A/UX; cover bugs*/
		
		if (fs1.parID == 1) { /*it's a volume*/
			
			fs1.vRefNum = -1; /*a/ux wants to see the root as the parent*/
			}
		else { /*not a volume: path parsing works better under a/ux*/
			
			/*
			bigstring path;
			
			if (!filespectopath (fs1, path))
				return (false);
			
			folderfrompath (path, path);
			
			if (!pathtofilespec (path, &fs1))
				return (false);
			
			goto gotfs1;
			*/
			}
		}
	else {
		
		if (fs1.parID == 1) { /*it's a volume*/
			
			err = FindFolder(fs1.vRefNum, kDesktopFolderType, FALSE, &fs1.vRefNum, &fs1.parID);
			
			if (err != noErr)
				goto exit;
			}
		}
	
	err = FSMakeFSSpec(fs1.vRefNum, fs1.parID, "\p", &fs1);
	
	if (err != noErr)
		goto exit;
	
	gotfs1:
	
	err = NewAliasMinimal(&fs1, &a1);
	
	if (err != noErr)
		goto exit;
	
	hs1 = GetHandleSize((Handle) a1);
	hs2 = GetHandleSize((Handle) a2);
	
	MoveHHi((Handle) a1); HLock((Handle) a1);
	MoveHHi((Handle) a2); HLock((Handle) a2);
	
	er.what = kHighLevelEvent;				/* PHLE fills this in, but we do it just to be clear. */
	er.message = kAEFinderEvents;			/* this is the event class */
	*(long *)&er.where = idverb;			/* this is the event type - see Card 131 */
	er.when = TickCount();					/* this also gets filled in, but... */
	er.modifiers = 0;						/* doesn't matter */
	
	message = appnewhandle (24 + hs1 + 28 + hs2);
	
	/*
	normally you'd use Apple Events manager calls to pack up a message and
	send it.  I do things this way only because I can't be sure the frontmost
	app has initialized that manager.
	*/
	
	if (message) {
		register long *filler = (void *)*message;
		
		*filler++ = 'aevt';
		*filler++ = 0x00010001;
		*filler++ = ';;;;';
		*filler++ = '----';
		*filler++ = 'alis';
		*filler++ = hs1;
		BlockMove(*a1, filler, hs1); filler = (long *)(hs1 + (long) filler);
		*filler++ = 'fsel';
		*filler++ = 'list';
		*filler++ = hs2 + 16;
		*filler++ = 1;
		*filler++ = 0;
		*filler++ = 'alis';
		*filler++ = hs2;
		BlockMove(*a2, filler, hs2);
		};
	
	DisposHandle((Handle) a1);
	DisposHandle((Handle) a2);
	
	if (!message) {
		
		err = memFullErr;
		
		goto exit;
		}
	
	HLock(message);
	
	err = PostHighLevelEvent (&er, (long) kAEFinderID, 237, *message, GetHandleSize (message), receiverIDisSignature);
	
	if ((pb.dirInfo.ioFlAttrib & ioDirMask) || (pb.hFileInfo.ioFlFndrInfo.fdType == 'cdev')) 
		bringapptofront (kAEFinderID);
	
	DisposHandle(message);
	
	exit:
	
	return (err == noErr);
	} /*finderevent*/
	

boolean launchusingfinder (FSSpec *fs) {
	
	return (finderevent (kAEOpenSelection, fs));
	} /*launchusingfinder*/
	
	
boolean revealinfinder (FSSpec *fs) {

	return (finderevent ('srev', fs));
	} /*revealinfinder*/
	
	
static boolean apprunningflag;


static boolean fileisrunningvisit (typrocessinfo *info, long refcon) {
	
	FSSpec *fs = (FSSpec *) refcon;
	
	if (equalfilespecs (&(*info).file, fs)) {
		
		apprunningflag = true;
		
		return (false);
		}
	
	return (true);
	} /*fileisrunningvisit*/
	
	
boolean fileisrunning (FSSpec *fs) {
	
	apprunningflag = false;
	
	processloop (&fileisrunningvisit, (long) fs);
	
	return (apprunningflag);
	} /*fileisrunning*/
	

boolean equalprocesses (typrocessinfo *rec1, typrocessinfo *rec2) {
	
	Boolean flsameprocess;
	
	SameProcess (&(*rec1).serialnumber, &(*rec2).serialnumber, &flsameprocess);
	
	return (flsameprocess);
	} /*equalprocesses*/
	

boolean quitapplication (OSType idapp) {
	
	AppleEvent event, reply;
	boolean fl;
	
	if (!IACnewverb (idapp, kCoreEventClass, kAEQuitApplication, &event))
		return (false);
	
	fl = IACsendverb (&event, &reply);
	
	AEDisposeDesc (&reply);
	
	AEDisposeDesc (&event);	
	
	return (fl);
	} /*quitapplication*/
	
	
boolean launchdeskaccessoryfile (FSSpec *fs) {
	
	OSErr ec;
	
	ec = LaunchDeskAccessory (fs, nil);
	
	return (!syserror (ec));
	} /*launchdeskaccessoryfile*/
	

static boolean helpercallback (bigstring fname, tyfileinfo *info, long pxinfo) {
	
	#pragma unused (pxinfo)
	
	tyfilespec fs;
	
	filemakespec ((*info).vnum, (*info).dirid, fname, &fs);
	
	if ((*info).filetype != 'APPL') /*keep looking*/
		return (true);
	
	if ((*info).filecreator != helperappinfo.apptype) /*keep looking*/
		return (true);
	
	if (!launchappwithdoc (&fs, nil, false)) {
		
		bigstring bs;
		
		copystring ("\pCouldn't launch \"", bs);
		
		pushstring (fname, bs);
		
		pushstring ("\p\".", bs);
		
		copystring (bs, helperappinfo.errormessage);
		}
	else
		helperappinfo.launched = true;
	
	return (false); /*the search is over*/
	} /*helpercallback*/
	
	
boolean getcurrentappfilespec (ptrfilespec pfs) {
	
	#ifndef macBirdRuntime /*it has no app.xxx record*/
	
		typrocessinfo info;
		
		if (!findrunningapp (app.creator, &info)) /*get info about the app*/
			return (false);
		
		*pfs = info.file;
	
		return (true);
	
	#else
	
		#pragma unused (pfs)
	
		return (false);
		
	#endif
	} /*getcurrentappfilespec*/
	
	
boolean getcurrentappfolder (ptrfilespec pfs) {
	
	if (!getcurrentappfilespec (pfs))
		return (false);

	return (filegetparentfolderspec (pfs, pfs));
	} /*getcurrentappfolder*/
	
	
static boolean gethelperfolder (ptrfilespec pfshelperfolder, bigstring errormessage) {

	tyfilespec fsfolder;
	
	setstringlength (errormessage, 0);
	
	if (!getcurrentappfolder (&fsfolder))
		return (false);
	
	filegetsubitemspec (&fsfolder, "\pHelper Apps", pfshelperfolder);
	
	if (!fileexists (pfshelperfolder)) {
		
		copystring ("\pCouldn't locate the \"Helper Apps\" folder.", errormessage);
		
		return (false);
		}
	
	return (true);
	} /*gethelperfolder*/
	
	
boolean launchHelperApp (OSType serverid, void *verrormessage) {
	
	byte *errormessage = (byte *) verrormessage;
	typrocessinfo info;
	tyfilespec fshelperfolder;
	
	if (findrunningapp (serverid, &info)) /*already running, nothing to do*/
		return (true);
		
	if (!gethelperfolder (&fshelperfolder, errormessage))
		return (false);
	
	helperappinfo.launched = false;
	
	helperappinfo.apptype = serverid;
	
	setstringlength (helperappinfo.errormessage, 0);
	
	folderloop (&fshelperfolder, false, &helpercallback, 0);
	
	copystring (helperappinfo.errormessage, errormessage);
	
	return (helperappinfo.launched);
	} /*launchHelperApp*/
	

static boolean launchFrontier (void) { /*sample code*/
	
	bigstring errormessage;
	
	if (!launchHelperApp ('LAND', errormessage)) {
		
		if (stringlength (errormessage) == 0)
			copystring ("\pCouldn't locate Frontier or Frontier Runtime in the Helper Apps folder.", errormessage);
			
		alertdialog (errormessage);
		
		return (false);
		}

	return (true);
	} /*launchFrontier*/
	
	