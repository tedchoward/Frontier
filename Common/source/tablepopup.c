
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

#ifdef MACVERSION
#include <standard.h>
#endif

#ifdef WIN95VERSION
#include "standard.h"
#include "WinLand.h"
#endif

#include "cursor.h"
#include "dialogs.h"
#include "kb.h"
#include "memory.h"
#include "menu.h"
#include "ops.h"
#include "popup.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "threads.h"
#include "frontierwindows.h"
#include "shellhooks.h"
#include "shell.rsrc.h"
#include "langexternal.h"
#include "langinternal.h"
#include "windowlayout.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "claybrowser.h"
#include "iowaverbs.h" /*3/18/92 dmb*/



#define sortpopuplist 156

#define ctsorts 3

#define sortpopuptitlestring (ctsorts + 1)


#define kindpopuplist 157

#define ctkinds 25

#define kindpopuptitlestring (ctkinds + 1)


#define ixfirstefptype 16

#define ixbinarytype 24

#define firstefptype outlinevaluetype



typedef struct tykindmenuinfo {
	
	/*byte *bsitem; /*the text of the menu item*/
	
	tyvaluetype type; /*the associated value type*/
	} tykindmenuinfo;


static tykindmenuinfo kindmenuinfo [] = { /*menu contents, in menu order*/

	/*Boolean*/ booleanvaluetype, 		/*1*/
	
	/*Character*/ charvaluetype, 		/*2*/
	
	/*Number*/ longvaluetype, 			/*3*/
	
	/*Float*/ doublevaluetype,			/*4*/
	
	/*Date*/ datevaluetype, 			/*5*/
	
	/*Direction*/ directionvaluetype,	/*6*/
	
	/*String*/ stringvaluetype, 		/*7*/
	
	/*-*/ novaluetype,					/*8*/
	
	/*String4*/ ostypevaluetype,		/*9*/
	
	/*Enumerator*/ enumvaluetype,		/*10*/
	
	/*File Specifier*/ filespecvaluetype, /*11*/
	
	/*Alias*/ aliasvaluetype,			/*12*/
	
	/*Object Specifier*/ objspecvaluetype, /*13*/
	
	/*Address*/ addressvaluetype, 		/*14*/
	
	/*-*/ novaluetype,					/*15*/
	
	/*Table*/ tablevaluetype, 			/*16*/
	
	/*WP-Text*/ wordvaluetype, 			/*17*/
	
	/*Picture*/ pictvaluetype, 			/*18*/
	
	/*Outline*/ outlinevaluetype, 		/*19*/
	
	/*Script*/ scriptvaluetype, 		/*20*/
	
	/*MenuBar*/ menuvaluetype, 			/*21*/
	
	/*-*/ novaluetype,
	
	/*List*/ listvaluetype,				/*23*/
	
	/*Record*/ recordvaluetype,			/*24*/
	
	/*Binary*/ binaryvaluetype			/*25*/
	};

#define ctkindmenuitems 25

static boolean tablemapmenutovaluetype (short ixmenu, tyvaluetype *valuetype) {
	
	register tyvaluetype vt;
	
	if (ixmenu > ctkindmenuitems)
		return (false);
	
	vt = kindmenuinfo [ixmenu - 1].type;
	
	*valuetype = vt;
	
	return (vt != novaluetype);
	} /*tablemapmenutovaluetype*/


static boolean tablemapvaluetypetomenu (tyvaluerecord val, short *ixmenu) {
	
	register short ix;
	
	if (val.valuetype == externalvaluetype)
		val.valuetype = (tyvaluetype) (firstefptype + langexternalgettype (val));
	
	for (ix = 0; ix < ctkinds; ++ix) {
		
		if (kindmenuinfo [ix].type == val.valuetype) {
			
			*ixmenu = ix + 1;
			
			return (true);
			}
		}
	
	*ixmenu = 0;
	
	return (false);
	} /*tablemapvaluetypetomenu*/


