
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

#define searchinclude /*so other includes can tell if we've been loaded*/




/*global search/sort settings*/

typedef struct tysearchparameters {
	
	boolean flfirsttime: 1;
	
	boolean flunicase: 1;
	
	boolean flwraparound: 1;
	
	boolean flwholewords: 1;
	
	boolean flonelevel: 1;
	
	boolean flonetype: 1;
	
	boolean floneobject: 1;
	
	boolean flclosebehind: 1;
	
	boolean flfindall: 1;
	
	boolean flreplaceall: 1;
	
	boolean flregexp: 1;
	
	boolean flzoomfound: 1;
	
	boolean flwindowzoomed: 1;
	
	bigstring bsorigfind; /*as entered by the user*/
	
	bigstring bsorigreplace; /*as entered by the user*/
	
	bigstring bsfind; /*corrected for case-sensitivity etc.*/
	
	bigstring bsreplace; /*depends on current match for regexps*/
	
	short ctfound;
	
	short ctreplaced;
	
	long searchrefcon;

#ifdef flregexpverbs
	
	Handle hcompiledpattern;
	
	Handle hovector;

#endif
	} tysearchparameters;


extern tysearchparameters searchparams; /*set this global to influence searching process*/



/*prototypes*/

extern boolean isword (byte *, long, long, long); /*search.c*/

extern boolean textsearch (byte *, long, long *, long *);

extern boolean handlesearch (Handle, long *, long *);

extern boolean stringsearch (bigstring, short *, short *);

// extern void getsearchstring (bigstring);

extern void startnewsearch (boolean, boolean);

extern boolean startingtosearch (long);

extern boolean searchshouldwrap (long);

extern boolean searchshouldcontinue (long);

extern void endcurrentsearch (void);

extern boolean initsearch (void);


extern boolean getsearchparams (void); /*shellverbs.c*/

extern boolean setsearchparams (void);




