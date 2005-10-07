
/*	$Id$    */

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

#ifdef MACVERSION
#include "mac.h"
#endif

#include "filealias.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "scrap.h"
#include "strings.h"
#include "threads.h"
#include "frontierwindows.h"
#include "file.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "langinternal.h" /*for langbackgroundtask*/
#include "versions.h" /* 2005-09-23 creedon */


#ifdef MACVERSION

#include "SetUpA5.h"

#define sfgetfileid 5000
#define sfputfileid 5001
#define sfgetfolderid 5002
#define sfgetdiskid 5003

#define sfgetfolderbutton 11
#define sfgetpromptitem 10


typedef struct tysfdata { /*data passed to hook routines*/
	
	StandardFileReply sfreply;
	
	bigstring sfprompt;
	
	Str63 sfname;
	
	ptrsftypelist sftypes;
	} tysfdata, *ptrsfdata;


#if !TARGET_API_MAC_CARBON

static pascal short sfputfilehook (short item, DialogPtr pdialog, tysfdata *pdata) {
	
	/*
	6/11/93 dmb: added for System 7 Standard File
	*/
	#ifdef flcomponent
	long curA5;
	#endif
	if (GetWRefCon (pdialog) != sfMainDialogRefCon)
		return (item);
	
	#ifdef flcomponent
	
	curA5 = SetUpAppA5 ();
	
	#endif
	
	if (item == sfHookFirstCall) {
		
		if ((*pdata).sfreply.sfFile.vRefNum != 0)
			item = sfHookChangeSelection;
		}
	
	#ifdef flcomponent
	
	RestoreA5 (curA5);
	
	#endif
	
	return (item);
	} /*sfputfilehook*/


static pascal short sfprompthook (short item, DialogPtr pdialog, tysfdata *pdata) {
	
	/*
	6/11/93 dmb: recoded to System 7 Standard File
	*/
	#ifdef flcomponent
	long curA5;
	#endif
	if (GetWRefCon (pdialog) != sfMainDialogRefCon)
		return (item);
	
	#ifdef flcomponent
	
	curA5 = SetUpAppA5 ();
	
	#endif
	
	if (item == sfHookFirstCall) {
		Rect ritem, rdialog;
		CGrafPtr	dialogPort;
		dialoggetobjectrect (pdialog, sfgetpromptitem, &ritem);
		
		//Code change by Timothy Paustian Sunday, April 30, 2000 9:20:45 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		dialogPort = GetDialogPort(pdialog);
		GetPortBounds(dialogPort, &rdialog);
		#else
		//old code
		#pragma unused(dialogPort)
		rdialog = (*pdialog).portRect;
		#endif
		if (isemptystring ((*pdata).sfprompt)) {
			
			rdialog.bottom = ritem.top;
			
			/*
			hidedialogitem (pdialog, sfgetpromptitem);
			*/
			}
		else {
			
			rdialog.bottom = ritem.bottom + 4;
			
			setdialogtext (pdialog, sfgetpromptitem, (*pdata).sfprompt);
			}
		
		sizewindow (pdialog, rdialog.right - rdialog.left, rdialog.bottom - rdialog.top);
		
		if ((*pdata).sfreply.sfFile.vRefNum != 0)
			item = sfHookChangeSelection;
		}
	
	#ifdef flcomponent
	
	RestoreA5 (curA5);
	
	#endif
	
	return (item);
	} /*sfprompthook*/


