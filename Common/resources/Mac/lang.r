
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

#ifdef TARGET_API_MAC_CARBON
#include <Carbon/Carbon.r>
#else
#include <MacTypes.r>
#include <Dialogs.r>
#endif

resource 'STR#' (135, "Directions", purgeable) {
	{	/* array StringArray: 13 elements */
		/* [1] */
		"nodirection",
		/* [2] */
		"up",
		/* [3] */
		"down",
		/* [4] */
		"left",
		/* [5] */
		"right",
		/* [6] */
		"flatup",
		/* [7] */
		"flatdown",
		/* [8] */
		"sorted",
		/* [9] */
		"pageup",
		/* [10] */
		"pagedown",
		/* [11] */
		"pageleft",
		/* [12] */
		"pageright",
		/* [13] */
		""
	}
};

#if 0

resource 'STR#' (136, "IAC Toolkit Errors", purgeable) {
	{	/* array StringArray: 7 elements */
		/* [1] */
		"Not enough parameters to evaluate verb message.",
		/* [2] */
		"Incorrect type for verb parameter",
		/* [3] */
		"The verb is undefined",
		/* [4] */
		"The application isn’t set up to handle verb messages",
		/* [5] */
		"Out of memory",
		/* [6] */
		"Operation cancelled by user",
		/* [7] */
		"Can’t perform operation because no file is open"
	}
};

#endif

resource 'STR#' (137, "Stack names", purgeable) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"hash table",
		/* [2] */
		"error info",
		/* [3] */
		"source code",
		/* [4] */
		"locals",
		/* [5] */
		"application"
	}
};

resource 'STR#' (138, "lang UI") {
	{	/* array StringArray: 12 elements */
		/* [1] */
		"  If you intended to call the script, you must include parenthesis after the script name.",
		/* [2] */
		"^0 on “^1”",
		/* [3] */
		"<unavailable>",
		/* [4] */
		"the application whose ID is ",
		/* [5] */
		"appInfo",
		/* [6] */
		"id",
		/* [7] */
		"name",
		/* [8] */
		" on the network",
		/* [9] */
		"",
		/* [10] */
		"There are no statements in this script to ^0.",
		/* [11] */
		"This script has no statements in its body.  To ^0 “^1”, add a statement below the script that calls it with the appropriate parameter list.",
		/* [12] */
		"  If you intended to take its address, use the '@' operator.",
		/* [13] */
		"UserTalk",
		/* [14] */
		"anonymous",
		/* [15] */
		"apple event ['^0', '^1'] from “^2”"
	}
};

resource 'STR#' (139, "System Scripts", locked, preload) {
	{	/* array StringArray: 22 elements */
		/* [1] */
		"edit (@system.misc.menubar)",
		/* [2] */
		"edit (@root)",
		/* [3] */
		"window.quickscript ()",
		/* [4] */
		"system.misc.techsupport ()",
		/* [5] */
		"Frontier.finder2click(\"^0\")",
		/* [6] */
		"Frontier.finderToFront = ^0",
		/* [7] */
		"Frontier.clickers.typeXXXX",
		/* [8] */
		"system.callbacks.control2click(“^0”)",
		/* [9] */
		"if defined(system.callbacks.cmd2click){system.callbacks.cmd2click(“^0”)}else{edit(“^0”)}",
		/* [10] */
		"system.callbacks.option2click(“^0”)",
		/* [11] */
		"if defined(system.callbacks){system.callbacks.openWindow(“^0”);}",
		/* [12] */
		"system.callbacks.saveWindow(“^0”,^1)",
		/* [13] */
		"system.callbacks.closeWindow(“^0”)",
		/* [14] */
		"system.callbacks.compileChangedScript(address(“^0”))",
		/* [15] */
		"system.startup.firsttime||(Frontier.version()<2.1)||defined(nothreads)",
		/* [16] */
		"webBrowser.openURL(“^0”);webBrowser.bringToFront ()",
		/* [17] */
		"user.html.prefs.iso8859map",
		/* [18] */
		"user.prefs.fonts.^0Font",
		/* [19] */
		"user.prefs.expertMode",
		/* [20] */
		"Frontier.setExpertMode(^0)",
		/* [21] */
		"user.prefs.requiredeclarations",
		/* [22] */
		"if defined(system.callbacks){system.callbacks.suspend();}",
		/* [23] */
		"if defined(system.callbacks){system.callbacks.resume();}",
		/* [24] */
		"user.prefs.search.xxx",
		/* [25] */
		"user.prefs.agentsenabled",
		/* [26] */
		"user.prefs.autosave",
		/* [27] */
		"system.temp.Frontier.startingUp",
		/* [28] */
		"user.webserver.prefs.flWaitDuringStartup",
		/* [29] */
		"user.webserver.prefs.flStats",
		/* [30] */
		"user.inetd.shutdown",
		/* [31] */
		"Frontier.tools.windowTypes.isFileMenuItemEnabled(“^0”)",
		/* [32] */
		"pike.getFileMenuItemName(“^0”)",
		/* [33] */
		"Frontier.tools.windowTypes.runFileMenuScript(“^0”)",
		/* [34] */
		"if defined(system.callbacks.opStruct2Click){return(system.callbacks.opStruct2Click())}else{return(false)}",
		/* [35] */
		"if defined(system.callbacks.opReturnKey){return(system.callbacks.opReturnKey())}else{return(false)}",
		/* [36] */
		"if defined(system.callbacks.opExpand){return(system.callbacks.opExpand())}else{return(false)}",
		/* [37] */
		"if defined(system.callbacks.opCollapse){return(system.callbacks.opCollapse())}else{return(false)}",
		/* [38] */
		"if defined(system.callbacks.opCursorMoved){return(system.callbacks.opCursorMoved())}else{return(false)}", /*7.0b6 PBS: opCursorMoved callback*/
		/* [39] */
		"if defined(system.callbacks.opRightClick){return(system.callbacks.opRightClick())}else{return(false)}", /*7.0b11 PBS: opRightClick callback*/
		/* [40] */
		"Frontier.tools.windowTypes.runEditMenuScript(“^0”)",
		/* [41] */
		"Frontier.tools.windowTypes.isFileMenuItemChecked(“^0”)",
		/* [42] */
		"if defined(system.callbacks.opInsert){return(system.callbacks.opInsert())}else{return(false)}",
		/* [43] */
		"if defined(system.callbacks.systemTrayIconRightClick){return(system.callbacks.systemTrayIconRightClick())}else{return(false)}"	
		
	}
};

