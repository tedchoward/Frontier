
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

#include "strings.h"
#include "osaparseaete.h"


static byte *aete;


static void grabbytes (long ctbytes, void *pbytes) {
	
	BlockMove (aete, pbytes, ctbytes);
	
	aete += ctbytes;
	} /*grabbytes*/


static short getshort (void) {
	
	short n;
	
	grabbytes (2, &n);
	
	return (n);
	} /*getshort*/


static void getstring (bigstring bs) {
	
	grabbytes (1, bs);
	
	grabbytes (stringlength (bs), bs + 1);
	} /*getstring*/


static OSType getstring4 (void) {
	
	OSType x;
	
	grabbytes (4, &x);
	
	return (x);
	} /*getstring4*/


static void getname (bigstring bsname) {
	
	short ixspace = 1;
	
	getstring (bsname);
	
	alllower (bsname);
	
	while (scanstring (chspace, bsname, &ixspace)) {
		
		deletestring (bsname, ixspace, 1);
		
		if (ixspace > stringlength (bsname))
			break;
		
		bsname [ixspace] = toupper (bsname [ixspace]);
		}
	} /*getname*/


static void skipstring (void) {
	
	bigstring bs;
	
	getstring (bs);
	} /*skipstring*/


static void skipbytes (short n) {
	
	aete += n;
	} /*skipbytes*/


static void align (void) {
	
	if (odd ((long) aete))
		++aete;
	} /*align*/


static boolean parseParams (AEKeyword eventkey, bigstring bsparam) {
	
	short ctparams = getshort ();
	OSType key;
	short ctremaining = ctparams;
	short flags;
	
	while (--ctremaining >= 0) {
		
		getname (bsparam); // param name
		
		align ();
		
		key = getstring4 ();
		
		skipbytes (4); // param type
		
		skipstring ();
		
		align ();
		
		flags = getshort (); // param flags
		
		if (key == eventkey) { /*the one caller seeks*/
			
			if ((ctparams == 1) && (flags >= 0)) /*only extra param, not optional*/
				setemptystring (bsparam);
			
			return (true);
			}
		}
	
	setemptystring (bsparam);
	
	return (false);
	} /*parseParams*/


static boolean parseEvents (AEEventClass eventclass, AEEventID eventid, bigstring bsevent) {
	
	/*
	2.1b2 dmb: when event is found, return just before parsing parameters, so 
	their position in the aete can be saved & restored
	*/
	
	short ctevents = getshort ();
	OSType class, id, paramtype;
	bigstring bsparam;
	
	while (--ctevents >= 0) {
		
		getname (bsevent);
		
		skipstring (); // comment
		
		align ();
		
		class = getstring4 ();
		
		id = getstring4 ();
		
		skipbytes (4); // reply type
		
		skipstring (); // reply description
		
		align ();
		
		skipbytes (2); // flags
		
		paramtype = getstring4 (); // direct param type
		
		skipstring (); // reply description
		
		align ();
		
		skipbytes (2); // reply flags
		
		if ((class == eventclass) && (id == eventid))
			return (true);
		
		parseParams (0, bsparam);
		}
	
	setemptystring (bsevent);
	
	return (false);
	} /*parseEvents*/


static void parseProperties (void) {
	
	short ctproperties = getshort ();
	bigstring bs;
	OSType prop;
	
	while (--ctproperties >= 0) {
		
		getname (bs); //property name
		
		align ();
		
		prop = getstring4 ();
		
		skipbytes (4); // property class
		
		getstring (bs); // property description
		
		align ();
		
		skipbytes (2);
		}
	} /*parseProperties*/


static void parseElements (void) {
	
	short ctelements = getshort ();
	OSType elem;
	
	while (--ctelements >= 0) {
		
		short ctkeyforms;
		
		elem = getstring4 ();
		
		ctkeyforms = getshort ();
		
		while (--ctkeyforms >= 0)
			skipbytes (4); // ignore it
		}
	} /*parseElements*/


static void parseClasses (void) {
	
	short ctclasses = getshort ();
	OSType class;
	bigstring bsclass;
	
	while (--ctclasses >= 0) {
		
		getname (bsclass);
		
		align ();
		
		class = getstring4 ();
		
		skipstring (); // the class description
		
		align ();
		
		parseProperties ();
		
		parseElements ();
		}
	} /*parseClasses*/


static void parseComparisonOps (void) {
	
	short ctcomparisons = getshort ();
	
	while (--ctcomparisons >= 0) {
		
		skipstring ();
		
		align ();
		
		skipbytes (4);
		
		skipstring ();
		
		align ();
		}
	} /*parseComparisonOps*/


static void parseEnumerators (void) {
	
	short ctenumerators = getshort ();
	bigstring bsname;
	OSType id;
	
	while (--ctenumerators >= 0) {
		
		getname (bsname);
		
		align ();
		
		id = getstring4 ();
		
		skipstring ();
		
		align ();
		}
	} /*parseEnumerators*/


static void parseEnumerations (void) {
	
	short ctenumerations = getshort ();
	
	while (--ctenumerations >= 0) {
		
		skipbytes (4);
		
		parseEnumerators ();
		}
	} /*parseEnumerations*/


boolean osaparseaeteparamlist (Handle haete, long offset, AEKeyword key, bigstring bsparamname) {
	
	/*
	a direct translation of the UserTalk "parseAete" script, except that 
	we're specifically seeking the name of the one event specified
	*/
	
	if (haete == nil)
		return (false);
	
	aete = (byte *) *haete + offset;
	
	return (parseParams (key, bsparamname));
	} /*osaparseaete*/


boolean osaparseaete (Handle haete, AEEventClass class, AEEventID id, bigstring bseventname, long *paramoffset) {
	
	/*
	a direct translation of the UserTalk "parseAete" script, except that 
	we're specifically seeking the name of the one event specified
	*/
	
	short ctsuites;
	OSType suitecode;
	
	if (haete == nil)
		return (false);
	
	aete = (byte *) *haete;
	
	skipbytes (6); // skip up to suite array
	
	ctsuites = getshort ();
	
	while (--ctsuites >= 0) {
		
		skipstring (); // name of the suite
		
		skipstring (); // description
		
		align ();
		
		suitecode = getstring4 ();
		
		skipbytes (4); // skip up to event array
		
		if (parseEvents (class, id, bseventname)) {
			
			*paramoffset = aete - (byte *) *haete;
			
			return (true);
			}
		
		if (suitecode == class)
			break;
		
		if (ctsuites == 0) // no point in parsing remainder
			break;
		
		parseClasses ();
		
		parseComparisonOps ();
		
		parseEnumerations ();
		}
	
	return (false);
	} /*osaparseaete*/



