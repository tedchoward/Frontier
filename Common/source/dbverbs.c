
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

/*
	4.1b4 dmb: new verbs based on ODB Engine API
*/

#include "frontier.h"
#include "standard.h"

#ifdef MACVERSION
#include <land.h>
#include "mac.h"
#endif

#include "ops.h"
#include "memory.h"
#include "error.h"
#include "file.h"
#include "resources.h"
#include "scrap.h"
#include "strings.h"
#include "launch.h"
#include "notify.h"
#include "shell.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langipc.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "tablestructure.h"
#include "process.h"
#include "processinternal.h"
#include "odbinternal.h"

/*
if we're generating cfm (powerpc), we're linking to an odb engine shared 
library, which has it's own globals. on 68k machines, we're staically linked,
so the odb calls mess with out global data. so we need to protect it.
*/

#ifdef usingsharedlibrary

	#define odbnewfile		odbNewFile
	#define odbopenfile		odbOpenFile
	#define odbsavefile		odbSaveFile
	#define odbclosefile	odbCloseFile
	#define odbdefined		odbDefined
	#define odbdelete		odbDelete
	#define odbgettype		odbGetType
	#define odbgetvalue		odbGetValue
	#define odbsetvalue		odbSetValue
	#define odbnewtable		odbNewTable
	#define odbcountitems	odbCountItems
	#define odbgetnthitem	odbGetNthItem
	#define odbgetmoddate	odbGetModDate
	#define odbdisposevalue	odbDisposeValue
	#define odbgeterror		odbGetError

#else

	/*Prototypes*/

	boolean odbnewfile (hdlfilenum fnum);
	
	boolean odbaccesswindow (WindowPtr w, odbref *odb);
	
	boolean odbopenfile (hdlfilenum fnum, odbref *odb, boolean flreadonly);
	
	boolean odbsavefile (odbref odb);
	
	boolean odbclosefile (odbref odb);
	
	boolean odbdefined (odbref odb, bigstring bspath);
	
	boolean odbdelete (odbref odb, bigstring bspath);
	
	boolean odbgettype (odbref odb, bigstring bspath, OSType *odbType);
	
	boolean odbgetvalue (odbref odb, bigstring bspath, odbValueRecord *value);
	
	boolean odbsetvalue (odbref odb, bigstring bspath, odbValueRecord *value);
	
	boolean odbnewtable (odbref odb, bigstring bspath);
	
	boolean odbcountitems (odbref odb, bigstring bspath, long *count);
	
	boolean odbgetnthitem (odbref odb, bigstring bspath, long n, bigstring bsname);
	
	boolean odbgetmoddate (odbref odb, bigstring bspath, unsigned long *date);
	
	boolean odbdisposevalue (odbref odb, odbValueRecord *value);
	
	
	/*Functions*/
	
	boolean odbnewfile (hdlfilenum fnum) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbNewFile (fnum);
		
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbaccesswindow (WindowPtr w, odbref *odb) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbAccessWindow (w, odb);
		
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbopenfile (hdlfilenum fnum, odbref *odb, boolean flreadonly) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbOpenFile (fnum, odb, flreadonly);
		
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbsavefile (odbref odb) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbSaveFile (odb);
		
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbclosefile (odbref odb) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbCloseFile (odb);
		
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbdefined (odbref odb, bigstring bspath) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbDefined (odb, bspath);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbdelete (odbref odb, bigstring bspath) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbDelete (odb, bspath);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbgettype (odbref odb, bigstring bspath, OSType *odbType) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbGetType (odb, bspath, odbType);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbgetvalue (odbref odb, bigstring bspath, odbValueRecord *value) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbGetValue (odb, bspath, value);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbsetvalue (odbref odb, bigstring bspath, odbValueRecord *value) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbSetValue (odb, bspath, value);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbnewtable (odbref odb, bigstring bspath) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbNewTable (odb, bspath);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbcountitems (odbref odb, bigstring bspath, long *count) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbCountItems (odb, bspath, count);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbgetnthitem (odbref odb, bigstring bspath, long n, bigstring bsname) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbGetNthItem (odb, bspath, n, bsname);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbgetmoddate (odbref odb, bigstring bspath, unsigned long *date) {

		boolean fl;
		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		fl = odbGetModDate (odb, bspath, date);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (fl);
		}

	boolean odbdisposevalue (odbref odb, odbValueRecord *value) {

		hdlthreadglobals htg = getcurrentthreadglobals ();
		
		copythreadglobals (htg);
		
		swapinthreadglobals (nil);
		
		odbDisposeValue (odb, value);
				
		cancoonglobals = nil;
		
		swapinthreadglobals (htg);
		
		return (true);
		}

	#define odbgeterror		odbGetError