static boolean coercionpossible (hdlhashtable ht, tyvaluerecord val1, tyvaluetype valuetype2) {
	
	/*
	return true if a variable of type valuetype1 could be coerced to type valuetype2.
	
	5/6/93 dmb: added support for new types, using brute force approach instead of 
	second-guessing the language

	5.0a16 dmb: allow coercion between string and anything
	*/
	
	register tyvaluetype vt1 = val1.valuetype;
	register tyvaluetype vt2 = valuetype2;
	boolean fl;
	
	if (vt1 == vt2) /*same type, coercion is unnecessary, but certainly possible!*/
		return (true);
	
	if (vt2 == binaryvaluetype) /*everything converts to binary*/
		return (true);
	
	if (vt2 == stringvaluetype) /*everything converts to string*/
		return (true);
	
	if (vt1 == externalvaluetype)
		return (firstefptype + langexternalgettype (val1) == vt2);
	
	if (vt1 == binaryvaluetype) {
		
		vt1 = langgetvaluetype (getbinarytypeid (val1.data.binaryvalue));
		
		if (vt1 >= firstefptype)
			return (vt1 == vt2);
		}

	if (vt1 == stringvaluetype) { /*special cases for converting from a string*/
		switch (vt2) {
			case wordvaluetype:
			case outlinevaluetype:
			case scriptvaluetype:
				return (true);
			}
		}
	
	shellpusherrorhook ((errorhookcallback) &falsenoop);
	
	disablelangerror ();
	
	tablepushcontext (ht, vt2);
	
	if (!copyvaluerecord (val1, &val1))
		fl = false;
	
	else {
		
		fl = coercevalue (&val1, vt2);
		
		cleartmpstack ();  /*disposevaluerecord (val1, true)*/
		}
	
	tablepopcontext (ht, vt2);
	
	enablelangerror ();
	
	shellpoperrorhook ();
	
	return (fl);
	
	} /*coercionpossible*/


static boolean getcoercionstring (tyvaluetype valuetype, bigstring bs) {
	
	if (valuetype == enumvaluetype)
		return (langgetmiscstring (enumstring, bs));
	
	return (langgettypestring (valuetype, bs));
	} /*getcoercionstring*/


static boolean tablefillkindpopup (hdlmenu hmenu, short *checkeditem) {
	
	tyvaluerecord val;
	register short i;
	register short ixmenu;
	register boolean flenabled;
	tyvaluetype valuetype;	
	hdlhashtable ht;
	bigstring bspath;
	bigstring bsitem;
	hdlhashnode hnode;
	
	tablegetcursorinfo (&ht, bspath, &val, &hnode);
	
	for (i = 0; i < ctkinds; i++) {
		
		ixmenu = i + 1;
		
		switch (val.valuetype) {
			
			case novaluetype:
				flenabled = i <= ixbinarytype;
				
				break;
			
			default:
				tablemapmenutovaluetype (ixmenu, &valuetype);
				
				flenabled = coercionpossible (ht, val, valuetype);
			}
		
		getstringlist (kindpopuplist, ixmenu, bsitem);
		
		if (!pushpopupitem (hmenu, bsitem, flenabled, 0))
			return (false);
		} /*for*/
	
	tablemapvaluetypetomenu (val, checkeditem);
	
	return (true); 
	} /*tablefillkindpopup*/


