
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef __IAC__
#define __IAC__ /*so other modules can tell that we've been included*/


#ifndef __APPLEEVENTS__

	#include <AppleEvents.h>

#endif


typedef struct tyIACglobals { /*this global record helps keep param lists short*/
	
	pascal short (*waitroutine) (EventRecord *);
	
	AppleEvent *event; /*the current event being processed*/
	
	AppleEvent *reply; /*the reply to the current event*/
	
	long refcon; /*the refcon info passed with the message*/
	
	OSType idprocess; /*creator id of the process we're running inside of*/
	
	OSErr errorcode; /*the specific error code returned from the Apple Event Manager*/
	
	Boolean nextparamoptional; /*if true, any IACgetxxxparam won't cause error if param isn't there*/
	
	Boolean dontdisposenextparam; /*dmb 4.1: if true, any IACpushxxxparam won't dispose param if it didn't allocate it*/
	} tyIACglobals;
	
extern tyIACglobals IACglobals;


typedef pascal Boolean (*tyFScallback) (FSSpec *);

typedef pascal OSErr (*tyAEHandler) (const AppleEvent *, AppleEvent *, long);

typedef pascal void (*tyAsynchCallback) (AppleEvent *);



/*iac.c*/
	
	Boolean IACinit (void);
	
	Boolean IAChaveappleevents (void);
	
	Boolean IACinstallhandler (AEEventClass, AEEventID, ProcPtr);
	
	Boolean IACremovehandler (AEEventClass, AEEventID, ProcPtr);
	
	Boolean IAChandlerinstalled (OSType, OSType, Boolean);
	
	Boolean IACinstallcoercionhandler (DescType, DescType, ProcPtr);
	
	Boolean IACnewverb (OSType, OSType, OSType, AppleEvent *);
	
	Boolean IACsendverb (AppleEvent *, AppleEvent *);
	
	Boolean IACsendverbnoreply (AppleEvent *, AppleEvent *);
	
	Boolean IACdisposeverb (AppleEvent *);
	
	OSType IACgetverbclass (void);
	
	OSType IACgetverbtoken (void);
	
	void IACparamerror (OSErr, Str255, OSType);
	
	Boolean IACiserrorreply (Str255);
	
	Boolean IACreturnerror (short, Str255);
	
	void IACnothandlederror (void);
	
	OSErr IACdrivefilelist (tyFScallback);
	
	OSType IACgetsender (void);

	Boolean IACnextparamisoptional (void);

/*iacsysevents.c*/

	Boolean IACinstallsystemhandler (AEEventClass, AEEventID, ProcPtr);
	
	Boolean IACremovesystemhandler (AEEventClass, AEEventID, ProcPtr);
	
	Boolean IACnewsystemverb (OSType, OSType, AppleEvent *);
	
	void IACremovesystemhandlers (void);
	
/*iacasynch.c*/
	
	Boolean IACsendasynch (AppleEvent *, tyAsynchCallback);

/*iacops.c*/
	
	pascal short IACwaitroutine (EventRecord *, long *, RgnHandle *);
	
	extern AEIdleUPP IACwaitroutineUPP;
	
/*iacbinary.c*/
	
	Boolean IACpushbinaryparam (Handle, OSType, OSType);
	
	Boolean IACgetbinaryparam (OSType, Handle *, OSType *);
	
	Boolean IACreturnbinary (Handle, OSType);
	
	Boolean IACgetbinaryitem (AEDescList *, long, Handle *, OSType *);

	Boolean IACpushbinaryitem (AEDescList *, Handle, OSType, long);

/*iacboolean.c*/
	
	Boolean IACpushbooleanparam (Boolean val, OSType keyword);
	
	Boolean IACgetbooleanparam (OSType, Boolean *);
	
	Boolean IACreturnboolean (Boolean);
	
	Boolean IACgetbooleanitem (AEDescList *, long, Boolean *);

	Boolean IACpushbooleanitem (AEDescList *, Boolean, long);

/*iacdate.c*/
	
	Boolean IACpushdateparam (long, OSType);
	
	Boolean IACgetdateparam (OSType, long *);
	
	Boolean IACreturndate (long);
	
	Boolean IACgetdateitem (AEDescList *, long, long *);

	Boolean IACpushdateitem (AEDescList *, long, long);
	
/*iacdouble.c*/
	
	Boolean IACpushdoubleparam (double, OSType);
	
	Boolean IACgetdoubleparam (OSType, double *);
	
	Boolean IACreturndouble (double);

	Boolean IACgetdoubleitem (AEDescList *, long, double *);

	Boolean IACpushdoubleitem (AEDescList *, double, long);

