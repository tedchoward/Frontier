
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
#include "ops.h"
#include "resources.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langsystem7.h"
#include "process.h"
#include "kernelverbs.h"
#include "file.h"
#include "filealias.h"
#include "tablestructure.h"
#include "kernelverbdefs.h"
#include "shell.rsrc.h"
#include "byteorder.h"		/* 2006-04-16 aradke: for SWAP_REZ_BYTE_ORDER */
#include "oplist.h"			/* 2006-04-23 creedon */


#ifdef MACVERSION

	#include "MoreFilesX.h"
	#define chpathseparator ':'
	
#endif

#ifdef WIN95VERSION
	#define chpathseparator '\\'
#endif

boolean flsupportslargevolumes = false; /*true if volumes over 2 GB are supported by the OS*/


#ifdef WIN95VERSION

tyGetDiskFreeSpaceEx adrGetDiskFreeSpaceEx = NULL;

#endif


typedef enum tyfiletoken { /*verbs that are processed by file.c*/
	
	filecreatedfunc, 
	
	filemodifiedfunc, 
	
	filetypefunc, 
	
	filecreatorfunc,
	
	setfilecreatedfunc, 
	
	setfilemodifiedfunc, 
	
	setfiletypefunc,
	
	setfilecreatorfunc,
	
	fileisfolderfunc,
	
	fileisvolumefunc,
	
	fileislockedfunc, 
	
	filelockfunc,
	
	fileunlockfunc,
	
	filecopyfunc, 
	
	filecopydataforkfunc,
	
	filecopyresourceforkfunc,
	
	filedeletefunc,
	
	filerenamefunc,
	
	fileexistsfunc,
	
	filesizefunc,
	
	filefullpathfunc,

	filegetpathfunc,
	
	filesetpathfunc,
	
	filefrompathfunc, 
	
	folderfrompathfunc, 
	
	getsystempathfunc,
	
	getspecialpathfunc,
	
	newfunc,
	
	newfolderfunc,
	
	newaliasfunc,
	
	sfgetfilefunc, 
	
	sfputfilefunc,
	
	sfgetfolderfunc,
	
	sfgetdiskfunc,
	
	filegeticonposfunc,
	
	fileseticonposfunc,
	
	getshortversionfunc,
	
	setshortversionfunc,
	
	getlongversionfunc,
	
	setlongversionfunc,
	
	filegetcommentfunc,
	
	filesetcommentfunc,
	
	filegetlabelfunc,
	
	filesetlabelfunc,
	
	filefindappfunc,
	
	/*
	fileeditlinefeedsfunc,
	*/
	
	fileisbusyfunc,
	
	filehasbundlefunc,
	
	filesetbundlefunc,
	
	fileisaliasfunc,
	
	fileisvisiblefunc,
	
	filesetvisiblefunc,
	
	filefollowaliasfunc,
	
	filemovefunc,
	
	/*
	filesinfolderfunc,
	*/
	
	volumeejectfunc,
	
	volumeisejectablefunc, 
	
	volumefreespacefunc,
	
	volumesizefunc,
	
	volumeblocksizefunc,
	
	filesonvolumefunc,
	
	foldersonvolumefunc,
	
	unmountvolumefunc,
	
	mountservervolumefunc,
	
	/*
	filelaunchfunc,
	*/
	
	/*start of new verbs added by DW, 7/27/91*/
	
		findinfilefunc,
		
		countlinesfunc,
		
		openfilefunc,
		
		closefilefunc,
		
		endoffilefunc,
		
		setendoffilefunc,
		
		getendoffilefunc,

		setpositionfunc,

		getpositionfunc,

		readlinefunc,
		
		writelinefunc,
		
		readfunc,
		
		writefunc,
		
		comparefunc,
	
	// end of new verbs added by DW, 7/27/91
	
	writewholefilefunc,
		
	getpathcharfunc,

	volumefreespacedoublefunc,

	volumesizedoublefunc,
	
	getmp3infofunc,
	
	readwholefilefunc,			// 2006-04-11 aradke
	
	getlabelindexfunc,			// 2006-04-23 creedon
	
	setlabelindexfunc,			// 2006-04-23 creedon
	
	getlabelnamesfunc,			// 2006-04-23 creedon
	
	getposixpathfunc,			// 2006-10-07 creedon
	
	ctfileverbs
	
	} tyfiletoken;


typedef enum tyreztoken {
	
	rezgetresourcefunc,
	
	rezputresourcefunc,
	
	rezgetnamedresourcefunc,
	
	rezputnamedresourcefunc,
	
	rezcountrestypesfunc,
	
	rezgetnthrestypefunc,
	
	rezcountresourcesfunc,
	
	rezgetnthresourcefunc,
	
	rezgetnthresinfofunc,
	
	rezresourceexistsfunc,
	
	reznamedresourceexistsfunc,
	
	rezdeleteresourcefunc,
	
	rezdeletenamedresourcefunc,
	
	rezgetresourceattrsfunc,
	
	rezsetresourceattrsfunc,
	
	ctrezverbs
	} tyreztoken;



#if 0

static bigstring bsdefaultpath; /*see setpath*/


static checkfordrivenum (bigstring bspath) {
	
	/*
	interpret single-digit vol name as driver number
	*/
	
	if ((stringlength (bspath) > 1) && (bspath [2] == chpathseparator)) {
		
		byte ch = bspath [1];
		short drivenum;
		byte bsvol [64];
		
		if (isnumeric (ch)) {
			
			drivenum = ch - '0';
			
			if (filegetvolumename(drivenum, bsvol))
				replacestring (bspath, 1, 1, bsvol);
			}
		}
	} /*checkfordrivenum*/


filecheckdefaultpath (bigstring bspath) {
	
	/*
	if bspath is a partial path, add our local current path, if set.
	
	12/5/91 dmb: sneak in support drive number as "n:" here
	
	12/27/91 dmb: if bsdefaultpath isn't empty, and we have a partial path, 
	make sure we don't double-up on colons if bspath starts with one.
	*/
	
	short ix = 1;
	
	if (!isemptystring (bspath)) {
		
		if (!scanstring (chpathseparator, bspath, &ix) || (ix == 1)) { /*a partial path*/
			
			if (!isemptystring (bsdefaultpath)) {
				
				if (bspath [1] == chpathseparator) /*default path ends in one already*/
					deletefirstchar (bspath);
				
				insertstring (bsdefaultpath, bspath);
				}
			}
		
		/*
		else
			checkfordrivenum (bspath);
		*/
		}
	} /*filecheckdefaultpath*/

#endif

static boolean getpathvalue (hdltreenode hparam1, short pnum, ptrfilespec fspath) {
	
	/*
	get a path parameter for the parameter list.
	
	2.1b2 dmb: now that we use filespecs everywhere, we don't have much to do!
	*/
	
	return (getfilespecvalue (hparam1, pnum, fspath));
	
	/*
	tyvaluerecord v;
	bigstring bspath;
	
	if (!getparamvalue (hparam1, pnum, &v))
		return (false);
	
	switch (v.valuetype) {
		
		case stringvaluetype:
			
			pullstringvalue (&v, bspath);
			
			filecheckdefaultpath (bspath);
			
			if (!pathtofilespec (bspath, fspath)) {
				
				filenotfounderror (bspath);
				
				return (false);
				}
			
			break;
		
		default:
			if (!coercetofilespec (&v))
				return (false);
			
			*fspath = **(*v).data.filespecvalue;
			
			break;
	
		return (true);
		}
	*/
	
	return (true);
	} /*getpathvalue*/


static boolean getvolumevalue (hdltreenode hparam1, short pnum, ptrfilespec fsvol) {
	
	//
	// get a volume path parameter for the parameter list.
	//
	// make sure that a colon is included so a volume name isn't interpreted as
	// a partial path
	//
	// 2.1b11 dmb:	ooops, we were copying bsvol into fsvol.name, potentially 
	//				overflowing the str64. fileparsevolname now returns the vol
	//				name.  note that if the caller needs to distinguish between
	//				volumes and non-volumes, it can't call us.
	//
	// 2.1b8 dmb:	don't use pathtofilespec to convert the volume name, because
	//				FSMakeFSSpec will prompt the user to insert the disk if the
	//				volume has been ejected
	
	
	bigstring bsvol;
	tyvaluerecord v;
	
	if (!getparamvalue (hparam1, pnum, &v))
		return (false);
	
	switch (v.valuetype) {
		
		case stringvaluetype: // already a string, easy case
			
			pullstringvalue (&v, bsvol);
			
			if ( ! fileparsevolname ( bsvol, fsvol ) ) {
				
				setoserrorparam (bsvol);
				
				oserror (errorVolume);
				
				return (false);
				}
			
			break;
		
		default:
			if (!coercetofilespec (&v))
				return (false);
			
			*fsvol = **v.data.filespecvalue;
			
			break;
		
		return (true);
		}
	
	return (true);
	} // getvolumevalue


static boolean copyfileverb (boolean fldata, boolean flresources, hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: for Mac, FSRef-ize
	//
	
	tyfilespec fs1, fs2;
	
	if (!getpathvalue (hparam1, 1, &fs1)) // fs1 holds the source path
		return (false);
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 2, &fs2)) // fs2 holds the dest path
		return (false);
	
	if (equalfilespecs (&fs1, &fs2)) {
		
		(*v).data.flvalue = true;	// easy case is making a copy of itself
		}
	else {
			
		(*v).data.flvalue = copyfile ( &fs1, &fs2, fldata, flresources );
		}
	
	return (true);
	
	} // copyfileverb


static boolean filefrompathverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	//
	// 2006-06-19 creedon: for Mac, FSRef-ized
	//
	// 2.1b3 dmb: be sure to add colon to name of folder even when filespec is being used.
	//
	// 2.1b2 dmb:	do string manipulation if given a string, but otherwise work with filespecs. less critical than with
	//			folderfrompath, but might avoid full path string overflow
	//
	
	tyvaluerecord v;
	tyfilespec fs;
	bigstring bs;
	boolean flfolder;
	
	flnextparamislast = true;
	
	if (!getparamvalue (hparam1, 1, &v))
		return (false);
	
	switch (v.valuetype) {
	
		case stringvaluetype:
			pullstringvalue (&v, bs);

			flfolder = endswithpathsep(bs);

			if (flfolder)
				setstringlength (bs, stringlength (bs) - 1);
			
			filefrompath (bs, bs); // bs now holds the filename
			
			break;
		
		default:
			if ( ! coercetofilespec ( &v ) )
				return ( false );
			
			fs = **v.data.filespecvalue;

			#ifdef MACVERSION
				
				macgetfilespecnameasbigstring(&fs, bs);

			#endif
			
			#ifdef WIN95VERSION
			
				copystring (fs.fullSpecifier, bs);
				
			#endif
			
			fileexists (&fs, &flfolder); // don't care about return, just flfolder value
			
			#ifdef WIN95VERSION
			
				if (endswithpathsep (bs))
					setstringlength (bs, stringlength (bs) - 1);

				filefrompath (bs, bs);
						
			#endif
			
			break;
		}

	if (flfolder)
		pushchar (chpathseparator, bs);
	
	return (setstringvalue (bs, vreturned));
	} // filefrompathverb


