
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

	#define moofsmalliconlist 142



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


