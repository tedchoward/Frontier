
/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

#ifndef shellinclude
#define shellinclude /*so other includes can tell if we've been loaded*/


#ifndef shelltypesinclude
	
	#include "shelltypes.h"

#endif


#ifndef configinclude
	
	#include "frontierconfig.h"

#endif

#ifndef scrollbarinclude

	#include "scrollbar.h"

#endif

#ifdef WIN95VERSION
extern HWND findreplacewindow;
#endif

/*constants*/

enum { /*indexes of internal errors*/
	
	idbadopversionnumber = 1,
	iderrorloadingformats,
	idbadtableformatsversion,
	idtoomanypophashtables,
	iddisposingsystemtable,
	idpackstackfull,
	idpackstackempty,
	idbadtempdatatype,
	idwindowzoombug,
	idoutlinestackfull,
	idopwindowopenbug,
	idinvalidoutline,
	idprocessstackfull,
	idpenstackempty,
	idportstackfull,
	idportstackempty,
	idregionstackfull,
	idclipstackfull,
	idclipstackempty,
	idstylestackfull,
	idstylestackempty,
	idscriptpoptablefailed,
	ideditorstackfull,
	idbuttonlistmissing,
	idglobalsstackfull,
	idtoomanykeyboardhooks,
	idtoomanydirtyhooks,
	idtoomanymenuhooks,
	idtoomanyeventhooks,
	idtoomanyerrorhooks,
	idtoomanyscraphooks,
	idtoomanymemoryhooks,
	idtoomanyfilehooks,
	idformatstackfull,
	idformatstackempty,
	idnilhashtable,
	idbadbucketliststring,
	iderrorloadingtable,
	idunsortedhashlist,
	idniltableaddress,
	idwpstackfull
	};

#ifdef fltrialsize

enum { /*indexes of trial errors*/

	dbsizelimitstring = 1,
	threadlimitstring,
	expirationstring,
	buyfrontierstring,
	noguestdatabasesstring
	};

#endif


/*typedefs*/

typedef struct tywindowposition {
	
	Rect windowrect;
	
	#ifdef version5orgreater
	
		short unused [6];
	
	#else
		
		short vertcurrent, vertmin, vertmax;
		
		short horizcurrent, horizmin, horizmax;
	
	#endif
	
	long ctopens; /*the number of times the file has been opened*/
	
	short rowcursor, colcursor; /*where the cursor is located*/
	
	short configresnum; /*which set of callback routines handle this document?*/
	
	boolean flhidden: 1; /*set if the window isn't visible*/
	
	short unused1, unused2, unused3, unused4; /*room for expansion*/
	
	char appitems [62]; /*reserved for use by application*/
	} tywindowposition;


typedef struct tysavedfont { /*format of a 'styl' resource*/
	
	short fontsize, fontstyle;
	
	bigstring fontname; 
	} tysavedfont;


typedef struct tyselectioninfo {
	
	boolean fldirty: 1; /*something changed, menus need updating*/
	
	boolean flcansetfont: 1; /*can we set font in current context?*/
	
	boolean flcansetsize: 1; /*can we set size?*/
	
	boolean flcansetstyle: 1; /*can we set style?*/
	
	boolean flcansetjust: 1; /*can we set justification?*/
	
	boolean flcansetleading: 1; /*can we set leading?*/
	
	short fontnum;
	
	short fontsize;
	
	tyjustification justification;
	
	short leading;
	
	short fontstyle;
	} tyselectioninfo;
	
	
