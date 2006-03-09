
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

#ifndef memoryinclude /*so other includes can tell if we've been loaded*/
#define memoryinclude /*so other includes can tell if we've been loaded*/


#ifndef shelltypesinclude

	#include "shelltypes.h"

#endif


typedef struct handlestream {
	
	/*
	5.0.2 dmb: new structure for stream-like operations on handle-based data
	*/
	
	Handle data;
	long pos;
	long eof;
	long size;
	} handlestream, *ptrhandlestream;


#define isemptyhandle(h) (gethandlesize(h)==0)


#if (MEMTRACKER != 1)
	/*function prototypes*/
	extern boolean newhandle (long, Handle *);

	extern boolean newemptyhandle (Handle *);

	extern void disposehandle (Handle);

	extern boolean newclearhandle (long, Handle *);

	extern boolean newfilledhandle (ptrvoid, long, Handle *);

	#ifdef flnewfeatures
		extern boolean newgrowinghandle (long, Handle *);
	#endif

	extern boolean copyhandle (Handle, Handle *);

	extern boolean newtexthandle (const bigstring, Handle *);

	extern boolean newheapstring (const bigstring, hdlstring *);

	extern boolean concathandles (Handle, Handle, Handle *);

	extern boolean mergehandles (Handle, Handle, Handle *);

	extern boolean unmergehandles (Handle, Handle *, Handle *);

	extern boolean newintarray (short, hdlintarray *);

	extern boolean loadfromhandletohandle (Handle, long *, long, boolean, Handle *);

	extern boolean loadhandleremains  (long, Handle, Handle *);

#else

	#ifdef MACVERSION
		#define GetCurrentThreadId() (0L)
	#endif

	#define newhandle(sz, ph) debugnewhandle(__FILE__, __LINE__, GetCurrentThreadId(), sz, ph)

	#define newemptyhandle(ph) debugnewemptyhandle(__FILE__, __LINE__, GetCurrentThreadId(), ph)

//	#define disposehandle(h) debugdisposehandle(__FILE__, __LINE__, GetCurrentThreadId(), h)

	#define newclearhandle(sz, ph) debugnewclearhandle(__FILE__, __LINE__, GetCurrentThreadId(), sz, ph)

	#define newfilledhandle(ptr, sz, ph) debugnewfilledhandle(__FILE__, __LINE__, GetCurrentThreadId(), ptr, sz, ph)

	#ifdef flnewfeatures
		#define newgrowinghandle(sz, ph) debugnewgrowinghandle(__FILE__, __LINE__, GetCurrentThreadId(), sz, ph)
	#endif

	#define copyhandle(h, ph) debugcopyhandle(__FILE__, __LINE__, GetCurrentThreadId(), h, ph)

	#define newtexthandle(bs, ph) debugnewtexthandle(__FILE__, __LINE__, GetCurrentThreadId(), bs, ph)

	#define newheapstring(bs, ph) debugnewheapstring(__FILE__, __LINE__, GetCurrentThreadId(), bs, ph)

	#define concathandles(h1, h2, ph) debugconcathandles(__FILE__, __LINE__, GetCurrentThreadId(), h1, h2, ph)

	#define mergehandles(h1, h2, ph) debugmergehandles(__FILE__, __LINE__, GetCurrentThreadId(), h1, h2, ph)

	#define unmergehandles(h, ph1, ph2) debugunmergehandles(__FILE__, __LINE__, GetCurrentThreadId(), h, ph1, ph2)

	#define newintarray(sz, ph) debugnewintarray(__FILE__, __LINE__, GetCurrentThreadId(), sz, ph)

	#define loadfromhandletohandle(h, posptr, sz, fl, ph) debugloadfromhandletohandle(__FILE__, __LINE__, GetCurrentThreadId(), h, posptr, sz, fl, ph)

	#define loadhandleremains(sz, h, ph) debugloadhandleremains(__FILE__, __LINE__, GetCurrentThreadId(), sz, h, ph)


	extern boolean debugnewhandle (char * filename, unsigned long linenumber, unsigned long threadid, long, Handle *);

	extern boolean debugnewemptyhandle (char * filename, unsigned long linenumber, unsigned long threadid, Handle *);

	extern void disposehandle (Handle);

	extern boolean debugnewclearhandle (char * filename, unsigned long linenumber, unsigned long threadid, long, Handle *);

	extern boolean debugnewfilledhandle (char * filename, unsigned long linenumber, unsigned long threadid, ptrvoid, long, Handle *);

	#ifdef flnewfeatures
		extern boolean debugnewgrowinghandle (char * filename, unsigned long linenumber, unsigned long threadid, long, Handle *);
	#endif

	extern boolean debugcopyhandle (char * filename, unsigned long linenumber, unsigned long threadid, Handle, Handle *);

	extern boolean debugnewtexthandle (char * filename, unsigned long linenumber, unsigned long threadid, const bigstring, Handle *);

	extern boolean debugnewheapstring (char * filename, unsigned long linenumber, unsigned long threadid, const bigstring, hdlstring *);

	extern boolean debugconcathandles (char * filename, unsigned long linenumber, unsigned long threadid, Handle, Handle, Handle *);

	extern boolean debugmergehandles (char * filename, unsigned long linenumber, unsigned long threadid, Handle, Handle, Handle *);

	extern boolean debugunmergehandles (char * filename, unsigned long linenumber, unsigned long threadid, Handle, Handle *, Handle *);

	extern boolean debugnewintarray (char * filename, unsigned long linenumber, unsigned long threadid, short, hdlintarray *);

	extern boolean debugloadfromhandletohandle (char * filename, unsigned long linenumber, unsigned long threadid, Handle, long *, long, boolean, Handle *);

	extern boolean debugloadhandleremains  (char * filename, unsigned long linenumber, unsigned long threadid, long, Handle, Handle *);
