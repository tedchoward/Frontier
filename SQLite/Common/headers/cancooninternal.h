
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

#define cancooninternalinclude /*so other guys can tell if we've been included*/



#ifndef dbinclude

	#include "db.h"

#endif

#define cancoonversionnumber 0x03

typedef struct tyversion1cancoonrecord {
	
	short versionnumber;
	
	dbaddress adrroottable;
	
	Rect msgwindowrect;
	
	diskfontstring msgfontname;
	
	short msgfontsize;
	
	Rect langerrorwindowrect;
	
	diskfontstring langerrorfontname;
	
	short langerrorfontsize;
	
	char waste [8]; /*room to grow*/
	} tyversion1cancoonrecord;
	
	
typedef struct tyversion2cancoonrecord {
	
	short versionnumber;
	
	dbaddress adrroottable;
	
	tycancoonwindowinfo windowinfo [ctwindowinfo];
	
	dbaddress adrscriptstring; /*the string that appears in the quickscript window*/
	
	unsigned short flags;
	
	short ixprimaryagent;
	
	short waste [28]; /*room to grow*/
	} tyversion2cancoonrecord;

#ifdef MACVERSION
	#define flflagdisabled_mask 0x8000 /*hide the flag?*/
	#define flpopupdisabled_mask 0x4000 /*hide the agents popup menu?*/
	#define flbigwindow_mask 0x2000 /*is the flag toggled to the big window state?*/
#endif

#ifdef WIN95VERSION
	#define flflagdisabled_mask 0x0080 /*hide the flag?*/
	#define flpopupdisabled_mask 0x0040 /*hide the agents popup menu?*/
	#define flbigwindow_mask 0x0020 /*is the flag toggled to the big window state?*/
#endif

typedef struct tyOLD42version2cancoonrecord {
	
	short versionnumber;
	
	dbaddress adrroottable;
	
	tycancoonwindowinfo windowinfo [ctwindowinfo];
	
	dbaddress adrscriptstring; /*the string that appears in the quickscript window*/
	
	unsigned short flflagdisabled: 1; /*hide the flag?*/
	
	unsigned short flpopupdisabled: 1; /*hide the agents popup menu?*/
	
	unsigned short flbigwindow: 1; /*is the flag toggled to the big window state?*/
	
	unsigned short unusedbits: 13; /*make sure the used bits never move*/
	
	short ixprimaryagent;
	
	short waste [28]; /*room to grow*/
	} tyOLD42version2cancoonrecord;


/*internal globals*/

extern hdlcancoonrecord cancoondata;

extern hdlwindowinfo cancoonwindowinfo;

extern WindowPtr cancoonwindow;


/*prototypes*/

extern boolean ccgetwindowinfo (short, tycancoonwindowinfo *); /*cancoon.c*/

extern boolean ccsetwindowinfo (short, tycancoonwindowinfo);

extern boolean ccloadfile (hdlfilenum, short);

extern boolean ccloadspecialfile (ptrfilespec, OSType);

extern boolean ccsavefile (ptrfilespec, hdlfilenum, short, boolean, boolean);

extern boolean ccnewrecord (void);

extern boolean ccdisposerecord (void);

extern boolean ccsetdatabase (void);

extern boolean ccgetdatabase (hdldatabaserecord *);

extern boolean ccsetsuperglobals (void);

extern boolean ccbackground (void);

extern boolean ccfnumchanged (hdlfilenum);

extern boolean ccfindusedblocks (void);

extern boolean ccpreclose (WindowPtr);	/*4.1b5 dmb*/

extern boolean ccclose (void);

extern boolean ccchildclose (WindowPtr);

extern boolean cceditmenubar (boolean);

extern boolean ccagentpopuphit (Rect, Point); /*cancoonpopup.c*/

extern void ccupdateagentpopup (Rect);

extern boolean ccgetprimaryagent (short *);

extern boolean ccsetprimaryagent (short);

extern boolean cccodereplaced (hdltreenode, hdltreenode);


extern boolean ccinitverbs (void); /*cancoonverbs.c*/


extern void ccwindowsetup (boolean, boolean); /*cancoonwindow.c*/

extern boolean cchelpcommand (void);

extern boolean cctoggleflag (void);

extern boolean cctoggleagentspopup (void);

extern boolean ccflipflag (void);

extern boolean ccmsg (bigstring, boolean);

extern boolean ccwindowstart (void);



