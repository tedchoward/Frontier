
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

#include <Aliases.h>
#include <AppleEvents.h>
#include <AEPackObject.h>
#include <AEObjects.h>
#include <AERegistry.h>
#include <Gestalt.h>
#include <standard.h>
#include "memory.h"
#include "strings.h"
#include "ops.h"
#include "error.h"
#include "file.h"
#include "lang.h"
#include "langinternal.h"
#include "langipc.h"
#include "langsystem7.h"
#include "tableinternal.h" /*for hdltablevariable; so we can avoid loading unloaded tables*/
#include "tablestructure.h"


#if 0 //def THINK_C

pascal OSErr FollowFinderAlias (const FSSpec *fromFile, AliasHandle alias, Boolean logon, FSSpec *target, Boolean *wasChanged) = {
                                   
	/*
    resolves an alias taken from a Finder alias file,
	updating the alias record (but not the alias resource in the file) if
	necessary.
	*/
	
	0x700F, 0xA823};  /*MOVEQ #$0F,D0; _AliasDispatch;*/
	/*FollowFinderAlias*/

#endif
	
	

static boolean comparelists (AEDesc *, AEDesc *, tytreetype); /*forward*/


static boolean langgestaltcheck (OSType selector, short stringnum) {
	
	long result;
	
	if (!gestalt (selector, &result) || (result == 0)) {
		
		langerror (stringnum);
		
		return (false);
		}
	
	return (true);
	} /*langgestaltcheck*/


boolean langcanusealiases (void) {
	
	#ifdef flsystem6
	
	return (langgestaltcheck (gestaltAliasMgrAttr, cantusealiaseserror));
	
	#else
	
	return (true);
	
	#endif
	} /*langcanusealiases*/


#ifdef flsystem6

boolean langcanuseappleevents (void) {
	
	return (langgestaltcheck (gestaltAppleEventsAttr, cantuseobjspecserror));
	} /*langcanuseappleevents*/

#endif


static boolean equaldescriptors (AEDesc *desc1, AEDesc *desc2) {
	
	register AEDesc *d1 = desc1;
	register AEDesc *d2 = desc2;
	register DescType t1 = (*d1).descriptorType;
	
	if (t1 != (*d2).descriptorType)
		return (false);
	
	if ((t1 == typeAERecord) || (t1 == typeAEList)) {
		#ifdef oplanglists
			tyvaluerecord v1, v2, vequal;
			
			if (!langipcconvertaelist (d1, &v1) || !langipcconvertaelist (d2, &v2))
				return (false);
			
			return (EQvalue (v1, v2, &vequal) && vequal.data.flvalue);
		#else
			return (comparelists (d1, d2, EQop));
		#endif
		}
	else
		return (equalhandles ((*d1).dataHandle, (*d2).dataHandle));
	} /*equaldescriptors*/


//static tyvaluerecord valfind;


static boolean findvaluevisit (bigstring bs, hdlhashnode hnode, tyvaluerecord val, ptrvoid valfind) {
	
	return (val.data.longvalue == (long) valfind);	//.data.longvalue
	} /*findvaluevisit*/


static boolean langfindvalue (tyvaluerecord val, hdlhashtable *htable, bigstring bsname) {
	
	/*
	search through the stack of symbol tables until you find a value that 
	matches val.  return the name of the matching value.
	*/
	
	register hdlhashtable h = currenthashtable;
	register long refcon;
	register short n;
	long valfind;
	hdlhashnode hnode;
	
	*htable = nil;
	
	setemptystring (bsname);
	
	if (h == nil)
		return (false);
	
	refcon = (**h).lexicalrefcon;
	
	valfind = val.data.longvalue;
	
	while (true) { /*chain through each linked hash table*/
		
		if (h == nil) /*symbol not defined*/
			return (false);
		
		if ((!(**h).fllocaltable) || (refcon == 0) || ((**h).lexicalrefcon == refcon)) {
			
			if (hashinversesearch (h, &findvaluevisit, (ptrvoid) valfind, bsname)) { /*symbol is defined in htable*/
				
				*htable = h;
				
				return (true);
				}
			
			for (n = (**h).ctwithvalues; n > 0; --n) { /*scan with values*/
				
				tyvaluerecord valwith;
				hdlhashtable hwith;
				bigstring bswith;
				
				langgetwithvaluename (n, bswith);
				
				if (!hashtablelookup (h, bswith, &valwith, &hnode)) /*missing with value; keep going*/
					continue;
				
				if (!getaddressvalue (valwith, &hwith, bswith)) /*error*/
					return (false);
				
				if (hashinversesearch (hwith, &findvaluevisit, (ptrvoid) valfind, bsname)) { /*found symbol*/
					
					*htable = hwith;
					
					return (true);
					}
				}
			}
		
		h = (**h).prevhashtable;
		} /*while*/
	} /*langfindvalue*/


//static tyvaluerecord vallookfor;


static boolean getostypevalnamevisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, tyvaluerecord *vallookfor) {
	
	/*
	3.0.2b1 dmb: we now look in all loaded app tables for a match when 
	converting an terminology value (a string4 value) its name
	*/
	
	register hdltablevariable hv;
	register hdlhashtable ht;
	register boolean fltempload;
	hdlhashtable htable;
	hdltablevariable hvariable;
	short errorcode;
	boolean fl;
	
	if (!gettablevariable (val, &hvariable, &errorcode))
		return (false);
	
	hv = hvariable;
	
	fltempload = !(**hv).flinmemory;
	
	if (fltempload) /*we don't want to search every app table, just those in use*/
		return (false);
	
	/*
	if (!tableverbinmemory ((hdlexternalvariable) hv))
		return (false);
	*/
	
	ht = (hdlhashtable) (**hv).variabledata; 
	
	if (ht == currenthashtable) /*it's already been searched*/
		return (false);
	
	pushhashtable (ht);
	
	fl = langfindvalue (*vallookfor, &htable, bsname);
	
	pophashtable ();
	
	/*
	if (fltempload)
		tableverbunload ((hdlexternalvariable) hv);
	*/
	
	return (fl);
	} /*getostypevalnamevisit*/


static boolean getostypedisplaystring (OSType key, bigstring bsdisplay) {
	
	/*
	key is an enum or string4 value that may correspond to a bit of 
	object model user terminology. our job is to scan the current context 
	for a match, and return the symbolic name if found. otherwise, return 
	a string4 in single quotes
	
	2.1b6 dmb: if result is a quoted literal, add escape sequences
	*/
	
	tyvaluerecord val;
	hdlhashtable htable;
	byte bskey [16];
	boolean fl;
	
	setostypevalue (key, &val);
	
	if (langfindvalue (val, &htable, bsdisplay))
		return (true);
	
	if (objectmodeltable != nil) {
		
		pushhashtable (objectmodeltable);
		
		fl = langfindvalue (val, &htable, bsdisplay);
		
		pophashtable ();
		
		if (fl)
			return (true);
		}
	
	if (iacgluetable != nil) {
		
		//vallookfor = val;
		
		if (hashinversesearch (iacgluetable, &getostypevalnamevisit, &val, bsdisplay))
			return (true);
		}
	
	ostypetostring (key, bskey);
	
	langdeparsestring (bskey, chsinglequote); /*add needed escape sequences*/
	
	parsedialogstring ("\p'^0'", bskey, nil, nil, nil, bsdisplay);
	
	return (false);
	} /*getostypedisplaystring*/


static boolean getlimitedvaluestring (tyvaluerecord *val, short limit, char chquote, bigstring bsvalue) {
	
	/*
	2.1b2 dmb: try using AE coercion for unknown (binary) types
	
	2.1b3 dmb: if binary ends up needing to be coerced to string, add quotes 
	and escape sequences
	*/
	
	register tyvaluerecord *v = val;
	AEDesc desc, coerceddesc;
	OSErr err;
	
	if ((*v).valuetype == binaryvaluetype) {
		
		binarytodesc ((*v).data.binaryvalue, &desc); /*binary handle remains in temp stack*/
		
		err = AECoerceDesc (&desc, typeChar, &coerceddesc);
		
		if (err == noErr)
			chquote = chdoublequote;
		else
			err = AECoerceDesc (&desc, typeObjectSpecifier, &coerceddesc);
		
		if (err != noErr) /*no AE coercion to objspec*/
			err = AECoerceDesc (&desc, typeAERecord, &coerceddesc);
		
		if (err != noErr) { /*no AE coercion to record either*/
			
			chquote = chdoublequote;
			
			/*
			hashgetvaluestring (*v, bsvalue); /*go with hex string%/
			
			goto limit;
			*/
			}
		else {
			
			disposevaluerecord (*v, true);
			
			if (!setdescriptorvalue (coerceddesc, v))
				return (false);
			}
		}
	
	if (!coercetostring (v))
		return (false);
	
	pullstringvalue (v, bsvalue);
	
	limit:
	
	if (chquote != chnul)
		langdeparsestring (bsvalue, chquote); /*add needed escape sequences*/
	
	if (stringlength (bsvalue) > limit) {
		
		setstringlength (bsvalue, limit - 1);
		
		pushchar ('É', bsvalue);
		}
	
	if (chquote != chnul) {
		
		insertchar (chquote, bsvalue);
		
		pushchar (chquote, bsvalue);
		}
	
	return (true);
	} /*getlimitedvaluestring*/


boolean getobjectmodeldisplaystring (tyvaluerecord *vitem, bigstring bsdisplay) {
	
	/*
	get the value's string representation for inclusion in a list 
	or object specifier, adding single or double quotes as necessary, and 
	mapping string4 values to identifiers in the current context.
	
	limit the length of the returned string to 253 characters. if we're returning 
	a quoted string and need to add an ellipse, add it inside of the quotes.
	
	2.1b1 dmb: don't quote objspec values. exported for osacomponent
	*/
	
	register tyvaluerecord *v = vitem;
	
	switch ((*v).valuetype) {
		
		case novaluetype:
			langgetmiscstring (justnilstring, bsdisplay);
			
			break;
		
		case ostypevaluetype:
		case enumvaluetype:
			getostypedisplaystring ((*v).data.ostypevalue, bsdisplay);
			
			break;
		
		case charvaluetype: /*these need single quotes*/
			if (!getlimitedvaluestring (v, 251, chsinglequote, bsdisplay))
				return (false);
			
			break;
		
		case stringvaluetype: /*these need to be quoted*/
		case filespecvaluetype:
		case aliasvaluetype:
		case datevaluetype:
		case addressvaluetype:
			if (!getlimitedvaluestring (v, 251, chdoublequote, bsdisplay))
				return (false);
			
			break;
		
		case pointvaluetype: /*these should look like lists*/
		case rectvaluetype:
		case rgbvaluetype:
			if (!getlimitedvaluestring (v, 251, chnul, bsdisplay))
				return (false);
			
			insertchar ('{', bsdisplay);
			
			pushchar ('}', bsdisplay);
			
			break;
		
		case objspecvaluetype:
			if (!getlimitedvaluestring (v, 253, chnul, bsdisplay))
				return (false);
			
			if (isemptystring (bsdisplay))
				copystring ("\p\"\"", bsdisplay);
			
			break;
			
		default:
			if (!getlimitedvaluestring (v, 253, chnul, bsdisplay))
				return (false);
			
			break;
		}
	
	return (true);
	} /*getobjectmodeldisplaystring*/


