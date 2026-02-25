#include "odb_engine.h"
#include "odb_error_internal.h"
#include "odb_errors.h"
#include "odb_db_internal.h"
#include "legacy/odb_legacy.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

static int icasecmp(const char *a, const char *b) {
	while (*a && *b) {
		unsigned char ca = (unsigned char)*a;
		unsigned char cb = (unsigned char)*b;
		if (ca >= 'A' && ca <= 'Z') {
			ca = (unsigned char)(ca - 'A' + 'a');
		}
		if (cb >= 'A' && cb <= 'Z') {
			cb = (unsigned char)(cb - 'A' + 'a');
		}
		if (ca != cb) {
			return (int)ca - (int)cb;
		}
		a++;
		b++;
	}
	return (unsigned char)*a - (unsigned char)*b;
}

/* Internal file helpers. */
hdlfilenum odbFileOpen(const char *path, const char *mode);
void odbFileClose(hdlfilenum file);

#pragma pack(push, 2)
typedef struct tycancoonrecord {
	hdldatabaserecord hdatabase;
	hdlhashtable hroottable;
	Handle hrootvariable;
	hdltablestack htablestack;
	boolean accesssing;
	void *shellwindow;
} tycancoonrecord, *ptrcancoonrecord, **hdlcancoonrecord;
#pragma pack(pop)

#define cancoonversionnumber 0x03
#define ctwindowinfo 6

#pragma pack(push, 2)
typedef struct tycancoonwindowinfo {
	Rect windowrect;
	unsigned char fontname[34];
	int16_t fontnum;
	int16_t fontsize;
	int16_t fontstyle;
	void *w;
	char waste[10];
} tycancoonwindowinfo;
#pragma pack(pop)

#pragma pack(push, 2)
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

struct odb_ {
	hdlfilenum file;
	odbBool read_only;
	hdlcancoonrecord hc;
};

static boolean disposecancoonrecord(hdlcancoonrecord hcancoon) {
	hdlcancoonrecord hc = hcancoon;
	if (hc == nil) {
		return false;
	}
	if (!(**hc).accesssing) {
		cleartablestructureglobals();
		tabledisposetable((**hc).hroottable, false);
		disposehandle((Handle)(**hc).hrootvariable);
	}
	disposehandle((Handle)(**hc).htablestack);
	disposehandle((Handle)hc);
	return true;
}

static boolean newcancoonrecord(hdlcancoonrecord *hcancoon) {
	hdlcancoonrecord hc;
	hdltablestack htablestack;
	if (!newclearhandle(sizeof(tycancoonrecord), (Handle *)hcancoon)) {
		return false;
	}
	hc = *hcancoon;
	if (!newclearhandle(sizeof(tytablestack), (Handle *)&htablestack)) {
		disposehandle((Handle)hc);
		return false;
	}
	(**hc).htablestack = htablestack;
	return true;
}

static boolean ccloadsystemtable(hdlcancoonrecord hcancoon, dbaddress adr) {
	hdlcancoonrecord hc = hcancoon;
	Handle hvariable = nil;
	hdlhashtable htable = nil;
	hashtablestack = (**hc).htablestack;
	if (!tableloadsystemtable(adr, &hvariable, &htable, false)) {
		return false;
	}
	cleartablestructureglobals();
	(**hc).hrootvariable = rootvariable = hvariable;
	(**hc).hroottable = roottable = htable;
	currenthashtable = roottable;
	return true;
}

static void setcancoonglobals(hdlcancoonrecord hcancoon) {
	hdlcancoonrecord hc = hcancoon;
	databasedata = (**hc).hdatabase;
	hashtablestack = (**hc).htablestack;
	settablestructureglobals((**hc).hrootvariable, false);
	currenthashtable = roottable;
}

static void clearcancoonglobals(void) {
	databasedata = nil;
	cleartablestructureglobals();
	currenthashtable = nil;
	hashtablestack = nil;
}

static boolean loadversion2cancoonfile(dbaddress adr, hdlcancoonrecord hcancoon) {
	tyversion2cancoonrecord info;
	if (!dbreference(adr, sizeof(info), &info)) {
		return false;
	}
	disktomemlong(info.adrroottable);
	return ccloadsystemtable(hcancoon, info.adrroottable);
}

