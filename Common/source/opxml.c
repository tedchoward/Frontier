
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

/*
opxml.c -- Implementation of op<->xml verbs. OPML is Outline Processor Markup Language.

Spec: http://radio.userland.com/opmlspec.html

7.0b21 PBS 09/00
*/

#include "frontier.h"
#include "standard.h"

#include "op.h"
#include "opinternal.h"
#include "opxml.h"
#include "tableinternal.h"
#include "langxml.h"
#include "strings.h"
#include "lang.h"
#include "langinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "ops.h"


#define STR_quot			("\x06" "&quot;")
#define STR_quot_replace	("\x01" "\"")
#define STR_lt				("\x04" "&lt;")
#define STR_lt_replace		("\x01" "<")
#define STR_gt				("\x04" "&gt;") 
#define STR_gt_replace		("\x01" ">")
#define STR_amp				("\x05" "&amp;")
#define STR_amp_replace		("\x01" "&")
#define STR_attstablename	("\x05" "/atts")
#define STR_textitemname	("\x04" "text")
#define STR_windowtop		("\x09" "windowTop")
#define STR_windowleft		("\x0a" "windowLeft")
#define STR_windowbottom	("\x0c" "windowBottom")
#define STR_windowright		("\x0b" "windowRight")
#define STR_title			("\x05" "title")
#define STR_datecreated		("\x0b" "dateCreated")
#define STR_datemodified	("\x0c" "dateModified")
#define STR_ownername		("\x09" "ownerName")
#define STR_owneremail		("\x0a" "ownerEmail")
#define STR_expansionstate	("\x0e" "expansionState")
#define STR_vertscrollstate ("\x0f" "vertScrollState")
#define STR_outlinedocument ("\x0f" "outlineDocument")
#define STR_opmldocument	("\x04" "opml")
#define STR_head			("\x04" "head")
#define STR_openhead		("\x06" "<head>")
#define STR_closehead		("\x07" "</head>")
#define STR_body			("\x04" "body")
#define STR_xmlheader		("\x2b" "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>")
#define STR_docheader		("\x14" "<opml version=\"1.1\">")
#define STR_closedoc		("\x07" "</opml>")
#define STR_userprefs		("\x0a" "user.prefs")
#define STR_name			("\x04" "name")
#define STR_email			("\x0b" "mailaddress")
#define STR_outlinetag		("\x07" "outline")
#define	STR_closeoutline	("\x0a" "</outline>")
#define STR_openbody		("\x06" "<body>")
#define STR_closebody		("\x07" "</body>")
#define STR_openoutlinetext	("\x0f" "<outline text=\"")
#define STR_space			("\x01" " ")
#define STR_equalsquotes	("\x02" "=\"")
#define STR_iscomment		("\x09" "isComment")
#define STR_isbreakpoint	("\x0c" "isBreakpoint")
#define STR_booleantrue		("\x04" "true")
#define STR_xstruct			("\x07" "xstruct")

#define STR_version			("\x07" "version")
#define STR_cloud			("\x05" "cloud")
#define STR_opencloud		("\x06" "<cloud")
#define STR_domain			("\x06" "domain")
#define STR_port			("\x04" "port")
#define STR_path			("\x04" "path")
#define STR_regProcedure	("\x11" "registerProcedure")
#define STR_protocol		("\x08" "protocol")
#define STR_legalprotocols	("\x16" "xml-rpc|soap|http-post")
#define CH_legaldelim		('|')

#define STR_errornotopml	("\x41" "Can't convert to an outline because this is not an OPML document.")

#define STR_errornohead     ("\x42" "Can't convert to an outline because the <head> section is missing.")

#define STR_errornobody     ("\x4b" "Can't convert to an outline because the <body> section is missing or empty.")

#define STR_errornooutlines ("\x54" "Can't convert to an outline because the <body> section contains no outline elements.")



/*Prototypes*/

static boolean opxmltooutlinevisit (hdlhashtable ht, short ixlevel, bigstring bsname, boolean flfirstline);

static boolean opxmlgetheadlinetextfromatts (hdlhashtable ht, Handle htext);

static boolean opxmlsetwindowatts (hdlhashtable ht, hdloutlinerecord ho, hdlhashtable hto, bigstring bso, tyvaluerecord vo);

static boolean opxmlsetwindowpositionandsize (hdlhashtable ht, hdlwindowinfo hinfo);

static boolean opxmlfindtableitem (hdlhashtable ht, bigstring bsname, hdlhashtable *htfound);

static boolean opxmlgetonelongvalue (hdlhashtable ht, bigstring bsname, long *v);

//static boolean opxmlpushxmlheader (Handle htext, short indentlevel);

static boolean opxmlencodetext (Handle htext);

static boolean opxmlpushstringline (Handle htext, bigstring bs, short indentlevel);

static boolean opxmlbuildhead (Handle htext, hdloutlinerecord ho, Handle hname, Handle hemail, short indentlevel, hdlwindowinfo hinfo, hdlhashtable hcloud);

static boolean opxmlpushhandleline (Handle htext, Handle h, short indentlevel);

static boolean opxmlgetwindowexpansionstate (hdloutlinerecord ho, Handle *hstate);

static boolean opxmlbuildbody (hdloutlinerecord ho, Handle htext, short indentlevel);

static boolean opxmlensureopenwindow (hdloutlinerecord ho, hdlhashtable hto, bigstring bso, tyvaluerecord vo, hdlwindowinfo *hinfo, boolean *flwasopen);

static boolean opxmlpushindents (Handle htext, short indentlevel);

static boolean opxmlheadexists (hdlhashtable ht);

static boolean opxmlrecursivelyvisit (hdlheadrecord h, short lev, opvisitcallback visit, ptrvoid refcon);

static boolean opxmlvisitnondynamicnodes (opvisitcallback visit, ptrvoid refcon);

static boolean opxmlgetcloudatts (hdlhashtable hhead, hdlhashtable hcloudspec);

static boolean opxmlpushoneattribute (Handle htext, bigstring bsname, Handle hvalue);

static boolean opxmlgetattribute (hdlhashtable ht, bigstring bsname, tyvaluetype type, tyvaluerecord *val);


/*Functions*/

boolean opoutlinetoxml (hdloutlinerecord ho, Handle hname, Handle hemail, Handle *htext, 
						hdlhashtable hto, bigstring bso, tyvaluerecord vo, hdlhashtable hcloud) {
	
	/*
	7.0b21 PBS: convert an outline to XML text.
	
	7.1b42 dmb: added hcloud parameter

	Note: it might be wise to use handlestreams at some point. Since I don't know how they work yet,
	I'm sticking with the tried-and-true.
	
	Caller is responsible for disposing htext.
	*/
	
	short indentlevel = 0;
	hdlwindowinfo hinfo;
	boolean flwindowwasopen = false;
	boolean fl = false;

	if (!newemptyhandle (htext))
		goto exit;
	
	if (!opxmlpushstringline (*htext, STR_xmlheader, indentlevel)) /*Push <?xml...>*/
		goto exit;
	
	if (!opxmlpushstringline (*htext, STR_docheader, indentlevel)) /*Push <opml...>*/
		goto exit;
		
	indentlevel++;
	
	if (!opxmlensureopenwindow (ho, hto, bso, vo, &hinfo, &flwindowwasopen)) /*Make sure op window is open.*/
		goto exit;

	if (!opxmlbuildhead (*htext, ho, hname, hemail, indentlevel, hinfo, hcloud)) /*Build <head> section.*/
		goto exit;
	
	if (!opxmlbuildbody (ho, *htext, indentlevel)) /*Build <body> section.*/
		goto exit;

	if (!flwindowwasopen) /*Close window if necessary*/
		shellclosewindow ((**hinfo).macwindow);
	
	if (!opxmlpushstringline (*htext, STR_closedoc, indentlevel)) /*Push </opml>*/
		goto exit;
	
	fl = true; /*success*/
	
	exit:
	
	return (fl);
	} /*opoutlinetoxml*/