static pascal short sffolderhook (short item, DialogPtr pdialog, tysfdata *pdata) {
	
	/*
	12/5/91 dmb: if a folder is selected, use it instead of the current folder
	
	1/30/92 dmb: added code to handle folder aliases, which are files
	
	9/15/92 dmb: save path in sfstring (255 chars) instead of reply.fName (63 chars)
	
	6/11/93 dmb: recoded to System 7 Standard File
	
	2.1b9 dmb: push dialog port when ellipsizing button title
	*/
	
	bigstring bs;
	#ifdef flcomponent
	long curA5;
	#endif
	
	if (GetWRefCon (pdialog) != sfMainDialogRefCon)
		return (item);
	
	#ifdef flcomponent
	
	curA5 = SetUpAppA5 ();
	
	#endif
	
	item = sfprompthook (item, pdialog, pdata);
	
	switch (item) {
		
		case sfgetfolderbutton: {
		
			if ((*pdata).sfreply.sfFlags & kIsAlias) { /*must be the alias of a folder*/
				Boolean flfolder, flwasalias;
				OSErr errcode;
				
				errcode = ResolveAliasFile (&(*pdata).sfreply.sfFile, true, &flfolder, &flwasalias);
				
				switch (errcode) {
					
					case noErr:
						break;
					
					case userCanceledErr:
						item = sfHookNullEvent;
						
						goto exit;
					
					default:
						getsystemerrorstring (errcode, bs);
						
						parsedialogstring (bs, (*pdata).sfreply.sfFile.name, nil, nil, nil, bs);
						
						customalert (sferrordialogid, bs);
						
						item = sfHookNullEvent;
						
						goto exit;
					}
				}
			
			(*pdata).sfreply.sfGood = true;
			
			item = sfItemCancelButton; /*force exit*/
			
			break;
			};
		
		case sfHookNullEvent:
			if (!equalstrings ((*pdata).sfname, (*pdata).sfreply.sfFile.name)) { /*selection changed*/
				FSSpec fs;
				
				fs = (*pdata).sfreply.sfFile;
				
				copystring (fs.name, (*pdata).sfname); /*remember for next time before changing*/
				
				if (isemptystring (fs.name)) /*nothing selected*/
					FSMakeFSSpec (fs.vRefNum, fs.parID, nil, &fs);
				
				copystring (fs.name, bs);
				//Code change by Timothy Paustian Monday, August 21, 2000 4:20:21 PM
				//pushport must have a CGrafPtr on OS X
				{
				CGrafPtr	thePort;
				#if TARGET_API_MAC_CARBON == 1
				thePort = GetDialogPort(pdialog);
				#else
				thePort = (CGrafPtr)pdialog;
				#endif
		
				pushport (thePort);
				}
				ellipsize (bs, 72);
				
				popport ();
				
				setdialogbutton (pdialog, sfgetfolderbutton, bs);
				}
			
			break;
		}
	
	exit:
	
	#ifdef flcomponent
	
	RestoreA5 (curA5);
	
	#endif
	
	return (item);
	} /*sffolderhook*/


static pascal short sfdiskhook (short item, DialogPtr pdialog, tysfdata *pdata) {
	
	/*
	6/11/93 dmb: recoded to System 7 Standard File; back to using Drive 
	button like Frontier 1.0.
	*/
	#ifdef flcomponent
	long curA5;
	#endif
	if (GetWRefCon (pdialog) != sfMainDialogRefCon)
		return (item);
	
	#ifdef flcomponent
	
	curA5 = SetUpAppA5 ();
	
	#endif
	
	item = sfprompthook (item, pdialog, pdata);
	
	switch (item) {
		
		case sfItemOpenButton:
		case sfHookOpenFolder:
			#if 0
			
			(*pdata).sfreply.vRefNum = -SFSaveDisk; /*IM IV-72*/
			
			setemptystring ((*pdata).sfreply.fName); /*we just want the volume*/
			
			#endif
			
			FSMakeFSSpec ((*pdata).sfreply.sfFile.vRefNum, 0, 0, &(*pdata).sfreply.sfFile);
			
			(*pdata).sfreply.sfGood = true;
			
			item = sfItemCancelButton; /*force exit*/
			
			break;
		
		case sfHookGoToDesktop:
			item = sfHookGoToNextDrive;
			
			break;
		
		case sfHookNullEvent:
			if ((*pdata).sfreply.sfIsVolume) /*need to open volume to enable Drive button*/
				item = sfHookOpenFolder;
			else {
				if (isemptystring ((*pdata).sfreply.sfFile.name)) /*no selection*/
					item = sfHookGoToParent;
				}
			
			break;
		
		case sfHookGoToParent:
			item = sfHookNullEvent;
			
			break;
		
		default:
			if (item >= sfHookCharOffset) /*typing -- ignore*/
				item = sfHookNullEvent;
		}
	
	#ifdef flcomponent
	
	RestoreA5 (curA5);
	
	#endif
	
	return (item);
	} /*sfdiskhook*/


static pascal Boolean onlyfoldersfilter (ParmBlkPtr pb, tysfdata *pdata) {
	
	#pragma unused (pdata)
	
	/*
	if (foldertest (pb))
	*/
	
	if (pb->fileParam.ioFlAttrib & ioDirMask)
		return (0);
	
	return (-1); /*...don't show files*/
	} /*onlyfoldersfilter*/


static pascal Boolean knowntypesfilter (ParmBlkPtr pb, tysfdata *pdata) {
	
	short i;
	
	if (pb->fileParam.ioFlAttrib & ioDirMask)
		return (0);
	
	if (pdata->sftypes == nil) // show all files
		return (0);
	
	for (i = 0; i < pdata->sftypes->cttypes; ++i) {
		
		OSType type = pdata->sftypes->types [i];
		byte bstype [6];
		bigstring bssuffix;
		
		ostypetostring (type, bstype);
		
		lastword (pb->fileParam.ioNamePtr, '.', bssuffix);
		
		if (stringlength (bssuffix) == 3) //handle 8.3 names
			setstringlength (bstype, 3);
		
		if (equalidentifiers (bssuffix, bstype))
			return (0);
		
		if (pb->fileParam.ioFlFndrInfo.fdType == type)
			return (0);
		}
	
	return (-1); // didn't find it in our list
	} /*knowntypesfilter*/


		
