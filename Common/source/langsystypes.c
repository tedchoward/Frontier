
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

#include "memory.h"
#include "strings.h"
#include "ops.h"
#include "error.h"
#include "file.h"
#include "lang.h"
#include "langinternal.h"
#include "langipc.h"
#include "langsystem7.h"
#include "oplist.h"
#include "tableinternal.h" /*for hdltablevariable; so we can avoid loading unloaded tables*/
#include "tablestructure.h"


	#include "aeutils.h"

	static byte bsellipses [] = "\x01É";

static byte filerecordpath [] = "\x04" "path";

static byte filerecordalias [] = "\x05" "alias";

static byte filerecordmachine [] = "\x07" "machine";

static byte filerecordplatform [] = "\x04" "plat";

static byte bsplatformmac [] = "\x03" "mac";

static byte bsplatformwin [] = "\x03" "win";



static boolean langgestaltcheck (OSType selector, short stringnum) {
	
	long result;
	
	if (!gestalt (selector, &result) || (result == 0)) {
		
		langerror (stringnum);
		
		return (false);
		}
	
	return (true);
	} /*langgestaltcheck*/


static boolean equaldescriptors (AEDesc *desc1, AEDesc *desc2) {
	
	register AEDesc *d1 = desc1;
	register AEDesc *d2 = desc2;
	register DescType t1;
	
	if (desc1 == nil) /*7.1b24 PBS: sometimes it's nil, and that's okay, but crashes are bad.*/
		return (desc1 == desc2);
	
	t1 = (*d1).descriptorType;
	
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
	
		
			{
			Handle hcopy1, hcopy2;
			boolean fl = false;
			
			copydatahandle (d1, &hcopy1);
			
			copydatahandle (d2, &hcopy2);
			
			fl = equalhandles (hcopy1, hcopy2);
			
			disposehandle (hcopy1);
			
			disposehandle (hcopy2);
			
			return (fl);
			}
				
	} /*equaldescriptors*/



boolean langcanusealiases (void) {
	
	
		return (langgestaltcheck (gestaltAliasMgrAttr, cantusealiaseserror));
	
	} /*langcanusealiases*/



