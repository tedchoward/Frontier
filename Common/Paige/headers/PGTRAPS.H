/* This header file maps machine-specific OS calls to the respective function
for the currently defined platform. */

#ifndef PGTRAPS_H
#define PGTRAPS_H

#include "CPUDefs.h"

#ifndef PAIGE_H
#include "Paige.h"
#endif

#define RIGHT_FONT_FLAG			0x8000		/* Font is right-to-left */
#define NON_ROMAN_FLAG			0x4000		/* Font is not Roman */
#define MULTIBYTE_FLAG			0x2000		/* Multibytes possible */
#define SCRIPT_CODE_MASK		0x0FFF		/* Script type mask */

#ifdef MAC_PLATFORM
/*******************************************************************************/
/*																			   */
/*								MACINTOSH									   */
/*																			   */
/*******************************************************************************/

/* Updated by TR Shaw, OITC, Inc. 28 Dec 1994 for complete Universal Headers
	support and added more universal character values */

#ifndef THINK_C

#include <Quickdraw.h>
#include <Memory.h>
#include <Events.h>
#include <fonts.h>
#include <Packages.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <Resources.h>
#include <files.h>
#include <errors.h>
#include <gestalt.h>

#endif

#include <script.h>

/*********** Memory Definitions (for allocation, resize, etc.) ***********/

#define SYSTEM_7_VAL 			0x0700

#define DEF_GLOBAL_RESTYPE		'pgdf'		/* Default globals resource */
#define DEF_GLOBAL_ID			128			/* Default globals res ID */

#define MAC_STYLE_MASK			0x007F		/* Mask for Macintosh style bits */

#define DEF_POINT_SIZE	 (long) 0x000C		/* Default point size */
#define DEF_CHAR_WIDTH	 (long) 0x0000		/* Default font width (not used in Mac) */

/* Special underline bits:  */

#define DOTTED_PAT				0x8888		/* Dot pattern for underlines */
#define HIDDEN_TEXT_PAT			0x55AA		/* Gray pattern (to bypass QD globals) */
#define SPECIAL_SYMBOL_TXSIZE	12			/* Text size for invisible symbols */

#define MAX_RGN					32700		/* Tallest non-scrolling shape */
#define MAX_SCROLLABLE_RGN		8192		/* Tallest shape for clipping */

/* Default values for pg_globals. These get placed in the "globals" area at startup
time if the default resource is not available.  */

#define CR_CHAR				0x0D		/* Return char */
#define SOFT_CR_CHAR		0x0A		/* Soft CR */
#define TEXT_BRK_CHAR		0x1B		/* Alternate text break char */
#define TAB_CHAR			0x09		/* Tab char */
#define SOFT_HYPHEN			0x1F		/* Soft hyphen char */
#define FF_CHAR				0x0C		/* Form feed char */
#define CONTAINER_BRK_CHAR	0x0E		/* Container/column-break char */
#define DELETE_CHAR			0x08		/* Delete character (backspace) */
#define LEFT_ARROW			0x1C		/* Left arrow key */
#define RIGHT_ARROW			0x1D		/* Right arrow key */
#define UP_ARROW			0x1E		/* Up arrow key */
#define DOWN_ARROW			0x1F		/* Down arrow key */
#define FWD_DELETE_CHAR		0x7F		/* Forward Delete */
#define DECIMAL_CHAR		0x2E		/* Decimal */
#define HYPHEN_CHAR			0x2D		/* Hyphen */
#define INVIS_TAB			0x13		/* Invisible tab char */
#define INVIS_SPACE			0x2E		/* Invisible space char */
#define INVIS_BREAK			0xAD		/* Invisible container break char */
#define INVIS_PBREAK		0xAD		/* Invisible page break char */
#define APOSTROPHE			0x27		/* Single quote (flat single quote) */
#define FLAT_QUOTE_CHAR		0x22		/* Flat double quote char */
#define LEFT_SINGLE_QUOTE	0xD4		/* Left single quote */
#define RIGHT_SINGLE_QUOTE	0xD5		/* Right single quote */
#define LEFT_DOUBLE_QUOTE	0xD2		/* Left double quote */
#define RIGHT_DOUBLE_QUOTE	0xD3		/* Right double quote */
#define INVIS_CR			0xA6		/* Invisible CR char */
#define INVIS_LF			0xB9		/* Invisible soft cr char */
#define INVIS_END			0xB0		/* Invisible end-of-doc char */
#define INVIS_UNKNOWN		0xC9		/* Unknown symbol value */
#define ELIPSE_SYMBOL		0xC9		/* elipse symbol */
#define UNSUPPORTED_CHAR	0xC9		/* Symbol to use for unsupported characters */
#define BULLET_CHAR			0xA5		/* Bullet character */