static boolean folderfrompathverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	//
	// 2006-08-24 creedon: for Mac, FSRef-ized
	//
	// 2.1b2 dmb:	do string manipulation if given a string, but otherwise work with filespecs. in addition to avoiding string
	//			overflow, this preserves ability to distinguish between identically-named volumes
	//
	
	tyvaluerecord v;
	bigstring bs;
	
	flnextparamislast = true;
	
	if (!getparamvalue (hparam1, 1, &v))
		return (false);
	
	#ifdef MACVERSION
	
		if (v.valuetype != stringvaluetype) {
		
			tyfilespec fs, fsparent;
			OSErr err;
			
			if (!coercetofilespec(&v))
				return (false);
			
			fs = **v.data.filespecvalue;
			
			err = macgetfilespecparent(&fs, &fsparent);
			
			if (err != noErr) {
				oserror(err);
				return (false);
				}
				
			return (setfilespecvalue(&fsparent,vreturned));
			}
	
	#endif
	
	if (!coercetostring (&v))
		return (false);

	pullstringvalue (&v, bs);
	
	cleanendoffilename (bs);
	
	folderfrompath (bs, bs); /*bs now holds the foldername*/
	
	return (setstringvalue (bs, vreturned));
	} // folderfrompathverb


static boolean gettypelistvalue (hdltreenode hparam1, short pnum, tysftypelist *filetypes, ptrsftypelist *x) {
#pragma unused(pnum)
	/*
	2.1b4 dmb: new feature: accept a list of filetype for sfgetfile
	*/
	
	tyvaluerecord val;
	tyvaluerecord vitem;
	long ctitems;
	short i;
//	OSType toss;
	OSType filetype;
	
	if (!getparamvalue (hparam1, 3, &val))
		return (false);
	
	*x = filetypes; /*assume success & point to the typelist buffer*/
	
	switch (val.valuetype) {
		
		case listvaluetype:
			if (!langgetlistsize (&val, &ctitems))
				return (false);
			
			ctitems = min (ctitems, maxsftypelist);
			
			(*filetypes).cttypes = (short)ctitems;
			
			for (i = 0; i < ctitems; ++i) {
				
				if (!langgetlistitem (&val, i + 1, nil, &vitem))
					return (false);
				
				if (!coercetoostype (&vitem))
					return (false);
				
				(*filetypes).types [i] = vitem.data.ostypevalue;
				}
			
			break;
		
		default:
			if (!coercetoostype (&val))
				return (false);
			
			filetype = val.data.ostypevalue;
			
			if (filetype == 0) /*no file type specified*/
				*x = nil;
			
			else {
				
				(*filetypes).cttypes = 1;
				
				(*filetypes).types [0] = filetype;
				}
		}
	
	return (true);
	} /*gettypelistvalue*/


static boolean filedialogverb (tysfverb sfverb, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	//
	// put up one of the "standard file" dialogs.  if sfverb is sfputfileverb we use the "put" dialog, otherwise the "get" dialog.
	//
	// we take at least one parameter -- the name of a variable to receive the full path specified by the user.
	//
	// if it's the getfile dialog, we take a second parameter -- it indicates the type of the file.
	//
	// 2006-08-16 creedon: FSRef-ized
	//
	// 2005-10-06 creedon: added creator parameter
	//
	// 1991-12-27 dmb: in all cases, check the current value of the filename variable, and pass it on to sf dialog so it can
	//				potentially set default directory.
	//
	
	bigstring bsprompt;
	bigstring bsvarname;
	tyfilespec fs;
	tyvaluerecord val;
	tysftypelist filetypes;
	ptrsftypelist typelist = nil;
	hdlhashtable htable;
	boolean fl;
	OSType ostype, oscreator = kNavGenericSignature;
	bigstring bsext;
	hdlhashnode hnode;
	
	if (!getstringvalue (hparam1, 1, bsprompt))
		return (false);
	
	if (sfverb != sfgetfileverb)
		flnextparamislast = true;
	
	if (!getvarparam (hparam1, 2, &htable, bsvarname)) // returned filename holder
		return (false);
	
	if (sfverb == sfgetfileverb) { // get extra parameters for get file dialog, indicating file type(s) and file creator
		
		short ctconsumed = 3;
		short ctpositional = 3;
		tyvaluerecord lval;

		if (!gettypelistvalue (hparam1, 3, &filetypes, &typelist))
			return (false);
			
		flnextparamislast = true;
		
		setostypevalue (oscreator, &lval);

		if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x07""creator"), &lval))
			return (false);
	
		oscreator = lval.data.ostypevalue;

		}
	
	clearfilespec (&fs);
	
	//
	// if (idstringvalue (htable, bsvarname, bsfname))
	// 	filecheckdefaultpath (bsfname);
	//
	
	if (hashtablelookup (htable, bsvarname, &val, &hnode)) {
		
		if (!copyvaluerecord (val, &val))
			return (false);
		
		disablelangerror ();
		
		if (coercetofilespec (&val))
			fs = **val.data.filespecvalue;
		
		enablelangerror ();
		}

	if (sfverb == sfputfileverb) {
	
		bigstring bs;
		
		getfsfile ( &fs, bs );
		
		lastword ( bs, '.', bsext);

		if (!((stringlength (bs) == stringlength (bsext)) || (stringlength (bsext) > 4))) { // extension
			stringtoostype (bsext, &ostype);
			filetypes.cttypes = 1;
			filetypes.types [0] = ostype;
			typelist = &filetypes;
			}
		}
	
	setbooleanvalue (false, vreturned); 
	
	if (!sfdialog (sfverb, bsprompt, typelist, &fs, oscreator)) // user hit cancel
		return (true);
	
	if (!setfilespecvalue (&fs, &val))
		return (false);
	
	pushhashtable (htable);
	
	fl = langsetsymbolval (bsvarname, val);
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	exemptfromtmpstack (&val);
	
	(*vreturned).data.flvalue = true; // the user did select a file
	
	return (true);
	
	} // filedialogverb


static boolean getstringorintvalue (hdltreenode hfirst, short pnum, boolean flstring, short *intval, bigstring bsval) {
	
	/*
	if the parameter value is a string, set intval to -1 and return the string; 
	otherwise, return the value as an integer and set bsval to the empty string.
	
	7/29/91 dmb: now strictly enforce flstring; if true, paramter must coerce to a 
	string, otherwise to an int.
	*/
	
	if (flstring) {
		
		if (!getstringvalue (hfirst, pnum, bsval))
			return (false);
		
		*intval = -1;
		}
	else {
		
		if (!getintvalue (hfirst, pnum, intval))
			return (false);
		
		setemptystring (bsval);
		}
	
	return (true);
	} /*getstringorintvalue*/


static boolean getresourceverb (hdltreenode hparam1, boolean flnamed, tyvaluerecord *v) {

	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	tyfilespec fs;
	OSType type;
	short id, forktype;
	short ctconsumed = 4;
	short ctpositional = 4;
	Handle h;
	hdlhashtable htable;
	bigstring bs, bsvarname;
	tyvaluerecord val;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!getstringorintvalue (hparam1, 3, flnamed, &id, bs))
		return (false);
	
	if (!getvarparam (hparam1, 4, &htable, bsvarname)) /*returned handle holder*/
		return (false);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	if (!loadresourcehandle (&fs, type, id, bs, &h, forktype)) {
		
		(*v).data.flvalue = false;
		
		return (true);
		}
	
	if (!insertinhandle (h, 0L, &type, sizeof (type))) { /*out of memory*/
		
		disposehandle (h);
		
		return (false);
		}
	
	if (!langsetbinaryval (htable, bsvarname, h))
		return (false);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*getresourceverb*/


static boolean putresourceverb (hdltreenode hparam1, boolean flnamed, tyvaluerecord *v) {

	//
	// 2006-06-17 creedon: FSRef-ized
	//
	// 2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	// 
	
	tyfilespec fs;
	OSType type, bintype;
	short id, forktype;
	short ctconsumed = 4;
	short ctpositional = 4;
	Handle hbinary;
	bigstring bs;
	tyvaluerecord val;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!getstringorintvalue (hparam1, 3, flnamed, &id, bs))
		return (false);
	
	if (!getbinaryvalue (hparam1, 4, false, &hbinary))
		return (false);
	
	pullfromhandle (hbinary, 0L, sizeof (bintype), &bintype);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);

	forktype = val.data.intvalue;

	(*v).data.flvalue = saveresourcehandle (&fs, type, id, bs, hbinary, forktype);
	
	return (true);
	
	} // putresourceverb


static boolean countrestypesverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	tyfilespec fs;
	short cttypes, forktype;
	short ctconsumed = 1;
	short ctpositional = 1;
	tyvaluerecord val;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	if (!getnumresourcetypes (&fs, &cttypes, forktype))
		cttypes = 0;
	
	setlongvalue (cttypes, v);
	
	return (true);
	} /*countrestypesverb*/


static boolean getnthrestypeverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	tyfilespec fs;
	short n, forktype;
	short ctconsumed = 3;
	short ctpositional = 3;
	OSType type;
	hdlhashtable htable;
	bigstring bsvarname;
	tyvaluerecord val;
	boolean fl;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getintvalue (hparam1, 2, &n))
		return (false);
	
	if (!getvarparam (hparam1, 3, &htable, bsvarname)) /*returned handle holder*/
		return (false);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	if (!getnthresourcetype (&fs, n, &type, forktype)) { /*not a fatal error*/
		
		(*v).data.flvalue = false;
		
		return (true);
		}
	
	setostypevalue (type, &val);
	
	fl = langsetsymboltableval (htable, bsvarname, val);
	
	(*v).data.flvalue = fl;
	
	return (fl);
	} /*getnthrestypeverb*/


static boolean countresourcesverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	tyfilespec fs;
	OSType type;
	short ctresources, forktype;
	short ctconsumed = 2;
	short ctpositional = 2;
	tyvaluerecord val;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	if (!getnumresources (&fs, type, &ctresources, forktype))
		ctresources = 0;
	
	setlongvalue (ctresources, v);
	
	return (true);
	} /*countresourcesverb*/


