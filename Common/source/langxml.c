
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

#include "memory.h"
#include "strings.h"
#include "ops.h"
#include "resources.h"
#include "timedate.h"
#include "lang.h"
#include "langinternal.h"
#include "langhtml.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "oplist.h"
#include "BASE64.H" //for xmlvaltostring
#include "langxml.h" /*7.0b21 PBS*/
#include "process.h"

#define stringerrorlist 264
#define notimplementederror 1

#define STR_atts 	(BIGSTRING ("\x05" "/atts"))
#define STR_i4		(BIGSTRING ("\x02" "i4"))
#define STR_i2		(BIGSTRING ("\x02" "i2"))
#define STR_i1		(BIGSTRING ("\x02" "i1"))
#define STR_float	(BIGSTRING ("\x05" "float"))

#define STR_base64_begin	(BIGSTRING ("\x08" "<base64>"))
#define STR_base64_end		(BIGSTRING ("\x09" "</base64>"))
#define STR_value_begin		(BIGSTRING ("\x07" "<value>"))
#define STR_value_end		(BIGSTRING ("\x08" "</value>"))
#define STR_array_begin		(BIGSTRING ("\x07" "<array>"))
#define STR_array_end		(BIGSTRING ("\x08" "</array>"))
#define STR_data_begin		(BIGSTRING ("\x06" "<data>"))
#define STR_data_end		(BIGSTRING ("\x07" "</data>"))
#define STR_struct_begin	(BIGSTRING ("\x08" "<struct>"))
#define STR_struct_end		(BIGSTRING ("\x09" "</struct>"))
#define STR_member_begin	(BIGSTRING ("\x08" "<member>"))
#define STR_member_end		(BIGSTRING ("\x09" "</member>"))
#define STR_name_begin		(BIGSTRING ("\x06" "<name>"))
#define STR_name_end		(BIGSTRING ("\x07" "</name>"))

#define STR_struct		(BIGSTRING ("\x06" "struct"))
#define STR_member		(BIGSTRING ("\x06" "member"))
#define STR_base64		(BIGSTRING ("\x06" "base64"))
#define STR_array		(BIGSTRING ("\x05" "array"))
#define STR_value		(BIGSTRING ("\x05" "value"))
#define STR_data		(BIGSTRING ("\x04" "data"))
#define STR_name		(BIGSTRING ("\x04" "name"))

#define STR_contents	(BIGSTRING ("\x09" "/contents"))
#define STR_namespace	(BIGSTRING ("\x09" "namespace"))
#define STR_version		(BIGSTRING ("\x07" "version"))
#define STR_prefix		(BIGSTRING ("\x06" "prefix"))
#define STR_pcdata		(BIGSTRING ("\x07" "/pcdata"))
#define STR_cdata		(BIGSTRING ("\x06" "/cdata"))
#define STR_pi			(BIGSTRING ("\x03" "/pi"))
#define STR_doctype		(BIGSTRING ("\x08" "/doctype"))
#define STR_comment		(BIGSTRING ("\x08" "/comment"))

#define STR_xmldecl				(BIGSTRING ("\x04" "?xml"))
#define STR_xmlversion			(BIGSTRING ("\x15" "<?xml version=\"1.0\"?>"))
#define STR_xmlnamespace		(BIGSTRING ("\x0e" "?xml:namespace"))
#define STR_endtag				(BIGSTRING ("\x01" ">"))
#define STR_startCDATA			(BIGSTRING ("\x09" "<![CDATA["))
#define STR_endCDATA			(BIGSTRING ("\x03" "]]>"))
#define STR_startcomment		(BIGSTRING ("\x04" "<!--"))
#define STR_endcomment			(BIGSTRING ("\x03" "-->"))
#define STR_startdoctype		(BIGSTRING ("\x0a" "<!DOCTYPE "))

#define STR_datetimeiso8601		(BIGSTRING ("\x10" "dateTime.iso8601"))
#define STR_cantendwithLT 		(BIGSTRING ("\x1d" "can't end the text with a '<'"))
#define STR_didntfindGTafterLT	(BIGSTRING ("\x1d" "didn't find a '>' after a '<'"))
#define STR_tagmustbeofform		(BIGSTRING ("\x1f" "tag must be of the form <?xxx?>"))
#define STR_improperlyformatted (BIGSTRING ("\x27" "string constant is improperly formatted"))
#define STR_itmustendwithtag	(BIGSTRING ("\x19" "it must end with a </tag>"))
#define STR_itcantendontag		(BIGSTRING ("\x17" "it can't end on a <tag>"))
#define STR_wewereexpecting		(BIGSTRING ("\x17" "we were expecting </^0>"))
#define STR_toomanytags			(BIGSTRING ("\x10" "too many </tag>s"))
#define STR_wewereexpectingtag	(BIGSTRING ("\x1d" "we were expecting a </^0> tag"))
#define STR_atcharacteroffset	(BIGSTRING ("\x14" " (At character #^0.)"))
#define STR_tablehasnosubs		(BIGSTRING ("\x1c" " the table has no sub-items."))

#define STR_user_protocols				(BIGSTRING ("\x0e" "user.protocols"))
#define STR_sys_protocols				(BIGSTRING ("\x12" "Frontier.protocols"))
#define STR_unknown_protocol			(BIGSTRING ("\x35" "Can't call \"^0\" because the \"^1\" protocol is unknown."))
#define STR_serverprefix				(BIGSTRING ("\x03" "://"))
#define STR_cant_decompile_empty_table	(BIGSTRING ("\x46" "Can't decompile the table to an XML string because the table is empty."))


typedef enum tyxmlverbtoken { /*verbs that are processed by langxml.c*/
	
	xmladdtablefunc,
	xmladdvaluefunc,
	xmlcompilefunc,
	xmldecompilefunc,
	xmlgetaddressfunc,
	xmlgetaddresslistfunc,
	xmlgetattributefunc,
	xmlgetattributevaluefunc,
	xmlgetvaluefunc,
	xmlvaltostringfunc,
	xmlfrontiervaltotaggedtextfunc,
	xmlstructtofrontiervaluefunc,
	xmlgetpathaddressfunc,
	xmlconverttodisplaynamefunc,
	
	ctxmlverbs
	} tyxmlverbtoken;


typedef struct xmltoken {

	long pos; // 5.1.3: offset in text stream of token start
	boolean isTag;
	boolean openTag;
	boolean isDoctype;
	boolean isCDATA;
	boolean isPI; //processing instruction
	boolean isComment;
	Handle tokenstring;
	hdlhashtable atts;
	tyvaluerecord attsvalue;
	} xmltoken, *ptrxmltoken;





/* function templates */

static boolean xmlvaltostring (tyvaluerecord xmlval, short indentlevel, boolean fltranslatestrings, Handle *string);

static boolean xmlfrontiervaltotaggedtext (tyvaluerecord *val, short indentlevel, Handle *xmltext, hdlhashnode);

static boolean xmlstructtofrontiervalue (tyaddress *adrstruct, tyvaluerecord *v);

static boolean xmlgetaddress (hdlhashtable ht, bigstring name);


static boolean handlebeginswith (Handle h, bigstring bs) {
	
	long ct = gethandlesize (h);
	long ctcompare = stringlength (bs);
	
	if (ct < ctcompare)
		return (false);
	
	return (strncmp (stringbaseaddress ((char *) bs), ((char *) *h), ctcompare) == 0);
	} /*handleendswith*/




static boolean handlecontains (Handle h, bigstring bs) {

	return (textpatternmatch ((byte *)(*h), gethandlesize (h), bs, false) >= 0);
	} /*handlecontains*/


static boolean replaceallinstring (char chfind, bigstring bsreplace, bigstring bs) {
	
	/*
	replace all instances of ch1 in bs with ch2
	
	5.0d14 dmb: scanstring is 1-based, setstringcharacter is 0-based.
	*/
	
	short ix = 1;
	
	while (scanstring (chfind, bs, &ix)) {
	
		replacestring (bs, ix, 1, bsreplace);
		
		ix += stringlength (bsreplace) - 1;
		}
	
	return (true);
	} /*replaceallinstring*/


boolean replaceallinhandle (bigstring bsfind, bigstring bsreplace, Handle htext) {
	
	Handle hfind = nil;
	Handle hreplace = nil;
	boolean fl = false;
	
	if (!newtexthandle (bsfind, &hfind))
		return (false);
	
	if (!newtexthandle (bsreplace, &hreplace))
		goto exit;
	
	fl = textfindreplace (hfind, hreplace, htext, true, false);
	
 exit:
	disposehandle (hfind);
	
	disposehandle (hreplace);
	
	return (fl);
	} /*replaceallinhandle*/


static boolean trimtrailingwhitespace (Handle htext) {
	
	/*
	5.1.3 dmb: remove any trailing whitespace from the handle. return true
	if anything is trimmed
	
	whitespace is defined as cr, lf, tab, space
	*/
	
	long ctbytes = gethandlesize (htext);
	long ctorig = ctbytes;
	
	while (ctbytes > 0) {
		
		switch ((*htext) [ctbytes - 1]) {
			
			case chreturn:
			case chlinefeed:
			case chspace:
			case chtab:
				--ctbytes;
				break;
			
			default:
				goto exit;
			}
		}
	
	exit:
	
	if (ctbytes == ctorig) // nothing needs to be trimmed
		return (false);
	
	sethandlesize (htext, ctbytes); // getting smaller, can't fail
	
	return (true);
	} /*trimtrailingwhitespace*/


static boolean breakatfirstwhitespacechar (bigstring bs) {
	
	/*
	trunate at first whitespace character. return true if whitespace is actually found
	*/
	
	short i;
	
	for (i = 0; i < stringlength (bs); ++i) {
		
		if (isspace (getstringcharacter (bs, i))) {
			
			setstringlength (bs, i);
			
			return (true);
			}
		}
	
	return (false);
	} /*breakatfirstwhitespacechar*/




static boolean xmladdtaggedvalue (tyvaluerecord *val, short indentlevel, handlestream *sptr) {

	/*
	6.1d3 AR: add the xml representation of a Frontier value to the handlestream.
	*/
	
	Handle h = nil;
	
	assert (!(*val).fldiskval); /*08/04/2000 AR*/
	
	if ((*val).valuetype == stringvaluetype) {
		
		if (!xmlfrontiervaltotaggedtext (val, indentlevel, &h, HNoNode)
				|| !inserttextinhandle (h, 0, STR_value_begin)
				|| !pushtexthandle (STR_value_end, h)
				|| !writehandlestreamhandleindent (sptr, h, indentlevel))
			goto exit;
		}
	else {
		
		if (!writehandlestreamstringindent (sptr, STR_value_begin, indentlevel))
			goto exit;
		
		indentlevel++;
		
		if (!xmlfrontiervaltotaggedtext (val, indentlevel, &h, HNoNode))
			goto exit;
		
		if (!writehandlestreamhandleindent (sptr, h, indentlevel))
			goto exit;
		
		if (!writehandlestreamstringindent (sptr, STR_value_end, indentlevel))
			goto exit;

		indentlevel--;
		}
	
	disposehandle (h);

	return (true);

exit:
	disposehandle (h);
	
	return (false);
	} /*xmladdtaggedvalue*/

	
/*
on frontierValueToTaggedText (adrFrontierValue, indentlevel) {
	local (xmltext = "");
	case typeOf (adrFrontierValue^) {
		listtype {
			local (item, ix);
			add ("<array>"); indentlevel++;
			add ("<data>"); indentlevel++;
			for ix = 1 to sizeof (adrFrontierValue^) { //Thu, 27 May 1999 22:02:21 GMT by AR
				table.assign (@item, adrFrontierValue^[ix]);
				addValue (@item)};
			add ("</data>"); indentlevel--;
			add ("</array>"); indentlevel--};
		tabletype {
			local (i, ct = sizeof (adrFrontierValue^), adritem);
			add ("<struct>"); indentlevel++;
			for i = 1 to ct {
				adritem = @adrFrontierValue^ [i];
				add ("<member>"); indentlevel++;
				add ("<name>" + nameof (adritem^) + "</name>");
				addValue (adritem);
				add ("</member>"); indentlevel--};
			add ("</struct>"); indentlevel--}}
	else {
		add (xml.valToString (adrFrontierValue^, indentlevel))};
	xmltext = string.popleading (xmltext, "\t");
	xmltext = string.poptrailing (xmltext, "\r");
	return (xmltext)}	*/

		
