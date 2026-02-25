#include "odb_engine.h"
#include "odb_errors.h"
#include "odb_compat.h"
#include "legacy/odb_legacy.h"

#include <stdio.h>
#include <string.h>

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

static void print_handle_sizes(const char *label, Handle h) {
	long total = h ? gethandlesize(h) : 0;
	printf("%s total=%ld", label, total);
	if (!h || total < (long)sizeof(int32_t)) {
		printf("\n");
		return;
	}
	int32_t size1 = 0;
	memcpy(&size1, *h, sizeof(size1));
	disktomemlong(size1);
	long size2 = total - (long)sizeof(int32_t) - size1;
	printf(" size1=%ld size2=%ld\n", (long)size1, size2);
}

static Handle slice_handle(Handle h, long ix, long size) {
	if (!h || size < 0) {
		return NULL;
	}
	Handle out = NULL;
	if (!newhandle(size, &out)) {
		return NULL;
	}
	if (size > 0 && !loadfromhandle(h, &ix, size, *out)) {
		disposehandle(out);
		return NULL;
	}
	return out;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: odb_packinfo <dbpath>\n");
		return 1;
	}
	odbRef odb = NULL;
	if (!odbOpenPath(argv[1], &odb)) {
		odbString err;
		odbGetError(err);
		fprintf(stderr, "odbOpenPath failed (code=%d): %.*s\n", odbGetLastErrorCode(), err[0], err + 1);
		return 1;
	}
	odbString bspath;
	odbValueRecord value;
	odb_string_from_c(bspath, "root");
	if (!odbGetValue(odb, bspath, &value) || value.valuetype != tablevaluetype) {
		fprintf(stderr, "odbGetValue root failed\n");
		odbCloseFile(odb);
		return 1;
	}
	hdlexternalvariable hv = (hdlexternalvariable)value.data.externalvalue;
	if (!hv) {
		fprintf(stderr, "root external handle missing\n");
		odbCloseFile(odb);
		return 1;
	}
	hdlhashtable ht = (hdlhashtable)(**hv).variabledata;
	if (!ht) {
		fprintf(stderr, "root hashtable missing\n");
		odbCloseFile(odb);
		return 1;
	}
	long count = 0;
	hashcountitems(ht, &count);
	printf("root count=%ld\n", count);
	bigstring bsname;
	odb_string_from_c(bsname, "workspace");
	tyvaluerecord val;
	hdlhashnode hnode = NULL;
	if (hashtablelookup(ht, bsname, &val, &hnode)) {
		if (val.valuetype == ty_externalvaluetype) {
			Handle hexternal = NULL;
			boolean flnew = false;
			if (langexternalpack((hdlexternalhandle)val.data.externalvalue, &hexternal, &flnew)) {
				long esize = hexternal ? gethandlesize(hexternal) : 0;
				printf("external packed size=%ld bytes\n", esize);
				if (hexternal && esize > 0) {
					unsigned char *p = *hexternal;
					long dump = esize < 16 ? esize : 16;
					printf("external bytes:");
					for (long i = 0; i < dump; ++i) {
						printf(" %02X", p[i]);
					}
					printf("\n");
				}
				disposehandle(hexternal);
			} else {
				printf("langexternalpack failed\n");
			}
		} else {
			printf("workspace not external type\n");
		}
	} else {
		printf("workspace lookup failed\n");
	}
	Handle hpacked = NULL;
	boolean mustsave = false;
	if (!hashpacktable(ht, false, &hpacked, &mustsave)) {
		fprintf(stderr, "hashpacktable failed\n");
		odbCloseFile(odb);
		return 1;
	}
	print_handle_sizes("packed", hpacked);
	Handle hpacked_copy = NULL;
	if (copyhandle(hpacked, &hpacked_copy)) {
		Handle hrec = NULL;
		Handle hstr = NULL;
		if (unmergehandles(hpacked_copy, &hrec, &hstr)) {
			long rec_size = hrec ? gethandlesize(hrec) : 0;
			long str_size = hstr ? gethandlesize(hstr) : 0;
			printf("unmerge records=%ld strings=%ld\n", rec_size, str_size);
			if (hrec && rec_size > 0) {
				unsigned char *p = *hrec;
				long dump = rec_size < 32 ? rec_size : 32;
				printf("records bytes:");
				for (long i = 0; i < dump; ++i) {
					printf(" %02X", p[i]);
				}
				printf("\n");
			}
			if (hstr && str_size > 0) {
				unsigned char *p = *hstr;
				long dump = str_size < 32 ? str_size : 32;
				printf("strings bytes:");
				for (long i = 0; i < dump; ++i) {
					printf(" %02X", p[i]);
				}
				printf("\n");
				unsigned char len = p[0];
				if (len > 0 && len < str_size) {
					printf("strings[0]=%.*s\n", len, p + 1);
				}
			}
			if (hrec && hstr && rec_size >= 26) {
				long ix = 16;
				int32_t ixkey = 0;
				unsigned char valuetype = 0;
				unsigned char version = 0;
				int32_t datalong = 0;
				if (loadfromhandle(hrec, &ix, sizeof(ixkey), &ixkey)
					&& loadfromhandle(hrec, &ix, sizeof(valuetype), &valuetype)
					&& loadfromhandle(hrec, &ix, sizeof(version), &version)
					&& loadfromhandle(hrec, &ix, sizeof(datalong), &datalong)) {
					disktomemlong(ixkey);
					disktomemlong(datalong);
					long ixlen = datalong;
					int32_t ctbytes = 0;
					if (loadfromhandle(hstr, &ixlen, sizeof(ctbytes), &ctbytes)) {
						disktomemlong(ctbytes);
						Handle hpacked_ext = slice_handle(hstr, ixlen, ctbytes);
						if (hpacked_ext) {
							if (gethandlesize(hpacked_ext) >= 8) {
								long ixext = 0;
								int16_t vraw = 0;
								uint16_t idraw = 0;
								int32_t adrraw = 0;
								loadfromhandle(hpacked_ext, &ixext, sizeof(vraw), &vraw);
								loadfromhandle(hpacked_ext, &ixext, sizeof(idraw), &idraw);
								loadfromhandle(hpacked_ext, &ixext, sizeof(adrraw), &adrraw);
								int16_t v = vraw;
								int32_t adr = adrraw;
								disktomemshort(v);
								disktomemlong(adr);
								printf("external parsed version=%d id=%u adr=%d\n", (int)v, (unsigned)idraw, (int)adr);
							}
							hdlexternalhandle hext = NULL;
							boolean ok = langexternalunpack(hpacked_ext, &hext);
							printf("external unpack=%s (valuetype=%u)\n", ok ? "ok" : "fail", (unsigned)valuetype);
							disposehandle(hpacked_ext);
							if (ok) {
								disposehandle((Handle)hext);
							}
						} else {
							printf("external slice failed\n");
						}
					}
				}
			}
			disposehandle(hrec);
			disposehandle(hstr);
		}
	}
	hdlhashtable hroundtrip = NULL;
	if (newhashtable(&hroundtrip)) {
		if (hashunpacktable(hpacked, false, hroundtrip)) {
			long roundtrip_count = 0;
			hashcountitems(hroundtrip, &roundtrip_count);
			printf("roundtrip count=%ld\n", roundtrip_count);
			bigstring first;
			if (hashgetiteminfo(hroundtrip, 0, first, NULL)) {
				printf("roundtrip first=%.*s\n", first[0], first + 1);
			} else {
				printf("roundtrip first=<none>\n");
			}
		} else {
			printf("roundtrip unpack failed\n");
		}
		disposehashtable(hroundtrip, false);
	} else {
		printf("roundtrip newhashtable failed\n");
	}
	disposehandle(hpacked);
	odbDisposeValue(odb, &value);
	odbCloseFile(odb);
	return 0;
}