static boolean getnthresourceverb (hdltreenode hparam1, tyvaluerecord *v) {

	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	tyfilespec fs;
	OSType type;
	short n, id, forktype;
	short ctconsumed = 5;
	short ctpositional = 5;
	Handle h;
	hdlhashtable ht1, ht2;
	bigstring bs, bs1, bs2;
	boolean fl;
	tyvaluerecord val;

	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!getintvalue (hparam1, 3, &n))
		return (false);
	
	if (!getvarparam (hparam1, 4, &ht1, bs1)) /*returned name holder*/
		return (false);
	
	if (!getvarparam (hparam1, 5, &ht2, bs2)) /*returned handle holder*/
		return (false);
		
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	if (!getnthresourcehandle (&fs, type, n, &id, bs, &h, forktype)) {
		
		(*v).data.flvalue = false;
		
		return (true);
		}
	
	pushhashtable (ht1);
	
	fl = langsetstringval (bs1, bs);
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	if (!insertinhandle (h, 0L, &type, sizeof (type))) {
		
		disposehandle (h);
		
		return (false);
		}
	
	if (!langsetbinaryval (ht2, bs2, h))
		return (false);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*getnthresourceverb*/


static boolean getnthresinfoverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	2005-12-26 creedon: commented out param count check
	
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	
	6/2/92 dmb: created.
	*/
	
	tyfilespec fs;
	OSType type;
	short n, id, forktype;
	short ctconsumed = 5;
	short ctpositional = 5;
	hdlhashtable ht1;
	bigstring bs, bs1;
	boolean fl;
	tyvaluerecord val;
	
	/* if (!langcheckparamcount (hparam1, 5))
		return (false); */
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!getintvalue (hparam1, 3, &n))
		return (false);
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	if (!getnthresourcehandle (&fs, type, n, &id, bs, nil, forktype)) {
		
		(*v).data.flvalue = false;
		
		return (true);
		}
	
	if (!langsetlongvarparam (hparam1, 4, id))
		return (false);
	
	if (!getvarparam (hparam1, 5, &ht1, bs1)) /*returned name holder*/
		return (false);
	
	pushhashtable (ht1);
	
	fl = langsetstringval (bs1, bs);
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*getnthresinfoverb*/


static boolean resourceexistsverb (hdltreenode hparam1, boolean flnamed, tyvaluerecord *v) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	tyfilespec fs;
	OSType type;
	short id, forktype;
	short ctconsumed = 3;
	short ctpositional = 3;
	bigstring bs;
	tyvaluerecord val;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!getstringorintvalue (hparam1, 3, flnamed, &id, bs))
		return (false);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	(*v).data.flvalue = loadresourcehandle (&fs, type, id, bs, nil, forktype);
	
	return (true);
	} /*resourceexistsverb*/


static boolean getresourceattrsverb (hdltreenode hparam1, boolean flnamed, tyvaluerecord *v) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	
	2.1b4 dmb: new verb
	*/
	
	tyfilespec fs;
	OSType type;
	short id, attrs, forktype;
	short ctconsumed = 3;
	short ctpositional = 3;
	bigstring bs;
	tyvaluerecord val;

	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!getstringorintvalue (hparam1, 3, flnamed, &id, bs))
		return (false);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	if (!getresourceattributes (&fs, type, id, bs, &attrs, forktype))
		return (false);
	
	return (setintvalue (attrs, v));
	} /*getresourceattrsverb*/


static boolean setresourceattrsverb (hdltreenode hparam1, boolean flnamed, tyvaluerecord *v) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	
	2.1b4 dmb: new verb
	*/
	
	tyfilespec fs;
	OSType type;
	short id, attrs, forktype;
	short ctconsumed = 4;
	short ctpositional = 4;
	bigstring bs;
	tyvaluerecord val;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!getstringorintvalue (hparam1, 3, flnamed, &id, bs))
		return (false);
	
	if (!getintvalue (hparam1, 4, &attrs))
		return (false);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	if (!setresourceattributes (&fs, type, id, bs, attrs, forktype))
		return (false);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*setresourceattrsverb*/


static boolean deleteresourceverb (hdltreenode hparam1, boolean flnamed, tyvaluerecord *v) {
	
	/*
	2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
	*/ 
	
	tyfilespec fs;
	OSType type;
	short id, forktype;
	short ctconsumed = 3;
	short ctpositional = 3;
	bigstring bs;
	tyvaluerecord val;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!getstringorintvalue (hparam1, 3, flnamed, &id, bs))
		return (false);
	
	flnextparamislast = true;
	
	setintvalue (resourcefork, &val); /* defaults to 1 */

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
		return (false);
	
	forktype = val.data.intvalue;

	(*v).data.flvalue = deleteresource (&fs, type, id, bs, forktype);
	
	return (true);
	} /*deleteresourceverb*/


static boolean geticonposverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	9/30/91 dmb: use setintvarparam, saves code
	*/
	
	tyfilespec fs;
	Point pos;
	/*
	hdlhashtable htable;
	bigstring bs;
	*/
	
	if (!langcheckparamcount (hparam1, 3))
		return (false);
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getfilepos (&fs, &pos))
		return (false);
	
	if (!setintvarparam (hparam1, 2, pos.h))
		return (false);
	
	if (!setintvarparam (hparam1, 3, pos.v))
		return (false);
	
	/*
	if (!getvarparam (hparam1, 2, &htable, bs)) 
		return (false);
	
	pushhashtable (htable);
	
	hashassignint (bs, pos.h);
	
	pophashtable ();
	
	if (!getvarparam (hparam1, 3, &htable, bs)) 
		return (false);
	
	pushhashtable (htable);
	
	hashassignint (bs, pos.v);
	
	pophashtable ();
	*/
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*geticonposverb*/


static boolean seticonposverb (hdltreenode hparam1, tyvaluerecord *v) {

	tyfilespec fs;
	Point pos;
	
	if (!getpathvalue (hparam1, 1, &fs)) 
		return (false);
	
	if (!getintvalue (hparam1, 2, &pos.h)) 
		return (false);
	
	flnextparamislast = true;
	
	if (!getintvalue (hparam1, 3, &pos.v)) 
		return (false);
	
	(*v).data.flvalue = setfilepos (&fs, pos);
	
	return (true);
	} /*seticonposverb*/

