#include "odb_file_ops.h"
#include "odb_file_internal.h"

#include <stdio.h>

boolean filegeteof(hdlfilenum fnum, long *eof) {
	int64_t size = 0;
	if (!eof) {
		return false;
	}
	if (!odbFileGetSize(fnum, &size)) {
		return false;
	}
	*eof = (long)size;
	return true;
}

boolean fileseteof(hdlfilenum fnum, long eof) {
	return odbFileSetEof(fnum, (int64_t)eof);
}

boolean filesetposition(hdlfilenum fnum, long position) {
	return odbFileSeek(fnum, (int64_t)position);
}

boolean filewrite(hdlfilenum fnum, long ctbytes, void *pdata) {
	return odbFileWrite(fnum, pdata, (size_t)ctbytes);
}

boolean fileread(hdlfilenum fnum, long ctbytes, void *pdata) {
	return odbFileRead(fnum, pdata, (size_t)ctbytes);
}

boolean fileflush(hdlfilenum fnum) {
	return odbFileFlush(fnum);
}
