#include "legacy/odb_legacy.h"
#include "odb_byteorder.h"
#include "odb_error_internal.h"
#include "odb_errors.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

hdlhashtable currenthashtable = nil;
hdltablestack hashtablestack = nil;
hdlhashtable roottable = nil;
Handle rootvariable = nil;
hdlhashtable systemtable = nil;
hdlhashtable pathstable = nil;
hdlhashtable filewindowtable = nil;

boolean fllanghashassignprotect = false;
boolean fllangexternalvalueprotect = false;
boolean flscriptrunning = false;

static hdlhashtable hfirstfreetable = nil;

static unsigned char getlower(unsigned char ch) {
	if (ch >= 'A' && ch <= 'Z') {
		return (unsigned char)(ch - 'A' + 'a');
	}
	return ch;
}

static void clearhandle(Handle h) {
	if (!h) {
		return;
	}
	long size = gethandlesize(h);
	if (size > 0) {
		memset(*h, 0, (size_t)size);
	}
}

static boolean equalidentifiers(const bigstring bs1, const bigstring bs2) {
	short len = stringlength(bs1);
	if (len != stringlength(bs2)) {
		return false;
	}
	const unsigned char *p1 = bs1 + len;
	const unsigned char *p2 = bs2 + len;
	while (--len >= 0) {
		if (getlower(*p1--) != getlower(*p2--)) {
			return false;
		}
	}
	return true;
}

static short compareidentifiers(const bigstring bs1, const bigstring bs2) {
	short len1 = stringlength(bs1);
	short len2 = stringlength(bs2);
	short len = min(len1, len2);
	for (short i = 0; i < len; ++i) {
		unsigned char c1 = getlower((unsigned char)bs1[i + 1]);
		unsigned char c2 = getlower((unsigned char)bs2[i + 1]);
		if (c1 < c2) {
			return -1;
		}
		if (c1 > c2) {
			return 1;
		}
	}
	if (len1 < len2) {
		return -1;
	}
	if (len1 > len2) {
		return 1;
	}
	return 0;
}

static uint32_t timenow(void) {
	const uint32_t mac_epoch_delta = 2082844800U; /* seconds from 1904-01-01 to 1970-01-01 */
	return (uint32_t)time(NULL) + mac_epoch_delta;
}

/* value helpers */
void initvalue(tyvaluerecord *val, tyvaluetype type) {
	if (!val) {
		return;
	}
	memset(val, 0, sizeof(*val));
	val->valuetype = type;
}

boolean copyvaluerecord(tyvaluerecord val, tyvaluerecord *dest) {
	if (!dest) {
		return false;
	}
	*dest = val;
	return true;
}

boolean copyvaluedata(tyvaluerecord *val) {
	if (!val) {
		return false;
	}
	if (val->fltmpdata) {
		return true;
	}
	Handle hsrc = NULL;
	switch (val->valuetype) {
		case ty_stringvaluetype:
		case ty_binaryvaluetype:
		case ty_objspecvaluetype:
		case ty_filespecvaluetype:
		case ty_aliasvaluetype:
			 hsrc = val->data.binaryvalue;
			 break;
		case ty_addressvaluetype:
			 hsrc = (Handle)val->data.addressvalue;
			 break;
		case ty_externalvaluetype:
			 hsrc = (Handle)val->data.externalvalue;
			 break;
		case ty_listvaluetype:
		case ty_recordvaluetype:
			 hsrc = (Handle)val->data.listvalue;
			 break;
		default:
			 return true;
	}
	if (!hsrc) {
		return true;
	}
	Handle hcopy = NULL;
	if (!copyhandle(hsrc, &hcopy)) {
		return false;
	}
	switch (val->valuetype) {
		case ty_stringvaluetype:
		case ty_binaryvaluetype:
		case ty_objspecvaluetype:
		case ty_filespecvaluetype:
		case ty_aliasvaluetype:
			 val->data.binaryvalue = hcopy;
			 break;
		case ty_addressvaluetype:
			 val->data.addressvalue = (hdlstring)hcopy;
			 break;
		case ty_externalvaluetype:
			 val->data.externalvalue = hcopy;
			 break;
		case ty_listvaluetype:
		case ty_recordvaluetype:
			 val->data.listvalue = (tylistrecord **)hcopy;
			 break;
		default:
			 break;
	}
	return true;
}

void disposevaluerecord(tyvaluerecord val, boolean fldisk) {
	(void)fldisk;
	switch (val.valuetype) {
		case ty_stringvaluetype:
		case ty_binaryvaluetype:
		case ty_objspecvaluetype:
		case ty_filespecvaluetype:
		case ty_aliasvaluetype:
			disposehandle(val.data.binaryvalue);
			break;
		case ty_addressvaluetype:
			disposehandle((Handle)val.data.addressvalue);
			break;
		case ty_externalvaluetype: {
			hdlexternalvariable hv = (hdlexternalvariable)val.data.externalvalue;
			if (hv) {
				if ((**hv).flopaque && (**hv).variabledata) {
					disposehandle((Handle)(**hv).variabledata);
				}
				disposehandle((Handle)hv);
			}
			break;
		}
		default:
			break;
	}
}

void exemptfromtmpstack(tyvaluerecord *val) {
	if (val) {
		val->fltmpstack = false;
		val->fltmpdata = false;
	}
}

boolean setbinaryvalue(Handle h, OSType typeid, tyvaluerecord *val) {
	(void)typeid;
	if (!val) {
		return false;
	}
	initvalue(val, ty_binaryvaluetype);
	val->data.binaryvalue = h;
	return true;
}

boolean setaddressvalue(hdlhashtable htable, const bigstring bs, tyvaluerecord *val) {
	(void)htable;
	if (!val) {
		return false;
	}
	hdlstring hstring = NULL;
	if (!newheapstring(bs, &hstring)) {
		return false;
	}
	initvalue(val, ty_addressvaluetype);
	val->data.addressvalue = hstring;
	return true;
}

boolean getaddresspath(tyvaluerecord val, bigstring bs) {
	if (val.valuetype != ty_addressvaluetype || !val.data.addressvalue) {
		setemptystring(bs);
		return false;
	}
	copyheapstring((hdlstring)val.data.addressvalue, bs);
	return true;
}

/* external values */
boolean langnewexternalvariable(boolean flinmemory, intptr_t variabledata, hdlexternalvariable *h) {
	if (!h) {
		return false;
	}
	if (!newclearhandle(sizeof(tyexternalvariable), (Handle *)h)) {
		return false;
	}
	(**h)->flinmemory = flinmemory ? 1 : 0;
	(**h)->variabledata = variabledata;
	(**h)->hdatabase = databasedata;
	(**h)->oldaddress = nildbaddress;
	return true;
}

