
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

#include "shelltypes.h"
#include "iowacore.h"
#ifndef iowaRuntime
	#include "iowaobject.h"
	#include "iowaicons.h"
#endif
#include <appletfont.h>
#include <appletresource.h>


typedef struct tycomponentrecord {
	
	ComponentInstance component;
	
	long objectTypeID;
	} tycomponentrecord, **hdlcomponentrecord;
	
	
static tycomponentrecord **componentarray = nil; /*dynamic array of components*/


static short countcomponents (void) {
	
	if (componentarray == nil)
		return (0);
		
	return ((short) (GetHandleSize ((Handle) componentarray) / sizeof (tycomponentrecord)));
	} /*countcomponents*/
	
	
static boolean addcomponent (ComponentInstance component, long objectTypeID) {

	tycomponentrecord rec;
	short ct;
	
	rec.component = component;
	
	rec.objectTypeID = objectTypeID;
	
	if (componentarray == nil)
		return (newfilledhandle (&rec, sizeof (tycomponentrecord), (Handle *) &componentarray));
	
	ct = countcomponents ();
	
	SetHandleSize ((Handle) componentarray, (ct + 1) * sizeof (tycomponentrecord));

	if (MemError () != noErr)
		return (false);
	
	(*componentarray) [ct] = rec;
	
	return (true);
	} /*addcomponent*/
	
	
static ComponentInstance findcomponent (long objectTypeID) {
	
	short ct, i;
	tycomponentrecord rec;
	
	ct = countcomponents ();
	
	for (i = 0; i < ct; i++) {
		
		rec = (*componentarray) [i];
		
		if (rec.objectTypeID == objectTypeID) 			
			return (rec.component);
		} /*for*/
	
	return (0); /*not found*/
	} /*findcomponent*/
	
