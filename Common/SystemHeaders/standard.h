
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

/*
standard.h -- standard types and constants
*/

#ifndef standardinclude
#define standardinclude /*so other modules can tell that we've been included*/

#ifdef WIN95VERSION
	#include "macconv.h"
#endif

#ifdef MACVERSION  /* 2005-01-09 SMD - for getmilliseconds */
	#include "FastTimes.h"
#endif

#ifdef PACKFLIPPED
	#define conditionallongswap(x) dolongswap(x)
	#define conditionalshortswap(x) doshortswap(x)
	#define conditionalenumswap(x) doshortswap(x)
	#define disklong(x) dolongswap(x)
	#define memlong(x) dolongswap(x)
	#define diskshort(x) doshortswap(x)
	#define memshort(x) doshortswap(x)
	#define disktomemshort(x) shortswap(x)
	#define disktomemlong(x)  longswap(x)
	#define memtodiskshort(x) shortswap(x)
	#define memtodisklong(x) longswap(x)
#else
	#define conditionallongswap(x) x
	#define conditionalshortswap(x) x
	#define conditionalenumswap(x) x
	#define disklong(x) x
	#define memlong(x) x
	#define diskshort(x) x
	#define memshort(x) x
	#define disktomemshort(x)
	#define disktomemlong(x)
	#define memtodiskshort(x)
	#define memtodisklong(x)
#endif

#include "stringdefs.h"		/* embedded string definitions */

#ifndef appletdefsinclude

#define appletdefsinclude

#ifdef MPWC

	#define SystemSevenOrLater 1
	/* A/UX is case sensitive, so use correct case for include file names */
	#include <values.h>
	#include <types.h>
	#include <quickdraw.h>
	#include <fonts.h>
	#include <events.h>
	#include <controls.h>
	#include <windows.h>
	#include <menus.h>
	#include <textedit.h>
	#include <resources.h>
	#include <dialogs.h>
	#include <desk.h>
	#include <scrap.h>
	#include <toolutils.h>
	//MW must add LowMem for certain functions (see notes).
	#include <LowMem.h>
	#include <segload.h>
	#include <memory.h>
	#include <packages.h>

#endif



/*constants*/

#ifndef ctdirections
	
	typedef enum tydirection {
		
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
	
	#define ctdirections 12 /*for arrays indexed on directions*/

#endif

typedef enum tybitdirection {
	
	upbit = 0x01, 
	
	downbit = 0x02, 
	
	leftbit = 0x04, 
	
	rightbit = 0x08
	} tybitdirection;


typedef enum tylinespacing {
	
	singlespaced = 1,
	
	oneandalittlespaced = 2,
	
	oneandaquarterspaced = 3,
	
	oneandahalfspaced = 4,
	
	doublespaced = 5,
	
	triplespaced = 6
	} tylinespacing;
	

typedef enum tyjustification {
	
	leftjustified, 
	
	centerjustified, 
	
	rightjustified,
	
	fulljustified,
	
	unknownjustification
	} tyjustification;
	

#define true 1
#define false 0

#define infinity 32767
#define longinfinity (long)0x7FFFFFFF
#define intinfinity 32767
#define intminusinfinity -32768

#define emptystring (ptrstring) "\0"

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

#ifdef MACVERSION
	#define chcomment			((byte) 0xc7)	/* 'Ç' */
	#define chendcomment		((byte) 0xc8)	/* 'È' */
	#define chopencurlyquote	((byte) 0xd2)	/* 'Ò' */
	#define chclosecurlyquote	((byte) 0xd3)	/* 'Ó' */
	#define chtrademark			((byte) 0xaa)	/* 'ª' */
	#define chnotequals			((byte) 0xad)	/* '­' */
	#define chdivide			((byte) 0xd6)	/* 'Ö' */
#endif

#ifdef WIN95VERSION
	#define chcomment			((byte) 0xab)	/* '«' */
	#define chendcomment 		((byte) 0xbb)
	#define chopencurlyquote 	((byte) 0x93)
	#define chclosecurlyquote 	((byte) 0x94)
	#define chtrademark			((byte) 0x99)
	#define chnotequals			((byte) 0xad)	/* '­' */
	#define chdivide			((byte) 0xf7)	/* '÷' */
#endif


#ifdef WIN95VERSION
typedef unsigned char Str255[258];
#endif

#define lenbigstring 255
   
#define sizegrowicon 15 /*it's square, this is the length of each side*/
 
#define dragscreenmargin 4 /*for dragging windows, leave this many pixels on all sides*/

#define doctitlebarheight 18 /*number of pixels in the title bar of each standard window*/

/*#define nil 0L*/

#define private static /*use "private" when a value or routine should not be seen outside the file it appears in*/


/*types*/

#define bigstring Str255

#ifndef __RPCNDR_H__

	typedef unsigned char boolean;

#endif

typedef unsigned char *ptrstring, **hdlstring;

typedef void * ptrvoid;

typedef char *ptrchar;

typedef short *ptrint;

typedef RgnHandle hdlregion;

typedef const Rect *rectparam;

typedef Rect *ptrrect;

typedef boolean (*callback) (void); /* 2004-10-24 aradke: was ... instead of void on Mac */

#if defined(__RPCNDR_H_VERSION__)
typedef	unsigned char *ptrbyte;	/* 2004-12-29 trt: byte defined by Win32 rpcndr.h */
#else
typedef	unsigned char byte, *ptrbyte;	
#endif


/*macros*/

#undef verify

#ifdef NDEBUG
	#define verify(x)	((void) x)
#else
	#define verify(x)	assert(x)
#endif

#ifdef MACVERSION
	#define sysbeep() SysBeep(1) 
#endif

#ifdef WIN95VERSION
	//#define sysbeep() Beep (800, 300)
	#define sysbeep() MessageBeep(MB_OK)
#endif

#ifndef abs
	#define abs(x) ((x) < 0? -(x) : (x))
#endif

#define odd(x) ((x) & 0x0001)

#define even(x) (!odd (x))

#ifndef max
#define max(x,y) ((x) > (y)? (x) : (y))
#endif

#ifndef min
#define min(x,y) ((x) < (y)? (x) : (y))
#endif

#define sgn(x) ((x) < 0? -1 : ((x) > 0? 1 : 0))

#define loword(x) ((x) & 0x0000ffff)
#define hiword(x) ((x) >> 16)
#define makelong(lo, hi) ((hi) << 16 | (lo))
#define diskwordstomemlong(lo, hi) makelong(conditionalshortswap(lo), conditionalshortswap(hi))
#define memlongtodiskwords(x, lo, hi) do { \
							lo = conditionalshortswap (loword (x)); \
							hi = conditionalshortswap (hiword (x));} while (0)