#if !TARGET_RT_MAC_CFM
	
	#define onlyfoldersfilterUPP ((FileFilterYDUPP) &onlyfoldersfilter)
	#define knowntypesfilterUPP ((FileFilterYDUPP) &knowntypesfilter)
	#define sfputfilehookUPP (&sfputfilehook)
	#define sfprompthookUPP (&sfprompthook)
	#define sffolderhookUPP (&sffolderhook)
	#define sfdiskhookUPP (&sfdiskhook)

#else

	#if !TARGET_API_MAC_CARBON
	static RoutineDescriptor onlyfoldersfilterDesc = BUILD_ROUTINE_DESCRIPTOR (uppFileFilterYDProcInfo, onlyfoldersfilter);
	static RoutineDescriptor knowntypesfilterDesc = BUILD_ROUTINE_DESCRIPTOR (uppFileFilterYDProcInfo, knowntypesfilter);
	static RoutineDescriptor sfputfilehookDesc = BUILD_ROUTINE_DESCRIPTOR (uppDlgHookYDProcInfo, sfputfilehook);
	static RoutineDescriptor sfprompthookDesc = BUILD_ROUTINE_DESCRIPTOR (uppDlgHookYDProcInfo, sfprompthook);
	static RoutineDescriptor sffolderhookDesc = BUILD_ROUTINE_DESCRIPTOR (uppDlgHookYDProcInfo, sffolderhook);
	static RoutineDescriptor sfdiskhookDesc = BUILD_ROUTINE_DESCRIPTOR (uppDlgHookYDProcInfo, sfdiskhook);

	
	#define onlyfoldersfilterUPP (&onlyfoldersfilterDesc)
	#define knowntypesfilterUPP (&knowntypesfilterDesc)
	#define sfputfilehookUPP (&sfputfilehookDesc)
	#define sfprompthookUPP (&sfprompthookDesc)
	#define sffolderhookUPP (&sffolderhookDesc)
	#define sfdiskhookUPP (&sfdiskhookDesc)
	
	#endif


#endif