//Code change by Timothy Paustian Thursday, June 29, 2000 9:42:56 PM
//Update macro
/* 2004-10-22 aradke: Not sure if this is the right thing to do for the Mach-O build,
		but at least it makes the link errors for _drawobjectGlue etc. go away
*/
#if TARGET_RT_MAC_CFM || TARGET_RT_MAC_MACHO
	enum {
		uppCallComponentProcInfo = kPascalStackBased
				| RESULT_SIZE(kFourByteCode)
				| STACK_ROUTINE_PARAMETER(1, kFourByteCode)
	};
	
	#pragma options align=mac68k
	
	static pascal ComponentResult getconfigGlue (ComponentInstance comp, tyioaconfigrecord *config) {
		
		#define	getconfigGlueParamSize	(sizeof (config))
		
		struct getconfigGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			tyioaconfigrecord *config;
			ComponentInstance comp;
		};
		
		struct getconfigGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = getconfigGlueParamSize;
		pb.componentWhat = IOAgetconfigcommand;
		pb.config = config;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		{
		return  CallComponentDispatch((ComponentParameters*) &pb);
		}
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*getconfigGlue*/
	
	
	static pascal ComponentResult drawobjectGlue (ComponentInstance comp, hdlobject h) {
		
		#define	drawobjectGlueParamSize	(sizeof (h))
		
		struct drawobjectGlueGluePB {
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct drawobjectGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = drawobjectGlueParamSize;
		pb.componentWhat = IOAdrawobjectcommand;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*drawobjecttGlue*/
	
	
	static pascal ComponentResult initobjectGlue (ComponentInstance comp, tyobject *obj) {
		
		#define	initobjectGlueParamSize	(sizeof (obj))
		
		struct initobjectGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			tyobject *obj;
			ComponentInstance comp;
		};
		
		struct initobjectGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = initobjectGlueParamSize;
		pb.componentWhat = IOAinitobjectcommand;
		pb.obj = obj;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*initobjectGlue*/
	
	
	static pascal ComponentResult debugGlue (ComponentInstance comp, hdlobject h, ptrstring errorstring) {
		
		#define	debugGlueParamSize	(sizeof (h) + sizeof (errorstring))
		
		struct debugGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			ptrstring errorstring;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct debugGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = debugGlueParamSize;
		pb.componentWhat = IOAdebugobjectcommand;
		pb.errorstring = errorstring;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*debugGlue*/
	
	
	static pascal ComponentResult clickGlue (ComponentInstance comp, hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
		
		#define	clickGlueParamSize	(sizeof (listhead) + sizeof (h) + sizeof (pt) + sizeof (short) + sizeof (short))
		
		struct clickGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			boolean fl2click;
			boolean filler;
			boolean flshiftkey;
			boolean filler2;
			Point pt;
			hdlobject h;
			hdlobject listhead;
			ComponentInstance comp;
		};
		
		struct clickGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = clickGlueParamSize;
		pb.componentWhat = IOAclickcommand;
		pb.fl2click = fl2click;
		pb.flshiftkey = flshiftkey;
		pb.pt = pt;
		pb.h = h;
		pb.listhead = listhead;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*clickGlue*/
	
	
	static pascal ComponentResult cleanGlue (ComponentInstance comp, hdlobject h, short height, short width, Rect *r) {
		
		#define	cleanGlueParamSize	(sizeof (h) + sizeof (height) + sizeof (width) + sizeof (r))
		
		struct cleanGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Rect *r;
			short width;
			short height;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct cleanGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = cleanGlueParamSize;
		pb.componentWhat = IOAcleanupcommand;
		pb.r = r;
		pb.width = width;
		pb.height = height;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*cleanGlue*/
	
	
	static pascal ComponentResult recalcGlue (ComponentInstance comp, hdlobject h, boolean flmajorrecalc) {
		
		#define	recalcGlueParamSize	(sizeof (h) + sizeof (short))
		
		struct recalcGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			boolean flmajorrecalc;
			boolean filler;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct recalcGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = recalcGlueParamSize;
		pb.componentWhat = IOArecalccommand;
		pb.flmajorrecalc = flmajorrecalc;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*recalcGlue*/
	
	
	static pascal ComponentResult editGlue (ComponentInstance comp, hdlobject h, boolean flgoin) {
		
		#define	editGlueParamSize	(sizeof (h) + sizeof (short))
		
		struct editGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			boolean flgoin;
			boolean filler;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct editGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = editGlueParamSize;
		pb.componentWhat = IOAeditobjectcommand;
		pb.flgoin = flgoin;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*editGlue*/
	
	
	static pascal ComponentResult keyGlue (ComponentInstance comp, hdlobject h, short ch) {
		
		#define	keyGlueParamSize	(sizeof (h) + sizeof (ch))
		
		struct keyGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			short ch;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct keyGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = keyGlueParamSize;
		pb.componentWhat = IOAkeystrokecommand;
		pb.ch = ch;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*keyGlue*/
	
	
	static pascal ComponentResult idleGlue (ComponentInstance comp, hdlobject h) {
		
		#define	idleGlueParamSize	(sizeof (h))
		
		struct idleGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct idleGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = idleGlueParamSize;
		pb.componentWhat = IOAidlecommand;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*idleGlue*/
	
	
	static pascal ComponentResult getvalueGlue (ComponentInstance comp, hdlobject h, Handle *r) {
		
		#define	getvalueGlueParamSize	(sizeof (h) + sizeof (r))
		
		struct getvalueGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Handle *r;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct getvalueGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = getvalueGlueParamSize;
		pb.componentWhat = IOAgetvaluecommand;
		pb.r = r;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*getvalueGlue*/
	
	
	static pascal ComponentResult setvalueGlue (ComponentInstance comp, hdlobject h, Handle hvalue) {
		
		#define	setvalueGlueParamSize	(sizeof (h) + sizeof (hvalue))
		
		struct setvalueGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Handle hvalue;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct setvalueGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = setvalueGlueParamSize;
		pb.componentWhat = IOAsetvaluecommand;
		pb.hvalue = hvalue;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*setvalueGlue*/
	
	
	static pascal ComponentResult setcursorGlue (ComponentInstance comp, hdlobject h, Point pt) {
		
		#define	setcursorGlueParamSize	(sizeof (h) + sizeof (pt))
		
		struct setcursorGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Point pt;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct setcursorGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = setcursorGlueParamSize;
		pb.componentWhat = IOAsetcursorcommand;
		pb.pt = pt;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*setcursorGlue*/
	
	
	static pascal ComponentResult unpackdataGlue (ComponentInstance comp, hdlobject h) {
		
		#define	unpackdataGlueParamSize	(sizeof (h))
		
		struct unpackdataGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct unpackdataGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = unpackdataGlueParamSize;
		pb.componentWhat = IOAunpackdatacommand;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*unpackdataGlue*/
	
	
	static pascal ComponentResult getattributesGlue (ComponentInstance comp, hdlobject h, AppleEvent *event) {
		
		#define	getattributesGlueParamSize	(sizeof (h) + sizeof (event))
		
		struct getattributesGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			AppleEvent *event;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct getattributesGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = getattributesGlueParamSize;
		pb.componentWhat = IOAgetattributescommand;
		pb.event = event;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*getattributesGlue*/
	
	
	static pascal ComponentResult setattributesGlue (ComponentInstance comp, hdlobject h, AppleEvent *event) {
		
		#define	setattributesGlueParamSize	(sizeof (h) + sizeof (event))
		
		struct setattributesGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			AppleEvent *event;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct setattributesGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = setattributesGlueParamSize;
		pb.componentWhat = IOAsetattributescommand;
		pb.event = event;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*setattributesGlue*/
	
#ifndef iowaRuntime
	
	static pascal ComponentResult geticonGlue (ComponentInstance comp, Handle *h) {
		
		#define	geticonGlueParamSize	(sizeof (h))
		
		struct geticonGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Handle *h;
			ComponentInstance comp;
		};
		
		struct geticonGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = geticonGlueParamSize;
		pb.componentWhat = IOAgeticoncommand;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*geticonGlue*/

#endif
	
	
	static pascal ComponentResult getcardGlue (ComponentInstance comp, Handle *hcard) {
		
		#define	getcardGlueParamSize	(sizeof (hcard))
		
		struct getcardGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Handle *hcard;
			ComponentInstance comp;
		};
		
		struct getcardGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = getcardGlueParamSize;
		pb.componentWhat = IOAgetcardcommand;
		pb.hcard = hcard;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*getcardGlue*/
	
	
	static pascal ComponentResult packdataGlue (ComponentInstance comp, hdlobject h, Handle *hpackeddata) {
		
		#define	packdataGlueParamSize	(sizeof (h) + sizeof (hpackeddata))
		
		struct packdataGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Handle *hpackeddata;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct packdataGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = packdataGlueParamSize;
		pb.componentWhat = IOApackdatacommand;
		pb.hpackeddata = hpackeddata;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*packdataGlue*/
	
	
	static pascal ComponentResult disposedataGlue (ComponentInstance comp, hdlobject h) {
		
		#define	disposedataGlueParamSize	(sizeof (h))
		
		struct disposedataGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct disposedataGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = disposedataGlueParamSize;
		pb.componentWhat = IOAdisposedatacommand;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*disposedataGlue*/
	
	
	static pascal ComponentResult canreplicateGlue (ComponentInstance comp, hdlobject h) {
		
		#define	canreplicateGlueParamSize	(sizeof (h))
		
		struct canreplicateGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct canreplicateGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = canreplicateGlueParamSize;
		pb.componentWhat = IOAcanreplicatcommand;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*canreplicateGlue*/
	
	
	static pascal ComponentResult catchreturnGlue (ComponentInstance comp, hdlobject h) {
		
		#define	catchreturnGlueParamSize	(sizeof (h))
		
		struct catchreturnGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct catchreturnGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = catchreturnGlueParamSize;
		pb.componentWhat = IOAcatchreturncommand;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*catchreturnGlue*/
	
	
	static pascal ComponentResult getinvalrectGlue (ComponentInstance comp, hdlobject h, Rect *r) {
		
		#define	getinvalrectGlueParamSize	(sizeof (h) + sizeof (r))
		
		struct getinvalrectGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Rect *r;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct getinvalrectGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = getinvalrectGlueParamSize;
		pb.componentWhat = IOAgetobjectinvalrectcommand;
		pb.r = r;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*getinvalrectGlue*/
	
	
	static pascal ComponentResult geteditrectGlue (ComponentInstance comp, hdlobject h, Rect *r) {
		
		#define	geteditrectGlueParamSize	(sizeof (h) + sizeof (r))
		
		struct geteditrectGlueGluePB {	/* extension of ComponentParameters system type */
			unsigned char componentFlags;
			unsigned char componentParamSize;
			short componentWhat;
			Rect *r;
			hdlobject h;
			ComponentInstance comp;
		};
		
		struct geteditrectGlueGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = geteditrectGlueParamSize;
		pb.componentWhat = IOAgetobjecteditrectcommand;
		pb.r = r;
		pb.h = h;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return  CallComponentDispatch((ComponentParameters*) &pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} /*geteditrectGlue*/

	#pragma options align=reset

#else	// not CFM

	static pascal ComponentResult getconfigGlue (ComponentInstance comp, tyioaconfigrecord *config)
		
		ComponentCallNow (IOAgetconfigcommand, sizeof (tyioaconfigrecord *)); 
		/*getconfigGlue*/
	
	
	static pascal ComponentResult drawobjectGlue (ComponentInstance comp, hdlobject h) 
		
		ComponentCallNow (IOAdrawobjectcommand, sizeof (hdlobject)); 
		/*drawobjecttGlue*/
	
	
	static pascal ComponentResult initobjectGlue (ComponentInstance comp, tyobject *obj) 
		
		ComponentCallNow (IOAinitobjectcommand, sizeof (tyobject *)); 
		/*initobjectGlue*/
	
	
	static pascal ComponentResult debugGlue (ComponentInstance comp, hdlobject h, bigstring errorstring) 
		
		ComponentCallNow (IOAdebugobjectcommand, sizeof (hdlobject) + sizeof (char *)); 
		/*debugGlue*/
	
	
	static pascal ComponentResult clickGlue (ComponentInstance comp, hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) 
		
		ComponentCallNow (IOAclickcommand, (2 * sizeof (hdlobject)) + sizeof (Point) + sizeof (short) + sizeof (short)); 
		/*clickGlue*/
	
	
	static pascal ComponentResult cleanGlue (ComponentInstance comp, hdlobject h, short height, short width, Rect *r) 
		
		ComponentCallNow (IOAcleanupcommand, sizeof (hdlobject) + sizeof (short) + sizeof (short) + sizeof (Rect *)); 
		/*cleanGlue*/
	
	
	static pascal ComponentResult recalcGlue (ComponentInstance comp, hdlobject h, boolean flmajorrecalc) 
		
		ComponentCallNow (IOArecalccommand, sizeof (hdlobject) + sizeof (short)); 
		/*recalcGlue*/
	
	
	static pascal ComponentResult editGlue (ComponentInstance comp, hdlobject h, boolean flgoin) 
		
		ComponentCallNow (IOAeditobjectcommand, sizeof (hdlobject) + sizeof (short)); 
		/*editGlue*/
	
	
	static pascal ComponentResult keyGlue (ComponentInstance comp, hdlobject h, short ch) 
		
		/*
		1/20/93 DW: last param used to be a char, but this was causing the size computed
		below to be an odd number. by changing it to a short, we keep things clean and
		working.
		*/
		
		ComponentCallNow (IOAkeystrokecommand, sizeof (hdlobject) + sizeof (short)); 
		/*keyGlue*/
	
	
	static pascal ComponentResult idleGlue (ComponentInstance comp, hdlobject h) 
		
		ComponentCallNow (IOAidlecommand, sizeof (hdlobject)); 
		/*idleGlue*/
	
	
	static pascal ComponentResult getvalueGlue (ComponentInstance comp, hdlobject h, Handle *r) 
		
		ComponentCallNow (IOAgetvaluecommand, sizeof (hdlobject) + sizeof (Handle *)); 
		/*getvalueGlue*/
	
	
	static pascal ComponentResult setvalueGlue (ComponentInstance comp, hdlobject h, Handle hvalue) 
		
		ComponentCallNow (IOAsetvaluecommand, sizeof (hdlobject) + sizeof (Handle)); 
		/*setvalueGlue*/
	
	
	static pascal ComponentResult setcursorGlue (ComponentInstance comp, hdlobject h, Point pt) 
		
		ComponentCallNow (IOAsetcursorcommand, sizeof (hdlobject) + sizeof (Point)); 
		/*setcursorGlue*/
	
	
	static pascal ComponentResult unpackdataGlue (ComponentInstance comp, hdlobject h) 
		
		ComponentCallNow (IOAunpackdatacommand, sizeof (hdlobject)); 
		/*unpackdataGlue*/
	
	
	static pascal ComponentResult getattributesGlue (ComponentInstance comp, hdlobject h, AppleEvent *event) 
		
		ComponentCallNow (IOAgetattributescommand, sizeof (hdlobject) + sizeof (AppleEvent *)); 
		/*getattributesGlue*/
	
	
	static pascal ComponentResult setattributesGlue (ComponentInstance comp, hdlobject h, AppleEvent *event) 
		
		ComponentCallNow (IOAsetattributescommand, sizeof (hdlobject) + sizeof (AppleEvent *)); 
		/*setattributesGlue*/
	

#ifndef iowaRuntime
	
	static pascal ComponentResult geticonGlue (ComponentInstance comp, Handle *h) 
		
		ComponentCallNow (IOAgeticoncommand, sizeof (Handle *)); 
		/*geticonGlue*/

#endif
	
	
	static pascal ComponentResult getcardGlue (ComponentInstance comp, Handle *hcard) 
		
		ComponentCallNow (IOAgetcardcommand, sizeof (Handle *)); 
		/*getcardGlue*/
	
	
	static pascal ComponentResult packdataGlue (ComponentInstance comp, hdlobject h, Handle *hpackeddata) 
		
		ComponentCallNow (IOApackdatacommand, sizeof (hdlobject) + sizeof (Handle *)); 
		/*packdataGlue*/
	
	
	static pascal ComponentResult disposedataGlue (ComponentInstance comp, hdlobject h) 
		
		ComponentCallNow (IOAdisposedatacommand, sizeof (hdlobject)); 
		/*disposedataGlue*/
	
	
	static pascal ComponentResult canreplicateGlue (ComponentInstance comp, hdlobject h) 
		
		ComponentCallNow (IOAcanreplicatcommand, sizeof (hdlobject)); 
		/*canreplicateGlue*/
	
	
	static pascal ComponentResult catchreturnGlue (ComponentInstance comp, hdlobject h) 
		
		ComponentCallNow (IOAcatchreturncommand, sizeof (hdlobject)); 
		/*catchreturnGlue*/
	
	
	static pascal ComponentResult getinvalrectGlue (ComponentInstance comp, hdlobject h, Rect *r) 
		
		ComponentCallNow (IOAgetobjectinvalrectcommand, sizeof (hdlobject) + sizeof (Rect *)); 
		/*getinvalrectGlue*/
	
	
	static pascal ComponentResult geteditrectGlue (ComponentInstance comp, hdlobject h, Rect *r) 
		
		ComponentCallNow (IOAgetobjecteditrectcommand, sizeof (hdlobject) + sizeof (Rect *)); 
		/*geteditrectGlue*/

#endif

boolean callinitobject (tyobject *obj) {
	
	(*obj).owningcard = iowadata; /*make card record available to the object handler*/
	
	(**iowadata).fontinfo = globalfontinfo;
	
	return (initobjectGlue (findcomponent ((*obj).objecttype), obj));
	} /*callinitobject*/
	
	
static boolean pregluecall (hdlobject h, ComponentInstance *comp) {
	
	/*
	set up the object record so the callback can find information
	that many of them need. 
	
	1/23/93 DW: beef it up to get the component id from the array and
	return false if it's 0.
	
	2/16/93 dmb: check iowadata for nil -- defensive driving
	*/
	
	(**h).owningcard = iowadata; /*make card record available to the object handler*/
	
	if (iowadata == nil)
		return (false);
	
	(**iowadata).fontinfo = globalfontinfo;
	
	*comp = findcomponent ((**h).objecttype);
	
	return (*comp != 0);
	} /*pregluecall*/
	
	
boolean calldrawobject (hdlobject h) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp)) {
		
		Rect r = (**h).objectrect;
		
		#if TARGET_API_MAC_CARBON == 1
		Pattern gray;
		GetQDGlobalsGray(&gray);
		FillRect (&r, &gray);
		#else
		
		FillRect (&r, (ConstPatternParam) &quickdrawglobal (gray));
		#endif
		
		FrameRect (&r);
		
		return (false);
		}
	
	return (drawobjectGlue (comp, h));
	} /*calldrawobject*/
	
	
boolean callclick (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (clickGlue (comp, listhead, h, pt, flshiftkey, fl2click));
	} /*callclick*/
	
	
boolean callcleanobject (hdlobject h, short height, short width, Rect *r) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (cleanGlue (comp, h, height, width, r));
	} /*callcleanobject*/
	
	
boolean callrecalcobject (hdlobject h, boolean flmajorrecalc) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (recalcGlue (comp, h, flmajorrecalc));
	} /*callrecalcobject*/
	
	
boolean calleditobject (hdlobject h, boolean flgoin) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (editGlue (comp, h, flgoin));
	} /*calleditobject*/


boolean callkeyobject (hdlobject h, char ch) {
	
	ComponentInstance comp;
	
	if (h == nil)
		return (true);
		
	if (!pregluecall (h, &comp))
		return (true);
	
	return (keyGlue (comp, h, (short) ch));
	} /*callkeyobject*/
	
	
boolean callidleobject (hdlobject h) {
	
	ComponentInstance comp;
	
	if (h == nil)
		return (true);
		
	if (!pregluecall (h, &comp))
		return (true);
	
	return (idleGlue (comp, h));
	} /*callidleobject*/
	
	
boolean callcanreplicate (hdlobject h) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (canreplicateGlue (comp, h));
	} /*callcanreplicate*/
	
	
boolean callcatchreturn (hdlobject h) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (catchreturnGlue (comp, h));
	} /*callcatchreturn*/
	
	
boolean callgetinvalrect (hdlobject h, Rect *r) {
	
	ComponentInstance comp;
	
	getobjectrect (h, r); /*allow for groups*/
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (getinvalrectGlue (comp, h, r));
	} /*callinvalobject*/
	
	
boolean callgeteditrect (hdlobject h, Rect *r) {
	
	ComponentInstance comp;
	
	*r = (**h).objectrect; /*geteditrect has special case for groups*/
	
	if (!pregluecall (h, &comp))
		return (false);
	
	getobjectrect (h, r); 
		
	return (geteditrectGlue (comp, h, r));
	} /*callgeteditrect*/
	
	
boolean callgetvalue (hdlobject h, Handle *hvalue) {
	
	ComponentInstance comp;
	
	*hvalue = nil;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (getvalueGlue (comp, h, hvalue));
	} /*callgetvalue*/
	
	
boolean callsetvalue (hdlobject h, Handle hvalue) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (setvalueGlue (comp, h, hvalue));
	} /*callsetvalue*/
	
	
boolean callsetcursor (hdlobject h, Point pt) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (setcursorGlue (comp, h, pt));
	} /*callsetcursor*/
	
	
boolean calldisposedata (hdlobject h) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (disposedataGlue (comp, h));
	} /*calldisposedata*/
	

boolean callgetattributes (hdlobject h, AppleEvent *event) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (getattributesGlue (comp, h, event));
	} /*callgetattributes*/
	
	
boolean callsetattributes (hdlobject h, AppleEvent *event) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (setattributesGlue (comp, h, event));
	} /*callsetattributes*/
	
	
boolean callgetcard (hdlobject h, Handle *hcard) {
	
	ComponentInstance comp;
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (getcardGlue (comp, hcard));
	} /*callgetcard*/
	

boolean callpackdata (hdlobject h, Handle *hpackeddata) {
	
	switch ((**h).objecttype) {
	
		case grouptype: case clonetype:
			return (true);
			
		default: {
			ComponentInstance comp;
	
			if (!pregluecall (h, &comp))
				return (false);
	
			return (packdataGlue (comp, h, hpackeddata));
			}
		} /*switch*/
	} /*callpackdata*/
	
	
boolean callunpackdata (hdlobject h) {
	
	switch ((**h).objecttype) {
	
		case grouptype: case clonetype:
			return (true);
			
		default: {
			ComponentInstance comp;
	
			if (!pregluecall (h, &comp))
				return (false);
			
			return (unpackdataGlue (comp, h));
			}
		} /*switch*/
	} /*callunpackdata*/
	
	
boolean calldebug (hdlobject h, Str255 errorstring) {
	
	ComponentInstance comp;
	
	setstringlength (errorstring, 0);
	
	if (!pregluecall (h, &comp))
		return (false);
	
	return (debugGlue (comp, h, errorstring));
	} /*calldebug*/
	
	
static boolean getconfigrecord (hdlobject h, tyioaconfigrecord *config) {
	
	ComponentInstance comp;
	
	clearbytes (config, sizeof (tyioaconfigrecord));
	
	comp = findcomponent ((**h).objecttype);
	
	if (comp == 0)
		return (false);
		
	return (getconfigGlue (comp, config));
	} /*getconfigrecord*/
	
	
boolean callgetobjectname (hdlobject h, Str255 objectname) {

	switch ((**h).objecttype) {
	
		case grouptype:
			copystring ("\pGroup", objectname);
			
			break;
			
		case clonetype:
			copystring ("\pClone", objectname);
			
			break;
			
		default: {
			tyioaconfigrecord config;
			
			getconfigrecord (h, &config);
			
			copystring (config.objectTypeName, objectname);
			
			break;
			}
		} /*switch*/
	
	return (true);
	} /*callgetobjectname*/


boolean callgetobjectflagname (hdlobject h, Str255 objectflagname) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	copystring (config.objectFlagName, objectflagname);
	
	return (true);
	} /*callgetobjectflagname*/


boolean callframewhenediting (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.frameWhenEditing);
	} /*callframewhenediting*/


boolean callcaneditvalue (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.canEditValue);
	} /*callcaneditvalue*/


boolean calltoggleflagwhenhit (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.toggleFlagWhenHit);
	} /*calltoggleflagwhenhit*/


boolean callmutallyexclusive (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.mutuallyExclusive);
	} /*callmutallyexclusive*/


boolean callspeaksforgroup (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.speaksForGroup);
	} /*callspeaksforgroup*/


boolean callhandlesmousetrack (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.handlesMouseTrack);
	} /*callhandlesmousetrack*/


