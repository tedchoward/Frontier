#ifndef CPUDEFS_H
#define CPUDEFS_H

/* CPUDefs.h defines the machine-specific macros to compile Paige
source files for a specific platform.

The idea is to change THIS source file, and this source file only, to
satisfy your local compiler. */

/* Updated by TR Shaw, OITC, Inc. 14 Feb/19 Apr 1994 for linkages and power PC control
	and finer control over configurations.
*/

/* Updated by G. Crandall 27 April 1994 to allow for *memory_ref instead of
**memory_ref for Windows (and possibly others) platform.
Updated again by G. Crandall 6 May 1994 to lessen the complexity of ifdefs since
some each compiler seems to interpret differently.  */

/* Updated by TR Shaw, OITC, Inc. 26 Sep 1994 for mixed C and C++ support with 
	Symantic C++ and Think C++.
*/

/* Updated by TR Shaw/J Smith/C Priddy, OITC, Inc. 27 JUL 1995 for Window 32, NT, 95. */
/* More changes for INTEL by TR Shaw/C Priddy, OITC, Inc. 04 SEP 1995 for Window 32, NT, 95. */

/******** THE FOLLOWING DEFINES CONTROL BASIC CONFIGURATIONS *********/

/* Version Control: */

#define PAIGE_VERSION		0x00010017		/* Version number */

//Auto configure for Metrowerks

#ifdef __MWERKS__
//#define PG_DEBUG				/* If defined then everything is in debug mode. */
//#define PG_DEBUG_STATICS		/* If defined then debugging remembers. */
//#define PG_BIG_CHECKSUMS		/* If defined then checksum memory debugging. */
//define NO_C_INLINE			/* If defined then no inline code generated */
//define NO_OS_INLINE			/* If defined no OS function inline code */
//define C_LIBRARY				/* If defined then assume standard C libraries */
//#define C_LINKAGE				/* If defines than extern functions are "C" */
//#define DO_RTF					/* If defined then including Paige RTF */
#endif

/* General Compiler Options: */

// #define PG_DEBUG				/* If defined then everything is in debug mode. */
//#define PG_DEBUG_STATICS		/* If defined then debugging remembers. */
//#define PG_BIG_CHECKSUMS		/* If defined then checksum memory debugging. */

// #define NO_C_INLINE			/* If defined then no inline code generated */
// #define NO_OS_INLINE			/* If defined no OS function inline code */
// #define C_LIBRARY			/* If defined then assume standard C libraries */
// #define C_LINKAGE			/* If defined then extern functions are "C" */

/* Platform Control (define only one!) */

//#define PPC_COMPILE				/* If defined then compiles for Mac Power PC */
//#define MAC_COMPILE				/* If defined then source is for Macintosh */
//#define WIN16_COMPILE				/* If defined then source is for Windows 3.1 */
//#define WIN32_COMPILE				/* If defined then source is for Windows NT */
//#define WIN95_COMPILE				/* If defined then source is for Windows 95 */
//#define UNIX_COMPILE				/* If defined then source is for Unix */
//#define OS2_COMPILE				/* If defined then source is for OS/2 */
//#define WINDOWS_EMULATE_COMPILE   /* Fakes Windows environment on Mac */
//#define MAC2WIN_COMPILE			/* Fakes Mac environment in Windows (for Altura) */
//#define ACCESS_MS_DLL				/* if using Paige as a DLL,this is defined in the make file */
//#define CREATE_MS_DLL				/* if making a Paige DLL,this is defined in the make file */

//Auto configure for Metrowerks
#ifdef __MWERKS__
	#ifdef _X86_
		#define WIN32_COMPILE
	//define MS_DLL					// if making a DLL,this is defined in the make file
	#else
		//Code change by Timothy Paustian Sunday, May 7, 2000 9:58:30 PM
		//Changed to Opaque call for Carbon
		#if TARGET_CARBON
			#define PPC_COMPILE
		#else
			#if GENERATINGPOWERPC
			#define PPC_COMPILE
			#else
			#define MAC_COMPILE
			#endif
		#endif
	#endif
#endif

//Auto configure for Apple
#ifdef applec
#if GENERATINGPOWERPC
#define PPC_COMPILE
#else
#define MAC_COMPILE
#endif
#endif

