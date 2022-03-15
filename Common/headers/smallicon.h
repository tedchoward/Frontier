
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

#ifndef smalliconinclude
#define smalliconinclude


#define widthsmallicon 16 /*dimensions of a SICN resource*/
#define heightsmallicon 16

#define miscsmalliconlist 131 /*miscellaneous small icons*/
#define lockicon 0
#define checkedicon 1
#define upflagicon 2
#define downflagicon 3
#define sideflagicon 4
#define closeboxicon 5
#define selectedcloseboxicon 6
#define stophereicon 7
#define blackpopupicon 8
#define graypopupicon 9

#ifdef version42orgreater
	#define moofsmalliconlist 142

#else
	#define xxxdraggingsmalliconlist 130 /*small icons for dragging something*/
	#define rightarrowicon 0
	#define leftarrowicon 1
	#define downarrowicon 2
	#define uparrowicon 3

	#define xxxfindersmalliconlist 128 /*small icons for the UserLand finder*/
	#define foldericon 0
	#define filledfoldericon 1
	#define fileicon 2
	#define filledfileicon 3
	#define programicon 4
	#define filledprogramicon 5

	#define xxxleadersmalliconlist 128 /*small icons for the structured display*/
	#define fullicon 0
	#define emptyicon 1
	#define linkedicon 2
	#define commenticon 3
	#define fullcommenticon 4
	#define whiteboxicon 5
	#define grayboxicon 6
	#define whitediamondicon 7
	#define tablefullicon 8
	#define tableemptyicon 9
	#define tablefullselicon 10
	#define tableemptyselicon 11
	#define whiteclosedfoldericon 12
	#define blackclosedfoldericon 13
	#define whiteopenfoldericon 14
	#define blackopenfoldericon 15

	#define xxxcirclesmalliconlist 132 /*different shades of circles*/
	#define whitecircleicon 0
	#define blackcircleicon 1
	#define graycircleicon 2

	#define xxxtablesmalliconlist 134 /*icons that appear in front of table entries*/
	#define scalaricon 0
	#define tableicon 1
	#define wordicon 2
	#define picticon 3
	#define outlineicon 4
	#define scripticon 5
	#define menuicon 6
	#define unknownicon 7
#endif


typedef enum tyfillstate {
	
	filledwithblack, 
	
	filledwithgray, 
	
	filledwithwhite
	} tyfillstate;


typedef char smalliconbits [32];

typedef smalliconbits *ptrsmalliconbits, **hdlsmalliconbits;

#pragma pack(2)
typedef struct tysmalliconspec { /*bundle all the plotting parameters into a record*/
	
	hdlsmalliconbits hbits; /*if nil, use iconlist*/
	
	short iconlist;
	
	short iconnum;
	
	WindowPtr iconwindow;
	
	Rect iconrect;
	
	boolean flinverted: 1;
	
	boolean flclearwhatsthere: 1;
	} tysmalliconspec;
#pragma options align=reset

/*prototypes*/

extern boolean displaypopupicon (Rect, boolean);

extern boolean displayleadericon (Rect, tyfillstate);

extern boolean plotsmallicon (tysmalliconspec);

extern boolean loadsmallicon (short, hdlsmalliconbits *);

extern boolean initsmallicons (void);

extern boolean myMoof (short, long);

#endif


