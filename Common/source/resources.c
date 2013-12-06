
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

#include "error.h"
#include "memory.h"
#include "strings.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "file.h"
#include "resources.h"
#include "langinternal.h" /* 2006-01-30 creedon */


#ifdef WIN95VERSION
	extern HINSTANCE hInst;
#endif


boolean getstringlist (short listnum, short id, bigstring bs) {
#ifdef MACVERSION	
	
//	fprintf(stderr, "getstringlist(%d, %d, %.*s\n", listnum, id, bs[0], &bs[1]);
	
	CFStringRef stringKey = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d.%d"), listnum, id);
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFStringRef stringValue = CFBundleCopyLocalizedString(mainBundle, stringKey, CFSTR("[string not found]"), CFSTR("Localizable"));\
	
	if (CFStringCompare(stringValue, CFSTR("[string not found]"), (CFStringCompareFlags) 0) == kCFCompareEqualTo) {
//		shellerrormessage("\pString Resource Not Found");
		fprintf(stderr, "string resource not found: %d.%d\n", listnum, id);
		
		setemptystring(bs);
	} else {
		CFStringGetPascalString(stringValue, bs, sizeof(bigstring), kCFStringEncodingMacRoman);
	}
	
	CFRelease(stringValue);
	CFRelease(stringKey);
	
	return (stringlength (bs) > 0);
#endif

#ifdef WIN95VERSION
	HANDLE reshandle, hdata;
	
	setstringlength(bs,0);
	reshandle = FindResource (hInst, MAKEINTRESOURCE(id), MAKEINTRESOURCE(listnum));
	if (reshandle != NULL)
		{
		hdata = LoadResource (hInst, reshandle);
		strcpy (stringbaseaddress(bs), LockResource(hdata));
		setstringlength (bs, strlen (stringbaseaddress(bs)));
		}
	return (stringlength (bs) > 0);
#endif
	} /*getstringlist*/
	

boolean findstringlist (bigstring bs, short listnum, short *id) {
	
	/*
	search in the indicated STR# resource for a string equal to bs.  if
	the string was found, return its index in id.  the search is unicase.
	*/
	
	bigstring bsid;
	bigstring bsresource;
	register short ixresource = 1;
	
	*id = 0; /*default returned value*/
	
	copystring (bs, bsid); /*work on a copy*/
	
	alllower (bsid); /*search is unicase*/
	
	while (true) {
		
		if (!getstringlist (listnum, ixresource, bsresource)) /*ran out of strings in STR# resource*/
			return (false);
			
		alllower (bsresource); /*search is unicase*/
		
		if (equalstrings (bsid, bsresource)) {
			
			*id = ixresource;
			
			return (true);
			}
			
		ixresource++; /*advance to next string*/
		} /*while*/
	} /*findstringlist*/
	

boolean closeresourcefile (short rnum) {
#ifdef MACVERSION	
	/*
	close the last file opened with openresourcefile.
	
	8/20/90 dmb:  take rnum as parm.  no need to SetVol to original location
	
	12/13/91 dmb: make sure we never close the System or ROM resource files!
	*/
	
	if (rnum != -1) { /*resource file was successfully opened*/
		
		if (rnum > 2) /*not System or ROM*/
			CloseResFile (rnum);
		else
			UpdateResFile (rnum); /*make sure map is consistent*/
		
		if (ResError () != noErr)
			return (false);
		}
#endif	
	return (true);
	} /*closeresourcefile*/


