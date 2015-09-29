
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

#define landinclude /*so other modules can tell if we've been loaded*/



#ifndef THINKC

	#define THINKC /*possible values are MPWC or THINKC*/

#endif


#ifdef MPWC

	#undef THINKC /*turning on MPWC from command line overrides default*/
	
	#include <types.h>
	#include <resources.h>
	#include <dialogs.h>
	#include <menus.h>
	#include <fonts.h>
	#include <events.h>
	#include <controls.h>
	#include <segload.h>
	#include <memory.h>
	#include <toolutils.h>
	#include <packages.h>
	#include <sound.h>
	#include <files.h>
	#include <EPPC.h>
	
#endif


#ifndef __ALIASES__

	#include <Aliases.h>

#endif

#ifndef __AEREGISTRY__

	#include <AERegistry.h>

#endif

#ifndef standardinclude

	#include <standard.h>

#endif


typedef unsigned char **hdlbigstring;

typedef pascal boolean (*landeventrecordcallback) (EventRecord *);

typedef pascal OSErr (*landnoparamcallback) (void);

typedef pascal OSErr (*landfilespeccallback) (FSSpec *);

typedef pascal boolean (*landqueuepopcallback) (Handle, long);	/* 2004-10-24 aradke: param list was ... */

typedef pascal OSErr (*tyeventcreatecallback)(AEEventClass theAEEventClass, AEEventID theAEEventID, const AEAddressDesc *target, short returnID, long transactionID, AppleEvent *result);

typedef pascal OSErr (*tyeventsendcallback)(const AppleEvent *theAppleEvent, AppleEvent *reply, AESendMode sendMode, AESendPriority sendPriority, long timeOutInTicks, AEIdleUPP idleProc, AEFilterUPP filterProc);

typedef pascal OSErr (*tyeventhandler) (AppleEvent *, AppleEvent *, long);

typedef unsigned long tydate;

typedef unsigned long tyenumerator;

typedef AEKeyword typaramkeyword;

typedef unsigned long tyapplicationid;

typedef unsigned long tyverbclass;

typedef unsigned long tyverbtoken;

typedef short tysystem6processid;

#pragma pack(2)
typedef struct tyverbarrayelement {
	
	tyverbtoken token;
	
	tyverbclass class;
	
	boolean flfasthandler;
	} tyverbarrayelement;
#pragma options align=reset

typedef tyverbarrayelement tyverbarray [1];

typedef tyverbarray **hdlverbarray;


typedef enum typaramtype {
	
	notype = typeWildCard, /*allows you to ask for a param w/o specifying its type*/
	
	booleantype = typeBoolean, /*1 == true, 0 == false*/
	
	charactertype = 'char', /*a 1-byte unsigned character*/
	
	inttype = typeShortInteger, /*a 2-byte signed number*/
	
	longtype = typeLongInteger, /*a 4-byte signed number*/
	
	datetype = 'date', /*4-bytes worth of time/date info, same format as Mac GetDateTime call*/
	
	floattype = 'flot', /*a SANE single-precision floating point number, same as singletype*/
	
	directiontype = 'dir ', /*up, down, left, right, etc*/
	
	pointtype = typeQDPoint, /*a QuickDraw Point*/
	
	recttype = typeQDRectangle, /*a QuickDraw rectangle*/

	enumtype = typeEnumerated, /*4-character id, a Macintosh OSType*/

	stringtype = 's255', /*Pascal-format string, 1-byte length indicator at head*/
	
	texttype = typeChar, /*a handle that holds a stream of ASCII characters*/
	
	binarytype = 'BINA',  /*untyped binary chunk of data*/
	
	pathtype = 'path', /*file path, separate so cross-platform coercion is possible*/
	
	aliastype = typeAlias, /*a file alias, as defined by the Macintosh Alias Manager*/
	
	ostype = typeType,  /*4-character id, a Macintosh OSType*/
	
	singletype = typeShortFloat, /*a SANE single-precision floating point number*/
	
	doubletype = typeExtended, /*a SANE extended-precision floating point number*/
	
	rgbtype = typeRGBColor, /*a Mac color, three components, red, green and blue values*/
	
	patterntype = 'tptn', /*a Mac pattern, as defined by QuickDraw*/
	
	fixedtype = typeFixed, /*a fixed point number, dmb: how many places to left and right of decimal?*/
	
	fsspectype = typeFSS, /*a Macintosh filespec*/
	
	objectspectype = cObjectSpecifier /*a Mac object specifier, for Frontier, not fully supported in Toolkit*/
	} typaramtype;

