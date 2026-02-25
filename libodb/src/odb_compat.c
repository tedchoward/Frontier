#include "odb_compat.h"
#include "odb_error_internal.h"
#include "odb_byteorder.h"

#include <stdlib.h>

Handle odbNewHandle(size_t size);
void odbDisposeHandle(Handle h);
size_t odbGetHandleSize(Handle h);
odbBool odbSetHandleSize(Handle h, size_t size);
void odbLockHandle(Handle h);
void odbUnlockHandle(Handle h);

boolean newhandle(long size, Handle *h) {
	if (!h) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle output");
		return false;
	}
	*h = odbNewHandle((size_t)size);
	return *h != NULL;
}

boolean newemptyhandle(Handle *h) {
	return newhandle(0, h);
}

boolean newclearhandle(long size, Handle *h) {
	if (!newhandle(size, h)) {
		return false;
	}
	if (size > 0 && *h && **h) {
		memset(**h, 0, (size_t)size);
	}
	return true;
}

boolean newfilledhandle(ptrvoid data, long size, Handle *h) {
	if (!newhandle(size, h)) {
		return false;
	}
	if (size > 0 && data && *h && **h) {
		memcpy(**h, data, (size_t)size);
	}
	return true;
}

boolean copyhandle(Handle source, Handle *dest) {
	if (!dest) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle output");
		return false;
	}
	if (!source) {
		*dest = NULL;
		return true;
	}
	long size = gethandlesize(source);
	if (!newhandle(size, dest)) {
		return false;
	}
	if (size > 0 && *dest && **dest) {
		memcpy(**dest, *source, (size_t)size);
	}
	return true;
}

boolean enlargehandle(Handle h, long ctgrow, ptrvoid newdata) {
	if (!h) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle");
		return false;
	}
	if (ctgrow <= 0) {
		return true;
	}
	long oldsize = gethandlesize(h);
	if (!sethandlesize(h, oldsize + ctgrow)) {
		return false;
	}
	unsigned char *base = (unsigned char *)*h;
	if (newdata) {
		memcpy(base + oldsize, newdata, (size_t)ctgrow);
	} else {
		memset(base + oldsize, 0, (size_t)ctgrow);
	}
	return true;
}

void disposehandle(Handle h) {
	odbDisposeHandle(h);
}

long gethandlesize(Handle h) {
	return (long)odbGetHandleSize(h);
}

boolean sethandlesize(Handle h, long size) {
	return odbSetHandleSize(h, (size_t)size) ? true : false;
}

void lockhandle(Handle h) {
	odbLockHandle(h);
}

void unlockhandle(Handle h) {
	odbUnlockHandle(h);
}

boolean pushhandle(Handle hsource, Handle hdest) {
	if (!hdest) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid destination handle");
		return false;
	}
	if (!hsource) {
		return true;
	}
	long src = gethandlesize(hsource);
	if (src == 0) {
		return true;
	}
	long dst = gethandlesize(hdest);
	if (!sethandlesize(hdest, dst + src)) {
		return false;
	}
	memcpy((unsigned char *)*hdest + dst, *hsource, (size_t)src);
	return true;
}

boolean popfromhandle(Handle hpop, long ctpop, ptrvoid pdata) {
	if (!hpop || ctpop < 0) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle");
		return false;
	}
	long size = gethandlesize(hpop);
	if (ctpop > size) {
		odbSetLastError(ODB_ERR_IO, "pop beyond handle size");
		return false;
	}
	long start = size - ctpop;
	if (pdata && ctpop > 0) {
		memcpy(pdata, (unsigned char *)*hpop + start, (size_t)ctpop);
	}
	return sethandlesize(hpop, start);
}

boolean loadfromhandle(Handle hload, long *ixload, long ctload, ptrvoid pdata) {
	if (!hload || !ixload || ctload < 0) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle load");
		return false;
	}
	long size = gethandlesize(hload);
	if (*ixload + ctload > size) {
		odbSetLastError(ODB_ERR_IO, "load beyond handle size");
		return false;
	}
	if (ctload > 0 && pdata) {
		memcpy(pdata, (unsigned char *)*hload + *ixload, (size_t)ctload);
	}
	*ixload += ctload;
	return true;
}

boolean loadfromhandletohandle(Handle hload, long *ixload, long ctload, boolean fltemp, Handle *hnew) {
	(void)fltemp;
	if (!hnew) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle output");
		return false;
	}
	if (ctload < 0) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid load size");
		return false;
	}
	if (!newhandle(ctload, hnew)) {
		return false;
	}
	if (ctload == 0) {
		return true;
	}
	if (!*hnew || !**hnew) {
		disposehandle(*hnew);
		*hnew = NULL;
		return false;
	}
	if (!loadfromhandle(hload, ixload, ctload, **hnew)) {
		disposehandle(*hnew);
		*hnew = NULL;
		return false;
	}
	return true;
}

boolean mergehandles(Handle h1, Handle h2, Handle *hmerged) {
	if (!hmerged) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid merged handle output");
		return false;
	}
	long size1 = h1 ? gethandlesize(h1) : 0;
	long size2 = h2 ? gethandlesize(h2) : 0;
	long total = (long)sizeof(int32_t) + size1 + size2;
	if (!newhandle(total, hmerged)) {
		return false;
	}
	int32_t size1disk = size1;
	memtodisklong(size1disk);
	if (!*hmerged || !**hmerged) {
		return false;
	}
	memcpy(**hmerged, &size1disk, sizeof(size1disk));
	if (size1 > 0) {
		memcpy((unsigned char *)**hmerged + sizeof(size1disk), *h1, (size_t)size1);
	}
	if (size2 > 0) {
		memcpy((unsigned char *)**hmerged + sizeof(size1disk) + size1, *h2, (size_t)size2);
	}
	disposehandle(h1);
	disposehandle(h2);
	return true;
}

