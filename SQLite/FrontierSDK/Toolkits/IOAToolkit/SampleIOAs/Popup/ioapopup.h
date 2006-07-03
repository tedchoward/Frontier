
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define ioapopupinclude


typedef struct typopupdata {
	
	boolean haslabel; /*true if the value is displayed as the label for the popup*/
	
	bigstring bspopup; /*the string that's displayed inside the popup rectangle*/
	
	short checkeditem; /*the item that's checked in the menu*/
	
	Handle hmenuitems; /*semicolon-separated text handle*/
	
	short popupwidth; /*set by cleaning up, the width of the popup menu part of the object*/
	
	MenuHandle hmenu;
	} typopupdata, **hdlpopupdata;


boolean getnthpopupitem (hdlobject, short, bigstring);
