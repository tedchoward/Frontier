
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

#ifdef MACVERSION
#include <standard.h>
#endif

#ifdef WIN95VERSION
#include "standard.h"
#endif

#include "error.h"
#include "memory.h"
#include "shellhooks.h"



#define safetycushionsize 0x2800 /*10K*/

#ifdef fldebug

long cttemphandles = 0;

THz tempzone;

#endif


static Handle hsafetycushion = nil; /*a buffer to allow memory error reporting*/

static boolean flholdsafetycushion = false;


static boolean getsafetycushion (void) {
	
	if (hsafetycushion == nil)
		hsafetycushion = NewHandle (safetycushionsize);
	
	return (hsafetycushion != nil);
	} /*getsafetycushion*/


static boolean safetycushionhook (long *ctbytesneeded) {
	
	if (!flholdsafetycushion && (hsafetycushion != nil)) {
		
		*ctbytesneeded -= safetycushionsize;
		
		DisposeHandle (hsafetycushion);
		
		hsafetycushion = nil;
		}
	
	return (true); /*call remaining hooks*/
	} /*safetycushionhook*/

#if (MEMTRACKER == 1)
	static Handle debuggetnewhandle (char * filename, unsigned long linenumber, unsigned long threadid, long ctbytes, boolean fltemp) {
		
		/*
		2.1b3 dmb: new fltemp parameter. if true, try temp memory first, then 
		our heap.
		*/
		
		register Handle h;
		OSErr err;
		long extrasize;

		extrasize = strlen(filename) + 1 + sizeof(long) + sizeof(long) + sizeof(long) + (2 * (sizeof(Handle))) + 4;

		#ifdef MACVERSION
			if (fltemp) { /*try grabbing temp memory first*/
				
				h = TempNewHandle (ctbytes + extrasize, &err);
				
				if (h != nil) {
					
					debugaddmemhandle (h, ctbytes, filename, linenumber, threadid);

					#ifdef fldebug
					
					++cttemphandles;
					
					tempzone = HandleZone (h);
					
					#endif
					
					return (h);
					}
				}
		#endif
		
		if (hsafetycushion == nil) { /*don't allocate new stuff w/out safety cushion*/
			
			if (!getsafetycushion ())
				return (nil);
			}
		
		flholdsafetycushion = true;
		
		#ifdef MACVERSION
			h = NewHandle (ctbytes);

			if (h != nil)
				debugaddmemhandle (h, ctbytes, filename, linenumber, threadid);
		#endif
		#ifdef WIN95VERSION
			h = debugfrontierAlloc (filename, linenumber, threadid, ctbytes);
		#endif
		
		flholdsafetycushion = false;
		
		return (h);
		} /*getnewhandle*/

#else
	static Handle getnewhandle (long ctbytes, boolean fltemp) {
		
		/*
		2.1b3 dmb: new fltemp parameter. if true, try temp memory first, then 
		our heap.
		*/
		
		register Handle h;
		OSErr err;

		#ifdef MACVERSION
			if (fltemp) { /*try grabbing temp memory first*/
				
				h = TempNewHandle (ctbytes, &err);
				
				if (h != nil) {
					
					#ifdef fldebug
					
					++cttemphandles;
					
					tempzone = HandleZone (h);
					
					#endif
					
					return (h);
					}
				}
		#endif
		
		if (hsafetycushion == nil) { /*don't allocate new stuff w/out safety cushion*/
			
			if (!getsafetycushion ())
				return (nil);
			}
		
		flholdsafetycushion = true;
		
		h = NewHandle (ctbytes);
		
		flholdsafetycushion = false;
		
		return (h);
		} /*getnewhandle*/
#endif

static boolean resizehandle (Handle hresize, long size) {
	
	register Handle h = hresize;
	
	if (size > gethandlesize (h)) {
		
		if (hsafetycushion == nil) { /*don't allocate new stuff w/out safety cushion*/
			
			if (!getsafetycushion ())
				return (false);
			}
		}
	
	flholdsafetycushion = true;
	
	SetHandleSize (h, size);
	
	flholdsafetycushion = false;
	
	return (MemError () == noErr);
	} /*resizehandle*/


/*
heapmess (void) {
	
	Handle ray [1000];
	short ixray = 0;
	Handle h;
	short i;
	
	while (true) {
		
		h = NewHandle (1024L);
		
		if (h == nil)
			break;
		
		ray [ixray++] = h;
		} /*while%/
	
	for (i = 0; i < ixray; i++)
		disposehandle (ray [i]);
	} /*heapmess*/

/*
boolean analyzeheap (void) {
	
	/*
	get some statistics about the heap.  doesn't work in 32-bit mode.
	%/
	
	register byte *pblock;
	register long size;
	register byte *plimit;
	static long cthandles;
	static long ctbytes;
	static long avghandlesize;
	
	pblock = (byte *) &(*TheZone).heapData; /*point for first block in heap zone%/
	
	plimit = (byte *) (*TheZone).bkLim;
	
	cthandles = 0;
	
	ctbytes = 0; /*logical size of each handle%/
	
	while (pblock < plimit) {
		
		size = *(long *)pblock & 0x00ffffff;
		
		if (*pblock & 0x80) { /*a relocateable block%/
			
			ctbytes += size; /*add physical size%/
			
			/*ctbytes -= 8 + (*pblock & 0x0f); /*subtract header & size correction%/
			
			++cthandles;
			}
		
		pblock += size;
		}
	
	avghandlesize = ctbytes / cthandles;
	} /*analyzeheap*/


boolean haveheapspace (long size) {
	
	/*
	see if there's enough space in the heap to allocate a handle of the given size.
	
	if not, return false, but don't generate an error.
	*/
	
	Handle h;

	#if (MEMTRACKER == 1)
		h = debuggetnewhandle (__FILE__, __LINE__, GetCurrentThreadId(), size, false);
	#else
		h = getnewhandle (size, false);
	#endif

	if (h == nil)
		return (false);
	
	DisposeHandle (h);
	
	return (true);
	} /*haveheapspace*/


boolean testheapspace (long size) {
	
	/*
	see if there's enough space in the heap to allocate a handle of the given size.
	
	if not, generate an error, and return false.
	*/
	
	if (haveheapspace (size))
		return (true);
	
	memoryerror ();
	
	return (false);
	} /*testheapspace*/


