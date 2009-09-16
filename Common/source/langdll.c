
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

	#include "langxcmd.h"
	#include "MoreFilesX.h"
	
#endif

#include "memory.h"
#include "frontierconfig.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "file.h"
#include "font.h"
#include "kb.h"
#include "mouse.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "sounds.h"
#include "strings.h"
#include "frontierwindows.h"
#include "cancoon.h"
#include "shell.h"
#include "shellhooks.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langipc.h"
#ifdef WIN95VERSION
#include "langwinipc.h"
#include "FrontierWinMain.h"
#endif
#include "langsystem7.h"
#include "langtokens.h"
#include "oplist.h"
#include "BASE64.H"
#include "tablestructure.h"
#include "tableverbs.h"
#include "process.h"
#include "processinternal.h"
#include "kernelverbdefs.h"
#include "langdll.h"

#if defined(MACVERSION) && TARGET_RT_MAC_MACHO
#include "CallMachOFrameWork.h"
#endif

#define NEW_DLL_INTERFACE 1 /* 2002-11-03 AR: defined to enable new DLL interface */
//#undef NEW_DLL_INTERFACE


#ifdef NEW_DLL_INTERFACE

	/*
	#pragma message ("*********************** NEW_DLL_INTERFACE is ON ***********************")
	*/

	/* type definitions */
	
	#ifdef WIN95VERSION
		typedef HINSTANCE tydllsyshandle;
	#endif
	#ifdef MACVERSION
		typedef CFragConnectionID tydllsyshandle;
	#endif

	#define ctprocinfohashbuckets 29 /* should be a prime number */

	typedef struct typrocinfostruct *typrocinfoptr, **typrocinfohandle; /* forward declaration */
	
	typedef struct typrocinfostruct {
	
		typrocinfohandle hashlink;					/* handle linking to next node in hash bucket */
		
		long ctparams;								/* number of params expected by the proc */
		
		tyvaluetype paramtypes[maxdllparams];		/* types of params expected by the proc */

		tyvaluetype resulttype;						/* type of result value returned by the proc */

		tyDLLEXTROUTINE procaddress;				/* pointer to the proc in the library */
		
		#if MACVERSION && !TARGET_API_MAC_CARBON
			RoutineDescriptor moduledesc;			/* needed for calling the proc on Mac OS Classic */
			UniversalProcPtr moduleUPP;
		#endif
		
		bigstring bsprocname;						/* name of the proc (null-terminated pascal string!) */

		} typrocinfostruct;
	
	typedef struct tydllinfostruct *tydllinfoptr, **tydllinfohandle; /* forward declaration */
	
	typedef struct tydllinfostruct {

		tydllinfohandle hnextdll;								/* if stay-resident, handle to next library in linked list */
		
		tyfilespec fs;											/* file path of library */
		
		tydllsyshandle hdllsyshandle; 							/* platform-specific handle returned by system function for loading DLL */
		
		Handle hres;											/* temp handle to ProcInfo resource */
	
		char *resdata;											/* temp pointer to data in ProcInfo resource */
		
		long ctreferences;										/* number of calls into library currently under way */
		
		boolean flunload;										/* set flag to unload library when ctreferences drops to nil */

		typrocinfohandle hashbucket[ctprocinfohashbuckets];		/* array of hash buckets for procinfo structs */
		
		} tydllinfostruct;
	
	/* static variables */
	
	static tydllinfohandle loadeddlls; /* linked list of currently loaded stay-resident DLLs */

	static XDLLProcTable *dllcallbacks; /* global pointer to array of callback functions */

#else

	typedef struct tydllmoduleinfo {
		
		long ctparams;
		
		tyvaluetype paramtypes [maxdllparams];
		
		tyvaluetype resulttype;
		
		Handle moduleHandle;

		#ifdef MACVERSION
			RoutineDescriptor moduleDesc;
			UniversalProcPtr moduleUPP;
			Handle hresdata; 
		#endif

		char * pdata;

		tyDLLEXTROUTINE procAddress;
		} tydllmoduleinfo;

#endif /* NEW_DLL_INTERFACE */


/* External Reference functions */
	pascal boolean odbUpdateOdbref (WindowPtr w, odbRef odb);

	boolean odbnewfile (hdlfilenum fnum);
	boolean odbaccesswindow (WindowPtr w, odbRef *odb);
	boolean odbopenfile (hdlfilenum fnum, odbRef *odb, boolean flreadonly);
	boolean odbsavefile (odbRef odb);
	boolean odbclosefile (odbRef odb);
	boolean odbdefined (odbRef odb, bigstring bspath);
	boolean odbdelete (odbRef odb, bigstring bspath);
	boolean odbgettype (odbRef odb, bigstring bspath, OSType *odbType);
	boolean odbgetvalue (odbRef odb, bigstring bspath, odbValueRecord *value);
	boolean odbsetvalue (odbRef odb, bigstring bspath, odbValueRecord *value);
	boolean odbnewtable (odbRef odb, bigstring bspath);
	boolean odbcountitems (odbRef odb, bigstring bspath, long *count);
	boolean odbgetnthitem (odbRef odb, bigstring bspath, long n, bigstring bsname);
	boolean odbgetmoddate (odbRef odb, bigstring bspath, unsigned long *date);
	boolean odbdisposevalue (odbRef odb, odbValueRecord *value);
	extern pascal void odbGetError (bigstring bs);

#ifdef MACVERSION

	enum {
		 uppdllcallProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(tydllparamblock *)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(XDLLProcTable *)))
		 };

#endif


Handle xCALLBACK extfrontierReAlloc (Handle h, long sz) {
	grabthreadglobals ();

	SetHandleSize (h, sz);

	releasethreadglobals ();

	return (h);
	} /*extfrontierReAlloc*/
	

Handle xCALLBACK extfrontierAlloc (long sz) {
	Handle h;

	grabthreadglobals ();

	h = NewHandle(sz);

	releasethreadglobals ();

	return (h);
	} /*extfrontierAlloc*/


char * xCALLBACK extfrontierLock (Handle h) {
	char * res;

	grabthreadglobals ();

	#ifdef MACVERSION
		HLock (h);

		res = (char *) *h;
	#endif

	#ifdef WIN95VERSION
		res = frontierLock (h);
	#endif

	releasethreadglobals ();

	return (res);
	} /*extfrontierLock*/


void xCALLBACK extfrontierFree (Handle h) {
	grabthreadglobals ();

	DisposeHandle (h);

	releasethreadglobals ();
	} /*extfrontierFree*/


long xCALLBACK extfrontierSize (Handle h) {
	long res;

	grabthreadglobals ();

	res = GetHandleSize(h);

	releasethreadglobals ();

	return (res);
	} /*extfrontierSize*/


void xCALLBACK extfrontierUnlock (Handle h) {
	grabthreadglobals ();

	HUnlock (h);

	releasethreadglobals ();
	} /*extfrontierUnlock*/


static boolean convertodbtotyval (odbValueRecord *odbval, tyvaluerecord * val) {
	tyvaluetype type;
	type = langexternalgetvaluetype ((OSType) (*odbval).valuetype);
	
	if (type == -1)	/*no match; must have been a binary value*/
		type = binaryvaluetype;
	
	initvalue (val, type);
	
	val->data.binaryvalue = (*odbval).data.binaryvalue;

	return (true);
	} /*convertodbtotyval*/


static boolean converttyvaltoodb (tyvaluerecord * val, odbValueRecord *odbval) {
	tyvaluerecord newval;

	if (!copyvaluerecord (*val, &newval))
		return (false);
	
	if (!copyvaluedata (&newval))
		return (false);
	
	exemptfromtmpstack (&newval);
	
	(*odbval).valuetype = (odbValueType) langexternalgettypeid (newval);
	
	/*
	if (val.valuetype == binaryvaluetype)
		pullfromhandle (val.data.binaryvalue, 0L, sizeof (typeid), &(*value).valuetype);
	*/
	
	(*odbval).data.binaryvalue = newval.data.binaryvalue; /*largest field covers everything*/

	return (true);
	} /*converttyvaltoodb*/


odbBool xCALLBACK extOdbNewListValue (odbRef odb, odbValueRecord *value, odbBool flRecord) {
	odbBool res;
	hdllistrecord hlist;

	res = false;

	grabthreadglobals ();

	if (opnewlist (&hlist, flRecord)) {
	
		if (flRecord) {
			value->valuetype = odb_recordvaluetype;
			value->data.recordvalue = (Handle) hlist;
			}
		else {
			value->valuetype = odb_listvaluetype;
			value->data.listvalue = (Handle) hlist;
			}

		if (hlist != NULL)
			res = true;
		}

	releasethreadglobals ();

	return (res);
	} /*extOdbNewListValue*/

				
odbBool xCALLBACK extOdbGetListCount (odbRef odb, odbValueRecord *value, long * cnt) {
#pragma unused (odb)
	odbBool res;

	res = false;

	grabthreadglobals ();

	if (((value->valuetype == odb_recordvaluetype) || (value->valuetype == odb_listvaluetype)) && (value->data.listvalue != NULL)) {
		*cnt = opcountlistitems ((hdllistrecord) value->data.listvalue);

		res = true;
		}

	releasethreadglobals ();

	return (res);
	} /*extOdbGetListCount*/


odbBool xCALLBACK extOdbDeleteListValue (odbRef odb, odbValueRecord *value, long idx, char * recordname) {
#pragma unused (odb)
	odbBool res;

	res = false;

	grabthreadglobals ();

	if (((value->valuetype == odb_recordvaluetype) || (value->valuetype == odb_listvaluetype)) && (value->data.listvalue != NULL)) {
		res = opdeletelistitem ((hdllistrecord) value->data.listvalue, (short) idx, (ptrstring)recordname);
		}

	releasethreadglobals ();

	return (res);
	} /*extOdbDeleteListValue*/


