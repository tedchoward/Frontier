/* This header file contains user definitions which you CAN MODIFY without being
affected by DataPak's possible upgrades.

These definitions are how you append to a Paige data structure without upsetting
the rest of Paige.

Changes, however, must be made at the source code level.

For example, suppose you want to hang some extra fields to the style_info struct.
The default record -- style_append_t -- looks like this:

struct style_append_t {
	long		refcon;
};
typedef struct style_append_t style_append_t;

All you would do is add your own fields, and re-compile. Note that DataPak will
not update this header file, so you can simply re-compile for updates.	*/

#ifndef USERDEFS_H
#define USERDEFS_H


struct port_append_t {
	long		refcon;
};					/* Graphics port append */

typedef struct port_append_t port_append_t;


struct tb_append_t {
	long		refcon;
};					/* Text block record append */

typedef struct tb_append_t tb_append_t;


struct style_append_t {
	long		refcon;
};					/* Style record append */

typedef struct style_append_t style_append_t;


struct font_append_t {
	long		refcon;
};					/* Font record append */

typedef struct font_append_t font_append_t;

#ifdef PG_BASEVIEW

#define kStyleSheetNameMaxSize	64

struct par_append_t {
	long			refcon;
	unsigned char	styleSheetName[kStyleSheetNameMaxSize];
	short			styleSheetResID;
	short			unused;
};					/* Paragraph format record append */

typedef struct par_append_t par_append_t;

#else

struct par_append_t {
	long		refcon;
};					/* Paragraph format record append */

typedef struct par_append_t par_append_t;

#endif

#endif