boolean langexternalnewvalue(tyexternalid id, Handle hdata, tyvaluerecord *val) {
	if (!val) {
		return false;
	}
	hdlexternalvariable hv = NULL;
	if (!langnewexternalvariable(true, (long)hdata, &hv)) {
		return false;
	}
	(**hv).id = (uint16_t)id;
	initvalue(val, ty_externalvaluetype);
	val->data.externalvalue = (Handle)hv;
	return true;
}

OSType langexternalgettypeid(tyvaluerecord val) {
	switch (val.valuetype) {
		case ty_charvaluetype:
			return (OSType)'char';
		case ty_intvaluetype:
			return (OSType)'shor';
		case ty_longvaluetype:
			return (OSType)'long';
		case ty_binaryvaluetype:
			return (OSType)'data';
		case ty_booleanvaluetype:
			return (OSType)'bool';
		case ty_tokenvaluetype:
			return (OSType)'tokn';
		case ty_datevaluetype:
			return (OSType)'date';
		case ty_addressvaluetype:
			return (OSType)'addr';
		case ty_codevaluetype:
			return (OSType)'code';
		case ty_doublevaluetype:
			return (OSType)'doub';
		case ty_stringvaluetype:
			return (OSType)'TEXT';
		case ty_directionvaluetype:
			return (OSType)'dir ';
		case ty_pointvaluetype:
			return (OSType)'QDpt';
		case ty_rectvaluetype:
			return (OSType)'qdrt';
		case ty_patternvaluetype:
			return (OSType)'tptn';
		case ty_rgbvaluetype:
			return (OSType)'cRGB';
		case ty_fixedvaluetype:
			return (OSType)'fixd';
		case ty_singlevaluetype:
			return (OSType)'sing';
		case ty_objspecvaluetype:
			return (OSType)'obj ';
		case ty_filespecvaluetype:
			return (OSType)'fss ';
		case ty_aliasvaluetype:
			return (OSType)'alis';
		case ty_enumvaluetype:
			return (OSType)'enum';
		case ty_listvaluetype:
			return (OSType)'list';
		case ty_recordvaluetype:
			return (OSType)'reco';
		case ty_externalvaluetype: {
			hdlexternalvariable hv = (hdlexternalvariable)val.data.externalvalue;
			if (!hv) {
				return (OSType)'xtrn';
			}
			switch ((tyexternalid)(**hv).id) {
				case idtableprocessor:
					return (OSType)'tabl';
				case idscriptprocessor:
					return (OSType)'scpt';
				case idmenuprocessor:
					return (OSType)'mbar';
				case idoutlineprocessor:
					return (OSType)'optx';
				default:
					return (OSType)'xtrn';
			}
		}
		default:
			return (OSType)0x3F3F3F3F; // '????'
	}
}

tyvaluetype langexternalgetvaluetype(OSType typeid) {
	switch (typeid) {
		case (OSType)'char':
			return ty_charvaluetype;
		case (OSType)'shor':
			return ty_intvaluetype;
		case (OSType)'long':
			return ty_longvaluetype;
		case (OSType)'data':
			return ty_binaryvaluetype;
		case (OSType)'bool':
			return ty_booleanvaluetype;
		case (OSType)'tokn':
			return ty_tokenvaluetype;
		case (OSType)'date':
			return ty_datevaluetype;
		case (OSType)'addr':
			return ty_addressvaluetype;
		case (OSType)'code':
			return ty_codevaluetype;
		case (OSType)'doub':
			return ty_doublevaluetype;
		case (OSType)'TEXT':
			return ty_stringvaluetype;
		case (OSType)'dir ':
			return ty_directionvaluetype;
		case (OSType)'QDpt':
			return ty_pointvaluetype;
		case (OSType)'qdrt':
			return ty_rectvaluetype;
		case (OSType)'tptn':
			return ty_patternvaluetype;
		case (OSType)'cRGB':
			return ty_rgbvaluetype;
		case (OSType)'fixd':
			return ty_fixedvaluetype;
		case (OSType)'sing':
			return ty_singlevaluetype;
		case (OSType)'obj ':
			return ty_objspecvaluetype;
		case (OSType)'fss ':
			return ty_filespecvaluetype;
		case (OSType)'alis':
			return ty_aliasvaluetype;
		case (OSType)'enum':
			return ty_enumvaluetype;
		case (OSType)'list':
			return ty_listvaluetype;
		case (OSType)'reco':
			return ty_recordvaluetype;
		case (OSType)'tabl':
			return ty_externalvaluetype;
		default:
			return (tyvaluetype)-1;
	}
}

boolean langexternalvaltotable(tyvaluerecord val, hdlhashtable *htable, hdlhashnode hnode) {
	(void)hnode;
	if (val.valuetype != ty_externalvaluetype) {
		return false;
	}
	hdlexternalvariable hv = (hdlexternalvariable)val.data.externalvalue;
	if (!hv || (**hv).id != idtableprocessor) {
		return false;
	}
	if (!tableverbinmemory(hv, hnode)) {
		return false;
	}
	*htable = (hdlhashtable)(**hv).variabledata;
	return true;
}

hdldatabaserecord langexternalgetdatabase(hdlexternalvariable hv) {
	if (!hv) {
		return nil;
	}
	return (**hv).hdatabase;
}

void langexternalsetdatabase(hdlexternalvariable hv, hdldatabaserecord hdb) {
	if (!hv || !hdb) {
		return;
	}
	if ((**hv).hdatabase == hdb) {
		return;
	}
	(**hv).hdatabase = hdb;
}

static boolean external_pack_table(hdlexternalhandle h, Handle *hpacked, boolean *flnewdbaddress) {
	return tableverbpack((hdlexternalvariable)h, hpacked, flnewdbaddress);
}

static boolean external_pack_opaque(hdlexternalhandle h, Handle *hpacked) {
	if (!h || !hpacked) {
		return false;
	}
	hdlexternalvariable hv = (hdlexternalvariable)h;
	Handle hsrc = (Handle)(**hv).variabledata;
	return copyhandle(hsrc, hpacked);
}

boolean langexternalpack(hdlexternalhandle h, Handle *hpacked, boolean *flnewdbaddress) {
	if (!h || !hpacked) {
		return false;
	}
	hdlexternalvariable hv = (hdlexternalvariable)h;
	if ((**hv).flopaque) {
		*flnewdbaddress = false;
		return external_pack_opaque(h, hpacked);
	}
	if ((**hv).id == idtableprocessor) {
		return external_pack_table(h, hpacked, flnewdbaddress);
	}
	*flnewdbaddress = false;
	return external_pack_opaque(h, hpacked);
}