boolean opxmltooutline (Handle htext, hdloutlinerecord ho, boolean flnewoutline, 
						hdlhashtable hto, bigstring bso, tyvaluerecord vo, hdlhashtable hcloud) {
	
	/*
	7.0b21 PBS -- convert XML text to an outline.



	7.0b33 PBS: changes to conform to changing spec. Both <head> and <body> are required. <head> may be empty.

	7.1b42 dmb: added hcloud parameter

	<body> must contain one or more <outline> elements.
	*/
	
	hdlhashtable ht, htfirst, htopmldocument, htbody, hthead;
	hdlhashnode hn;
	xmladdress xstruct;
	boolean flheadfound = false;
	boolean fl = false;
	boolean florigdisplayenabled = true;
	boolean flcanhavecloud = true;
	tyvaluerecord v, vvers;
	bigstring bsvers;
	
	/*New local table*/
	
	if (!newhashtable (&ht))
		goto exit1;	
	
	xstruct.ht = ht;
	
	copystring (STR_xstruct, xstruct.bs);
	
	/*Compile text into XML table*/
	
	if (!xmlcompile (htext, &xstruct))
		goto exit1;
		
	hn = (**xstruct.ht).hfirstsort; /*Get first item.*/
	
	if (!gethashnodetable (hn, &htfirst))
		goto exit2;

	/*Get <opml> item*/
	
	if (!opxmlfindtableitem (htfirst, STR_opmldocument, &htopmldocument)) { /*Find <opml> item*/
		
		if (!opxmlfindtableitem (htfirst, STR_outlinedocument, &htopmldocument)) { /*<outlineDocument> grandfathered in*/
			
			langerrormessage (STR_errornotopml);
			
			goto exit2;
			} /*if*/

		} /*if*/
	
	// Get the version number and test for 1.1
	if (!opxmlgetattribute (htopmldocument, STR_version, stringvaluetype, &vvers))
		goto exit2;

	pullstringvalue (&vvers, bsvers);

	dateversionlessthan (bsvers, "\x03" "1.1", &v);

	flcanhavecloud = !v.data.flvalue;

	/*Get <body> item and convert it to an outline.*/
	
	if (!opxmlfindtableitem (htopmldocument, STR_body, &htbody)) { /*<body> is not optional*/

		langerrormessage (STR_errornobody);

		goto exit2;
		} /*if*/
	
	oppushoutline (ho);

	if (!flnewoutline) { /*7.1b12 PBS: Inhibit display*/

		florigdisplayenabled = (**ho).flinhibitdisplay;
		
		(**ho).flinhibitdisplay = true;
		}

	if (!opxmltooutlinevisit (htbody, 0, STR_body, true)) { /*Convert XML to outline -- recursive routine*/

		langerrormessage (STR_errornooutlines);

		oppopoutline ();

		goto exit2;

		} /*if*/

	if (!flnewoutline) { /*7.1b12 PBS: Restore display setting; update window*/
		
		hdlwindowinfo hinfo;

		(**ho).flinhibitdisplay = florigdisplayenabled;
		
		if (shellfinddatawindow ((Handle) ho, &hinfo)) { /*get window info*/

			shellupdatenow ((**hinfo).macwindow);
			} /*if*/
		} /*if*/

	
	oppopoutline ();
		
	/*Get <head> item and set the window attributes if this is a new outline.*/
	
//	head:
	
	disablelangerror ();

	flheadfound = opxmlfindtableitem (htopmldocument, STR_head, &hthead);

	if (flnewoutline && flheadfound) {
			
		oppushoutline (ho);
		
		opxmlsetwindowatts (hthead, ho, hto, bso, vo); /*set window attributes*/
		
		oppopoutline ();
		} /*if*/

	enablelangerror ();

	if (!flheadfound) { /*7.0b33 PBS: make sure there's a head section of the document.*/

		assert (!opxmlheadexists (htopmldocument));

		langerrormessage (STR_errornohead);

		goto exit2;
		} /*if*/

	if (hcloud != nil && flcanhavecloud) { /*7.1b43 dmb: load the cloud elements into the table*/

		if (!opxmlgetcloudatts (hthead, hcloud))
			goto exit2;
		}
	
	fl = true;
	
	exit2:
	
	disposehashtable (ht, false);
	
	exit1:
	
	return (fl);
	} /*opxmltooutline*/


static boolean opxmlgetwindowscrollstate (hdloutlinerecord ho, long *ix) {
	
	/*
	7.0b21 PBS: get the scroll state of an outline.
	*/
	
	tyvaluerecord val;
	boolean fl = false;
	
	oppushoutline (ho);
	
	if (!opgetscrollstateverb (&val)) /*Call the standard verb for getting the scroll state.*/
		goto exit;
	
	fl = true;
	
	*ix = val.data.longvalue;
	
	exit:
	
	oppopoutline ();
	
	disposevaluerecord (val, false);
	
	return (fl);
	} /*opxmlgetwindowscrollstate*/


static boolean opxmlgetwindowexpansionstate (hdloutlinerecord ho, Handle *hstate) {
	
	/*
	7.0b21 PBS: get the expansion state.
	*/
	
	tyvaluerecord val;
	boolean fl = false;
	char p [1];
	
	oppushoutline (ho);
	
	if (!opgetexpansionstateverb (&val)) /*Call the standard verb for getting the expansion state.*/
		goto exit;
	
	if (!coercevalue (&val, stringvaluetype)) /*Verb returns a list -- but a string is desired.*/
		goto exit;
	
	if (!copyhandle (val.data.stringvalue, hstate))
		goto exit;
	
	/*Remove braces from beginning and end of string.*/
	
	pullfromhandle (*hstate, gethandlesize (*hstate) - 1, 1, p); /*pop trailing brace*/
	
	pullfromhandle (*hstate, 0, 1, p); /*pop leading brace*/
	
	fl = true;
	
	exit:
	
	disposevaluerecord (val, false);
	
	oppopoutline ();
	
	return (fl);
	} /*opxmlgetwindowexpansionstate*/


static boolean opxmlbuildtaggedstring (bigstring bstag, Handle hvalue, Handle hdest) {
	
	/*
	7.0b21 PBS: build a string like <tag>value</tag>.
	*/
	
	boolean fl = false;
	
	if (!pushtexthandle ("\x01""<", hdest)) /*<*/
		goto exit;
		
	if (!pushtexthandle (bstag, hdest)) /*<tag*/
		goto exit;
		
	if (!pushtexthandle ("\x01"">", hdest)) /*<tag>*/
		goto exit;
		
	if (!pushhandle (hvalue, hdest)) /*<tag>value*/
		goto exit;
		
	if (!pushtexthandle ("\x02""</", hdest)) /*<tag>value</*/
		goto exit;
		
	if (!pushtexthandle (bstag, hdest)) /*<tag>value</tag*/
		goto exit;
		
	if (!pushtexthandle ("\x01"">", hdest)) /*<tag>value</tag>*/
		goto exit;
		
	fl = true; /*success*/
	
	exit:
	
	return (fl);
	} /*opxmlbuildtaggedstring*/


static boolean opxmlpushonetag (bigstring bstag, Handle hvalue, short indentlevel, Handle htext) {

	/*
	7.0b21 PBS: Take <tag> and a value and push it on the text we're building.
	*/
	
	Handle hdest;
	boolean fl = false;
	
	if (!newemptyhandle (&hdest))
		return (false);
	
	if (!opxmlbuildtaggedstring (bstag, hvalue, hdest))
		goto exit;

	opxmlpushhandleline (htext, hdest, indentlevel);

	fl = true;
	
	exit:
	
	disposehandle (hdest);
	
	return (fl);
	} /*opxmlpushonetag*/


static boolean opxmlpushonetaglong (bigstring bstag, long v, short indentlevel, Handle htext) {
	
	/*
	7.0b21 PBS: push a tag line with a number value.
	*/
	
	Handle h;
	bigstring bs;
	boolean fl = false;
	
	numbertostring (v, bs);

	if (!newemptyhandle (&h))
		return (false);
	
	if (!pushtexthandle (bs, h))
		goto exit;
	
	fl = opxmlpushonetag (bstag, h, indentlevel, htext);
	
	exit:
	
	disposehandle (h);
	
	return (fl);
	} /*opxmlpushonetaglong*/


static boolean teststringvalue (const tyvaluerecord *val, bigstring legalvalues) {

	/*
	7.1b43 dmb: validate the string value against the list provided
	*/
	
	bigstring bsval;
	bigstring bslegal;
	int i;

	if (legalvalues == nil)
		return (true);
	
	pullstringvalue (val, bsval);

	for (i = 0; i < countwords (legalvalues, CH_legaldelim); i++ ) {

		nthword (legalvalues, i, CH_legaldelim, bslegal);

		if (equalstrings (bslegal, bsval))
			return (true);
		}

	return (false);
	} /*teststringvalue*/


