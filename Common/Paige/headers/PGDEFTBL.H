/* Default look-up tables for PAIGE import-export mechanism. Copyright 1995-96 DataPak Software,
Inc.  Software by Gar. */

#ifndef PGDEFTBL_H
#define PGDEFTBL_H

#include "CPUDEFS.H"
#include "pgtxr.h"

#ifdef __cplusplus
extern "C" {
#endif

extern pg_char PG_FAR cross_font_table[];
extern pg_char PG_FAR font_type_table[];
extern pg_char PG_FAR def_font_type[];
extern pg_char PG_FAR cross_character_table[HIGH_CHARS_QTY];

#ifdef __cplusplus
	};
#endif

#ifdef MAC_PLATFORM

#define BULLET_CROSS_CHAR			0x95
#define LEFTQUOTE_CROSS_CHAR		0x91
#define RIGHTQUOTE_CROSS_CHAR		0x92
#define LEFTDBLQUOTE_CROSS_CHAR		0x93
#define RIGHTDBLQUOTE_CROSS_CHAR	0x94

#define FILETYPE_STR_SIZE	4

#ifdef UNICODE
#define FILETYPE_TEXT	L"TEXT"
#define FILETYPE_RTF	L"TEXT"
#define FILETYPE_PAIGE	L"paig"
#else
#define FILETYPE_TEXT	"TEXT"
#define FILETYPE_RTF	"TEXT"
#define FILETYPE_PAIGE	"paig"
#endif
#endif

#ifdef WINDOWS_PLATFORM

#define BULLET_CROSS_CHAR			0xA5
#define LEFTQUOTE_CROSS_CHAR		0xD4
#define RIGHTQUOTE_CROSS_CHAR		0xD5
#define LEFTDBLQUOTE_CROSS_CHAR		0xD2
#define RIGHTDBLQUOTE_CROSS_CHAR	0xD3

#define FILETYPE_STR_SIZE	3

#ifdef UNICODE
#define FILETYPE_TEXT	L"TXT"
#define FILETYPE_RTF	L"RTF"
#define FILETYPE_PAIGE	L"PGF"
#else
#define FILETYPE_TEXT	"TXT"
#define FILETYPE_RTF	"RTF"
#define FILETYPE_PAIGE	"PGF"
#endif
#endif

#endif