static boolean stringtoalias (tyvaluerecord *val) {
	
	/*
	10/7/91 dmb: make sure we're actually passing a full path to the NewAlias routine
	
	7/2/92 dmb: don't call getfullfilepath; makes it impossible to create aliases of 
	not-yet-existing files, or offline volumes
	
	7/23/92 dmb: OK, try to getfullfilepath, but with errors disabled
	
	2.1b2 dmb: try converting to a filespec first to ensure that partial path or 
	drive number if processed properly. also, in the filespec case, the alias isn't 
	minimal
	*/
	
	register Handle htext;
	bigstring bspath;
	tyfilespec fs;
	AliasHandle halias;
	boolean flfolder;
	OSErr errcode;
	
	if (!langcanusealiases ())
		return (false);
	
	htext = (*val).data.stringvalue;
	
	texthandletostring (htext, bspath);
	
	if (pathtofilespec (bspath, &fs) && fileexists (&fs, &flfolder))
		errcode = NewAlias (nil, &fs, &halias);
	else
		errcode = NewAliasMinimalFromFullPath (stringlength (bspath), bspath + 1, nil, nil, &halias);
	
	if (oserror (errcode))
		return (false);
	
	if (!setheapvalue ((Handle) halias, aliasvaluetype, val))
		return (false);
	
	releaseheaptmp ((Handle) htext);
	
	return (true);
	} /*stringtoalias*/


boolean filespectoalias (const tyfilespec *fs, boolean flminimal, AliasHandle *halias) {
	
	bigstring bs;
	OSErr err;
	
	if (flminimal)
		err = NewAliasMinimal (fs, halias);
	else
		err = NewAlias (nil, fs, halias);
	
	if (err == fnfErr) { /*alias manager isn't friendly enough to do anything for us here*/
		
		if (filespectopath (fs, bs))
			err = NewAliasMinimalFromFullPath (stringlength (bs), bs + 1, nil, nil, halias);
		}
	
	if (err == noErr)
		return (true);
	
	if (langerrorenabled ()) {
		
		setoserrorparam ((ptrstring) (*fs).name);
		
		oserror (err);
		}
	
	return (false);
	} /*filespectoalias*/


static boolean filespecvaltoalias (tyvaluerecord *val) {
	
	register FSSpecHandle hfs;
	FSSpec fs;
	AliasHandle halias;
	
	if (!langcanusealiases ())
		return (false);
	
	hfs = (FSSpecHandle) (*val).data.filespecvalue;
	
	fs = **hfs;
	
	if (!filespectoalias (&fs, false, &halias))
		return (false);
	
	if (!setheapvalue ((Handle) halias, aliasvaluetype, val))
		return (false);
	
	releaseheaptmp ((Handle) hfs);
	
	return (true);
	} /*filespecvaltoalias*/


boolean aliastostring (Handle halias, bigstring bs) {
	
	/*
	10/4/91 dmb: if alias can't be resolved, just say what volume it's on.
	
	4/12/93 dmb: accept fnfErr result from ResolveAlias
	
	2.1b9 dmb: use FollowFinderAlias to avoid mounting volumes during 
	alias resolution
	
	4.0b6 4/26/96 dmb: restored FollowFinderAlias code; must use if we get fnfErr.
	*/
	
	register AliasHandle h = (AliasHandle) halias;
	short flchanged;
	FSSpec fs;
	bigstring bsinfo;
	AliasInfoType ix = asiAliasName;
	OSErr err;
	
	if (!langcanusealiases ())
		return (false);
	
	err = FollowFinderAlias (nil, h, false, &fs, (Boolean *) &flchanged);
	
	if ((err == noErr) /*|| (err == fnfErr)*/ ) {
		
		if (flchanged)
			UpdateAlias (nil, &fs, h, (Boolean *) &flchanged);
		
		return (filespectopath (&fs, bs));
		}
	
	langgettypestring (aliasvaluetype, bs);
	
	/*
	if (GetAliasInfo (h, asiVolumeName, bsinfo) == noErr) { //add the volume name
		
		bigstring bsaliasondisk;
		
		langgetstringlist (unresolvedaliasstring, bsaliasondisk);
		
		parsedialogstring (bsaliasondisk, bs, bsinfo, nil, nil, bsaliasondisk);
		
		copystring (bsaliasondisk, bs);
		}
	*/
	
	setemptystring (bs);
	
	// get each path element out of the alias
	while (GetAliasInfo (h, ix, bsinfo) == noErr) {
		
		if (isemptystring (bsinfo)) // reached top of path hierarchy
			break;
		
		if (ix > asiAliasName)
			pushchar (':', bsinfo);
		
		if (!insertstring (bsinfo, bs))
			break;
		
		++ix;
		}
	
	// add the volume name
	GetAliasInfo (h, asiVolumeName, bsinfo);
	
	pushchar (':', bsinfo);
	
	insertstring (bsinfo, bs);
	
	return (true);
	} /*aliastostring*/


boolean aliastofilespec (AliasHandle halias, FSSpec *fs) {
	
	/*
	2.1a6 dmb: ignore fnfErr
	
	2.1b2 dmb: on error, try to get as much info from the alias as possible, 
	& just return false to caller
	
	2.1b9 dmb: use FollowFinderAlias to avoid mounting volumes during 
	alias resolution
	*/
	
	Boolean flchanged;
	bigstring bs;
	OSErr err;
	
	if (!langcanusealiases ())
		return (false);
	
	err = FollowFinderAlias (nil, halias, false, fs, &flchanged);
	
	if ((err == noErr) || (err == fnfErr))
		return (true);
	
	(*fs).parID = 0;
	
	(*fs).vRefNum = 0;
	
	if (GetAliasInfo (halias, asiVolumeName, bs) == noErr) /*try to get vol info*/
		fileparsevolname (bs, &(*fs).vRefNum, nil);
	
	if (GetAliasInfo (halias, asiAliasName, (*fs).name) != noErr) /*try to set file name*/
		langgetmiscstring (unknownstring, (*fs).name);
	
	if (langerrorenabled ()) {
		
		setoserrorparam ((*fs).name);
		
		oserror (err);
		}
	
	return (false);
	} /*aliastofilespec*/


boolean coercetoalias (tyvaluerecord *v) {
	
	/*
	10/4/91 dmb: when v is novaluetype, fail silently so the new verb 
	will just return false.  a specific error message might be better, but 
	I don't expect this to come up much, if at all.
	*/
	
	switch ((*v).valuetype) {
		
		case aliasvaluetype:
			return (true);
		
		case novaluetype: /*easy way is via file spec to make alias of default folder*/
			if (flinhibitnilcoercion)
				return (false);
			
			if (!coercetofilespec (v))
				return (false);
			
			return (filespecvaltoalias (v));
		
		case stringvaluetype:
			return (stringtoalias (v));
		
		case binaryvaluetype:
			return (coercebinaryval (v, aliasvaluetype, 0L, aliasvaluetype));
		
		case filespecvaluetype:
			return (filespecvaltoalias (v));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, aliasvaluetype));
		
		default:
			langcoerceerror (v, aliasvaluetype);
			
			return (false);
		} /*switch*/
	} /*coercetoalias*/


boolean filespecaddvalue (tyvaluerecord *v1, tyvaluerecord *v2, tyvaluerecord *vreturned) {
	
	/*
	add v2 to the filespec v1 by using it as a partial path. if anything bug a valid 
	fspec results, return a string value that is simple concatenation
	
	2.1b3 dmb: if resulting path is to a non-existent folder, don't return a filespec 
	
	2.1b6 dmb: if resulting specifier exists, but doesn't agree with bsadd as far as 
	whether or not it's a folder, return a string.
	*/
	
	tyfilespec fs;
	bigstring bsadd;
	boolean fl, flfolder;
	boolean flfolderpath;
	OSErr err;
	
	if (!coercetostring (v2))
		return (false);
	
	fs = **(*v1).data.filespecvalue;
	
	pullstringvalue (v2, bsadd);
	
	if (fileexists (&fs, &flfolder)) {
		
	//	fileisfolder (&fs, &flfolder);
		
		if (flfolder)
			pushchar (':', fs.name);
		}
	
	insertstring (fs.name, bsadd);
	
	if (stringfindchar (':', bsadd)) /*will be interpreted as full path, so make it partial*/
		insertchar (':', bsadd);
	
	err = FSMakeFSSpec (fs.vRefNum, fs.parID, bsadd, &fs);
	
	flfolderpath = lastchar (bsadd) == ':';
	
	switch (err) {
		
		case noErr: /*valid spec, file exists*/
			
			fileisfolder (&fs, &flfolder);
			
			fl = flfolder == flfolderpath; /*make sure endings match*/
			
			break;
		
		case fnfErr: /*valid spec, file doesn't exist*/
			
			fl = !flfolderpath;
			
			break;
		
		default:
			fl = false;
			
			break;
		}
	
	if (!fl) { /*couldn't extend filespec*/
		
		coercetostring (v1);
		
		return (addvalue (*v1, *v2, vreturned));
		}
	
	return (setfilespecvalue (&fs, vreturned));
	} /*filespecaddvalue*/


boolean filespecsubtractvalue (tyvaluerecord *v1, tyvaluerecord *v2, tyvaluerecord *vreturned) {
	
	/*
	subtract v2 from the filespec v1's file name, iff v2 is a simple string (no 
	colons). otherwise, return a string value that is simple string subtraction
	*/
	
	tyfilespec fs;
	bigstring bssub;
	boolean fl;
	Str63 bsname;
	OSErr err;
	
	if (!coercetostring (v2))
		return (false);
	
	fs = **(*v1).data.filespecvalue;
	
	pullstringvalue (v2, bssub);
	
	fl = !stringfindchar (':', bssub);
	
	if (fl) {
		
		subtractstrings (fs.name, bssub, bsname);
		
		err = FSMakeFSSpec (fs.vRefNum, fs.parID, bsname, &fs);
		
		fl = (err == noErr) || (err == fnfErr);
		}
	
	if (!fl) { /*couldn't extend filespec*/
	
		coercetostring (v1);
		
		return (subtractvalue (*v1, *v2, vreturned));
		}
	
	return (setfilespecvalue (&fs, vreturned));
	} /*filespecsubtractvalue*/


static pascal OSErr langsystem7accessobject (
						DescType	classWanted,
						AEDesc		*container,
						DescType	containerClass, 
						DescType	keyform,
					    AEDesc		*keydesc,
						AEDesc		*resultToken,
						long 		theRefCon) {
	
	#pragma unused (classWanted,container,containerClass,theRefCon)
	
	AEDesc tempdesc;
	hdlhashtable htable;
	bigstring bs;
	register OSErr err;
	
	switch (keyform) {
		
		case formName:
			err = AECoerceDesc (keydesc, typeChar, &tempdesc);
			
			if (err != noErr)
				return (err);
			
			texthandletostring ((*keydesc).dataHandle, bs);
			
			if (!langexpandtodotparams (bs, &htable, bs)) {
				
				AEDisposeDesc (&tempdesc);
				
				return(errAENoSuchObject);
				}
			
			*resultToken = tempdesc;
			
			return (noErr);
		
		/*
		case formAbsolutePosition:
			err = AECoerceDesc (keydesc, typeLongInteger, &tempdesc);
			
			if (err != noErr)
				return (err);
			
			return (errAEEventNotHandled);		// I've got no clue of how to access files by number.
		*/
		
		default:								// I don't handle any other key forms.
			return (errAEEventNotHandled);
		}
	} /*langsystem7accessobject*/


static void setupdescriptor (Handle hdata, AEDesc *desc) {
	
	register AEDesc *d = desc;
	
	(*d).dataHandle = hdata;
	
	if ((*d).dataHandle == nil)
		(*d).descriptorType = typeNull;
	else
		(*d).descriptorType = typeObjectSpecifier;
	} /*setupdescriptor*/