void lockhandle (Handle h) {
	
	HLock (h);
	} /*lockhandle*/


void unlockhandle (Handle h) {
	
	HUnlock (h);
	} /*unlockhandle*/


boolean validhandle (Handle h) {
	
	if (h == nil)
		return (true);
	
#ifdef MACVERSION

	if (GetHandleSize (h) < 0) /*negative length never valid*/
		return (false);
	
	return (MemError () == noErr);
#endif

#ifdef WIN95VERSION
	if (GlobalSize (h) <= 0)
		return (false);

	return (true);
#endif
	} /*validhandle*/


#if (MEMTRACKER == 1)
	boolean debugnewhandle (char * filename, unsigned long linenumber, unsigned long threadid, long size, Handle *h) {
		
		*h = debuggetnewhandle (filename, linenumber, threadid, size, false);
		
		if (*h == nil) {
			
			memoryerror ();
			
			return (false);
			}
		
		return (true);
		} /*newhandle*/


	boolean debugnewemptyhandle (char * filename, unsigned long linenumber, unsigned long threadid, Handle *h) {
		
		return (debugnewhandle (filename, linenumber, threadid, (long) 0, h));
		} /*newemptyhandle*/
#else
	boolean newhandle (long size, Handle *h) {
		
		*h = getnewhandle (size, false);
		
		if (*h == nil) {
			
			memoryerror ();
			
			return (false);
			}
		
		return (true);
		} /*newhandle*/


	boolean newemptyhandle (Handle *h) {
		
		return (newhandle ((long) 0, h));
		} /*newemptyhandle*/
#endif

void disposehandle (Handle h) {
	
	if (h != nil) {
		
	#ifdef WIN95VERSION
		DisposeHandle (h);
	#endif

	#ifdef MACVERSION		
		#if (MEMTRACKER == 1)
			debugremovememhandle(h);
		#endif

		#ifdef fldebug

		if (HandleZone (h) == tempzone)
			--cttemphandles;

		#endif
		
		DisposeHandle (h);
		
		#ifdef fldebug
		
		if (MemError ())
			 memoryerror ();
		
		#endif
	#endif		
		}
	} /*disposehandle*/


long gethandlesize (Handle h) {
	
	if (h == nil)
		return (0L);
	
	return (GetHandleSize (h));
	} /*gethandlesize*/


boolean sethandlesize (Handle h, long size) {
	
	if (h == nil) /*defensive driving*/
		return (false);
	
	if (!resizehandle (h, size)) {
		
		memoryerror ();
		
		return (false);
		}
	
	return (true);
	} /*sethandlesize*/
	
	
boolean minhandlesize (Handle h, long size) {
	
	if (gethandlesize (h) >= size) /*already big enough*/
		return (true);
		
	return (sethandlesize (h, size)); /*try to make it larger*/
	} /*minhandlesize*/


void moveleft (ptrvoid psource, ptrvoid pdest, long length) {
	
	/*
	do a mass memory move with the left edge leading.  good for closing
	up a gap in a buffer, among other thingsÉ
	*/
#if defined (MACVERSION)
	BlockMoveData (psource, pdest, length);
	
#elif defined (WIN95VERSION)
	CopyMemory (pdest, psource, length);
	
#else
	register ptrbyte ps = psource, pd = pdest;
	register long ctloops = length;
	
	while (--ctloops >= 0)
		*pd++ = *ps++;
#endif
	} /*moveleft*/


void moveright (ptrvoid psource, ptrvoid pdest, long length) {
	
	/*
	do a mass memory move with the right edge leading.  good for opening
	up a gap in a buffer, among other thingsÉ
	*/
	
#if defined (MACVERSION)
	BlockMoveData (psource, pdest, length);
	
#else
	register ptrbyte ps, pd;
	register long ctloops;
	
	ctloops = length;
	
	if (ctloops > 0) {
	
		ps = (ptrbyte) psource + length - 1; /*right edge of source*/
	
		pd = (ptrbyte) pdest + length - 1; /*right edge of destination*/
	
		while (ctloops--) 
			*pd-- = *ps--;
		}
#endif
	} /*moveright*/
	
	
void fillchar (ptrvoid pfill, long ctfill, char chfill) {
	
	/*
	do a mass memory fill -- copy ctfill chfills at pfill.
	*/
	
#ifdef MACVERSION
	register ptrbyte p = pfill;
	register long ct = ctfill;
	register char ch = chfill;
	
	while (--ct >= 0)
		*p++ = (char) ch; /*tight loop*/
#endif

#ifdef WIN95VERSION
	FillMemory (pfill, ctfill, chfill);
#endif
	} /*fillchar*/
	

void clearbytes (ptrvoid pclear, long ctclear) {
	
	/*
	fill memory with 0's.
	*/
	
	fillchar (pclear, ctclear, (char) 0);
	} /*clearbytes*/
	

void clearhandle (Handle hclear) {
	
	/*
	fill a handle's data with 0's.
	*/
	
	register Handle h = hclear;
	register long size;
	
	size = gethandlesize (h);
	
	/*
	lockhandle (h);
	*/
	
	fillchar (*h, size, (char) 0);
	
	/*
	unlockhandle (h);
	*/
	} /*clearhandle*/
	

void movefromhandle (Handle h, ptrvoid pdest, long length) {
	
	if (h != nil) {
	
		/*
		HLock (h);
		*/
	
		moveleft (*h, pdest, length);
	
		/*
		HUnlock (h);
		*/
		}
	} /*movefromhandle*/
	
#if (MEMTRACKER == 1)
	boolean debugnewclearhandle (char * filename, unsigned long linenumber, unsigned long threadid, long size, Handle *hreturned) {
		
		register Handle h;
		register long ctbytes;
		
		ctbytes = size; /*copy into a register*/
		
		h = debuggetnewhandle (filename, linenumber, threadid, ctbytes, false);
		
		if (h == nil) {
			
			*hreturned = nil;
			
			memoryerror ();
			
			return (false);
			}
			
		clearhandle (h);
		
		*hreturned = h;
		
		return (true);
		} /*newclearhandle*/
		

	boolean debugnewfilledhandle (char * filename, unsigned long linenumber, unsigned long threadid, ptrvoid pdata, long size, Handle *hreturned) {
		
		register Handle h;
		register long ctbytes;
		
		ctbytes = size; 
		
		h = debuggetnewhandle (filename, linenumber, threadid, ctbytes, false);
		
		if (h == nil) {
			
			*hreturned = nil;
			
			memoryerror ();
			
			return (false);
			}
		
		moveleft (pdata, *h, ctbytes);
			
		*hreturned = h;
		
		return (true);
		} /*newfilledhandle*/