#ifdef flruntime

resource 'STR#' (141, "alerts", purgeable) {
	{	/* array StringArray: 7 elements */
		/* [1] */
		"The version number of this database file is not recognized by this version of Frontier Runtime.",
		/* [2] */
		"The Runtime.root file must be in the same folder as the Frontier Runtime application.",
		/* [3] */
		"",
		/* [4] */
		"",
		/* [5] */
		"",
		/* [6] */
		"Not enough memory available to run Frontier Runtime.",
		/* [7] */
		"Frontier Runtime requires Macintosh System 7.0 or higher.",
		/* [8] */
		"The Thread Manager is not installed.  Some features will be unavailable."
	}
};

#elif defined (PIKE)

resource 'STR#' (141, "alerts", purgeable) {
	{	/* array StringArray: 7 elements */
		/* [1] */
		"The version number of this database file is not recognized by this version of Pike.",
		/* [2] */
		"That name is too long.  Item names can have up to 255 characters",
		/* [3] */
		"That name is already in use.  Please use a different name",
		/* [4] */
		"A fresh copy of the file could not be opened.",
		/* [5] */
		"Can't paste this Clipboard type here.",
		/* [6] */
		"Not enough memory available to run Pike.",
		/* [7] */
		"Pike cannot run on systems older than System 7.0.",
		/* [8] */
		"The Thread Manager is not installed.  Some features will be unavailable."
	}
};

#else

resource 'STR#' (141, "alerts", purgeable) {
	{	/* array StringArray: 7 elements */
		/* [1] */
		"The version number of this database file is not recognized by this version of Frontier.",
		/* [2] */
		"That name is too long.  Item names can have up to 255 characters",
		/* [3] */
		"That name is already in use.  Please use a different name",
		/* [4] */
		"A fresh copy of the file could not be opened.",
		/* [5] */
		"Can't paste this Clipboard type here.",
		/* [6] */
		"Not enough memory available to run Frontier.",
		/* [7] */
		"Frontier cannot run on systems older than System 7.0.",
		/* [8] */
		"The Thread Manager is not installed.  Some features will be unavailable."
	}
};

#endif

resource 'STR#' (142, "special folder", purgeable) {
	{	/* array StringArray: 11 elements */
		/* [1] */
		"system",
		/* [2] */
		"desktop",
		/* [3] */
		"trash",
		/* [4] */
		"shared",
		/* [5] */
		"printmonitor",
		/* [6] */
		"startup",
		/* [7] */
		"apple",
		/* [8] */
		"control",
		/* [9] */
		"extensions",
		/* [10] */
		"preferences",
		/* [11] */
		"temporary",
		/* [12] */
		"fonts"
	}
};

resource 'STR#' (158, "lang misc", locked, preload) {
	{	/* array StringArray: 9 elements */
		/* [1] */
		"unknown",
		/* [2] */
		"error",
		
		#ifndef flruntime
		
		/* [3] */
		"on disk",
		/* [4] */
		"1 node",
		/* [5] */
		"^0 nodes",
		/* [6] */
		"token = ",
		/* [7] */
		"breakpoint",
		/* [8] */
		"level",
		/* [9] */
		"enum",
		
		#endif
		
		/* [10] */
		"number",
		/* [11] */
		"(none)",
		/* [12] */
		"(nil)",
		/* [13] */
		"nil"
	}
};

resource 'STR#' (256, "DB errors", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"File was created by an incompatible version of this program.",
		/* [2] */
		"Internal database error: attempted to read a free block. Try to Save a Copy and relaunch with the new database.",
		/* [3] */
		"This database has a damaged free list. Use the Save a Copy command to create a new, compacted database.",
		/* [4] */
		"This database has an inconsistent list of free blocks. Use the Save a Copy command to create a new, compacted database.",
		/* [5] */
		"Internal database error: attempted to assign to a free block. Try to Save a Copy and relaunch with the new database.",
		/* [6] */
		"Internal database error: attempted to grow the file beyond the maximum database size.",
		/* [7] */
		"Internal database error: attempted to release a free block.",
		/* [8] */
		"Internal database error: attempted to release an invalid block.",
		/* [9] */
		"Internal database error: attempted to merge with an invalid block."
	}
};


