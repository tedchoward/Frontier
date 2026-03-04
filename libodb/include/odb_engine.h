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

#define ODB_FOURCC(a, b, c, d) \
	((OSType)(((uint32_t)(uint8_t)(a) << 24) | ((uint32_t)(uint8_t)(b) << 16) | ((uint32_t)(uint8_t)(c) << 8) | (uint32_t)(uint8_t)(d)))

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

typedef struct odbRGBColor {
	uint16_t red;
	uint16_t green;
	uint16_t blue;
} odbRGBColor;

typedef struct odbPattern {
	uint8_t bytes[8];
} odbPattern;

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
	unknownvaluetype = ODB_FOURCC('?', '?', '?', '?'),
	charvaluetype = ODB_FOURCC('c', 'h', 'a', 'r'),
	shortvaluetype = ODB_FOURCC('s', 'h', 'o', 'r'),
	longvaluetype = ODB_FOURCC('l', 'o', 'n', 'g'),
	binaryvaluetype = ODB_FOURCC('d', 'a', 't', 'a'),
	booleanvaluetype = ODB_FOURCC('b', 'o', 'o', 'l'),
	tokenvaluetype = ODB_FOURCC('t', 'o', 'k', 'n'),
	datevaluetype = ODB_FOURCC('d', 'a', 't', 'e'),
	addressvaluetype = ODB_FOURCC('a', 'd', 'd', 'r'),
	codevaluetype = ODB_FOURCC('c', 'o', 'd', 'e'),
	extendedvaluetype = ODB_FOURCC('e', 'x', 't', 'e'),
	stringvaluetype = ODB_FOURCC('T', 'E', 'X', 'T'),
	externalvaluetype = ODB_FOURCC('x', 't', 'r', 'n'),
	directionvaluetype = ODB_FOURCC('d', 'i', 'r', ' '),
	string4valuetype = ODB_FOURCC('t', 'y', 'p', 'e'),
	pointvaluetype = ODB_FOURCC('Q', 'D', 'p', 't'),
	rectvaluetype = ODB_FOURCC('q', 'd', 'r', 't'),
	patternvaluetype = ODB_FOURCC('t', 'p', 't', 'n'),
	rgbvaluetype = ODB_FOURCC('c', 'R', 'G', 'B'),
	fixedvaluetype = ODB_FOURCC('f', 'i', 'x', 'd'),
	singlevaluetype = ODB_FOURCC('s', 'i', 'n', 'g'),
	doublevaluetype = ODB_FOURCC('d', 'o', 'u', 'b'),
	objspecvaluetype = ODB_FOURCC('o', 'b', 'j', ' '),
	filespecvaluetype = ODB_FOURCC('f', 's', 's', ' '),
	aliasvaluetype = ODB_FOURCC('a', 'l', 'i', 's'),
	enumeratorvaluetype = ODB_FOURCC('e', 'n', 'u', 'm'),
	listvaluetype = ODB_FOURCC('l', 'i', 's', 't'),
	recordvaluetype = ODB_FOURCC('r', 'e', 'c', 'o'),
	outlinevaluetype = ODB_FOURCC('o', 'p', 't', 'x'),
	wptextvaluetype = ODB_FOURCC('w', 'p', 't', 'x'),
	tablevaluetype = ODB_FOURCC('t', 'a', 'b', 'l'),
	scriptvaluetype = ODB_FOURCC('s', 'c', 'p', 't'),
	menubarvaluetype = ODB_FOURCC('m', 'b', 'a', 'r'),
	picturevaluetype = ODB_FOURCC('p', 'i', 'c', 't')
} odbValueType;

typedef union odbValueData {
	odbBool flvalue;
	unsigned char chvalue;
	int16_t intvalue;
	int16_t tokenvalue;
	int32_t longvalue;
	int32_t fixedvalue;
	uint32_t datevalue;
	odbDirection dirvalue;
	OSType ostypevalue;
	Handle stringvalue;
	Handle addressvalue;
	Handle binaryvalue;
	Handle externalvalue;
	odbPoint pointvalue;
	odbRect **rectvalue;
	odbPattern **patternvalue;
	odbRGBColor **rgbvalue;
	float singlevalue;
	double **doublevalue;
	Handle objspecvalue;
	Handle filespecvalue;
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