static boolean tablekindrecalc (tyvaluetype valuetype) {
	
	/*
	create a string that looks like "x.y = long (x.y)" and run it
	
	for external types, make the string look like "external ("efp", "new", @x.y)"
	
	12/13/90 dmb: external type coersion now relies on handers: "efp.new (@x.y)".
	
	1/25/91 dmb: always exit edit mode (not just for external types)
	
	2.1b2 dmb: establish context for recalc using new tablepush/popcontext
	
	5.1.5b12 dmb: don't use full path to item, just use name. tablepushcontext always
	sets our table now.
	*/
	
	register tyvaluetype vt = valuetype;
	tyvaluerecord val;
	tyexternalid externalid;
	hdlhashtable ht;
	bigstring bs;
	bigstring bspath;
	bigstring bsscript;
	byte bsefp [16];
	ptrstring pbsparse;
	boolean fl;
	hdlhashnode hnode;
	
	if (!tableexiteditmode ())
		return (false);
	
	if (vt >= firstefptype) { /*a virtual external value - convert to normal*/
		
		externalid = (tyexternalid) (vt - firstefptype);
		
		vt = externalvaluetype;
		}
	
	if (!tablegetcursorinfo (&ht, bs, &val, &hnode))
		return (false);
	
	if (val.valuetype == vt)
		return (true); /*no change*/
	
	//if (!langexternalgetquotedpath (ht, bs, bspath))
	//	return (false);
	copystring (bs, bspath);
	
	langexternalbracketname (bspath);
	
	if (vt == externalvaluetype) {
		
		if (val.valuetype == novaluetype) {
			
			pbsparse = (ptrstring) "\x12" "lang.new (^1, @^0)";
			
			langgettypestring ((tyvaluetype) (outlinevaluetype + externalid), bsefp);
			
			pushstring ("\x04" "type", bsefp);
			}
		else if (val.valuetype == stringvaluetype) {
			
			switch (externalid) {
				
				case idwordprocessor:
					copystring ((ptrstring) "\x0f" "wp.settext(x); ", bs);
					break;

				case idoutlineprocessor:
				case idscriptprocessor:
					copystring ((ptrstring) "\x37" "op.insert(x, down); op.firstsummit(); op.deleteline(); ", bs);
					break;
				
				default:
					return (false);
				}

			pbsparse = (ptrstring) "\x44" "local (x=^0); lang.new (^1, @^0); target.set (@^0); ^2target.clear()";
			
			langgettypestring ((tyvaluetype) (outlinevaluetype + externalid), bsefp);
			
			pushstring ("\x04" "type", bsefp);
			}
		else
			pbsparse = (ptrstring) "\x11" "unpack (@^0, @^0)";
		}
	else {
		
		if (val.valuetype == externalvaluetype) {
			
			if (vt == stringvaluetype)
				pbsparse = (ptrstring) "\x36" "local (x = string (^0)); table.moveAndRename (@x, @^0)";
			
			else {
				
				assert (vt == binaryvaluetype);
				
				pbsparse = (ptrstring) "\x0e" "pack (^0, @^0)";
				}
			}
		else {
			
			if (!getcoercionstring (vt, bs))
				return (false);
			
			pbsparse = (ptrstring) "\x0c" "^0 = ^2 (^0)";
			}
		}
	
	parsedialogstring (pbsparse, bspath, bsefp, bs, nil, bsscript);
	
	tablepushcontext (ht, vt);
	
	fl = langrunstring (bsscript, bs);
	
	tablepopcontext (ht, vt);
	
	return (fl);
	} /*tablekindrecalc*/


static boolean tablekindpopupselect (hdlmenu hmenu, short itemselected) {
	
	tyvaluetype valuetype;
	
	if (!tablemapmenutovaluetype (itemselected, &valuetype)) 
		return (false);
	
	return (tablekindrecalc (valuetype));
	} /*tablekindpopupselect*/

	
boolean tablekindpopuphit (Point pt) {
	
	/*
	5/6/93 dmb: added support for new types
	*/
	
	register hdltableformats hf = tableformatsdata;
	Rect r;
	
	r = (**hf).kindpopuprect;
	
	return (popupmenuhit (r, true, &tablefillkindpopup, &tablekindpopupselect));
	} /*tablekindpopuphit*/
	

void tableupdatekindpopup (void) {
	
	register hdltableformats hf = tableformatsdata;
	bigstring bs;
	
	getstringlist (kindpopuplist, kindpopuptitlestring, bs);
	
	drawpopup ((**hf).kindpopuprect, bs, true);
	} /*tableupdatekindpopup*/
	
	
