
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

#define oldTemp


#include "frontier.r"

resource 'MENU' (1, "Apple") {
	1,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About ^0É", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

#ifdef PIKE

/*PBS 7.0b1: File menu changes for Radio UserLand.
It's a standard File menu now.*/
#ifndef OPMLEDITOR
resource 'MENU' (2, "File") {
	2,
	textMenuProc,
	0x7FFFFE77,
	enabled,
	"File",
	{	/* array: 9 elements */
		/* [1] */
		"New", noIcon, "N", noMark, plain,
		/* [2] */
		"OpenÉ", noIcon, "O", noMark, plain,
		/* [3] */
		"Open URLÉ", noIcon, noKey, noMark, plain, /*7.0b17 PBS*/
		/* [4] */
		"Open Manila SiteÉ", noIcon, noKey, noMark, plain, /*7.0b27 PBS*/
		/* [5] */
		"Close", noIcon, "W", noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"Save", noIcon, "S", noMark, plain,
		/* [8] */
		"Save AsÉ", noIcon, noKey, noMark, plain,
		/* [9] */
		"Save As HTMLÉ", noIcon, noKey, noMark, plain,
		/* [10] */
		"Save As Plain TextÉ", noIcon, noKey, noMark, plain,
		/* [11] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"View in Browser", noIcon, "P", noMark, plain,
		/* [14] */
		"-", noIcon, noKey, noMark, plain,
		/* [15] */
		"Update Radio.rootÉ", noIcon, noKey, noMark, plain,
		/* [16] */
		"Work Offline", noIcon, noKey, noMark, plain,
		/* [17] */
		"-", noIcon, noKey, noMark, plain,
		/* [18] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

/*Carbon File menu*/

resource 'MENU' (412, "File") {
	2,
	textMenuProc,
	0x7FFFFE77,
	enabled,
	"File",
	{	/* array: 9 elements */
		/* [1] */
		"New", noIcon, "N", noMark, plain,
		/* [2] */
		"OpenÉ", noIcon, "O", noMark, plain,
		/* [3] */
		"Open URLÉ", noIcon, noKey, noMark, plain, /*7.0b17 PBS*/
		/* [4] */
		"Open Manila SiteÉ", noIcon, noKey, noMark, plain, /*7.0b27 PBS*/
		/* [5] */
		"Close", noIcon, "W", noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"Save", noIcon, "S", noMark, plain,
		/* [8] */
		"Save AsÉ", noIcon, noKey, noMark, plain,
		/* [9] */
		"Save As HTMLÉ", noIcon, noKey, noMark, plain,
		/* [10] */
		"Save As Plain TextÉ", noIcon, noKey, noMark, plain,
		/* [11] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"View in Browser", noIcon, "P", noMark, plain,
		/* [14] */
		"-", noIcon, noKey, noMark, plain,
		/* [15] */
		"Update Radio.rootÉ", noIcon, noKey, noMark, plain,
		/* [16] */
		"Work Offline", noIcon, noKey, noMark, plain
	}
};
#else // OPMLEDITOR

resource 'MENU' (2, "File") {
	2,
	textMenuProc,
	0x7FFFFE77,
	enabled,
	"File",
	{	/* array: 9 elements */
		/* [1] */
		"New", noIcon, "N", noMark, plain,
		/* [2] */
		"OpenÉ", noIcon, "O", noMark, plain,
		/* [3] */
		"Open URLÉ", noIcon, noKey, noMark, plain, /*7.0b17 PBS*/
		
		/* [4] */
		"Close", noIcon, "W", noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"Save", noIcon, "S", noMark, plain,
		/* [7] */
		"Save AsÉ", noIcon, noKey, noMark, plain,
		/* [9] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"View in Browser", noIcon, "P", noMark, plain,
		/* [11] */
		"-", noIcon, noKey, noMark, plain,
		/* [12] */
		"Update opml.rootÉ", noIcon, noKey, noMark, plain,
		/* [13] */
		"Work Offline", noIcon, noKey, noMark, plain,
		/* [14] */
		"-", noIcon, noKey, noMark, plain,
		/* [15] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

/*Carbon File menu*/

resource 'MENU' (412, "File") {
	2,
	textMenuProc,
	0x7FFFFE77,
	enabled,
	"File",
	{	/* array: 9 elements */
		/* [1] */
		"New", noIcon, "N", noMark, plain,
		/* [2] */
		"OpenÉ", noIcon, "O", noMark, plain,
		/* [3] */
		"Open URLÉ", noIcon, noKey, noMark, plain, /*7.0b17 PBS*/
		/* [4] */
		"Close", noIcon, "W", noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"Save", noIcon, "S", noMark, plain,
		/* [7] */
		"Save AsÉ", noIcon, noKey, noMark, plain,
		/* [8] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"View in Browser", noIcon, "P", noMark, plain,
		/* [11] */
		"-", noIcon, noKey, noMark, plain,
		/* [12] */
		"Update opml.rootÉ", noIcon, noKey, noMark, plain,
		/* [13] */
		"Work Offline", noIcon, noKey, noMark, plain
	}
};
#endif // OPMLEDITOR
#else

resource 'MENU' (2, "File") {
	2,
	textMenuProc,
	0x7FFFFE77,
	enabled,
	"File",
	{	/* array: 14 elements */
#ifdef version42orgreater
		/* [1] */
		"New", noIcon, hierarchicalMenu, "\0d135", plain,
#else
		/* [1] */
		"New", noIcon, noKey, noMark, plain,
#endif
		/* [2] */
		"OpenÉ", noIcon, "O", noMark, plain,
		/* [3] */
		"Open Recent", noIcon, hierarchicalMenu, "\0d137", plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Close", noIcon, "W", noMark, plain,
		/* [6] */
		"Save", noIcon, "S", noMark, plain,
		/* [7] */
		"Save as RunnableÉ", noIcon, noKey, noMark, plain,
		/* [8] */
		"Save AsÉ", noIcon, noKey, noMark, plain, /* 2005-11-01 creedon */
		/* [9] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [10] */
		"-", noIcon, noKey, noMark, plain,

//		/* [10] */
//		"Expert Mode", noIcon, noKey, noMark, plain,
//		/* [9] */
//		"-", noIcon, noKey, noMark, plain,

		/* [11] */
		"Page SetupÉ", noIcon, noKey, noMark, plain,
		/* [12] */
		"PrintÉ", noIcon, "P", noMark, plain,
		/* [13] */
		"-", noIcon, noKey, noMark, plain,
		/* [14] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

//Code change by Timothy Paustian Saturday, July 22, 2000 10:22:52 PM
//This is the conditional menu for carbon. You do not need a quit 
//item in carbon
resource 'MENU' (412, "File") {
	2,
	textMenuProc,
	0x7FFFFE77,
	enabled,
	"File",
	{	/* array: 12 elements */
#ifdef version42orgreater
		/* [1] */
		"New", noIcon, hierarchicalMenu, "\0d135", plain,
#else
		/* [1] */
		"New", noIcon, noKey, noMark, plain,
#endif
		/* [2] */
		"OpenÉ", noIcon, "O", noMark, plain,
		/* [3] */
		"Open Recent", noIcon, hierarchicalMenu, "\0d137", plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Close", noIcon, "W", noMark, plain,
		/* [6] */
		"Save", noIcon, "S", noMark, plain,
		/* [7] */
		"Save as RunnableÉ", noIcon, noKey, noMark, plain,
		/* [8] */
		"Save AsÉ", noIcon, noKey, noMark, plain, /* 2005-11-01 creedon */
		/* [9] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [10] */
		"-", noIcon, noKey, noMark, plain,
		/* [11] */
		"Page SetupÉ", noIcon, noKey, noMark, plain,
		/* [12] */
		"PrintÉ", noIcon, "P", noMark, plain
	}
};
#endif

#ifdef PIKE /*7.0b26 PBS: Radio UserLand has a different Edit menu.*/

#ifndef OPMLEDITOR
resource 'MENU' (3, "Edit") {
	3,
	textMenuProc,
	0x7FFFFF5D,
	enabled,
	"Edit",
	{	/* array: 17 elements */
		/* [1] */
		"Undo", noIcon, "Z", noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Cut", noIcon, "X", noMark, plain,
		/* [4] */
		"Copy", noIcon, "C", noMark, plain,
		/* [5] */
		"Paste", noIcon, "V", noMark, plain,
		/* [6] */
		"Clear", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Select All", noIcon, "A", noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Find and Replace", noIcon, hierarchicalMenu, "\0d133", plain,
		/* [11] */
		"Common Styles", noIcon, hierarchicalMenu, "\0d134", plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Font", noIcon, hierarchicalMenu, "\0d128", plain,
		/* [14] */
		"Size", noIcon, hierarchicalMenu, "\0d130", plain,
		/* [15] */
		"-", noIcon, noKey, noMark, plain,
		/* [16] */
		"Open Notepad", noIcon, "Y", noMark, plain,
		/* [17] */
		"Insert Date/Time", noIcon, "4", noMark, plain,
	}
};
#else //OPMLEDITOR
resource 'MENU' (3, "Edit") {
	3,
	textMenuProc,
	0x7FFFFF5D,
	enabled,
	"Edit",
	{	/* array: 14 elements */
		/* [1] */
		"Undo", noIcon, "Z", noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Cut", noIcon, "X", noMark, plain,
		/* [4] */
		"Copy", noIcon, "C", noMark, plain,
		/* [5] */
		"Paste", noIcon, "V", noMark, plain,
		/* [6] */
		"Clear", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Select All", noIcon, "A", noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Find and Replace", noIcon, hierarchicalMenu, "\0d133", plain,
		/* [11] */
		"Common Styles", noIcon, hierarchicalMenu, "\0d134", plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Font", noIcon, hierarchicalMenu, "\0d128", plain,
		/* [14] */
		"Size", noIcon, hierarchicalMenu, "\0d130", plain,
		/* [15] */
		"-", noIcon, noKey, noMark, plain,
		/* [16] */
		"Insert Date/Time", noIcon, "4", noMark, plain,
		
	}
};

#endif //OPMLEDITOR

#else

resource 'MENU' (3, "Edit") {
	3,
	textMenuProc,
	0x7FFFFF5D,
	enabled,
	"Edit",
	{	/* array: 19 elements */
		/* [1] */
		"Undo", noIcon, "Z", noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Cut", noIcon, "X", noMark, plain,
		/* [4] */
		"Copy", noIcon, "C", noMark, plain,
		/* [5] */
		"Paste", noIcon, "V", noMark, plain,
		/* [6] */
		"Clear", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Select All", noIcon, "A", noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Find and Replace", noIcon, hierarchicalMenu, "\0d133", plain,
		/* [11] */
		"-", noIcon, noKey, noMark, plain,
		/* [12] */
		"Font", noIcon, hierarchicalMenu, "\0d128", plain,
		/* [13] */
		"Size", noIcon, hierarchicalMenu, "\0d130", plain,
		/* [14] */
		"Style", noIcon, hierarchicalMenu, "\0d129", plain,
		/* [15] */
		"Leading", noIcon, hierarchicalMenu, "\0d131", plain,
		/* [16] */
		"Justify", noIcon, hierarchicalMenu, "\0d132", plain,
		/* [17] */
		"Common Styles", noIcon, hierarchicalMenu, "\0d134", plain,
		/* [18] */
		"-", noIcon, noKey, noMark, plain,
		/* [19] */
		"Insert Date/Time", noIcon, "4", noMark, plain
	}
};

#endif

resource 'MENU' (4, "Window") {
	4,
	textMenuProc,
	allEnabled,
	enabled,
	"Window",
	{	/* array: 2 elements */
		/* [1] */
		"Hide Window", noIcon, noKey, noMark, plain,		
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

/*Carbon Window menu with extra OS X commands.*/

resource 'MENU' (413, "Window") {
	4,
	textMenuProc,
	allEnabled,
	enabled,
	"Window",
	{	/* array: 5 elements */
		/* [1] */
		"Hide Window", noIcon, noKey, noMark, plain,
		/* [2] */
		"Minimize", noIcon, "M", noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Bring All to Front", noIcon, noKey, noMark, plain,		
		/* [5] */
		"-", noIcon, noKey, noMark, plain
	}
};


resource 'MENU' (128, "Font") {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"Font",
	{	/* array: 0 elements */
	}
};

resource 'MENU' (129, "Style") {
	129,
	textMenuProc,
	0x7FFFFBFF,
	enabled,
	"Style",
	{	/* array: 9 elements */
		/* [1] */
		"Plain Text", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Bold", noIcon, "B", noMark, 1,
		/* [4] */
		"Italic", noIcon, "I", noMark, 2,
		/* [5] */
		"Underline", noIcon, "U", noMark, 4,
		/* [6] */
		"Outline", noIcon, "O", noMark, 8,
		/* [7] */
		"Shadow", noIcon, "S", noMark, 16,
		/* [8] */
		"Superscript", noIcon, noKey, noMark, plain,
		/* [9] */
		"Subscript", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (130, "Size") {
	130,
	textMenuProc,
	0x7FFFFDBF,
	enabled,
	"Size",
	{	/* array: 11 elements */
		/* [1] */
		"9 Point", noIcon, noKey, noMark, plain,
		/* [2] */
		"10 Point", noIcon, noKey, noMark, plain,
		/* [3] */
		"12 Point", noIcon, noKey, noMark, plain,
		/* [4] */
		"14 Point", noIcon, noKey, noMark, plain,
		/* [5] */
		"18 Point", noIcon, noKey, noMark, plain,
		/* [6] */
		"24 Point", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Larger", noIcon, ">", noMark, plain,
		/* [9] */
		"Smaller", noIcon, "<", noMark, plain,
		/* [10] */
		"-", noIcon, noKey, noMark, plain,
		/* [11] */
		"CustomÉ", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (131, "Leading") {
	131,
	textMenuProc,
	0x7FFFFF7D,
	enabled,
	"Leading",
	{	/* array: 9 elements */
		/* [1] */
		"No Leading", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"1 Pixel", noIcon, noKey, noMark, plain,
		/* [4] */
		"2 Pixels", noIcon, noKey, noMark, plain,
		/* [5] */
		"3 Pixels", noIcon, noKey, noMark, plain,
		/* [6] */
		"4 Pixels", noIcon, noKey, noMark, plain,
		/* [7] */
		"5 Pixels", noIcon, noKey, noMark, plain,
		/* [8] */
		"-", noIcon, noKey, noMark, plain,
		/* [9] */
		"CustomÉ", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (132, "Justify") {
	132,
	textMenuProc,
	allEnabled,
	enabled,
	"Justify",
	{	/* array: 4 elements */
		/* [1] */
		"Left Justify", noIcon, noKey, noMark, plain,
		/* [2] */
		"Right Justify", noIcon, noKey, noMark, plain,
		/* [3] */
		"Center Justify", noIcon, noKey, noMark, plain,
		/* [4] */
		"Full Justify", noIcon, noKey, noMark, plain
	}
};


resource 'MENU' (133, "Find and Replace") {
	133,
	textMenuProc,
	allEnabled,
	enabled,
	"Find and Replace",
	{	/* array: 4 elements */
		/* [1] */
		"FindÉ", noIcon, "F", noMark, plain,
		/* [2] */
		"Replace", noIcon, noKey, noMark, plain,
		/* [3] */
		"Find Next", noIcon, "G", noMark, plain,
		/* [4] */
		#ifdef PIKE /* 2005-09-25 creedon */
		
		"Replace & Find Next", noIcon, "H", noMark, plain
		
		#else
		
		"Replace & Find Next", noIcon, "1", noMark, plain
		
		#endif
	}
};

resource 'MENU' (134, "Common Styles") {
	134,
	textMenuProc,
	allEnabled,
	enabled,
	"Common Styles",
	{	/* array: 4 elements */
		/* [1] */
		"Geneva 9", noIcon, "9", noMark, plain,
		/* [2] */
		"Geneva 12", noIcon, "2", noMark, plain,
		/* [3] */
		"Palatino 12", noIcon, "7", noMark, plain,
		/* [4] */
		"Helvetica 18", noIcon, "8", noMark, plain
	}
};

resource 'MENU' (135, "New") {
	135,
	textMenuProc,
	allEnabled,
	enabled,
	"New",
	{	/* array: 5 elements */
		"Script", noIcon, "N", noMark, plain,
		"WP-Text", noIcon, noKey, noMark, plain,
		//"Picture", noIcon, noKey, noMark, plain,
		"Outline", noIcon, noKey, noMark, plain,
		"Menubar", noIcon, noKey, noMark, plain,
		"Table", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Database", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (136) {
	136,
	textMenuProc,
	allEnabled,
	enabled,
	"Extended",
	{	/* array: 5 elements */
		/* [1] */
		"Help", noIcon, noKey, noMark, plain,
		/* [2] */
		"Home", noIcon, noKey, noMark, plain,
		/* [3] */
		"End", noIcon, noKey, noMark, plain,
		/* [4] */
		"Page Up", noIcon, noKey, noMark, plain,
		/* [5] */
		"Page Down", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (137, "Open Recent") {
	137,
	textMenuProc,
	allEnabled,
	enabled,
	"Open Recent",
	{	/* array: 2 elements */
		/* [1] */
		"-", noIcon, noKey, noMark, plain,
		/* [2] */
		"Clear Menu", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (257, "geneva9 popup") {
	257,
	256,
	allEnabled,
	enabled,
	"geneva9",
	{	/* array: 0 elements */
	}
};

resource 'MENU' (258, "normal popup") {
	258,
	textMenuProc,
	allEnabled,
	enabled,
	"popup",
	{	/* array: 0 elements */
	}
};

resource 'xmnu' (129, "Style") {
	versionZero
	{
		{
			/* array ItemExtensions: 9 elements */
			/* [1] */
			skipItem {},
			/* [2] */
			skipItem {},
			/* [3] */
			dataItem {0, kMenuShiftModifier, currScript, 0, 0, noHierID, sysFont, naturalGlyph},
			/* [4] */
			dataItem {0, kMenuShiftModifier, currScript, 0, 0, noHierID, sysFont, naturalGlyph},
			/* [5] */
			dataItem {0, kMenuShiftModifier, currScript, 0, 0, noHierID, sysFont, naturalGlyph},
			/* [6] */
			dataItem {0, kMenuShiftModifier, currScript, 0, 0, noHierID, sysFont, naturalGlyph},
			/* [7] */
			dataItem {0, kMenuShiftModifier, currScript, 0, 0, noHierID, sysFont, naturalGlyph},
			/* [8] */
			skipItem {},
			/* [9] */
			skipItem {}
			}
		}
	};


resource 'STR#' (129, "Defaults", purgeable) {
	{	/* array StringArray: 6 elements */
#ifdef PIKE
#ifndef OPMLEDITOR
		/* [1] */
		"Radio UserLandª", /*PBS 7.0b1: name change to Radio UserLand.*/
#else  // OPMLEDITOR
		"OPML",
#endif  //OPMLEDITOR
#else
		/* [1] */
		"Frontier", /* 2005-01-04 creedon - removed UserLand and (tm) for open source release */
#endif
		/* [2] */
		"Untitled",
		/* [3] */
		"Untitled",
		/* [4] */
		"Select a database file:",
		/* [5] */
		"",
#ifdef PIKE
#ifndef OPMLEDITOR
		/* [6] -- default DB name*/ /*PBS 7.0b1: changed from Pike.root to Radio.root.*/
		"Radio.root",
#else  // OPMLEDITOR
		"opml.root",
#endif  //OPMLEDITOR
#else
		/* [6] */
		"Frontier.root",
#endif
		/* [7] */
		"Open a file:"
	}
};

resource 'STR#' (130, "Interface", locked, preload) {
	{	/* array StringArray: 15 elements */
		/* [1] */
		"Script",
		/* [2] */
		"Zoom",
		/* [3] */
		"CanÕt access printing resources.  Make sure that a printer has been selected with the Chooser.",
		/* [4] */
		"Cmd",
		/* [5] */
		"Set Command KeyÉ",
		/* [6] */
		"Command Key:",
		/* [7] */
		"Internal error#^0.  Please report to <http://groups.yahoo.com/group/frontierkernel/>",
		/* [8] */
		"; ",
		/* [9] */
		"Cancel",
		/* [10] */
		"Script for Ò^0Ó",
		/* [11] */
		"Font Size:",
		/* [12] */
		"K",
		/* [13] */
		"Leading:",
		/* [14] */
		",",
		/* [15] */
		"Out of memory",
		/* [16] */
		"Save",
		/* [17] */
		"Save Database",
		/* [18] */
		"Save AsÉ",
		/* [19] */
		"Save a CopyÉ",
		/* [20] */
		"This database was created by an older version of Frontier and must be converted. Proceed?"
	}
};


#ifdef fltrialsize

resource 'STR#' (400, "Trial", locked, preload) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"This trial version of Frontier limits databases to 7 megabytes in size",
		/* [2] */
		"This is a trial version of Frontier and no more than three threads can be created",
		/* [3] */
		"The trial version of Frontier expired on ^0.  For more information please visit http://www.userland.com/.",
		/* [4] */
		". To purchase a license for the full version of Frontier, visit http://www.userland.com/.",
		/* [5] */
		"This trial version of Frontier does not support the use of guest databases"
	}
};

#endif


resource 'STR#' (131, "Error Info", locked, preload) {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Go To",
		/* [2] */
		"Error Info",
		/* [3] */
		""
	}
};

resource 'STR#' (132, "Font Names", purgeable) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"Palatino",
		/* [2] */
		"Geneva",
		/* [3] */
		"Helvetica",
		/* [4] */
		"Chicago"
	}
};

resource 'STR#' (133, "QuickScript") {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Run",
		/* [2] */
		"Quick Script"
	}
};

resource 'STR#' (134, "Undo") {
	{	/* array StringArray: 14 elements */
		/* [1] */
		"CanÕt Undo",
		/* [2] */
		"Undo ",
		/* [3] */
		"Redo ",
		/* [4] */
		"Cut",
		/* [5] */
		"Copy",
		/* [6] */
		"Paste",
		/* [7] */
		"Clear",
		/* [8] */
		"Typing",
		/* [9] */
		"Move",
		/* [10] */
		"Sort",
		/* [11] */
		"Promote",
		/* [12] */
		"Demote",
		/* [13] */
		"Deletion",
		/* [14] */
		"Formatting"
	}
};

resource 'STR#' (140, "DB Stats", purgeable) {
	{	/* array StringArray: 22 elements */
		/* [1] */
		"Database Stats",
		/* [2] */
		"nodes in the release list",
		/* [3] */
		"bytes released",
		/* [4] */
		"nodes in the avail list",
		/* [5] */
		"bytes free",
		/* [6] */
		"largest free node",
		/* [7] */
		"header/trailer inconsistencies",
		/* [8] */
		"sequentially, nodes free",
		/* [9] */
		"bytes free",
		/* [10] */
		"nodes in use",
		/* [11] */
		"bytes in use",
		/* [12] */
		"logically, nodes in use",
		/* [13] */
		"bytes in use",
		/* [14] */
		"orphaned bytes",
		/* [15] */
		"error reading header of released node",
		/* [16] */
		"error reading header of used node",
		/* [17] */
		"a free block is in use",
		/* [18] */
		"error reading free node",
		/* [19] */
		"error getting end of file",
		/* [20] */
		"error reading sequential header",
		/* [21] */
		"error reading sequential trailer",
		/* [22] */
		"header/trailer info disagrees"
		/* [23] */
		"free list shadow out of synch"
	}
};

resource 'STR#' (156, "sort popup", locked, preload) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"Sort By Name",
		/* [2] */
		"Sort By Value",
		/* [3] */
		"Sort By Kind",
		/* [4] */
		"Sort:"
	}
};

resource 'STR#' (157, "kind popup", locked, preload) {
	{	/* array StringArray: 22 elements */
		/* [1] */
		"Boolean",
		/* [2] */
		"Character",
		/* [3] */
		"Number",
		/* [4] */
		"Float",
		/* [5] */
		"Date",
		/* [6] */
		"Direction",
		/* [7] */
		"String",
		/* [8] */
		"-",
		/* [9] */
		"String4",
		/* [10] */
		"Enumerator",
		/* [11] */
		"File Specifier",
		/* [12] */
		"Alias",
		/* [13] */
		"Object Specifier",
		/* [14] */
		"Address",
		/* [14] */
		"-",
		/* [15] */
		"Table",
		/* [16] */
		"WP-Text",
		/* [17] */
		"Picture",
		/* [18] */
		"Outline",
		/* [19] */
		"Script",
		/* [20] */
		"MenuBar",
		/* [21] */
		"-",
		/* [22] */
		"List",
		/* [23] */
		"Record",
		/* [24] */
		"Binary",
		/* [25] */
		"Kind:"
	}
};

resource 'STR#' (159, "OP UI", locked, preload) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"outline",
		/* [2] */
		"script",
		/* [3] */
		"1 line",
		/* [4] */
		"^0 lines"
	}
};

resource 'STR#' (161, "interface UI", locked, preload) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"interface",
		/* [2] */
		"ÇinterfaceÈ",
		/* [3] */
		"1 byte",
		/* [4] */
		"^0 bytes",
		/* [5] */
		"error"
	}
};

resource 'STR#' (164, "WP UI", locked, preload) {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"wp text",
		/* [2] */
		"1 character",
		/* [3] */
		"^0 characters"
	}
};

resource 'STR#' (165, "Table UI", locked, preload) {
	{	/* array StringArray: 12 elements */
		/* [1] */
		"system table",
		/* [2] */
		"table",
		/* [3] */
		"1 item",
		/* [4] */
		"^0 items",
		/* [5] */
		" copy",
		/* [6] */
		"pasted text",
		/* [7] */
		"pasted ",
		/* [8] */
		"Paste as:",
		/* [9] */
		"???",
		/* [10] */
		"Name",
		/* [11] */
		"Value",
		/* [12] */
		"Kind",
		/* [13] */
		"xml"
	}
};

resource 'STR#' (166, "Menu UI", locked, preload) {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"menu bar",
		/* [2] */
		"1 item",
		/* [3] */
		"^0 items"
	}
};

resource 'STR#' (167, "Pict UI", locked, preload) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"picture",
		/* [2] */
		"ÇpictureÈ",
		/* [3] */
		"1 byte",
		/* [4] */
		"^0 bytes",
		/* [5] */
		"error"
	}
};

resource 'STR#' (300, "Script Window Buttons", purgeable) {
	{	/* array StringArray: 11 elements */
		/* [1] */
		"Run",
		/* [2] */
		"Debug",
		/* [3] */
		"Record",
		/* [4] */
		"Step",
		/* [5] */
		"In",
		/* [6] */
		"Out",
		/* [7] */
		"Follow",
		/* [8] */
		"Go",
		/* [9] */
		"Stop",
		/* [10] */
		"Kill",
		/* [11] */
		"Lookup",
		/* [12] */
		"Compile"
	}
};

resource 'STR#' (301, "Outline Window Buttons", purgeable) {
	{	/* array StringArray: 1 element */
		/* [1] */
		"Synthetic"
	}
};

resource 'STR#' (302, "Home Window Buttons", purgeable) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"Menu Bar",
		/* [2] */
		"Object DB",
		/* [3] */
		"Quick Script",
		/* [4] */
		"Tech Support"
	}
};