resource 'STR#' (257, "Lang Errors", purgeable) {
	{	/* array StringArray: 98 elements */
		/* [1] */
		"Unknown error.",
		/* [2] */
		"Can’t coerce the string “^0” into a string4 because it isn’t four characters long.",
		/* [3] */
		"Can’t call “^0” because the only script it contains is named “^1”",
		/* [4] */
		"Can’t coerce the string “^0” into a character because it isn’t exactly one character long.",
		/* [5] */
		"Can’t coerce the string “^0” into a number because it contains non-numeric characters.",
		/* [6] */
		"Can’t evaluate the expression because the name “^0” hasn’t been defined.",
		/* [7] */
		"Can’t call “^0” because there aren’t enough parameters.",
		/* [8] */
		"Internal error -- unexpected opcode encountered (^0).",
		/* [9] */
		"Can’t call the script because the name “^0” hasn’t been defined.",
		/* [10] */
		"Can’t delete “^0” because it hasn’t been defined.",
		/* [11] */
		"“^0” is too big a number to convert to a character.  Maximum is 255.",
		/* [12] */
		"“^0” is too big a number to convert to an integer.  Maximum is 32767.",
		/* [13] */
		"“^0” is too small a number to convert to an integer.  Minimum is -32768.",
		/* [14] */
		"String constant isn’t correctly specified.  Must be of the form \"abcd\".",
		/* [15] */
		"Character constant isn’t correctly specified.  Must be of the form 'c'.",
		/* [16] */
		"Can’t compile this script because “^0” is an illegal character.",
		/* [17] */
		"String must be either \"true\" or \"false\".",
		/* [18] */
		"Can’t divide by zero.",
		/* [19] */
		"Can’t compile this script because of a ^0.",
		/* [20] */
		"The expression is too big. Try breaking it up into several statements.",
		/* [21] */
		"An expression is not allowed here; a variable name is required.",
		/* [22] */
		"Can’t create a new local named “^0” because there is already a local with that name.",
		/* [23] */
		"Can’t coerce the value to a 16-bit number.",
		/* [24] */
		"Can’t coerce the value to an 8-bit character.",
		/* [25] */
		"Can’t coerce the value to a 32-bit number.",
		/* [26] */
		"Can’t coerce the value to a date.",
		/* [27] */
		"Can’t coerce the value to a string.",
		/* [28] */
		"Can’t coerce the value to a true or false “Boolean” value.",
		/* [29] */
		"Can’t call “^0” because it isn’t a script.",
		/* [30] */
		"Can’t call “^0” because there are too many parameters.",
		/* [31] */
		"Can’t do replacement because the target window is not in edit mode.",
		/* [32] */
		"Address value doesn’t refer to a valid table.",
		/* [33] */
		"“^0” reported the following error:  “^2”.",
		/* [34] */
		"Can’t send the message because ^1“^0” isn’t running or isn’t IAC-aware.",
		/* [35] */
		"Assignment over existing ^0 object “^1” is not allowed.  Delete the object first, or use table.assign to override protection.",
		/* [36] */
		"Can’t coerce the value to a binary value.",
		/* [37] */
		"Can’t coerce a ^0 value to a ^1.",
		/* [38] */
		"Can't take the negative of this type of value.",
		/* [39] */
		"The logical ‘not’ operation is not supported with this type of value.",
		/* [40] */
		"The “^0” verb only operates on agents.  This script is not running as an agent.",
		/* [41] */
		"A “direction” is required here.  Examples include up, down, left, right and nodirection.",
		/* [42] */
		"The indicated list was not a binary value of the appropriate type.",
		/* [43] */
		"A string4 keyword or a numeric index was expected here.",
		/* [44] */
		"“^0” is too big a number to work with.  Maximum is 2147483647.",
		/* [45] */
		"Couldn’t complete the message to ^1“^0” because of the following error:  “^2”.",
		/* [46] */
		"Can’t find a sub-table named “^0”.",
		/* [47] */
		"Error in kernel call.  The verb  “^0” does not exist, or isn’t set up to handle messages.",
		/* [48] */
		"Array references can only be applied to tables, strings, lists and records.  “^0” is a ^1.",
		/* [49] */
		"Array index is out of range.  The ^1 “^0” doesn’t have an item #^2.",
		/* [50] */
		"Can’t evaluate the array reference because the ^1 “^0” has no item named “^2”.",
		/* [51] */
		"Can’t determine the size of this value.",
		/* [52] */
		"The table index is out of range.  There is no item #^0.",
		/* [53] */
		"A positive number was expected, but a negative number was given.",
		/* [54] */
		"Can’t pack this type of value.",
		/* [55] */
		"Can’t unpack this type of binary value.",
		/* [56] */
		"Can’t unpack.  You can only unpack binary values.",
		/* [57] */
		"Internal error -- can’t pack unknown type of external value.",
		/* [58] */
		"Error encountered unpacking the object.",
		/* [59] */
		"Can’t send the message because the application “^0” isn’t running^1.",
		/* [60] */
		"Can’t coerce “^0” to an address because it doesn't specify a valid object in the database structure.",
		/* [61] */
		"Can’t send any messages because IAC initialization failed.",
		/* [62] */
		"Can’t use “^0” in a with statement because it isn’t a table.",
		/* [63] */
		"Can’t get the address of “^0” because it isn’t in the object database structure.",
		/* [64] */
		"Can’t return a ^0 as the result of a script.^1",
		/* [65] */
		"Can’t pass a ^0 as a parameter to a script.^1",
		/* [66] */
		"Assignment of a ^0 to another value is not allowed.  Use table.assign instead.^1",
		/* [67] */
		"The “^0” verb can only be called by a trap script handling an incoming event.  This script is not handling an event.",
		/* [68] */
		"The “^0” verb can only be called by a dialog item hit callback script.  This script is not handling a dialog item hit.",
		/* [69] */
		"Couldn’t open a dialog with that id (^0).  The DLOG or DITL resource is probably missing.",
		/* [70] */
		"Item number is out of range.  The dialog doesn’t have an item #^0.",
		/* [71] */
		"Can’t nest more than three modal dialogs.",
		/* [72] */
		"This operation is not supported for ^0 values.^1",
		/* [73] */
		"Addition is not supported between values of this type.",
		/* [74] */
		"Subtraction is not supported between values of this type.",
		/* [75] */
		"Multiplication is not supported between values of this type.",
		/* [76] */
		"Division is not supported between values of this type.",
		/* [77] */
		"The “modulus” operation is not supported between values of this type.",
		/* [78] */
		"Comparison is not supported between these two values.",
		/* [79] */
		"Can’t specify more than ^0 tables in a single with statement. Try nesting two or more with statements instead.",
		/* [80] */
		"Stack overflow:  ^0 stack.  If your script is recursive, make sure that it’s terminating.",
		/* [81] */
		"Can’t send this type of value in an IAC message.",
		/* [82] */
		"Can’t run the card “^0”.",
		/* [83] */
		"Can’t coerce the binary value to this type because it isn’t exactly ^0 bytes long.",
		/* [84] */
		"Can’t coerce the string “^0” into a point because it isn’t in the form “h, v”.",
		/* [85] */
		"Can’t coerce the string “^0” into a rectangle because it isn’t in the form “top, left, bottom, right”.",
		/* [86] */
		"Can’t coerce the string “^0” into an RGB because it isn’t in the form “red, green, blue”.",
		/* [87] */
		"Can’t coerce the string “^0” into a pattern because it isn’t in the form of an 8-byte hexidecimal number.",
		/* [88] */
		"The string “^0” isn’t a valid file specification.",
		/* [89] */
		"Can’t coerce the string “^0” into an alias because it isn’t a valid file system path.",
		/* [90] */
		"Random number lower bound is greater than the upper bound.",
		/* [91] */
		"Can’t operate on alias value because the system software in use does not support aliases.",
		/* [92] */
		"Can’t operate on object specifier value because the system software in use does not support Apple events.",
		/* [93] */
		"“^0” is not a supported key format specification.  Valid formats are 'name', 'indx' and 'prop'.",
		/* [94] */
		"Can’t perform the operation because the address of a binary value is required here.",
		/* [95] */
		"Can’t coerce the string “^0” into a floating point number because it isn’t in the form “1.234”.",
		/* [96] */
		"Can’t call “^0” as an XCMD because it isn’t a binary value with binary type 'XCMD' or 'XFCN'.",
		/* [97] */
		"Can’t call “^0” as a UCMD because it isn’t a binary value with binary type 'UCMD'.",
		/* [98] */
		"Can’t run “^0” as a card because it isn’t a binary value with binary type 'CARD'.",
		/* [99] */
		"A range of items can’t be specified here.",
		/* [100] */
		"Can’t interpret this object specification.",
		/* [101] */
		"Can’t open the scripting component whose ID is '^0'.",
		/* [102] */
		"Can’t call “^0” from the background; the clipboard contents are only valid for the frontmost application.",
		/* [103] */
		"Can’t coerce the list value to this type because it doesn’t contain at least ^0 items.",
		/* [104] */
		"A named item can’t be specified here.",
		/* [105] */
		"Can’t call the script “^0” because it doesn’t define a parameter named “^1”",
		/* [106] */
		"Can’t call the script “^0” because the parameter “^1” has already been given a value.",
		/* [107] */
		"Can’t coerce the binary value to this type because its binaryType isn’t '^0'.",
		/* [108] */
		"Can’t perform this operation because the Component Manager is not installed.",
		/* [109] */
		"Bit number must be between 0 and 31.",
		/* [110] */
		"Can’t interpret “^0” as a network volume specification because it isn’t in the form “zone:machine:volume”.",
		/* [111] */
		"Can’t perform the operation because no file is open.",
		/* [112] */
		"Can’t coerce “^0” to a long because it isn’t in the range -2147483648 to 2147483647",
		/* [113] */
		"Semaphore timer expired after ^0 sixtieths of a second.",
		/* [114] */
		"The thread whose ID is ^0 does not exist.",
		/* [115] */
		"Can’t call “^0” because the database “^1” has not been opened.",
		/* [116] */
		"Can’t call “^0” because the database “^1” was opened for reading only.",
		/* [117] */
		"Can’t call the verb “^0”  because it isn't implemented on this platform.",
		/* [118] */
		"Can’t open this file because a database has not been opened.",
		/* [119] */
		"Can’t target “^0” because it doesn’t specify a window.",
		/* [120] */
		"Can’t reference the value because a nil address was given.",
		/* [121] */
		"Can’t load the table “^0” into memory because an error was encountered: “^1”.",
		/* [122] */
		"Can’t call “^0” because the file “^1” has not been opened.",
		/* [123] */
		"Can’t call “^0” because the library “^1” couldn’t be loaded.",
		/* [124] */
		"Can’t call “^0” because the ProcInfo resource could not be found in library “^1”.",
		/* [125] */
		"Can’t call “^0” because that name wasn’t found in the library “^1”.",
		/* [126] */
		"Can’t call “^0” because that name wasn’t found in the ProcInfo resource of library “^1”.",
		/* [127] */
		"Can’t get the “^0” attribute because the table doesn’t have a sub-table named /atts.",
		/* [128] */
		"Can’t get the “^0” attribute because the table doesn’t an attribute with that name.",
		/* [129 */
		"Can’t get the address of “^0” because the table doesn’t have an object with that name.",
		/* [130] */
		"Poorly formed XML text, ^0.",
		/* [131] */
		"Can’t compile the XML text; xml:namespace must have “ns” and “prefix” attributes.",
		/* [132] */
		"Can’t decompile XML because “^0” is not a table.",
		/* [133] */
		"Can’t process the request because a value of type “^0” can’t be represented in XML-Data at this time.",
		/* [134] */
		"Error packing “^0”: ^1",
		/* [135] */
		"Error unpacking “^0”: ^1",
		/* [136] */
		"Error evaluating #^0: ^1.",
		/* [137] */
		"Empty sub-outline in “^0” #define directive.",
		/* [138] */
		"Can’t create item “^0.^1” because “^1” is an illegal name.",
		/* [139] */
		"Can’t save the database because there was an ^0.",
		/* [140] */
		"Can’t rename “^0” as “^1” because an item with that name already exists.",
		/* [141] */
		"Can’t encode “^0” as an IP address because it isn’t in the form “0.0.0.0”.",
		/* [142] */
		"Can’t decode “^0” as an IP address.",
		/* [143] */
		"Can’t ^0 the specified window because it doesn't exist.",
		/* [144] */
		"Can't parse the address because of a syntax error.",
		/* [145] */
		"Can't split the URL because it is not of the form 'http://www.server.com/hello.html'.",
		/* [146] */
		"Can't generate the OPML text because the cloud table is incomplete. “^0” is missing or not recognized.",
		/* [147] */
		"Can't process the outline because the XML cloud element is incomplete.",
		/* [148] */
		"[Bitshift error message missing.]",
		/* [149] */
		"Can’t load library because the ProcInfo resource could not be found in library “^0”.",
		/* [150] */
		"Can't compile regular expression because ^0 at character #^1.",
		/* [151] */
		"Can't call re.^0 because the compiled regular expression is invalid.",
		/* [152] */
		"Can't do re.^0 because an internal regexp error occurred (code ^1).",
		/* [153] */
		"Can't do re.^0 because the replacement string contains a bad group name at character #^1.",
		/* [154] */
		"Can't do re.^0 because the replacement string contains a non-existant group number at character #^1.",
		/* [155] */
		"Can't do re.^0 because the replacement string contains a non-existant group name at character #^1.",
		/* [156] */
		"Can't do re.^0 because the groups list contains a non-existant group name at position #^1.",
		/* [157] */
		"Can't execute command because an internal regexp error occurred (code ^0).",
		/* [158] */
		"Can't execute command because the replacement string contains a bad group name at character #^0.",
		/* [159] */
		"Can't execute command because the replacement string contains a non-existant group number at character #^0.",
		/* [160] */
		"Can't execute command because the replacement string contains a non-existant group name at character #^0."
	}
};

