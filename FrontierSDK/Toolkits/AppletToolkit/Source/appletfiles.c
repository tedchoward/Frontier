
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Finder.h>
#include <Script.h>
#include <StandardFile.h>

#include "appletfiles.h"
#include "appletcursor.h"
#include "appletops.h"
#include "appletmemory.h"
#include "appletstrings.h"
#include "appletbits.h"
#include "appletfilesinternal.h"
#include "appletsyserror.h"



boolean flerrordialogdisabled = false; /*allows high-level code to turn off error dialogs*/

static OSErr internalerrorcode = noErr;

#define sfgetfolderid 132
#define sfgetfolderbutton 11
#define sfgetpromptitem 10


void setfileerrorfile (ptrfilespec pfs) {
	
	bigstring fname;
	
	if (pfs == nil)
		setstringlength (fname, 0);
	else
		filegetfilename (pfs, fname);
	
	setsyserrorparam (fname);
	} /*setfileerrorfile*/
	
	
boolean fileerror (ptrfilespec pfs, OSErr ec) {
	
	if (ec == noErr)
		return (false);
		
	internalerrorcode = ec;
	
	setfileerrorfile (pfs);
	
	if (flerrordialogdisabled)
		return (true);
	
	return (syserror (ec));
	} /*fileerror*/
	
	
boolean filemakespec (short vnum, long dirid, bigstring fname, ptrfilespec pfs) {

	OSErr ec;
	
	ec = FSMakeFSSpec (vnum, dirid, fname, pfs);
	
	return ((ec == noErr) || (ec == fnfErr));
	} /*filemakespec*/
	
	
boolean fileexists (ptrfilespec pfs) {

	FSSpec fs;
	
	return (FSMakeFSSpec ((*pfs).vRefNum, (*pfs).parID, (*pfs).name, &fs) == noErr);
	} /*fileexists*/
	

boolean equalfilespecs (ptrfilespec pfs1, ptrfilespec pfs2) {
	
	if ((*pfs1).vRefNum != (*pfs2).vRefNum)
		return (false);
	
	if ((*pfs1).parID != (*pfs2).parID)
		return (false);
		
	if (!equalstrings ((*pfs1).name, (*pfs2).name))
		return (false);
	
	return (true);
	} /*equalfilespecs*/
	

boolean isvolumefilespec (ptrfilespec pfs) {
	
	if (((*pfs).parID == 0) && (stringlength ((*pfs).name) == 0))
		return (true);
		
	if ((*pfs).parID == 1) /*added, DW 9/9/93*/
		return (true);
	
	return (false);
	} /*isvolumefilespec*/
	

/*
static void convertvolumespec (ptrfilespec pfs) {
	
	/*
	for getting and setting file info only, if we're dealing with a 
	volume, convert the file spec to point to the root folder on the
	volume. according to Think Reference, (see hint in page for 
	PBGetCatInfo) the dirid of the root directory is always 2.
	
	unfortunately it didn't help. we still can't set the label of
	a volume.
	
	dmb 9/21/93: this is wrong. true, the dirID of the root is 2, but 
	if you change that you'd also have to set the name to the empty 
	string. there's no reason to do either.
	%/
	
	if (isvolumefilespec (pfs)) 
		(*pfs).parID = 2;
	} /*convertvolumespec*/
	

boolean getmacfileinfo (ptrfilespec pfs, CInfoPBRec *pb) {
	
	OSErr ec;
	
	/*
	convertvolumespec (pfs);
	*/
	
	clearbytes (pb, longsizeof (*pb));
	
	(*pb).hFileInfo.ioNamePtr = (*pfs).name;
	
	(*pb).hFileInfo.ioVRefNum = (*pfs).vRefNum;
	
	(*pb).hFileInfo.ioDirID = (*pfs).parID;
	
	ec = PBGetCatInfoSync (pb);
	
	return (!fileerror (pfs, ec));
	} /*getmacfileinfo*/
 

boolean setmacfileinfo (ptrfilespec pfs, CInfoPBRec *pb) {
	
	OSErr ec;
	
	/*
	convertvolumespec (pfs);
	*/
	
	(*pb).hFileInfo.ioNamePtr = (*pfs).name;
	
	(*pb).hFileInfo.ioVRefNum = (*pfs).vRefNum;
	
	(*pb).hFileInfo.ioDirID = (*pfs).parID;
	
	ec = PBSetCatInfoSync (pb);
	
	return (!fileerror (pfs, ec));
	} /*setmacfileinfo*/


