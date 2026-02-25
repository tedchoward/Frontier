/*
 * ODB Engine API (Frontier 5) - adapted for libodb
 * Original copyright 1996 UserLand Software, Inc.
 */

#ifndef ODB_ENGINE_H
#define ODB_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Cross-platform, modernized base types.
 * File handles are opaque to callers; use odbOpenPath/odbNewPath helpers.
 */

typedef struct odb_ *odbRef;

typedef struct odb_file odbFile;

typedef odbFile *hdlfilenum;

typedef uint8_t odbBool;

typedef uint32_t OSType;

typedef unsigned char odbString[256];

typedef struct odbRect {
	int16_t top;
	int16_t left;
	int16_t bottom;
	int16_t right;
} odbRect;

typedef struct odbPoint {
	int16_t v;
	int16_t h;
} odbPoint;

/* Handle abstraction (classic pointer-to-pointer style). */
typedef unsigned char **Handle;

typedef enum odbDirection {
	nodirection = 0,
	dir_up = 1,
	dir_down = 2,
	dir_left = 3,
	dir_right = 4,
	dir_flatup = 5,
	dir_flatdown = 6,
	dir_sorted = 8,
	dir_pageup = 9,
	dir_pagedown = 10,
	dir_pageleft = 11,
	dir_pageright = 12
} odbDirection;

typedef enum odbValueType {
	unknownvaluetype = '\?\?\?\?',
	charvaluetype = 'char',
	shortvaluetype = 'shor',
	longvaluetype = 'long',
	binaryvaluetype = 'data',
	booleanvaluetype = 'bool',
	tokenvaluetype = 'tokn',
	datevaluetype = 'date',
	addressvaluetype = 'addr',
	codevaluetype = 'code',
	extendedvaluetype = 'exte',
	stringvaluetype = 'TEXT',
	externalvaluetype = 'xtrn',
	directionvaluetype = 'dir ',
	string4valuetype = 'type',
	pointvaluetype = 'QDpt',
	rectvaluetype = 'qdrt',
	patternvaluetype = 'tptn',
	rgbvaluetype = 'cRGB',
	fixedvaluetype = 'fixd',
	singlevaluetype = 'sing',
	doublevaluetype = 'doub',
	objspecvaluetype = 'obj ',
	filespecvaluetype = 'fss ',
	aliasvaluetype = 'alis',
	enumeratorvaluetype = 'enum',
	listvaluetype = 'list',
	recordvaluetype = 'reco',
	outlinevaluetype = 'optx',
	wptextvaluetype = 'wptx',
	tablevaluetype = 'tabl',
	scriptvaluetype = 'scpt',
	menubarvaluetype = 'mbar',
	picturevaluetype = 'pict'
} odbValueType;

typedef union odbValueData {
	odbBool flvalue;
	unsigned char chvalue;
	int16_t intvalue;
	int32_t longvalue;
	uint32_t datevalue;
	odbDirection dirvalue;
	OSType ostypevalue;
	Handle stringvalue;
	Handle addressvalue;
	Handle binaryvalue;
	Handle externalvalue;
	odbPoint pointvalue;
	odbRect **rectvalue;
	float singlevalue;
	double **doublevalue;
	Handle objspecvalue;
	Handle aliasvalue;
	OSType enumvalue;
	Handle listvalue;
	Handle recordvalue;
} odbValueData;

typedef struct odbValueRecord {
	odbValueType valuetype;
	odbValueData data;
} odbValueRecord;

/* Core API (Frontier 5). */
extern odbBool odbNewFile(hdlfilenum);
extern odbBool odbOpenFile(hdlfilenum, odbRef *odb);
extern odbBool odbSaveFile(odbRef odb);
extern odbBool odbCloseFile(odbRef odb);
extern odbBool odbDefined(odbRef odb, odbString bspath);
extern odbBool odbDelete(odbRef odb, odbString bspath);
extern odbBool odbGetType(odbRef odb, odbString bspath, OSType *type);
extern odbBool odbGetValue(odbRef odb, odbString bspath, odbValueRecord *value);
extern odbBool odbSetValue(odbRef odb, odbString bspath, odbValueRecord *value);
extern odbBool odbNewTable(odbRef odb, odbString bspath);
extern odbBool odbCountItems(odbRef odb, odbString bspath, int32_t *count);
extern odbBool odbGetNthItem(odbRef odb, odbString bspath, int32_t n, odbString bsname);
extern odbBool odbGetModDate(odbRef odb, odbString bspath, uint32_t *date);
extern void odbDisposeValue(odbRef odb, odbValueRecord *value);
extern void odbGetError(odbString bs);

/* libodb extensions */
extern int odbGetLastErrorCode(void);

/* Path-based helpers (opaque file handles). */
extern odbBool odbOpenPath(const char *path, odbRef *odb);
extern odbBool odbNewPath(const char *path);

#ifdef __cplusplus
}
#endif

#endif