typedef struct tywindowinfo { /*one of these records is linked into every window*/
	
	short configresnum; /*governs which content provider gets this window's messages*/
	
	struct tywindowinfo **nextwindow, **prevwindow; /*window list is doubly linked*/
	
	struct tywindowinfo **parentwindow;
	
	WindowPtr macwindow; /*points to the Macintosh data structure for the window*/
	
	typrocessid processid;
	
	Rect rzoomfrom; /*where this window gets zoomed from when it's revealed*/
	
	Rect zoomedrect; /*calculated fully-zoomed rect, based on window's content*/

	Rect contentrect; /*the portion of window "owned" by content handler*/
	
	Rect oldcontentrect; /*some handler's need this to resize a window*/
	
	Rect growiconrect; /*shellwindow may need this to resize a window*/
	
	Rect buttonsrect; /*where buttons are drawn, if the window has buttons*/
	
	short lastbuttonhit; /*for default routine, this is the bold button*/
	
	unsigned short buttonsenabled; /*16 bits, one for each button*/

	unsigned short buttonsdisplayed; /*16 bits, one for each button*/

	boolean flhidebuttons; /*7.1b18 PBS: hide buttons in outlines*/

	boolean flsyntheticbuttons; /*7.1b18 PBS: outlines may or may not have synthetic buttons*/

	Handle buttonscripttable; /*7.1b18 PBS: hashtable of scripts for synthetic buttons*/

	Handle buttonlist; /*7.1b18 PBS: list of buttons*/

	RgnHandle drawrgn; /*for updating routines, this is the region that needs drawing*/
	
	Rect messagerect; /*the rectangle that messages are displayed in, pre-computed*/
	
	hdlstring hmessagestring; /*the string that's displayed in the window message area*/
	
	hdlscrollbar vertscrollbar, horizscrollbar; /*the window's scroll bars*/
	
	tyscrollinfo vertscrollinfo, horizscrollinfo; /*extracted scrollbar info*/
	
	short lastcurrent; /*scrollbar value, before thumb move, for scroll-to operations*/
	
	short defaultfont, defaultsize, defaultstyle; /*app may or may not pay attention*/
	
	tyfilespec fspec; /*the specifier of the file, name usually displayed in window title bar*/
	
	hdlfilenum fnum; /*the refnum of the file that's displayed in the window, 0 if new*/
	
	short rnum; /*resource refnum, analogous to fnum, above, -1 if not open*/
	
	tywindowposition wpos; /*information that gets saved in each file*/
	
	tyselectioninfo selectioninfo; /*for two-way communication about style changes*/
	
	hdlstring hundostring; /*the operation waiting to be undone, nil if can't undo*/
	
	Handle hdata; /*the data displayed in the window, maintained by content handler*/
	
	Handle hundostack, hredostack; /*handles are untyped here so avoid shellundo.h*/
	
	hdlstring hwindowtitle; /*so we don't have to ask the window for it*/
	
	long ctpushes;
	
	boolean flwindowactive: 1; /*is the window activated?*/
	
	boolean flmadechanges: 1; /*were editing changes made to this window?*/
	
	boolean fldirtyscrollbars: 1; /*application can force update of scrollbars*/
	
	boolean flsaveas: 1; /*allows content handler to distinguish save from save-as*/
	
	boolean flhidden: 1; /*indicates whether the window is hidden or visible*/
	
	boolean flmessagelocked: 1; /*indicates whether window message is locked or not*/
	
	boolean flmadeviewchanges: 1; /*set for non-content changes: scroll, cursor, window pos*/
	
	boolean flneversaved: 1; /*new db window, never saved?*/
	
	boolean flopenedforfind: 1; /*was window opened to show found occurence during search?*/
	
	boolean fldontownwindow: 1; /*if true, the shell doesn't own the macwindow itself*/
	
	boolean flbeingclosed: 1; /*if true, we're inside of shellclosexxx right now*/
	
	boolean fldisposewhenpopped: 1; //if true, dispose was called while we were pushed
	} tywindowinfo, *ptrwindowinfo, **hdlwindowinfo;


typedef boolean (*shellcallback) (void);

typedef void (*shellvoidcallback) (void);

typedef void (*shellactivatecallback) (boolean);

typedef boolean (*shellbooleancallback) (boolean);

typedef boolean (*shellexportscrapcallback) (void *, tyscraptype, Handle *, boolean *);

typedef void (*shelldisposescrapcallback) (void *);

