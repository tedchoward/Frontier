
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

#define shellrsrcinclude


#define defaultlistnumber 129 /*resource number for program defaults STR# resource*/
#define programname 1
#define untitledfilename 2
#define defaultfilename 3
#define startupfileprompt 4
#define preferencesfilename 5
#define startupfilename 6
#define nonstartupfileprompt 7

#define interfacelistnumber 130 /*misc strings that appear in UI of program*/

#if flruntime
	#define timedateseperatorstring 1
	#define cancelbuttonstring 2
	#define kilobytestring 3
	#define commastring 4
#else
	#define scriptbuttonstring 1
	#define zoombuttonstring 2
	#define trychooserstring 3
	#define cmdkeypopupstring 4
	#define setcmdkeyitemstring 5
	#define cmdkeypromptstring 6
	#define internalerrorstring 7
	#define timedateseperatorstring 8
	#define cancelbuttonstring 9
	#define scriptforstring 10
	#define customsizestring 11
	#define kilobytestring 12
	#define customleadingstring 13
	#define commastring 14
	#define outofmemorystring 15
	#define saveitemstring 16
	#define savedatabaseitemstring 17
	#define saveasitemstring 18
	#define saveacopyitemstring 19
	#define openolddatabasestring 20
#endif

#ifdef fltrialsize
	#define trialsizelistnumber 400
#endif

#define langerrorlistnumber 131 /*strings used by langerror.c, the Error Info window*/
#define scripticonstring 1
#define langerrortitlestring 2
#define errorlocationstring 3

#define fontnamelistnumber 132 /*font names stored in a STR# list*/

#define commandlistnumber 133 /*strings used by command.c, the QuickScript window*/
#define runiconstring 1
#define commandtitlestring 2

#define undolistnumber 134 /*operations that can be undone, in string format*/
#define cantundoitem 1
#define undostring 2
#define redostring 3
#define undocutstring 4
#define undocopystring 5
#define undopastestring 6
#define undoclearstring 7
#define undotypingstring 8
#define undomovestring 9
#define undosortstring 10
#define undopromotestring 11
#define undodemotestring 12
#define undodeletionstring 13
#define undoformatstring 14

#define directionlistnumber 135 /*the strings for up, down, left, right, etc.*/

#define alertstringlistnumber 141 /*strings that go into alerts*/
#define baddatabaseversionstring 1
#if flruntime
	#define databasenotfoundstring 2
#else
	#define itemnametoolongstring 2
	#define itemnameinusestring 3
	#define reopenerrorstring 4
	#define cantpasteherestring 5
#endif
#define notenoughmemorystring 6
#define archaicsystemstring 7
#define needthreadmanagerstring 8

#define specialfolderlistnumber 142

#define idvertbar 256 /*standard horizontal/vertical scrollbars*/
#define idhorizbar 257

#define idvertbaroid 258 /*"windoid" versions of horizontal/vertical scrollbars*/
#define idhorizbaroid 259



