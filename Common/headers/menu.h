
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

#define menuinclude /*so that other modules can tell that we've been included*/

#ifndef shelltypesinclude
	
	#include "shelltypes.h"

#endif


#define chcommand commandMark

#define insertsubmenu -1

#define insertatend -2


/*prototypes*/

extern void drawmenubar (void);

extern void disposemenu (hdlmenu);

extern hdlmenu getresourcemenu (short);

extern boolean getcommandkeystring (byte, tykeyflags, bigstring);

//Code change by Timothy Paustian Saturday, April 29, 2000 9:30:20 PM
//Changed for UH 3.3.1, newmenu conflicts with headers def in Menus.h
extern hdlmenu Newmenu (short id, bigstring bstitle);

extern hdlmenu getmenuhandle (short);

extern boolean insertmenu (hdlmenu, long);

extern boolean inserthierarchicmenu (hdlmenu, short);

extern void removemenu (short);

extern long trackmenu (Point);

extern boolean sethierarchicalmenuitem (hdlmenu hmenu, short itemnumber, hdlmenu hsubmenu, short idsubmenu);

extern boolean gethierarchicalmenuitem (hdlmenu hmenu, short ixmenu, hdlmenu *hsubmenu);

extern void setmenutitleenable (hdlmenu, short, boolean);

extern void setmenuitemenable (hdlmenu, short, boolean);

extern boolean getmenutitleenable (hdlmenu, short);

extern boolean getmenuitemenable (hdlmenu, short);

extern void disablemenuitem (hdlmenu, short);

extern void enablemenuitem (hdlmenu, short);

extern short countmenuitems (hdlmenu);

extern void enableallmenuitems (hdlmenu, boolean);

extern void hilitemenu (short);

extern void checkmenuitem (hdlmenu, short, boolean);

extern boolean menuitemmarked (hdlmenu, short);

extern void markmenuitem (hdlmenu, short, short);

extern void stylemenuitem (hdlmenu, short, short);

extern boolean setmenutitle (hdlmenu, bigstring);

extern boolean setmenuitem (hdlmenu, short, bigstring);

extern boolean getmenuitem (hdlmenu, short, bigstring);

extern boolean setmenuitemcommandkey (hdlmenu, short, short);

extern void getmenuitemcommandkey (hdlmenu, short, short *);
//Code change by Timothy Paustian Saturday, April 29, 2000 9:31:06 PM
//Changed for UH 3.3.1 conflicts with insertmenuitem in Menus.h
extern boolean Insertmenuitem (hdlmenu, short, bigstring);

extern boolean deletemenuitem (hdlmenu, short);

extern boolean deleteallmenuitems (hdlmenu, short);

extern boolean deletelastmenuitem (hdlmenu);

extern boolean pushmenuitem (hdlmenu, short, bigstring, short);

extern boolean pushresourcemenuitems (hdlmenu, short, OSType);

extern boolean pushdottedlinemenuitem (hdlmenu);

extern boolean newtempmenu (hdlmenu *, short *);

extern short getprevmenuitem (hdlmenu);

extern short getnextmenuitem (hdlmenu);

extern boolean initmenusystem (void);