#pragma pack(2)
typedef struct typaramrecord {
	
	typaramkeyword key; /*parameter id, allows optional params; order not important*/
	
	AEDesc desc; /*a type and handle*/
	} typaramrecord, *ptrparamrecord;
#pragma options align=reset

#if 0

typedef enum typaramtype {
	
	booleantype = 1, /*1 == true, 0 == false*/
	
	charactertype = 2, /*a 1-byte unsigned character*/
	
	inttype = 3, /*a 2-byte signed number*/
	
	longtype = 4, /*a 4-byte signed number*/
	
	datetype = 5, /*4-bytes worth of time/date info, same format as Mac GetDateTime call*/
	
	floattype = 6, /*a SANE single-precision floating point number, same as singletype*/
	
	directiontype = 7, /*up, down, left, right, etc*/
	
	pointtype = 8, /*a QuickDraw Point*/
	
	recttype = 9, /*a QuickDraw rectangle*/

	enumtype = 10, /*4-character id, a Macintosh OSType*/

	stringtype = 11, /*Pascal-format string, 1-byte length indicator at head*/
	
	texttype = 12, /*a handle that holds a stream of ASCII characters*/
	
	binarytype = 13,  /*untyped binary chunk of data*/
	
	pathtype = 14, /*file path, separate so cross-platform coercion is possible*/
	
	aliastype = 15, /*a file alias, as defined by the Macintosh Alias Manager*/
	
	ostype = 16,  /*4-character id, a Macintosh OSType*/
	
	singletype = 17, /*a SANE single-precision floating point number*/
	
	doubletype = 18, /*a SANE double-precision floating point number*/
	
	rgbtype = 19, /*a Mac color, three components, red, green and blue values*/
	
	patterntype = 20, /*a Mac pattern, as defined by QuickDraw*/
	
	fixedtype = 21, /*a fixed point number, dmb: how many places to left and right of decimal?*/
	
	fsspectype = 22, /*a Macintosh filespec*/
	
	objectspectype = 23, /*a Mac object specifier, for Frontier, not fully supported in Toolkit*/
	
	objectdesctype = 24, /*a Mac object descriptor, for Frontier, not fully supported in Toolkit*/
	
	notype = 0 /*allows you to ask for a param w/o specifying its type*/
	} typaramtype;


typedef union typaramvalue {
	
	unsigned char booleanvalue;
	
	char charvalue;
	
	short intvalue;
	
	long longvalue;
	
	tydate datevalue;
	
	float floatvalue;
	
	tydirection directionvalue;
	
	Point pointvalue;
	
	Rect rectvalue;
	
	tyenumerator enumvalue;
	
	hdlbigstring stringvalue;
	
	Handle textvalue;
	
	Handle binaryvalue;
	
	hdlbigstring pathvalue;
	
	AliasHandle aliasvalue;

	OSType osvalue;
	
	float singlevalue;
	
	double **doublevalue;
	
	RGBColor rgbvalue;
	
	Pattern patternvalue;
	
	Fixed fixedvalue;
	
	FSSpecHandle fsspecvalue;
	
	AEDesc objectspecvalue;
	
	AEDesc objectdescvalue;
	} typaramvalue;
	
#pragma pack(2)
typedef struct typaramrecord {
	
	typaramtype type; /*boolean, short, long, etc.*/
	
	typaramkeyword key; /*parameter id, allows optional params; order not important*/
	
	typaramvalue val; /*a char, long, or a handle to a more complex type*/
	} typaramrecord, *ptrparamrecord;
#pragma options align=reset

#endif