static boolean odb_parse_path(const odbString bspath, hdlhashtable *htable, bigstring bsname, odbBool *is_root) {
	if (!bspath || !htable || !bsname) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid path arguments");
		return false;
	}
	if (!roottable) {
		odbSetLastError(ODB_ERR_IO, "root table not loaded");
		return false;
	}
	if (bspath[0] == 0) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "empty path");
		return false;
	}
	char path[256];
	memcpy(path, &bspath[1], bspath[0]);
	path[bspath[0]] = '\0';

	/* split on '.' */
	char *segments[128];
	int segcount = 0;
	char *p = path;
	while (*p) {
		char *start = p;
		while (*p && *p != '.') {
			p++;
		}
		if (p == start) {
			odbSetLastError(ODB_ERR_INVALID_ARG, "invalid path segment");
			return false;
		}
		if (*p == '.') {
			*p = '\0';
			p++;
		}
		segments[segcount++] = start;
	}
	if (segcount == 0) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "empty path");
		return false;
	}
	for (int i = 0; i < segcount; ++i) {
		const char *seg = segments[i];
		if (!seg || !seg[0]) {
			odbSetLastError(ODB_ERR_INVALID_ARG, "invalid path segment");
			return false;
		}
		if (!isalpha((unsigned char)seg[0])) {
			odbSetLastError(ODB_ERR_INVALID_ARG, "invalid path segment");
			return false;
		}
		for (const char *ch = seg + 1; *ch; ++ch) {
			if (!isalnum((unsigned char)*ch)) {
				odbSetLastError(ODB_ERR_INVALID_ARG, "invalid path segment");
				return false;
			}
		}
	}
	int index = 0;
	if (icasecmp(segments[0], "root") == 0) {
		index = 1;
		if (segcount == 1) {
			if (is_root) {
				*is_root = 1;
			}
			*htable = roottable;
			setemptystring(bsname);
			return true;
		}
	}
	if (is_root) {
		*is_root = 0;
	}
	hdlhashtable cur = roottable;
	for (int i = index; i < segcount - 1; ++i) {
		bigstring seg;
		size_t len = strlen(segments[i]);
		if (len == 0 || len > 255) {
			odbSetLastError(ODB_ERR_INVALID_ARG, "invalid path segment length");
			return false;
		}
		seg[0] = (unsigned char)len;
		memcpy(&seg[1], segments[i], len);
		tyvaluerecord val;
		hdlhashnode hnode = nil;
		if (!hashtablelookup(cur, seg, &val, &hnode)) {
			odbSetLastError(ODB_ERR_NOT_FOUND, "path segment not found");
			return false;
		}
		hdlhashtable next = nil;
		if (!tablevaltotable(val, &next, hnode)) {
			odbSetLastError(ODB_ERR_NOT_FOUND, "path segment not a table");
			return false;
		}
		cur = next;
	}
	const char *last = segments[segcount - 1];
	size_t lastlen = strlen(last);
	if (lastlen == 0 || lastlen > 255) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid path segment length");
		return false;
	}
	bsname[0] = (unsigned char)lastlen;
	memcpy(&bsname[1], last, lastlen);
	*htable = cur;
	return true;
}

static OSType getbinarytypeid(Handle h) {
	if (!h || gethandlesize(h) < 4) {
		return (OSType)'data';
	}
	OSType type = 0;
	memcpy(&type, *h, sizeof(type));
	return type;
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
	if (!file) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid file");
		return 0;
	}
	if (!dbnew(file)) {
		return 0;
	}
	tyversion2cancoonrecord info;
	clearbytes(&info, sizeof(info));
	info.versionnumber = cancoonversionnumber;
	memtodiskshort(info.versionnumber);
	dbaddress adr = nildbaddress;
	boolean fl = dbassign(&adr, sizeof(info), &info);
	if (fl) {
		dbsetview(cancoonview, adr);
		dbclose();
	}
	clearcancoonglobals();
	dbdispose();
	return fl;
}