//Auto configure for Think
#ifdef THINK_C
#if GENERATINGPOWERPC
#define PPC_COMPILE
#else
#define MAC_COMPILE
#endif
#endif

//Auto configure for Symantec
#ifdef __SC__
#if GENERATINGPOWERPC
#define PPC_COMPILE
#else
#define MAC_COMPILE
#endif
#endif

#ifdef ACCESS_MS_DLL
#define MS_DLL
#endif
#ifdef CREATE_MS_DLL
#define MS_DLL
#endif

//DSI-only configurations (DO NOT UNCOMMENT, for DSI only):
//#define PG_SUBSETS
//#define DOING_EQ

//OITC-only configurations (DO NOT UNCOMMENT, for OITC only):
//define PG_OITC_SUBSETS

// Do NOT COMMENT THIS NEXT LINE OUT!!!  PG_DEBUG IS ABOVE!!!
#ifdef PG_DEBUG

// Define the following for easier source-level debugging

#define SOURCE_LEVEL_DEBUG
	// SOURCE_LEVEL_DEBUG does not break to MacsBug (place source breakpoint instead)

#endif

/* Other platform-specific controls */

// #define ASLMBUILD				/* If defined then using Mac SLM */
#define CONTINUE_AFTER_FAILURE		/* Continue execution after debugger assert */


//OITC RTF Defines
//#define DO_RTF						/* If defined then including Paige RTF */
#ifdef DO_RTF
/* Required compiler variables' states for Paige RTF */
#define AUTO_DEFINE
#define RTF_NO_INTERNAL_STATIC_TABLES
#define RTF_EXTERNAL_CHAR_SETS
#define RTF_AS_EMBEDDED
#define RTF_USE_ALTERNATE_IO
#define RTF_USING_ERROR_CODES
#define RTF_NO_EXIT
#define RFT_NO_CHAR_TABLES
#define STDARG
#undef NO_TRANS_DOC
#endif

/*********************************************************************/

#ifdef WIN16_COMPILE
#define WINDOWS_COMPILE

/* Win 16 Multilingual defines for source code users only */
/* Change the below to match your destination configuration */
/* Default to English */
//define WIN_MULTILINGUAL	//Define the following for Windows multilingual
#define WIN_CHARSET			ANSI_CHARSET	//Define for Windows 16 chracter set
#define WIN_CODEPAGE		CP_WINANSI		//Define for Windows 16 code page
#define WIN_LANGUAGE		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), SORT_DEFAULT)	//Define for Windows 16 language
#define WIN_DIRECTION		1				//Define for Windows 16 language direction (matches left_right_direction in Paige.h)

#ifdef MS_DLL
#define WIN16_DLL
#undef C_LINKAGE
#endif
#endif

#ifdef WIN32_COMPILE
#define WINDOWS_COMPILE
#define WIN_MULTILINGUAL	//Define the following for Windows multilingual
#ifdef MS_DLL
#define WIN32_DLL
#undef C_LINKAGE
#endif
#define C_LIBRARY
#endif

#ifdef WIN95_COMPILE
#define WINDOWS_COMPILE
#define WIN32_COMPILE
#define WIN_MULTILINGUAL	//Define the following for Windows multilingual
#ifdef MS_DLL
#define WIN32_DLL
#undef C_LINKAGE
#endif
#define C_LIBRARY
#endif

#ifdef WINDOWS_EMULATE_COMPILE
#ifndef NO_OS_INLINE
#define NO_OS_INLINE
#endif
#endif

#ifdef ASLMBUILD						/* Required for ASLM */
#ifdef C_LIBRARY
#undef C_LIBRARY						/* Some C Libs don't work in ALSM!  Remove with caution. */
#endif
#endif

#ifdef PPC_COMPILE
#ifndef C_LINKAGE
#define C_LINKAGE
#endif
#endif

#ifdef WINDOWS_COMPILE
#ifndef NO_OS_INLINE
#define NO_OS_INLINE
#endif
#define STRICT
#include <windows.h>
#ifdef WIN32_COMPILE
#include <windowsx.h>
#endif

#ifdef MS_DLL

#ifdef WIN16_DLL
#define LIB_PREFIX _export
#define DLL_PREFIX 
#endif

