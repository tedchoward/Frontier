
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletmemory.h"
#include "appletstrings.h"
#include "appletsyserror.h"


boolean usetempmemory = false; /*can be set externally to force allocations into temporary memory*/




void moveleft (void *psource, void *pdest, long length) {
	
	/*
	do a mass memory move with the left edge leading.  good for closing
	up a gap in a buffer, among other thingsÉ
	*/
	
	char *ps, *pd;
	long ctloops;
	
	ctloops = length;
	
	if (ctloops > 0) {
	
		ps = psource; /*copy into a register*/
	
		pd = pdest; /*copy into a register*/
	
		while (ctloops--) *pd++ = *ps++;
		}
	} /*moveleft*/
	
	
void moveright (void *psource, void *pdest, long length) {
	
	/*
	do a mass memory move with the right edge leading.  good for opening
	up a gap in a buffer, among other thingsÉ
	*/
	
	char *ps, *pd;
	long ctloops;
	
	ctloops = length;
	
	if (ctloops > 0) {
	
		ps = (char *) psource + length - 1; /*right edge of source*/
	
		pd = (char *) pdest + length - 1; /*right edge of destination*/
	
		while (ctloops--) *pd-- = *ps--;
		}
	} /*moveright*/
	
	
boolean equalmemory (void *v1, void *v2, long ct) { 
	
	/*
	return true if the bytes are identical. useful for comparing records,
	remember to clearbyte them before copying data into the records.
	garbage bytes can make this test return false, when logically the
	records contain the same information.
	*/
	
	char *p1 = (char *) v1;
	char *p2 = (char *) v2;
	
	if (ct <= 0)
		return (true);
		
	while (ct--) 
		if (*p1++ != *p2++)
			return (false);
		
	return (true);
	} /*equalmemory*/
	

boolean equalhandles (Handle h1, Handle h2) {
	
	unsigned long len;
	
	if (h1 == nil)
		return (h2 == nil);
		
	if (h2 == nil)
		return (false);
		
	len = GetHandleSize (h1);
	
	if (GetHandleSize (h2) != len)
		return (false);
		
	return (equalmemory (*h1, *h2, len));
	} /*equalhandles*/
	
	
void fillchar (void *pfill, long ctfill, char chfill) {
	
	/*
	do a mass memory fill -- copy ctfill chfills at pfill.
	*/
	
	char *p = pfill;
	long ct = ctfill;
	char ch = chfill;
	
	assert (ct >= 0);
	
	while (ct--) *p++ = (char) ch; /*tight loop*/
	} /*fillchar*/
	

void clearbytes (void *pclear, long ctclear) {
	
	/*
	fill memory with 0's.
	*/
	
	fillchar (pclear, ctclear, (char) 0);
	} /*clearbytes*/
	

void disposehandle (Handle h) {
	
	/*
	our own bottleneck for this built-in.  we don't require type coercion
	and we check if its nil.
	*/
	
	if (h != nil)
		DisposHandle (h);
	} /*disposehandle*/
	
	
long gethandlesize (Handle h) {
	
	if (h == nil)
		return (0);
		
	return (GetHandleSize (h));
	} /*gethandlesize*/
	
	
boolean sethandlesize (Handle h, long size) {
	
	SetHandleSize (h, size);
	
	return (MemError () == noErr);
	} /*gethandlesize*/
	
	
boolean getnewhandle (unsigned long ctbytes, Handle *h) {
	
	boolean fl;
	
	if (usetempmemory) {
		
		OSErr ec;
		
		*h = TempNewHandle (ctbytes, &ec);
		
		fl = ec == noErr;
		}
	else {
		*h = NewHandle (ctbytes);
		
		fl = *h != nil;
		}
		
	if (!fl)
		sysmemoryerror ();
		
	return (fl);
	} /*getnewhandle*/
	
	
boolean newemptyhandle (Handle *h) {
	
	return (getnewhandle (0, h));
	} /*newemptyhandle*/
	
	
Handle appnewhandle (Size ctbytes) {
	
	/*
	this api is designed to easily replace calls to NewHandle.
	*/
	
	Handle h;
	
	if (!getnewhandle (ctbytes, &h))
		return (nil);
		
	return (h);
	} /*appnewhandle*/
	
	
unsigned long memavail (void) {
	
	if (usetempmemory) 
		return (TempFreeMem ());
		
	return (FreeMem ());
	} /*memavail*/
	
	
boolean copyhandle (Handle horig, Handle *hcopy) {
	
	unsigned long ctbytes;
	
	if (horig == nil) {
		
		*hcopy = nil;
		
		return (true);
		}
	
	ctbytes = GetHandleSize (horig);
	
	if (!getnewhandle (ctbytes, hcopy))
		return (false);
	
	moveleft (*horig, **hcopy, ctbytes);
	
	return (true);
	} /*copyhandle*/
	