#else	
	boolean newclearhandle (long size, Handle *hreturned) {
		
		register Handle h;
		register long ctbytes;
		
		ctbytes = size; /*copy into a register*/
		
		h = getnewhandle (ctbytes, false);
		
		if (h == nil) {
			
			*hreturned = nil;
			
			memoryerror ();
			
			return (false);
			}
			
		clearhandle (h);
		
		*hreturned = h;
		
		return (true);
		} /*newclearhandle*/
		

	boolean newfilledhandle (ptrvoid pdata, long size, Handle *hreturned) {
		
		register Handle h;
		register long ctbytes;
		
		ctbytes = size; 
		
		h = getnewhandle (ctbytes, false);
		
		if (h == nil) {
			
			*hreturned = nil;
			
			memoryerror ();
			
			return (false);
			}
		
		moveleft (pdata, *h, ctbytes);
			
		*hreturned = h;
		
		return (true);
		} /*newfilledhandle*/
#endif

#if (flruntime==0)
#if (odbengine==0)
static long getidealchunksize (void) {
	
	/*
	our caller wants to pre-allocate something large. as large as possible 
	without necessarily forcing a heap compaction.
	
	our algorith: find out what the largest possible size would be if the 
	heap was compated, then back off a bit, depending on what the number is. 
	often, there's one big free block and many small ones, so hopefully this 
	will yield a good result on average.
	
	4/20/93 dmb: tweaked algorith; if more than 64K is available, grab half
	*/
#ifdef MACVERSION		
	register long ctgrab = MaxBlock ();
	
	if (ctgrab < 0x4000)
		ctgrab -= 0x0400;
	else
		if (ctgrab < 0x10000)
			ctgrab -= 0x2000;
		else
			ctgrab >>= 1;
	
	return (ctgrab);
#endif

#ifdef WIN95VERSION
	return (32768L);
#endif

	} /*getidealchunksize*/

#if (MEMTRACKER == 1)
	boolean debugnewgrowinghandle (char * filename, unsigned long linenumber, unsigned long threadid, long size, Handle *h) {
		
		/*
		allocate a handle of the specified size, but with room to grow.
		
		note that the first getnewhandle should never fail, since the memory 
		manager just told us that an even large block could be produced. but 
		it's easy enough to handle that case too, so we do.
		*/
		
		register long ctgrab = getidealchunksize ();
		
		if (ctgrab > size) {
			
			*h = debuggetnewhandle (filename, linenumber, threadid, ctgrab, false);
			
			if (*h != nil) {
				
				SetHandleSize (*h, size); /*can't fail, getting smaller*/
				
				return (true);
				}
			}
		
		return (debugnewhandle (filename, linenumber, threadid, size, h));
		} /*newhandlewithroomtogrow*/
#else
	boolean newgrowinghandle (long size, Handle *h) {
		
		/*
		allocate a handle of the specified size, but with room to grow.
		
		note that the first getnewhandle should never fail, since the memory 
		manager just told us that an even large block could be produced. but 
		it's easy enough to handle that case too, so we do.
		*/
		
		register long ctgrab = getidealchunksize ();
		
		if (ctgrab > size) {
			
			*h = getnewhandle (ctgrab, false);
			
			if (*h != nil) {
				
				SetHandleSize (*h, size); /*can't fail, getting smaller*/
				
				return (true);
				}
			}
		
		return (newhandle (size, h));
		} /*newhandlewithroomtogrow*/
#endif

#endif
#endif


boolean prepareforgrowing (Handle h) {
	
	/*
	like newgrowinghandle, but for an already-existing handle. the 
	caller is about to push a bunch of stuff onto it, and wants to 
	avoid having it continually compacting the heap along the way.
	*/
	
	return (true); /*it turns out to be incredibly slow to resize an existing handle like this*/
	
	/*
	register long ctgrab = getidealchunksize ();
	register long ctorig = gethandlesize (h);
	
	if (h == nil) /*defensive driving%/
		return (false);
	
	ctgrab >>= 1; /*cut in half%/
	
	if (ctgrab > ctorig) {
		
		if (resizehandle (h, ctgrab)) {
			
			SetHandleSize (h, ctorig);
			
			return (true);
			}
		}
	
	return (false);
	*/
	} /*prepareforgrowing*/
		

#if (MEMTRACKER == 1)
boolean debugcopyhandle (char * filename, unsigned long linenumber, unsigned long threadid, Handle horig, Handle *hcopy) {
	
	register Handle h;
	register long ct;
	
	if (horig == nil) /*easy to copy a nil handle*/
		h = nil;
	
	else {
		
		ct = gethandlesize (horig);
		
		h = debuggetnewhandle (filename, linenumber, threadid, ct, false);
		
		if (h == nil) {
			
			memoryerror ();
			
			return (false);
			}
		
		moveleft (*horig, *h, ct);
		}
	
	*hcopy = h;
	
	return (true);
	} /*copyhandle*/
#else
boolean copyhandle (Handle horig, Handle *hcopy) {
	
	register Handle h;
	register long ct;
	
	if (horig == nil) /*easy to copy a nil handle*/
		h = nil;
	
	else {
		
		ct = gethandlesize (horig);
		
		h = getnewhandle (ct, false);
		
		if (h == nil) {
			
			memoryerror ();
			
			return (false);
			}
		
		moveleft (*horig, *h, ct);
		}
	
	*hcopy = h;
	
	return (true);
	} /*copyhandle*/
#endif