static OSErr langsystem7parseobject (const AEDesc *object, DescType *class, AEDesc *container, DescType *keyform, AEDesc *keydata) {
	
	AEDesc objdesc;
	DescType type;
	long size;
	register OSErr err;
	
	err = AECoerceDesc (object, typeAERecord, &objdesc);
	
	if (err != noErr)
		return (err);
	
	setupdescriptor (nil, container);
	
	setupdescriptor (nil, keydata);
	
	err = AEGetKeyPtr (&objdesc, keyAEDesiredClass, typeType, &type, (Ptr) class, sizeof (class), &size);
	
	if (err != noErr)
		goto exit;
	
	err = AEGetKeyPtr (&objdesc, keyAEKeyForm, typeEnumerated, &type, (Ptr) keyform, sizeof (keyform), &size);
	
	if (err != noErr)
		goto exit;
	
	err = AEGetKeyDesc (&objdesc, keyAEContainer, typeWildCard, container);
	
	if (err != noErr)
		goto exit;
	
	err = AEGetKeyDesc (&objdesc, keyAEKeyData, typeWildCard, keydata);
	
	exit:
	
	AEDisposeDesc (&objdesc);
	
	if (err != noErr) {
		
		AEDisposeDesc (container);
		
		AEDisposeDesc (keydata);
		}
	
	return (err);
	} /*langsystem7parseobject*/


static OSErr langsystem7resolve (const AEDesc *object, AEDesc *resultdesc) {
	
	DescType class;
	DescType keyform;
	AEDesc keydata;
	AEDesc container;
	register OSErr err;
	
	err = langsystem7parseobject (object, &class, &container, &keyform, &keydata);
	
	if (err != noErr)
		return (err);
	
	if (container.descriptorType != typeNull)
		err = errAEEventNotHandled;
	else
		err = langsystem7accessobject (cCell, &container, cApplication, keyform, &keydata, resultdesc, 0);
	
	AEDisposeDesc (&container);
	
	AEDisposeDesc (&keydata);
	
	return (err);
	} /*langsystem7resolve*/


boolean objspectoaddress (tyvaluerecord *val) {
	
	/*
	9/23/92 dmb: turn the objspec value into an address value.
	
	this will only work if the object specification is for a cell 
	in the data. currently, that means it must be a formName 
	specifier of a cCell element with a null container
	*/
	
	register tyvaluerecord *v = val;
	AEDesc objdesc;
	AEDesc token;
	OSErr errcode;
	
	setupdescriptor ((*v).data.objspecvalue, &objdesc);
	
	errcode = langsystem7resolve (&objdesc, &token);
	
	if (errcode != noErr)
		return (false);
	
	disposevaluerecord (*v, true);
	
	if (!setheapvalue (token.dataHandle, stringvaluetype, v)) /*consumes handle*/
		return (false);
	
	return (stringtoaddress (v));
	} /*objspectoaddress*/


boolean objspectofilespec (tyvaluerecord *val) {
	
	/*
	2.1b5 dmb: added for AppleScript compatibility
	*/
	
	register tyvaluerecord *v = val;
	AEDesc objdesc;
	DescType class;
	DescType containertype;
	DescType keyform;
	AEDesc keydata;
	AEDesc container;
	
	setupdescriptor ((*v).data.objspecvalue, &objdesc);
	
	if (oserror (langsystem7parseobject (&objdesc, &class, &container, &keyform, &keydata)))
		return (false);
	
	containertype = container.descriptorType;
	
	AEDisposeDesc (&container);
	
	if ((containertype == typeNull) && (class == cFile) && (keyform == formName)) {
		
		disposevaluerecord (*v, true); /*dispose orig value which is also objdesc*/
		
		if (!setdescriptorvalue (keydata, v)) /*keydata is on temp stack, or disposed on error*/
			return (false);
		
		if (!coercetostring (v))
			return (false);
		
		return (coercetofilespec (v)); /*a little recursion here, since that's probably our caller*/
		}
	else {
		
		AEDisposeDesc (&keydata);
		
		langcoerceerror (v, filespecvaluetype);
		
		return (false);
		}
	} /*objspectofilespec*/


boolean filespectoobjspec (tyvaluerecord *val) {
	
	/*
	2.1b5 dmb: added for AppleScript compatibility
	*/
	
	register tyvaluerecord *v = val;
	AEDesc objdesc;
	AEDesc keydata;
	tyfilespec fs = **(*v).data.filespecvalue;
	bigstring bs;
	AEDesc container = {typeNull, nil};
	OSErr err;
	
	filespectopath (&fs, bs);
	
	err = AECreateDesc (typeChar, (Ptr) bs + 1, stringlength (bs), &keydata);
	
	if (err == noErr)
		err = CreateObjSpecifier (cFile, &container, formName, &keydata, true, &objdesc);
	
	if (oserror (err))
		return (false);
	
	disposevaluerecord (*v, true);
	
	return (setheapvalue (objdesc.dataHandle, objspecvaluetype, v));
	} /*filespectoobjspec*/


static boolean stringtoobjspec (tyvaluerecord *val) {
	
	/*
	2.1b2 dmb: fixed bug where true would be returned for a bad objspec.
	
	2.1b2 dmb: added special case for empty strings
	
	2.1b6 dmb: fixed error offset maintenance
	*/
	
	Handle htext;
	hdltreenode hmodule;
	boolean fl = false;
	unsigned short savelines;
	unsigned short savechars;
	
	if (gethandlesize ((*val).data.stringvalue) == 0) { /*empty string -> null spec*/
		
		disposevaluerecord (*val, true);
		
		(*val).valuetype = objspecvaluetype;
		
		(*val).data.objspecvalue = nil;
		
		return (true);
		}
	
	if (!copyhandle ((*val).data.stringvalue, &htext))
		return (false);
	
	savelines = ctscanlines;
	
	savechars = ctscanchars;
	
	disablelangerror ();
	
	fl = langcompiletext (htext, false, &hmodule); /*always disposes htext*/
	
	enablelangerror ();
	
	if (fl) {
		
		register hdltreenode h = (**hmodule).param1; /*copy into register*/
		
		fl = isobjspectree (h);
		
		if (fl) {
			
			disposevaluerecord (*val, true);
			
			disablelangerror (); /*we'll generate own error w/correct position*/
			
			fl = evaluateobjspec (h, val);
			
			enablelangerror ();
			}
		
		langdisposetree (hmodule);
		}
	
	ctscanlines = savelines;
	
	ctscanchars = savechars;
	
	if (!fl)
		langerror (badobjectspecificationerror);
	
	return (fl);
	} /*stringtoobjspec*/


static boolean objtostring (AEDesc *, boolean, DescType, AEDesc *, bigstring); /*forward*/


typedef struct tyobjspecitem { /*data within special object specifier structures*/
	
	AEKeyword key;
	
	DescType type;
	
	long size;
	
	/*data follows*/
	} tyobjspecitem;


static boolean getobjspeckeydesc (AEDesc *objdata, OSType desiredkey, AEDesc *keydata) {
	
	register Handle h = (*objdata).dataHandle;
	register byte *p;
	long ctitems;
	tyobjspecitem objspecitem;
	OSErr err;
	
	HLock (h);
	
	p = (byte *) *(*objdata).dataHandle;
	
	if (*(OSType *)p == (*objdata).descriptorType) /*data begins with redundant type; skip it*/
		p += 4;
	
	BlockMove (p, &ctitems, 4);
	
	p += 8;
	
	while (--ctitems >= 0) {
		
		BlockMove (p, &objspecitem, sizeof (tyobjspecitem));
		
		p += sizeof (objspecitem);
		
		if (objspecitem.key == desiredkey) {
			
			err = AECreateDesc (objspecitem.type, (Ptr) p, objspecitem.size, keydata);
			
			goto exit;
			}
		
		p += objspecitem.size;
		
		if (odd (objspecitem.size))
			++p;
		}
	
	err = errAEDescNotFound;
	
	exit:
	
	HUnlock (h);
	
	return (!oserror (err));
	} /*getobjspeckeydesc*/


static void operatortostring (OSType op, bigstring bsop) {
	
	byte *p;
	
	switch (op) {
		
		case kAEEquals:
			p = "\p==";
			
			break;
		
		case kAEGreaterThan:
			p = "\p>";
			
			break;
		
		case kAELessThan:
			p = "\p<";
			
			break;
		
		case kAEGreaterThanEquals:
			p = "\p>=";
			
			break;
		
		case kAELessThanEquals:
			p = "\p<=";
			
			break;
		
		case kAEBeginsWith:
			p = "\pbeginsWith";
			
			break;
		
		case kAEEndsWith:
			p = "\pendsWith";
			
			break;
		
		case kAEContains:
			p = "\pcontains";
			
			break;
		
		case '<>  ':
		case '!=  ':
			p = "\p!=";
			
			break;
		
		case kAENOT:
			p = "\pnot";
		
			break;
		
		case kAEAND:
			p = "\pand";
		
			break;
		
		case kAEOR:
			p = "\por";
			
			break;
		}
	
	copystring (p, bsop);
	} /*operatortostring*/


static boolean testtostring (AEDesc *testdata, bigstring bstest) {
	
	bigstring bs1, bs2;
	byte bsop [16];
	AEDesc desc;
	DescType type;
	boolean fl = false;
	OSType op;
	
	setemptystring (bstest);
	
	type = (*testdata).descriptorType;
	
	switch (type) {
		
		case typeLogicalDescriptor: {
			AEDesc itemdesc;
			OSType key;
			long n, ctitems;
			
			if (!getobjspeckeydesc (testdata, keyAELogicalOperator, &desc))
				goto exit;
			
			op = **(OSType **)desc.dataHandle;
			
			operatortostring (op, bsop);
			
			AEDisposeDesc (&desc);
			
			if (!getobjspeckeydesc (testdata, keyAELogicalTerms, &desc))
				goto exit;
			
			if (oserror (AECountItems (&desc, &ctitems)))
				goto exit;
			
			for (n = 1; n <= ctitems; ++n) {
				
				if (oserror (AEGetNthDesc (&desc, n, typeWildCard, &key, &itemdesc)))
					goto exit;
				
				if (!testtostring (&itemdesc, bs1))
					goto exit;
				
				AEDisposeDesc (&itemdesc);
				
				if (n == 1) {
					
					copystring (bs1, bstest);
					
					if (op == kAENOT) { /*special case -- not infix, only 1 comparison*/
						
						short ix = patternmatch ("\p==", bstest); /*'nother special case*/
						
						if (ix > 0)
							bstest [ix] = '!';
						else
							parsedialogstring ("\p^0 (^1)", bsop, bs1, nil, nil, bstest);
						}
					}
				else {
					copystring (bstest, bs2);
					
					parsedialogstring ("\p^0 ^1 ^2", bs2, bsop, bs1, nil, bstest);
					}
				}
			
			AEDisposeDesc (&desc);
			
			break;
			}
		
		case typeCompDescriptor:
			
			if (!getobjspeckeydesc (testdata, keyAECompOperator, &desc))
				goto exit;
			
			op = **(OSType **)desc.dataHandle;
			
			operatortostring (op, bsop);
			
			AEDisposeDesc (&desc);
			
			if (!getobjspeckeydesc (testdata, keyAEObject1, &desc))
				goto exit;
			
			if (!objtostring (&desc, true, 0, nil, bs1))
				goto exit;
			
			if (!getobjspeckeydesc (testdata, keyAEObject2, &desc))
				goto exit;
			
			if (!objtostring (&desc, true, 0, nil, bs2))
				goto exit;
			
			parsedialogstring ("\p^0 ^1 ^2", bs1, bsop, bs2, nil, bstest);
			
			break;
		
		default:
			return (false);
		}
	
	fl = true;
	
	exit:
	
	return (fl);
	} /*testtostring*/


