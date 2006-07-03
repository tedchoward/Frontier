
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


//#ifdef isFrontier
//	#include "quickdraw.h"
//#else
#ifndef appletquickdrawinclude
#define appletquickdrawinclude


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif


extern RGBColor blackcolor, whitecolor, lightbluecolor, lightgraycolor, graycolor, darkgraycolor;

extern RGBColor lightyellowcolor, lightgreencolor, lightpurplecolor, lightredcolor;

extern RGBColor darkredcolor, darkbluecolor, darkgreencolor, mediumgraycolor, darkpurplecolor;



boolean pushstyle (short, short, short);

boolean popstyle (void);

short getmenubarheight (void);

void setrect (Rect *, short , short , short , short);

void eraserect (Rect);

void movepento (short, short);

void pendrawline (short, short);

void invalrect (Rect);

void smashrect (Rect);

void insetrect (Rect *, short, short);

boolean equalpoints (Point, Point);

boolean emptyrect (Rect);

boolean rectlessthan (Rect, Rect);

void dropshadowrect (Rect, short, boolean);

void invaldropshadow (Rect, short);

boolean equalcolors (RGBColor *, RGBColor *);

boolean pushforecolor (const RGBColor *);

boolean popforecolor (void);

boolean pushbackcolor (const RGBColor *);

boolean popbackcolor (void);

boolean pushcolors (const RGBColor *, const RGBColor *);

boolean popcolors (void);

boolean pushgridcolor (RGBColor *);

boolean pushhighlightcolor (RGBColor *);

boolean systemhascolor (void);

boolean getcolorpalette (WindowPtr);

boolean openbitmap (Rect, WindowPtr);

void closebitmap (WindowPtr);

boolean plotsmallicon (Rect, short, short, boolean);

boolean colorenabled (void);
//Code change by Timothy Paustian Friday, May 5, 2000 10:44:55 PM
//Changed to Opaque call for Carbon
//probably a general bug. Carbon enforces consistancy here
//GrafPtr cannot be WindowPtrs
boolean constraintodesktop (WindowPtr, Rect *);

void drawicon (Rect, short, Handle);

//Code change by Timothy Paustian Friday, May 5, 2000 10:44:55 PM
//Changed to Opaque call for Carbon
//probably a general bug. Carbon enforces consistancy here
//GrafPtr cannot be WindowPtrs
void localtoglobalrect (WindowPtr, Rect *);

short pointdist (Point, Point);

boolean pushpen (void);

boolean poppen (void);

boolean pushmacport (GrafPtr);

boolean popmacport (void);

boolean pushclip (Rect);

boolean popclip (void);

void centerrect (Rect *, Rect);

void ellipsize (Str255, short);

#ifndef centerstring
void centerstring (Rect, bigstring);
#endif

void centerwindow (WindowPtr, Rect);

void grayrect (Rect);

boolean pointinrect (Point, Rect);

void validrect (Rect);

boolean pushemptyclip (void);

void globaltolocalpoint (WindowPtr, Point *);

void localtoglobalpoint (WindowPtr, Point *);

void zerorect (Rect *);

boolean equalrects (Rect, Rect);

void scrollrect (Rect, short, short);

boolean oldclutconverter (short, RGBColor *);

#endif

