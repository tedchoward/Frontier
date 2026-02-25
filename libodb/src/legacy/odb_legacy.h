#ifndef ODB_LEGACY_H
#define ODB_LEGACY_H

#include "odb_compat.h"
#include "odb_db_internal.h"
#include "odb_engine.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HNoNode ((hdlhashnode) -1)
#define ctbuckets 11
#define cthashtables 80

typedef int16_t tydirection;

typedef struct {
	int16_t top;
	int16_t left;
	int16_t bottom;
	int16_t right;
} Rect;

typedef struct {
	int16_t v;
	int16_t h;
} Point;

typedef struct {
	uint16_t red;
	uint16_t green;
	uint16_t blue;
} RGBColor;

typedef int32_t Fixed;

typedef void *hdlfilespec;

typedef struct tytreenode tytreenode;

typedef struct tylistrecord tylistrecord;

typedef enum tyvaluetype {
	ty_uninitializedvaluetype = -1,
	ty_novaluetype = 0,
	ty_charvaluetype = 1,
	ty_intvaluetype = 2,
	ty_longvaluetype = 3,
	ty_oldstringvaluetype = 4,
	ty_binaryvaluetype = 5,
	ty_booleanvaluetype = 6,
	ty_tokenvaluetype = 7,
	ty_datevaluetype = 8,
	ty_addressvaluetype = 9,
	ty_codevaluetype = 10,
	ty_doublevaluetype = 11,
	ty_stringvaluetype = 12,
	ty_externalvaluetype = 13,
	ty_directionvaluetype = 14,
	ty_passwordvaluetype = 15,
	ty_ostypevaluetype = 16,
	ty_unused2valuetype = 17,
	ty_pointvaluetype = 18,
	ty_rectvaluetype = 19,
	ty_patternvaluetype = 20,
	ty_rgbvaluetype = 21,
	ty_fixedvaluetype = 22,
	ty_singlevaluetype = 23,
	ty_olddoublevaluetype = 24,
	ty_objspecvaluetype = 25,
	ty_filespecvaluetype = 26,
	ty_aliasvaluetype = 27,
	ty_enumvaluetype = 28,
	ty_listvaluetype = 29,
	ty_recordvaluetype = 30,
	ty_outlinevaluetype,
	ty_wordvaluetype,
	ty_headvaluetype,
	ty_tablevaluetype,
	ty_scriptvaluetype,
	ty_menuvaluetype,
	ty_pictvaluetype,
	ty_ctvaluetypes
} tyvaluetype;

#pragma pack(push, 2)
typedef struct tydiskvalue {
	dbaddress adr;
	Handle hvalue;
} tydiskvalue, *ptrdiskvalue, **hdldiskvalue;
#pragma pack(pop)

typedef union tyvaluedata {
	boolean flvalue;
	byte chvalue;
	int16_t intvalue;
	int32_t longvalue;
	uint32_t datevalue;
	tydirection dirvalue;
	OSType ostypevalue;
	Handle stringvalue;
	hdlstring addressvalue;
	Handle binaryvalue;
	int16_t tokenvalue;
	tytreenode **codevalue;
	Handle externalvalue;
	Point pointvalue;
	Rect **rectvalue;
	void **patternvalue;
	RGBColor **rgbvalue;
	Fixed fixedvalue;
	float singlevalue;
	double **doublevalue;
	Handle objspecvalue;
	hdlfilespec filespecvalue;
	Handle aliasvalue;
	OSType enumvalue;
	tylistrecord **listvalue;
	tylistrecord **recordvalue;
	dbaddress diskvalue;
} tyvaluedata;

#pragma pack(push, 2)
typedef struct tyvaluerecord {
	tyvaluetype valuetype;
	byte fltmpdata: 1;
	byte fltmpstack: 1;
	byte flformalval: 1;
	byte fldiskval: 1;
	tyvaluedata data;
} tyvaluerecord;
#pragma pack(pop)