resource 'STR#' (258, "Mac Errors", purgeable) {
	{	/* array StringArray: 87 elements */
		/* [1] */
		"The “^0” folder is full.",
		/* [2] */
		"The disk is full.",
		/* [3] */
		"Couldn’t find the disk volume.",
		/* [4] */
		"There was a media error.",
		/* [5] */
		"The file name “^0” is illegal.",
		/* [6] */
		"The file “^0” is not open.",
		/* [7] */
		"We attempted to read beyond the end of the file.",
		/* [8] */
		"We tried to position to before start of file.",
		/* [9] */
		"Memory is full or information from the file won’t fit in memory.",
		/* [10] */
		"Too many files are open.",
		/* [11] */
		"The file “^0” wasn’t found.",
		/* [12] */
		"The disk is write-protected.",
		/* [13] */
		"The file “^0” is locked.",
		/* [14] */
		"The volume is locked.",
		/* [15] */
		"The file “^0” is busy.",
		/* [16] */
		"A file with the name “^0” already exists.",
		/* [17] */
		"The file “^0” is already open with write permission.",
		/* [18] */
		"Error in user parameter list for file system call.",
		/* [19] */
		"Internal error, refnum error.",
		/* [20] */
		"Internal error, get-file-position error.",
		/* [21] */
		"The volume is not on-line, it was ejected.",
		/* [22] */
		"Internal error, permissions error on file open.",
		/* [23] */
		"Internal error, driver volume already on-line at MountVol.",
		/* [24] */
		"Internal error, no such drive, tried to mount a bad drive number.",
		/* [25] */
		"The diskette is not a Macintosh diskette.  Sorry!",
		/* [26] */
		"The volume belongs to an external file system.",
		/* [27] */
		"Error renaming the file “^0”.",
		/* [28] */
		"Disk error, bad master directory block.",
		/* [29] */
		"Write permissions error.",
		/* [30] */
		"Drive not installed.",
		/* [31] */
		"Read/write requested for an off-line drive.",
		/* [32] */
		"Couldn’t find 5 nibbles in 200 tries.",
		/* [33] */
		"Couldn’t find valid address mark.",
		/* [34] */
		"Read verify compare failed.",
		/* [35] */
		"Address mark checksum didn’t check.",
		/* [36] */
		"Bad address mark bit slip nibbles.",
		/* [37] */
		"Couldn’t find a data mark header.",
		/* [38] */
		"Bad data mark checksum.",
		/* [39] */
		"Bad data mark bit slip nibbles.",
		/* [40] */
		"Write underrun.",
		/* [41] */
		"Step handshake failed.",
		/* [42] */
		"Track 0 detect doesn’t change.",
		/* [43] */
		"Unable to initialize IWM.",
		/* [44] */
		"Tried to read second side on a one-side drive.",
		/* [45] */
		"Unable to correctly adjust disk speed.",
		/* [46] */
		"Track number wrong at address mark.",
		/* [47] */
		"Sector number never found on a track.",
		/* [48] */
		"The folder “^0” wasn’t found.",
		/* [49] */
		"No free WDCB available.",
		/* [50] */
		"Move into offspring error.",
		/* [51] */
		"Wrong volume type, can’t work under MFS.",
		/* [52] */
		"Resource not found.",
		/* [53] */
		"Resource file not found.",
		/* [54] */
		"AddResource failed.",
		/* [55] */
		"RmveResource failed.",
		/* [56] */
		"Attribute does not permit operation.",
		/* [57] */
		"Map does not permit operation.",
		/* [58] */
		"Desk scrap isn’t initialized.",
		/* [59] */
		"Scrap manager noTypeErr.",
		/* [60] */
		"Out of memory.",
		/* [61] */
		"Internal error, master pointer is nil.",
		/* [62] */
		"Internal error, attempted to operate on a free block.",
		/* [63] */
		"Internal error, can’t purge a locked or non-purgable block.",
		/* [64] */
		"Internal error, memory block is locked.",
		/* [65] */
		"Invalid item specification for this list.",
		/* [66] */
		"The list index was out of range.",
		/* [67] */
		"Invalid descriptor list.",
		/* [68] */
		"Operation involving a list item failed.",
		/* [69] */
		"Verb not handled.",
		/* [70] */
		"No reply was received within the timeout period.",
		/* [71] */
		"Waiting was cancelled by the user.",
		/* [72] */
		"The application isn’t running or isn’t IAC-aware.",
		/* [73] */
		"Encountered an unexpected data type.",
		/* [74] */
		"The specified descriptor wasn’t found.",
		/* [75] */
		"Can’t interpret the object specification.",
		/* [76] */
		"The specified object wasn’t found.",
		/* [77] */
		"Access to “^0” was denied.",
		/* [78] */
		"The specified server wasn’t found on the network.",
		/* [79] */
		"Unknown user, or logon is disabled.",
		/* [80] */
		"The password was not accepted by the server.",
		/* [81] */
		"Can’t log on to a server that is running on the same machine.",
		/* [82] */
		"An error was encountered handling the event.",
		/* [83] */
		"The index is too large.",
		/* [84] */
		"Can’t accept the event because a transaction is in progress.",
		/* [85] */
		"That value is not modifiable.",
		/* [86] */
		"A single object must be specified for this operation.",
		/* [87] */
		"A newer version of the Apple Event Manager is required for this operation.",
		/* [88] */
		"An error occurred executing the script.",
		/* [89] */
		"Can’t apply this operation to objects of this type.",
		/* [90] */
		"The connection is no longer valid.",
		/* [90] */
		"Can’t complete the operation because user interaction is not allowed.",
		/* [91] */
		"The source code is not available.",
		/* [92] */
		"Mac OS Error: ^0."
	}
};