resource 'DLOG' (255, "new value", purgeable) {
	{56, 74, 199, 288},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	255,
	"",
	centerParentWindowScreen
};

resource 'DLOG' (256, "Save?", purgeable) { /* 2005-09-06 creedon - sizes generally match threeway */
	{0, 0, 105, 347},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	256,
	"",
	centerMainScreen
};

resource 'DLOG' (257, "replace?", purgeable) {
	{0, 0, 117, 286},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	257,
	"",
	centerParentWindowScreen
};

resource 'DLOG' (258, "Menubar", purgeable) {
	{0, 0, 212, 312},
	zoomDocProc,
	invisible,
	goAway,
	0x0,
	258,
	"Menubar",
	noAutoCenter
};

resource 'dctb' (258, "Menubar") {

	#ifdef oldTemp
	0,
	0,
	#endif
	
	{	/* array ColorSpec: 5 elements */
	
	#if 0
		/* [1] */
		wContentColor, 65535, 65535, 65535,
		/* [2] */
		wFrameColor, 0, 0, 0,
		/* [3] */
		wTextColor, 0, 0, 0,
		/* [4] */
		wHiliteColor, 0, 0, 0,
		/* [5] */
		wTitleBarColor, 65535, 65535, 65535
	#endif
	
	}
};

resource 'DLOG' (267, "QuickScript", purgeable) {
	{82, 144, 182, 444},
	2080, /*dmb: use 1986 to use Infinity Windoid*/
	invisible,
	goAway,
	0x0,
	267,
	"QuickScript",
	noAutoCenter
};

