
/*	$Id$    */

/* This is the generic header file for "machinespecific.h". You do NOT need
to modify this header as its definitions are intended to compile on various
platforms. However, you will need to completely swap out "machinespecific.c"
since that file -- and only that file -- contains code unique to your platform
and/or OS. */

/* Updated by Board Member TR Shaw, OITC, Inc. 12 Feb/19 Apr 1994 for C/C++/Pascal linkage and PPC control
	and for size and speed */

/* Updated by Board Member TR Shaw, OITC, Inc. / 25 Apr 1994 for platform independant font changes */


#ifndef MACHINE_H
#define MACHINE_H

#include "Paige.h"

#define OFFSCREEN_SUCCESSFUL	1			/* Offscreen enable was successful */
#define OFFSCREEN_UNSUCCESSFUL	-1			/* Offscreen enable was not successful */

#define CTL_CHAR_BITS		(PAR_SEL_BIT | LINE_SEL_BIT | TAB_BIT)
#define NON_CTL_CHAR_BITS	(long) (~CTL_CHAR_BITS)
#define DOUBLE_UNDERLINE		0x0001		/* Double underline */
#define GRAY_UNDERLINE			0x0002		/* "Dotted" underline */
#define WORD_UNDERLINE			0x0004		/* Word(s) underline */
#define FAKE_UNDERLINE			0x0008		/* Underline is faked */
#define OVERLINE_LINE			0x0010		/* Overline style */

#define FONT_NAME_SIZE(fname) (short)fname[0]

/* Clipping verbs (passed to pgClipGrafDevice) : */

enum {
	clip_standard_verb,
	clip_with_none_verb
};

#ifdef MAC_PLATFORM
#if !TARGET_API_MAC_CARBON
#include <FixMath.h>
#include <ToolUtils.h>
#endif
#endif

