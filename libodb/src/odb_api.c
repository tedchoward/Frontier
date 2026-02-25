#include "odb_engine.h"
#include "odb_error_internal.h"
#include "odb_errors.h"

#include <stdio.h>
#include <string.h>

/* Internal file helpers. */
hdlfilenum odbFileOpen(const char *path, const char *mode);
void odbFileClose(hdlfilenum file);

static odbBool odb_fail_not_implemented(const char *fn) {
	char message[128];
	snprintf(message, sizeof(message), "%s not implemented", fn);
	odbSetLastError(ODB_ERR_NOT_IMPLEMENTED, message);
	return 0;
}

odbBool odbOpenPath(const char *path, odbRef *odb) {
	if (!path || !odb) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlfilenum file = odbFileOpen(path, "rb+");
	if (!file) {
		return 0;
	}
	odbBool ok = odbOpenFile(file, odb);
	if (!ok) {
		odbFileClose(file);
		return 0;
	}
	return 1;
}

odbBool odbNewPath(const char *path) {
	if (!path) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlfilenum file = odbFileOpen(path, "wb+");
	if (!file) {
		return 0;
	}
	odbBool ok = odbNewFile(file);
	odbFileClose(file);
	return ok;
}

odbBool odbNewFile(hdlfilenum file) {
	( void )file;
	return odb_fail_not_implemented("odbNewFile");
}

odbBool odbOpenFile(hdlfilenum file, odbRef *odb) {
	( void )file;
	( void )odb;
	return odb_fail_not_implemented("odbOpenFile");
}

odbBool odbSaveFile(odbRef odb) {
	( void )odb;
	return odb_fail_not_implemented("odbSaveFile");
}

odbBool odbCloseFile(odbRef odb) {
	( void )odb;
	return odb_fail_not_implemented("odbCloseFile");
}

odbBool odbDefined(odbRef odb, odbString bspath) {
	( void )odb;
	( void )bspath;
	return odb_fail_not_implemented("odbDefined");
}

odbBool odbDelete(odbRef odb, odbString bspath) {
	( void )odb;
	( void )bspath;
	return odb_fail_not_implemented("odbDelete");
}

odbBool odbGetType(odbRef odb, odbString bspath, OSType *type) {
	( void )odb;
	( void )bspath;
	( void )type;
	return odb_fail_not_implemented("odbGetType");
}

odbBool odbGetValue(odbRef odb, odbString bspath, odbValueRecord *value) {
	( void )odb;
	( void )bspath;
	( void )value;
	return odb_fail_not_implemented("odbGetValue");
}

odbBool odbSetValue(odbRef odb, odbString bspath, odbValueRecord *value) {
	( void )odb;
	( void )bspath;
	( void )value;
	return odb_fail_not_implemented("odbSetValue");
}

odbBool odbNewTable(odbRef odb, odbString bspath) {
	( void )odb;
	( void )bspath;
	return odb_fail_not_implemented("odbNewTable");
}

odbBool odbCountItems(odbRef odb, odbString bspath, int32_t *count) {
	( void )odb;
	( void )bspath;
	( void )count;
	return odb_fail_not_implemented("odbCountItems");
}

odbBool odbGetNthItem(odbRef odb, odbString bspath, int32_t n, odbString bsname) {
	( void )odb;
	( void )bspath;
	( void )n;
	( void )bsname;
	return odb_fail_not_implemented("odbGetNthItem");
}

odbBool odbGetModDate(odbRef odb, odbString bspath, uint32_t *date) {
	( void )odb;
	( void )bspath;
	( void )date;
	return odb_fail_not_implemented("odbGetModDate");
}

void odbDisposeValue(odbRef odb, odbValueRecord *value) {
	( void )odb;
	( void )value;
	odbSetLastError(ODB_ERR_NOT_IMPLEMENTED, "odbDisposeValue not implemented");
}
