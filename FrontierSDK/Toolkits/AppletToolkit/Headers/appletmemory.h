
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifdef isFrontier

	/* TRT - 20 Mar 2005 - 10.1a2 - take advantage of Frontier memory management (debugging) */
	#include "memory.h"

	boolean pushhandleonhandle (Handle, Handle);

	boolean pushathandlestart (ptrvoid, long, Handle);

#elif !defined(appletmemoryinclude)
#define appletmemoryinclude


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif

#ifndef shelltypesinclude

	typedef short **hdlintarray;

#endif


boolean equalmemory (void *, void *, long);

boolean equalhandles (Handle, Handle);

void moveleft (void *, void *, long);

void moveright (void *, void *, long);

void clearbytes (void *, long);

boolean loadfromhandle (Handle, long *, long, void *);

boolean loadhandlefromhandle (Handle, long *, long, Handle *);

long gethandlesize (Handle);

boolean sethandlesize (Handle, long);

void disposehandle (Handle);

boolean newtexthandle (void *, Handle *);

boolean pushtexthandle (void *, Handle);

boolean pushhandleonhandle (Handle, Handle);

boolean enlargehandle (Handle, long, void *);

boolean pushathandlestart (ptrvoid, long, Handle);

boolean getnewhandle (unsigned long, Handle *h);

boolean newemptyhandle (Handle *);

Handle appnewhandle (Size);

unsigned long memavail (void);

boolean copyhandle (Handle, Handle *);

void texthandletostring (Handle, void *);

void lockhandle (Handle);

void unlockhandle (Handle);

boolean newheapstring (ptrstring, hdlstring *);

void copyheapstring (hdlstring, void *);

boolean newclearhandle (long, Handle *);

boolean newfilledhandle (ptrvoid, long, Handle *);

boolean sethandlecontents (void *, long, Handle);

boolean testheapspace (long);

boolean minhandlesize (Handle, long);

boolean newintarray (short, hdlintarray *);

boolean setintarray (hdlintarray, short, short);

boolean getintarray (hdlintarray, short, short *);

void fillintarray (hdlintarray, short);

boolean numbertotexthandle (long, Handle *);

void texthandletonumber (Handle, long *, boolean *);

boolean popfromhandle (Handle, long, ptrvoid);

boolean insertinhandle (bigstring, Handle, unsigned long);

boolean inserthandleinhandle (Handle, Handle, unsigned long);

void deleteinhandle (Handle, unsigned long, unsigned long);

boolean handlesearch (Handle, bigstring, boolean, unsigned long *);

boolean handlereplaceall (Handle, bigstring, Handle);

long handlepatternmatch (void *, Handle);

boolean getstringfromhandle (Handle, unsigned long, unsigned long, bigstring);

#endif
