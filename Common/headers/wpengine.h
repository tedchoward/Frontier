
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

#define wpengineinclude /*so other includes can tell if we've been loaded*/




#ifndef shellinclude

	#include "shell.h"

#endif


#ifndef dbinclude

	#include "db.h"

#endif

#if !TARGET_CARBON
#if GENERATINGCFM
		typedef UniversalProcPtr TrackClickUPP;
		
		enum {
			uppTrackClickProcInfo = kPascalStackBased
					| RESULT_SIZE(kNoByteCode)
					| STACK_ROUTINE_PARAMETER(1, kFourByteCode)
					| STACK_ROUTINE_PARAMETER(2, kFourByteCode)
		};
		
		#define NewTrackClickProc(userRoutine)		\
				(TrackClickUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppTrackClickProcInfo, GetCurrentArchitecture())
		
		#define CallTrackClickProc(userRoutine, wp, pt)		\
				CallUniversalProc((UniversalProcPtr)(userRoutine), uppTrackClickProcInfo, wp, pt)
	#else
		typedef pascal void (*TrackClickProcPtr) (struct tywprecord **, Point);
		
		typedef TrackClickProcPtr TrackClickUPP;
		
		#define NewTrackClickProc(userRoutine)		\
				((TrackClickUPP) (userRoutine))
		
		#define CallTrackClickProc(userRoutine, wp, pt)		\
				(*(userRoutine))(wp, pt)
#endif
#else //TARGET CARBON Timothy Paustian
		typedef pascal void (*TrackClickProcPtr) (struct tywprecord **, Point);
		
		typedef TrackClickProcPtr TrackClickUPP;
		//this ain't used.
		//#define NewTrackClickProc(userRoutine)		\
		//		((TrackClickUPP) (userRoutine))
		
		#define CallTrackClickProc(userRoutine, wp, pt)		\
				((userRoutine))(wp, pt)
#endif

typedef void (*wpvoidcallback) (void);


#define	wpnoflags		0
#define	wponeline 		0x0001
#define wpneverscroll	0x0002
#define wpalwaysmeasure	0x0004
#define wpprinting		0x0008


typedef unsigned long tywpflags;


typedef struct tywprecord {
	
	void * wpbuffer; /*really as WSHandle; untyped here to avoid includes*/
	
	long startsel, endsel; /*beginning and end of selection*/
	
	tyscrollinfo vertscrollinfo; /*vertical scrollbar info*/
	
	tyscrollinfo horizscrollinfo; /*horiz scrollbar info*/
	
	long ctsaves; /*the number of times this structure has been saved to disk*/
	
	long timelastsave, timecreated; 
	
	dbaddress adr; /*where this edit record came from in the database*/
	
	hdlstring hname; /*the name of this edit record*/
	
	Rect wprect; /*area of window that wp is displayed in*/
	
	Rect windowrect; /*the size and position of window that last displayed wp doc*/
	
	long printpos; /*current character position during printing*/
	
	void * hlastundodata; /*for tricky typing undo*/
	
	short hpixels, vpixels; //in outlines, we keep track of wp dimensions
	
	TrackClickUPP trackclickroutine; /*must be declared pascal void*/
	
	wpvoidcallback dirtyroutine; /*text has changed*/
	
	callback preeditroutine; /*called before every edit*/
	
	wpvoidcallback posteditroutine; /*called after every edit*/
	
	wpvoidcallback setscrollbarsroutine; /*usually links scrollbars to windowinfo*/
	
	long wprefcon; /*for use by application*/
	
	boolean fldirty; /*has the structure been changed since last save?*/
	
	boolean fldirtyview; /*did user change view: scroll, cursor, or window position?*/
	
	boolean fllocked; /*are changes allowed to be made?*/
	
	boolean floneline; /*true if this is a single-line, false otherwise*/
	
	boolean fltextchanged; /*on last edit operation, did text change?*/
	
	boolean flwindowopen; /*is it being displayed in a window?*/
	
	boolean flstartedtyping; /*has user started typing at this position?*/
	
	boolean flshowruler; /*visible ruler?*/
	
	boolean flinhibitdisplay; /*inhibit display? -- not saved to disk*/
	
	boolean flexpandvariables; /*expand variables into text?*/
	
	boolean flhilitevariables; /*show visual indication of variable text?*/
	
	boolean flprinting; /*printing currently in progress?*/
	
	boolean flneverscroll; /*defaults to false; set true if client does all scrolling*/
	
	boolean flcheckvisi; /*so we can defer visiing*/
	
	boolean flalwaysmeasuretext; //keep track of text's width or height?
	
	boolean fleditingnow; //between pre- and postedit?
	} tywprecord, *ptrwprecord, **hdlwprecord;


