
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef isFrontier
#define appletpopupinclude


#ifndef appletdefsinclude
	
	#include <appletdefs.h>
	
#endif


#define popuparrowwidth 16 

#define popupdrawinset 5


typedef boolean (*popupcallback) (void);

typedef boolean (*popupdrawcontentscallback) (Rect);


typedef struct typopuprecord {
	
	bigstring bs;
	
	Rect popuprect;
	
	boolean flarrows;
	
	boolean flleftarrow, flrightarrow; /*you can have one arrow pointing out of the popup*/
	
	boolean trackleftarrow, trackrightarrow; /*you can handle mouse tracking*/
	
	short fontnum, fontsize, fontstyle;
	
	Rect leftarrowrect;
	
	Rect rightarrowrect;
	
	short checkeditem;
	
	short itemselected;
	
	short idmenu;
	
	boolean fldisposemenu;
	
	MenuHandle hmenu;
	
	popupcallback getmenucallback;
	
	popupcallback leftarrowcallback;
	
	popupcallback rightarrowcallback;
	
	popupcallback menuselectcallback;
	
	popupdrawcontentscallback drawcontentscallback;
	} typopuprecord;


	
boolean initpopuprecord (typopuprecord *);

boolean pushpopupitem (typopuprecord *, bigstring bs, boolean);

void drawpopup (typopuprecord *);

boolean popupclick (typopuprecord *);

boolean adjustpopupcursor (typopuprecord *);

void idealpopupsize (typopuprecord *, short *, short *);

#endif