#ifdef flruntime

resource 'STR#' (259, "OP Errors", purgeable) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"",
		/* [2] */
		"",
		/* [3] */
		"",
		/* [4] */
		"The address of a script was expected here.",
		/* [5] */
		""
	}
};

resource 'STR#' (265, "Table Verb Errors", purgeable) {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"",
		/* [2] */
		"",
		/* [3] */
		"The address of a table was expected here."
	}
};

resource 'STR#' (266, "Menu Verb Errors", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"",
		/* [2] */
		"The address of a menubar was expected here."
	}
};

#else

resource 'STR#' (259, "OP Errors", purgeable) {
	{	/* array StringArray: 6 elements */
		/* [1] */
		"Can’t do “op.^0” because the target window does not contain an outline.",
		/* [2] */
		"Internal error running “op.^0”.",
		/* [3] */
		"The address of an outline was expected here.",
		/* [4] */
		"The address of a script was expected here.",
		/* [5] */
		"Can’t use “op.^0” on a menubar.  Menubars use outline links for scripts, which can be accessed with “menu.getScript” and “menu.setScript”.",
		/* [6] */
		"Can’t use “op.^0” on a table. Tables use outline links to store information about the object database."
	}
};

#ifdef fliowa

resource 'STR#' (261, "Iowa Errors", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Can’t do “draw.^0” because the target window does not contain a drawing.",
		/* [2] */
		"The address of an drawing was expected here."
	}
};