odbBool odbOpenFile(hdlfilenum file, odbRef *odb) {
	if (!file || !odb) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	if (!dbopenfile(file, false)) {
		return 0;
	}
	dbaddress adr;
	dbgetview(cancoonview, &adr);
	int16_t versionnumber = 0;
	if (!dbreference(adr, sizeof(versionnumber), &versionnumber)) {
		dbdispose();
		return 0;
	}
	disktomemshort(versionnumber);
	hdlcancoonrecord hc = nil;
	if (!newcancoonrecord(&hc)) {
		dbdispose();
		return 0;
	}
	(**hc).hdatabase = databasedata;
	if (versionnumber != cancoonversionnumber && versionnumber != 2) {
		disposecancoonrecord(hc);
		clearcancoonglobals();
		dbdispose();
		odbSetLastError(ODB_ERR_UNSUPPORTED, "unsupported database version");
		return 0;
	}
	if (!loadversion2cancoonfile(adr, hc)) {
		disposecancoonrecord(hc);
		clearcancoonglobals();
		dbdispose();
		return 0;
	}
	struct odb_ *state = (struct odb_ *)calloc(1, sizeof(struct odb_));
	if (!state) {
		disposecancoonrecord(hc);
		clearcancoonglobals();
		dbdispose();
		odbSetLastError(ODB_ERR_IO, "out of memory");
		return 0;
	}
	state->file = file;
	state->read_only = 0;
	state->hc = hc;
	*odb = state;
	return 1;
}

odbBool odbSaveFile(odbRef odb) {
	if (!odb) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	tyversion2cancoonrecord info;
	clearbytes(&info, sizeof(info));
	info.versionnumber = cancoonversionnumber;
	memtodiskshort(info.versionnumber);
	dbaddress adr = nildbaddress;
	if (!tablesavesystemtable((**hc).hrootvariable, &info.adrroottable)) {
		return 0;
	}
	memtodisklong(info.adrroottable);
	dbgetview(cancoonview, &adr);
	if (!dbassign(&adr, sizeof(info), &info)) {
		return 0;
	}
	dbsetview(cancoonview, adr);
	return dbsave();
}

odbBool odbCloseFile(odbRef odb) {
	if (!odb) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	struct odb_ *state = (struct odb_ *)odb;
	hdlcancoonrecord hc = state->hc;
	setcancoonglobals(hc);
	if (!(**hc).accesssing) {
		dbdispose();
	}
	disposecancoonrecord(hc);
	clearcancoonglobals();
	odbFileClose(state->file);
	free(state);
	return 1;
}

odbBool odbDefined(odbRef odb, odbString bspath) {
	if (!odb || !bspath) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring bsname;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, bsname, &is_root)) {
		return 0;
	}
	if (is_root) {
		return 1;
	}
	return hashtablesymbolexists(htable, bsname) ? 1 : 0;
}

odbBool odbDelete(odbRef odb, odbString bspath) {
	if (!odb || !bspath) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring bsname;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, bsname, &is_root)) {
		return 0;
	}
	if (is_root) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "cannot delete root table");
		return 0;
	}
	return hashtabledelete(htable, bsname) ? 1 : 0;
}

odbBool odbGetType(odbRef odb, odbString bspath, OSType *type) {
	if (!odb || !bspath || !type) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring bsname;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, bsname, &is_root)) {
		return 0;
	}
	if (is_root) {
		*type = (OSType)'tabl';
		return 1;
	}
	tyvaluerecord val;
	hdlhashnode hnode = nil;
	if (!hashtablelookup(htable, bsname, &val, &hnode)) {
		odbSetLastError(ODB_ERR_NOT_FOUND, "value not found");
		return 0;
	}
	if (val.valuetype == ty_binaryvaluetype) {
		*type = getbinarytypeid(val.data.binaryvalue);
	} else {
		*type = langexternalgettypeid(val);
	}
	return 1;
}