boolean calleditableinrunmode (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config); 
	
	return (config.editableInRunMode);
	} /*calleditableinrunmode*/


boolean callfontaware (hdlobject h) {
	
	switch ((**h).objecttype) {
		
		case clonetype:
			return (true); /*allow centering using the Justify menu*/
	
		default: {
			tyioaconfigrecord config;
	
			getconfigrecord (h, &config);
	
			return (config.isFontAware);
			}
		} /*switch*/
	} /*callfontaware*/


boolean callalwaysidle (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.alwaysIdle);
	} /*callalwaysidle*/
	

boolean callhasspecialcard (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.hasSpecialCard);
	} /*callhasspecialcard*/
	

boolean callcantdrawinbitmap (hdlobject h) {

	tyioaconfigrecord config;
	
	getconfigrecord (h, &config);
	
	return (config.cantDrawInBitmap);
	} /*callcantdrawinbitmap*/
	

		
boolean initIOAcomponents (Handle *harray) {
	
	/*
	2/27/93 DW: return a handle to the component array so that Iowa Runtime
	can save this between calls to run a card.
	*/
	
	ComponentDescription desc;
	Component comp, prevcomponent;
	ComponentInstance instance;
	tyioaconfigrecord config;
	long ct, i;
	
	clearbytes (&desc, sizeof (desc));
	
	desc.componentType = 'IOAb';
	
	ct = CountComponents (&desc);
	
	prevcomponent = nil; /*start with the first one*/
	
	for (i = 0; i < ct; i++) {
		
		comp = FindNextComponent (prevcomponent, &desc);
		
		instance = OpenComponent (comp);
		
		getconfigGlue (instance, &config);
		
		addcomponent (instance, config.objectTypeID);
		
		#ifndef iowaRuntime
			{
			Handle hsmallicon;
			
			geticonGlue (instance, &hsmallicon);
			
			addtoiconarray (hsmallicon, config.objectTypeID);
			}
		#endif
		
		prevcomponent = comp;
		} /*for*/
	
	#ifndef iowaRuntime
	
		converticonarray ();
	
	#endif
	
	*harray = (Handle) componentarray;
	
	return (true);
	} /*initIOAcomponents*/
	

void restorecomponentarray (Handle harray) {
	
	componentarray = (hdlcomponentrecord) harray;
	} /*restorecomponentarray*/
	

boolean closeIOAcomponents (void) {
	
	short ct, i;
	
	ct = countcomponents ();
	
	for (i = 0; i < ct; i++) 
		CloseComponent ((*componentarray) [i].component);
		
	disposehandle ((Handle) componentarray);
	
	componentarray = nil;
	
	return (true);
	} /*closeIOAcomponents*/
	
	