resource 'DLOG' (269, "revert?", purgeable) {
	{0, 0, 112, 262},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	269,
	"",
	noAutoCenter
};

/* kw - 2006-02-05 --- missing; taken from carbon DLOG 518 */
resource 'DLOG' (512, "find", purgeable) {
	{0, 0, 123, 451},
	noGrowDocProc,
	invisible,
	goAway,
	0x0,
	512,
	"Find & Replace Dialog",
	noAutoCenter
};

/* kw - 2006-02-05 --- personalisation dialog - was in root resourcefork */
resource 'DLOG' (515, "personalisation", purgeable) {
	{32, 188, 326, 453},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	515,
	"",
	noAutoCenter
};

/* kw - 2006-02-05 --- personalisation dialog - was in root resourcefork */
resource 'DLOG' (516, "tableinfo", purgeable) {
	{50, 100, 250, 452},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	516,
	"",
	noAutoCenter
};

/* kw - 2006-02-05 --- reconcile dialog - was in root resourcefork */
resource 'DLOG' (25000, "reconcile", purgeable) {
	{137, 259, 339, 611},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	25000,
	"",
	noAutoCenter
};

/*unsorted root resources*/

data 'FMAT' (128) {
	$"3B00 0006 000B 0010 0010 0006 000B 0004"            /* ;............... */
	$"0004 0004 0000 0000 0000 0100 0100 0100"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0F08 0606"            /* ................ */
	$"0606 0906 0606 0608 0606 0606"                      /* ..Æ......... */
};

data 'ictb' (517) {
	$"0000 0000 0000 0000 0000 0000"                      /* ............ */
};

resource 'itl0' (128) {
	period,
	comma,
	semicolon,
	dollarsign,
	"",
	"",
	leadingZero,
	trailingZero,
	minusSign,
	leads,
	monDayYear,
	century,
	noMonthLeadZero,
	noDayLeadZero,
	slash,
	twentyFourHour,
	hoursLeadZero,
	minutesLeadZero,
	secondsLeadZero,
	"",
	"",
	":",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	standard,
	0,
	1
};

resource 'itl1' (128) {
	{	/* array: 7 elements */
		/* [1] */
		"Sunday",
		/* [2] */
		"Monday",
		/* [3] */
		"Tuesday",
		/* [4] */
		"Wednesday",
		/* [5] */
		"Thursday",
		/* [6] */
		"Friday",
		/* [7] */
		"Saturday"
	},
	{	/* array: 12 elements */
		/* [1] */
		"January",
		/* [2] */
		"February",
		/* [3] */
		"March",
		/* [4] */
		"April",
		/* [5] */
		"May",
		/* [6] */
		"June",
		/* [7] */
		"July",
		/* [8] */
		"August",
		/* [9] */
		"September",
		/* [10] */
		"October",
		/* [11] */
		"November",
		/* [12] */
		"December"
	},
	dayName,
	-31,
	noDayLeadZero,
	3,
	"",
	", ",
	" ",
	" ",
	"",
	0,
	1,
	extFormat {
		0x700,
		0x1,
		0,
		{	/* array extraDaysArray: 0 elements */
		},
		{	/* array extraMonthArray: 0 elements */
		},
		{	/* array abbrevDaysArray: 7 elements */
			/* [1] */
			"Sun",
			/* [2] */
			"Mon",
			/* [3] */
			"Tue",
			/* [4] */
			"Wed",
			/* [5] */
			"Thu",
			/* [6] */
			"Fri",
			/* [7] */
			"Sat"
		},
		{	/* array abbrevMonthArray: 12 elements */
			/* [1] */
			"Jan",
			/* [2] */
			"Feb",
			/* [3] */
			"Mar",
			/* [4] */
			"Apr",
			/* [5] */
			"May",
			/* [6] */
			"Jun",
			/* [7] */
			"Jul",
			/* [8] */
			"Aug",
			/* [9] */
			"Sep",
			/* [10] */
			"Oct",
			/* [11] */
			"Nov",
			/* [12] */
			"Dec"
		},
		{	/* array extraSeparatorsArray: 2 elements */
			/* [1] */
			"-",
			/* [2] */
			"."
		}
	}
};

resource 'itl4' (128) {
	0x0,
	128,
	0x700,
	haveLengthFields {
		0,
		0,
		0,
		354,
		172,
		12
	},
	1082261584,
	{	/* array MapChar: 256 elements */
		/* [1] */
		0,
		/* [2] */
		0,
		/* [3] */
		0,
		/* [4] */
		0,
		/* [5] */
		0,
		/* [6] */
		0,
		/* [7] */
		0,
		/* [8] */
		0,
		/* [9] */
		0,
		/* [10] */
		1,
		/* [11] */
		0,
		/* [12] */
		0,
		/* [13] */
		0,
		/* [14] */
		6,
		/* [15] */
		0,
		/* [16] */
		0,
		/* [17] */
		0,
		/* [18] */
		0,
		/* [19] */
		0,
		/* [20] */
		0,
		/* [21] */
		0,
		/* [22] */
		0,
		/* [23] */
		0,
		/* [24] */
		0,
		/* [25] */
		0,
		/* [26] */
		0,
		/* [27] */
		0,
		/* [28] */
		0,
		/* [29] */
		0,
		/* [30] */
		0,
		/* [31] */
		0,
		/* [32] */
		0,
		/* [33] */
		1,
		/* [34] */
		43,
		/* [35] */
		51,
		/* [36] */
		69,
		/* [37] */
		70,
		/* [38] */
		54,
		/* [39] */
		57,
		/* [40] */
		52,
		/* [41] */
		16,
		/* [42] */
		17,
		/* [43] */
		26,
		/* [44] */
		24,
		/* [45] */
		45,
		/* [46] */
		25,
		/* [47] */
		46,
		/* [48] */
		29,
		/* [49] */
		5,
		/* [50] */
		5,
		/* [51] */
		5,
		/* [52] */
		5,
		/* [53] */
		5,
		/* [54] */
		5,
		/* [55] */
		5,
		/* [56] */
		5,
		/* [57] */
		5,
		/* [58] */
		5,
		/* [59] */
		68,
		/* [60] */
		53,
		/* [61] */
		31,
		/* [62] */
		33,
		/* [63] */
		32,
		/* [64] */
		60,
		/* [65] */
		58,
		/* [66] */
		4,
		/* [67] */
		4,
		/* [68] */
		4,
		/* [69] */
		4,
		/* [70] */
		4,
		/* [71] */
		4,
		/* [72] */
		4,
		/* [73] */
		4,
		/* [74] */
		4,
		/* [75] */
		4,
		/* [76] */
		4,
		/* [77] */
		4,
		/* [78] */
		4,
		/* [79] */
		4,
		/* [80] */
		4,
		/* [81] */
		4,
		/* [82] */
		4,
		/* [83] */
		4,
		/* [84] */
		4,
		/* [85] */
		4,
		/* [86] */
		4,
		/* [87] */
		4,
		/* [88] */
		4,
		/* [89] */
		4,
		/* [90] */
		4,
		/* [91] */
		4,
		/* [92] */
		18,
		/* [93] */
		30,
		/* [94] */
		19,
		/* [95] */
		55,
		/* [96] */
		56,
		/* [97] */
		0,
		/* [98] */
		4,
		/* [99] */
		4,
		/* [100] */
		4,
		/* [101] */
		4,
		/* [102] */
		4,
		/* [103] */
		4,
		/* [104] */
		4,
		/* [105] */
		4,
		/* [106] */
		4,
		/* [107] */
		4,
		/* [108] */
		4,
		/* [109] */
		4,
		/* [110] */
		4,
		/* [111] */
		4,
		/* [112] */
		4,
		/* [113] */
		4,
		/* [114] */
		4,
		/* [115] */
		4,
		/* [116] */
		4,
		/* [117] */
		4,
		/* [118] */
		4,
		/* [119] */
		4,
		/* [120] */
		4,
		/* [121] */
		4,
		/* [122] */
		4,
		/* [123] */
		4,
		/* [124] */
		20,
		/* [125] */
		59,
		/* [126] */
		21,
		/* [127] */
		44,
		/* [128] */
		0,
		/* [129] */
		4,
		/* [130] */
		4,
		/* [131] */
		4,
		/* [132] */
		4,
		/* [133] */
		4,
		/* [134] */
		4,
		/* [135] */
		4,
		/* [136] */
		4,
		/* [137] */
		4,
		/* [138] */
		4,
		/* [139] */
		4,
		/* [140] */
		4,
		/* [141] */
		4,
		/* [142] */
		4,
		/* [143] */
		4,
		/* [144] */
		4,
		/* [145] */
		4,
		/* [146] */
		4,
		/* [147] */
		4,
		/* [148] */
		4,
		/* [149] */
		4,
		/* [150] */
		4,
		/* [151] */
		4,
		/* [152] */
		4,
		/* [153] */
		4,
		/* [154] */
		4,
		/* [155] */
		4,
		/* [156] */
		4,
		/* [157] */
		4,
		/* [158] */
		4,
		/* [159] */
		4,
		/* [160] */
		4,
		/* [161] */
		0,
		/* [162] */
		0,
		/* [163] */
		0,
		/* [164] */
		0,
		/* [165] */
		0,
		/* [166] */
		78,
		/* [167] */
		0,
		/* [168] */
		4,
		/* [169] */
		0,
		/* [170] */
		0,
		/* [171] */
		0,
		/* [172] */
		0,
		/* [173] */
		0,
		/* [174] */
		40,
		/* [175] */
		4,
		/* [176] */
		4,
		/* [177] */
		67,
		/* [178] */
		28,
		/* [179] */
		35,
		/* [180] */
		37,
		/* [181] */
		0,
		/* [182] */
		65,
		/* [183] */
		0,
		/* [184] */
		63,
		/* [185] */
		66,
		/* [186] */
		61,
		/* [187] */
		64,
		/* [188] */
		0,
		/* [189] */
		0,
		/* [190] */
		0,
		/* [191] */
		4,
		/* [192] */
		4,
		/* [193] */
		0,
		/* [194] */
		0,
		/* [195] */
		0,
		/* [196] */
		62,
		/* [197] */
		0,
		/* [198] */
		0,
		/* [199] */
		0,
		/* [200] */
		22,
		/* [201] */
		23,
		/* [202] */
		77,
		/* [203] */
		71,
		/* [204] */
		4,
		/* [205] */
		4,
		/* [206] */
		4,
		/* [207] */
		4,
		/* [208] */
		4,
		/* [209] */
		0,
		/* [210] */
		0,
		/* [211] */
		47,
		/* [212] */
		48,
		/* [213] */
		49,
		/* [214] */
		50,
		/* [215] */
		27,
		/* [216] */
		0,
		/* [217] */
		4,
		/* [218] */
		4,
		/* [219] */
		72,
		/* [220] */
		73,
		/* [221] */
		74,
		/* [222] */
		75,
		/* [223] */
		4,
		/* [224] */
		4,
		/* [225] */
		0,
		/* [226] */
		78,
		/* [227] */
		0,
		/* [228] */
		0,
		/* [229] */
		76,
		/* [230] */
		4,
		/* [231] */
		4,
		/* [232] */
		4,
		/* [233] */
		4,
		/* [234] */
		4,
		/* [235] */
		4,
		/* [236] */
		4,
		/* [237] */
		4,
		/* [238] */
		4,
		/* [239] */
		4,
		/* [240] */
		4,
		/* [241] */
		0,
		/* [242] */
		4,
		/* [243] */
		4,
		/* [244] */
		4,
		/* [245] */
		4,
		/* [246] */
		4,
		/* [247] */
		4,
		/* [248] */
		4,
		/* [249] */
		4,
		/* [250] */
		4,
		/* [251] */
		4,
		/* [252] */
		4,
		/* [253] */
		4,
		/* [254] */
		4,
		/* [255] */
		4,
		/* [256] */
		4
	},
	$"4E72 0000",
	$"206D 0014 262D 0010 676E 5383 4A2D 0021"
	$"6704 D1ED 001C 246F 0006 2C48 DDED 0018"
	$"2548 000A 266A 0002 2C2A 0006 7E00 1E06"
	$"DE88 BE8E 6A4C 10C6 672A 5306 3E12 0C47"
	$"000C 6708 10DB 51CE FFFC 6018 1A1B 0C05"
	$"0030 6D06 0C05 0039 6F04 1A3C 002E 10C5"
	$"51CE FFEA 4218 2C08 5286 0886 0000 2046"
	$"45EA 000E 51CB FFAA 91ED 0014 2B48 001C"
	$"4E75 3F7C 0002 0006 4E75",
	{	/* array unTokenStringOffsets: 79 elements */
		/* [1] */
		194,
		/* [2] */
		198,
		/* [3] */
		200,
		/* [4] */
		202,
		/* [5] */
		186,
		/* [6] */
		190,
		/* [7] */
		204,
		/* [8] */
		206,
		/* [9] */
		210,
		/* [10] */
		182,
		/* [11] */
		214,
		/* [12] */
		174,
		/* [13] */
		178,
		/* [14] */
		170,
		/* [15] */
		162,
		/* [16] */
		166,
		/* [17] */
		216,
		/* [18] */
		218,
		/* [19] */
		220,
		/* [20] */
		222,
		/* [21] */
		224,
		/* [22] */
		226,
		/* [23] */
		228,
		/* [24] */
		230,
		/* [25] */
		232,
		/* [26] */
		234,
		/* [27] */
		236,
		/* [28] */
		238,
		/* [29] */
		240,
		/* [30] */
		242,
		/* [31] */
		244,
		/* [32] */
		246,
		/* [33] */
		248,
		/* [34] */
		250,
		/* [35] */
		252,
		/* [36] */
		256,
		/* [37] */
		258,
		/* [38] */
		262,
		/* [39] */
		264,
		/* [40] */
		268,
		/* [41] */
		272,
		/* [42] */
		274,
		/* [43] */
		278,
		/* [44] */
		282,
		/* [45] */
		284,
		/* [46] */
		286,
		/* [47] */
		288,
		/* [48] */
		290,
		/* [49] */
		292,
		/* [50] */
		294,
		/* [51] */
		296,
		/* [52] */
		298,
		/* [53] */
		300,
		/* [54] */
		302,
		/* [55] */
		304,
		/* [56] */
		306,
		/* [57] */
		308,
		/* [58] */
		310,
		/* [59] */
		312,
		/* [60] */
		314,
		/* [61] */
		316,
		/* [62] */
		318,
		/* [63] */
		320,
		/* [64] */
		322,
		/* [65] */
		324,
		/* [66] */
		326,
		/* [67] */
		328,
		/* [68] */
		330,
		/* [69] */
		332,
		/* [70] */
		334,
		/* [71] */
		336,
		/* [72] */
		338,
		/* [73] */
		340,
		/* [74] */
		342,
		/* [75] */
		344,
		/* [76] */
		346,
		/* [77] */
		348,
		/* [78] */
		350,
		/* [79] */
		352
	},
	{	/* array: 79 elements */
		/* [1] */
		"??",
		/* [2] */
		"??",
		/* [3] */
		"??",
		/* [4] */
		"??",
		/* [5] */
		"??",
		/* [6] */
		"??",
		/* [7] */
		"??",
		/* [8] */
		"??",
		/* [9] */
		"??",
		/* [10] */
		" ",
		/* [11] */
		"\"",
		/* [12] */
		"\"",
		/* [13] */
		"\n",
		/* [14] */
		"/*",
		/* [15] */
		"*/",
		/* [16] */
		"\\",
		/* [17] */
		"(",
		/* [18] */
		")",
		/* [19] */
		"[",
		/* [20] */
		"]",
		/* [21] */
		"{",
		/* [22] */
		"}",
		/* [23] */
		"Ç",
		/* [24] */
		"È",
		/* [25] */
		"+",
		/* [26] */
		"-",
		/* [27] */
		"*",
		/* [28] */
		"Ö",
		/* [29] */
		"±",
		/* [30] */
		"/",
		/* [31] */
		"\\",
		/* [32] */
		"<",
		/* [33] */
		">",
		/* [34] */
		"=",
		/* [35] */
		"<=",
		/* [36] */
		"²",
		/* [37] */
		">=",
		/* [38] */
		"³",
		/* [39] */
		"==",
		/* [40] */
		":=",
		/* [41] */
		"­",
		/* [42] */
		"<>",
		/* [43] */
		"!=",
		/* [44] */
		"!",
		/* [45] */
		"~",
		/* [46] */
		",",
		/* [47] */
		".",
		/* [48] */
		"Ò",
		/* [49] */
		"Ó",
		/* [50] */
		"Ô",
		/* [51] */
		"Õ",
		/* [52] */
		"\"",
		/* [53] */
		"'",
		/* [54] */
		";",
		/* [55] */
		"%",
		/* [56] */
		"^",
		/* [57] */
		"_",
		/* [58] */
		"&",
		/* [59] */
		"@",
		/* [60] */
		"|",
		/* [61] */
		"?",
		/* [62] */
		"¹",
		/* [63] */
		"Ã",
		/* [64] */
		"·",
		/* [65] */
		"º",
		/* [66] */
		"µ",
		/* [67] */
		"¸",
		/* [68] */
		"°",
		/* [69] */
		":",
		/* [70] */
		"#",
		/* [71] */
		"$",
		/* [72] */
		"Ê",
		/* [73] */
		"Ú",
		/* [74] */
		"Û",
		/* [75] */
		"Ü",
		/* [76] */
		"Ý",
		/* [77] */
		"ä",
		/* [78] */
		"É",
		/* [79] */
		"¥"
	},
	1,
	{	/* array: 31 elements */
		/* [1] */
		0, "'",
		/* [2] */
		0, "'",
		/* [3] */
		0, "^",
		/* [4] */
		0, "Ê",
		/* [5] */
		0, "#",
		/* [6] */
		0, "0",
		/* [7] */
		0, "%",
		/* [8] */
		0, "+",
		/* [9] */
		0, "-",
		/* [10] */
		0, ",",
		/* [11] */
		0, "",
		/* [12] */
		0, ";",
		/* [13] */
		0, "\\",
		/* [14] */
		0, ".",
		/* [15] */
		0, "(",
		/* [16] */
		0, ")",
		/* [17] */
		0, "",
		/* [18] */
		0, "",
		/* [19] */
		0, "",
		/* [20] */
		0, "",
		/* [21] */
		0, "",
		/* [22] */
		0, "",
		/* [23] */
		0, "",
		/* [24] */
		0, "",
		/* [25] */
		0, "",
		/* [26] */
		0, "",
		/* [27] */
		0, "",
		/* [28] */
		0, "",
		/* [29] */
		0, "",
		/* [30] */
		0, "",
		/* [31] */
		0, ""
	},
	1,
	{	/* array: 10 elements */
		/* [1] */
		0, "E",
		/* [2] */
		0, "+",
		/* [3] */
		0, "",
		/* [4] */
		0, "",
		/* [5] */
		0, "",
		/* [6] */
		0, "",
		/* [7] */
		0, "",
		/* [8] */
		0, "",
		/* [9] */
		0, "",
		/* [10] */
		0, ""
	},
	1,
	{	/* array: 10 elements */
		/* [1] */
		0, "E",
		/* [2] */
		0, "-",
		/* [3] */
		0, "",
		/* [4] */
		0, "",
		/* [5] */
		0, "",
		/* [6] */
		0, "",
		/* [7] */
		0, "",
		/* [8] */
		0, "",
		/* [9] */
		0, "",
		/* [10] */
		0, ""
	},
	0,
	{	/* array: 10 elements */
		/* [1] */
		0, "E",
		/* [2] */
		0, "",
		/* [3] */
		0, "",
		/* [4] */
		0, "",
		/* [5] */
		0, "",
		/* [6] */
		0, "",
		/* [7] */
		0, "",
		/* [8] */
		0, "",
		/* [9] */
		0, "",
		/* [10] */
		0, ""
	},
	9,
	{	/* array: 10 elements */
		/* [1] */
		0, "0",
		/* [2] */
		0, "1",
		/* [3] */
		0, "2",
		/* [4] */
		0, "3",
		/* [5] */
		0, "4",
		/* [6] */
		0, "5",
		/* [7] */
		0, "6",
		/* [8] */
		0, "7",
		/* [9] */
		0, "8",
		/* [10] */
		0, "9"
	},
	{	/* array: 1 elements */
		/* [1] */
		{	/* array whiteSpaceStringOffsets: 2 elements */
			/* [1] */
			8,
			/* [2] */
			10
		},
		{	/* array: 2 elements */
			/* [1] */
			" ",
			/* [2] */
			"\t"
		}
	}
};

