
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

#define shellprintinclude /*so other includes can tell if we've been loaded*/


typedef struct typrintinfo {
	
	Rect margins; /*the margins we're enforcing to calculate paperrect*/
	
	Rect paperrect;

	long scaleMult;

	long scaleDiv;
	
	short ctpages; /*the number of pages in the document being printed*/
	
	#if MACVERSION

		#if TARGET_API_MAC_CARBON == 1
		GrafPtr	printport;
		PMPrintSession 	printhandle;
		PMPageFormat	pageformat;
		PMPrintSettings	printsettings;
		Rect			pagerect;
		#else
		
		TPPrPort printport;

		THPrint printhandle; /*a handle to the machine's print record*/
		#endif
	#endif

	#ifdef WIN95VERSION
	
		HDC printport;

		PAGESETUPDLG pagesetupinfo;

	#endif
	} typrintinfo;


extern typrintinfo shellprintinfo;


/*prototypes*/

extern boolean shellinitprint (void); /*shellprint.c*/

extern boolean shellpagesetup (void);

extern boolean shellprint (WindowPtr, boolean);

extern boolean iscurrentportprintport (void);

extern boolean isprintingactive (void);

extern boolean getprintscale (long * scaleMult, long * scaleDiv);