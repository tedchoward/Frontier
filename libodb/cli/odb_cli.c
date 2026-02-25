#include "odb_engine.h"
#include "odb_errors.h"

#include <stdio.h>
#include <string.h>

static void print_usage(void) {
	printf("usage: odb_cli <path>\n");
}

int main(int argc, char **argv) {
	if (argc != 2) {
		print_usage();
		return 1;
	}
	const char *path = argv[1];
	odbRef odb = NULL;
	if (!odbOpenPath(path, &odb)) {
		odbString err;
		odbGetError(err);
		printf("odbOpenPath failed (code=%d): %.*s\n", odbGetLastErrorCode(), err[0], err + 1);
		return 1;
	}
	printf("odbOpenPath succeeded (stub implementation)\n");
	odbCloseFile(odb);
	return 0;
}