#endif


extern boolean analyzeheap (void);

extern boolean haveheapspace  (long);

extern boolean testheapspace (long);

extern void lockhandle (Handle);

extern void unlockhandle (Handle);

extern boolean validhandle (Handle);

extern long gethandlesize (Handle);

extern boolean sethandlesize (Handle, long);

extern boolean minhandlesize (Handle, long);

extern void moveleft (ptrvoid, ptrvoid, long);

extern void moveright (ptrvoid, ptrvoid, long);

extern void fillchar (ptrvoid, long, char);

extern void clearbytes (ptrvoid, long);

extern void clearhandle (Handle);

extern void movefromhandle (Handle, ptrvoid, long);

#ifdef flnewfeatures

extern boolean prepareforgrowing (Handle);

#endif

extern short comparehandles (Handle, Handle);

extern boolean equalhandles (Handle, Handle);

extern long searchhandle (Handle, Handle, long, long);

extern long searchhandleunicase (Handle, Handle, long, long);

extern boolean sethandlecontents (ptrvoid, long, Handle);

extern void texttostring (ptrvoid, long, bigstring);

extern void texthandletostring (Handle, bigstring);

extern boolean insertinhandle (Handle, long, ptrvoid, long);

extern boolean inserttextinhandle (Handle, long, bigstring);

extern boolean inserthandleinhandle (Handle hinsert, Handle h, unsigned long ix);

extern boolean pullfromhandle (Handle, long, long, ptrvoid);

extern boolean enlargehandle (Handle, long, ptrvoid);

extern boolean mungehandle (Handle, long, long, ptrvoid, long);

extern boolean pushtexthandle (const bigstring, Handle);

extern boolean pushindentedline (short, bigstring, Handle);

extern boolean setheapstring (const bigstring, hdlstring);

extern boolean pushhandle (Handle, Handle);

extern boolean pushstringhandle (const bigstring, Handle);

//extern boolean concatheapstrings (hdlstring *, hdlstring *, hdlstring *);

extern boolean loadfromhandle (Handle, long *, long, ptrvoid);

extern boolean pushlongondiskhandle (long, Handle);

extern boolean loadlongfromdiskhandle (Handle, long *, long *);

extern boolean popfromhandle  (Handle, long, ptrvoid);

extern boolean copyhandlecontents (Handle, Handle);

extern boolean setintarray (hdlintarray, short, short);

extern boolean getintarray (hdlintarray, short, short *);

extern void fillintarray (hdlintarray, short);

extern void openhandlestream (Handle h, handlestream *s);
	
extern boolean growhandlestream (handlestream *s, long ct);
	
extern boolean writehandlestream (handlestream *s, void *p, long ct);

extern boolean writehandlestreamlong (handlestream *s, long x); /*6.2a8 AR*/

extern boolean writehandlestreamchar (handlestream *s, byte ch);

extern boolean writehandlestreamstring (handlestream *s, bigstring bs);
	
extern boolean writehandlestreamhandle (handlestream *s, Handle h);

extern boolean writehandlestreamhandlepart (handlestream *s, Handle h, long ix, long len); /*2003-04-27 AR*/

extern boolean writehandlestreamhandleindent (handlestream *s, Handle h, long i);

extern boolean writehandlestreamstringindent (handlestream *s, bigstring bs, long i);

extern boolean mergehandlestreamdata (handlestream *s, long ctreplace, ptrvoid pdata, long ctmerge);

extern boolean mergehandlestreamhandle (handlestream *s, long ctreplace, Handle hmerge);

extern boolean mergehandlestreamstring (handlestream *s, long ctreplace, bigstring bsmerge);

extern boolean readhandlestream (handlestream *, void *, long);

extern boolean pullfromhandlestream (handlestream *s, long ctpull, void *pdata);

extern long skiphandlestreamchars (handlestream *s, byte chskip);

extern boolean skiphandlestreamwhitespace (handlestream *);

extern long seekhandlestreamchar (handlestream *s, byte chseek);

extern boolean readhandlestreamfieldtohandle (handlestream *s, byte chdelimiter, Handle *hreturned);

extern boolean readhandlestreamfield (handlestream *s, byte chdelimiter, bigstring bsfield);
	
extern byte gethandlestreamcharacter (handlestream *s, long pos);
	
extern byte nexthandlestreamcharacter (handlestream *s);
	
extern byte lasthandlestreamcharacter (handlestream *s);
	
extern boolean athandlestreameof (handlestream *s);

extern Handle closehandlestream (handlestream *s);

extern void disposehandlestream (handlestream *s);



extern boolean initmemory (void);

#endif