resource 'MENU' (25000) {
	25000,
	44,
	allEnabled,
	enabled,
	"Jump List",
	{	/* array: 0 elements */
	}
};

data 'scpt' (128) {
	$"7461 626C 0001 031C 0000 04A8 0000 0014"            /* tabl.......¨.... */
	$"0000 0000 0A01 0000 0006 0000 02C8 0C01"            /* ....Â........È.. */
	$"0000 02D0 055F 636F 6465 0000 02BE 0000"            /* ...Ð._code...¾.. */
	$"01F2 0000 0000 0028 0000 0000 0000 0000"            /* .ò.....(........ */
	$"0000 0000 2100 0000 0000 0001 00BC 2800"            /* ....!........¼(. */
	$"2100 0000 0000 0001 0040 2C80 2F00 0000"            /* !........@,€/... */
	$"0000 0001 00AD 3600 2D00 0000 0000 0001"            /* .....­6.-....... */
	$"00E3 2C00 2200 0000 0000 0001 00CF 2C80"            /* .ã,."........Ï,€ */
	$"2200 0000 0000 0001 00C0 2C00 0600 0000"            /* "........À,..... */
	$"0008 0001 00BB 0000 0600 0000 0008 0001"            /* .....».......... */
	$"00C0 0000 0600 0000 0012 0001 00CF 0000"            /* .À...........Ï.. */
	$"2200 0000 0000 0001 00E1 2C00 2200 0000"            /* "........á,."... */
	$"0000 0001 00DA 2C00 0600 0000 0008 0001"            /* .....Ú,......... */
	$"00D5 0000 0600 0000 0008 0001 00DA 0000"            /* .Õ...........Ú.. */
	$"0600 0000 000A 0001 00E1 0000 1C00 0000"            /* .....Â...á...... */
	$"0000 0001 0006 1800 0B00 0000 0000 0001"            /* ................ */
	$"0004 2C00 0600 0000 000A 0001 00F5 0000"            /* ..,......Â...õ.. */
	$"0B00 0000 0000 0001 0003 2C00 0600 0000"            /* ..........,..... */
	$"000C 0001 00FF 0000 0600 0000 0005 0001"            /* .....ÿ.......... */
	$"0002 0000 0B00 0000 0000 0001 003E 2C00"            /* .............>,. */
	$"0600 0000 000F 0001 001D 0000 0B00 0000"            /* ................ */
	$"0000 0001 003D 2C00 2200 0000 0000 0001"            /* .....=,."....... */
	$"0032 2C00 0600 0000 0008 0001 0023 0000"            /* .2,..........#.. */
	$"0600 0000 0012 0001 0032 0000 0600 0000"            /* .........2...... */
	$"000C 0001 003C 0000 1F00 0000 0000 0001"            /* .....<.......... */
	$"0084 2C00 0600 0000 0008 0001 0080 0000"            /* .„,..........€.. */
	$"0600 0000 0005 0001 0083 0000 2100 0000"            /* .........ƒ..!... */
	$"0000 0001 00BC 2C00 1C00 0000 0000 0001"            /* .....¼,......... */
	$"00BC 1800 0B00 0000 0000 0001 00BA 2C00"            /* .¼...........º,. */
	$"2200 0000 0000 0001 00B3 2C00 0600 0000"            /* "........³,..... */
	$"0008 0001 00A7 0000 0600 0000 000F 0001"            /* .....§.......... */
	$"00B3 0000 0600 0000 0008 0001 00B9 0000"            /* .³...........¹.. */
	$"1F00 0000 0000 0001 0050 2C00 0600 0000"            /* .........P,..... */
	$"0008 0001 0049 0000 0600 0000 0008 0001"            /* .....I.......... */
	$"004F 0000 5445 5854 6874 6D6C 5445 5854"            /* .O..TEXThtmlTEXT */
	$"6461 7461 5445 5854 7374 616E 6461 7264"            /* dataTEXTstandard */
	$"6D61 6372 6F73 5445 5854 7573 6572 5445"            /* macrosTEXTuserTE */
	$"5854 6874 6D6C 5445 5854 6D61 6372 6F73"            /* XThtmlTEXTmacros */
	$"5445 5854 7374 7269 6E67 5445 5854 6576"            /* TEXTstringTEXTev */
	$"616C 7561 7465 5445 5854 7354 4558 5473"            /* aluateTEXTsTEXTs */
	$"6372 6970 7445 7272 6F72 5445 5854 746F"            /* criptErrorTEXTto */
	$"7973 5445 5854 636C 6561 6E66 6F72 6578"            /* ysTEXTcleanforex */
	$"706F 7274 5445 5854 7472 7945 7272 6F72"            /* portTEXTtryError */
	$"5445 5854 646F 7363 5445 5854 7354 4558"            /* TEXTdoscTEXTsTEX */
	$"5468 746D 6C54 4558 5472 6566 476C 6F73"            /* ThtmlTEXTrefGlos */
	$"7361 7279 5445 5854 6E61 6D65 5445 5854"            /* saryTEXTnameTEXT */
	$"7265 6667 5445 5854 6E61 6D65 075F 736F"            /* refgTEXTname._so */
	$"7572 6365 0000 01BC C774 6869 7320 7363"            /* urce...¼Çthis sc */
	$"7269 7074 2069 7320 696E 7374 616C 6C65"            /* ript is installe */
	$"6420 696E 2046 726F 6E74 6965 722E 726F"            /* d in Frontier.ro */
	$"6F74 2773 2072 6573 6F75 7263 6520 666F"            /* ot's resource fo */
	$"726B 0D09 C769 7427 7320 696E 2027 7363"            /* rk.ÆÇit's in 'sc */
	$"7074 2720 7265 736F 7572 6365 2031 3238"            /* pt' resource 128 */
	$"0D09 C769 7427 7320 6361 6C6C 6564 2069"            /* .ÆÇit's called i */
	$"6E20 6F73 6166 726F 6E74 6965 722E 630D"            /* n osafrontier.c. */
	$"0D6F 6E20 646F 7363 2028 7329 207B 20C7"            /* .on dosc (s) { Ç */
	$"7275 6E73 2061 206D 6163 726F 2C20 666F"            /* runs a macro, fo */
	$"7220 7468 6520 6874 6D6C 2073 7569 7465"            /* r the html suite */
	$"0D09 7472 7920 7B0D 0909 7769 7468 2068"            /* .Ætry {.ÆÆwith h */
	$"746D 6C2E 6461 7461 2E73 7461 6E64 6172"            /* tml.data.standar */
	$"646D 6163 726F 732C 2075 7365 722E 6874"            /* dmacros, user.ht */
	$"6D6C 2E6D 6163 726F 7320 7B0D 0909 0972"            /* ml.macros {.ÆÆÆr */
	$"6574 7572 6E20 2873 7472 696E 6720 2865"            /* eturn (string (e */
	$"7661 6C75 6174 6520 2873 2929 297D 7D0D"            /* valuate (s)))}}. */
	$"0965 6C73 6520 7B0D 0909 7363 7269 7074"            /* Æelse {.ÆÆscript */
	$"4572 726F 7220 2874 6F79 732E 636C 6561"            /* Error (toys.clea */
	$"6E66 6F72 6578 706F 7274 2028 7472 7945"            /* nforexport (tryE */
	$"7272 6F72 2929 7D7D 3B0D 6F6E 2072 6566"            /* rror))}};.on ref */
	$"6720 286E 616D 6529 207B 20C7 7475 726E"            /* g (name) { Çturn */
	$"2061 2067 6C6F 7373 6172 7920 656E 7472"            /*  a glossary entr */
	$"7920 6E61 6D65 2069 6E74 6F20 6120 7374"            /* y name into a st */
	$"7269 6E67 2066 6F72 2069 6E73 6572 7469"            /* ring for inserti */
	$"6F6E 2069 6E74 6F20 4854 4D4C 2074 6578"            /* on into HTML tex */
	$"740D 0972 6574 7572 6E20 2868 746D 6C2E"            /* t.Æreturn (html. */
	$"7265 6647 6C6F 7373 6172 7920 286E 616D"            /* refGlossary (nam */
	$"6529 297D 4C41 4E44 0001 000C FADE DEAD"            /* e))}LAND....úÞÞ­ */
};