typedef boolean (*shellscrollcallback) (tydirection, boolean, long);

typedef boolean (*shellgetundoglobalscallback) (long *);

typedef boolean (*shellsetundoglobalscallback) (long, boolean);

typedef boolean (*shellmousecallback) (Point, tyclickflags);

typedef boolean (*shellcontentsizecallback) (long *, long *);

typedef boolean (*shellcmdkeycallback) (char);

typedef boolean (*shellshortcallback) (short);

typedef boolean (*shellbuttonstatuscallback) (short, tybuttonstatus *);

typedef boolean (*shellpointcallback) (Point);

typedef boolean (*shellwindowcallback) (WindowPtr);

typedef boolean (*shellfnumcallback) (hdlfilenum);

typedef boolean (*shellloadcallback) (hdlfilenum, short);

typedef boolean (*shellloadspecialcallback) (ptrfilespec, OSType);

typedef boolean (*shellsavecallback) (ptrfilespec, hdlfilenum, short, boolean, boolean);

typedef boolean (*shellgetvariablecallback) (void *);

typedef boolean (*shellmessagecallback) (bigstring, boolean);

typedef boolean (*shellwindowinfocallback) (hdlwindowinfo);

typedef boolean (*shellwindowvisitcallback) (WindowPtr, ptrvoid);

typedef boolean (*shellzoomwindowcallback) (hdlwindowinfo, boolean);

typedef boolean (*shelldatabasecallback) (struct tydatabaserecord ***);


typedef struct tyshellglobals {
	
	WindowPtr *windowholder; /*the caller's global window record*/
	
	Handle *dataholder; /*the caller's global data handle*/
	
	hdlwindowinfo *infoholder; /*the caller's global window info holder*/

	short configresnum; /*config resource associated with this window type*/
	
	tyconfigrecord config; /*configuration information about the caller*/
	
	Handle buttonlist; /*list of buttons associated with windows of this type*/
	
	shellcallback initroutine; /*called after Mac toolbox is init'd*/
	
	shellvoidcallback quitroutine; /*called before we quit*/
	
	shellcallback setglobalsroutine;
	
	shellcallback pushroutine;

	shellcallback poproutine;
	
	shellcallback setsuperglobalsroutine;
	
	shellcallback newrecordroutine;
	
	shellcallback disposerecordroutine;
	
	shellloadcallback loadroutine;
	
	shellloadspecialcallback loadspecialroutine;
	
	shellwindowcallback precloseroutine;	/*4.1b5 dmb*/
	
	shellcallback closeroutine;
	
	shellwindowcallback childcloseroutine;
	
	shellcallback presaveroutine;
	
	shellsavecallback saveroutine;
	
	shellcallback pagesetuproutine;
	
	shellcallback beginprintroutine;
	
	shellcallback endprintroutine;
	
	shellshortcallback printroutine;
	
	shellcallback setprintinfoproutine;
	
	shellvoidcallback updateroutine;
	
	shellactivatecallback activateroutine;
	
	shellactivatecallback resumeroutine;
	
	shellcontentsizecallback getcontentsizeroutine;
	
	shellwindowinfocallback resetrectsroutine;
	
	shellvoidcallback resizeroutine;
	
	shellscrollcallback scrollroutine;
	
	shellvoidcallback setscrollbarroutine;
	
	shellmousecallback mouseroutine;

	shellmousecallback rmouseroutine;
	
	shellmousecallback cmouseroutine;

	shellmousecallback wmouseroutine;
	
	shellshortcallback buttonroutine;
	
	shellbuttonstatuscallback buttonstatusroutine;
	
	shellcallback keystrokeroutine;
	
	shellcmdkeycallback cmdkeyfilterroutine;
	
	shellcallback menuroutine;
	
	shellpointcallback titleclickroutine;
	
	shellcallback undoroutine;
	
	shellcallback setundostatusroutine;
	
	shellgetundoglobalscallback getundoglobalsroutine;
	
	shellsetundoglobalscallback setundoglobalsroutine;
	
	shellcallback cutroutine;
	
	shellcallback copyroutine;
	
	shellcallback pasteroutine;
	
	shellcallback clearroutine;
	
	shellcallback selectallroutine;
	
	shellcallback fontroutine;
	
	shellcallback sizeroutine;
	
	shellcallback styleroutine;
	
	shellcallback leadingroutine;
	
	shellcallback justifyroutine;
	
	shellcallback setselectioninforoutine;
	
	shellcallback searchroutine;
	
	shellcallback replaceroutine;
	
	shellcallback executeroutine;
	
	shellcallback setdatabaseroutine; 
	
	shelldatabasecallback getdatabaseroutine; 
	
	shellfnumcallback fnumchangedroutine;
	
	shellvoidcallback idleroutine;
	
	shellpointcallback adjustcursorroutine;
	
	shellcallback backgroundroutine;
	
	shellmessagecallback msgroutine;
	
	shellshortcallback gettargetdataroutine;
	
	shellgetvariablecallback getvariableroutine;
	
	shellcallback findusedblocksroutine;
	
	shellbooleancallback settextmoderoutine;
	
	shellzoomwindowcallback zoomwindowroutine;
	
	} tyshellglobals, *ptrcallbacks;