#pragma pack(push, 2)
typedef struct tyhashnode {
	struct tyhashnode **hashlink;
	struct tyhashnode **sortedlink;
	tyvaluerecord val;
	byte fldontsave: 1;
	byte fllocked: 1;
	byte flprotected: 1;
	byte flunresolvedaddress: 1;
	byte fldisposewhenunlocked: 1;
	byte flunused: 3;
	byte ctlocks: 8;
	byte hashkey[];
} tyhashnode, *ptrhashnode, **hdlhashnode;
#pragma pack(pop)

#define gethashkey(h, bs) copystring((**(h)).hashkey, (bs));

#pragma pack(push, 2)
typedef struct tyhashtable {
	hdlhashnode hashbucket[ctbuckets];
	hdlhashnode hfirstsort;
	struct tyhashtable **prevhashtable;
	struct tyhashtable **parenthashtable;
	hdlhashnode thistableshashnode;
	boolean fldirty: 1;
	boolean fllocked: 1;
	boolean flwindowopen: 1;
	boolean flnopurge: 1;
	boolean fllocaltable: 1;
	boolean flchained: 1;
	boolean fldisposewhenunchained: 1;
	boolean ctwithvalues: 3;
	boolean flverbsrequirewindow: 1;
	boolean flneedsort: 1;
	boolean flmayaffectdisplay: 1;
	boolean flsubsdirty: 1;
	intptr_t hashtablerefcon;
	intptr_t lexicalrefcon;
	void **hashtableformats;
	int16_t sortorder;
	uint32_t timecreated;
	uint32_t timelastsave;
	void *valueroutine;
	int16_t cttmpstack;
	tyvaluerecord tmpstack[];
} tyhashtable, *ptrhashtable, **hdlhashtable;
#pragma pack(pop)

#pragma pack(push, 2)
typedef struct tablestack {
	int16_t toptables;
	hdlhashtable stack[cthashtables];
} tytablestack, *ptrtablestack, **hdltablestack;
#pragma pack(pop)

extern hdlhashtable currenthashtable;
extern hdltablestack hashtablestack;

extern hdlhashtable roottable;
extern Handle rootvariable;
extern hdlhashtable systemtable;
extern hdlhashtable pathstable;
extern hdlhashtable filewindowtable;

extern boolean fllanghashassignprotect;
extern boolean fllangexternalvalueprotect;
extern boolean flscriptrunning;
extern boolean fldatabasesaveas;

/* value helpers */
void initvalue(tyvaluerecord *val, tyvaluetype type);
boolean copyvaluerecord(tyvaluerecord val, tyvaluerecord *dest);
boolean copyvaluedata(tyvaluerecord *val);
void disposevaluerecord(tyvaluerecord val, boolean fldisk);
void exemptfromtmpstack(tyvaluerecord *val);
boolean setbinaryvalue(Handle h, OSType typeid, tyvaluerecord *val);
boolean setaddressvalue(hdlhashtable htable, const bigstring bs, tyvaluerecord *val);
boolean getaddresspath(tyvaluerecord val, bigstring bs);

/* external values */
typedef enum tyexternalid {
	idoutlineprocessor,
	idwordprocessor,
	idheadrecord,
	idtableprocessor,
	idscriptprocessor,
	idmenuprocessor,
	idpictprocessor,
	idappleprocessor,
	idcardprocessor,
	ctexternalprocessors
} tyexternalid;

#pragma pack(push, 2)
typedef struct tyexternalvariable {
	uint16_t id;
	uint16_t flinmemory: 1;
	uint16_t flmayaffectdisplay: 1;
	uint16_t flpacked: 1;
	uint16_t flscript: 1;
	uint16_t flsystemtable: 1;
	uint16_t flopaque: 1;
	uint16_t reserved: 10;
	intptr_t variabledata;
	hdldatabaserecord hdatabase;
	dbaddress oldaddress;
} tyexternalvariable, *ptrexternalvariable, **hdlexternalvariable;
#pragma pack(pop)

