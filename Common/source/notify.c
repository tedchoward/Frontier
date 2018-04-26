
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

#include "memory.h"
#include "strings.h"
#include "lang.h"
#include "shell.h"
#include "process.h"
#include "notify.h"
#include "dialogs.h"
#include "versions.h"


#pragma pack(2)
typedef struct tyiconfamily {
	
	Handle hicl1, hicl4, hicl8, hmskl;
	
	Handle hics1, hics4, hics8, hmsks;
	
	long unknown1, unknown2;
	} tyiconfamily, **hdliconfamily;
#pragma options align=reset


boolean notifyuser (bigstring bsmessage) {
	
	/*
	use the Notification Manager to ask the user to bring our app to the front.
	
	note that we must allocate the record in the heap because multi-threading 
	makes stack addresses non-persistent.
	
	1/18/93 dmb: langbackgroundtask now takes flresting parameter; don't set global
	
	6/9/93 dmb: don't ignore the result of the background callbacks
	
	2.1b5 dmb: if we're in the main thread, need to do same as if yield is disabled.

	7.0d6 PBS: In Pike, the header of the dialog should not read UserLand Frontier,
	it should be UserLand [Whatever]. At this writing, [Whatever] is still undefined,
	so we'll go with Whatever for the moment.
	*/
	

	NMRecPtr pb;
	tyiconfamily icons;
	OSErr errcode;
	boolean fl = true;

	#define systemevents (osMask | updateMask | activMask | highLevelEventMask)

	
		{
		
			SInt16 itemhit = 0;
			OSErr err = noErr;
			
			err = StandardAlert (kAlertNoteAlert, bsmessage, nil, nil, &itemhit);
	
			return (err == noErr);
		}
	
	
	pb = (NMRecPtr) NewPtrClear (longsizeof (NMRec));
	
	if (pb == nil)
		return (false);
	/*
	clearbytes (&pb, longsizeof (pb));
	*/
	
	(*pb).qType = nmType;
	
	(*pb).nmMark = 1;
	
	clearbytes (&icons, longsizeof (icons));
	
	icons.hics1 = GetResource ('ics#', 128);
	
	icons.hics4 = GetResource ('ics4', 128);
	
	icons.hics8 = GetResource ('ics8', 128);
	
	newfilledhandle (&icons, longsizeof (tyiconfamily), &(*pb).nmIcon);
	
	(*pb).nmSound = (Handle) -1;
	
	if (isemptystring (bsmessage))
		(*pb).nmStr = nil;
	
	else {
		(*pb).nmStr = (StringPtr) NewPtr (stringsize (bsmessage));
		
		copystring (bsmessage, (*pb).nmStr);
		}
	
	(*pb).nmResp = nil;
	
	errcode = NMInstall (pb);
	
	if (errcode == noErr) {
		
		while (!shellisactive ()) {
			
			if (flscriptrunning) {
				
				if (fldisableyield || inmainthread ())
					fl = langpartialeventloop (systemevents);
				else
					fl = langbackgroundtask (true); /*let main thread field events*/
				}
			else
				fl = shellpartialeventloop (systemevents);
			
			if (!fl)
				break;
			}
		
		NMRemove (pb);
		}
	
	disposehandle ((*pb).nmIcon);
	
	if ((*pb).nmStr != nil)
		DisposePtr ((Ptr) (*pb).nmStr);
	
	DisposePtr ((Ptr) pb);

	return (fl && (errcode == noErr));

	} /*notifyuser*/