boolean langexternalunpack(Handle hpacked, hdlexternalhandle *h) {
	if (!h) {
		return false;
	}
	*h = nil;
	if (!hpacked) {
		return false;
	}
	long ixload = 0;
	tydiskexternalhandle rec;
	if (!loadfromhandle(hpacked, &ixload, sizeof(rec), &rec)) {
		return false;
	}
	disktomemshort(rec.versionnumber);
	if (rec.versionnumber != externaldiskversionnumber) {
		return false;
	}
	tyexternalid id = (tyexternalid)rec.id;
	if (id == idtableprocessor) {
		if (!tableverbunpack(hpacked, &ixload, (hdlexternalvariable *)h, false)) {
			return false;
		}
		(**(hdlexternalvariable *)h)->id = (uint16_t)id;
		return true;
	}
	/* opaque external: preserve full packed handle */
	hdlexternalvariable hv = NULL;
	Handle hcopy = NULL;
	if (!copyhandle(hpacked, &hcopy)) {
		return false;
	}
	if (!langnewexternalvariable(true, (long)hcopy, &hv)) {
		disposehandle(hcopy);
		return false;
	}
	(**hv).id = (uint16_t)id;
	(**hv).flopaque = 1;
	*h = (hdlexternalhandle)hv;
	return true;
}

boolean langexternalmemorypack(hdlexternalhandle h, Handle *hpacked, hdlhashnode hnode) {
	(void)hnode;
	return langexternalpack(h, hpacked, &(boolean){false});
}

boolean langexternalmemoryunpack(Handle hpacked, hdlexternalhandle *h) {
	return langexternalunpack(hpacked, h);
}

/* table helpers */
boolean newtablevariable(boolean flinmemory, intptr_t variabledata, hdlexternalvariable *h, boolean flxml) {
	(void)flxml;
	if (!langnewexternalvariable(flinmemory, variabledata, h)) {
		return false;
	}
	(**h)->id = (uint16_t)idtableprocessor;
	return true;
}

boolean tablenewtable(hdlexternalvariable *hvariable, hdlhashtable *htable) {
	if (!newtablevariable(true, 0L, (hdlexternalvariable *)hvariable, false)) {
		return false;
	}
	if (!newhashtable(htable)) {
		disposehandle((Handle)*hvariable);
		return false;
	}
	hdlexternalvariable hv = *hvariable;
	hdlhashtable ht = *htable;
	(**ht).timecreated = timenow();
	(**ht).timelastsave = (**ht).timecreated;
	(**hv).variabledata = (intptr_t)ht;
	(**ht).hashtablerefcon = (long)hv;
	(**ht).fldirty = true;
	return true;
}

boolean tablevaltotable(tyvaluerecord val, hdlhashtable *htable, hdlhashnode hnode) {
	return langexternalvaltotable(val, htable, hnode);
}

boolean tableverbinmemory(hdlexternalvariable hvariable, hdlhashnode hnode) {
	(void)hnode;
	hdlexternalvariable hv = hvariable;
	if (!hv) {
		return false;
	}
	if ((**hv).flinmemory) {
		return true;
	}
	dbaddress adr = (dbaddress)(**hv).variabledata;
	if (adr == nildbaddress) {
		return false;
	}
	dbpushdatabase((**hv).hdatabase);
	Handle hpacked = NULL;
	boolean fl = dbrefhandle(adr, &hpacked);
	if (!fl) {
		dbpopdatabase();
		return false;
	}
	hdlhashtable htable = nil;
	fl = tableunpacktable(hpacked, false, &htable);
	dbpopdatabase();
	if (!fl) {
		return false;
	}
	(**hv).flinmemory = true;
	(**hv).variabledata = (intptr_t)htable;
	(**hv).oldaddress = adr;
	(**htable).hashtablerefcon = (long)hv;
	(**htable).thistableshashnode = hnode;
	return true;
}

boolean tablepacktable(hdlhashtable htable, boolean flmemory, Handle *hpacked, boolean *flmustsave) {
	if (flmustsave) {
		*flmustsave = true;
	}
	return hashpacktable(htable, false, hpacked, flmustsave);
}

boolean tableunpacktable(Handle hpacked, boolean flmemory, hdlhashtable *htable) {
	Handle hpackedtable = nil;
	Handle hpackedformats = nil;
	if (!unmergehandles(hpacked, &hpackedtable, &hpackedformats)) {
		return false;
	}
	if (!newhashtable(htable)) {
		disposehandle(hpackedtable);
		disposehandle(hpackedformats);
		return false;
	}
	if (!hashunpacktable(hpackedtable, flmemory, *htable)) {
		disposehashtable(*htable, false);
		disposehandle(hpackedformats);
		return false;
	}
	if (hpackedformats) {
		disposehandle(hpackedformats);
	}
	return true;
}

boolean tableverbpack(hdlexternalvariable h, Handle *hpacked, boolean *flnewdbaddress) {
	if (!h || !hpacked || !flnewdbaddress) {
		return false;
	}
	tydiskexternalhandle rec;
	rec.versionnumber = externaldiskversionnumber;
	memtodiskshort(rec.versionnumber);
	rec.id = (uint16_t)idtableprocessor;
	if (!newfilledhandle(&rec, sizeof(rec), hpacked)) {
		return false;
	}
	hdlexternalvariable hv = h;
	dbaddress adr = nildbaddress;
	if (!(**hv).flinmemory) {
		adr = (dbaddress)(**hv).variabledata;
		*flnewdbaddress = false;
		memtodisklong(adr);
		return enlargehandle(*hpacked, sizeof(adr), &adr);
	}
	hdlhashtable ht = (hdlhashtable)(**hv).variabledata;
	Handle hpackedtable = NULL;
	boolean flmustsave = false;
	if (!tablepacktable(ht, false, &hpackedtable, &flmustsave)) {
		return false;
	}
	if (!dbsavehandle(hpackedtable, &adr)) {
		disposehandle(hpackedtable);
		return false;
	}
	disposehandle(hpackedtable);
	*flnewdbaddress = ((**hv).oldaddress != adr);
	(**hv).oldaddress = adr;
	(**ht).fldirty = false;
	(**ht).flsubsdirty = false;
	memtodisklong(adr);
	return enlargehandle(*hpacked, sizeof(adr), &adr);
}

boolean tableverbunpack(Handle hpacked, long *ixload, hdlexternalvariable *h, boolean flxml) {
	(void)flxml;
	dbaddress adr;
	if (!loadfromhandle(hpacked, ixload, sizeof(adr), &adr)) {
		return false;
	}
	disktomemlong(adr);
	return newtablevariable(false, adr, (hdlexternalvariable *)h, false);
}