static boolean opxmlpushcloudattribute (Handle htag, hdlhashtable hcloud, bigstring bsatt, bigstring legalvalues) {

	/*
	7.1b43 dmb: push the named attribute into the cloud tag, getting the value from the hcloud table
				if legalvalues is not nil, it's a |-delimited list of legal string values for this attribute
	*/
	
	tyvaluerecord v;
	hdlhashnode hn;

	if (!hashtablelookup (hcloud, bsatt, &v, &hn) ||
			!copyvaluerecord (v, &v) ||
			!coercetostring (&v) ||
			!teststringvalue (&v, legalvalues)) {

		langparamerror (cloudspecerror, bsatt);

		return (false);
		}

	if (!opxmlpushoneattribute (htag, bsatt, v.data.stringvalue)) //Add att to returned text.
		return (false);

	disposevaluerecord (v, false);

	return (true);
	} /*opxmlpushcloudattribute*/


static boolean opxmlpushcloudtag (hdlhashtable hcloud, short indentlevel, Handle htext) {
	
	/*
	7.1b43 dmb: push a cloud tag with attributes coming from hcloud table values
	*/
	
	Handle hcloudtag;
	boolean fl = false;
	
	if (!newtexthandle (STR_opencloud, &hcloudtag))
		return (false);
	
	if (!opxmlpushcloudattribute (hcloudtag, hcloud, STR_domain, nil))
		goto exit;

	if (!opxmlpushcloudattribute (hcloudtag, hcloud, STR_port, nil))
		goto exit;

	if (!opxmlpushcloudattribute (hcloudtag, hcloud, STR_path, nil))
		goto exit;

	if (!opxmlpushcloudattribute (hcloudtag, hcloud, STR_regProcedure, nil))
		goto exit;

	if (!opxmlpushcloudattribute (hcloudtag, hcloud, STR_protocol, STR_legalprotocols))
		goto exit;

	if (!pushtexthandle ("\x02" "/>", hcloudtag))
		goto exit;
	
	fl = opxmlpushhandleline (htext, hcloudtag, indentlevel);
	
	exit:
	
	disposehandle (hcloudtag);
	
	return (fl);
	} /*opxmlpushcloudtag*/


static boolean opxmlpushstringindented (bigstring bs, Handle htext, short indentlevel) {
	
	/*
	7.0b21 PBS: push one indent string plus a string on the returned text.
	*/
	
	if (!opxmlpushindents (htext, indentlevel)) /*Push indent tabs*/
	
		return (false);
	
	return (pushtexthandle (bs, htext)); /*Push the string on the handle*/
	} /*opxmlpushstringindented*/


static boolean opxmlpushoutlinetext (Handle headlinetext, Handle htext, short indentlevel) {
	
	/*
	7.0b21 PBS: push \t\t\t<outline text=" on the handle.
	*/
	
	Handle h;
	boolean fl = false;
	
	if (!newemptyhandle (&h))
		return (false);
	
	if (!opxmlpushstringindented (STR_openoutlinetext, htext, indentlevel)) /*Push <outline text="*/
		goto exit;
	
	if (!pushhandle (headlinetext, h)) /*Make a copy.*/
		goto exit;
	
	if (!opxmlencodetext (h)) /*Encode the text, make it legal for XML.*/
		goto exit;
		
	if (!pushhandle (h, htext)) /*push encoded headline text on the handle*/
		goto exit;
		
	fl = pushtexthandle (STR_quot_replace, htext); /*Push closing quotation mark.*/
	
	exit:
	
	disposehandle (h);
	
	return (fl);	
	} /*opxmlpushoutlinetext*/


static boolean opxmlpushclosingoutlinetags (Handle htext, short currlevel, short destlevel) {
	
	/*
	7.0b21 PBS: push closing </outline> tags until we're at the right level.
	*/
	
	short ixlevel = currlevel;
	
	while (ixlevel > destlevel) { /*Add closing outline tags until we're at the next level.*/
			
		if (!opxmlpushstringline (htext, STR_closeoutline, ixlevel))
			return (false); /*error of some kind, return early*/
			
		ixlevel--;			
		} /*while*/

	return (true);
	} /*opxmlpushclosingoutlinetags*/


static boolean opxmlpushoneattribute (Handle htext, bigstring bsname, Handle hvalue) {
	
	/*
	7.0b21 PBS: Push one attribute onto the returned text.
	*/
	
	Handle h;
	boolean fl = false;
	
	if (!newemptyhandle (&h))
		return (false);
	
	if (!pushhandle (hvalue, h)) /*copy*/
		goto exit;
		
	if (!opxmlencodetext (h)) /*Encode special characters.*/
		goto exit;
		
	if (!pushtexthandle (STR_space, htext)) /*add a space character*/
		goto exit;
		
	if (!pushtexthandle (bsname, htext)) /*add the attribute name*/
		goto exit;
		
	if (!pushtexthandle (STR_equalsquotes, htext)) /*add ="*/
		goto exit;
		
	if (!pushhandle (h, htext)) /*add the value*/
		goto exit;
		
	fl = pushtexthandle (STR_quot_replace, htext); /*add "*/
		
	exit:
	
	disposehandle (h);
	
	return (fl);
	} /*opxmlpushoneattribute*/


static boolean opxmlpushattributes (hdlheadrecord hnode, Handle htext, short indentlevel) {
	
	/*
	7.0b21 PBS: push the attributes onto the current <outline> item.
	*/
	
	tyvaluerecord v;
	hdlhashtable ht;
	hdlhashnode hn = nil, hn2 = nil;
	boolean fl = false;
	boolean flhasatts = false;
	Handle htrue;
	
	/*Possibly set isComment and isBreakpoint.*/
	
	if (!newemptyhandle (&htrue))
		return (false);
	
	if (!pushtexthandle (STR_booleantrue, htrue))
		goto exit;
	
	if ((**hnode).flcomment)
	
		if (!opxmlpushoneattribute (htext, STR_iscomment, htrue)) /*isComment="true"*/
			goto exit;
	
	if ((**hnode).flbreakpoint)
	
		if (!opxmlpushoneattribute (htext, STR_isbreakpoint, htrue)) /*isBreakpoint="true"*/
			goto exit;
	
	/*Get other attributes from the refcon -- the packed table.*/
	
	disablelangerror ();
	
	flhasatts = opattributesgetpackedtablevalue (hnode, &v);
	
	enablelangerror ();
	
	if (!flhasatts) {
		
		fl = true;
		
		goto exit; /*no atts -- not an error*/
		} /*if*/
	
	if (!tablevaltotable (v, &ht, hn))
		goto exit; /*Error.*/
	
	/*Loop through the table*/
	
	for (hn2 = (**ht).hfirstsort; hn2 != nil; hn2 = (**hn2).sortedlink) { 
		
		bigstring bsname;

		if (coercevalue (&((**hn2).val), stringvaluetype)) { /*Does the coercion to string succeed?*/
		
			gethashkey (hn2, bsname); /*Get att name*/
			
			if (!opxmlpushoneattribute (htext, bsname, (**hn2).val.data.stringvalue)) /*Add att to returned text.*/
				goto exit;	
			} /*if*/
		} /*for*/
	
	fl = true; /*success*/
	
	exit:
	
	disposehandle (htrue);
	
	if (flhasatts)
		disposevaluerecord (v, false);
	
	return (fl);
	} /*opxmlpushattributes*/


static boolean opxmlrecursivelyvisit (hdlheadrecord h, short lev, opvisitcallback visit, ptrvoid refcon) {

	

	register hdlheadrecord nomad, nextnomad;

	

	if (h == nil)

		return (true);

	

	nomad = (**h).headlinkright;

	

	if (nomad == h) /*nothing to the right*/

		return (true);

	

	while (true) {

		

		nextnomad = (**nomad).headlinkdown;

		

		if (!(*visit) (nomad, refcon))

			return (false);

			

		if (lev > 1) {



			if (!(**nomad).fldynamic) {

		

				if (!oprecursivelyvisit (nomad, lev - 1, visit, refcon))

					return (false);

				} /*if*/

			}

			

		if (nextnomad == nomad) /*just processed last subhead*/

			return (true);

			

		nomad = nextnomad;

		} /*while*/

	} /*opxmlrecursivelyvisit*/