data 'scpt' (129) {
	$"7461 626C 0001 03F0 0000 0746 0000 0014"            /* tabl...ð...F.... */
	$"0000 0000 0A01 0000 0006 0000 0506 0C01"            /* ....Â........... */
	$"0000 050E 055F 636F 6465 0000 04FC 0000"            /* ....._code...ü.. */
	$"028E 0000 0000 0035 0000 0000 0000 0000"            /* .Ž.....5........ */
	$"0000 0000 2100 0000 0000 0001 001C 2800"            /* ....!.........(. */
	$"2100 0000 0000 0001 001C 2C00 1C00 0000"            /* !.........,..... */
	$"0000 0001 001C 1800 0B00 0000 0000 0001"            /* ................ */
	$"001A 2C00 2200 0000 0000 0001 0020 2C00"            /* ..,."........ ,. */
	$"0600 0000 000D 0001 0018 0000 0600 0000"            /* ................ */
	$"000B 0001 0020 0000 0600 0000 000C 0001"            /* ..... .......... */
	$"002A 0080 0600 0000 0012 0001 003A 0080"            /* .*.€.........:.€ */
	$"0600 0000 000C 0001 0044 0080 0600 0000"            /* .........D.€.... */
	$"000C 0001 004E 0080 0600 0000 000C 0001"            /* .....N.€........ */
	$"0058 0080 0600 0000 0011 0001 0067 0080"            /* .X.€.........g.€ */
	$"0600 0000 000E 0001 0073 0080 0600 0000"            /* .........s.€.... */
	$"000E 0001 0080 0080 0600 0000 000E 0001"            /* .....€.€........ */
	$"008C 0080 0600 0000 000F 0001 0099 0080"            /* .Œ.€.........™.€ */
	$"0600 0000 000B 0001 00A2 0080 0600 0000"            /* .........¢.€.... */
	$"000D 0001 00AD 0080 0600 0000 000A 0001"            /* .....­.€.....Â.. */
	$"00B5 0080 0600 0000 000E 0001 00C1 0080"            /* .µ.€.........Á.€ */
	$"0600 0000 000C 0001 00CB 0080 0600 0000"            /* .........Ë.€.... */
	$"000A 0001 00D3 0080 0600 0000 000C 0001"            /* .Â...Ó.€........ */
	$"00DD 0080 0600 0000 000F 0001 00EA 0080"            /* .Ý.€.........ê.€ */
	$"0600 0000 0010 0001 00F8 0080 0600 0000"            /* .........ø.€.... */
	$"0009 0001 00FE 0080 0600 0000 000E 0001"            /* .Æ...þ.€........ */
	$"000A 0080 0600 0000 0011 0001 0019 0000"            /* .Â.€............ */
	$"1F00 0000 0000 0001 0001 2C00 0600 0000"            /* ..........,..... */
	$"0008 0001 0007 0000 0600 0000 000C 0001"            /* ................ */
	$"0011 0080 0600 0000 0012 0001 0021 0080"            /* ...€.........!.€ */
	$"0600 0000 000C 0001 002B 0080 0600 0000"            /* .........+.€.... */
	$"000C 0001 0035 0080 0600 0000 000C 0001"            /* .....5.€........ */
	$"003F 0080 0600 0000 0011 0001 004E 0080"            /* .?.€.........N.€ */
	$"0600 0000 000E 0001 005A 0080 0600 0000"            /* .........Z.€.... */
	$"000E 0001 0067 0080 0600 0000 000E 0001"            /* .....g.€........ */
	$"0073 0080 0600 0000 000F 0001 0080 0080"            /* .s.€.........€.€ */
	$"0600 0000 000B 0001 0089 0080 0600 0000"            /* .........‰.€.... */
	$"000D 0001 0094 0080 0600 0000 000A 0001"            /* .....”.€.....Â.. */
	$"009C 0080 0600 0000 000E 0001 00A8 0080"            /* .œ.€.........¨.€ */
	$"0600 0000 000C 0001 00B2 0080 0600 0000"            /* .........².€.... */
	$"000A 0001 00BA 0080 0600 0000 000C 0001"            /* .Â...º.€........ */
	$"00C4 0080 0600 0000 000F 0001 00D1 0080"            /* .Ä.€.........Ñ.€ */
	$"0600 0000 0010 0001 00DF 0080 0600 0000"            /* .........ß.€.... */
	$"0009 0001 00E5 0080 0600 0000 000E 0001"            /* .Æ...å.€........ */
	$"00F1 0080 0600 0000 0011 0001 0000 0000"            /* .ñ.€............ */
	$"5445 5854 7765 6273 6572 7665 7254 4558"            /* TEXTwebserverTEX */
	$"5468 616E 646C 6572 5445 5854 7061 7468"            /* ThandlerTEXTpath */
	$"4172 6773 5445 5854 6874 7470 5365 6172"            /* ArgsTEXThttpSear */
	$"6368 4172 6773 5445 5854 7573 6572 6E61"            /* chArgsTEXTuserna */
	$"6D65 5445 5854 7061 7373 776F 7264 5445"            /* meTEXTpasswordTE */
	$"5854 6672 6F6D 5573 6572 5445 5854 636C"            /* XTfromUserTEXTcl */
	$"6965 6E74 4164 6472 6573 7354 4558 5473"            /* ientAddressTEXTs */
	$"6572 7665 724E 616D 6554 4558 5473 6572"            /* erverNameTEXTser */
	$"7665 7250 6F72 7454 4558 5473 6372 6970"            /* verPortTEXTscrip */
	$"744E 616D 6554 4558 5463 6F6E 7465 6E74"            /* tNameTEXTcontent */
	$"5479 7065 5445 5854 7265 6665 7265 7254"            /* TypeTEXTrefererT */
	$"4558 5475 7365 7241 6765 6E74 5445 5854"            /* EXTuserAgentTEXT */
	$"6163 7469 6F6E 5445 5854 6163 7469 6F6E"            /* actionTEXTaction */
	$"5061 7468 5445 5854 706F 7374 4172 6773"            /* PathTEXTpostArgs */
	$"5445 5854 6D65 7468 6F64 5445 5854 636C"            /* TEXTmethodTEXTcl */
	$"6965 6E74 4970 5445 5854 6675 6C6C 5265"            /* ientIpTEXTfullRe */
	$"7175 6573 7454 4558 5463 6F6E 6E65 6374"            /* questTEXTconnect */
	$"696F 6E49 4454 4558 5461 7070 4944 5445"            /* ionIDTEXTappIDTE */
	$"5854 7365 7276 6572 5061 7468 5445 5854"            /* XTserverPathTEXT */
	$"7765 6273 6974 6546 6F6C 6465 7254 4558"            /* websiteFolderTEX */
	$"5466 6367 6954 4558 5470 6174 6841 7267"            /* TfcgiTEXTpathArg */
	$"7354 4558 5468 7474 7053 6561 7263 6841"            /* sTEXThttpSearchA */
	$"7267 7354 4558 5475 7365 726E 616D 6554"            /* rgsTEXTusernameT */
	$"4558 5470 6173 7377 6F72 6454 4558 5466"            /* EXTpasswordTEXTf */
	$"726F 6D55 7365 7254 4558 5463 6C69 656E"            /* romUserTEXTclien */
	$"7441 6464 7265 7373 5445 5854 7365 7276"            /* tAddressTEXTserv */
	$"6572 4E61 6D65 5445 5854 7365 7276 6572"            /* erNameTEXTserver */
	$"506F 7274 5445 5854 7363 7269 7074 4E61"            /* PortTEXTscriptNa */
	$"6D65 5445 5854 636F 6E74 656E 7454 7970"            /* meTEXTcontentTyp */
	$"6554 4558 5472 6566 6572 6572 5445 5854"            /* eTEXTrefererTEXT */
	$"7573 6572 4167 656E 7454 4558 5461 6374"            /* userAgentTEXTact */
	$"696F 6E54 4558 5461 6374 696F 6E50 6174"            /* ionTEXTactionPat */
	$"6854 4558 5470 6F73 7441 7267 7354 4558"            /* hTEXTpostArgsTEX */
	$"546D 6574 686F 6454 4558 5463 6C69 656E"            /* TmethodTEXTclien */
	$"7449 7054 4558 5466 756C 6C52 6571 7565"            /* tIpTEXTfullReque */
	$"7374 5445 5854 636F 6E6E 6563 7469 6F6E"            /* stTEXTconnection */
	$"4944 5445 5854 6170 7049 4454 4558 5473"            /* IDTEXTappIDTEXTs */
	$"6572 7665 7250 6174 6854 4558 5477 6562"            /* erverPathTEXTweb */
	$"7369 7465 466F 6C64 6572 075F 736F 7572"            /* siteFolder._sour */
	$"6365 0000 021C 6F6E 2066 6367 6920 2870"            /* ce....on fcgi (p */
	$"6174 6841 7267 732C 2068 7474 7053 6561"            /* athArgs, httpSea */
	$"7263 6841 7267 732C 2075 7365 726E 616D"            /* rchArgs, usernam */
	$"652C 2070 6173 7377 6F72 642C 2066 726F"            /* e, password, fro */
	$"6D55 7365 722C 2063 6C69 656E 7441 6464"            /* mUser, clientAdd */
	$"7265 7373 2C20 7365 7276 6572 4E61 6D65"            /* ress, serverName */
	$"2C20 2073 6572 7665 7250 6F72 742C 2073"            /* ,  serverPort, s */
	$"6372 6970 744E 616D 652C 2063 6F6E 7465"            /* criptName, conte */
	$"6E74 5479 7065 2C20 7265 6665 7265 722C"            /* ntType, referer, */
	$"2075 7365 7241 6765 6E74 2C20 6163 7469"            /*  userAgent, acti */
	$"6F6E 2C20 6163 7469 6F6E 5061 7468 2C20"            /* on, actionPath,  */
	$"706F 7374 4172 6773 2C20 6D65 7468 6F64"            /* postArgs, method */
	$"2C20 636C 6965 6E74 4970 2C20 6675 6C6C"            /* , clientIp, full */
	$"5265 7175 6573 742C 2063 6F6E 6E65 6374"            /* Request, connect */
	$"696F 6E49 442C 6170 7049 442C 2073 6572"            /* ionID,appID, ser */
	$"7665 7250 6174 682C 2077 6562 7369 7465"            /* verPath, website */
	$"466F 6C64 6572 2920 7B0D 090D 0972 6574"            /* Folder) {.Æ.Æret */
	$"7572 6E20 2877 6562 7365 7276 6572 2E68"            /* urn (webserver.h */
	$"616E 646C 6572 2028 7061 7468 4172 6773"            /* andler (pathArgs */
	$"2C20 6874 7470 5365 6172 6368 4172 6773"            /* , httpSearchArgs */
	$"2C20 7573 6572 6E61 6D65 2C20 7061 7373"            /* , username, pass */
	$"776F 7264 2C20 6672 6F6D 5573 6572 2C20"            /* word, fromUser,  */
	$"636C 6965 6E74 4164 6472 6573 732C 2073"            /* clientAddress, s */
	$"6572 7665 724E 616D 652C 2020 7365 7276"            /* erverName,  serv */
	$"6572 506F 7274 2C20 7363 7269 7074 4E61"            /* erPort, scriptNa */
	$"6D65 2C20 636F 6E74 656E 7454 7970 652C"            /* me, contentType, */
	$"2072 6566 6572 6572 2C20 7573 6572 4167"            /*  referer, userAg */
	$"656E 742C 2061 6374 696F 6E2C 2061 6374"            /* ent, action, act */
	$"696F 6E50 6174 682C 2070 6F73 7441 7267"            /* ionPath, postArg */
	$"732C 206D 6574 686F 642C 2063 6C69 656E"            /* s, method, clien */
	$"7449 702C 2066 756C 6C52 6571 7565 7374"            /* tIp, fullRequest */
	$"2C20 636F 6E6E 6563 7469 6F6E 4944 2C61"            /* , connectionID,a */
	$"7070 4944 2C20 7365 7276 6572 5061 7468"            /* ppID, serverPath */
	$"2C20 7765 6273 6974 6546 6F6C 6465 7229"            /* , websiteFolder) */
	$"297D 4C41 4E44 0001 000C FADE DEAD"                 /* )}LAND....úÞÞ­ */
};

resource 'STR ' (512) {
	"Oh the buzzing of the bees."
};

data 'TEXT' (128) {
};

data 'TMPL' (128) {
	$"0546 6C61 6773 4857 5244 0454 7970 6554"            /* .FlagsHWRD.TypeT */
	$"4E41 4D02 4944 4457 5244 0756 6572 7369"            /* NAM.IDDWRD.Versi */
	$"6F6E 4857 5244 0646 6F72 6D61 7444 5752"            /* onHWRD.FormatDWR */
	$"4408 5265 7365 7276 6564 4657 5244 0852"            /* D.ReservedFWRD.R */
	$"6573 6572 7665 6446 4C4E 4706 5461 626C"            /* eservedFLNG.Tabl */
	$"6573 4457 5244 0A4D 6170 204F 6666 7365"            /* esDWRDÂMap Offse */
	$"7444 4C4E 4711 5374 7269 6E67 436F 7079"            /* tDLNG.StringCopy */
	$"204F 6666 7365 7444 4C4E 470C 4665 7463"            /*  OffsetDLNG.Fetc */
	$"6820 4F66 6673 6574 444C 4E47 0E55 6E54"            /* h OffsetDLNG.UnT */
	$"6F6B 656E 204F 6666 7365 7444 4C4E 470F"            /* oken OffsetDLNG. */
	$"4465 6650 6172 7473 204F 6666 7365 7444"            /* DefParts OffsetD */
	$"4C4E 4711 5768 6974 6553 7061 6365 204F"            /* LNG.WhiteSpace O */
	$"6666 7365 7444 4C4E 4708 5265 7365 7276"            /* ffsetDLNG.Reserv */
	$"6564 464C 4E47 0852 6573 6572 7665 6446"            /* edFLNG.ReservedF */
	$"4C4E 4715 4D61 7020 4C65 6E67 7468 2028"            /* LNG.Map Length ( */
	$"7265 7365 7276 6564 2946 5752 441C 5374"            /* reserved)FWRD.St */
	$"7269 6E67 436F 7079 204C 656E 6774 6820"            /* ringCopy Length  */
	$"2872 6573 6572 7665 6429 4657 5244 1746"            /* (reserved)FWRD.F */
	$"6574 6368 204C 656E 6774 6820 2872 6573"            /* etch Length (res */
	$"6572 7665 6429 4657 5244 0E55 6E54 6F6B"            /* erved)FWRD.UnTok */
	$"656E 204C 656E 6774 6844 5752 440F 4465"            /* en LengthDWRD.De */
	$"6650 6172 7473 204C 656E 6774 6844 5752"            /* fParts LengthDWR */
	$"4411 5768 6974 6553 7061 6365 204C 656E"            /* D.WhiteSpace Len */
	$"6774 6844 5752 4408 5265 7365 7276 6564"            /* gthDWRD.Reserved */
	$"4657 5244 0852 6573 6572 7665 6446 5752"            /* FWRD.ReservedFWR */
	$"4412 4665 7463 6820 4162 6F72 7420 4F66"            /* D.Fetch Abort Of */
	$"6673 6574 444C 4E47 074D 6170 4368 6172"            /* fsetDLNG.MapChar */
	$"4C53 5452 0F43 6F64 6520 616E 6420 5461"            /* LSTR.Code and Ta */
	$"626C 6573 4845 5844"                                /* blesHEXD */
};

