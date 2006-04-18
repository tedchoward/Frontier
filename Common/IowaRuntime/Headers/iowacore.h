
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

#ifndef iowacoreinclude
#define iowacoreinclude

#include "byteorder.h"	/* 2006-04-17 aradke: for SWAP_BYTE_ORDER */

#ifdef coderesource /*we're building into a UCMD*/
	
	#define ucmd /*do things to save space*/

#endif


#ifndef texteditinclude
	
	#include <applettextedit.h>

#endif


#ifndef memoryinclude

	#include <appletmemory.h>

#endif


#ifndef stringsinclude

	#include <appletstrings.h>

#endif


#ifndef appletpopupinclude

	#include <appletpopup.h>

#endif


#ifndef ioainclude
	
	#include <ioa.h>
	
#endif

#ifndef iowavalidateinclude

	#include "iowavalidate.h"

#endif


extern hdlcard iowadata; /*points to a card record, the one we're currently working with*/

extern boolean displaydebug;

#define checkboxinset 5 /*the number of pixels to skip between checkbox and text*/

#define uistringlist 130
#define initialpopupscript 1
#define pastepicturehere 2
#define defaulticonresnum 3
#define shadowdepthprompt 4
#define setgridprompt 5
#define untitledobjectname 6


typedef struct tydiskheader {
	
	short versionnumber;
	
	short v2backcolor;
	
	long lenembeddedtable; /*only in version 2 records*/
	
	short defaultfillcolor;
	
	short defaulttextcolor;
	
	short defaultframecolor;

#ifdef SWAP_BYTE_ORDER
	
	unsigned short wastebits: 10; /*room for more booleans*/
	
	unsigned short floater: 1;
	
	unsigned short flinvisiblegrid: 1; 
	
	unsigned short flselection: 1;
	
	unsigned short flgrid: 1;
	
	unsigned short defaulttransparent: 1;

	unsigned short defaulthasframe: 1;

#else

	unsigned short defaulthasframe: 1;
	
	unsigned short defaulttransparent: 1;
	
	unsigned short flgrid: 1;
	
	unsigned short flselection: 1;
	
	unsigned short flinvisiblegrid: 1; 
	
	unsigned short floater: 1;
	
	unsigned short wastebits: 10; /*room for more booleans*/

#endif

	short gridunits;
	
	short rightborder, bottomborder;
	
	RGBColor backcolor;
	
	long idwindow;
	
	long lenwindowtitle;

	char waste [64]; /*room for growth*/
	} tydiskheader;
	
	
typedef struct tydiskobject {
	
	short versionnumber; /*currently set at 3*/
	
	unsigned char v1objecttype; /*this is where v1 records stored the object type*/
	
	Rect objectrect;
	
	long lenname, lenvalue, lenscript, lendata;
	
	char objectfont [32]; /*name of the the object's font*/
	
	short objectfontsize;
	
	short objectstyle;
	
	short objectjustification;
	
	short objectrecalcstatus;
	
	short objectrecalcperiod;
	
	short unused1;
	
	short objectdropshadowdepth;
	
	short objectlinespacing;
	
	short objectindentation;

#ifdef SWAP_BYTE_ORDER
	
	unsigned short wastebits: 9;	/* room for more booleans */
	
	unsigned short objectdisabled: 1;
	
	unsigned short objectfastscript: 1;
	
	unsigned short objecttransparent: 1;
	
	unsigned short lastinlist: 1;
	
	unsigned short objectinvisible: 1;
	
	unsigned short objectflag: 1;

	unsigned short objecthasframe: 1;

#else

	unsigned short objecthasframe: 1;
	
	unsigned short objectflag: 1;
	
	unsigned short objectinvisible: 1;
	
	unsigned short lastinlist: 1;
	
	unsigned short objecttransparent: 1;
	
	unsigned short objectfastscript: 1;
	
	unsigned short objectdisabled: 1;
	
	unsigned short wastebits: 9;	/* room for more booleans */

#endif

	long objecttype; /*room for lots of object types*/
	
	RGBColor objectfillcolor, objecttextcolor, objectframecolor;
	
	long objectlanguage; /*component subtype for linked script's interpreter*/ 
	
	long lenrecalcscript;
	
	char waste [2]; /*room for growth*/
	} tydiskobject;
	
	
typedef struct tyunpackglobals {

	ptrchar p, lastp;
	
	short hoffset, voffset;
	} tyunpackglobals;
	

extern tyunpackglobals unpack;


#ifndef iowaRuntime /*it's card editor building, not card runner*/
	
	#ifndef iowainclude
	
		#include "iowa.h"
	
	#endif

#endif

short makemultiple (short, short);

short closestmultiple (short, short);

boolean iowaopenbitmap (Rect);

boolean iowaclosebitmap (void);

boolean iowagetstring (short, bigstring);

boolean geteditrect (hdlobject, Rect *);

void getobjectnamestring (hdlobject, bigstring);

void getobjectname (hdlobject, Handle *);

void getobjectvalue (hdlobject, Handle *);

void getobjectscript (hdlobject, Handle *);

void getobjectrecalcscript (hdlobject, Handle *);

void getobjectsize (hdlobject, short *, short *);

void setobjectvalue (hdlobject, Handle);

void setobjectscript (hdlobject, Handle);

void setobjectrecalcscript (hdlobject, Handle);

void setobjectname (hdlobject, Handle);

void setobjecterrormessage (hdlobject, Handle);

void getgrouprect (hdlobject, Rect *);

boolean getobjectrect (hdlobject, Rect *);