static boolean getinfofromvolpb (const HVolumeParam *pb, tyfileinfo *info) {
	
	/*
	dmb 9/21/93: for a volume, dirid should be fsRtParID, not fsRtDirID.
	*/
	
	short drivenum;
	QHdrPtr dqtop;
	QElemPtr dqelem;
	
	clearbytes (info, sizeof (tyfileinfo));
	
	(*info).vnum = (*pb).ioVRefNum;
	
	(*info).dirid = fsRtParID; /*fsRtDirID*/
	
	(*info).flvolume = true;
	
	(*info).flfolder = true;
	
	(*info).timecreated = (*pb).ioVCrDate;
	
	(*info).timemodified = (*pb).ioVLsMod;
	
	(*info).fllocked = ((*pb).ioVAtrb & 0x8000) != 0;
	
	(*info).flhardwarelock = ((*pb).ioVAtrb & 0x0080) != 0;
	
	if ((*info).flhardwarelock)
		(*info).fllocked = true;
	
	(*info).ctfreebytes = (*pb).ioVAlBlkSiz * (*pb).ioVFrBlk;
	
	(*info).cttotalbytes = (*pb).ioVAlBlkSiz * (*pb).ioVNmAlBlks;
	
	(*info).ctfiles = (*pb).ioVFilCnt + (*pb).ioVDirCnt;
	
	drivenum = (*pb).ioVDrvInfo;
	
	dqtop = GetDrvQHdr ();
	
	dqelem = (*dqtop).qHead;
	
	while (true) {
		
		if (dqelem == nil) { /*volume is no longer in a drive; it must be ejectable!*/
			
			(*info).flejectable = true;
			
			break;
			}
		
		if ((*(DrvQEl *)dqelem).dQDrive == drivenum) {
			
			byte driveflag = *((byte *) dqelem - 3);
			
			(*info).flejectable = ((driveflag != 8) && (driveflag != 0x48)); /*IM IV-181*/
			
			break;
			}
		
		dqelem = (*dqelem).qLink;
		} /*while*/
	
	return (true);
	} /*getinfofromvolpb*/


boolean filegetvolumeinfo (tyvolnum vnum, tyfileinfo *info) {
	
	/*
	dmb 9/21/93: take vnum as parameter, not volname. otherwise, 
	we can't distinguish between two vols w/the same name.
	*/
	
	HVolumeParam pb;
	bigstring bs;
	
	setemptystring (bs);
	
	clearbytes (&pb, longsizeof (pb)); /*init all fields to zero*/
	
	pb.ioVRefNum = vnum;
	
	pb.ioNamePtr = bs;
	
	pb.ioVolIndex = -1;
	
	(*info).errcode = PBHGetVInfoSync ((HParmBlkPtr) &pb);
	
	clearbytes (info, longsizeof (tyfileinfo)); /*init all fields to zero*/
	
	if ((*info).errcode != noErr)
		return (false);
	
	getinfofromvolpb (&pb, info);
	
	/*DW 9/7/93: determine if it's a network volume*/ {
		
		HParamBlockRec pb;
		GetVolParmsInfoBuffer buffer;
		OSErr ec;
		
		clearbytes (&pb, longsizeof (pb)); /*init all fields to zero*/
		
		pb.ioParam.ioVRefNum = (*info).vnum;
		
		pb.ioParam.ioBuffer = (Ptr) &buffer;
		
		pb.ioParam.ioReqCount = longsizeof (buffer);
		
		ec = PBHGetVolParmsSync (&pb); 
	
		if (ec == noErr) 
			(*info).flremotevolume = buffer.vMServerAdr != 0; /*see Apple TN-Files docviewer doc*/
		}
	
	return (true);
	} /*filegetvolumeinfo*/
	
	
boolean filegetvolumename (short vnum, bigstring volname) {
	
	HVolumeParam pb;
	OSErr ec;
	
	clearbytes (&pb, longsizeof (pb)); /*init all fields to zero*/
	
	pb.ioNamePtr = volname;
	
	pb.ioVolIndex = 0;
	
	pb.ioVRefNum = vnum;
	
	ec = PBHGetVInfoSync ((HParmBlkPtr) &pb);
	
	if (ec != noErr) {
		
		setstringlength (volname, 0);
		
		return (false);
		}
		
	return (true);
	} /*filegetvolumename*/
	
	