#ifdef PASCALSTRINGVERSION
#ifdef WIN95VERSION
char * pwstringbaseaddress(char * bs);
void pwsetstringlength(char * bs, short len);
#endif

#define stringbaseaddress(bs) (bs+1)
#define setstringlength(bs,len) (bs[0]=(char)(len))
#define	stringlength(bs) ((unsigned char)(bs)[0])
#define setstringwithchar(ch,bs) {bs[0]=1;bs[1]=(ch);}
#define getstringcharacter(bs,pos) bs[(pos)+1]
#define setstringcharacter(bs,pos,ch) {bs[(pos)+1] = (ch);}
#define stringsize(bs) (stringlength (bs) + 1)
#define lastchar(bs) (bs [stringlength (bs)])
#endif

#ifdef CSTRINGVERSION
#define stringbaseaddress(bs) (bs)
#define	stringlength(bs) (strlen(bs))
#define stringsize(bs) (stringlength (bs) + 1)
#define setstringlength(bs,len) (bs[len]=0)
#define setstringwithchar(ch,bs) {bs[1]=0;bs[0]=ch;}
#define getstringcharacter(bs,pos) bs[pos]
#define setstringcharacter(bs,pos,ch) {bs[pos] = ch;}
#define lastchar(bs) (bs [stringlength (bs)-1])
#endif

#define setemptystring(bs) (setstringlength(bs,0))

#define isemptystring(bs) (stringlength(bs)==0)

#define isemptyrect(r) (((r).bottom <= (r).top) || ((r).right <= (r).left))

#define bitboolean(fl) ((fl)?true:false)

#define ouchreturn {ouch ();return(false);}

#define optiondebug {if(optionkeydown())Debugger();}

#define longsizeof(x) (long)sizeof(x)

#define bundle /**/

#ifdef MACVERSION
#define gettickcount() TickCount()

typedef short hdlfilenum;

typedef Pattern xppattern;
#endif

#ifdef WIN95VERSION
typedef HANDLE hdlfilenum;

typedef HBRUSH xppattern;

#define gettickcount() ((GetTickCount()*3L)/50L)

boolean appendcstring (bigstring dest, char * cstringsource);
boolean buildfilename (unsigned char * pathsource, short pathtype, unsigned char * filesource, short filetype, char * dest);

void NumToString (long theNum, char * theString);
void StringToNum (char * theString, long * theNum);

typedef long double double_t;

typedef struct tydecform
	{
	char style;
	char unused;
	short digits;
	} decform;

#define SIGDIGLEN 36
typedef struct tydecimal
	{
	char sgn;
	char unused;
	short exp;
	struct
		{
		unsigned char length;
		unsigned char text[SIGDIGLEN];
		unsigned char unused;
		} sig;
	} decimal;


typedef struct tyDateTimeRec
	{
	short year;
	short month;
	short day;
	short hour;
	short minute;
	short second;
	short dayOfWeek;
	} DateTimeRec;

typedef union tyLongDateRec
	{
	struct
		{
		short era;
		short year;
		short month;
		short day;
		short hour;
		short minute;
		short second;
		short dayOfWeek;
		short dayOfYear;
		short weekOfYear;
		short pm;
		short res1;
		short res2;
		short res3;
		}	ld;
	short	list[14];
	struct
		{
		short	eraAlt;
		DateTimeRec oldDate;
		}	od;
	} LongDateRec;

typedef struct tyDateCacheRecord
	{
	short	hidden[256];
	} DateCacheRecord;
#endif

#define isnumeric(x) ((x >= '0') && (x <= '9'))

#define quickdrawglobal(x) qd.x

#endif

#endif