static boolean findvaluevisit (bigstring bs, hdlhashnode hnode, tyvaluerecord val, ptrvoid valfind) {
#pragma unused (bs, hnode)

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


static boolean getostypevalnamevisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
#pragma unused (hnode)

	/*
	3.0.2b1 dmb: we now look in all loaded app tables for a match when 
	converting an terminology value (a string4 value) its name
	*/
	
	tyvaluerecord *vallookfor = (tyvaluerecord *) refcon;
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
	
	5.0.2b16 dmb: don't return "id" as a string4 displaystring
	*/
	
	tyvaluerecord val;
	hdlhashtable htable;
	byte bskey [16];
	boolean fl;
	
	setostypevalue (key, &val);
	
	if (langfindvalue (val, &htable, bsdisplay))
		goto exit;
	
	if (objectmodeltable != nil) {
		
		pushhashtable (objectmodeltable);
		
		fl = langfindvalue (val, &htable, bsdisplay);
		
		pophashtable ();
		
		if (fl)
			goto exit;
		}
	
	if (iacgluetable != nil) {
		
		//vallookfor = val;
		
		if (hashinversesearch (iacgluetable, &getostypevalnamevisit, &val, bsdisplay))
			goto exit;
		}
	
	exit:
	
	if (!isemptystring (bsdisplay) && !equalstrings (bsdisplay, BIGSTRING ("\x02" "id")))
		return (true);
	
	ostypetostring (key, bskey);
	
	langdeparsestring (bskey, chsinglequote); /*add needed escape sequences*/
	
	parsedialogstring (BIGSTRING ("\x04'^0'"), bskey, nil, nil, nil, bsdisplay);
	
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
			hashgetvaluestring (*v, bsvalue); /%go with hex string%/
			
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
	
	if (chquote != chnul)
		langdeparsestring (bsvalue, chquote); /*add needed escape sequences*/
	
	if (stringlength (bsvalue) > limit) {
		
		setstringlength (bsvalue, limit - stringlength (bsellipses));
		
		pushstring (bsellipses, bsvalue);
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
	
	5.0b16 dmb: leave addresses as addresses, not strings
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
			if (!getlimitedvaluestring (v, 251, chdoublequote, bsdisplay))
				return (false);
			
			break;
		
		case addressvaluetype:
			if (!getlimitedvaluestring (v, 251, chnul, bsdisplay))
				return (false);
			
			if (isemptystring (bsdisplay))
				langgetmiscstring (justnilstring, bsdisplay);
			else
				insertchar ('@', bsdisplay);
			
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
				copystring (BIGSTRING ("\x02\"\""), bsdisplay);
			
			break;
			
		default:
			if (!getlimitedvaluestring (v, 253, chnul, bsdisplay))
				return (false);
			
			break;
		}
	
	return (true);
	} /*getobjectmodeldisplaystring*/



	static boolean stringtoalias ( tyvaluerecord *val ) {
	
		//
		// 2007-06-11 creedon: fileexists wasn't working, need to extend the
		//				   filespec, if successful then file exists
		//
		// 2006-06-24 creedon: FSRef-ized
		//
		// 2.1b2 dmb: try converting to a filespec first to ensure that
		//		    partial path or drive number if processed properly.
		//		    also, in the filespec case, the alias isn't minimal
		//
		// 1992-07-23 dmb: OK, try to getfullfilepath, but with errors
		//			    disabled
		//
		// 1992-07-02 dmb: don't call getfullfilepath; makes it impossible to
		//			    create aliases of not-yet-existing files, or
		//			    offline volumes
		//
		// 1991-10-07 dmb: make sure we're actually passing a full path to the
		//			    NewAlias routine
		//
		
		register Handle htext;
		FSRef fsref;
		AliasHandle halias;
		OSErr errcode;
		bigstring bspath;
		tyfilespec fs;
		
		if ( ! langcanusealiases ( ) )
			return ( false );
		
		htext = ( *val ).data.stringvalue;
		
		texthandletostring ( htext, bspath );
		
		if ( pathtofilespec ( bspath, &fs ) && (macgetfsref (&fs, &fsref) == noErr) )
			errcode = FSNewAlias ( NULL, &fsref, &halias );
		else
			errcode = NewAliasMinimalFromFullPath ( stringlength (bspath), stringbaseaddress (bspath), NULL, NULL, &halias );
		
		if ( oserror ( errcode ) )
			return ( false );
		
		if ( ! setheapvalue ( ( Handle ) halias, aliasvaluetype, val ) )
			return (false);
		
		releaseheaptmp ( ( Handle ) htext );
		
		return ( true );
		
		} // stringtoalias
		


boolean filespectoalias (const ptrfilespec fs, boolean flminimal, AliasHandle *halias) {

	//
	// 2006-10-26 creedon: on Mac, check that fs.fsref is valid
	//
	// 2006-06-24 creedon: for Mac, FSRef-ized
	//
	

		OSErr err = noErr;
		
		if (!macfilespecisvalid (fs))
			return (false);
		
		if (fs->flags.flvolume) {

			if (flminimal)
				err = FSNewAliasMinimal (&fs->ref, halias);
			else
				err = FSNewAlias (nil, &fs->ref, halias);
			}
		else {

			if (flminimal)
				err = FSNewAliasMinimalUnicode ( &fs->ref, fs->name.length, fs->name.unicode, halias, NULL );
			else
				err = FSNewAliasUnicode (nil, &fs->ref, fs->name.length, fs->name.unicode, halias, NULL);
			}

		if ((err == noErr) || (err == fnfErr))
			return (true);
		
		if (langerrorenabled ()) {
			
			setfserrorparam (fs);
			
			oserror (err);
			}
		
		return (false);
		

	
	} // filespectoalias



	static boolean filespecvaltoalias (tyvaluerecord *val) {

		//
		// 2006-06-24 creedon: FSRef-ized
		//
		
		register hdlfilespec hfs;
		tyfilespec fs;
		AliasHandle halias;
		
		if (!langcanusealiases ())
			return (false);
		
		hfs = (hdlfilespec) (*val).data.filespecvalue;
		
		fs = **hfs;
		
		if (!filespectoalias (&fs, false, &halias))
			return (false);
		
		if (!setheapvalue ((Handle) halias, aliasvaluetype, val))
			return (false);
		
		releaseheaptmp ((Handle) hfs);
		
		return (true);
		
		} // filespecvaltoalias