#endif /* !TARGET_API_MAC_CARBON */

	
	boolean sfdialog (tysfverb sfverb, bigstring bsprompt, ptrsftypelist filetypes, tyfilespec *fspec, OSType filecreator) {
	
	/*
	return true if the user selected a file with one of the SF routines,
	return false otherwise.
	
	as a bonus, we return the full path for the selected file in the path string.
	
	2005-10-06 creedon: added filecreator parameter, used for get file dialog
	
	2005-09-21 creedon: changed from TimsGetFile to getafile
	
	Tuesday, June 20, 2000 8:50:41 PM Timothy Paustian: I am going to hack the heck out of this routine. See what you think
	
	4.1b13 dmb: un-commented out the shellwritescrap and shellactive calls.
	I don't know how they got commented; I must have been experimenting with 
	something.
	
	2.1b2 dmb: updated interface to be filespec-based. for putfile, the name in the 
	filespec is the default name. for all verbs, a non-empty filespec seeds the dialog
	
	2.1b1 dmb: set sfdata.sfname to an impossible value to ensure button update
	
	6/11/93 dmb: recoded to System 7 Standard File
	
	12/18/92 dmb: call shellwritescrap
	
	9/15/92 dmb: when picking a folder, grab path from sfstring instead of reply
	record.
	
	2/10/92 dmb: added call to new shellactivate; standard file breaks when brought 
	up in the background.
	
	12/27/91 dmb: if the default fname includes a colon, attempt to set default 
	directory, and remove folder specification from dialog default
	
	4/16/91 dmb: replaced filetype parameter with pointer to type list.  callers 
	can pass nil when irrelevant, or when all types are to be shown.
	
	11/16/90 dmb:  replace flput boolean with sfverb enum.  added code for folder 
	and disk selection dialogs

	8/1/90 dmb:  if filetype is zero, show all files by passing zero to SFGetFile
	*/
	
	Str255 bs;
	tysfdata sfdata;
	FSSpec *fs = &sfdata.sfreply.sfFile;
	OSErr	anErr = noErr;
	#if !TARGET_API_MAC_CARBON
	 	Point pt = {-1, -1};
	 	short cttypes = -1;
	 	OSType *types = nil;
	#endif	
	#ifdef flcomponent
	long appA5;
	#endif

	//move the switch statement to below because it's smarter to call it there when using 
	//the new routines for Nav services.
	
	clearbytes (&sfdata, sizeof (sfdata));
	
	copystring (bsprompt, sfdata.sfprompt);
	
	sfdata.sftypes = filetypes;
	
	setemptystring (bs);
	
	if (!isemptystring ((*fspec).name)) { /*if path is included, set default dir and strip to file name*/
		
		*fs = *fspec; /*seed directory & file selection*/
		
		/*
		if (pathtofilespec (fname, fs) && ((*fs).vRefNum != 0))
			copystring ((*fs).name, fname);
		else
			filefrompath (fname, fname);
		*/
		}
	
	setstringlength (sfdata.sfname, -1); /*make sure it can't match fsspec*/
	
	shellwritescrap (textscraptype);
	
	shellactivate ();
	
	#ifdef flcomponent
	
	appA5 = SetUpCurA5 (); /*for system*/
	
	#endif
	
	switch (sfverb) {
		
		case sfputfileverb:
			if(gCanUseNavServ) {
				anErr = TimsPutFile(bsprompt, (*fs).name, &sfdata.sfreply);
				}
			#if TARGET_API_MAC_CARBON != 1
			//we can get away with this because nav services is always there in OSX.
			else
				CustomPutFile (bsprompt, (*fs).name, &sfdata.sfreply, sfputfileid, pt, 
						sfputfilehookUPP, nil, nil, nil, &sfdata);
			#endif

			break;
		
		case sfgetfileverb:
			if(gCanUseNavServ) 
				anErr = getafile (bsprompt, filetypes, &sfdata.sfreply, filecreator);
			#if !TARGET_API_MAC_CARBON
			else
				CustomGetFile (knowntypesfilterUPP, cttypes, types, &sfdata.sfreply, sfgetfileid, pt, 
						sfprompthookUPP, nil, nil, nil, &sfdata);
			#endif
			break;
		
		case sfgetfolderverb:
			if(gCanUseNavServ) 
				anErr = TimsGetFolderOrVolume(bsprompt, sfgetfolderid, &sfdata.sfreply);
			#if !TARGET_API_MAC_CARBON
			else
				CustomGetFile (onlyfoldersfilterUPP, cttypes, types, &sfdata.sfreply, sfgetfolderid, pt, 
						sffolderhookUPP, nil, nil, nil, &sfdata);
			#endif
			break;
		
		case sfgetdiskverb:
			if(gCanUseNavServ) 
				anErr = TimsGetFolderOrVolume(bsprompt, sfgetdiskid, &sfdata.sfreply);
			#if !TARGET_API_MAC_CARBON
			else
				CustomGetFile (nil, cttypes, types, &sfdata.sfreply, sfgetdiskid, pt, 
						sfdiskhookUPP, nil, nil, nil, &sfdata);
			#endif

			break;
		}
	
	//code moved to above
	//if (sfverb == sfputfileverb)
		//Code change by Timothy Paustian Tuesday, June 20, 2000 8:44:39 PM
		//I added nav services as the dialog. This works well so I updated it
		//in the PPC version also.
		//CustomPutFile (bsprompt, (*fs).name, &sfdata.sfreply, id, pt, 
		//	sfhook, nil, nil, nil, &sfdata);
	//else
		//Code change by Timothy Paustian Tuesday, June 20, 2000 2:58:33 PM
		//Use nav services verbs and a switch statment
		
		//CustomGetFile (sffilefilter, cttypes, types, &sfdata.sfreply, id, pt, 
		//	sfhook, nil, nil, nil, &sfdata);
	
	#ifdef flcomponent
	
	RestoreA5 (appA5);
	
	#endif
	//if the user canceled return false
	//I know that oserror can handle this, but lets make it
	//obvious that we are checking.
	if (userCanceledErr == anErr) 
		return false;

	if(oserror(anErr)) 
		return false;

	if (sfdata.sfreply.sfGood) {
		
		FSMakeFSSpec ((*fs).vRefNum, (*fs).parID, (*fs).name, fspec); /*canonize*/
		
		/*
		*fspec = sfdata.sfreply.sfFile;
		*/
		return (true);
		}
	
	return (false);
	} /*sfdialog*/

#endif

#ifdef WIN95VERSION
static void buildfilter (char * filter, short * len, bigstring bsname, bigstring bsext) {
	short namelen, extlen;

	namelen = stringlength (bsname);
	extlen = stringlength (bsext);

	memmove (filter + *len, stringbaseaddress(bsname), namelen);
	*len = *len + namelen;
	memmove (filter + *len, "\0", 1);
	*len = *len + 1;
	memmove (filter + *len, stringbaseaddress(bsext), extlen);
	*len = *len + extlen;
	memmove (filter + *len, "\0", 1);
	*len = *len + 1;
	memmove (filter + *len, "\0", 1);  //alway finish the filter but don't count it in the length
	} /*buildfilter*/