#define MAX_OFFSCREEN		48000		/* Maximum offscreen size */

#define MAX_TEXTBLOCK		4096		/* Maximum text block size */
#define DEF_TAB_SPACE		24			/* Default tab space, in pixels */
#define DEF_MIN_WIDTH		16			/* Default minimum line width */

/* GrafPort preserver record (to save/restore GrafPort info): */

struct port_preserve {
	GrafPtr				prev_port;		/* Previous port set */
	graf_device_ptr		last_device;	/* Previous device */
	RgnHandle			clip_rgn;		/* Preserved clip (or NULL) */
	Rect				simple_clip;	/* Clip (if just a rect) */
	Point				origin;			/* Preserves previous origin */
	PenState			pen_state;		/* Presevred pen state */
	short				text_mode;		/* Preserved text draw mode */
	short				text_font;		/* Preserved font */
	short				text_face;		/* Preserved style */
	short				text_size;		/* Preserved point */
	RGBColor			text_fg;		/* Foreground color */
	RGBColor			text_bk;		/* Background color */
	long				old_fg;			/* Old-world fg color */
	long				old_bk;			/* Old-word bk color */
};

typedef struct port_preserve port_preserve;
typedef port_preserve PG_FAR *port_preserve_ptr;


/* Mac index to "previous_items" */

#define ORIGINAL_CLIP		0


/***************************** Special struct & function macros *************************/


#ifdef GLOBALS_ARE_PSTRINGS

#define DRAW_SYMBOL(the_str) DrawString((StringPtr) the_str)
#define MEASURE_SYMBOL(the_str) StringWidth((StringPtr) the_str)

#else

#define DRAW_SYMBOL(the_str) DrawText((Ptr) the_str, 0, global_char_size(the_str))
#define MEASURE_SYMBOL(the_str) TextWidth((Ptr) the_str, 0, global_char_size(the_str))

#endif

/*** Mac-specific Utilities ***/

#ifdef __cplusplus
extern	"C" {
#endif

/* The following are linkable only if you have pgMacPictUtils in your project */

extern PG_PASCAL (void) PictOutlineToShape (PicHandle pict, shape_ref target_shape,
		short accuracy);
extern PG_PASCAL (void) RegionOutlineToShape (RgnHandle rgn, shape_ref target_shape,
		short accuracy); // From TRS/OITC

extern PG_PASCAL (memory_ref) HandleToMemory (pgm_globals *mem_globals, Handle h, pg_short_t rec_size);
extern PG_PASCAL (Handle) MemoryToHandle (memory_ref ref);

/* Exception handling */

extern PG_PASCAL (void) pgFailMemError( pgm_globals_ptr pgm_globals_p );

#define PG_FAIL_MEM_ERROR()										\
		pgFailMemError( __pg_globals_p)
		
#ifdef __cplusplus
	}
#endif

#endif	/* MAC_PLATFORM */



#ifdef WINDOWS_PLATFORM
/*******************************************************************************/
/*																			   */
/*								WINDOWS 									   */
/*																			   */
/*******************************************************************************/

#ifdef WINDOWS_EMULATE_COMPILE
#include "WinMac.h"
#else

#ifdef WIN32_COMPILE
#define STRICT
#endif
#include <WINDOWS.H>
#endif

#include <string.h>

#define DEF_POINT_SIZE	 (long) 0x000C		/* Default point size */
#define DEF_CHAR_WIDTH	 (long) 0			/* Default font width */