odbBool xCALLBACK extOdbSetListValue (odbRef odb, odbValueRecord *value, long idx, char * recordname, odbValueRecord *valueData) {
	odbBool res;
	tyvaluerecord val;

	res = false;

	grabthreadglobals ();

	if (((value->valuetype == odb_recordvaluetype) || (value->valuetype == odb_listvaluetype)) && (value->data.listvalue != NULL)) {

		convertodbtotyval (valueData, &val);

		res = setnthlistval ((hdllistrecord) value->data.listvalue, idx, (ptrstring) recordname, &val);
		}

	releasethreadglobals ();

	return (res);
	} /*extOdbSetListValue*/


odbBool xCALLBACK extOdbGetListValue (odbRef odb, odbValueRecord *value, long idx, char * recordname, odbValueRecord *valueReturn) {
#pragma unused (odb)
	odbBool res;
	tyvaluerecord valret;

	res = false;

	grabthreadglobals ();

	if (((value->valuetype == odb_recordvaluetype) || (value->valuetype == odb_listvaluetype)) && (value->data.listvalue != NULL)) {
		res = getnthlistval ((hdllistrecord) value->data.listvalue, idx, (ptrstring) recordname, &valret);

		if (res)
			res = converttyvaltoodb (&valret, valueReturn);
		}

	releasethreadglobals ();

	return (res);
	} /*extOdbGetListValue*/


odbBool xCALLBACK extOdbAddListValue (odbRef odb, odbValueRecord *value, char * recordname, odbValueRecord *valueData) {
#pragma unused (odb)
	odbBool res;
	tyvaluerecord val;

	res = false;

	grabthreadglobals ();

	if (((value->valuetype == odb_recordvaluetype) || (value->valuetype == odb_listvaluetype)) && (value->data.listvalue != NULL)) {

		convertodbtotyval (valueData, &val);

		res = langpushlistval ((hdllistrecord) value->data.listvalue, (ptrstring) recordname, &val);
		}

	releasethreadglobals ();

	return (res);
	} /*extOdbAddListValue*/

//This global is so users of getcurrentroot do not have to do a close on the return value each time.
odbRef globalodb = NULL;

odbRef xCALLBACK extOdbGetCurrentRoot (void) {
	odbRef odb;

	grabthreadglobals ();

	if (globalodb == NULL)
		odbaccesswindow (NULL, &globalodb);
	else
		odbUpdateOdbref (NULL, globalodb);

	odb = globalodb;

	releasethreadglobals ();

	return (odb);
	} /*extOdbGetCurrentRoot*/


odbBool xCALLBACK extOdbNewFile (hdlfilenum h) {
	odbBool res;

	grabthreadglobals ();

	res = odbnewfile (h);

	releasethreadglobals ();

	return (res);
	} /*extOdbNewFile*/


odbBool xCALLBACK extOdbOpenFile (hdlfilenum h, odbRef *odb) {
	odbBool res;

	grabthreadglobals ();

	res = odbopenfile (h, odb, false);

	releasethreadglobals ();

	return (res);
	} /*extOdbOpenFile*/


odbBool xCALLBACK extOdbSaveFile (odbRef odb) {
	odbBool res;

	grabthreadglobals ();

	res = odbsavefile (odb);

	releasethreadglobals ();

	return (res);
	} /*extOdbSaveFile*/


odbBool xCALLBACK extOdbCloseFile (odbRef odb) {
	odbBool res;

	grabthreadglobals ();

	res = odbclosefile (odb);

	releasethreadglobals ();

	return (res);
	} /*extOdbCloseFile*/


odbBool xCALLBACK extOdbDefined (odbRef odb, bigstring bspath) {
	odbBool res;

	grabthreadglobals ();

	res = odbdefined (odb, bspath);

	releasethreadglobals ();

	return (res);
	} /*extOdbDefined*/


odbBool xCALLBACK extOdbDelete (odbRef odb, bigstring bspath) {
	odbBool res;

	grabthreadglobals ();

	res = odbdelete (odb, bspath);

	releasethreadglobals ();

	return (res);
	} /*extOdbDelete*/


odbBool xCALLBACK extOdbGetType (odbRef odb, bigstring bspath, OSType *type) {
	odbBool res;

	grabthreadglobals ();

	res = odbgettype (odb, bspath, type);

	releasethreadglobals ();

	return (res);
	} /*extOdbGetType*/


odbBool xCALLBACK extOdbCountItems (odbRef odb, bigstring bspath, long *count) {
	odbBool res;

	grabthreadglobals ();

	res = odbcountitems (odb, bspath, count);

	releasethreadglobals ();

	return (res);
	} /*extOdbCountItems*/


odbBool xCALLBACK extOdbGetNthItem (odbRef odb, bigstring bspath, long n, bigstring bsname) {
	odbBool res;

	grabthreadglobals ();

	res = odbgetnthitem (odb, bspath, n, bsname);

	releasethreadglobals ();

	return (res);
	} /*extOdbGetNthItem*/


odbBool xCALLBACK extOdbGetValue (odbRef odb, bigstring bspath, odbValueRecord *value) {
	odbBool res;

	grabthreadglobals ();

	res = odbgetvalue (odb, bspath, value);

	releasethreadglobals ();

	return (res);
	} /*extOdbGetValue*/


odbBool xCALLBACK extOdbSetValue (odbRef odb, bigstring bspath, odbValueRecord *value) {
	odbBool res;

	grabthreadglobals ();

	res = odbsetvalue (odb, bspath, value);

	releasethreadglobals ();

	return (res);
	} /*extOdbSetValue*/


odbBool xCALLBACK extOdbNewTable (odbRef odb, bigstring bspath) {
	odbBool res;

	grabthreadglobals ();

	res = odbnewtable (odb, bspath);

	releasethreadglobals ();

	return (res);
	} /*extOdbNewTable*/


odbBool xCALLBACK extOdbGetModDate (odbRef odb, bigstring bspath, unsigned long *date) {
	odbBool res;

	grabthreadglobals ();

	res = odbgetmoddate (odb, bspath, date);

	releasethreadglobals ();

	return (res);
	} /*extOdbGetModDate*/


void xCALLBACK extOdbDisposeValue (odbRef odb, odbValueRecord *value) {
	grabthreadglobals ();

	if (value->data.binaryvalue != NULL) {
		odbdisposevalue (odb, value);

		value->data.binaryvalue = NULL;
		}

	releasethreadglobals ();
	} /*extOdbDisposeValue*/


void xCALLBACK extOdbGetError (bigstring bs) {
	grabthreadglobals ();

	odbGetError (bs);

	releasethreadglobals ();
	} /*extOdbGetError*/


odbBool xCALLBACK extDoScript (char * script, long len, odbValueRecord *value) {
	tyvaluerecord val;
	Handle h;
	odbBool res;

	res = false; // until otherwise

	clearbytes (value, sizeof (*value)); // 5.1.4
	
	grabthreadglobals ();

	if (newfilledhandle (script, len,  &h)) {
		if (langrun (h, &val)) {

			exemptfromtmpstack (&val);
			
			(*value).valuetype = (odbValueType) langexternalgettypeid (val);
			
			/*
			if (val.valuetype == binaryvaluetype)
				pullfromhandle (val.data.binaryvalue, 0L, sizeof (typeid), &(*value).valuetype);
			*/
			
			(*value).data.binaryvalue = val.data.binaryvalue; /*largest field covers everything*/

			res = true;
			}
		}

	releasethreadglobals ();
	return (res);
	} /*extDoScript*/


odbBool xCALLBACK extDoScriptText (char * script, long len, Handle * text) {
	tyvaluerecord val;
	Handle h;
	odbBool res;

	res = false; // until otherwise

	*text = nil; // 5.1.4
	
	grabthreadglobals ();

	if (newfilledhandle (script, len,  &h)) {
		if (langrun (h, &val)) {

			if (coercetostring (&val)) {
				exemptfromtmpstack (&val);

				*text = val.data.binaryvalue;
			
				res = true;
				}
			}
		}

	releasethreadglobals ();
	
	return (res);
	} /*extDoScriptText*/


odbBool xCALLBACK extInvoke (bigstring bsscriptname, void * pDispParams, odbValueRecord * retval, boolean *flfoundhandler, unsigned int * errarg) {
#if MACVERSION
#	pragma unused (bsscriptname, pDispParams, flfoundhandler, errarg)
#endif
	boolean res;
	tyvaluerecord val;

	res = false; // until otherwise

	grabthreadglobals ();

	#ifdef WIN95VERSION
		convertodbtotyval (retval, &val);
		res = langwinipchandleCOM (bsscriptname, pDispParams, &val, flfoundhandler, errarg);
	#endif
	#ifdef MACVERSION
		res = false;
		setstringvalue (BIGSTRING ("\x29" "Invoke is not supported on this platform."), &val);
	#endif

	converttyvaltoodb (&val, retval);

	disposevaluerecord (val, false);

	releasethreadglobals ();
	
	return (res);
	} /*extInvoke*/


