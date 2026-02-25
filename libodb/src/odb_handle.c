#include "odb_engine.h"
#include "odb_error_internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Classic Handle layout:
 * [header:size_t size][data...]
 * Handle points to a heap pointer to data.
 */

typedef struct odb_handle_header {
	size_t size;
} odb_handle_header;

static odb_handle_header *odb_handle_header_from_data(void *data) {
	if (!data) {
		return NULL;
	}
	return (odb_handle_header *)((unsigned char *)data - sizeof(odb_handle_header));
}

static void *odb_handle_data_from_header(odb_handle_header *header) {
	if (!header) {
		return NULL;
	}
	return (void *)((unsigned char *)header + sizeof(odb_handle_header));
}

Handle odbNewHandle(size_t size) {
	odb_handle_header *header = (odb_handle_header *)calloc(1, sizeof(odb_handle_header) + size);
	if (!header) {
		odbSetLastError(ODB_ERR_IO, "out of memory");
		return NULL;
	}
	header->size = size;
	void *data = odb_handle_data_from_header(header);
	Handle h = (Handle)calloc(1, sizeof(void *));
	if (!h) {
		free(header);
		odbSetLastError(ODB_ERR_IO, "out of memory");
		return NULL;
	}
	*h = data;
	fprintf(stderr, "odbNewHandle: size=%zu h=%p data=%p header=%p\n",
		size, (void *)h, data, (void *)header);
	return h;
}

void odbDisposeHandle(Handle h) {
	if (!h) {
		return;
	}
	void *data = *h;
	odb_handle_header *header = odb_handle_header_from_data(data);
	free(header);
	free(h);
}

size_t odbGetHandleSize(Handle h) {
	if (!h || !*h) {
		return 0;
	}
	odb_handle_header *header = odb_handle_header_from_data(*h);
	return header ? header->size : 0;
}

odbBool odbSetHandleSize(Handle h, size_t new_size) {
	if (!h || !*h) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid handle");
		return 0;
	}
	odb_handle_header *header = odb_handle_header_from_data(*h);
	if (!header) {
		odbSetLastError(ODB_ERR_INTERNAL, "invalid handle header");
		return 0;
	}
	size_t total = sizeof(odb_handle_header) + new_size;
	odb_handle_header *resized = (odb_handle_header *)realloc(header, total);
	if (!resized) {
		odbSetLastError(ODB_ERR_IO, "out of memory");
		return 0;
	}
	if (new_size > resized->size) {
		unsigned char *data = (unsigned char *)odb_handle_data_from_header(resized);
		memset(data + resized->size, 0, new_size - resized->size);
	}
	resized->size = new_size;
	*h = odb_handle_data_from_header(resized);
	return 1;
}

void odbLockHandle(Handle h) {
	(void)h;
}

void odbUnlockHandle(Handle h) {
	(void)h;
}