boolean openresourcefile ( const ptrfilespec fs, short *rnum, short forktype ) {

	/*
	2006-01-30 creedon: on Mac added check for non-carbon OS and trying to access the data fork of a file, error message
	
	2005-09-02 creedon: on Mac added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
		
	#ifdef MACVERSION
	
		short resourcerefnum = -1;
		FSRef fsref;
		HFSUniStr255 fork;
		OSErr err;
			
		SetResLoad (false);
		
		switch (forktype) {
			case resourcefork:
				FSGetResourceForkName (&fork);
				break;

			case datafork:
				FSGetDataForkName (&fork);
				break;
			}

		/*
		2005-09-01 creedon - in my reading about the FSOpenResourceFile function someone mentioned
		that it might not deal well with corrupted resources and that dropping back to FSpOpenResFile
		seemed to do the trick.  i've not done that here but could be tried if the problem is real
		and manifests. 
		*/
		
		if ( oserror ( macgetfsref ( fs, &fsref ) ) )
			return ( false );
		
		err = FSOpenResourceFile ( &fsref, fork.length, fork.unicode, fsRdWrPerm, &resourcerefnum );

		SetResLoad (true);

		if (ResError () == -39 ) { /*eof error, file has no resource fork, create one*/

			err = FSCreateResourceFork (&fsref, fork.length, fork.unicode, 0);
		
			if (err != noErr) /*failed to create resource fork*/
				goto error;
			
			FSOpenResourceFile (&fsref, fork.length, fork.unicode, fsRdWrPerm, &resourcerefnum);
			}

		if (resourcerefnum != -1) /*it's open*/ {
			
			UseResFile (resourcerefnum); /*in case it was already open*/
			
			*rnum = resourcerefnum;
			
			return (true);
			}
		
	error:

		setfserrorparam ( fs ); /*in case error message takes a filename parameter*/

		oserror (ResError ());

		closeresourcefile (resourcerefnum); /*checks for -1*/

		*rnum = -1;

	#endif // MACVERSION
	
	return (false);
	} /* openresourcefile */


#ifdef MACVERSION
boolean writeresource (ResType type, short id, bigstring bsname, long ctwrite, void *pdata) {
	
	/*
	6/2/92 dmb: don't setresinfo; if the resource exists, we want to retain 
	current name & id.  note that we don't support a verb that allows a resource 
	to be put with both name & id specified, nor does Frontier do so internally.
	
	7/7/92 dmb: use Unique1ID instead of UniqueID
	
	12/18/92 dmb: don't call oserror from this routine. if the error is encountered 
	accessing the program's resource fork, bringing up the Error Info window here 
	seems to sqrew up the resource chain irreperably, leading to a bus error. so, 
	callers must make sure to check resError themselves.
	*/
	
	Handle hresource;
	
	if (bsname == nil)
		bsname = emptystring;
	
	if ((id == -1) && (!isemptystring (bsname))) { /*no id provided; try to use name*/
		
		id = Unique1ID (type);
		
		hresource = Get1NamedResource (type, bsname);
		}
	else
		hresource = Get1Resource (type, id);
	
	if (hresource != nil) { /*resource already exists*/
		
		if (!sethandlecontents (pdata, ctwrite, hresource)) /*couldn't increase size of handle*/
			return (false);
		
		//SetResInfo (hresource, id, bsname); /*set the name of the resource*/
		
		ChangedResource (hresource);
		}
	else {
		
		if (!newfilledhandle (pdata, ctwrite, &hresource))
			return (false);
		
		AddResource (hresource, type, id, bsname);
		}
	
	return (true);
	} /*writeresource*/
#endif


#ifdef MACVERSION
static boolean copyresourcehandle (Handle hresource, short destnum) {
	
	/*
	copy the given resource handle to the destination file.  our caller is 
	managing the resource chain; we don't need to preserve the current resource 
	file.
	
	return true if the resource is not nil and is sucessfully copied.  generate an 
	oserror only if a memory or IO error occurs.
	*/
	
	ResType type;
	short id;
	bigstring bsname;
	Handle htemp;
	
	if (hresource == nil) { /*didn't get the resource*/
		
		if (ResError () != resNotFound) /*only generate an alert on memory/IO error*/
			oserror (ResError ());
		
		return (false);
		}
	
	GetResInfo (hresource, &id, &type, bsname);	/*get all info*/
	
	UseResFile (destnum);
	
	htemp = Get1Resource (type, id); /*try to see if res of same number in dest file*/
	
	if (htemp != nil) { /*an entry already exists*/
		
		if (!copyhandlecontents (hresource, htemp)) /*memory failure*/
			return (false);
		
		SetResInfo (htemp, id, bsname); /*set name*/
		
		ChangedResource (htemp);
		}
	else {
		
		DetachResource (hresource); /*required for copying!*/
		
		AddResource (hresource, type, id, bsname);
		
		WriteResource (hresource);
		}
	
	return (!oserror (ResError ()));
	} /*copyresourcehandle*/