odbBool xCALLBACK extCoerce (odbValueRecord * odbval, odbValueType newtype) {
	boolean res;
	tyvaluerecord val;
	tyvaluetype newvaltype;

	res = false; // until otherwise

	grabthreadglobals ();

	convertodbtotyval (odbval, &val);

	newvaltype = langexternalgetvaluetype ((OSType) newtype);
	
	if (newvaltype == -1)	/*no match; must have been a binary value*/
		newvaltype = binaryvaluetype;

	if (coercevalue (&val, newvaltype)) {
		converttyvaltoodb (&val, odbval);

		res = true;
		}

	releasethreadglobals ();
	
	return (res);
	} /*extInvoke*/


odbBool xCALLBACK extCallScript (odbString bspath, odbValueRecord *vparams, odbValueRecord *value) {
	
	/*
	2002-10-13 AR: Run the script at bspath with the parameters supplied in vparams.
	This is an adaption of callscriptverb (langverbs.c) for the DLL callback interface.
	*/

	odbBool res = false;
	tyvaluerecord val;
	tyvaluerecord vret;

	grabthreadglobals ();

	if (((vparams->valuetype == odb_recordvaluetype) || (vparams->valuetype == odb_listvaluetype)) && (vparams->data.listvalue != NULL)) {

		convertodbtotyval (vparams, &val);

		if (langrunscript (bspath, &val, nil, &vret)) {

			exemptfromtmpstack (&vret);
			
			(*value).valuetype = (odbValueType) langexternalgettypeid (vret);
			
			(*value).data.binaryvalue = vret.data.binaryvalue; /*largest field covers everything*/

			res = true;
			}
		}

	releasethreadglobals ();

	return (res);
	} /* extCallScript */


odbBool xCALLBACK extCallScriptText (odbString bspath, odbValueRecord *vparams, Handle * text) {

	/*
	2002-10-13 AR: Like extCallScript, but coerce the result to a string and return it
	*/

	odbBool res = false;
	tyvaluerecord val;
	tyvaluerecord vret;

	grabthreadglobals ();

	if (((vparams->valuetype == odb_recordvaluetype) || (vparams->valuetype == odb_listvaluetype)) && (vparams->data.listvalue != NULL)) {

		convertodbtotyval (vparams, &val);

		if (langrunscript (bspath, &val, nil, &vret)) {

			if (coercetostring (&vret)) {
			
				exemptfromtmpstack (&vret);

				*text = vret.data.binaryvalue;
			
				res = true;
				}
			}
		}

	releasethreadglobals ();

	return (res);
	} /* extCallScriptText */


odbBool xCALLBACK extThreadYield (void) {
	
	/*
	2003-04-22 AR: Yield the processor to other threads. This is only really useful
	on the Mac. On Win32, DLL threads multitask preemptively anyway.
	
	If this function returns false, the caller should terminate processing and return
	to the kernel as soon as possible since the the current thread has been killed.
	*/
	
	odbBool res;

	grabthreadglobals ();
	
	res = langbackgroundtask(false);
	
	if (res) {
		res = !languserescaped(false);
		}

	releasethreadglobals ();

	return (res);
	} /* extThreadYield */


odbBool xCALLBACK extThreadSleep (long sleepticks) {
	
	/*
	2003-04-22 AR: Go to sleep for the indicated number of ticks (60ths of a second).
	
	If this function returns false, the caller should terminate processing and return
	to the kernel as soon as possible since the the current thread has been killed.
	*/

	odbBool res;

	grabthreadglobals ();
	
	res = processsleep (nil, sleepticks);
	
	if (res) {
		res = !languserescaped(false);
		}

	releasethreadglobals ();

	return (res);
	} /* extThreadSleep */


////// Bob: loaddllmodule should move into its own file, langdll.c. All
////// of these typedefs should be in langdll.h
////// Also, note that we will almost certainly want to implement the DLL
////// verb and API on the Mac, for PowerPC (and CFM-68K). I know that, on
////// the Mac, we can load DLLs (code fragments) from disk or memory.


static tyvaluetype gettypefromchar (char val) {

	tyvaluetype ret;

	switch (val)
		{
		case 'v':
		case 'V':
			ret = novaluetype;
			break;

		case 'i':
		case 'I':
			ret = longvaluetype;
			break;

		case 's':
		case 'S':
			ret = stringvaluetype;
			break;

		case 'l':
		case 'L':
			ret = listvaluetype;
			break;

		case 'r':
		case 'R':
			ret = recordvaluetype;
			break;

		default:
			ret = novaluetype;
			break;
		}

	return (ret);
	} /*gettypefromchar*/


#ifdef NEW_DLL_INTERFACE


static unsigned long procnamehashfunction (bigstring bs) {

	register unsigned long x;
	register unsigned long len = stringlength (bs);
	
	// #define	stringlength(bs) ((unsigned char)(bs)[0])
	
	len = (unsigned long) stringlength (bs);
	 
	if (len == 0)
		return (0);
	
	// #define getstringcharacter(bs,pos) bs[(pos)+1]
	
	x = getstringcharacter (bs, 0);
	
	x += getstringcharacter (bs, (len-1)/3);
	
	x += getstringcharacter (bs, (2*(len-1))/3);
	
	x += getstringcharacter (bs, len-1);
	
	return (x % ctprocinfohashbuckets);  /* results range 0..ctprocinfohashbuckets */
	} /*procnamehashfunction*/


static void addprocinfo (tydllinfohandle hdll, typrocinfohandle hprocinfo) {

	/*
	Insert proc info node at start of hash bucket
	*/

	long ixhashbucket;

	ixhashbucket = procnamehashfunction ((**hprocinfo).bsprocname);
	
	(**hprocinfo).hashlink = (**hdll).hashbucket[ixhashbucket];
	
	(**hdll).hashbucket[ixhashbucket] = hprocinfo;
	 
	return;
	} /*addprocinfo*/


static boolean locateprocinfo (tydllinfohandle hdll, bigstring bsprocname, typrocinfohandle *hprocinfoptr) {

	//
	// Look up the proc name in the hash table of procs
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//

	bigstring bs;
	typrocinfohandle hnomad;
	tyfilespec fs = (**hdll).fs;
	long ixhashbucket;

	//assert (hdll != nil && *hdll != nil);
	
	ixhashbucket = procnamehashfunction (bsprocname);
	
	hnomad = (**hdll).hashbucket[ixhashbucket];
	
	while (hnomad != nil) {
	
		if (comparestrings ((**hnomad).bsprocname, bsprocname) == 0) {
		
			*hprocinfoptr = hnomad;
			
			return (true);
			}
		
		hnomad = (**hnomad).hashlink;
		} // while
	
	getfsfile ( &fs, bs );
	
	lang2paramerror (cantfindprocinfofunctionerror, bsprocname,  bs );

	return (false);
	} // locateprocinfo


static void freeprocinfobuckets (tydllinfohandle hdll) {
	
	typrocinfohandle h, hnext;
	long k;
	
	for (k = 0; k < ctprocinfohashbuckets; k++) {
	
		h = (**hdll).hashbucket[k];
		
		while (h != nil) {
		
			hnext = (**h).hashlink;
			
			#if defined(MACVERSION) && TARGET_RT_MAC_MACHO
				disposemachofuncptr ((void *) (**h).procaddress);	
			#endif

			disposehandle ((Handle) h);
			
			h = hnext;
			}/*while*/
		
		(**hdll).hashbucket[k] = nil;
		}/*for*/
	
	return;
	} /*freeprocinfobuckets*/


static typrocinfohandle newprocinfo (char *pname, long lenname, char *pparams, long lenparams) {
	
	/*
	Create a new proc info handle from the given information
	*/
	
	typrocinfohandle hprocinfo;
	long k;
	
	/* Allocate and clear memory */
	
	if (!newclearhandle (sizeof (**hprocinfo), (Handle *)&hprocinfo))
		return (nil);

	/* Copy procname to a null-terminated pascal string */
	
	memcpy (stringbaseaddress ((**hprocinfo).bsprocname), pname, lenname);
	
	setstringlength((**hprocinfo).bsprocname, lenname);
	
	nullterminate((**hprocinfo).bsprocname);
	
	/* Set result type */
	
	(**hprocinfo).resulttype = gettypefromchar (pparams[0]);
	
	/* Set param count and types */
	
	(**hprocinfo).ctparams = lenparams - 1; /* subtract one for result type */

	for (k = 0; k < (**hprocinfo).ctparams; k++)
		(**hprocinfo).paramtypes[k] = gettypefromchar (pparams[k+1]);

	return (hprocinfo);
	} /*newprocinfo*/


static boolean loadprocinforesource (tydllinfohandle hdll) {

	//
	// Platoform-specific code for loading the library's ProcInfo resource
	//
	// Caller is responsible for setting langerror
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//

	#ifdef MACVERSION
	
		short resfile;
		Handle hRes;
		HFSUniStr255 resourceforkname;
		FSRef fsref;
		
		if (macgetfsref(&(**hdll).fs, &fsref) != noErr)
			return (false);
		
		FSGetResourceForkName(&resourceforkname);

		FSOpenFork(&fsref, resourceforkname.length, resourceforkname.unicode, fsRdWrPerm, &resfile);

		if (ResError() == noErr) {
			
			hRes = Get1NamedResource ('DATA', "\pProcInfo");

			if (hRes != NULL) {
			
				DetachResource (hRes);
				
				HLock (hRes);
				
				(**hdll).hres = hRes;
				
				(**hdll).resdata = *hRes;
				}
			
			FSCloseFork ( resfile );
			
			}
			
	#endif

	#ifdef WIN95VERSION
	
		HRSRC frh;
		HGLOBAL rh;

		frh = FindResource ((**hdll).hdllsyshandle, "ProcInfo", RT_RCDATA);

		if (frh != NULL) {
		
			rh = LoadResource ((**hdll).hdllsyshandle, frh);
			
			if (rh != NULL) {
			
				(**hdll).hres = (Handle) rh;
			
				(**hdll).resdata = LockResource (rh);
				}
			}
	#endif

	return ((**hdll).hres != NULL);
	} // loadprocinforesource