boolean tableloadsystemtable(dbaddress adr, Handle *hvariable, hdlhashtable *htable, boolean flcreate) {
	(void)flcreate;
	if (adr == nildbaddress) {
		return tablenewtable((hdlexternalvariable *)hvariable, htable);
	}
	hdlexternalvariable hv = NULL;
	if (!newtablevariable(false, adr, (hdlexternalvariable *)hvariable, false)) {
		return false;
	}
	hv = (hdlexternalvariable)*hvariable;
	if (!tableverbinmemory(hv, HNoNode)) {
		disposehandle((Handle)hv);
		return false;
	}
	*htable = (hdlhashtable)(**hv).variabledata;
	(**hv).id = (uint16_t)idtableprocessor;
	return true;
}

boolean tablesavesystemtable(Handle hvariable, dbaddress *adr) {
	if (!hvariable || !adr) {
		return false;
	}
	hdlexternalvariable hv = (hdlexternalvariable)hvariable;
	if (!(**hv).flinmemory) {
		*adr = (dbaddress)(**hv).variabledata;
		return true;
	}
	hdlhashtable ht = (hdlhashtable)(**hv).variabledata;
	Handle hpacked = NULL;
	boolean flmustsave = false;
	if (!tablepacktable(ht, false, &hpacked, &flmustsave)) {
		return false;
	}
	if (!dbsavehandle(hpacked, adr)) {
		disposehandle(hpacked);
		return false;
	}
	disposehandle(hpacked);
	(**hv).oldaddress = *adr;
	(**ht).fldirty = false;
	(**ht).flsubsdirty = false;
	return true;
}

boolean cleartablestructureglobals(void) {
	rootvariable = nil;
	roottable = nil;
	systemtable = nil;
	pathstable = nil;
	filewindowtable = nil;
	return true;
}

boolean settablestructureglobals(Handle hvariable, boolean flcreatesubs) {
	(void)flcreatesubs;
	hdlexternalvariable hv = (hdlexternalvariable)hvariable;
	if (!hv) {
		return false;
	}
	hdlhashtable ht = (hdlhashtable)(**hv).variabledata;
	if (!ht) {
		return false;
	}
	cleartablestructureglobals();
	rootvariable = (Handle)hv;
	roottable = ht;
	return true;
}

boolean tabledisposetable(hdlhashtable htable, boolean fldisk) {
	if (!htable) {
		return true;
	}
	disposehashtable(htable, fldisk);
	return true;
}

/* hash table core */
static short hashfunction(const bigstring bs) {
	unsigned short sum = 0;
	short len = stringlength(bs);
	for (short i = 1; i <= len; ++i) {
		sum = (unsigned short)(sum * 31 + getlower(bs[i]));
	}
	return (short)(sum % ctbuckets);
}

boolean newhashtable(hdlhashtable *htable) {
	if (!htable) {
		return false;
	}
	if (hfirstfreetable != nil) {
		hdlhashtable ht = hfirstfreetable;
		hfirstfreetable = (**hfirstfreetable).prevhashtable;
		clearhandle((Handle)ht);
		*htable = ht;
		return true;
	}
	if (!newclearhandle(sizeof(tyhashtable), (Handle *)htable)) {
		return false;
	}
	(**(*htable)).timecreated = timenow();
	return true;
}

boolean pushhashtable(hdlhashtable h) {
	if (!hashtablestack) {
		if (!newclearhandle(sizeof(tytablestack), (Handle *)&hashtablestack)) {
			return false;
		}
	}
	if ((**hashtablestack).toptables >= cthashtables) {
		return false;
	}
	(**hashtablestack).stack[(**hashtablestack).toptables++] = currenthashtable;
	currenthashtable = h;
	return true;
}

boolean pophashtable(void) {
	if (!hashtablestack || (**hashtablestack).toptables <= 0) {
		currenthashtable = nil;
		return false;
	}
	currenthashtable = (**hashtablestack).stack[--(**hashtablestack).toptables];
	return true;
}

hdlhashtable sethashtable(hdlhashtable hset) {
	hdlhashtable prev = currenthashtable;
	currenthashtable = hset;
	return prev;
}

static boolean hashsortedinsert(hdlhashnode hnode) {
	hdlhashtable ht = currenthashtable;
	if (!ht) {
		return false;
	}
	if ((**ht).hfirstsort == nil) {
		(**ht).hfirstsort = hnode;
		(**hnode).sortedlink = nil;
		return true;
	}
	hdlhashnode nomad = (**ht).hfirstsort;
	hdlhashnode prev = nil;
	while (nomad != nil) {
		bigstring bsnomad;
		gethashkey(nomad, bsnomad);
		bigstring bsnode;
		gethashkey(hnode, bsnode);
		if (compareidentifiers(bsnode, bsnomad) < 0) {
			break;
		}
		prev = nomad;
		nomad = (**nomad).sortedlink;
	}
	if (prev == nil) {
		(**hnode).sortedlink = (**ht).hfirstsort;
		(**ht).hfirstsort = hnode;
	} else {
		(**hnode).sortedlink = nomad;
		(**prev).sortedlink = hnode;
	}
	return true;
}

static void hashsorteddelete(hdlhashnode hnodedelete) {
	hdlhashtable ht = currenthashtable;
	if (!ht || !hnodedelete) {
		return;
	}
	hdlhashnode nomad = (**ht).hfirstsort;
	hdlhashnode prev = nil;
	while (nomad != nil) {
		if (nomad == hnodedelete) {
			if (prev == nil) {
				(**ht).hfirstsort = (**nomad).sortedlink;
			} else {
				(**prev).sortedlink = (**nomad).sortedlink;
			}
			return;
		}
		prev = nomad;
		nomad = (**nomad).sortedlink;
	}
}

static boolean hashlinknode(hdlhashtable htable, hdlhashnode hnode) {
	short bucket = hashfunction((**hnode).hashkey);
	(**hnode).hashlink = (**htable).hashbucket[bucket];
	(**htable).hashbucket[bucket] = hnode;
	return true;
}

boolean hashinsertnode(hdlhashnode hnode, hdlhashtable htable) {
	if (!hnode || !htable) {
		return false;
	}
	if (!hashlinknode(htable, hnode)) {
		return false;
	}
	hdlhashtable prev = sethashtable(htable);
	hashsortedinsert(hnode);
	sethashtable(prev);
	return true;
}