extern WindowPtr wpwindow;

extern hdlwprecord wpdata;

extern hdlwindowinfo wpwindowinfo;

extern boolean flwperror; /*true if there was an error reported by wp-engine*/


/*function prototypes*/

extern boolean wppushdata (hdlwprecord);

extern boolean wppopdata (void);

extern boolean wpinit (void);

extern void wpshutdown (void);

extern boolean wpgetruler (void);

extern boolean wpsetruler (boolean);

extern hdlwprecord wpnewbuffer (Handle, const Rect *, const Rect *, tywpflags, boolean);

extern boolean wpnewrecord (Rect, hdlwprecord *);

extern void wpsetupwindow (void);

extern boolean wpsetbufferrect (Rect, Rect);

extern boolean wphidecursor (void);

extern boolean wppack (Handle *);

extern boolean wpunpack (Handle, hdlwprecord *);

extern boolean wpscroll (tydirection, boolean, long);

extern void wpresetscrollbars (void);

extern void wpdisposerecord (hdlwprecord);

extern void wpdispose (void);

extern boolean wpsetname (bigstring);

extern boolean wptotalrecalc (void);

extern boolean wpgetdisplay (void);

extern boolean wpsetdisplay (boolean);

extern void wpresize (void);

extern boolean wpkeystroke (void);

extern boolean wpgo (tydirection, long);

extern void wpclick (Point, tyclickflags);

extern void wpactivate (boolean);

extern void wpupdate (void);

extern void wpidle (void);

extern boolean wpadjustcursor (Point);

extern boolean wpundo (void);

extern boolean wpscraphook (Handle);

extern boolean wpcut (void);

extern boolean wpcopy (void);

extern boolean wppaste (boolean);

extern boolean wpclear (void);

extern boolean wpinserthandle (Handle);

extern boolean wpinsert (bigstring);

extern boolean wpsetselection (long, long);

extern boolean wpgetselection (long *, long *);

extern boolean wpgetselrect (Rect *);

extern boolean wpgetselpoint (Point *);

extern boolean wpsetselpoint (Point);

extern boolean wpselectword (void);

extern boolean wpselectline (void);

extern boolean wpselectpara (void);

extern boolean wpselectall (void);

extern boolean wpsetfont (void);

extern boolean wpsetsize (void);

extern boolean wpsetstyle (void);

extern boolean wpsetleading (void);

extern boolean wpsetjustify (void);

extern boolean wpgetmaxpos (long *);

extern boolean wpsetselectioninfo (void);

extern boolean wpgettexthandle (Handle *);

extern boolean wpgettext (bigstring, long *);

extern boolean wpgetseltexthandle (Handle *);

extern boolean wpgetseltext (bigstring);

extern boolean wpsettexthandle (Handle);

extern boolean wpsettext (bigstring);

extern short wpavailwidth (void);

extern boolean wppagesetup (void);

extern boolean wpsetprintinfo (void);

extern boolean wpbeginprint (void);

extern boolean wpendprint (void);

extern boolean wpprint (short);


/*these verbs are for setting formats that aren't supported by tywindowinfo*/

extern boolean wpgetindent (short *);

extern boolean wpsetindent (short);

extern boolean wpgetleftmargin (short *);

extern boolean wpsetleftmargin (short);

extern boolean wpgetrightmargin (short *);

extern boolean wpsetrightmargin (short);

extern boolean wpsettab (short, short, byte);

extern boolean wpcleartabs (void);

/*end non-windowinfo format settings*/


extern boolean wpgetcontentsize (long *, long *);

extern boolean wpmeasuretext (Handle, Rect *, tywpflags);

extern boolean wpdrawtext (Handle, const Rect *, const Rect *, tywpflags);

extern boolean wpsearch (boolean, boolean, boolean, boolean);

extern boolean wpreadfromtextfile (bigstring);

extern boolean wpwritetotextfile (bigstring);


/*HTML wizzy functions.*/

extern boolean wptraversehiddentext (tydirection dir, long *);

extern boolean wptrimhiddentext (void);
