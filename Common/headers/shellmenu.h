
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

#ifndef shellmenuinclude
#define shellmenuinclude /*so other includes can tell if we've been loaded*/


#ifdef MACVERSION

	/*
	2006-02-25 aradke: must keep in sync with SHELL.R
	*/

	#define mainmenuincrement		1
	#define hiermenuincrement		1

	#define applemenu				1
	#define filemenu				2
	#define editmenu				3
	#define windowsmenu				4

	#define firstmainmenu			applemenu
	#define lastmainmenu			windowsmenu

	#define fontmenu				128
	#define stylemenu				129
	#define sizemenu				130
	#define leadingmenu				131
	#define justifymenu				132
	#define findandreplacemenu		133
	#define commonstylesmenu		134
	#define newobjectmenu			135
	#define virtualmenu				136 /* never appears in user interface -- used for extended keyboard */
	#define openrecentmenu			137

	#define firsthiermenu			fontmenu
	#define lasthiermenu			openrecentmenu
	
#endif //MACVERSION


#ifdef WIN95VERSION	

	/*
	2006-02-25 aradke: must keep in sync with WinLand.rc
	
	2006-02-25 aradke: undefine menus for Pike and OPML Editor that aren't actually used by the app
		in order to avoid picking up their keyboard shortcuts at runtime
	*/

	#define mainmenuincrement	1000
	#define hiermenuincrement 	100

	#define applemenu			1000
	#define filemenu			2000
	#define editmenu			3000
	#define windowsmenu			4000
	#define helpmenu			5000

	#define firstmainmenu		filemenu
	#define lastmainmenu		helpmenu

	#ifdef PIKE

		#undef stylemenu
		#undef leadingmenu
		#undef justifymenu
		#undef newobjectmenu

		#define virtualmenu			5100	// 2006-02-25 aradke: never actually displayed
		#define fontmenu			5200
		#define sizemenu			5300
		#define findandreplacemenu	5400
		#define commonstylesmenu	5500	// 7.0b26 PBS
		
		#ifdef IUDADESKTOP
		
			#define openrecentmenu	5600	// 2007-10-22 creedon
		
		#else
		
			#undef openrecentmenu

		#endif
		
		#define firsthiermenu		fontmenu

		#ifdef IUDADESKTOP
		
			#define lasthiermenu	openrecentmenu	// 2007-10-22 creedon
		
		#else
		
			#define lasthiermenu	commonstylesmenu

		#endif
		
	#else //!PIKE

		#define virtualmenu			5100	/* 2006-02-25 aradke: never actually displayed */
		#define fontmenu			5200
		#define stylemenu			5300
		#define sizemenu			5400
		#define leadingmenu			5500
		#define justifymenu			5600
		#define findandreplacemenu	5700
		#define commonstylesmenu	5800	/*7.0b26 PBS*/
		#define newobjectmenu		5900
		#define openrecentmenu		6000	/* 2006-02-05 aradke */

		#define firsthiermenu		fontmenu
		#define lasthiermenu		openrecentmenu

	#endif //!PIKE

#endif //WIN95VERSION


#define aboutitem 1