data 'TMPL' (129) {
	$"052A 2A2A 2A2A 4C53 5442 135A 6F6E 6520"            /* .*****LSTB.Zone  */
	$"4F66 6673 6574 204D 696E 7574 6573 4457"            /* Offset MinutesDW */
	$"5244 095A 6F6E 6520 4E61 6D65 4553 5452"            /* RDÆZone NameESTR */
	$"052A 2A2A 2A2A 4C53 5445"                           /* .*****LSTE */
};

data 'wpos' (128) {
	$"0029 0019 003E 019B 0000 0000 0000 0000"            /* .)...>.›........ */
	$"0000 0000 0000 0661 0000 0000 008E 0000"            /* .......a.....Ž.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000"                                     /* ...... */
};

data 'ZON#' (128) {
	$"FE20 0370 7374 FE5C 0370 6474 FE5C 036D"            /* þ .pstþ\.pdtþ\.m */
	$"7374 FE98 036D 6474 FE98 0363 7374 FED4"            /* stþ˜.mdtþ˜.cstþÔ */
	$"0363 6474 FED4 0365 7374 FF10 0365 6474"            /* .cdtþÔ.estÿ..edt */
	$"0000 0367 6D74 0000 0275 7400 0000 0375"            /* ...gmt...ut....u */
	$"7463 003C 036D 6574 0078 076D 6574 2064"            /* tc.<.met.x.met d */
	$"7374 00B4 0369 6474 030C 036E 7A64 003C"            /* st.´.idt...nzd.< */
	$"0362 7374 01E0 0373 7374 FDA8 0368 7374"            /* .bst.à.sstý¨.hst */
	$"021C 0377 7374 003C 0368 6F65 003C 036D"            /* ...wst.<.hoe.<.m */
	$"657A 003C 0364 6E74 0258 0461 6573 7400"            /* ez.<.dnt.X.aest. */
	$"021C 036B 7374 021C 036A 7374"                      /* ...kst...jst */
};

/* end of unsorted root resources*/


#if 0

resource 'DLOG' (-3999, "put file", purgeable) {
	{0, 0, 184, 304},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	-3999,
	"",
	noAutoCenter
};

resource 'DLOG' (-4000, "get file", purgeable) {
	{0, 0, 200, 348},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	-4000,
	"",
	noAutoCenter
};

resource 'DLOG' (4000, "Get Folder", purgeable) {
	{0, 0, 214, 348},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	4000,
	"",
	noAutoCenter
};

resource 'DLOG' (4001, "get disk", purgeable) {
	{0, 0, 175, 348},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	4001,
	"",
	noAutoCenter
};

#endif

resource 'DLOG' (128, "About", purgeable) {
	{40, 40, 160, 380},
	altDBoxProc,
	invisible,
	noGoAway,
	0x0,
	128,
	"",
	noAutoCenter
};

resource 'DITL' (255, "new value", purgeable) {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{110, 130, 130, 200},
		Button {
			enabled,
			"Zoom"
		},
		/* [2] */
		{110, 10, 130, 80},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{10, 10, 26, 64},
		StaticText {
			disabled,
			"Name:"
		},
		/* [4] */
		{10, 70, 26, 200},
		EditText {
			enabled,
			"???"
		},
		/* [5] */
		{40, 10, 58, 110},
		RadioButton {
			enabled,
			"Table"
		},
		/* [6] */
		{60, 10, 78, 110},
		RadioButton {
			enabled,
			"WP-Text"
		},
		/* [7] */
		{80, 10, 98, 110},
		RadioButton {
			enabled,
			"Picture"
		},
		/* [8] */
		{40, 110, 58, 216},
		RadioButton {
			enabled,
			"Outline"
		},
		/* [9] */
		{60, 110, 78, 216},
		RadioButton {
			enabled,
			"Script"
		},
		/* [10] */
		{80, 110, 98, 216},
		RadioButton {
			enabled,
			"MenuBar"
		}
	}
};

resource 'DITL' (256, "Save?", purgeable) { /* 2005-09-06 creedon - sizes generally match threeway */ 
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{72, 246, 92, 331},
		Button {
			enabled,
			"Yes"
		},
		/* [2] */
		{72, 50, 92, 135},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{72, 148, 92, 233},
		Button {
			enabled,
			"No"
		},
		/* [4] */
		{15, 56, 63, 330},
		StaticText {
			disabled,
			"Save Ò^0Ó before closing?"
		},
		/* [5] */
		{8, 16, 40, 48},
		Icon {
			disabled,
			0
		}
	}
};

resource 'DITL' (257, "Replace?", purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{87, 196, 107, 270},
		Button {
			enabled,
			"Replace"
		},
		/* [2] */
		{10, 61, 74, 270},
		StaticText {
			disabled,
			"An item named Ò^0Ó already exists.  Replace it or create a duplicate?"
		},
		/* [3] */
		{10, 16, 42, 48},
		Icon {
			disabled,
			0
		},
		/* [4] */
		{87, 106, 107, 180},
		Button {
			enabled,
			"Duplicate"
		},
		/* [5] */
		{87, 16, 107, 90},
		Button {
			enabled,
			"Cancel"
		}
	}
};

resource 'DITL' (258, "Menubar", purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{8, 8, 168, 264},
		UserItem {
			enabled
		},
		/* [2] */
		{188, 8, 204, 60},
		UserItem {
			enabled
		},
		/* [3] */
		{170, 8, 186, 264},
		UserItem {
			enabled
		},
		/* [4] */
		{188, 60, 204, 60},
		UserItem {
			enabled
		},
		/* [5] */
		{8, 272, 52, 304},
		UserItem {
			enabled
		}
	}
};

resource 'DITL' (267, "QuickScript", purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{8, 8, 92, 252},
		EditText {
			enabled,
			""
		},
		/* [2] */
		{8, 260, 52, 292},
		UserItem {
			enabled
		}
	}
};

resource 'DITL' (269, "Revert?", purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{80, 180, 104, 246},
		Button {
			enabled,
			"Discard"
		},
		/* [2] */
		{80, 98, 104, 164},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{13, 64, 69, 246},
		StaticText {
			disabled,
			"Discard all changes made to Ò^0Ó?"
		},
		/* [4] */
		{13, 16, 45, 48},
		Icon {
			disabled,
			2
		}
	}
};


/* kw - 2006-02-05 --- missing; taken from carbon DLOG 518 */
resource 'DITL' (512, "Find Dialog", purgeable, preload) {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{91, 340, 111, 433},
		Button {
			enabled,
			"Find"
		},
		/* [2] */
		{158, 11, 178, 81},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{11, 340, 31, 433},
		Button {
			enabled,
			"Replace All"
		},
		/* [4] */
		{40, 340, 60, 433},
		Button {
			enabled,
			"Replace"
		},
		/* [5] */
		{13, 10, 29, 58},
		StaticText {
			disabled,
			"Find:"
		},
		/* [6] */
		{13, 78, 29, 317},
		EditText {
			enabled,
			"Find"
		},
		/* [7] */
		{41, 10, 57, 70},
		StaticText {
			disabled,
			"Replace:"
		},
		/* [8] */
		{41, 78, 57, 317},
		EditText {
			enabled,
			"Replace"
		},
		/* [9] */
		{72, 9, 90, 115},
		CheckBox {
			enabled,
			"Whole words"
		},
		/* [10] */
		{72, 116, 90, 217},
		CheckBox {
			enabled,
			"Ignore case"
		},
		/* [11] */
		{72, 217, 90, 320},
		CheckBox {
			enabled,
			"Wrap around"
		},
		/* [12] */
		{92, 9, 110, 217},
		CheckBox {
			enabled,
			"Use Regular Expressions"
		}
	}
};


/* kw - 2006-02-05 --- personalisation dialog - was in root resourcefork */
resource 'DITL' (515, "Personalisation Dialog", purgeable, preload) {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{254, 168, 274, 228},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{254, 90, 274, 150},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{88, 23, 104, 242},
		EditText {
			disabled,
			""
		},
		/* [4] */
		{144, 23, 160, 242},
		EditText {
			disabled,
			""
		},
		/* [5] */
		{66, 18, 82, 247},
		StaticText {
			disabled,
			"Name:"
		},
		/* [6] */
		{120, 18, 136, 247},
		StaticText {
			disabled,
			"Organization:"
		},
		/* [7] */
		{180, 18, 196, 247},
		StaticText {
			disabled,
			"Initials (4 character max):"
		},
		/* [8] */
		{204, 23, 220, 242},
		EditText {
			disabled,
			""
		},
		/* [9] */
		{20, 20, 60, 243},
		StaticText {
			disabled,
			"Please personalize your copy of UserLan"
			"d Frontier:"
		}
	}
};

/* kw - 2006-02-05 --- table info dialog - was in root resourcefork */
resource 'DITL' (516, "tableinfo", purgeable) {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{156, 253, 180, 333},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{20, 16, 39, 343},
		StaticText {
			enabled,
			"Info for cell ÒsetbarvalueÓ:"
		},
		/* [3] */
		{48, 98, 64, 340},
		StaticText {
			disabled,
			"system.verbs.apps.BarChart"
		},
		/* [4] */
		{68, 98, 84, 340},
		StaticText {
			disabled,
			"March 22, 1991 at 10:02:10AM"
		},
		/* [5] */
		{88, 98, 104, 340},
		StaticText {
			disabled,
			"May 2, 1991 at 5:10PM"
		},
		/* [6] */
		{108, 98, 124, 340},
		StaticText {
			disabled,
			"212K Bytes"
		},
		/* [7] */
		{128, 98, 144, 340},
		StaticText {
			disabled,
			"ÔscptÕ"
		},
		/* [8] */
		{48, 16, 64, 64},
		StaticText {
			enabled,
			"Table:"
		},
		/* [9] */
		{68, 16, 84, 76},
		StaticText {
			enabled,
			"Created:"
		},
		/* [10] */
		{88, 16, 104, 84},
		StaticText {
			enabled,
			"Modified:"
		},
		/* [11] */
		{108, 16, 124, 84},
		StaticText {
			enabled,
			"Size:"
		},
		/* [12] */
		{128, 16, 144, 84},
		StaticText {
			enabled,
			"Type:"
		}
	}
};

/* kw - 2006-02-05 --- reconcile dialog - was in root resourcefork */
resource 'DITL' (25000) {
	{	/* array DITLarray: 8 elements */
		/* [1] */
		{168, 266, 188, 334},
		Button {
			enabled,
			"Reconcile"
		},
		/* [2] */
		{168, 98, 188, 162},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{168, 182, 188, 246},
		Button {
			enabled,
			"Switch"
		},
		/* [4] */
		{42, 70, 82, 337},
		EditText {
			disabled,
			"Programming:Iowa:"
		},
		/* [5] */
		{98, 70, 141, 338},
		EditText {
			disabled,
			"RoverBook:Programming:Iowa:"
		},
		/* [6] */
		{14, 14, 35, 334},
		StaticText {
			disabled,
			"Reconcile:"
		},
		/* [7] */
		{98, 14, 118, 50},
		StaticText {
			disabled,
			"To:"
		},
		/* [8] */
		{42, 14, 62, 57},
		StaticText {
			disabled,
			"From:"
		}
	}
};

/* kw - 2006-02-05 --- personalisation dialog - was in root resourcefork */
/* kw - 2006-02-05 --- personalisation dialog - was in root resourcefork */


#if 0

resource 'DITL' (-3999, "Put File", purgeable) {
	{	/* array DITLarray: 8 elements */
		/* [1] */
		{158, 218, 176, 288},
		Button {
			enabled,
			"Save"
		},
		/* [2] */
		{132, 218, 150, 288},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{136, 14, 152, 197},
		StaticText {
			disabled,
			"Save as:"
		},
		/* [4] */
		{29, 200, 49, 302},
		UserItem {
			disabled
		},
		/* [5] */
		{56, 218, 74, 288},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{82, 218, 100, 288},
		Button {
			enabled,
			"Drive"
		},
		/* [7] */
		{157, 17, 173, 194},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{29, 14, 127, 197},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (-4000, "Get File", purgeable) {
	{	/* array DITLarray: 11 elements */
		/* [1] */
		{163, 255, 181, 335},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{138, 255, 156, 335},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {-6042}
		},
		/* [4] */
		{39, 232, 59, 347},
		UserItem {
			enabled
		},
		/* [5] */
		{68, 256, 86, 336},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{93, 256, 111, 336},
		Button {
			enabled,
			"Desktop"
		},
		/* [7] */
		{39, 12, 185, 230},
		UserItem {
			enabled
		},
		/* [8] */
		{39, 229, 185, 246},
		UserItem {
			enabled
		},
		/* [9] */
		{124, 252, 125, 340},
		Picture {
			disabled,
			11
		},
		/* [10] */
		{1044, 20, 1145, 116},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{196, 12, 212, 342},
		StaticText {
			disabled,
			"Static Text"
		}
	}
};

resource 'DITL' (4000, "Get Folder", purgeable) {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{156, 256, 174, 336},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{1152, 59, 1232, 77},
		Button {
			enabled,
			""
		},
		/* [3] */
		{131, 256, 149, 336},
		Button {
			enabled,
			"Cancel"
		},
		/* [4] */
		{39, 232, 59, 347},
		UserItem {
			disabled
		},
		/* [5] */
		{65, 256, 83, 336},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{90, 256, 108, 336},
		Button {
			enabled,
			"Desktop"
		},
		/* [7] */
		{39, 12, 201, 230},
		UserItem {
			enabled
		},
		/* [8] */
		{39, 229, 201, 246},
		UserItem {
			enabled
		},
		/* [9] */
		{119, 252, 120, 340},
		UserItem {
			disabled
		},
		/* [10] */
		{1044, 20, 1145, 116},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{209, 12, 225, 342},
		StaticText {
			disabled,
			"Static Text"
		},
		/* [12] */
		{181, 256, 199, 336},
		Button {
			enabled,
			"Folder"
		}
	}
};

