
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "iacinternal.h"


/*
7/16/94 dmb: Updated for Univeral Headers, PowerPC
*/


#define ctreplyhandlers 10

typedef struct tyreplyhandler {
	
	long id;
	
	tyAsynchCallback callback;
	} tyreplyhandler;
	
tyreplyhandler replyhandlers [ctreplyhandlers];




static Boolean IACaddreplyhandler (tyAsynchCallback callback, long id) {
	
	short i;
	
	for (i = 0; i < ctreplyhandlers; i++) { /*find an empty slot*/
		
		if (replyhandlers [i].id == 0) {
			
			replyhandlers [i].id = id;
			
			replyhandlers [i].callback = callback;
			
			return (true);
			}
		} /*for*/
	
	return (false); /*all slots are in use*/
	} /*IACaddreplyhandler*/


static pascal OSErr IAChandleanswer (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	long replyid;
	short i;
	
	if (!IACgetlongattr (event, keyReturnIDAttr, typeLongInteger, &replyid))
		return (errAEEventNotHandled);
		
	for (i = 0; i < ctreplyhandlers; i++) {
		
		if (replyhandlers [i].id == replyid) {
			
			replyhandlers [i].id = 0;
			
			(*replyhandlers [i].callback) (event);
			}
		} /*for*/
		
	return (errAEEventNotHandled);
	} /*IAChandleanswer*/


static Boolean IACasynchinitcheck (void) {
	
	static Boolean fl = false;
	short i;
	
	if (fl) /*it's already been initialized*/
		return (true);

	for (i = 0; i < ctreplyhandlers; i++) 	
		replyhandlers [i].id = 0;
		
	fl = IACinstallhandler (kCoreEventClass, kAEAnswer, (ProcPtr) &IAChandleanswer);
	
	return (fl);
	} /*IACasynchinitcheck*/
	

Boolean IACsendasynch (AppleEvent *event, tyAsynchCallback callback) {

	AppleEvent reply;
	OSErr ec;
	long replyid;
	
	if (!IACasynchinitcheck ())
		return (false);
	
	if (!IACgetlongattr (event, keyReturnIDAttr, typeLongInteger, &replyid))
		return (false);
	
	if (callback != nil) { /*DW 7/30/93*/
	
		if (!IACaddreplyhandler (callback, replyid))
			return (false);
		}
	
	ec = AESend (
		
		event, &reply, kAEQueueReply + kAECanInteract + kAECanSwitchLayer, 
		
		kAENormalPriority, kNoTimeOut, IACwaitroutineUPP, nil);
	
	AEDisposeDesc (event);	
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACsendasynch*/