short comparehandles (Handle h1, Handle h2) {
	
	/*
	return -1 if h1 is less than h2, or +1 if h1 is greater than h2.
	
	return zero if the two byte streams are equal.
	
	9/21/92 dmb: make sure we don't dereference nil handles.
	*/
	
	long len1, len2;
	register long ct;
	
	len1 = gethandlesize (h1);
	
	len2 = gethandlesize (h2);
	
	ct = min (len1, len2);
	
	if (ct > 0) { /*both handles are non-nil; safe to dereference*/
		
		register ptrbyte p1 = (ptrbyte) *h1;
		register ptrbyte p2 = (ptrbyte) *h2;
		
		for (; --ct >= 0; ++p1, ++p2)
		
			if (*p1 != *p2) { /*unequal chars*/
				
				register char n = (char) (*p1 - *p2); /*change into signed value*/
				
				return (sgn (n));
				}
		}
	
	return (sgn (len1 - len2));
	} /*comparehandles*/


boolean equalhandles (Handle h1, Handle h2) {
	
	/*
	we could check for equal handle sizes first, we don't expect this 
	to be a time-critical routine
	*/
	
	return (comparehandles (h1, h2) == 0);
	} /*equalhandles*/


long searchhandle (Handle hsearch, Handle hpattern, long ixstart, long ixlimit) {
	
	/*
	the beginning of something bigger.  first version -- search for the 
	pattern in the byte stream, return the offset of the first occurrence of
	the pattern.  -1 if not found.
	
	8/12/92 dmb: make sure ixstart is in range
	
	8/14/92 dmb: added ixlimit parameter
	*/
	
	register ptrbyte psearch = (ptrbyte) *hsearch;
	register ptrbyte ppattern = (ptrbyte) *hpattern;
	register long ixstring = ixstart;
	register long i;
	register long lensearch = gethandlesize (hsearch);
	register long lenpattern = gethandlesize (hpattern);
	register byte chfirst;
	
	lensearch = min (ixlimit, lensearch) - lenpattern + 1;
	
	if ((lensearch <= 0) || (lenpattern == 0) || (ixstring >= lensearch))
		return (-1L);
	
	chfirst = ppattern [0];
	
	while (true) {
		
		if (psearch [ixstring] == chfirst) { /*matched at least first character in string*/
			
			for (i = 1; i < lenpattern; ++i) {
				
				if (psearch [ixstring + i] != ppattern [i]) 
					goto L1;
				} /*for*/
			
			return (ixstring); /*loop terminated, full match*/
			}
		
		L1: /*advance to next character in string*/
		
		if (++ixstring == lensearch) /*reached end of string, not found*/
			return (-1L);
		} /*while*/
	} /*searchhandle*/


boolean sethandlecontents (ptrvoid pdata, long ctset, Handle hset) {
	
	if (!sethandlesize (hset, ctset))
		return (false);
	
	moveleft (pdata, *hset, ctset);
	
	return (true);
	} /*sethandlecontents*/


void texttostring (ptrvoid ptext, long ctchars, bigstring bs) {
	
	register long len = ctchars;
	
	if (len > lenbigstring)
		len = lenbigstring;
	
	setstringlength (bs, len);
	
	moveleft (ptext, &bs [1], len);
	} /*texttostring*/


void texthandletostring (Handle htext, bigstring bs) {
	
	texttostring (*htext, gethandlesize (htext), bs);
	} /*texthandletostring*/

#if (MEMTRACKER == 1)
boolean debugnewtexthandle (char * filename, unsigned long linenumber, unsigned long threadid, const bigstring bs, Handle *htext) {
	
	/*
	create a new handle to hold the text of the string.
	
	if the string is "\pABC" -- you get a handle of size 3.
	*/
	
	register long len = stringlength (bs);
	register Handle h;
	
	h = debuggetnewhandle (filename, linenumber, threadid, len, false);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	if (len > 0)
		moveleft ((ptrstring) stringbaseaddress (bs), *h, len);
	
	*htext = h; /*pass handle back to caller*/
	
	return (true);
	} /*newtexthandle*/
#else
boolean newtexthandle (const bigstring bs, Handle *htext) {
	
	/*
	create a new handle to hold the text of the string.
	
	if the string is "\pABC" -- you get a handle of size 3.
	*/
	
	register long len = stringlength (bs);
	register Handle h;
	
	h = getnewhandle (len, false);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	if (len > 0)
		moveleft ((ptrstring) stringbaseaddress (bs), *h, len);
	
	*htext = h; /*pass handle back to caller*/
	
	return (true);
	} /*newtexthandle*/
#endif

boolean insertinhandle (Handle hgrow, long ix, ptrvoid pdata, long ctgrow) {
	
	/*
	make the handle big enough to hold the new data, and insert the new 
	data into the newly enlarged handle.
	
	8.1.97 dmb: if pdata is nil, initialize new data to zeros
	*/
	
	Handle h = hgrow;
	unsigned long origsize;
	
	if (ctgrow == 0)
		return (true);
	
	origsize = gethandlesize (hgrow);
	
	if (!sethandlesize (hgrow, origsize + ctgrow))
		return (false);
	
	if (ix < origsize)
		moveright (*hgrow + ix, *hgrow + ix + ctgrow, origsize - ix);
	
	if (pdata == nil)
		clearbytes (*hgrow + ix, ctgrow);
	else
		moveleft (pdata, *hgrow + ix, ctgrow);
	
	return (true);
	} /*insertinhandle*/


boolean inserthandleinhandle (Handle hinsert, Handle h, unsigned long ix) {
	
	unsigned long insertlen = gethandlesize (hinsert);
	unsigned long ct = gethandlesize (h);
	unsigned long newlen = ct + insertlen;
	
	if (hinsert == nil) /*nothing to do*/
		return (true);
		
	if (!sethandlesize (h, newlen))
		return (false);
	
	moveright (&(*h) [ix], &(*h) [ix + insertlen], ct - ix);
	
	moveleft (*hinsert, &(*h) [ix], insertlen);
	
	return (true);
	} /*inserthandleinhandle*/
	
	
boolean pullfromhandle (Handle hpull, long ixpull, long ctpull, ptrvoid pdata) {
	
	/*
	8/14/91 dmb: accept nil for pdata to remove bytes without retaining them
	*/
	
	register Handle h = hpull;
	register long ix = ixpull;
	register long ct = ctpull;
	register long newsize = gethandlesize (h) - ct;
	
	if ((ix < 0) || (ix > newsize))
		return (false);
	
	if (pdata != nil)
		moveleft (*h + ix, pdata, ct); /*copy out of the handle*/
	
	moveleft (*h + ix + ct, *h + ix, newsize - ix);
	
	return (sethandlesize (h, newsize)); /*should never fail -- getting smaller*/
	} /*pullfromhandle*/