static boolean opxmlvisitnondynamicnodes (opvisitcallback visit, ptrvoid refcon) {

	

	/*

	visit every node in the outline -- unless it's the child of a dynamic node.

	*/

	

	hdlheadrecord nomad = (**outlinedata).hsummit, nextnomad;

	

	while (true) {

		

		nextnomad = (**nomad).headlinkdown;

		

		if (!(*visit) (nomad, refcon)) /*visit the summit*/

			return (false);



		if (!(**nomad).fldynamic) {

			

			if (!opxmlrecursivelyvisit (nomad, infinity, visit, refcon)) /*visit its subs*/

				return (false);

			} /*if*/

		

		if (nextnomad == nomad)

			return (true);

			

		nomad = nextnomad;

		} /*while*/

	

	} /*opxmlvisitnondynamicnodes*/





static boolean opxmlbodyvisit (hdlheadrecord hnode, ptrvoid htext) {
	
	/*
	7.0b21 PBS: process one headline, add it to the returned text.
	
	Look ahead to the next flatdown headline, and add a /> if no subs.
	If next headline is at a higher level, close off <outline> elements,
	as many as needed.



	7.0b30 PBS: handle a dynamic node -- it has subs, but pretend it doesn't,

	since the children should not be saved.
	*/
	
	short indentlevel, nextlevel;
	Handle h;
	hdlheadrecord hflatdown;
	boolean fl = false;

	boolean flsubs = false;
	
	if (!copyhandle ((**hnode).headstring, &h))
		return (false);
	
	indentlevel = (**hnode).headlevel + 2;
	
	if (!opxmlpushoutlinetext (h, htext, indentlevel))
		goto exit;
	
	if (!opxmlpushattributes (hnode, htext, indentlevel))
		goto exit;
	
	hflatdown = opbumpflatdown (hnode, false);
	
	nextlevel = (**hflatdown).headlevel + 2;
	
	if (nextlevel == indentlevel)
	
		fl = true;



	if (nextlevel > indentlevel)



		flsubs = true;



	if ((**hnode).fldynamic) /*7.0b30: dynamic nodes may have subs, but we ignore them.*/



		flsubs = false;
	
	if (flsubs) /*Has subs?*/
		
		fl = pushtexthandle ("\x02"">\r", htext); /*Add closing > and carriage return.*/
	
	else { /*No subs*/
	
		if (!pushtexthandle ("\x03""/>\r", htext)) /*Add closing /> and carriage return.*/
			goto exit;
		
		fl = true;
		} /*if*/
	
	if (nextlevel < indentlevel) /*Need to close off outline items?*/
	
		fl = opxmlpushclosingoutlinetags (htext, indentlevel, nextlevel); /*Add as many </outline> tags as needed.*/
	
	if (hnode == hflatdown) /*At the end of the outline?*/
		
		fl = opxmlpushclosingoutlinetags (htext, indentlevel, 2); /*Add as many </outline> tags as needed.*/
	
	exit:
	
	disposehandle (h);
	
	return (fl);
	} /*opxmlbodyvisit*/


static boolean opxmlbuildbody (hdloutlinerecord ho, Handle htext, short indentlevel) {
	
	/*
	7.0b21 PBS: Build the <body> section.



	7.0b30 PBS: don't save children of dynamic headlines.
	*/
	
	boolean fl = false;
	
	if (!opxmlpushstringline (htext, STR_openbody, indentlevel)) /*<body>*/
		goto exit;

	indentlevel++;
	
	oppushoutline (ho);
	
	if (!opxmlvisitnondynamicnodes (&opxmlbodyvisit, htext)) /*visit every headline, top to bottom, calling opxmlbodyvisit on each*/
		goto exit;
	
	oppopoutline ();
	
	fl = opxmlpushstringline (htext, STR_closebody, indentlevel); /*</body>*/

	exit:
	
	return (fl);
	} /*opxmlbuildbody*/


static boolean opxmlensureopenwindow (hdloutlinerecord ho, hdlhashtable hto, bigstring bso, tyvaluerecord vo, hdlwindowinfo *hinfo, boolean *flwasopen) {
	
	/*
	7.0b21 PBS: make sure the window is open. *hinfo gets the window info.
	*/
	
	*flwasopen = true;
	
	if (!shellfinddatawindow ((Handle) ho, hinfo)) { /*try to find window -- it might be open*/
		
		*flwasopen = false;
		
		if (!langzoomvalwindow  (hto, bso, vo, false)) /*open window*/
			return (false);
		} /*if*/
		
	if (!shellfinddatawindow ((Handle) ho, hinfo)) /*get window info*/
		return (false);

	return (true);
	} /*opxmlensureopenwindow*/


static boolean opxmlbuildhead (Handle htext, hdloutlinerecord ho, Handle hname, Handle hemail, short indentlevel, hdlwindowinfo hinfo, hdlhashtable hcloud) {
	
	/*
	7.0b21 PBS: build the <head> section of the document.
	
	First get info about the window -- title, rect, etc. -- then push the tagged values on the returned text.
	*/
	
	long scrollstate;
	Handle hexpansionstate;
	Rect r;
	boolean fl = false;
	unsigned long timecreated, timemodified;
	tyvaluerecord vtimecreated, vtimemodified;
	bigstring bstitle;
	Handle htitle;
	
	/*Get window info.*/
	
	if (!opxmlgetwindowscrollstate (ho, &scrollstate)) /*vertical scroll state*/
		return (false);
	
	if (!opxmlgetwindowexpansionstate (ho, &hexpansionstate)) /*expansion state*/
		goto exit;
		
	if (!shellgetglobalwindowrect (hinfo, &r)) /*window rect*/
		goto exit;
	
	shellgetwindowtitle (hinfo, bstitle); /*window title -- no return value*/
	
	timecreated = (**ho).timecreated; /*time created*/
	
	timemodified = (**ho).timelastsave; /*time last saved*/
	
	if (!datenetstandardstring (timecreated, &vtimecreated)) /*Convert to date net standard strings*/
		goto exit;
	
	if (!datenetstandardstring (timemodified, &vtimemodified))
		goto exit2;
		
	/*Push head items*/
	
	if (!opxmlpushstringline (htext, STR_openhead, indentlevel))							/*<head>*/
		goto exit3;
	
	indentlevel++;
	
	if (!newemptyhandle (&htitle))
		goto exit3;
	
	if (!pushtexthandle (bstitle, htitle))
		goto exit3;
	
	if (!opxmlpushonetag (STR_title, htitle, indentlevel, htext))								/*<title>*/
		goto exit3;
	
	disposehandle (htitle);
	
	if (!opxmlpushonetag (STR_datecreated, vtimecreated.data.stringvalue, indentlevel, htext))	/*<dateCreated>*/
		goto exit3;
	
	if (!opxmlpushonetag (STR_datemodified, vtimemodified.data.stringvalue, indentlevel, htext))/*<dateModified>*/
		goto exit3;
	
	if (!opxmlpushonetag (STR_ownername, hname, indentlevel, htext))							/*<ownerName>*/
		goto exit3;
	
	if (!opxmlpushonetag (STR_owneremail, hemail, indentlevel, htext))							/*<ownerEmail>*/
		goto exit3;
	
	if (!opxmlpushonetag (STR_expansionstate, hexpansionstate, indentlevel, htext))				/*<expansionState>*/	
		goto exit3;
	
	if (!opxmlpushonetaglong (STR_vertscrollstate, scrollstate, indentlevel, htext))			/*<vertScrollState>*/
		goto exit3;
	
	if (!opxmlpushonetaglong (STR_windowtop, r.top, indentlevel, htext))						/*<windowTop>*/
		goto exit3;
	
	if (!opxmlpushonetaglong (STR_windowleft, r.left, indentlevel, htext))						/*<windowLeft>*/
		goto exit3;
	
	if (!opxmlpushonetaglong (STR_windowbottom, r.bottom, indentlevel, htext))					/*<windowBottom>*/
		goto exit3;

	if (!opxmlpushonetaglong (STR_windowright, r.right, indentlevel, htext))					/*<windowRight>*/
		goto exit3;

	if (hcloud != nil)
		if (!opxmlpushcloudtag (hcloud, indentlevel, htext))
			goto exit3;

	if (!opxmlpushstringline (htext, STR_closehead, indentlevel))								/*</head>*/
		goto exit3;
	
	fl = true; /*success*/
	
	/*Clean up*/
	
	exit3:
	
	disposevaluerecord (vtimemodified, false);
	
	exit2:
	
	disposevaluerecord (vtimecreated, false);
	
	exit:
	
	disposehandle (hexpansionstate);
	
	return (fl);
	} /*opxmlbuildhead*/