boolean hashunlinknode(hdlhashtable htable, hdlhashnode hnode) {
	if (!htable || !hnode) {
		return false;
	}
	short bucket = hashfunction((**hnode).hashkey);
	hdlhashnode nomad = (**htable).hashbucket[bucket];
	hdlhashnode prev = nil;
	while (nomad != nil) {
		if (nomad == hnode) {
			if (prev == nil) {
				(**htable).hashbucket[bucket] = (**nomad).hashlink;
			} else {
				(**prev).hashlink = (**nomad).hashlink;
			}
			break;
		}
		prev = nomad;
		nomad = (**nomad).hashlink;
	}
	hdlhashtable prevtable = sethashtable(htable);
	hashsorteddelete(hnode);
	sethashtable(prevtable);
	return true;
}

boolean hashsetnodekey(hdlhashtable htable, hdlhashnode hnode, const bigstring bs) {
	if (!hnode || !htable) {
		return false;
	}
	hashunlinknode(htable, hnode);
	long len = stringlength(bs);
	Handle hnew = NULL;
	if (!newhandle(sizeof(tyhashnode) + len + 1, &hnew)) {
		return false;
	}
	hdlhashnode hnewnode = (hdlhashnode)hnew;
	(**hnewnode) = (**hnode);
	copystring(bs, (**hnewnode).hashkey);
	disposehandle((Handle)hnode);
	return hashinsertnode(hnewnode, htable);
}

static boolean newhashnode(hdlhashnode *hnode, const bigstring bskey) {
	if (!hnode) {
		return false;
	}
	long len = stringlength(bskey);
	if (!newclearhandle(sizeof(tyhashnode) + len + 1, (Handle *)hnode)) {
		return false;
	}
	copystring(bskey, (**hnode)->hashkey);
	return true;
}

static boolean hashlocate(const bigstring bs, hdlhashnode *hnode, hdlhashnode *hprev) {
	hdlhashtable ht = currenthashtable;
	if (!ht) {
		return false;
	}
	short bucket = hashfunction(bs);
	hdlhashnode nomad = (**ht).hashbucket[bucket];
	hdlhashnode prev = nil;
	while (nomad != nil) {
		if (equalidentifiers((**nomad).hashkey, bs)) {
			if (hnode) {
				*hnode = nomad;
			}
			if (hprev) {
				*hprev = prev;
			}
			return true;
		}
		prev = nomad;
		nomad = (**nomad).hashlink;
	}
	return false;
}

static boolean disposehashnode(hdlhashtable ht, hdlhashnode hnode, boolean fldisposevalue) {
	if (!hnode) {
		return true;
	}
	if (fldisposevalue) {
		disposevaluerecord((**hnode).val, true);
	}
	hashunlinknode(ht, hnode);
	disposehandle((Handle)hnode);
	return true;
}

static boolean hashinsert(const bigstring bs, tyvaluerecord val) {
	hdlhashtable ht = currenthashtable;
	hdlhashnode hnode = nil;
	if (!newhashnode(&hnode, bs)) {
		return false;
	}
	(**hnode).val = val;
	if (!hashinsertnode(hnode, ht)) {
		disposehandle((Handle)hnode);
		return false;
	}
	return true;
}

boolean hashtabledelete(hdlhashtable htable, bigstring bs) {
	if (!htable) {
		return false;
	}
	pushhashtable(htable);
	hdlhashnode hnode = nil;
	hdlhashnode hprev = nil;
	boolean fl = hashlocate(bs, &hnode, &hprev);
	if (fl) {
		disposehashnode(htable, hnode, true);
	}
	pophashtable();
	return fl;
}

boolean hashsymbolexists(const bigstring bs) {
	hdlhashnode hnode = nil;
	return hashlocate(bs, &hnode, nil);
}

boolean hashtablesymbolexists(hdlhashtable htable, const bigstring bs) {
	boolean fl;
	pushhashtable(htable);
	fl = hashsymbolexists(bs);
	pophashtable();
	return fl;
}

static boolean hashresolvevalue(hdlhashtable htable, hdlhashnode hnode) {
	(void)htable;
	if (!hnode) {
		return false;
	}
	if ((**hnode).val.fldiskval) {
		dbaddress adr = (dbaddress)(**hnode).val.data.diskvalue;
		Handle h = NULL;
		if (!dbrefhandle(adr, &h)) {
			return false;
		}
		(**hnode).val.data.binaryvalue = h;
		(**hnode).val.fldiskval = false;
	}
	return true;
}

static void hashsetlocality(tyvaluerecord *val, boolean fllocal) {
	(void)val;
	(void)fllocal;
}

boolean hashassign(const bigstring bs, tyvaluerecord val) {
	if (val.fltmpdata) {
		if (val.fltmpstack) {
			val.fltmpdata = false;
		} else if (!copyvaluedata(&val)) {
			return false;
		}
	}
	val.fltmpstack = false;
	hashsetlocality(&val, (currenthashtable != nil) ? (**currenthashtable).fllocaltable : false);
	hdlhashnode hnode = nil;
	hdlhashnode hprev = nil;
	if (!hashlocate(bs, &hnode, &hprev)) {
		return hashinsert(bs, val);
	}
	/* replace */
	disposevaluerecord((**hnode).val, true);
	(**hnode).val = val;
	return true;
}

boolean hashtableassign(hdlhashtable htable, const bigstring bs, tyvaluerecord val) {
	boolean fl;
	pushhashtable(htable);
	fl = hashassign(bs, val);
	pophashtable();
	return fl;
}

static boolean hashlookup(const bigstring bs, tyvaluerecord *vreturned, hdlhashnode *hnode) {
	hdlhashnode h = nil;
	if (!hashlocate(bs, &h, nil)) {
		return false;
	}
	if (!hashresolvevalue(currenthashtable, h)) {
		return false;
	}
	if (vreturned) {
		*vreturned = (**h).val;
	}
	if (hnode) {
		*hnode = h;
	}
	return true;
}

boolean hashtablelookup(hdlhashtable htable, const bigstring bs, tyvaluerecord *val, hdlhashnode *hnode) {
	boolean fl;
	pushhashtable(htable);
	fl = hashlookup(bs, val, hnode);
	pophashtable();
	return fl;
}

boolean hashtablelookupnode(hdlhashtable htable, const bigstring bs, hdlhashnode *hnode) {
	boolean fl;
	pushhashtable(htable);
	fl = hashlookup(bs, nil, hnode);
	pophashtable();
	return fl;
}

boolean hashcountitems(hdlhashtable htable, long *ctitems) {
	if (!htable || !ctitems) {
		return false;
	}
	long ct = 0;
	hdlhashnode nomad = (**htable).hfirstsort;
	while (nomad != nil) {
		ct++;
		nomad = (**nomad).sortedlink;
	}
	*ctitems = ct;
	return true;
}