boolean aliastostring (Handle halias, bigstring bs) {

		
		//
		// 2006-07-26 creedon: FSRef-ized
		//
		// 1996-04-26 dmb: restored FollowFinderAlias code; must use if we get fnfErr.
		//
		// 2.1b9 dmb: use FollowFinderAlias to avoid mounting volumes during alias resolution
		//
		// 1993-04-12 dmb: accept fnfErr result from ResolveAlias
		//
		// 1991-10-04 dmb: if alias can't be resolved, just say what volume it's on.
		//
		
		register AliasHandle h = (AliasHandle) halias;
		FSRef fsref;
		Boolean flchanged;
		bigstring bsinfo;
		AliasInfoType ix = asiAliasName;
		OSErr err;
		
		if (!langcanusealiases ())
			return (false);
		
		err = FSFollowFinderAlias (nil, h, false, &fsref, &flchanged);
		
		if ((err == noErr) /* || (err == fnfErr) */ ) {
		
			tyfilespec fs;
			
			if (flchanged)
				FSUpdateAlias (nil, &fsref, h, &flchanged);
			
			if (macmakefilespec (&fsref, &fs) == noErr)
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
		

	
	} // aliastostring


boolean aliastofilespec ( AliasHandle halias, ptrfilespec fs ) {
	
	//
	// 2006-07-29 creedon: for Mac FSRef-ized
	//
	// 7.0b34 PBS: Can't use ResolveAliasWithMountFlags on pre-Carbon Macs reliably, so fall back to FollowFinderAlias.
	//
	// 2.1b9 dmb: use FollowFinderAlias to avoid mounting volumes during alias resolution
	//
	// 2.1b2 dmb: on error, try to get as much info from the alias as possible, & just return false to caller
	//
	// 2.1a6 dmb: ignore fnfErr
	//
	
	
		FSRef fsref;
		boolean flchanged;
		bigstring bs;
		OSErr err;
		OSStatus status;
		
		if ( ! langcanusealiases ( ) )
			return ( false );
			
		err = FSResolveAliasWithMountFlags ( NULL, halias, &fsref, &flchanged, kARMNoUI );

		if ( ( err == noErr ) || ( err == fnfErr ) ) {
		
			return ( macmakefilespec (&fsref, fs) == noErr );
			}
		
		FSAliasInfoBitmap whichAliasInfo = kFSAliasInfoNone;
		tyfsname volumeName;
		
		status = FSCopyAliasInfo ( halias, NULL, &volumeName, NULL, &whichAliasInfo, NULL );
		
		if ( status == noErr ) { // try to get vol info
		
			fsnametobigstring ( &volumeName, bs );

			fileparsevolname ( bs, fs );
			}
		
		status = FSCopyAliasInfo ( halias, &fs->name, NULL, NULL, &whichAliasInfo, NULL );
		
		if ( status != noErr ) {
		
			langgetmiscstring ( unknownstring, bs );
			
			bigstringtofsname ( bs, &fs->name );
			}
		
		if ( langerrorenabled ( ) ) {
			
			setoserrorparam ( bs );
			
			oserror ( err );
			
			}
		
		return ( false );
		

	
	} // aliastofilespec


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


boolean filespecaddvalue ( tyvaluerecord *v1, tyvaluerecord *v2, tyvaluerecord *vreturned ) {

	//
	// add v2 to the filespec v1 by using it as a partial path. if anything but a valid 
	// fspec results, return a string value that is simple concatenation
	//
	// 2006-10-29 creedon: for Mac, if bs ends with ':', pop leading ':' from bsadd
	//
	// 2006-10-16 creedon: for Mac, FSRef-ized
	//
	// 2.1b6 dmb: if resulting specifier exists, but doesn't agree with bsadd as far as 
	// whether or not it's a folder, return a string.
	//
	// 2.1b3 dmb: if resulting path is to a non-existent folder, don't return a filespec 
	//
	

		FSRef fsref;
		bigstring bs, bsadd;
		tyfilespec fs;
			
		fs = **(*v1).data.filespecvalue;
		
		filespectopath (&fs, bs);
		
		if (!coercetostring (v2))
			return (false);
		
		pullstringvalue (v2, bsadd);
		
		if (lastchar (bs) == ':')
			popleadingchars (bsadd, ':');
		
		pushstring (bsadd, bs);
				
		if (pathtofsref (bs, &fsref) != noErr) {	// fall back to string addition

			if (!coercetostring (v1))
				return (false);
			
			return (addvalue (*v1, *v2, vreturned));
			}
		
		(void) macmakefilespec (&fsref, &fs);

		return (setfilespecvalue (&fs, vreturned));
		

	
	} // filespecaddvalue


boolean filespecsubtractvalue (tyvaluerecord *v1, tyvaluerecord *v2, tyvaluerecord *vreturned) {

	//
	// subtract v2 from the filespec v1's file name, if v2 is a simple string (no 
	// colons). otherwise, return a string value that is simple string subtraction
	//
	// 2006-06-24 creedon: FSRef-ized
	//
	

		tyfilespec fs;
		bigstring bssub;
		
		if (!coercetostring (v2))
			return (false);
		
		fs = **(*v1).data.filespecvalue;	// copy
		
		pullstringvalue (v2, bssub);
		
		if (!fs.flags.flvolume && !stringfindchar (':', bssub)) {	// it's a simple string, just subtract from file name
		
			bigstring bsname;
			
			getfsfile (&fs, bsname);
			
			subtractstrings (bsname, bssub, bsname);
			
			if (stringlength (bsname) > 0) {
				bigstringtofsname (bsname, &fs.name);
				}
			else {
				macgetfilespecparent (&fs, &fs);
				}

			return (setfilespecvalue (&fs, vreturned));
			}
		
		if (!coercetostring (v1))
			return (false);
		
		return (subtractvalue (*v1, *v2, vreturned));
		

	
	} // filespecsubtractvalue


boolean langpackfileval (const tyvaluerecord *vfile, Handle *hpacked) {
	
	//
	// we create a record containing an alias to the file (Mac only), 
	// the full path to the file (as a URL), and the machine name (for
	// future use).
	//
	// 2006-06-24 creedon: FSRef-ized
	//
	// 5.0a24 dmb: new format for storing filespec and alias values in the odb.
	//
	
	hdllistrecord hlist;
	bigstring bspath, bsmachine;
	tyfilespec fs;
	Handle halias = nil;
	boolean fl = false;
	
	if (!opnewlist (&hlist, true))
		return (false);
	
	switch ((*vfile).valuetype) {
		
		case aliasvaluetype:
			if (!copyhandle ((*vfile).data.aliasvalue, &halias))
				goto exit;
			
			aliastostring (halias, bspath);
			
			break;
		
		case filespecvaluetype: {
		
			fs = **(*vfile).data.filespecvalue;
			
			disablelangerror ();
			
			if ( ! filespectoalias ( &fs, true, ( AliasHandle * ) &halias ) )
				halias = nil;

			enablelangerror ();
			
			fl = filespectopath (&fs, bspath);
			
			break;
			}
		
		default:
			assert (false);

			goto exit;
		}
	
	// *** should convert path to URL ***
	
	if (!oppushstring (hlist, filerecordpath, bspath))
		goto exit;
	
	if (halias != nil)
		if (!oppushhandle (hlist, filerecordalias, halias))
			goto exit;
	
	//5.0a24 rab: adding platform indicator rather then converting to URL - URL's have as much difference as
	//just indicating what platform the path is specific to.

	
		if (!oppushstring (hlist, filerecordplatform, bsplatformmac))
			goto exit;
			


	if (getmachinename (bsmachine)) // *** could push machine name too
		if (!oppushstring (hlist, filerecordmachine, bsmachine))
			goto exit;
	
	fl = oppacklist (hlist, hpacked);
	
  exit:
	
	opdisposelist (hlist);
	
	return (fl);
	
	} // langpackfileval


boolean langunpackfileval (Handle hpacked, tyvaluerecord *vfile) {
	
	//
	// 2006-07-01 creedon: clearbytes of fs first thing
	//
	// 5.0fc2 dmb: unpacking an alias on the pc, go straight for the path string
	//

	hdllistrecord hlist;
	Handle halias;
	bigstring bspath, bsplat;
	tyfilespec fs;
	boolean fl = false;
	
	clearbytes ( &fs, sizeof ( fs ) );

	if (!opunpacklist (hpacked, &hlist))
		return (false);
	
	switch ((*vfile).valuetype) {
		
		case aliasvaluetype:
		
			
				fl = opgetlisthandle (hlist, -1, filerecordalias, &halias);

				if (fl)
					fl = copyhandle (halias, &(*vfile).data.aliasvalue);
					

			
			break;

		case filespecvaluetype:
			fl = opgetlisthandle (hlist, -1, filerecordalias, &halias);

			if (fl) {
				
				disablelangerror ();
				
				fl = aliastofilespec ((AliasHandle) halias, &fs);
				
				enablelangerror ();
				}

			if (!fl) {
				
				fl = opgetliststring (hlist, -1, filerecordpath, bspath);
				
				if (fl) {
				
					// here we should check to see if the file path is from the correct platform???
					
					if (opgetliststring (hlist, -1, filerecordplatform, bsplat)) {
					
						
							if (equalidentifiers (bsplat, bsplatformwin)) {
								// ERROR or Conversion HERE!
								}
								

						}
					
					fl = pathtofilespec (bspath, &fs);
					
					}
				}
			
			if (fl)
				fl = newfilledhandle (&fs, filespecsize (fs), (Handle *) &(*vfile).data.filespecvalue);
			else {
			
				// *** once we make tyfilespec the new record on both platforms, this will be ok
				// *** but for now, we need to make a string value
				
				(*vfile).valuetype = stringvaluetype;

				fl = newtexthandle (bspath, &(*vfile).data.stringvalue);
				}

			break;
		
		default:
		
			assert (false);
			
			break;
		}
	
	opdisposelist (hlist);

	return (fl);
	
	} // langunpackfileval




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
			
			
				datahandletostring (keydesc, bs);
				
			
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
	
	
		DescType type = typeObjectSpecifier;
		
		if (hdata == nil)
			type = typeNull;
		
		newdescwithhandle (desc, type, hdata);
	
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
	
	
		{
		Handle h;
		
		copydatahandle (&token, &h);
		
		if (!setheapvalue (h, stringvaluetype, v))
			return (false);
		}
	
	
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
	
	
	
		{
		Handle h;
		
		copydatahandle (&objdesc, &h);
		
		return (setheapvalue (h, objspecvaluetype, v));
		}
	
	
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


#pragma pack(2)
typedef struct tyobjspecitem { /*data within special object specifier structures*/
	
	AEKeyword key;
	
	DescType type;
	
	long size;
	
	/*data follows*/
	} tyobjspecitem;
#pragma options align=reset


static boolean getobjspeckeydesc (AEDesc *objdata, OSType desiredkey, AEDesc *keydata) {

    // MJL 17/08/05: As of Jaguar the AEDesc datahandle is opaque so use toolbox accessor functions rather
    //  than trying to parse the data structure.
	AEKeyword		curKeyWord;
	OSErr 			err;
	SInt32			i, numItems;
    
	err = AECountItems((AEDescList *)objdata, &numItems);
	if (err != noErr)
		goto exit;
    
	for (i = 1; i <= numItems; i++) {
		err = AEGetNthDesc((AEDescList *)objdata, i, typeWildCard, &curKeyWord, keydata);
        
		if (err != noErr)
			goto exit;
		
		if (curKeyWord == desiredkey) {
			goto exit;
		}
		AEDisposeDesc(keydata);
	}
	
	err = errAEDescNotFound;
	
    exit:
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
		
		default:
			p = "\p";
			assert(false);	/* 2006-02-08 aradke: this should never happen, but if it does we set p to the empty string */
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
			
			
				{
				Handle hcopy;
				
				copydatahandle (&desc, &hcopy);
				
				op = **(OSType **)hcopy;
				
				disposehandle (hcopy);
				}

			
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
			
			
			
				{
				Handle hcopy;
				
				copydatahandle (&desc, &hcopy);
				
				op = **(OSType **)hcopy;
				
				disposehandle (hcopy);
				}


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
    AEDesc tempDesc, *tempDescPtr;  // MJL 17/08/05: Temp desc and ptr so we can compile for both 9 & X
	
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
		
        // MJL 17/08/05: On X we cannot keep a lazy handle in the AEDesc to the keydata datahandle that gets 
        //  pushed onto the Frontier temp stack in the vkey. As we need it later we must duplicate the AEDesc before pushing it.
        tempDescPtr = &tempDesc;
        AEDuplicateDesc (&keydata, tempDescPtr);
		if (!setdescriptorvalue (*tempDescPtr, &vkey)) /*if successful, keydata is on temp stack*/
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
		
        AEDisposeDesc (&keydata);   // MJL 17/08/05 dispose of our duplicated temp AEDesc. On 9 the dataHandle belongs also to the vkey which is disposed above so we don't need to explicitly dispose the AEDesc
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
	
		
		newdescnull (containerdesc, nulltype);
	
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
	
	
		if (x == nil)
		
			newdescwithhandle (obj, typeNull, x);
		
		else {
		
			exemptfromtmpstack (v);
			
			newdescwithhandle (obj, typeObjectSpecifier, x);
			} /*else*/
	
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
			
			
				{
				Handle h;
				
				copydatahandle (&objectdesc, &h);
				
				return (setheapvalue (h, objspecvaluetype, v));
				}			
			
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


		{
		Handle h;
		
		copydatahandle (&objspecdesc, &h);
		
		return (setheapvalue (h, objspecvaluetype, val));
		}			

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
	
	
		Handle hcopy;
		
	
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
			
			
				copydatahandle (&containerdesc, &hcopy);
				
				pushtmpstack (hcopy);
						
			
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
	
		
			releaseheaptmp (hcopy);

		
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
	
	
		Handle hcopy;
	
	
	if (!evaluateobject ((**h).param1, typeObjectBeingExamined, &objectdesc))
		return (false);
	
	
		copydatahandle (&objectdesc, &hcopy);
		
		pushtmpstack (hcopy);
	
	
	if (!evaluatetree ((**h).param2, &val))
		return (false);
	
	if (!valuetodescriptor (&val, &valdesc))
		return (false);
	
	errcode = CreateCompDescriptor (operator, &objectdesc, &valdesc, false, keydatadesc);
	

		releaseheaptmp (hcopy);
	
	
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
		
	
		nildatahandle (&containerdesc);
	

	containerdesc.descriptorType = typeCurrentContainer;
	
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
	
	
		Handle hcopy;
	
	
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
			
			
				copydatahandle (&containerdesc, &hcopy);
				
				pushtmpstack (hcopy);
			
			
			fltmp = true;
			
			if (!getclassvalue ((**hp1).param2, &class))
				return (false);
			
			break;
		
		case arrayop:
			if (!evaluateelement (hp1, nulltype, &class, &containerdesc))
				return (false);

			
				copydatahandle (&containerdesc, &hcopy);
				
				pushtmpstack (hcopy);
			
			
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
			releaseheaptmp (hcopy);
	
	
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
	
	//langseterrorline (h); /*set globals for error reporting*/
	
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
	
	
		{
		Handle h;
		
		copydatahandle (&objectdesc, &h);
		
		return (setheapvalue (h, objspecvaluetype, vreturned));
		}
	
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


