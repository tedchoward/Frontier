
/*	$Id$    */

/* Paige Technology, copyright 1993-1995 by DataPak Software, Inc.
This header file defines all the components for the built-in import-export mechanism,
including the generic Import and Export classes and default member functions for file
transfer. */

#ifndef PGTXR_H
#define PGTXR_H

#include "Paige.h"

#define MINIMUM_RTF_VERSION		0x00010015 /* Minimum PAIGE version for RTF */

/* Standard file types supported automatically: */

enum {
	pg_unknown_type,	// Unknown file type
	pg_text_type,		// Standard ASCII text
	pg_rtf_type,		// RTF format
	pg_paige_type		// Standard PAIGE file type
};

typedef void PG_FAR * c_import_filter;
typedef void PG_FAR * c_export_filter;

/* Translation filter structure to and from a pg_ref */

struct pg_translator {
	memory_ref			data;				// Data transferred (read) or to-transfer (write)
	memory_ref			stylesheet_table;	// Contains list of possible stylesheets
	long				bytes_transferred;	// Number of bytes in buffer
	long				total_text_read;	// Total transferred to-moment
	long				cache_begin;		// Beginning file offset (if cache enabled)
	style_info			format;				// Style(s) and character format of text
	par_info			par_format;			// Paragraph format(s) of the text
	font_info			font;				// Font applied to this text
	pg_doc_info			doc_info;			// General document information
	unsigned long		flags;				// Attributes of last transfer
	pg_boolean			format_changed;		// Set to TRUE if format is different than last txr
	pg_boolean			par_format_changed;	// Set to TRUE if par format different than last txr
	pg_boolean			font_changed;		// Set to TRUE if font different than last txr
	pg_boolean			doc_info_changed;	// Set to TRUE if document info changed since last txr
// The following items used only by export class:
	long				data_type;			// Type of data being exported (can by pict, etc.)
	long 				data_param1;		// Additional data for data type */
	long 				data_param2;		// Additional data for data type */
};

/* Data types for export: */

enum {
	export_text_type,
	export_embed_type
};

/* Character types out: */

enum {
	ascii_char_out,
	delimiter_char_out,
	command_char_out
};

typedef struct pg_translator pg_translator;
typedef pg_translator PG_FAR *pg_translator_ptr;

/* Stylesheet table record: */

struct style_table_entry {
	pg_char				name[FONT_SIZE];
	style_info			style;
	font_info			font;
	par_info			par;
	pg_boolean			has_styles;
	pg_boolean			has_par_styles;
	pg_boolean			has_font;
	short				style_id;
};

typedef struct style_table_entry style_table_entry;
typedef style_table_entry PG_FAR *style_entry_ptr;

struct style_table {
	short				style_id;
	short				rsrv;
	long				index;
};

typedef struct style_table style_table;
typedef style_table PG_FAR *style_table_ptr;

/* Flag settings for the translator record: */

#define TRANSFER_NOT_TEXT		0x00000001	/* Data transfer is not text */
#define TRANSFER_PAR_END		0x00000002	/* Ending char ends paragraph */
#define TRANSFER_WAS_DIRECT		0x00000004	/* Data transferred directly (no insert(s) required */


/* Import preparation flags: */

#define IMPORT_TEXT_FLAG				0x00000001	/* Import raw text */
#define IMPORT_TEXT_FORMATS_FLAG		0x00000002	/* Import text formats */
#define IMPORT_PAR_FORMATS_FLAG			0x00000004	/* Import paragraph formats */
#define IMPORT_PAGE_INFO_FLAG			0x00000008	/* Import page info (do not necessarily apply) */
#define IMPORT_CONTAINERS_FLAG			0x00000010	/* Import container boxes */
#define IMPORT_HEADERS_FLAG				0x00000020	/* Import headers */
#define IMPORT_FOOTERS_FLAG				0x00000040	/* Import footers */
#define IMPORT_FOOTNOTES_FLAG			0x00000080	/* Import footnotes */
#define IMPORT_EMBEDDED_OBJECTS_FLAG	0x00000100	/* Import recognized embed_refs */
#define IMPORT_PAGE_GRAPHICS_FLAG		0x00000200	/* Import page-anchored pictures */
#define IMPORT_STYLESHEETS_FLAG			0x00000400	/* Import defined stylesheets */
#define APPLY_PAGE_DIMENSIONS			0x02000000	/* Apply paper and page sizes */
#define IMPORT_CACHE_FLAG				0x04000000	/* Import cache (paging) method */
#define IMPORT_EVERYTHING_FLAG			0x00FFFFFF	/* Import everything you can */