static boolean hashgetnthnode(hdlhashtable htable, long n, hdlhashnode *hnode) {
	hdlhashnode nomad = (**htable).hfirstsort;
	if (!nomad) {
		return false;
	}
	while (n-- > 0) {
		nomad = (**nomad).sortedlink;
		if (!nomad) {
			return false;
		}
	}
	*hnode = nomad;
	return true;
}

boolean hashgetiteminfo(hdlhashtable htable, long item, bigstring bsname, tyvaluerecord *val) {
	hdlhashnode hnode = nil;
	if (!hashgetnthnode(htable, item, &hnode)) {
		return false;
	}
	if (bsname != nil) {
		gethashkey(hnode, bsname);
	}
	if (val != nil) {
		if (!hashresolvevalue(htable, hnode)) {
			return false;
		}
		*val = (**hnode).val;
	}
	return true;
}

static boolean loadlongfromdiskhandle(Handle hget, long *ix, long *x);

static boolean hashpackstring(handlestream *s, bigstring bs, int32_t *ix) {
	*ix = (int32_t)s->pos;
	byte len = bs[0];
	if (!writehandlestream(s, &len, sizeof(len))) {
		return false;
	}
	if (len > 0 && !writehandlestream(s, &bs[1], len)) {
		return false;
	}
	return true;
}

static void hashunpackstring(Handle hget, bigstring bs, long ix) {
	byte len = 0;
	loadfromhandle(hget, &ix, sizeof(len), &len);
	bs[0] = len;
	if (len > 0) {
		loadfromhandle(hget, &ix, len, &bs[1]);
	}
}

static boolean hashpackdata(handlestream *s, void *pdata, long ctbytes, int32_t *ix) {
	*ix = (int32_t)s->pos;
	memtodisklong(ctbytes);
	if (!writehandlestream(s, &ctbytes, sizeof(ctbytes))) {
		return false;
	}
	return writehandlestream(s, pdata, ctbytes);
}

static boolean hashpackbinary(handlestream *s, Handle hbinary, int32_t *ix) {
	long ctbytes = gethandlesize(hbinary);
	*ix = (int32_t)s->pos;
	memtodisklong(ctbytes);
	if (!writehandlestream(s, &ctbytes, sizeof(ctbytes))) {
		return false;
	}
	return writehandlestreamhandle(s, hbinary);
}

static boolean hashunpackbinary(Handle hget, Handle *hbinary, long ix) {
	long ctbytes = 0;
	if (!loadfromhandle(hget, &ix, sizeof(ctbytes), &ctbytes)) {
		return false;
	}
	disktomemlong(ctbytes);
	return loadfromhandletohandle(hget, &ix, ctbytes, false, hbinary);
}

#define maxinlinescalarsize 1023
#define diskvalsizeflag (-1)

static boolean hashpackexternal(handlestream *s, hdlexternalvariable h, int32_t *ix, boolean *flnewdbaddress) {
	Handle hpacked = NULL;
	boolean fl;
	*ix = (int32_t)s->pos;
	if (flnewdbaddress) {
		*flnewdbaddress = false;
	}
	fl = langexternalpack((hdlexternalhandle)h, &hpacked, flnewdbaddress);
	if (!fl) {
		return false;
	}
	unsigned long ctbytes = (unsigned long)gethandlesize(hpacked);
	unsigned long ctstore = ctbytes;
	int32_t ctstore_disk = (int32_t)ctstore;
	memtodisklong(ctstore_disk);
	ctstore = (unsigned long)ctstore_disk;
	if (!writehandlestream(s, &ctstore, sizeof(ctstore))) {
		disposehandle(hpacked);
		return false;
	}
	if (!writehandlestreamhandle(s, hpacked)) {
		disposehandle(hpacked);
		return false;
	}
	disposehandle(hpacked);
	return true;
}

static boolean hashunpackexternal(Handle hget, boolean flmemory, hdlexternalhandle *h, long ix) {
	long ctbytes = 0;
	if (!loadlongfromdiskhandle(hget, &ix, &ctbytes)) {
		return false;
	}
	Handle hpacked = NULL;
	if (!loadfromhandletohandle(hget, &ix, ctbytes, true, &hpacked)) {
		return false;
	}
	boolean fl = flmemory ? langexternalmemoryunpack(hpacked, h) : langexternalunpack(hpacked, h);
	disposehandle(hpacked);
	return fl;
}

#pragma pack(push, 2)
typedef struct tydiskvaluerecord {
	long sizeflag;
	dbaddress adr;
} tydiskvaluerecord;
#pragma pack(pop)

static boolean hashpackscalar(handlestream *s, hdlhashnode hnode, int32_t *ix) {
	Handle hbinary = (**hnode).val.data.binaryvalue;
	unsigned long ctbytes = (unsigned long)gethandlesize(hbinary);
	*ix = (int32_t)s->pos;
	if ((ctbytes > maxinlinescalarsize) && !(**hnode).val.fldiskval) {
		tydiskvaluerecord diskvalue;
		diskvalue.sizeflag = diskvalsizeflag;
		diskvalue.adr = nildbaddress;
		if (!dbassignhandle(hbinary, &diskvalue.adr)) {
			return false;
		}
		memtodisklong(diskvalue.sizeflag);
		memtodisklong(diskvalue.adr);
		return writehandlestream(s, &diskvalue, sizeof(diskvalue));
	}
	long ct = (long)ctbytes;
	memtodisklong(ct);
	if (!writehandlestream(s, &ct, sizeof(ct))) {
		return false;
	}
	return writehandlestreamhandle(s, hbinary);
}

static boolean loadlongfromdiskhandle(Handle hget, long *ix, long *x) {
	int32_t tmp = 0;
	if (!loadfromhandle(hget, ix, (long)sizeof(tmp), &tmp)) {
		return false;
	}
	disktomemlong(tmp);
	if (x) {
		*x = (long)tmp;
	}
	return true;
}

static boolean hashunpackscalar(Handle hget, tyvaluerecord *val, long ix) {
	long ctbytes = 0;
	if (!loadlongfromdiskhandle(hget, &ix, &ctbytes)) {
		return false;
	}
	if (ctbytes == diskvalsizeflag) {
		val->fldiskval = true;
		return loadlongfromdiskhandle(hget, &ix, (long *)&val->data.diskvalue);
	}
	return loadfromhandletohandle(hget, &ix, ctbytes, false, &val->data.binaryvalue);
}

#pragma pack(push, 2)
typedef union tydiskvaluedata {
	int32_t longvalue;
	int16_t intvalue;
	unsigned char chvalue;
} tydiskvaluedata;