#endif


typedef struct tyodblistrecord {
	
	struct tyodblistrecord **hnext;
	
	tyfilespec fs;
	
	hdlfilenum fref;
	
	boolean flreadonly;
	
	odbref odb;
	} tyodbrecord, *ptrodbrecord, **hdlodbrecord;


static hdlodbrecord hodblist = nil;


typedef enum tydbtoken { /*verbs that are processed by db*/
	
	newfunc,
	
	openfunc,
	
	savefunc,
	
	closefunc,
	
	definedfunc,
	
	/*
	gettypefunc,
	*/
	
	getvaluefunc,
	
	setvaluefunc,
	
	deletefunc,
	
	newtablefunc,
	
	istablefunc,
	
	countitemsfunc,
	
	getnthitemfunc,
	
	getmoddatefunc,
	
	ctdbverbs
	} tydbtoken;


static boolean odberror (boolean flresult) {
	
	bigstring bserror;
	
	if (flresult)
		return (false);
	
	odbgeterror (bserror);
	
	langerrormessage (bserror);
	
	return (true);
	} /*odberror*/

	
static boolean getodbparam (hdltreenode hparam1, short pnum, hdlodbrecord *hodbrecord) {

	hdlodbrecord hodb;
	tyfilespec fs;
	tyfilespec * ptrfs;
	
	ptrfs = &fs; 

	if (!getfilespecvalue (hparam1, pnum, ptrfs))
		return (false);
	
	for (hodb = hodblist; hodb != nil; hodb = (**hodb).hnext) {
		
		if (equalfilespecs (&(**hodb).fs, ptrfs)) {
			
			*hodbrecord = hodb;
			
			return (true);
			}
		}
	
	lang2paramerror (dbnotopenederror, bsfunctionname, fsname(ptrfs));
	
	return (false);
	} /*getodbparam*/


static boolean getodbvalue (hdltreenode hparam1, short pnum, tyodbrecord *odb, boolean flreadonly) {

	hdlodbrecord hodb;
	
	if (!getodbparam (hparam1, pnum, &hodb))
		return (false);
	
	*odb = **hodb;
	
	if ((*odb).flreadonly && !flreadonly) {
		
		lang2paramerror (dbopenedreadonlyerror, bsfunctionname, fsname(&(*odb).fs));
		
		return (false);
		}
	
	return (true);
	} /*getodbvalue*/


static boolean dbclosefile (hdlodbrecord hodb) {
	
	if (!odbclosefile ((**hodb).odb))
		return (false);
	
	closefile ((**hodb).fref);
	
	listunlink ((hdllinkedlist) hodblist, (hdllinkedlist) hodb);
	
	disposehandle ((Handle) hodb);
	
	return (true);
	} /*dbclosefile*/


boolean dbcloseallfiles (long refcon) {

	return (true);
	} /*dbcloseallfiles*/


#if 0

static ptrfilespec fsfind;

static WindowPtr wfound;

static boolean odbfindfilevisit (WindowPtr w) {
	
	/*
	if the filespec for the given window is the one we're looking for, 
	select that window and stop visiting
	*/
	
	tyfilespec fs;
	
	windowgetfspec (w, &fs);
	
	if (equalfilespecs (fsfind, &fs)) {
		
		wfound = w;
		
		return (false);
		}
	
	return (true);
	} /*odbfindfilevisit*/

#endif


static boolean dbnewverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	4.1b5 dmb: new verb
	*/
	
	tyodbrecord odbrec;
	boolean fl;
	
	flnextparamislast = true;
	
	if (!getfilespecvalue (hparam1, 1, &odbrec.fs))
		return (false);
	
	shellpushdefaultglobals (); /*so that config is correct*/
	
	fl = opennewfile (&odbrec.fs, config.filecreator, config.filetype, &odbrec.fref);
	
	shellpopglobals ();
	
	if (!fl)
		return (false);
	
	fl = odbnewfile (odbrec.fref);
	
	closefile (odbrec.fref);
	
	if (odberror (fl)) {
		
		deletefile (&odbrec.fs);
		
		return (false);
		}
	
	return (setbooleanvalue (true, vreturned));
	} /*dbnewverb*/


