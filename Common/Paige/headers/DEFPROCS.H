
/*	$Id$    */

/* This header file defines all the default hooks in a Paige structure.
This includes both format functions (for style and paragraph) as well as
"global" functions that apply to the whole Paige object. The declarations
in this header are prototypes only, the actual functions will be found
in different (and hopefully appropriate) source files. */

#ifndef DEFPROCS_H
#define DEFPROCS_H

#include "Paige.h"

#ifdef __cplusplus
extern	"C" {
#endif

extern PG_PASCAL (void) pgLineMeasureProc (paige_rec_ptr pg, pg_measure_ptr line_info);
extern PG_PASCAL (void) pgLineAdjustProc (paige_rec_ptr pg, pg_measure_ptr measure_info,
		point_start_ptr line_starts, pg_short_t num_starts,
		rectangle_ptr line_fit, par_info_ptr par_format);
extern PG_PASCAL (pg_boolean) pgLineValidate (paige_rec_ptr pg, pg_measure_ptr measure_info);
extern PG_PASCAL (long) pgLineParse (paige_rec_ptr pg, pg_measure_ptr measure_info,
		pg_char_ptr text, point_start_ptr line_start, long global_offset, long remaining_length);
extern PG_PASCAL (void) pgInitLineProc (paige_rec_ptr pg, pg_measure_ptr measure_info,
		short init_verb);
extern PG_PASCAL (void) pgExamineClickProc (paige_rec_ptr pg, short click_verb,
		short modifiers, long refcon_return, t_select_ptr begin_select,
		t_select_ptr end_select);
extern PG_PASCAL (pg_boolean) pgHyphenateProc (paige_rec_ptr pg, text_block_ptr block,
		style_walk_ptr styles, pg_char_ptr block_text, long line_begin,
		long PG_FAR *line_end, long PG_FAR *positions, short PG_FAR *char_types,
		long PG_FAR *line_width_extra, pg_boolean zero_length_ok);
extern PG_PASCAL (void) pgTextIncrementProc (paige_rec_ptr pg, long base_offset,
		long increment_amount);
extern PG_PASCAL (void) pgHiliteProc (paige_rec_ptr pg, t_select_ptr selections,
		pg_short_t select_qty, shape_ref rgn);
extern PG_PASCAL (void) pgDrawHiliteProc (paige_rec_ptr pg, shape_ref rgn);
extern PG_PASCAL (void) pgDrawCursorProc (paige_rec_ptr pg, t_select_ptr select,
		short verb);
extern PG_PASCAL (void) pgIdleProc (paige_rec_ptr pg, short verb);
extern PG_PASCAL (void) pgTextLoadProc (paige_rec_ptr pg, text_block_ptr block);
extern PG_PASCAL (long) pgTextBreakProc (paige_rec_ptr pg, text_block_ptr block);
extern PG_PASCAL (void) pgPt2OffsetProc (paige_rec_ptr pg, co_ordinate_ptr point,
		short conversion_info, t_select_ptr selection);
extern PG_PASCAL (void) pgSmartQuotesProc (paige_rec_ptr pg, long insert_offset,
		long info_bits, pg_char_ptr char_to_insert, short PG_FAR *insert_length);
extern PG_PASCAL (void) pgModifyContainerProc (paige_rec_ptr pg, pg_short_t container_num,
		rectangle_ptr container, pg_scale_ptr scaling, co_ordinate_ptr screen_extra,
		short verb, void PG_FAR *misc_info);
extern PG_PASCAL (pg_boolean) pgParBoundaryProc (paige_rec_ptr pg, select_pair_ptr boundary);
extern PG_PASCAL (void) pgInitFont (paige_rec_ptr pg, font_info_ptr info);
extern PG_PASCAL (void) pgStyleInitProc (paige_rec_ptr pg, style_info_ptr style,
			font_info_ptr font);
extern PG_PASCAL (void) pgInstallFont (paige_rec_ptr pg, style_info_ptr the_style,
		font_info_ptr the_font, style_info_ptr composite_style, pg_boolean include_offscreen);
extern PG_PASCAL (void) pgSpecialCharProc (paige_rec_ptr pg, style_walk_ptr walker, pg_char_ptr data,
		pg_short_t offset, pg_short_t length, draw_points_ptr draw_position,
		long extra, short draw_mode);
extern PG_PASCAL (void) pgAutoScrollProc (paige_rec_ptr pg, short h_verb, short v_verb,
		co_ordinate_ptr mouse_point, short draw_mode);
extern PG_PASCAL (void) pgScrollAdjustProc (paige_rec_ptr pg, long amount_h,
		long amount_v, short draw_mode);
extern PG_PASCAL (void) pgDrawScrollProc (paige_rec_ptr pg, shape_ref update_rgn,
		co_ordinate_ptr scroll_pos, pg_boolean post_call);
extern PG_PASCAL (void) pgDrawPageProc (paige_rec_ptr pg, shape_ptr page_shape,
		pg_short_t r_qty, pg_short_t page_num, co_ordinate_ptr vis_offset,
		short draw_mode_used, short call_order);
extern PG_PASCAL (void) pgBitmapModifyProc (paige_rec_ptr pg, graf_device_ptr bits_port,
		pg_boolean post_call, rectangle_ptr bits_rect, co_ordinate_ptr screen_offset,
		long text_offset);
extern PG_PASCAL (void) pgWaitProc (paige_rec_ptr pg, short wait_verb, long progress_ctr,
		long completion_ctr);
extern PG_PASCAL (void) pgMeasureProc (paige_rec_ptr pg, style_walk_ptr walker,
		pg_char_ptr data, long length, pg_short_t slop, long PG_FAR *positions,
		short PG_FAR *types, short measure_verb, long current_offset, pg_boolean scale_widths,
		short call_order);
extern PG_PASCAL (pg_boolean) pgMergeProc (paige_rec_ptr pg, style_info_ptr style, pg_char_ptr text_data,
		pg_short_t length, text_ref merged_data, long ref_con);
extern PG_PASCAL (long) pgTrackCtlProc (paige_rec_ptr pg, short verb,
		t_select_ptr first_select, t_select_ptr last_select, style_walk_ptr styles,
		pg_char_ptr associated_text, point_start_ptr bounds_info, short modifiers,
		long track_refcon);
extern PG_PASCAL (long) pgCharInfoProc (paige_rec_ptr pg, style_walk_ptr style_walker,
		pg_char_ptr data, long block_offset, long offset_begin, long offset_end, long char_offset,
		long mask_bits);
extern PG_PASCAL (void) pgDrawProc (paige_rec_ptr pg, style_walk_ptr walker, pg_char_ptr data,
		pg_short_t offset, pg_short_t length, draw_points_ptr draw_position,
		long extra, short draw_mode);
extern PG_PASCAL (void) pgDupStyleProc (paige_rec_ptr src_pg, paige_rec_ptr target_pg,
		short reason_verb, format_ref all_styles, style_info_ptr style);
extern PG_PASCAL (void) pgDeleteStyleProc (paige_rec_ptr pg, pg_globals_ptr globals,
		short reason_verb, format_ref all_styles, style_info_ptr style);
extern PG_PASCAL (void) pgAlterStyleProc (paige_rec_ptr pg, style_info_ptr old_style,
		style_info_ptr new_style, style_info_ptr style_mask);
extern PG_PASCAL (void) pgSaveStyleProc (paige_rec_ptr pg, style_info_ptr style_to_save);
extern PG_PASCAL (void) pgActivateStyleProc (paige_rec_ptr pg, style_info_ptr style,
		select_pair_ptr text_range, pg_char_ptr text, short front_back_state,
		short perm_state, pg_boolean show_hilite);
extern PG_PASCAL (void) pgCopyTextProc (paige_rec_ptr src_pg, paige_rec_ptr target_pg,
		short reason_verb, style_info_ptr style, long style_position,
		long text_position, pg_char_ptr text, long length);
extern PG_PASCAL (void) pgDeleteTextProc (paige_rec_ptr pg, short reason_verb,
		style_info_ptr style, long style_position, long text_position,
		pg_char_ptr text, long length);
extern PG_PASCAL (pg_boolean) pgSetupInsertProc (paige_rec_ptr pg, style_info_ptr style,
		long position);
extern PG_PASCAL (void) pgLineGlitterProc (paige_rec_ptr pg, style_walk_ptr walker,
		long line_number, long par_number, text_block_ptr block, point_start_ptr first_line,
		point_start_ptr last_line, point_start_ptr previous_first, point_start_ptr previous_last,
		co_ordinate_ptr offset_extra, rectangle_ptr vis_rect, short call_verb);
extern PG_PASCAL (long) pgTabMeasureProc (paige_rec_ptr pg, style_walk_ptr walker, long cur_pos,
		long cur_text_pos, long line_left, pg_char_ptr text, pg_short_t text_length,
		long PG_FAR *char_positions, pg_short_t PG_FAR *tab_rec_info);
extern PG_PASCAL (void) pgTabDrawProc (paige_rec_ptr pg, style_walk_ptr walker, tab_stop_ptr tab,
		draw_points_ptr draw_position);
extern PG_PASCAL (void) pgDupParProc (paige_rec_ptr src_pg, paige_rec_ptr target_pg,
		short reason_verb, par_ref all_pars, par_info_ptr par_style);
extern PG_PASCAL (void) pgDeleteParProc (paige_rec_ptr pg, short reason_verb,
		par_ref all_pars, par_info_ptr par_style);
extern PG_PASCAL (void) pgAlterParProc (paige_rec_ptr pg, par_info_ptr old_par,
		par_info_ptr new_par);
extern PG_PASCAL (void) pgEnhanceUndo (paige_rec_ptr pg, pg_undo_ptr undo_rec,
		void PG_FAR *insert_ref, short action_to_take);
extern PG_PASCAL (void) pgPostPaginateProc (paige_rec_ptr pg, text_block_ptr block,
		smart_update_ptr update_info, long lineshift_begin, long lineshift_end,
		short action_taken_verb);
extern PG_PASCAL (void) pgSetGrafDevice (paige_rec_ptr pg, short verb, graf_device_ptr device,
			color_value_ptr bk_color);
extern PG_PASCAL (void) pgPageModify  (paige_rec_ptr pg, long page_num, rectangle_ptr margins);
extern PG_PASCAL (long) pgBreakInfoProc (paige_rec_ptr pg, pg_char_ptr the_char, short charsize,
		style_info_ptr style, font_info_ptr font, long current_settings);
extern PG_PASCAL (pg_word) pgCharClassProc (paige_rec_ptr pg, pg_char_ptr the_char, short charsize,
		style_info_ptr style, font_info_ptr font);
extern PG_PASCAL (short) pgInsertQuery (paige_rec_ptr pg, pg_char_ptr the_char, short charsize);

/* Default file functions */

extern PG_PASCAL (pg_boolean) pgReadHandlerProc (paige_rec_ptr pg, pg_file_key key, memory_ref key_data,
		long PG_FAR *element_info, void PG_FAR *aux_data, long PG_FAR *unpacked_size);
extern PG_PASCAL (pg_boolean) pgWriteHandlerProc (paige_rec_ptr pg, pg_file_key key, memory_ref key_data,
		long PG_FAR *element_info, void PG_FAR *aux_data, long PG_FAR *unpacked_size);
extern PG_PASCAL (pg_boolean) pgDummyReadHandler (paige_rec_ptr pg, pg_file_key key, memory_ref key_data,
		long PG_FAR *element_info, void PG_FAR *aux_data, long PG_FAR *unpacked_size);
extern PG_PASCAL (pg_boolean) pgDummyWriteHandler (paige_rec_ptr pg, pg_file_key key, memory_ref key_data,
		long PG_FAR *element_info, void PG_FAR *aux_data, long PG_FAR *unpacked_size);
extern PG_PASCAL (pg_error) pgStandardReadProc (void PG_FAR *data, short verb, long PG_FAR *position,
		long PG_FAR *data_size, file_ref filemap);
extern PG_PASCAL (pg_error) pgStandardWriteProc (void PG_FAR *data, short verb, long PG_FAR *position,
		long PG_FAR *data_size, file_ref filemap);
extern PG_PASCAL (pg_error) pgOSReadProc (void PG_FAR *data, short verb, long PG_FAR *position,
		long PG_FAR *data_size, file_ref filemap);
extern PG_PASCAL (pg_error) pgOSWriteProc (void PG_FAR *data, short verb, long PG_FAR *position,
		long PG_FAR *data_size, file_ref filemap);
extern PG_PASCAL (pg_error) pgScrapMemoryWrite (void PG_FAR *data, short verb, long PG_FAR *position,
		long PG_FAR *data_size, file_ref filemap);
extern PG_PASCAL (pg_error) pgScrapMemoryRead (void PG_FAR *data, short verb, long PG_FAR *position,
		long PG_FAR *data_size, file_ref filemap);

/* Default extend and misc. procs */

extern PG_PASCAL (void) pgExtendProc (void PG_FAR *param, short verb);
extern PG_PASCAL (long) pgCacheFree (pgm_globals_ptr mem_globals, memory_ref dont_free, long desired_free);


#ifdef __cplusplus
		}
#endif

#endif