static boolean rangetostring (DescType rangeclass, AEDesc *rangecontainer, AEDesc *rangedata, bigstring bsrange) {
	
	bigstring bs1, bs2;
	AEDesc desc;
	boolean fl = false;
	
	if (!getobjspeckeydesc (rangedata, keyAERangeStart, &desc))
		goto exit;
	
	if (!objtostring (&desc, true, rangeclass, rangecontainer, bs1))
		goto exit;
	
	if (!getobjspeckeydesc (rangedata, keyAERangeStop, &desc))
		goto exit;
	
	if (!objtostring (&desc, true, rangeclass, rangecontainer, bs2))
		goto exit;
	
	parsedialogstring ("\p^0 to ^1", bs1, bs2, nil, nil, bsrange);
	
	fl = true;
	
	exit:
	
	return (fl);
	} /*rangetostring*/


static boolean getostypeidentifier (OSType id, bigstring bsidentifier) {
	
	if (getostypedisplaystring (id, bsidentifier)) /*found name for ostype*/
		return (true);
	
	insertchar ('[', bsidentifier); /*must bracket string4 to make it an identifier*/
	
	pushchar (']', bsidentifier);
	
	return (false);
	} /*getostypeidentifier*/


static boolean objtostring (AEDesc *objdesc, boolean fldisposeobj, DescType examinedclass, AEDesc *examinedcontainer, bigstring bsobj) {
	
	/*
	create a string representation of objdesc.
	
	examinedclass isusually 0, but if we're being called while parsing a range, it's 
	the class of the object being examined, in which case we can potentially omit 
	redundant class specification of this element
	
	2.1b1 dmb: supplementing examinedclass, we now take examinedcontainer as well to 
	avoid redundancy.
	
	2.1b4 dmb: check for typeNull before getobjectmodeldisplaystring to avoid 
	infinite recursion
	*/
	
	DescType class;
	DescType keyform;
	AEDesc keydata;
	AEDesc container;
	tyvaluerecord vobj;
	tyvaluerecord vkey;
	byte bsclass [64];
	bigstring bsitem;
	boolean fl;
	
	setemptystring (bsobj);
	
	if ((*objdesc).descriptorType != typeObjectSpecifier) {
		
		switch ((*objdesc).descriptorType) {
			
			case typeObjectBeingExamined:
				copystring ("\pit", bsobj);
				
				AEDisposeDesc (objdesc);
				
				break;
			
			case typeNull:
				AEDisposeDesc (objdesc); /*leave string empty*/
				
				break;
			
			default:
				if (!setdescriptorvalue (*objdesc, &vobj))
					return (false);
				
				if (!getobjectmodeldisplaystring (&vobj, bsobj))
					return (false);
				
				disposevaluerecord (vobj, false);
				
				break;
			}
		
		return (true);
		}
	
	fl = false;
	
	while (true) {
		
		if (oserror (langsystem7parseobject (objdesc, &class, &container, &keyform, &keydata)))
			goto exit;
		
		if (!setdescriptorvalue (keydata, &vkey)) /*if successful, keydata is on temp stack*/
			goto exit;
		
		getostypeidentifier (class, bsclass);
		
		switch (keyform) {
			
			case formTest: {
				bigstring bstest;
				
				if (!testtostring (&keydata, bstest))
					goto exit;
				
				parsedialogstring ("\p^0 [^1]", bsclass, bstest, nil, nil, bsitem);
				
				break;
				}
			
			case formRange: {
				bigstring bsrange;
				
				if (!rangetostring (class, &container, &keydata, bsrange))
					goto exit;
				
				parsedialogstring ("\p^0 [^1]", bsclass, bsrange, nil, nil, bsitem);
				
				break;
				}
			
			case formPropertyID: {
				if (!coercetoostype (&vkey))
					goto exit;
				
				getostypeidentifier (vkey.data.ostypevalue, bsitem);
				
				break;
				}
			
			case formName:
			case formAbsolutePosition:
			case formRelativePosition: {
				bigstring bskey;
				
				if (!getobjectmodeldisplaystring (&vkey, bskey))
					goto exit;
				
				if (equaldescriptors (examinedcontainer, &container)) {
					
					AEDisposeDesc (&container);
					
					container.descriptorType = typeCurrentContainer;
					}
				
				if ((examinedclass == class) && (container.descriptorType == typeCurrentContainer))
					copystring (bskey, bsitem);
				else
					parsedialogstring ("\p^0 [^1]", bsclass, bskey, nil, nil, bsitem);
				
				break;
				}
			
			default: {
				byte bsform [64];
				bigstring bskey;
				
				if (!getobjectmodeldisplaystring (&vkey, bskey))
					goto exit;
				
				getostypedisplaystring (keyform, bsform);
				
				parsedialogstring ("\p^0 [^1:^2]", bsclass, bsform, bskey, nil, bsitem);
				
				break;
				}
			}
		
		disposevaluerecord (vkey, false);
		
		if (fldisposeobj)
			AEDisposeDesc (objdesc);
		
		*objdesc = container;
		
		fldisposeobj = true;
		
		if (!isemptystring (bsobj))
			insertchar ('.', bsobj);
		
		if (!insertstring (bsitem, bsobj)) {
			
			insertchar ('É', bsobj);
			
			break;
			}
		
		if ((*objdesc).descriptorType != typeObjectSpecifier)
			break;
		}
	
	fl = true;
	
	exit:
	
	if (fldisposeobj)
		AEDisposeDesc (objdesc);
	
	return (fl);
	} /*objtostring*/


boolean objspectostring (Handle hobjspec, bigstring bs) {
	
	AEDesc objdesc;
	
	setupdescriptor (hobjspec, &objdesc);
	
	return (objtostring (&objdesc, false, 0, nil, bs));
	} /*objspectostring*/


static void getdefaultcontainer (OSType nulltype, AEDesc *containerdesc, boolean fluseexternalcontainer) {
	
	/*
	8/13/92 dmb: create a container descriptor based on nulltype.
	
	the smarts: if nulltype is typeNull, and there is a with statement that 
	defines a container value, then use that container. otherwise, a normal
	null container is returned.
	
	3.0.3 dmb: added fluseexternalcontainer parameter. if true, allow
	"with" statements in external contexts to show through by setting the 
	flfindanyspecialsymbol global to true. in practice, we're allowing 
	string4 property values to be coerced to objspecs in glue scripts with 
	the caller's "with <obj>" statement respected. at the same time, fully-
	formed object specifications don't erroniously pick up a parent 
	container from a calling script. so, we've created an inconsistency 
	that suites our needs and will create expected behavior in most scripts.
	*/
	
	hdlhashnode hnode;

	if (nulltype == typeNull) { /*not a special type of null container*/
		
		tyvaluerecord containerval;
		boolean fl;
		
		flfindanyspecialsymbol = fluseexternalcontainer;
		
		fl = langgetsymbolval (bscontainername, &containerval, &hnode);
		
		flfindanyspecialsymbol = false; /*restore to default*/
				
		if (fl) { /*got container*/
			
			setupdescriptor (containerval.data.objspecvalue, containerdesc);
			
			return;
			}
		}
	
	(*containerdesc).descriptorType = nulltype;
	
	(*containerdesc).dataHandle = nil;
	} /*getdefaultcontainer*/


static boolean createpropertyspecifier (AEDesc *containerdesc, OSType propkey, AEDesc *objectdesc) {
	
	AEDesc keydatadesc;
	OSErr errcode;
	
	errcode = AECreateDesc (typeType, (Ptr) &propkey, longsizeof (propkey), &keydatadesc);
	
	if (oserror (errcode))
		return (false);
	
	errcode = CreateObjSpecifier (cProperty, containerdesc, formPropertyID, &keydatadesc, false, objectdesc);
	
	AEDisposeDesc (&keydatadesc);
	
	return (!oserror (errcode));
	} /*createpropertyspecifier*/


static boolean valtoobjspec (tyvaluerecord *val, OSType nulltype, AEDesc *objectdesc) {
	
	/*
	return an object specifier interpretation of the given value record. 
	val should be a temporary value. object is returned free & clear; the 
	caller is responsible for its handle.
	
	12/24/91 dmb: when coercing from binary, make sure we form null spec
	properly with nil handle.
	
	6/24/92 dmb: fixed heap bug in binary-to-objspec coercion (when null)
	
	9/14/92 dmb: finalized treatment of null objects.
	*/
	
	register tyvaluerecord *v = val;
	register AEDesc *obj = objectdesc;
	OSType id;
	AEDesc containerdesc;
	Handle x;
	
	switch ((*v).valuetype) {
		
		case objspecvaluetype:
			x = (*v).data.objspecvalue;
			
			break;
		
		case binaryvaluetype:
			if (!coercebinaryval (v, objspecvaluetype, 0L, objspecvaluetype))
				return (false);
			
			x = (*v).data.objspecvalue;
			
			if (gethandlesize (x) == 0) { /*null spec; special case*/
				
				releaseheaptmp (x);
				
				x = nil;
				}
			
			break;
		
		case recordvaluetype: {
			
			AEDesc desc;
			
			if (!langipcconvertoplist (v, &desc))
				return (false);
			
			if (oserror (AECoerceDesc (&desc, typeObjectSpecifier, obj)))
				return (false);
			
			exemptfromtmpstack (v);
			
			return (true);
			}
		
		case ostypevaluetype:
			id = (*v).data.ostypevalue;
			
			getdefaultcontainer (nulltype, &containerdesc, true);
			
			if (id == typeObjectBeingExamined) /*special "it" value*/
				return (!oserror (AEDuplicateDesc (&containerdesc, obj)));
			
			return (createpropertyspecifier (&containerdesc, id, obj));
		
		default:
			if ((*v).data.longvalue == 0) { /*a nil value -- special case*/
				
				x = nil;
				
				break;
				}
			
			langcoerceerror (v, objspecvaluetype);
			
			return (false);
		} /*switch*/
	
	(*obj).dataHandle = x;
	
	if (x == nil) {
		
		(*obj).descriptorType = typeNull; /*don't use nulltype here; nil spec is always typeNull*/
		}
	else {
		
		exemptfromtmpstack (v);
		
		(*obj).descriptorType = typeObjectSpecifier;
		}
		
	return (true);
	} /*valtoobjspec*/


boolean coercetoobjspec (tyvaluerecord *v) {
	
	/*
	9/14/92 dmb: this guy's guts have been moved into valtoobjspec
	
	2.1b2 dmb: added missing case for list->objspec
	
	2.1b5 dmb: added support for filespec->objspec
	*/
	
	AEDesc objectdesc;
	
	switch ((*v).valuetype) {
		
		case objspecvaluetype:
			return (true);
		
		case stringvaluetype:
			return (stringtoobjspec (v));
		
		case listvaluetype:
			return (coercelistvalue (v, objspecvaluetype));
		
		case filespecvaluetype:
			return (filespectoobjspec (v));
		
		default:
			if (!valtoobjspec (v, typeNull, &objectdesc))
				return (false);
			
			return (setheapvalue (objectdesc.dataHandle, objspecvaluetype, v));
		} /*switch*/
	} /*coercetoobjspec*/