static boolean tablefillsortpopup (hdlmenu hmenu, short *checkeditem) {
	
	hdlhashtable ht;
	bigstring bs;
	short i;
	boolean flenabled;
	short sortorder;
	
	tablegetcursorinfo (&ht, bs, nil, nil);
	
	flenabled = tablegetsortorder (ht, &sortorder);
	
	for (i = 1; i <= ctsorts; i++) {
		
		getstringlist (sortpopuplist, i, bs);
		
		if (!pushpopupitem (hmenu, bs, flenabled, 0))
			return (false);
		} /*for*/
	
	*checkeditem = 1; /*default, defensive driving*/
	
	switch (sortorder) {
		
		case sortbyname:
			*checkeditem = 1;
			
			break;
			
		case sortbyvalue:
			*checkeditem = 2;
			
			break;
			
		case sortbykind:
			*checkeditem = 3;
			
			break;
		} /*switch*/
		
	return (true); 
	} /*tablefillsortpopup*/


static boolean tablesortpopupselect (hdlmenu hmenu, short itemselected) {
	
	hdlhashtable ht;
	bigstring bs;
	
	register short sortorder;
	
	switch (itemselected) {
		
		case 1:
			sortorder = sortbyname;
			
			break;
			
		case 2:
			sortorder = sortbyvalue;
			
			break;
			
		case 3:
			sortorder = sortbykind;
			
			break;
		} /*switch*/
	
	tablegetcursorinfo (&ht, bs, nil, nil);
	
	tablesetsortorder (ht, sortorder);
	
	return (true);
	} /*tablesortpopupselect*/

	
boolean tablesortpopuphit (Point pt) {

	register hdltableformats hf = tableformatsdata;
	Rect r;
	
	r = (**hf).sortpopuprect;
	
	return (popupmenuhit (r, true, &tablefillsortpopup, &tablesortpopupselect));
	} /*tablesortpopuphit*/
	

void tableupdatesortpopup (void) {
	
	register hdltableformats hf = tableformatsdata;
	bigstring bs;
	
	getstringlist (sortpopuplist, sortpopuptitlestring, bs);
	
	drawpopup ((**hf).sortpopuprect, bs, true);
	} /*tableupdatesortpopup*/


boolean tablesetitemname (hdlhashtable ht, bigstring bsname, hdlheadrecord headnode, boolean flediting) {
	
	/*
	12/17/91 dmb: added name validation
	
	1/3/92 dmb: check for just case change (equalidentifiers), and don't 
	ignore hashsetnodekey result
	
	4.1b2 dmb: now resort, using the appropriate level call based on flediting
	
	5.0.1 dmb: special case for xml
	
	6.2b16 AR: Take headnode param instead of bsnew bigstring.
	*/
	
	hdlhashnode hnode;
	bigstring bskey, bsnew;
	boolean flvalidate = true;
	long newlength;
	
	assert (headnode != nil);
	
	if (!hashtablelookupnode (ht, bsname, &hnode)) /*internal error*/
		return (false);
	
	gethashkey (hnode, bskey);
	
	newlength = gethandlesize ((**headnode).headstring);
	
	if (newlength == 0) { /*don't permit empty name*/
		
		if (!isemptystring (bskey)) /*has a name -- leave it unchanged*/
			return (true);
		
		return (false);
		}
	
	if (flvalidate && newlength > 255) {
			
		alertstring (itemnametoolongstring);
		
		return (false);
		}
	
	opgetheadstring (headnode, bsnew);
	
	if (equalstrings (bsnew, bskey)) /*no change*/
		return (true);
	
	if (equalidentifiers (bsnew, bskey)) /*just a case change*/
		flvalidate = false;
	
	if (flvalidate) {
		
		#ifdef xmlfeatures
		if (!(**ht).flxml)
		#endif
			if (hashtablesymbolexists (ht,  bsnew)) {
				
				alertstring (itemnameinusestring);
				
				return (false);
				}
		}
	
	if (!hashsetnodekey (ht, hnode, bsnew))
		return (false);
	
	#ifdef xmlfeatures
	if (!(**ht).flxml)
	#endif
		
		if (flediting)
			hashresort (ht, hnode);
		else
			tableresort (ht, hnode);
	
	return (true);
	} /*tablesetitemname*/



typedef struct typopupkindinfo {
	bigstring bstitle;
	short zoomkind;
	boolean flkindok;
	} typopupkindinfo;


#ifdef MACVERSION
static typopupkindinfo *zoominfoptr;

