
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <AppleEvents.h>


static void initMacintosh (void) {

	/*
	the magic stuff that every Macintosh application needs to do 
	before doing anything else.
	*/

	register short i;
		
	MaxApplZone ();
	
	for (i = 0; i < 10; i++) 
		MoreMasters ();
	
	InitGraf (&qd.thePort);
	
	InitFonts ();
	
	FlushEvents (everyEvent, 0);
	
	InitWindows ();
	
	InitMenus ();
	
	TEInit ();
	
	InitDialogs (0L);
	
	InitCursor ();
	
	for (i = 0; i < 5; i++) { /*register with Multifinder*/
		
		EventRecord ev;
		
		EventAvail (everyEvent, &ev); /*see TN180 -- splash screen*/
		} /*for*/
	} /*initMacintosh*/


static Boolean sendMessageToServer (OSType appid, OSType eventclass, OSType eventid, short i) {
	
	AEAddressDesc adr = {typeNull, nil}; 
	AppleEvent event = {typeNull, nil};
	AppleEvent reply = {typeNull, nil};
	OSErr ec;
	
	/*1. create the Apple Event, address it to the server app*/ {
	
		ec = AECreateDesc (typeApplSignature, (Ptr) &appid, sizeof (appid), &adr);
		
		if (ec != noErr)
			goto error;
			
		ec = AECreateAppleEvent (
			eventclass, eventid, &adr, 
			
			kAutoGenerateReturnID, kAnyTransactionID, 
			
			&event);
			
		if (ec != noErr)
			goto error;
		}
	
	/*2. convert the number to a string, push it onto the Apple Event*/ {
		
		Str255 s;
		
		NumToString (i, s);
		
		if (AEPutParamPtr (&event, keyDirectObject, typeChar, (Ptr) &s [1], s [0]) != noErr)
			goto error;
		}
		
	/*3. send the message, wait for a reply*/ {
	
		ec = AESend (		
			&event, &reply, kAEWaitReply + kAECanInteract + kAECanSwitchLayer, 
			
			kAENormalPriority, kNoTimeOut, nil, nil);
			
		if (ec != noErr)
			goto error;
		}
	
	/*4. make sure we got the correct reply*/ {
	
		DescType actualtype;
		Size actualsize;
		long num;

		ec = AEGetParamPtr (
			&reply, keyDirectObject, typeLongInteger, 
		
			&actualtype, (Ptr) &num, sizeof (num), &actualsize);
			
		if (ec != noErr)
			goto error;
		
		if (num != i) {
			
			DebugStr ("\pThe reply is not what we expected.");
			
			goto error;
			}
		}
	
	AEDisposeDesc (&adr);
		
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);	
	
	return (true);
	
	error:
	
	/*report the error code using DebugStr*/ {
		
		Str255 s;
		
		NumToString (ec, s);
		
		DebugStr (s);
		}
	
	AEDisposeDesc (&adr);
		
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);	
	
	return (false);
	} /*sendMessageToServer*/
	
	
void main (void) {
	
	short i;
	
	initMacintosh ();
	
	for (i = 1; i <= 100; i++) 
		if (!sendMessageToServer ('AESV', 'SERV', 'DISP', i))
			break;
	} /*main*/




