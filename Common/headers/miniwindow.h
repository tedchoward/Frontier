
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

#define miniwindowinclude /*so other modules can tell we've been loaded*/


#ifndef shellinclude

	#include "shell.h"
	
#endif


#ifndef langinclude

	#include "lang.h" /*our record has a valuerecord in it*/
	
#endif

#ifndef popupinclude

	#include "popup.h"

#endif

#define maxtextitems 2 

#define maxpopups 2


typedef boolean (*minisavestringcallback) (short, Handle); 
	
typedef boolean (*miniloadstringcallback) (short, Handle *);
	
typedef boolean (*minitexthitcallback) (Point);
	

typedef struct tyminirecord {
	
	short idconfig; /*resource number for config that governs this window*/
	
	short windowtype; /*an index into cancoon.h's windowinfo array*/
	
	short cttextitems; /*actual number of valid item numbers*/
	
	short textitems [maxtextitems]; /*dialog item numbers of the text items*/
	
	Rect textrects [maxtextitems]; /*location of text items*/
	
	struct tywprecord ** textdata [maxtextitems]; /*data of text items*/
	
	short ctpopups; /*actual number of popup menus*/
	
	Rect popuprects [maxpopups]; /*each rect covers a whole popup item, including arrow*/
	
	hdlstring popupmessages [maxpopups]; /*the string displayed in the rect*/
	
	short popupnumber; /*during a popup hit, this is the index of the popup, else undefined*/
	
	short activetextitem; /*index of active text item*/
	
	Rect iconrect; /*location of the icon, for mouse clicks, cursor adjustment*/
	
	byte iconlabel [11]; /*max 10 chars for string under icon, length byte at head*/
	
	byte windowtitle [33]; /*max 32 chars for window title, length byte at head*/
	
	tyvaluerecord minivalue; /*any miniwindow can have a value associated with it*/
	
	Rect msgrect; /*location of messages displayed in this window*/
	
	bigstring bsmsg; /*string displayed in msgrect, result of shell.windowmessage call*/
	
	long forecolor, backcolor; /*uses the old QuickDraw colors, should be modernized*/
	
	boolean fliconenabled; /*enabled if there's text that can be run*/
	
	boolean flactive; /*determines whether items are drawn as active or inactive*/
	
	boolean flmassiveupdate; /*erase whole rect, use offscreen bitmap*/
	
	boolean flselectallpending; /*selectall in idle, if flactive is true*/
	
	boolean flbitmapactive; /*for some drawing, if true don't open a bitmap*/
	
	minisavestringcallback savestringroutine; 
	
	miniloadstringcallback loadstringroutine;
	
	shellcallback setvalueroutine; /*fills in the minivalue field of this record*/
	
	minitexthitcallback texthitroutine;
	
	shellvoidcallback iconenableroutine;
	
	shellcallback iconhitroutine;
	
	fillpopupcallback fillpopuproutine;
	
	popupselectcallback popupselectroutine;
	
	shellshortcallback gettargetdataroutine;
	
	long minirefcon; /*optional data for the next layer up*/
	} tyminirecord, *ptrminirecord, **hdlminirecord;


/*globals*/

extern WindowPtr miniwindow;

extern hdlwindowinfo miniwindowinfo;

extern hdlminirecord minidata;


/*prototypes*/

extern boolean minisetstring (short, Handle);

extern boolean minigetstring (short, Handle *);

extern boolean minigetselstring (short, bigstring);

extern boolean minisetpopupmessage (short, bigstring);

extern void minisetselect (short, short);

extern boolean miniinvalicon (short);

extern boolean minisetwindowmessage (short, bigstring);

extern boolean startminidialog (short, callback);

extern boolean ministart (short);