#ifdef MACVERSION

	typedef struct lNumVersion {
	
		//
		// 2007-09-22 creedon: the nonRelRev is not a BCD number, see < http://developer.apple.com/technotes/tn/tn1132.html >
		//
		// 1992-11-17 dmb: this definition of the version resource makes it easier
		//			    to pick apart
		
		#ifdef SWAP_REZ_BYTE_ORDER
		
			/* unsigned short nonRelRev2: 4; // 2nd nibble of revision level
			unsigned short nonRelRev1: 4; // revision level of non-released version */
			
			unsigned short nonRelRev:  8; // revision level of non-released version
			unsigned short stage:	  8; // stage code: dev, alpha, beta, final
			unsigned short bugFixRev:  4; // 3rd part is 1 nibble in BCD
			unsigned short minorRev:   4; // 2nd part is 1 nibble in BCD
			unsigned short majorRev2:  4; // 2nd nibble of 1st part
			unsigned short majorRev1:  4; // 1st part of version number in BCD
			
		#else
		
			unsigned short majorRev1:  4; // 1st part of version number in BCD
			unsigned short majorRev2:  4; // 2nd nibble of 1st part
			unsigned short minorRev:	  4; // 2nd part is 1 nibble in BCD
			unsigned short bugFixRev:  4; // 3rd part is 1 nibble in BCD
			unsigned short stage:	  8; // stage code: dev, alpha, beta, final
			unsigned short nonRelRev:  8; // revision level of non-released version
			
			/* unsigned short nonRelRev1:  4; // revision level of non-released version
			unsigned short nonRelRev2: 4; // 2nd nibble of revision level */
			
		#endif
		
		} lNumVersion;
		
	
	typedef struct lVersRec {
		lNumVersion numericVersion;		/*encoded version number*/
		short countryCode;				/*country code from intl utilities*/
		Str255 shortVersion;			/*version number string - worst case*/
		Str255 reserved;				/*longMessage string packed after shortVersion*/
		} lVersRec, *lVersRecPtr, **lVersRecHndl;
		
	
	static byte bsstages [] = "\pdab";	/*dev, alpha, beta*/
	
	
	#define emptyversionsize ((long) sizeof (lNumVersion) + sizeof (short) + 2)
	
	
	static boolean versionnumtostring (lNumVersion numvers, bigstring bs) {
	
		//
		// 2007-09-22 creedon: bug fix, when 10.1a16 was entered into
		//				   versions.h, function would return 10.1a10,
		//				   problem, non-released revision was being
		//				   treated as BCD instead of number as specified
		//				   in < http://developer.apple.com/technotes/tn/tn1132.html >,
		//				   had to change lNumVersion struct
		//
		// return the packed version number as a string, e.g. "1.0b2". need 
		// definitions above, which is mis-defined in the Think C headers
		//
		
		/*
		lNumVersion numvers;
		
		numvers = *(lNumVersion *) &versionnum;
		*/
		
		setemptystring (bs);
		
		if (numvers.majorRev1 != 0)
			shorttostring (numvers.majorRev1, bs);
			
		pushint (numvers.majorRev2, bs);
		
		pushchar ('.', bs);
		
		pushint (numvers.minorRev, bs);
		
		if (numvers.bugFixRev > 0) {
		
			pushchar ('.', bs);
			
			pushint (numvers.bugFixRev, bs);
			
			}
			
		if (numvers.stage < finalStage) {
			
			pushchar (bsstages [numvers.stage / developStage], bs);
			
			/* if (numvers.nonRelRev1 > 0)
				pushint (numvers.nonRelRev1, bs);
			
			pushint (numvers.nonRelRev2, bs); */
			
			pushint (numvers.nonRelRev, bs);
			
			}
			
		return (true);
		
		} // versionnumtostring
		
	
	typedef byte shortstring [16];
	
	static short stringtobcd (bigstring bs) {
	
		register short i;
		register short n = 0;
		
		for (i = 1; i <= stringlength (bs); ++i)
			n = (n << 4) + (bs [i] - '0');
			
		return (n);
		} /*stringtobcd*/


	static boolean stringtoversionnum (bigstring bs, NumVersion *versionnum) {
		
		/*
		convert the string to a packed version number.  see above.
		*/
		
		NumVersion numvers;
		register short i;
		register byte ch;
		shortstring bsnumber [4]; /*the three rev numbers, as strings*/
		short number [4]; /*the rev numbers*/
		short ixnumber = 0;
		
		numvers.stage = finalStage;
		
		for (i = 0; i < 4; ++i)
			setemptystring (bsnumber [i]);
		
		for (i = 1; i <= stringlength (bs); ++i) {
			
			ch = bs [i];
			
			if (isnumeric (ch)) { /*digit: add to current number string*/
				
				pushchar (ch, bsnumber [ixnumber]);
				
				continue;
				}
			
			if (ixnumber == 3) /*we're full: take what we've got so far*/
				goto exit;
			
			if (ch == '.') { /*decimal point: move on to next number*/
				
				++ixnumber;
				
				continue;
				}
			
			ixnumber = 3; /*only valid data would be stage character & number*/
			
			switch (lowercasechar (ch)) {
				
				case 'a':
					numvers.stage = alphaStage;
					
					break;
				
				case 'b':
					numvers.stage = betaStage;
					
					break;
				
				case 'd':
					numvers.stage = developStage;
					
					break;
				
				default:
					goto exit;
				}
			}
		
		exit:
		
		for (i = 0; i < 4; ++i)
			number [i] = stringtobcd (bsnumber [i]);
		
		numvers.majorRev = number [0];
		
		numvers.minorAndBugRev = (number [1] << 4) + number [2];
		
		numvers.nonRelRev = number [3];
		
		*versionnum = numvers;
		
		return (true);
		} /*stringtoversionnum*/


	boolean filegetprogramversion (bigstring bsversion) {
		
		/*
		12/19/91 dmb: this routine is here because this is the only file that currently 
		knows the format of a 'vers' resource.  it would more logically be in file.c
		*/
		
		lNumVersion versionnumber;
		
		if (filereadresource (filegetapplicationrnum (), 'vers', 1, nil, sizeof (versionnumber), &versionnumber))
			return (versionnumtostring (versionnumber, bsversion));
		
		setemptystring (bsversion);
		
		return (false);
		} /*filegetprogramversion*/


	static boolean getshortversionverb (hdltreenode hparam1, tyvaluerecord *v) {
		
		/*
		file.getversion (path): string; return the version number as a string, e.g. "1.0b2".
		
		2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
		*/
		
		tyfilespec fs;
		lNumVersion versionnumber;
		bigstring bs;
		short forktype;
		short ctconsumed = 1;
		short ctpositional = 1;
		tyvaluerecord val;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		flnextparamislast = true;
		
		setintvalue (resourcefork, &val); /* defaults to 1 */

		if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
			return (false);
		
		forktype = val.data.intvalue;

		if (loadresource (&fs, -1, 'vers', 1, nil, sizeof (versionnumber), &versionnumber, forktype))
			versionnumtostring (versionnumber, bs);
		else
			setemptystring (bs);
		
		return (setstringvalue (bs, v));
		} /*getshortversionverb*/


	static boolean mungeversionstring (VersRecHndl hvers, bigstring bsversion, boolean fllongvers) {
		
		register byte *p;
		long ixvers;
		long ctvers;
		
		p = (**hvers).shortVersion;
		
		if (fllongvers)
			p += stringlength (p) + 1; /*skip to next contiguous string -- the long version*/
		
		ixvers = p - (byte *) *hvers;
		
		ctvers = stringsize (p);
		
		return (mungehandle ((Handle) hvers, ixvers, ctvers, bsversion, stringsize (bsversion)));
		} /*mungeversionstring*/


	static boolean setshortversionverb (hdltreenode hparam1, tyvaluerecord *v) {
		
		/*
		file.setversion (path): boolean; set the short version string.  if a valid version number is specified, set the numeric fields as well
		
		2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
		*/
		
		tyfilespec fs;
		bigstring bsversion;
		NumVersion versionnumber;
		VersRecHndl hvers;
		short forktype;
		short ctconsumed = 2;
		short ctpositional = 2;
		tyvaluerecord val;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		if (!getstringvalue (hparam1, 2, bsversion)) 
			return (false);
		
		flnextparamislast = true;
		
		setintvalue (resourcefork, &val); /* defaults to 1 */

		if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
			return (false);
		
		forktype = val.data.intvalue;

		if (!loadresourcehandle (&fs, 'vers', 1, nil, (Handle *) &hvers, forktype))
			if (!newclearhandle (emptyversionsize, (Handle *) &hvers))
				return (false);
		
		if (stringtoversionnum (bsversion, &versionnumber)) /*convert to BCD if possible*/
			(**hvers).numericVersion = versionnumber;
		
		if (mungeversionstring (hvers, bsversion, false))
			if (saveresourcehandle (&fs, 'vers', 1, nil, (Handle) hvers, forktype))
				(*v).data.flvalue = true;
		
		disposehandle ((Handle) hvers);
		
		return (true);
		} /*setshortversionverb*/


	static boolean getlongversionverb (hdltreenode hparam1, tyvaluerecord *v) {

		/*
		file.getfullversion (path): string; return the long version string "1.0b2 © Copyright 1991 UserLand Software.".  need definitions above, 
		which don't appear in the Think C headers anywhere
		
		2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
		*/
		
		tyfilespec fs;
		lVersRecHndl hvers;
		bigstring bs;
		short forktype;
		short ctconsumed = 1;
		short ctpositional = 1;
		tyvaluerecord val;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		flnextparamislast = true;
		
		setintvalue (resourcefork, &val); /* defaults to 1 */

		if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
			return (false);
		
		forktype = val.data.intvalue;

		setemptystring (bs);
		
		if (loadresourcehandle (&fs, 'vers', 1, nil, (Handle *) &hvers, forktype)) {
			
			register byte *p;
			
			p = (**hvers).shortVersion;
			
			p += stringlength (p) + 1; /*skip to next contiguous string -- the long version*/
			
			copystring (p, bs);
			
			disposehandle ((Handle) hvers);
			}
		
		return (setstringvalue (bs, v));
		} /*getlongversionverb*/


	static boolean setlongversionverb (hdltreenode hparam1, tyvaluerecord *v) {
		
		/*
		file.setversion (path): boolean; set the long version string.
		
		2005-09-02 creedon: added support for fork parameter, see resources.c: openresourcefile and pushresourcefile
		*/
		
		tyfilespec fs;
		bigstring bsversion;
		VersRecHndl hvers;
		short forktype;
		short ctconsumed = 2;
		short ctpositional = 2;
		tyvaluerecord val;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		if (!getstringvalue (hparam1, 2, bsversion)) 
			return (false);
		
		flnextparamislast = true;
		
		setintvalue (resourcefork, &val); /* defaults to 1 */

		if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x04""fork"), &val))
			return (false);
		
		forktype = val.data.intvalue;

		if (!loadresourcehandle (&fs, 'vers', 1, nil, (Handle *) &hvers, forktype))
			if (!newclearhandle (emptyversionsize, (Handle *) &hvers))
				return (false);
		
		if (mungeversionstring (hvers, bsversion, true))
			if (saveresourcehandle (&fs, 'vers', 1, nil, (Handle) hvers, forktype))
				(*v).data.flvalue = true;
		
		disposehandle ((Handle) hvers);
		
		return (true);
		} /*setlongversionverb*/


	static boolean getcommentverb (hdltreenode hparam1, tyvaluerecord *v) {
		
		tyfilespec fs;
		bigstring bscomment;
		
		flnextparamislast = true;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		if (!getfilecomment (&fs, bscomment))
			setemptystring (bscomment);
		
		return (setstringvalue (bscomment, v));
		} /*getcommentverb*/


	static boolean setcommentverb (hdltreenode hparam1, tyvaluerecord *v) {
		
		tyfilespec fs;
		bigstring bscomment;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		flnextparamislast = true;
		
		if (!getstringvalue (hparam1, 2, bscomment)) 
			return (false);
		
		return (setbooleanvalue (setfilecomment (&fs, bscomment), v));
		} /*setcommentverb*/


	static boolean getlabelverb (hdltreenode hparam1, tyvaluerecord *v) {
		
		//
		// 2006-06-18 creedon: FSRef-ized
		//
		
		tyfilespec fs;
		bigstring bslabel;
		
		flnextparamislast = true;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		if (!getfilelabel (&fs, bslabel))
			setemptystring (bslabel);
		
		return (setstringvalue (bslabel, v));
		
		} // getlabelverb


	static boolean setlabelverb (hdltreenode hparam1, tyvaluerecord *v) {

		//
		// 2006-06-18 creedon: FSRef-ized
		//
		
		tyfilespec fs;
		bigstring bslabel;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		flnextparamislast = true;
		
		if (!getstringvalue (hparam1, 2, bslabel)) 
			return (false);
		
		return (setbooleanvalue (setfilelabel (&fs, bslabel), v));
		
		} // setlabelverb


	static boolean getlabelindexverb (hdltreenode hparam1, tyvaluerecord *v) {

		//
		// 2006-06-18 creedon: FSRef-ized
		//
		// 2006-04-24 creedon: created, cribbed from getlabelverb function
		//
		
		tyfilespec fs;
		short ixlabel;
		
		flnextparamislast = true;

		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);

		if (!getfilelabelindex (&fs, &ixlabel))
			return (false);
		
		return (setintvalue (ixlabel, v));
		
		} // getlabelindexverb


	static boolean setlabelindexverb (hdltreenode hparam1, tyvaluerecord *v) {

		//
		// 2006-06-18 creedon: FSRef-ized
		//
		// 2006-04-24 creedon: created, cribbed from setlabelverb function
		//
		
		tyfilespec fs;
		short index;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		flnextparamislast = true;
		
		if (!getintvalue (hparam1, 2, &index)) 
			return (false);
		
		return (setbooleanvalue (setfilelabelindex (&fs, index, true), v));
		
		} // setlabelindexverb


	static boolean getlabelnamesverb (hdltreenode hparam1, tyvaluerecord *v) {
		#pragma unused (hparam1)

		/*
		2006-04-24 creedon: created, cribbed from statusbargetsectionsfunc case of langfunctionvalue function
		*/

		bigstring bsarray [8];
		hdllistrecord hlist;
		int ix;
		RGBColor labelcolor;
		short mapfromuserinterfaceindex [8] = {0, 6, 7, 5, 2, 4, 3, 1};

		for (ix = 0; ix < 8; ++ix)
		
			if (GetLabel (ix, &labelcolor, bsarray [ix]) != noErr)
			
				return (false);
				
		if (!opnewlist (&hlist, false))
			return (false);

		for (ix = 0; ix < 8; ++ix)

			if (!langpushliststring (hlist, bsarray [mapfromuserinterfaceindex [ix]]))
			
				goto getlabelnamesverberror;

		return (setheapvalue ((Handle) hlist, listvaluetype, v));
		
		getlabelnamesverberror:

		opdisposelist (hlist);
		
		return (false);

		} /* getlabelnamesverb */
			