void filegetinfofrompb (CInfoPBRec *pb, tyfileinfo *info) {
	
	/*
	dmb 9/24/93: handle volumes here, combining vol info with root 
	directory folder info. I'm not sure if a volume lock is always 
	reflected in the root directory, so don't set fllocked false 
	if the attribute isn't set in the pb. (it starts out cleared anyway.)
	*/
	
	clearbytes (info, longsizeof (tyfileinfo)); /*init all fields to zero*/
	
	(*info).vnum = (*pb).hFileInfo.ioVRefNum;
	
	(*info).dirid = (*pb).hFileInfo.ioFlParID;
	
	if ((*info).dirid == fsRtParID)
		filegetvolumeinfo ((*info).vnum, info);
	else
		(*info).flvolume = false;
	
	if (BitTst (&(*pb).dirInfo.ioFlAttrib, 7)) /*if it's a volume, fllocked may already be set*/
		(*info).fllocked = true;
	
	(*info).flfolder = BitTst (&(*pb).dirInfo.ioFlAttrib, 3);
	
	(*info).ixlabel = ((*pb).hFileInfo.ioFlFndrInfo.fdFlags & 0x000E) >> 1;
	
	if ((*info).flfolder) {
		
		(*info).flbusy = (*pb).dirInfo.ioDrNmFls > 0;
		
		(*info).filecreator = (*info).filetype = '    ';
		
		if (!(*info).flvolume) { /*these aren't the same for a volume & its root dir*/
			
			(*info).timecreated = (*pb).dirInfo.ioDrCrDat;
			
			(*info).timemodified = (*pb).dirInfo.ioDrMdDat;
			
			(*info).ctfiles = (*pb).dirInfo.ioDrNmFls;
			}
		
		(*info).iconposition = (*pb).dirInfo.ioDrUsrWds.frLocation;
		
		(*info).folderview = (tyfolderview) ((*pb).dirInfo.ioDrUsrWds.frView >> 8);
		}
	else { /*fill in fields for a file, somewhat different format than a folder*/
	
		(*info).flbusy = BitTst (&(*pb).hFileInfo.ioFlAttrib, 0);
			
		(*info).filecreator = (*pb).hFileInfo.ioFlFndrInfo.fdCreator;
		
		(*info).filetype = (*pb).hFileInfo.ioFlFndrInfo.fdType;
		
		(*info).timecreated = (*pb).hFileInfo.ioFlCrDat;

		(*info).timemodified = (*pb).hFileInfo.ioFlMdDat;
		
		(*info).sizedatafork = (*pb).hFileInfo.ioFlLgLen;
		
		(*info).sizeresourcefork = (*pb).hFileInfo.ioFlRLgLen;
		
		(*info).iconposition = (*pb).hFileInfo.ioFlFndrInfo.fdLocation;

		/*copy from the finder bits into the record*/ {
		
			short finderbits = (*pb).hFileInfo.ioFlFndrInfo.fdFlags;
			
			(*info).flalias = (finderbits & kIsAlias) != 0;
			
			(*info).flbundle = (finderbits & kHasBundle) != 0;
			
			(*info).flinvisible = (finderbits & kIsInvisible) != 0;
			
			(*info).flstationery = (finderbits & kIsStationary) != 0;
			
			(*info).flshared = (finderbits & kIsShared) != 0;
			
			(*info).flnamelocked = (finderbits & kNameLocked) != 0;
			
			(*info).flcustomicon = (finderbits & kHasCustomIcon) != 0;
			}
		}
	} /*filegetinfofrompb*/
	
	
boolean filegetinfo (ptrfilespec pfs, tyfileinfo *info) {
	
	/*
	dmb 9/24/93: let filegetinfofrompb take care of volumes
	*/
	
	CInfoPBRec pb;
	
	/*
	if (isvolumefilespec (pfs)) {
		
		return (filegetvolumeinfo ((*pfs).vRefNum, info));
		}
	*/
		
	if (!getmacfileinfo (pfs, &pb)) {
		
		clearbytes (info, longsizeof (tyfileinfo)); /*init all fields to zero*/
	
		(*info).errcode = internalerrorcode;
		
		return (false);
		}
	
	filegetinfofrompb (&pb, info);
	
	return (true);
	} /*filegetinfo*/
	
	
boolean filegetfilename (FSSpec *pfs, bigstring name) {
	
	/*
	filespecs for volumes have their strings set to the empty string.
	*/
	
	copystring ((*pfs).name, name);
	
	if (stringlength (name) > 0)
		return (true);
		
	return (filegetvolumename ((*pfs).vRefNum, name));
	} /*filegetfilename*/
	
	
boolean filesettype (ptrfilespec pfs, OSType type) {
	
	FInfo info;
	OSErr ec;
	
	ec = FSpGetFInfo (pfs, &info);
	
	if (fileerror (pfs, ec))
		return (false);
	
	info.fdType = type;
	
	ec = FSpSetFInfo (pfs, &info);
	
	return (!fileerror (pfs, ec));
	} /*filesettype*/
	
	