static boolean xmlfrontiervaltotaggedtext (tyvaluerecord *val, short indentlevel, Handle *xmltext, hdlhashnode hnode) {
	
	/*
	6.1d3 AR: build the xml representation of a Frontier value.
	*/

	handlestream s;
	
	openhandlestream (nil, &s);
	
	switch ((*val).valuetype) {
		
		case listvaluetype: {
			
			long ix, ct;
			tyvaluerecord v;
			
			if (!writehandlestreamstringindent (&s, STR_array_begin, indentlevel))
				goto exit;
			
			indentlevel++;

			if (!writehandlestreamstringindent (&s, STR_data_begin, indentlevel))
				goto exit;
			
			indentlevel++;
			
			if (!langgetlistsize (val, &ct))
				goto exit;
			
			for (ix = 1; ix <= ct; ix++) {
				
				if (!langgetlistitem (val, ix, nil, &v))
					goto exit;
				
				if (!xmladdtaggedvalue (&v, indentlevel, &s))
					goto exit;
				} /*for*/
			
			if (!writehandlestreamstringindent (&s, STR_data_end, indentlevel))
				goto exit;
			
			indentlevel--;

			if (!writehandlestreamstringindent (&s, STR_array_end, indentlevel))
				goto exit;
			
			indentlevel--;

			break;
			}
		
		case externalvaluetype: {
			hdlhashtable ht;
			boolean fl;
			
			disablelangerror ();
			
			fl = langexternalvaltotable (*val, &ht, hnode);
			
			enablelangerror ();
			
			if (fl) {
				hdlhashnode x;
				register long ix = 0;
				tyvaluerecord v;
				bigstring bsname;
				Handle h;
				
				if (!writehandlestreamstringindent (&s, STR_struct_begin, indentlevel))
					goto exit;
				
				indentlevel++;
				
				while (hashgetnthnode (ht, ix++, &x)) { /*loop through the table*/
					
					if (!hashresolvevalue (ht, x)) /*08/04/2000 AR*/
						goto exit;
					
					if (!writehandlestreamstringindent (&s, STR_member_begin, indentlevel))
						goto exit;
					
					indentlevel++;
					
					gethashkey (x, bsname);
					
					if (!newtexthandle (bsname, &h))
						goto exit;
					
					fl = inserttextinhandle (h, 0, STR_name_begin)
							&& pushtexthandle (STR_name_end, h)
							&& writehandlestreamhandleindent (&s, h, indentlevel);
					
					disposehandle (h);						
					
					if (!fl)
						goto exit;
					
					v = (**x).val;
					
					if (!xmladdtaggedvalue (&v, indentlevel, &s))
						goto exit;
								
					if (!writehandlestreamstringindent (&s, STR_member_end, indentlevel))
						goto exit;
					
					indentlevel--;
					} /*while*/
			
				if (!writehandlestreamstringindent (&s, STR_struct_end, indentlevel))
					goto exit;
				
				indentlevel--;

				break;
				}
			} /*note fall-through for externals that are not a table*/
		
		default: {
			Handle h;
			boolean fl;
			
			if (!xmlvaltostring (*val, indentlevel, true, &h))
				goto exit;
			
			fl = writehandlestreamhandleindent (&s, h, indentlevel);
			
			disposehandle (h);
			
			if (!fl)
				goto exit;
			
			break;
			}	
		
		} /*switch*/
	
	*xmltext = closehandlestream (&s);

	handlepopleadingchars (*xmltext, '\t');

	handlepoptrailingchars (*xmltext, '\r');
	
	return (true);

exit:

	disposehandlestream (&s);
	
	return (false);
	} /*xmlfrontiervaltotaggedtext*/


	/*
on structToFrontierValue (adrstruct, adrFrontierVal) {
	local (name = string.nthfield (nameof (adrstruct^), '\t', 2));
	case name {
		"struct" {
			new (tabletype, adrFrontierVal);
			local (i);
			for i = 1 to sizeOf (adrstruct^) {
				local (itemName = string.nthField (nameOf (adrstruct^ [i]), '\t', 2));
				if itemName == "member" {
					local (member = @adrstruct^ [i]);
					local (name, adrvalue);
					name = xml.getValue (member, "name");
					adrvalue = xml.getAddress (member, "value");
					if typeof (adrvalue^) == tabletype {
						local (coercedvalue);
						xml.coercions.structToFrontierValue (@adrvalue^ [1], @coercedvalue);
						adrFrontierVal^.[name] = coercedvalue}
					else {
						adrFrontierVal^.[name] = adrvalue^}}};
			return (true)};
		"array" {
			local (i, val = {}, adrdata = xml.getAddress (adrstruct, "data"));
			for i = 1 to sizeOf (adrdata^) {
				local (itemName = string.nthField (nameOf (adrdata^ [i]), '\t', 2));
				if itemName == "value" {
					local (adrvalue = @adrdata^ [i]);
					if typeof (adrvalue^) == tabletype {
						local (coercedvalue);
						xml.coercions.structToFrontierValue (@adrvalue^ [1], @coercedvalue);
						val = val + {coercedvalue}}
					else {
						val = val + adrvalue^}}};
			adrFrontierVal^ = val;
			return (true)};
		"base64" { //2/28/99; 11:32:12 AM by DW
			adrFrontierVal^ = binary (base64.decode (adrstruct^));
			return (true)}};
	adrFrontierVal^ = adrstruct^;
	return (true)}	*/

static boolean structtofrontiervalvisit (hdlhashnode hn, hdlhashtable htnew) {

	hdlhashtable ht;
	bigstring bsname;
	hdlhashnode hnode;
	
	gethashkey (hn, bsname);

	if (stringfindchar ('\t', bsname))
		nthword (bsname, 2, '\t', bsname);
	
	if (equalstrings (bsname, STR_member)) {
	
		bigstring bsvalue, bsvaluename, bsnewname;
		tyvaluerecord vname, val;
		tyaddress adrstruct;
		
		if (!langexternalvaltotable ((**hn).val, &ht, hn))
			return (false);
		
		copystring (STR_name, bsvaluename);
		
		if (!xmlgetaddress (ht, bsvaluename))
			return (false);
				
		if (!hashtablelookup (ht, bsvaluename, &vname, &hnode))
			return (false);
		
		if (langexternalvaltotable (vname, &ht, hnode))
			if (!hashtablelookup (ht, STR_pcdata, &vname, &hnode))
				if (!hashtablelookup (ht, STR_contents, &vname, &hnode))
					return (false);
		
		if (vname.valuetype != stringvaluetype)
			if (!copyvaluerecord (vname, &vname) || !coercetostring (&vname))
				return (false);
		
		pullstringvalue (&vname, bsnewname);

		copystring (STR_value, bsvalue);
		
		if (!xmlgetaddress (ht, bsvalue))
			return (false);
		
		if (!hashtablelookup (ht, bsvalue, &val, &hnode))
			return (false);
				
		if (langexternalvaltotable (val, &adrstruct.ht, hnode)) {
			
			tyvaluerecord vcoerced;
			hdlhashnode hnode2;
			
			if (!hashgetnthnode (adrstruct.ht, 0L, &hnode2))
				return (false);
			
			gethashkey (hnode2, adrstruct.bs);
			
			if (!xmlstructtofrontiervalue (&adrstruct, &vcoerced))
				return (false);
			
			exemptfromtmpstack (&vcoerced);
			
			if (!hashtableassign (htnew, bsnewname, vcoerced)) {
			
				disposevaluerecord (vcoerced, false);
				
				return (false);
				}
			}
		else {
		
			if (!copyvaluerecord (val, &val))
				return (false);
			
			exemptfromtmpstack (&val);
			
			if (!hashtableassign (htnew, bsnewname, val)) {
				
				disposevaluerecord (val, false);
				
				return (false);
				}
			}
		}
	
	return (true);
	} /*structtofrontiervalvisit*/


static boolean arraytofrontiervalvisit (hdlhashnode hn, hdllistrecord hlist) {

	tyvaluerecord val;
	bigstring bsname;
	
	gethashkey (hn, bsname);

	if (stringfindchar ('\t', bsname))
		nthword (bsname, 2, '\t', bsname);
	
	if (equalstrings (bsname, STR_value)) {
	
		tyaddress adrstruct;

		val = (**hn).val;
		
		if (langexternalvaltotable (val, &adrstruct.ht, hn)) {
			
			tyvaluerecord vcoerced;
			hdlhashnode hnode;
			
			if (!hashgetnthnode (adrstruct.ht, 0L, &hnode))
				return (false);
			
			gethashkey (hnode, adrstruct.bs);
			
			if (!xmlstructtofrontiervalue (&adrstruct, &vcoerced))
				return (false);
			
			if (!langpushlistval (hlist, nil, &vcoerced))
				return (false);
			}
		else if (!langpushlistval (hlist, nil, &val))
			return (false);
		}
	
	return (true);
	} /*arraytofrontiervalvisit*/


static boolean xmlstructtofrontiervalue (tyaddress *adrstruct, tyvaluerecord *v) {
	
	bigstring bsname;
	tyvaluerecord vstruct, val;
	hdlhashnode hnode;

	if (!langhashtablelookup ((*adrstruct).ht, (*adrstruct).bs, &vstruct, &hnode))
		return (false);

	copystring ((*adrstruct).bs, bsname);

	if (stringfindchar ('\t', bsname))
		nthword (bsname, 2, '\t', bsname);
	
	if (equalstrings (bsname, STR_struct)) {
		
		hdlhashtable ht, htnew;
		hdlhashnode hn;
		long ix = 0;
			
		if (!tablenewtablevalue (&htnew, &val))
			return (false);
		
		//pushtmpstackvalue (&val);
		pushvalueontmpstack (&val);
		
		if (!langexternalvaltotable (vstruct, &ht, hnode))
			goto done;
		
		while (hashgetnthnode (ht, ix++, &hn))
			if (!structtofrontiervalvisit (hn, htnew))
				return (false);
		}
	else if (equalstrings (bsname, STR_array)) {
		
		tyvaluerecord vdata;
		tyaddress adrdata;
		hdlhashtable ht;
		hdlhashnode hn;
		hdllistrecord hlist;
		long ix = 0;

		if (!opnewlist (&hlist, false))
			return (false);
	
		setheapvalue ((Handle) hlist, listvaluetype, &val);

		if (!langexternalvaltotable (vstruct, &adrdata.ht, hnode))
			goto done;
				
		copystring (STR_data, adrdata.bs);
		
		if (!xmlgetaddress (adrdata.ht, adrdata.bs))
			return (false);
		
		if (!langhashtablelookup (adrdata.ht, adrdata.bs, &vdata, &hnode))
			return (false);
		
		if (!langexternalvaltotable (vdata, &ht, hnode))
			goto done;
				
		while (hashgetnthnode (ht, ix++, &hn))
			if (!arraytofrontiervalvisit (hn, hlist))
				return (false);
		}
	else if (equalstrings (bsname, STR_base64)) {
		
		Handle htext;

		if (!copyvaluerecord (vstruct, &vstruct)
				|| !coercetostring (&vstruct))
			return (false);
		
		if (!newemptyhandle (&htext))
			return (false);
			
		if (!base64decodehandle (vstruct.data.stringvalue, htext)) {
			
			disposehandle (htext);
			
			return (false);
			}
		
		setbinaryvalue (htext, '\?\?\?\?', &val);
		}
	else {

		if (!copyvaluerecord (vstruct, &val) || !copyvaluedata (&val))
			return (false);
		}	

done:
	*v = val;
	
	return (true);
	} /*xmlstructtofrontierval*/


static boolean langfindnamedparam (hdltreenode hp1) {

	while (hp1 != nil) {

		if ((**hp1).nodetype == fieldop)
			return (true);

		hp1 = (**hp1).link; /*advance to the next parameter name*/
		}

	return (false);
	} /*langfindnamedparam*/


static boolean callprotocolhandler (hdlhashtable hremotetable, bigstring bsprotocol, hdltreenode hcode, 
								bigstring bsserver, bigstring bsfunction, hdltreenode hparam1, tyvaluerecord *vreturned) {

	/*
	02/05/02 dmb: call the specified remote function. NOTE: bsserver is a misnomer; it's really 
	the full url of the server, less the path; i.e. is includes the protocol, port, the path
	*/

	hdllistrecord hparamlist = nil;
	tyvaluerecord vparamlist;
	tyvaluerecord vparams;
	tyvaluerecord vserver;
	tyvaluerecord vfunction;
	boolean fl;

	if (langfindnamedparam (hparam1))
		fl = makerecordvalue (hparam1, true, &vparams);
	else
		fl = makelistvalue (hparam1, &vparams);

	if (!fl)
		return (false);
	
	if (!setstringvalue (bsserver, &vserver))
		return (false);
	
	if (!setstringvalue (bsfunction, &vfunction))
		return (false);
	
	if (!opnewlist (&hparamlist, false))
		return (false);
	
	if (!langpushlistval (hparamlist, nil, &vserver))
		goto error;

	if (!langpushlistval (hparamlist, nil, &vfunction))
		goto error;

	if (!langpushlistval (hparamlist, nil, &vparams))
		goto error;
	
	if (!setheapvalue ((Handle) hparamlist, listvaluetype, &vparamlist))
		return (false);

	if (!langrunscriptcode (hremotetable, bsprotocol, hcode, &vparamlist, nil, vreturned))
		return (false);

	return (true);

	error: {
		
		opdisposelist (hparamlist);
		
		return (false);
		}

	} /*callprotocolhandler*/