/*
shellevent, shellwindow and shellwindowinfo provide information for the shell 
level about the current event being processed and the current window, the one
whose globals are currently "pushed".

in some cases, handlers refer to shellevent, especially where we've licensed 
code that needs a Macintosh eventrecord as part of its API.

shellglobals holds the addresses of all the current callback routines, as well
as the addresses of the data, window, and windowinfo record pointers for the
current "pushed" window.
*/
	extern EventRecord shellevent; /*the last event received from the Mac event manager*/

	extern WindowPtr shellwindow; /*the window that the current globals came from*/

	extern hdlwindowinfo shellwindowinfo; /*the windowinfo record linked into current window*/

	extern tyshellglobals shellglobals; /*the callback routines for the current content provider*/

/*
special case global flags
*/
	extern boolean flinhibitclosedialogs;

	extern boolean flconvertingolddatabase;

	extern boolean flshellclosingall;


/*prototypes*/

extern void shellerrormessage (bigstring); /*shell.c*/

#ifdef fldebug
	
	#define shellinternalerror(n, s) shellinternalerrormessage (s)
	
	extern void shellinternalerrormessage (bigstring);

#else
	
	#define shellinternalerror(n, s) shellinternalerrormessage (n)

	extern void shellinternalerrormessage (short);

#endif


#ifdef fltrialsize

	extern void shelltrialerror (short);

#endif


extern boolean shellgetstring (short, bigstring);

extern boolean shellshutdown (void);

extern boolean shellquit (void);

extern void shellforcecursoradjust (void);

extern void shelladjustcursor (void);

extern void shellidle (void);

extern boolean shellyield (boolean);

extern void shellforcebackgroundtask (void);

extern boolean shellbackgroundtask (void);

extern boolean shellprocessevent (EventRecord *);

extern boolean shellpostevent (EventRecord *);

extern boolean shelleventloop (callback);

extern boolean shellshorteventloop (void);

extern boolean shellpartialeventloop (short);

extern void shellexitmaineventloop (void);

extern void shellmaineventloop (void);

extern boolean shellinit (void);

extern boolean shellstart (void);


extern boolean shellactivatewindow (WindowPtr, boolean); /*shellactivate.c*/


extern short shellblockedevents (void); /*shellblocker.c*/

extern boolean shellblocked (short);

extern boolean shellpushblock (short, boolean);

extern boolean shellpopblock (void);

extern boolean shelleventsblocked (void);

extern boolean shellblockevents (void);

extern boolean shellpopevents (void);


extern boolean shellnewcallbacks (ptrcallbacks *); /*shellcallbacks.c*/

extern boolean shelldefaultfontroutine (void);

extern boolean shelldefaultsizeroutine (void);

