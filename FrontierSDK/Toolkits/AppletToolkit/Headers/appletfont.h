
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifdef isFrontier
	#include "font.h"
#else
	#define fontinclude /*so other includes can tell if we've been loaded*/


extern FontInfo globalfontinfo;

#define diskfontnamelength 32 /*number of bytes for a font name stored on disk*/

typedef char diskfontstring [diskfontnamelength + 1];



short sumcharwidths (void *);

void setglobalfontsizestyle (short, short, short);

void fontgetname (short, bigstring);

void fontgetnumber (bigstring, short *);

void diskgetfontnum (diskfontstring, short *);

void diskgetfontname (short, diskfontstring);

short setnamedfont (bigstring, short, short, short);

void getfontsizestyle (short *, short *, short *);

void getstyle (short, short *, short *, short *, short *, short *, short *);

void checkstyle (boolean, bigstring, bigstring);

void getnextfont (short *);

void getprevfont (short *);

void getnextstyle (short *);

void getprevstyle (short *);

void setfontsizestyle (short, short, short);

short getfontheight (void);	

#endif