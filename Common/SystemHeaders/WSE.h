
/*	$Id$    */

/* Word Solution 2.01 Header File for "C"
    ©1993 by DataPak Software, Inc.   */

// modifications for TCP/Connect II and PowerPC
// 12/93 by Amanda Walker, InterCon Systems Corporation
// 03/94 AW: fixed structure alignment, made into "universal"-style include file


#ifndef WSINTF_H
#define WSINTF_H

typedef struct {
		long	top; 			/* "top" of rectangle */
		long	left;			/* "top" of rectangle */
		long	bottom;			/* "top" of rectangle */
		long	right;			/* "top" of rectangle */
} LongRect, *LongRPtr;

typedef struct {
		long			Pos;		/* Offset into text where line begins */
		short			Length;		/* Length of line */
		long			Top;		/* Relative top of line */
		short			Height;		/* The line's total height, in pixels */
		short			Left;		/* Indent -- always zero by WS */
		short			Base;		/* Vertical baseline to draw text */
		unsigned short	Flags;		/* Reserved for internal use */
		unsigned short	rNum;		/* Reserved for internal use */
		short			Width;		/* The line's total width, in pixels */
		short			MaxWidth;	/* Maximum width for this line */
} LineRec, LineArray[1];
typedef LineArray *LineAPtr, **LineHandle;	/* Ptrs and Handles to above */

typedef struct {
		UniversalProcPtr	hMeasureText;		/* Text widths measurement proc */
		UniversalProcPtr	hDrawText;			/* Text drawing proc */
	   	UniversalProcPtr	hFmtDelete;			/* Pre-delete proc */
	   	UniversalProcPtr	hFontInfo;			/* GetFontInfo */
	   	UniversalProcPtr	hWordBreak;			/* Hyphenate a word */
 	   	UniversalProcPtr	hTabWidth;			/* Return width of TAB */
	   	UniversalProcPtr	hTabDraw;			/* Draw Tab */
	   	UniversalProcPtr	hMaxWidth;			/* Return maximum line width */
	   	UniversalProcPtr	hPreCalc;			/* Prepare for line calculation */
	   	UniversalProcPtr	hPostCalc;			/* Finish line calculation */
	   	UniversalProcPtr	hDoneCalc;			/* Thinks it's done with line calc */
	   	UniversalProcPtr	hSmartScroll;		/* CR/backspace scrolling */
	   	UniversalProcPtr	hPt2Line;			/* Find line with point */
	   	UniversalProcPtr	hSelectRgn;			/* Make a selection region */
	   	UniversalProcPtr	hGetBuf;			/* Get a text buffer */
	   	UniversalProcPtr	hNewBuf;			/* Create a new text buffer */
	   	UniversalProcPtr	hDeleteBuf;			/* Delete buffer */
	   	UniversalProcPtr	hPreDraw;			/* Pre-Draw of a line */
	   	UniversalProcPtr	hPostDraw;			/* Called after any drawing */
		UniversalProcPtr	hAlterRun;			/* Called before insert or delete */
		UniversalProcPtr	hEditCall;			/* Called before & after Cut, Copy, Paste */
	   	UniversalProcPtr	hLineShift;			/* Vertical line shift */
	   	UniversalProcPtr	hBoundsMoved;		/* tBounds moved internally */
} WSHooks, *WSHooksPtr;

typedef struct {
		UniversalProcPtr	hMeasureText;		/* Text widths measurement proc */
		UniversalProcPtr	hDrawText;			/* Text drawing proc */
	   	UniversalProcPtr hFmtDelete;			/* Pre-delete proc */
	   	UniversalProcPtr	hFontInfo;			/* GetFontInfo */
} UserStyles, *UserStylesPtr;

/* The Format Run info */

typedef struct  {
		long			Pos;		/* Offset to text */
		UserStylesPtr	UserProcs; 	/* Pointer to User Style routines */
		short			fReal;		/* Font/Pt Size = REAL if non-zero */
		char			fName[32];	/* Font Name */
		short			fStyle;		/* Text style */
		Fixed			fPoint;		/* Point size (fractional) */
		RGBColor		fgColor;	/* Foreground color */
		short			Txr;		/* Script Manager info */
		Fixed			cExtra;		/* CharExtra value (char spacing) */
		short			Leading;	/* +- offset for baseline */
		long			UserSpace;	/* Space provided for UserStyles */
} FormatRec, *FormatPtr;
typedef FormatRec FormatArray[1], *FormatAPtr, **FormatHandle;