typedef hdlexternalvariable hdlexternalhandle;

typedef struct tydiskexternalhandle {
	int16_t versionnumber;
	uint16_t id;
} tydiskexternalhandle;

#define externaldiskversionnumber 1

boolean langnewexternalvariable(boolean flinmemory, intptr_t variabledata, hdlexternalvariable *h);
boolean langexternalnewvalue(tyexternalid id, Handle hdata, tyvaluerecord *val);
OSType langexternalgettypeid(tyvaluerecord val);
tyvaluetype langexternalgetvaluetype(OSType typeid);
boolean langexternalvaltotable(tyvaluerecord val, hdlhashtable *htable, hdlhashnode hnode);
hdldatabaserecord langexternalgetdatabase(hdlexternalvariable hv);
void langexternalsetdatabase(hdlexternalvariable hv, hdldatabaserecord hdb);
boolean langexternalpack(hdlexternalhandle h, Handle *hpacked, boolean *flnewdbaddress);
boolean langexternalunpack(Handle hpacked, hdlexternalhandle *h);
boolean langexternalmemorypack(hdlexternalhandle h, Handle *hpacked, hdlhashnode hnode);
boolean langexternalmemoryunpack(Handle hpacked, hdlexternalhandle *h);

/* table helpers */
boolean tablenewtable(hdlexternalvariable *hvariable, hdlhashtable *htable);
boolean tablevaltotable(tyvaluerecord val, hdlhashtable *htable, hdlhashnode hnode);
boolean tableverbinmemory(hdlexternalvariable hvariable, hdlhashnode hnode);
boolean tablepacktable(hdlhashtable htable, boolean flmemory, Handle *hpacked, boolean *flmustsave);
boolean tableunpacktable(Handle hpacked, boolean flmemory, hdlhashtable *htable);
boolean tableverbpack(hdlexternalvariable h, Handle *hpacked, boolean *flnewdbaddress);
boolean tableverbunpack(Handle hpacked, long *ixload, hdlexternalvariable *h, boolean flxml);

boolean tableloadsystemtable(dbaddress adr, Handle *hvariable, hdlhashtable *htable, boolean flcreate);
boolean tablesavesystemtable(Handle hvariable, dbaddress *adr);
boolean settablestructureglobals(Handle hvariable, boolean flcreatesubs);
boolean cleartablestructureglobals(void);
boolean tabledisposetable(hdlhashtable htable, boolean fldisk);

/* hash table core */
boolean newhashtable(hdlhashtable *htable);
boolean disposehashtable(hdlhashtable htable, boolean fldisk);
boolean pushhashtable(hdlhashtable h);
boolean pophashtable(void);
hdlhashtable sethashtable(hdlhashtable hset);
boolean hashtableassign(hdlhashtable htable, const bigstring bs, tyvaluerecord val);
boolean hashtablelookup(hdlhashtable htable, const bigstring bs, tyvaluerecord *val, hdlhashnode *hnode);
boolean hashtablelookupnode(hdlhashtable htable, const bigstring bs, hdlhashnode *hnode);
boolean hashtabledelete(hdlhashtable htable, bigstring bs);
boolean hashsymbolexists(const bigstring bs);
boolean hashtablesymbolexists(hdlhashtable htable, const bigstring bs);
boolean hashcountitems(hdlhashtable htable, long *ctitems);
boolean hashgetiteminfo(hdlhashtable htable, long item, bigstring bsname, tyvaluerecord *val);
boolean hashpacktable(hdlhashtable htable, boolean flmemory, Handle *hpackedtable, boolean *flmustsave);
boolean hashunpacktable(Handle hpackedtable, boolean flmemory, hdlhashtable htable);

#ifdef __cplusplus
}
#endif

#endif