#ifdef WIN32_DLL
#ifdef ACCESS_MS_DLL
#define LIB_PREFIX __declspec(dllimport)
#define DLL_PREFIX __declspec(dllimport)
#endif
#ifdef CREATE_MS_DLL
#define LIB_PREFIX __declspec(dllexport)
#define DLL_PREFIX __declspec(dllexport)
#endif
#endif

#else
#define DLL_PREFIX		/* nothing */
#define LIB_PREFIX		/* nothing */
#endif

#else
#define DLL_PREFIX		/* nothing */
#define LIB_PREFIX		/* nothing */
#endif

/* Some platform pre-defined constants (placed in Paige structures): */

/* Hardware configurations */

#define MACINTOSH_68000		0x00000001	/* Macintosh, MC68000 or greater */
#define MACINTOSH_68020		0x00000002	/* Macintosh, MC68020 or greater */
#define MACINTOSH_68030		0x00000003	/* Macintosh, MC68030 or greater */
#define MACINTOSH_68040		0x00000004	/* Macintosh, MC68040 or greater */
#define MAC_POWER_PC		0x10000001	/* Macintosh PowerPC 601 or greater */

#define INTEL_80286			0x00000101	/* Intel, 80286 or greater */
#define INTEL_80386			0x00000102	/* Intel, 80386 or greater */
#define INTEL_80486			0x00000103	/* Intel, 80486 or greater */
#define INTEL_PENTIUM		0x00000105	/* Intel, PENTIUM or greater */

#define OTHER				0x01000001	/* Other */
#define OTHER_RISC			0x11000001	/* Other, RISC */

/* Operating System Configurations */

#define MACINTOSH_6			0x00000001	/* Macintosh, System 6.0.5 or greater */
#define MACINTOSH_7			0x00000002	/* Macintosh, System 7 or greater */
#define MACINTOSH_71		0x00000003	/* Macintosh, System 7.1 or greater */
#define MACINTOSH_711		0x00000004	/* Macintosh, System 7 Pro or greater */
#define MACINTOSH_72		0x00000005	/* Macintosh, System 7.2 or greater */
#define MACINTOSH_75		0x00000006	/* Macintosh, System 7.5 or greater */
#define MACINTOSH_751		0x00000007	/* Macintosh, System 7.5.1 or greater */
#define MACINTOSH_752		0x00000008	/* Macintosh, System 7.5.2 or greater */
#define MACINTOSH_753		0x00000008	/* Macintosh, System 7.5.3 or greater */
#define MAX_MAC_OS			MACINTOSH_753

#define WINDOWS_2			0x00000100	/* Windows version 2.0 */
#define WINDOWS_3			0x00000101	/* Windows version 3.0/3.1 */
#define WINDOWS_MAC			0x00000105	/* Windows version 3.0/3.1 emulated */
#define WINDOWS_NT			0x00000110	/* Windows NT version 1.0 */
#define WINDOWS_95			0x00000120	/* Windows 95 version 1.0 */
#define MAX_WINDOWS_OS		WINDOWS_NT

#define UNIX_4				0x00100001	/* Generic unix system 4 */
#define UNIX_5				0x00100002	/* Generic unix system 5 */
#define MAX_UNIX_OS			UNIX_5

#define OS2_WARP			0x01000001	/* Generic OS/2 Warp */
#define MAX_OS2_OS			OS2_WARP

/* Graphic Configurations */

#define ORIGINAL_QUICKDRAW	0x00000001	/* Macintosh, original QuickDraw or greater */
#define COLOR_QUICKDRAW		0x00000002	/* Macintosh, color QuickDraw or greater */
#define COLOR_QUICKDRAW_32	0x00000003	/* Macintosh, 32-bit color QuickDraw or greater */
#define QUICKDRAW_GX		0x00000004	/* Macintosh, QuickDraw GX or greater */
#define QUICKDRAW_GX1		0x00000005	/* Macintosh, QuickDraw GX 1.1 or greater */
#define MAX_MAC_GRAPHICS	QUICKDRAW_GX1

#define WINDOWS_CGA			0x00000100	/* Windows CGA  or greater */
#define WINDOWS_VGA			0x00000101	/* Windows VGA  or greater */
#define WINDOWS_EGA			0x00000102	/* Windows EGA  or greater */
#define MAX_WINDOWS_GRAPHICS WINDOWS_EGA