boolean setobjspecverb (hdltreenode hparam1, tyvaluerecord *val) {
	
	/*
	a brute-force verb to building object specifiers.
	
	8/31/92 dmb: if the keyform isn't one of the three simple grammar forms, 
	assume that the script writer is trying to do a custom keyform, and 
	take the keydata literally. with this change, setobj continues to offer 
	functionality that isn't otherwise available.
	*/
	
	OSType class;
	tyvaluerecord container;
	OSType keyform;
	long longkey;
	bigstring bskey;
	OSType propkey;
	tyvaluerecord customkey;
	AEDesc containerdesc;
	AEDesc keydatadesc;
	AEDesc objspecdesc;
	OSErr errcode = noErr;
	
	
	#ifdef flsystem6
	
	if (!langcanuseappleevents ())
		return (false);
	
	#endif
	
	if (!getostypevalue (hparam1, 1, &class))
		return (false);
	
	if (!getobjspecparam (hparam1, 2, &container))
		return (false);
	
	if (!getostypevalue (hparam1, 3, &keyform))
		return (false);
	
	flnextparamislast = true;
	
	switch (keyform) {
		
		case formName:
			if (!getstringvalue (hparam1, 4, bskey))
				return (false);
			
			errcode = AECreateDesc (typeChar, (Ptr) bskey + 1, (long) stringlength (bskey), &keydatadesc);
			
			break;
		
		case formAbsolutePosition:
			if (!getlongvalue (hparam1, 4, &longkey))
				return (false);
			
			errcode = AECreateDesc (typeLongInteger, (Ptr) &longkey, longsizeof (longkey), &keydatadesc);
			
			break;
		
		case formPropertyID:
			if (!getostypevalue (hparam1, 4, &propkey))
				return (false);
			
			errcode = AECreateDesc (typeType, (Ptr) &propkey, longsizeof (propkey), &keydatadesc);
			
			break;
		
		default:
			if (!getparamvalue (hparam1, 4, &customkey))
				return (false);
			
			if (!valuetodescriptor (&customkey, &keydatadesc)) /*consumes customkey*/
				return (false);
			
			break;
			
		}
	
	if (oserror (errcode))
		return (false);
	
	setupdescriptor (container.data.objspecvalue, &containerdesc);
	
	errcode = CreateObjSpecifier (class, &containerdesc, keyform, &keydatadesc, false, &objspecdesc);
	
	AEDisposeDesc (&keydatadesc);
	
	if (oserror (errcode))
		return (false);
	
	return (setheapvalue (objspecdesc.dataHandle, objspecvaluetype, val));
	} /*setobjspecverb*/


static boolean getclassvalue (hdltreenode htree, OSType *class) {
	
	tyvaluerecord val;
	
	if ((**htree).nodetype == bracketop) /*shed brackets if present*/
		htree = (**htree).param1;
	
	if (!evaluatetree (htree, &val))
		return (false);
	
	if (!coercetoostype (&val))
		return (false);
	
	*class = val.data.ostypevalue;
	
	return (true);
	} /*getclassvalue*/


static boolean evaluateobject (hdltreenode htree, OSType nulltype, AEDesc *objectdesc); /*forward*/


static boolean evaluateproperty (hdltreenode htree, OSType nulltype, AEDesc *objectdesc) {
	
	/*
	1/25/93 dmb: keep track of tmps and dispose if created (to avoid tmpstack overflow)
	*/
	
	register hdltreenode h = htree;
	register hdltreenode hp1;
	register tytreetype op;
	AEDesc containerdesc;
	OSType propkey;
	boolean fltmp = false;
	
	assert (h != nil);
	
	langseterrorline (h); /*set globals for error reporting*/
	
	op = (**h).nodetype; /*copy into register*/
	
	hp1 = (**h).param1;
	
	switch (op) {
		
		case identifierop:
		case bracketop:
			if (!getclassvalue (h, &propkey))
				return (false);
			
			getdefaultcontainer (nulltype, &containerdesc, true);
			
			break;
		
		case dotop:
			if (!evaluateobject ((**h).param1, nulltype, &containerdesc)) /*daisy-chain recursion*/
				return (false);
			
			pushtmpstack (containerdesc.dataHandle); /*until it's merged*/
			
			fltmp = true;
			
			if (!getclassvalue ((**h).param2, &propkey))
				return (false);
			
			break;
		
		default:
			langerror (badobjectspecificationerror);
			
			return (false);
		}
	
	if (!createpropertyspecifier (&containerdesc, propkey, objectdesc))
		return (false);
	
	if (fltmp)
		releaseheaptmp (containerdesc.dataHandle);
	
	return (true);
	} /*evaluateproperty*/


static boolean evaluatesimplekey (hdltreenode htree, OSType *keyform, AEDesc *keydatadesc) {
	
	/*
	8/10/92 dmb: fixed constants for formRelativePosition specifiers
	*/
	
	bigstring bskey;
	DescType type;
	long longkey;
	tyvaluerecord keyval;
	OSErr errcode;
	
	if (!evaluatetree (htree, &keyval))
		return (false);
	
	switch (keyval.valuetype) {
		
		case stringvaluetype:
			pullstringvalue (&keyval, bskey);
			
			errcode = AECreateDesc (typeChar, (Ptr) bskey + 1, (long) stringlength (bskey), keydatadesc);
			
			*keyform = formName;
			
			break;
		
		case ostypevaluetype:
			longkey = (long) keyval.data.ostypevalue;
			
			if ((longkey == kAENext) || (longkey == kAEPrevious)) {
				
				type = typeEnumeration;
				
				*keyform = formRelativePosition;
				}
			else {
				
				type = typeAbsoluteOrdinal;
				
				*keyform = formAbsolutePosition;
				}
			
			errcode = AECreateDesc (type, (Ptr) &longkey, longsizeof (longkey), keydatadesc);
			
			break;
		
		default:
			if (!coercetolong (&keyval))
				return (false);
			
			longkey = keyval.data.longvalue;
			
			errcode = AECreateDesc (typeLongInteger, (Ptr) &longkey, longsizeof (longkey), keydatadesc);
			
			*keyform = formAbsolutePosition;
			
			break;
		}
	
	return (!oserror (errcode));
	} /*evaluatesimplekey*/


static boolean evaluatefield (hdltreenode htree, OSType *key, AEDesc *data) {
	
	/*
	3/23/93 dmb: htree is a x:y field specification. return the field key, 
	and the field data as a descriptor record whose dataHandle is not on the 
	temp stack.
	*/
	
	register hdltreenode h = htree;
	tyvaluerecord keyval;
	tyvaluerecord itemval;
	
	assert ((**h).nodetype == fieldop);
	
	if (!evaluatetree ((**h).param1, &keyval))
		return (false);
	
	if (!coercetoostype (&keyval))
		return (false);
	
	if (!evaluatetree ((**h).param2, &itemval))
		return (false);
	
	if (!valuetodescriptor (&itemval, data)) /*consumes itemval*/
		return (false);
	
	*key = keyval.data.ostypevalue;
	
	return (true);
	} /*evaluatefield*/


static boolean evaluatecustomkey (hdltreenode htree, OSType *keyform, AEDesc *keydatadesc) {
	
	/*
	3/23/93 dmb: a custom key looks like a single field in a record, 
	an x:y field specification. our job is to return the keyform and keydata, without anything on 
	the temp stack
	*/
	
	if (!evaluatefield (htree, keyform, keydatadesc))
		return (false);
	
	/*
	removeheaptmp ((*keydatadesc).dataHandle);
	*/
	
	return (true);
	} /*evaluatecustomkey*/


static boolean evaluatecomparison (hdltreenode htree, DescType operator, AEDesc *keydatadesc) {
	
	/*
	1/25/93 dmb: dispose tmps (to avoid tmpstack overflow)
	*/
	
	register hdltreenode h = htree;
	AEDesc objectdesc;
	tyvaluerecord val;
	AEDesc valdesc;
	OSErr errcode;
	
	if (!evaluateobject ((**h).param1, typeObjectBeingExamined, &objectdesc))
		return (false);
	
	pushtmpstack (objectdesc.dataHandle); /*until it's merged*/
	
	if (!evaluatetree ((**h).param2, &val))
		return (false);
	
	if (!valuetodescriptor (&val, &valdesc))
		return (false);
	
	errcode = CreateCompDescriptor (operator, &objectdesc, &valdesc, false, keydatadesc);
	
	releaseheaptmp (objectdesc.dataHandle); /*keep tmpstack cleared out*/
	
	AEDisposeDesc (&valdesc);
	
	return (!oserror (errcode));
	} /*evaluatecomparison*/


static boolean evaluatetest (hdltreenode, tytreetype, AEDesc *);


static boolean evaluatelogical (hdltreenode hcomp1, tytreetype op1, hdltreenode hcomp2, tytreetype op2, DescType operator, AEDesc *keydatadesc) {
	
	AEDesc compdesc1, compdesc2;
	AEDesc listdesc;
	
	if (oserror (AECreateList (nil, 0, false, &listdesc)))
		return (false);
	
	if (!evaluatetest (hcomp1, op1, &compdesc1))
		goto error;
	
	if (oserror (AEPutDesc (&listdesc, 0, &compdesc1)))
		goto error;
	
	AEDisposeDesc (&compdesc1);
	
	if (hcomp2 != nil) {
		
		if (!evaluatetest (hcomp2, op2, &compdesc2))
			goto error;
		
		if (oserror (AEPutDesc (&listdesc, 0, &compdesc2)))
			goto error;
		
		AEDisposeDesc (&compdesc2);
		}
	
	return (!oserror (CreateLogicalDescriptor (&listdesc, operator, true, keydatadesc)));
	
	error: {
		
		AEDisposeDesc (&listdesc);
		
		return (false);
		}
	} /*evaluatelogical*/


static boolean evaluatetest (hdltreenode htree, tytreetype op, AEDesc *keydatadesc) {
	
	/*
	12/8/92 dmb: created this routine to dispath logical & comarison ops, so 
	that logical operands can in turn be any kind of test, not just comparisons. 
	this removes the 2-criteria limiation of the 1st release of 2.0.
	
	not: the reason that op is passed in is to support the handling of NEop, since 
	there is not corresponding AE comparison operator. instead, if must be forced 
	to EQop, and put inside of a logical NOT expression.
	*/
	
	register hdltreenode h = htree;
	register DescType operator;
	hdltreenode hp1 = (**h).param1;
	
	if (op == noop) /*not pre-determined*/
		
		op = (**h).nodetype;
	
	switch (op) {
		
		case EQop:
			operator = kAEEquals;
			
			break;
		
		case GTop:
			operator = kAEGreaterThan;
			
			break;
		
		case LTop:
			operator = kAELessThan;
			
			break;
		
		case GEop:
			operator = kAEGreaterThanEquals;
			
			break;
		
		case LEop:
			operator = kAELessThanEquals;
			
			break;
		
		case beginswithop:
			operator = kAEBeginsWith;
			
			break;
		
		case endswithop:
			operator = kAEEndsWith;
			
			break;
		
		case containsop:
			operator = kAEContains;
			
			break;
		
		case NEop:
			return (evaluatelogical (h, EQop, nil, (tytreetype) 0, kAENOT, keydatadesc));
		
		case notop:
			return (evaluatelogical (hp1, noop, nil, (tytreetype) 0, kAENOT, keydatadesc));
		
		case andandop:
			return (evaluatelogical (hp1, noop, (**h).param2, (tytreetype) 0, kAEAND, keydatadesc));
		
		case ororop:
			return (evaluatelogical (hp1, noop, (**h).param2, (tytreetype) 0, kAEOR, keydatadesc));
		
		default:
			langlongparamerror (badobjectspecificationerror, (long) op);
			
			return (false);
		}
	
	/*common code for comarisons*/
	
	return (evaluatecomparison (h, operator, keydatadesc));
	} /*evaluatetest*/


static boolean evaluateboundryobject (hdltreenode htree, OSType rangeclass, AEDesc *objectdesc) {
	
	register hdltreenode h = htree;
	OSType keyform;
	AEDesc containerdesc;
	AEDesc keydatadesc;
	OSErr errcode;
	
	if (isobjspectree (h))
		return (evaluateobject (h, typeCurrentContainer, objectdesc));
	
	if (!evaluatesimplekey (h, &keyform, &keydatadesc))
		return (false);
	
	containerdesc.descriptorType = typeCurrentContainer;
	
	containerdesc.dataHandle = nil;
	
	errcode = CreateObjSpecifier (rangeclass, &containerdesc, keyform, &keydatadesc, true, objectdesc);
	
	return (!oserror (errcode));
	} /*evaluateboundryobject*/