#endif

resource 'STR#' (264, "WP Verb Errors", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Can’t do “wp.^0” because the target window is not in edit mode, or does not contain editable text.",
		/* [2] */
		"The address of a wptext object was expected here."
	}
};

resource 'STR#' (265, "Table Verb Errors", purgeable) {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Can’t do “table.^0” because the target window does not contain a table.",
		/* [2] */
		"Internal error running “table.^0”.",
		/* [3] */
		"The address of a table was expected here."
	}
};

resource 'STR#' (266, "Menu Verb Errors", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Can’t do “menu.^0” because the target window does not contain a menubar.",
		/* [2] */
		"The address of a menubar was expected here."
	}
};

resource 'STR#' (267, "PICT Errors", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Can’t do “pict.^0” because the target window does not contain a picture.",
		/* [2] */
		"The address of a picture was expected here."
	}
};

#endif

resource 'STR#' (262, "File Errors", purgeable) {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Internal error running “file.^0”."
	}
};

resource 'STR#' (263, "String Errors", purgeable) {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Internal error running “string.^0”."
	}
};

resource 'STR#' (268, "Window Errors", purgeable) {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Internal error running “window.^0”."
	}
};

resource 'DLOG' (259, "notify", purgeable) {
	{0, 0, 107, 375},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	259,
	"",
	noAutoCenter
};