#define X_WINDOWS_11		0x00100010	/* Unix X-Windows v11 or greater */
#define X_WINDOWS_12		0x00100020	/* Unix X-Windows v12 or greater */
#define MOTIF				0x00100030	/* Unix Motif or greater */
#define OPEN_LOOK			0x00100040	/* Unix OpenLook or greater */
#define MAX_UNIX_GRAPHICS	OPEN_LOOK

#define OS2_VGA				0x01000001	/* OS/2 VGA  or greater */
#define OS2_EGA				0x01000002	/* OS/2 EGA  or greater */
#define MAX_OS2_GRAPHICS	OS2_EGA

#define MACINTOSH_OS		1
#define WINDOWS_OS			2
#define UNIX_OS				3
#define OS2_OS				4

/* Paige configuration constants */

#ifdef MAC_COMPILE
#ifndef PPC_COMPILE
#define PAIGE_PLATFORM		MACINTOSH_68000 	/* Minimum hardware platform this header works with */
#define PAIGE_OS			MACINTOSH_6			/* Minimum OS this header works with */
#define PAIGE_GRAPHICS		ORIGINAL_QUICKDRAW	/* Minimum graphics this header works with */
#define CURRENT_OS			MACINTOSH_OS    	/* Generic OS */
#endif
#endif

#ifdef PPC_COMPILE
#define PAIGE_PLATFORM		MAC_POWER_PC    	/* Minimum hardware platform this header works with */
#define PAIGE_OS			MACINTOSH_75		/* Minimum OS this header works with */
#define PAIGE_GRAPHICS		ORIGINAL_QUICKDRAW	/* Minimum graphics this header works with */
#define CURRENT_OS			MACINTOSH_OS    	/* Generic OS */
#endif

#ifdef WINDOWS_COMPILE
#define PAIGE_PLATFORM		WINDOWS_3    	/* Minimum hardware platform this header works with */
#define PAIGE_OS			WINDOWS_3		/* Minimum OS this header works with */
#define PAIGE_GRAPHICS		WINDOWS_VGA		/* Minimum graphics this header works with */
#define CURRENT_OS			WINDOWS_OS    	/* Generic OS */
#endif

#ifdef UNIX_COMPILE
#define PAIGE_PLATFORM		UNIX_5    		/* Minimum hardware platform this header works with */
#define PAIGE_OS			UNIX_5			/* Minimum OS this header works with */
#define PAIGE_GRAPHICS		X_WINDOWS_11	/* Minimum graphics this header works with */
#define CURRENT_OS			UNIX_OS    		/* Generic OS */
#endif

#ifdef OS2_COMPILE
#define PAIGE_PLATFORM		OS2_WARP		/* Minimum hardware platform this header works with */
#define PAIGE_OS			OS2_WARP		/* Minimum OS this header works with */
#define PAIGE_GRAPHICS		OS2_VGA			/* Minimum graphics this header works with */
#define CURRENT_OS			OS2_OS    		/* Generic OS */
#endif

#ifdef WINDOWS_EMULATE_COMPILE
#define PAIGE_PLATFORM		WINDOWS_MAC
#define PAIGE_OS			WINDOWS_MAC
#define PAIGE_GRAPHICS		ORIGINAL_QUICKDRAW
#endif


/* General platform identifications for character set mapping: */

#define GENERAL_PLATFORM_MASK	0x0FFFFF00	/* AND with this to get general platform */
#define	GENERAL_MACINTOSH		0x00000000	/* Macintosh, all versions */
#define GENERAL_WINDOWS			0x00000100	/* Windows, all versions */
#define GENERAL_UNIX			0x00100000	/* Unix, all versions */
#define GENERAL_OS2				0x01000000	/* OS/2, all versions */

/* Which platform */
#ifdef MAC_COMPILE
#define GENERAL_PLATFORM		GENERAL_MACINTOSH
#endif
#ifdef PPC_COMPILE
#define GENERAL_PLATFORM		GENERAL_MACINTOSH
#endif
#ifdef WINDOWS_COMPILE
#define GENERAL_PLATFORM		GENERAL_WINDOWS
#endif
#ifdef UNIX_COMPILE
#define GENERAL_PLATFORM		GENERAL_UNIX
#endif
#ifdef OS2_COMPILE
#define GENERAL_PLATFORM		GENERAL_OS2
#endif
#ifdef WINDOWS_EMULATE_COMPILE
#define GENERAL_PLATFORM		GENERAL_WINDOWS
#endif