boolean sfdialog (tysfverb sfverb, bigstring bsprompt, ptrsftypelist filetypes, tyfilespec *fspec, OSType filecreator) {

	#pragma unused (filecreator)
	
	/*
	2005-10-06 creedon: added filecreator, unused on Windows
	
	5.0.2b4 dmb: fixed bug in above change that would generate an error for empty paths

	5.0.1 dmb: make sure default file and directory are valid, or we'll fail (silently)
	*/

	TCHAR szFile[MAX_PATH];
	OPENFILENAME OpenFileName;
	BROWSEINFO BrowseInfo;
	LPITEMIDLIST itemList;
	char title [256];
	char filter [1024];
	short filterlen = 0;
	char defaultdir [256];
	char defaultfile [256];
	bigstring extension;
	byte type [6];
	bigstring osstring;
	boolean fl = false;
	short i;
	boolean fldatabases = false;
	boolean flfatpages = false;
	// Global pointer to the shell's IMalloc interface.  
	static LPMALLOC pMalloc = NULL;

	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner         = shellframewindow;
	OpenFileName.hInstance         = shellinstance;
	OpenFileName.lpstrFilter       = NULL;
	OpenFileName.lpstrCustomFilter = NULL;
	OpenFileName.nMaxCustFilter    = 0;
	OpenFileName.nFilterIndex      = 0;
	OpenFileName.lpstrFile		   = stringbaseaddress(fsname (fspec));
	OpenFileName.nMaxFile          = sizeof(fsname (fspec)) - 2;
	OpenFileName.lpstrFileTitle    = NULL;
	OpenFileName.nMaxFileTitle     = 0;
	OpenFileName.lpstrTitle			= title;
	OpenFileName.lpstrInitialDir	= NULL;
	OpenFileName.lpstrTitle			= NULL;
	OpenFileName.nFileOffset       = 0;
	OpenFileName.nFileExtension    = 0;
	OpenFileName.lpstrDefExt       = NULL;
	OpenFileName.lCustData         = (LPARAM)NULL;
	OpenFileName.lpfnHook 		   = NULL;
	OpenFileName.lpTemplateName    = 0;

	strcpy (szFile, "");
	
	// set the title
	if (bsprompt != NULL) {
		
		copyptocstring (bsprompt, title);
		
		OpenFileName.lpstrTitle = title;
		}

	// set default dir, file and extension fields
	if (!isemptystring (fsname (fspec))) {

		tyfilespec fsdir;
		boolean flfolder;

		folderfrompath (fsname (fspec), defaultdir);
		
		if (pathtofilespec (defaultdir, &fsdir) && 
			fileexists (&fsdir, &flfolder) && flfolder) {
			
			OpenFileName.lpstrInitialDir = defaultdir;
			
			//if (!isemptystring (defaultdir))
			//	OpenFileName.nFileOffset = stringlength (defaultdir) + 1;
			
			convertpstring (defaultdir);
			}
		
		filefrompath (fsname (fspec), defaultfile);

		lastword (defaultfile, ':', defaultfile); //skip any Mac path 

		//OpenFileName.nFileExtension = stringlength (fsname (fspec));
		
		lastword (defaultfile, '.', extension);
		
		//if (stringlength (extension) < stringlength (fsname (fspec)))
		//	OpenFileName.nFileExtension -= stringlength (extension);
		
		copystring (defaultfile, fsname (fspec));

		nullterminate (fsname (fspec));
		}

	releasethreadglobals ();
	
	switch (sfverb) {
		
		case sfputfileverb:
			OpenFileName.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_OVERWRITEPROMPT; //| OFN_NOCHANGEDIR;
			
			if (filetypes != nil) {
				setemptystring (filter);

				//RAB: 1/22/98 use string since the windows type can be any case
				ostypetostring ((*filetypes).types [0], osstring);

				if (equalidentifiers (osstring, "\x04" "fatp"))
					copystring ("\x016" "Fat Page [*.fatp]\0*.*\0", filter);

				else if (equalidentifiers (osstring, "\x04" "ftop"))
					copystring ("\x015" "Outline [*.ftop]\0*.*\0", filter);

				else if (equalidentifiers (osstring, "\x04" "ftwp"))
					copystring ("\x019" "WP Document [*.ftwp]\0*.*\0", filter);

				else if (equalidentifiers (osstring, "\x04" "fttb"))
					copystring ("\x013" "Table [*.fttb]\0*.*\0", filter);

				else if (equalidentifiers (osstring, "\x04" "ftmb"))
					copystring ("\x012" "Menu [*.ftmb]\0*.*\0", filter);

				else if (equalidentifiers (osstring, "\x04" "ftsc"))
					copystring ("\x014" "Script [*.ftsc]\0*.*\0", filter);

				else if (equalidentifiers (osstring, "\x04" "ftds"))
					copystring ("\x01c" "Desktop Script [*.ftds]\0*.*\0", filter);

				else if (equalidentifiers (osstring, "\x04" "root"))
					copystring ("\x016" "Database [*.root]\0*.*\0", filter);

				if (! isemptystring (filter)) {
					convertpstring (filter);

					OpenFileName.lpstrFilter = filter;
					}

				ostypetostring ((*filetypes).types [0], type);
				
				popleadingchars (type, '.');

				poptrailingwhitespace (type);
					
//				convertpstring (type);
				
//				OpenFileName.lpstrDefExt = type;
				OpenFileName.lpstrDefExt = NULL;
				}

			// Call the common dialog function.
			fl = GetSaveFileName (&OpenFileName);

			if (fl && (filetypes != NULL) && (stringlength(type) > 0)) {

				setstringlength (fsname (fspec), strlen(stringbaseaddress(fsname (fspec))));
				lastword (fsname (fspec), '.', extension);

				if ((stringlength (fsname (fspec)) == stringlength (extension)) || (stringlength (extension) > 4)) {	/* no extension */
					pushstring ("\x01.", fsname(fspec));
					pushstring (type, fsname(fspec));
					nullterminate (fsname (fspec));
					}
				}
		
			break;
		
		case sfgetfileverb:
			OpenFileName.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; //| OFN_NOCHANGEDIR;

			// create the filter string
			if (filetypes != nil) { /*showing specific files*/

				//copystring ("\x09Types: [\0", filter);
				setemptystring (extension);

				for (i = 0; i < (*filetypes).cttypes; ++i) {
					
					if (i > 0)
						pushchar (';', extension);
					
					if ((*filetypes).types [i] == 'root')
						fldatabases = true;

					if ((*filetypes).types [i] == 'fatp')
						flfatpages = true;

					if ((*filetypes).types [i] == 'ROOT')
						fldatabases = true;

					if ((*filetypes).types [i] == 'FATP')
						flfatpages = true;

					ostypetostring ((*filetypes).types [i], type);
					
					popleadingchars (type, '.');

					poptrailingwhitespace (type);
					
					pushstring ("\x02" "*.", extension);

					pushstring (type, extension);

					if (stringlength (type) > 3) {
						
						setstringlength (type, 3);

						pushstring ("\x03" ";*.", extension);
						
						pushstring (type, extension);
						}
					}
				

				filterlen = 0;

				buildfilter (filter, &filterlen, "\x1c" "Openable Types: [*.clickers]", extension);
				
				if (fldatabases)
					buildfilter (filter, &filterlen, "\x12" "Databases [*.root]", "\x0c" "*.root;*.roo");

				//					pushstring ("\x20\0Databases [*.root]\0*.roo;*.root", filter);

				if (flfatpages) {
					buildfilter (filter, &filterlen, "\x12" "Fat Pages [*.fatp]", "\x5a" "*.fatp;*.fat;*.FTsc;*.FTs;*.FTwp;*.FTw;*.FTop;*.FTo;*.FTmb;*.FTm;*.FTtb;*.FTt;*.Ftds;*.FTd");
					buildfilter (filter, &filterlen, "\x17" "Frontier Menus [*.FTmb]",    "\x0c" "*.FTmb;*.FTm");
					buildfilter (filter, &filterlen, "\x1a" "Frontier Outlines [*.FTop]", "\x0c" "*.FTop;*.FTo");
					buildfilter (filter, &filterlen, "\x19" "Frontier Scripts [*.FTsc]",  "\x0c" "*.FTsc;*.FTs");
					buildfilter (filter, &filterlen, "\x21" "Frontier Desktop Scripts [*.FTds]",  "\x0c" "*.FTds;*.FTd");
					buildfilter (filter, &filterlen, "\x18" "Frontier Tables [*.FTtb]",	  "\x0c" "*.FTtb;*.FTt");
					buildfilter (filter, &filterlen, "\x19" "Frontier WP Text [*.FTwp]",  "\x0c" "*.FTwp;*.FTw");
					}

				//					pushstring ("\x20\0Fat Pages [*.fatp]\0*.fat;*.fatp", filter);

				buildfilter (filter, &filterlen, "\x12" "Plain Text [*.txt]",  "\x12" "*.Text;*.txt;*.tex");
				buildfilter (filter, &filterlen, "\x0f" "All Files [*.*]",  "\x03" "*.*");

				OpenFileName.lpstrFilter = filter;

				ostypetostring ((*filetypes).types [0], type);
				
				//RAB: 1/22/98  added next two lines
				//				poptrailingwhitespace is no longer done in ostypetostring.
				popleadingchars (type, '.');

				poptrailingwhitespace (type);

				convertpstring (type);
				
				OpenFileName.lpstrDefExt = type;
				}

			// Call the common dialog function.
			fl = GetOpenFileName (&OpenFileName);
		
			break;
		
		case sfgetfolderverb:
		case sfgetdiskverb:
			// Get the shell's allocator. 
			if (pMalloc == NULL && !SUCCEEDED(SHGetMalloc(&pMalloc))) 
				break; 
 
			BrowseInfo.hwndOwner = OpenFileName.hwndOwner;
			BrowseInfo.pidlRoot = NULL;
			BrowseInfo.pszDisplayName = szFile;
			BrowseInfo.lpszTitle = OpenFileName.lpstrTitle;
			BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
			if (sfverb == sfgetdiskverb)
				BrowseInfo.ulFlags |= BIF_RETURNFSANCESTORS;
			BrowseInfo.lpfn = NULL;
			BrowseInfo.lParam = 0;
			
			itemList = SHBrowseForFolder (&BrowseInfo);
			
			if (itemList != NULL) {

				fl = SHGetPathFromIDList (itemList, szFile);

				copyctopstring (szFile, fsname (fspec));

				if (sfverb == sfgetdiskverb) {

					firstword (fsname (fspec), ':', fsname (fspec));
				
					pushstring ("\x02:\\", fsname (fspec));
					}
				else {

					pushstring ("\x01\\", fsname (fspec));
					}
				
				nullterminate (fsname (fspec));

				// deallocate itemList
				pMalloc->lpVtbl->Free (pMalloc, itemList);

		/*
		LPSHELLFOLDER ppshf;
		if (SHGetDesktopFolder (&ppshf) == NOERROR) {
			ULONG ctchars;
			WCHAR szWide [300];

			MultiByteToWideChar (CP_ACP, 0, szFile, -1, szWide, 300);

			ppshf->lpVtbl->ParseDisplayName (ppshf, NULL, szWide, &ctchars, &itemList,NULL);

			fl = SHGetPathFromIDList (itemList, szFile);

			pMalloc->lpVtbl->Free (pMalloc, itemList);

			ppshf->lpVtbl->Release (ppshf);
			}
		*/
				}
			
			break;
		
		}
	
	grabthreadglobals ();
	
	if (!fl) {
		
		oserror (GetLastError ());
		
		return (false);
		}
	
	setstringlength (fsname (fspec), strlen(stringbaseaddress(fsname (fspec))));
	
	return (true);
	} /*sfdialog*/