#ifdef PIKE

	#if defined ( IUDADESKTOP ) || defined ( OPMLEDITOR )
	
		#ifdef IUDADESKTOP
		
			#define newitem		   1
			#define openitem		   2
			#define openrecentitem	   3
			#define closeitem		   4
			#define saveitem		   6
			#define saveasitem		   7
			// #define updateradiorootitem 9
			#define quititem		   9
			
		#endif // IUDADESKTOP
		
		#ifdef OPMLEDITOR // OPMLEDITOR 2005-04-06 dluebbert
		
			#define newitem		    1
			#define openitem		    2
			#define openurlitem		    3 // 7.0b17 PBS: Open URL... command
		//	#define openmanilasiteitem 4
			#define closeitem		    4
			#define saveitem		    6
			#define saveasitem		    7
		//	#define saveashtmlitem 9 /*7.0b32 PBS: Save As HTML... command*/
		//	#define saveasplaintextitem 10 /*7.0b32 PBS: Save As Plain Text... command*/
			#define revertitem		    8
			#define changetitleitem	    5
			#define viewinbrowseritem   10
			#define updateradiorootitem 12
			#define workofflineitem	   13 // 7.0b25 PBS: Work Offline
			#define quititem		   15
					
		#endif // OPMLEDITOR
		
	#else // Radio UserLand
	
		//
		// 7.0d10 PBS: Radio UserLand has an Update Radio.root... command in the
		//			File menu.
		//
		// PBS 07/22/00: Pike's File menu has been revised -- the number, order,
		//			  and names of items have changed.
		//
		
		// #define filemenu (applemenu + mainmenuincrement)

		#define newitem		    1
		#define openitem		    2
		#define openurlitem		    3 // 7.0b17 PBS: Open URL... command
		#define openmanilasiteitem   4
		#define closeitem		    5
		#define saveitem		    7
		#define saveasitem		    8
		#define saveashtmlitem	    9 // 7.0b32 PBS: Save As HTML... command
		#define saveasplaintextitem 10 // 7.0b32 PBS: Save As Plain Text... command
		#define revertitem		   11
		#define changetitleitem	    8
		#define viewinbrowseritem   13
		#define updateradiorootitem 15
		#define workofflineitem     16 // 7.0b25 PBS: Work Offline
		#define quititem		   18

	#endif // Radio UserLand
	
#else // Frontier

	// #define filemenu (applemenu + mainmenuincrement)
	
	#define newitem		1
	#define openitem		2
	#define openrecentitem	3
	#define closeitem		5
	#define saveitem		6
	#define saverunnableitem 7
	#define saveasitem		8
	#define revertitem		9

	//#define expertitem 10

	#define pagesetupitem	11
	#define printitem		12
	#define quititem		14
	
#endif // Frontier

// #define editmenu (filemenu + mainmenuincrement)

#define windowsitem				-1

#ifdef PIKE

	#if defined ( IUDADESKTOP ) || defined ( OPMLEDITOR )
	
		#ifdef IUDADESKTOP
		
			#define undoitem		  1
			#define cutitem		  3
			#define copyitem		  4
			#define pasteitem		  5
			#define clearitem		  6
			#define selectallitem	  8
			#define findandreplaceitem 10
			#define fontitem		  12
			#define sizeitem		  13
			#define commonstylesitem	  14
			#define insertdatetimeitem 16
			
		#endif // IUDADESKTOP
		
		#ifdef OPMLEDITOR
		
			#define undoitem		  1
			#define cutitem		  3
			#define copyitem		  4
			#define pasteitem		  5
			#define clearitem		  6
			#define selectallitem	  8
			#define findandreplaceitem 10
			#define commonstylesitem	  11
			#define fontitem		  13
			#define sizeitem		  14
			#define insertdatetimeitem 16
			
			#define styleitem		  -1
			#define leadingitem		  -1
			#define justifyitem		  -1
					
		#endif // OPMLEDITOR
		
	#else // Radio UserLand
	
		#define undoitem		  1
		#define cutitem		  3
		#define copyitem		  4
		#define pasteitem		  5
		#define clearitem		  6
		#define selectallitem	  8
		#define findandreplaceitem 10
		#define commonstylesitem	  11
		#define fontitem		  13
		#define sizeitem		  14
		#define opennotepaditem	  16
		#define insertdatetimeitem 17
		
		#define styleitem		  -1
		#define leadingitem		  -1
		#define justifyitem		  -1
		
	#endif // Radio UserLand
	
#else // Frontier

	#define undoitem		  1
	#define cutitem		  3
	#define copyitem		  4
	#define pasteitem		  5
	#define clearitem		  6
	#define selectallitem	  8
	#define findandreplaceitem 10
	#define fontitem		  12
	#define sizeitem		  13
	#define styleitem		  14
	#define leadingitem		  15
	#define justifyitem		  16
	#define commonstylesitem	  17
	#define insertdatetimeitem 19
	
#endif // Frontier

// #define windowsmenu (editmenu + mainmenuincrement)
#define hidewindowitem 1
#if TARGET_API_MAC_CARBON == 1
	#define minimizewindowitem 2
	#define bringalltofrontwindowitem 4
#endif

// #define helpmenu (windowsmenu + mainmenuincrement)
#define aboutitem 1