resource 'DITL' (4001, "get disk", purgeable) {
	{	/* array DITLarray: 11 elements */
		/* [1] */
		{138, 255, 156, 335},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{1172, 59, 1252, 77},
		Button {
			enabled,
			""
		},
		/* [3] */
		{113, 255, 131, 335},
		Button {
			enabled,
			"Cancel"
		},
		/* [4] */
		{39, 232, 59, 347},
		UserItem {
			disabled
		},
		/* [5] */
		{68, 256, 86, 336},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{93, -256, 111, -176},
		Button {
			enabled,
			"Desktop"
		},
		/* [7] */
		{39, 12, 160, 230},
		UserItem {
			enabled
		},
		/* [8] */
		{39, 229, 160, 246},
		UserItem {
			enabled
		},
		/* [9] */
		{99, 252, 100, 340},
		UserItem {
			disabled
		},
		/* [10] */
		{1064, 20, 1165, 116},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{175, 12, 191, 342},
		StaticText {
			disabled,
			"Static Text"
		}
	}
};

#endif

resource 'DITL' (128, "about", purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{23, 50, 55, 82},
		UserItem {
			disabled
		},
		/* [2] */
		{59, 33, 91, 108},
		StaticText {
			disabled,
			"UserLand\nFrontierª"
		},
		/* [3] */
		{22, 125, 52, 328},
		StaticText {
			disabled,
			"Powerful cross-platform web scripting."
		},
		/* [4] */
		{55, 125, 67, 328},
		StaticText {
			disabled,
			"© 1992-2000 UserLand Software, Inc."
		},
		/* [5] */
		{67, 125, 80, 328},
		StaticText {
			enabled,
			"http://www.scripting.com/gimme5/"
		},
		/* [6] */
		{80, 125, 90, 328},
		StaticText {
			disabled,
			"Executing PowerPC code"
		},
		/* [7] */
		{80, 125, 90, 328},
		StaticText {
			disabled,
			"Executing 680x0 code"
		},
		/* [8] */
		{103, 280, 120, 332},
		StaticText {
			disabled,
			"4.0"
		}
	}
};

resource 'WIND' (129, "Document", purgeable) {
	{40, 22, 240, 262},
	zoomDocProc,
	invisible,
	goAway,
	0x0,
	"New Window",
	staggerParentWindowScreen
};

resource 'WIND' (128, "Windoid") {
	{40, 22, 240, 262},	
	2080, /*dmb: use 1986 to use Infinity Windoid*/
	invisible,
	goAway,
	0x0,
	"New Window",
	noAutoCenter
};

resource 'WIND' (130, "Home") {
	{76, 184, 180, 460},
#if TARGET_API_MAC_CARBON
	noGrowDocProc,
#else
	rDocProc,	/* 2004-10-20 aradke: not supported in Carbon */
#endif
	invisible,
	goAway,
	0x0,
	"Home",
	noAutoCenter
};

resource 'WIND' (131, "Little Home") {
	{0, 0, 18, 275},
	altDBoxProc,
	invisible,
	goAway,
	0x10000,
	"Little Home",
	noAutoCenter
};

resource 'WIND' (132, "Moof O'Rama") {
	{40, 40, 72, 72},
	altDBoxProc,
	invisible,
	noGoAway,
	0x0,
	"Moof O'Rama",
	noAutoCenter
};

resource 'WIND' (133, "WindoidCarbon") {
	{40, 22, 240, 262},
	zoomDocProc,
	invisible,
	goAway,
	0x0,
	"New Window",
	noAutoCenter
};

resource 'CNTL' (258, "Vert Baroid", preload) {
	{0, 0, 100, 13},
	0,
	invisible,
	0,
	0,
	scrollBarProc,
	0,
	"scroll bar"
};

resource 'CNTL' (257, "Horiz Bar", preload) {
	{0, 0, 0, 0},
	0,
	visible,
	0,
	0,
	scrollBarProc,
	0,
	"scroll bar"
};

resource 'CNTL' (259, "Horiz Baroid", preload) {
	{0, 0, 13, 100},
	0,
	invisible,
	0,
	0,
	scrollBarProc,
	0,
	"scroll bar"
};

resource 'CNTL' (256, "Vert Bar", preload) {
	{0, 0, 0, 0},
	0,
	visible,
	0,
	0,
	scrollBarProc,
	0,
	"scroll bar"
};

/* kw - 2006-02-05 --- missing; taken from old root */
resource 'CNTL' (25000) {
	{64, 9, 89, 169},
	0,
	visible,
	0,
	25000,
	1008,
	0,
	"Hot List:"
};

resource 'cnfg' (128, "menu bar") {
	nohorizscroll,
	vertscroll,
	dontfloat,
	messagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow, /*dialogwindow*/
	isgrowable,
	createonnew,
	nowindoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onethird,
	#ifdef version42orgreater
		'LAND',
		'FTmb',
	#else
		'BigD',
		'HOME',
	#endif
	129,
	{0, 0, 140, 260},
	4,
	size12,
	plain,
	0,
	{110, 220, 370, 500}
};

#ifdef fliowa

resource 'cnfg' (129, "Iowa") {
	nohorizscroll,
	novertscroll,
	dontfloat,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	nowindoidscrollbars,
	storedindatabase,
	handlesownsave,
	donteraseonresize,
	consumefrontclicks,
	colorwindow,
	onethird,
	'IOWA',
	'CARD',
	129,
	{0, 0, 100, 150},
	2,
	size12,
	plain,
	0,
	{100, 75, 250, 400}
};

#endif

resource 'cnfg' (130, "outline") {
	nohorizscroll,
	vertscroll,
	windowfloats,
	nomessagearea,
	insetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	nowindoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onethird,
	#ifdef version42orgreater
		'LAND',
		'FTop',
	#else
		'BigD',
		'OUTL',
	#endif
	129,
	{0, 0, 100, 150},
	2,
	size12,
	plain,
	301,
	{100, 75, 300, 460}
};

resource 'cnfg' (131, "script") {
	nohorizscroll,
	vertscroll,
	windowfloats,
	messagearea, /*7.0b26 PBS: not used for messages, but needed for lang popup.*/
	insetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	nowindoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onehalf,
	#ifdef version42orgreater
		'LAND',
		'FTsc',
	#else
		'BigD',
		'STRC',
	#endif
	129,
	{0, 0, 100, 150},
	2,
	size9,
	plain,
	300,
	{100, 100, 270, 540}
};

resource 'cnfg' (132, "table") {
	nohorizscroll,
	vertscroll,
	windowfloats,
	messagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	createonnew,
	nowindoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onethird,
	#ifdef version42orgreater
		'LAND',
		'FTtb',
	#else
		'BigD',
		'TABL',
	#endif
	129,
	{0, 0, 100, 260},
	4,
	size12,
	plain,
	0,
	{100, 100, 250, 500}
};

resource 'cnfg' (134, "wpengine") {
	horizscroll,
	vertscroll,
	windowfloats,
	messagearea,
	insetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	nowindoidscrollbars,
	notstoredindatabase,
	parentwindowhandlessave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onethird,
	#ifdef version42orgreater
		'LAND',
		'FTwp',
	#else
		'DBWP',
		'WSWP',
	#endif
	129,
	{0, 0, 100, 150},
	1,
	size12,
	plain,
	0,
	{75, 75, 250, 475}
};

resource 'cnfg' (137, "about") {
	nohorizscroll,
	novertscroll,
	windowfloats,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	openresfile,
	normalwindow,
	isgrowable,
	createonnew,
	nowindoidscrollbars,
	storedindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onehalf,
	'LAND',
	'ABOU',
	129,
	{0, 0, 26, 260},
	4,
	size12,
	plain,
	//302,
	0, /*7.0b50 PBS: the About window has no buttons*/
	{40, 40, 100, 460},
};

resource 'cnfg' (138, "quick script") {
	nohorizscroll,
	novertscroll,
	dontfloat,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	windoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	monochromewindow,
	onehalf,
	'BigD',
	'DLOG',
	128,
	{0, 0, 75, 150},
	4,
	size12,
	plain,
	0,
	{100, 100, 190, 490}
};

resource 'cnfg' (139, "langerror") {
	nohorizscroll,
	novertscroll,
	dontfloat,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	windoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onehalf,
	'BigD',
	'LERR',
	128,
	{100, 25, 170, 200},
	2,
	size9,
	plain,
	0,
	{75, 75, 175, 380}
};

resource 'cnfg' (140, "stats") {
	nohorizscroll,
	novertscroll,
	dontfloat,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	windoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onehalf,
	'BigD',
	'STAT',
#if	TARGET_API_MAC_CARBON
	133,	/* normal window */
#else
	128,
#endif
	{100, 25, 175, 200},
	2,
	size9,
	plain,
	0,
	{100, 150, 350, 350}
};

resource 'cnfg' (141, "PICT") {
	horizscroll,
	vertscroll,
	windowfloats,
	messagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	nowindoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onethird,
	#ifdef version42orgreater
		'LAND',
		'FTpc',
	#else
		'BigD',
		'PICT',
	#endif
	129,
	{0, 0, 100, 150},
	2,
	size12,
	plain,
	0,
	{100, 75, 250, 400}
};

resource 'cnfg' (142, "cancoon") {
	nohorizscroll,
	vertscroll,
	windowfloats,
	messagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	openresfile,
	normalwindow,
	isgrowable,
	createonnew,
	nowindoidscrollbars,
	storedindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onethird,
	'LAND',
	'TABL',
	129,
	{0, 0, 100, 200},
	4,
	size12,
	plain,
	0,
	{100, 100, 250, 500}
};

resource 'cnfg' (143, "lang dialog") {
	nohorizscroll,
	novertscroll,
	dontfloat,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	dialogwindow,
	notgrowable,
	dontcreateonnew,
	nowindoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	donteraseonresize,
	consumefrontclicks,
	colorwindow,
	onehalf,
	'LAND',
	'DLOG',
	0,
	{0, 0, 100, 150},
	4,
	size12,
	plain,
	0,
	{75, 75, 200, 300}
};


resource 'cnfg' (144, "player") { /*7.0b4 PBS: resource for the player window*/
	nohorizscroll,
	novertscroll,
	dontfloat,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	notgrowable,
	createonnew,
	nowindoidscrollbars,
	notstoredindatabase,
	parentwindowhandlessave,
	donteraseonresize,
	consumefrontclicks,
	colorwindow,
	onethird,
	'LAND',
	'PLAY',
	129,
	{0, 0, 100, 150},
	4,
	size12,
	plain,
	0,
	{100, 100, 300, 300},
};

resource 'cnfg' (145, "langerrorcarbon") {
	nohorizscroll,
	novertscroll,
	dontfloat,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	windoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	colorwindow,
	onehalf,
	'BigD',
	'LERR',
	133, /*normal window*/
	{100, 25, 170, 200},
	2,
	size9,
	plain,
	0,
	{75, 75, 175, 380}
};


resource 'cnfg' (146, "quick script carbon") {
	nohorizscroll,
	novertscroll,
	dontfloat,
	nomessagearea,
	dontinsetcontentrect,
	nonewonlaunch,
	dontopenresfile,
	normalwindow,
	isgrowable,
	dontcreateonnew,
	windoidscrollbars,
	notstoredindatabase,
	handlesownsave,
	eraseonresize,
	consumefrontclicks,
	monochromewindow,
	onehalf,
	'BigD',
	'DLOG',
	133, /*normal window*/
	{0, 0, 75, 150},
	4,
	size12,
	plain,
	0,
	{100, 100, 190, 490}
};

resource 'CURS' (132) {
	$"0000 0000 0800 0478 2238 1178 08E8 0440"
	$"0220 1710 1E88 1C44 1E20 0010",
	$"0000 0C00 1E7C 3F7C 7FFC 7FFC 3FFC 1FF0"
	$"0FF8 3FFC 3FFE 3FFE 3EFC 3E78 0030",
	{7, 8}
};

resource 'CURS' (131) {
	$"0000 0000 0180 03C0 07E0 0180 7FFE 0000"
	$"0000 7FFE 0180 07E0 03C0 0180",
	$"0000 0180 03C0 07E0 0FF0 FFFF FFFF FFFF"
	$"FFFF FFFF FFFF 0FF0 07E0 03C0 0180",
	{8, 8}
};

resource 'CURS' (130) {
	$"0000 0240 0240 0240 0240 0A50 1A58 3E7C"
	$"3E7C 1A58 0A50 0240 0240 0240 0240",
	$"07E0 07E0 07E0 07E0 0FF0 1FF8 3FFC 7FFE"
	$"7FFE 3FFC 1FF8 0FF0 07E0 07E0 07E0 07E0",
	{8, 8}
};

resource 'CURS' (257, "beachball #4", locked, preload) {
	$"07C0 1830 2008 701C 783C FC7E FEFE FFFE"
	$"FEFE FC7E 783C 701C 2008 1830 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (258, "beachball #3", locked, preload) {
	$"07C0 19F0 21F8 41FC 41FC 81FE 81FE FFFE"
	$"FF02 FF02 7F04 7F04 3F08 1F30 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (259, "beachball #2", locked, preload) {
	$"07C0 1FF0 3FF8 5FF4 4FE4 87C2 8382 8102"
	$"8382 87C2 4FE4 5FF4 3FF8 1FF0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (260, "beachball #1", locked, preload) {
	$"07C0 1F30 3F08 7F04 7F04 FF02 FF02 FFFE"
	$"81FE 81FE 41FC 41FC 21F8 19F0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (261, "popup", locked, preload) {
	$"0000 4000 6000 7000 7800 7C00 7E00 7F78"
	$"7FF8 7C7F 6C41 467F 0641 037F 0341 007F",
	$"0000 4000 6000 7000 7800 7C00 7E00 7F78"
	$"7FF8 7C7F 6C7F 467F 067F 037F 037F 007F",
	{4, 2}
};

resource 'CURS' (269, "hollow arrow") {
	$"0000 4000 6000 5000 4800 4400 4200 4100"
	$"4380 5400 6C00 4600 0600 0300 03",
	$"C000 E000 F000 F800 FC00 FE00 FF00 FF80"
	$"FFC0 FFE0 FE00 EF00 CF00 8780 0780 0380",
	{1, 1}
};