#pragma pack(2)
typedef struct tyverbrecord {
	
	tyverbclass verbclass; /*4-byte class, often same as receiver id*/
	
	tyverbtoken verbtoken; /*4-byte token, e.g. 'save', 'go  '*/
	
	tyapplicationid idreceiver; /*signature of the receiver, for outgoing verbs*/
	
	tyapplicationid idsender; /*signature of the caller, for incoming verbs*/
	
	long idtransaction; /*for use by caller*/
	
	Handle transportinfo; /*info the transport layer needs*/
	
	boolean flverbgetsnoreply : 1; /*set true before landsendverb call*/
	
	boolean flnextparamisoptional : 1; /*set true before landgetxxxparam call*/
	
	boolean flverbisdirect: 1; /*set true if verb is to be dispatched directly to code extension*/
	
	boolean flsystemverb: 1; /*verb was received by a system event handler*/
	
	Handle hdirectcode; /*for code extension*/
	
	short ctparams; /*number of items in the params array*/
	
	#if 0 /*sys6*/
	
	short nextparam; /*next open slot in the params array*/
	
	typaramrecord params []; /*variable-length array of parameters*/
	
	#endif
	
	long landrefcon; /*for client use*/
	} tyverbrecord, *ptrverbrecord, **hdlverbrecord;
#pragma options align=reset

typedef pascal boolean (*landverbrecordcallback) (hdlverbrecord);


typedef enum tytransport { /*indicates which transport layer is active*/
	
	macsystem7, /*using AppleEvents transport on System 7*/
	
	macsystem6 /*using System 6 INIT as transport mechanism*/
	
	/*room for support of other IAC protocols in our super-protocol*/
	} tytransport;

#pragma pack(2)
typedef struct tymacnetglobals { /*for mac system 7 networked messages*/
	
	boolean	flhavebrowsed;
	
	tyapplicationid idforbrowser;
	} tymacnetglobals;



typedef struct tynetworkaddress {
	
	TargetID target;
	
	/*
	boolean flself;
	
	LocationNameRec	location;
	
	PortInfoRec	port;
	*/
	} tynetworkaddress;
#pragma options align=reset
	
#ifndef __MENUSHARING__

#pragma pack(2)
typedef struct tyruntimemenurecord { /*structures to support Frontier menu sharing*/
	
	short idmenu;
	
	short flhierarchic: 1;
	
	short flinserted: 1; 
	
	MenuHandle hmenu;
	} tyruntimemenurecord;
#pragma options align=reset
	
typedef tyruntimemenurecord tymenuarray [1];

typedef tymenuarray **hdlmenuarray;

#endif

#pragma pack(2)
typedef struct tymaceventsettings { /*for system 7, info for sending events*/
	
	long timeoutticks; /*the timeout amount*/
	
	short interactionlevel; /*the interaction level*/
	
	long transactionid; /*the transaction id*/
	} tymaceventsettings;


typedef struct tyfastverbcontext {
	
	THz savezone;
	
	Handle savemaphandle;
	
	short saveresfile;
	} tyfastverbcontext;

	