static boolean parseremotefunction (bigstring bs, ptrstring bsprotocol, ptrstring bsserver, short *port) {
#pragma unused (port)

	/*
	5.0.2 dmb: given a string like "rpc2://betty.userland.com:81", set bsprotocol to "rpc2", 
	server to "betty.userland.com"
	*/
	
	firstword (bs, ':', bsprotocol);
	
	if (isemptystring (bsprotocol) || stringlength (bsprotocol) < 1)
		return (false);
	
	midstring (bs, stringlength (bsprotocol) + 1, stringlength (bs) - stringlength (bsprotocol), bsserver);
	
	if (stringlength (bsserver) < 6) // room for ://x.y
		return (false);
	
	// is STR_serverprefix next?

	if (!equaltextidentifiers (stringbaseaddress (bsserver), stringbaseaddress (STR_serverprefix), stringlength (STR_serverprefix)))
		return (false);
	
	deletestring (bsserver, 1, stringlength (STR_serverprefix));
	
	/*nthword (bsserver, 2, ':', bsport);
	
	if (isemptystring (bsport))
		*port = -1;
	else {
		firstword (bsserver, ':', bsserver);
		
		stringtoshort (bsport, port);
		}
	*/

	return (true);
	} /*parseremotefunction*/


static boolean findoneprotocolhandler (hdlhashtable hlookin, bigstring bshandlertable, bigstring bsprotocol, hdlhashtable *hremotetable, hdltreenode *hcode) {

	/*
	Given the <b>bsprotocol</b>, look for a script at user.remoteCallers.[bsprotocol].
	If none found, look for a script at Frontier.remoteCallers.[bsprotocol].
	
	2002-10-13 AR: Declared static to eliminate compiler warning about missing prototype.
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	boolean fl;

	disablelangerror ();
	
	fl = langfastaddresstotable (hlookin, bshandlertable, hremotetable);
	
	enablelangerror ();
	
	if (!fl)
		return (false);

	if (hashtablelookup (*hremotetable, bsprotocol, &val, &hnode)) {
	
		while (langfollowifaddressvalue (&val)) // follow indirection
			;
		
		return (langvaltocode (&val, hcode));
		}
	
	return (false);
	} /*findoneprotocolhandler*/


static boolean findprotocolhandler (bigstring bsprotocol, hdlhashtable *hremotetable, hdltreenode *hcode) {

	/*
	Given the <b>bsprotocol</b>, look for a script at user.remoteCallers.[bsprotocol].
	If none found, look for a script at Frontier.remoteCallers.[bsprotocol].
	
	2002-10-13 AR: Declared static to eliminate compiler warning about missing prototype.
	*/

	if (findoneprotocolhandler (roottable, STR_user_protocols, bsprotocol, hremotetable, hcode))
		return (true);

	if (findoneprotocolhandler (builtinstable, STR_sys_protocols, bsprotocol, hremotetable, hcode))
		return (true);
	
	return (false);
	} /*findprotocolhandler*/


boolean langisremotefunction (hdltreenode htree) {
	
	/*
	5.0.2b8 dmb: carefully walk the code tree to determine if it's a remote
	call, without side effects if it's not.
	
	we're looking for a dotted id lead by a bracked expression whose (string)
	value looks like a simple url
	*/
	
	bigstring bs;
	bigstring bsprotocol;
	bigstring bsserver;
	short port;
	tyvaluerecord val;
	
	while ((**htree).nodetype == dotop)
		htree = (**htree).param1;
	
	if ((**htree).nodetype != bracketop)
		return (false);
	
	htree = (**htree).param1;
	
	if (!evaluatereadonlyparam (htree, &val))
		return (false);

	pullstringvalue (&val, bs);

	return (parseremotefunction (bs, bsprotocol, bsserver, &port));
	} /*langisremotefunction*/


boolean langremotefunctioncall (hdltreenode htree, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	5.0.2b8 dmb: we assume that htree has already been examined by langisremotefunction,
	or has been constructed according to its definition of a remote function call tree.

	1. Decompile the call and construct the parameters
		The code tree from ["xmlrpc://127.0.0.1:5335/RPC2"].radio.helloworld ("Dave") does not have the string "radio.helloworld" in it.
		Walk the code tree and contruct that part of the text that created it.
	2. Create a new param list, with the original param list ( {"Dave"} as the second parameter, the procedureName from step 1 ( "radio.helloWorld" ) the first parameter.
	3. Given the <b>bsprotocol</b>, look for a script at user.remoteCallers.[bsprotocol].
	4. if none found, look for a script at Frontier.remoteCallers.[bsprotocol].
	5. Make a param list of { server, procedureName, params } and call the script.
	*/
	
	bigstring bs;
	bigstring bsprotocol;
	bigstring bsserver;
	bigstring bsfunction;
	hdlhashtable hremotetable;
	hdltreenode hcode;
	short port;

	setemptystring (bsfunction);
	
	while ((**htree).nodetype == dotop) { // build the function name from the dotted id
	
		if (!langgetidentifier ((**htree).param2, bs))
			return (false);
		
		if (isemptystring (bsfunction))
			copystring (bs, bsfunction);
		
		else {
			insertchar ('.', bsfunction);
			
			insertstring (bs, bsfunction);
			}
		
		htree = (**htree).param1;
		}
	
	if (!langgetidentifier (htree, bs))
		return (false);
	
	if (!parseremotefunction (bs, bsprotocol, bsserver, &port))
		return (false);
	
	if (!findprotocolhandler (bsprotocol, &hremotetable, &hcode)) {
	
		parsedialogstring (STR_unknown_protocol, bsfunction, bsprotocol, nil, nil, bs);
		
		langerrormessage (bs);
		
		return (false);
		}
	
	return (callprotocolhandler (hremotetable, bsprotocol, hcode, bs, bsfunction, hparam1, vreturned));
	} /*langremotefunctioncall*/


static boolean newxmltoken (xmltoken *token) {
	
	clearbytes (token, sizeof (xmltoken));
	
	return (true);
	} /*newxmltoken*/


static void disposexmltoken (xmltoken *token) {
	
	disposehandle ((*token).tokenstring);
	
	//disposehashtable ((*token).atts, false);
	if ((*token).atts != nil)
		disposevaluerecord ((*token).attsvalue, false);
	
	clearbytes (token, sizeof (xmltoken));
	} /*disposexmltoken*/


static void assignxmltoken (xmltoken *totoken, xmltoken *fromtoken) {

	disposexmltoken (totoken); // collect garbage

	*totoken = *fromtoken;
	
	clearbytes (fromtoken, sizeof (*fromtoken)); // doesn't own the data anymore
	} /*assignxmltoken*/


static boolean lookupstringvalue (hdlhashtable ht, const bigstring bs, bigstring value) {
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!hashtablelookup (ht, bs, &val, &hnode))
		return (false);
	
	assert (val.valuetype == stringvaluetype);
	
	pullstringvalue (&val, value);
	
	return (true);
	} /*lookupstringvalue*/


/*static boolean assignstringvalue (hdlhashtable ht, const bigstring bs, const bigstring value) {
	
	tyvaluerecord val;
	
	if (!setstringvalue ((ptrstring) value, &val))
		return (false);
	
	if (!hashtableassign (ht, bs, val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	return (true);
	} /%assignstringvalue%/
*/

static boolean assigntokenstringvalue (hdlhashtable ht, const bigstring bs, xmltoken *value) {
	
	/*
	assign the token's tokenstring to a table cell, consuming the token
	
	5.1.3 dmb: "compile" &amp; and &lt; as we assign to table
	*/
	
	tyvaluerecord val;
	boolean fl;
	
	fl = setheapvalue ((*value).tokenstring, stringvaluetype, &val);
	
	(*value).tokenstring = nil; // it's been consumed
	
	if (!fl)
		return (false);
	
	/*
	if (!(*value).isCDATA) {
		
		if (!replaceallinhandle ("\x05" "&amp;", "\x01" "&", val.data.stringvalue))
			return (false);
		
		if (!replaceallinhandle ("\x04" "&lt;", "\x01" "<", val.data.stringvalue))
			return (false);
		
		if (!replaceallinhandle ("\x06" "]]&gt;", "\x03" "]]>", val.data.stringvalue))
			return (false);
		}
	*/
	
	if (!hashtableassign (ht, bs, val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	return (true);
	} /*assigntokenstringvalue*/


static boolean createtokentable (xmladdress *adrtable, xmltoken *token, hdlhashtable *newtable) {
	
	/*
	create a new table at the given xmladdress. if the token has attributes, 
	move them into the new table
	*/
	
	if (!langassignnewtablevalue ((*adrtable).ht, (*adrtable).bs, newtable))
		return (false);
	
	if ((*token).atts != nil) {
		
		if (!hashtableassign (*newtable, STR_atts, (*token).attsvalue))
			return (false);
		
		(*token).atts = nil; // token doesn't own the table anymore
		}
	
	return (true);
	} /*createtokentable*/


/*
static boolean skipblanks (Handle s) {
	
	/%
	on skipBlanks () {
		loop {
			if sizeof (s) == 0 {
				break};
			if s [1] != ' ' {
				break};
			s = string.delete (s, 1, 1)}};
	%/
	
	long ix, ct = gethandlesize (s);
	
	for (ix = 0; (ix < ct) && ((*s) [ix] == chspace); ++ix)
		;
	
	pullfromhandle (s, 0, ix, nil);
	} /%skipblanks%/
*/

static boolean namenomad (xmladdress *nomad, bigstring name) {
	
	/*
	on namenomad () { //return the name of the current table, without serialization
		return (string.nthfield (nameof (nomad^), '\t', 2))};
	*/
	
	return (nthword ((*nomad).bs, 2, '\t', name));
	} /*namenomad*/


#define serialinterval 1000

static long nextserialnum (hdlhashtable ht) {
	
	hdlhashnode hn = (**ht).hfirstsort;
	hdlhashnode hnext;
	bigstring bs;
	long ct = 0, n;
	
	if (hn == nil)
		return (0);
	
	while (true) {
		
		++ct;
		
		hnext = (**hn).sortedlink;
		
		if (hnext == nil)
			break;
		
		hn = hnext;
		}
	
	gethashkey (hn, bs);
	
	firstword (bs, '\t', bs);
	
	stringtonumber (bs, &n);
	
	return (max (n / serialinterval, ct));
	} /*nextserialnum*/
	
	
static boolean serialstring (hdlhashtable ht, bigstring serializedname) {
	
	/*
	on serialstring () { //return a serial number string for an about-to-be-created object
		return (string.padwithzeros (sizeof (nomad^) + 1, 4) + "\t")};
	
	5.1.3 dmb: make serial strings 8 digits, with 3 trailing zeros
	*/
	
	long ct;
	
	//hashcountitems (ht, &ct);
	ct = nextserialnum (ht);
	
	numbertostring (ct + 1, serializedname);
	
	while (stringlength (serializedname) < 5)
		insertchar ('0', serializedname);
	
	pushstring (BIGSTRING ("\x04" "000\t"), serializedname);
	
	return (true);
	} /*serialstring*/


static void getnewitemaddress (hdlhashtable ht, bigstring bs, xmladdress *adr) {

	/*
	on newitemaddress () { //this code was turning up all over
		return (@nomad^.[serialstring () + token.tokenstring])};
	*/
	
	(*adr).ht = ht;
	
	serialstring (ht, (*adr).bs);
	
	pushstring (bs, (*adr).bs);
	} /*getnewitemaddress*/


static boolean assignemptytag (hdlhashtable htable, bigstring bstoken, xmltoken *tagtoken) {

	//	if defined (token.atts)
	//		new (tabletype, adrnewitem)
	//		adrnewitem^.["/contents"] = ""
	//		adrnewitem^.["/atts"] = token.atts
	//	else
	//		adrnewitem^ = ""
	
	xmladdress adrnewitem;
	hdlhashtable newitemtable;
	
	getnewitemaddress (htable, bstoken, &adrnewitem);
	
	if ((*tagtoken).atts != nil) {
		
		if (!createtokentable (&adrnewitem, tagtoken, &newitemtable))
			return (false);
		
		if (!langassignstringvalue (newitemtable, STR_pcdata, zerostring))
			return (false);
		}
	else {
		if (!langassignstringvalue (adrnewitem.ht, adrnewitem.bs, zerostring))
			return (false);
		}
	
	return (true);
	} /*assignemptytag*/
					

static boolean assignstringtag (hdlhashtable htable, bigstring bstoken, xmltoken *tagtoken, xmltoken *elementtoken) {
	
	xmladdress adrnewitem;
	hdlhashtable newitemtable;
	
	// local (adrnewitem = newitemaddress ())
	getnewitemaddress (htable, bstoken, &adrnewitem);
	
	// if defined (token.atts)
	if ((*tagtoken).atts != nil) {
	
		// new (tabletype, adrnewitem)
		if (!createtokentable (&adrnewitem, tagtoken, &newitemtable))
			return (false);
		
		// adrnewitem^.["/contents"] = lookaheadtoken.tokenstring
		if ((*elementtoken).isCDATA) {
			
			if (!assigntokenstringvalue (newitemtable, STR_cdata, elementtoken))
				return (false);
			}
		else {
			if (!assigntokenstringvalue (newitemtable, STR_pcdata, elementtoken))
				return (false);
			}
		}
	else {
		// adrnewitem^ = lookaheadtoken.tokenstring;
		if (!assigntokenstringvalue (adrnewitem.ht, adrnewitem.bs, elementtoken))
			return (false);
		}
	
	return (true);
	} /*assignstringtag*/


static void langparamerrormessage (short num, const bigstring bs, const bigstring bs1, long pos) {
	
	/*
	5.1.3 dmb: take and report error position
	*/
	
	bigstring bs2, bs3, bspos;
	
	parsedialogstring (bs, (ptrstring) bs1, nil, nil, nil, bs2);
	
	getstringlist (langerrorlist, num, bs3);
	
	parsedialogstring (bs3, bs2, nil, nil, nil, bs3);
	
	numbertostring (pos, bspos);
	
	parsedialogstring (STR_atcharacteroffset, bspos, nil, nil, nil, bs2);
	
	pushstring (bs2, bs3);
	
	langerrormessage (bs3);
	} /*langparamerrormessage*/


static void push2digitnum (int n, bigstring s) {

	bigstring bsint;
	
	numbertostring (n, bsint);
	
	if (stringlength (bsint) == 1)
		insertchar ('0', bsint);
	
	pushstring (bsint, s);
	} /*push2digitnum*/


static void getiso8601datetimestring (unsigned long secs, bigstring bs) {
	
	/*
	6.1b2 AR: Return seconds as a two-digit number, too.
	*/

	//	return (string (year) + string.padWithZeros(month, 2) + string.padWithZeros(day, 2) +"T"+ string.padWithZeros(hour, 2)+":"+ string.padWithZeros(minute, 2)+":"+second;
	
	short day, month, year, hour, minute, second;
	
	secondstodatetime (secs, &day, &month, &year, &hour, &minute, &second);
	
	shorttostring (year, bs);
	push2digitnum (month, bs);
	push2digitnum (day, bs);
	
	pushchar ('T', bs);
	
	push2digitnum (hour, bs);
	pushchar (':', bs);
	
	push2digitnum (minute, bs);
	pushchar (':', bs);

	//pushint (second, bs);
	push2digitnum (second, bs);
	} /*getiso8601datetimestring*/


static void setiso8601datetimestring (bigstring bsiso8601, unsigned long *secs) {

	//	19980616T09:54:52
	
	short day, month, year, hour, minute, second;
	bigstring bs;
	
	midstring (bsiso8601, 1, 4, bs);
	stringtoshort (bs, &year);
	
	midstring (bsiso8601, 5, 2, bs);
	stringtoshort (bs, &month);
	
	midstring (bsiso8601, 7, 2, bs);
	stringtoshort (bs, &day);
	
	midstring (bsiso8601, 10, 2, bs);
	stringtoshort (bs, &hour);
	
	midstring (bsiso8601, 13, 2, bs);
	stringtoshort (bs, &minute);
	
	midstring (bsiso8601, 16, 2, bs);
	stringtoshort (bs, &second);
	
	*secs = datetimetoseconds (day, month, year, hour, minute, second);
	} /*setiso8601datetimestring*/


#define scriptError(n, s, x, pos) do {langparamerrormessage (n, s, x, pos); goto exit;} while (0)


static boolean findinhandlestream (handlestream *s, bigstring bsfind, boolean flunicase) {
	
	byte *p = (byte *) (*(*s).data);
	long ix;
	
	ix = textpatternmatch (p + (*s).pos, (*s).eof - (*s).pos, bsfind, flunicase);
	
	if (ix < 0)
		return (false);
	
	(*s).pos += ix;
	
	return (true);
	} /*findinhandlestream*/


static boolean athandlestreamstring (handlestream *s, bigstring bs) {
	
	/*
	5.1.4 dmb: return true if the next bytes of the stream match bs
	*/
	
	bigstring bstemp;
	long ixtemp = (*s).pos;
	
	if (loadfromhandle ((*s).data, &ixtemp, stringlength (bs), stringbaseaddress (bstemp))) {
		
		setstringlength (bstemp, stringlength (bs));
		
		return (equalidentifiers (bstemp, bs));
		}
		
	return (false);
	} /*athandlestreamstring*/


static boolean getnexttoken (handlestream *source, hdlhashtable namespaces, xmltoken *adrtoken) {
	
	/*
	on getNextToken (adrtoken) {
	
	5.1.3 dmb: accept single or double quoted strings; ignore !DOCTYPE tags
	
	5.1.4 dmb: smarter doctype, comment and processing instruction parsing; handle
	any whitespace, not just chspace.

	6.2a9 AR: Decode &lt; before &amp; and pop trailing whitespace from attribute names
	
	6.2b10 AR: Decode &gt; in CDATA sections
	*/
	
	handlestream *x = source;
	Handle tokenstring = nil;
	boolean fl = false;
	byte bsendtag [16];
	#undef getchar
	#define getchar(x) ((*(*x).data) [(*x).pos])
	#define nextchar(x) ((*(*x).data) [(*x).pos+1])
	#define getnextchar(x) ((*(*x).data) [++(*x).pos])
	
	disposexmltoken (adrtoken);
	
	while (true) { //skip white space
		byte ch;
		
		if ((*x).pos >= (*x).eof)
			return (false);
		
		ch = getchar (x);
		
		if ((ch != '\r') && (ch != '\n') && (ch != '\t') && (ch != ' '))
			break;
		
		++(*x).pos;
		}
	
	(*adrtoken).pos = (*x).pos;
	
	if (getchar (x) == '<') {
		long ixstart, ixtemp;
		bigstring bstag;
		byte ch;
		
		if ((*x).pos == (*x).eof)
			scriptError (badxmltexterror, STR_cantendwithLT, nil, (*x).pos);
		
		(*adrtoken).isTag = true;
		
		copystring (STR_endtag, bsendtag);
		
		ch = nextchar(x);
		
		if (ch == '/') {
			
			(*adrtoken).openTag = false;
			
			++(*x).pos;
			}
		else {
			
			(*adrtoken).openTag = true;
			
			ixtemp = (*x).pos;
			
			if (athandlestreamstring (x, STR_startCDATA)) {
				
				(*adrtoken).isTag = false;
				
				(*adrtoken).isCDATA = true;
				
				(*x).pos += stringlength (STR_startCDATA) - 1; // we've already read the 1st char
				
				copystring (STR_endCDATA, bsendtag);
				}
			else if (athandlestreamstring (x, STR_startcomment)) {
				
				//(*adrtoken).isTag = false;
				
				(*adrtoken).isComment = true;
				
				(*x).pos += stringlength (STR_startcomment) - 1;
				
				copystring (STR_endcomment, bsendtag);
				}
			else if (athandlestreamstring (x, STR_startdoctype)) {
				
				(*adrtoken).isDoctype = true;
				
				(*x).pos += stringlength (STR_startdoctype) - 1;
				}
			}
		
		ixstart = (*x).pos + 1;
		
		//find the end of the tag
		if (!findinhandlestream (x, bsendtag, false))
			scriptError (badxmltexterror, STR_didntfindGTafterLT, nil, (*adrtoken).pos);
		
		ixtemp = ixstart;
		
		if (!loadfromhandletohandle ((*x).data, &ixtemp, (*x).pos - ixstart, true, &tokenstring))
			goto exit;
		
		(*x).pos += stringlength (bsendtag); //point past the ">" or "]]>"
		
		if ((*adrtoken).isCDATA) { // take it literally and return
		
			(*adrtoken).tokenstring = tokenstring;
			
			return (true);
			}
		
		//if (handlebeginswith (tokenstring, "\x03" "!--") && handleendswith (tokenstring, "\x02" "--")) 
		if ((*adrtoken).isComment) {
			
			/*
			disposehandle (tokenstring);
			
			return (getnexttoken (source, namespaces, adrtoken)); //recurse
			*/
			
			(*adrtoken).isComment = true;
			
			(*adrtoken).tokenstring = tokenstring;
			
			return (true);
			}
		
		if ((*adrtoken).isDoctype) { //it's for validating parsers
			
			if (handlecontains (tokenstring, BIGSTRING ("\x01" "[")) && !handlecontains (tokenstring, BIGSTRING ("\x01" "]"))) { // didn't read far enough, 
			
				// skip to end of [decls]
				if (!findinhandlestream (x, BIGSTRING ("\x01" "]"), false))
					scriptError (badxmltexterror, STR_didntfindGTafterLT, nil, (*adrtoken).pos);
				
				// now skip to true end of tag
				if (!findinhandlestream (x, bsendtag, false))
					scriptError (badxmltexterror, STR_didntfindGTafterLT, nil, (*adrtoken).pos);
				
				// rebuild the token handle
				disposehandle (tokenstring);
				
				ixtemp = ixstart;
				
				if (!loadfromhandletohandle ((*x).data, &ixtemp, (*x).pos - ixstart, true, &tokenstring))
					goto exit;
				
				(*x).pos += stringlength (bsendtag); //point past the ">"
				}
			
			(*adrtoken).tokenstring = tokenstring;
			
			return (true);
			}
		
		texthandletostring (tokenstring, bstag); // tags should be < 255 chars
		
		if (breakatfirstwhitespacechar (bstag)) { // has attributes, or is a processing instruction
		
			handlestream s;
			
			if (!tablenewtablevalue (&(*adrtoken).atts, &(*adrtoken).attsvalue))
				goto exit;
			
			if (!newtexthandle (bstag, &(*adrtoken).tokenstring))
				goto exit;
			
			openhandlestream (tokenstring, &s);
			pullfromhandlestream (&s, stringlength (bstag), nil);
			
			if (getstringcharacter (bstag, 0) == '?') {
				
				if (lasthandlestreamcharacter (&s) != '?')
					scriptError (badxmltexterror, STR_tagmustbeofform, nil, (*adrtoken).pos);
				
				(*adrtoken).isPI = true;
				
				s.eof--;
				}
			
			if (lasthandlestreamcharacter (&s) == '/') {
			
				pushtexthandle (BIGSTRING ("\x01" "/"), (*adrtoken).tokenstring);
				
				s.eof--;
				}
			
			while (true) {
				bigstring attname;
				byte chquote;
				
				skiphandlestreamwhitespace (&s);
				
				if (athandlestreameof (&s))
					break;
				
				readhandlestreamfield (&s, '=', attname);
				
				if (isemptystring (attname))
					break;
				
				poptrailingwhitespace (attname); /*6.2a9 AR*/
				
				skiphandlestreamwhitespace (&s);
				
				if ((*adrtoken).isPI && athandlestreameof (&s)) { // a processing instruction w/out assignment

					if (!langassignstringvalue ((*adrtoken).atts, STR_pi, attname))
						goto exit;
					}
				else {
					Handle hattval;
					
					chquote = nexthandlestreamcharacter (&s);
					
					if (chquote != '"' && chquote != '\'')
						scriptError (badxmltexterror, STR_improperlyformatted, nil, (*adrtoken).pos + stringlength (bstag) + s.pos);
					
					s.pos++;
					
					if (!readhandlestreamfieldtohandle (&s, chquote, &hattval))
						goto exit;
					
					if (gethandlestreamcharacter (&s, s.pos - 1) != chquote) {
					
						disposehandle (hattval);
						
						scriptError (badxmltexterror, STR_improperlyformatted, nil, (*adrtoken).pos + s.pos);
						}

					if (!langassigntextvalue ((*adrtoken).atts, attname, hattval))
						goto exit;
					}
				}
			}
	
		else {
			(*adrtoken).tokenstring = tokenstring;
			tokenstring = nil;
			}
		}
	else {
		long ixstart = (*x).pos;
		
		(*adrtoken).isTag = false;
		
		while (true) {
			if (getchar (x) == '<') {
				
				if (!loadfromhandletohandle ((*x).data, &ixstart, (*x).pos - ixstart, true, &(*adrtoken).tokenstring))
					goto exit;
				
				break;
				}
			
			if ((*x).pos++ == (*x).eof)
				scriptError (badxmltexterror, STR_itmustendwithtag, nil, (*x).pos);
			}
		}
	
	if ((*adrtoken).isTag) { //check tokenstring for namespaces
	
		if (handlecontains ((*adrtoken).tokenstring, BIGSTRING ("\x01" ":")) && !handlebeginswith ((*adrtoken).tokenstring, BIGSTRING ("\x01" "?"))) {
		
			bigstring shortname, longname;
			
			texthandletostring ((*adrtoken).tokenstring, shortname);
			
			firstword (shortname, ':', shortname);
			
			copystring (shortname, longname);
			
			pushchar (':', longname);
			
			lookupstringvalue (namespaces, shortname, longname);
		
			if ((*adrtoken).atts == nil) {
				
				if (!tablenewtablevalue (&(*adrtoken).atts, &(*adrtoken).attsvalue))
					goto exit;
				}
			
			if (!langassignstringvalue ((*adrtoken).atts, STR_namespace, longname))
				goto exit;
			
			bundle {
				Handle h = (*adrtoken).tokenstring;
				long ctbytes = gethandlesize ((*adrtoken).tokenstring);
				long ixword, lenword;
			
				if (!textnthword ((ptrbyte)(*h), ctbytes, 2, ':', true, &ixword, &lenword))
					sethandlesize (h, 0);
				else {
					
					popfromhandle (h, ctbytes - (ixword + lenword), nil);
					
					pullfromhandle (h, 0, ixword, nil);
					}
				}
			}
		
		if (!replaceallinhandle (BIGSTRING ("\x04" "&gt;"), BIGSTRING ("\x01" ">"), (*adrtoken).tokenstring))
			return (false);
		}
	else { //not a tag
		
		trimtrailingwhitespace ((*adrtoken).tokenstring);
		
		if (!replaceallinhandle (BIGSTRING ("\x06" "]]&gt;"), BIGSTRING ("\x03" "]]>"), (*adrtoken).tokenstring))
			return (false);
		}

	if (!(*adrtoken).isCDATA) { // always true; we bail early for cdata
		
		if (!replaceallinhandle (BIGSTRING ("\x04" "&lt;"), BIGSTRING ("\x01" "<"), (*adrtoken).tokenstring))
			return (false);
		
		if (!replaceallinhandle (BIGSTRING ("\x04" "&gt;"), BIGSTRING ("\x01" ">"), (*adrtoken).tokenstring))
			return (false);

		if (!replaceallinhandle (BIGSTRING ("\x05" "&amp;"), BIGSTRING ("\x01" "&"), (*adrtoken).tokenstring))
			return (false);
		}
	
	fl = true;
	
	exit:
		disposehandle (tokenstring);
		
		return (fl);
	} /*getnexttoken*/


#define compileall

boolean xmlcompile (Handle htext, xmladdress *xmladr) {
	
	/*
	on compile (htext, adrtable) {
		ÇAn XML compiler running in Frontier 5
			ÇOriginal code, Dave Winer, 12/4/97
			ÇRewritten, Dave Winer, 3/19/98
		
		5.0.2b8 dmb: coded in C

		5.1 dmb: fixed memory leak
		
		5.1.3 dmb: handle cdata; multiple anomynous elements; fixed boolean bug
		
		5.1.4 dmb: smarter doctype, comment and processing instruction handling; 
		ready to create structure
		
		7.0b21 PBS: no longer static -- used by op<-->xml routines.
	*/
	
	handlestream source;
	byte xmlversion [16] = "\x03" "0.0";
	xmladdress nomad = *xmladr;
	xmladdress adrnewitem;
	xmltoken token = {0}, lookaheadtoken = {0}, closetoken = {0};
	bigstring bstoken;
	boolean reuselookahead = false;
	hdlhashtable namespaces = NULL;
	boolean fl = false; //success?
	//hdlhashtable newitemtable;
	hdlhashtable nomadtable;
	hdlhashnode hnode;
	//long ixsleep = 0;
	long lastsleeptickcount = gettickcount ();
	long ticksbetweensleeps = 60; /*one second*/
	long tickcount;
	
	openhandlestream (htext, &source);
	
	if (!langassignnewtablevalue ((*xmladr).ht, (*xmladr).bs, &nomadtable))
		return (false);
	
	assert ((**nomadtable).parenthashtable == (*xmladr).ht);
	
	if (!newxmltoken (&token) || !newxmltoken (&lookaheadtoken) || !newxmltoken (&closetoken))
		goto exit;
	
	if (!newhashtable (&namespaces))
		goto exit;
	
	while (true) {

		if (!inmainthread () && !debuggingcurrentprocess ()) { /*PBS 7.1b25: sleep some, once a second*/

			tickcount = gettickcount ();

			if (tickcount >= lastsleeptickcount + ticksbetweensleeps) {

				lastsleeptickcount = tickcount;

				processsleep (getcurrentthread (), 0);
				} /*if*/
			} /*if*/

		if (reuselookahead) {
			
			assignxmltoken (&token, &lookaheadtoken); // token = lookaheadtoken
			
			reuselookahead = false;
			}
		else {
			// if not getNextToken (@token)
			if (!getnexttoken (&source, namespaces, &token)) { //no more tokens left, parsing is finished
				
				fl = true;
				
				break;
				}
			};
		
		texthandletostring (token.tokenstring, bstoken);
		
		if (token.isPI) {
			
			if (equalidentifiers (bstoken, STR_xmldecl))
				lookupstringvalue (token.atts, STR_version, xmlversion);
			
			else if (equalidentifiers (bstoken, STR_xmlnamespace)) { // "?xml:namespace"
				
				bigstring prefix, ns;
			
				// namespaces.[token.atts.prefix] = token.atts.ns
				if (lookupstringvalue (token.atts, STR_prefix, prefix) && lookupstringvalue (token.atts, BIGSTRING ("\x02" "ns"), ns)) {
				
					if (!langassignstringvalue (namespaces, prefix, ns))
						goto exit;
					}
				else
					scriptError (missingxmlattributeserror, zerostring, nil, token.pos);
				}
			
			#ifdef compileall
			getnewitemaddress (nomadtable, bstoken, &adrnewitem);
			
			if (!hashtableassign (adrnewitem.ht, adrnewitem.bs, token.attsvalue))
				return (false);
			
			token.atts = nil; // token doesn't own the table anymore
			#endif
			
			continue;
			}
		
		if (token.isDoctype) {
			
			#ifdef compileall
			getnewitemaddress (nomadtable, STR_doctype, &adrnewitem);
			
			if (!assigntokenstringvalue (adrnewitem.ht, adrnewitem.bs, &token))
				return (false);
			#endif
			
			continue;
			}
		
		if (token.isComment) {
			
			#ifdef compileall
			getnewitemaddress (nomadtable, STR_comment, &adrnewitem);
			
			if (!assigntokenstringvalue (adrnewitem.ht, adrnewitem.bs, &token))
				return (false);
			#endif
			
			continue;
			}
		
		if (token.isTag) {
			
			// if token.tokenstring beginswith '?'
			// assert (token.isPI == (getstringcharacter (bstoken, 0) == '?'));
			
			if (token.openTag) {
				
				if (lastchar (bstoken) == '/') { //self-contained empty tag, like <hello/>
					
					setstringlength (bstoken, stringlength (bstoken) - 1);
					
					if (!assignemptytag (nomadtable, bstoken, &token))
						goto exit;
					
					assert (reuselookahead == false); // dmb: shouldn't need this anymore
					}
						
				else {
					if (!getnexttoken (&source, namespaces, &lookaheadtoken))
						scriptError (badxmltexterror, STR_itcantendontag, nil, token.pos);
					
					if (lookaheadtoken.isTag) {
						
						if (lookaheadtoken.openTag) { //create a sub-table
							
							// nomad = newitemaddress ()
							getnewitemaddress (nomadtable, bstoken, &nomad);
							
							// new (tabletype, nomad)
							if (!createtokentable (&nomad, &token, &nomadtable))
								goto exit;
							
							assert ((**nomadtable).parenthashtable == nomad.ht);
							
							reuselookahead = true;
							}
				
						else { // create an empty tag
						
							// if lookaheadtoken.tokenstring != token.tokenstring
							if (!equalhandles (lookaheadtoken.tokenstring, token.tokenstring))
								scriptError (badxmltexterror, STR_wewereexpecting, bstoken, lookaheadtoken.pos);
							
							// newitemaddress ()^ = ""
							if (!assignemptytag (nomadtable, bstoken, &token))
								goto exit;
							
							assert (reuselookahead == false); // dmb: shouldn't need this anymore
							}
						}
					else { // lookahead is not a tag
						
						if (!getnexttoken (&source, namespaces, &closetoken))
							scriptError (badxmltexterror, STR_itmustendwithtag, nil, source.pos);
						
					///	if (!closetoken.isTag || closetoken.openTag || !equalhandles (closetoken.tokenstring, token.tokenstring))
					///		scriptError (badxmltexterror, STR_wewereexpecting, bstoken, closetoken.pos);
					
						if (!closetoken.isTag || closetoken.openTag) { // not closing, must add lookahead and reuse closetoken
						
							getnewitemaddress (nomadtable, bstoken, &nomad);
							
							if (!createtokentable (&nomad, &token, &nomadtable))
								goto exit;
							
							if (lookaheadtoken.isCDATA)
								getnewitemaddress (nomadtable, STR_cdata, &adrnewitem);
							else
								getnewitemaddress (nomadtable, STR_pcdata, &adrnewitem);
							
							if (!assigntokenstringvalue (adrnewitem.ht, adrnewitem.bs, &lookaheadtoken))
								goto exit;
							
							assignxmltoken (&lookaheadtoken, &closetoken);
							
							reuselookahead = true;
							}
						else { // closing, must validate

							if (!equalhandles (closetoken.tokenstring, token.tokenstring))
								scriptError (badxmltexterror, STR_wewereexpecting, bstoken, closetoken.pos);
							
							if (!assignstringtag (nomadtable, bstoken, &token, &lookaheadtoken))
								goto exit;
							
							reuselookahead = false; // dmb: shouldn't need this anymore
							}
						}
					}
				}
			else { // it's a close token, closing out a table
				
				bigstring bsnomad;
				long ctitems;				
				
				//if (!inmainthread () && !debuggingcurrentprocess ()) { /*PBS 7.1b14: give up some time to other threads periodically.*/

				/*	ixsleep++;

					if (ixsleep == 500) {

						ixsleep = 0;
					
						processsleep (getcurrentthread (), 0);
						} /%if%/
					} /%if%/
				*/
				
				// if nomad == adrtable
				if (nomad.ht == (*xmladr).ht && equalstrings (nomad.bs, (*xmladr).bs))
					scriptError (badxmltexterror, STR_toomanytags, nil, token.pos);
				
				// if namenomad () != token.tokenstring
				namenomad (&nomad, bsnomad);
				
				if (!equalstrings (bsnomad, bstoken))
					scriptError (badxmltexterror, STR_wewereexpectingtag, bstoken, token.pos);
				
				//if sizeOf (nomad^) == 1 //has one sub-item, maybe a special name?
				//	ÇSee //http://www.microsoft.com/standards/xml/xmldata.htm#ComplexTypes
				//	local (adrsubitem, val, flspecialname = false, namesubitem)
				//	adrsubitem = @nomad^ [1]
				//	namesubitem = string.nthfield (nameof (adrsubitem^), '\t', 2)
				
				if (hashcountitems (nomadtable, &ctitems) && (ctitems == 1)) {
					xmladdress adrsubitem;
					tyvaluerecord val;
					//boolean flspecialname = false;
					bigstring namesubitem;
					tyvaluetype specialtype;
					
					adrsubitem.ht = nomadtable;
					gethashkey ((**nomadtable).hfirstsort, adrsubitem.bs);
					
					namenomad (&adrsubitem, namesubitem);
					
					//case namesubitem //is it a special name?
					//	"boolean"
					//		val = boolean (adrsubitem^)
					//		flspecialname = true
					//	"string"
					//		val = string (adrsubitem^)
					//		flspecialname = true
					//	"int"
					//	"i4"
					//		val = long (adrsubitem^)
					//		flspecialname = true
					//	"i1"
					//	"i2"
					//		val = short (adrsubitem^)
					//		flspecialname = true
					//	"float"
					//	"double"
					//		val = double (adrsubitem^)
					//		flspecialname = true
					
					specialtype = novaluetype;
					
					if (equalstrings (namesubitem, STR_boolean))
						specialtype = booleanvaluetype;
					
					else if (equalstrings (namesubitem, STR_string))
						specialtype = stringvaluetype;
					
					else if (equalstrings (namesubitem, STR_int) || equalstrings (namesubitem, STR_i4))
						specialtype = longvaluetype;
					
					else if (equalstrings (namesubitem, STR_i1) || equalstrings (namesubitem, STR_i2))
						specialtype = intvaluetype;
					
					else if (equalstrings (namesubitem, STR_float) || equalstrings (namesubitem, STR_double))
						specialtype = doublevaluetype;
					
					else if (equalstrings (namesubitem, STR_datetimeiso8601)) {
						bigstring bsiso8601;
						unsigned long secs;
						
						if (!hashtablelookup (adrsubitem.ht, adrsubitem.bs, &val, &hnode))
							goto exit;
						
						pullstringvalue (&val, bsiso8601);
						
						setiso8601datetimestring (bsiso8601, &secs);
						
						setdatevalue (secs, &val);
						
						if (!hashtableassign (nomad.ht, nomad.bs, val))
							goto exit;
						
						exemptfromtmpstack (&val);
						}
					
					if (specialtype != novaluetype) { // do table surgery, nomad changes from an XML table to a native Frontier type
						
						hashtablelookup (adrsubitem.ht, adrsubitem.bs, &val, &hnode);
						
						if (!copyvaluerecord (val, &val))
							goto exit;
						
						if ((specialtype == booleanvaluetype) && (val.valuetype == stringvaluetype)) { // special case for booleans
							
							bigstring bs;
							
							pullstringvalue (&val, bs);
							
							if (isallnumeric (bs))
								if (!coercevalue (&val, longvaluetype))	// make sure that "0" and "0.0", etc. coece to false
									goto exit;
							}
						
						if (!coercevalue (&val, specialtype))
							goto exit;
						
						if (!hashtabledelete (nomad.ht, nomad.bs) || !hashtableassign (nomad.ht, nomad.bs, val))
							goto exit;
						
						exemptfromtmpstack (&val);
						}
					}
				
				// nomad = parentof (nomad^);
				nomadtable = nomad.ht;
				
				findinparenttable (nomadtable, &nomad.ht, nomad.bs);
				}
			}
		else { // not a tag, add text to current table
		
			if (token.isCDATA)
				getnewitemaddress (nomadtable, STR_cdata, &adrnewitem);
			else
				getnewitemaddress (nomadtable, STR_pcdata, &adrnewitem);
			
			if (!assigntokenstringvalue (adrnewitem.ht, adrnewitem.bs, &token))
				goto exit;
			}
		}
	
	exit:
	
	disposexmltoken (&token); disposexmltoken (&lookaheadtoken); disposexmltoken (&closetoken);
	
	if (!fl)
		hashtabledelete ((*xmladr).ht, (*xmladr).bs);
	
	disposehashtable (namespaces, true);
	
	return (fl);
	} /*xmlcompile*/


boolean xmlgetname (bigstring bsname) {
	
	/*
	on getname (adr) {
		local (name = nameof (adr^));
		if name contains "\t" {
			return (string.nthfield (name, "\t", 2))}
		else {
			return (name)}};
	*/
	
	if (stringfindchar ('\t', bsname))
		nthword (bsname, 2, '\t', bsname);
	
	replaceallinstring ('&', BIGSTRING ("\x05" "&amp;"), bsname);
	
	replaceallinstring ('<', BIGSTRING ("\x04" "&lt;"),  bsname);
	
	replaceallinstring ('>', BIGSTRING ("\x04" "&gt;"),  bsname);
	
	return (true);
	} /*xmlgetname*/


static boolean xmlvaltostring (tyvaluerecord xmlval, short indentlevel, boolean fltranslatestrings, Handle *string) {
#pragma unused(indentlevel)

	/*
	on valToString (val, indentlevel=0) { //http://www.microsoft.com/standards/xml/xmldata.htm#ComplexTypes
		ÇReturn an XML-Data representation of a Frontier value
			ÇWe only support a small subset of the types that XML-Data specifies.
			ÇIf we get a type that's not supported, we throw an error.
			ÇThe caller is expected to watch for this, within a try statement.
	
	5.1.3 dmb: don't add <string> tags to string values. it's the default type; added fltranslatestrings
	
	6.2b10 AR: No longer emit <i2> tags -- they never made it into the XML-RPC spec
	*/
	
//	bigstring bsindent;
//	#define add(s) do {filledstring ('\t', indentlevel, bsindent); 

	tyvaluerecord val;
	bigstring bstag, bsval, bsxml;
	
	assert (!xmlval.fldiskval); /*08/04/2000 AR*/

	if (!copyvaluerecord (xmlval, &val))
		return (false);
	
	switch (val.valuetype) {
				
		//	stringType
		//		return ("<string>"+ val + "</string>")
		case stringvaluetype:
			// copystring (STR_string, bstag)
			// break
			
			if (fltranslatestrings) {
				
				if (!replaceallinhandle (BIGSTRING ("\x01" "&"), BIGSTRING ("\x05" "&amp;"), val.data.stringvalue))
					return (false);
				
				if (!replaceallinhandle (BIGSTRING ("\x01" "<"), BIGSTRING ("\x04" "&lt;"), val.data.stringvalue))
					return (false);
				
				if (!replaceallinhandle (BIGSTRING ("\x03" "]]>"), BIGSTRING ("\x06" "]]&gt;"), val.data.stringvalue))
					return (false);
				}
			
			exemptfromtmpstack (&val);
			
			*string = val.data.stringvalue;
			
			return (true);
		
		//	binaryType
		//		return ("<base64>" + base64.encode (val, 0) + "</base64>")
		case binaryvaluetype: {
			Handle h64;
			
			if (!coercetostring (&val))
				return (false);
			
			if (!newemptyhandle (&h64))
				return (false);
			
			if (!base64encodehandle (val.data.stringvalue, h64, 0)
					|| !inserttextinhandle (h64, 0, STR_base64_begin)
					|| !pushtexthandle (STR_base64_end, h64)) {

				disposehandle (h64);
				
				return (false);
				}
			
			*string = h64;
			
			return (true);
			}
		
		//	longType //a 32-bit signed integer
		//		return ("<i4>" + val + "</i4>")
		case intvaluetype:
		case longvaluetype:
			copystring (STR_i4, bstag);
			break;
		
		//	shortType //a 16-bit signed integer
		//		return ("<i2>" + val + "</i2>")
		/*
		case intvaluetype:
			copystring (STR_i2, bstag);
			break;
		*/

		//	doubleType
		//		return ("<double>" + val + "</double>")
		case doublevaluetype:
			copystring (STR_double, bstag);
			break;
		
		//	booleanType
		//		local (bit)
		//		if val
		//			bit = 1
		//		else
		//			bit = 0
		//		return ("<boolean>" + bit + "</boolean>")
		case booleanvaluetype:
			coercetolong (&val); // [force string coercsion to get {1, 0}, not {true, false}]
			copystring (STR_boolean, bstag);
			break;
		
		//	dateType
		//		local
		//			year, month, day, hour, minute, second
		//		date.get (val, @day, @month, @year, @hour, @minute, @second
		//		return ("<dateTime.iso8601>" + string (year) + string.padWithZeros(month, 2) + string.padWithZeros(day, 2) +"T"+ string.padWithZeros(hour, 2)+":"+ string.padWithZeros(minute, 2)+":"+second + "</dateTime.iso8601>")
		case datevaluetype:
			getiso8601datetimestring (val.data.datevalue, bsval);
			
			copystring (STR_datetimeiso8601, bstag);
			
			parsedialogstring (BIGSTRING ("\x0b" "<^0>^1</^0>"), bstag, bsval, nil, nil, bsxml);
			
			return (newtexthandle (bsxml, string));
		
		//	local (s = "Can't process the request because a Frontier value of type \"")
		//	s = s + string (typeof (val)) + "\" can't be represented in XML-Data at this time."
		//	scriptError (s)
		default:
			langgettypestring (val.valuetype, bsval);
			
			langparamerror (frontierxmldatatypeerror, bsval);
			
			return (false);
		}
	
	if (!coercetostring (&val))
		return (false);
	
	insertchar ('<', bstag);
	pushchar ('>', bstag);
	
	if (!newtexthandle (bstag, string))
		return (false);
	
	if (!pushhandle (val.data.stringvalue, *string))
		return (false);
	
	midinsertstring (BIGSTRING ("\x01" "/"), bstag, 2);
	 
	return (pushtexthandle (bstag, *string));
	} /*xmlvaltostring*/


static boolean xmladdstring (bigstring bs, Handle h, short indentlevel, handlestream *xmltext) {
	
	/*
	write _either_ the string or the handle to the stream
	
	on add (s) {
		xmltext = xmltext + string.filledstring ("\t", indentlevel) + s + "\r\n"};
	*/
	
	bigstring bsindent;
	boolean fl;
	
	filledstring ('\t', indentlevel, bsindent);
	
	if (!writehandlestreamstring (xmltext, bsindent))
		return (false);
	
	if (h != nil)
		fl = writehandlestreamhandle (xmltext, h);
	else
		fl = writehandlestreamstring (xmltext, bs);
	
	return (fl && writehandlestreamstring (xmltext, BIGSTRING ("\x02\r\n")));
	} /*xmladdstring*/


boolean gethashnodetable (hdlhashnode hn, hdlhashtable *ht) {

	bigstring bsname;
	
	if (hn && langexternalvaltotable ((**hn).val, ht, hn))
		return (true);
	
	if (hn == nil)
		langgetmiscstring (nilstring, bsname);
	else
		gethashkey (hn, bsname);
	
	langparamerror (cantdecompilerror, bsname);
	
	return (false);
	} /*gethashnodetable*/


static boolean decompilespecialtable (hdlhashtable ht, Handle *hnamevalpairs, bigstring bsspecial, Handle *hspecial, boolean flwantleadingspace) {
	
	/*
	5.1.4 dmb: for /attr and PI tables, decompile all items into name=val expressions, 
	except for the item potentially named bsspecial, whose value is saved separately
	*/
	
	hdlhashnode hn;
	bigstring attname;
	tyvaluerecord attvalue;
	tyvaluerecord specialval = {novaluetype};
	Handle attstring = nil;
	hdldatabaserecord hdb = tablegetdatabase (ht);
	boolean fl;
		
	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
		
		gethashkey (hn, attname);
		
		if (hdb)
			dbpushdatabase (hdb);
		
		fl = copyvaluerecord ((**hn).val, &attvalue) && coercetostring (&attvalue);
		
		if (hdb)
			dbpopdatabase ();

		if (!fl) {
			
			disposehandle (attstring);
			
			return (false);
			}
		
		if (equalidentifiers (attname, bsspecial)) {
			
			specialval = attvalue;
			}
		else {
			pushstring (BIGSTRING ("\x02" "=\""), attname);
			
			if (flwantleadingspace || attstring != nil)
				insertchar (' ', attname);
			
			if (attstring == nil)
				newtexthandle (attname, &attstring);
			else
				pushtexthandle (attname, attstring);
			
			pushhandle (attvalue.data.stringvalue, attstring);
			
			pushtexthandle (BIGSTRING ("\x01" "\""), attstring);
			
			disposevaluerecord (attvalue, false);
			}
		}
	
	*hnamevalpairs = attstring;
	
	if (specialval.valuetype != novaluetype) {
	
		*hspecial = specialval.data.stringvalue; 
		
		exemptfromtmpstack (&specialval);
		}
	
	return (true);
	} /*decompilespecialtable*/

static boolean decompilevisit (hdlhashtable adrtable, bigstring nametable, short indentlevel, handlestream *xmltext) {
	
	/*
	on visit (adrtable) {
	
	5.1.3 dmb: don't prettyprint terminal data nodes; strings no longer have type tags
	handle cdata nodes.
	
	5.1.4 dmb: decompile comments, pis, doctypes
	*/
	
	Handle namespace = nil, attstring = nil, opentag = nil, hadd = nil;
	hdlhashtable adratts;
	hdlhashnode hn;
	hdlhashtable ht;
	bigstring bsadd;
	tyvaluerecord contentsval;
	boolean fl = false;
	
	xmlgetname (nametable);
	
	//	if defined (adratts^) {
	if (hashtablelookupnode (adrtable, STR_atts, &hn)) {
		
		if (!gethashnodetable (hn, &adratts))
			return (false);
		
		//	local (i, ct = sizeof (adratts^), adratt);
		//	for i = 1 to ct {
		//		adratt = @adratts^ [i];
		//		if nameof (adratt^) == "namespace" {
		//			namespace = adratt^}
		//		else {
		//			attstring = attstring + " " + nameof (adratt^) + "=\"" + adratt^ + "\""}}};
		
		#ifdef compileall
		if (!decompilespecialtable (adratts, &attstring, STR_namespace, &namespace, true))
			goto exit;
		#else
		for (hn = (**adratts).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
			
			bigstring attname;
			tyvaluerecord attvalue;
			
			gethashkey (hn, attname);
			
			if (!copyvaluerecord ((**hn).val, &attvalue) || !coercetostring (&attvalue))
				goto exit;
			
			if (equalidentifiers (attname, STR_namespace)) {
				
				namespace = attvalue.data.stringvalue; 
				
				exemptfromtmpstack (&attvalue);
				}
			else {
				insertchar (' ', attname);
				
				pushstring (BIGSTRING ("\x02" "=\""), attname);
				
				if (attstring == nil)
					newtexthandle (attname, &attstring);
				else
					pushtexthandle (attname, attstring);
				
				pushhandle (attvalue.data.stringvalue, attstring);
				
				pushtexthandle (BIGSTRING ("\x01" "\""), attstring);
				
				disposevaluerecord (attvalue, false);
				}
			}
		#endif
		}
		
	//	local (opentag = "<");
	//	if namespace != "" {
	//		opentag = opentag + namespace};
	//	opentag = opentag + nametable + attstring + ">";
	
	newtexthandle (BIGSTRING ("\x01" "<"), &opentag);
	
	if (namespace != nil)
		pushhandle (namespace, opentag);
	
	pushtexthandle (nametable, opentag);
	pushhandle (attstring, opentag);
	pushtexthandle (BIGSTRING ("\x01" ">"), opentag);
	
	//	local (adrcontents = @adrtable^.["/contents"]);
	//	if defined (adrcontents^) {
	//		add (opentag + adrcontents^ + "</" + namespace + nametable + ">");
	//		return};
	
	// if there's just a pcdata subnode, unserialized, merge into single tag
	if (hashtablelookupnode (adrtable, STR_pcdata, &hn) || hashtablelookupnode (adrtable, STR_contents, &hn)) {
		
		if (!copyvaluerecord ((**hn).val, &contentsval) || !coercetostring (&contentsval))
			goto exit;
		
		if (!pushhandle (contentsval.data.stringvalue, opentag))
			goto exit;
		
		disposevaluerecord (contentsval, false);
		
		pushtexthandle (BIGSTRING ("\x02" "</"), opentag);
		
		if (!pushhandle (namespace, opentag))
			goto exit;
		
		pushchar ('>', nametable);
		
		if (!pushtexthandle (nametable, opentag))
			goto exit;
		
		fl = xmladdstring (nil, opentag, indentlevel, xmltext);
		
		goto exit;
		}
	
	//	add (opentag); indentlevel++;
	#ifdef compileall
	if (indentlevel < 0)
		++indentlevel;
	else
	#endif
	if (!xmladdstring (nil, opentag, indentlevel++, xmltext))
		goto exit;
	
	// loop through all of the items in the table
	for (hn = (**adrtable).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
		
		bigstring nameitem;
		Handle valitem;
		boolean flcdata, flpcdata;
		#ifdef compileall
		boolean flcomment, fldoctype;
		#endif
		
		gethashkey (hn, nameitem);
		
		xmlgetname (nameitem);
		
		if (langexternalgettype ((**hn).val) == idtableprocessor) {
		
			switch (getstringcharacter (nameitem, 0)) {
			
				case '/': //one of the special tables, probably /atts
					break;
				
				#ifdef compileall
				case '?': {
					Handle pistring = nil;
					
					gethashnodetable (hn, &ht);
					
					if (!decompilespecialtable (ht, &pistring, STR_pi, &pistring, false))
						goto exit;
					
					/*
					hdlhashnode hpi;
					bigstring piname;
					tyvaluerecord pivalue;
					if (hashtablelookup (ht, STR_pi, &pivalue)) {
						
						if (!copyvaluerecord (pivalue, &pivalue) || !coercetostring (&pivalue))
							goto exit;
						
						exemptfromtmpstack (&pivalue);
						
						pistring = pivalue.data.stringvalue;
						}
					else {
						for (hpi = (**ht).hfirstsort; hpi != nil; hpi = (**hpi).sortedlink) {
							
							gethashkey (hpi, piname);
							
							if (!copyvaluerecord ((**hpi).val, &pivalue) || !coercetostring (&pivalue))
								goto exit;
							
							pushstring ("\x02" "=\"", piname);
							
							if (pistring == nil)
								newtexthandle (piname, &pistring);
								
							else {
								insertchar (chspace, piname);
								
								pushtexthandle (piname, pistring);
								}
							
							pushhandle (pivalue.data.stringvalue, pistring);
							
							pushtexthandle ("\x01" "\"", pistring);
							
							disposevaluerecord (pivalue, false);
							}
						}
					*/
					
					insertchar ('<', nameitem);
					
					pushchar (chspace, nameitem);
					
					fl = insertinhandle (pistring, 0, stringbaseaddress (nameitem), stringlength (nameitem));
					
					if (fl)
						fl = pushtexthandle (BIGSTRING ("\x02" "?>"), pistring);
				
					if (fl)
						fl = xmladdstring (nil, pistring, indentlevel, xmltext);
					
					disposehandle (pistring);
					
					break;
					}
				#endif
				
				default:
					gethashnodetable (hn, &ht);
					
					if (!decompilevisit (ht, nameitem, indentlevel, xmltext))
						goto exit;
				}
			
			fl = true;
			}
		
		// not a table		
		else {
			flcdata = equalidentifiers (nameitem, STR_cdata);
			
			flpcdata = equalidentifiers (nameitem, STR_pcdata);
			
			#ifdef compileall
			flcomment = equalidentifiers (nameitem, STR_comment);
			
			fldoctype = equalidentifiers (nameitem, STR_doctype);
			
			if (!hashresolvevalue (adrtable, hn)) /*08/04/2000 AR*/
				goto exit;
			
			if (!xmlvaltostring ((**hn).val, indentlevel + 1, !flcdata && !flcomment && !fldoctype, &valitem))
				goto exit;
			
			#else
			if (!xmlvaltostring ((**hn).val, indentlevel + 1, !flcdata, &valitem))
				goto exit;
			#endif
			
			if (valitem == nil) {
				
				// add ("<" + nameitem + "/>")
				parsedialogstring (BIGSTRING ("\x05" "<^0/>"), nameitem, nil, nil, nil, bsadd);
				
				fl = xmladdstring (bsadd, nil, indentlevel, xmltext);
				}
			
			else {
				
				if (flcdata) {
				
					// add ("<![CDATA[" + valitem + ">]]")
					fl = insertinhandle (valitem, 0, stringbaseaddress (STR_startCDATA), stringlength (STR_startCDATA));
					
					if (fl)
						fl = pushtexthandle (STR_endCDATA, valitem);
					}
				else if (flpcdata) {
					
					// add (valitem)
					fl = true;
					}
				#ifdef compileall
				else if (fldoctype) {
					fl = insertinhandle (valitem, 0, stringbaseaddress (STR_startdoctype), stringlength (STR_startdoctype));
					
					if (fl)
						fl = pushtexthandle (STR_endtag, valitem);
					}
				else if (flcomment) {
					fl = insertinhandle (valitem, 0, stringbaseaddress (STR_startcomment), stringlength (STR_startcomment));
					
					if (fl)
						fl = pushtexthandle (STR_endcomment, valitem);
					}
				#endif
				else {
				
					// add ("<" + nameitem + ">" + valitem + "</" + nameitem + ">")
					parsedialogstring (BIGSTRING ("\x04" "<^0>"), nameitem, nil, nil, nil, bsadd);
					
					fl = insertinhandle (valitem, 0, stringbaseaddress (bsadd), stringlength (bsadd));
					
					if (fl) {
					
						parsedialogstring (BIGSTRING ("\x05" "</^0>"), nameitem, nil, nil, nil, bsadd);
						
						fl = pushtexthandle (bsadd, valitem);
						}
					}
				
				if (fl)
					fl = xmladdstring (nil, valitem, indentlevel, xmltext);
				
				disposehandle (valitem);
				}
			}
		}
	
	if (!fl)
		goto exit;
		
	//	add ("</" + namespace + nametable + ">"); indentlevel--};
	newtexthandle (BIGSTRING ("\x02" "</"), &hadd);
	pushhandle (namespace, hadd);
	pushstring (BIGSTRING ("\x01" ">"), nametable);
	
	if (!pushtexthandle (nametable, hadd))
		goto exit;
	
	#ifdef compileall
	if (indentlevel <= 0)
		--indentlevel;
	else
	#endif
	fl = xmladdstring (nil, hadd, indentlevel--, xmltext);
	
	exit:
		disposehandle (hadd);
		disposehandle (namespace);
		disposehandle (opentag);
		disposehandle (attstring);
		return (fl);
	} /*decompilevisit*/


static boolean xmldecompile (hdlhashtable hxmltable, Handle *htext) {
	
	/*
	turn a Frontier-table structure into XML text
	
	5.0.2b8 dmb: coded in C
	
	5.1.4 dmb: decompile whole table, not just 1st item in it.
	*/
	
	handlestream xmltext;
	hdlhashnode hn;
	hdlhashtable ht, htoss;
	bigstring tablename;
	short indentlevel = 0;
	boolean fl;
	
	openhandlestream (nil, &xmltext);
	
	#ifdef compileall
		ht = hxmltable;
		
		//see if there's an xml declaration starting the structure
		hn = (**ht).hfirstsort;
		
		if (hn == 0) {		// 9.0.1 JES: don't crash if the table is empty //
			langerrormessage (STR_cant_decompile_empty_table);
			return (false);
			}
		
		gethashkey (hn, tablename);
		
		xmlgetname (tablename);
		
		//if not, add the stock xml decl
		if (!equalidentifiers (tablename, STR_xmldecl))
			if (!xmladdstring (STR_xmlversion, nil, indentlevel, &xmltext))
				return (false);
		
		// decompile our whole table
		if (!findinparenttable (ht, &htoss, tablename))
			return (false);
		
		// if we're at the top of the structure, inhibit outer tags
		if (!stringfindchar (chtab, tablename))
			--indentlevel;
	#else
	// set ht to @adrtable^ [1]
	hn = (**hxmltable).hfirstsort;
	
	if (!gethashnodetable (hn, &ht))
		return (false);
	
	gethashkey (hn, tablename);
	
	if (!xmladdstring (STR_xmlversion, nil, indentlevel, &xmltext))
		return (false);
	#endif
	
	fl = decompilevisit (ht, tablename, indentlevel, &xmltext);
	
	closehandlestream (&xmltext);
	
	*htext = xmltext.data;

	return (fl);
	} /*xmldecompile*/


boolean isxmlmatch (hdlhashnode hn, bigstring name) {
	
	bigstring bs;
	
	gethashkey (hn, bs);
	
	return (nthword (bs, 2, '\t', bs) && equalstrings (bs, name));
	} /*isxmlmatch*/


static void xmlmakenewaddress (hdlhashtable ht, bigstring name) {
	
	/*
		adr = @adrparent^.[string.padwithzeros (sizeof (adrparent^) + 1, 4) + "\t" + name];
	*/
	
	bigstring prefix;
	
	serialstring (ht, prefix);
	
	insertstring (prefix, name);
	} /*xmlmakenewaddress*/


static boolean xmlgetaddress (hdlhashtable ht, bigstring name) {
	
	/*
	on getAddress (adrtable, name) { //return the address of the first object in the table with the indicated name
	*/
	
	hdlhashnode hn;
	
	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
		
		if (isxmlmatch (hn, name)) {
			
			gethashkey (hn, name);
			
			return (true);
			}
		}
	
	langparamerror (cantgetxmladdresserror, name);
	
	return (false);
	} /*xmlgetaddress*/


static boolean xmlgetaddresslist (hdlhashtable ht, bigstring name, boolean justone, hdllistrecord *hlist) {
	
	/*
	on getAddressList (adrtable, commonname, justone=false) { //return a list of all 
	objects with this name in the table
	
	5.1.3 dmb: if ht is nil, just return the empty list
	*/
	
	hdlhashnode hn;
	bigstring bs;
	tyvaluerecord val;
	
	if (!opnewlist (hlist, false))
		return (false);
	
	if (ht == nil)
		return (true);
	
	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
		
		if (isxmlmatch (hn, name)) {
			
			gethashkey (hn, bs);
			
			if (!setaddressvalue (ht, bs, &val))
				goto error;
			
			if (!langpushlistval (*hlist, nil, &val))
				goto error;
			
			disposevaluerecord (val, false);
			
			if (justone)
				break;
			}
		}
	
	return (true);
	
	error:
		opdisposelist (*hlist);
		
		return (false);
	} /*xmlgetaddresslist*/