static void unloadprocinforesource (tydllinfohandle hdll) {

	/*
	Platoform-specific code for unloading the library's ProcInfo resource
	*/

#ifdef MACVERSION

	HUnlock ((**hdll).hres);

	disposehandle ((**hdll).hres);

#endif
	
	return;
	} /*unlockprocinforesource*/


static boolean parseprocinforesource (tydllinfohandle hdll, bigstring bsprocname, typrocinfohandle *hprocinfoptr) {
	
	/*
	Parse the library's ProcInfo resource
	
	Stay-resident DLLs: If bsprocname and hprocinfoptr are nil,
	build a hash table in hdll with information about all procs found in the DLL.
	
	Volatile DLLs: If bsprocname and hprocinfoptr are not nil,
	the caller wants us to just return info about the specified proc in hprocinfoptr.
	The caller is then responsible for disposing *hprocinfoptr.
	
	Here's all the info we published about the format of the ProcInfo resource:
	
		Procedure Entries in the Resource File
		<http://frontier.userland.com/stories/storyReader$1181>

		Program errors, including failures in Frontier, can occur
		if the format is not followed. Every line and procedure must
		be separated by a NULL, and the end of the data must have
		two NULLs. A semicolon indicates the start of a comment which
		will extend to the next NULL -- i.e. the remainder of the line.

		Each Frontier-visible procedure must have its own line which
		starts out with the return type and parameter types listed.
		The supported types are I for integer (a long value), S for
		string, and V for void (used only for the return type when
		the procedure has no result).

		In the example below, the procedure UpperCase has a result
		value of S (a string) and a single parameter of type S (string).
		The procedure Counter has no parameters and a return type of
		integer.

		#include
		ProcInfo RT_RCDATA
		BEGIN
		"; This is a comment to describe your library\0",
		"SS UpperCase ;Uppercase the string\0",
		"I Counter ;Count the next number\0",
		"\0\0"
		END
	
	2006-06-25 creedon: for Mac, FSRef-ized

 	*/

	typrocinfohandle hprocinfo;
	char *p;
	char *pparams;
	char *pprocname;
	long lenparams;
	long lenprocname;
	long lenwhitespace;
	boolean flresult = false;
	
	/* Load the ProcInfo resource from the DLL */
	
	if (!loadprocinforesource (hdll)) {
	
		tyfilespec fs = (**hdll).fs;
		bigstring bs;
		
		getfsfile ( &fs, bs );
		
		if (bsprocname != nil)
		
			lang2paramerror (cantfindprocinfoerror, bsprocname,  bs);
			
		else
			langparamerror (cantfindprocinfoloaderror, bs );
			
			
		return (false);
		}
	
	/* Parse the ProcInfo resource line by line */
	
	p = (**hdll).resdata;
	
	do {
		/* Get params at start of line */
		
		pparams = p;
		
		lenparams = strspn (p, "VISLRvislr");
		
		if (lenparams == 0 || lenparams > maxdllparams)
			goto nextline;
		
		p += lenparams;
		
		/* Jump over whitespace between params and procname */
		
		lenwhitespace = strspn (p, " \t");
		
		if (lenwhitespace == 0)
			goto nextline;
		
		p += lenwhitespace;

		/* Get procname, ends before semicolon or whitespace */
		
		pprocname = p;
		
		lenprocname = strcspn (p, "; \t");
		
		if (lenprocname == 0 || lenprocname > lenbigstring)
			goto nextline;
		
		/* We got a valid entry, deal with it as requested by the caller */
		
		if (bsprocname != nil) {
			
			/* Check whether it's the proc we are looking for */
			
			if (memcmp (stringbaseaddress (bsprocname), pprocname, lenprocname) == 0) {
			
				//assert (hprocinfoptr != nil);
					
				hprocinfo = newprocinfo (pprocname, lenprocname, pparams, lenparams);
				
				if (hprocinfo == nil)
					goto exit;
			
				*hprocinfoptr = hprocinfo;
				
				flresult = true; /* we found it and built the info handle */
				
				goto exit;
				}
			}
		else {
			
			/* Build the proc info handle and add it to the hash table */
			
			hprocinfo = newprocinfo (pprocname, lenprocname, pparams, lenparams);
			
			if (hprocinfo == nil)
				goto exit;
				
			addprocinfo (hdll, hprocinfo);		
			} 
		
	nextline:
		/* Advance ptr to beginning of next line */
		
		p += strlen (p) + 1;
	
		} while (*p != '\0'); /* Resource must end with two consecutive nil chars */

	flresult = true;
	
exit:
	/* Unload the ProcInfo resource (no-op on Win32) */
		
	unloadprocinforesource (hdll);
	
	return (flresult);	
	} /*parseprocinforesource*/


static void addlibrary (tydllinfohandle hdll) {

	/*
	Insert at head of linked-list of stay-resident libraries
	*/

	//assert (hdll != nil && *hdll != nil);
	
	(**hdll).hnextdll = loadeddlls;
	
	loadeddlls = hdll;

	return;
	} /*addlibrary*/


static void removelibrary (tydllinfohandle hdll) {

	/*
	Remove from linked-list of stay-resident libraries
	*/

	tydllinfohandle hnomad;
	tydllinfohandle hlastdll;
	
	//assert (loadeddlls != nil && *loadeddlls != nil);
	//assert (hdll != nil && *hdll != nil);
	
	if (loadeddlls == hdll) {
		loadeddlls = (**hdll).hnextdll;
		(**hdll).hnextdll = nil;
		return;
		}
	
	hlastdll = loadeddlls;
	hnomad = (**loadeddlls).hnextdll;
	
	while (hnomad) {
		
		if (hnomad == hdll) {
			(**hlastdll).hnextdll = (**hnomad).hnextdll;
			(**hnomad).hnextdll = nil;
			return;
			}
	
		hlastdll = hnomad;
		hnomad = (**hnomad).hnextdll;
		}/*while*/

	return;
	} /*removelibrary*/


static tydllinfohandle getlibrary (const ptrfilespec fs) {

	/*
	Search linked list of stay-resident libraries for one with the given file path
	*/

	tydllinfohandle hnomad = loadeddlls;

	while (hnomad) {

		if (equalfilespecs (&(**hnomad).fs, fs))
			return (hnomad);
		
		hnomad = (**hnomad).hnextdll;
		
		}/*while*/

	return (nil);	
	} /*getlibrary*/


static tydllinfohandle newlibrary (const ptrfilespec fs) {

	/*
	Allocate and clear a handle for the library
	*/
	
	tydllinfohandle hdll;
	
	if (!newclearhandle (sizeof (**hdll), (Handle *)&hdll))
		return (nil);
	
	(**hdll).fs = *fs;

	return (hdll);
	} /*newlibrary*/


static void freelibrary (tydllinfohandle hdll) {

	/*
	Free a handle for the library
	
	The caller is responsible for removing it from the linked list
	*/
	
	//assert (hdll == nil || (**hdll).hnextdll == nil);

	if (hdll) {
		
		freeprocinfobuckets (hdll);
	
		disposehandle ((Handle) hdll);
		}

	return;
	} /*freelibrary*/


static boolean openlibrary (tydllinfohandle hdll) {

	//
	// Platform-specific code for loading the library code into memory
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//

	tyfilespec fs = (**hdll).fs;

	#ifdef WIN95VERSION
	
		bigstring fn;
	
		#if (FRONTIERCOM == 1)
			filefrompath ((ptrstring) fsname (&fs), fn);
	
			nullterminate(fn);
	
			if (stricmp (stringbaseaddress(fn), "COMDLL.DLL") == 0) {
			
				(**hdll).hdllsyshandle = (tydllsyshandle) COMStartup(); /*** FIXME: make sure we deal properly with the COM DLL ***/
				
				return ((**hdll).hdllsyshandle != nil);
				}
		#endif
	
		copystring (fsname (&fs), fn);
		
		nullterminate(fn);
	
		(**hdll).hdllsyshandle = LoadLibrary (stringbaseaddress(fn));
	
	#endif
	
	#ifdef MACVERSION
	
		long response;
		OSErr err;
		CFragConnectionID connID;
		Ptr mainAddr;
		Str255 errName;
		
		err = Gestalt (gestaltCFMAttr, &response);	/* make sure we have the Code Fragment Manager (CFM) */
		
		if ((err != noErr) || (response & (1 << gestaltCFMPresent)) == 0)
			goto exit;
			
		bigstring bs;
		FSSpec fst;
		
		getfsfile (&fs, bs);
		
		if (macgetfsspec(&fs, &fst) != noErr)
			goto exit;
		
		err = GetDiskFragment (&fst, 0, kCFragGoesToEOF, bs, kReferenceCFrag, &connID, &mainAddr, errName);
		
		if (err != noErr)
			goto exit;
			
		(**hdll).hdllsyshandle = connID;
	
	exit:
	
	#endif
		
	if ((**hdll).hdllsyshandle == NULL) {
	
		bigstring bs;
		
		getfsfile ( &fs, bs );
	
		lang2paramerror (cantconnecttodllerror, bsfunctionname, bs );
		
		return (false);
		}
	
	return (true);
	
	} // openlibrary