#endif


#ifdef MACVERSION
boolean copyresource (short sourcenum, short destnum, ResType type, short id) {
	
	/*
	copy the resource with the indicated type and id from the source resource 
	file to the destination.
	
	return true if the resource is found and is sucessfully copied.  generate an 
	oserror if a memory or IO error occurs.
	*/
	
	short saveresfile;
	boolean fl;
	
	saveresfile = CurResFile ();
	
	UseResFile (sourcenum);
	
	fl = copyresourcehandle (Get1Resource (type, id), destnum);
	
	UseResFile (saveresfile);
	
	return (fl);
	} /*copyresource*/
#endif


boolean copyallresources (short sourcenum, short destnum) {
	
	/*
	copy all of the resources in the source resource file to the destination.
	
	return true if the resource is found and is sucessfully copied.  generate an 
	oserror if a memory or IO error occurs.
	*/
	
#ifdef MACVERSION
	short saveresfile;
	short cttypes, ixtype;
	short ctresources, ixresource;
	ResType type;
	boolean fl = true;
	
	saveresfile = CurResFile ();
	
	UseResFile (sourcenum);
	
	cttypes = Count1Types ();
	
	for (ixtype = 1; fl && (ixtype <= cttypes); ++ixtype) {
		
		Get1IndType (&type, ixtype);
		
		ctresources = Count1Resources (type);
		
		for (ixresource = 1; fl && (ixresource <= ctresources); ++ixresource) {
			
			fl = copyresourcehandle (Get1IndResource (type, ixresource), destnum);
			
			UseResFile (sourcenum); /*copyresourcehandle changes it*/
			}
		}
	
	UseResFile (saveresfile);
	
	return (fl);
#endif
#ifdef WIN95VERSION
	return (false);
#endif
	} /*copyallresources*/



static Handle get1resource (ResType type, short id, bigstring bsname) {
#ifdef MACVERSION	
	if ((bsname == nil) || isemptystring (bsname))
		return (Get1Resource (type, id));
	else
		return (Get1NamedResource (type, bsname));
#endif

#ifdef WIN95VERSION
	HANDLE reshandle, hdata;
	Handle hreturn;
	long sz;
	char restype[5];
	char restypex[5];
	memmove (restypex, &type, 4);
	restype[0] = restypex[3];
	restype[1] = restypex[2];
	restype[2] = restypex[1];
	restype[3] = restypex[0];

	restype[4] = 0;

	hdata = NULL;
	hreturn = NULL;

	if ((bsname == nil) || isemptystring (bsname))
		reshandle = FindResource (hInst, MAKEINTRESOURCE(id), restype);
	else
		reshandle = FindResource (hInst, stringbaseaddress(bsname), restype);

	if (reshandle != NULL) 
		{
		sz = SizeofResource (hInst, reshandle);
		hdata = LoadResource (hInst, reshandle);
		
		if (hdata != NULL) 
			{
			hreturn = NewHandle (sz);

			if (hreturn != NULL)
				{
//				memmove (GlobalLock (hreturn), LockResource(hdata), sz);
//				GlobalUnlock (hreturn);
				//5.1.2 new memory handling RAB
				memmove (*hreturn, LockResource(hdata), sz);
				}
			}
		}
	return (hreturn);

#endif
	} /*get1resource*/


Handle filegetresource (short rnum, ResType type, short id, bigstring bsname) {
	
	/*
	4/9/91 dmb: now take id and name as parameters; use id if name is nil
	*/
	
	register Handle hresource;
#ifdef MACVERSION
	short saveresfile;
#endif
	
	if (rnum == -1)
		return (nil);
#ifdef MACVERSION	
	saveresfile = CurResFile ();
	
	UseResFile (rnum);
#endif	

	hresource = get1resource (type, id, bsname);

#ifdef MACVERSION	
	UseResFile (saveresfile);
#endif	
	return (hresource);
	} /*filegetresource*/


boolean filereadresource (short rnum, ResType type, short id, bigstring bsname, long ctread, void *pdata) {
	
	register Handle hresource;
	
	hresource = filegetresource (rnum, type, id, bsname);
	
	if (hresource) {
		
		long ctactualsize = gethandlesize (hresource);

		moveleft (*hresource, pdata, min (ctread, ctactualsize));
		
		return (true);
		}
	
	return (false);
	} /*filereadresource*/