#define DOTTED_PAT				0x8888		/* Dot pattern for underlines */
#define SPECIAL_SYMBOL_TXSIZE	12			/* Text size for invisible symbols */

#define MAX_RGN					32700		/* Tallest non-scrolling shape */
#define MAX_SCROLLABLE_RGN		8192		/* Tallest shape for clipping */

/* Default values for pg_globals. These get placed in the "globals" area at startup
time if the default resource is not available.  */

#define CR_CHAR				0x0D		/* Return char */
#define SOFT_CR_CHAR		0x0A		/* Soft CR */
#define TEXT_BRK_CHAR		0x1B		/* Alternate text break char */
#define TAB_CHAR			0x09		/* Tab char */
#define SOFT_HYPHEN			0x1F		/* Old soft hyphen char */
#define PC_SOFT_HYPHEN		0xAD		/* PC's Soft hyphen char */
#define FF_CHAR				0x0C		/* Form feed char */
#define CONTAINER_BRK_CHAR	0x0E		/* Container/column-break char */
#define DELETE_CHAR			0x08		/* Delete character (backspace) */
#define LEFT_ARROW			0x1C		/* Left arrow key */
#define RIGHT_ARROW			0x1D		/* Right arrow key */
#define UP_ARROW			0x1E		/* Up arrow key */
#define DOWN_ARROW			0x1F		/* Down arrow key */
#define FWD_DELETE_CHAR     0x7F		/* Forward delete key */
#define DECIMAL_CHAR		0x2E		/* Decimal */
#define HYPHEN_CHAR			0x2D		/* Hyphen */
#define ELIPSE_SYMBOL		0x85		/* Elipse symbol */
#define INVIS_CR			0xB6		/* Invisible CR char */
#define INVIS_LF			0xB5		/* Invisible soft cr char */
#define INVIS_SPACE			0x2E		/* Invisible space char */
#define INVIS_BREAK			0xA5		/* Invisible container break char */
#define INVIS_PBREAK		0xA5		/* Invisible page break char */
#define INVIS_END			0xB5		/* Invisible end-of-doc char */
#define INVIS_UNKNOWN		0xC9		/* Unknown symbol value */
#define APOSTROPHE			0x27		/* Single quote (flat single quote) */
#define FLAT_QUOTE_CHAR		0x22		/* Flat double quote char */
#define UNSUPPORTED_CHAR	0x83		/* Symbol to use for unsupported characters */

#ifdef UNICODE
	#define INVIS_TAB			0x2022		/* Invisible tab char */
	#define LEFT_SINGLE_QUOTE	0x2018		/* Left single quote */
	#define RIGHT_SINGLE_QUOTE	0x2019		/* Right single quote */
	#define LEFT_DOUBLE_QUOTE	0x201C		/* Left double quote */
	#define RIGHT_DOUBLE_QUOTE	0x201D		/* Right double quote */
	#define BULLET_CHAR			0x2022		/* Bullet character */
#else
	#define INVIS_TAB			0x95		/* Invisible tab char */
	#define LEFT_SINGLE_QUOTE	0x91		/* Left single quote */
	#define RIGHT_SINGLE_QUOTE	0x92		/* Right single quote */
	#define LEFT_DOUBLE_QUOTE	0x93		/* Left double quote */
	#define RIGHT_DOUBLE_QUOTE	0x94		/* Right double quote */
	#define BULLET_CHAR			0x95		/* Bullet character */
#endif

#define MAX_OFFSCREEN		48000		/* Maximum offscreen size */

#define MAX_TEXTBLOCK		4096		/* Maximum text block size */
#define DEF_TAB_SPACE		24			/* Default tab space, in pixels */
#define DEF_MIN_WIDTH		16			/* Default minimum line width */

/***************************** Special struct & function macros *************************/

typedef RECT	Rect;
typedef HRGN 	RgnHandle;

struct Point {
	short		v;				/* vertical */
	short		h;				/* horizontal */
};								/* machine-specific point */
typedef struct Point Point;