typedef struct tylandglobals {
	
	boolean flconnected; /*tells us if we can do IAC or not*/
	
	boolean flacceptanyverb; /*determines whether all verbs are accepted by landhandleverb*/	
	
	tytransport transport; /*set by landinit, indicates which transport we're using*/
	
	tyapplicationid applicationid; /*the creator id of the application we're inside of*/
	
	short iderrorlist; /*resource id of STR# list which holds error messages*/
	
	hdlverbarray verbarray; /*holds all registered verbs*/
	
	WindowPtr statswindow; /*if non-nil, the stats window is open*/
	
	tyverbclass currentclass; /*set by landaddclass*/
	
	pascal boolean (*handleverbroutine) (hdlverbrecord); /*if non-nil, gets all verbs we don't handle*/
	
	pascal boolean (*findhandlerroutine) (hdlverbrecord); /*for handling wildcard verbs*/
	
	landeventrecordcallback breakembraceroutine; /*if non-nil, break a synchronous deadly embrace*/
	
		/*landcallback menuscriptcompleteroutine; /%if non-nil, callback when script completes*/
	
	/*names that begin with "mac" are inherently NOT cross-platform*/
	
	landnoparamcallback macopenapproutine; /*system 7 open application event handler*/
	
	landfilespeccallback macopendocroutine; /*system 7 open document event handler*/
	
	landfilespeccallback macprintdocroutine; /*system 7 print document event handler*/
	
	landnoparamcallback macquitapproutine; /*system 7 quit application event handler*/
	
	tyapplicationid maceventsender; /*id of app that invoked macxxxroutine*/
	
	tyeventcreatecallback eventcreatecallback; /*for OSA support*/
	
	tyeventsendcallback eventsendcallback; /*for OSA support*/
	
	tymacnetglobals macnetglobals; /*info for system 7 network transport*/
	
	boolean fldirtyruntimemenus; /*program's runtime menus need updating*/
	
	boolean flscriptcancelled; /*indicate that menu script has been cancelled*/
	
	long idmenuscript; /*if not 0, Frontier's id for the currently running script*/
	
	hdlmenuarray hmenuarray; /*we keep a copy of the menuarray data structure*/
	
	tysystem6processid system6processid; /*the INIT's id for our mailbox*/
	
	short ipcrefnum; /*for system 6, the refnum of the UserLand INIT driver*/
	
	Handle hqueue; /*for system 6, queues message that arrive while we're waiting for a response*/
	
	short maxqueuedepth; /*for stats window -- the longest chain in the system 6 queue*/
	
	/*Handle mactypes; /%for system 7, maps our param types to Apple's*/
	
	long initialfreemem; /*for stats window -- the amount of memory free after landinit*/
	
	tymaceventsettings eventsettings;
	
	OSErr landerrorcode; /*error code that caused land call to return false*/
	
	} tylandglobals, *ptrlandglobals, **hdllandglobals;
#pragma options align=reset

/*verbs of this class are automatically handled by UserLand Toolkits*/

	#define userlandclass 'land' /*automatically handled UserLand verbs*/
	
	
/*tokens and keys for communicating with the application scripting system*/

	#define idappscriptrunner 'LAND' /*id of application scripting system*/
	
	#define runscripttoken 'dosc' /*run the script in the 'scpt' text value*/
	
	#define idscriptvalue '----' /*identifies the text value for the script*/
	
	#define schedulescripttoken 'schs' /*schedule the script*/
	
	
/*tokens and keys for communicating with the runtime application scripting system*/
	
	#define getmenuarraytoken 'gmry'
	
	#define idmenuprogram 'menp' /*id of program whose menu we're getting*/
	
	#define idstartingresource 'res1'
	
	#define getmenuhandletoken 'gmhd'
	
	#define idarrayindex 'idix'
	
	#define idrunmenuitem 'runm'
	
	#define idmenuidvalue 'mid '
	
	#define idmenuitemvalue 'mitm'

	#define idupdatemenus 'updm'
	
	#define idscriptcompleted 'done'
	
	#define killscripttoken 'kill'


/*messages that are generated and handled transparently by the API*/
	
	#define idrunning 'zzz?'
	
	#define idgetverbs 'vbs?'
	
	#define idopenstatswindow 'stat'
	
	#define idstatsopen 'sts?'


/*standard  parameter keywords*/
	
	#define returnedvaluekey '----' /*normal return value*/
	
	#define directparamkey '----' /*the key for a direct parameter*/
	
	#define errornumberkey 'errn'
	
	#define errorstringkey 'errs'


/*error numbers -- indexes into the STR# resource*/
	
	#define nosuchparamerror 1
	
	#define wrongtypeerror 2
	
	#define undefinedverberror 3
	
	#define nohandlererror 4 
	
	#define outofmemoryerror 5
	
	#define usercancellederror 6 
	
	#define lastinternalerror usercancellederror