#ifdef MACVERSION
boolean filewriteresource (short rnum, ResType type, short id, bigstring bsname, long ctwrite, void *pdata) {
	
	register boolean fl;
	short saveresfile;
	OSErr err;
	
	if (rnum == -1)
		return (false);
	
	saveresfile = CurResFile ();
	
	UseResFile (rnum);
	
	fl = writeresource (type, id, bsname, ctwrite, pdata);
	
	err = ResError ();
	
	UseResFile (saveresfile);
	
	return (fl && !oserror (err));
	} /*filewriteresource*/
#endif

#ifdef MACVERSION
boolean saveresource (const ptrfilespec fs, short rnum, ResType type, short id, bigstring bsname, long sizedata, void *pdata, short forktype) {

	/*
	open the file indicated by fname and vnum, and assign pdata to the resource
	indicated by type and id.  return true if it worked.
	
	this will only work for resource types that are not used in your application
	and are not used in the system.  be careful in choosing your types!
	
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/
	
	register boolean fl;
	short oldrnum;
	OSErr err;
	
	if (rnum != -1) /*have open resource fork to write to*/
		return (filewriteresource (rnum, type, id, bsname, sizedata, pdata));
	
	oldrnum = CurResFile ();
	
	if (!openresourcefile (fs, &rnum, forktype))
		return (false);
	
	fl = writeresource (type, id, bsname, sizedata, pdata);
	
	err = ResError ();
	
	if ((rnum == oldrnum) || (rnum == filegetapplicationrnum ())) { /*don't close active resource fork!*/
		
		UpdateResFile (rnum);
		
		UseResFile (oldrnum);
		}
	else {
		
		if (!closeresourcefile (rnum))
			fl = false;
		}
	
	return (fl && !oserror (err));
	} /*saveresource*/
#endif

#ifdef MACVERSION
boolean saveresourcehandle (const ptrfilespec fs, ResType type, short id, bigstring bsname, Handle h, short forktype) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	boolean fl;
	
	lockhandle (h);
	
	fl = saveresource (fs, -1, type, id, bsname, gethandlesize (h), *h, forktype);
	
	unlockhandle (h);
	
	return (fl);
	} /*saveresourcehandle*/
#endif

/*
static for push/popresourcefile
*/

private short newrnum, oldrnum;

#ifdef MACVERSION

	static boolean pushresourcefile (const ptrfilespec fs, char permission, short forktype) {
		
		/*
		2006-01-30 creedon: added check for non-carbon OS and trying to access the data fork of a file, error message
		
		2005-09-02 creedon: added support for fork parameter, allows access to resources in data forks

		2.1b8 dmb: call SetResLoad (false) before opening a resource fork to 
		prevent all preload resources from being loaded into our heap
		*/
		
		register OSErr errcode;
		FSRef fsref;
		HFSUniStr255 fork;
		
		if (oserror (macgetfsref (fs, &fsref)))
			return (false);
		
		switch (forktype) {
			case resourcefork:
				FSGetResourceForkName (&fork);
				break;
				
			case datafork:
				FSGetDataForkName (&fork);
				break;
			}

		oldrnum = CurResFile ();
	
		SetResLoad (false);
	
		/*
		2005-08-31 creedon - in my reading about the FSOpenResourceFile function someone mentioned
		that it might not deal well with corrupted resources and that dropping back to FSpOpenResFile
		seemed to do the trick.  i've not done that here but could be tried if the problem is real
		and manifests. 
		*/
			
		errcode = FSOpenResourceFile (&fsref, fork.length, fork.unicode, permission, &newrnum);

		SetResLoad (true);

		if (errcode != -1) /*opened OK*/
			return (true);

		errcode = ResError ();

		if (errcode != eofErr) { /*don't want an alert if there isn't a resource fork*/
			
			setfserrorparam ( fs ); /*in case error message takes a filename parameter*/
			
			oserror (errcode);
			}
		
		return (false);
		} /* pushresourcefile */

	static boolean pushresourcefilereadonly (const ptrfilespec fs, short forktype) {
		
		/*
		2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
		*/ 
		
		return (pushresourcefile (fs, fsRdPerm, forktype));
		} /*pushresourcefilereadonly*/


	static boolean pushresourcefilereadwrite (const ptrfilespec fs, short forktype) {
		
		/*
		2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
		*/ 
		
		return (pushresourcefile (fs, fsRdWrPerm, forktype));
		} /*pushresourcefilereadwrite*/


	static boolean popresourcefile (void) {
		
		if ((newrnum == oldrnum) || (newrnum == filegetapplicationrnum ())) { /*don't close active resource fork!*/
			
			UpdateResFile (newrnum);
			
			UseResFile (oldrnum);
			}
		else
			closeresourcefile (newrnum);
		
		return (true);
		} /*popresourcefile*/