typedef struct  {
		long	Pos;		/* Offset into text */
		long	Buf;		/* Buffer Offset */
		long	Line;		/* Offset into Line Array */
		short	Caret;		/* "Caret" or left-edge, in pixels */
} tSelectPos;

/* TEXT BUFFER STUFF */

typedef struct {
	    long			TBBegin;	/* Absolute text beginning */
		long			TBEnd;		/* Absolute text end */
		LineHandle		TBLines;	/* Line array */
		long			TBLineBeg;	/* Absolute line beginning */
		long			TBLineEnd;	/* Absolute line ending */
		long			TBPixBeg;	/* Pixel height beginning */
		long			TBPixEnd;	/* Pixel height ending */
		unsigned long	TBFlags;	/* Misc. attributes */
		Handle			TBHandle;	/* Actual text */
		Handle			TBCtl;		/* Control Char Run */
		long			TBLev1;		/* Reserved for level 1 */
		long			TBLev2;		/* Reserved for level 2 */
} TextBlock, TextArray[1];
typedef TextArray *TextArrayPtr, **TextStuff;

typedef struct {
		TextStuff	tHandle;	/* Handle to text structures */
		long		tLength;	/* Total length of text */
		diskrect	tRect;		/* Display rectangle */
		LongRect	tBounds;	/* Bounds rectangle */
		long		tvBase;		/* Original BOUNDS position.v */
		long		thBase;		/* Original BOUNDS position.h */
		tSelectPos	tBegin;		/* Begin selection */
		tSelectPos	tEnd;		/* End Selection */
		tSelectPos	tCalc;		/* Next place to recalc */
		tSelectPos	tCalcEnd;	/* Place to end recalc */
		tSelectPos	tTop;		/* Top visible line in tRect */
		FormatHandle tFormat;	/* Format Run */
		FormatRec	tFmt;		/* Current Format (of caret position) */
		short		tJust;		/* Justification of text */
		unsigned long tFlags;	/* Misc. attributes */
		short		tArrow;		/* Used internally for arrow keys */
		long		tRefCon;	/* For application use */
		WSHooksPtr	tProcs;		/* Hooks for a variety of stuff */
} WSRec, *WSPtr, **WSHandle;


typedef struct {
		short	verb;			/* Tab type. Zero = no tab */
		short	pos;			/* Pixel position */
		short	leader;			/* Leader char */
} tabEntry, tabArray[1];
typedef tabArray *tabPtr, **tabHandle;

typedef struct {
	    short	onTop;			/* Top border enable */
		short	onLeft;			/* Left border enable */
		short	onBottom;		/* Bottom border enable */
		short	onRight;		/* Right border enable */
		diskrect	spacing;		/* Pixel widths for borders */
		short	penPat;			/* pen pattern index for borders */
		Fixed	penSize;		/* Pen size for border (fractional) */
		short	strike;			/* Single, double, etc. */
} borderInfo;

typedef struct {
		long	rBegin;			/* Where it begins in text */
		short	left;			/* Left indent */
		short	right;			/* Right indent */
		short	indent;			/* 1st line indent */
		short	just;			/* Justification */
		tabEntry tabs[18];		/* Tabs stops*/
		borderInfo	border;		/* par border */
		short	flags;			/* Internal use attributes */
		Fixed	spacing;		/* Line spacing */
		short	spaceType;		/* Type of spacing */
		long	refCon;			/* For user */
} rulerRecord, *rulerPtr, **rulerHandle;

typedef struct {
		Fixed	rIncrement;		/* Pixels per increment */
		short	rNumIncs;		/* Number of increments per "inch" unit */
		short	rUnitNum;		/* Number symbol per unit */
		short	rHeight;		/* Height of ruler on screen */
		short	rBase1;			/* Height of first baseline */
		short	rBase2;			/* Height of second baseline */
		short	rZero;			/* What constitutes zeroth pixel */
		long	rFlags;			/* Special flags -- used internally */
		rulerRecord  rCurRuler;	/* Currently displaying ruler info */
} rulerInfo,*rulerInfoPtr,**rulerInfoHandle;

#endif