boolean filesetcreator (ptrfilespec pfs, OSType creator) {
	
	FInfo info;
	OSErr ec;
	
	ec = FSpGetFInfo (pfs, &info);
	
	if (fileerror (pfs, ec))
		return (false);
	
	info.fdCreator = creator;
	
	ec = FSpSetFInfo (pfs, &info);
	
	return (!fileerror (pfs, ec));
	} /*filesetcreator*/
	
	
void fileclose (short fnum) {
	
	if (fnum != 0)
		FSClose (fnum);
	} /*fileclose*/
	

boolean fileseteof (short fnum, long eof) {
	
	OSErr errcode;
	
	if (fnum != 0) {
	
		errcode = SetEOF (fnum, eof);
		
		return (errcode == noErr);
		}
		
	return (true);
	} /*fileseteof*/
	

boolean fileopen (ptrfilespec pfs, short *fnum) {
	
	OSErr ec;
	
	setfileerrorfile (pfs); /*set name for read/write ops that only know about fnums*/
	
	ec = FSpOpenDF (pfs, fsCurPerm, fnum);

	return (!fileerror (pfs, ec));
	} /*fileopen*/
	
	
boolean fileopenforreading (ptrfilespec pfs, short *fnum) {
	
	OSErr ec;
	
	setfileerrorfile (pfs); /*set name for read/write ops that only know about fnums*/
	
	ec = FSpOpenDF (pfs, fsRdPerm, fnum);

	return (!fileerror (pfs, ec));
	} /*fileopenforreading*/
	
	
boolean fileopenresourcefork (ptrfilespec pfs, short *fnum) {
	
	OSErr ec;
	
	setfileerrorfile (pfs); /*set name for read/write ops that only know about fnums*/
	
	ec = FSpOpenRF (pfs, fsCurPerm, fnum);

	return (!fileerror (pfs, ec));
	} /*fileopenresourcefork*/
	
	
boolean filedeletefile (ptrfilespec pfs) {
	
	/*
	we just delete files, not folders
	*/
		
	OSErr ec;
	
	setfileerrorfile (pfs); /*set name for read/write ops that only know about fnums*/
	
	ec = FSpDelete (pfs);

	return (!fileerror (pfs, ec));
	} /*filedeletefile*/
	
	
boolean filenew (ptrfilespec pfs, OSType creator, OSType filetype, short *fnum) {
	
	OSErr errcode;

	setfileerrorfile (pfs); /*set name for read/write ops that only know about fnums*/
	
	if (FSpOpenDF (pfs, fsCurPerm, fnum) == noErr) { /*file exists, delete it*/
	
		FSClose (*fnum);
	
		filedeletefile (pfs);
		}
		
	errcode = FSpCreate (pfs, creator, filetype, smSystemScript);
	
	if (fileerror (pfs, errcode)) /*failed to open the file for writing*/
		return (false);
	
	errcode = FSpOpenDF (pfs, fsCurPerm, fnum);
	
	if (fileerror (pfs, errcode)) {
	
		FSClose (*fnum);
			
		filedeletefile (pfs);
		
		return (false); /*failed to open the file for writing*/
		}
		
	return (true); /*file exists and its open*/
	} /*filenew*/
	
	
boolean filetruncate (short fnum) {
	
	return (!fileerror (nil, SetEOF (fnum, 0)));
	} /*filetruncate*/


long filegetsize (short fnum) {
	
	/*
	get the size of a file that's already open.
	*/
	
	long filesize;
	
	if (GetEOF (fnum, &filesize) != noErr)
		filesize = 0;
	
	return (filesize);
	} /*filegetsize*/


boolean filewrite (short fnum, long ctwrite, void *buffer) {
	
	/*
	write ctwrite bytes from buffer to the current position in file number
	fnum.  return true iff successful.
	*/

	if (ctwrite > 0) {

		if (fileerror (nil, FSWrite (fnum, &ctwrite, buffer)))
		
			return (false);
		}
	
	return (true);
	} /*filewrite*/
	
	
boolean fileread (short fnum, long ctread, void *buffer) {
	
	/*
	read ctread bytes from the current position in file number fnum into
	the buffer.  return true iff successful.
	*/

	if (ctread > 0) {
		
		if (fileerror (nil, FSRead (fnum, &ctread, buffer)))
		
			return (false);
		}
			
	return (true);
	} /*fileread*/
	
	