boolean enlargehandle (Handle hgrow, long ctgrow, ptrvoid newdata) {
	
	/*
	make the handle big enough to hold the new data, and move the new data in
	at the end of the newly enlarged handle.
	
	8.1.97 dmb: if newdata is nil, initialize new data to zeros
	*/
	
	return (insertinhandle (hgrow, gethandlesize (hgrow), newdata, ctgrow));
	
	/*
	register Handle h = hgrow;
	register long ct = ctgrow;
	register ptrbyte p = newdata;
	register long origsize;
	
	if (hgrow == nil)
		Debugger ();
	
	if (ct == 0)
		return (true);
	
	origsize = gethandlesize (h);
	
	if (!sethandlesize (h, origsize + ct))
		return (false);
	
	moveleft (p, *h + origsize, ct);
	
	return (true);
	*/
	
	} /*enlargehandle*/


boolean mungehandle (Handle hmunge, long ixmunge, long ctmunge, ptrvoid pinsert, long ctinsert) {
	
	/*
	modeled after the Mac Toolbox utility "Munger", minus the search 
	capability, plus some error handling...
	
	overwrite ctmunge bytes in the handle hmunge, starting at offset ixmunge, 
	with the bytes indicated by pinsert, ctinsert.  resize the handle as 
	required, shifting any bytes to the right
	*/
	
	register long sizediff = ctinsert - ctmunge;
	register byte *pdata = pinsert;
	
	switch (sgn (sizediff)) { /*insert or delete unmatched bytes*/
		
		case +1: /*growing*/
			if (!insertinhandle (hmunge, ixmunge + ctmunge, pdata + ctmunge, sizediff))
				return (false);
			
			break;
		
		case -1: /*shrinking*/
			pullfromhandle (hmunge, ixmunge + ctinsert, -sizediff, nil);
			
			break;
		
		default: /*no change*/
			break;
		}
	
	moveleft (pdata, *hmunge + ixmunge, min (ctinsert, ctmunge));
	
	return (true);
	} /*mungehandle*/


boolean pushstringhandle (const bigstring bs, Handle htext) {
	
	/*
	htext is a handle created with newtexthandle.
	
	increase the size of the handle so we can push the pascal string bs 
	at the end of the handle (including length byte).
	*/
	
	return (enlargehandle (htext, (long) stringsize (bs), (ptrvoid) bs));
	} /*pushstringhandle*/


boolean pushtexthandle (const bigstring bs, Handle htext) {
	
	/*
	htext is a handle created with newtexthandle.
	
	increase the size of the handle so we can push the text of bs at 
	the end of the handle (not including length byte).
	*/
	
	return (enlargehandle (htext, (long) stringlength (bs), (ptrvoid) stringbaseaddress (bs)));
	} /*pushtexthandle*/


/*
boolean pushindentedline (short level, bigstring bs, Handle htext) {
	
	/*
	add a line to the indicated text buffer, indented with tab characters and
	terminated with a carriage return.
	
	return false if it failed, we dispose of htext if it failed.
	%/
	
	bigstring bsoutput;
	
	filledstring (chtab, level, bsoutput); /*put out leading tabs%/
	
	pushstring (bs, bsoutput); /*put out the string itself%/
	
	pushchar (chreturn, bsoutput); /*put out a terminating carriage return character%/
	
	if (!pushtexthandle (bsoutput, htext)) { /*out of memory%/
	
		disposehandle (htext);
		
		return (false);
		}
	
	return (true);
	} /*pushindentedline*/
	   

#if (MEMTRACKER == 1)
boolean debugnewheapstring (char * filename, unsigned long linenumber, unsigned long threadid, const bigstring bs, hdlstring *hstring) {
	
	/*
	3/28/97 dmb: rewrote x-plat; don't use NewString toolbox
	*/

	return (debugnewfilledhandle (filename, linenumber, threadid, (void *) bs, stringsize (bs), (Handle *)hstring));
	} /*newheapstring*/
#else
boolean newheapstring (const bigstring bs, hdlstring *hstring) {
	
	/*
	3/28/97 dmb: rewrote x-plat; don't use NewString toolbox
	*/

	return (newfilledhandle ((void *) bs, stringsize (bs), (Handle *)hstring));
	} /*newheapstring*/
#endif

boolean setheapstring (const bigstring bs, hdlstring hstring) {
	
	return (sethandlecontents ((ptrvoid) bs, stringlength (bs) + 1, (Handle) hstring));
	} /*setheapstring*/


/*
boolean concatheapstrings (h1, h2, hreturned) hdlstring *h1, *h2, *hreturned; {

	/*
	given two heap strings, return a heap string that's the result of 
	concatenating the two strings.
	
	return false if the resulting string would be too long, or if there
	was a memory allocation error.
	%/
	
	bigstring bs1, bs2;
	bigstring bs;
	register long len1, len2;
	register long len;
	register hdlstring hstring;
	
	copyheapstring (*h1, bs1);
	
	copyheapstring (*h2, bs2);
	
	len1 = stringlength(bs1);
	
	len2 = stringlength(bs2);
	
	len = len1 + len2;
	
	if (len > lenbigstring)
		return (false);
		
	setstringlength(bs, len);
	
	moveleft (&bs1 [1], &bs [1], len1);
	
	moveleft (&bs2 [1], &bs [1 + len1], len2);
		
	return (newheapstring (bs, hreturned));
	} /*concatheapstrings%/
*/


boolean pushhandle (Handle hsource, Handle hdest) {
	
	/*
	add the content of the source handle at the end of the destination handle.
	*/
	
	register boolean fl;
	
	if (hsource == nil)
		return (true);
	
	HLock (hsource);
	
	fl = enlargehandle (hdest, gethandlesize (hsource), *hsource);
	
	HUnlock (hsource);
	
	return (fl);
	} /*pushhandle*/
	