static boolean dbopenverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	4.1b5 dmb: added ability to access already-open root in Frontier
	*/
	
	tyodbrecord odbrec;
	hdlodbrecord hodb;
	WindowPtr w;
	
	odbrec.fref = 0;
	
	if (!getfilespecvalue (hparam1, 1, &odbrec.fs))
		return (false);
	
	flnextparamislast = true;
	
	if (!getbooleanvalue (hparam1, 2, &odbrec.flreadonly))
		return (false);
	
	w = shellfindfilewindow (&odbrec.fs);
	
	if (w != nil) {
		
		if (odberror (odbaccesswindow (w, &odbrec.odb)))
			return (false);
		
		/*fref remains zero, so unwanted closefiles aren't a problem*/
		}
	else {
		
		if (!openfile (&odbrec.fs, &odbrec.fref, odbrec.flreadonly))
			return (false);
		
		if (odberror (odbopenfile (odbrec.fref, &odbrec.odb, odbrec.flreadonly))) {
			
			closefile (odbrec.fref);
			
			return (false);
			}
		}
	
	if (!newfilledhandle (&odbrec, sizeof (odbrec), (Handle *) &hodb)) {
		
		odbclosefile (odbrec.odb);
		
		closefile (odbrec.fref);
		
		return (false);
		}
	
	listlink ((hdllinkedlist) hodblist, (hdllinkedlist) hodb);
	
	return (setbooleanvalue (true, vreturned));
	} /*dbopenverb*/


static boolean dbsaveverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyodbrecord odbrec;
	
	flnextparamislast = true;
	
	if (!getodbvalue (hparam1, 1, &odbrec, false))
		return (false);
	
	return (setbooleanvalue (odbsavefile (odbrec.odb), vreturned));
	} /*dbsaveverb*/


static boolean dbcloseverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	hdlodbrecord hodb;
	
	flnextparamislast = true;
	
	if (!getodbparam (hparam1, 1, &hodb))
		return (false);
	
	return (setbooleanvalue (dbclosefile (hodb), vreturned));
	} /*dbcloseverb*/


static boolean dbdefinedverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	/*
	4.1b5 dmb: new verb
	*/
	
	tyodbrecord odbrec;
	bigstring bsaddress;
	boolean fl;
	
	if (!getodbvalue (hparam1, 1, &odbrec, true))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	fl = odbdefined (odbrec.odb, bsaddress);
	
	return (setbooleanvalue (fl, vreturned));
	} /*dbdefinedverb*/


static boolean dbgetvalueverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	/*
	4.1.1b1 dmb: fixed memory leak; push value on temp stack
	
	5.0b17 dmb: use pushtmpstackvalue to put external types returned from 
	the other odb onto our temp stack

	5.0.1 dmb: but only pushtmpstack if heapallocated
	*/
	
	tyodbrecord odbrec;
	bigstring bsaddress;
	odbValueRecord value;
	tyvaluetype type;
	
	if (!getodbvalue (hparam1, 1, &odbrec, true))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	if (odberror (odbgetvalue (odbrec.odb, bsaddress, &value)))
		return (false);
	
	type = langexternalgetvaluetype (value.valuetype);
	
	if (type == (tyvaluetype) -1) {
	
		return (setbinaryvalue (value.data.binaryvalue, value.valuetype, vreturned));
		}
	else {
		
		initvalue (vreturned, type);
		
		(*vreturned).data.binaryvalue = value.data.binaryvalue;
		
		if (langheapallocated (vreturned, nil))
			pushtmpstackvalue (vreturned); //5.0b17
		
		return (true);
		
		/*
		initvalue (&val, type);
		
		val.data.binaryvalue = value.data.binaryvalue;
		
		return (copyvaluerecord (val, vreturned));
		*/
		}
	} /*dbgetvalueverb*/


static boolean dbsetvalueverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	tyodbrecord odbrec;
	bigstring bsaddress;
	odbValueRecord value;
	tyvaluerecord val;
	boolean flerror;
	
	if (!getodbvalue (hparam1, 1, &odbrec, false))
		return (false);
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	flnextparamislast = true;
	
	if (!getparamvalue (hparam1, 3, &val))
		return (false);
	
	if (!copyvaluedata (&val))
		return (false);
	
	value.valuetype = (odbValueType) langexternalgettypeid (val);
	
	/*
	if (val.valuetype == binaryvaluetype)
		pullfromhandle (val.data.binaryvalue, 0L, sizeof (value.valuetype), &value.valuetype);
	*/
	
	value.data.binaryvalue = val.data.binaryvalue; /*largest field covers everything*/
	
	flerror = odberror (odbsetvalue (odbrec.odb, bsaddress, &value));
	
	disposevaluerecord (val, false);
	
	if (flerror)
		return (false);

	return (setbooleanvalue (true, vreturned));
	} /*dbsetvalueverb*/


