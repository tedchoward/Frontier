#include "odb_engine.h"
#include "odb_errors.h"
#include "odb_error_internal.h"

#include <string.h>

static _Thread_local odbErrorCode odb_last_error_code = ODB_ERR_NONE;
static _Thread_local char odb_last_error_message[256] = "";

static void odb_set_pascal_string(odbString out, const char *message) {
	if (!out) {
		return;
	}
	if (!message) {
		out[0] = 0;
		return;
	}
	unsigned char len = 0;
	while (message[len] != '\0' && len < 255) {
		out[len + 1] = (unsigned char)message[len];
		len++;
	}
	out[0] = len;
}

void odbSetLastError(odbErrorCode code, const char *message) {
	odb_last_error_code = code;
	if (message) {
		strncpy(odb_last_error_message, message, sizeof(odb_last_error_message) - 1);
		odb_last_error_message[sizeof(odb_last_error_message) - 1] = '\0';
	} else {
		odb_last_error_message[0] = '\0';
	}
}

const char *odbGetLastErrorMessage(void) {
	return odb_last_error_message;
}

int odbGetLastErrorCode(void) {
	return (int)odb_last_error_code;
}

void odbGetError(odbString bs) {
	odb_set_pascal_string(bs, odb_last_error_message);
}
