
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

#define kbinclude /*so other includes can tell if we've been loaded*/



typedef struct tykeystrokerecord {

	unsigned char chkb;
	
	boolean flshiftkey, flcmdkey, floptionkey, flalphalock, flcontrolkey;
		
	short ctmodifiers; /*the number of booleans that are on*/

	short keycode; /*see Toolbox Event Manager -- this is the hardware key code*/
	
	boolean flkeypad: 1; /*if true, keystroke comes from numeric keypad*/
	
	boolean flautokey: 1; /*if true, keystroke is an automatic key*/
	
	tydirection keydirection; 
	} tykeystrokerecord, *ptrkeystrokerecord, **hdlkeystrokerecord;


extern tykeystrokerecord keyboardstatus;


tydirection keystroketodirection (char ch); /*prototype*/


/*prototypes*/

extern boolean iscmdperiodevent (long eventmessage, long eventwhat, long eventmodifiers);

extern boolean arrowkey (char);

extern tydirection keystroketodirection (char);

extern void setkeyboardstatus (long, long, long);

extern void keyboardclearescape (void);

extern void keyboardsetescape (void);

extern boolean keyboardescape (void);

extern void keyboardpeek (tykeystrokerecord *);

extern boolean enterkeydown (void);

extern boolean optionkeydown (void);

extern boolean cmdkeydown (void);

extern boolean shiftkeydown (void);

extern boolean controlkeydown (void);

extern short getkeyboardstartrepeattime (void);