typedef struct tydisksymbolrecord {
	int32_t ixkey;
	unsigned char valuetype;
	unsigned char version;
	tydiskvaluedata data;
} tydisksymbolrecord;
#pragma pack(pop)

#pragma pack(push, 2)
typedef struct tydisktablerecord {
	int16_t version;
	int16_t sortorder;
	uint32_t timecreated;
	uint32_t timelastsave;
	int32_t flags;
} tydisktablerecord;
#pragma pack(pop)

#define tablediskversion 0x03

#pragma pack(push, 2)
typedef struct typackinforecord {
	handlestream s1;
	handlestream s2;
	boolean flmustsave;
} typackinforecord;
#pragma pack(pop)

static boolean hashpackvisit(bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
	typackinforecord *lpi = (typackinforecord *)refcon;
	tydisksymbolrecord rec;
	bigstring bsvalue;
	long size;
	clearbytes(&rec, sizeof(rec));
	if (!hashpackstring(&lpi->s2, bsname, &rec.ixkey)) {
		return true;
	}
	memtodisklong(rec.ixkey);
	rec.valuetype = (unsigned char)val.valuetype;
	rec.data.longvalue = val.data.longvalue;
	size = sizeof(long);
	switch (val.valuetype) {
		case ty_oldstringvaluetype:
			copyheapstring((hdlstring)val.data.stringvalue, bsvalue);
			if (!hashpackstring(&lpi->s2, bsvalue, &rec.data.longvalue)) {
				return true;
			}
			break;
		case ty_addressvaluetype:
			if (!getaddresspath(val, bsvalue)) {
				setemptystring(bsvalue);
			}
			if (!hashpackstring(&lpi->s2, bsvalue, &rec.data.longvalue)) {
				return true;
			}
			break;
		case ty_stringvaluetype:
		case ty_passwordvaluetype:
		case ty_patternvaluetype:
		case ty_objspecvaluetype:
		case ty_binaryvaluetype:
		case ty_filespecvaluetype:
		case ty_aliasvaluetype:
			if (!hashpackscalar(&lpi->s2, hnode, &rec.data.longvalue)) {
				return true;
			}
			break;
		case ty_externalvaluetype: {
			boolean flnew = false;
			if (!hashpackexternal(&lpi->s2, (hdlexternalvariable)val.data.externalvalue, &rec.data.longvalue, &flnew)) {
				return true;
			}
			if (flnew) {
				lpi->flmustsave = true;
			}
			break;
		}
		case ty_novaluetype:
		case ty_booleanvaluetype:
		case ty_charvaluetype:
			rec.data.chvalue = val.data.chvalue;
			size = sizeof(char);
			break;
		case ty_intvaluetype:
		case ty_tokenvaluetype:
			rec.data.intvalue = val.data.intvalue;
			size = sizeof(short);
			break;
		case ty_pointvaluetype: {
			Point diskpt = val.data.pointvalue;
			memtodiskshort(diskpt.h);
			memtodiskshort(diskpt.v);
			memcpy(&rec.data.longvalue, &diskpt, sizeof(diskpt));
			size = 0;
			break;
		}
		case ty_longvaluetype:
		case ty_ostypevaluetype:
		case ty_enumvaluetype:
		case ty_fixedvaluetype:
		case ty_singlevaluetype:
			rec.data.longvalue = val.data.longvalue;
			size = sizeof(long);
			break;
		case ty_directionvaluetype:
			rec.data.intvalue = (int16_t)val.data.dirvalue;
			size = sizeof(short);
			break;
		case ty_datevaluetype:
			rec.data.longvalue = (int32_t)val.data.datevalue;
			size = sizeof(long);
			break;
		case ty_listvaluetype:
		case ty_recordvaluetype:
			return true;
		default:
			break;
	}
	if (size == sizeof(long)) {
		memtodisklong(rec.data.longvalue);
	}
	if (size == sizeof(short)) {
		memtodiskshort(rec.data.intvalue);
	}
	if (!writehandlestream(&lpi->s1, &rec, sizeof(rec))) {
		return true;
	}
	return false;
}

boolean hashpacktable(hdlhashtable htable, boolean flmemory, Handle *hpackedtable, boolean *flmustsave) {
	tydisktablerecord header;
	typackinforecord packrec;
	Handle h1 = NULL;
	Handle h2 = NULL;
	clearbytes(&header, sizeof(header));
	header.version = tablediskversion;
	memtodiskshort(header.version);
	header.timecreated = (**htable).timecreated;
	header.timelastsave = (**htable).timelastsave;
	header.sortorder = (**htable).sortorder;
	int32_t tc = (int32_t)header.timecreated;
	int32_t ts = (int32_t)header.timelastsave;
	memtodisklong(tc);
	memtodisklong(ts);
	header.timecreated = (uint32_t)tc;
	header.timelastsave = (uint32_t)ts;
	memtodiskshort(header.sortorder);
	clearbytes(&packrec, sizeof(packrec));
	packrec.flmustsave = flmustsave ? *flmustsave : false;
	openhandlestream(nil, &packrec.s1);
	openhandlestream(nil, &packrec.s2);
	if (!writehandlestream(&packrec.s1, &header, sizeof(header))) {
		return false;
	}
	hdlhashtable prev = sethashtable(htable);
	hdlhashnode nomad = (**htable).hfirstsort;
	while (nomad != nil) {
		bigstring bsname;
		gethashkey(nomad, bsname);
		if (hashpackvisit(bsname, nomad, (**nomad).val, &packrec)) {
			sethashtable(prev);
			disposehandlestream(&packrec.s1);
			disposehandlestream(&packrec.s2);
			return false;
		}
		nomad = (**nomad).sortedlink;
	}
	sethashtable(prev);
	h1 = closehandlestream(&packrec.s1);
	h2 = closehandlestream(&packrec.s2);
	if (!mergehandles(h1, h2, hpackedtable)) {
		return false;
	}
	if (flmustsave) {
		*flmustsave = packrec.flmustsave;
	}
	return true;
}