// #define stylemenu (fontmenu + hiermenuincrement)
#define plainitem 1
#define bolditem 3
#define italicitem 4
#define underlineitem 5
#define outlineitem 6
#define shadowitem 7
/*
#define condenseditem 8
*/
#define superscriptitem 8
#define subscriptitem 9

// #define sizemenu (stylemenu + hiermenuincrement)
#define point9item 1
#define point10item 2
#define point12item 3
#define point14item 4
#define point18item 5
#define point24item 6
#define pointupitem 8
#define pointdownitem 9
#define pointcustomitem 11

// #define leadingmenu (sizemenu + hiermenuincrement)
#define leading0item 1
#define leading1item 3
#define leading2item 4
#define leading3item 5
#define leading4item 6
#define leading5item 7
#define leadingcustomitem 9

// #define justifymenu (leadingmenu + hiermenuincrement)
#define leftjustifyitem 1
#define rightjustifyitem 2
#define centerjustifyitem 3
#define fulljustifyitem 4

// #define virtualmenu (justifymenu + hiermenuincrement) /*never appears in user interface -- used for externed keyboard*/
#define helpitem 1
#define homeitem 2
#define enditem 3
#define pageupitem 4
#define pagedownitem 5
#define moveupitem 6
#define movedownitem 7
#define moveleftitem 8
#define moverightitem 9

// #define openrecentmenu (virtualmenu + hiermenuincrement)
#define clearrecentmenuitem 2


#if !defined(REZ) && !defined(RC_INVOKED)

#include "shelltypes.h"

//#define ctmenustack (lastmainmenu - firstmainmenu + lasthiermenu - firsthiermenu + 2)
#define ctmenustack ( (lastmainmenu - firstmainmenu) / mainmenuincrement \
					+ (lasthiermenu - firsthiermenu) / hiermenuincrement \
					+ 2)

typedef struct tymenuinfo {
	
	short idmenu; /*the resource id / menu id of this menu*/
	hdlmenu macmenu; /*a handle to the menu manager's data structure for the menu*/
	} tymenuinfo;

extern tymenuinfo menustack [ctmenustack];

extern short topmenustack;


typedef enum tyeditcommand {
	
	undocommand, cutcommand = 2, copycommand, pastecommand, clearcommand, selectallcommand = 7
	
	} tyeditcommand;
	

/*prototypes*/

extern hdlmenu shellmenuhandle (short); /*shellmenu.c*/

extern boolean shelltgetmainmenu (bigstring, hdlmenu *, short *);

extern boolean shellinitmenus (void);

extern void shellgetlastmenuid (short *);

extern boolean shellapplemenu (bigstring);

extern boolean shelleditcommand (tyeditcommand);

extern void shellforcemenuadjust (void);

extern void shellmodaldialogmenuadjust (void);

extern void shellupdatemenus (void);

extern boolean shellhandlemenu (long);

extern void runfilemenuscript (short); /* 2005-09-15 creedon - all targets can now run a script associated with some of the file menu commands */

extern void runeditmenuscript (short, short); /* 2005-09-25 creedon - all targets can now run a script associated with some of the edit menu commands */

extern void getcommonstylesmenuitemtext (short, bigstring); /* 2005-09-25 creedon - all targets can now run a script associated with some of the edit menu commands */ 

extern void getfilemenuitemidentifier (short, bigstring); /* 2005-09-15 creedon - all targets can now run a script associated with some of the file menu commands */

extern void geteditmenuitemidentifier (short, short, bigstring); /* 2005-09-25 creedon - all targets can now run a script associated with some of the edit menu commands */

extern void runopenrecentmenuscript (short); /* 2005-09-24 creedon */

extern boolean shellupdatewindowmenu (void); /*shellwindowmenu.c*/

extern void shellwindowmenuselect (short);

extern void shelladjustundo (void); /*7.0fc1 PBS: no longer static*/

extern boolean pikequit (void); /*7.0 PBS: called when the X in the frame window is clicked in Windows.*/

extern void shellupdateopenrecentmenu (void); /* 2005-09-25 creedon */

#endif //!defined(REZ) && !defined(RC_INVOKED)

#endif //shellmenuinclude