boolean xmlgetattribute (hdlhashtable ht, bigstring name, hdlhashtable *adratts) {

	/*
	on getAttribute (adrtable, name) { //return the address of the attribute with the indicated name
		local (adratts = @adrtable^.["/atts"]);
		if not defined (adratts^) {
			scriptError ("Can't get the \"" + name + "\" attribute because the table doesn't have a sub-table named /atts.")};
		local (adratt = @adratts^.[name]);
		if not defined (adratt^) {
			scriptError ("Can't get the \"" + name + "\" attribute because the table doesn't an attribute with that name.")};
		return (adratt)}
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!hashtablelookup (ht, STR_atts, &val, &hnode) || !langexternalvaltotable (val, adratts, hnode)) {
		
		langparamerror (noattributestableerror, name);
		
		return (false);
		}
	
	if (!hashtablesymbolexists (*adratts, name)) {
		
		langparamerror (cantfindattributeerror, name);
		
		return (false);
		}
	
	return (true);
	} /*xmlgetattribute*/


static boolean xmlgetpathaddress (tyaddress *xtable, Handle h, tyaddress *adrresult, boolean *flvalid) {
	
	/*
	6.1d5 AR: Kernelized. From the root of the table, travel from the top down
	the /-separated path, e.g. "/ticket/header/title". Set adrresult^ to point
	to the the value at the end of the path. Set *flvalid to true if the path
	is valid.
	*/
	
	tyvaluerecord val;
	tyaddress nomad = *xtable;
	long len = gethandlesize (h);
	long ix;
	long lenwd;
	long ctwd = 1;
	boolean fl;
	hdlhashnode hnode;
	
	*flvalid = false;
	
	if ((*h)[0] == '/') /*handle leading slash*/
		ctwd++;
	
	while (textnthword ((ptrbyte)(*h), len, ctwd++, '/', true, &ix, &lenwd)) {

		disablelangerror ();

		fl = hashtablelookup (nomad.ht, nomad.bs, &val, &hnode);
		
		fl = fl && langexternalvaltotable (val, &nomad.ht, hnode);
		
		enablelangerror ();
		
		if (!fl)
			return (true);

		if (lenwd > lenbigstring)
			lenwd = lenbigstring;
		
		assert ((0 <= ix) && (ix < len) && (ix + lenwd <= len));
		
		setstringlength (nomad.bs, lenwd);

		moveleft (*h + ix, stringbaseaddress (nomad.bs), lenwd);

		disablelangerror ();

		fl = xmlgetaddress (nomad.ht, nomad.bs);
		
		enablelangerror ();
		
		if (!fl)
			return (true);
		
		} /*while*/
		
	*flvalid = true;	

	*adrresult = nomad;
/*
	(*adrresult).ht = nomad.ht;
	
	copystring (nomad.bs, (*adrresult).bs);
*/	
	return (true);
	} /*xmlgetpathaddress*/


	
static boolean xmlconverttodisplayname (Handle h) {

	/*
	6.1d5 AR: Kernelized.
	*/
	
	long len = gethandlesize (h);
	
	if (len > 5 && ((*h)[4] == chtab))
		return (pullfromhandle (h, 0, 5, nil));

	if (len > 9 && ((*h)[8] == chtab))
		return (pullfromhandle (h, 0, 9, nil));
		
	return (true);
	} /*xmlconverttodisplayname*/





static boolean xmlcompileverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.0.2b8 dmb: new verb
		on compile (htext, adrtable) {
	*/
	
	Handle x;
	xmladdress adr;
	
	if (!gettextvalue (hp1, 1, &x))
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hp1, 2, &adr.ht, adr.bs))
		return (false);
	
	if (!xmlcompile (x, &adr))
		return (false);
	
	return (setbooleanvalue (true, v));
	} /*xmlcompileverb*/


