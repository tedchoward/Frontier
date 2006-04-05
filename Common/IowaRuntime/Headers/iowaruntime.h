
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

#ifndef iowaruntimeinclude
#define iowaruntimeinclude

#ifndef iowacoreinclude

	#include "iowacore.h"

#endif


#define iowaInitEvent 1000 /*constants for (*ev).what in callbacks*/
#define iowaButtonHitEvent 1001
#define iowaCloseEvent 1002
#define iowaRunScriptEvent 1003
#define iowaCancelEvent 1004 
#define iowaSetHandleEvent 1005
#define iowaOkeyDokeyEvent 1006 /*dmb 1.0b21*/


typedef boolean (*tycardwindowvisitroutine) (WindowPtr);

typedef void (*tycardeventcallback) (EventRecord *); 


typedef struct tyruntimerecord {
	
	hdlcard hc; /*the unpacked version of the card*/
	
	WindowPtr macwindow; /*the window used to display the card*/
	
	boolean initfields; /*set true when the card starts up, consumed when the first event comes in*/
	
	boolean modeless; /*true if it's a modeless card, false if modal*/
	
	boolean closethiswindow; /*if true it's time to close this card's window*/
	
	boolean ownwindow; /*if true we created this window*/
	
	tycardeventcallback callback; /*a C routine that events filter thru*/
	} tyruntimerecord, **hdlruntimerecord;


extern hdlruntimerecord runtimedata;

extern boolean flquickrecalc; /*set false in cardrunner.c, under special circumstances*/

extern boolean flexitcardloop; /*for modal cards, the loop should exit if true*/

extern boolean flalertdialogrunning; /*prevent nested alerts*/


boolean initIowaRuntime (void);

WindowPtr newCardWindow (Point, boolean, Handle);

boolean handleCardEvent (WindowPtr w, EventRecord *, boolean *);

boolean cardEditCommand (short);

boolean isCardWindow (WindowPtr);

boolean cardToFront (bigstring);

boolean recalcCardWindow (WindowPtr, boolean);

boolean visitCardWindows (tycardwindowvisitroutine);

short countCardWindows (void);

boolean closeCardWindow (WindowPtr);

void closeAllCardWindows (void);

void getCardData (WindowPtr, hdlruntimerecord *); 

boolean isModelessCardEvent (EventRecord *, boolean *);

boolean cardIsModal (Handle);

void installRuntimeEventHandlers (void); /*1.0b20 dmb*/

boolean runModalCard (Handle, boolean, short, short, tycardeventcallback);

boolean runModelessCard (Handle, boolean, short, short, tycardeventcallback);

boolean runCard (Handle, boolean, short, short, tycardeventcallback);

// 2006-04-03 - kw --- renamed parameter
boolean runHostedCard (WindowPtr hostwindow, Handle hpackedcard, boolean flscriptedcard, tycardeventcallback pcallback);

boolean setObjectValue (hdlcard, bigstring, Handle);

boolean getObjectValue (hdlcard, bigstring, Handle *);

boolean getObjectHandle (hdlcard, bigstring, hdlobject *);

boolean recalcObject (hdlobject);

boolean updateCard (hdlcard);

WindowPtr getcardrecordwindow (hdlcard card);

#endif