odbBool odbGetValue(odbRef odb, odbString bspath, odbValueRecord *value) {
	if (!odb || !bspath || !value) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring bsname;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, bsname, &is_root)) {
		return 0;
	}
	tyvaluerecord val;
	hdlhashnode hnode = nil;
	if (is_root) {
		initvalue(&val, ty_externalvaluetype);
		val.data.externalvalue = rootvariable;
	} else {
		if (!hashtablelookup(htable, bsname, &val, &hnode)) {
			odbSetLastError(ODB_ERR_NOT_FOUND, "value not found");
			return 0;
		}
	}
	if (val.valuetype == ty_externalvaluetype) {
		OSType typeid = langexternalgettypeid(val);
		if (typeid == (OSType)'tabl') {
			if (!copyvaluerecord(val, &val)) {
				return 0;
			}
			if (!copyvaluedata(&val)) {
				return 0;
			}
			exemptfromtmpstack(&val);
			value->valuetype = tablevaluetype;
			value->data.externalvalue = val.data.externalvalue;
			return 1;
		}
		Handle hpacked = NULL;
		boolean flnewdbaddress = false;
		if (!langexternalpack((hdlexternalhandle)val.data.externalvalue, &hpacked, &flnewdbaddress)) {
			return 0;
		}
		value->valuetype = externalvaluetype;
		value->data.externalvalue = hpacked;
		return 1;
	}
	if (!copyvaluerecord(val, &val)) {
		return 0;
	}
	if (!copyvaluedata(&val)) {
		return 0;
	}
	exemptfromtmpstack(&val);
	value->valuetype = (odbValueType)langexternalgettypeid(val);
	switch (val.valuetype) {
		case ty_charvaluetype:
			value->data.chvalue = val.data.chvalue;
			break;
		case ty_intvaluetype:
			value->data.intvalue = val.data.intvalue;
			break;
		case ty_longvaluetype:
			value->data.longvalue = val.data.longvalue;
			break;
		case ty_booleanvaluetype:
			value->data.flvalue = val.data.flvalue;
			break;
		case ty_datevaluetype:
			value->data.datevalue = val.data.datevalue;
			break;
		case ty_directionvaluetype:
			value->data.dirvalue = (odbDirection)val.data.dirvalue;
			break;
		case ty_stringvaluetype:
			value->data.stringvalue = val.data.stringvalue;
			break;
		case ty_addressvaluetype:
			value->data.addressvalue = (Handle)val.data.addressvalue;
			break;
		case ty_binaryvaluetype:
			value->data.binaryvalue = val.data.binaryvalue;
			break;
		case ty_pointvaluetype:
			value->data.pointvalue.v = val.data.pointvalue.v;
			value->data.pointvalue.h = val.data.pointvalue.h;
			break;
		case ty_rectvaluetype:
			value->data.rectvalue = (odbRect **)val.data.rectvalue;
			break;
		case ty_singlevaluetype:
			value->data.singlevalue = val.data.singlevalue;
			break;
		case ty_doublevaluetype:
			value->data.doublevalue = val.data.doublevalue;
			break;
		case ty_objspecvaluetype:
			value->data.objspecvalue = val.data.objspecvalue;
			break;
		case ty_aliasvaluetype:
			value->data.aliasvalue = val.data.aliasvalue;
			break;
		case ty_enumvaluetype:
			value->data.enumvalue = val.data.enumvalue;
			break;
		case ty_listvaluetype:
			value->data.listvalue = (Handle)val.data.listvalue;
			break;
		case ty_recordvaluetype:
			value->data.recordvalue = (Handle)val.data.recordvalue;
			break;
		default:
			value->data.binaryvalue = val.data.binaryvalue;
			break;
	}
	return 1;
}

odbBool odbSetValue(odbRef odb, odbString bspath, odbValueRecord *value) {
	if (!odb || !bspath || !value) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring bsname;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, bsname, &is_root)) {
		return 0;
	}
	if (is_root) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "cannot set root table value");
		return 0;
	}
	tyvaluerecord val;
	tyvaluetype type = langexternalgetvaluetype(value->valuetype);
	if (type == (tyvaluetype)-1) {
		if (!setbinaryvalue(value->data.binaryvalue, value->valuetype, &val)) {
			return 0;
		}
	} else {
		initvalue(&val, type);
		val.data.binaryvalue = value->data.binaryvalue;
	}
	if (!copyvaluerecord(val, &val)) {
		return 0;
	}
	if (!hashtableassign(htable, bsname, val)) {
		disposevaluerecord(val, true);
		return 0;
	}
	exemptfromtmpstack(&val);
	return 1;
}

