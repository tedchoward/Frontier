
/*	$Id$    */

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

#include "frontier.h"
#include "standard.h"

#ifdef WIN95VERSION 
#include "frontierwinmain.h"
#endif

#include "memory.h"
#include "font.h"
#include "kb.h"
#include "notify.h"
#include "ops.h"
#include "search.h"
#include "strings.h"
#include "frontierwindows.h"
#include "shell.h"
#include "shellmenu.h"
#include "shellprint.h"
#include "shellprivate.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "process.h"
#include "kernelverbs.h"
#include "wpengine.h"
#include "kernelverbdefs.h"
#include "cancoon.h"
#include "resources.h"

#ifdef WIN95VERSION 
#include "WinLand.h"
#include "dlgs.h"
#endif

#include "langregexp.h"


typedef enum tysearchtoken { /*search verbs*/
	
	resetfunc,
	
	findnextfunc,
	
	replacefunc,
	
	replaceallfunc,
	
	findtextdialogfunc,

	replacetextdialogfunc,
	
	ctsearchverbs
	} tysearchtoken;


typedef enum tyfilemenutoken { /*fileMenu verbs*/
	
	newfunc,
	
	openfunc,
	
	closefunc,
	
	closeallfunc,
	
	savefunc,
	
	savecopyfunc,
	
	revertfunc,
	
	printfunc,
	
	quitfunc,
	
	ctfilemenuverbs
	} tyfilemenutoken;


typedef enum tyeditmenutoken { /*editMenu verbs*/
	
	undofunc,
	
	cutfunc,
	
	copyfunc,
	
	pastefunc,
	
	clearfunc,
	
	selectallfunc,
	
	getfontfunc,
	
	getfontsizefunc,
	
	fontfunc,
	
	fontsizefunc,
	
	plainfunc,
	
	boldfunc,
	
	italicfunc,
	
	underlinefunc,
	
	outlinefunc,
	
	shadowfunc,
	
	cteditmenuverbs
	} tyeditmenutoken;


#define str_hidden ("\x06" "hidden")


#ifdef WIN95VERSION

typedef struct tyfindreplaceinfo {
	
	int func;
	
	boolean flwraparound;
	
	boolean flregexp;
	
	} tyfindreplaceinfo;
	

UINT_PTR CALLBACK shellfindreplacehook (HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam); /*forward declaration*/


static tyfindreplaceinfo frinfo;
static FINDREPLACE frstruct;
static bigstring windialogfind;
static bigstring windialogreplace;


static boolean copytodialog (bigstring bssource, bigstring bsdest) {

	/*
	5.0.2b20 dmb: copy the pascal search string bssource to bsdest
	
	escape all crs and tabs, and null terminate the result
	*/
	
	short ix;
	
	copystring (bssource, bsdest);
	
	while ((ix = patternmatch ("\x01" "\r", bsdest)) > 0)
		replacestring (bsdest, ix, 1, "\x02" "\\r");
	
	while ((ix = patternmatch ("\x01" "\t", bsdest)) > 0)
		replacestring (bsdest, ix, 1, "\x02" "\\t");
	
	nullterminate (bsdest);

	return (true);
	} /*copytodialog*/


static boolean copyfromdialog (bigstring bssource, bigstring bsdest) {

	/*
	5.0.2b20 dmb: copy the null-terminated dialog string to bsdest
	
	unescape all crs and tabs.
	*/
	
	short ix;
	
	copyctopstring (stringbaseaddress (bssource), bsdest);
	
	while ((ix = patternmatch ("\x02" "\\r", bsdest)) > 0)
		replacestring (bsdest, ix, 2, "\x01" "\r");
	
	while ((ix = patternmatch ("\x02" "\\t", bsdest)) > 0)
		replacestring (bsdest, ix, 2, "\x01" "\t");
	
	return (true);
	} /*copyfromdialog*/


#endif


static WindowPtr lastwindowsearched = nil;


static byte *nameparams [] = {
	
	"\x09" "searchfor",
	
	"\x0b" "replacewith",
	
	"\x0d" "casesensitive",
	
	"\x0a" "wholewords",
	
	"\x0a" "wraparound",
	
	"\x0a" "flatsearch",
	
	"\x0f" "frontwindowonly",
	
	"\x0c" "closewindows",
	
	"\x11" "regularexpression"
	};

enum nameindexes {
	
	ixsearchfor,
	
	ixreplacewith,
	
	ixcasesensitive,
	
	ixwholewords,
	
	ixwraparound,
	
