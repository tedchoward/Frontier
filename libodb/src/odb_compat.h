#ifndef ODB_COMPAT_H
#define ODB_COMPAT_H

#include "odb_engine.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define nil 0

typedef unsigned char boolean;
typedef unsigned char byte;
typedef unsigned char *ptrstring;
typedef unsigned char **hdlstring;
typedef void *ptrvoid;
typedef unsigned char bigstring[256];

#define lenbigstring 255
#define longsizeof(x) ((long)sizeof(x))
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

typedef struct handlestream {
	Handle data;
	long pos;
	long eof;
	long size;
} handlestream;

typedef struct tyavailnodeshadow {
	long adr;
	long size;
} tyavailnodeshadow, **hdlavaillistshadow;

static inline short stringlength(const bigstring bs) {
	return (short)bs[0];
}

static inline void setstringlength(bigstring bs, short len) {
	bs[0] = (unsigned char)len;
}

static inline void setemptystring(bigstring bs) {
	bs[0] = 0;
}

static inline boolean isemptystring(const bigstring bs) {
	return bs[0] == 0;
}

static inline void copystring(const bigstring src, bigstring dst) {
	memcpy(dst, src, (size_t)src[0] + 1);
}

static inline char *stringbaseaddress(bigstring bs) {
	return (char *)&bs[1];
}

/* memory / handle helpers */
boolean newhandle(long size, Handle *h);
boolean newemptyhandle(Handle *h);
boolean newclearhandle(long size, Handle *h);
boolean newfilledhandle(ptrvoid data, long size, Handle *h);
boolean copyhandle(Handle source, Handle *dest);
boolean enlargehandle(Handle h, long ctgrow, ptrvoid newdata);
void disposehandle(Handle h);
long gethandlesize(Handle h);
boolean sethandlesize(Handle h, long size);
void lockhandle(Handle h);
void unlockhandle(Handle h);
boolean pushhandle(Handle hsource, Handle hdest);
boolean popfromhandle(Handle hpop, long ctpop, ptrvoid pdata);
boolean loadfromhandle(Handle hload, long *ixload, long ctload, ptrvoid pdata);
boolean loadfromhandletohandle(Handle hload, long *ixload, long ctload, boolean fltemp, Handle *hnew);
boolean mergehandles(Handle h1, Handle h2, Handle *hmerged);
boolean unmergehandles(Handle hmerged, Handle *hfirst, Handle *hsecond);

boolean newheapstring(const bigstring bs, hdlstring *hstring);
void copyheapstring(hdlstring hsource, bigstring bsdest);

void rollbeachball(void);

void clearbytes(ptrvoid p, long ct);

/* handlestream helpers */
void openhandlestream(Handle h, handlestream *s);
boolean writehandlestream(handlestream *s, void *p, long ct);
boolean mergehandlestreamdata(handlestream *s, long ctreplace, ptrvoid pdata, long ctmerge);
boolean pullfromhandlestream(handlestream *s, long ctpull, void *pdata);
Handle closehandlestream(handlestream *s);
void disposehandlestream(handlestream *s);
boolean writehandlestreamhandle(handlestream *s, Handle h);

#endif