odbBool odbNewTable(odbRef odb, odbString bspath) {
	if (!odb || !bspath) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring bsname;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, bsname, &is_root)) {
		return 0;
	}
	if (is_root) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "cannot create root table");
		return 0;
	}
	hdlexternalvariable hv = nil;
	hdlhashtable hnewtable = nil;
	if (!tablenewtable(&hv, &hnewtable)) {
		return 0;
	}
	tyvaluerecord val;
	initvalue(&val, ty_externalvaluetype);
	val.data.externalvalue = (Handle)hv;
	if (!hashtableassign(htable, bsname, val)) {
		disposevaluerecord(val, true);
		return 0;
	}
	exemptfromtmpstack(&val);
	return 1;
}

odbBool odbCountItems(odbRef odb, odbString bspath, int32_t *count) {
	if (!odb || !bspath || !count) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring bsname;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, bsname, &is_root)) {
		return 0;
	}
	if (!is_root) {
		tyvaluerecord val;
		hdlhashnode hnode = nil;
		if (!hashtablelookup(htable, bsname, &val, &hnode)) {
			odbSetLastError(ODB_ERR_NOT_FOUND, "value not found");
			return 0;
		}
		if (!tablevaltotable(val, &htable, hnode)) {
			odbSetLastError(ODB_ERR_INVALID_ARG, "value not a table");
			return 0;
		}
	}
	long ctitems = 0;
	if (!hashcountitems(htable, &ctitems)) {
		return 0;
	}
	*count = (int32_t)ctitems;
	return 1;
}

odbBool odbGetNthItem(odbRef odb, odbString bspath, int32_t n, odbString bsname) {
	if (!odb || !bspath || !bsname) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	if (n <= 0) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid item index");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring pathseg;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, pathseg, &is_root)) {
		return 0;
	}
	if (!is_root) {
		tyvaluerecord val;
		hdlhashnode hnode = nil;
		if (!hashtablelookup(htable, pathseg, &val, &hnode)) {
			odbSetLastError(ODB_ERR_NOT_FOUND, "value not found");
			return 0;
		}
		if (!tablevaltotable(val, &htable, hnode)) {
			odbSetLastError(ODB_ERR_INVALID_ARG, "value not a table");
			return 0;
		}
	}
	long ctitems = 0;
	if (!hashcountitems(htable, &ctitems)) {
		return 0;
	}
	if (n > ctitems) {
		odbSetLastError(ODB_ERR_NOT_FOUND, "item not found");
		return 0;
	}
	if (!hashgetiteminfo(htable, (long)(n - 1), bsname, nil)) {
		odbSetLastError(ODB_ERR_NOT_FOUND, "item not found");
		return 0;
	}
	return 1;
}

odbBool odbGetModDate(odbRef odb, odbString bspath, uint32_t *date) {
	if (!odb || !bspath || !date) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return 0;
	}
	hdlcancoonrecord hc = ((struct odb_ *)odb)->hc;
	setcancoonglobals(hc);
	hdlhashtable htable = nil;
	bigstring bsname;
	odbBool is_root = 0;
	if (!odb_parse_path(bspath, &htable, bsname, &is_root)) {
		return 0;
	}
	if (!is_root) {
		tyvaluerecord val;
		hdlhashnode hnode = nil;
		if (!hashtablelookup(htable, bsname, &val, &hnode)) {
			odbSetLastError(ODB_ERR_NOT_FOUND, "value not found");
			return 0;
		}
		if (!tablevaltotable(val, &htable, hnode)) {
			odbSetLastError(ODB_ERR_NOT_FOUND, "value not a table");
			return 0;
		}
	}
	*date = (**htable).timelastsave;
	return 1;
}

void odbDisposeValue(odbRef odb, odbValueRecord *value) {
	if (!odb || !value) {
		odbSetLastError(ODB_ERR_INVALID_ARG, "invalid argument");
		return;
	}
	if (value->valuetype == externalvaluetype) {
		disposehandle(value->data.externalvalue);
		return;
	}
	tyvaluetype type = langexternalgetvaluetype((OSType)(*value).valuetype);
	if (type == (tyvaluetype)-1) {
		type = ty_binaryvaluetype;
	}
	tyvaluerecord val;
	initvalue(&val, type);
	val.data.binaryvalue = (*value).data.binaryvalue;
	disposevaluerecord(val, false);
}