#endif

boolean loadresource (const ptrfilespec fs, short rnum, ResType type, short id, bigstring bsname, long sizedata, void *pdata, short forktype) {
	
	/*
	the inverse of saveresource.  we load in the indicated type and id from
	the indicated file on the indicated volume into the indicated data area.
	
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/
	
	register boolean fl = false;
	register Handle h;
	
	if (rnum != -1) /*have an open resource file to read from*/
		return (filereadresource (rnum, type, id, bsname, sizedata, pdata));
	
	#ifdef MACVERSION	
		if (!pushresourcefilereadonly (fs, forktype))
			return (false);
	#endif
	
	h = get1resource (type, id, bsname);
	
	if (h != nil) { /*file had the indicated resource*/
		
		long ctactualsize = gethandlesize (h);
		
		moveleft (*h, pdata, min (sizedata, ctactualsize)); 
		
		fl = true;

		#ifdef WIN95VERSION
			releaseresourcehandle (h);
		#endif
		}
	
	#ifdef MACVERSION
		popresourcefile ();
	#endif
		
	return (fl);
	} /*loadresource*/


#ifdef MACVERSION
boolean loadresourcehandle (const ptrfilespec fs, ResType type, short id, bigstring bsname, Handle *hresource, short forktype) {
	
	/*
	load a resource into a handle.
	
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	
	9/13/91 dmb: now accept nil hresource to just check for resource's existence
	
	3.0b15 dmb: make sure handle isn't purgable after detaching
	*/
	
	register boolean fl = false;
	register Handle h;
	
	if (!pushresourcefilereadonly (fs, forktype))
		return (false);
	
	if (hresource == nil) /*just checking existence*/
		SetResLoad (false);
	
	h = get1resource (type, id, bsname);
	
	if (hresource == nil) /*restore resload immediately!*/
		SetResLoad (true);
	
	if (h != nil) { /*got the indicated resource*/
		
		if (hresource != nil) { /*caller actually wants data*/
			
			DetachResource (h);
			
			*hresource = h;
			}
		
		fl = true;
		}
	
	popresourcefile ();
	
	return (fl);
	} /*loadresourcehandle*/
#endif


#ifdef MACVERSION
boolean deleteresource (const ptrfilespec fs, ResType type, short id, bigstring bsname, short forkttype) {
	
	/*
	delete the resource specified by type, id or bsname
	
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/
	
	register boolean fl = false;
	register Handle h;
	
	if (!pushresourcefilereadwrite (fs, forkttype))
		return (false);
	
	SetResLoad (false);
	
	h = get1resource (type, id, bsname);
	
	SetResLoad (true);
	
	if (h != nil) { /*got the indicated resource*/
		
		RemoveResource (h);
		
		fl = !oserror (ResError ());
		
		disposehandle (h);
		}
	
	popresourcefile ();
	
	return (fl);
	} /*deleteresource*/
#endif


#ifdef MACVERSION
boolean getnumresourcetypes (const ptrfilespec fs, short *cttypes, short forktype) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	if (!pushresourcefilereadonly (fs, forktype))
		return (false);
	
	*cttypes = Count1Types ();
	
	popresourcefile ();
	
	return (true);
	} /*getnumresourcetypes*/
#endif

#ifdef MACVERSION
boolean getnthresourcetype (const ptrfilespec fs, short n, ResType *type, short forktype) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	if (!pushresourcefilereadonly (fs, forktype))
		return (false);
	
	Get1IndType (type, n);
	
	popresourcefile ();
	
	return (*type != 0L);
	} /*getnthresourcetype*/