#ifdef __cplusplus
extern	"C" {
#endif

#define	CLIP_PAGE_CHANGED		0x0001		/* Page area changed since last clip region */
#define CLIP_VIS_CHANGED		0x0002		/* Vis area has changed */
#define CLIP_EXCLUDE_CHANGED	0x0004		/* Exclude area changed since last clip region */
#define CLIP_DEVICE_CHANGED		0x0008		/* Graf device changed since last clip region */
#define CLIP_SCROLL_CHANGED		0x0010		/* Clip region changed by virtue of scrolling. */

 /* QD style conversions */

extern PG_PASCAL (long) PaigeToQDStyle (const style_info_ptr the_style);
extern PG_PASCAL (void) QDStyleToPaige (long qd_styles, style_info_ptr the_style);


extern PG_PASCAL (void) pgMachineInit (pg_globals_ptr globals);
extern PG_PASCAL (void) pgMachineShutdown (const pg_globals_ptr globals);
extern PG_PASCAL (void) pgInitDefaultDevice (const pg_globals_ptr globals, graf_device_ptr device);
extern PG_PASCAL (void) pgInitDefaultFont (const pg_globals_ptr globals, font_info_ptr font);
extern PG_PASCAL (void) pgInitDefaultStyle (const pg_globals_ptr globals, style_info_ptr style, font_info_ptr def_font);
extern PG_PASCAL (void) pgInitDefaultPar (const pg_globals_ptr globals, par_info_ptr def_par);
#ifdef MAC_PLATFORM
extern PG_PASCAL (GrafPtr) pgGetPlatformDevice (graf_device_ptr the_device);
#define pgReleasePlatformDevice(the_device)
#endif
#ifdef WINDOWS_PLATFORM
extern PG_PASCAL (HDC) pgGetPlatformDevice (graf_device_ptr the_device);
extern PG_PASCAL (void) pgReleasePlatformDevice (graf_device_ptr the_device);
#endif
extern PG_PASCAL (void) pgClipGrafDevice (paige_rec_ptr pg, short clip_verb, shape_ref alternate_vis);
extern PG_PASCAL (void) pgSetMeasureDevice (paige_rec_ptr pg);
extern PG_PASCAL (void) pgUnsetMeasureDevice (paige_rec_ptr pg);
extern PG_PASCAL (void) pgPrintDeviceChanged (paige_rec_ptr pg);
extern PG_PASCAL (void) pgPrepareOffscreen (paige_rec_ptr pg, rectangle_ptr target_area,
			rectangle_ptr real_bits_target, co_ordinate_ptr offset_adjust,
			long text_offset, point_start_ptr line_start, short draw_mode);
extern PG_PASCAL (pg_boolean) pgFinishOffscreen (paige_rec_ptr pg, long text_offset,
		point_start_ptr line_start, co_ordinate_ptr new_offset,
		rectangle_ptr new_target, short draw_mode);
extern PG_PASCAL (void) pgScaleGrafDevice (paige_rec_ptr pg);
extern PG_PASCAL (pg_region) pgScrollRect (paige_rec_ptr pg, rectangle_ptr rect, long distance_h,
		long distance_v, rectangle_ptr affected_area, short draw_mode);
extern PG_PASCAL (void) pgEraseRect (pg_globals_ptr globals, rectangle_ptr rect,
		pg_scale_ptr scaling, co_ordinate_ptr offset_extra);
extern PG_PASCAL (pg_short_t) pgMeasureText (paige_rec_ptr pg, short measure_verb, pg_char_ptr data,
		long length, long slop, long num_spaces, pg_text_int PG_FAR *positions,
		style_walk_ptr walker);
extern PG_PASCAL (pg_boolean) pgTransColor (pg_globals_ptr globals, color_value_ptr color);
extern PG_PASCAL (pg_boolean) pgEqualColor (color_value_ptr color1, color_value_ptr color2);
extern PG_PASCAL (short) pgScalePointSize (paige_rec_ptr pg, style_walk_ptr walker,
		pg_char_ptr text, long length, pg_boolean PG_FAR *did_scale);
extern PG_PASCAL (void) pgDrawWordUnderline (paige_rec_ptr pg, style_walk_ptr walker,
		short draw_bits, pg_char_ptr text, pg_short_t offset, pg_short_t length,
		long extra, draw_points_ptr draw_position);
extern PG_PASCAL (short) pgSystemDirection (pg_globals_ptr globals);
extern PG_PASCAL (short) pgGlobalStrCopy (pg_char_ptr src_str, pg_char_ptr target_str,
		short max_target_size);
extern PG_PASCAL (void) pgFixFontName (font_info_ptr font);
extern PG_PASCAL (pg_boolean) pgIsRealFont(pg_globals_ptr globals, font_info_ptr font, pg_boolean use_alternate);
extern PG_PASCAL (short) SpecialUnderline (style_info_ptr style);
extern PG_PASCAL (pg_fixed) pgPointsizeToScreen (pg_ref pg, pg_fixed pointsize);
extern PG_PASCAL (pg_fixed) pgScreenToPointsize (pg_ref pg, pg_fixed screensize);
extern PG_PASCAL (void) pgOSToPgColor (const pg_plat_color_value PG_FAR *os_color, color_value_ptr pg_color);
extern PG_PASCAL (void) pgColorToOS (const color_value_ptr pg_color, pg_plat_color_value PG_FAR *os_color);
extern PG_PASCAL (long) pgDeviceResolution (graf_device_ptr device);
extern PG_PASCAL (void) pgTransLiterate (pg_char_ptr text, long length, pg_char_ptr target,
		pg_boolean do_uppercase);
extern PG_PASCAL (void) pgOpenPrinter (paige_rec_ptr pg_rec, graf_device_ptr print_dev,
		long first_position, rectangle_ptr page_rect);
extern PG_PASCAL (void) pgClosePrinter (paige_rec_ptr pg_rec, graf_device_ptr print_dev);
extern PG_PASCAL (pg_boolean) pgIsCaretTime(paige_rec_ptr pg);

#ifdef NO_OS_INLINE

extern PG_PASCAL (pg_fixed) pgMultiplyFixed (pg_fixed fix1, pg_fixed fix2);
extern PG_PASCAL (pg_fixed) pgDivideFixed (pg_fixed fix1, pg_fixed fix2);
extern PG_PASCAL (pg_fixed) pgRoundFixed (pg_fixed fix);
extern PG_PASCAL (pg_fixed) pgFixedRatio (short n, short d);

#else

#define pgMultiplyFixed(fix1,fix2)	(pg_fixed)FixMul(fix1,fix2)
#define pgDivideFixed(fix1,fix2)	(pg_fixed)FixDiv(fix1,fix2)
#define pgRoundFixed(fix)			(pg_fixed)(((fix) + 0x00008000) & 0xFFFF0000)
#define pgFixedRatio(n,d)			(pg_fixed)FixRatio(n,d)

#endif

#ifdef __cplusplus
	}
#endif

#endif