static boolean xmldecompileverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.0.2b8 dmb: new verb
	
		on decompile (adrtable)
			return s
	*/
	
	hdlhashtable ht;
	Handle xmltext;
	
	flnextparamislast = true;
	
	if (!gettablevalue (hp1, 1, &ht))
		return (false);
	
	if (!xmldecompile (ht, &xmltext))
		return (false);
	
	return (setheapvalue (xmltext, stringvaluetype, v));
	} /*xmldecompileverb*/


static boolean xmladdtableverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on addTable (adrparent, name) {
		local (adr = @adrparent^.[string.padwithzeros (sizeof (adrparent^) + 1, 4) + "\t" + name]);
		new (tabletype, adr);
		return (adr)}
	*/
	
	hdlhashtable ht, hnew;
	bigstring name;
	
	if (!gettablevalue (hp1, 1, &ht))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hp1, 2, name))
		return (false);
	
	xmlmakenewaddress (ht, name);
	
	if (!langassignnewtablevalue (ht, name, &hnew))
		return (false);
	
	return (setaddressvalue (ht, name, v));
	} /*xmladdtableverb*/


static boolean xmladdvalueverb (hdltreenode hp1, tyvaluerecord *v) {

	/*
	on addValue (adrparent, name, value) {
		local (adr = @adrparent^.[string.padwithzeros (sizeof (adrparent^) + 1, 4) + "\t" + name]);
		adr^ = value;
		return (adr)}
	*/
	
	hdlhashtable ht;
	bigstring name;
	tyvaluerecord val;
	
	if (!gettablevalue (hp1, 1, &ht))
		return (false);
	
	if (!getstringvalue (hp1, 2, name))
		return (false);
	
	flnextparamislast = true;
	
	if (!getparamvalue (hp1, 3, &val))
		return (false);
	
	xmlmakenewaddress (ht, name);
	
	if (!hashtableassign (ht, name, val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	return (setaddressvalue (ht, name, v));
	} /*xmladdvalueverb*/


static boolean xmlgetvalueverb (hdltreenode hp1, tyvaluerecord *v) {

	/*
	on getValue (adrtable, name) { //return the contents of the indicated object
		local (adrobject = xml.getAddress (adrtable, name));
		if typeof (adrobject^) == tabletype {
			try {
				return (adrobject^.["/contents"]^)}};
		return (adrobject^)}
	*/
	
	hdlhashtable ht;
	bigstring name;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!gettablevalue (hp1, 1, &ht))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hp1, 2, name))
		return (false);
	
	if (!xmlgetaddress (ht, name))
		return (false);
	
	hashtablelookup (ht, name, &val, &hnode);
	
	if (langexternalvaltotable (val, &ht, hnode))
		if (!hashtablelookup (ht, STR_pcdata, &val, &hnode))
			hashtablelookup (ht, STR_contents, &val, &hnode);
	
	return (copyvaluerecord (val, v));
	} /*xmlgetvalueverb*/