/*prototypes -- initializing, polling & closing, misc stuff*/

	pascal boolean landinit (void);
	
	pascal boolean landaddverb (tyverbtoken);
	
	pascal boolean landaddfastverb (tyverbtoken);
	
	pascal boolean landaddclass (tyverbclass);
	
	pascal boolean landdeleteverb (tyverbclass, tyverbtoken);
	
	pascal boolean landeventfilter (EventRecord *);
	
	pascal boolean landsettimeout (long);
	
	pascal boolean landsettransactionid (long);
	
	pascal boolean landsetinteractionlevel (short);
	
	pascal boolean landapprunning (tyapplicationid);
	
	pascal boolean landclose (void);

	pascal boolean landsetapplicationid (tyapplicationid); 

	pascal boolean landacceptanyverb (boolean);	
	
	pascal boolean landclaimparam (hdlverbrecord, typaramkeyword);

	pascal boolean landefaultbreakembrace (EventRecord *);
	
	pascal OSErr landsystem7defaultcreate (AEEventClass, AEEventID, const AEAddressDesc *, short, long, AppleEvent *);
	
	pascal OSErr landsystem7defaultsend (const AppleEvent *, AppleEvent *, AESendMode, AESendPriority, long, AEIdleUPP, AEFilterUPP);
	
	pascal void landseterror (OSErr);
	
	pascal OSErr landgeterror (void);
	
	pascal boolean landvisitsleepingthreads (landqueuepopcallback, long);
	
	#ifdef flnewfeatures
	
	pascal boolean landsystem7geteventrecords (hdlverbrecord, AppleEvent *, AppleEvent *);
	
	pascal OSErr landsystem7getsenderinfo (const AppleEvent *, ProcessSerialNumber *, FSSpec *, OSType *);
	
	pascal boolean landsystem7pushparam (AERecord *, typaramtype, Handle, void *, long, typaramkeyword);

	pascal boolean landsystem7getparam (const AERecord *, typaramkeyword, typaramtype, typaramrecord *);

	pascal boolean landsystem7getnthparam (const AERecord *, short, typaramrecord *);

	pascal boolean landpushfastcontext (tyfastverbcontext *);
	
	pascal void landpopfastcontext (const tyfastverbcontext *);

	#else
	
	pascal OSErr landsystem7callhandler (hdlverbrecord, tyeventhandler);
	
	#endif
	
	pascal OSErr landsystem7suspendevent (hdlverbrecord);

	pascal OSErr landsystem7resumeevent (hdlverbrecord);
	
	pascal boolean landsystem7installfasthandler (tyverbclass, tyverbtoken, tyeventhandler);
	
	pascal boolean landsystem7removefasthandler (tyverbclass, tyverbtoken);

	pascal boolean landsystem7unpackverb (AppleEvent *, AppleEvent *, hdlverbrecord *);



/* communicating with Frontier scripting system */

	pascal boolean landruntext (Handle, boolean, Str255);
	
	pascal boolean landrunstring (Str255, boolean, Str255);
	
	pascal boolean landgetmenuarray (short, hdlmenuarray *);
	
	pascal boolean landinstallmenuarray (short, hdlmenuarray);
	
	pascal boolean landmenuhit (short, hdlmenuarray);
	
	pascal boolean landrunmenuitem (short, short, hdlmenuarray);
	
	pascal boolean landcancelmenuscript (void);
	
	pascal boolean landremovemenuarray (hdlmenuarray);
	
	pascal boolean landdisposemenuarray (hdlmenuarray);
	
	pascal boolean landenablemenuarray (hdlmenuarray, boolean);
	
	pascal boolean landmenuneedsupdate (void);
	
	
/*opening and closing the Toolkit stats window*/
	
	pascal boolean landopenstatswindow (void);

	pascal boolean landclosestatswindow (void);
	
	pascal boolean landisstatswindow (WindowPtr); /*return true if it's the stats window*/
	