resource 'DLOG' (260, "SF error", purgeable) {
	{0, 0, 100, 223},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	260,
	"",
	noAutoCenter
};

resource 'DLOG' (261, "Alert", purgeable) {
	{0, 0, 123, 375},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	261,
	"",
	noAutoCenter
};

resource 'DLOG' (262, "ask", purgeable) {
	{0, 0, 96, 320},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	262,
	"ask",
	noAutoCenter
};

resource 'DLOG' (263, "twoway", purgeable) {
	{0, 0, 105, 347},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	263,
	"",
	noAutoCenter
};

resource 'DLOG' (264, "threeway", purgeable) {
	{0, 0, 105, 347},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	264,
	"",
	noAutoCenter
};

resource 'DLOG' (265, "getint", purgeable) {
	{0, 0, 70, 192},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	265,
	"",
	noAutoCenter
};

resource 'DLOG' (266, "Char Dialog", purgeable) {
	{0, 0, 69, 180},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	266,
	"",
	centerParentWindowScreen
};

resource 'DLOG' (270, "fileinfo", purgeable) {
	{50, 100, 258, 483},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	270,
	"",
	centerParentWindowScreen
};

resource 'DLOG' (5000, "get file", purgeable) {
	{0, 0, 166, 344},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	5000,
	"",
	noAutoCenter
};

resource 'DLOG' (5001, "put file", purgeable) {
	{0, 0, 188, 344},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	5001,
	"",
	noAutoCenter
};

resource 'DLOG' (5002, "get folder", purgeable) {
	{0, 0, 228, 344},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	5002,
	"",
	noAutoCenter
};

resource 'DLOG' (5003, "get disk", purgeable) {
	{0, 0, 132, 228},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	5003,
	"",
	noAutoCenter
};


resource 'DITL' (259, "Notify", purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{74, 296, 94, 362},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{13, 23, 45, 55},
		Icon {
			disabled,
			1
		},
		/* [3] */
		{13, 78, 61, 362},
		StaticText {
			disabled,
			"Notification message goes here…\n2\n3"
		}
	}
};

resource 'DITL' (260, "SF error", purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{70, 140, 90, 210},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 10, 66, 210},
		StaticText {
			disabled,
			"^0"
		}
	}
};

resource 'DITL' (261, "Alert") {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{90, 296, 110, 362},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{13, 23, 45, 55},
		Icon {
			disabled,
			2
		},
		/* [3] */
		{13, 78, 77, 362},
		StaticText {
			disabled,
			"Alert message goes here…\n2\n3\n4"
		}
	}
};

resource 'DITL' (262, "Ask", purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{64, 242, 84, 306},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{64, 165, 84, 229},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{9, 13, 25, 304},
		StaticText {
			disabled,
			"^0"
		},
		/* [4] */
		{30, 17, 46, 304},
		EditText {
			disabled,
			""
		}
	}
};

resource 'DITL' (263, "twoway", purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{72, 266, 92, 332},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{72, 184, 92, 250},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{15, 16, 63, 334},
		StaticText {
			disabled,
			"prompt goes here\n2\n3"
		}
	}
};

resource 'DITL' (264, "threeway", purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{72, 261, 92, 331},
		Button {
			enabled,
			"Yes"
		},
		/* [2] */
		{72, 178, 92, 248},
		Button {
			enabled,
			"No"
		},
		/* [3] */
		{72, 95, 92, 165},
		Button {
			enabled,
			"Cancel"
		},
		/* [4] */
		{15, 16, 63, 330},
		StaticText {
			disabled,
			"prompt goes here\n2\n3"
		}
	}
};

resource 'DITL' (265, "getint", purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{41, 113, 61, 179},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{41, 8, 61, 74},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{9, 8, 25, 122},
		StaticText {
			disabled,
			"Integer:"
		},
		/* [4] */
		{9, 130, 25, 177},
		EditText {
			disabled,
			""
		}
	}
};

resource 'DITL' (266, "Char Dialog", purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{40, 101, 60, 167},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{40, 8, 60, 74},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 7, 24, 110},
		StaticText {
			disabled,
			"Command Key:"
		},
		/* [4] */
		{8, 148, 24, 164},
		EditText {
			enabled,
			""
		}
	}
};