resource 'CURS' (270, "dragging move cursor") {
	$"C000 A000 5FC0 2AA0 1510 0A08 6408 5404"
	$"2C04 101E 0837 0624 0164 00F8 0060 0020",
	$"C000 E000 7FC0 3FE0 1FF0 0FF8 67F8 77FC"
	$"3FFC 1FFE 0FFF 07FC 01FC 00F8 0060 0020",
	{0, 0}
};

resource 'CURS' (274, "goto") {
	$"0000 4000 6000 7000 7800 7C00 7E00 7F7F"
	$"7FC1 7C7F 6C41 4641 0641 0341 037F",
	$"0000 4000 6000 7000 7800 7C00 7E00 7F7F"
	$"7FFF 7C7F 6C7F 467F 067F 037F 037F",
	{1, 1}
};

/*resource 'ICON' (128, "Frontier") {
	$"00E0 01C0 0110 0220 0113 0410 0114 8410"
	$"0714 8438 0918 8244 F100 8183 8101 0601"
	$"991E C801 A510 3001 A510 0D01 A510 0001"
	$"A310 0001 E011 FC01 A01E 03C1 9010 0031"
	$"8F10 CF0F 8111 4481 8112 8241 8112 8F41"
	$"8112 9041 8112 6041 8112 0C21 8111 1E11"
	$"8112 8609 8111 7005 82E2 AE15 8541 5585"
	$"8A80 0AF9 9500 0151 AA00 0001 FFFF FFFF"
};*/

resource 'ICON' (134, "Shrunken Windoid") {
	$"FFFF FFFE 8000 0003 AAAA AAAB 8000 0003"
	$"AAAA AAAB 8000 0003 AAAA AAAB 8000 0003"
	$"FFFF FFFF 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 FFFF FFFF 7FFF FFFF"
};

resource 'ICON' (135, "Pressed Windoid") {
	$"0000 0000 7FFF FFFF 4000 0001 5555 5555"
	$"4000 0001 5555 5555 4000 0001 5555 5555"
	$"4000 0001 7FFF FFFF 4000 0001 4000 0001"
	$"4000 0001 4000 0001 4000 0001 4002 0001"
	$"4003 0001 4003 8001 4003 C001 4003 E001"
	$"4003 C001 4003 8001 4003 0001 4002 0001"
	$"4000 0001 4000 0001 4000 0001 4000 0001"
	$"4000 0001 4000 0001 4000 0001 7FFF FFFF"
};

resource 'ICON' (138, "Shrunken Windoid With No Text") {
	$"FFFF FFFE 8000 0003 AAAA AAAB 8000 0003"
	$"AAAA AAAB 8000 0003 AAAA AAAB 8000 0003"
	$"FFFF FFFF 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8004 0003 8002 0003"
	$"8005 0003 8002 8003 8005 4003 8002 8003"
	$"8005 0003 8002 0003 8004 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 FFFF FFFF 7FFF FFFF"
};

resource 'ICON' (133, "Shrunken Windoid") {
	$"FFFF FFFE 8000 0003 AAAA AAAB 8000 0003"
	$"AAAA AAAB 8000 0003 AAAA AAAB 8000 0003"
	$"FFFF FFFF 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 FFFF FFFF 7FFF FFFF"
};

resource 'ICON' (137, "Shrunken Windoid With Text") {
	$"FFFF FFFE 8000 0003 AAAA AAAB 8000 0003"
	$"AAAA AAAB 8000 0003 AAAA AAAB 8000 0003"
	$"FFFF FFFF 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 8004 0003 8006 0003"
	$"8007 0003 8007 8003 8007 C003 8007 8003"
	$"8007 0003 8006 0003 8004 0003 8000 0003"
	$"8000 0003 8000 0003 8000 0003 8000 0003"
	$"8000 0003 8000 0003 FFFF FFFF 7FFF FFFF"
};

type 'MCFG' {
	
	hex longint; /*minimum stack space required*/
	
	hex longint; /*minimum heap space to run program*/
	
	hex longint; /*to determine # of master blocks; use zero for none*/
	
	hex longint; /*approx. size of not-yet-loaded CODE resources*/
	
	hex longint = 0; /*reserved*/
	};

resource 'MCFG' (1) {
	
	0x00020000, /*minstacksize*/
	
	0x000A0000, /*minheapsize*/
	
	0x00000040, /*avghandlesize*/
	
	0x00080000  /*reserveforcode*/
};


#ifdef PIKE
#ifndef OPMLEDITOR
type 'PIKE' as 'STR ';

resource 'PIKE' (0, "Owner resource")
#else   //OPMLEDITOR
type 'OPML' as 'STR ';

resource 'OPML' (0, "Owner resource")
#endif // OPMLEDITOR

#else

type 'LAND' as 'STR ';

resource 'LAND' (0, "Owner resource")
#endif
{
	"Copyright © 1992-" copyright_year_string ", UserLand Software, Inc."
};


#ifdef version5orgreater
	resource 'vers' (1) {
	#ifdef PIKE
	#ifndef OPMLEDITOR
		radio_major_version_bcd,
		radio_subminor_version_bcd,
		radio_stage_code,
		radio_revision_level,
		verUS,
		radio_version_string,
	#else
		opml_major_version_bcd,
		opml_subminor_version_bcd,
		opml_stage_code,
		opml_revision_level,
		verUS,
		opml_version_string,
	#endif
	#else
		frontier_major_version_bcd,
		frontier_subminor_version_bcd,
		frontier_stage_code,
		frontier_revision_level,
		verUS,
		frontier_version_string,
	#endif
	#ifdef fltrialsize
		"Trial "
	#endif
	#ifdef PIKE
	#ifndef OPMLEDITOR
		radio_version_string "; © 1992-" copyright_year_string ", UserLand Software, Inc."
	#else  //OPMLEDITOR
	 	opml_version_string "; © 1992-" copyright_year_string ", Scripting News, Inc."
	#endif // OPMLEDITOR
	#else
		frontier_version_string "; © 1992-" copyright_year_string ", UserLand Software, Inc."
	#endif
	};
#else
	resource 'vers' (1) {
		0x4,
		0x10,
		final,
		0x0,
		verUS,
		"4.1",
		"4.1; © 1992-2001, UserLand Software, Inc."
	};
#endif

#ifdef oldTemp

resource 'cicn' (128, purgeable) {
	16,
	{0, 0, 32, 32},
	0,
	unpacked,
	0,
	0x480000,
	0x480000,
	chunky,
	4,
	1,
	4,
	0,
	0,
	4,
	{0, 0, 32, 32},
	4,
	{0, 0, 32, 32},
	$"00E0 01C0 01F0 03E0 01F3 07F0 01F7 87F0"
	$"07F7 87F8 0FFF 83FC FFFF 81FF FFFF 07FF"
	$"FFFF CFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF",
	$"00E0 01C0 0110 0220 0113 0410 0114 8410"
	$"0714 8438 0918 8244 F100 8183 8101 0601"
	$"991E C801 A510 3001 A510 0D01 A510 0001"
	$"A310 0001 E011 FC01 A01E 03C1 9010 0031"
	$"8F10 CF0F 8111 4481 8112 8241 8112 8F41"
	$"8112 9041 8112 6041 8112 0C21 8111 1E11"
	$"8112 8609 8111 7005 82E2 AE15 8541 5585"
	$"8A80 0AF9 9500 0151 AA00 0001 FFFF FFFF",
	0x0,
	0,
	{	/* array ColorSpec: 7 elements */
		/* [1] */
		0, 65535, 65535, 65535,
		/* [2] */
		1, 52428, 39321, 26214,
		/* [3] */
		2, 13107, 39321, 26214,
		/* [4] */
		3, 13107, 26214, 26214,
		/* [5] */
		4, 52428, 52428, 52428,
		/* [6] */
		5, 65535, 65535, 0,
		/* [7] */
		15, 0, 0, 0
	},
	$"0000 0000 FFF0 0000 0000 000F FF00 0000"
	$"0000 000F 233F 0000 0000 00F5 55F0 0000"
	$"0000 000F 233F 00FF 0000 0F55 555F 0000"
	$"0000 000F 233F 0F23 F000 0F55 555F 0000"
	$"0000 0FFF 233F 0F23 F000 0F55 55FF F000"
	$"0000 F44F 233F FF23 F000 00F5 5F44 4F00"
	$"FFFF 444F 2332 2233 F000 000F F444 44FF"
	$"F444 444F 2333 333F 0000 0FF4 4444 444F"
	$"F44F F44F 233F FFF4 FF00 F444 4444 444F"
	$"F4F2 3F4F 233F 4444 44FF 4444 4444 444F"
	$"F4F2 3F4F 233F 4444 4444 FF4F 4444 444F"
	$"F4F2 3F4F 233F 4444 4444 4444 4444 444F"
	$"F4F2 3FFF 233F 4444 4444 4444 4444 444F"
	$"FFF2 3222 333F 444F FFFF FF44 4444 444F"
	$"F1F2 3333 333F FFF1 1111 11FF FF44 444F"
	$"F11F 3333 333F 1111 1111 1111 11FF 444F"
	$"F111 FFFF 233F 1111 FF11 FFFF 1111 FFFF"
	$"F111 111F 233F 111F 0F11 1F00 F111 111F"
	$"F111 111F 233F 11F0 F111 11F0 0F11 111F"
	$"F111 111F 233F 11F0 F111 FFFF 0F11 111F"
	$"F111 111F 233F 11F0 F11F 0000 0F11 111F"
	$"F111 111F 233F 11F0 0FF0 0000 0F11 111F"
	$"F111 111F 233F 11F0 0000 FF00 00F1 111F"
	$"F111 111F 233F 111F 000F FFF0 000F 111F"
	$"F111 111F 233F 11F1 F000 0FF0 0000 F11F"
	$"F111 111F 233F 111F 1FFF 0000 0000 0F1F"
	$"F111 11F1 F3F1 11F1 F1F1 FFF0 000F 0F1F"
	$"F111 1F1F 1F11 111F 1F1F 1F1F F000 0F1F"
	$"F111 F1F1 F111 1111 1111 F1F1 FFFF F11F"
	$"F11F 1F1F 1111 1111 1111 111F 1F1F 111F"
	$"F1F1 F1F1 1111 1111 1111 1111 1111 111F"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
};

#else

resource 'cicn' (128, purgeable) {
	16,
	{0, 0, 32, 32},
	4,
	$"00E0 01C0 01F0 03E0 01F3 07F0 01F7 87F0"
	$"07F7 87F8 0FFF 83FC FFFF 81FF FFFF 07FF"
	$"FFFF CFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF",
	$"00E0 01C0 0110 0220 0113 0410 0114 8410"
	$"0714 8438 0918 8244 F100 8183 8101 0601"
	$"991E C801 A510 3001 A510 0D01 A510 0001"
	$"A310 0001 E011 FC01 A01E 03C1 9010 0031"
	$"8F10 CF0F 8111 4481 8112 8241 8112 8F41"
	$"8112 9041 8112 6041 8112 0C21 8111 1E11"
	$"8112 8609 8111 7005 82E2 AE15 8541 5585"
	$"8A80 0AF9 9500 0151 AA00 0001 FFFF FFFF",
	{	/* array ColorSpec: 7 elements */
		/* [1] */
		0, 65535, 65535, 65535,
		/* [2] */
		1, 52428, 39321, 26214,
		/* [3] */
		2, 13107, 39321, 26214,
		/* [4] */
		3, 13107, 26214, 26214,
		/* [5] */
		4, 52428, 52428, 52428,
		/* [6] */
		5, 65535, 65535, 0,
		/* [7] */
		15, 0, 0, 0
	},
	$"0000 0000 FFF0 0000 0000 000F FF00 0000"
	$"0000 000F 233F 0000 0000 00F5 55F0 0000"
	$"0000 000F 233F 00FF 0000 0F55 555F 0000"
	$"0000 000F 233F 0F23 F000 0F55 555F 0000"
	$"0000 0FFF 233F 0F23 F000 0F55 55FF F000"
	$"0000 F44F 233F FF23 F000 00F5 5F44 4F00"
	$"FFFF 444F 2332 2233 F000 000F F444 44FF"
	$"F444 444F 2333 333F 0000 0FF4 4444 444F"
	$"F44F F44F 233F FFF4 FF00 F444 4444 444F"
	$"F4F2 3F4F 233F 4444 44FF 4444 4444 444F"
	$"F4F2 3F4F 233F 4444 4444 FF4F 4444 444F"
	$"F4F2 3F4F 233F 4444 4444 4444 4444 444F"
	$"F4F2 3FFF 233F 4444 4444 4444 4444 444F"
	$"FFF2 3222 333F 444F FFFF FF44 4444 444F"
	$"F1F2 3333 333F FFF1 1111 11FF FF44 444F"
	$"F11F 3333 333F 1111 1111 1111 11FF 444F"
	$"F111 FFFF 233F 1111 FF11 FFFF 1111 FFFF"
	$"F111 111F 233F 111F 0F11 1F00 F111 111F"
	$"F111 111F 233F 11F0 F111 11F0 0F11 111F"
	$"F111 111F 233F 11F0 F111 FFFF 0F11 111F"
	$"F111 111F 233F 11F0 F11F 0000 0F11 111F"
	$"F111 111F 233F 11F0 0FF0 0000 0F11 111F"
	$"F111 111F 233F 11F0 0000 FF00 00F1 111F"
	$"F111 111F 233F 111F 000F FFF0 000F 111F"
	$"F111 111F 233F 11F1 F000 0FF0 0000 F11F"
	$"F111 111F 233F 111F 1FFF 0000 0000 0F1F"
	$"F111 11F1 F3F1 11F1 F1F1 FFF0 000F 0F1F"
	$"F111 1F1F 1F11 111F 1F1F 1F1F F000 0F1F"
	$"F111 F1F1 F111 1111 1111 F1F1 FFFF F11F"
	$"F11F 1F1F 1111 1111 1111 111F 1F1F 111F"
	$"F1F1 F1F1 1111 1111 1111 1111 1111 111F"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
};

#endif


data 'hfdr' (-5696, purgeable) {
	$"0002 0000 0000 0000 0000 0001 0006 0007"            /* ................ */
	$"0100"                                               /* .. */
};

resource 'STR ' (256) {
	"Frontierª\n\nHigh performance Web content management, object database, system-level and Internet scripting environment, including source code editing and debugging"
};