/*creating, sending & disposing verbs, pushing all standard types*/

	pascal boolean landnewverb (tyapplicationid, ProcessSerialNumber *, tyverbclass, tyverbtoken, short, hdlverbrecord *);
	
	pascal boolean landpushparam (hdlverbrecord, typaramtype, Handle, void *, long, typaramkeyword);
	
	pascal boolean landpushbooleanparam (hdlverbrecord, Boolean, typaramkeyword);
	
	pascal boolean landpushcharparam (hdlverbrecord, char, typaramkeyword);
	
	pascal boolean landpushintparam (hdlverbrecord, short, typaramkeyword);
	
	pascal boolean landpushlongparam (hdlverbrecord, long, typaramkeyword);
	
	pascal boolean landpushdateparam (hdlverbrecord, tydate, typaramkeyword);
	
	pascal boolean landpushfloatparam (hdlverbrecord, float, typaramkeyword);
	
	pascal boolean landpushdirectionparam (hdlverbrecord, tydirection, typaramkeyword);
		
	pascal boolean landpushpointparam (hdlverbrecord, Point, typaramkeyword);
	
	pascal boolean landpushrectparam (hdlverbrecord, Rect *, typaramkeyword); 
	
	pascal boolean landpushenumparam (hdlverbrecord, tyenumerator, typaramkeyword);
	
	pascal boolean landpushstringparam (hdlverbrecord, Str255, typaramkeyword);
	
	pascal boolean landpushtextparam (hdlverbrecord, Handle, typaramkeyword);
	
	pascal boolean landpushbinaryparam (hdlverbrecord, Handle, typaramkeyword);

	pascal boolean landpushaliasparam (hdlverbrecord, AliasHandle, typaramkeyword);

	pascal boolean landpushpathparam (hdlverbrecord, Str255, typaramkeyword);
	
	pascal boolean landpushosparam (hdlverbrecord, OSType, typaramkeyword);
	
	pascal boolean landpushsingleparam (hdlverbrecord, float, typaramkeyword);
	
	pascal boolean landpushdoubleparam (hdlverbrecord, double *, typaramkeyword);
	
	pascal boolean landpushrgbparam (hdlverbrecord, RGBColor *, typaramkeyword);
	
	pascal boolean landpushpatternparam (hdlverbrecord, Pattern *, typaramkeyword);
	
	pascal boolean landpushfixedparam (hdlverbrecord, Fixed *, typaramkeyword);
	
	pascal boolean landpushfsspecparam (hdlverbrecord, FSSpec *, typaramkeyword);
	
	pascal boolean landpushobjectspecparam (hdlverbrecord, AEDesc *, typaramkeyword);
	
	pascal boolean landpushkeybinaryparam (hdlverbrecord, Handle, typaramkeyword);
	
	pascal boolean landsendverb (hdlverbrecord, typaramrecord *, Str255, short *);
	
	pascal boolean landcomplexsend (hdlverbrecord, hdlverbrecord *);
	
	pascal boolean landdisposeparamrecord (const typaramrecord *); 
	
	pascal boolean landdisposeverb (hdlverbrecord);


/*getting parameters from a verb record you've received*/

	pascal boolean landgetparam (hdlverbrecord, typaramkeyword, typaramtype, typaramrecord *); 
	
	pascal boolean landgetbooleanparam (hdlverbrecord, typaramkeyword, Boolean *); 
	
	pascal boolean landgetcharparam (hdlverbrecord, typaramkeyword, char *);
	
	pascal boolean landgetintparam (hdlverbrecord, typaramkeyword, short *); 
	
	pascal boolean landgetlongparam (hdlverbrecord, typaramkeyword, long *); 
	
	pascal boolean landgetdateparam (hdlverbrecord, typaramkeyword, tydate *); 
	
	pascal boolean landgetfloatparam (hdlverbrecord, typaramkeyword, float *); 
	
	pascal boolean landgetdirectionparam (hdlverbrecord, typaramkeyword, tydirection *); 
	
	pascal boolean landgetpointparam (hdlverbrecord, typaramkeyword, Point *); 
	
	pascal boolean landgetrectparam (hdlverbrecord, typaramkeyword, Rect *); 
	
	pascal boolean landgetenumparam (hdlverbrecord, typaramkeyword, tyenumerator *); 
	
	pascal boolean landgetstringparam (hdlverbrecord, typaramkeyword, Str255); 
	
	pascal boolean landgettextparam (hdlverbrecord, typaramkeyword, Handle *); 
	
	pascal boolean landgetbinaryparam (hdlverbrecord, typaramkeyword, Handle *); 

	pascal boolean landgetaliasparam (hdlverbrecord, typaramkeyword, AliasHandle *); 

	pascal boolean landgetpathparam (hdlverbrecord, typaramkeyword, Str255);
	
	pascal boolean landgetosparam (hdlverbrecord, typaramkeyword, OSType *);
	
	pascal boolean landgetsingleparam (hdlverbrecord, typaramkeyword, float *);
	
	pascal boolean landgetdoubleparam (hdlverbrecord, typaramkeyword, double *);
	
	pascal boolean landgetrgbparam (hdlverbrecord, typaramkeyword, RGBColor *);
	
	pascal boolean landgetpatternparam (hdlverbrecord, typaramkeyword, Pattern *);
	
	pascal boolean landgetfixedparam (hdlverbrecord, typaramkeyword, Fixed *);
	
	pascal boolean landgetfsspecparam (hdlverbrecord, typaramkeyword, FSSpec *);
	
	pascal boolean landgetobjectspecparam (hdlverbrecord, typaramkeyword, AEDesc *);
	
	pascal boolean landgetkeybinaryparam (hdlverbrecord, typaramkeyword, Handle *);
	
	pascal boolean landgetnthparam (hdlverbrecord, short, typaramrecord *);