#endif // MACVERSION


static boolean findapplicationverb ( hdltreenode hparam1, tyvaluerecord *v ) {

	OSType creator;
	tyfilespec fsapp;
	
	flnextparamislast = true;
	
	if ( ! getostypevalue ( hparam1, 1, &creator ) ) 
		return ( false );
		
	if ( ! findapplication ( creator, &fsapp ) )
		clearbytes ( &fsapp, sizeof ( fsapp ) );
		
	return ( setfilespecvalue ( &fsapp, v ) );
	
	} // findapplicationverb


#ifdef WIN95VERSION

	boolean filegetprogramversion (bigstring bsversion) {
		
		return (getstringlist (defaultlistnumber, programversion, bsversion));
		} /*filegetprogramversion*/


	static boolean getshortversionverb (hdltreenode hparam1, tyvaluerecord *v) {
		
		/*
		file.getversion (path): string; return the version number as 
		a string, e.g. "1.0b2".
		*/
		
		tyfilespec fs;
		DWORD dummyhandle;
		bigstring bs;
		DWORD buflen;
		char * buf;
		VS_FIXEDFILEINFO * ffi;
		char * info;
		boolean flffi;
		
		flnextparamislast = true;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		nullterminate (fsname(&fs));

		setemptystring (bs);

		buflen = GetFileVersionInfoSize (stringbaseaddress(fsname(&fs)), &dummyhandle);

		if (buflen > 0) {
			buf = (char *) LocalAlloc (LPTR, buflen);
			
			if (buf != NULL) {
				if (GetFileVersionInfo (stringbaseaddress(fsname(&fs)), dummyhandle, buflen, buf)) {
					buflen = sizeof(VS_FIXEDFILEINFO);

					flffi = VerQueryValue (buf, "\\", &ffi, &buflen);

					if (flffi)
						flffi = buflen > 0;

					buflen = 0;

					VerQueryValue (buf, "\\StringFileInfo\\040904E4\\FileVersion", &info, &buflen);
					
					if (buflen == 0) {
						VerQueryValue (buf, "\\StringFileInfo\\040904B0\\FileVersion", &info, &buflen);
						}

					if (buflen != 0)
						copyctopstring (info, bs);
					else if (flffi) {
						wsprintf (stringbaseaddress (bs), "%d.%d.%d.%d", HIWORD(ffi->dwFileVersionMS), 
								LOWORD(ffi->dwFileVersionMS), HIWORD(ffi->dwFileVersionLS), LOWORD(ffi->dwFileVersionLS));
						setstringlength (bs, strlen(stringbaseaddress(bs)));
						}
					}

				LocalFree (buf);
				}
			}

		return (setstringvalue (bs, v));
		} /*getshortversionverb*/


	static boolean getlongversionverb (hdltreenode hparam1, tyvaluerecord *v) {

		/*
		file.getfullversion (path): string; return the long version string 
		"1.0b2 © Copyright 1991 UserLand Software.".  need definitions above, 
		which don't appear in the Think C headers anywhere
		*/
		
		tyfilespec fs;
		DWORD dummyhandle;
		bigstring bs, bs2;
		DWORD buflen;
		char * buf;
		VS_FIXEDFILEINFO * ffi;
		char * info;
		
		flnextparamislast = true;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		nullterminate (fsname(&fs));

		setemptystring (bs);
		setemptystring (bs2);

		buflen = GetFileVersionInfoSize (stringbaseaddress(fsname(&fs)), &dummyhandle);

		if (buflen > 0) {
			buf = (char *) LocalAlloc (LPTR, buflen);
			
			if (buf != NULL) {
				if (GetFileVersionInfo (stringbaseaddress(fsname(&fs)), dummyhandle, buflen, buf)) {
					buflen = sizeof(VS_FIXEDFILEINFO);
					VerQueryValue (buf, "\\", &ffi, &buflen);

					VerQueryValue (buf, "\\StringFileInfo\\040904E4\\FileVersion", &info, &buflen);
					
					if (buflen == 0) {
						VerQueryValue (buf, "\\StringFileInfo\\040904B0\\FileVersion", &info, &buflen);
						}

					if (buflen != 0)
						copyctopstring (info, bs);

					VerQueryValue (buf, "\\StringFileInfo\\040904E4\\LegalCopyright", &info, &buflen);
					
					if (buflen == 0) {
						VerQueryValue (buf, "\\StringFileInfo\\040904B0\\LegalCopyright", &info, &buflen);
						}

					if (buflen != 0)
						copyctopstring (info, bs2);

					if (stringlength(bs) > 0)
						pushspace (bs);

					pushstring (bs2, bs);
					}

				LocalFree (buf);
				}
			}

		return (setstringvalue (bs, v));
		} /*getlongversionverb*/

	static boolean getcommentverb (hdltreenode hparam1, tyvaluerecord *v) {
		
		/*
		file.getversion (path): string; return the version number as 
		a string, e.g. "1.0b2".
		*/
		
		tyfilespec fs;
		DWORD dummyhandle;
		bigstring bs;
		DWORD buflen;
		char * buf;
		VS_FIXEDFILEINFO * ffi;
		char * info;
		
		flnextparamislast = true;
		
		if (!getpathvalue (hparam1, 1, &fs)) 
			return (false);
		
		nullterminate (fsname(&fs));

		setemptystring (bs);

		buflen = GetFileVersionInfoSize (stringbaseaddress(fsname(&fs)), &dummyhandle);

		if (buflen > 0) {
			buf = (char *) LocalAlloc (LPTR, buflen);
			
			if (buf != NULL) {
				if (GetFileVersionInfo (stringbaseaddress(fsname(&fs)), dummyhandle, buflen, buf)) {
					buflen = sizeof(VS_FIXEDFILEINFO);
					VerQueryValue (buf, "\\", &ffi, &buflen);

					VerQueryValue (buf, "\\StringFileInfo\\040904E4\\FileDescription", &info, &buflen);
					
					if (buflen == 0) {
						VerQueryValue (buf, "\\StringFileInfo\\040904B0\\FileDescription", &info, &buflen);
						}

					if (buflen != 0)
						copyctopstring (info, bs);
					}

				LocalFree (buf);
				}
			}

		return (setstringvalue (bs, v));
		} /*getcommentverb*/

#endif // WIN95VERSION


/*start of new verbs added by DW, 7/27/91*/

static boolean findfileverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	8/25/92 dmb: set return value to true or false, so glue isn't needed
	*/
	
	tyfilespec fs;
	bigstring pattern;
	long idx;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, pattern))
		return (false);
		
	fiffindinfile (&fs, pattern, &idx);
	
	/*
	setlongvalue (idx, v);
	*/
	
	setbooleanvalue ((boolean) (idx >= 0), v);
	
	return (true);
	} /*findfileverb*/
	

static boolean countlinesverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	tyfilespec fs;
	long ctlines;
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
		
	fifcharcounter (&fs, chreturn, &ctlines);
	
	setlongvalue (ctlines, v);
	
	return (true);
	} /*countlinesverb*/
	

static boolean openfileverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	// 5.0.1 dmb: removed incorrect case for W95
	//

	tyfilespec fs;
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	(*v).data.flvalue = fifopenfile (&fs, (long) currentprocess);
	
	return (true);
	
	} // openfileverb
	

static boolean closefileverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-17 creedon: for Mac, FSRef-ized
	//

	tyfilespec fs;
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	(*v).data.flvalue = fifclosefile (&fs);
	
	return (true);
	
	} // closefileverb


static boolean endoffileverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: FSRef-ized
	//

	tyfilespec fs;
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
		
	(*v).data.flvalue = fifendoffile (&fs);
	
	return (true);
	} // endoffileverb
	

static boolean setendoffileverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: FSRef-ized
	//

	tyfilespec fs;
	long eof;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 2, &eof))
		return (false);
		
	(*v).data.flvalue = fifsetendoffile (&fs, eof);
	
	return (true);
	} // endoffileverb
	

static boolean getendoffileverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: FSRef-ized
	//

	tyfilespec fs;
	long eof;
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	if (!fifgetendoffile (&fs, &eof))
		return (false);
	
	return (setlongvalue (eof, v));
	
	} // endoffileverb


static boolean setpositionverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: FSRef-ized
	//

	tyfilespec fs;
	long pos;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 2, &pos))
		return (false);
	
	(*v).data.flvalue = fifsetposition (&fs, pos);
	
	return (true);
	
	} // positionverb
	

static boolean getpositionverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: FSRef-ized
	//

	tyfilespec fs;
	long pos;
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	if (!fifgetposition (&fs, &pos))
		return (false);
	
	return (setlongvalue (pos, v));
	
	} // positionverb
	

static boolean readlineverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: FSRef-ized
	//

	tyfilespec fs;
	Handle linestring;
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	fifreadline (&fs, &linestring);
	
	return (setheapvalue (linestring, stringvaluetype, v));
	
	} // readlineverb
	

static boolean writelineverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: for Mac, FSRef-ized
	//

	tyfilespec fs;
	Handle linestring;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	flnextparamislast = true;
	
	if (!gettextvalue (hparam1, 2, &linestring))
		return (false);
	
	(*v).data.flvalue = fifwriteline (&fs, linestring);
	
	return (true);
	
	} // writelineverb


static boolean readwholefileverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	//
	// Read the whole file into memory and return the data to the caller.
	//
	// 2006-06-18 creedon: FSRef-ized
	//
	// 2006-04-11 aradke:	Kernelized file.readWholeFile. Obsolete script code follows:
	//
	//				on readWholeFile (f) {
	//					«10/31/97 at 1:02:04 PM by DW -- moved from toys.readWholeFile
	//					local (s);
	//					file.open (f);
	//					s = file.read (f, infinity);
	//					file.close (f);
	//					return (s)}
	//
	//				This kernel implementation is much more efficient than file.read because it pre-allocates a handle
	//				large enough for the whole file.
	//

	tyfilespec fs;
	Handle x;
	boolean fl;
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);

	if (!fifopenfile (&fs, (long) currentprocess))
		return (false);

	fl = fifreadfile (&fs, &x);
		
	disablelangerror ();
		
	(void) fifclosefile (&fs); // ignore return value, we got what we wanted
	
	enablelangerror ();
	
	return (fl && setbinaryvalue (x, '\?\?\?\?', v));
	
	} // readwholefileverb


static boolean readverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	//
	// 2006-06-18 creedon: FSRef-ized
	//
	
	tyfilespec fs;
	Handle x;
	long ctbytes;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 2, &ctbytes))
		return (false);
		
	if (!fifreadhandle (&fs, ctbytes, &x))
		return (true);
	
	if ((ctbytes < longinfinity) && (ctbytes != gethandlesize (x))) { // ran out of data
		
		disposehandle (x);
		
		return (true);
		}
	
	return (setbinaryvalue (x, '\?\?\?\?', v));
	
	} // readverb