static boolean dbdeleteverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyodbrecord odbrec;
	bigstring bsaddress;
	
	if (!getodbvalue (hparam1, 1, &odbrec, false))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	if (odberror (odbdelete (odbrec.odb, bsaddress)))
		return (false);
	
	return (setbooleanvalue (true, vreturned));
	} /*dbdeleteverb*/


static boolean dbnewtableverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyodbrecord odbrec;
	bigstring bsaddress;
	
	if (!getodbvalue (hparam1, 1, &odbrec, false))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	if (odberror (odbnewtable (odbrec.odb, bsaddress)))
		return (false);

	return (setbooleanvalue (true, vreturned));
	} /*dbnewtableverb*/


static boolean dbistableverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyodbrecord odbrec;
	bigstring bsaddress;
	OSType odbtype;
	
	if (!getodbvalue (hparam1, 1, &odbrec, true))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	if (odberror (odbgettype (odbrec.odb, bsaddress, &odbtype)))
		return (false);
	
	return (setbooleanvalue (odbtype == tableT, vreturned));
	} /*dbistableverb*/


static boolean dbcountitemsverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyodbrecord odbrec;
	bigstring bsaddress;
	long ctitems;
	
	if (!getodbvalue (hparam1, 1, &odbrec, true))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	if (odberror (odbcountitems (odbrec.odb, bsaddress, &ctitems)))
		return (false);
	
	return (setlongvalue (ctitems, vreturned));
	} /*dbcountitemsverb*/


static boolean dbgetnthitemverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyodbrecord odbrec;
	bigstring bsaddress;
	bigstring bsname;
	long n;
	
	if (!getodbvalue (hparam1, 1, &odbrec, true))
		return (false);
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 3, &n))
		return (false);
	
	if (odberror (odbgetnthitem (odbrec.odb, bsaddress, n, bsname)))
		return (false);
	
	return (setstringvalue (bsname, vreturned));
	} /*dbgetnthitemverb*/


static boolean dbgetmoddateverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyodbrecord odbrec;
	bigstring bsaddress;
	unsigned long moddate;
	
	if (!getodbvalue (hparam1, 1, &odbrec, true))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsaddress))
		return (false);
	
	if (odberror (odbgetmoddate (odbrec.odb, bsaddress, &moddate)))
		return (false);
	
	return (setdatevalue (moddate, vreturned));
	} /*dbgetmoddateverb*/


static boolean dbfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	4.1b4 dmb: new verb set based on odbEngine
	
	5.0b17 dmb: use swapinthreadglobals (nil) for all our odb calls to protect ours
	*/
	
	register hdltreenode hp1 = hparam1;
	register tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	switch (token) {
		
		case newfunc:
			return (dbnewverb (hp1, v));
		
		case openfunc:
			return (dbopenverb (hp1, v));
		
		case savefunc:
			return (dbsaveverb (hp1, v));
		
		case closefunc:
			return (dbcloseverb (hp1, v));
		
		case definedfunc:
			return (dbdefinedverb (hp1, v));
		
		case getvaluefunc:
			return (dbgetvalueverb (hp1, v));
		
		case setvaluefunc:
			return (dbsetvalueverb (hp1, v));
		
		case deletefunc:
			return (dbdeleteverb (hp1, v));
		
		case newtablefunc:
			return (dbnewtableverb (hp1, v));
		
		case istablefunc:
			return (dbistableverb (hp1, v));
		
		case countitemsfunc:
			return (dbcountitemsverb (hp1, v));
		
		case getnthitemfunc:
			return (dbgetnthitemverb (hp1, v));
		
		case getmoddatefunc:
			return (dbgetmoddateverb (hp1, v));
		
		default:
			return (false);
		}
	} /*dbfunctionvalue*/


boolean dbinitverbs (void) {
	
	if (!loadfunctionprocessor (iddbverbs, &dbfunctionvalue))
		return (false);
	
	if (!newclearhandle (sizeof (tyodbrecord), (Handle *) &hodblist))
		return (false);
	
	return (true);
	} /*dbinitverbs*/