static void closelibrary (tydllinfohandle hdll) {

	/*
	Platform-specific code for unloading the library code from memory
	*/

#ifdef WIN95VERSION

	#if (FRONTIERCOM == 1)
		if ((**hdll).hdllsyshandle == (tydllsyshandle) COMSYSModule()) /*** FIXME: make sure we deal properly with the COM DLL ***/
			COMShutdown();
		else
	#endif
			FreeLibrary ((**hdll).hdllsyshandle);

#endif

#ifdef MACVERSION

	lockhandle ((Handle) hdll);
	
	CloseConnection (&(**hdll).hdllsyshandle);
	
	unlockhandle ((Handle) hdll);

#endif
	
	return;
	} /*closelibrary*/


static tydllinfohandle loadlibrary (const ptrfilespec fs) {
	
	/*
	Load the library into memory and prepare it for use
	*/
	
	tydllinfohandle hdll;

	hdll = newlibrary (fs);

	if (hdll == nil)
		return (nil);

	if (!openlibrary (hdll)) {
	
		freelibrary (hdll);
		
		return (nil);
		}

	return (hdll);
	} /*loadlibrary*/


static void unloadlibrary (tydllinfohandle hdll) {

	/*
	Unload the library from memory
	
	If there are currently calls into the library under way,
	we defer until all calls have completed
	*/

	if ((**hdll).ctreferences == 0) {
		
		closelibrary (hdll);
		
		freelibrary (hdll);
	
		}
	else {

		/* defer unloading until all calls have completed */

		(**hdll).flunload = true;
		}

	return;
	} /*unloadlibrary*/


static boolean initparamblock (hdltreenode hp1, typrocinfohandle hprocinfo, tydllparamblock *params, Handle *orighandles) {
	
	/*
	Initialize the param block for the call,
	save and lock original handles for heap-based params,
	and convert params to FDllCall.h format
	*/
	
	tyvaluerecord val;
	tyvaluetype paramtype;
	Handle hdata;
	long kmax = (**hprocinfo).ctparams;
	long k;
	
	if (!langcheckparamcount (hp1, (short) ((**hprocinfo).ctparams + 2)))
		return (false);
	
	clearbytes (params, sizeof (tydllparamblock));
	
	clearbytes (orighandles, kmax * sizeof (Handle));
	
	for (k = 0; k < kmax; k++) {
		
		paramtype = (**hprocinfo).paramtypes[k];

		if (!getparamvalue (hp1, (short)(3 + k), &val))
			return (false);
		
		if (!coercevalue (&val, paramtype))
			return (false);
		
		langgetvalsize (val, &params->paramsize[k]);
		
		if (langheapallocated (&val, &hdata)) {
			
			exemptfromtmpstack (&val);
			
			orighandles[k] = hdata;

			if ((paramtype != listvaluetype) && (paramtype != recordvaluetype)) {
				lockhandle (hdata);
				params->paramdata[k] = (long) *hdata;
				}
			else {
				params->paramdata[k] = (long) hdata;
				}
			}
		else
			params->paramdata[k] = val.data.longvalue;
		}
	
	return (true);
	} /*initparamblock*/


static void freeparamblock (typrocinfohandle hprocinfo, Handle *orighandles) {

	/*
	Unlock and dispose handles of heap-based params
	*/
	
	tyvaluerecord val;
	tyvaluetype paramtype;
	long kmax = (**hprocinfo).ctparams;
	long k;

	for (k = 0; k < kmax; k++)

		if (orighandles[k] != nil) {
		
			paramtype = (**hprocinfo).paramtypes[k];
			
			if ((paramtype != listvaluetype) && (paramtype != recordvaluetype))
				unlockhandle (orighandles[k]);
			
			initvalue (&val, paramtype);
			
			val.data.binaryvalue = orighandles[k];
			
			disposevaluerecord (val, false);
			}

	return;
	} /*freeparamblock*/


static boolean lookupprocaddress (tydllinfohandle hdll, typrocinfohandle hprocinfo) {

	/*
	Platform-specific code for looking up the address of a proc in the library
	*/
	
	#ifdef MACVERSION
		CFragSymbolClass procclass;
		OSErr err;
		#if !TARGET_API_MAC_CARBON
			RoutineDescriptor desctemplate = BUILD_ROUTINE_DESCRIPTOR (uppdllcallProcInfo, NULL);
		#endif
		
		err = FindSymbol ((**hdll).hdllsyshandle, (**hprocinfo).bsprocname, (Ptr*) &(**hprocinfo).procaddress, &procclass); 

		#if !TARGET_API_MAC_CARBON
			if (err == noErr) {
				(**hprocinfo).moduledesc = desctemplate;
				(**hprocinfo).moduledesc.routineRecords[0].procDescriptor = (ProcPtr) (**hprocinfo).procaddress;	/* fill in the blank */
				(**hprocinfo).moduleUPP = (UniversalProcPtr) &(**hprocinfo).moduledesc;
				}
		#elif TARGET_RT_MAC_MACHO
			if (err == noErr) {
				(**hprocinfo).procaddress = convertcfmtomachofuncptr ((**hprocinfo).procaddress);
				}
		#endif
	#endif

	#ifdef WIN95VERSION
		(**hprocinfo).procaddress = (tyDLLEXTROUTINE) GetProcAddress ((**hdll).hdllsyshandle, stringbaseaddress((**hprocinfo).bsprocname));
	#endif

	return ((**hprocinfo).procaddress != nil);
	} /*lookupprocaddress*/


static boolean callprocwithparams (tydllinfohandle hdll, typrocinfohandle hprocinfo, tydllparamblock *params, tyvaluerecord *vreturned) {
	
	//
	// Call the library proc with the given set of parameters
	//
	// If the proc address hasn't been looked up yet, we do so now and save it for later
	//
	// After completing the call, we set up the result value or the error message
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	
	boolean fl = false;

	lockhandle ((Handle) hprocinfo); // just to be sure our data doesn't move around

	// If procaddress is undefined, look it up now and save it for future reference
	
	if ((**hprocinfo).procaddress == nil)
	
		if (!lookupprocaddress (hdll, hprocinfo)) {
		
			bigstring bs;
			
			tyfilespec fs = (**hdll).fs;
			
			getfsfile ( &fs, bs );
			
			lang2paramerror (cantfinddllfunctionerror, (**hprocinfo).bsprocname, bs );
		
			goto exit;
			}

	// Now release the thread globals, call the library proc, and grab the thread globals again

	releasethreadglobals ();
	
	#if MACVERSION && !TARGET_API_MAC_CARBON
		#if GENERATINGCFM
			fl = CallUniversalProc ((**hprocinfo).moduleUPP, uppdllcallProcInfo, params, dllcallbacks);
		#else
			fl = (*(tyDLLEXTROUTINE) ((**hprocinfo).moduleUPP)) (params, dllcallbacks);
		#endif
	#else
		fl = (*(**hprocinfo).procaddress) (params, dllcallbacks);
	#endif
	
	grabthreadglobals ();

	// Set up the return value of the call or the error message
	
	if (fl) {
	
		if (langheaptype ((**hprocinfo).resulttype))
			fl = sethandlesize ((Handle) params->resultdata, params->resultsize);
		
		if (fl) {
			
			initvalue (vreturned, (**hprocinfo).resulttype);
			
			(*vreturned).data.longvalue = params->resultdata;
			
			pushvalueontmpstack (vreturned); 
			}
		}
	else
		{
		bigstring errmsg;
		
		#ifdef MACVERSION
			copystring (params->errormessage, errmsg);
		#else
			copyctopstring (params->errormessage, errmsg);
		#endif
		
		langerrormessage (errmsg);
		}

exit:

	unlockhandle ((Handle) hprocinfo);
		
	return (fl);
	
	} // callprocwithparams


static boolean callproc (hdltreenode hparam1, tydllinfohandle hdll, typrocinfohandle hprocinfo, tyvaluerecord *vreturned) {

	/*
	Build param block and call the named proc in the library
	*/
	
	tydllparamblock dllparamblock;
	Handle orighandles[maxdllparams];
	boolean fl;

	/* Increment the reference count so that will know not to unload it while we're still executing library code */
	
	(**hdll).ctreferences++;
	
	/* Set up the paramblock for the call and save the original param handles */
	
	fl = initparamblock (hparam1, hprocinfo, &dllparamblock, orighandles);
	
	if (fl) {
		
		/* Call the desired library proc */

		fl = callprocwithparams (hdll, hprocinfo, &dllparamblock, vreturned);
	
		/* Restore original param handles and free heap-allocated params */
	
		freeparamblock (hprocinfo, orighandles);
		}

	/* Decrement reference count and possibly unload if the task has been deferred to us */
	
	(**hdll).ctreferences--;
	
	return (fl);
	} /*callproc*/


static boolean callstayresident (hdltreenode hparam1, tydllinfohandle hdll, bigstring bsprocname, tyvaluerecord *vreturned) {

	/*
	Locate the proc info and make the call
	*/

	typrocinfohandle hprocinfo;
	boolean fl = false;
	
	/* Look up the procinfo for the given procname */
	
	if (!locateprocinfo (hdll, bsprocname, &hprocinfo))
		return (false);
	
	/* Make the call */
	
	fl = callproc (hparam1, hdll, hprocinfo, vreturned);
	
	/* Unload the DLL if the task has been deferred to us*/
	
	if ((**hdll).flunload)
		unloadlibrary (hdll);

	return (fl);
	} /*callstayresident*/