boolean loadfromhandle (Handle hload, long *ixload, long ctload, ptrvoid pdata) {
	
	/*
	copy the next ctload bytes from hload into pdata and increment the index.
	
	return false if there aren't enough bytes.
	
	start ixload at 0.
	*/
	
	register Handle h = hload;
	register ptrbyte p = pdata;
	register long ct = ctload;
	register long ix = *ixload;
	register long size;
	
	size = gethandlesize (h);
	
	if (ix < 0) /*bad index*/
		return (false);
	
	if ((ix + ct) > size) /*asked for more bytes than there are*/
		return (false); 
		
	moveleft (*h + ix, p, ct); /*copy out of the handle*/
	
	*ixload = ix + ct; /*increment the index into the handle*/
	
	return (true);
	} /*loadfromhandle*/

#if (MEMTRACKER == 1)
boolean debugloadfromhandletohandle (char * filename, unsigned long linenumber, unsigned long threadid, Handle hload, long *ixload, long ctload, boolean fltemp, Handle *hnew) {
	
	/*
	load from the source handle, creating a new handle to hold the loaded stuff.
	
	6/8/91 dmb: fixed memory leak if loadfromhandle fails
	
	11/27/91 dmb: reject ctload < 0
	
	2.1b3 dmb: added fltemp parameter to determine whether caller is going 
	to dispose the result soon.
	*/
	
	Handle h;
	
	if (ctload < 0)
		return (false);
	
	h = debuggetnewhandle (filename, linenumber, threadid, ctload, fltemp);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	if (!loadfromhandle (hload, ixload, ctload, *h)) {
		
		disposehandle (h);
		
		return (false);
		}
	
	*hnew = h;
	
	return (true);
	} /*loadfromhandletohandle*/


boolean debugloadhandleremains (char * filename, unsigned long linenumber, unsigned long threadid, long ix, Handle hsource, Handle *hdest) {

	/*
	load all the bytes following ix from the source handle into the dest handle.
	
	2.1b3 dmb: callers of this routine are (almost) always fine with 
	temporary memory, so let's use it.
	*/
	
	return (debugloadfromhandletohandle (filename, linenumber, threadid, hsource, &ix, gethandlesize (hsource) - ix, true, hdest));
	} /*loadhandleremains*/
#else
boolean loadfromhandletohandle (Handle hload, long *ixload, long ctload, boolean fltemp, Handle *hnew) {
	
	/*
	load from the source handle, creating a new handle to hold the loaded stuff.
	
	6/8/91 dmb: fixed memory leak if loadfromhandle fails
	
	11/27/91 dmb: reject ctload < 0
	
	2.1b3 dmb: added fltemp parameter to determine whether caller is going 
	to dispose the result soon.
	*/
	
	Handle h;
	
	if (ctload < 0)
		return (false);
	
	h = getnewhandle (ctload, fltemp);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	if (!loadfromhandle (hload, ixload, ctload, *h)) {
		
		disposehandle (h);
		
		return (false);
		}
	
	*hnew = h;
	
	return (true);
	} /*loadfromhandletohandle*/


boolean loadhandleremains (long ix, Handle hsource, Handle *hdest) {

	/*
	load all the bytes following ix from the source handle into the dest handle.
	
	2.1b3 dmb: callers of this routine are (almost) always fine with 
	temporary memory, so let's use it.
	*/
	
	return (loadfromhandletohandle (hsource, &ix, gethandlesize (hsource) - ix, true, hdest));
	} /*loadhandleremains*/
#endif
	
	
boolean pushlongondiskhandle (long x, Handle hpush) {
	
	memtodisklong (x);
	
	return (enlargehandle (hpush, sizeof (long), &x));
	} /*pushlongtodiskhandle*/


boolean loadlongfromdiskhandle (Handle hload, long *ixload, long *x) {
	
	if (!loadfromhandle (hload, ixload, sizeof (long), x))
		return (false);
	
	disktomemlong (*x);
	
	return (true);
	} /*loadlongfromdiskhandle*/


boolean popfromhandle (Handle hpop, long ctpop, ptrvoid pdata) {
	
	register Handle h = hpop;
	register long ixpop = gethandlesize (h) - ctpop;
	
	if (ixpop < 0)
		return (false);
	
	if (pdata != nil)
		moveleft (*h + ixpop, pdata, ctpop); /*copy out of the handle*/
	
	return (sethandlesize (h, ixpop)); /*should never fail -- getting smaller*/
	} /*popfromhandle*/


boolean copyhandlecontents (Handle hsource, Handle hdest) {
	
	/*
	copy the contents of hsource into hdest's space.  if hdest isn't large
	enough to receive hsource's content, we enlarge hdest.  if this fails 
	we return false.
	
	7/5/91 dmb: we now implement a more strict copy; the destination handle 
	is always sized to match the original.
	*/
	
	register long ctsource;
	
	ctsource = gethandlesize (hsource);
	
	if (!sethandlesize (hdest, ctsource)) /*heap allocation failed*/
		return (false);
	
	moveleft (*hsource, *hdest, ctsource);
	
	return (true);
	} /*copyhandlecontents*/
	

#if (MEMTRACKER == 1)
boolean debugconcathandles (char * filename, unsigned long linenumber, unsigned long threadid, Handle h1, Handle h2, Handle *hmerged) {
	
	/*
	create a new handle which is the concatenation of two handles.
	*/
	
	register Handle h;
	long sizefirsthandle;
	long sizesecondhandle;
	register ptrbyte p;
	
	*hmerged = nil; /*default return value*/
	
	sizefirsthandle = gethandlesize (h1);
	
	sizesecondhandle = gethandlesize (h2);
	
	h = debuggetnewhandle (filename, linenumber, threadid, sizefirsthandle + sizesecondhandle, false);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	p = (ptrbyte) *h;
		
	moveleft (*h1, p, sizefirsthandle);
	
	p += sizefirsthandle;
	
	moveleft (*h2, p, sizesecondhandle);
	
	*hmerged = h;
	
	return (true);
	} /*concathandles*/
	
	