static boolean xmlgetaddressverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on getAddress (adrtable, name) { //return the address of the first object in the table with the indicated name
		local (addresslist);
		addresslist = xml.getAddressList (adrtable, name, justone:true);
		if sizeof (addresslist) == 0 {
			scriptError ("Can't get the address of \"" + name + "\" because the table doesn't have an object with that name.")};
		return (addresslist [1])}
	*/
	
	hdlhashtable ht;
	bigstring name;
	
	if (!gettablevalue (hp1, 1, &ht))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hp1, 2, name))
		return (false);
	
	if (!xmlgetaddress (ht, name))
		return (false);
	
	return (setaddressvalue (ht, name, v));
	} /*xmlgetaddressverb*/


static boolean xmlgetaddresslistverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on getAddressList (adrtable, commonname, justone=false) { //return a list of all objects with this name in the table
		local (i, ct = sizeof (adrtable^), adritem, nameitem, addresslist = {});
		for i = 1 to ct {
			adritem = @adrtable^ [i];
			nameitem = string.nthfield (nameof (adritem^), '\t', 2);
			if nameitem == commonname {
				addresslist = addresslist + adritem;
				if justone {
					break}}};
		return (addresslist)};
	
	5.1.3 dmb: if the first parameter is the address of a string, not a table, just return the empty list.
	*/
	
	hdlhashtable ht;
	bigstring name;
	tyvaluerecord val;
	boolean justone = false;
	bigstring bserror;
	hdllistrecord hlist;
	hdlhashnode hnode;
	
	//if (!gettablevalue (hp1, 1, &ht))
	//	return (false);
	if (!getvarparam (hp1, 1, &ht, name))
		return (false);
	
	if (!langsymbolreference (ht, name, &val, &hnode))
		return (false);
	
	if (val.valuetype == stringvaluetype)
		ht = nil;
	
	else {
		if (!tablevaltotable (val, &ht, hnode)) {
			
			if (!fllangerror) {
		
				getstringlist (tableerrorlist, namenottableerror, bserror);
				
				langerrormessage (bserror);
				}
			
			return (false);
			}
		}
	
	if (!getstringvalue (hp1, 2, name))
		return (false);
	
	if (langgetparamcount (hp1) > 2) {
	
		flnextparamislast = true;
		
		if (!getbooleanvalue (hp1, 3, &justone))
			return (false);
		}
	
	if (!xmlgetaddresslist (ht, name, justone, &hlist))
		return (false);
	
	return (setheapvalue ((Handle) hlist, listvaluetype, v));
	} /*xmlgetaddresslistverb*/