void getobjectframe (hdlobject, Rect *);

void drawobjectlist (hdlobject x); 

void drawobject (hdlobject);

void drawobjectnow (hdlobject);

void pushobjectstyle (hdlobject);

void popobjectstyle (void);

boolean inselection (hdlobject);

boolean iowaupdate (void);

void debugrect (Rect);

void waitfornexttick (void);

boolean findterminalobject (Point, hdlobject, hdlobject *, hdlobject *);

boolean setactivetextobject (hdlobject);

void clearactivetextobject (void);

hdleditrecord getactiveeditrecord (void);

boolean visitobjects (hdlobject, tyobjectvisitroutine);

boolean visittoplevelobjects (hdlobject, tyobjectvisitroutine);

void getobjectinvalrect (hdlobject, Rect *);

void invalobject (hdlobject);

void validobject (hdlobject);

void toggleobjectflag (hdlobject);

boolean iowasetpictrect (void);

void sortobjectlist (void);

void laythread (void);

boolean findobject (Point, hdlobject, hdlobject *);

boolean getnthobject (hdlobject, short, hdlobject *);

short countobjects (hdlobject);

void getchildobjectlist (hdlobject, hdlobject *);

void iowainvalrect (Rect *);

void iowaupdatenow (void);

boolean iowaunpack (Handle);

void clearalltmpbits (void);

void getlastinlist (hdlobject, hdlobject *);

void disposeobjectlist (hdlobject);

void disposeobject (hdlobject);

boolean iowadisposerecord (void);

boolean lookupname (bigstring, hdlobject *);

boolean iowapreupdate (void);

void pushtemphandle (Handle);

void releasetemphandles (void);

void disposetemphandle (Handle);

boolean iowagetbooleanvalue (hdlobject, Handle *);

boolean iowasetbooleanvalue (hdlobject, Handle);

boolean iowagetstringvalue (hdlobject, Handle *);

void checkrecalc (void);

void schedulerecalc (hdlobject, short);

boolean recalcobjectvalue (hdlobject);

boolean runbuttonscript (hdlobject);

void recalcobject (hdlobject);

boolean recalcclone (hdlobject);

boolean derefclone (hdlobject *);

boolean minorrecalcvisit (hdlobject);

boolean majorrecalcvisit (hdlobject);

boolean initexclusiveobjectsvisit (hdlobject);

void turnonfirstexclusiveobject (hdlobject);

void turnoffallexclusiveobjects (hdlobject);

short groupvertfudge (hdlobject);

boolean setCardValue (bigstring, Handle);

boolean setCardValueCopy (bigstring, Handle);

boolean setCardString (bigstring, bigstring);

boolean getCardValue (bigstring, Handle *);

boolean getCardValueCopy (bigstring, Handle *);

boolean setCardFlag (bigstring, boolean);

boolean getCardFlag (bigstring, boolean *);

boolean unlinkobject (hdlobject);

void getgrouprect (hdlobject, Rect *); 

void movegroupto (hdlobject, short, short); 

boolean putobjectrect (hdlobject, Rect);

boolean initIOAcomponents (Handle *);

void restorecomponentarray (Handle);

boolean closeIOAcomponents (void);

boolean callinitobject (tyobject *);

boolean calldrawobject (hdlobject);

boolean calleditobject (hdlobject, boolean);

boolean callidleobject (hdlobject);

boolean callkeyobject (hdlobject, char);

boolean callclick (hdlobject, hdlobject, Point, boolean, boolean);

boolean callcleanobject (hdlobject, short, short, Rect *);

boolean callrecalcobject (hdlobject, boolean);

boolean callcanreplicate (hdlobject);

boolean callgetinvalrect (hdlobject, Rect *);

boolean callgeteditrect (hdlobject, Rect *);

boolean callgetvalue (hdlobject, Handle *);

boolean callsetvalue (hdlobject, Handle);

boolean callsetcursor (hdlobject, Point);

boolean callcatchreturn (hdlobject);

boolean callunpackdata (hdlobject);

boolean calldisposedata (hdlobject);

boolean callgetattributes (hdlobject, AppleEvent *);

boolean callsetattributes (hdlobject, AppleEvent *);

boolean callgetcard (hdlobject, Handle *);

boolean callpackdata (hdlobject, Handle *);

boolean calldebug (hdlobject, Str255);

boolean callgetobjectname (hdlobject, Str255);

boolean callgetobjectflagname (hdlobject, Str255);

boolean callframewhenediting (hdlobject);

boolean callcaneditvalue (hdlobject);

boolean calltoggleflagwhenhit (hdlobject);

boolean callmutallyexclusive (hdlobject);

boolean callspeaksforgroup (hdlobject);

boolean callhandlesmousetrack (hdlobject);

boolean calleditableinrunmode (hdlobject);

boolean callfontaware (hdlobject);

boolean callalwaysidle (hdlobject);

boolean callhasspecialcard (hdlobject);

boolean callcantdrawinbitmap (hdlobject);

void initIOAcallbacks (void);

void geteventwindow (EventRecord *, WindowPtr *);

boolean alertdialog (bigstring);

boolean recalcbottleneck (hdlobject, boolean);

boolean runcardscript (Handle, long, boolean, bigstring, Handle *);

boolean cardFastScript (bigstring, Handle, bigstring);

boolean showinvisiblesloop (void);

void editclickbottleneck (Point, boolean, hdleditrecord);

boolean autonameobject (hdlobject);

void setnubonborder (void);


#endif	// iowacoreinclude