static boolean opxmlencodetext (Handle htext) {
	
	/*
	7.0b21 PBS: encode text -- translate & to &amp;, etc.
	
	The opposite translation happens in
	opxmldecodetext when converting from XML
	to an outline.
	*/
	
	if (!replaceallinhandle (STR_amp_replace, STR_amp, htext))
		return (false);
	
	if (!replaceallinhandle (STR_quot_replace, STR_quot, htext))
		return (false);
	
	if (!replaceallinhandle (STR_lt_replace, STR_lt, htext))
		return (false);
	
	if (!replaceallinhandle (STR_gt_replace, STR_gt, htext))
		return (false);

	return (true);
	} /*opxmlencodetext*/


static boolean opxmldecodetext (Handle htext) {
	
	/*
	7.0b21 PBS: decode text -- translate &amp; to &, etc.
	
	This is the inverse of opxmlencodetext.
	*/
	
	replaceallinhandle (STR_quot, STR_quot_replace, htext);
	
	replaceallinhandle (STR_lt, STR_lt_replace, htext); 

	replaceallinhandle (STR_gt, STR_gt_replace, htext); 

	replaceallinhandle (STR_amp, STR_amp_replace, htext);

	return (true);
	} /*opxmldecodetext*/


static boolean opxmlpushindents (Handle htext, short indentlevel) {
	
	/*
	7.0b21 PBS: push indentlevel \t's at the end of htext.
	*/
	
	bigstring bs;
	short ix = 1;
	
	if (indentlevel < 1)
		return (true); /*nothing to do*/
	
	setemptystring (bs);
	
	if (indentlevel > 255) /*maximum indentlevel is maximum characters in a bigstring*/
		indentlevel = 255; /*indentlevel might be larger -- that's alright -- just stop pretty printing at this absurd level*/
	
	setstringlength (bs, indentlevel);
	
	while (ix <= indentlevel) {
		
		bs [ix] = '\t'; /*Add a \t to the indent string*/
		
		ix++;
		} /*while*/
	
	return (pushtexthandle (bs, htext)); /*push indents at the end of htext*/
	} /*opxmlpushindents*/


static boolean opxmlpushstringline (Handle htext, bigstring bs, short indentlevel) {
	
	/*
	7.0b21 PBS: push a string that's an entire line, such as the XML header. Add
	a \r to the end.
	
	This the equivalent of the Frontier line: xmltext = xmltext + string.filledString ("\t", indentlevel) + s + "\r"
	*/
	
	if (!opxmlpushindents (htext, indentlevel)) /*Add \t characters.*/
		return (false);
	
	if (!pushtexthandle (bs, htext)) /*Add string to htext.*/
		return (false);
	
	return (pushtexthandle ("\x01" "\r", htext)); /*Add \r to htext.*/
	} /*opxmlpushstringline*/


static boolean opxmlpushhandleline (Handle htext, Handle h, short indentlevel) {

	/*
	7.0b21 PBS: push a handle that's an entire line, such as the XML header. Add
	a \r to the end.
	
	This the equivalent of the Frontier line: xmltext = xmltext + string.filledString ("\t", indentlevel) + s + "\r"
	*/

	if (!opxmlpushindents (htext, indentlevel)) /*Add \t characters.*/
		return (false);
	
	if (!pushhandle (h, htext)) /*Add h to htext.*/
		return (false);
	
	return (pushtexthandle ("\x01" "\r", htext)); /*Add \r to htext.*/
	} /*opxmlpushhandleline*/
	

static boolean opxmlfindtableitem (hdlhashtable ht, bigstring bsname, hdlhashtable *htfound) {
	
	/*
	7.0b21 PBS: find one table item in a table, such as an <outlineDocument>,
	<head>, or <body> item.
	*/
	
	hdlhashnode hn;
	
	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) { /*loop through the table*/
		
		if (isxmlmatch (hn, bsname)) { /*is this the name we're looking for?*/
			
			if (gethashnodetable (hn, htfound)) { /*Try to get the table*/
			
				return (true); /*Found, it's a table, return true.*/
				} /*if*/
			} /*if*/
		} /*for*/

	return (false);
	} /*opxmlfindtableitem*/



static boolean opxmlheadexists (hdlhashtable ht) {



	/*

	7.0b33 PBS: return true if there's a required <head> section of the document.

	*/



	hdlhashnode hn;

	

	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) { /*loop through the table*/

		

		if (isxmlmatch (hn, STR_head)) /*is this the name we're looking for?*/

						

			return (true); /*Found; return true.*/

		} /*for*/



	return (false);

	} /*opxmlheadexists*/




static boolean opxmlgetonevalue (hdlhashtable ht, bigstring bsname, Handle htext) {
	
	/*
	7.0b21 PBS: get the value of one item from a table.
	
	Everything is text at this level -- if you're not expecting text,
	don't use this routine. Other wrapper routines convert from text
	to numbers or lists or whatever.
	*/
	
	tyvaluerecord val;
	hdlhashnode hn;
	boolean fl = false;
	
	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
		
		if (isxmlmatch (hn, bsname)) {
		
			val = (**hn).val;
			
			if (val.valuetype == stringvaluetype) { /*Everything we're looking for is expected to be a string. Check anyway.*/
			
				if (copyhandlecontents (val.data.stringvalue, htext)) {
				
					fl = true;
					} /*if*/
				} /*if*/
				
			break; /*Break once the item has been found, whether or not htext contains the data sought.*/
			} /*if*/
		} /*for*/

	return (fl);
	} /*opxmlgetonevalue*/


static boolean opxmlgetonelongvalue (hdlhashtable ht, bigstring bsname, long *v) {
	
	/*
	7.0b21 PBS: get one long value. See comments in opxmlgetoneshortvalue.
	*/
	
	Handle h;
	bigstring bs;
	
	newemptyhandle (&h);
	
	if (!opxmlgetonevalue (ht, bsname, h)) /*Get value in Handle h*/
		return (false);
	
	texthandletostring (h, bs); /*Convert to bigstring -- it's going to be a number, so a bigstring's size limit is okay*/
	
	disposehandle (h);
	
	return (stringtonumber (bs, v)); /*Convert bigstring to a long*/
	} /*opxmlgetonelongvalue*/


static boolean opxmlgetoneshortvalue (hdlhashtable ht, bigstring bsname, short *v) {
	
	/*
	7.0b21 PBS: get one number value.
	
	This function goes from handle to bigstring to short. Why? Because I wanted just one
	routine that gets the value -- that's opxmlgetonevalue. All other routines use
	that routine to get a value, then do coercions. Is that a performance problem? I seriously
	doubt it, but testing will tell.
	*/
	
	Handle h;
	bigstring bs;
	
	newemptyhandle (&h);
	
	if (!opxmlgetonevalue (ht, bsname, h)) /*Get value in Handle h*/
		return (false);
	
	texthandletostring (h, bs); /*Convert to bigstring -- it's going to be a number, so a bigstring's size limit is okay*/
	
	disposehandle (h);
	
	return (stringtoshort (bs, v)); /*Convert bigstring to a short*/
	} /*opxmlgetoneshortvalue*/




static boolean opxmlsetwindowpositionandsize (hdlhashtable ht, hdlwindowinfo hinfo) {
	
	/*
	7.0b21 PBS: set the window's position and size.
	
	Get windowtop, windowleft, windowbottom, and windowright from the <head>
	section of the XML.
	
	Even though the return value is ignored by opxmlsetwindowatts, this script still returns
	a boolean just in case at some point later someone does care about success or failure.
	*/
	
	short windowtop, windowleft, windowbottom, windowright;
	
	/*Set window position*/

	if (!opxmlgetoneshortvalue (ht, STR_windowtop, &windowtop))
		return (false);

	if (!opxmlgetoneshortvalue (ht, STR_windowleft, &windowleft))
		return (false);
	
//	if (!shellmovewindow (hinfo, windowleft, windowtop)) /*Set window position*/
//		return (false); /*shellmovewindow always returns true at this writing, but that could change*/

	if (!shellmovewindowhidden (hinfo, windowleft, windowtop)) /*Set window position*/

		return (false); /*shellmovewindow always returns true at this writing, but that could change*/

	
	/*Set the window size*/
	
	if (!opxmlgetoneshortvalue (ht, STR_windowright, &windowright))
		return (false);
	
	if (!opxmlgetoneshortvalue (ht, STR_windowbottom, &windowbottom))
		return (false);
	
	if (!shellsizewindowhidden (hinfo, windowright - windowleft, windowbottom - windowtop)) /*Set window size*/
		return (false); /*shellsizewindow returns false if the window isn't growable*/

	return (true);
	} /*opxmlsetwindowpositionandsize*/


