
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef texteditinclude
#define texteditinclude /*so other includes can tell if we've been loaded*/


#ifndef appletdefsinclude

	#include <appletdefs.h>
	
#endif



#define textvertinset 2
#define texthorizinset 2
#define mintextwidth 25


typedef struct tyeditrecord {
	
	TEHandle macbuffer;
	
	boolean flwindowbased; /*set this true if the text is displayed in its own window*/
	
	boolean flscrolled; /*true if a clicking operation resulted in a scroll*/
	
	boolean flselectall; /*on idle, select all the text*/
	
	short vertcurrent, horizcurrent; /*allows app to maintain scrollbars*/
	} tyeditrecord, *ptreditrecord, **hdleditrecord;
	


void editrewrap (hdleditrecord);

boolean editvisiselection (hdleditrecord);

void editgetsize (Handle, short *, short *);

boolean editnewbuffer (Rect, boolean, bigstring, hdleditrecord *);

void editsetrect (hdleditrecord, Rect);

void editdispose (hdleditrecord);
	
void editidle (hdleditrecord);
	
void editautoscroll (hdleditrecord);

void editpaste (hdleditrecord);

void editcut (hdleditrecord);

void editcopy (hdleditrecord);

boolean editgettext (hdleditrecord, bigstring);

void editkeystroke (char, hdleditrecord);

void editclick (Point, boolean, hdleditrecord);

void editcleartextchanged (hdleditrecord);

void editupdate (hdleditrecord hbuffer);

void editupdateport (hdleditrecord, Rect, WindowPtr);

void editactivate (hdleditrecord, boolean);

void editsetwordwrap (hdleditrecord, boolean);

void editselectall (hdleditrecord);

boolean editpointinrect (Point, hdleditrecord);

boolean editreplace (bigstring, hdleditrecord);

void editsetjustification (tyjustification, hdleditrecord);

void editdrawtext (bigstring, Rect, tyjustification);

boolean editgettexthandle (hdleditrecord, Handle *);

void editdrawtexthandle (Handle, Rect, tyjustification);

boolean editnewbufferfromhandle (Rect, boolean, Handle, hdleditrecord *);

boolean editsettexthandle (hdleditrecord, Handle, boolean);

boolean editgettexthandlecopy (hdleditrecord, Handle *);

boolean editreplacehandle (Handle, hdleditrecord);

boolean editgetselectedtexthandle (hdleditrecord, Handle *);

void editgetbuffersize (hdleditrecord, short *, short *);

boolean edithaveselection (hdleditrecord);

boolean editscroll (hdleditrecord, short, short);

void editscrollto (hdleditrecord, short, short);

boolean editgetselection (hdleditrecord, short *, short *);

boolean editsetselection (hdleditrecord, short, short);

void editsetfont (hdleditrecord, short, short);

#endif /*texteditinclude*/