static boolean writeverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	//
	// 2006-06-18 creedon: FSRef-ized
	//
	// 5.0.2b17 dmb: use new getreadonlytextvalue
	//
	
	tyfilespec fs;
	Handle hdata;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	flnextparamislast = true;
	
	if (!getreadonlytextvalue (hparam1, 2, &hdata))
		return (false);
	
	(*v).data.flvalue = fifwritehandle (&fs, hdata);
	
	return (true);
	
	} // writeverb


#if 0

static boolean writewholefileverb (hdltreenode hparam1, tyvaluerecord *v) {

	/*
	on writeWholeFile (f, s, type = nil, creator = nil, creationdate = clock.now ()) {
		Ç10/31/97 at 1:41:20 PM by DW -- moved from toys.writeWholeFile
		ÇFriday, July 18, 1997 at 10:22:07 AM by PBS
			ÇConditionalized for multiple platforms, with optional parameters.
		file.new (f);
		file.open (f);
		if sys.os () == "MacOS" {
			if type != nil {
				file.setType (f, type)};
			if creator != nil {
				file.setCreator (f, creator)};
			file.setCreated (f, creationdate)};
		file.write (f, s);
		file.close (f);
		return (true)}
	*/
	
	tyfilespec fs;
	tyvaluerecord val;
	
	if (!getpathvalue (hparam1, 1, &fs))
		return (false);
	
	flnextparamislast = true;
	
	if (!getbinaryparam (hparam1, 2, &val))
		return (false);
	
	if (!coercetostring (&val)) //strip binary type
		return (false);
	
	(*v).data.flvalue = fifwritehandle (&fs, (Handle) val.data.stringvalue);
	
	return (true);
	} /*writewholefileverb*/

#endif


static boolean comparefilesverb (hdltreenode hparam1, tyvaluerecord *v) {

	//
	// 2006-06-18 creedon: for Mac, FSRef-ized
	//
	
	tyfilespec fs1, fs2;
	
	if (!getpathvalue (hparam1, 1, &fs1))
		return (false);
	
	flnextparamislast = true;
	
	if (!getpathvalue (hparam1, 2, &fs2))
		return (false);
	
	(*v).data.flvalue = fifcomparefiles (&fs1, &fs2);
	
	return (true);
	
	} // comparefilesverb
	

/*end of new verbs added by DW, 7/27/91*/


static boolean filedeleteverb ( hdltreenode hp1, tyvaluerecord *vreturned ) {

	//
	// 2006-06-23 creedon:	FSRef-ized
	//
	//				created, factored from filefunctionvalue function
	//
	
	tyfilespec fs;
	
	flnextparamislast = true;
	
	if ( ! getfilespecvalue ( hp1, 1, &fs ) )
		return ( false );
	
	if ( ! deletefile ( &fs ) )
		return ( false );
				
	return ( setbooleanvalue ( true, vreturned ) );
		
	} // filedeleteverb


static boolean getposixpathverb ( hdltreenode hp1, tyvaluerecord *vreturned ) {
	
	// 2013-12-06 TedCHoward: corrected a compiler warning and made the code clearer by using a UInt8[] instead of a bigstring for holding the posix path
	//
	// 2006-11-01 creedon: for Mac, replace convertcstring function with code to convert from c string in UTF-8 to bigstring in
	//			       Mac Roman
	//
	// 2006-10-07 creedon: created
	//
	
	#ifdef	MACVERSION
	
		bigstring bs;
		UInt8 pathBuffer[256];
		tyfilespec fs;
		boolean flfolder;
		
		flnextparamislast = true;
		
		if ( ! getfilespecvalue ( hp1, 1, &fs ) )
			return ( false );
		
		if ( oserror ( FSRefMakePath ( &fs.ref, pathBuffer, 256 ) ) )
			return ( false );
		
		/* convert from c string to bigstring */ {
			
		
			CFStringRef csr = CFStringCreateWithCString ( kCFAllocatorDefault, (char *)pathBuffer, kCFStringEncodingUTF8 );
			
			CFStringGetPascalString(csr, bs, sizeof (bigstring), kCFStringEncodingMacRoman);
			
			//cfstringreftobigstring ( csr, bs );
			
			CFRelease ( csr );
			
			}
		
		if ( ! fs.flags.flvolume ) {	// add name
			
			bigstring bsfile;
			
			fsnametobigstring ( &fs.name, bsfile );
			
			pushchar ( '/', bs );
			
			pushstring ( bsfile, bs );
			}
		
		if ( fileexists ( &fs, &flfolder ) )
			if ( flfolder )
				assurelastchariscolon ( bs );

		return ( setstringvalue ( bs, vreturned ) );
	
	#endif // MACVERSION
	
	#ifdef	WIN95VERSION
	
		return ( false );
	
	#endif
	
	} // getposixpathverb


#ifdef MACVERSION

	static boolean newaliasverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
		
		//
		// 2006-06-18 creedon: FSRef-ized
		//
		// 1992-06-02 dmb: make sure that destination path doesn't end in a colon; otherwise pathtofilespec will fail
		//
		
		boolean fl = false;
		tyfilespec fs, fsalias;
		
		if (!langcanusealiases ())
			return (false);
		
		if (!getpathvalue (hparam1, 1, &fs)) // bs holds the source file's path
			return (false);
		
		flnextparamislast = true;
		
		if (!getpathvalue (hparam1, 2, &fsalias)) // bsalias holds the new alias's path
			return (false);
		
		if (!surefile (&fs))	// make sure source file exists
			return (false);
		
		setfserrorparam ( &fsalias ); // assume error will relate to new file
		
//		FSRef fst, fsaliast;
//		
//		if (oserror(macgetfsref(&fs, &fst)) || oserror(macgetfsref(&fsalias, &fsaliast))) {
//			return false;
//		}
		
		fl = MakeAliasFile ( &fs, &fsalias );
		
		setbooleanvalue ( fl, vreturned );
		
		return (true);
		
		} // newaliasverb


	static boolean followaliasverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
		
		//
		// 2006-06-18 creedon: FSRef-ized
		//
		// 1992-01-29 dmb: a kernel version of what used to be implemented in glue.  there a difference in the functionality
		// as coded here:  ResolveAliasFile is being asked to follow chains of aliases; it will follow an alias to an alias to the
		// original.  to match the original version & documentation, if the original file is not an alias, the empty string is
		// returned.  also, we need to Gestalt aliases...
		//
		
		bigstring bs;
		tyfilespec fs;
		FSRef fsref;
		Boolean flaliasfolder, flwasalias;
		OSErr errcode;
		
		if (!langcanusealiases ())
			return (false);
		
		flnextparamislast = true;
		
		if (!getpathvalue (hparam1, 1, &fs)) // fs holds the file path
			return (false);
			
		if (macgetfsref (&fs, &fsref) != noErr) { // make sure original
			
			setfserrorparam (&fs);
			
			oserror (errorFileNotFound); // file not found
			
			return (false);
			}
		
		errcode = FSResolveAliasFile (&fsref, true, &flaliasfolder, &flwasalias);
		
		switch (errcode) {
			
			case errorNone:
				if (flwasalias) {
				
					if (oserror (macmakefilespec (&fsref, &fs)))
						return (false);
					
					return (setfilespecvalue (&fs, vreturned));
					}
				else
					setemptystring (bs);
				
				break;
			
			case errorFileNotFound:
				setemptystring (bs);
				
				break;
			
			case userCanceledErr:
				return (false);
			
			default:
				oserror (errcode);
				
				return (false);
			}
		
		return (setstringvalue (bs, vreturned));
		
		} // followaliasverb
		

	boolean filelaunchanythingverb ( hdltreenode hparam1, tyvaluerecord *vreturned ) {
		
		//
		// 2006-06-26 creedon: replace System7Open with LSOpenFSRef
		//
		// 1992-06-01 dmb: generate oserror if System7Open fails
		//

		tyfilespec fs;
		FSRef fsref;
		
		flnextparamislast = true;
		
		if (!getpathvalue(hparam1, 1, &fs)) // fs holds the file's path
			return (false);
		
		setfserrorparam(&fs);
		
		if (oserror(macgetfsref(&fs, &fsref)))
			return (false);
		
		if (oserror(LSOpenFSRef(&fsref, NULL)))
			return (false);
		
		return (setbooleanvalue(true, vreturned));
		
		} // filelaunchanythingverb
		
#endif


