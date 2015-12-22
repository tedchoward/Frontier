
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

#include "mac.h"
#include "memory.h"
#include "strings.h"


#include "error.h"
#define kNumberOfMasters 128

long	gSystemVersion;
boolean	gHasColorQD;
boolean gCanUseNavServ;

tymemoryconfig macmemoryconfig;


boolean initmacintosh (void) {
	
	/*
	the magic stuff that every Macintosh application needs to do 
	before doing anything else.
	
	4/24/91 dmb: added memory config resource stuff
	
	3.0.4 dmb: use LMGetCurStackBase, not CurStackBase global
	
	3.0.4 dmb: pass 0L to InitDialogs
	*/
	
	register short i;
	register hdlmemoryconfig h;
	register long ctbytes;
	//Code change by Timothy Paustian Thursday, June 8, 2000 3:45:13 PM
	//
	#if TARGET_API_MAC_CARBON == 0
	long ctstack;
	#endif
	long ctheap, ctcode;
	short ctmasters;
	
	h = (hdlmemoryconfig) Get1Resource ('MCFG', 1);
	
	if (h == nil)
		clearbytes (&macmemoryconfig, sizeof (macmemoryconfig));
	else
		macmemoryconfig = **h;
	
	//Code change by Timothy Paustian Saturday, June 3, 2000 10:13:20 PM
	//Changed to Opaque call for Carbon
	//we don't need this in carbon.
	#if TARGET_API_MAC_CARBON == 0
	if (h != nil) { /*first check stack size*/
		
		ctbytes = (**h).minstacksize;
		
		ctstack = LMGetCurStackBase () - GetApplLimit (); /*current stack size*/
		
		if (ctbytes > ctstack)
			SetApplLimit (LMGetCurStackBase () - ctbytes);
		}
	MaxApplZone ();
	
	#endif

	
	
	if (h != nil) { /*check heap size and master pointers*/
		
		ctbytes = (**h).minheapsize;
		
		//Code change by Timothy Paustian Thursday, June 8, 2000 3:04:31 PM
		//Changed to Opaque call for Carbon
		//This is meaningless for OS X since it has unlimited memory.
		
		#if TARGET_API_MAC_CARBON == 1
		//we need to do somethings else. FreeMem is going to return some large value
		//of all the available system memory
		//This whole thing is pointless. We can get as much memory as we need.

			#pragma unused (ctmasters)
			#pragma unused (ctcode)
			#pragma unused (ctheap)
		
		#else

		ctheap = FreeMem ();
		
		if (ctbytes > ctheap) {
			
			return (false);
			}
		
		ctbytes = (**h).avghandlesize;
		
		ctcode = (**h).reserveforcode;
		
		if ((ctbytes > 0) && (ctheap > ctcode)) {
			
			ctmasters = ((ctheap - ctcode) / ctbytes) / kNumberOfMasters;
			
			ctmasters = min (ctmasters, 1024);  /*7.1b37 PBS: You'd think 1024 calls would be enough. With large memory alottments on Mac
			  						  			Classic, we're calling MoreMasters in excess of 20,000 times. That makes the app take
			  						  			a minute or so to start up! So instead I've chosen an arbitrary limit.*/
			  						  
			for (i = 1; i < ctmasters; i++) 
				MoreMasters ();
			}
		#endif

		ReleaseResource ((Handle) h); /*we're done with it*/
		}
	
	//Code change by Timothy Paustian Thursday, June 8, 2000 3:21:06 PM
	//Changed to Opaque call for Carbon
	//we don't need this initialization in carbon
	#if TARGET_API_MAC_CARBON == 0
	InitGraf (&qd.thePort);
	
	InitFonts ();
	
	FlushEvents (everyEvent, 0);
	
	InitWindows ();
	
	InitMenus ();
	
	TEInit ();
	
	InitDialogs (0L);
	#endif

	
	InitCursor ();
	//Code change by Timothy Paustian Thursday, June 8, 2000 3:22:57 PM
	//Changed to Opaque call for Carbon
	//this is obsolete, we should be using gestalt for this.
	
	{	
		long quickDrawFeatures;
		OSErr theErr = Gestalt(gestaltSystemVersion, &gSystemVersion);
		if(oserror(theErr))
			ExitToShell();

		theErr = Gestalt(gestaltQuickdrawFeatures, &quickDrawFeatures);
		
		if(oserror(theErr))
			ExitToShell();
		gHasColorQD = (quickDrawFeatures & (1 << gestaltHasColor));
		//Nav services has to be present and we want the 1.1 or greater version.
		gCanUseNavServ = (NavServicesAvailable() && (NavLibraryVersion() >= 0x01108000));
	}
	
	
	//SysEnvirons (1, &macworld);
	
	//gee I bet this isn't required anymore either.
	for (i = 1; i <= 5; i++) { /*register with Multifinder*/
		
		EventRecord ev;
		
		EventAvail (everyEvent, &ev); /*see TN180 -- splash screen*/
		} /*for*/
	
	#if TARGET_API_MAC_CARBON == 1
	
		RegisterAppearanceClient ();
		
	#endif
	
	return (true);
	} /*initmacintosh*/


#ifdef flsystem6

short countinitialfiles (void) {

	short message, ctfiles;
	
	CountAppFiles (&message, &ctfiles);
	
	if (message == 0)
		return (ctfiles);
		
	return (0);
	} /*countinitialfiles*/
	
	
void getinitialfile (short ix, bigstring fname, short *vnum) {

	AppFile appfilerecord;

	GetAppFiles (ix, &appfilerecord);
	
	copystring (appfilerecord.fName, fname);
	
	*vnum = appfilerecord.vRefNum;
	} /*getinitialfile*/

#endif

#if 0

boolean installgestaltfunction (void) {
	
	Handle hgdef;
	ProcPtr x;
	
	hgdef = GetResource ('GDEF', idgestaltfunction);
	
	if (hgdef == nil) /*didn't find gestalt definition function*/
		return (false);
	
	x = (ProcPtr) *hgdef;
	
	if (NewGestalt (idgestaltselector, x) != noErr) {
		
		if (ReplaceGestalt (idgestaltselector, x, &x) != noErr) {
			
			ReleaseResource (hgdef);
			
			return (false);
			}
		}
	
	DetachResource (hgdef);
	
	return (true);
	} /*installgestaltfunction*/

#endif



#if TARGET_API_MAC_CARBON

void WriteToConsole (char *s)
{
    CFStringRef logStr = CFStringCreateWithCString(NULL,s,kCFStringEncodingMacRoman);
	CFShow(logStr);
    CFRelease(logStr);
		
}

void DoErrorAlert(OSStatus status, CFStringRef errorFormatString)
{	
    CFStringRef formatStr = NULL, printErrorMsg = NULL;
    SInt16      alertItemHit = 0;
    Str255      stringBuf;

    if ((status != noErr) && (status != 2))           
    {
		formatStr =  CFCopyLocalizedString (errorFormatString, NULL);	
		if (formatStr != NULL)
		{
			printErrorMsg = CFStringCreateWithFormat(
													 NULL,
													 NULL,
													 formatStr,
													 status);
			if (printErrorMsg != NULL)
			{
				if (CFStringGetPascalString (
											 printErrorMsg,
											 stringBuf,
											 sizeof(stringBuf),
											 GetApplicationTextEncoding()))
				{
					StandardAlert(kAlertStopAlert, stringBuf, NULL, NULL, &alertItemHit);
				}
				CFRelease (printErrorMsg);                     
			}
			CFRelease (formatStr);                             
		}
	}
}
	
#endif

