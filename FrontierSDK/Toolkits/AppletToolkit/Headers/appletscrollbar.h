
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


void invalscrollbar (hdlscrollbar);

void validscrollbar (hdlscrollbar);

boolean pointinscrollbar (Point, hdlscrollbar);

void activatescrollbar (hdlscrollbar, boolean);

short getscrollbarcurrent (hdlscrollbar);

void showscrollbar (hdlscrollbar);

void hidescrollbar (hdlscrollbar);

void drawscrollbar (hdlscrollbar);

void displayscrollbar (hdlscrollbar);

void setscrollbarminmax (hdlscrollbar, short, short);

void setscrollbarcurrent (hdlscrollbar, short);

short getscrollbarwidth (void);

boolean newscrollbar (WindowPtr, boolean, hdlscrollbar *);

void disposescrollbar (hdlscrollbar);

void getscrollbarrect (hdlscrollbar, Rect *);

void setscrollbarrect (hdlscrollbar, Rect);

void scrollbarflushright (Rect, hdlscrollbar);

void scrollbarflushbottom (Rect, hdlscrollbar);

boolean findscrollbar (Point, WindowPtr, hdlscrollbar *, short *);

boolean scrollbarhit (hdlscrollbar, short, boolean *, boolean *);

void resetappscrollbars (hdlappwindow);

void resizeappscrollbars (hdlappwindow);

void showappscrollbars (hdlappwindow);

void updateappscrollbars (hdlappwindow);

void activateappscrollbars (hdlappwindow, boolean);

tydirection scrolldirection (boolean, boolean);

void handlescrollbar (boolean, hdlscrollbar, short, Point);

boolean scrollappwindow (tydirection, boolean, short);

boolean scrolltoappwindow (void);

void enablescrollbar (hdlscrollbar);

void disablescrollbar (hdlscrollbar);

boolean scrollbarenabled (hdlscrollbar);

void getscrollbarinfo (hdlscrollbar, short *, short *, short *);

void setscrollbarinfo (hdlscrollbar, short, short, short);

void installscroll (void);

boolean appdefaultscroll (tydirection, boolean, short);

boolean appdefaultresetscroll (void);

