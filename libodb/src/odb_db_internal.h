#ifndef ODB_DB_INTERNAL_H
#define ODB_DB_INTERNAL_H

#include "odb_compat.h"
#include "odb_byteorder.h"

#define nildbaddress 0L

#define ctviews 3
#define cancoonview 0

#define SMART_DB_OPENING 1

typedef int32_t dbaddress, *ptrdbaddress, **hdldbaddress;

#pragma pack(push, 2)
typedef struct tydatabaserecord {
	unsigned char systemid;
	unsigned char versionnumber;
	dbaddress availlist;
	int16_t oldfnumdatabase;
	int16_t flags;
	dbaddress views[ctviews];
	uint32_t releasestack;
	int32_t fnumdatabase;
	int32_t headerLength;
	int16_t longversionMajor;
	int16_t longversionMinor;
	union {
		char growthspace[50];
		struct {
			dbaddress availlistblock;
			handlestream availlistshadow;
			boolean flreadonly;
		} extensions;
	} u;
} tydatabaserecord, *ptrdatabaserecord, **hdldatabaserecord;
#pragma pack(pop)

extern hdldatabaserecord databasedata;
extern boolean fldatabasesaveas;

#define dbversionnumber 6
#define dbfirstversionwithcachedshadowavaillist 6
#define dbversionnumberminor 0

#define dbsystemidMac 0
#define dbsystemidWin32 1

#define minblocksize 32L
#define sizeheader (long)sizeof(tyheader)
#define sizetrailer (long)sizeof(tytrailer)
#define firstphysicaladdress (long)sizeof(tydatabaserecord)

typedef enum {
	dbdirtymask = 0x0001
} tydbflagmask;

typedef int32_t tyvariance;

#pragma pack(push, 2)
typedef struct tysizefreeword {
	int32_t size;
} tysizefreeword;

typedef struct tyheader {
	tysizefreeword sizefreeword;
	tyvariance variance;
} tyheader, *ptrheader, **hdlheader;

typedef struct tytrailer {
	tysizefreeword sizefreeword;
} tytrailer, *ptrtrailer, **hdltrailer;
#pragma pack(pop)

/* db.c prototypes */
boolean dbpushdatabase(hdldatabaserecord);
boolean dbpopdatabase(void);
boolean dbreference(dbaddress, long, ptrvoid);
boolean dbrefhandle(dbaddress, Handle *);
boolean dbassign(dbaddress *, long, ptrvoid);
boolean dbcopy(dbaddress, dbaddress *);
boolean dballochandle(Handle, dbaddress *);
boolean dbassignhandle(Handle, dbaddress *);
boolean dbsavehandle(Handle, dbaddress *);
boolean dbrefheapstring(dbaddress, hdlstring *);
boolean dbassignheapstring(dbaddress *, hdlstring);
void dbsetview(short, dbaddress);
void dbgetview(short, dbaddress *);
void dbcurrentdatabase(hdldatabaserecord);
void dbgetcurrentdatabase(hdldatabaserecord *);
boolean dbflushreleasestack(void);
boolean dbfnumchanged(hdlfilenum);
boolean dbdispose(void);
boolean dbnew(hdlfilenum);
boolean dbopenfile(hdlfilenum, boolean);
boolean dbclose(void);
boolean dbstartsaveas(hdlfilenum);
boolean dbendsaveas(void);
boolean dbsave(void);

#endif