static boolean opxmlsetwindowtitle (hdlhashtable ht, hdlwindowinfo hinfo) {
	
	/*
	7.0b21 PBS: set the window title.
	*/
	
	Handle htext;
	bigstring bstitle;
	
	if (!newemptyhandle (&htext))
		return (false);
	
	if (!opxmlgetonevalue (ht, STR_title, htext)) /*Get the <title> value*/
		return (false);
	
	texthandletostring (htext, bstitle); /*Convert to bigstring -- window titles are bigstrings*/
	
	return (shellsetwindowtitle (hinfo, bstitle)); /*Set the window title*/
	} /*opxmlsetwindowtitle*/


static boolean opxmlsetwindowexpansionstate (hdlhashtable ht, hdlwindowinfo hinfo) {
	
	/*
	7.0b21 PBS: set the expansion state for a window.
	*/
	
	Handle htext;
	tyvaluerecord val, v;
	boolean fl = false;
	
	if (!newemptyhandle (&htext))
		return (false);
	
	if (!opxmlgetonevalue (ht, STR_expansionstate, htext))
		goto exit1;
	
	/*Surround text with { and }, so it can be coerced to a list.*/
	
	if (!insertinhandle (htext, 0, "{", 1)) /*text = { + text*/
		goto exit1;

	if (!pushtexthandle ("\x01" "}", htext)) /*text = text + }*/
		goto exit1;
	
	/*Create a value record containing the text.*/
	
	initvalue (&val, stringvaluetype); /*no return value*/
	
	val.data.stringvalue = htext;
	
	/*Coerce to a list.*/
	
	if (!coercevalue (&val, listvaluetype))
		goto exit2;
	
	/*Set the expansion state of the window.*/
	
	if (!shellpushwindowglobals (hinfo)) /*Make our window the current window, so it can be updated.*/
		goto exit2;
		
	fl = opsetexpansionstateverb (&val, &v); /*v is asked for but never referenced.*/

	//shellsetscrollbars ((**hinfo).macwindow); /*Update scroll bars*/
	
	shellpopglobals (); /*Finished updating.*/
	
	exit2:
	
	disposevaluerecord (val, false); /*disposes htext*/
	
	htext = nil;
	
	exit1:
	
	if (htext != nil)
		disposehandle (htext);
	
	return (fl);
	} /*opxmlsetwindowexpansionstate*/


static boolean opxmlsetwindowverticalscrollstate (hdlhashtable ht, hdlwindowinfo hinfo) {
	
	/*
	7.0b21 PBS: set vertical scroll state for the outline window.

	7.0b22 PBS: fix display glitch by updating the outline window.



	7.0b23 PBS: display glitch fix part two -- set hline1 to hbarcursor, not the

	other way around. It's hbarcursor that's correct.
	*/
	
	long line1;
	tyvaluerecord val; /*needed by opsetscrollstateverb*/
	hdlwindowinfo oldoutlinewindow;
	
	if (!opxmlgetonelongvalue (ht, STR_vertscrollstate, &line1)) /*Get vertical scroll state number.*/
		return (false);

	if (!shellpushwindowglobals (hinfo)) /*Push window globals so the window can be updated.*/
		return (false);

	opsetscrollstateverb (line1, &val); /*return value is uninteresting*/

	(**outlinedata).hline1 = (**outlinedata).hbarcursor; /*7.0b23: these were switched, now they're correct.*/

	/*7.0b22 PBS: update the window, fix display glitch.*/

	oldoutlinewindow = outlinewindowinfo;

	outlinewindowinfo = hinfo;

	oppushoutline (outlinedata);

	oppostfontchange (); /*adjust display area*/
		
	opresetscrollbars (); /*make sure scrollbars are accurate*/

	#ifdef MACVERSION
	
		shellupdatewindow ((**hinfo).macwindow);

	#endif
	
	outlinewindowinfo = oldoutlinewindow;

	oppopoutline ();

	shellpopglobals (); /*Finished updating*/
	
	disposevaluerecord (val, false);
	
	return (true);
	} /*opxmlsetwindowverticalscrollstate*/


static boolean opxmlsetwindowatts (hdlhashtable ht, hdloutlinerecord ho, hdlhashtable hto, bigstring bso, tyvaluerecord vo) {
	
	/*
	7.0b21 PBS: set window attributes from the head section of the outlineDocument.
	*/
	
	hdlwindowinfo hinfo;
	
	/*Open window as hidden. Get window info handle.*/
	
	if (!langzoomvalwindow  (hto, bso, vo, false)) /*open window*/
		return (false);
		
	if (!shellfinddatawindow ((Handle) ho, &hinfo)) /*get window info*/
		return (false);
	
	/*Set window position and size.*/
	
	opxmlsetwindowpositionandsize (ht, hinfo); /*Errors are non-fatal -- in the script this is in a try block*/
	
	/*Set expansion state.*/
	
	oppushoutline (ho);
	
	opxmlsetwindowexpansionstate (ht, hinfo); /*Errors non-fatal*/
	
	oppopoutline ();
	
	/*Set vertical scroll state.*/
	
	oppushoutline (ho);

	opxmlsetwindowverticalscrollstate (ht, hinfo); /*Errors non-fatal*/
	
	oppopoutline ();

	/*Set window title.*/
	
	opxmlsetwindowtitle (ht, hinfo); /*Errors non-fatal*/

	return (true);	
	} /*opxmlsetwindowatts*/


static boolean opxmlgetattribute (hdlhashtable ht, bigstring bsname, tyvaluetype type, tyvaluerecord *val) {

	/*
	12/19/01 dmb: get one attribute value of the given type. val result is a temp value
	*/
	
	hdlhashnode hn;

	if (!xmlgetattribute (ht, bsname, &ht)) /*find the attribute in the /atts table*/
		return (false);
	
	if (!hashtablelookup (ht, bsname, val, &hn)) /*find the hash node*/
		return (false);
	
	if (!copyvaluerecord (*val, val))
		return (false);

	if (type != novaluetype)
		if (!coercevalue (val, type))
			return (false);

	return (true);
	} /*opxmlgetattribute*/


static boolean opxmlpullcloudattribute (hdlhashtable ht, bigstring bsatt, tyvaluetype type, bigstring legalvalues, hdlhashtable hatts) {

	/*
	12/19/01 dmb: pull an attribute out of ht and put it into hatts
	*/

	tyvaluerecord val;
	boolean fl;

	disablelangerror();

	fl = opxmlgetattribute (ht, bsatt, type, &val) && teststringvalue (&val, legalvalues);

	enablelangerror();

	if (!fl) {

		langparamerror (cloudelementerror, bsatt);

		return (false);
		}

	if (!hashtableassign (hatts, bsatt, val))
		return (false);
	
	exemptfromtmpstack (&val);

	return (true);
	} /*opxmlpullattribute*/


static boolean opxmlgetcloudatts (hdlhashtable hhead, hdlhashtable hcloud) {
	
	/*
	7.1b43 dmb: get the cloud attributes from the head section of the outlineDocument.
				populdate hcloud with the values
	*/

	hdlhashtable hcloudtag;
	
	if (hcloud == nil)
		return (true);

	if (!opxmlfindtableitem (hhead, STR_cloud, &hcloudtag))
		return (false);

	if (!opxmlpullcloudattribute (hcloudtag, STR_domain, stringvaluetype, nil, hcloud))
		return (false);

	if (!opxmlpullcloudattribute (hcloudtag, STR_port, intvaluetype, nil, hcloud))
		return (false);

	if (!opxmlpullcloudattribute (hcloudtag, STR_path, stringvaluetype, nil, hcloud))
		return (false);

	if (!opxmlpullcloudattribute (hcloudtag, STR_regProcedure, stringvaluetype, nil, hcloud))
		return (false);

	if (!opxmlpullcloudattribute (hcloudtag, STR_protocol, stringvaluetype, STR_legalprotocols, hcloud))
		return (false);

	return (true);
	} /*opxmlgetcloudatts*/


