
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef __appletmain__
#define __appletmain__

void installmenu (short, MenuHandle *);

void initmenus (void);

void getappdata (WindowPtr, Handle *);

boolean getappwindow (WindowPtr, hdlappwindow *);

boolean findappwindow (Point, hdlappwindow *);

void getappwindowtitle (hdlappwindow, bigstring);

void setappwindowtitle (hdlappwindow, bigstring);

void hideappwindow (hdlappwindow);

void getappwindowfilespec (hdlappwindow, ptrfilespec);

void showappwindow (hdlappwindow);

void selectappwindow (hdlappwindow);

boolean frontappwindow (hdlappwindow);

void updateappwindow (hdlappwindow);
 
void zoomappwindow (hdlappwindow);

void computewindowinfo (WindowPtr, hdlappwindow);

void disposeappwindow (hdlappwindow);

boolean visitappwindows (boolean  (*) ());

boolean selectvisit (hdlappwindow);

boolean selectwindowbytitle (bigstring);

boolean findbytitlevisit (hdlappwindow);

boolean findnthvisit (hdlappwindow);

boolean countwindowsvisit (hdlappwindow);

boolean resetdirtyscrollbarsvisit (hdlappwindow);

boolean resetdirtyscrollbars (void);

boolean getuntitledtitle (bigstring);

boolean setfrontglobalsvisit (hdlappwindow);

boolean setfrontglobals (void);

boolean newuntitledappwindow (boolean);
 
boolean openappwindow (ptrfilespec);

boolean editappwindow (Handle, OSType, AEDesc *, bigstring);

boolean saveappwindow (ptrfilespec);

boolean closeappwindow (hdlappwindow, boolean);

boolean closewindowvisit (hdlappwindow);

boolean closeallwindows (void);

boolean saveaswindow (ptrfilespec);

boolean saveasfrontwindow (void);

boolean savefrontwindow (void);

boolean openfrontwindow (void);

boolean revertfrontwindow (void);

boolean exitmainloop (void);

void adjustmenus (void);

void maineventloop (void);

boolean appserviceeventqueue (void);

boolean mousedoubleclick (Point);

void setselectionstyleinfo (tyselectioninfo *, boolean);

boolean selectallcommand (void);

void appsetfont (bigstring);

void appsetfontsize (short);

void adjustaftergrow (WindowPtr, Rect);

boolean findbyfile (ptrfilespec, hdlappwindow *);

void appleteventhandler (EventRecord *);

void appleteventdispatcher (EventRecord *);

boolean appuseractive (void);

#endif	// __appletmain__