	ixflatsearch,
	
	ixfrontwindowonly,
	
	ixclosewindows,
	
	ixregularexpression
	};

static void shellverbsetselectioninfo () {
	
	/*
	2.1b14 dmb: make sure menus update when a verb changes font/size/style
	*/
	
	shellsetselectioninfo (); /*make changes effect window*/
	
	(**shellwindowinfo).selectioninfo.fldirty = true; /*make sure menus update*/
	} /*shellverbsetselectioninfo*/


static boolean shellstyleverb (hdltreenode hparam1, short style, tyvaluerecord *v) {
	
	register hdlwindowinfo hw = shellwindowinfo;
	boolean flwantstyle;
	boolean flhavestyle;
	boolean fl = false;
	short fontstyle;
	
	if ((**hw).selectioninfo.fldirty)
		shellsetselectioninfo (); /*get latest*/
	
	fontstyle = (**hw).selectioninfo.fontstyle;
	
	if (style == normal) {
		
		if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
			return (false);
		
		flwantstyle = false;
		
		flhavestyle = fontstyle != 0;
		}
	else {
		
		flnextparamislast = true;
		
		if (!getbooleanvalue (hparam1, 1, &flwantstyle))
			return (false);
		
		flhavestyle = (fontstyle & style) != 0;
		}
	
	if (flwantstyle != flhavestyle) {
		
		(**hw).selectioninfo.fontstyle = style;
		
		fl = (*shellglobals.styleroutine) ();
		
		shellverbsetselectioninfo ();
		}
	
	return (setbooleanvalue (fl, v));
	} /*shellstyleverb*/


static boolean getstringsearchparam (short ixparam, bigstring bsvalue) {
	
	/*
	4/16/92 dmb: use hashgetvaluestring in case value isn't a string, as expected
	
	2.1b4 dmb: don't use hashgetvaluestring; do our own coercion to avoid display stuff
	*/
	
	tyvaluerecord val;
	bigstring bsname;
	hdlhashnode hnode;
	
	copystring (nameparams [ixparam], bsname);
	
	if (!hashlookup (bsname, &val, &hnode))
		return (false);
	
	if (!copyvaluerecord (val, &val))
		return (false);
	
	if (coercetostring (&val))
		pullstringvalue (&val, bsvalue);
	
	releaseheaptmp ((Handle) val.data.stringvalue);
	
	return (true);
	} /*getstringsearchparam*/