extern boolean shelldefaultselectioninfo (void);

extern boolean shellsetglobals (WindowPtr);

extern boolean shellpushglobals (WindowPtr);

extern boolean shellpopglobals (void);

extern boolean shellpushwindowglobals (hdlwindowinfo);

extern void shellinvalidglobals (WindowPtr);

extern boolean shellgetfrontrootinfo (hdlwindowinfo *hinfo);

extern boolean shellpushconfigglobals (short);

extern boolean shellpushdefaultglobals (void);

extern boolean shellpushfrontglobals (void);

extern boolean shellpushrootglobals (WindowPtr);

extern boolean shellpushparentglobals (void);

extern boolean shellpushfrontrootglobals (void);

extern boolean shellsetsuperglobals (void);

extern boolean shellgetconfig (short, tyconfigrecord *);

extern boolean shellsetconfig (short, tyconfigrecord);

extern boolean shellgetundoglobals (long *);

extern boolean shellsetundoglobals (long, boolean);


extern boolean shellopenfile (ptrfilespec, boolean, WindowPtr *); /*shellfile.c*/

extern boolean shellopen (void);

extern void shellopeninitialfiles (void);

extern boolean shellopendefaultfile (void);

extern boolean shellsaveas (WindowPtr, ptrfilespec, boolean);

extern boolean shellnewfile (ptrfilespec, boolean, WindowPtr *);

extern boolean shellnew (void);

extern boolean shellsave (WindowPtr);

extern boolean shellrevert (WindowPtr, boolean);

extern boolean shellclosechildwindows (hdlwindowinfo);

extern boolean shellclosewindow (WindowPtr);

extern boolean shellcloseall (WindowPtr, boolean);

extern boolean shellclose (WindowPtr, boolean);


extern boolean shellisactive (void); /*shelljuggler.c*/

extern boolean shellactivate (void);


extern void shellhandlekeystroke (void); /*shellkb.c*/

extern boolean shellfilterscrollkey (byte);
	
extern void shellouch (void);


extern void shellhandledrag (EventRecord, WindowPtr); /*shellmouse.c*/

extern void shellhandlemouse (void);

extern void shellhandlemouseup (void);

extern boolean shellcheckautoscroll (Point, boolean, tydirection *);

extern void initmouse (void);


extern void shellsetselectioninfo (void); /*shellops.c*/

extern void shellsetdefaultstyle (hdlwindowinfo);


extern void shelldisposescrap (void); /*shellscrap.c*/

extern boolean shellsetscrap (void *, tyscraptype, shelldisposescrapcallback, shellexportscrapcallback);

extern boolean shellgetscrap (Handle *, tyscraptype *);

extern boolean shellconvertscrap (tyscraptype, Handle *, boolean *);

extern boolean shellexportscrap (tyscraptype);

extern boolean shellreadscrap (void);

extern boolean shellwritescrap (tyscraptype);

extern void initscrap (void);


extern void shellupdatescrollbars (hdlwindowinfo); /*shellscroll.c*/

extern void shellcheckdirtyscrollbars (void);

extern void shellsetscrollbars (WindowPtr);

extern void shellscroll (boolean, hdlscrollbar, short, Point);
//Code change by Timothy Paustian Saturday, July 22, 2000 9:56:19 PM
//two new routines to allow initialization and destruction of the scroll UPPs
extern void	shellinitscroll();

extern void shellshutdownscroll();

extern void fileinit();

extern void	fileshutdown();

#ifdef WIN95VERSION
void winscroll (boolean isvertscroll, int scrolltype, long pos);
#endif

extern void shellupdatewindow (WindowPtr); /*shellupdate.c*/

extern void shellupdatecontent (Rect);

extern boolean shellupdatenow (WindowPtr);

extern void shellupdateallnow (void);

extern boolean shellhandleupdate (void);


extern void shellwindowinval (hdlwindowinfo); /*shellwindow.c*/

extern void shelladjustaftergrow (WindowPtr);