boolean debugmergehandles (char * filename, unsigned long linenumber, unsigned long threadid, Handle h1, Handle h2, Handle *hmerged) {
	
	/*
	create a new handle which is the concatenation of two handles.  the first
	four bytes of the new handle store the size of the first handle so the merged
	handle can be easily unpacked.
	
	6/8/90 DW: modified so it could deal with nil handles.
	
	10/7/91 dmb: try to merge result into the larger of the original handles, so 
	that our memory overhead can be almost cut in half.
	
	2.1b3 dmb: in the unusual case the we allocated a new handle, go ahead 
	and use temporary memory if available. this might not always be ideal, but 
	more often than not it will be best -- we're likely Saving, and tossing 
	the merged handle soon.
	*/
	
	register Handle h;
	long sizefirsthandle;
	long sizesecondhandle;
	long sizemergedhandle;
	long storesizefirsthandle;
	register ptrbyte p;
	
	*hmerged = nil; /*default return value*/
	
	sizefirsthandle = gethandlesize (h1);
	storesizefirsthandle = conditionallongswap (sizefirsthandle);
	
	sizesecondhandle = gethandlesize (h2);
	
	sizemergedhandle = sizeof (long) + sizefirsthandle + sizesecondhandle;
	
	if (sizefirsthandle > sizesecondhandle) { /*try using h1 for result*/
		
		if (resizehandle (h1, sizemergedhandle)) {
			
			p = (ptrbyte) *h1;
			
			moveright (p, p + sizeof (long), sizefirsthandle);
			
			moveleft (&storesizefirsthandle, p, sizeof (long));
			
			if (h2 != nil)
				moveleft (*h2, p + sizeof (long) + sizefirsthandle, sizesecondhandle);
			
			*hmerged = h1;
			
			disposehandle (h2);
			
			return (true);
			}
		}
	
	else if (h2 != nil) { /*try using h2 for result*/
		
		if (resizehandle (h2, sizemergedhandle)) {
			
			p = (ptrbyte) *h2;
			
			moveright (p, p + sizeof (long) + sizefirsthandle, sizesecondhandle);
			
			moveleft (&storesizefirsthandle, p, sizeof (long));
			
			if (h1 != nil)
				moveleft (*h1, p + sizeof (long), sizefirsthandle);
			
			*hmerged = h2;
			
			disposehandle (h1);
			
			return (true);
			}
		}
	
	/*resizing didn't work; try it the old way, using a newly-allocated handle*/
	
	h = debuggetnewhandle (filename, linenumber, threadid, sizemergedhandle, true);
	
	if (h == nil) {
		
		memoryerror ();
		
		disposehandle (h1);
		
		disposehandle (h2);
		
		return (false);
		}
	
	p = (ptrbyte) *h;
	
	moveleft (&storesizefirsthandle, p, sizeof (long));
	
	p += sizeof (long);
	
	if (h1 != nil)
		moveleft (*h1, p, sizefirsthandle);
	
	if (h2 != nil)
		moveleft (*h2, p + sizefirsthandle, sizesecondhandle);
	
	*hmerged = h;
	
	disposehandle (h1);
	
	disposehandle (h2);
	
	return (true);
	} /*mergehandles*/


boolean debugunmergehandles (char * filename, unsigned long linenumber, unsigned long threadid, Handle hmerged, Handle *hfirst, Handle *hsecond) {
	
	/*
	split up a handle created by mergehandle.
	
	3/12/91 dmb: rewrote so that we own hmerged; caller no longer diposes it.  
	this allows us to reuse the handle to greatly reduce memory requirements.  
	we could further optimize by reusing hmerged for the larger handle instead 
	of always hsecond.
	
	also, avoid locking handles when we're trying to allocate potentially 
	large chunks of memory.
	
	2.1b3 dmb: newly-created handle is always ok as temporary memory
	*/
	
	register Handle h1 = nil, h2 = nil;
	register Handle h = hmerged;
	long ix;
	long sizefirsthandle, sizesecondhandle;
	
	*hfirst = *hsecond = nil; /*default return values*/
	
	moveleft (*h, &sizefirsthandle, sizeof (long));

#ifdef PACKFLIPPED
	longswap (sizefirsthandle);
#endif
	
	ix = sizeof (long);
	
	if (sizefirsthandle == 0)
		h1 = nil;
		
	else {
		h1 = debuggetnewhandle (filename, linenumber, threadid, sizefirsthandle, true);
		
		if (h1 == nil) 
			goto error;
		
		moveleft (*h + ix, *h1, sizefirsthandle);
		}
	
	ix += sizefirsthandle;
	
	sizesecondhandle = gethandlesize (h) - sizefirsthandle - sizeof (long);
	
	if (sizesecondhandle == 0) {
		
		h2 = nil;
		
		disposehandle (h);
		}
	else {
		
		h2 = h; /*second handle can safely re-use merged handle*/
		
		moveleft (*h2 + ix, *h2, sizesecondhandle);
		
		sethandlesize (h2, sizesecondhandle);
		}
	
	*hfirst = h1; /*return handles to caller*/
	
	*hsecond = h2;
	
	return (true);
		
	error:
	
	disposehandle (h);
	
	disposehandle (h1);
		
	disposehandle (h2);
	
	memoryerror ();
	
	return (false);
	} /*unmergehandles*/
	
	
boolean debugnewintarray (char * filename, unsigned long linenumber, unsigned long threadid, short ct, hdlintarray *harray) {
	
	Handle h;
	
	if (!debugnewclearhandle (filename, linenumber, threadid, sizeof (short) * ct, &h))
		return (false);
		
	*harray = (hdlintarray) h;
	
	return (true);
	} /*newintarray*/
#else	
boolean concathandles (Handle h1, Handle h2, Handle *hmerged) {
	
	/*
	create a new handle which is the concatenation of two handles.
	*/
	
	register Handle h;
	long sizefirsthandle;
	long sizesecondhandle;
	register ptrbyte p;
	
	*hmerged = nil; /*default return value*/
	
	sizefirsthandle = gethandlesize (h1);
	
	sizesecondhandle = gethandlesize (h2);
	
	h = getnewhandle (sizefirsthandle + sizesecondhandle, false);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	p = (ptrbyte) *h;
		
	moveleft (*h1, p, sizefirsthandle);
	
	p += sizefirsthandle;
	
	moveleft (*h2, p, sizesecondhandle);
	
	*hmerged = h;
	
	return (true);
	} /*concathandles*/
	
	