static boolean filefunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	//
	// bridges file.c with the language.  the name of the verb is bs, its first
	// parameter is hparam1, and we return a value in vreturned.
	//
	// we use a limited number of support routines from lang.c to get parameters
	// and to return values. 
	//
	// return false only if the error is serious enough to halt the running of
	// the script that called us, otherwise error values are returned through
	// the valuerecord, which is available to the script.
	//
	// if we return false, we try to provide a descriptive error message in the 
	// returned string bserror.
	//
	// 2006-10-23 creedon: for Mac, extend filespec for fs2 in case filemovefunc
	//
	// 2006-06-25 creedon: FSRef-ized
	//
	// 2006-04-11 aradke: added readwholefilefunc
	//
	// 5.0.2b16 dmb: added getpathcharfunc
	//
	// 2.1b12 dmb:	if mountservervolumefunc's volumepath is improperly formed,
	//				generate informative message instead of failing silently
	//
	// 2.1b2 dmb: use filespecs everywhere. added volumeblocksize verb
	//
	// 1992-09-30 dmb:	call new langcheckstackspace; most file functions chew
	//					up a good bit
	//
	// 1992-09-26 dmb:	use one bigstring instead of two for filefrompath and
	//					folderfrompath.  also, dont allow getfullfilepath to
	//					generate an error in folderfrompath
	//
	// 1992-08-25 dmb: made newfunc delete existing file, so glue isn't necessary
	//
	// 1992-06-09 dmb: added file.isvisible, file.setvisible
	//
	// 1992-06-02 dmb: made filefrompath work as documented so that glue isn't needed
	//
	// 1992-02-13 dmb: re-hooked up hasbundle verb
	//
	// 1991-10-15: file.setpath, file.folderfrompath now handle partial paths corectly
	//
	
	register hdltreenode hp1 = hparam1;
	register tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /*by default, file functions return false*/
	
	if (!langcheckstackspace ())
		return (false);
	
	switch (token) {
		
		case filecreatedfunc: {
		
			unsigned long datecreated, datetoss;
			
			boolean fl;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs)) /*fs holds the file path*/
				break;
				
			if (fileisvolume (&fs))
				fl = volumecreated (&fs, &datecreated);
			else
				fl = getfiledates (&fs, &datecreated, &datetoss);
			
			if (!fl)
				break;
			
			return (setdatevalue (datecreated, v));
			}
			
		case filemodifiedfunc: {
			
			unsigned long datemodified, datetoss;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!getfiledates (&fs, &datetoss, &datemodified))
				break;
			
			return (setdatevalue (datemodified, v));
			}
			
		case filetypefunc: {
			
			OSType type;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
				
			if (!getfiletype (&fs, &type))
				break;
			
			return (setostypevalue (type, v));
			}
			
		case filecreatorfunc: {
			
			OSType creator;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;

			if (!getfilecreator (&fs, &creator))
				break;
			
			return (setostypevalue (creator, v));
			}
			
		case setfilecreatedfunc: {
			tyfilespec fs;
			unsigned long when;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			flnextparamislast = true;
			
			if (!getdatevalue (hp1, 2, &when))
				break;
			
			if (!setfilecreated (&fs, when))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case setfilemodifiedfunc: {
			tyfilespec fs;
			unsigned long when;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			flnextparamislast = true;
			
			if (!getdatevalue (hp1, 2, &when))
				break;
			
			if (!setfilemodified (&fs, when))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case fileisfolderfunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!fileisfolder (&fs, &(*v).data.flvalue))
				break;
			
			return (true);
			}
		
		case fileisvolumefunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs)) /*was getvolumevalue*/
				break;
			
			(*v).data.flvalue = fileisvolume (&fs);
			
			return (true);
			}
		
		case fileislockedfunc: {
		
			boolean fl;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (fileisvolume (&fs))
				fl = isvolumelocked (&fs, &(*v).data.flvalue);
			else
				fl = fileislocked (&fs, &(*v).data.flvalue);
			
			if (!fl)
				break;
			
			return (true);
			
			}
		
		case fileisbusyfunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!fileisbusy (&fs, &(*v).data.flvalue))
				break;
			
			return (true);
			}
		
		case filehasbundlefunc: {
		
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if ( ! getpathvalue ( hp1, 1, &fs ) )
				break;
			
			if ( ! filehasbundle ( &fs, &( *v ).data.flvalue ) )
				break;
			
			return ( true );
			
			}
		
		case filesetbundlefunc: {
			tyfilespec fs;
			boolean flbundle;
			boolean flfolder;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hp1, 2, &flbundle))
				break;
			
			if (!fileisfolder (&fs, &flfolder))
				break;
			
			if (!flfolder) {
			
				if (!filesetbundle (&fs, flbundle))
					break;
				
				(*v).data.flvalue = true;
				}
			
			return (true);
			}
		
		case fileisaliasfunc: {
		
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!fileisalias (&fs, &(*v).data.flvalue))
				break;
			
			return (true);
			}
		
		case fileisvisiblefunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!fileisvisible (&fs, &(*v).data.flvalue))
				break;
			
			return (true);
			}
		
		case filesetvisiblefunc: {
			tyfilespec fs;
			boolean flvisible;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hp1, 2, &flvisible))
				break;
			
			if (!filesetvisible (&fs, flvisible))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case filesizefunc: {
			tyfilespec fs;
			long size;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!filesize (&fs, &size))
				break;
			
			return (setlongvalue (size, v));
			}
		
		case filelockfunc: {
		
			tyfilespec fs;
			boolean fl;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (fileisvolume (&fs))
				fl = lockvolume (&fs, true);
			else
				fl = lockfile (&fs);
			
			if (!fl)
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			
			}
			
		case fileunlockfunc: {
		
			tyfilespec fs;
			boolean fl;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (fileisvolume (&fs))
				fl = lockvolume (&fs, false);
			else
				fl = unlockfile (&fs);
			
			if (!fl)
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			
			}
			
		case filefullpathfunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			/*getfullfilepath*/
			
			return (setfilespecvalue (&fs, v));
			}
		
		case filecopyfunc:
			return (copyfileverb (true, true, hp1, v));
		
		case filecopydataforkfunc:
			return (copyfileverb (true, false, hp1, v));
		
		case filecopyresourceforkfunc:
			return (copyfileverb (false, true, hp1, v));
			
		case filedeletefunc:
			return ( filedeleteverb ( hp1, v ) );
		
		case fileexistsfunc: {
		
			boolean fl, flfolder;
			tyfilespec fs;

			if (!langcheckparamcount (hp1, 1))
				break;
			
			disablelangerror ();
			
			fl = getpathvalue (hp1, 1, &fs);
			
			enablelangerror ();
			
			(*v).data.flvalue = fl && fileexists (&fs, &flfolder);
			
			return (true);
			}
		
		case filefrompathfunc:
			return (filefrompathverb (hp1, v));
		
		case filegetpathfunc: {
			tyfilespec fs;
			
			if (!langcheckparamcount (hp1, 0)) // no parameters expected
				break;
			
			if (!filegetdefaultpath (&fs))
				break;
			
			return (setfilespecvalue (&fs, v));
			}
			
		case filesetpathfunc: {
		
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!filesetdefaultpath (&fs))
				break;

			(*v).data.flvalue = true;
			
			return (true);
			
			}
		
		case newfunc: {
		
			tyfilespec fs;
			boolean flfolder;

			flnextparamislast = true;
			
			if ( ! getfilespecvalue ( hp1, 1, &fs ) )
				break;
			
			if (fileexists (&fs, &flfolder)) { // 8/25/92 dmb
			
				if (!deletefile (&fs))
					break;
				}
				
			if ( ! newfile ( &fs, '\?\?\?\?', '\?\?\?\?') )
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			
			}
		
		case newfolderfunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!newfolder (&fs))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case filerenamefunc: {
			tyfilespec fs;
			bigstring bs;
			
			if (!getpathvalue (hp1, 1, &fs)) /*bs1 holds the path*/
				break;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hp1, 2, bs)) /*bs2 holds the new name*/
				break;
			
			/*
			if (fileisvolume (bs1))
				(*v).data.flvalue = renamevolume (bs1, bs2);
			else
			*/
			
			if (!renamefile (&fs, bs))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case filemovefunc: {
		
			tyfilespec fs1, fs2;
			
			if ( ! getpathvalue ( hp1, 1, &fs1 ) ) // bs1 holds the path
				break;
			
			flnextparamislast = true;
			
			if ( ! getpathvalue ( hp1, 2, &fs2 ) ) // bs2 holds the new name
				break;
			
			if ( ! movefile ( &fs1, &fs2 ) )
				break;
			
			( *v ).data.flvalue = true;
			
			return ( true );
			
			}
		
		/*
		case filesinfolder: {
			tyfilespec fs;
			long ctfiles;
			
			flnextparamislast = true;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			if (!filesinfolder (&fs, &ctfiles))
				break;
			
			return (setlongvalue (ctfiles, v));
			}
		*/
		
		case sfgetfilefunc:
			return (filedialogverb (sfgetfileverb, hp1, v));
		
		case sfputfilefunc:
			return (filedialogverb (sfputfileverb, hp1, v));
		
		case sfgetfolderfunc:
			return (filedialogverb (sfgetfolderverb, hp1, v));
		
		case sfgetdiskfunc:
			return (filedialogverb (sfgetdiskverb, hp1, v));
		
		/*
		case fileeditlinefeedsfunc:
			return (editlinefeedsverb (hp1, v));
		*/
		
		case volumeisejectablefunc: {
		
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;
			
			return (isejectable (&fs, &(*v).data.flvalue));
			
			}

		case volumefreespacefunc: {
		
			long ctbytes;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;
			
			if (!getfreespace (&fs, &ctbytes))
				break;

			return (setlongvalue (ctbytes, v));
			
			}
		
		case volumesizefunc: {
		
			long ctbytes;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;

			if (!getvolumesize (&fs, &ctbytes))
				break;
			
			return (setlongvalue (ctbytes, v));
			
			}
		
		case volumefreespacedoublefunc: { // 6.1b16 AR
		
			double freebytes, totalbytes;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;
			
			if (!langgetextendedvolumeinfo (&fs, &totalbytes, &freebytes))
				break;

			return (setdoublevalue (freebytes, v));
			
			}
		
		case volumesizedoublefunc: { // 6.1b16 AR
		
			double freebytes, totalbytes;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;
			
			if (!langgetextendedvolumeinfo (&fs, &totalbytes, &freebytes))
				break;

			return (setdoublevalue (totalbytes, v));
			
			}

		
		case volumeblocksizefunc: {
		
			long ctbytes;
			tyfilespec fs;

			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;
			
			if (!getvolumeblocksize (&fs, &ctbytes))
				break;
			
			return (setlongvalue (ctbytes, v));
			
			}
		
		case filesonvolumefunc: {
		
			long ctfiles;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if ( ! getvolumevalue ( hp1, 1, &fs ) )
				break;
			
			if ( ! filesonvolume ( &fs, &ctfiles ) )
				break;
			
			return ( setlongvalue ( ctfiles, v ) );
			
			}
		
		case foldersonvolumefunc: {
		
			long ctfolders;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;
			
			if (!foldersonvolume (&fs, &ctfolders))
				break;
			
			return (setlongvalue (ctfolders, v));
			
			}
		
		/* 11/7/91 dmb: now called from shellsysverbs.c
		case filelaunchfunc:
			return (filelaunchanythingverb (hp1, v));
		*/
		
		/*start of new verbs added by DW, 7/27/91*/
		
		case findinfilefunc:
			return (findfileverb (hp1, v));
		
		case countlinesfunc:
			return (countlinesverb (hp1, v));
		
		case openfilefunc:
			return (openfileverb (hp1, v));
		
		case closefilefunc:
			return (closefileverb (hp1, v));
		
		case endoffilefunc:
			return (endoffileverb (hp1, v));
		
		case setendoffilefunc:
			return (setendoffileverb (hp1, v));
		
		case getendoffilefunc:
			return (getendoffileverb (hp1, v));

		case setpositionfunc:
			return (setpositionverb (hp1, v));

		case getpositionfunc:
			return (getpositionverb (hp1, v));

		case readlinefunc:
			return (readlineverb (hp1, v));
		
		case writelinefunc:
			return (writelineverb (hp1, v));
		
		case readfunc:
			return (readverb (hp1, v));
		
		case writefunc:
			return (writeverb (hp1, v));
		
		case comparefunc:
			return (comparefilesverb (hp1, v));
	
		/*end of new verbs added by DW, 7/27/91*/
		
		//case writewholefilefunc:
		//	return (writewholefileverb (hp1, v));
		
		case getpathcharfunc:
			if (!langcheckparamcount (hp1, 0))
				return (false);
			
			#ifdef MACVERSION
				return (setstringvalue (BIGSTRING ("\x01" ":"), v));
			#endif
			#ifdef WIN95VERSION
				return (setstringvalue (BIGSTRING ("\x01" "\\"), v));
			#endif
		
		case getshortversionfunc: 
			return (getshortversionverb (hp1, v));

		case getlongversionfunc: 
			return (getlongversionverb (hp1, v));
		
		case filegetcommentfunc:
			return (getcommentverb (hp1, v));
		
		case filefindappfunc:
			return (findapplicationverb (hp1, v));

		/* 3/20/97 - The following are MAC speciifc verbs and are therefore grouped
			together here for ease of ifdefing */

	#ifdef MACVERSION
		case newaliasfunc:
			return (newaliasverb (hp1, v));
		
		case filefollowaliasfunc:
			return (followaliasverb (hp1, v));
		
		case filegeticonposfunc:
			return (geticonposverb (hp1, v));
		
		case fileseticonposfunc: 
			return (seticonposverb (hp1, v));
				
		case setshortversionfunc: 
			return (setshortversionverb (hp1, v));
		
		case setlongversionfunc: 
			return (setlongversionverb (hp1, v));
		
		case filesetcommentfunc:
			return (setcommentverb (hp1, v));
		
		case filegetlabelfunc:
			return (getlabelverb (hp1, v));
		
		case filesetlabelfunc:
			return (setlabelverb (hp1, v));
		
		case unmountvolumefunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;
			
			if (!unmountvolume (&fs))
				break;
			
			return (setbooleanvalue (true, v));
			}
		
		#if TARGET_API_MAC_CARBON != 1 /*7.0B59 PBS: not implemented in OS X yet*/
		
			case mountservervolumefunc: {
				bigstring bsvol, bsuser, bspassword;
				
				if (!getstringvalue (hp1, 1, bsvol))
					break;
				
				if (!getstringvalue (hp1, 2, bsuser))
					break;
				
				flnextparamislast = true;
				
				if (!getstringvalue (hp1, 3, bspassword))
					break;
				
				if (countwords (bsvol, chpathseparator) != 3) {
					
					langparamerror (badnetworkvolumespecificationerror, bsvol);
					
					return (false);
					}
				
				if (!mountvolume (bsvol, bsuser, bspassword))
					break;
				
				return (setbooleanvalue (true, v));
				}
		#endif
		
		case volumeejectfunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getvolumevalue (hp1, 1, &fs))
				break;
			
			(*v).data.flvalue = ejectvol (&fs);
			
			return (true);
			}
			
		case setfiletypefunc: {
			tyfilespec fs;
			OSType type;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
				
			flnextparamislast = true;
			
			if (!getostypevalue (hp1, 2, &type))
				break;
			
			if (!setfiletype (&fs, type))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case setfilecreatorfunc: {
			tyfilespec fs;
			OSType creator;
			
			if (!getpathvalue (hp1, 1, &fs))
				break;
			
			flnextparamislast = true;
			
			if (!getostypevalue (hp1, 2, &creator))
				break;
		
			if (!setfilecreator (&fs, creator))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
			
		case getlabelindexfunc: // 2006-04-24 creedon
			return (getlabelindexverb (hp1, v));
		
		case setlabelindexfunc: // 2006-04-24 creedon
			return (setlabelindexverb (hp1, v));
		
		case getlabelnamesfunc: // 2006-04-24 creedon
			return (getlabelnamesverb (hp1, v));
	#endif

		case folderfrompathfunc:
			return (folderfrompathverb (hp1, v));
		

		case getsystempathfunc: {
		
			byte bsvol [258];		// 2
			byte bsfolder [258];	// 6
			tyfilespec fs;
			
			if (!langcheckparamcount (hp1, 0)) /*no parameters expected*/
				break;
			
			#ifdef flsystem6
			
				filegetpath (filegetsystemvnum (), bs);
				
				return (setstringvalue (bs, v));
				
			#else
			
				setemptystring (bsvol);

			clearbytes ( &fs, sizeof ( fs ) );
			
			#ifdef MACVERSION
			
				ostypetostring ('pref', bsfolder);
				
			#endif
				
			#ifdef WIN95VERSION
			
				copyctopstring ("SYSTEM", bsfolder);
				
			#endif
			
			if (!getspecialfolderpath (bsvol, bsfolder, false, &fs))
				break;
			
			return (setfilespecvalue (&fs, v));
			
			#endif
			
			}
		
		case getspecialpathfunc: {
			bigstring bsvol, bsfolder;
			tyfilespec fs;
			boolean flcreate;
			
			if (!getstringvalue (hp1, 1, bsvol))
				break;
			
			if (!getstringvalue (hp1, 2, bsfolder)) 
				break;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hp1, 3, &flcreate))
				break;
			
			clearbytes ( &fs, sizeof ( fs ) );

			if (!getspecialfolderpath (bsvol, bsfolder, flcreate, &fs))
				break;
			
			return (setfilespecvalue (&fs, v));
			}
		
		case getmp3infofunc: {
			
			long seconds, bitrate, frequency, offset;
			tyfilespec fs;
			boolean fl, flvariablebitrate;
			
			if (!langcheckparamcount (hparam1, 6)) /*preflight before changing values*/
				return (false);
			
			clearbytes ( &fs, sizeof ( fs ) );

			if (!getpathvalue (hp1, 1, &fs))
				return (false);
			
			if (!fifopenfile (&fs, (long) currentprocess))
				return (false);
			
			fl = getmp3info (&fs, &seconds, &bitrate, &frequency, &offset, &flvariablebitrate);

			if (!fifclosefile (&fs))
				return (false);
			
			if (!fl)
				return (false);
			
			if (!langsetlongvarparam (hp1, 2, seconds))
				return (false);
			
			if (!langsetlongvarparam (hp1, 3, bitrate))
				return (false);
			
			if (!langsetlongvarparam (hp1, 4, frequency))
				return (false);
			
			if (!langsetlongvarparam (hp1, 5, offset))
				return (false);
			
			if (!langsetbooleanvarparam (hp1, 6, flvariablebitrate))
				return (false);
			
			return (true);
			}
		
		case readwholefilefunc: // 2006-04-11 aradke
		
			return (readwholefileverb (hparam1, v));
			
		case getposixpathfunc: // 2006-10-07 creedon
		
			return ( getposixpathverb ( hp1, v ) );
		
		#ifdef WIN95VERSION
		
			case newaliasfunc:
			case filefollowaliasfunc:
			case filegeticonposfunc:
			case fileseticonposfunc: 
			case setshortversionfunc: 
			case setlongversionfunc: 
			case filesetcommentfunc:
			case filegetlabelfunc:
			case filesetlabelfunc:
			case unmountvolumefunc: 	
			case mountservervolumefunc:
			case volumeejectfunc:
			case setfiletypefunc:
			case setfilecreatorfunc:
			
		#endif
		
		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			break;
			
		} // switch
	
	return (false);
	
	} // filefunctionvalue


