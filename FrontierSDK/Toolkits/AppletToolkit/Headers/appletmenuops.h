
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define menuinclude


typedef boolean (*tymenuvisitcallback) (MenuHandle, short);


boolean setmenuitem (MenuHandle, short, bigstring);

boolean getmenuitem (MenuHandle, short, bigstring);

void deletemenuitem (MenuHandle, short);

void deleteallmenuitems (MenuHandle hmenu);

void stylemenuitem (MenuHandle, short, short);

void disablemenuitem (MenuHandle, short);

void enablemenuitem (MenuHandle, short);

void setmenuitemenable (MenuHandle, short, boolean);

void installmenubefore (short, short, MenuHandle *);

void installmenu (short, MenuHandle *);

void installhierarchicmenu (short, MenuHandle *);

void deletemenu (short);

short countmenuitems (MenuHandle);

void uncheckallmenuitems (MenuHandle);

void enableallmenus (void);

void disableallmenus (void);

void disableallmenuitems (MenuHandle);

void enableallmenuitems (MenuHandle);

void checkmenuitem (MenuHandle, short, boolean);

void setmenuitemmark (MenuHandle, short, char);

boolean visitonemenu (short, tymenuvisitcallback);

boolean visitallmenus (tymenuvisitcallback);

boolean pushmenuitem (MenuHandle, bigstring, boolean);

void disableemptymenuitems (MenuHandle);

boolean findinmenu (MenuHandle, bigstring, short *);