/* Import capability bits: */

#define IMPORT_TEXT_FEATURE				0x00000001	/* Can import raw text */
#define IMPORT_TEXT_FORMATS_FEATURE		0x00000002	/* Can import text formats */
#define IMPORT_PAR_FORMATS_FEATURE		0x00000004	/* Can import paragraph formats */
#define IMPORT_PAGE_INFO_FEATURE		0x00000008	/* Can import page dimensions */
#define IMPORT_CONTAINERS_FEATURE		0x00000010	/* Can import containers */
#define IMPORT_HEADERS_FEATURE			0x00000020	/* Can import headers */
#define IMPORT_FOOTERS_FEATURE			0x00000040	/* Can import footers */
#define IMPORT_FOOTNOTES_FEATURE		0x00000080	/* Can import footnotes */
#define IMPORT_EMBEDDED_OBJECTS_FEATURE	0x00000100	/* Can import standard, supported embed_refs */
#define IMPORT_PAGE_GRAPHICS_FEATURE	0x00000200	/* Can import graphics anchored to page */
#define IMPORT_SERIAL_SETUP				0x01000000	/* Setup serial byte read */
#define IMPORT_CACHE_CAPABILITY			0x40000000	/* Can import with "cache" feature */

/* Export preparation flags: */

#define EXPORT_TEXT_FLAG				0x00000001	/* Export raw text */
#define EXPORT_TEXT_FORMATS_FLAG		0x00000002	/* Export text formats */
#define EXPORT_PAR_FORMATS_FLAG			0x00000004	/* Export paragraph formats */
#define EXPORT_PAGE_INFO_FLAG			0x00000008	/* Export page info (do not necessarily apply) */
#define EXPORT_CONTAINERS_FLAG			0x00000010	/* Export container boxes */
#define EXPORT_HEADERS_FLAG				0x00000020	/* Export headers */
#define EXPORT_FOOTERS_FLAG				0x00000040	/* Export footers */
#define EXPORT_FOOTNOTES_FLAG			0x00000080	/* Export footnotes */
#define EXPORT_EMBEDDED_OBJECTS_FLAG	0x00000100	/* Export recognized embed_refs */
#define EXPORT_PAGE_GRAPHICS_FLAG		0x00000200	/* Export page-anchored pictures */
#define EXPORT_STYLESHEETS_FLAG			0x00000400	/* Export defined stylesheets */
#define INCLUDE_LF_WITH_CR				0x02000000	/* Add LF with CR if not already */
#define EXPORT_CACHE_FLAG				0x04000000	/* Export cached file */
#define EXPORT_UNICODE_FLAG				0x08000000	/* Write text as UNICODE */

#define EXPORT_EVERYTHING_FLAG			0x00FFFFFF	/* Export everything you can */

/* Export capability bits: */

#define EXPORT_TEXT_FEATURE				0x00000001	/* Can Export raw text */
#define EXPORT_TEXT_FORMATS_FEATURE		0x00000002	/* Can Export text formats */
#define EXPORT_PAR_FORMATS_FEATURE		0x00000004	/* Can Export paragraph formats */
#define EXPORT_PAGE_INFO_FEATURE		0x00000008	/* Can Export page dimensions */
#define EXPORT_CONTAINERS_FEATURE		0x00000010	/* Can Export containers */
#define EXPORT_HEADERS_FEATURE			0x00000020	/* Can Export headers */
#define EXPORT_FOOTERS_FEATURE			0x00000040	/* Can Export footers */
#define EXPORT_FOOTNOTES_FEATURE		0x00000080	/* Can Export footnotes */
#define EXPORT_EMBEDDED_OBJECTS_FEATURE	0x00000100	/* Can Export standard, supported embed_refs */
#define EXPORT_PAGE_GRAPHICS_FEATURE	0x00000200	/* Can Export graphics anchored to page */
#define EXPORT_CACHE_FEATURE			0x00100000	/* Can Export cache method */
#define EXPORT_UNICODE_FEATURE			0x00200000	/* Can export UNICODE */