static boolean setstringsearchparam (short ixparam, bigstring bsvalue) {
	
	tyvaluerecord val;
	bigstring bsname;
	
	copystring (nameparams [ixparam], bsname);
	
	if (!setstringvalue (bsvalue, &val))
		return (false);
	
	if (!hashassign (bsname, val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	return (true);
	} /*setstringsearchparam*/


static boolean getbooleansearchparam (short ixparam, boolean *flvalue) {
	
	tyvaluerecord val;
	bigstring bsname;
	hdlhashnode hnode;
	
	copystring (nameparams [ixparam], bsname);
	
	if (!hashlookup (bsname, &val, &hnode))
		return (false);
	
	*flvalue = val.data.flvalue;
	
	return (true);
	} /*getbooleansearchparam*/


static boolean setbooleansearchparam (short ixparam, boolean flvalue) {
	
	tyvaluerecord val;
	bigstring bsname;
	
	copystring (nameparams [ixparam], bsname);
	
	setbooleanvalue (flvalue, &val);
	
	return (hashassign (bsname, val));
	} /*setbooleansearchparam*/


static boolean pushsearchparamstable (void) {
	
	bigstring bs;
	hdlhashtable hparamstable = nil;
	
	if (!getsystemtablescript (idsearchparamstable, bs))
		return (false);
	
	disablelangerror ();
	
	if (langexpandtodotparams (bs, &hparamstable, bs))
		; /*table being non-nil is our flag*/
	
	enablelangerror ();
	
	if (hparamstable == nil)
		return (false);
	
	return (pushhashtable (hparamstable));
	} /*pushsearchparamstable*/


boolean getsearchparams (void) {
	
	/*
	9/20/91 dmb: load up the searchparam record from the database params 
	table.  temporarily change the search and replace strings to a special 
	value so that they won't be found during the search.
	
	this call should be paired with a call to setsearchparams so that 
	the search and replace strings are restored.
	
	2003-05-17 AR: extended to handle regular expression find and replace strings
	*/
	
	bigstring bs;
	boolean fl;
	boolean flupdateregexp = false;
	boolean flresult = false;
		
	if (!pushsearchparamstable ())
		return (false);
	
	if (getbooleansearchparam (ixcasesensitive, &fl)) {
		flupdateregexp = flupdateregexp || (fl != searchparams.flunicase);
		searchparams.flunicase = !fl;
		}
	
	if (getbooleansearchparam (ixwholewords, &fl))
		searchparams.flwholewords = fl;
	
	if (getbooleansearchparam (ixwraparound, &fl))
		searchparams.flwraparound = fl;
	
	if (getbooleansearchparam (ixflatsearch, &fl))
		searchparams.floneobject = !fl;
	
	if (getbooleansearchparam (ixfrontwindowonly, &fl))
		searchparams.flonelevel = fl;
	
	if (getbooleansearchparam (ixclosewindows, &fl))
		searchparams.flclosebehind = fl;
	
	if (getbooleansearchparam (ixregularexpression, &fl)) {
		flupdateregexp = flupdateregexp || (fl != searchparams.flregexp);
		searchparams.flregexp = fl;
		}
	
	if (getstringsearchparam (ixsearchfor, bs)) {
		
		if (!equalstrings (bs, searchparams.bsorigfind)) { /*it changed*/
			
			endcurrentsearch ();

			copystring (bs, searchparams.bsorigfind);

			copystring (bs, searchparams.bsfind);

			if (searchparams.flunicase)
				alllower (searchparams.bsfind);
					
			flupdateregexp = true;
			}
		}
	
	if (getstringsearchparam (ixreplacewith, bs)) {

		if (!equalstrings (bs, searchparams.bsorigreplace)) { /*it changed*/
			
			copystring (bs, searchparams.bsorigreplace);
			
			copystring (bs, searchparams.bsreplace);
			
			flupdateregexp = true;
			}
		}
	
	searchparams.floneobject = searchparams.floneobject || searchparams.flonelevel;

#ifdef flregexpverbs

	if (flupdateregexp) {
		
		disposehandle (searchparams.hcompiledpattern);
		searchparams.hcompiledpattern = nil;
		
		disposehandle (searchparams.hovector);
		searchparams.hovector = nil;
		
		if (searchparams.flregexp) {
			
			bigstring bsmsg;
			int options;

			setemptystring (bsmsg);
						
			nullterminate (searchparams.bsorigfind);
			
			options = PCRE_DOTALL | PCRE_MULTILINE | PCRE_NOTEMPTY | (searchparams.flunicase ? PCRE_CASELESS : 0L);

			if (!regexpcompile (stringbaseaddress (searchparams.bsorigfind), options, bsmsg, &searchparams.hcompiledpattern)) {
				if (!isemptystring (bsmsg))
					langerrormessage (bsmsg);
				goto exit;
				}
			
			if (!regexpcheckreplacement (searchparams.hcompiledpattern,
											stringbaseaddress (searchparams.bsorigreplace),
											stringlength (searchparams.bsorigreplace))) {
				goto exit;
				}
				
			if (!regexpnewovector (searchparams.hcompiledpattern, &searchparams.hovector)) {
				goto exit;
				}
			}
		}

#endif 
	
	setstringsearchparam (ixsearchfor, zerostring);
	
	setstringsearchparam (ixreplacewith, zerostring);

	flresult = true;
	
exit:
	
	pophashtable ();
	
	return (flresult);
	} /*getsearchparams*/


boolean setsearchparams (void) {
	
	/*
	9/20/91 dmb: load up the database params table from  the searchparam 
	record.
	
	this call should be after paired with any call to setsearchparams so 
	that the search and replace strings are restored.
	*/
	
	if (!pushsearchparamstable ())
		return (false);
	
	setstringsearchparam (ixsearchfor, searchparams.bsorigfind);
	
	setstringsearchparam (ixreplacewith, searchparams.bsorigreplace);
	
	setbooleansearchparam (ixcasesensitive, !searchparams.flunicase);
	
	setbooleansearchparam (ixwholewords, searchparams.flwholewords);
	
	setbooleansearchparam (ixwraparound, searchparams.flwraparound);
	
	setbooleansearchparam (ixflatsearch, !searchparams.floneobject);
	
	setbooleansearchparam (ixfrontwindowonly, searchparams.flonelevel);
	
	setbooleansearchparam (ixclosewindows, searchparams.flclosebehind);
	
	setbooleansearchparam (ixregularexpression, searchparams.flregexp);
	
	pophashtable ();
	
	return (true);
	} /*resetsearchparams*/


boolean shellpushtargetglobals (void) {
	
	WindowPtr targetwindow;
	
	if (!langfindtargetwindow (-1, &targetwindow))  { /*should never fail, but...*/
		
		return (shellpushfrontrootglobals ()); /*was: (false)*/
		}
	
	return (shellpushglobals (targetwindow));
	} /*shellpushtargetglobals*/


static boolean doreplaceallfunc() {
	
	if (shellwindow != lastwindowsearched)
		endcurrentsearch ();
	
	if (!getsearchparams ())
		return (false);
	
	startnewsearch (false, true);
	
	if ((*shellglobals.gettargetdataroutine) (idwordprocessor)) {
		/*
		this bit of dirtyness is to replace the current text selection, if 
		there is one.  we shouldn't know about wp, but it's late in the game!
		*/
		
		bigstring bs;
		
		wpgetseltext (bs);
		
		if (!isemptystring (bs)) {
			
			wpinsert (searchparams.bsreplace);
			
			searchparams.ctreplaced = 1;
			}
		}
	
	(*shellglobals.searchroutine) ();
	
	setsearchparams ();
	
	return (true);
	} /*doreplaceallfunc*/


static boolean searchfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	bridges shell.c with the language.  the name of the verb is bs, its first parameter
	is hparam1, and we return a value in vreturned.
	
	we use a limited number of support routines from lang.c to get parameters and
	to return values. 
	
	return false only if the error is serious enough to halt the running of the script
	that called us, otherwise error values are returned through the valuerecord, which
	is available to the script.
	
	if we return false, we try to provide a descriptive error message in the 
	returned string bserror.
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	
	2003-05-18 AR: On Windows, handle custom Find & Replace dialogs with
	"Wrap around" and "Regular Expression" options.
	
	2003-05-24 AR: On Windows, ignore lang errors from getsearchparams
	in findtextdialogfunc and replacetextdialogfunc.
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl = false;
	
	if (v == nil) /*need Frontier process?*/
		return (true);
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	if (!shellpushtargetglobals ())
		return (false);
	
	switch (token) { /*these verbs assume that the frontmost window's globals are pushed*/
		
		case resetfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			endcurrentsearch ();
			
			fl = setbooleanvalue (true, v);
			
			break;
		
		case findnextfunc: {
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (shellwindow != lastwindowsearched)
				endcurrentsearch ();
			
			if (!getsearchparams ())
				break;
			
			startnewsearch (true, false);
			
			(*v).data.flvalue = (*shellglobals.searchroutine) ();
			
			langfindtargetwindow (-1, &lastwindowsearched); /*remember where we left off*/
			
			setsearchparams ();
			
			searchparams.flfirsttime = false;
			
			if (keyboardescape ()) {
				
				keyboardclearescape (); /*consume it*/
				
				//break; /*we'll return false to halt script execution*/
				}
			
			fl = true;
			
			break;
			}
		
		case replacefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (!(*shellglobals.gettargetdataroutine) (idwordprocessor)) {
				
				langerror (badreplacecontexterror);
				
				break;
				}
			
			if (!getsearchparams ())
				break; 
				
			(*v).data.flvalue = wpinsert (searchparams.bsreplace);
			
			setsearchparams ();
			
			fl = true;
		
			break;
		
		case replaceallfunc: {
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (!doreplaceallfunc())
				break;
			
			if (keyboardescape ()) {
				
				keyboardclearescape (); /*consume it*/
				
				//break; /*we'll return false to halt script execution*/
				}
			
			fl = setlongvalue (searchparams.ctreplaced, v);
			
			break;
			}

	#ifdef WIN95VERSION
		case findtextdialogfunc: /*2003-05-18 AR*/
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (shellwindow != lastwindowsearched)
				endcurrentsearch ();
			
			disablelangerror (); /*2003-05-24 AR*/
			
			getsearchparams ();
		
			enablelangerror (); /*2003-05-24 AR*/

			frstruct.lStructSize = sizeof (frstruct);
			frstruct.hwndOwner = shellframewindow;
			frstruct.Flags = 0;

			if (! searchparams.flunicase)
				frstruct.Flags |= FR_MATCHCASE;

			if (searchparams.flwholewords)
				frstruct.Flags |= FR_WHOLEWORD;

			frinfo.flwraparound = searchparams.flwraparound;
			frinfo.flregexp = searchparams.flregexp;
			frinfo.func = findtextdialogfunc;
			frstruct.lCustData = (LPARAM) &frinfo;

			copytodialog (searchparams.bsorigfind, windialogfind);

			frstruct.lpstrFindWhat = stringbaseaddress (windialogfind);
			frstruct.lpstrReplaceWith = NULL;
			frstruct.wFindWhatLen = sizeof(bigstring) - 2;
			frstruct.wReplaceWithLen = 0;

			frstruct.hInstance = hInst;
			frstruct.lpfnHook = &shellfindreplacehook; /*set wrap around and regexp checkbox on init*/
			frstruct.lpTemplateName = MAKEINTRESOURCE (IDD_CUSTOMFIND);

			frstruct.Flags |= FR_ENABLETEMPLATE | FR_ENABLEHOOK | FR_HIDEUPDOWN ;
			
			setsearchparams ();

			releasethreadglobals ();
			SendMessage (shellframewindow, wm_startsearch, 0, (LPARAM) &frstruct);
			grabthreadglobals ();

			fl = setbooleanvalue ((findreplacewindow != NULL), v);

			break;

		case replacetextdialogfunc: /*2003-05-18 AR*/
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (shellwindow != lastwindowsearched)
				endcurrentsearch ();
			
			disablelangerror (); /*2003-05-24 AR*/
			
			getsearchparams ();
		
			enablelangerror (); /*2003-05-24 AR*/
			
			frstruct.lStructSize = sizeof (frstruct);
			frstruct.hwndOwner = shellframewindow;
			frstruct.Flags = 0;

			if (! searchparams.flunicase)
				frstruct.Flags |= FR_MATCHCASE;

			if (searchparams.flwholewords)
				frstruct.Flags |= FR_WHOLEWORD;

			frinfo.flwraparound = searchparams.flwraparound;
			frinfo.flregexp = searchparams.flregexp;
			frinfo.func = replacetextdialogfunc;
			frstruct.lCustData = (LPARAM) &frinfo;

			copytodialog (searchparams.bsorigfind, windialogfind);
			copytodialog (searchparams.bsorigreplace, windialogreplace);

			frstruct.lpstrFindWhat = stringbaseaddress (windialogfind);
			frstruct.lpstrReplaceWith = stringbaseaddress (windialogreplace);
			frstruct.wFindWhatLen = sizeof(bigstring) - 2;
			frstruct.wReplaceWithLen = sizeof(bigstring) - 2;

			frstruct.hInstance = hInst;
			frstruct.lpfnHook = &shellfindreplacehook; /*set wrap around and regexp checkbox on init*/
			frstruct.lpTemplateName = MAKEINTRESOURCE (IDD_CUSTOMREPLACE);

			frstruct.Flags |= FR_ENABLETEMPLATE | FR_ENABLEHOOK | FR_HIDEUPDOWN ;
			
			setsearchparams ();

			releasethreadglobals ();
			SendMessage (shellframewindow, wm_startreplace, 0, (LPARAM) &frstruct);
			grabthreadglobals ();

			fl = setbooleanvalue ((findreplacewindow != NULL), v);
			
			break;
	#endif

		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			break;

		
		} /*switch -- funcs with front globals pushed*/
	
	shellpopglobals ();
	
	return (fl);
	} /*searchfunctionvalue*/


#ifdef WIN95VERSION


UINT_PTR CALLBACK shellfindreplacehook (HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
	
	/*
	2003-05-18 AR: Initialize custom "wrap around" and "regular expression" checkboxes
	*/
	
	switch (uiMsg) {
		
		case WM_INITDIALOG: {

			FINDREPLACE * fr = (FINDREPLACE *) lParam;
			tyfindreplaceinfo *fri = (tyfindreplaceinfo*)(fr->lCustData);
			
			CheckDlgButton (hdlg, chx3, (fri->flwraparound) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton (hdlg, chx4, (fri->flregexp) ? BST_CHECKED : BST_UNCHECKED);
			
			return (1); /*make sure the default proc makes the dialog appear*/
			}
		
		}/*switch*/
		
	return (0);
	} /*shellfindreplacehook*/
	
	
boolean shellprocessfindmessage (FINDREPLACE * fr) {
	
	/*
	5.0a21 dmb: push/pop root table for locating search params table

	5.0.2b20 dmb: report number of replacements made

	2003-05-18 AR: Don't save settings if the user hit the Cancel button.
	Get flags directly from the checkboxes.
	
	2003-05-19 AR: Modify user.prefs.search table directly instead of
	the searchparams global, just like the search.dialog script does
	on the Mac. This is neccessary so that getsearchparams can detect
	changes in the find and replace strings and update the internal
	regexp variables accordingly.
	*/

	WindowPtr targetwindow;
	bigstring bstemp;
	boolean fl = true;

	if (fr->Flags & FR_DIALOGTERM) { /*2003-05-18 AR: exit early, don't even grab globals*/
		
		findreplacewindow = NULL;
		
		return (true); /*return early, don't save settings to database*/
		}

	grabthreadglobals ();
	
	pushhashtable (roottable);
	
	if (!pushsearchparamstable ()) {
		
		pophashtable (); /*roottable*/
		
		releasethreadglobals ();

		return (false);
		}
		
	setstringlength (windialogfind, strlen(stringbaseaddress(windialogfind)));
	copyfromdialog (windialogfind, bstemp);
	setstringsearchparam (ixsearchfor, bstemp);
	
	setstringlength (windialogreplace, strlen(stringbaseaddress(windialogreplace)));
	copyfromdialog (windialogreplace, bstemp);
	setstringsearchparam (ixreplacewith, bstemp);
	
	setbooleansearchparam (ixwholewords, IsDlgButtonChecked (findreplacewindow, chx1) == BST_CHECKED);
	
	setbooleansearchparam (ixcasesensitive, IsDlgButtonChecked (findreplacewindow, chx2) == BST_CHECKED);
	
	setbooleansearchparam (ixwraparound, IsDlgButtonChecked (findreplacewindow, chx3) == BST_CHECKED);
	
	setbooleansearchparam (ixregularexpression, IsDlgButtonChecked (findreplacewindow, chx4) == BST_CHECKED);
	
	pophashtable (); /*searchparamstable*/
	
	pophashtable (); /*roottable*/
		
	if (!shellfindtargetwindow (-1, &targetwindow)) {
		
		releasethreadglobals ();

		return (false);
		}
	
	shellpushglobals (targetwindow);
	
	//	if (fr->Flags & FR_DIALOGTERM) {
	//		
	//		findreplacewindow = NULL;
	//		/* rab: should we do an endcurrentsearch here */
	//		/* dmb: I don't think so */
	//		
	//		goto exit;
	//		}

	if (fr->Flags & FR_FINDNEXT) {
		bigstring bsresult;
		
		langrunstring ("\x11" "search.findnext()", bsresult);
		
		if (equalstrings (bsresult, bstrue)) {
			
			if (((tyfindreplaceinfo*)(fr->lCustData))->func == findtextdialogfunc)
				PostMessage (findreplacewindow, WM_COMMAND, IDABORT, 0);
			}
		else
			sysbeep ();
		
		goto exit;
		}

	if (fr->Flags & FR_REPLACE) {
		bigstring bsresult;
		
		langrunstring ("\x3b" "if search.replace() {search.findnext()}else{speaker.beep()}", bsresult);
		
		goto exit;
		}

	if (fr->Flags & FR_REPLACEALL) {

		if (!doreplaceallfunc())
			goto exit; /*** FIXME: make sure the error will actually be reported! ***/
		
		if (searchparams.ctreplaced == 0)
			sysbeep ();
		else {
			bigstring bs;

			numbertostring (searchparams.ctreplaced, bs);

			pushstring ("\x13" " replacements made.", bs);

			notifyuser (bs);
			}

		goto exit;
		}
	
	fl = false;
	
	exit:

	shellpopglobals ();

	releasethreadglobals ();

	return (fl);
	} /*shellprocessfindmessage*/

#endif


static boolean filemenufunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	2/7/91 dmb: added parameters to new, open & save; added boolean to avoid 
	dialog in various shell routines.
	
	2/7/91 dmb: functions that change the current file (closeall, new, open, revert) have 
	to save & restore the language globals.  we don't want the language to have 
	to do this on every external call, so we do it selectively here
	
	10/3/91 dmb: split off fileMenu verbs from shell verbs
	
	10/23/91 dmb: must try to restore globals after revert, so new globals aren't 
	in effect when deferred disposal takes place.
	
	5/19/93 dmb: closefunc uses getfrontwindow, not shellwindow
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	
	3.0b15 dmb: quitfunc should just return false and let everything flow normally, 
	instead of yielding here. this could problems calling filemenu.quit from a 
	component menusharing (osa) client...  however it doesn't, so this change is 
	disabled for the time being.
	
	5.0d16 dmb: savefunc, newfunc need new (optional) parameters
	
	5.0d18 dmb: closefunc respects the target
	
	5.1.4 dmb: closefunc takes optional win parameter; savefunc checks hinfo for nil
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl;
	
	if (v == nil) /*need Frontier process?*/
		return (true);
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	switch (token) { /*these verbs don't need any special globals pushed*/
		
		case newfunc: {
			tyvaluerecord val;
			tyfilespec fs;
			tyexternalid id;
			tyvaluerecord vhidden;
			short ctconsumed = 0;
			short ctpositional;
			WindowPtr w;
			
			if (!getparamvalue (hparam1, ++ctconsumed, &val))
				return (false);
			
			flnextparamislast = true;
			
			ctpositional = ctconsumed;
			
			setbooleanvalue (false, &vhidden); //default value
			
			if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, str_hidden, &vhidden))
				return (false);
			
			disablelangerror ();
			
			fl = coercetoostype (&val);
			
			enablelangerror ();
			
			if (!fl) { // old style, new database
			
				if (!coercetofilespec (&val))
					return (false);
				
				fs = (**val.data.filespecvalue);
				
				langsaveglobals (); /*see comment at function head*/
				
				fl = shellnewfile (&fs, vhidden.data.flvalue, &w);
				
				if (!langrestoreglobals ())
					fl = false;
				
				if (!fl)
					return (false);
				}
			else {
				id = (tyexternalid) (langgetvaluetype (val.data.ostypevalue) - outlinevaluetype);
				
				if (!ccnewfilewindow (id, &w, vhidden.data.flvalue))
					return (false);
				}
			
			return (setwinvalue (w, v));
			}
		
		case openfunc: {
			tyfilespec fs;
			tyvaluerecord vhidden;
			short ctconsumed = 0;
			short ctpositional;
			WindowPtr w;
			
			if (!getfilespecvalue (hparam1, ++ctconsumed, &fs))
				return (false);
			
			flnextparamislast = true;
			
			ctpositional = ctconsumed;
			
			setbooleanvalue (false, &vhidden); //default value
			
			if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, str_hidden, &vhidden))
				return (false);
			
			langsaveglobals (); /*see comment at function head*/
			
			fl = shellopenfile (&fs, vhidden.data.flvalue, &w);
			
			if (!langrestoreglobals ())
				return (false);
			
			return (fl && setwinvalue (w, v));
			}
		
		case closefunc: { /*close the frontmost window*/
			hdlwindowinfo hinfo;
			WindowPtr targetwindow = nil;
			
			if (langgetparamcount (hparam1) == 0) { // old style, use the target
			
				if (!langfindtargetwindow (-1, &targetwindow))
					targetwindow = getfrontwindow ();
				}
			else {
				flnextparamislast = true;
				
				if (!getwinparam (hparam1, 1, &hinfo))
					break;
				
				if (hinfo != nil)
					targetwindow = (**hinfo).macwindow;
				}
			
			//langsaveglobals (); /*see comment at function head*/
			
			(*v).data.flvalue = (targetwindow != nil) && shellclose (targetwindow, false);
			
			//return (langrestoreglobals ());
			
			return (true);
			}
		
		case closeallfunc:
			if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
				return (false);
			
			langsaveglobals (); /*see comment at function head*/
			
			(*v).data.flvalue = shellcloseall (nil, false);
			
			return (langrestoreglobals ());
		
		case quitfunc:
			if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
				return (false);
			
			shellexitmaineventloop (); /*sets flag for next iteration*/
			
			#if 0  /*maybe try this out after 3.0*/
			
			return (false); /*kill the script*/
			
			#else
			
			processyield (); /*give it a chance to happen*/
			
			(*v).data.flvalue = false; /*we didn't quit -- we're got here, didn't we?*/
			
			return (true);
			
			#endif
		} /*switch*/
	
	if (!shellpushtargetglobals ())
		return (false);
	
	fl = false; /*default return value for this function*/
	
	switch (token) { /*these verbs assume that the frontmost window's globals are pushed*/
		
		case savefunc: { /*save the frontmost window*/
			hdlwindowinfo hinfo;
			
			if (langgetparamcount (hparam1) == 0) { // old style, save the root
			
				if (!ccfindrootwindow (&hinfo))
					break;
				}
			else {
				flnextparamislast = true;
				
				if (!getwinparam (hparam1, 1, &hinfo))
					break;
				
				if (hinfo == nil) {
					
					getstringlist (langerrorlist, badwindowerror, bserror);
					
					break;
					}
				}
			
			(*v).data.flvalue = shellsave ((**hinfo).macwindow);
			
			fl = true;
			
			break;
			}
		
		case savecopyfunc: {
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getfilespecvalue (hparam1, 1, &fs))
				break;
			
			(*v).data.flvalue = shellsaveas (shellwindow, &fs, false);
			
			fl = true;
			
			break;
			}
		
		case revertfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			langsaveglobals (); /*see comment at function head*/
			
			(*v).data.flvalue = shellrevert (shellwindow, false);
			
			fl = langrestoreglobals (); /*should return false; script can't continue after a revert*/
			
			break;
		
		case printfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = shellprint (shellwindow, false);
		
			fl = true;
			
			break;
		
		} /*switch -- funcs with front globals pushed*/
	
	shellpopglobals ();
	
	return (fl);
	} /*filemenufunctionvalue*/