/*iacfilespec.c*/
	
	Boolean IACpushfilespecparam (FSSpec *, OSType);
	
	Boolean IACgetfilespecparam (OSType, FSSpec *);
	
	Boolean IACreturnfilespec (FSSpec *);

	Boolean IACgetfilespecitem (AEDescList *, long, FSSpec *);

	Boolean IACpushfilespecitem (AEDescList *, FSSpec *, long);

/*iaclist.c*/

	Boolean IACpushlistparam (AEDescList *, OSType);

	Boolean IACreturnlist (AEDescList *);

	Boolean IACgetlistparam (OSType, AEDescList *);

	Boolean IACgetlistitem (AEDescList *, long, AEDescList *);

	Boolean IACpushlistitem (AEDescList *, AEDescList *, long);

	Boolean IACnewlist (AEDescList *);

/*iaclong.c*/
	
	Boolean IACpushlongparam (long, OSType);
	
	Boolean IACgetlongparam (OSType, long *);
	
	Boolean IACreturnlong (long);
	
	Boolean IACgetlongitem (AEDescList *, long, long *);

	Boolean IACpushlongitem (AEDescList *, long, long);

/*iacpoint.c*/
	
	Boolean IACpushpointparam (Point, OSType);
	
	Boolean IACgetpointparam (OSType, Point *);
	
	Boolean IACreturnpoint (Point);

	Boolean IACgetpointitem (AEDescList *, long, Point *);

	Boolean IACpushpointitem (AEDescList *, Point, long);

/*iacrecord.c*/

	Boolean IACpushrecordparam (AERecord *, OSType);

	Boolean IACreturnrecord (AERecord *);

	Boolean IACgetrecordparam (OSType, AERecord *);

	Boolean IACgetrecorditem (AEDescList *, long, AERecord *);

	Boolean IACpushrecorditem (AEDescList *, AERecord *, long);

	Boolean IACnewrecord (AERecord *);

	Boolean IACgetiteminfo (AERecord *, long, AEKeyword *, DescType *, Size *);

/*iacrect.c*/
	
	Boolean IACpushrectparam (Rect *, OSType);
	
	Boolean IACgetrectparam (OSType, Rect *);

	Boolean IACreturnrect (Rect *);
	
	Boolean IACgetrectitem(AEDescList *, long, Rect *);

	Boolean IACpushrectitem (AEDescList *, Rect *, long);
	
/*iacrgb.c*/
	
	Boolean IACpushRGBColorparam (RGBColor *, OSType);
	
	Boolean IACgetRGBColorparam (OSType, RGBColor *);
	
	Boolean IACreturnRGBColor (RGBColor *);

	Boolean IACgetRGBColoritem (AEDescList *, long, RGBColor *);

	Boolean IACpushRGBColoritem (AEDescList *, RGBColor *, long);

/*iacshort.c*/
	
	Boolean IACpushshortparam (short, OSType);
	
	Boolean IACgetshortparam (OSType, short *);
	
	Boolean IACreturnshort (short);
	
	Boolean IACgetshortitem (AEDescList *, long, short *);

	Boolean IACpushshortitem (AEDescList *, short, long);

/*iacstring.c*/
	
	Boolean IACpushstringparam (StringPtr, OSType);
	
	Boolean IACgetstringparam (OSType, StringPtr);
	
	Boolean IACreturnstring (StringPtr);

	Boolean IACgetstringitem (AEDescList *, long, StringPtr);

	Boolean IACpushstringitem (AEDescList *, StringPtr, long);

/*iacstring4.c*/
	
	Boolean IACpushstring4param (OSType, OSType);
	
	Boolean IACgetstring4param (OSType, OSType *);
	
	Boolean IACreturnstring4 (OSType);
	
	Boolean IACgetstring4item (AEDescList *, long, OSType *);

	Boolean IACpushstring4item (AEDescList *, OSType, long);

/*iactable.c*/
	
	Boolean IACpushtableparam (Handle, OSType);
	
	Boolean IACgettableparam (OSType, Handle *);
	
	Boolean IACreturntable (Handle);
	
	Boolean IACgettableitem (AEDescList *, long, Handle *);

	Boolean IACpushtableitem (AEDescList *, Handle, long);

/*iactext.c*/
	
	Boolean IACpushtextparam (Handle, OSType);
	
	Boolean IACgettextparam (OSType, Handle *);
	
	Boolean IACreturntext (Handle);
	
	Boolean IACgettextitem (AEDescList *, long, Handle *);

	Boolean IACpushtextitem (AEDescList *, Handle, long);

	Boolean IACpushtextitemcopy (AEDescList *, Handle, long);
	
/*iacnetwork.c*/

	Boolean IACnewnetworkverb (ConstStr255Param, OSType, OSType, AppleEvent *);
	
#endif 	// __IAC__

	

	
	