boolean filegetchar (short fnum, byte *buffer) {
	
	/*
	read the next character from the indicated file, returning it in *buffer.
	
	return false if we're at the end of the file, without triggering an error
	dialog.
	*/
	
	long fpos, eof;
	
	if (GetFPos (fnum, &fpos) != noErr)
		return (false);
	
	if (GetEOF (fnum, &eof) != noErr)
		return (false);
	
	if (fpos == eof)
		return (false);
	
	if (!fileread (fnum, 1L, buffer))
		return (false);
		
	return (true);
	} /*filegetchar*/
	
	
boolean filewritehandle (short fnum, Handle h) {
	
	/*
	write the indicated handle to the open file indicated by fnum at the
	current position in the file.
	*/
	
	boolean fl;
	
	HLock (h);
	
	fl = filewrite (fnum, GetHandleSize (h), *h);
	
	HUnlock (h);
	
	return (fl);
	} /*filewritehandle*/
	
	
boolean filereadhandle (short fnum, long ctbytes, Handle *hreturned) {
	
	Handle h;
	boolean fl;
	long filesize;
	
	filesize = filegetsize (fnum);
	
	if (ctbytes > filesize)
		ctbytes = filesize;
	
	if (!newclearhandle (ctbytes, hreturned))
		return (false);
		
	h = *hreturned; /*copy into register*/
	
	lockhandle (h);
	
	fl = fileread (fnum, ctbytes, *h);
	
	unlockhandle (h);
		
	if (!fl) {
		
		disposehandle (h);
		
		*hreturned = nil;
		
		return (false);
		}
		
	return (true);
	} /*filereadhandle*/	


boolean filereadwholefile (short fnum, Handle *hreturned) {
	
	/*
	load the whole file into memory, return true if it worked, with
	the handle holding all the data from the file.
	*/
	
	return (filereadhandle (fnum, filegetsize (fnum), hreturned));
	} /*filereadwholefile*/

	
static sfcallback sfglobalfilter = nil;


static pascal Boolean sffilter (CInfoPBRec *pb) {
	
	/*
	if we return true, the file is filtered, meaning it is not included in the
	standard file dialog list.
	*/
	
	tyfileinfo info;
	
	if (sfglobalfilter == nil)
		return (-1);
		
	filegetinfofrompb (pb, &info);
	
	if ((*sfglobalfilter) (&info))
		return (-1);
		
	return (0);
	} /*sffilter*/


#if GENERATINGCFM

	static RoutineDescriptor sffilterDesc = BUILD_ROUTINE_DESCRIPTOR (uppFileFilterProcInfo, sffilter);

	#define sffilterUPP ((FileFilterUPP) &sffilterDesc)

#else

	#define sffilterUPP ((FileFilterUPP) &sffilter)

#endif


boolean sfdialog (boolean flput, bigstring prompt, ptrfilespec pfs, sfcallback filterproc, OSType filetype) {
	
	DialogTHndl hdialog;
	short id;
	Rect r, rscreen;
	Point pt;
	SFReply reply;
	SFTypeList typesrec;

	cometofront ();
	
	arrowcursor ();
	
	if (flput)
		id = getDlgID;
	else
		id = putDlgID;
		
	hdialog = (DialogTHndl) GetResource ('DLOG', id);
	
	if (hdialog == nil) {
		
		pt.h = pt.v = 85;
		}
	else {
		r = (**hdialog).boundsRect;
	
		rscreen = quickdrawglobal (screenBits).bounds;
	
		pt.h = rscreen.left + (((rscreen.right - rscreen.left) - (r.right - r.left)) / 2);
	
		pt.v = rscreen.top + ((rscreen.bottom - rscreen.top) - (r.bottom - r.top)) / 3;
		}

	if (flput)
		SFPutFile (pt, prompt, (*pfs).name, nil, &reply);
		
	else {
		if (filterproc == nil) {
		
			short ctfiles;
			
			if (filetype == 0) /*set filetype to 0 to get all files*/
				ctfiles = -1;
			else {
				ctfiles = 1;
				
				typesrec [0] = filetype;
				}
			
			SFGetFile (pt, prompt, nil, ctfiles, typesrec, nil, &reply);
			}
		else {	
			sfglobalfilter = filterproc;
		
			SFGetFile (pt, prompt, sffilterUPP, -1, typesrec, nil, &reply);
			}
		}
		
	if (reply.good) {
		
		clearbytes (pfs, longsizeof (tyfilespec));
		
		(*pfs).vRefNum = reply.vRefNum;
		
		copystring (reply.fName, (*pfs).name);
		}
		
	return (reply.good);
	} /*sfdialog*/

