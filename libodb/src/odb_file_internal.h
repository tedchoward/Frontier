#ifndef ODB_FILE_INTERNAL_H
#define ODB_FILE_INTERNAL_H

#include "odb_engine.h"

#include <stdint.h>
#include <stdio.h>

struct odb_file {
	FILE *fp;
};

hdlfilenum odbFileOpen(const char *path, const char *mode);
void odbFileClose(hdlfilenum file);
odbBool odbFileRead(hdlfilenum file, void *buffer, size_t bytes);
odbBool odbFileWrite(hdlfilenum file, const void *buffer, size_t bytes);
odbBool odbFileSeek(hdlfilenum file, int64_t offset);
odbBool odbFileTell(hdlfilenum file, int64_t *out);
odbBool odbFileGetSize(hdlfilenum file, int64_t *out);
odbBool odbFileSetEof(hdlfilenum file, int64_t size);
odbBool odbFileFlush(hdlfilenum file);

#endif