#endif


#ifdef MACVERSION

boolean initfiledialog (void) {

	#ifdef flcomponent
	
	#if !TARGET_API_MAC_CARBON
	RememberA5 ();
	#endif /*for hook*/
	
	#endif
	
	return (true);
	} /*initfile*/


//Code change by Timothy Paustian Tuesday, June 20, 2000 2:55:17 PM
//New routine to use Nav services for this instead of CustomPutFile.
OSErr
TimsPutFile(bigstring prompt, Str255 fileName, StandardFileReply * 	outReply)
{

    OSErr               anErr = noErr;
    NavReplyRecord      reply;
    NavDialogOptions    dialogOptions;
    OSType              fileTypeToSave = 'TEXT';
    NavEventUPP         eventProc = NewNavEventUPP(NavEventProc);

    anErr = NavGetDefaultDialogOptions(&dialogOptions);
    copystring(fileName, dialogOptions.savedFileName);
    copystring(prompt, dialogOptions.message);
    dialogOptions.dialogOptionFlags |= kNavNoTypePopup; /* 08/25/2000 AR */
    if (anErr == noErr)
    {
        anErr = NavPutFile( nil, &reply, &dialogOptions, eventProc,
                            fileTypeToSave, 'LAND', nil);

        if (anErr == noErr && reply.validRecord)
        {
            AEKeyword   theKeyword;
            DescType    actualType;
            Size        actualSize;
            FSSpec      documentFSSpec;

            anErr = AEGetNthPtr(&(reply.selection), 1, typeFSS,
                                &theKeyword, &actualType,
                                &documentFSSpec, sizeof(documentFSSpec),
                                &actualSize );
           if (anErr == noErr)
            {

                outReply->sfReplacing = reply.replacing;
                FSMakeFSSpec (documentFSSpec.vRefNum, documentFSSpec.parID, documentFSSpec.name, &(outReply->sfFile));
                outReply->sfGood = true;
            }
            // Always call NavCompleteSave() to complete
			#if TARGET_API_MAC_CARBON != 1
				anErr = NavCompleteSave(&reply, kNavTranslateInPlace);
			#endif
 
            (void) NavDisposeReply(&reply);
        }
    }
    DisposeNavEventUPP(eventProc);
    return anErr;
}


