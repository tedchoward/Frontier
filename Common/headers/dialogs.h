
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

#ifndef dialogsinclude
#define dialogsinclude /*so other includes can tell if we've been loaded*/

#define okitem 1 /*can be used by any dialog if something more specific isn't needed*/
#define cancelitem 2

#define quitdialogid 254 /*standard “quit” dialog (shares items with Save)*/

#define savedialogid 256 /*standard “save” dialog*/
#define saveyesitem 1
#define savecancelitem 2
#define savenoitem 3
#define savefileprompt 4
#define savequitprompt 5

#define replacedialogid 257
#define replacereplaceitem 1
#define replaceduplicateitem 4
#define replacecancelitem 5

#define askdialogid 262 /*standard “ask” dialog for UserLand language*/
#define askokitem 1
#define askcancelitem 2
#define askpromptitem 3
#define askansweritem 4

#define msgdialogid 263 /*standard “msg” dialog for UserLand language*/
#define msgokitem 1
#define msgcancelitem 2
#define msgmsgitem 3

#define twowaydialogid 263 /*standard “msg” dialog for UserLand language*/
#define twowayokitem 1
#define twowaycancelitem 2
#define twowaymsgitem 3

#define threewaydialogid 264 /*standard “msg” dialog for UserLand language*/
#define threewayyesitem 1
#define threewaynoitem 2
#define threewaycancelitem 3
#define threewaymsgitem 4

#define intdialogid 265 /*dialog for prompting for a single number*/
#define intokitem 1
#define intcancelitem 2
#define intpromptitem 3
#define intintitem 4

#define chardialogid 266 /*dialog for prompting for a single character*/
#define charokitem 1
#define charcancelitem 2
#define charpromptitem 3
#define charvalitem 4

#define sferrordialogid 260 /*error while std file is up*/

#define alertdialogid 261 /*standard “alert” dialog for UserLand language*/
#define alertokitem 1
#define alertmsgitem 3

#define revertdialogid 269 /*standard “revert” dialog for UserLand language*/
#define revertokitem 1

#define newvaluedialogid 255 /*zooming unknown value*/
#define valuenameitem 4
#define firstkinditem 5
#define lastkinditem 10


typedef boolean (*dialogcallback) (DialogPtr, short);


/*function prototypes*/

extern short dialogcountitems (DialogPtr);

extern void boldenbutton (DialogPtr, short);

extern void positiondialogwindow (DialogPtr);

extern void disabledialogitem (DialogPtr, short);

extern void enabledialogitem (DialogPtr, short);

extern void hidedialogitem (DialogPtr, short);

extern void showdialogitem (DialogPtr, short);

extern void setdefaultitem (DialogPtr, short);

extern boolean dialogitemisbutton (DialogPtr, short);

extern DialogPtr newmodaldialog (short, short);

extern void disposemodaldialog (DialogPtr);

extern void setdialogcheckbox (DialogPtr, short, boolean);

extern boolean getdialogcheckbox (DialogPtr, short);

extern void toggledialogcheckbox (DialogPtr, short);

extern boolean setdialogradiovalue (DialogPtr, short, short, short);

extern short getdialogradiovalue (DialogPtr, short, short);

extern void setdialogicon (DialogPtr, short, short);

extern void setdialogtext (DialogPtr, short, bigstring);

extern void getdialogtext (DialogPtr, short, bigstring);

extern void selectdialogtext (DialogPtr, short);

extern short getdialogint (DialogPtr, short);

extern void setdialogint (DialogPtr, short, short);

extern OSType getdialogostype (DialogPtr, short);

extern void setdialogostype (DialogPtr, short, OSType);

extern void setdialogbutton (DialogPtr, short, bigstring);

extern void dialoggetobjectrect (DialogPtr, short, Rect *);

extern void dialogsetobjectrect (DialogPtr, short, Rect);

extern boolean dialogsetfontsize (DialogPtr, short, short);

extern boolean ptinuseritem (Point, DialogPtr, short);

extern boolean setuseritemdrawroutine (DialogPtr, short, callback);

extern pascal boolean modaldialogcallback (DialogPtr, EventRecord *, short *);

extern void dialogupdate (EventRecord *, DialogPtr);

extern boolean dialogevent (EventRecord *, DialogPtr, short *);

extern boolean dialogidle (DialogPtr);

extern boolean dialogactivate (DialogPtr, boolean);

extern boolean dialogsetselect (DialogPtr, short, short);

extern boolean dialogselectall (DialogPtr);

extern boolean dialoggetselect (DialogPtr, short *, short *);

extern short savedialog (bigstring);

extern short replacevariabledialog (bigstring);

extern boolean revertdialog (bigstring);

extern boolean askdialog (bigstring, bigstring);

extern boolean askpassword (bigstring, bigstring);

extern boolean twowaydialog (bigstring, bigstring, bigstring);

extern short threewaydialog (bigstring, bigstring, bigstring, bigstring);

extern boolean initdialogs (void);

extern boolean intdialog (bigstring, short *);

extern boolean chardialog (bigstring, short *);

extern boolean msgdialog (bigstring);

extern boolean alertdialog (bigstring);

extern boolean alertstring (short);

extern short customalert (short, bigstring);

extern boolean customdialog (short, short, dialogcallback);

#ifdef WIN95VERSION
	extern boolean userinfodialog (bigstring username, bigstring userinitials, bigstring userorg, bigstring email);

	extern void centerdialog (HWND hwndDlg);
#else
	char X0_p2cstrcpy(char *dst, StringPtr src);
#endif

#endif