static boolean callvolatile (hdltreenode hparam1, const ptrfilespec fs, bigstring bsprocname, tyvaluerecord *vreturned) {

	/*
	Load the library just for this call, get the procinfo, make the call, and unload the library
	*/

	tydllinfohandle hdll;
	typrocinfohandle hprocinfo;
	boolean fl;

	/* Load the library */
		
	hdll = loadlibrary (fs);

	if (hdll == nil)
		return (false);
	
	/* Look up just the one proc we are interested in, don't build the hash table */
		
	if (!parseprocinforesource (hdll, bsprocname, &hprocinfo)) {
		
		unloadlibrary (hdll);
		
		return (false);
		}
		
	/* Make the call */
	
	fl = callproc (hparam1, hdll, hprocinfo, vreturned);
	
	/* Dispose proc info here because we didn't ask for the hash table to be built */
	
#if defined(MACVERSION) && TARGET_RT_MAC_MACHO
	disposemachofuncptr ((void*) (**hprocinfo).procaddress);
#endif
	
	disposehandle ((Handle) hprocinfo);

	/* Unload the DLL */

	unloadlibrary (hdll);
	
	return (fl);
	} /*callvolatile*/


static boolean islibraryloaded (const ptrfilespec fs) {
	
	/*
	Implements the dll.isloaded kernel verb
	
	Determine whether the library with the given file path has already been loaded
	*/

	return (getlibrary (fs) != nil);
	} /*islibraryloaded*/


static boolean dodllload (const ptrfilespec fs, tydllinfohandle *hdllptr) {
	
	/*
	Implements dll.load kernel verb
	
	Load the library into memory as stay-resident 
	*/
	
	tydllinfohandle hdll;
	
	hdll = getlibrary (fs);
	
	if (hdll == nil) {
	
		hdll = loadlibrary (fs);
	
		if (hdll == nil)
			return (false);
		
		if (!parseprocinforesource (hdll, nil, nil)) {
			
			unloadlibrary (hdll);
			
			return (false);
			}

		addlibrary (hdll);
		}
	
	*hdllptr = hdll;
	
	return (true);
	} /*dodllload*/


static boolean dodllunload (const ptrfilespec fs) {

	/*
	Implements dll.unload kernel verb
	
	Unload a stay-resident library
	*/
	
	tydllinfohandle hdll;
	
	hdll = getlibrary (fs);
	
	if (hdll) {
		
		removelibrary (hdll);
		
		unloadlibrary (hdll);
		}
	
	return (true);
	} /*dodllunload*/


static boolean dodllcall (hdltreenode hparam1, const ptrfilespec fs, bigstring bsprocname, tyvaluerecord *vreturned) {

	/*
	Implements the dll.call kernel verb
	*/

	tydllinfohandle hdll;
	boolean fl;
	
	hdll = getlibrary (fs);
	
	if (hdll != nil)
		fl = callstayresident (hparam1, hdll, bsprocname, vreturned);
	else
		fl = callvolatile (hparam1, fs, bsprocname, vreturned);
	
	return (fl);
	} /*dodllcall*/


#else


static boolean parseprocdata (bigstring bsprocname, tydllmoduleinfo *info) {
	/* It is assumed that the bsprocname is a std string with a null termination */

	char * pdata;
	char inputline[1024];
	char parameterline[100];
	short nextblock, commentpoint, endparam, endwhitespace, startname;
	int i;

	pdata = info->pdata;

	while (true) {
		nextblock = strlen (pdata) + 1;

		if (nextblock > 1000)
			return (false);

		if (nextblock <= 1)
			return (false);

		strcpy (inputline, pdata);

		pdata = pdata + nextblock;				/*advance pdata in event we need next line */

		commentpoint = strcspn(inputline, ";");  /*locate start of comment */

		inputline [commentpoint] = 0;			/* remove comment from line */

		if ((short) strlen (inputline) < (stringlength(bsprocname) + 2))
			continue;

		/*peel off parameter info*/

		endparam = strspn (inputline, "VISLRvislr");

		if (endparam == 0)		/* Not a proper line */
			continue;

		memmove (parameterline, inputline, endparam);
		parameterline[endparam] = 0;

		info->ctparams = strlen(parameterline) - 1;  /*minus one for return code*/

		endwhitespace = strspn (inputline + endparam, " \t");

		if (endwhitespace == 0)		/*must have white space between parameters and module name*/
			continue;

		startname = endwhitespace + endparam;

		endwhitespace = strcspn (inputline + startname, " \t");		/*Find end of name */

		inputline[endwhitespace + startname] = 0;

		if (strcmp (inputline + startname, stringbaseaddress(bsprocname)) == 0) {
			/*set parameter values */
			info->resulttype = gettypefromchar (parameterline[0]);

			for (i = 0; i < info->ctparams; i++) {
				info->paramtypes[i] = gettypefromchar (parameterline[i+1]);
				}

			return (true);
			}

		}

	} /*parseprocdata*/

//Code change by Timothy Paustian Wednesday, June 14, 2000 8:30:47 PM
//Changed to Opaque call for Carbon
//I have to ask andre how to handle this stuff, this looks like plug in stuff.
//This routine
static boolean getprocinfo (const ptrfilespec fs, bigstring bsprocname, tydllmoduleinfo *info) {

	/*
	5.0.2 dmb: added fs parameter for errror reporting
	*/
	
	char procname[300];

	//Code change by Timothy Paustian Friday, June 16, 2000 1:03:09 PM
	//Changed to Opaque call for Carbon - UPP aren't needed for Carbon
	#ifdef MACVERSION

		CFragSymbolClass procclass;
		OSErr err;
		 #if !TARGET_API_MAC_CARBON
		RoutineDescriptor desctemplate = BUILD_ROUTINE_DESCRIPTOR (uppdllcallProcInfo, NULL);
		#endif
	#endif
	/*
	load the dll, find the module, and map its parameter info to our types.
	*/
	
	copystring (bsprocname, procname);
	
	nullterminate (procname);

	#ifdef MACVERSION
		#if TARGET_API_MAC_CARBON
		info->procAddress = NULL;
	
		err = FindSymbol ((CFragConnectionID)info->moduleHandle, procname, (Ptr*)&(info->procAddress), &procclass); 

		if (err == noErr) {
			
			info->moduleDesc.routineRecords [0].procDescriptor = (ProcPtr)info->procAddress;	// fill in the blank
			//null this out so that we get a nasty crash if it is dereferences.
			//This may be dangerrous
			//I switched it below to call procPtr directly as in windows and not the UPP.
			//This is the only place the frontier code uses moduleUPP so it should be save. 
			info->moduleUPP = NULL;
			}
		#else
		info->moduleDesc = desctemplate;

		info->procAddress = NULL;
	
		err = FindSymbol ((CFragConnectionID)info->moduleHandle, procname, (Ptr*)&(info->procAddress), &procclass); 

		if (err == noErr) {
			
			info->moduleDesc.routineRecords [0].procDescriptor = (ProcPtr)info->procAddress;	// fill in the blank
	
			info->moduleUPP = (UniversalProcPtr) &(info->moduleDesc);
			}
		#endif
	#endif

	#ifdef WIN95VERSION
		info->procAddress = (tyDLLEXTROUTINE) GetProcAddress ((HINSTANCE) info->moduleHandle, stringbaseaddress(procname));
	#endif

	if (info->procAddress == NULL) {
		
		lang2paramerror (cantfinddllfunctionerror, bsprocname, fsname (fs));
		
		return (false);
		}

	if (!parseprocdata (procname, info)) {
		
		lang2paramerror (cantfindprocinfofunctionerror, bsprocname, fsname (fs));
		
		return (false);
		}

	return (true);
	} /*getprocinfo*/


static boolean islibraryloaded (const ptrfilespec fs, Handle * hModule) {

	#ifdef WIN95VERSION
		bigstring fn;
		
		copystring (fsname (fs), fn);
		
		nullterminate(fn);
		
		*hModule = (Handle) GetModuleHandle (stringbaseaddress(fn));
	#endif

	#ifdef MACVERSION
		long response;
		OSErr err;
		CFragConnectionID connID;
		Ptr mainAddr;
		Str255 errName;
		
		err = Gestalt (gestaltCFMAttr, &response);	// make sure we have the CFM
		
		if ((err != noErr) || (response & (1 << gestaltCFMPresent)) == 0)
			return (false);
		
		#if TARGET_API_MAC_CARBON == 1
			err = GetDiskFragment (fs, 0, kCFragGoesToEOF, fs->name, kReferenceCFrag, &connID, &mainAddr, errName);		
		#else
			err = GetDiskFragment (fs, 0, kCFragGoesToEOF, fs->name, kFindCFrag, &connID, &mainAddr, errName);		
		#endif
		
		if (err != noErr)
			return (false);
		
		*hModule = (Handle) connID;
	#endif
	
	return (*hModule != NULL);
	} /*islibraryloaded*/


static Handle doloadlibrary (const ptrfilespec fs, boolean flforce) {

	Handle hModule = NULL;

	#ifdef WIN95VERSION
		bigstring fn;

	//	if (! flforce) {
	//		if (islibraryloaded (fs, &hModule))
	//			return (hModule);
	//		}

		#if (FRONTIERCOM == 1)
			filefrompath ((ptrstring) fsname (fs), fn);

			nullterminate(fn);

			if (stricmp (stringbaseaddress(fn), "COMDLL.DLL") == 0)
					return (COMStartup());
		#endif

		copystring (fsname (fs), fn);
		
		nullterminate(fn);

		hModule = (Handle) LoadLibrary (stringbaseaddress(fn));

	//	if (hModule != NULL) {
	//		namelen = GetModuleFileName (hModule, stringbaseaddress(namebuf), sizeof(namebuf));
	//		setstringlength (namebuf, namelen);
	//		alllower(namebuf);
	//		addopenlibrary (hModule, fs, namebuf);
	//		}

	#endif

	#ifdef MACVERSION
		long response;
		OSErr err;
		CFragConnectionID connID;
		Ptr mainAddr;
		Str255 errName;
		
		err = Gestalt (gestaltCFMAttr, &response);	// make sure we have the CFM
		
		if ((err != noErr) || (response & (1 << gestaltCFMPresent)) == 0)
			goto exit;
		
		err = GetDiskFragment (fs, 0, kCFragGoesToEOF, fs->name, kReferenceCFrag, &connID, &mainAddr, errName);
		
		if (err != noErr)
			goto exit;
		
		hModule = (Handle) connID;
	
		exit:
	#endif
	
	if (hModule == NULL)
		lang2paramerror (cantconnecttodllerror, bsfunctionname, fsname (fs));
	
	return (hModule);
	} /*doloadlibrary*/