#endif

#ifdef MACVERSION
boolean getnumresources (const ptrfilespec fs, ResType type, short *ctresources, short forktype) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	if (!pushresourcefilereadonly (fs, forktype))
		return (false);
	
	*ctresources = Count1Resources (type);
	
	popresourcefile ();
	
	return (true);
	} /*getnumresources*/
#endif

#ifdef MACVERSION
boolean getnthresourcehandle (const ptrfilespec fs, ResType type, short n, short *id, bigstring bsname, Handle *hresource, short forktype) {
	
	/*
	load the nth resource of the given type into a handle.  also return the 
	id and name of the resource
	
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	
	6/2/92 dmb: accept nil hresource for info retrieval
	
	3.0b15 dmb: make sure handle isn't purgable after detaching
	*/
	
	register boolean fl = false;
	register Handle h;
	
	if (!pushresourcefilereadonly (fs, forktype))
		return (false);
	
	if (hresource == nil) /*just checking existence*/
		SetResLoad (false);
	
	h = Get1IndResource (type, n);
	
	if (hresource == nil) /*restore resload immediately!*/
		SetResLoad (true);
	
	if (h != nil) { /*got the indicated resource*/
		
		GetResInfo (h, id, &type, bsname);
		
		if (hresource != nil) { /*caller actually wants data*/
			
			DetachResource (h);
			
			*hresource = h;
			}
		
		fl = true;
		}
	
	popresourcefile ();
	
	return (fl);
	} /*getnthresourcehandle*/
#endif


#ifdef MACVERSION
static boolean getemptyresourcehandle (ResType type, short id, bigstring bsname, Handle *hresource) {
	
	/*
	get a handle to the resource in current resoure file specified by id or 
	bsname, but don't load the actual data into memory.
	*/
	
	register Handle h;
	
	SetResLoad (false);
	
	h = get1resource (type, id, bsname);
	
	SetResLoad (true);
	
	*hresource = h;
	
	if (h == nil)  { /*didn't find the indicated resource*/
		
		oserror (resNotFound);
		
		return (false);
		}
	
	return (true);
	} /*getemptyresourcehandle*/
#endif


#ifdef MACVERSION
boolean getresourceattributes (const ptrfilespec fs, ResType type, short id, bigstring bsname, short *resattrs, short forktype) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	
	2.1b4 dmb: new routine
	*/
	
	Handle h;
	register boolean fl = false;
	
	if (!pushresourcefilereadonly (fs, forktype))
		return (false);
	
	if (getemptyresourcehandle (type, id, bsname, &h)) { /*got the indicated resource*/
		
		*resattrs = GetResAttrs (h);
		
		fl = !oserror (ResError ());
		}
	
	popresourcefile ();
	
	return (fl);
	} /*getresourceattributes*/
#endif


#ifdef MACVERSION
boolean setresourceattributes (const ptrfilespec fs, ResType type, short id, bigstring bsname, short resattrs, short forktype) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile

	2.1b4 dmb: new routine
	*/
	
	Handle h;
	register boolean fl = false;
	short validattrs = (resSysHeap | resPurgeable | resLocked | resProtected | resPreload);
//	#define validattrs (resSysHeap | resPurgeable | resLocked | resProtected | resPreload)
 	
	if (!pushresourcefilereadwrite (fs, forktype))
		return (false);
	
	if (getemptyresourcehandle (type, id, bsname, &h)) { /*got the indicated resource*/
		
		SetResAttrs (h, resattrs & validattrs);
		
		fl = !oserror (ResError ());
		}
	
	popresourcefile ();
	
	return (fl);
	} /*setresourceattributes*/
#endif


Handle getresourcehandle (ResType type, short id)	{
#ifdef MACVERSION
	return (GetResource (type, id));
#endif

#ifdef WIN95VERSION
	return (filegetresource (0, type, id, NULL));
#endif
	}


void releaseresourcehandle (Handle h) {
#ifdef MACVERSION
	ReleaseResource (h);
#endif

#ifdef WIN95VERSION
	DisposeHandle (h);
#endif
	} /*releaseresourcehandle*/

