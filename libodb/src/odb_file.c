#include "odb_engine.h"
#include "odb_error_internal.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

struct odb_file {
	FILE *fp;
};

static odbBool odb_file_set_error(const char *prefix) {
	char buffer[256];
	if (prefix) {
		snprintf(buffer, sizeof(buffer), "%s: %s", prefix, strerror(errno));
		odbSetLastError(ODB_ERR_IO, buffer);
	} else {
		odbSetLastError(ODB_ERR_IO, strerror(errno));
	}
	return 0;
}

hdlfilenum odbFileOpen(const char *path, const char *mode) {
	FILE *fp = fopen(path, mode);
	if (!fp) {
		odb_file_set_error("fopen failed");
		return NULL;
	}
	struct odb_file *file = (struct odb_file *)calloc(1, sizeof(struct odb_file));
	if (!file) {
		fclose(fp);
		odbSetLastError(ODB_ERR_IO, "out of memory");
		return NULL;
	}
	file->fp = fp;
	return file;
}

void odbFileClose(hdlfilenum file) {
	if (!file) {
		return;
	}
	if (file->fp) {
		fclose(file->fp);
	}
	free(file);
}

odbBool odbFileRead(hdlfilenum file, void *buffer, size_t bytes) {
	if (!file || !file->fp) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid file");
		return 0;
	}
	if (bytes == 0) {
		return 1;
	}
	size_t read = fread(buffer, 1, bytes, file->fp);
	if (read != bytes) {
		return odb_file_set_error("fread failed");
	}
	return 1;
}

odbBool odbFileWrite(hdlfilenum file, const void *buffer, size_t bytes) {
	if (!file || !file->fp) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid file");
		return 0;
	}
	if (bytes == 0) {
		return 1;
	}
	size_t wrote = fwrite(buffer, 1, bytes, file->fp);
	if (wrote != bytes) {
		return odb_file_set_error("fwrite failed");
	}
	return 1;
}

odbBool odbFileSeek(hdlfilenum file, int64_t offset) {
	if (!file || !file->fp) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid file");
		return 0;
	}
#if defined(_WIN32)
	if (_fseeki64(file->fp, offset, SEEK_SET) != 0) {
		return odb_file_set_error("fseek failed");
	}
#else
	if (fseeko(file->fp, (off_t)offset, SEEK_SET) != 0) {
		return odb_file_set_error("fseek failed");
	}
#endif
	return 1;
}

odbBool odbFileTellEof(hdlfilenum file, int64_t *out) {
	if (!file || !file->fp || !out) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid file");
		return 0;
	}
#if defined(_WIN32)
	__int64 pos = _ftelli64(file->fp);
	if (pos < 0) {
		return odb_file_set_error("ftell failed");
	}
#else
	off_t pos = ftello(file->fp);
	if (pos < 0) {
		return odb_file_set_error("ftell failed");
	}
#endif
	*out = (int64_t)pos;
	return 1;
}

odbBool odbFileSetEof(hdlfilenum file, int64_t size) {
	if (!file || !file->fp) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid file");
		return 0;
	}
#if defined(_WIN32)
	if (_chsize_s(_fileno(file->fp), (size_t)size) != 0) {
		return odb_file_set_error("chsize failed");
	}
#else
	if (ftruncate(fileno(file->fp), (off_t)size) != 0) {
		return odb_file_set_error("ftruncate failed");
	}
#endif
	return 1;
}