boolean enlargehandle (Handle hgrow, long ctgrow, void *newdata) {
	
	/*
	make the handle big enough to hold the new data, and move the new data in
	at the end of the newly enlarged handle.
	
	8/23/94 DW: check if ctgrow <= 0, drive defensively.
	*/
	
	Handle h = hgrow;
	long ct = ctgrow;
	ptrchar p = newdata;
	long origsize;
	
	if (ctgrow <= 0) /*defensive driving*/
		return (true);
	
	origsize = gethandlesize (h);
	
	sethandlesize (h, origsize + ct);
		
	if (MemError () != noErr)
		return (false);
		
	moveleft (p, *h + origsize, ct);
	
	return (true);
	} /*enlargehandle*/ 
	
	
boolean pushathandlestart (ptrvoid pdata, long ctbytes, Handle h) {
	
	unsigned long origsize = gethandlesize (h);
	
	if (!sethandlesize (h, origsize + ctbytes))
		return (false);
	
	moveright (*h, *h + ctbytes, origsize);
	
	moveleft (pdata, *h, ctbytes);
	
	return (true);
	} /*pushathandlestart*/
	
	
boolean loadfromhandle (Handle hload, long *ixload, long ctload, void *pdata) {
	
	/*
	copy the next ctload bytes from hload into pdata and increment the index.
	
	return false if there aren't enough bytes.
	
	start ixload at 0.
	*/
	
	Handle h = hload;
	ptrchar p = pdata;
	long ct = ctload;
	long ix = *ixload;
	long size;
	
	size = gethandlesize (h);
	
	if ((ix + ct) > size) /*asked for more bytes than there are*/
		return (false); 
		
	moveleft (*h + ix, p, ct); /*copy out of the handle*/
	
	*ixload = ix + ct; /*increment the index into the handle*/
	
	return (true);
	} /*loadfromhandle*/
	

boolean loadhandlefromhandle (Handle hsource, long *ixsource, long ct, Handle *hdest) {
	
	boolean fl;
	
	fl = newfilledhandle (&(*hsource) [*ixsource], ct, hdest);
	
	*ixsource += ct;
	
	return (fl);
	} /*loadhandlefromhandle*/


#define newstringlength(bs) ((unsigned char) (bs [0]))


boolean newtexthandle (void *pstring, Handle *htext) {
	
	/*
	create a new handle to hold the text of the string.
	
	if the string is "\pABC" -- you get a handle of size 3.
	
	DW 1/21/95: ooops -- this guy couldn't handle strings 
	that are longer than 128 characters. yow. gotta change
	the definition of stringlength.
	*/
	
	char *bs = (char *) pstring;
	unsigned long len;
	
	len = newstringlength (bs);
	
	if (!getnewhandle (len, htext))
		return (false);
		
	if (len > 0)
		moveleft (&bs [1], **htext, len);
	
	return (true);
	} /*newtexthandle*/


boolean pushtexthandle (void *pstring, Handle htext) {
	
	/*
	htext is a handle created with newtexthandle.
	
	increase the size of the handle so we can push the text of bs at 
	the end of the handle (not including length byte).
	*/
	
	unsigned char *bs = (unsigned char *) pstring;
	
	return (enlargehandle (htext, (unsigned long) stringlength (bs), (ptrchar) bs + 1));
	} /*pushtexthandle*/


boolean pushhandleonhandle (Handle hsource, Handle hdest) {
	
	boolean fl;
	
	if (hsource == nil)
		return (true);
	
	lockhandle (hsource);
	
	fl = enlargehandle (hdest, gethandlesize (hsource), *hsource);
	
	unlockhandle (hsource);
	
	return (fl);
	} /*pushhandleonhandle*/


void texthandletostring (Handle htext, void *pstring) {
	
	char *bs = (char *) pstring;
	long len;
	
	if (htext == nil) {
		
		setstringlength (bs, 0);
		
		return;
		}
	
	len = gethandlesize (htext);
	
	if (len > lenbigstring)
		len = lenbigstring;
	
	setstringlength (bs, len);
	
	moveleft (*htext, &bs [1], len);
	} /*texthandletostring*/
	
	
boolean numbertotexthandle (long num, Handle *htext) {
	
	bigstring bs;
	
	NumToString (num, bs);
	
	return (newtexthandle (bs, htext));
	} /*numbertotexthandle*/
	
	