boolean unmergehandles(Handle hmerged, Handle *hfirst, Handle *hsecond) {
	if (!hmerged || !hfirst || !hsecond) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid unmerge handles");
		return false;
	}
	int32_t size1 = 0;
	if (!*hmerged) {
		return false;
	}
	memcpy(&size1, *hmerged, sizeof(size1));
	int32_t size1_disk = size1;
	disktomemlong(size1);
	long total = gethandlesize(hmerged);
	long size2 = total - (long)sizeof(int32_t) - size1;
	if (size1 < 0 || size2 < 0) {
		int32_t size1_alt = size1_disk;
		long size2_alt = total - (long)sizeof(int32_t) - size1_alt;
		if (size1_alt < 0 || size2_alt < 0) {
			odbSetLastError(ODB_ERR_IO, "invalid merged handle");
			return false;
		}
		size1 = size1_alt;
		size2 = size2_alt;
	}
	*hfirst = NULL;
	*hsecond = NULL;
	if (size1 > 0) {
		if (!newhandle(size1, hfirst)) {
			return false;
		}
		if (!*hfirst || !**hfirst) {
			return false;
		}
		memcpy(**hfirst, (unsigned char *)*hmerged + sizeof(int32_t), (size_t)size1);
	}
	if (size2 > 0) {
		if (!newhandle(size2, hsecond)) {
			disposehandle(*hfirst);
			*hfirst = NULL;
			return false;
		}
		if (!*hsecond || !**hsecond) {
			disposehandle(*hfirst);
			*hfirst = NULL;
			return false;
		}
		memcpy(**hsecond, (unsigned char *)*hmerged + sizeof(int32_t) + size1, (size_t)size2);
	}
	disposehandle(hmerged);
	return true;
}

boolean newheapstring(const bigstring bs, hdlstring *hstring) {
	if (!hstring) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle output");
		return false;
	}
	short len = stringlength(bs);
	if (!newhandle(len + 1, (Handle *)hstring)) {
		return false;
	}
	if (!*hstring || !**hstring) {
		return false;
	}
	memcpy(**hstring, bs, (size_t)len + 1);
	return true;
}

void copyheapstring(hdlstring hsource, bigstring bsdest) {
	if (hsource == NULL) {
		setemptystring(bsdest);
		return;
	}
	copystring(*hsource, bsdest);
}

void rollbeachball(void) {
	/* no-op in libodb */
}

void clearbytes(ptrvoid p, long ct) {
	if (p && ct > 0) {
		memset(p, 0, (size_t)ct);
	}
}

void openhandlestream(Handle h, handlestream *s) {
	if (!s) {
		return;
	}
	s->data = h;
	s->pos = 0;
	s->eof = h ? (long)gethandlesize(h) : 0;
	s->size = s->eof;
}

static boolean handlestream_grow(handlestream *s, long needed) {
	if (!s) {
		return false;
	}
	if (needed <= s->size) {
		return true;
	}
	long newsize = needed;
	if (!s->data) {
		if (!newhandle(newsize, &s->data)) {
			return false;
		}
	} else {
		if (!sethandlesize(s->data, newsize)) {
			return false;
		}
	}
	s->size = newsize;
	return true;
}

boolean writehandlestream(handlestream *s, void *p, long ct) {
	if (!s || ct < 0) {
		return false;
	}
	long end = s->pos + ct;
	if (!handlestream_grow(s, end)) {
		return false;
	}
	if (ct > 0 && p) {
		memcpy((unsigned char *)*s->data + s->pos, p, (size_t)ct);
	}
	s->pos = end;
	if (end > s->eof) {
		s->eof = end;
	}
	return true;
}

boolean mergehandlestreamdata(handlestream *s, long ctreplace, ptrvoid pdata, long ctmerge) {
	if (!s || ctreplace < 0 || ctmerge < 0) {
		return false;
	}
	long start = s->pos;
	long neweof = s->eof - ctreplace + ctmerge;
	if (!handlestream_grow(s, neweof)) {
		return false;
	}
	unsigned char *base = (unsigned char *)*s->data;
	if (ctreplace != ctmerge) {
		memmove(base + start + ctmerge, base + start + ctreplace, (size_t)(s->eof - (start + ctreplace)));
	}
	if (ctmerge > 0 && pdata) {
		memcpy(base + start, pdata, (size_t)ctmerge);
	}
	s->pos = start + ctmerge;
	s->eof = neweof;
	return true;
}

boolean pullfromhandlestream(handlestream *s, long ctpull, void *pdata) {
	if (!s || ctpull < 0) {
		return false;
	}
	if (s->pos + ctpull > s->eof) {
		return false;
	}
	if (ctpull > 0 && pdata) {
		memcpy(pdata, (unsigned char *)*s->data + s->pos, (size_t)ctpull);
	}
	s->pos += ctpull;
	return true;
}

Handle closehandlestream(handlestream *s) {
	if (!s) {
		return NULL;
	}
	if (s->data) {
		sethandlesize(s->data, s->eof);
	}
	Handle h = s->data;
	s->data = NULL;
	s->pos = 0;
	s->eof = 0;
	s->size = 0;
	return h;
}

void disposehandlestream(handlestream *s) {
	if (!s) {
		return;
	}
	disposehandle(s->data);
	s->data = NULL;
	s->pos = 0;
	s->eof = 0;
	s->size = 0;
}

boolean writehandlestreamhandle(handlestream *s, Handle h) {
	if (!h) {
		return true;
	}
	return writehandlestream(s, *h, gethandlesize(h));
}