static boolean tablekindhitroutine (DialogPtr pdialog, short itemhit) {
	
	switch (itemhit) {
		
		case -1: /*initialize*/
			setdialogradiovalue (pdialog, firstkinditem, lastkinditem, zoominfoptr->zoomkind);
			
			setdialogtext (pdialog, valuenameitem, zoominfoptr->bstitle);
			
			break;
		
		case okitem:
			zoominfoptr->zoomkind = getdialogradiovalue (pdialog, firstkinditem, lastkinditem);
			
			getdialogtext (pdialog, valuenameitem, zoominfoptr->bstitle);
			
			zoominfoptr->flkindok = true;
			
			return (false);
		
		case cancelitem:
			zoominfoptr->flkindok = false;
			
			return (false);
		
		case valuenameitem:
			break;
		
		default:
			setdialogradiovalue (pdialog, firstkinditem, lastkinditem, itemhit - firstkinditem);
			
			break;
		}

	return (true);
	} /*tablekindhitroutine*/
#endif

#ifdef WIN95VERSION
LRESULT CALLBACK tablekindDialogCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	
	typopupkindinfo * p;

	switch (message) {
		
		case WM_INITDIALOG:
			centerdialog (hwnd);

			p = (typopupkindinfo *) lParam;

			SetWindowLong (hwnd, DWL_USER, lParam); 

			CheckRadioButton (hwnd, IDC_RADIO1, IDC_RADIO6, IDC_RADIO1 + p->zoomkind);

			SendDlgItemMessage (hwnd, IDC_EDIT1, EM_SETLIMITTEXT, (WPARAM) sizeof(bigstring) - 2, 0);
			SetDlgItemText (hwnd, IDC_EDIT1, stringbaseaddress (p->bstitle));
			SendDlgItemMessage (hwnd, IDC_EDIT1, EM_SETSEL, (WPARAM) (INT) 0, (LPARAM) (INT) -1);

			SetFocus (GetDlgItem (hwnd, IDC_EDIT1));
			return (false);

		case WM_COMMAND:
			p = (typopupkindinfo *) GetWindowLong (hwnd, DWL_USER);

			switch (LOWORD(wParam))
				{
				case IDOK:
					GetDlgItemText (hwnd, IDC_EDIT1, stringbaseaddress(p->bstitle), sizeof(bigstring) - 2);
					setstringlength (p->bstitle, strlen(stringbaseaddress(p->bstitle)));

					p->flkindok = true;

				    EndDialog (hwnd, TRUE);
					return TRUE;

				case IDCANCEL:
					p->flkindok = false;

				    EndDialog (hwnd, FALSE);
				    return TRUE;

				case IDC_RADIO1:
				case IDC_RADIO2:
				case IDC_RADIO3:
				case IDC_RADIO4:
				case IDC_RADIO5:
				case IDC_RADIO6:
					if (HIWORD(wParam) == BN_CLICKED) {
						p->zoomkind = LOWORD(wParam) - IDC_RADIO1;
						}
					break;

				default:
					break;
				}

		default:
			break;
		}

	return FALSE;
	} /*tablekindDialogCallback*/
#endif

//RAB: 1/21/98 This dialog shows the user the current title and
//		allows that to be changed and to select a kind for the item
//		The bstitle parameter is an in/out parameter having the 
//		default string to show and returns with the users result.
//		The kind parameter is also an in/out for which button is
//		default and returns with the user value.
//		The procedure returns true if everything worked and the
//		user presses ok, otherwise it returns false.
static boolean showpopupkinddialog (typopupkindinfo * pki) {
#ifdef MACVERSION
	zoominfoptr = pki;

	if (!customdialog (newvaluedialogid, 1, &tablekindhitroutine))
		return (false);
	
	if (! pki->flkindok)
		return (false);

	return (true);
#endif

#ifdef WIN95VERSION
	int res;

	nullterminate (pki->bstitle);

	releasethreadglobals ();

	res = DialogBoxParam (hInst, MAKEINTRESOURCE (IDD_DIALOGTABLEENTRY), hwndMDIClient, (DLGPROC)tablekindDialogCallback, (LPARAM) pki);
	
	grabthreadglobals ();

	if (res > 0)
		return (true);
	
	return (false);
#endif
	} /*showpopupkinddialog*/