boolean mergehandles (Handle h1, Handle h2, Handle *hmerged) {
	
	/*
	create a new handle which is the concatenation of two handles.  the first
	four bytes of the new handle store the size of the first handle so the merged
	handle can be easily unpacked.
	
	6/8/90 DW: modified so it could deal with nil handles.
	
	10/7/91 dmb: try to merge result into the larger of the original handles, so 
	that our memory overhead can be almost cut in half.
	
	2.1b3 dmb: in the unusual case the we allocated a new handle, go ahead 
	and use temporary memory if available. this might not always be ideal, but 
	more often than not it will be best -- we're likely Saving, and tossing 
	the merged handle soon.
	*/
	
	register Handle h;
	long sizefirsthandle;
	long sizesecondhandle;
	long sizemergedhandle;
	long storesizefirsthandle;
	register ptrbyte p;
	
	*hmerged = nil; /*default return value*/
	
	sizefirsthandle = gethandlesize (h1);
	storesizefirsthandle = conditionallongswap (sizefirsthandle);
	
	sizesecondhandle = gethandlesize (h2);
	
	sizemergedhandle = sizeof (long) + sizefirsthandle + sizesecondhandle;
	
	if (sizefirsthandle > sizesecondhandle) { /*try using h1 for result*/
		
		if (resizehandle (h1, sizemergedhandle)) {
			
			p = (ptrbyte) *h1;
			
			moveright (p, p + sizeof (long), sizefirsthandle);
			
			moveleft (&storesizefirsthandle, p, sizeof (long));
			
			if (h2 != nil)
				moveleft (*h2, p + sizeof (long) + sizefirsthandle, sizesecondhandle);
			
			*hmerged = h1;
			
			disposehandle (h2);
			
			return (true);
			}
		}
	
	else if (h2 != nil) { /*try using h2 for result*/
		
		if (resizehandle (h2, sizemergedhandle)) {
			
			p = (ptrbyte) *h2;
			
			moveright (p, p + sizeof (long) + sizefirsthandle, sizesecondhandle);
			
			moveleft (&storesizefirsthandle, p, sizeof (long));
			
			if (h1 != nil)
				moveleft (*h1, p + sizeof (long), sizefirsthandle);
			
			*hmerged = h2;
			
			disposehandle (h1);
			
			return (true);
			}
		}
	
	/*resizing didn't work; try it the old way, using a newly-allocated handle*/
	
	h = getnewhandle (sizemergedhandle, true);
	
	if (h == nil) {
		
		memoryerror ();
		
		disposehandle (h1);
		
		disposehandle (h2);
		
		return (false);
		}
	
	p = (ptrbyte) *h;
	
	moveleft (&storesizefirsthandle, p, sizeof (long));
	
	p += sizeof (long);
	
	if (h1 != nil)
		moveleft (*h1, p, sizefirsthandle);
	
	if (h2 != nil)
		moveleft (*h2, p + sizefirsthandle, sizesecondhandle);
	
	*hmerged = h;
	
	disposehandle (h1);
	
	disposehandle (h2);
	
	return (true);
	} /*mergehandles*/


boolean unmergehandles (Handle hmerged, Handle *hfirst, Handle *hsecond) {
	
	/*
	split up a handle created by mergehandle.
	
	3/12/91 dmb: rewrote so that we own hmerged; caller no longer diposes it.  
	this allows us to reuse the handle to greatly reduce memory requirements.  
	we could further optimize by reusing hmerged for the larger handle instead 
	of always hsecond.
	
	also, avoid locking handles when we're trying to allocate potentially 
	large chunks of memory.
	
	2.1b3 dmb: newly-created handle is always ok as temporary memory
	*/
	
	register Handle h1 = nil, h2 = nil;
	register Handle h = hmerged;
	long ix;
	long sizefirsthandle, sizesecondhandle;
	
	*hfirst = *hsecond = nil; /*default return values*/
	
	moveleft (*h, &sizefirsthandle, sizeof (long));

#ifdef PACKFLIPPED
	longswap (sizefirsthandle);
#endif
	
	ix = sizeof (long);
	
	if (sizefirsthandle == 0)
		h1 = nil;
		
	else {
		h1 = getnewhandle (sizefirsthandle, true);
		
		if (h1 == nil) 
			goto error;
		
		moveleft (*h + ix, *h1, sizefirsthandle);
		}
	
	ix += sizefirsthandle;
	
	sizesecondhandle = gethandlesize (h) - sizefirsthandle - sizeof (long);
	
	if (sizesecondhandle == 0) {
		
		h2 = nil;
		
		disposehandle (h);
		}
	else {
		
		h2 = h; /*second handle can safely re-use merged handle*/
		
		moveleft (*h2 + ix, *h2, sizesecondhandle);
		
		sethandlesize (h2, sizesecondhandle);
		}
	
	*hfirst = h1; /*return handles to caller*/
	
	*hsecond = h2;
	
	return (true);
		
	error:
	
	disposehandle (h);
	
	disposehandle (h1);
		
	disposehandle (h2);
	
	memoryerror ();
	
	return (false);
	} /*unmergehandles*/
	
	
boolean newintarray (short ct, hdlintarray *harray) {
	
	Handle h;
	
	if (!newclearhandle (sizeof (short) * ct, &h))
		return (false);
		
	*harray = (hdlintarray) h;
	
	return (true);
	} /*newintarray*/
#endif

boolean setintarray (hdlintarray harray, short ix, short val) {
	
	/*
	assign into a cell in a variable-size array of integers, 0-based index.
	
	return false if the array needed to be extended but there isn't enough
	memory to do it.
	*/
	
	register hdlintarray h = harray;

	if (!minhandlesize ((Handle) h, (ix + 1) * sizeof (short)))
		return (false);
	
	(*harray) [ix] = val;
	
	return (true);
	} /*setintarray*/
	
	
boolean getintarray (hdlintarray harray, short ix, short *val) {
	
	*val = (*harray) [ix];
	
	return (true);
	} /*getintarray*/
	
	
void fillintarray (hdlintarray harray, short val) {
	
	register hdlintarray h = harray;
	register short ct;
	/*
	register short i;
	*/
	
	ct = (short) gethandlesize ((Handle) h) / sizeof (short);

	while (--ct	>= 0)
		(*h) [ct] = val;

	/*
	lockhandle ((Handle) h);
	
	for (i = 0; i < ct; i++)
		(*h) [i] = x;

	unlockhandle ((Handle) h);
	*/

	} /*fillintarray*/

#if (flruntime==0)
#if (odbengine==0)
boolean initmemory (void) {
	
	shellpushmemoryhook (&safetycushionhook);
	
	return (getsafetycushion ());
	} /*initmemory*/
#endif
#endif







