
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define optoolkitinclude /*so other modules can tell that we've been included*/


typedef enum opLineSpacing {
	
	opSinglespaced = 1,
	
	opOneandalittlespaced = 2,
	
	opOneandaquarterspaced = 3,
	
	opOneandahalfspaced = 4,
	
	opDoublespaced = 5,
	
	opTriplespaced = 6
	} opLineSpacing;
	

typedef enum opDirection { /*used to navigate and reorganize outlines*/

	opNoDirection = 0, 
	
	opUp = 1, 
	
	opDown = 2, 
	
	opLeft = 3,
	
	opRight = 4, 
	
	opFlatup = 5, 
	
	opFlatdown = 6
	} opDirection;


typedef struct tyheadrecord {

	StringHandle hstring; /*the text of the headline*/
	
	struct tyheadrecord **headlinkdown, **headlinkup, **headlinkleft, **headlinkright;
	
	short headlevel; /*summits are at level 0, their children at level 1*/
	
	Boolean flexpanded: 1; /*is this line expanded?*/
	
	Boolean fldirty: 1; /*is this line in need of display?*/
	
	Boolean flcomment: 1; /*is this line a comment?*/
	
	Boolean flbreakpoint; /*in a script, is a breakpoint set here?*/
	
	Boolean tmpbit: 1; /*use this for anything you like*/
	
	Handle headrecordrefcon; /*you can link anything you like into a headrecord*/
	} tyheadrecord, *ptrheadrecord, **hdlheadrecord;


typedef struct tyoutlinerecord {
	
	hdlheadrecord hsummit; /*the first summit of the structure*/
	
	hdlheadrecord hbarcursor; /*the line the bar cursor is on*/
	
	hdlheadrecord hline1; /*this node is displayed on the 0th line in the window*/
	
	short lnumbarcursor; /*line number that the bar cursor is on, may be off screen*/
	
	short ctexpanded; /*number of nodes expanded*/
	
	short fontnum, fontsize, fontstyle; /*the display fontsizestyle*/
	
	short lineindent; /*how many pixels to indent for each level*/
	
	opLineSpacing linespacing; /*indicates number of pixels to add to each line*/
	
	Rect outlinerect; /*the rectangle that the outline is displayed in*/
	
	Rect windowrect; /*the size and position of window that last displayed outline*/
	
	short vertmin, vertmax, vertcurrent; /*vertical scrollbar info*/
	
	short horizmin, horizmax, horizcurrent; /*horiz scrollbar info*/
	
	long timecreated, timelastsave; /*the time and date the outline was created/last saved*/
	
	long ctsaves; /*the number of times this outline has been saved*/
	
	long outlinerefcon; /*for use by application*/
	} tyoutlinerecord, *ptroutlinerecord, **hdloutlinerecord;


typedef Boolean (*opvisitcallback) (hdlheadrecord); /*callback for the visit routines*/


extern hdloutlinerecord outlinedata; /*all OS Toolkit routines operate off this global*/


/*function prototypes -- creating and disposing outline records*/

	pascal Boolean opNewOutlineRecord (hdloutlinerecord *);

	pascal void opDisposeOutlineRecord (hdloutlinerecord);
	

/*setting the current outline*/

	pascal Boolean opSetTarget (hdloutlinerecord);
	
	pascal Boolean opPushOutline (hdloutlinerecord);
	
	pascal Boolean opPopOutline (void);


/*sending and receiving outline structures thru Apple Events*/

	pascal Boolean IACgetoutlineparam (OSType, hdloutlinerecord *);

	pascal Boolean IACpushoutlineparam (hdloutlinerecord, OSType);

	pascal Boolean IACreturnoutline (hdloutlinerecord);
	

/*packing/unpacking outlines into/from contiguous handles*/

	pascal Boolean opPack (Handle *);

	pascal Boolean opUnpack (Handle, long *);


/*navigating thru an outline structure*/

	pascal Boolean opGo (hdlheadrecord *, opDirection);
	
	
/*getting and setting headline strings*/
	
	pascal void opGetHeadString (hdlheadrecord, Str255);

	pascal Boolean opSetHeadString (hdlheadrecord, Str255);
	
	
/*structure editing*/

	pascal Boolean opAddHeadline (hdlheadrecord, opDirection, Str255, hdlheadrecord *);

	pascal void opUnlink (hdlheadrecord);
	
	pascal void opDisposeStructure (hdlheadrecord);
	
	pascal Boolean opDeposit (hdlheadrecord, opDirection, hdlheadrecord);
	
	pascal Boolean opPromoteSubheads (hdlheadrecord);
	

/*getting information about a headrecord*/

	pascal Boolean opIsFirstInList (hdlheadrecord);

	pascal Boolean opIsLastInList (hdlheadrecord);

	pascal Boolean opHasSubheads (hdlheadrecord);

	pascal Boolean opSubheadsExpanded (hdlheadrecord);
	
	pascal Boolean opIsSubordinateTo (hdlheadrecord, hdlheadrecord);

	pascal Boolean opIsNestedInComment (hdlheadrecord);

	pascal hdlheadrecord opGetAncestor (hdlheadrecord, short);

	pascal hdlheadrecord opGetLastSubhead (hdlheadrecord);

	pascal hdlheadrecord opGetNthSubhead (hdlheadrecord, short);

	pascal hdlheadrecord opGetNthSummit (short);

	pascal hdlheadrecord opGetLastExpanded (hdlheadrecord);

	pascal hdlheadrecord opGetFirstAtLevel (hdlheadrecord);
	
	pascal short opGetSiblingNumber (hdlheadrecord);
	

/*counting things*/

	pascal short opCountAtLevel (hdlheadrecord);
	
	pascal short opCountSummits (void);

	pascal short opCountSubs (hdlheadrecord);

	pascal short opCountAllHeads (void);


/*traversing the outline*/

	pascal Boolean opVisitOutline (opvisitcallback visit);
	
	pascal Boolean opRecursivelyVisit (hdlheadrecord, short, opvisitcallback);

	pascal Boolean opRecursivelyVisitKidsFirst (hdlheadrecord, short, opvisitcallback);

	pascal Boolean opSiblingVisiter (hdlheadrecord, Boolean, opvisitcallback);
	
	
/*miscellaneous routines*/

	pascal void opResetLevels (hdlheadrecord);

	pascal short opSetCountExpanded (void);

	