boolean tablepopupkinddialog (void) {
	
	typopupkindinfo pki;
	tyvaluerecord val;
	hdlhashtable ht;
	hdlhashnode hnode;
	
	tablegetcursorinfo (&ht, pki.bstitle, &val, &hnode);
	
	pki.zoomkind = 0;  //default to table

	if (! showpopupkinddialog (&pki))
		return (false);

	if (!opsetheadstring ((**outlinedata).hbarcursor, pki.bstitle))
		return (false);
	
	opvisibarcursor ();
	
	opupdatenow ();
	
	return (tablekindpopupselect (nil, (short)(pki.zoomkind + ixfirstefptype)));
	} /*tablepopupkinddialog*/


typedef struct tyancestor {
	
	hdlhashtable htable;
	
	hdlhashnode hnode;
	} tyancestor, tyancestry [];


typedef tyancestry *ptrancestry, **hdlancestry;


static hdlancestry hancestry;

static boolean flinhibitmenupush;

static hdlexternalvariable clientvariable;

static boolean flmenubarscript;


static boolean ancestorroutine (hdlhashtable htable, hdlhashnode hnode) {
	
	/*
	5.0.2b21 dmb: don't popup file windows table or its ancestors
	
	5.1.5 dmb: don't inhibit the root of guest databases
	*/
	
	tyancestor ancestor;
	
	if (flinhibitmenupush)
		return (true);
	
	ancestor.htable = htable;
	
	ancestor.hnode = hnode;
	
	if (!enlargehandle ((Handle) hancestry, sizeof (ancestor), &ancestor))
		return (false);
	
	if (htable == filewindowtable)
		flinhibitmenupush = true;
	
	return (true);
	} /*ancestorroutine*/


static boolean tablefilltitlepopup (hdlmenu hmenu, short *checkeditem) {
	
	hdlhashtable htable;
	bigstring bsname;
	register short i;
	short ctancestors;
	
	if (clientvariable != (hdlexternalvariable) rootvariable) {
		
		if (!findvariablesearch (roottable, clientvariable, true, &htable, bsname, &ancestorroutine))
			return (false);
		
		ctancestors = gethandlesize ((Handle) hancestry) / sizeof (tyancestor);
		
		for (i = 0; i < ctancestors; ++i) {
			
			gethashkey ((**hancestry) [i].hnode, bsname);
			
			if (!pushpopupitem (hmenu, bsname, true, 0))
				return (false);
			}
		}
	
	if (!flinhibitmenupush)
		if (!pushpopupitem (hmenu, nameroottable, true, 0))
			return (false);
	
	*checkeditem = -1; /*select, but don't check, the first item*/
	
	return (true); 
	} /*tablefilltitlepopup*/


static boolean tabletitlepopupselect (hdlmenu hmenu, short itemselected) {
	
	/*
	note: the last item, "root", is really just a dummy which has no matching element 
	in the array.  the first item in the array will actually go to the parent table, 
	which is the one named in the second menu item.
	
	8.0.4 Radio PBS: Fixed a crashing bug when no item is selected and itemselected is garbage.
	*/
	
	tyancestor ancestor;
	bigstring bsname;
	tyvaluerecord val;
	hdlwindowinfo hinfo;
		
	if (itemselected > 128) /*8.0.4 PBS: when no item is selected, itemselected is very large. Don't crash.*/
		return (true);

	if (itemselected == 1) {
		
		if (!flmenubarscript) /*it's the object itself; nothing to do*/
			return (true);
		
		val.valuetype = externalvaluetype;
		
		val.data.externalvalue = (Handle) clientvariable;
		
		if (!langexternalwindowopen (val, &hinfo))
			return (false);
		
		shellbringtofront (hinfo);
		}
	else {
		
		ancestor = (**hancestry) [itemselected - 2];
		
		gethashkey (ancestor.hnode, bsname);

		if (!tablezoomfromtable (ancestor.htable))
			return (false);
		
		if (!tablezoomtoname (ancestor.htable, bsname))
			return (false);
		}
	
	if (keyboardstatus.floptionkey)
		shellclosewindow (shellwindow);
	
	return (true);
	} /*tabletitlepopupselect*/