#define pgFromThisPlatform(platform)	((platform & GENERAL_PLATFORM_MASK) == GENERAL_PLATFORM)

/* Automatic Configurations */

#if (PAIGE_OS <= MAX_MAC_OS)
#define MAC_PLATFORM					/* This version is for Macintosh */
#elif (PAIGE_OS <= MAX_WINDOWS_OS)
#define WINDOWS_PLATFORM				/* This version is for Windows */
#elif (PAIGE_OS <= MAX_UNIX_OS)
#define UNIX_PLATFORM					/* This version is for Unix */
#elif (PAIGE_OS <= MAX_OS2_OS)
#define OS2_PLATFORM					/* This version is for OS/2 */
#endif


/* Misc Defines */

#ifndef FALSE
#define FALSE		0
#endif

#ifndef TRUE
#define TRUE		1
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL		0L
#else
#ifdef WINDOWS_PLATFORM
#define NULL		((void far *) 0)
#else
#define NULL		((void *) 0)
#endif
#endif
#endif


/* ------------------ Platform-specific Definitions ----------------- */

#ifdef MAC_PLATFORM

#include <types.h>
#include <quickdraw.h>		// for RGBColor
#include <OSUtils.h>		// for DateTimeRec

#define PG_FAR
#define REGISTER_PREFIX

#define ADDRESS_2			/* Memory is aligned to words */

//¥ TRS/OITC
#define PG_CDECL			/* Support Everyone else and Think C */
#ifdef C_LINKAGE
#ifdef __SC__
#undef PG_CDECL
#define PG_CDECL	_cdecl	/* Allow support for Symantic C++ */
#endif
#define PG_PASCAL(return_type) return_type PG_CDECL
#define PG_C(return_type) return_type PG_CDECL
#define PG_DLL_IMPORT(return_type) return_type PG_CDECL
#define PG_DLL_EXPORT(return_type) return_type PG_CDECL
#define PG_FN_PASCAL(returnType,functionName)	returnType (PG_CDECL *functionName)		/* For function pointer typedefs */
#define PG_FN_C(returnType,functionName)	returnType (PG_CDECL *functionName)		/* For C function pointer typedefs */
#define STATIC_PASCAL(return_type) static return_type
#define STATIC_C(return_type) static return_type
#else
#define PG_PASCAL(return_type) pascal return_type
#define PG_C(return_type) return_type PG_CDECL
#define PG_DLL_IMPORT(return_type) pascal return_type
#define PG_DLL_EXPORT(return_type) pascal return_type
#define PG_FN_PASCAL(returnType,functionName)	pascal returnType (*functionName)		/* For function pointer typedefs */
#define PG_FN_C(returnType,functionName)	returnType (PG_CDECL *functionName)		/* For C function pointer typedefs */
#define STATIC_PASCAL(return_type) static pascal return_type
#define STATIC_C(return_type) static return_type
#endif

#define PG_LONGWORD(cast) (cast)

/* General Types: */

typedef unsigned short pg_word;				/* WORD type */
typedef unsigned short pg_text_int;			/* Used for measuring text */
typedef unsigned short pg_short_t;			/* 64K address range value */

#define PG_BOM			0xFFFE
#define PG_REVERSE_BOM	0xFEFF

#ifdef UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif

typedef pg_word pg_char;					/* General purpose byte / char / whatever */
typedef pg_char *pg_char_ptr;				/* Pointer to pg_char */
#else
typedef unsigned char pg_char;				/* General purpose byte / char / whatever */
typedef pg_char *pg_char_ptr;				/* Pointer to pg_char */
#endif

/* The following is defined for historical purposes and backwards compatibility: */

typedef pg_char		pg_byte;
typedef pg_byte		*pg_byte_ptr;

typedef unsigned char	pg_bits8;
typedef pg_bits8 		*pg_bits8_ptr;