boolean hashunpacktable(Handle hpackedtable, boolean flmemory, hdlhashtable htable) {
	Handle hrecords = nil;
	Handle hstrings = nil;
	long total = hpackedtable ? gethandlesize(hpackedtable) : 0;
	if (total >= (long)(sizeof(int32_t) + sizeof(tydisktablerecord)) && hpackedtable && *hpackedtable) {
		int32_t recordbytes = 0;
		memcpy(&recordbytes, *hpackedtable, sizeof(recordbytes));
		disktomemlong(recordbytes);
		if (recordbytes > 0 && recordbytes <= total - (long)sizeof(int32_t)) {
			long stringsbytes = total - (long)sizeof(int32_t) - recordbytes;
			if (!newhandle(recordbytes, &hrecords)) {
				return false;
			}
			memcpy(*hrecords, (unsigned char *)*hpackedtable + sizeof(int32_t), (size_t)recordbytes);
			if (stringsbytes > 0) {
				if (!newhandle(stringsbytes, &hstrings)) {
					disposehandle(hrecords);
					return false;
				}
				memcpy(*hstrings,
					(unsigned char *)*hpackedtable + sizeof(int32_t) + recordbytes,
					(size_t)stringsbytes);
			}
			disposehandle(hpackedtable);
		}
	}
	if (!hrecords && !hstrings) {
		if (!unmergehandles(hpackedtable, &hrecords, &hstrings)) {
			return false;
		}
	}
	long ix = 0;
	int32_t count = 0;
	int16_t version = 0;
	int16_t sortorder = 0;
	uint32_t timecreated = 0;
	uint32_t timelastsave = 0;
	int32_t flags = 0;
	boolean has_count = false;
	long peek = 0;
	if (!loadfromhandle(hrecords, &peek, sizeof(count), &count)) {
		disposehandle(hrecords);
		disposehandle(hstrings);
		return false;
	}
	if (!loadfromhandle(hrecords, &peek, sizeof(version), &version)) {
		disposehandle(hrecords);
		disposehandle(hstrings);
		return false;
	}
	disktomemshort(version);
	if (version == tablediskversion || version == 2 || version == 1) {
		has_count = true;
		ix = peek;
		if (!loadfromhandle(hrecords, &ix, sizeof(sortorder), &sortorder)
			|| !loadfromhandle(hrecords, &ix, sizeof(timecreated), &timecreated)
			|| !loadfromhandle(hrecords, &ix, sizeof(timelastsave), &timelastsave)
			|| !loadfromhandle(hrecords, &ix, sizeof(flags), &flags)) {
			disposehandle(hrecords);
			disposehandle(hstrings);
			return false;
		}
		disktomemshort(sortorder);
		disktomemlong(timecreated);
		disktomemlong(timelastsave);
		disktomemlong(flags);
		(**htable).sortorder = sortorder;
		(**htable).timecreated = timecreated;
		(**htable).timelastsave = timelastsave;
	} else {
		ix = 0;
		tydisktablerecord header;
		if (!loadfromhandle(hrecords, &ix, sizeof(header), &header)) {
			disposehandle(hrecords);
			disposehandle(hstrings);
			return false;
		}
		version = header.version;
		disktomemshort(version);
		if (version > 0) {
			header.version = version;
			disktomemshort(header.sortorder);
			int32_t tc = (int32_t)header.timecreated;
			int32_t ts = (int32_t)header.timelastsave;
			disktomemlong(tc);
			disktomemlong(ts);
			header.timecreated = (uint32_t)tc;
			header.timelastsave = (uint32_t)ts;
			(**htable).sortorder = header.sortorder;
			(**htable).timecreated = header.timecreated;
			(**htable).timelastsave = header.timelastsave;
		} else {
			ix = 0;
			(**htable).timecreated = timenow();
			(**htable).timelastsave = (**htable).timecreated;
		}
	}
	while (true) {
		tydisksymbolrecord rec;
		long start = ix;
		if (!loadfromhandle(hrecords, &ix, sizeof(rec), &rec)) {
			break;
		}
		if (ix == start) {
			break;
		}
		disktomemlong(rec.ixkey);
		int32_t ixstrings = rec.data.longvalue;
		disktomemlong(ixstrings);
		bigstring bsname;
		hashunpackstring(hstrings, bsname, rec.ixkey);
		if (isemptystring(bsname)) {
			continue;
		}
		tyvaluerecord val;
		initvalue(&val, (tyvaluetype)rec.valuetype);
		memset(&val.data, 0, sizeof(val.data));
		memcpy(&val.data, &rec.data, sizeof(rec.data));
		switch (val.valuetype) {
			case ty_oldstringvaluetype: {
				bigstring bsvalue;
				hashunpackstring(hstrings, bsvalue, ixstrings);
				if (!newheapstring(bsvalue, (hdlstring *)&val.data.stringvalue)) {
					continue;
				}
				break;
			}
			case ty_addressvaluetype: {
				bigstring bsvalue;
				hashunpackstring(hstrings, bsvalue, ixstrings);
				setaddressvalue((hdlhashtable)-1, bsvalue, &val);
				break;
			}
			case ty_stringvaluetype:
			case ty_passwordvaluetype:
			case ty_patternvaluetype:
			case ty_objspecvaluetype:
			case ty_binaryvaluetype:
			case ty_filespecvaluetype:
			case ty_aliasvaluetype:
				if (!hashunpackscalar(hstrings, &val, ixstrings)) {
					continue;
				}
				break;
			case ty_externalvaluetype:
				if (!hashunpackexternal(hstrings, flmemory, (hdlexternalhandle *)&val.data.externalvalue, ixstrings)) {
					continue;
				}
				break;
			case ty_novaluetype:
			case ty_booleanvaluetype:
			case ty_charvaluetype:
				break;
			case ty_intvaluetype:
			case ty_tokenvaluetype:
				disktomemshort(val.data.intvalue);
				break;
			case ty_pointvaluetype:
				disktomemshort(val.data.pointvalue.h);
				disktomemshort(val.data.pointvalue.v);
				break;
			case ty_longvaluetype:
			case ty_ostypevaluetype:
			case ty_enumvaluetype:
			case ty_fixedvaluetype:
			case ty_singlevaluetype:
			case ty_datevaluetype:
				disktomemlong(val.data.longvalue);
				break;
			case ty_directionvaluetype:
				disktomemshort(val.data.intvalue);
				break;
			default:
				break;
		}
		hdlhashtable prev = sethashtable(htable);
		hashinsert(bsname, val);
		sethashtable(prev);
	}
	disposehandle(hrecords);
	disposehandle(hstrings);
	return true;
}

boolean disposehashtable(hdlhashtable htable, boolean fldisk) {
	(void)fldisk;
	if (!htable) {
		return true;
	}
	for (short i = 0; i < ctbuckets; ++i) {
		hdlhashnode nomad = (**htable).hashbucket[i];
		while (nomad != nil) {
			hdlhashnode next = (**nomad).hashlink;
			disposehashnode(htable, nomad, true);
			nomad = next;
		}
	}
	(**htable).prevhashtable = hfirstfreetable;
	hfirstfreetable = htable;
	return true;
}