#define EXPORT_SERIAL_SETUP				0x01000000	/* Setup serial byte buffer */

/* Miscellaneous: */

#define MAX_TEXT_BUFFER					4096		/* Max buffer used by filter in serial mode */
#define TRANSLATOR_BUFFER_SIZE			4096		/* Default buffer size */
#define UNKNOWN_POSITION				-1			/* Used for unknown file position */
#define PICT_APPEND_SIZE				512			/* Size to append memory_ref when reading pict */
#define HIGH_CHARS_BASE					128
#define HIGH_CHARS_QTY					128
#define N_A								0x00	/* Char not supported */
#define MSWORD_TOPBOTTOM_MARGIN			72		/* Default top and bottom MS Word margins (point) */
#define MSWORD_LEFTRIGHT_MARGIN			90		/* Default left and right MS Word margins (point) */
#define KIND_STR_SIZE					4		/* Max size of filetype string */

/* RTF Character Constants: */

#define RTF_COMMAND_CHAR		0x5C
#define RTF_GROUPBEGIN_CHAR		0x7B
#define RTF_GROUPEND_CHAR		0x7D
#define RTF_HEX_CHAR			0x27
#define RTF_STAR_CHAR			0x2A
#define RTF_COLON				0x3A

#define TABLE_TERMINATOR		0x3B
#define MAX_COMMAND_BYTES		32		/* Maximum bytes for any command */

/* Special characters: */

#define NON_BREAKING_SPACE_CHAR	'~'
#define NON_REQUIRED_HYPHEN_CHAR	'-'
#define NON_BREAKING_HYPHEN	'_'
#define NEGATIVE_SIGN_CHAR '-'

/* Internal table tokens (used by me, not in RTF stream): */

#define TABLE_TERMINATOR_CHAR	0x7C

/* Import mode record */

struct pg_import_rec {
	long				target_pos;		 /* Target import position once done. */
	long				t_length;		 /* Text length so far */
	pg_short_t			last_style_item; /* Last style_info item applied */
	pg_short_t			last_par_item;	 /* Last paragraph item applied */
	short				last_font_index; /* Last font item applied */
	memory_ref			t_blocks;		 /* Text blocks solely for importing */
	memory_ref			t_style_run;	 /* Style run used for importing */
	memory_ref			par_style_run;	 /* Paragraph run used for importing */
	memory_ref			previous_import; /* Previous import control, if any (so we can nest) */
};

typedef struct pg_import_rec pg_import_rec;
typedef pg_import_rec PG_FAR *pg_import_ptr;


#ifdef __cplusplus
extern	"C" {
#endif

extern PG_PASCAL (void) pgInitTranslatorRec (pg_globals_ptr globals, pg_translator_ptr translator_ptr);
extern PG_PASCAL (void) pgDisposeTranslatorRec (pg_translator_ptr translator_ptr);
extern PG_PASCAL (pg_filetype) pgDetermineFileType (pg_file_unit fileref, file_io_proc io_proc,
		long starting_position);
extern PG_PASCAL (pg_boolean) pgVerifyRTF (pg_file_unit fileref, file_io_proc io_proc,
		long starting_position);
extern PG_PASCAL (short) pgConvertResolution (paige_rec_ptr pg, short pointvalue);
extern PG_PASCAL (short) pgResolutionConvert (paige_rec_ptr pg, short value);
extern PG_PASCAL (pg_error) pgImportFileFromC (pg_ref pg, pg_filetype filetype, long feature_flags,
		long file_begin, pg_file_unit f_ref);
extern PG_PASCAL (pg_error) pgExportFileFromC (pg_ref pg, pg_filetype filetype, long feature_flags,
		long file_begin, select_pair_ptr output_range, pg_boolean use_selection, pg_file_unit f_ref);
extern PG_PASCAL (void) pgMapCharacters (pg_globals_ptr globals, pg_char_ptr chars, long num_chars,
		pg_char_ptr character_table);
extern PG_PASCAL (short) pgCompareFontTable (pg_char_ptr source_font, pg_char_ptr table_font);

#ifdef __cplusplus
	}
#endif

#endif