static boolean evaluaterange (hdltreenode htree, OSType class, AEDesc *keydatadesc) {
	
	register hdltreenode h = htree;
	AEDesc rangedesc1, rangedesc2;
	OSErr errcode;
	
	if (!evaluateboundryobject ((**h).param1, class, &rangedesc1))
		return (false);
	
	if (!evaluateboundryobject ((**h).param2, class, &rangedesc2)) {
		
		AEDisposeDesc (&rangedesc1);
		
		return (false);
		}
	
	errcode = CreateRangeDescriptor (&rangedesc1, &rangedesc2, true, keydatadesc);
	
	return (!oserror (errcode));
	} /*evaluaterange*/


static boolean evaluateelement (hdltreenode htree, OSType nulltype, OSType *elementclass, AEDesc *objectdesc) {
	
	/*
	9/14/92 dmb: interpret double-array operations has specifying the same class as the 
	container, so that:
		
		word [it beginsWith "foo"] [last]
		
	means "the last word that begins with "foo". assuming, of course, that the app 
	knows how to resolve such a specifier
	
	1/25/93 dmb: keep track of tmps and dispose if created (to avoid tmpstack overflow)
	*/
	
	register hdltreenode h = htree;
	register hdltreenode hp1, hp2;
	register tytreetype op;
	OSType keyform;
	AEDesc containerdesc;
	AEDesc keydatadesc;
	OSType class;
	OSErr errcode;
	boolean fltmp = false;
	
	assert (h != nil);
	
	langseterrorline (h); /*set globals for error reporting*/
	
	op = (**h).nodetype; /*copy into register*/
	
	assert (op == arrayop);
	
	hp1 = (**h).param1;
	
	op = (**hp1).nodetype;
	
	switch (op) {
		
		case identifierop:
		case bracketop:
			if (!getclassvalue (hp1, &class))
				return (false);
			
			getdefaultcontainer (nulltype, &containerdesc, false);
			
			break;
		
		case dotop:
			if (!evaluateobject ((**hp1).param1, nulltype, &containerdesc)) /*daisy-chain recursion*/
				return (false);
			
			pushtmpstack (containerdesc.dataHandle); /*until it's merged*/
			
			fltmp = true;
			
			if (!getclassvalue ((**hp1).param2, &class))
				return (false);
			
			break;
		
		case arrayop:
			if (!evaluateelement (hp1, nulltype, &class, &containerdesc))
				return (false);
			
			pushtmpstack (containerdesc.dataHandle); /*until it's merged*/
			
			fltmp = true;
			
			break;
		
		default:
			langlongparamerror (badobjectspecificationerror, (long) op);
			
			return (false);
		}
	
	hp2 = (**h).param2;
	
	op = (**hp2).nodetype;
	
	switch (op) {
		
		case EQop:
		case GTop:
		case LTop:
		case GEop:
		case LEop:
		case beginswithop:
		case endswithop:
		case containsop:
		case NEop:
		case notop:
		case andandop:
		case ororop:
			if (!evaluatetest (hp2, op, &keydatadesc))
				return (false);
			
			keyform = formTest;
			
			break;
		
		case rangeop:
			if (!evaluaterange (hp2, class, &keydatadesc))
				return (false);
			
			keyform = formRange;
			
			break;
		
		case fieldop:
			if (!evaluatecustomkey (hp2, &keyform, &keydatadesc))
				return (false);
			
			break;
		
		default:
			if (!evaluatesimplekey (hp2, &keyform, &keydatadesc))
				return (false);
			
			break;
		}
	
	errcode = CreateObjSpecifier (class, &containerdesc, keyform, &keydatadesc, false, objectdesc);
	
	if (fltmp)
		releaseheaptmp (containerdesc.dataHandle);
	
	AEDisposeDesc (&keydatadesc);
	
	*elementclass = class;
	
	return (!oserror (errcode));
	} /*evaluateelement*/


static boolean evaluateobject (hdltreenode htree, OSType nulltype, AEDesc *objectdesc) {
	
	/*
	evaluate an object specifier code tree
	
	3/18/93 dmb: added stack space check
	*/
	
	register hdltreenode h = htree;
	register tytreetype op;
	OSType class;
	tyvaluerecord val;
	
	if (fllangerror) { /*a language error dialog has appeared, unwind*/
		
		return (false); /*return false, aid in the unwinding process*/
		}
	
	if (!langcheckstackspace ())
		return (false);
	
	assert (h != nil);
	
	op = (**h).nodetype; /*copy into register*/
	
	/*langseterrorline (h); /*set globals for error reporting*/
	
	switch (op) {
		
		case identifierop:
			if (!evaluatetree (h, &val))
				return (false);
			
			if (!valtoobjspec (&val, nulltype, objectdesc))
				return (false);
			
			break;
			
		case bracketop:
			if (!evaluatetree ((**h).param1, &val))
				return (false);
			
			if (!valtoobjspec (&val, nulltype, objectdesc))
				return (false);
			
			break;
		
		case dotop:
			if (!evaluateproperty (h, nulltype, objectdesc))
				return (false);
			
			break;
		
		case arrayop:
			if (!evaluateelement (h, nulltype, &class, objectdesc))
				return (false);
			
			break;
		
		default:
			langlongparamerror (badobjectspecificationerror, (long) op);
			
			return (false);
		}
	
	return (true);
	} /*evaluateobject*/


boolean evaluateobjspec (hdltreenode htree, tyvaluerecord *vreturned) {
	
	AEDesc objectdesc;
	
	if (!evaluateobject (htree, typeNull, &objectdesc))
		return (false);
	
	return (setheapvalue (objectdesc.dataHandle, objspecvaluetype, vreturned));
	} /*evaluateobjspec*/


static boolean getidvalue (hdltreenode htree, tyvaluerecord *val) {
	
	/*
	just like idvalue, but we don't make a copy
	
	2.1b2 dmb: dive into bracketops to more closely simulate objspec evaluation
	
	7.2.97 dmb: prevent extended symbol searching in langgetsymbolval
	*/
	
	register hdltreenode h = htree;
	bigstring bs;
	hdlhashtable htable;
	boolean fl = false;
	hdlhashnode hnode;
	
	switch ((**h).nodetype) {
		
		case bracketop: /*look for string4 literal only*/
			
			h = (**h).param1;
			
			if ((**h).nodetype != constop)
				break;
			
			*val = (**h).nodeval;
			
			fl = true;
			
			break;
		
		case identifierop:
			
			if (!langgetidentifier (h, bs))
				break;
			
			langsearchpathlookup (bs, &htable);
			
			pushhashtable (htable);
			
			fl = langgetsymbolval (bs, val, &hnode);
			
			pophashtable ();
			
			break;
		
		default:
			break;
		}
	
	return (fl);
	} /*getidvalue*/


boolean isobjspectree (hdltreenode htree) {
	
	/*
	determine whether the given code tree appears to be an object specification.
	
	at this point we're observing the following rule: an objspec expression may 
	be a dotted id or an array operation (which yeild properties & elements 
	respectively), and the root of the tree must be an objspec or a string4 (a 
	class id).  any such expression is likely to be intended as an objspec, and 
	will certainly fail when evaluated normally.
	
	note that the objspec verb can always be used to force an expression to be 
	evaluated as an object specifier.
	
	also note that we don't call idvalue at the end to avoid creating a copy of 
	the value record.
	
	special note: for performance, we currently assume that we're only being called 
	by dotvalue and arrayvalue, so we can go directly to param1.
	
	1/13/93 dmb: if param2 of a dotop is an undefined identifier, keep scanning so 
	param1 can be checked. this allows the appropriate error message to be generated.
	*/
	
	register hdltreenode h = htree;
	register tytreetype op;
	tyvaluerecord val;
	
	while (true) {
		
		op = (**h).nodetype;
		
		switch (op) {
			
			case dotop: { /*verify current dot node, then continue scanning tree*/
				
				register hdltreenode hp2 = (**h).param2;
				
				if (true /*(**hp2).nodetype == identifierop*/) { /*a node we can check quickly w/no side-effects*/
					
					if (getidvalue (hp2, &val)) {
						
						if (val.valuetype != ostypevaluetype)
							return (false);
						}
					}
				
				break;
				}
			
			case arrayop: /*continue scanning tree*/
				break;
			
			case bracketop:
			case identifierop:
				if (!getidvalue (h, &val))
					return (false);
				
				return ((val.valuetype == objspecvaluetype) || (val.valuetype == ostypevaluetype));
				
			default:
				return (false);
			}
		
		h = (**h).param1;
		}
	} /*isobjspectree*/


#ifndef oplanglists

boolean makelistvalue (hdltreenode htree, tyvaluerecord *vreturned) {
	
	/*
	4/1/93 dmb: don't cleartmpstack here -- we might be creating a value in 
	an expression. just take care of what we create.
	*/
	
	register hdltreenode h;
	tyvaluerecord itemval;
	AEDesc itemdesc;
	AEDesc listdesc;
	OSErr err;
	
	if (oserror (AECreateList (nil, 0, false, &listdesc)))
		return (false);
	
	for (h = htree; h != nil; h = (**h).link) { /*process each expression in the list*/
		
		if (!evaluatetree (h, &itemval))
			goto error;
		
		if (!valuetodescriptor (&itemval, &itemdesc))
			goto error;
		
		err = AEPutDesc (&listdesc, 0, &itemdesc);
		
		AEDisposeDesc (&itemdesc);
		
		if (oserror (err))
			goto error;
		}
	
	return (setheapvalue (listdesc.dataHandle, listvaluetype, vreturned));
	
	error: {
		
		AEDisposeDesc (&listdesc);
		
		return (false);
		}
	} /*makelistvalue*/


boolean makerecordvalue (hdltreenode htree, tyvaluerecord *vreturned) {
	
	/*
	4/1/93 dmb: don't cleartmpstack here -- we might be creating a value in 
	an expression. just take care of what we create.
	*/
	
	register hdltreenode h;
	OSType key;
	AEDesc itemdesc;
	AEDesc listdesc;
	OSErr err;
	
	if (oserror (AECreateList (nil, 0, true, &listdesc)))
		return (false);
	
	for (h = htree; h != nil; h = (**h).link) { /*process each expression in the list*/
		
		if (!evaluatefield (h, &key, &itemdesc))
			goto error;
		
		err = AEPutKeyDesc (&listdesc, key, &itemdesc);
		
		AEDisposeDesc (&itemdesc);
		
		if (oserror (err))
			goto error;
		}
	
	return (setheapvalue (listdesc.dataHandle, recordvaluetype, vreturned));
	
	error: {
		
		AEDisposeDesc (&listdesc);
		
		return (false);
		}
	} /*makerecordvalue*/


static void listvaltodesc (const tyvaluerecord *val, AEDesc *desc) {
	
	(*desc).dataHandle = (*val).data.listvalue;
	
	if ((*val).valuetype == listvaluetype)
		(*desc).descriptorType = typeAEList;
	else
		(*desc).descriptorType = typeAERecord;
	} /*listvaltodesc*/


boolean langgetlistsize (const tyvaluerecord *vlist, long *size) {
	
	AEDesc desc;
	
	listvaltodesc (vlist, &desc);
	
	return (!oserror (AECountItems (&desc, size)));
	} /*langgetlistsize*/