static boolean opxmlgetheadlinetextfromatts (hdlhashtable ht, Handle htext) {
	
	/*
	7.0b21 PBS: get the headline text from an <outline...> item.
	*/
	
	tyvaluerecord val, vcopy;
	hdlhashnode hnode;
	
	if (!xmlgetattribute (ht, STR_textitemname, &ht)) /*find the attribute in the /atts table*/
		return (false);
	
	if (!hashtablelookup (ht, STR_textitemname, &val, &hnode)) /*find the hash node*/
		return (true); /*Not an error -- text is optional attribute.*/
	
	copyvaluerecord (val, &vcopy);
	
	if (!copyhandlecontents (vcopy.data.stringvalue, htext))
		return (false);
	
	/*Replace entities with the real characters.*/
	
	opxmldecodetext (htext);
	
	disposevaluerecord (vcopy, false);
	
	return (true);
	} /*opxmlgetheadlinetextfromatts*/


static boolean opxmldeleteitemfromatts (hdlhashtable htatts, bigstring bsitemtodelete) {
	
	/*
	7.0b21 PBS: delete an item from the /atts table.
	
	If item is not found, it's not an error.
	*/
	
	if (!pushhashtable (htatts))
		return (false);

	disablelangerror (); /*if not found, not an error*/
	
	hashdelete (bsitemtodelete, true, false);
	
	enablelangerror ();
	
	if (!pophashtable ())
		return (false);
	
	return (true);
	} /*opxmldeleteitemfromatts*/


static boolean opxmldecodeattstablevalues (hdlhashtable ht) {
	
	/*
	7.0b21 PBS: Decode all string values in the atts table.
	*/
	
	tyvaluerecord val;
	hdlhashnode hn;
	boolean fl = false;
	
	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
		
			val = (**hn).val;
			
			if (val.valuetype == stringvaluetype) /*Decode text only.*/
				
				opxmldecodetext (val.data.stringvalue);	/*Decode this text.*/		
		} /*for*/

	fl = true; /*Success*/
	
	return (fl);
	} /*opxmldecodeattstablevalues*/


static boolean opxmlsetrefcon (hdlhashtable ht) {
	
	/*
	7.0b21 PBS: set the nodetype refcon.
	
	If ht contains an /atts table, pack the table, pack into a binary,
	then set the refcon to the packed binary.
	*/
	
	hdlhashtable htatts;
	tyvaluerecord val;
	hdlhashnode hnode;
	Handle hpackedtable, hpackedbinary;
	tyvaluerecord vbinary;
	hdlheadrecord hbarcursor;
	boolean fl = false;
	
	/*Get the /atts table.*/

	if (!hashtablelookup (ht, STR_attstablename, &val, &hnode)) /*Is there an /atts table?*/
		return (false);

	if (!langexternalvaltotable (val, &htatts, hnode)) /*Try to get the atts table*/
		goto exit1;
	
	/*Delete the text, isBreakpoint, and isComment items from the /atts table.*/
	
	opxmldeleteitemfromatts (htatts, STR_textitemname); /*text*/
	
	opxmldeleteitemfromatts (htatts, STR_isbreakpoint); /*isBreakpoint*/
	
	opxmldeleteitemfromatts (htatts, STR_iscomment); /*isComment*/
	
	/*Decode values in the atts table.*/
	
	opxmldecodeattstablevalues (htatts);
	
	/*Pack the table; pack the binary.*/
	
	if (!langpackvalue (val, &hpackedtable, HNoNode)) /*Try to pack the table*/
		goto exit1;
	
	initvalue (&vbinary, binaryvaluetype);
	
	vbinary.data.binaryvalue = hpackedtable;
	
	if (!langpackvalue (vbinary, &hpackedbinary, HNoNode)) /*try to pack the binary*/
		goto exit2;
	
	/*Set the refcon.*/
	
	hbarcursor = (**outlinedata).hbarcursor;
	
	disposehandle ((**hbarcursor).hrefcon); /*dispose old refcon*/

	(**hbarcursor).hrefcon = hpackedbinary;
	
	fl = true; /*success*/
		
	exit2:
	
	disposevaluerecord (vbinary, false);
	
	exit1:
		
	return (fl);
	} /*opxmlsetrefcon*/


static boolean opxmlsetcommentandbreakpoint (hdlhashtable ht) {
	
	/*
	7.0b21 PBS: set the comment and breakpoint atts of this headline, if present.
	*/
	
	hdlhashtable htatts;
	tyvaluerecord val;
	hdlhashnode hnode;
	boolean fl = false;
	bigstring bs;
	hdlheadrecord hcursor;
	
	/*Get the /atts table.*/

	if (!hashtablelookup (ht, STR_attstablename, &val, &hnode)) /*Is there an /atts table?*/
		return (false);
	
	if (!langexternalvaltotable (val, &htatts, hnode)) /*Try to get the atts table*/
		goto exit;
	
	hcursor = (**outlinedata).hbarcursor;
	
	/*Is this is a comment?*/
		
	if (hashtablelookup (htatts, STR_iscomment, &val, &hnode)) {
		
		if (val.valuetype == stringvaluetype) {
			
			texthandletostring (val.data.stringvalue, bs);
			
			if (equalidentifiers (bs, STR_booleantrue))
			
				(**hcursor).flcomment = true;
			} /*if*/
		} /*if*/
	
	/*Is this a breakpoint?*/
	
	if (hashtablelookup (htatts, STR_isbreakpoint, &val, &hnode)) {
		
		if (val.valuetype == stringvaluetype) {
			
			texthandletostring (val.data.stringvalue, bs);
			
			if (equalidentifiers (bs, STR_booleantrue))
			
				(**hcursor).flbreakpoint = true;
			} /*if*/
		} /*if*/
	
	fl = true; /*success*/
	
	exit:
	
	return (fl);
	} /*opxmlsetcommentandbreakpoint*/


extern boolean opsetheadtext (hdlheadrecord hnode, Handle hstring);

static boolean opxmltooutlinevisit (hdlhashtable ht, short ixlevel, bigstring bsname, boolean flfirstline) {
	
	/*
	7.0b21 PBS -- recursive routine for converting an XML outlineDocument structure to an outline structure.
	*/
	
	hdlhashnode hn;
	tydirection dir = right;
	boolean flatleastoneinserted = false;
	hdlhashtable h;

	/*loop through all of the items in the table*/
	
	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
		
		Handle htext;
		bigstring nameitem;
						
		if (langexternalgettype ((**hn).val) == idtableprocessor) {
	
			gethashkey (hn, nameitem);
		
			xmlgetname (nameitem);
			
			alllower (nameitem);
			
			if (equalidentifiers (STR_outlinetag, nameitem)) { /*is it an <outline...> item?*/
						
				gethashnodetable (hn, &h);
				
				newemptyhandle (&htext);
				
				oppushoutline (outlinedata);
				
				opxmlgetheadlinetextfromatts (h, htext); /*text att is optional, so htext may be empty, which is okay.*/
										
				if (flfirstline) { /*is it the very first headline in the XML file?*/

					opsetheadtext ((**outlinedata).hbarcursor, htext); /*consumes htext*/
					
					flfirstline = false;
					} /*if*/
				
				else {					
		
					opinserthandle (htext, dir);
					
					disposehandle (htext);

					} /*else*/
				
				oppopoutline ();
					
				opxmlsetcommentandbreakpoint (h); /*Set comment and breakpoint attributes.*/
				
				opxmlsetrefcon (h); /*Set the refcon -- node types*/
				
				dir = down;
				
				flatleastoneinserted = true;
				
				} /*if*/

			switch (getstringcharacter (nameitem, 0)) {
			
				case '/': /*special tables*/
				case '?': 
					break;
				
				default:

					gethashnodetable (hn, &ht);
					
					if (opxmltooutlinevisit (ht, ixlevel + 1, nameitem, flfirstline))
					
						opmotionkey (left, 1, false);					
				} /*switch*/
			} /*if*/
		} /*for*/
	
	return (flatleastoneinserted);
	} /*opxmltooutlinevisit*/