typedef OSErr pg_error;						/* Error result */
typedef short pg_boolean;					/* TRUE/FALSE parameter */
typedef long pg_fixed;						/* "Fixed" fraction */
typedef StringPtr pg_file_name_ptr;			/* Pointer to filename */
typedef StringPtr pg_font_name_ptr;			/* Pointer to font name */
typedef short pg_file_unit;					/* File unit reference number */
typedef pg_file_unit *pg_file_unit_ptr;		/* Pointer to file unit reference number */
typedef unsigned long pg_filetype;			/* File type indicator */
typedef pg_filetype PG_FAR *pg_filetype_ptr;
typedef pg_char *pg_c_string_ptr;			/* Pointer to C string */
typedef pg_char *pg_p_string_ptr;			/* Pointer to Pascal string */
typedef pg_char *PGSTR;						/* General portable string */
typedef RGBColor pg_plat_color_value;		/* Platform color value */
typedef short pg_font_script;				/* Mac script value */

/* All-purpose variable for pointers or memory ID's: */

typedef void *			void_ptr;			/* Used for type void pointers */
typedef void PG_FAR*	generic_var;		/* Used for RgnHandles, GrafPtr, etc. */
typedef Handle			pg_handle;			/* Used for type 'Handle' */
typedef RgnHandle		pg_region;			/* Used for machine-specific region */

#define REGION_NULL		(RgnHandle)NULL

/* Time types for Paige based time (ie seconds since 00:00 01 JAN 1900: */

#define PLATFORM_DELTA_TIME		126144000
typedef unsigned long			pg_time_t;
typedef	DateTimeRec				pg_datetime;
typedef pg_time_t PG_FAR *		pg_time_ptr;
typedef pg_datetime PG_FAR *	pg_datetime_ptr;

#endif


#ifdef WINDOWS_PLATFORM

#include <windows.h>
#include <time.h>

#ifdef WINDOWS_EMULATE_COMPILE

#define PG_FAR
#define REGISTER_PREFIX

#define ADDRESS_2			/* Memory is aligned to words */

#define PG_CDECL			/* Support Everyone else and Think C */
#ifdef C_LINKAGE
#ifdef __SC__
#undef PG_CDECL
#define PG_CDECL	_cdecl	/* Allow support for Symantic C++ */
#endif
#define PG_PASCAL(return_type) return_type PG_CDECL
#define PG_C(return_type) return_type PG_CDECL
#define PG_DLL_IMPORT(return_type) return_type PG_CDECL
#define PG_DLL_EXPORT(return_type) return_type PG_CDECL
#define PG_FN_PASCAL(returnType,functionName)	returnType (PG_CDECL *functionName)		/* For function pointer typedefs */
#define PG_FN_C(returnType,functionName)	returnType (PG_CDECL *functionName)		/* For C function pointer typedefs */
#define STATIC_PASCAL(return_type) static return_type
#define STATIC_C(return_type) static return_type
#else
#define PG_PASCAL(return_type) pascal return_type
#define PG_C(return_type) return_type PG_CDECL
#define PG_DLL_IMPORT(return_type) pascal return_type
#define PG_DLL_EXPORT(return_type) pascal return_type
#define PG_FN_PASCAL(returnType,functionName)	pascal returnType (*functionName)		/* For function pointer typedefs */
#define PG_FN_C(returnType,functionName)	returnType (PG_CDECL *functionName)		/* For C function pointer typedefs */
#define STATIC_PASCAL(return_type) static pascal return_type
#define STATIC_C(return_type) static return_type
#endif

#else

#ifdef WIN16_COMPILE
#define PG_FAR		__huge
#define REGISTER_PREFIX __loadds
#else
#define PG_FAR
#define ADDRESS_2			/* Flat memory model is aligned to words */
#define REGISTER_PREFIX
#endif

#define PG_CDECL	CDECL