void texthandletonumber (Handle htext, long *num, boolean *flerror) {
	
	bigstring bs;
	
	texthandletostring (htext, bs);
	
	stringtonumber (bs, num, flerror);
	} /*texthandletonumber*/
	
	
void lockhandle (Handle h) {
	
	if (h != nil)
		HLock (h);
	} /*lockhandle*/
	
	
void unlockhandle (Handle h) {
	
	if (h != nil)
		HUnlock (h);
	} /*unlockhandle*/
	
	
boolean newclearhandle (long ctbytes, Handle *hnew) {
	
	if (!getnewhandle (ctbytes, hnew))
		return (false);
			
	clearbytes (**hnew, ctbytes);
	
	return (true);
	} /*newclearhandle*/
	
	
boolean newfilledhandle (ptrvoid pdata, long ctbytes, Handle *hnew) {
	
	if (!getnewhandle (ctbytes, hnew))
		return (false);
			
	moveleft (pdata, **hnew, ctbytes);
	
	return (true);
	} /*newfilledhandle*/
	
	
boolean newheapstring (ptrstring bs, hdlstring *hstring) {

	return (newfilledhandle (bs, (long) stringlength (bs) + 1, (Handle *) hstring));
	} /*newheapstring*/
	
	
void copyheapstring (hdlstring hstring, void *bs) {
	
	if (hstring == nil) {
		
		((char *) bs) [0] = 0;
		
		return;
		}
	
	lockhandle ((Handle) hstring);
	
	copystring (*hstring, bs);
	
	unlockhandle ((Handle) hstring);
	} /*copyheapstring*/
	

boolean sethandlecontents (void *p, long ct, Handle h) {

	sethandlesize (h, ct);
	
	if (MemError () != noErr)
		return (false);
	
	moveleft (p, *h, ct);
	
	return (true);
	} /*sethandlecontents*/


boolean minhandlesize (Handle h, long size) {
	
	if (gethandlesize (h) >= size) /*already big enough*/
		return (true);
		
	sethandlesize (h, size); /*try to make it larger*/
	
	return (MemError () == noErr);
	} /*minhandlesize*/
	
	
boolean newintarray (short ct, hdlintarray *harray) {
	
	Handle h;
	
	if (!newclearhandle (longsizeof (short) * ct, &h))
		return (false);
		
	*harray = (hdlintarray) h;
	
	return (true);
	} /*newintarray*/
	

boolean setintarray (hdlintarray harray, short ix, short val) {
	
	/*
	assign into a cell in a variable-size array of integers, 0-based index.
	
	return false if the array needed to be extended but there isn't enough
	memory to do it.
	*/
	
	hdlintarray h = harray;

	if (!minhandlesize ((Handle) h, (ix + 1) * longsizeof (short)))
		return (false);
	
	(*harray) [ix] = val;
	
	return (true);
	} /*setintarray*/
	
	
boolean getintarray (hdlintarray harray, short ix, short *val) {
	
	*val = (*harray) [ix];
	
	return (true);
	} /*getintarray*/
	
	
void fillintarray (hdlintarray harray, short val) {
	
	hdlintarray h = harray;
	short x = val;
	short ct;
	
	ct = gethandlesize ((Handle) h) / longsizeof (short);

	while (--ct	>= 0)
		(*h) [ct] = x;
	} /*fillintarray*/


boolean testheapspace (long size) {
	
	/*
	see if there's enough space in the heap to allocate a handle of the given size.
	
	if not, generate an error, and return false.
	*/
	
	Handle htest;
	
	htest = appnewhandle (size);
	
	if (htest == nil) 
		return (false);
	
	DisposHandle (htest);
	
	return (true);
	} /*testheapspace*/


boolean popfromhandle (Handle hpop, long ctpop, ptrvoid pdata) {
	
	Handle h = hpop;
	long ixpop = gethandlesize (h) - ctpop;
	
	if (ixpop < 0)
		return (false);
	
	if (pdata != nil)
		moveleft (*h + ixpop, pdata, ctpop); /*copy out of the handle*/
	
	return (sethandlesize (h, ixpop)); /*should never fail -- getting smaller*/
	} /*popfromhandle*/