OSErr getafile (bigstring prompt, ptrsftypelist filetypes, StandardFileReply * outReply, OSType filecreator) {

	/*
	2005-10-06 creedon: added filecreator parameter
	
	2005-09-21 creedon: created, cribbed from TimsGetFile
	*/ 

	NavDialogCreationOptions	dialogOptions;
	NavDialogRef			dialogRef;
	NavEventUPP			eventProc = NewNavEventUPP (NavEventProc);
	NavReplyRecord			reply;
	NavTypeListHandle		typeList = nil;
	OSErr				anErr = noErr;

	anErr = NavGetDefaultDialogCreationOptions (&dialogOptions);
	
	dialogOptions.clientName = CFStringCreateWithCString (NULL, APPNAME_SHORT, kCFStringEncodingMacRoman);
	dialogOptions.message = CFStringCreateWithPascalString (NULL, prompt, kCFStringEncodingMacRoman);
	
	if (anErr == noErr) {
	
		if (filetypes == nil)
			dialogOptions.optionFlags -= kNavNoTypePopup;
		else
			dialogOptions.optionFlags += kNavAllFilesInPopup; // add all documents to show pop-up

		dialogOptions.optionFlags ^=kNavAllowMultipleFiles; //no multiple files for now
        
		dialogOptions.optionFlags ^= kNavAllowPreviews; // clear preview option
        
		dialogOptions.optionFlags += kNavSupportPackages; // see packages
		
		// dialogOptions.dialogOptionFlags += kNavAllowOpenPackages; // can open packages

		if (filetypes != nil) { // translate into a type list NavServices understands
		
			NavTypeListPtr	typesP = nil;
			SInt32		hSize = (sizeof (NavTypeList) + sizeof (OSType) * (filetypes->cttypes - 1));
			newhandle		(hSize, (Handle*) &typeList);
			typesP		= (NavTypeListPtr) *((Handle) typeList);
			
			typesP->componentSignature = filecreator;
			typesP->reserved = 0;
			typesP->osTypeCount = filetypes->cttypes;
			
			BlockMoveData (&(filetypes->types), typesP->osType, (Size) (sizeof (OSType) * filetypes->cttypes));
			}
        
		anErr = NavCreateGetFileDialog (&dialogOptions, typeList, eventProc, NULL, NULL, NULL, &dialogRef);

		anErr = NavDialogRun (dialogRef);
		
		anErr = NavDialogGetReply (dialogRef, &reply);

		if (anErr == noErr && reply.validRecord) {
		
			AEKeyword theKeyword;
			DescType actualType;
			Size actualSize;
			FSSpec documentFSSpec;

			anErr = AEGetNthPtr (&(reply.selection), 1, typeFSS, &theKeyword, &actualType, &documentFSSpec, sizeof (documentFSSpec), &actualSize); // get a pointer to selected file
			
			assert (actualType == typeFSS);
		    
			if (anErr == noErr) {
				FSMakeFSSpec (documentFSSpec.vRefNum, documentFSSpec.parID, documentFSSpec.name, &(outReply->sfFile));
			
				outReply->sfGood = true;
				}

			anErr = NavDisposeReply (&reply); // dispose of NavReplyRecord, resources, descriptors
			}
		}
		
	DisposeNavEventUPP (eventProc);
	
	NavDialogDispose (dialogRef);
 
	return anErr;
	} /* getafile */