static boolean dofreelibrary (Handle hModule, boolean flforce) {
	#ifdef WIN95VERSION
		#if (FRONTIERCOM == 1)
			if (hModule == COMSYSModule())
				return (COMShutdown());
		#endif

		return (FreeLibrary ((HINSTANCE) hModule));
	#endif

	#ifdef MACVERSION
		return (CloseConnection ((CFragConnectionID *) &hModule) == noErr);
	#endif
	} /*dofreelibrary*/


static boolean loaddllmodule (const ptrfilespec fs, bigstring bsprocname, tydllmoduleinfo *info) {
	
	boolean fl = false;
	#ifdef WIN95VERSION
		HRSRC frh;
		HGLOBAL rh;
	#endif
	#ifdef MACVERSION
		short resfile;
		Handle hRes = nil;
	#endif

	/*
	load the dll, find the module, and map its parameter info to our types.
	*/
	
	copystring (bsprocname, bsfunctionname); // for error reporting
		
	info->moduleHandle = doloadlibrary (fs, false);

	if (info->moduleHandle == NULL)
		return (false);

	info->pdata = NULL;

	#ifdef WIN95VERSION
		frh = FindResource ((HINSTANCE) info->moduleHandle, "ProcInfo", RT_RCDATA);

		if (frh != NULL) {
			
			rh = LoadResource ((HINSTANCE) info->moduleHandle, frh);

			if (rh != NULL)
				info->pdata = LockResource (rh);
			}
	#endif

	#ifdef MACVERSION
		resfile = FSpOpenResFile (fs, fsRdPerm);

		if (ResError() == noErr) {
			
			hRes = Get1NamedResource ('DATA', "\pProcInfo");

			if (hRes != NULL) {
				
				DetachResource (hRes);
				
				HLock (hRes);
				
				info->pdata = *hRes;
				}
			
			CloseResFile (resfile);
			}
	#endif
	
	if (info->pdata == NULL) {
		
		lang2paramerror (cantfindprocinfoerror, bsprocname, fsname (fs));
		
		dofreelibrary (info->moduleHandle, false);
		
		goto exit;
		}
	
	if (getprocinfo (fs, bsprocname, info))
		fl = true;
	else
		dofreelibrary (info->moduleHandle, false);
	
	exit:
	
	#ifdef MACVERSION
		disposehandle (hRes);
	#endif
	
	return (fl);
	} /*loaddllmodule*/


static boolean langcalldll (tydllmoduleinfo *dllinfo, tydllparamblock *dllcall) {
	
	/*
	call the dll. 
	*/
	boolean fl = false;

	releasethreadglobals ();

	#ifdef WIN95VERSION
		fl = (*(dllinfo->procAddress)) (dllcall, dllcallbacks);
	#endif

	#ifdef MACVERSION
		#if TARGET_API_MAC_CARBON == 1
			//Code change by Timothy Paustian Friday, June 16, 2000 1:13:28 PM
			//Changed to Opaque call for Carbon - we don't need UPPs in Carbon.
			//fl = (*(tyDLLEXTROUTINE) (dllinfo->moduleUPP)) (dllcall, dllcallbacks); // call it
			fl = (*(dllinfo->procAddress)) (dllcall, dllcallbacks); // call it
		#else
			#if GENERATINGCFM
				fl = CallUniversalProc (dllinfo->moduleUPP, uppdllcallProcInfo, dllcall, dllcallbacks);
			#else
				fl = (*(tyDLLEXTROUTINE) (dllinfo->moduleUPP)) (dllcall, dllcallbacks); // call it
			#endif
		#endif
	#endif

	dofreelibrary (dllinfo->moduleHandle, false);  /*okay we used it, now release it.*/

	grabthreadglobals ();
	
	return fl;
	} /*langcalldll*/

#endif /* NEW_DLL_INTERFACE */


void fillcalltable (XDLLProcTable *pt) {

	#if defined(MACVERSION) && TARGET_RT_MAC_MACHO

		pt->xMemAlloc = convertmachotocfmfuncptr (&extfrontierAlloc);
		pt->xMemResize = convertmachotocfmfuncptr (&extfrontierReAlloc);
		pt->xMemLock = convertmachotocfmfuncptr (&extfrontierLock);
		pt->xMemUnlock = convertmachotocfmfuncptr (&extfrontierUnlock);
		pt->xMemFree = convertmachotocfmfuncptr (&extfrontierFree);
		pt->xMemGetSize = convertmachotocfmfuncptr (&extfrontierSize);

		pt->xOdbGetCurrentRoot = convertmachotocfmfuncptr (&extOdbGetCurrentRoot);
		pt->xOdbNewFile = convertmachotocfmfuncptr (&extOdbNewFile);
		pt->xOdbOpenFile = convertmachotocfmfuncptr (&extOdbOpenFile);
		pt->xOdbSaveFile = convertmachotocfmfuncptr (&extOdbSaveFile);
		pt->xOdbCloseFile = convertmachotocfmfuncptr (&extOdbCloseFile);
		pt->xOdbDefined = convertmachotocfmfuncptr (&extOdbDefined);
		pt->xOdbDelete = convertmachotocfmfuncptr (&extOdbDelete);
		pt->xOdbGetType = convertmachotocfmfuncptr (&extOdbGetType);
		pt->xOdbCountItems = convertmachotocfmfuncptr (&extOdbCountItems);
		pt->xOdbGetNthItem = convertmachotocfmfuncptr (&extOdbGetNthItem);
		pt->xOdbGetValue = convertmachotocfmfuncptr (&extOdbGetValue);
		pt->xOdbSetValue = convertmachotocfmfuncptr (&extOdbSetValue);
		pt->xOdbNewTable = convertmachotocfmfuncptr (&extOdbNewTable);
		pt->xOdbGetModDate = convertmachotocfmfuncptr (&extOdbGetModDate);
		pt->xOdbDisposeValue = convertmachotocfmfuncptr (&extOdbDisposeValue);
		pt->xOdbGetError = convertmachotocfmfuncptr (&extOdbGetError);

		pt->xDoScript = convertmachotocfmfuncptr (&extDoScript);
		pt->xDoScriptText = convertmachotocfmfuncptr (&extDoScriptText);

		pt->xOdbNewListValue = convertmachotocfmfuncptr (&extOdbNewListValue);
		pt->xOdbGetListCount = convertmachotocfmfuncptr (&extOdbGetListCount);
		pt->xOdbDeleteListValue = convertmachotocfmfuncptr (&extOdbDeleteListValue);
		pt->xOdbSetListValue = convertmachotocfmfuncptr (&extOdbSetListValue);
		pt->xOdbGetListValue = convertmachotocfmfuncptr (&extOdbGetListValue);
		pt->xOdbAddListValue = convertmachotocfmfuncptr (&extOdbAddListValue);

		pt->xInvoke = convertmachotocfmfuncptr (&extInvoke);
		pt->xCoerce = convertmachotocfmfuncptr (&extCoerce);
		
		pt->xCallScript = convertmachotocfmfuncptr (&extCallScript);
		pt->xCallScriptText = convertmachotocfmfuncptr (&extCallScriptText);
		
		pt->xThreadYield = convertmachotocfmfuncptr (&extThreadYield);
		pt->xThreadSleep = convertmachotocfmfuncptr (&extThreadSleep);

	#else

		pt->xMemAlloc = &extfrontierAlloc;
		pt->xMemResize = &extfrontierReAlloc;
		pt->xMemLock = &extfrontierLock;
		pt->xMemUnlock = &extfrontierUnlock;
		pt->xMemFree = &extfrontierFree;
		pt->xMemGetSize = &extfrontierSize;

		pt->xOdbGetCurrentRoot = &extOdbGetCurrentRoot;
		pt->xOdbNewFile = &extOdbNewFile;
		pt->xOdbOpenFile = &extOdbOpenFile;
		pt->xOdbSaveFile = &extOdbSaveFile;
		pt->xOdbCloseFile = &extOdbCloseFile;
		pt->xOdbDefined = &extOdbDefined;
		pt->xOdbDelete = &extOdbDelete;
		pt->xOdbGetType = &extOdbGetType;
		pt->xOdbCountItems = &extOdbCountItems;
		pt->xOdbGetNthItem = &extOdbGetNthItem;
		pt->xOdbGetValue = &extOdbGetValue;
		pt->xOdbSetValue = &extOdbSetValue;
		pt->xOdbNewTable = &extOdbNewTable;
		pt->xOdbGetModDate = &extOdbGetModDate;
		pt->xOdbDisposeValue = &extOdbDisposeValue;
		pt->xOdbGetError = &extOdbGetError;

		pt->xDoScript = &extDoScript;
		pt->xDoScriptText = &extDoScriptText;

		pt->xOdbNewListValue = &extOdbNewListValue;
		pt->xOdbGetListCount = &extOdbGetListCount;
		pt->xOdbDeleteListValue = &extOdbDeleteListValue;
		pt->xOdbSetListValue = &extOdbSetListValue;
		pt->xOdbGetListValue = &extOdbGetListValue;
		pt->xOdbAddListValue = &extOdbAddListValue;

		pt->xInvoke = &extInvoke;
		pt->xCoerce = &extCoerce;
		
		pt->xCallScript = &extCallScript;
		pt->xCallScriptText = &extCallScriptText;
		
		pt->xThreadYield = &extThreadYield;
		pt->xThreadSleep = &extThreadSleep;

	#endif

	} /*fillcalltable*/