static boolean xmlgetattributeverb (hdltreenode hp1, tyvaluerecord *v) {

	/*
	on getAttribute (adrtable, name) { //return the address of the attribute with the indicated name
		local (adratts = @adrtable^.["/atts"]);
		if not defined (adratts^) {
			scriptError ("Can't get the \"" + name + "\" attribute because the table doesn't have a sub-table named /atts.")};
		local (adratt = @adratts^.[name]);
		if not defined (adratt^) {
			scriptError ("Can't get the \"" + name + "\" attribute because the table doesn't an attribute with that name.")};
		return (adratt)}
	*/
	
	hdlhashtable ht;
	bigstring name;
	
	if (!gettablevalue (hp1, 1, &ht))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hp1, 2, name))
		return (false);
	
	if (!xmlgetattribute (ht, name, &ht))
		return (false);
	
	return (setaddressvalue (ht, name, v));
	} /*xmlgetattributeverb*/


static boolean xmlgetattributevalueverb (hdltreenode hp1, tyvaluerecord *v) {

	/*
	on getAttributeValue (adrtable, name) { //return the value of the attribute with the indicated name
		return (xml.getAttribute (adrtable, name)^)}
	*/
	
	hdlhashtable ht;
	bigstring name;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!gettablevalue (hp1, 1, &ht))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hp1, 2, name))
		return (false);
	
	if (!xmlgetattribute (ht, name, &ht))
		return (false);
	
	hashtablelookup (ht, name, &val, &hnode);
	
	return (copyvaluerecord (val, v));
	} /*xmlgetattributevalueverb*/