static boolean getnthlistval (const AEDesc *listdesc, long n, OSType *pkey, tyvaluerecord *val) {
	
	AEDesc itemdesc;
	
	if (oserror (AEGetNthDesc (listdesc, n, typeWildCard, pkey, &itemdesc)))
		return (false);
	
	return (setdescriptorvalue (itemdesc, val));
	} /*getnthlistval*/


boolean langgetlistitem (const tyvaluerecord *vlist, long ix, OSType *pkey, tyvaluerecord *vitem) {
	
	/*
	6/17/93 dmb: pass the key informatino on through
	*/
	
	AEDesc desc;
	
	listvaltodesc (vlist, &desc);
	
	return (getnthlistval (&desc, ix, pkey, vitem));
	} /*langgetlistitem*/


static boolean listtostring (const AEDesc *listdesc, tyvaluerecord *val) {
	
	/*
	12/22/92 dmb: starter version: 255-char limit
	
	4/2/93 dmb: next version: 255-char limit for individual items only.
	*/
	
	long i, n;
	OSType key;
	AEDesc itemdesc;
	tyvaluerecord itemval;
	bigstring bs;
	Handle hlist;
	
	if (oserror (AECountItems (listdesc, &n)))
		return (false);
	
	if (!newtexthandle ("\p{", &hlist))
		return (false);
	
	for (i = 1; i <= n; ++i) {
		
		if (oserror (AEGetNthDesc (listdesc, i, typeWildCard, &key, &itemdesc)))
			goto error;
		
		if (!setdescriptorvalue (itemdesc, &itemval))
			goto error;
		
		if ((*listdesc).descriptorType == typeAERecord) {
			
			getostypedisplaystring (key, bs);
			
			pushchar (':', bs);
			
			pushtexthandle (bs, hlist);
			}
		
		if (!getobjectmodeldisplaystring (&itemval, bs)) /*max 253 characters*/
			goto error;
		
		disposevaluerecord (itemval, true); /*don't clog temp stack*/
		
		if (i < n)
			pushstring ("\p, ", bs);
		
		if (!pushtexthandle (bs, hlist))
			goto error;
		}
	
	if (!pushtexthandle ("\p}", hlist))
		goto error;
	
	return (setheapvalue (hlist, stringvaluetype, val));
	
	error:
	
	disposehandle (hlist);
	
	return (false);
	} /*listtostring*/


static boolean makeintegerlist (tyvaluerecord *v, tyvaluetype listtype, void *pints, short ctints, AEDesc *intlist) {
	
	/*
	2.1b2 dmb: create a list containing the short integers in the pints array 
	
	2.1b8 dmb: for now, disallow coercion to record. later, we might have 
	an array of keys for each type.
	*/
	
	register short *pi = (short *) pints;
	boolean flrecord = listtype == recordvaluetype;
	
	if (flrecord) {
		
		langcoerceerror (v, listtype);
		
		return (false);
		}
	
	if (oserror (AECreateList (nil, 0, flrecord, intlist)))
		return (false);
	
	while (--ctints >= 0) { /*process each int in the array*/
		
		if (oserror (AEPutPtr (intlist, 0, typeShortInteger, (Ptr) pi++, sizeof (short)))) {
			
			AEDisposeDesc (intlist);
			
			return (false);
			}
		}
	
	return (true);
	} /*makeintegerlist*/


static boolean pullintegerlist (AEDesc *intlist, short ctints, void *pints) {
	
	/*
	2.1b2 dmb: pull out the array of short integers from the list
	*/
	
	register short *pi = (short *) pints;
	long ctitems;
	long n;
	OSType key;
	OSType type;
	long size;
	
	if (oserror (AECountItems (intlist, &ctitems)))
		return (false);
	
	if (ctitems < ctints) {
		
		langlongparamerror (listcoerceerror, ctints);
		
		return (false);
		}
	
	for (n = 1; n <= ctints; ++n) { /*grab each int in the list*/
		
		if (oserror (AEGetNthPtr (intlist, n, typeShortInteger, &key, &type, (Ptr) pi++, sizeof (short), &size)))
			return (false);
		}
	
	return (true);
	} /*pullintegerlist*/


static Point swappoint (Point pt) {
	
	short temp = pt.h;
	
	pt.h = pt.v;
	
	pt.v = temp;
	
	return (pt);
	} /*swappoint*/


static boolean stringtolist (tyvaluerecord *val, tyvaluetype type) {
	
	/*
	4.1b2 dmb: if a string is actually a list expression, that's what 
	this coercion should yield.
	*/
	
	Handle htext;
	hdltreenode hmodule;
	boolean fl = false;
	unsigned short savelines;
	unsigned short savechars;
	
	if (gethandlesize ((*val).data.stringvalue) == 0) { /*empty string -> null list*/
		
		disposevaluerecord (*val, true);
		
		if (type == listvaluetype)
			return (makelistvalue (nil, val));
		else
			return (makerecordvalue (nil, val));
		}
	
	if (!copyhandle ((*val).data.stringvalue, &htext))
		return (false);
	
	savelines = ctscanlines;
	
	savechars = ctscanchars;
	
	disablelangerror ();
	
	fl = langcompiletext (htext, false, &hmodule); /*always disposes htext*/
	
	enablelangerror ();
	
	if (fl) {
		
		register hdltreenode h = (**hmodule).param1;
		register tytreetype op = (**h).nodetype;
		
		if (type == listvaluetype)
			fl = op == listop;
		else
			fl = op == recordop;
		
		if (fl) {
			
			disposevaluerecord (*val, true);
			
			disablelangerror (); /*we'll generate own error w/correct position*/
			
			fl = evaluatetree (h, val);
			
			enablelangerror ();
			}
		
		langdisposetree (hmodule);
		}
	
	ctscanlines = savelines;
	
	ctscanchars = savechars;
	
	return (fl);
	} /*stringtolist*/


boolean coercetolist (tyvaluerecord *val, tyvaluetype type) {
	
	/*
	4/14/93 dmb: support list <-> record coercion when empty, but don't allow 
	it otherwise
	
	2.1b8 dmb: coercing a list to a record creates a true record
	
	4.1b2 dmb: use stringtolist for string values
	*/
	
	register tyvaluerecord *v = val;
	register tyvaluetype vtype = (*v).valuetype;
	AEDesc valdesc;
	AEDesc listdesc;
	long size;
	OSErr errcode;
	
	if (vtype == type)
		return (true);
	
	switch (vtype) {
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			if (type == listvaluetype)
				return (makelistvalue (nil, v));
			else
				return (makerecordvalue (nil, v));
		
		case externalvaluetype:
			langbadexternaloperror (badexternaloperationerror, *v);
			
			return (false);
		
		case listvaluetype:
		case recordvaluetype:
			if (!langgetlistsize (v, &size))
				return (false);
			
			if (size > 0) {
				
				langcoerceerror (v, type);
				
				return (false);
				}
			
			if (oserror (AECreateList (nil, 0, type == recordvaluetype, &listdesc)))
				return (false);
			
			break;
		
		case pointvaluetype: {
			Point pt = swappoint ((*v).data.pointvalue);
			
			if (!makeintegerlist (v, type, &pt, 2, &listdesc))
				return (false);
			
			break;
			}
		
		case rgbvaluetype: {
			RGBColor rgb = **(*v).data.rgbvalue;
			
			if (!makeintegerlist (v, type, &rgb, 3, &listdesc))
				return (false);
			
			break;
			}
		
		case rectvaluetype: {
			Rect r = **(*v).data.rectvalue;
			
			if (!makeintegerlist (v, type, &r, 4, &listdesc))
				return (false);
			
			break;
			}
		
		case stringvaluetype:
			
			if (stringtolist (v, type))
				return (true);
			
			/*else fall through...*/
		
		default:
			if (!coercetobinary (v))
				return (false);
			
			binarytodesc ((*v).data.binaryvalue, &valdesc);
			
			errcode = AECoerceDesc (&valdesc, langgettypeid (type), &listdesc);
			
			if (errcode != noErr) {
				
				if (errcode == errAECoercionFail) {
					
					coercevalue (v, vtype); /*back to it's original type for the error message*/
					
					langcoerceerror (v, type);
					}
				else
					oserror (errcode);
				
				return (false);
				}
			
			break;
		}
	
	disposevaluerecord (*v, true);
	
	return (setheapvalue (listdesc.dataHandle, type, v));
	} /*coercetolist*/


static boolean coercelistcontents (tyvaluerecord *val, tyvaluetype totype, AEDesc *listdesc, long ctitems) {
	
	/*
	4.0b7 dmb:  when list->totype coercion otherwise fails, we try to coerce each item 
	in the list to totype. if we success, we'll return true even though the value we 
	return will still be a list, not a totype value. since this used to be a failure 
	case, it can't break working scripts. but it means that glue scripts can coerce to
	objspec, alias, or whatever and still allow a list of those items be pass through.
	*/
	
	short ix;
	OSErr errcode;
	
	for (ix = 1; ix <= ctitems; ++ix) {

		tyvaluerecord itemval;
		AEDesc itemdesc;
		OSType key;
		
		if (!getnthlistval (listdesc, ix, &key, &itemval))
			return (false);
		
		if (!coercevalue (&itemval, totype))
			return (false);
		
		if (!valuetodescriptor (&itemval, &itemdesc))	// steals handle
			return (false);
	
		errcode = AEPutDesc (listdesc, ix, &itemdesc);	// merge handle into list
		
		AEDisposeDesc (&itemdesc);
		
		if (oserror (errcode))
			return (false);
		}
	
	return (true);
	} /*coercelistcontents*/


boolean coercelistvalue (tyvaluerecord *val, tyvaluetype totype) {
	
	/*
	2.1b6 dmb: coercing a list to a boolean indicates whether or not the 
	list is empty, except when the list contains a single, boolean item.
	
	2.1b8 dmb: for a single-item list, try to coerce item to desired type, 
	instead of requiring the the type already match
	*/
	
	register tyvaluerecord *v = val;
	AEDesc listdesc;
	long ctitems;
	tyvaluerecord itemval;
	
	if (totype == (*v).valuetype)
		return (true);
	
	listvaltodesc (v, &listdesc);
	
	switch (totype) {
		
		case listvaluetype: {
			AEDesc desc;
			
			if (oserror (AECoerceDesc (&listdesc, typeAEList, &desc)))
				return (false);
			
			if (!setheapvalue (desc.dataHandle, totype, v))
				return (false);
			
			break;
			}
		
		case stringvaluetype:
			if (!listtostring (&listdesc, v))
				return (false);
			
			break;
		
		case binaryvaluetype:
			return (coercetobinary (v));
		
		case pointvaluetype: {
			Point pt;
			
			if (!pullintegerlist (&listdesc, 2, &pt))
				return (false);
			
			if (!setpointvalue (swappoint (pt), v))
				return (false);
			
			break;
			}
		
		case rgbvaluetype: {
			RGBColor rgb = **(*v).data.rgbvalue;
			
			if (!pullintegerlist (&listdesc, 3, &rgb))
				return (false);
			
			if (!newheapvalue (&rgb, sizeof (rgb), rgbvaluetype, v))
				return (false);
			
			break;
			}
		
		case rectvaluetype: {
			Rect r = **(*v).data.rectvalue;
			
			if (!pullintegerlist (&listdesc, 4, &r))
				return (false);
			
			if (!newheapvalue (&r, sizeof (r), rectvaluetype, v))
				return (false);
			
			break;
			}
		
		default:
			if (oserror (AECountItems (&listdesc, &ctitems)))
				return (false);
			
			if (ctitems == 1) {
				
				OSType key;
				
				if (!getnthlistval (&listdesc, 1, &key, &itemval))
					return (false);
				
				if (coercevalue (&itemval, totype)) {
					
					disposevaluerecord (*v, true);
					
					*v = itemval;
					
					return (true);
					}
				
				return (false);
				}
			
			if (totype == booleanvaluetype) {
				
				if (!setbooleanvalue (ctitems > 0, v))
					return (false);
				
				break;
				}
			
			/*
			langcoerceerror (v, totype);
			
			return (false);
			*/
			return (coercelistcontents (v, totype, &listdesc, ctitems));	// 4.0b7 dmb
		}
	
	releaseheaptmp (listdesc.dataHandle);
	
	return (true);
	} /*coercelistvalue*/