#if 0

void smashcalltable (XDLLProcTable *pt) {

	#if defined(MACVERSION) && TARGET_RT_MAC_MACHO

		disposecfmfuncptr (pt->xMemAlloc);
		disposecfmfuncptr (pt->xMemResize);
		disposecfmfuncptr (pt->xMemLock);
		disposecfmfuncptr (pt->xMemUnlock);
		disposecfmfuncptr (pt->xMemFree);
		disposecfmfuncptr (pt->xMemGetSize);

		disposecfmfuncptr (pt->xOdbGetCurrentRoot);
		disposecfmfuncptr (pt->xOdbNewFile);
		disposecfmfuncptr (pt->xOdbOpenFile);
		disposecfmfuncptr (pt->xOdbSaveFile);
		disposecfmfuncptr (pt->xOdbCloseFile);
		disposecfmfuncptr (pt->xOdbDefined);
		disposecfmfuncptr (pt->xOdbDelete);
		disposecfmfuncptr (pt->xOdbGetType);
		disposecfmfuncptr (pt->xOdbCountItems);
		disposecfmfuncptr (pt->xOdbGetNthItem);
		disposecfmfuncptr (pt->xOdbGetValue);
		disposecfmfuncptr (pt->xOdbSetValue);
		disposecfmfuncptr (pt->xOdbNewTable);
		disposecfmfuncptr (pt->xOdbGetModDate);
		disposecfmfuncptr (pt->xOdbDisposeValue);
		disposecfmfuncptr (pt->xOdbGetError);

		disposecfmfuncptr (pt->xDoScript);
		disposecfmfuncptr (pt->xDoScriptText);

		disposecfmfuncptr (pt->xOdbNewListValue);
		disposecfmfuncptr (pt->xOdbGetListCount);
		disposecfmfuncptr (pt->xOdbDeleteListValue);
		disposecfmfuncptr (pt->xOdbSetListValue);
		disposecfmfuncptr (pt->xOdbGetListValue);
		disposecfmfuncptr (pt->xOdbAddListValue);

		disposecfmfuncptr (pt->xInvoke);
		disposecfmfuncptr (pt->xCoerce);
		
		disposecfmfuncptr (pt->xCallScript);
		disposecfmfuncptr (pt->xCallScriptText);
		
		disposecfmfuncptr (pt->xThreadYield);
		disposecfmfuncptr (pt->xThreadSleep);

	#endif
	
	} /*smashcalltable*/

#endif


boolean dllisloadedverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	/*
	Glue for dll.isloaded kernel verb

	2002-10-14 AR: Moved from langverbs.c to langdll.c.
	Started work on new DLL interface, still ifdef'd out.
	*/
	
#ifdef NEW_DLL_INTERFACE

	tyfilespec fsdll;
	
	flnextparamislast = true;

	if (!getfilespecvalue (hparam1, 1, &fsdll))
		return (false);

	return (setbooleanvalue (islibraryloaded (&fsdll), vreturned));

#else

	tyfilespec fsdll;
	Handle hModule;
	boolean res;
	
	flnextparamislast = true;

	if (!getfilespecvalue (hparam1, 1, &fsdll))
		return (false);

	res = islibraryloaded (&fsdll, &hModule);

	return (setbooleanvalue (res, vreturned));

#endif /* NEW_DLL_INTERFACE */

	} /*dllisloadedverb*/


boolean dllloadverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	/*
	Glue for dll.load kernel verb

	2002-10-14 AR: Moved from langverbs.c to langdll.c.
	Started work on new DLL interface, still ifdef'd out.
	*/
	
#ifdef NEW_DLL_INTERFACE

	tyfilespec fsdll;
	tydllinfohandle hdll;
	
	flnextparamislast = true;

	if (!getfilespecvalue (hparam1, 1, &fsdll))
		return (false);

	if (!dodllload (&fsdll, &hdll))
		return (false);

	return (setlongvalue ((long) hdll, vreturned));

#else

	tyfilespec fsdll;
	Handle hModule;
	
	flnextparamislast = true;

	if (!getfilespecvalue (hparam1, 1, &fsdll))
		return (false);

	hModule = doloadlibrary (&fsdll, true);

	return (setlongvalue ((long) hModule, vreturned));

#endif /* NEW_DLL_INTERFACE */

	} /*dllloadverb*/


boolean dllunloadverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	/*
	Glue for dll.unload kernel verb

	2002-10-14 AR: Moved from langverbs.c to langdll.c.
	Started work on new DLL interface, still ifdef'd out.
	*/
	
#ifdef NEW_DLL_INTERFACE

	tyfilespec fsdll;
	
	flnextparamislast = true;

	if (!getfilespecvalue (hparam1, 1, &fsdll))
		return (false);

	return (setbooleanvalue (dodllunload (&fsdll), vreturned));

#else

	tyfilespec fsdll;
	Handle hModule;
	
	flnextparamislast = true;

	if (!getfilespecvalue (hparam1, 1, &fsdll))
		return (false);

	if (islibraryloaded (&fsdll, &hModule))
		dofreelibrary (hModule, true);

	return (setbooleanvalue (true, vreturned));

#endif /* NEW_DLL_INTERFACE */

	} /*dllunloadverb*/


boolean dllcallverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	Glue for dll.call kernel verb
	
	5/4/92 dmb: created.
	
	5.0.2b3 dmb: on Mac, error message is already a pascal string

	2002-10-14 AR: Moved from langverbs.c to langdll.c.
	Started work on new DLL interface, still ifdef'd out.
	*/
	
#ifdef NEW_DLL_INTERFACE

	tyfilespec fsdll;
	bigstring bsprocname;

	if (!getfilespecvalue (hparam1, 1, &fsdll))
		return (false);
	
	if (!getstringvalue (hparam1, 2, bsprocname))
		return (false);

	return (dodllcall (hparam1, &fsdll, bsprocname, vreturned));

#else

	register hdltreenode hp1 = hparam1;
	tyfilespec fsdll;
	bigstring bsmodule;
	tydllmoduleinfo dllinfo;
	tydllparamblock dllcall;
	tyvaluerecord val;
	Handle hdata;
	Handle orighandles [maxdllparams];
	short i;
	boolean fl;
	
	if (!getfilespecvalue (hp1, 1, &fsdll))
		return (false);
	
	if (!getstringvalue (hp1, 2, bsmodule))
		return (false);
	
	if (!loaddllmodule (&fsdll, bsmodule, &dllinfo))
		return (false);
	
	if (!langcheckparamcount (hp1, (short) (dllinfo.ctparams + 2)))
		return (false);
	
	clearbytes (&dllcall, sizeof (dllcall));
	
	for (i = 0; i < dllinfo.ctparams; ++i) {
		
		if (!getparamvalue (hp1, (short)(3 + i), &val))
			return (false);
		
		if (!coercevalue (&val, dllinfo.paramtypes [i]))
			return (false);
		
		langgetvalsize (val, &dllcall.paramsize [i]);
		
		if (langheapallocated (&val, &hdata)) {
			
			exemptfromtmpstack (&val);
			
			orighandles [i] = hdata;

			if ((dllinfo.paramtypes[i] == listvaluetype) || (dllinfo.paramtypes[i] == recordvaluetype)) {
				dllcall.paramdata [i] = (long) hdata;
				}
			else {
				lockhandle (hdata);
				
				dllcall.paramdata [i] = (long) *hdata;
				}
			}
		else {
			orighandles [i] = nil;

			dllcall.paramdata [i] = val.data.longvalue;
			}
		}
	
	fl = langcalldll (&dllinfo, &dllcall);
	
	for (i = 0; i < dllinfo.ctparams; ++i) {
	
		unlockhandle (orighandles[i]);
		
		initvalue (&val, dllinfo.paramtypes [i]);
		
		val.data.binaryvalue = orighandles [i];
		
		disposevaluerecord (val, false);
		}
	
	if (fl) {
	
		if (langheaptype (dllinfo.resulttype))
			fl = sethandlesize ((Handle) dllcall.resultdata, dllcall.resultsize);
		
		if (fl) {
			
			initvalue (vreturned, dllinfo.resulttype);
			
			(*vreturned).data.longvalue = dllcall.resultdata;
			
			pushvalueontmpstack (vreturned); 
			}
		}
	else
		{
		bigstring errmsg;
		
		#ifdef MACVERSION
			copystring (dllcall.errormessage, errmsg);
		#else
			copyctopstring (dllcall.errormessage, errmsg);
		#endif
		
		langerrormessage (errmsg);
		}
		
	return (fl);

#endif /* NEW_DLL_INTERFACE */

	} /*calldllverb*/


void dllinitverbs (void) {
	
	if (dllcallbacks == nil) {

		dllcallbacks = (XDLLProcTable *) malloc (sizeof (XDLLProcTable));
		
		if (dllcallbacks != nil)
			fillcalltable (dllcallbacks);
		}
	} /*initdllverbs*/

