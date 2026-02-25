#include "odb_engine.h"
#include "odb_errors.h"

#include <stdio.h>
#include <string.h>

static void print_usage(void) {
	printf("usage:\n");
	printf("  odb_cli <dbpath> list <path>\n");
}

static void odb_string_from_c(odbString out, const char *msg) {
	if (!out) {
		return;
	}
	if (!msg) {
		out[0] = 0;
		return;
	}
	size_t len = strlen(msg);
	if (len > 255) {
		len = 255;
	}
	out[0] = (unsigned char)len;
	if (len > 0) {
		memcpy(&out[1], msg, len);
	}
}

static int cmd_list(odbRef odb, const char *path) {
	odbString bspath;
	odb_string_from_c(bspath, path);
	int32_t count = 0;
	if (!odbCountItems(odb, bspath, &count)) {
		return 1;
	}
	for (int32_t i = 1; i <= count; ++i) {
		odbString name;
		if (!odbGetNthItem(odb, bspath, i, name)) {
			return 1;
		}
		printf("%.*s\n", name[0], name + 1);
	}
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 4) {
		print_usage();
		return 1;
	}
	const char *dbpath = argv[1];
	const char *command = argv[2];
	const char *path = argv[3];
	odbRef odb = NULL;
	if (!odbOpenPath(dbpath, &odb)) {
		odbString err;
		odbGetError(err);
		printf("odbOpenPath failed (code=%d): %.*s\n", odbGetLastErrorCode(), err[0], err + 1);
		return 1;
	}
	int rc = 0;
	if (strcmp(command, "list") == 0) {
		rc = cmd_list(odb, path);
	} else {
		print_usage();
		rc = 1;
	}
	if (rc != 0) {
		odbString err;
		odbGetError(err);
		printf("command failed (code=%d): %.*s\n", odbGetLastErrorCode(), err[0], err + 1);
	}
	odbCloseFile(odb);
	return rc;
}