#ifdef C_LINKAGE
#define PG_DLL_IMPORT(return_type) DLL_PREFIX return_type WINAPI
#define PG_DLL_EXPORT(return_type) LIB_PREFIX return_type WINAPI
#define PG_DLL(return_type) LIB_PREFIX return_type WINAPI
#ifdef WIN32_DLL
#define PG_PASCAL(return_type) DLL_PREFIX return_type WINAPI
#define PG_C(return_type) DLL_PREFIX return_type PG_CDECL
#else
#define PG_PASCAL(return_type) return_type WINAPI LIB_PREFIX
#define PG_C(return_type) return_type PG_CDECL LIB_PREFIX
#endif
#define PG_FN_PASCAL(returnType,functionName)	returnType (WINAPI *functionName)   /* For function pointer typedefs */
#define PG_FN_C(returnType,functionName)	returnType (PG_CDECL *functionName)		/* For C function pointer typedefs */
#define STATIC_PASCAL(return_type) static return_type WINAPI
#define STATIC_C(return_type) static return_type
#else
#define PG_DLL_IMPORT(return_type) DLL_PREFIX return_type WINAPI
#define PG_DLL_EXPORT(return_type) LIB_PREFIX return_type WINAPI
#define PG_DLL(return_type) LIB_PREFIX return_type WINAPI
#ifdef WIN32_DLL
#define PG_PASCAL(return_type) DLL_PREFIX return_type WINAPI
#define PG_C(return_type) DLL_PREFIX return_type PG_CDECL
#else
#define PG_PASCAL(return_type) return_type WINAPI LIB_PREFIX
#define PG_C(return_type) return_type PG_CDECL LIB_PREFIX
#endif
#define PG_FN_PASCAL(returnType,functionName)	returnType (WINAPI *functionName)  /* For function pointer typedefs */
#define PG_FN_C(returnType,functionName)	returnType (PG_CDECL *functionName)		/* For C function pointer typedefs */
#define STATIC_PASCAL(return_type) static return_type WINAPI
#define STATIC_C(return_type) static return_type
#endif
#endif

/* General Types: */

typedef WORD pg_word;						/* WORD type */
typedef int pg_text_int;					/* Used for measuring text that works with 3.1, Win32 and Win95 */
typedef unsigned short pg_short_t;			/* 64K address range value */

#define PG_BOM			0xFFFE
#define PG_REVERSE_BOM	0xFEFF

#ifdef UNICODE
typedef unsigned short	pg_char;
typedef unsigned short PG_FAR *pg_char_ptr;
#ifndef _UNICODE
#define _UNICODE
#endif
#else
typedef unsigned char pg_char;				/* General purpose byte / char / whatever */
typedef pg_char PG_FAR *pg_char_ptr;		/* Pointer to pg_char */
#endif

typedef pg_char_ptr PGSTR;

/* The following is defined for historical purposes and backwards compatibility: */

typedef pg_char			pg_byte;
typedef pg_byte PG_FAR	*pg_byte_ptr;

typedef unsigned char	pg_bits8;
typedef pg_bits8 PG_FAR *pg_bits8_ptr;

typedef pg_short_t pg_error;				/* Error result */
typedef short pg_boolean;					/* TRUE/FALSE parameter */
typedef long pg_fixed;						/* "Fixed" fraction */
typedef pg_char_ptr pg_file_name_ptr;				/* Pointer to filename */
typedef pg_char_ptr pg_font_name_ptr;				/* Pointer to font name */
typedef HFILE pg_file_unit;					/* File unit reference number */
typedef HFILE PG_FAR *pg_file_unit_ptr;		/* Pointer to file unit reference number */
typedef pg_char_ptr pg_c_string_ptr;		/* Pointer to C string */
typedef unsigned long pg_filetype;			/* File type indicator */
typedef pg_filetype PG_FAR *pg_filetype_ptr;
typedef pg_char_ptr pg_p_string_ptr;				/* Pointer to Pascal string */
typedef COLORREF pg_plat_color_value;		/* Platform color value */
typedef UINT pg_font_script;				/* Windows CodePage or CharSet value */

/* All-purpose variable for pointers or memory ID's: */

typedef void PG_FAR *	void_ptr;			/* Used for type void pointers */
typedef unsigned long	generic_var;		/* Used for type 'HANDLE' */
typedef HANDLE			pg_handle;			/* Used for type 'HANDLE' */
typedef HRGN			pg_region;			/* Used as a region */

#define REGION_NULL		(HRGN)NULL

/* Macros for casting "near" pointers to long int: */

#ifdef WIN32_COMPILE
#define PG_LONGWORD(cast) (cast)
#else
#define PG_LONGWORD(cast) (cast)(int)
#endif

/* Time types for Paige based time (ie seconds since 00:00 01 JAN 1900: */

#define PLATFORM_DELTA_TIME		0
typedef time_t					pg_time_t;
typedef	struct tm				pg_datetime;
typedef pg_time_t PG_FAR *		pg_time_ptr;
typedef pg_datetime PG_FAR *	pg_datetime_ptr;

#endif

/* Miscellaneous definitions */

#define GLOBALS_ARE_PSTRINGS				/* Global chars = Pascal strings
											(otherwise cstrings)  */

#endif