static boolean editmenufunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	10/3/91 dmb: split off editMenu verbs from shell verbs
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl = false;
	
	if (v == nil) /*need Frontier process?*/
		return (true);
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	if (!shellpushtargetglobals ())
		return (false);
	
	switch (token) { /*these verbs assume that the frontmost window's globals are pushed*/
		
		case undofunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = shelleditcommand (undocommand);
			
			fl = true;
			
			break;
		
		case cutfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = shelleditcommand (cutcommand);
		
			fl = true;
			
			break;
			
		case copyfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = shelleditcommand (copycommand);
		
			fl = true;
			
			break;
			
		case pastefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = shelleditcommand (pastecommand);
		
			fl = true;
			
			break;
		
		case clearfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = shelleditcommand (clearcommand);
		
			fl = true;
			
			break;
		
		case selectallfunc: 
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = shelleditcommand (selectallcommand);
			
			fl = true;
			
			break;
		
		case fontfunc: {
			bigstring bs;
			short fontnumber;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs)) /*font name*/
				break;
			
			fontgetnumber (bs, &fontnumber);
			
			if (fontnumber > 0) {

				(**shellwindowinfo).selectioninfo.fontnum = fontnumber;
			
				(*v).data.flvalue = (*shellglobals.fontroutine) ();
				
				shellverbsetselectioninfo ();
				}
			
			fl = true;
			
			break;
			}
		
		case fontsizefunc: {
			short fontsize;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &fontsize))
				break;
			
			(**shellwindowinfo).selectioninfo.fontsize = fontsize;
			
			(*v).data.flvalue = (*shellglobals.sizeroutine) ();
			
			shellverbsetselectioninfo ();
			
			fl = true;
			
			break;
			}
		
		case plainfunc:
			fl = shellstyleverb (hparam1, normal, v);
			
			break;
		
		case boldfunc:
			fl = shellstyleverb (hparam1, bold, v);
			
			break;
		
		case italicfunc:
			fl = shellstyleverb (hparam1, italic, v);
			
			break;
		
		case underlinefunc:
			fl = shellstyleverb (hparam1, underline, v);
			
			break;
		
		case outlinefunc:
			fl = shellstyleverb (hparam1, outline, v);
			
			break;
		
		case shadowfunc:
			fl = shellstyleverb (hparam1, shadow, v);
			
			break;
		
		case getfontfunc: {
			bigstring bsfont;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			 
			shellsetselectioninfo ();
			
			fontgetname ((**shellwindowinfo).selectioninfo.fontnum, bsfont);
			
			fl = setstringvalue (bsfont, v);
			
			break;
			}
		
		case getfontsizefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			 
			shellsetselectioninfo ();
			
			fl = setintvalue ((**shellwindowinfo).selectioninfo.fontsize, v);
			
			break;
		
		
		} /*switch*/
	
	shellpopglobals ();
	
	return (fl);
	} /*editmenufunctionvalue*/



static boolean shellinitbuiltins (void) {
	
	if (!loadfunctionprocessor (idsearchverbs, &searchfunctionvalue))
		return (false);
	
	if (!loadfunctionprocessor (idfilemenuverbs, &filemenufunctionvalue))
		return (false);
	
	if (!loadfunctionprocessor (ideditmenuverbs, &editmenufunctionvalue))
		return (false);
	
	return (true);
	} /*shellinitbuiltins*/


boolean shellinitverbs (void) {
	
	/*
	if you just changed or added some definitions in shellinitbuiltins, call
	shellinstallbuiltins here.  rebuild, run the program, come back and change
	it to shellloadbuiltins, rebuild and go on...
	
	12/18/90 dmb: no longer save hash tables in program file, so we just 
	initialize the builtins directly.
	*/
	
	if (!sysinitverbs ())
		return (false);
	
	return (shellinitbuiltins ());
	} /*shellinitverbs*/