static void localtoscreenrect (WindowPtr w, Rect *r) {
	
	/*
	convert the rectangle inside w to global coordinates on the desktop.
	*/

#ifdef MACVERSION
	localtoglobalrect (w, r);
#endif

#ifdef WIN95VERSION
	POINT winpt;
	
	winpt.x = (*r).left;
	winpt.y = (*r).top;
	
	ClientToScreen (w, &winpt);
	
	offsetrect (r, (short)(winpt.x - (*r).left), (short) (winpt.y - (*r).top));
#endif
	} /*localtoscreenrect*/


boolean tableclienttitlepopuphit (Point pt, hdlexternalvariable hvariable) {
	
	/*
	4/15/92 dmb: build and track a popup in the object's title bar of its ancestry,
	much like the folder popup in the Finder.
	
	if something is selected, zoom to it.
	
	6/25/92 dmb: added flmenubarscript hack to quickly implement support for 
	-- you guessed it -- menubar scripts.
	*/
	
	boolean fl = false;
	
	Rect r;
	bigstring bs;
	short ctpixels;

#ifdef MACVERSION

	#if TARGET_API_MAC_CARBON != 1
	
		//Code change by Timothy Paustian Sunday, April 30, 2000 9:18:54 PM
		//Leave this till later, but I have a feeling these are going to be 
		//a real pain in Mac OS X
		//Friday, May 5, 2000 10:36:39 PM
		//I think I know how to handle this now.
		//Code change by Timothy Paustian Sunday, May 21, 2000 9:28:04 PM
		//I checked this out and it seems to work just fine.
		CGrafPtr deskport;
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		deskport = CreateNewPort();
		#else
		deskport = NewPtr(sizeof(CGrafPort));
		#endif
	#endif
#endif
	
	if (hvariable == nil) /*defensive driving*/
		return (false);
	
	getlocalwindowrect (shellwindow, &r);

	localtoscreenrect (shellwindow, &r);
	
	localtoglobalpoint (shellwindow, &pt);

#ifdef MACVERSION	
	#if TARGET_API_MAC_CARBON != 1
		if (!pushdesktopport (deskport)) /*failed to open up a port on the whole desktop*/
			return (false);
	#endif
#endif
	
	windowgettitle (shellwindow, bs);
	
	ctpixels = stringpixels (bs) + 10; /*five pixels of white space on each side*/
	
	r.bottom = r.top;
	
	r.top = r.bottom - doctitlebarheight;

#ifdef MACVERSION	
	r.left += (r.right - r.left - ctpixels) / 2;
#endif
	
	r.right = r.left + ctpixels;
	
#ifdef WIN95VERSION
	r.right += 24;  /* account for sysmenu and border */
#endif

	flmenubarscript = false;
	
	if (pt.v > r.bottom) { /*we're being called for a menubar script; must adjust*/
		
		pt.v -= 50;
		
		flmenubarscript = true;
		}
	
	if (!pointinrect (pt, r))
		goto exit;
	
	if (!newemptyhandle ((Handle *) &hancestry))
		goto exit;
	
	flinhibitmenupush = false;
	
	clientvariable = hvariable;
	
	pt.v = (short) r.top;

	globaltolocalpoint (shellwindow, &pt);

	r.left = pt.h - 12; /*position relative to cursor*/
	
	r.top = pt.v;

	fl = popupmenuhit (r, false, &tablefilltitlepopup, &tabletitlepopupselect);

	disposehandle ((Handle) hancestry);
	
	exit:

#ifdef MACVERSION
	#if TARGET_API_MAC_CARBON != 1

		popdesktopport (deskport);
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		 DisposePort(deskport);
		#else
		DisposePtr(deskport);
		#endif
	#endif
#endif

	return (fl);
	} /*tableclienttitlepopuphit*/