/*returning values*/

	pascal boolean landreturnboolean (hdlverbrecord, Boolean);
	
	pascal boolean landreturnchar (hdlverbrecord, char);
	
	pascal boolean landreturnint (hdlverbrecord, short);
	
	pascal boolean landreturnlong (hdlverbrecord, long);
	
	pascal boolean landreturndate (hdlverbrecord, tydate);
	
	pascal boolean landreturnfloat (hdlverbrecord, float);
	
	pascal boolean landreturndirection (hdlverbrecord, tydirection);
		
	pascal boolean landreturnpoint (hdlverbrecord, Point);

	pascal boolean landreturnrect (hdlverbrecord, Rect *); 
	
	pascal boolean landreturnenum (hdlverbrecord, tyenumerator);
	
	pascal boolean landreturnstring (hdlverbrecord, Str255);
	
	pascal boolean landreturntext (hdlverbrecord, Handle);
	
	pascal boolean landreturnbinary (hdlverbrecord, Handle);

	pascal boolean landreturnalias (hdlverbrecord, AliasHandle);

	pascal boolean landreturnpath (hdlverbrecord, Str255);
	
	pascal boolean landreturnos (hdlverbrecord, OSType);
	
	pascal boolean landreturnsingle (hdlverbrecord, float);
	
	pascal boolean landreturndouble (hdlverbrecord, double *);
	
	pascal boolean landreturnrgb (hdlverbrecord, RGBColor *);
	
	pascal boolean landreturnpattern (hdlverbrecord, Pattern *);

	pascal boolean landreturnfixed (hdlverbrecord, Fixed *);

	pascal boolean landreturnfsspec (hdlverbrecord, FSSpec *);
	
	pascal boolean landreturnobjectspec (hdlverbrecord, AEDesc *);
	
	pascal boolean landreturnkeybinaryparam (hdlverbrecord, Handle);
	
	pascal boolean landreturnerror (hdlverbrecord, short);


/*for returning multiple values*/

	pascal boolean landstartreturn (hdlverbrecord); 
	
	/*follow by landpushxxxparam for each return value*/


/*two routines that make it easy to find out if an application supports a verb*/

	pascal boolean landgetappverbs (tyapplicationid, hdlverbarray *);
	
	pascal boolean landverbinarray (tyverbclass, tyverbtoken, hdlverbarray);


/*routines for modifying normal behavior of standard calls*/
	
	pascal boolean landnextparamisoptional (hdlverbrecord); /*call before landgetxxxparam*/
	
	pascal boolean landverbgetsnoreply (hdlverbrecord); /*call before landsendverb*/


/*networked messages on Macintosh system 7*/
	
	pascal boolean landstring2networkaddress (ConstStr255Param, tynetworkaddress *);
	
	pascal boolean landbrowsenetworkapps (ConstStr255Param, tynetworkaddress *, tyapplicationid);

	pascal boolean landnewnetworkverb (tynetworkaddress *, tyverbclass, tyverbtoken, short, hdlverbrecord *);

	pascal OSErr landgetverbattr (hdlverbrecord, OSType, DescType, AEDesc *);

	pascal OSErr landsetverbattr (hdlverbrecord, OSType, DescType, const void * attrdata, long);


/*accessing the Toolkit globals*/

	pascal hdllandglobals landgetglobals (void);