extern void shellinvalcontent (hdlwindowinfo);

extern boolean shellrectneedsupdate (const Rect *); // 1/31/97 dmb

extern boolean shellsetwindowchanges (hdlwindowinfo, boolean);

extern boolean windowsetchanges (WindowPtr, boolean);

extern boolean getwindowinfo (WindowPtr, hdlwindowinfo *);

extern boolean shellfindwindow (short, WindowPtr *, hdlwindowinfo *, Handle *);

extern hdlfilenum windowgetfnum (WindowPtr);

extern void windowsetfnum (WindowPtr, hdlfilenum);

extern short windowgetrnum (WindowPtr);

extern void windowsetrnum (WindowPtr, short);

extern short windowgetvnum (WindowPtr);

extern boolean windowsetfspec (WindowPtr, tyfilespec *);

extern boolean windowgetfspec (WindowPtr, tyfilespec *);

extern boolean windowgetpath (WindowPtr, bigstring);

extern void windowsetvnum (WindowPtr, short);

extern boolean newchildwindow (short, hdlwindowinfo, Rect *, const Rect *, bigstring, WindowPtr *);

extern boolean newfilewindow (ptrfilespec, hdlfilenum, short, boolean, WindowPtr *);

extern boolean isfilewindow (WindowPtr);

extern boolean shellgetwindowcenter (hdlwindowinfo, Rect *);

extern boolean shellgetglobalwindowrect (hdlwindowinfo, Rect *);

extern boolean shellgetwindowrect (hdlwindowinfo, Rect *);

extern boolean windowzoom (WindowPtr);

extern boolean shellbringtofront (hdlwindowinfo);

extern boolean shellsetwindowtitle (hdlwindowinfo, bigstring);

extern void shellgetwindowtitle (hdlwindowinfo, bigstring);

extern boolean getfrontwindowinfo (hdlwindowinfo *);

extern WindowPtr shellnextwindow (WindowPtr);

extern void shellcalcgrowiconrect (Rect, hdlwindowinfo);

extern boolean shellgettargetdata (short, WindowPtr);

extern boolean shellfindtargetwindow (short, WindowPtr *);

extern boolean shellgetexternaldata (hdlwindowinfo, void *);

extern boolean shellvisitwindowlayer (shellwindowvisitcallback, ptrvoid);

extern boolean visitownedwindows (WindowPtr, short, shellwindowvisitcallback, ptrvoid);

extern boolean shellvisittypedwindows (short, shellwindowvisitcallback, ptrvoid);

extern boolean getrootwindow (WindowPtr, hdlwindowinfo *);

extern boolean frontrootwindow (hdlwindowinfo *);

extern boolean shellfinddatawindow (Handle, hdlwindowinfo *);

extern WindowPtr shellfindfilewindow (ptrfilespec);

extern boolean shellfindnamedwindow (const bigstring, hdlwindowinfo *);

extern boolean shellclosedatawindow (Handle);

extern void shellwindowmessage (bigstring);

extern boolean shellsetwindowmessage (hdlwindowinfo, bigstring);

extern boolean shelldrawwindowmessage (hdlwindowinfo);

extern boolean shellfrontwindowmessage (bigstring);

extern boolean shellmoveandsizewindow (hdlwindowinfo, Rect);

extern boolean shellmovewindow (hdlwindowinfo, short, short);

extern boolean shellmovewindowhidden (hdlwindowinfo, short, short);

extern boolean shellsizewindow (hdlwindowinfo, short, short);

extern boolean shellsizewindowhidden (hdlwindowinfo, short, short);

extern boolean shellzoomwindow (hdlwindowinfo, boolean);

extern boolean shellgetdatabase (WindowPtr, struct tydatabaserecord ***);

extern void shellwindowmenudirty (void); /*shellwindowmenu.c*/

#ifdef WIN95VERSION
	extern boolean shellprocessfindmessage (FINDREPLACE * fr);

	extern void shelldestoycaretinmainthread (void);
#endif

#endif



