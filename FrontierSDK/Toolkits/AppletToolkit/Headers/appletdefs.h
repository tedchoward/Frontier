
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef appletdefsinclude
#define appletdefsinclude /*so other modules can tell if we've been included*/

#define macintosh

#undef windows

#ifdef macintosh
	
	#define geneva kFontIDGeneva
	
	#ifdef MPWC
		
		#include <types.h>
		#include <resources.h>
		#include <dialogs.h>
		#include <menus.h>
		#include <fonts.h>
		#include <events.h>
		#include <controls.h>
		#include <segload.h>
		#include <memory.h>
		#include <toolutils.h>
		#include <packages.h>
		#include <sound.h>
		#include <files.h>
		#include <desk.h>
	
	#endif
	
	#define widthsmallicon 16 /*dimensions of a SICN resource*/
	#define heightsmallicon 16
	
	typedef ControlHandle hdlscrollbar;
	
	#ifdef coderesource
	
		#ifdef haveA5quickdrawglobals
		
			extern QDGlobals *a5quickdrawglobals;
		
		#else
			
			#define GetQDVarsPtr()	((char *)*LMGetCurrentA5())
		
			#define a5quickdrawglobals ((QDGlobals *) (GetQDVarsPtr () - (sizeof (QDGlobals) - sizeof (GrafPtr))))
		
		#endif
		
		#define quickdrawglobal(x) (*a5quickdrawglobals).x
		
	#else
	
		#define quickdrawglobal(x) qd.x
	
	#endif
	
	#define sizegrowicon 15 /*it's square, this is the length of each side*/
	 
	#define dragscreenmargin 4 /*for dragging windows, leave this many pixels on all sides*/
	
	#define doctitlebarheight 18 /*number of pixels in the title bar of each standard window*/

	#define lenbigstring 255
	   
	#define OsType OSType
	
	#define bigstring Str255
	
	typedef unsigned short boolean;
	
	typedef unsigned char *ptrstring, **hdlstring;
	
	#define sysbeep SysBeep(1) 
	
	#define	stringlength(bs) ((bs)[0])
	
	#define setstringlength(bs,len) (bs[0]=(char)len)
	
	#define setstringwithchar(ch,bs) {bs[0]=1;bs[1]=ch;}
	
	#define setemptystring(bs) (setstringlength(bs,0))
	
	#define isemptystring(bs) (stringlength(bs)==0)
	
	#define isemptyrect(r) EmptyRect (&r)
	
	#include <Printing.h>
	
#endif


#ifdef fldebug
	short __assert(char *, char *, short);
	#define assert(x)	((void) ((x) ? 0 : __assert(#x, __FILE__, __LINE__)))
#else
	#define assert(x)	((void) 0)
#endif



#define true 1
#define false 0

#define infinity 32767
#define longinfinity (long)0x7FFFFFFF
#define intinfinity 32767
#define intminusinfinity -32766

#define emptystring (ptrstring) "\p"

#define chnul			((char) 0)
#define chbacktab		((char) 0)
#define chhome 			((char) 1)
#define chenter			((char) 3)
#define chend 			((char) 4)
#define chhelp 			((char) 5)
#define chbackspace		((char) 8) 
#define chtab 			((char) 9)
#define chlinefeed		((char) 10)
#define chpageup 		((char) 11)
#define chpagedown 		((char) 12)
#define chreturn		((char) 13)
#define chescape		((char) 27)
#define chrightarrow 	((char) 29)
#define chleftarrow 	((char) 28)
#define chuparrow 		((char) 30)
#define chdownarrow 	((char) 31)
#define chsinglequote 	((char) 39)
#define chdoublequote 	((char) 34)
#define chspace			((char) 32)
#define chdelete 		((char) 127)


typedef struct tydaterecord { 
	
	short day, month, year;
	
	short hour, minute, second;
	} tydaterecord;

 
typedef enum tydirection { /*the possible values for a Toolkit direction parameter*/
	
	nodirection = 0, 
	
	up = 1, 
	
	down = 2, 
	
	left = 3,
	
	right = 4, 
	
	flatup = 5, 
	
	flatdown = 6, 
	
	sorted = 8,
	
	pageup = 9,
	
	pagedown = 10,
	
	pageleft = 11,
	
	pageright = 12
	} tydirection;


#define ctdirections 12 /*the number of directions, for looping and arrays*/


typedef enum tyjustification {
	
	leftjustified, 
	
	centerjustified, 
	
	rightjustified,
	
	fulljustified,
	
	unknownjustification
	} tyjustification;
	
	
typedef enum tylinespacing {
	
	singlespaced = 1,
	
	oneandalittlespaced = 2,
	
	oneandaquarterspaced = 3,
	
	oneandahalfspaced = 4,
	
	doublespaced = 5,
	
	triplespaced = 6
	} tylinespacing;


typedef void *ptrvoid;

typedef char *ptrchar;

typedef short *ptrint;

typedef boolean (*callback) (void);

typedef	unsigned char byte, *ptrbyte;	

#define mod %

#define div /

#define abs(x) ((x) < 0? -(x) : (x))

#define odd(x) ((x) % 2)

#define even(x) (!((x) % 2))

#define max(x,y) ((x) > (y)? (x) : (y))

#define min(x,y) ((x) < (y)? (x) : (y))

#define bitboolean(fl) ((fl)?true:false)

#define isnumeric(x) ((x >= '0') && (x <= '9'))

#define longsizeof(x) (long)sizeof(x)

#endif