static boolean rezfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	register hdltreenode hp1 = hparam1;
	register tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /*by default, rez functions return false*/
	
	switch (token) {
	
	#ifdef MACVERSION
	
			case rezgetresourcefunc:
				return (getresourceverb (hp1, false, v));
			
			case rezputresourcefunc:
				return (putresourceverb (hp1, false, v));
			
			case rezgetnamedresourcefunc:
				return (getresourceverb (hp1, true, v));
			
			case rezputnamedresourcefunc:
				return (putresourceverb (hp1, true, v));
			
			case rezcountrestypesfunc:
				return (countrestypesverb (hp1, v));
			
			case rezgetnthrestypefunc:
				return (getnthrestypeverb (hp1, v));
			
			case rezcountresourcesfunc:
				return (countresourcesverb (hp1, v));
			
			case rezgetnthresourcefunc:
				return (getnthresourceverb (hp1, v));
			
			case rezgetnthresinfofunc:
				return (getnthresinfoverb (hp1, v));
			
			case rezresourceexistsfunc:
				return (resourceexistsverb (hp1, false, v));
			
			case reznamedresourceexistsfunc:
				return (resourceexistsverb (hp1, true, v));
			
			case rezdeleteresourcefunc:
				return (deleteresourceverb (hp1, false, v));
			
			case rezdeletenamedresourcefunc:
				return (deleteresourceverb (hp1, true, v));
			
			case rezgetresourceattrsfunc:
				return (getresourceattrsverb (hp1, false, v));
			
			case rezsetresourceattrsfunc:
				return (setresourceattrsverb (hp1, false, v));
			
	#endif	

		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			break;
			
		} // switch
	
	return (false);
	
	} // rezfunctionvalue


boolean fileinitverbs (void) {
	
	/*
	if you just changed or added some definitions in fileinitbuiltins, call
	fileinstallbuiltins here.  rebuild, run the program, come back and change
	it to fileloadbuiltins, rebuild and go on...
	
	12/18/90 dmb: no longer save hash tables in program file, so we just 
	initialize the builtins directly.
	
	10/2/91 dmb: broke out resource verb into rez table
	
	2.1b5 dmb: use new loadfunctionprocessor verb
	*/
	
	if (!loadfunctionprocessor (idfileverbs, &filefunctionvalue))
		return (false);
	
	if (!loadfunctionprocessor (idrezverbs, &rezfunctionvalue))
		return (false);
	
	return (true);
	} /*fileinitverbs*/


boolean filestart (void) {

	/*
	6.1b15 AR: Initialize flsupportslargevolumes.

	Windows 95 OSR2: The GetDiskFreeSpaceEx function is available on Windows 95 systems
	beginning with OEM Service Release 2 (OSR2). 

	To determine whether GetDiskFreeSpaceEx is available, call the LoadLibrary
	or LoadLibraryEx function to load the KERNEL32.DLL file, then call the
	GetProcAddress function to obtain an address for GetDiskFreeSpaceEx.
	If GetProcAddress fails, or if GetDiskFreeSpaceEx fails with the
	ERROR_CALL_NOT_IMPLEMENTED code, use the GetDiskFreeSpace function
	instead of GetDiskFreeSpaceEx
	*/
	
#ifdef WIN95VERSION
	
	HMODULE hmodule;

	char kerneldllname[] = "kernel32.dll\0";
	char procname[] = "GetDiskFreeSpaceExA\0";

	hmodule = GetModuleHandle (kerneldllname);
 
	if (hmodule != nil) {
	
		adrGetDiskFreeSpaceEx = (tyGetDiskFreeSpaceEx) GetProcAddress (hmodule, procname);

 		if (adrGetDiskFreeSpaceEx != NULL)
 			flsupportslargevolumes = true;
		}
#endif

	return (true);
	}/*filestart*/

