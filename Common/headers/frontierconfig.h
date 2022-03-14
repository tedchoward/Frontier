
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

#define configinclude /*so other includes can tell if we've been loaded*/

#ifdef MACVERSION
#include <standard.h>
#endif

#ifdef WIN95VERSION 
#include "standard.h"
#endif

/*resnums of 'cnfg' resources for window types*/

#define idmenueditorconfig 128
#define idiowaconfig 129 /*3/18/92 dmb*/
#define idoutlineconfig 130 
#define idscriptconfig 131
#define idtableconfig 132
#define xxxidtextconfig 133
#define idwpconfig 134
#define xxxidmailconfig 135
#define xxxidprogressconfig 136
#define	idaboutconfig 137


#if TARGET_API_MAC_CARBON == 1
	#define idcommandconfig 146 /*OS X uses a normal window*/
	#define idlangerrorconfig 145
#else
	#define idcommandconfig 138
	#define idlangerrorconfig 139
#endif

#define idstatsconfig 140
#define idpictconfig 141
#define idcancoonconfig 142
#define idlangdialogconfig 143

#ifdef MACVERSION
	#define idplayerconfig 144 /*PBS 7.0b4: QuickTime Player window*/
#endif


#define typeunknown '\?\?\?\?'

#pragma pack(2)
typedef struct tyconfigrecord {

	short flhorizscroll; /*window has horiz scrollbar?*/
	
	short flvertscroll;
	
	short flwindowfloats; /*is it a floating palette window?*/
	
	short flmessagearea; /*allocate space for a message area?*/
	
	short flinsetcontentrect; /*if true we inset by 3 pixels*/
	
	short flnewonlaunch;
	
	short flopenresfile;
	
	short fldialog; /*do a GetNewDialog on creating one of these windows?*/
	
	short flgrowable; /*provide a grow box for window*/
	
	short flcreateonnew;
	
	short flwindoidscrollbars;
	
	short flstoredindatabase;
	
	short flparentwindowhandlessave;
	
	short fleraseonresize;
	
	short fldontconsumefrontclicks; 
	
	short flcolorwindow; 

	short messageareafraction;
	
	OSType filecreator, filetype;
	
	short templateresnum; 
	
	Rect rmin; /*for growable windows, the minimum size allowed*/
	
	short defaultfont; /*version on disk indexes into a STR# list*/
	
	short defaultsize;
	
	short defaultstyle;
	
	short idbuttonstringlist; /*if 0, no buttons for this window type*/
	
	Rect defaultwindowrect; /*new windows come up in this spot*/
	} tyconfigrecord, *ptrconfigrecord, **hdlconfigrecord;
#pragma options align=reset

/*global variables*/

extern tyconfigrecord config; /*load from resource file on initialization*/

extern short iddefaultconfig; /*the type of window a New command creates - set in main.c*/


/*prototypes*/

extern void loadconfigresource (short, tyconfigrecord *);

extern boolean saveconfigresource (short, tyconfigrecord *);

extern boolean getprogramname (bigstring);

extern boolean getuntitledfilename (bigstring);

extern boolean getdefaultfilename (bigstring);

extern boolean getusername (bigstring);

extern void initconfig (void);