typedef HWND	      GrafPtr;			/* Best equiv. to GrafPtr for WINDOWS */
// typedef COLORREF      RGBColor;			/* Color value for Windows. */

struct port_preserve {
    int					saved_dc_index; 	 /* return value of SaveDC */
	graf_device_ptr		last_device;		 /* Previous device in globals */
	RECT				rgn_box;			/* Original region rect of clip */
};
typedef struct port_preserve port_preserve;

typedef struct platform_globals    platform_globals;
typedef struct platform_globals PG_FAR *platform_globals_ptr;

#define PLATFORM_SIG	 0x1994


#ifdef GLOBALS_ARE_PSTRINGS

#define MEASURE_SYMBOL(the_str) (GetTextWidth(hdc, (pg_char_ptr)&the_str[1], global_char_size(the_str)) & 0x0000FFFF)

#else

#define MEASURE_SYMBOL(the_str) (GetTextWidth(hdc, (pg_char_ptr)the_str, global_char_size(the_str)) & 0x0000FFFF)

#endif

#ifdef WINDOWS_COMPILE
#ifdef WIN16_COMPILE
#define GetTextWidth(hdc, str, length) LOWORD(GetTextExtent(hdc, (PGSTR)str, (int)length))
#else
extern short GetTextWidth (HDC hdc, PGSTR str, int length);
#endif
#endif

/* The font object tracking record (we keep these around to know what fonts have
been built): */

#ifdef UNICODE
struct font_object_rec {
	long				the_font;			// The font object
	long				style_ptsize;		// Original point size from style_info
	LOGFONT				log;				// Style info stuff that made the font
};
#else
struct font_object_rec {
	long				the_font;			// The font object
	long				style_ptsize;		// Original point size from style_info
	LOGFONT				log;				// Style info stuff that made the font
	pg_short_t			valid_widths;		// Widths are all valid
	int					widths[256];		// Recorded character widths
};
#endif

typedef struct font_object_rec font_object_rec;
typedef font_object_rec PG_FAR *font_object_ptr;

/* Flags for valid_widths: */

#define CHARWIDTHS_COMPUTED		0x0001
#define USE_TEMP_WIDTHS			0x0002

/* Index values for graf_device.previous_items[] */

#define PREVIOUS_FONT		0
#define PREVIOUS_PALETTE	1
#define PREVIOUS_BITMAP		2


#ifdef __cplusplus
extern	"C" {
#endif

extern PG_PASCAL (memory_ref) HandleToMemory (pgm_globals_ptr mem_globals, HANDLE h, pg_short_t rec_size);
extern PG_PASCAL (HANDLE) MemoryToHandle (memory_ref ref);
extern PG_PASCAL (void) SetFontCharWidths (pg_ref pg, style_info_ptr style, int PG_FAR *charwidths);


#ifdef __cplusplus
	}
#endif

#endif  /* WINDOWS_PLATFORM */


/********************** Generic Multi-platform Utilities ********************/


#ifdef __cplusplus
extern	"C" {
#endif

/* Rect / rectangle conversion functions:  */

extern PG_PASCAL (void) RectToRectangle (Rect PG_FAR *r, rectangle_ptr pg_rect);
extern PG_PASCAL (void) RectangleToRect (rectangle_ptr pg_rect, co_ordinate_ptr offset,
		Rect PG_FAR *r);

/* Shape / Region functions:  */

/* ShapeToRgn fills in a region to match a Paige shape. The resulting region
is offset h and v. Note that regions are limited to 32K each side so the
values are truncated as needed. It is scaled if scale_factor is non-NULL.
ADDED JULY 20th for page_modify hook support, shape is intersected with "sect_rect"
if non-null. */

extern PG_PASCAL (void) ShapeToRgn (shape_ref src_shape, long offset_h, long offset_v,
		pg_scale_factor PG_FAR *scale_factor, short inset_amount,
		rectangle_ptr sect_rect, pg_region rgn);
extern PG_PASCAL (void) ScaleRect (pg_scale_ptr scale_factor, Rect PG_FAR *r);

#ifdef __cplusplus
	}
#endif


#endif  /* PGTRAPS_H */

