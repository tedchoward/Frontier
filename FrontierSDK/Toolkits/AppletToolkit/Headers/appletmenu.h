
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define applemenu 128
#define aboutitem 1

#define filemenu 129

#ifndef customizedfilemenu

	#define newitem 1
	#define openitem 2
	#define closeitem 3
	#define saveitem 5
	#define saveasitem 6
	#define revertitem 7
	#define pagesetupitem 9
	#define printitem 10
	#define quititem 12

#endif

#define editmenu 130

#ifndef customizededitmenu 

	#define undoitem 1
	#define cutitem 3
	#define copyitem 4
	#define pasteitem 5
	#define clearitem 6
	#define selectallitem 8
	#define fontitem 10
	#define sizeitem 11
	#define styleitem 12
	#define justifyitem 13
	
#endif

#define fontmenu 131

#define sizemenu 132
#define point9item 1
#define point10item 2
#define point12item 3
#define point14item 4
#define point18item 5
#define point24item 6
#define point36item 7
#define point48item 8
#define point72item 9
#define pointcustomitem 11
#define pointupitem -1
#define pointdownitem -2

#define stylemenu 133
#define plainitem 1
#define bolditem 3
#define italicitem 4
#define underlineitem 5
#define outlineitem 6
#define shadowitem 7

#define justifymenu 134
#define leftjustifyitem 1
#define rightjustifyitem 2
#define centerjustifyitem 3
#define fulljustifyitem 4

#define extendedmenu 135 /*never appears in user interface -- used for externed keyboard*/
#define helpitem 1
#define homeitem 2
#define enditem 3
#define pageupitem 4
#define pagedownitem 5

#define windowmenu 136



#define firstsharedmenu 200 /*leave room for 70 Applet-defined menus*/

extern MenuHandle hdlapplemenu, hdlfilemenu, hdleditmenu;

extern MenuHandle hdlfontmenu, hdlsizemenu, hdlstylemenu, hdljustifymenu;

void adjustmenus (void);

void initmenus (void);

void handlemenu (long);

boolean flipcustommenu (hdlappwindow, boolean);

void dialogmenuenabler (boolean);

void appcopycommand (void);

void disableeditmenuitems (void);