static boolean xmlvaltostringverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on valToString (val, indentlevel=0) { //http://www.microsoft.com/standards/xml/xmldata.htm#ComplexTypes
		ÇReturn an XML-Data representation of a Frontier value
			ÇWe only support a small subset of the types that XML-Data specifies.
			ÇIf we get a type that's not supported, we throw an error.
			ÇThe caller is expected to watch for this, within a try statement.
	*/
	
	tyvaluerecord val;
	long indentlevel = 0;
	Handle htext;
	
	if (!getparamvalue (hp1, 1, &val))
		return (false);
	
	if (langgetparamcount (hp1) > 1) {
		
		flnextparamislast = true;
		
		if (!getlongvalue (hp1, 2, &indentlevel))
			return (false);
		}
	
	if (!xmlvaltostring (val, indentlevel, true, &htext))
		return (false);
	
	return (setheapvalue (htext, stringvaluetype, v));
	} /*xmlvaltostringverb*/


static boolean xmlfrontiervaltotaggedtextverb (hdltreenode hp1, tyvaluerecord *v) {
	
	Handle htext;
	hdlhashtable ht;
	bigstring bs;
	tyvaluerecord val;
	long indentlevel;
	hdlhashnode hnode;
	
	if (!getaddressparam (hp1, 1, &val))
		return (false);
	
	if (!getaddressvalue (val, &ht, bs))
		return (false);

	flnextparamislast = true;

	if (!getlongvalue (hp1, 2, &indentlevel))
		return (false);
	
	if (!langhashtablelookup (ht, bs, &val, &hnode)) /*08/04/2000 AR*/
		return (false);

	if (!xmlfrontiervaltotaggedtext (&val, indentlevel, &htext, hnode))
		return (false);
	
	return (setheapvalue (htext, stringvaluetype, v));
	} /*xmlfrontiervaltotaggedtextverb*/


static boolean xmlstructtofrontiervalueverb (hdltreenode hp1, tyvaluerecord *v) {
	
	tyvaluerecord val;
	tyaddress adrstruct, adrvalue;
	
	if (!getaddressparam (hp1, 1, &val))
		return (false);
	
	if (!getaddressvalue (val, &adrstruct.ht, adrstruct.bs))
		return (false);
	
	if (!getaddressparam (hp1, 2, &val))
		return (false);
	
	if (!getaddressvalue (val, &adrvalue.ht, adrvalue.bs))
		return (false);

	if (!xmlstructtofrontiervalue (&adrstruct, &val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	if (!hashtableassign (adrvalue.ht, adrvalue.bs, val)) {
		
		disposevaluerecord (val, false);
		
		return (false);
		}

	return (setbooleanvalue (true, v));
	} /*xmlstructtofrontiervalueverb*/


static boolean xmlfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	*/
	
	hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /*by default, string functions return false*/
	
	switch (token) {
		
		case xmlcompilefunc:
			return (xmlcompileverb (hp1, v));
		
		case xmldecompilefunc:
			return (xmldecompileverb (hp1, v));
		
		case xmladdtablefunc:
			return (xmladdtableverb (hp1, v));
		
		case xmladdvaluefunc:
			return (xmladdvalueverb (hp1, v));
		
		case xmlgetvaluefunc:
			return (xmlgetvalueverb (hp1, v));

		case xmlgetaddressfunc:
			return (xmlgetaddressverb (hp1, v));

		case xmlgetaddresslistfunc:
			return (xmlgetaddresslistverb (hp1, v));

		case xmlgetattributefunc:
			return (xmlgetattributeverb (hp1, v));

		case xmlgetattributevaluefunc:
			return (xmlgetattributevalueverb (hp1, v));

		case xmlvaltostringfunc:
			return (xmlvaltostringverb (hp1, v));

		case xmlfrontiervaltotaggedtextfunc:
			return (xmlfrontiervaltotaggedtextverb (hp1, v));

		case xmlstructtofrontiervaluefunc:
			return (xmlstructtofrontiervalueverb (hp1, v));
		
		case xmlgetpathaddressfunc: {
			tyvaluerecord val;
			tyaddress xtable, adrresult, adr;
			Handle path;
			boolean fl;
			
			if (!getaddressparam (hp1, 1, &val))
				return (false);
			
			if (!getaddressvalue (val, &xtable.ht, xtable.bs))
				return (false);
			
			if (!getreadonlytextvalue (hp1, 2, &path))
				return (false);
				
			flnextparamislast = true;
			
			if (!getaddressparam (hp1, 3, &val))
				return (false);
			
			if (!getaddressvalue (val, &adrresult.ht, adrresult.bs))
				return (false);

			if (!xmlgetpathaddress (&xtable, path, &adr, &fl))
				return (false);
			
			if (!langassignaddressvalue (adrresult.ht, adrresult.bs, &adr))
				return (false);
			
			return (setbooleanvalue (fl, v));
			}
		
		case xmlconverttodisplaynamefunc: {
			Handle h;
			
			flnextparamislast = true;

			if (!getexempttextvalue (hp1, 1, &h))
				return (false);
			
			if (!xmlconverttodisplayname (h)) {
				
				disposehandle (h);
				
				return (false);
				}
			
			return (setheapvalue (h, stringvaluetype, v));		
			}
		
		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			return (false);
		} /*switch*/
	} /*xmlfunctionvalue*/


boolean xmlinitverbs (void) {
	
	/*
	5.0.2 dmb: new verbs
	*/
	
	return (loadfunctionprocessor (idxmlverbs, &xmlfunctionvalue));
	} /*xmlinitverbs*/
	