OSErr
TimsGetFolderOrVolume(bigstring prompt, SInt16 dialogType, StandardFileReply * 	outReply)
{
	NavDialogOptions    	dialogOptions;
    NavEventUPP         	eventProc = NewNavEventUPP(NavEventProc);
    OSErr       			anErr = noErr;
  	NavReplyRecord 			reply;
        
    //  Specify default options for dialog box
    //we don't really need to modify this, but it is needed for NavChooseFolder
    anErr = NavGetDefaultDialogOptions(&dialogOptions);
     copystring(prompt, dialogOptions.message);
    
    if(anErr == noErr)
    {
    	//display the dialog
    	if(sfgetfolderid == dialogType)
    		anErr = NavChooseFolder(nil, &reply, &dialogOptions, eventProc, nil, nil);
    	else
    	{
    		assert(sfgetdiskid == dialogType); 
    		anErr = NavChooseVolume(nil, &reply, &dialogOptions, eventProc, nil, nil);
    	}
    	
    	if (anErr == noErr && reply.validRecord)
        {
            AEKeyword   theKeyword;
            DescType    actualType;
            Size        actualSize;
            FSSpec      documentFSSpec;
            
            // Get a pointer to selected file
            anErr = AEGetNthPtr(&(reply.selection), 1,
                                typeFSS, &theKeyword,
                                &actualType, &documentFSSpec,
                                sizeof(documentFSSpec),
                                &actualSize);
 			assert(actualType == typeFSS);       
            if (anErr == noErr)
            {
                FSMakeFSSpec (documentFSSpec.vRefNum, documentFSSpec.parID, documentFSSpec.name, &(outReply->sfFile));
                outReply->sfGood = true;
            }
            //  Dispose of NavReplyRecord
            anErr = NavDisposeReply(&reply);
        }
    }
     DisposeNavEventUPP (eventProc);
     return anErr;
}






//Code change by Timothy Paustian Tuesday, June 20, 2000 9:07:26 PM
//a very simple event proc so that Nav file service dialogs are movable and resizable.
pascal void NavEventProc(NavEventCallbackMessage callBackSelector,
                        NavCBRecPtr callBackParms,
                        NavCallBackUserData callBackUD)
{
   #pragma unused(callBackUD)
    if (callBackSelector == kNavCBEvent)
    {
        if(((callBackParms->eventData)
                    .eventDataParms).event->what == updateEvt)
           	{
            	//I was having a crash due to getting the window ptr outside the switch statement.
            	// This now works.
            	//10/30/00 Timothy Paustian
            	WindowPtr window = (WindowPtr)(((callBackParms->eventData).eventDataParms).event)->message;
              	shellupdatenow(window);
            }
    }
}

#endif