static boolean equalsublists (AEDesc *list1, AEDesc *list2, long ixcompare, long ctcompare, boolean flbykey) {
	
	/*
	compare the sublist of list1 starting at ixcompare with list2.  if flbykey, 
	order doesn't matter and ixcompare is ignored.  ctcompare is expected to be 
	the size of list2
	*/
	
	register AEDesc *d1 = list1;
	register AEDesc *d2 = list2;
	register long ix;
	register long n = ctcompare;
	AEDesc item1;
	AEDesc item2;
	OSErr err;
	OSType key1;
	OSType key2;
	boolean fl = true;
	
	for (ix = 1; ix <= n; ++ix) {
		
		if (oserror (AEGetNthDesc (d2, ix, typeWildCard, &key1, &item2)))
			return (false);
		
		if (flbykey) {
			
			err = AEGetKeyDesc (d1, key1, typeWildCard, &item1);
			
			key2 = key1;
			}
		else
			err = AEGetNthDesc (d1, ix + ixcompare, typeWildCard, &key2, &item1);
		
		fl = ((err == noErr) && (key1 == key2) && equaldescriptors (&item1, &item2));
		
		AEDisposeDesc (&item1);
		
		AEDisposeDesc (&item2);
		
		if (!fl)
			break;
		}
	
	return (fl);
	} /*equalsublists*/


boolean listaddvalue (tyvaluerecord *v1, tyvaluerecord *v2, tyvaluerecord *vreturned) {
	
	/*
	add the two list values. both lists should be of the same type, since langvalue 
	does the necessary coercion first. But if one is an AEList and the other an AERecord,
	the AE Manager will generate errors if appropriate.
	
	when adding records, only add fields from the 2nd record that don't already exist
	in the first
	
	2.1b8 dmb: initialize errcode to noErr, or random error results if adding 
	two records where the 1st item in record 2 is already in record 1
	*/
	
	AEDesc desc1;
	AEDesc desc2;
	AEDesc desc3;
	long i, n;
	OSErr errcode = noErr;
	OSType key;
	AEDesc itemdesc;
	
	listvaltodesc (v1, &desc1);
	
	listvaltodesc (v2, &desc2);
	
	if (oserror (AECountItems (&desc2, &n)))
		return (false);
	
	if (oserror (AEDuplicateDesc (&desc1, &desc3))) /*start with first list*/
		return (false);
	
	if (!setheapvalue (desc3.dataHandle, (*v1).valuetype, vreturned)) /*get it into temp stack now*/
		return (false);
	
	for (i = 1; i <= n; ++i) { /*copy values over from second list*/
		
		if (oserror (AEGetNthDesc (&desc2, i, typeWildCard, &key, &itemdesc)))
			return (false);
		
		if (desc3.descriptorType == typeAERecord) {
			
			DescType type;
			long size;
			
			if (AESizeOfKeyDesc (&desc3, key, &type, &size) == errAEDescNotFound)
				errcode = AEPutKeyDesc (&desc3, key, &itemdesc);
			}
		else
			errcode = AEPutDesc (&desc3, 0, &itemdesc);
		
		AEDisposeDesc (&itemdesc);
		
		if (oserror (errcode))
			return (false);
		}
	
	return (true);
	} /*listaddvalue*/


boolean listsubtractvalue (tyvaluerecord *v1, tyvaluerecord *v2, tyvaluerecord *vreturned) {
	
	/*
	add the two list values. both lists should be of the same type, since langvalue 
	does the necessary coercion first. But if one is an AEList and the other an AERecord,
	the AE Manager will generate errors if appropriate.
	
	when adding records, only add fields from the 2nd record that don't already exist
	in the first
	*/
	
	AEDesc desc1;
	AEDesc desc2;
	AEDesc desc3;
	long ix1, ix2, n1, n2;
	
	listvaltodesc (v1, &desc1);
	
	listvaltodesc (v2, &desc2);
	
	if (oserror (AECountItems (&desc1, &n1)))
		goto error;
	
	if (oserror (AECountItems (&desc2, &n2)))
		goto error;
	
	if (oserror (AEDuplicateDesc (&desc1, &desc3))) /*start with first list*/
		goto error;
	
	if (!setheapvalue (desc3.dataHandle, (*v1).valuetype, vreturned)) /*get it into temp stack now*/
		goto error;
	
	if (desc3.descriptorType == typeAERecord) {
		
		for (ix1 = 1; ix1 <= n2; ++ix1) { /*delete values that appear in second record*/
			
			OSType key;
			AEDesc item1, item2;
			
			if (oserror (AEGetNthDesc (&desc2, ix1, typeWildCard, &key, &item1)))
				goto error;
			
			if (AEGetKeyDesc (&desc3, key, typeWildCard, &item2) == noErr) {
				
				if (equaldescriptors (&item1, &item2))
					AEDeleteKeyDesc (&desc3, key); /*ignore result, may be descNotFound*/
				
				AEDisposeDesc (&item2);
				}
			
			AEDisposeDesc (&item1);
			}
		}
	else {
		
		ix2 = n1 - n2;
		
		for (ix1 = 0; ix1 <= ix2; ++ix1) {
			
			if (equalsublists (&desc1, &desc2, ix1, n2, false)) {
				
				while (--n2 >= 0)
					if (oserror (AEDeleteItem (&desc3, ix1 + 1)))
						goto error;
				
				break;
				}
			}
		}
	
	return (true);
	
	error:
	
	return (false);
	} /*listsubtractvalue*/


static boolean comparelists (AEDesc *desc1, AEDesc *desc2, tytreetype comparisonop) {
	
	/*
	compare the two lists, returning true if the comparison holds, false 
	if it doesn't or an error occurs
	*/
	
	register AEDesc *d1 = desc1;
	register AEDesc *d2 = desc2;
	long n1, n2;
	register long ix1, ix2;
	boolean flbykey;
	
	if (oserror (AECountItems (d1, &n1)))
		goto exit;
	
	if (oserror (AECountItems (d2, &n2)))
		goto exit;
	
	ix1 = 0;
	
	ix2 = n1 - n2;
	
	if (ix2 < 0) /*v1 can't beginwith, endwith, contain or be equal to v2*/
		goto exit;
	
	flbykey = (*d1).descriptorType == typeAERecord;
	
	switch (comparisonop) {
		
		case beginswithop:
			ix2 = 0;
			
			flbykey = false;
			
			break;
		
		case endswithop:
			ix1 = ix2;
			
			flbykey = false;
			
			break;
		
		case EQop:
			if (ix2 != 0) /*n2 != n1*/
				goto exit;
			
			break;
		
		case containsop:
			if (flbykey)
				ix2 = 0;
			
			break;
		}
	
	for (; ix1 <= ix2; ++ix1) {
		
		if (equalsublists (d1, d2, ix1, n2, flbykey))
			return (true);
		}
	
	exit:
	
	return (false);
	} /*comparelists*/


boolean listcomparevalue (tyvaluerecord *v1, tyvaluerecord *v2, tytreetype comparisonop, tyvaluerecord *vreturned) {
	
	AEDesc d1;
	AEDesc d2;
	boolean fl;
	
	listvaltodesc (v1, &d1);
	
	listvaltodesc (v2, &d2);
	
	fl = comparelists (&d1, &d2, comparisonop);
	
	if (fllangerror)
		return (false);
	
	return (setbooleanvalue (fl, vreturned));
	} /*listcomparevalue*/


boolean coercetolistposition (tyvaluerecord *val) {
	
	/*
	get a list position parameter -- either an index (number) or a keyword (string4)
	*/
	
	tyvaluerecord *v = val;
	boolean fl;
	
	switch ((*v).valuetype) {
		
		case longvaluetype:
		case ostypevaluetype:
			return (true);
		
		default:
			disablelangerror ();
			
			fl = coercetolong (v) || coercetoostype (v);
			
			enablelangerror ();
			
			if (fl)
				return (true);
		}
	
	langerror (badipclistposerror);
	
	return (false);
	} /*coercetolistposition*/


static boolean listerror (OSErr errcode, bigstring bsname, tyvaluerecord *vlist, tyvaluerecord *vindex) {
	
	switch (errcode) {
		
		case noErr:
			return (false);
		
		case errAEIllegalIndex:
		case errAEBadListItem:
		case errAEDescNotFound:
			langarrayreferror (0, bsname, vlist, vindex);
			
			return (true);
		
		default:
			oserror (errcode);
			
			return (true);
		}
	} /*listerror*/
	

boolean listarrayvalue (tyvaluerecord *vlist, bigstring bsname, register tyvaluerecord *vindex, tyvaluerecord *vreturned) {
	
	/*
	bsname is provided for error reporting only
	*/
	
	AEDesc listdesc;
	AEDesc itemdesc;
	OSType key;
	OSErr errcode;
	
	listvaltodesc (vlist, &listdesc);
	
	if (!coercetolistposition (vindex))
		return (false);
	
	if ((*vindex).valuetype == longvaluetype)
		errcode = AEGetNthDesc (&listdesc, (*vindex).data.longvalue, typeWildCard, &key, &itemdesc);
	else
		errcode = AEGetKeyDesc (&listdesc, (*vindex).data.ostypevalue, typeWildCard, &itemdesc);
	
	if (listerror (errcode, bsname, vlist, vindex))
		return (false);
	
	return (setdescriptorvalue (itemdesc, vreturned));
	} /*listarrayvalue*/


boolean listassignvalue (tyvaluerecord *vlist, bigstring bsname, register tyvaluerecord *vindex, tyvaluerecord *vassign) {
	
	/*
	bsname is provided for error reporting only
	*/
	
	AEDesc listdesc;
	AEDesc itemdesc;
	OSErr errcode;
	
	listvaltodesc (vlist, &listdesc);
	
	if (!coercetolistposition (vindex))
		return (false);
	
	if (!valuetodescriptor (vassign, &itemdesc))
		return (false);
	
	if ((*vindex).valuetype == longvaluetype)
		errcode = AEPutDesc (&listdesc, (*vindex).data.longvalue, &itemdesc);
	else
		errcode = AEPutKeyDesc (&listdesc, (*vindex).data.ostypevalue, &itemdesc);
	
	AEDisposeDesc (&itemdesc);
	
	if (listerror (errcode, bsname, vlist, vindex))
		return (false);
	
	return (true);
	} /*listassignvalue*/


boolean listdeletevalue (tyvaluerecord *vlist, bigstring bsname, register tyvaluerecord *vindex) {
	
	/*
	bsname is provided for error reporting only
	*/
	
	AEDesc listdesc;
	OSErr errcode;
	
	listvaltodesc (vlist, &listdesc);
	
	if (!coercetolistposition (vindex))
		return (false);
	
	if ((*vindex).valuetype == longvaluetype)
		errcode = AEDeleteItem (&listdesc, (*vindex).data.longvalue);
	else
		errcode = AEDeleteKeyDesc (&listdesc, (*vindex).data.ostypevalue);
	
	if (listerror (errcode, bsname, vlist, vindex))
		return (false);
	
	return (true );
	} /*listdeletevalue*/

#endif




