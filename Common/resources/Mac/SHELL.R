
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

#ifdef TARGET_API_MAC_CARBON
#include <Carbon/Carbon.r>
#else
#include <MacTypes.r>
#include <AEUserTermTypes.r>
#include <Controls.r>
#include <Dialogs.r>
#include <Icons.r>
#include <Menus.r>
#endif

#include "config.r"
#include "versions.h"

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
#else

resource 'MENU' (2, "File") {
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
		"Close", noIcon, "W", noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Save", noIcon, "S", noMark, plain,
		/* [6] */
		"Save as RunnableÉ", noIcon, noKey, noMark, plain,
		/* [7] */
		"Save a CopyÉ", noIcon, noKey, noMark, plain,
		/* [8] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
//		/* [10] */
//		"Expert Mode", noIcon, noKey, noMark, plain,
//		/* [9] */
//		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Page SetupÉ", noIcon, noKey, noMark, plain,
		/* [11] */
		"PrintÉ", noIcon, "P", noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
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
	{	/* array: 11 elements */
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
		"Close", noIcon, "W", noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Save", noIcon, "S", noMark, plain,
		/* [6] */
		"Save as RunnableÉ", noIcon, noKey, noMark, plain,
		/* [7] */
		"Save a CopyÉ", noIcon, noKey, noMark, plain,
		/* [8] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Page SetupÉ", noIcon, noKey, noMark, plain,
		/* [11] */
		"PrintÉ", noIcon, "P", noMark, plain
	}
};
#endif

#ifdef PIKE /*7.0b26 PBS: Radio UserLand has a different Edit menu.*/

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
		"Open Notepad", noIcon, "Y", noMark, plain,
		/* [17] */
		"Insert Date/Time", noIcon, "4", noMark, plain,
		
	}
};

#else

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
		"Font", noIcon, hierarchicalMenu, "\0d128", plain,
		/* [11] */
		"Size", noIcon, hierarchicalMenu, "\0d130", plain,
		/* [12] */
		"Style", noIcon, hierarchicalMenu, "\0d129", plain,
		/* [13] */
		"Leading", noIcon, hierarchicalMenu, "\0d131", plain,
		/* [14] */
		"Justify", noIcon, hierarchicalMenu, "\0d132", plain
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
		"Bold", noIcon, noKey, noMark, 1,
		/* [4] */
		"Italic", noIcon, noKey, noMark, 2,
		/* [5] */
		"Underline", noIcon, noKey, noMark, 4,
		/* [6] */
		"Outline", noIcon, noKey, noMark, 8,
		/* [7] */
		"Shadow", noIcon, noKey, noMark, 16,
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
		"Find", noIcon, "F", noMark, plain,
		/* [2] */
		"Replace", noIcon, noKey, noMark, plain,
		/* [3] */
		"Find Next", noIcon, "G", noMark, plain,
		/* [4] */
		"Replace & Find Next", noIcon, "H", noMark, plain
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

resource 'STR#' (129, "Defaults", purgeable) {
	{	/* array StringArray: 6 elements */
#ifdef PIKE
		/* [1] */
		"Radio UserLandª", /*PBS 7.0b1: name change to Radio UserLand.*/
#else
		/* [1] */
		"UserLand Frontierª",
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
		/* [6] -- default DB name*/ /*PBS 7.0b1: changed from Pike.root to Radio.root.*/
		"Radio.root",
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
		"Internal error#^0.  Please report to UserLand Technical Support",
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

resource 'DLOG' (256, "Save?", purgeable) {
	{0, 0, 104, 256},
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

resource 'DITL' (256, "Save?", purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{72, 176, 96, 240},
		Button {
			enabled,
			"Yes"
		},
		/* [2] */
		{72, 16, 96, 80},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{72, 96, 96, 160},
		Button {
			enabled,
			"No"
		},
		/* [4] */
		{8, 56, 56, 240},
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
	128,
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

type 'PIKE' as 'STR ';

resource 'PIKE' (0, "Owner resource")

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
		radio_major_version_bcd,
		radio_subminor_version_bcd,
		radio_stage_code,
		radio_revision_level,
		verUS,
		radio_version_string,
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
		radio_version_string "; © 1992-" copyright_year_string ", UserLand Software, Inc."
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
	"UserLand Frontierª\n\nPowerful cross-platform web scripting.\n\nUse Frontier to manage large, dynamic websites with multiple authors."
};


