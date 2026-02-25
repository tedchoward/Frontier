#include "odb_db_internal.h"
#include "odb_compat.h"
#include "odb_file_internal.h"

#include <stdio.h>

#define ctwindowinfo 6

#pragma pack(push, 2)
typedef struct tycancoonwindowinfo {
	odbRect windowrect;
	unsigned char fontname[34];
	int16_t fontnum;
	int16_t fontsize;
	int16_t fontstyle;
	void *w;
	char waste[10];
} tycancoonwindowinfo;

typedef struct tyversion2cancoonrecord {
	int16_t versionnumber;
	dbaddress adrroottable;
	tycancoonwindowinfo windowinfo[ctwindowinfo];
	dbaddress adrscriptstring;
	uint16_t flags;
	int16_t ixprimaryagent;
	int16_t waste[28];
} tyversion2cancoonrecord;
#pragma pack(pop)

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: odb_dump <dbpath>\n");
		return 1;
	}
	const char *path = argv[1];
	hdlfilenum file = odbFileOpen(path, "rb");
	if (!file) {
		fprintf(stderr, "odbFileOpen failed\n");
		return 1;
	}
	if (!dbopenfile(file, false)) {
		fprintf(stderr, "dbopenfile failed\n");
		odbFileClose(file);
		return 1;
	}
	dbaddress adr = nildbaddress;
	dbgetview(cancoonview, &adr);
	int16_t versionnumber = 0;
	if (!dbreference(adr, sizeof(versionnumber), &versionnumber)) {
		fprintf(stderr, "dbreference version failed\n");
		dbdispose();
		odbFileClose(file);
		return 1;
	}
	disktomemshort(versionnumber);
	if (versionnumber != 3 && versionnumber != 2) {
		fprintf(stderr, "unsupported version %d\n", versionnumber);
		dbdispose();
		odbFileClose(file);
		return 1;
	}
	tyversion2cancoonrecord info;
	if (!dbreference(adr, sizeof(info), &info)) {
		fprintf(stderr, "dbreference record failed\n");
		dbdispose();
		odbFileClose(file);
		return 1;
	}
	disktomemlong(info.adrroottable);
	Handle hpacked = NULL;
	if (!dbrefhandle(info.adrroottable, &hpacked)) {
		fprintf(stderr, "dbrefhandle failed\n");
		dbdispose();
		odbFileClose(file);
		return 1;
	}
	long size = gethandlesize(hpacked);
	printf("packed size: %ld\n", size);
	unsigned char *data = *hpacked;
	long dump = size < 32 ? size : 32;
	for (long i = 0; i < dump; ++i) {
		printf("%02X ", data[i]);
	}
	printf("\n");
	disposehandle(hpacked);
	dbdispose();
	odbFileClose(file);
	return 0;
}