boolean insertinhandle (bigstring bs, Handle h, unsigned long ix) {
	
	short stringlen = stringlength (bs);
	unsigned long ct = gethandlesize (h);
	unsigned long newlen = ct + stringlen;
	
	if (!sethandlesize (h, newlen))
		return (false);
	
	moveright (&(*h) [ix], &(*h) [ix + stringlen], ct - ix);
	
	moveleft (&bs [1], &(*h) [ix], stringlen);
	
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
	
	
void deleteinhandle (Handle h, unsigned long ix, unsigned long ctdelete) {
	
	unsigned long origct = gethandlesize (h);
	unsigned long ct = origct - ctdelete;
	
	moveleft (&(*h) [ix + ctdelete], &(*h) [ix], ct - ix);
	
	sethandlesize (h, ct);
	} /*deleteinhandle*/
	
	
static unsigned char lowerchar (unsigned char ch) {
	
	if ((ch >= 'A') && (ch <= 'Z'))
		return (ch + 32);
	
	return (ch);
	} /**lowerchar*/
	
	
boolean handlesearch (Handle h, bigstring searchfor, boolean flunicase, unsigned long *ix) {
	
	unsigned long ct, i, j;
	unsigned char ch, chfirst;
	unsigned long remainingchars;
	short lensearchfor = stringlength (searchfor);
	bigstring bscopy;
	
	if (lensearchfor == 0) /*defensive driving*/
		return (false);
		
	copystring (searchfor, bscopy);
	
	if (flunicase)
		alllower (bscopy);
	
	chfirst = bscopy [1];
	
	ct = gethandlesize (h);
	
	for (i = *ix; i < ct; i++) {
		
		ch = (*h) [i];
		
		if (flunicase)
			ch = lowerchar (ch);
		
		remainingchars = ct - i - 1;
		
		if (remainingchars < lensearchfor)
			break;
		
		if (ch == chfirst) {
			
			boolean flmatch = true;
			
			for (j = 2; j <= lensearchfor; j++) {
				
				ch = (*h) [i + j - 1];
				
				if (flunicase)
					ch = lowerchar (ch);
					
				if (ch != bscopy [j]) {
					
					flmatch = false;
					
					break;
					}
				} /*for*/
				
			if (flmatch) {
				
				*ix = i;
				
				return (true);
				}
			}
		} /*for*/
	
	return (false);
	} /*handlesearch*/
	
	
boolean handlereplaceall (Handle h, bigstring searchfor, Handle replacewith) {
	
	unsigned long ct, i, j;
	unsigned char ch, chfirst;
	unsigned long remainingchars;
	short lensearchfor = stringlength (searchfor);
	
	if (lensearchfor == 0) /*defensive driving*/
		return (true);
	
	chfirst = searchfor [1];
	
	ct = gethandlesize (h);
	
	for (i = 0; i < ct; i++) {
		
		ch = (*h) [i];
		
		remainingchars = ct - i - 1;
		
		if (remainingchars < lensearchfor)
			break;
		
		if (ch == chfirst) {
			
			boolean flmatch = true;
			
			for (j = 2; j <= lensearchfor; j++) {
				
				if ((*h) [i + j - 1] != searchfor [j]) {
					
					flmatch = false;
					
					break;
					}
				} /*for*/
				
			if (flmatch) {
				
				deleteinhandle (h, i, lensearchfor);
				
				if (!inserthandleinhandle (replacewith, h, i))
					return (false);
					
				i += gethandlesize (replacewith);
				
				ct = gethandlesize (h);
				}
			}
		} /*for*/
	
	return (true);
	} /*handlereplaceall*/
	
	
boolean getstringfromhandle (Handle h, unsigned long ixstart, unsigned long ixend, bigstring bs) {
	
	long ixload = ixstart;
	unsigned long len;
	
	len = ixend - ixstart + 1;
	
	if (len > lenbigstring)
		return (false);
	
	if (!loadfromhandle (h, &ixload, len, &bs [1]))
		return (false);
	
	setstringlength (bs, len);
	
	return (true);
	} /*getstringfromhandle*/
	

long handlepatternmatch (void *ppattern, Handle pstringh) {
	
	unsigned char *bspattern = (unsigned char *) ppattern;
	unsigned long lenpattern = stringlength (bspattern);
	StringPtr	bs = (StringPtr)(*pstringh);
	unsigned long lenstring = GetHandleSize(pstringh);
	unsigned long i, ix, ixstring = 1;
	byte chfirst;
	
	if ((lenstring == 0) || (lenpattern == 0))
		return (0);
	
	chfirst = bspattern [1];
	
	while (true) {
		
		if (bs [ixstring] == chfirst) { /*matched at least first character in string*/
			
			for (i = 2; i <= lenpattern; i++) {
				
				ix = ixstring + i - 1;
				
				if (ix > lenstring) /*gone off end of string, can't match*/
					return (0);
				
				if (bs [ix] != bspattern [i]) 
					goto L1;
				} /*for*/
			
			return (ixstring); /*loop terminated, full match*/
			}
		
		L1: /*advance to next character in string*/
		
		if (++ixstring > lenstring) /*reached end of string, not found*/
			return (0);
		} /*while*/
	} /*patternmatch*/

