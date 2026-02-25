#include "odb_compat.h"
#include "odb_error_internal.h"

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
	return newhandle(size, h);
}

boolean newfilledhandle(ptrvoid data, long size, Handle *h) {
	if (!newhandle(size, h)) {
		return false;
	}
	if (size > 0 && data) {
		memcpy(*h, data, (size_t)size);
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
	if (size > 0) {
		memcpy(*dest, *source, (size_t)size);
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

boolean newheapstring(const bigstring bs, hdlstring *hstring) {
	if (!hstring) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle output");
		return false;
	}
	short len = stringlength(bs);
	if (!newhandle(len + 1, (Handle *)hstring)) {
		return false;
	}
	memcpy(*hstring, bs, (size_t)len + 1);
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