/* Original scripts

on outlineToXml (adroutline) { //7/19/00; 6:30:02 PM by DW
	ÇChanges:
		ÇWednesday, July 26, 2000 at 6:38:36 PM by DW
			ÇEncode ampersands, quotes and less-thans in attributes.
		Ç07/27/00; 3:24:37 PM by PBS
			ÇEncode > characters as &gt;, so the generated XML can be compiled later.
		ÇFriday, August 25, 2000 at 11:13:16 AM by DW
			ÇVersion 1.0d2. Add <ownerName> and <ownerEmail> to the <head>.
		ÇThu, Sep 7, 2000 at 2:53:03 PM by JES
			ÇSet the target to oldTarget before returning xmltext.
	local (scrollstate, expansionstate, windowtop, windowleft, windowheight, windowwidth, windowtitle);
	on encode (s) {
		s = string.replaceall (s, "&", "&amp;");
		s = string.replaceall (s, "\"", "&quot;");
		s = string.replaceall (s, "<", "&lt;");
		s = string.replaceall (s, ">", "&gt;"); //PBS 07/27/00: encode > characters, so XML can be compiled later
		return (s)};
	bundle { //set state variables based on the original window
		local (oldtarget = target.set (adroutline));
		scrollstate = op.getscrollstate ();
		expansionstate = op.getexpansionstate ();
		window.getSize (adroutline, @windowwidth, @windowheight);
		window.getPosition (adroutline, @windowleft, @windowtop);
		windowtitle = window.gettitle (adroutline);
		};Çtarget.set (oldtarget)
	local (xmltext = "", indentlevel = 0);
	on add (s) {
		xmltext = xmltext + string.filledstring ("\t", indentlevel) + s + "\r"};
	add ("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>");
	local (localoutline);
	localoutline = adroutline^;
	local (oldtarget = target.set (@localoutline));
	add ("<outlineDocument version=\"1.0d2\">"); indentlevel++;
	bundle { //add <head>
		add ("<head>"); indentlevel++;
		add ("<title>" + windowtitle + "</title>");
		add ("<dateCreated>" + date.netstandardstring (timecreated (adroutline)) + "</dateCreated>");
		add ("<dateModified>" + date.netstandardstring (timemodified (adroutline)) + "</dateModified>");
		add ("<ownerName>" + user.prefs.name + "</ownerName>");
		add ("<ownerEmail>" + user.prefs.mailAddress + "</ownerEmail>");
		bundle { //add expansion state
			local (expansionlist = string (expansionstate), num, s = "");
			for num in expansionlist {
				s = s + num + ","};
			s = string.delete (s, sizeof (s), 1); //delete last comma
			add ("<expansionState>" + s + "</expansionState>")};
		add ("<vertScrollState>" + scrollstate + "</vertScrollState>");
		bundle { //add <windowXxx> elements
			add ("<windowTop>" + windowtop + "</windowTop>");
			add ("<windowLeft>" + windowleft + "</windowLeft>");
			add ("<windowBottom>" + (windowtop + windowheight) + "</windowBottom>");
			add ("<windowRight>" + (windowleft + windowwidth) + "</windowRight>")};
		add ("</head>"); indentlevel--};
	bundle { //add <body>
		add ("<body>"); indentlevel++;
		op.fullexpand ();
		op.firstsummit ();
		on visitLevel () {
			local (s);
			loop {
				s = "<outline text=\"" + encode (op.getlinetext ()) + "\"";
				bundle { //add attributes from refcon, if there are any
					local (data = op.getrefcon ());
					if typeof (data) == binarytype { //has attributes
						local (attstable);
						unpack (@data, @attstable);
						for adr in @attstable {
							s = s + " " + nameof (adr^) + "=\"" + encode (adr^) + "\""}}}; //7/26/00 DW
				if script.isComment () {
					s = s + " isComment=\"true\""};
				if op.go (right, 1) {
					add (s + "\>"); indentlevel++;
					visitLevel ();
					add ("</outline>"); indentlevel--;
					op.go (left, 1)}
				else {
					add (s + "\/>")};
				if not op.go (down, 1) {
					break}}};
		visitLevel ();
		add ("</body>"); indentlevel--};
	add ("</outlineDocument>"); indentlevel--;
	target.set (oldTarget); // 09/07/00 JES
	return (xmltext)}
	

on xmlToOutline (xmltext, adroutline, flnewoutline = true) { //7/19/00; 6:51:02 PM by DW
	ÇChanges:
		Ç07/25/00; 8:52:21 PM by PBS
			ÇSet refcon only if there's data to set.
		Ç07/27/00; 3:27:12 PM by PBS
			ÇDecode &quot;, &lt;, &gt; and &amp; in text attributes so round-trip of HTML-in-XML works.
		ÇTuesday, August 08, 2000 at 6:01:44 PM by DW
			ÇAllow any attributes to be linked to a headline through the refcon. We have to make assumptions about the XML structure that xml.compile generates, there's no way to do this  through the procedural interface.
		ÇTuesday, August 08, 2000 at 6:24:31 PM by DW
			ÇCommented debugging code that had accidentally been left uncommented.
		ÇTuesday, August 15, 2000 at 1:29:57 PM by JES
			ÇBug fix -- no longer fails when converting deeply nested outlines.
	if flnewoutline {
		new (outlinetype, adroutline)};
	local (xstruct);
	local (oldtarget = target.set (adroutline));
	xml.compile (xmltext, @xstruct);
	Çscratchpad.xstruct = xstruct; wp.newtextobject (xmltext, @scratchpad.xtext)
	on dolevel (adrxoutline) {
		local (insertdir = right, flatleastoneinserted = false);
		local (item, text, attstable);
		for item in adrxoutline {
			if nameOf (item^) endsWith "\toutline" {
				bundle { //fill the atts table with atts we understand
					new (tabletype, @attstable);
					local (adratts = @item^.["/atts"], adratt);
					for adratt in adratts {
						attstable.[nameof (adratt^)] = adratt^};
					try {text = attstable.text; delete (@attstable.text)} else {text = ""}};
				bundle { //PBS 07/27/00: decode &quot;, &gt;, &lt; &amp;
					text = string.replaceall (text, "&quot;", "\"");
					text = string.replaceall (text, "&lt;", "<");
					text = string.replaceall (text, "&gt;", ">");
					text = string.replaceall (text, "&amp;", "&")};
				op.insert (text, insertdir); insertdir = down;
				if sizeOf (attstable) > 0 { //PBS 07/25/00: set refcon only if there's data to set
					local (data);
					pack (attstable, @data);
					op.setrefcon (data)};
				flatleastoneinserted = true;
				if dolevel (item) { //at least one item added
					op.go (left, 1)}}};
		return (flatleastoneinserted)};
	local (adroutlinedocument = xml.getaddress (@xstruct, "outlineDocument"));
	local (adrbody = xml.getaddress (adroutlinedocument, "body"));
	dolevel (adrbody);
	bundle { //process <head>, if new outline
		if flnewoutline {
			bundle { //perform outline surgery
				op.firstsummit ();
				op.promote ();
				op.deleteline ()};
			local (adrhead = xml.getaddress (adroutlinedocument, "head"));
			try { //set the window position/size
				local (windowTop = number (xml.getaddress (adrhead, "windowTop")^));
				local (windowLeft = number (xml.getaddress (adrhead, "windowLeft")^));
				local (windowBottom = number (xml.getaddress (adrhead, "windowBottom")^));
				local (windowRight = number (xml.getaddress (adrhead, "windowRight")^));
				window.setposition (adroutline, windowLeft, windowTop);
				window.setsize (adroutline, windowRight - windowLeft, windowBottom - windowTop)};
			try { //set the expansion state
				local (expansionString = xml.getaddress (adrhead, "expansionState")^);
				local (expansionList = {}, i);
				for i = 1 to string.countfields (expansionString, ',') {
					expansionList = expansionList + string.nthfield (expansionString, ',', i)};
				op.setexpansionstate (expansionList)};
			try { //set the vertical scroll state
				op.setscrollstate (xml.getaddress (adrhead, "vertScrollState")^)};
			try { //set the window title
				parentOf (adroutline^)^.title = xml.getaddress (adrhead, "title")^}}};
	target.set (oldtarget);
	return (true)}

*/