resource 'DITL' (270, "dialog.fileinfo", purgeable) {
	{	/* array DITLarray: 17 elements */
		/* [1] */
		{16, 13, 32, 75},
		StaticText {
			enabled,
			"Name:"
		},
		/* [2] */
		{54, 13, 70, 61},
		StaticText {
			enabled,
			"Folder:"
		},
		/* [3] */
		{74, 13, 91, 73},
		StaticText {
			enabled,
			"Created:"
		},
		/* [4] */
		{94, 13, 111, 81},
		StaticText {
			enabled,
			"Modified:"
		},
		/* [5] */
		{114, 13, 131, 81},
		StaticText {
			enabled,
			"Size:"
		},
		/* [6] */
		{134, 13, 151, 81},
		StaticText {
			enabled,
			"Version:"
		},
		/* [7] */
		{16, 95, 32, 372},
		StaticText {
			disabled,
			""
		},
		/* [8] */
		{54, 95, 70, 372},
		StaticText {
			disabled,
			""
		},
		/* [9] */
		{74, 95, 90, 372},
		StaticText {
			disabled,
			""
		},
		/* [10] */
		{94, 95, 110, 372},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{114, 95, 130, 372},
		StaticText {
			disabled,
			""
		},
		/* [12] */
		{134, 95, 150, 372},
		StaticText {
			disabled,
			""
		},
		/* [13] */
		{168, 287, 192, 367},
		Button {
			enabled,
			"OK"
		},
		/* [14] */
		{154, 13, 171, 52},
		StaticText {
			enabled,
			"Type:"
		},
		/* [15] */
		{154, 97, 171, 154},
		StaticText {
			disabled,
			""
		},
		/* [16] */
		{174, 14, 192, 71},
		StaticText {
			enabled,
			"Creator:"
		},
		/* [17] */
		{174, 97, 191, 153},
		StaticText {
			disabled,
			""
		}
	}
};

resource 'DITL' (5000, "get file", purgeable) {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{135, 252, 155, 332},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{104, 252, 124, 332},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {
				-6042
			}
		},
		/* [4] */
		{8, 235, 24, 337},
		UserItem {
			enabled
		},
		/* [5] */
		{32, 252, 52, 332},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{60, 252, 80, 332},
		Button {
			enabled,
			"Desktop"
		},
		/* [7] */
		{29, 12, 159, 230},
		UserItem {
			enabled
		},
		/* [8] */
		{6, 12, 25, 230},
		UserItem {
			enabled
		},
		/* [9] */
		{91, 251, 92, 333},
		Picture {
			disabled,
			11
		},
		/* [10] */
		{170, 12, 186, 342},
		StaticText {
			disabled,
			"Prompt"
		}
	}
};

resource 'DITL' (5001, "put file", purgeable) {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{161, 252, 181, 332},
		Button {
			enabled,
			"Save"
		},
		/* [2] */
		{130, 252, 150, 332},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {
				-6043
			}
		},
		/* [4] */
		{8, 235, 24, 337},
		UserItem {
			enabled
		},
		/* [5] */
		{32, 252, 52, 332},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{60, 252, 80, 332},
		Button {
			enabled,
			"Desktop"
		},
		/* [7] */
		{29, 12, 127, 230},
		UserItem {
			enabled
		},
		/* [8] */
		{6, 12, 25, 230},
		UserItem {
			enabled
		},
		/* [9] */
		{119, 250, 120, 334},
		Picture {
			disabled,
			11
		},
		/* [10] */
		{157, 15, 173, 227},
		EditText {
			enabled,
			""
		},
		/* [11] */
		{136, 15, 152, 227},
		StaticText {
			disabled,
			"Save as:"
		},
		/* [12] */
		{88, 252, 108, 332},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (5002, "get folder", purgeable) {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{135, 252, 155, 332},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{104, 252, 124, 332},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {
				-6042
			}
		},
		/* [4] */
		{8, 235, 24, 337},
		UserItem {
			enabled
		},
		/* [5] */
		{32, 252, 52, 332},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{60, 252, 80, 332},
		Button {
			enabled,
			"Desktop"
		},
		/* [7] */
		{29, 12, 190, 230},
		UserItem {
			enabled
		},
		/* [8] */
		{6, 12, 25, 230},
		UserItem {
			enabled
		},
		/* [9] */
		{91, 251, 92, 333},
		Picture {
			disabled,
			11
		},
		/* [10] */
		{201, 12, 217, 342},
		StaticText {
			disabled,
			"Prompt"
		},
		/* [11] */
		{166, 252, 186, 332},
		Button {
			enabled,
			"Folder"
		}
	}
};

resource 'DITL' (5003, "get disk", purgeable) {
	{	/* array DITLarray: 11 elements */
		/* [1] */
		{75, 132, 95, 212},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{44, 132, 64, 212},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {
				-6042
			}
		},
		/* [4] */
		{13, 26, 33, 201},
		UserItem {
			enabled
		},
		/* [5] */
		{44, 15, 64, 95},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{75, 15, 95, 95},
		Button {
			enabled,
			"Drive"
		},
		/* [7] */
		{129, 12, 290, 230},	/*3.0.4b4 dmb*/
		UserItem {
			enabled
		},
		/* [8] */
		{6, 232, 25, 350},	/*3.0.4b4 dmb*/
		UserItem {
			enabled
		},
		/* [9] */
		{44, 113, 97, 114},
		Picture {
			disabled,
			11
		},
		/* [10] */
		{107, 15, 123, 221},
		StaticText {
			disabled,
			"Prompt"
		}
	}
};


