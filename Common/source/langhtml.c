
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

/*dmb 6/17/96 4.0.2b1: integrated macroprocessor ucmd code*/

#include "frontier.h"
#include "standard.h"

#include "error.h"
#include "file.h"
#include "memory.h"
#include "ops.h"
#include "resources.h"
#include "strings.h"
#include "lang.h"
#include "langipc.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "langhtml.h"
#include "langwinipc.h"
#include "process.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "op.h"
#include "opinternal.h"
#include "oplist.h"
#include "opverbs.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "shell.rsrc.h"
#include "timedate.h"
#include "WinSockNetEvents.h"
#ifdef flcomponent
#include "osacomponent.h"
#endif

#include "tableverbs.h"  //6.1b8 AR: we need gettablevalue
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */

#include "iso8859.c"

extern boolean frontierversion (tyvaluerecord *v); //implemted in shellsysverbs.c
extern boolean sysos (tyvaluerecord *v); //implemted in shellsysverbs.c

#define fldebugwebsite false

#define str_separatorline BIGSTRING ("\x17<hr size=\"2\" width=\"100%\" />\r")/* 2005-12-18 creedon - end tag with space slash for compatibility with post HTML 4.01 standards */
#define str_macroerror BIGSTRING ("\x20<b>[</b>Macro error: ^0<b>]</b>\r")
#define str_mailto	BIGSTRING ("\x1A<a href=\"mailto:^0\">^0</a>")
#define str_hotlink	BIGSTRING ("\x13<a href=\"^0\">^1</a>")
#define str_pagebreak BIGSTRING ("\x05<p />") /* 2005-10-29 creedon - changed from <p> to <p /> for compatibility with post HTML 4.01 standards */
#define str_startbold BIGSTRING ("\x03<b>")
#define str_endbold	BIGSTRING ("\x04</b>")
#define str_default BIGSTRING ("\x07" "default")
#define str_index BIGSTRING ("\x05" "index")

#define str_closetag BIGSTRING ("\x05" "</^0>")

#define flcurlybracemacros true
#define startmacrochar '{'
#define endmacrochar '}'
#define flvariablemacrocharacters true /*PBS 7.1b1: variable macro characters*/

#define maxglossarynamelength 127 /*DW 5/3/96: upped from 31*/

#define str_no	BIGSTRING ("\x02" "no")
#define str_yes	BIGSTRING ("\x03" "yes")

#define str_adrpagetable		BIGSTRING ("\x16" "html.data.adrpagetable")
#define str_websitesdata		BIGSTRING ("\x0e" "websites.#data")
#define str_userhtmlprefs		BIGSTRING ("\x0f" "user.html.prefs")
#define str_usermacros			BIGSTRING ("\x10" "user.html.macros")
#define str_standardmacros		BIGSTRING ("\x18" "html.data.standardMacros")
#define str_tools				BIGSTRING ("\x05" "tools")
#define str_glossary			BIGSTRING ("\x08" "glossary")
#define str_images				BIGSTRING ("\x06" "images")
#define str_glosspatch			BIGSTRING ("\x0e" "[[#glossPatch ")
#define str_useglosspatcher		BIGSTRING ("\x0f" "useGlossPatcher")
#define str_renderedtext		BIGSTRING ("\x0c" "renderedtext")
	#define str_iso8859map		BIGSTRING ("\x15" "html.data.iso8859.mac")

#define str_template			BIGSTRING ("\x08" "template")
#define str_indirecttemplate	BIGSTRING ("\x10" "indirectTemplate")
#define str_adrobject			BIGSTRING ("\x09" "adrobject")
#define str_ftpsite				BIGSTRING ("\x07" "ftpsite")
#define str_fileextension 		BIGSTRING ("\x0d" "fileextension")
#define str_maxfilenamelength	BIGSTRING ("\x11" "maxfilenamelength")
#define str_defaulttemplate		BIGSTRING ("\x0f" "defaulttemplate")
#define str_defaultfilename		BIGSTRING ("\x0f" "defaultfilename")
#define str_directivesonlyatbeginning	BIGSTRING ("\x19" "directivesOnlyAtBeginning")

#define STR_P_ERRORPAGETEMPLATE			BIGSTRING ("\x4C" "<HTML><HEAD><TITLE>^0</TITLE></HEAD><BODY><H1>^0</H1><P>^1</P></BODY></HTML>")
#define STR_P_MISSING_HOST_HEADER		BIGSTRING ("\x40" "Every HTTP/1.1 request must include a Host header")
#define STR_P_UNSUPPORTED_VERSION		BIGSTRING ("\x24" "This server does not support HTTP/^0")
#define STR_P_INVALID_URI				BIGSTRING ("\x23" "All URIs must begin with / or http:")
#define STR_P_BODY_NOT_READ				BIGSTRING ("\x22" "The request body couldn't be read.")
#define STR_P_METHOD_NOT_ALLOWED		BIGSTRING ("\x20" "^0 isn't allowed on this object.")
#define STR_P_INVALID_REQUEST_LINE		BIGSTRING ("\x1C" "The request line is invalid.")
#define STR_P_USERWEBSERVERPOSTFILTERS	BIGSTRING ("\x1A" "user.webserver.postfilters")
#define STR_P_USERWEBSERVERPREFILTERS	BIGSTRING ("\x19" "user.webserver.prefilters")
#define STR_P_USERWEBSERVERRESPONDERS	BIGSTRING ("\x19" "user.webserver.responders")
#define STR_P_PROCESSING_STARTED		BIGSTRING ("\x18" "requestProcessingStarted")
#define STR_P_WEBSERVERDATARESPONSES	BIGSTRING ("\x18" "webserver.data.responses")

#define STR_P_macrostartchars			BIGSTRING ("\x14" "macrostartcharacters") /*PBS 7.1b1*/
#define STR_P_macroendchars				BIGSTRING ("\x12" "macroendcharacters")

#define STR_P_flprocessmacrosintags     BIGSTRING ("\x17" "processmacrosinhtmltags")

#ifdef PIKE /*7.0 PBS: server string is Radio UserLand*/

#ifndef OPMLEDITOR

		#define STR_P_SERVERSTRING				BIGSTRING ("\x15" "Radio UserLand/^0-^1X")

#else  // OPMLEDITOR 2005-04-06 dluebbert
		#define STR_P_SERVERSTRING				BIGSTRING ("\x0b" "OPML/^0-^1X")
#endif // OPMLEDITOR

#else // !PIKE

		#define STR_P_SERVERSTRING			BIGSTRING ("\x0f" "Frontier/^0-^1X") /* 2005-01-04 creedon - removed UserLand for open source release */

#endif  //!PIKE

#define STR_P_RESPONDERERROR			BIGSTRING ("\x16" "Responder method error")
#define STR_P_LOGADD					BIGSTRING ("\x16" "log.addToGuestDatabase")
#define STR_P_USERWEBSERVERCONFIG		BIGSTRING ("\x15" "user.webserver.config")
#define STR_P_USERWEBSERVERPREFS		BIGSTRING ("\x14" "user.webserver.prefs")
#define STR_P_USERWEBSERVERSTATS		BIGSTRING ("\x14" "user.webserver.stats")
#define STR_P_INETDCONFIGTABLEADR		BIGSTRING ("\x13" "inetdConfigTableAdr")
#define STR_P_DEFAULTTIMEOUTSECS		BIGSTRING ("\x12" "defaultTimeoutSecs")
#define STR_P_USERINETDLISTENS			BIGSTRING ("\x12" "user.inetd.listens")
#define STR_P_WEBSERVERDISPATCH			BIGSTRING ("\x12" "webserver.dispatch")
#define STR_P_RESPONDERTABLEADR			BIGSTRING ("\x11" "responderTableAdr")
#define STR_P_POSTFILTERERROR			BIGSTRING ("\x11" "Post Filter error")
#define STR_P_PREFILTERERROR			BIGSTRING ("\x10" "Pre filter error")
#define STR_P_DEFAULTRESPONDER			BIGSTRING ("\x10" "defaultResponder")
#define STR_P_USERINETDPREFS			BIGSTRING ("\x10" "user.inetd.prefs")
#define STR_P_RESPONSEHEADERS			BIGSTRING ("\x0F" "responseHeaders")
#define STR_P_WHATWEREWEDOING			BIGSTRING ("\x0F" "whatWereWeDoing")
#define STR_P_RETURNCHUNKSIZE			BIGSTRING ("\x0F" "returnChunkSize")
#define STR_P_ADRHEADERTABLE			BIGSTRING ("\x0E" "adrHeaderTable")
#define STR_P_MAXCONNECTIONS			BIGSTRING ("\x0E" "maxConnections")
#define STR_P_REQUESTHEADERS	 		BIGSTRING ("\x0E" "requestHeaders")
#define STR_P_CONTENT_LENGTH			BIGSTRING ("\x0E" "Content-Length")
#define STR_P_WHATWENTWRONG				BIGSTRING ("\x0D" "whatWentWrong")
#define STR_P_100CONTINUE				BIGSTRING ("\x0C" "100-continue")
#define STR_P_RESPONSEBODY				BIGSTRING ("\x0C" "responseBody")
#define STR_P_REQUESTBODY				BIGSTRING ("\x0B" "requestBody")
#define STR_P_MAXMEMAVAIL				BIGSTRING ("\x0B" "maxMemAvail")
#define STR_P_MINMEMAVAIL				BIGSTRING ("\x0B" "minMemAvail")
#define STR_P_CONNECTION				BIGSTRING ("\x0A" "Connection")
#define STR_P_PARAMTABLE 				BIGSTRING ("\x0A" "paramTable")
#define STR_P_SEARCHARGS				BIGSTRING ("\x0A" "searchArgs")
#define STR_P_FIRSTLINE					BIGSTRING ("\x09" "firstLine")
#define STR_P_CHUNKSIZE					BIGSTRING ("\x09" "chunksize")
#define STR_P_HTTP11					BIGSTRING ("\x09" "HTTP/1.1 ")
#define STR_P_CONDITION					BIGSTRING ("\x09" "condition")
#define STR_P_RESPONDER					BIGSTRING ("\x09" "responder")
#define STR_P_PATHARGS					BIGSTRING ("\x08" "pathArgs")
#define STR_P_ADRTABLE					BIGSTRING ("\x08" "adrTable")
#define STR_P_FLPARAMS					BIGSTRING ("\x08" "flParams")
#define STR_P_ENABLED					BIGSTRING ("\x07" "enabled")
#define STR_P_REQUEST					BIGSTRING ("\x07" "request")
#define STR_P_TIMEOUT					BIGSTRING ("\x07" "timeout")
#define STR_P_COOKIES					BIGSTRING ("\x07" "cookies")
#define STR_P_METHODS					BIGSTRING ("\x07" "methods")
#define STR_P_FLLEGAL					BIGSTRING ("\x07" "flLegal")
#define STR_P_FLCLOSE					BIGSTRING ("\x07" "flClose")
#define STR_P_NOWAIT					BIGSTRING ("\x06" "noWait")
#define STR_P_METHOD					BIGSTRING ("\x06" "method")
#define STR_P_EXPECT					BIGSTRING ("\x06" "Expect")
#define STR_P_STREAM					BIGSTRING ("\x06" "stream")
#define STR_P_REFCON					BIGSTRING ("\x06" "refcon")
#define STR_P_CLIENT					BIGSTRING ("\x06" "client")
#define STR_P_COOKIE					BIGSTRING ("\x06" "Cookie")
#define STR_P_SERVER					BIGSTRING ("\x06" "Server")
#define STR_P_UNKNOWN					BIGSTRING ("\x07" "UNKNOWN")
#define STR_P_THREAD					BIGSTRING ("\x06" "thread")
#define STR_P_DAEMON					BIGSTRING ("\x06" "daemon")
#define STR_P_ALLOW						BIGSTRING ("\x05" "ALLOW")
#define STR_P_CLOSE						BIGSTRING ("\x05" "close")
#define STR_P_READY						BIGSTRING ("\x05" "ready")
#define STR_P_STATS						BIGSTRING ("\x05" "stats")
#define STR_P_COUNT						BIGSTRING ("\x05" "count")
#define STR_P_CODE						BIGSTRING ("\x04" "code")
#define STR_P_HITS						BIGSTRING ("\x04" "hits")
#define STR_P_HOST						BIGSTRING ("\x04" "host")
#define STR_P_PORT						BIGSTRING ("\x04" "port")
#define STR_P_PATH						BIGSTRING ("\x04" "path")
#define STR_P_DATE						BIGSTRING ("\x04" "Date")
#define STR_P_CRLFCRLF					BIGSTRING ("\x04" "\r\n\r\n")
#define STR_P_ANY						BIGSTRING ("\x03" "any")
#define STR_P_URI						BIGSTRING ("\x03" "URI")
#define STR_P_DOLLAR_ENCODED			BIGSTRING ("\x03" "%24")
#define STR_P_CRLF						BIGSTRING ("\x02" "\r\n")
#define STR_P_COLON						BIGSTRING ("\x02" ": ")
#define STR_P_DOLLAR					BIGSTRING ("\x01" "$")
#define STR_P_SPACE						BIGSTRING ("\x01" " ")
#define STR_P_EMPTY						BIGSTRING ("\x00")
#define STR_P_USERWEBSERVERSTRING			BIGSTRING ( "\x11" "headerFieldServer" )

#define STR_STATUSCONTINUE				"HTTP/1.1 100 CONTINUE\r\n\r\n"
#define sizestatuscontinue				25

typedef enum tyhtmlverbtoken { /*verbs that are processed by langhtml.c*/
	
	processmacrosfunc,
	
	urldecodefunc,
	
	urlencodefunc,
	
	parseargsfunc,
	
	iso8859encodefunc,

	getgifheightwidthfunc,

	getjpegheightwidthfunc,
	
	buildpagetablefunc,
	
	refglossaryfunc,
	
	getpreffunc,
	
	getonedirectivefunc,
	
	rundirectivefunc,
	
	rundirectivesfunc,
	
	runoutlinedirectivesfunc,
	
	cleanforexportfunc,
	
	normalizenamefunc,
	
	glossarypatcherfunc,
	
	expandurlsfunc,
	
	traversalskipfunc,
	
	getpagetableaddressfunc,

	htmlneutermacrosfunc,

	htmlneutertagsfunc,
	
	htmlcalendardrawfunc,

	/* searchengine */

	stripmarkupfunc,
	
	deindexpagefunc,
	
	indexpagefunc,
	
	cleanindexfunc,
	
	unionmatchesfunc,
	
	/* mainResponder.calendar */
	
	mrcalendargetaddressdayfunc,
	
	mrcalendargetdayaddressfunc,
	
	mrcalendargetfirstaddressfunc,
	
	mrcalendargetfirstdayfunc,
	
	mrcalendargetlastaddressfunc,
	
	mrcalendargetlastdayfunc,
	
	mrcalendargetmostrecentaddressfunc,
	
	mrcalendargetmostrecentdayfunc,
	
	mrcalendargetnextaddressfunc,
	
	mrcalendargetnextdayfunc,
	
	mrcalendarnavigatefunc,

	/* webserver */

	webserverserverfunc,

	webserverdispatchfunc,

	webserverparseheadersfunc,

	webserverparsecookiesfunc,

	webserverbuildresponsefunc,

	webserverbuilderrorpagefunc,

	webservergetserverstringfunc,

	/* inetd */

	inetdsupervisorfunc,

	cthtmlverbs
	} tyhtmlverbtoken;

static bigstring bsdebug;

static boolean flpagemillfile = false;

#if version42orgreater

//static ptraddress callbackscript = nil;

#pragma pack(2)
typedef struct typrocessmacrosinfo {

	hdlhashtable hpagetable;
	hdlhashtable hstandardmacros;
	hdlhashtable huserprefs;
	hdlhashtable husermacros;
	hdlhashtable htools;
	hdlhashtable hmacrocontext;
	
	boolean flprocessmacros;
	boolean flexpandglossaryitems;
	boolean flautoparagraphs;
	boolean flactiveurls;
	boolean flclaycompatibility;
	boolean flisofilter;
	} typrocessmacrosinfo, *ptrprocessmacrosinfo;
#pragma options align=reset

#else

static tyvaluerecord osaval = { binaryvaluetype };

#endif


#pragma mark === processhtmlmacros ===

static boolean htmlcallbackerror (bigstring bsmsg, ptrvoid perrorstring) {
	
	/*
	4.0.2b1 dmb: this error trapping isn't bullet proof, but it should 
	be fine since once an error occurs, script execution quickly unwinds, 
	with no thread yielding.
	*/
	
	copystring (bsmsg, (ptrstring) perrorstring);
	
	return (true);
	} /*htmlcallbackerror*/

#if 0
#if version42orgreater

static boolean frontTextScriptCall (OSType idroutine, Handle stringparam, Handle *hresult, bigstring errorstring) {
	
	/*
	an interface for a script call that takes one parameter, a string,
	and returns a text value. a common situation, and it makes it possible for
	me to include some sample code in the toolkit.
	
	dmb 4.1b11: take Handle parameter, not bigstring, which we consume
	
	dmb 3/18/97: non-component version
	
	dmb 5.0d14: save/restore perrorstring; don't disable yield
	*/
	
	bigstring bsfunction;
	hdlhashnode hnode;
	hdltreenode hcode, hparam;
	boolean fl = false;
	langerrormessagecallback savecallback;
	ptrvoid saveerrorstring;
	tyvaluerecord vparam;
	tyvaluerecord vresult;
	
	*hresult = nil;
	
	ostypetostring (idroutine, bsfunction);
	
	if (!setheapvalue (stringparam, stringvaluetype, &vparam)) // consumes stringparam
		return (false);
	
	exemptfromtmpstack (&vparam); // 5.0.2b10 dmb
	
	if (!newconstnode (vparam, &hparam))	// consumes vparam
		return (false);
	
//	if (!getaddressvalue (callbackval, &htable, bs))
//		goto exit;
	
	if (!hashtablelookupnode ((*callbackscript).ht, (*callbackscript).bs, &hnode))
		goto exit;
	
	if (!langgetnodecode ((*callbackscript).ht, bsfunction, hnode, &hcode)) {
	
		if (!fllangerror)
			langparamerror (notfunctionerror, (*callbackscript).bs);
		
		goto exit;
		}
	
	savecallback = langcallbacks.errormessagecallback;
	
	saveerrorstring = langcallbacks.errormessagerefcon;
	
	langcallbacks.errormessagerefcon = errorstring;
	
	langcallbacks.errormessagecallback = &htmlcallbackerror;
	
	//	++fldisableyield;
	
	fl = langfunctioncall (nil, (*callbackscript).ht, hnode, bsfunction, hcode, hparam, &vresult);
	
	//	--fldisableyield;
	
	if (fl)
		fl = coercetostring (&vresult);
	else
		fllangerror = false;	/*we don't want to abort anything*/

	if (fl) {
		
		exemptfromtmpstack (&vresult);
		
		*hresult = vresult.data.stringvalue;
		}
	
	langcallbacks.errormessagerefcon = saveerrorstring;
	
	langcallbacks.errormessagecallback = savecallback;
	
	exit:
	
	langdisposetree (hparam);
	
	return (fl);
	} /*frontTextScriptCall*/

#else

static boolean frontTextScriptCall (OSType idroutine, Handle stringparam, Handle *hresult, bigstring errorstring) {
	
	/*
	an interface for a script call that takes one parameter, a string,
	and returns a text value. a common situation, and it makes it possible for
	me to include some sample code in the toolkit.
	
	dmb 4.1b11: take Handle parameter, not bigstring
	*/
	
	AppleEvent event, reply = {typeNull, nil};
	AEDesc script, result;
	boolean fl;
	langerrormessagecallback savecallback;
	ptrvoid saveerrorstring;
	OSErr ec;
	
	*hresult = nil;
	
	if (!newselfaddressedevent (idroutine, &event))
		return (false);
	
	
		typeAEList (&script, typeChar, stringparam);
	
	
	ec = AEPutKeyDesc (&event, 'prm1', &script);
	
	disposehandle (stringparam);
	
	if (ec != noErr)
		goto error;
	
	savecallback = langcallbacks.errormessagecallback;
	
	saveerrorstring = langcallbacks.errormessagerefcon;
	
	langcallbacks.errormessagerefcon = errorstring;
	
	langcallbacks.errormessagecallback = &htmlcallbackerror;
	
	fl = evaluateosascriptevent (&osaval, &event, &reply);
	
	langcallbacks.errormessagerefcon = saveerrorstring;
	
	langcallbacks.errormessagecallback = savecallback;

	if (!fl)
		goto error;
	
	AEDisposeDesc (&event);	
	
	ec = AEGetParamDesc (&reply, keyDirectObject, typeChar, &result);
	
	AEDisposeDesc (&reply);	
	
	if (ec != noErr)
		goto error;
	
	
		copydatahandle (&result, hresult);
		
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);	
	
	return (false);
	} /*frontTextScriptCall*/

#endif
#endif


static boolean strongcoercetostring (tyvaluerecord *val) {

	boolean fl;
	
	flcoerceexternaltostring = true;
	
	fl = coercetostring (val);
	
	flcoerceexternaltostring = false;
	
	return (fl);
	} /*strongcoercetostring*/


static boolean langpushwithtable (hdlhashtable ht, hdlhashtable hwith) {

	/*
	5.0.2b14 dmb: sucked out of evaluatewith, comes in at a different level
	*/
	
	short n = (**ht).ctwithvalues;
	bigstring bs;
	tyvaluerecord valwith;
	
	if (n == 7) { /*maximum value of ctwithvalues*/
		
		langlongparamerror (toomanywithtableserror, n);
		
		return (false);
		}
	
	if (!setaddressvalue (hwith, zerostring, &valwith))
		return (false);
	
	langgetwithvaluename (++n, bs);
	
	(**ht).ctwithvalues = n; /*optimization for langfindsymbol*/
	
	if (!hashtableassign (ht, bs, valwith)) {
		
		disposevaluerecord (valwith, false);
		
		return (false);
		}

	exemptfromtmpstack (&valwith); /*its in the local table now*/
	
	return (true);
	} /*langpushwithtable*/


static boolean htmlgetdefaultpagetable (hdlhashtable *hpagetable) {
	
	/*
	5.0.2 dmb: super-fast lookup of "websites.[#data]"
	*/
	
	return (langfastaddresstotable (roottable, str_websitesdata, hpagetable));
	} /*htmlgetdefaultpagetable*/


static boolean getoptionalpagetablevalue (hdltreenode hp1, short n, hdlhashtable *hpagetable) {

	if (langgetparamcount (hp1) >= n) {
		
		flnextparamislast = true;
		
		if (!gettablevalue (hp1, n, hpagetable))
			return (false);
		}
	else {
		if (!htmlgetdefaultpagetable (hpagetable))
			return (false);
		}
	
	return (true);
	} /*getoptionalpagetablevalue*/


#if 0

static boolean htmlgetpagetable (hdlhashtable *hpagetable) {
	
	/*
	5.0.2 dmb: super-fast lookup of html.data.adrpagetable. we skip the 
	normal error reporting, because we're responsible for setting this 
	value up properly
	*/
	
	hdlhashtable ht;
	bigstring bs;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!langexpandtodotparams (str_adrpagetable, &ht, bs))
		return (false);
	
	if (!hashtablelookup (ht, bs, &val, &hnode) || (val.valuetype != addressvaluetype))
		return (false);
	
	if (!getaddressvalue (val, &ht, bs))
		return (false);
	
	if (!hashtablelookup (ht, bs, &val, &hnode))
		return (false);
	
	return (tablevaltotable (val, hpagetable, hnode));
	} /*htmlgetpagetable*/

#endif

		
static boolean htmlgetprefstable (hdlhashtable *huserprefs) {
	
	/*
	5.0.2 dmb: super-fast lookup of "user.html.prefs"
	*/
	
	return (langfastaddresstotable (roottable, str_userhtmlprefs, huserprefs));
	} /*htmlgetprefstable*/


static boolean htmlgetpref (typrocessmacrosinfo *pmi, bigstring pref, tyvaluerecord *val) {

	/*
	5.0.2 dmb: pulled into kernel. we return val on the tmp stack
	
	on getPref (prefName, adrpagedata=@websites.["#data"]) { Çnew in 4.1
		ÇLook for a preference directive, a global pref, or return a default
	*/
	
	//local (val);
	bigstring bs;
	hdlhashnode hnode;
	
	//	try { Çtry to get it from pagedata
	//		val = adrpagedata^.[prefName];
	
	if (hashtablelookup ((*pmi).hpagetable, pref, val, &hnode)) {
		
		//	case val {
		//		"yes";
		//		"true" {
		//			return (true)};
		//		"no";
		//		"false" {
		//			return (false)}}
		//	else {
		//		return (val)}};
		
		if ((*val).valuetype == stringvaluetype) {
			
			pullstringvalue (val, bs);
			
			if (equalstrings (bs, str_yes) || equalstrings (bs, bstrue))
				return (setbooleanvalue (true, val));
			
			if (equalstrings (bs, str_no) || equalstrings (bs, bsfalse))
				return (setbooleanvalue (false, val));
			}
		
		return (copyvaluerecord (*val, val));
		}
	
	//	try { Çtry to get it from user.html.prefs
	//		return (user.html.prefs.[prefName])};
	
	if (hashtablelookup ((*pmi).huserprefs, pref, val, &hnode))
		return (copyvaluerecord (*val, val));
	
	//	case string.lower (prefName) { Çreturn a default value
	//		"fileextension" {
	//			return (".html")};
	//		"maxfilenamelength" {
	//			return (31)};
	//		"defaulttemplate" {
	//			return ("normal")};
	//		"defaultFileName" {
	//			"default"}}
	
	copystring (pref, bs);
	alllower (bs);
	
	if (equalstrings (bs, str_fileextension))
		return (setstringvalue (BIGSTRING ("\x05" ".html"), val));
	
	if (equalstrings (bs, str_maxfilenamelength))
		return (setlongvalue (31, val));
	
	if (equalstrings (bs, str_defaulttemplate))
		return (setstringvalue (BIGSTRING ("\x06" "normal"), val));
	
	if (equalstrings (bs, str_defaultfilename))
		return (setstringvalue (BIGSTRING ("\x07" "default"), val));

	//	else { Çunknown or default prefs are true
	//		return (true)}};
	
	return (setbooleanvalue (true, val));
	} /*htmlgetpref*/


static boolean htmlgetbooleanpref (typrocessmacrosinfo *pmi, bigstring pref, boolean *flpref) {
	
	tyvaluerecord val;
	
	if (!htmlgetpref (pmi, pref, &val))
		return (false);
	
	if (!coercetoboolean (&val))
		return (false);
	
	*flpref = val.data.flvalue;
	
	return (true);
	} /*htmlgetbooleanpref*/


static boolean htmlgetstringpref (typrocessmacrosinfo *pmi, bigstring pref, bigstring bspref) {
	
	tyvaluerecord val;
	
	if (!htmlgetpref (pmi, pref, &val))
		return (false);
	
	if (!coercetostring (&val))
		return (false);
	
	pullstringvalue (&val, bspref);
	
	return (true);
	} /*htmlgetstringpref*/


static boolean htmlrefglossary (typrocessmacrosinfo *pmi, Handle hreference, bigstring perrorstring, Handle *hresult) {
#pragma unused (pmi)

	/*
	expand the glossary item, consuming it. return the result in hresult, which 
	our caller must dispose. on error, return the error text in perrorstring.
	
	5.0.2b14 dmb: we used to frontTextScriptCall to call html.data.processmacroscallback, 
	which called html.refGlossary. now we call html.refGlossary directly. I looked at 
	kernelizing the whole thing here, but it would be a lot of work. plus, these type of 
	lookups aren't that slow in UserTalk.
	*/
	
	tyvaluerecord vparams, vresult;
	langerrormessagecallback savecallback = langcallbacks.errormessagecallback;
	ptrvoid saveerrorstring = langcallbacks.errormessagerefcon;
	boolean fl;
	
	setheapvalue (hreference, stringvaluetype, &vparams);
	
	if (!coercetolist (&vparams, listvaluetype))
		return (false);
	
	langcallbacks.errormessagecallback = &htmlcallbackerror;
	
	langcallbacks.errormessagerefcon = perrorstring;
	
	fl = langrunscript (BIGSTRING ("\x10" "html.refglossary"), &vparams, nil, &vresult) && strongcoercetostring (&vresult);
	
	langcallbacks.errormessagerefcon = saveerrorstring;
	
	langcallbacks.errormessagecallback = savecallback;
	
	fllangerror = false;
	
	if (fl) {
		
		exemptfromtmpstack (&vresult);
		
		*hresult = vresult.data.stringvalue;
		}
	
	return (fl);
	
	/*
	//	local (adrpagetable = html.data.adrPageTable);
	//	local (adrobject = adrpagetable^.adrobject);
	//	local (adrparent = parentOf (adrobject^));
	
	hdlhashtable hpagetable = (*pmi).hpagetable;
	tyaddress adrobject;
	tyaddress adrparent;
	tyaddress adrelement;
	tyvaluetype objecttype;
	tyvaluerecord val;
	
	if (!hashtablelookup (hpagetable, str_adrobject, &val) || !getaddressvalue (val, &adrobject.ht, adrobject.bs)) {
		
		langparamerror (unknownidentifiererror, str_adrobject);
		
		return (false);
		}
	
	findinparenttable (adrobject.ht, &adrparent.ht, adrparent.bs);
	
	
	if defined (adrpagetable^.glossary) { Çlook in the default glossary
		local (adrelement = @adrpagetable^.glossary^ [name]);
		if defined (adrelement^) {
			return (foundit (adrelement))}};
	try { Çcheck the hierarchy for glossary tables
		local (nomad = adrparent, newnomad, found = false);
		on glossLookup (glossName) {
			if (defined (nomad^.[glossName])) { Çthe table has a glossary
				local (adrelement = @nomad^.[glossName].[name]);
				if defined (adrelement^) { Çit defines the term
					found = true;
					return (foundit (adrelement))}}};
		loop {
			s = glossLookup ("glossary");
			if found {
				return (s)};
			s = glossLookup ("#glossary");
			if found {
				return (s)};
			newnomad = parentOf (nomad^);
			if newnomad == "" {
				break};
			nomad = newnomad}};
	try { Çcheck the user table glossary
		return (string (user.html.glossary [name]))};
	if not (name beginsWith "#") {
		try { Çlook for objects at the same level with the name
			local (normalizedName = html.normalizeName (name, adrpagetable));
			local (ext = html.getPref ("fileExtension", adrpagetable));
			if defined (adrparent^.[normalizedName]) {
				return ("<a href=\"" + normalizedName + ext + "\">" + name + "</a>")};
			if defined (adrparent^.[name]) {
				return ("<a href=\"" + normalizedName + ext + "\">" + name + "</a>")}}};
	scriptError ("There is no glossary entry named \"" + name + "\"")}
	
	foundit:
	//	on foundit (adr) { Çnew in 5.0
	//		case typeOf (adr^) {
	//			scriptType { //new in 5.0, glossary items can be scripts
	//				return (adr^ ())};
	//			tableType { //new in 5.0, convert a table (prettier to look at) to a glossPatch
	//				local (s = "[[#glossPatch ");
	//				s = s + adr^.linetext + "|";
	//				s = s + adr^.path + "|]]";
	//				return (s)}}
	//		else {
	//			local (s = string (adr^));
	//			if (s beginsWith "{") and (s endsWith "}") {
	//				return (html.processMacros (s))}
	//			else {
	//				return (s)}}};
	
		objecttype = langgetextendedvaluetype (&glossval);
		
		switch (objecttype) {
			
			case tablevaluetype:
					if (langexternalvaltotable (glossval, &ht)) { // should never fail
						if (!newtexthandle (str_glosspatch, hresult))
							return (false);
						
						if (!langlookupstringvalue (ht, "\x08" "linetext", &x)
							return (false);
						
						if (!pushhandle (x, *hresult))
							return (false);
						
						disposehandle (x);
						
						if (!pushtexthandle ("\x01" "|", *hresult))
							return (false);
						
						
			
			case scriptvaluetype:
				break;
			
			default:
				break;
			}
		
	//	return (frontTextScriptCall ('refg', term, hresult, errorstring));
	*/
	
	} /*htmlrefglossary*/
	

static boolean htmlcleanforexport (Handle x) {

	/*
	on cleanForExport (text) { Çprepare text to leave Mac environment
		if sys.os () == "MacOS" {
			on replace (searchfor, replacewith) {
				if string.patternMatch (searchfor, text) != 0 { Çoptimization
					text = string.replaceAll (text, searchfor, replacewith)}};
			
			replace ("Õ", "'");
			replace ("Ô", "'");
			replace ("Ò", Ò"Ó);
			replace ("Ó", Ò"Ó);
			replace ("¥", "o");
			replace ("...", "...");
			replace ("Ê", " "); Ça funny space that Word produces
			replace ("Ð", "--");
			replace ("Ç", "&lt;&lt;");
			replace ("È", "&gt;&gt;")};
		return (text)}
	*/
	
		handlestream s;
	
		openhandlestream (x, &s);
		
		for (s.pos = 0; s.pos < s.eof; ++s.pos) {
			
			switch ((*x) [s.pos]) { // set chreplace or bsreplace
			
				case (char)0xd4:	/* 'Ô' open single quote */
				case (char)0xd5:	/* 'Õ' close single quote */
					(*x) [s.pos] = '\'';
					break;
				
				case (char)chopencurlyquote:
				case (char)chclosecurlyquote:
					(*x) [s.pos] = '\"';
					break;
				
				case (char)0xa5:	/* '¥' disk */
					(*x) [s.pos] = 'o';
					break;
					
				case (char)0xca:	/* 'Ê' non-breaking space */
					(*x) [s.pos] = ' ';
					break;
				
				case (char)0xc9:	/* 'É' ellipsis */
					mergehandlestreamstring (&s, 1, BIGSTRING ("\x03" "..."));
					--s.pos;
					break;
				
				case (char)0xd0:	/* 'Ð' n-dash */
				case (char)0xd1:	/* 'Ñ' m-dash */
					mergehandlestreamstring (&s, 1, BIGSTRING ("\x02" "--"));
					--s.pos;
					break;
				
				case (char)chcomment:
					mergehandlestreamstring (&s, 1, BIGSTRING ("\x08" "&lt;&lt;"));
					--s.pos;
					break;
				
				case (char)chendcomment:
					mergehandlestreamstring (&s, 1, BIGSTRING ("\x08" "&gt;&gt;"));
					--s.pos;
					break;
				}
			}
		
		closehandlestream (&s);
	
	return (true);
	} /*htmlcleanforexport*/


static void htmldisposemacrocontext (typrocessmacrosinfo *pmi) {
	
	disposehashtable ((*pmi).hmacrocontext, false);
	
	(*pmi).hmacrocontext = nil;
	} /*htmldisposemacrocontext*/

	
static boolean htmlbuildmacrocontext (typrocessmacrosinfo *pmi) {
	
	/*
	5.0.2b15 dmb: build the with statement for evaluating macro expressions.
	also include a local "adrPageTable" value.
	
	5.0.2b17 dmb: restore the page table to the outermost with scope
	*/
	
	hdlhashtable hstandardmacros, husermacros, htools;
	hdlhashtable ht, hcontext;
	tyvaluerecord val;
	bigstring bs, bstools;
	hdlhashnode hnode;
	
	if ((*pmi).hmacrocontext != nil)
		return (true);
	
	// find macros tables
	if (!langfastaddresstotable (builtinstable, str_standardmacros, &hstandardmacros))
		return (false);
	
	if (!langfastaddresstotable (roottable, str_usermacros, &husermacros))
		return (false);
	
	if (!newhashtable (&hcontext)) /*new table for the function when it runs*/
		return (false);
	
	(**hcontext).fllocaltable = true;

	(**hcontext).lexicalrefcon = 0L; /*'with' gets global scope*/
	
	// local (adrpagetable = html.data.adrPageTable)
	if (!findinparenttable ((*pmi).hpagetable, &ht, bs))
		goto error;
	
	if (!setaddressvalue (ht, bs, &val))
		goto error;
	
	if (!hashtableassign (hcontext, BIGSTRING ("\x0c" "adrPageTable"), val))
		goto error;
	
	exemptfromtmpstack (&val);
	
	//	with html.data.adrPageTable^, html.data.standardMacros, user.html.macros, toolTableAdr^ {
	if (!langpushwithtable (hcontext, (*pmi).hpagetable))
		goto error;
	
	if (!langpushwithtable (hcontext, hstandardmacros))
		goto error;
	
	if (!langpushwithtable (hcontext, husermacros))
		goto error;
	
	if (hashtablelookup ((*pmi).hpagetable, str_tools, &val, &hnode) && 
		(val.valuetype == addressvaluetype) &&
		getaddressvalue (val, &htools, bstools) &&
		hashtablelookup (htools, bstools, &val, &hnode) &&
		tablevaltotable (val, &htools, hnode))
		
		if (!langpushwithtable (hcontext, htools))
			goto error;
	
	(*pmi).hmacrocontext = hcontext;
	
	return (true);
	
	error:
		disposehashtable (hcontext, false);
		
		return (false);
	} /*htmlbuildmacrocontext*/


static boolean htmlrunmacro (typrocessmacrosinfo *pmi, Handle macro, bigstring perrorstring, Handle *hresult) {
	
	/*
	run the macro, consuming it. return the result in hresult, which our caller 
	must dispose. on error, return the error text in perrorstring.
	
		local (toolTableAdr, nilTable);
		if defined (adrpagetable^.tools) {
			toolTableAdr = adrpagetable^.tools}
		else { Ç'with' wants a table
			new (tableType, @nilTable);
			toolTableAdr = @nilTable};
	
		try {
			local (macroResult);
			with html.data.adrPageTable^, html.data.standardMacros, user.html.macros, toolTableAdr^ {
				macroResult = string (evaluate (s))};
	
	5.0.2b14 dmb: we used to frontTextScriptCall to call html.data.processmacroscallback, 
	now we do all the work right here.
	
	5.0.2b15 dmb: we also need to create a local "adrPageTable" value

	5.1b21 dmb: plugged leak on error
	
	6.2b6 AR: If langruntraperror returns false but didn't set perrorstring, our thread has probably been killed.
	*/
	
	tyvaluerecord val;
	boolean fl = false;
	
	if (!htmlbuildmacrocontext (pmi))
		return (false);
		
	setemptystring (perrorstring); //6.2b6 AR
	
	chainhashtable ((*pmi).hmacrocontext);
	
	fl = langruntraperror (macro, &val, perrorstring) && strongcoercetostring (&val) && exemptfromtmpstack (&val);
	
	unchainhashtable ();
	
	if (fl) {
		
		Handle h = val.data.stringvalue;
		
		if ((*pmi).flexpandglossaryitems) {
		
			//	if (macroResult beginsWith '"') and (macroResult endsWith '"') {
			//		if html.getPref ("expandGlossaryItems", adrpagetable) {
			//			macroResult = string.mid (macroResult, 2, sizeOf (macroResult) - 2);
			//			macroResult = html.refGlossary (macroResult)}};
			
			if ((*h) [0] == '"' && (*h) [gethandlesize (h) - 1] == '"') {
				
				pullfromhandle (h, 0, 1, nil);
				
				popfromhandle (h, 1, nil);
				
				return (htmlrefglossary (pmi, h, perrorstring, hresult));
				}
			}
		
		*hresult = h;
		}
	else
		if (stringlength (perrorstring) > 0 && ingoodthread ()) { //6.2b6 AR
			
			Handle herror;

			if (newtexthandle (perrorstring, &herror)) {
				
				htmlcleanforexport (herror);
				
				texthandletostring (herror, perrorstring);

				disposehandle (herror);
				}
			}
		else
			*hresult = nil;	
	
	return (fl);
	/*
		else {
			local (errorString = html.cleanForExport (tryError));
			if html.getPref ("logMacroErrors", adrpagetable) {
				local (adroutline = @user.html.macroErrors);
				if not defined (adroutline^) {
					new (outlineType, adroutline)};
				local (oldtarget = target.get ());
				edit (adroutline);
				target.set (adroutline);
				op.firstSummit ();
				local (adr = adrpagetable^.adrObject);
				local (linetext = op.getLineText ());
				local (message = "Error rendering \"" + adr + "\" at " + string.timeString ());
				if linetext == "" {
					op.setLineText (message)}
				else {
					op.insert (message, up)};
				op.insert (errorString, right);
				op.insert ("Macro string: {" + s + "}", down); // PBS: add the macro string
				try {target.set (oldtarget)}};
			scriptError (errorString)}}
	*/
	//return (frontTextScriptCall ('dosc', macro, hresult, errorstring));
	} /*htmlrunmacro*/


static boolean htmlreportmacroerror (typrocessmacrosinfo *pmi, Handle macro, bigstring perrorstring) {
#pragma unused (pmi)

	/*
	call back to the odb to report the error. consume the macro handle
	*/
	
	tyvaluerecord val, vparams, vresult;
	
	setheapvalue (macro, stringvaluetype, &vparams);
	
	if (!coercetolist (&vparams, listvaluetype))
		return (false);
	
	if (!setstringvalue (perrorstring, &val))
		return (false);
		
	if (!langpushlistval (vparams.data.listvalue, nil, &val))
		return (false);
	
	return (langrunscript (BIGSTRING ("\x17" "html.data.logMacroError"), &vparams, nil, &vresult));
	} /*htmlreportmacroerror*/


#if 0

static boolean isPunctuationChar (char ch) {
	
	/*return (ispunct (ch));*/
	
	if ((ch >= '!') && (ch <= '/')) 
		return (true);
		
	if ((ch >= ':') && (ch <= '?'))
		return (true);
	
	return (false);
	} /*isPunctuationChar*/

#endif
	

static boolean isLegalURLPunctuationChar (char ch) {
	
	/*
	5.0a9 dmb: made this a function, stead of a huge or test in two places.
			   also added '?', '=', '$' and ',' to the list
	
	5.0.2b16 dmb: added '&'
	
	6.1d1 ar: added '@'

	6.1b15 AR: Added '+'
	*/
	
	switch (ch) {
		case '.':
		case ',':
		case '/':
		case '~':
		case '#':
		case '-':
		case ':':
		case '%':
		case '?':
		case '=':
		case '$':
		case '&':
		case '@':
		case '+':
			return (true);
		
		default:
			return (false);
		}
	} /*isLegalURLPunctuationChar*/
	

static boolean isAlphaChar (char ch) {
	
	return (isalnum (ch) || (ch == '_'));
	} /*isAlphaChar*/
	

static boolean processmacrosintags (typrocessmacrosinfo *pmi) {

	/*
	7.1b3 PBS: return true if we should process macros in HTML tags.
	If the pref isn't specified, the default is false,
	preserving current behavior.
	*/

	hdlhashnode hnode;
	tyvaluerecord val;
	bigstring bsval;
	
	if (hashtablelookup ((*pmi).hpagetable, STR_P_flprocessmacrosintags, &val, &hnode)) { /*page table*/

		if (val.valuetype == booleanvaluetype)

			return (val.data.flvalue);

		else {

			coercetostring (&val);

			pullstringvalue (&val, bsval);

			if (equalidentifiers (bsval, bstrue))

				return (true);

			else

				return (false);
			} /*else*/
		} /*if*/
				
	if (hashtablelookup ((*pmi).huserprefs, STR_P_flprocessmacrosintags, &val, &hnode)) { /*user.html.prefs*/

		if (val.valuetype == booleanvaluetype)

			return (val.data.flvalue);

		else {

			coercetostring (&val);

			pullstringvalue (&val, bsval);

			if (equalidentifiers (bsval, bstrue))

				return (true);

			else

				return (false);
			} /*else*/
		} /*if*/

	return (false);
	} /*processmacrosintags*/


static boolean getmacrocharacters (typrocessmacrosinfo *pmi, bigstring bsstart, bigstring bsend) {
	
	/*
	7.1b1 PBS: get the user-specified macro characters. Return false if there are none, in which
	case default behavior will apply.
	*/
	
	boolean flfoundone = false;

	if (!flvariablemacrocharacters)
		
		return (false);

	htmlgetstringpref (pmi, STR_P_macrostartchars, bsstart); /*get the start characters*/

	if (equalidentifiers (bsstart, bstrue)) /*not found*/
				
			copystring (BIGSTRING ("\x01{"), bsstart);

	else

		flfoundone = true;

	htmlgetstringpref (pmi, STR_P_macroendchars, bsend); /*get the end characters*/

	if (equalidentifiers (bsend, bstrue)) /*not found*/
		
		copystring (BIGSTRING ("\x01}"), bsend);
		
	else
		
		flfoundone = true;

	return (flfoundone);	
	} /*getmacrocharacters*/


static boolean processhtmltext (handlestream *s, typrocessmacrosinfo *pmi) {

	//boolean flactiveurls, boolean claycompatibity) {
	
	/*
	4.1b4 dmb: handled nested startmacrochars in macro text.
	
	4.1b5 dmb: don't translate Ç, È, ©, ¨ or non-breaking space. They'll all get 
	iso8859 encoded when appropriate.
	
	4.1b5 dmb: for glassary references, we were adding the full length of the 
	item to 'i', but we need to so one less since the loop is about to increment it
	
	4.1b11 dmb: added flactiveurls, claycompatibity params. 
	also, allow '#', '-', ':' and '%' characters in urls
	also, runmacro and glossaryreference take handles, not bigstrings

	5.0.1 dmb: fixed bug handling garbage text around a @ character
	
	5.0.2b14 dmb: use typrocessmacrosinfo, suck down much more logic into kernel.
	
	5.1.6 dmb: chprev for macro expansion security fix
	
	6.1d1 ar: fixed bug handling URLs and email addresses > 255 chars, use handles
	instead of bigstrings.

	7.1b1 PBS: allow variable macro characters. Get them from the page table.
	If not present, use the default curly braces.

	7.1b3 PBS: allow, optionally, for macros to be processed inside HTML tags.
	Default is false, preserving current behavior.

	7.1b16 PBS: don't do glossary lookups or active URLs inside HTML tags.
	*/
	
	#define ct ((*s).eof)
	#define i ((*s).pos)
	Handle h = (*s).data;
//	unsigned long ct, i;
	char ch = chnul;
	char chprev;
	long remainingchars;
	boolean flcustommacrochars = false; /*true if allowed and in the page table*/
	bigstring bsmacrostartchars, bsmacroendchars; /*start and end characters for macros*/
	unsigned char firststartmacrochar = startmacrochar;
	unsigned char firstendmacrochar = endmacrochar;
	long lenstartmacro = 1, lenendmacro = 1;
	boolean flprocessmacrosinhtmltags = false;
	boolean flinhtmltag = false;

	flcustommacrochars = getmacrocharacters (pmi, bsmacrostartchars, bsmacroendchars); /*PBS 7.1b1: get custom chars*/

	if (flcustommacrochars) {

		firststartmacrochar = bsmacrostartchars [1];

		firstendmacrochar = bsmacroendchars [1];

		lenstartmacro = bsmacrostartchars [0];

		lenendmacro =  bsmacroendchars [0];
		} /*if*/

	flprocessmacrosinhtmltags = processmacrosintags (pmi); /*PBS 7.1b3: true if we should process macros in HTML tags.*/
	
	if (fldebugwebsite) {
		
		setstringlength (bsdebug, 25); 
		
		memset(&bsdebug [1], ' ', 25);
		}
	
	for (i = 0; i < ct; i++) {
		
		chprev = ch;
		
		ch = (*h) [i];

		if (ch == '<')
			flinhtmltag = true;

		if (ch == '>')
			flinhtmltag = false;

		if (!flprocessmacrosinhtmltags)
			flinhtmltag = false;
		
		if (fldebugwebsite) { /*makes it easy to see where the process failed after the fact*/
			
			moveleft (&bsdebug [2], &bsdebug [1], 24);
			
			bsdebug [25] = ch;
			}
		
		remainingchars = ct - i - 1;

		if (ch == firststartmacrochar) /*7.1b1: variable macros support*/
			ch = startmacrochar;
		
		switch (ch) {
			
			case '\\': { /*delete the backslash, skip over the char after it*/

				if (flinhtmltag)
					break;
				
				pullfromhandlestream (s, 1, nil);
				
				i++; /*skip over next character*/
				
				ch = (*h) [i]; //note it for secure macro expansion
				
				break;
				}
			
			case '-': { /*support AutoWeb convention, three dashes is a separator line*/

				if (flinhtmltag)
					break;
				
				if ((*pmi).flclaycompatibility && remainingchars > 3) {  /*4.1b11 dmb*/
					
					if (((*h) [i + 1] == '-') && ((*h) [i + 2] == '-') && ((*h) [i + 3] == '\r')) {
						
						if (!mergehandlestreamstring (s, 3, str_separatorline))
							return (false);
						}
					}
				
				break;
				}
			
			case '*': { /*support AutoWeb convention, three asterisks emboldens the rest of the line*/

				if (flinhtmltag)
					break;
				
				if ((*pmi).flclaycompatibility && remainingchars > 3) {  /*4.1b11 dmb - claycompatibity*/
					
					if (((*h) [i + 1] == '*') && ((*h) [i + 2] == '*')) {
						
						long j;
						
						mergehandlestreamstring (s, 3, str_startbold);
						
						for (j = i; j < ct; j++) {
							
							if ((*h) [j] == '\r') {
								
								long savepos = i;
								
								i = j;
								
								if (!mergehandlestreamstring (s, 0, str_endbold))
									return (false);
								
								i = savepos;
								
								break;
								}
							} /*for*/

						--i;
						}
					}
					
				break;
				}
				
		case '<': 
		case startmacrochar: { /*process an HTML tag or inline macro*/

				boolean flismacro = false;
				unsigned long startmacro, endmacro;
				long j, k;
				unsigned short lenmacrooverhead;
				boolean flprocessed = false;
				boolean flfoundvariablemacro = false;
				
				if ((!flcurlybracemacros) && (ch == startmacrochar)) /*DW 11/4/95*/
					continue;
				
				if (chprev == '\\') //5.1.6 security fix
					continue;

				#if flvariablemacrocharacters

					if (((*h) [i] == firststartmacrochar) && (flcustommacrochars)) {

						if (remainingchars > lenstartmacro) {

							flismacro = true;

							for (j = 1; j < lenstartmacro; j++) {

								if ((*h) [i + j] != bsmacrostartchars [j + 1]) {

									flismacro = false;

									break;
									} /*if*/
								} /*for*/

							if (flismacro) { /*find end of macro*/

								flismacro = false; /*will be set true if we find the end of the macro*/

								startmacro = i + lenstartmacro;

								for (j = 1; j < (i + remainingchars); j++) {
				
									if ((*h) [j + startmacro] == firstendmacrochar) {

										flfoundvariablemacro = true;

										for (k = 1; k < lenendmacro; k++) {

											if ((*h) [j + startmacro + k] != bsmacroendchars [k + 1]) {

												flfoundvariablemacro = false;

												break;
												} /*if*/
											} /*for*/

										if (flfoundvariablemacro) {

											flismacro = true;

											endmacro = j + startmacro - 1;

											lenmacrooverhead = lenstartmacro + lenendmacro;

											goto processmacro;
											}
										} /*if*/
									} /*for*/
								} /*if*/
							} /*if*/
						} /*if*/

				#endif
				
				#ifdef acceptnoncurlymacros /*DW 11/17/95*/
				
					if (ch == '<') {
						
						if (remainingchars > 3) {
						
							if (((*h) [i + 1] == '!') && ((*h) [i + 2] == '-') && ((*h) [i + 3] == '-') && ((*h) [i + 4] == '#')) {
							
								for (j = i; j < (ct - 2); j++) {
								
									if (((*h) [j] == '-') && ((*h) [j + 1] == '-') && ((*h) [j + 2] == '>')) {
										
										flismacro = true;
										
										startmacro = i + 5;
										
										endmacro = j - 1;
										
										lenmacrooverhead = 5 + 3;
										
										goto processmacro;
										}
									}
								}
							}
						}
				#endif
				
				if ((ch == startmacrochar) && (!flcustommacrochars)) {
					
					if (remainingchars > 0) {
						
						short ctendsneeded = 1;
						
						for (j = i + 1; j < ct; j++) {
							
							char lch = (*h) [j];
							
							if (lch == endmacrochar) {
							
								if (--ctendsneeded == 0) {
							
									flismacro = true;
									
									startmacro = i + 1;
									
									endmacro = j - 1;
									
									lenmacrooverhead = 1 + 1;
									
									goto processmacro;
									}
								}
							else {
							
								if (lch == startmacrochar)
									++ctendsneeded;
								}
							}
						}
					}
			
				goto notmacro;
				
				processmacro: {
					long lenmacro;
					
					lenmacro = endmacro - startmacro + 1;
					
					if (!(*pmi).flprocessmacros) {

						i += lenmacro + lenmacrooverhead;
						}
					else {
								
						bigstring lerrorstring;
						Handle macro;
						Handle hmacroresult;

						if (!newhandle (lenmacro, &macro))
							return (false);
						
						moveleft (&(*h) [startmacro], *macro, lenmacro);
	
						if (flpagemillfile) {
							
							long ix;
							
							for (ix = 1; ix <= lenmacro; ix++) {
								
								if ((*macro) [ix] == '\r')
									(*macro) [ix] = ' ';
								} /*for*/
							}

						if (!htmlrunmacro (pmi, macro, lerrorstring, &hmacroresult)) {
							
							bigstring bs;
							boolean fllogerrors;
							
							if (hmacroresult == nil) /*6.2b6 AR: Assume that our thread has been killed and unwind recursion*/
								return (false);
							
							if (htmlgetbooleanpref (pmi, BIGSTRING ("\x0e" "logMacroErrors"), &fllogerrors) && fllogerrors) {
							
								if (!newhandle (lenmacro, &macro))
									return (false);
								
								moveleft (&(*h) [startmacro], *macro, lenmacro);
								
								htmlreportmacroerror (pmi, macro, lerrorstring);
								}
							
							parsedialogstring (str_macroerror, lerrorstring, nil, nil, nil, bs);
							
							if (!newtexthandle (bs, &hmacroresult))
								return (false);
							}
						
						if (!mergehandlestreamhandle (s, lenmacro + lenmacrooverhead, hmacroresult))
							return (false);
						}
					
					i--; /*DW 11/4/95*/
					
					flprocessed = true;
					}
					
				notmacro:
				
				if (flprocessmacrosinhtmltags) /*7.1b3 PBS: allow macros in HTML tags to be processed. Don't skip ahead.*/

					break;

				if (!flprocessed) {
				
					for (j = i; j < ct; j++) {
						
						if ((*h) [j] == '>') {
							
							i = j;
								
							break;
							}
						} /*for*/
					}
				
				break;
				}
				
			case '@': { /*process email addresses*/
				
				long startaddress, endaddress;
				long j; /*must be signed! -- loops need to go negative to terminate*/
				
				if (flinhtmltag)
					break;

				if (!(*pmi).flactiveurls)  /*4.1b11 dmb*/
					break;
				
				startaddress = 0; /*we might be at the beginning of the text handle*/
				
				for (j = i - 1; j >= 0; j--) { /*scan to the left, looking for the start of the mail address*/
					
					char lch = (*h) [j];
					
					if (!isAlphaChar (lch) && !isLegalURLPunctuationChar (lch)) {

						startaddress = j + 1;
						
						break;
						}
					} /*for*/
				
				if (startaddress == i) /*4.1b11 dmb*/
					break;
				
				endaddress = ct; /*we might be at the end of the text handle*/
				
				for (j = i + 1; j < ct; j++) { /*scan to the right, looking for the end of the mail address*/
				
					char lch = (*h) [j];
					
					if (!isAlphaChar (lch) && !isLegalURLPunctuationChar (lch)) {

						endaddress = j;
						
						break;
						}
					} /*for*/
				
				for (j = endaddress - 1; j > i; j--) { /*move endaddress to the left, past any non-alpha chars, like a comma after the mail address -- common!*/
					
					if (isAlphaChar ((*h) [j])) {
						
						endaddress = j;
						
						break;
						}
					} /*for*/
				
				if (endaddress - startaddress < 3) /*4.1b11 dmb*/
					break;
				
				/*6.1d1 AR: replace the text with a hot email address*/ {

					Handle hlink = nil;
					Handle hmail = nil;
					long len = endaddress - startaddress + 1;
					long ixload = startaddress;
					boolean fl;
					
					fl = newtexthandle (str_mailto, &hlink);

					fl = fl && loadfromhandletohandle (h, &ixload, len, true, &hmail);
					
					fl = fl && parsedialoghandle (hlink, hmail, nil, nil, nil);
					
					disposehandle (hmail);

					if (!fl) {

						disposehandle (hlink);

						return (false);
						}

					i = startaddress;
					
					if (!mergehandlestreamhandle (s, len, hlink)) /*consumes hlink*/
						return (false);
					
					i--;
					}
					
				break;
				}
				
			case '/': { /*process URL references, as in http://www.userland.com*/
			
				unsigned long startaddress, endaddress;
				long j; /*must be signed! -- loops need to go negative to terminate*/

				if (flinhtmltag)
					break;
				
				if (!(*pmi).flactiveurls)  /*4.1b11 dmb*/
					continue;
				
				if (remainingchars == 0)
					continue;
					
				if ((*h) [i + 1] != '/') 
					continue;
					
				startaddress = 0; /*we might be at the beginning of the text handle*/
				
				for (j = i - 1; j >= 0; j--) { /*scan to the left, looking for the start of the URL*/
					
					char lch = (*h) [j];
					
					if (isLegalURLPunctuationChar (lch))
						continue;
				
					if (!isAlphaChar (lch)) {
						
						startaddress = j + 1;
						
						break;
						}
					} /*for*/
					
				endaddress = ct; /*we might be at the end of the text handle*/ /*4.1b12 dmb: removed -1 bug*/
				
				for (j = i + 1; j < ct; j++) { /*scan to the right, looking for the end of the URL*/
				
					char lch = (*h) [j];
					
					if (isLegalURLPunctuationChar (lch))
						continue;
					
					if (!isAlphaChar (lch)) {
						
						endaddress = j;
						
						break;
						} /*for*/
					} /*for*/
					
				for (j = endaddress - 1; j > i; j--) { /*move endaddress to the left, past any non-alpha chars, like a comma after the mail address -- common!*/
					
					char lch = (*h) [j];
					
					if (isAlphaChar (lch) || (lch == '/')) {
						
						endaddress = j;
						
						break;
						}
					} /*for*/
									
				/*6.1d1 AR: replace the text with a hot URL link*/ {

					Handle hlink = nil;
					Handle hurl = nil;
					long ixload = startaddress;
					long len = endaddress - startaddress + 1;
					boolean fl;

					fl = newtexthandle (str_hotlink, &hlink);

					fl = fl && loadfromhandletohandle (h, &ixload, len, true, &hurl);

					fl = fl && parsedialoghandle (hlink, hurl, hurl, nil, nil);
					
					disposehandle (hurl);

					if (!fl) {

						disposehandle (hlink);

						return (false);
						}

					i = startaddress;
					
					if (!mergehandlestreamhandle (s, len, hlink)) /*consumes hlink*/
						return (false);
					
					i--;
					}
				
				break;
				}
				
			case '"': { /*possibly process a glossary entry*/
				long j;
				boolean flrewind = false;
				long ixrewind = 0;
				
				if (flinhtmltag)
					break;

				for (j = i + 1; j < ct; j++) {
					
					char lch = (*h) [j];
					
					if (flpagemillfile) {
					
						if (lch == '\r') { /*special hack for PageMill files*/
							
							(*h) [j] = lch = ' ';
							}
						}
					else {
						if (lch == '\r') /*unterminated quote*/
							break;
						}
					
					if (lch == '<') { /*7.0b39 PBS: rewind to this character if gloss lookup fails.*/

						if (!flrewind) {

							flrewind = true;

							ixrewind = j;
							} /*if*/
						} /*if*/


					if (lch == '"') {
						
						unsigned long startterm, endterm, lenterm;
						Handle hterm;
						bigstring lerrorstring;
						Handle hresult;
						long ixload;
						
						startterm = i + 1;
						
						endterm = j - 1;
						
						lenterm = endterm - startterm + 1;
						
						if (lenterm > maxglossarynamelength || !(*pmi).flexpandglossaryitems) {
							
							i += lenterm + 2 - 1; /*4.1b5: skip over the quoted string, less the last char*/

							if (flrewind) /*7.0b39 PBS: go back to < character.*/

								i = ixrewind - 1;
							
							break;
							}
												
						ixload = startterm;

						if (!loadfromhandletohandle (h, &ixload, lenterm, true, &hterm))
							return (false);

						if (!htmlrefglossary (pmi, hterm, lerrorstring, &hresult)) { /*error -- we don't substitute*/ 
							
							i += lenterm + 2 - 1; /**4.1b5: skip over the quoted string, less the last char*/

							if (flrewind) /*7.0b39 PBS: go back to < character.*/

								i = ixrewind - 1;
							
							break;
							}

						/*the term *was* in the glossary, perform the substitution*/
						
						if (!mergehandlestreamhandle (s, lenterm + 2, hresult)) /*consumes hresult*/
							return (false);
						
						--i; /*4.1b5: skip over the expanded glossary item, less the last char*/
						
						break;
						}
					} /*for*/
				
				break;
				}
				
			/* 4.1b12 dmb: these don't have html charactors equivalents, but we can do the mapping in the open-architecture isofilter */
			/*
			case 'Õ': case 'Ô':
				(*h) [i] = '\'';
				
				break;
				
			case 'Ó': case 'Ò':
				(*h) [i] = '"';
				
				break;
				
			case '¥': 
				(*h) [i] = 'o';
				
				break;
				
			case 'É':
				(*h) [i] = '.';
				
				if (!insertstringinhandle ("\p..", h, i + 1))
					return (false);
					
				ct = gethandlesize (h);
				
				i += 2;
				
				break;
				
			case 'Ð': case 'Ñ':
				(*h) [i] = '-';
				
				if (!insertstringinhandle ("\p-", h, i + 1))
					return (false);
					
				ct = gethandlesize (h);
				
				i += 1;
				
				break;
			*/
			/* 4.1b5 dmb: all of the following are html charactors and have iso8859 mappings */
			/*
			case 'Ê': // a funny space that Word produces
				(*h) [i] = ' ';
				
				break;
				
			case 'Ç':
				(*h) [i] = '&';
				
				if (!insertstringinhandle ("\plt;&lt;", h, i + 1))
					return (false);
					
				ct = gethandlesize (h);
				
				i += 7;
				
				break;
				
			case 'È':
				(*h) [i] = '&';
				
				if (!insertstringinhandle ("\pgt;&gt;", h, i + 1))
					return (false);
					
				ct = gethandlesize (h);
				
				i += 7;
				
				break;
				
			case '©':
				(*h) [i] = '&';
				
				if (!insertstringinhandle ("\pcopy;", h, i + 1))
					return (false);
					
				ct = gethandlesize (h);
				
				i += 5;
				
				break;
				
			case '¨':
				(*h) [i] = '&';
				
				if (!insertstringinhandle ("\preg;", h, i + 1))
					return (false);
					
				ct = gethandlesize (h);
				
				i += 4;
				
				break;
			*/

			} /*switch*/
		} /*for*/
	
	return (true);
	
	#undef ct
	#undef i
	} /*processhtmltext*/


static boolean autoparagraphs (handlestream *s) {
	
	//unsigned long ct, i,
	#define ct ((*s).eof)
	#define i ((*s).pos)
	Handle h = (*s).data;
	unsigned long remainingchars;
	char ch;
	boolean flinsidetag = false;
	
	//ct = gethandlesize (h);
	
	for (i = 0; i < ct; i++) {
	
		ch = (*h) [i];
		
		remainingchars = ct - i - 1;
		
		switch (ch) {
			
			case '<':
				flinsidetag = true;
				
				break;
				
			case '>':
				flinsidetag = false;
				
				break;
		
			case '\r': { /*insert a <p /> in front of a double-return*/
				
				if (!flinsidetag) {
			
					if (remainingchars > 0) { /*not the last char in the buffer*/
						
						char chnext = (*h) [i + 1];
						
						if (chnext == chreturn) { /*two returns, insert a pagebreak string*/
							
						//	if (!insertstringinhandle (pagebreakstring, h, i))
						//		return (false);
						//	
						//	ct = gethandlesize (h);
						//
						//	i += stringlength (pagebreakstring) + 1; /*point past the 2nd return*/
						
							if (!mergehandlestreamstring (s, 0, str_pagebreak))
								return (false);
							
							++i; //point past the 2nd return
							
							continue;
							}						
						}
					}
				
				break;
				}
			} /*switch*/
		} /*for*/
	
	return (true);
	
	#undef ct
	#undef i
	} /*autoparagraphs*/


#ifdef version5orgreater

static boolean iso8859encode (handlestream *s, hdlhashtable hiso8859usermap) {

	/*
	4.1b4 dmb: use user preferences for mapping, if table exists.
			   for extra speed, don't even try to mapp characters < ascii 128
	
	4.1b13 dmb: fixed heap-trashing off-by-one error in text replacement code
	
	5.0.2b14 dmb: take a handlestream, not a Handle
	
	5.0.2b16 dmb: backup after replacement, so we don't miss next char
	*/
	
	#define lentext ((*s).eof)
	#define ixtext ((*s).pos)
	Handle htext = (*s).data;
	unsigned char ch;
	Str255 bs;
	tyvaluerecord val;
	hdlhashnode hnode;

	
	#ifndef version5orgreater
		hdlhashtable hiso8859usermap = nil;
		
		if (getsystemtablescript (iduseriso8859map, bs)) {
			
			pushstring (BIGSTRING ("\x08" ".[\"128\"]"), bs);	/*refer to 1st entry in table*/
			
			disablelangerror ();
			
			if (langexpandtodotparams (bs, &hiso8859usermap, bs))
				; /*map being non-nil is our flag*/
			
			enablelangerror ();
			}
	#endif
	
	if (hiso8859usermap != nil)
		pushhashtable (hiso8859usermap);
	
	for (ixtext = 0; ixtext < lentext; ++ixtext) {

		ch = (*htext) [ixtext];

		if (ch <= 127)
			continue;
		
		if (hiso8859usermap != nil) {
			
			numbertostring ((long) ch, bs);
			
			if (!hashlookup (bs, &val, &hnode) || val.valuetype != stringvaluetype)
				continue;
			
			pullstringvalue (&val, bs);
			}
		else {
		
			if (iso8859table [ch] == 0) /*normal char, doesn't need processing*/
				continue;

			/*encode the character*/
			
			copyctopstring ((char *)(iso8859table [ch]), bs);	// 3/26/97 dmb: it's a c string now
			}
		
		if (!mergehandlestreamstring (s, 1, bs))
			return (false);
		
		--ixtext; // back up, so we can move ahead
		} /*while*/
	
	if (hiso8859usermap != nil)
		pophashtable ();
	
	return (true);
	
	#undef lentext
	#undef ixtext
	} /*iso8859encode*/


boolean processhtmlmacrosverb (hdltreenode hp1, tyvaluerecord *vreturned) {
	
	/*
	5.0.2b13 dmb: create handlestream here, and use for both processtext and autoparagraphs
	
	5.0.2b14 dmb: suck the whole damn thing into the kernel, changing our API
	
		on processMacros (s, plainprocessing = false) {

	6.1b17 AR: We were doing a lot of superfluous work to determine the page table,
				some of it even causing crashes. Calling getoptionalpagetableavalue
				is enough.	
	*/
	
	typrocessmacrosinfo pageinfo;
	boolean flplainprocessing = false;
	Handle htext = nil;
	handlestream s;
	
	// get all of the prefs and tables that we need
	
	clearbytes (&pageinfo, sizeof (pageinfo));
/*	
	if (!htmlgetpagetable (&pageinfo.hpagetable) && !htmlgetdefaultpagetable (&pageinfo.hpagetable))
		return (false);
*/	
	if (!htmlgetprefstable (&pageinfo.huserprefs))
		return (false);
	
	if (langgetparamcount (hp1) > 1) {
		
		if (!getbooleanvalue (hp1, 2, &flplainprocessing))
			return (false);
		}
/*	
	if (langgetparamcount (hp1) > 2) {
		
		flnextparamislast = true;
		
		if (!gettablevalue (hp1, 3, &pageinfo.hpagetable))
			return (false);
		}
*/	
	if (!getoptionalpagetablevalue (hp1, 3, &pageinfo.hpagetable))
		return (false);
	
	if (flplainprocessing) {
	
		pageinfo.flautoparagraphs = false;
		
		pageinfo.flclaycompatibility = false;
		
		pageinfo.flactiveurls = false;
		}
	else {
		if (!htmlgetbooleanpref (&pageinfo, BIGSTRING ("\x0e" "autoParagraphs"), &pageinfo.flautoparagraphs))
			return (false);
		
		if (!htmlgetbooleanpref (&pageinfo, BIGSTRING ("\x0a" "activeURLs"), &pageinfo.flactiveurls))
			return (false);
		
		if (!htmlgetbooleanpref (&pageinfo, BIGSTRING ("\x11" "clayCompatibility"), &pageinfo.flclaycompatibility))
			return (false);
		}
	
	if (!htmlgetbooleanpref (&pageinfo, BIGSTRING ("\x0d" "processMacros"), &pageinfo.flprocessmacros))
		return (false);
	
	if (!htmlgetbooleanpref (&pageinfo, BIGSTRING ("\x13" "expandGlossaryItems"), &pageinfo.flexpandglossaryitems))
		return (false);
	
	if (!htmlgetbooleanpref (&pageinfo, BIGSTRING ("\x09" "isoFilter"), &pageinfo.flisofilter))
		return (false);
	
	// get the text and process it!
	
	if (!getexempttextvalue (hp1, 1, &htext))
		return (false);
	
	openhandlestream (htext, &s);
	
	if (!processhtmltext (&s, &pageinfo))
		goto error;
	
	htmldisposemacrocontext (&pageinfo);

	if (pageinfo.flautoparagraphs) {
		
		if (!autoparagraphs (&s))
			goto error;
		}
	
	if (pageinfo.flisofilter) {
		
		hdlhashtable hisomap;
		
		if (!langfastaddresstotable (builtinstable, str_iso8859map, &hisomap))
			goto error;
		
		if (!iso8859encode (&s, hisomap))
			goto error;
		}
	
	closehandlestream (&s);
	
	return (setheapvalue (htext, stringvaluetype, vreturned));
	
	error: {
		
		htmldisposemacrocontext (&pageinfo);
		
		disposehandle (htext);
		
		return (false);
		}
	} /*processhtmlmacrosverb*/

#else

boolean processhtmlmacrosverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	5.0.2b13 dmb: create handlestream here, and use for both processtext and autoparagraphs
	*/
	
	Handle htext = nil;
	boolean flautoparagraphs, flactiveurls, claycompatibity;
	tyvaluerecord callbackval;
	tyaddress callback;
	ptraddress savecallback;
	handlestream s;
	
	if (!getbooleanvalue (hparam1, 2, &flautoparagraphs))
		return (false);
	
	if (!getbooleanvalue (hparam1, 3, &flactiveurls))
		return (false);
	
	if (!getbooleanvalue (hparam1, 4, &claycompatibity))
		return (false);
	
	flnextparamislast = true;
	
	#if version42orgreater
	
	if (!getaddressparam (hparam1, 5, &callbackval))
		return (false);
	
	if (!getaddressvalue (callbackval, &callback.ht, callback.bs))
		return (false);
	
	savecallback = callbackscript;
	
	callbackscript = &callback;
	
	#else
	
	if (!getbinaryvalue (hparam1, 5, true, &osaval.data.binaryvalue))
		return (false);
	
	#endif
	
	if (!getexempttextvalue (hparam1, 1, &htext))
		goto error;
	
	openhandlestream (htext, &s);
	
	if (!processtext (&s, flactiveurls, claycompatibity))
		goto error;
		
	if (flautoparagraphs) {
		
		if (!autoparagraphs (&s))
			goto error;
		}
	
	closehandlestream (&s);
	
	callbackscript = savecallback;
	
	return (setheapvalue (htext, stringvaluetype, vreturned));
	
	error: {
		
		callbackscript = savecallback;
		
		disposehandle (htext);
		
		return (false);
		}
	} /*processhtmlmacrosverb*/

#endif

#pragma mark === stringOps ucmd ===

static unsigned char hexchartonum (unsigned char ch) {

	if ((ch >= 'a') && (ch <= 'z')) /*DW 10/13/95 -- fix for toys.parseArgs*/
		ch -= 32;

	if ((ch >= '0') && (ch <= '9'))
		ch = ch - '0';
	else
		ch = (ch - 'A') + 10;
	
	return (ch);
	} /*hexchartonum*/


static unsigned char numtohexchar (unsigned char ch) {

	if (ch < 10)
		ch = ch + '0';
	else
		ch = (ch - 10) + 'A';
	
	return (ch);
	} /*numtohexchar*/


static void decodehandle (Handle htext) {

	/*
	5.1.5b12 dmb: make sure there are two characters following %
	*/
	
	unsigned long ixtext, lentext;
	unsigned char *p;

	p = (unsigned char *) *htext;

	lentext = gethandlesize (htext);

	ixtext = 0;

	while (true) {

		if (ixtext >= lentext)
			break;

		switch (*p) {

			case '%':
				if (lentext - ixtext > 2) { // there are two characters following %
					
					unsigned char ch1 = *(p + 1), ch2 = *(p + 2);

					//{Str255 s; s [0] = 3; s [1] = '%'; s [2] = ch1; s [3] = ch2; DebugStr (s);}

					*p = (hexchartonum (ch1) * 16) + hexchartonum (ch2);
					
					moveleft (p + 3, p + 1, lentext - ixtext - 3);

					lentext -= 2;
					}

				break;

			case '+':
				*p = ' ';

				break;
			} /*switch*/

		p++;

		ixtext++;
		} /*while*/

	sethandlesize (htext, lentext);
	} /*decodehandle*/


boolean urldecodeverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	Handle htext;
	
	flnextparamislast = true;
	
	if (!getexempttextvalue (hparam1, 1, &htext))
		return (false);
	
	decodehandle (htext);
	
	return (setheapvalue (htext, stringvaluetype, vreturned));
	} /*urldecodeverb*/


static void encodehandle (Handle htext) {
	
	/*
	4.1b5 dmb: fixed moveleft data length calculation so we don't overwrite memory
	
	5.0d12 dmb: gotta encode % too.
	*/
	
	unsigned long ixtext;
	unsigned long lentext;
	unsigned char ch;

	ixtext = 0;

	lentext = gethandlesize (htext);

	while (true) {

		if (ixtext >= lentext)
			break;

		ch = (*htext) [ixtext];

		if ((ch < 33) || (ch > 126))
			goto hexpush;

		switch (ch) {

			case ' ': case '$': case '&': case '=': case '+': case '"': 
			case '\\': case '\'': case ':': case '/': case '#': case '?':
			case '%':
				goto hexpush;

			} /*switch*/

		/*normal character, doesn't need encoding*/

		ixtext++;

		continue;

		hexpush: /*encode the character*/

		sethandlesize (htext, lentext + 2);

		moveright (*htext + ixtext + 1, *htext + ixtext + 3, lentext - ixtext - 1); /*4.1b5: added -1*/

		(*htext) [ixtext++] = '%';

		(*htext) [ixtext++] = numtohexchar (ch / 16);

		(*htext) [ixtext++] = numtohexchar (ch % 16);

		lentext += 2;
		} /*while*/
	} /*encodehandle*/


boolean urlencodeverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	Handle htext;
	
	flnextparamislast = true;
	
	if (!getexempttextvalue (hparam1, 1, &htext))
		return (false);
	
	encodehandle (htext);
	
	return (setheapvalue (htext, stringvaluetype, vreturned));
	} /*urlencodeverb*/


static boolean nthfieldhandle (Handle htext, byte fielddelim, long fieldnum, Handle *hreturnedtext) {

	/*
	6.1b18 AR: Use textnthword which is proven to work. The previous version of this function
	was buggy, e.g. it didn't work correctly if the first char was a field delimiter.
	*/

	unsigned long ixload = 0;
	unsigned long ctload = 0;
	
	if (textnthword ((ptrbyte)*htext, gethandlesize (htext), fieldnum, fielddelim, true, (long *)(&ixload), (long *)(&ctload)))
		return (loadfromhandletohandle (htext, (long *)(&ixload), ctload, false, hreturnedtext));

	return (newemptyhandle (hreturnedtext));
	} /*nthfieldhandle*/


boolean parseargsverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	/*
	6.1b7 AR: Bug fix: We no longer depend on the field value
	to not contain "=" signs.
	*/

	#ifdef oplanglists
		Handle htext, hreturnedtext = nil, hfieldname, hfieldvalue;
		hdllistrecord list = nil;
		short fieldnum = 1;
		long lenfield, lenfieldname;

		flnextparamislast = true;
		
		if (!getexempttextvalue (hparam1, 1, &htext))
			return (false);
		
		if (!opnewlist (&list, false))
			goto error;
		
		while (true) {
			
			if (!nthfieldhandle (htext, '&', fieldnum++, &hreturnedtext))
				break;

			lenfield = gethandlesize (hreturnedtext);
			
			if (lenfield == 0) { //ran out of fields
				
				disposehandle (hreturnedtext);
				
				break;
				}
			
			if (!nthfieldhandle (hreturnedtext, '=', 1, &hfieldname))
				goto error;
			
			lenfieldname = gethandlesize (hfieldname);

			decodehandle (hfieldname);
			
			if (!langpushlisttext (list, hfieldname))
				goto error;

/*			if (!nthfieldhandle (hreturnedtext, '=', 2, &hfieldvalue))
				goto error;
*/
			if (lenfieldname < lenfield)
				lenfieldname++;
			
			if (!loadhandleremains (lenfieldname, hreturnedtext, &hfieldvalue))
				goto error;
			
			decodehandle (hfieldvalue);

			if (!langpushlisttext (list, hfieldvalue))
				goto error;

			disposehandle (hreturnedtext);
			}
		
		disposehandle (htext);
		
		return (setheapvalue ((Handle) list, listvaluetype, vreturned));

		error: {
		
			disposehandle (hreturnedtext);
			
			disposehandle (htext);
			
			opdisposelist (list);

			return (false);
			}
	#else
		Handle htext, hreturnedtext = nil, hfieldname, hfieldvalue;
		AEDescList list = {typeNull, nil};
		short fieldnum = 1, ixlist = 1;

		flnextparamislast = true;
		
		if (!getexempttextvalue (hparam1, 1, &htext))
			return (false);
		
		if (!IACnewlist (&list))
			goto error;
		
		while (true) {
			
			if (!nthfieldhandle (htext, '&', fieldnum++, &hreturnedtext))
				break;
			
			if (gethandlesize (hreturnedtext) == 0) { //ran out of fields
				
				disposehandle (hreturnedtext);
				
				break;
				}
			
			if (!nthfieldhandle (hreturnedtext, '=', 1, &hfieldname))
				goto error;
			
			decodehandle (hfieldname);
			
			if (!IACpushtextitem (&list, hfieldname, ixlist++))
				goto error;
			
			if (!nthfieldhandle (hreturnedtext, '=', 2, &hfieldvalue))
				goto error;
			
			decodehandle (hfieldvalue);
			
			if (!IACpushtextitem (&list, hfieldvalue, ixlist++))
				goto error;
			
			disposehandle (hreturnedtext);
			}
		
		disposehandle (htext);
		
		
			{
			Handle h;
			
			copydatahandle (&list, &h);
			
			return (setheapvalue (h, listvaluetype, vreturned));			
			}
		
		
		error: {
		
			disposehandle (hreturnedtext);
			
			disposehandle (htext);
			
			AEDisposeDesc (&list); // 5.0d12 dmb

			if (!fllangerror)
				oserror (IACglobals.errorcode);

			return (false);
			}
	#endif
	} /*parseargsverb*/


boolean iso8859encodeverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	Handle htext;
	hdlhashtable htable;
	handlestream s;
	
	flnextparamislast = true;
	
	if (!gettablevalue (hparam1, 2, &htable))
		return (false);
	
	if (!getexempttextvalue (hparam1, 1, &htext))
		return (false);
	
	openhandlestream (htext, &s);
	
	if (!iso8859encode (&s, htable))
		return (false);
	
	closehandlestream (&s);
	
	return (setheapvalue (htext, stringvaluetype, vreturned));
	} /*iso8859encodeverb*/


static boolean getGifBounds (hdlfilenum fnum, unsigned short * height, unsigned short * width) {

	unsigned char buf[20];	/*0-2 - signature, 3-5 - version, 6 low byte width, 7 high byte width, 8-9 low/high height*/
	
	if (fileread (fnum, 10, buf)) {
		*width = ((unsigned short) buf[7] << 8) + (unsigned short)buf[6];
		*height = ((unsigned short) buf[9] << 8) + (unsigned short)buf[8];
		return (true);
		}

	return (false);
	} /*getGifBounds*/


boolean getgifheightwidthverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyfilespec fs;
	hdlfilenum fnum;
	boolean fl;
	unsigned short height, width;
	hdllistrecord list = nil;

	flnextparamislast = true;
	
	if (!getfilespecvalue (hparam1, 1, &fs))
		return (false);

	fl = openfile (&fs, &fnum, true);

	if (fl) {
	
		fl = getGifBounds (fnum, &height, &width);
		
		closefile(fnum);
		
		if (fl) {
			#ifdef xxxoplanglists
				if (opnewlist (&list, false)) {
					if (langpushlistlong (list, (long) height)) {
						if (langpushlistlong (list, (long) width)) {
							return (setheapvalue ((Handle) list, listvaluetype, vreturned));
							}
						}
					}
			#else
				Point pt;
				pt.v = width;
				pt.h = height;
				return (setpointvalue (pt, vreturned) && coercetolist (vreturned, listvaluetype));
			#endif
			}
		}

	if (list != nil)
		opdisposelist (list);

	return (false);
	} /*getgifheightwidthverb*/


// This is JPEG code courtsey of Jim Correia  correia@barebones.com

#define M_SOF0  0xC0        // Start Of Frame N 
#define M_SOF1  0xC1        // N indicates which compression process 
#define M_SOF2  0xC2        // Only SOF0-SOF2 are now in common use 
#define M_SOF3  0xC3
#define M_SOF5  0xC5        // NB: codes C4 and CC are NOT SOF markers 
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8        // Start Of Image (beginning of datastream) 
#define M_EOI   0xD9        // End Of Image (end of datastream) 
#define M_SOS   0xDA        // Start Of Scan (begins compressed data) 
#define M_COM   0xFE        // COMment 


static boolean read_1_byte(hdlfilenum file_ref, unsigned char * c) {

    long    count = 1;
    return  (fileread(file_ref, count, c));
	} /*read_1_byte*/


static boolean read_2_bytes(hdlfilenum file_ref, unsigned short * b) {

    long    count = 2;
	boolean res;

    res = fileread(file_ref, count, b);

	*b = conditionalshortswap(*b);

	return (res);
	} /*read_2_bytes*/


static boolean first_marker(hdlfilenum file_ref, unsigned char * c) {

    boolean           result = noErr;
    unsigned char   c1,c2;
    long            count;
    
    count = 1;
    result = fileread(file_ref, count, &c1);
    
    if ( result == true ) {
        count = 1;
        result = fileread(file_ref, count, &c2);
		}
    
    if ( result == true ) {
        if ( ( c1 != 0xFF ) && ( c2 != M_SOI ) )
            result = false;
		}
    
    *c = c2;
    
    return (result);
	} /*first_marker*/


static boolean next_marker(hdlfilenum file_ref, unsigned char * c) {

    boolean           result = true;
    long            discarded_bytes = 0;

	//  Find 0xFF byte; count and skip any non-FFs.
    
    result = read_1_byte(file_ref, c);
    while ( ( result == true ) && (*c != 0xFF) ) {
        discarded_bytes++;
        result = read_1_byte(file_ref, c);
		}
    
	//  Get marker code byte, swallowing any duplicate FF bytes.     Extra FFs
	//  are legal as pad bytes, so don't count them in discarded_bytes.
    
    do {
		result = read_1_byte(file_ref, c);
    } while (( result == true ) && (*c == 0xFF));

    return (result);
	}  /*next_marker*/


static boolean skip_variable (hdlfilenum file_ref) {

    boolean result = true;
    unsigned short length;

    // Get the marker parameter length count
    result = read_2_bytes(file_ref, &length);

    // Length includes itself, so must be at least 2
    if ( ( result == true ) && ( length < 2 ) )
        result = false;
        
    if ( result == true ) {
        length -= 2;

        // Skip over the remaining bytes

        while ( ( result == true ) && ( length > 0 ) ) {
            unsigned char c;
            result = read_1_byte(file_ref, &c);
            length--;
			}
		}

	return (result);		//Added RAB: 1/29/98
	} /*skip_variable*/


static boolean ScanJPEGHeader(hdlfilenum file_ref, unsigned short * height, unsigned short * width) {

    boolean   result = true;
    unsigned char marker;
    boolean done = false;
    
    *height = 0;
    *width = 0;

    // Expect SOI at start of file
    if ( ( true != first_marker(file_ref, &marker) ) || ( marker != M_SOI ) ) {
        result = false;
		}

    // Scan miscellaneous markers until we reach SOS.
        
    if ( result != true )
        done = true;
    
    while ( ! done ) {
        result = next_marker(file_ref, &marker);

        if ( result != true )
            break;      
        
        switch (marker)	{
            case M_SOF0:        // Baseline 
            case M_SOF1:        // Extended sequential, Huffman 
            case M_SOF2:        // Progressive, Huffman 
            case M_SOF3:        // Lossless, Huffman 
            case M_SOF5:        // Differential sequential, Huffman 
            case M_SOF6:        // Differential progressive, Huffman 
            case M_SOF7:        // Differential lossless, Huffman 
            case M_SOF9:        // Extended sequential, arithmetic 
            case M_SOF10:       // Progressive, arithmetic 
            case M_SOF11:       // Lossless, arithmetic 
            case M_SOF13:       // Differential sequential, arithmetic 
            case M_SOF14:       // Differential progressive, arithmetic 
            case M_SOF15:       // Differential lossless, arithmetic 
					{
                    unsigned short  length;
                    unsigned short  image_height, image_width;
                    unsigned char   data_precision;
                    
                    result = read_2_bytes(file_ref, &length);
                    
                    if ( ( result == true ) && ( length < 7 ) )
                        result = false;
                    
                    if ( result == true )
                        result = read_1_byte(file_ref, &data_precision);
                        
                    if ( result == true )
                        result = read_2_bytes(file_ref, &image_height);
                        
                    if ( result == true )
                        result = read_2_bytes(file_ref, &image_width);

                    if ( result == true ) {
                        *height = image_height;
                        *width = image_width;
						}
                    
                    done = true;
					}
                break;
    
            case M_SOS:             // stop before hitting compressed data 
                done = true;
                break;
    
            case M_EOI:             // in case it's a tables-only JPEG stream 
                done = true;
                break;
    
            case M_COM:
                skip_variable(file_ref);
                break;
    
            default:                // Anything else just gets skipped 
                skip_variable(file_ref);    // we assume it has a parameter count... 
                break;
			}
		}
            
    return (result);
	} /*ScanJPEGHeader*/


boolean getjpegheightwidthverb ( hdltreenode hparam1, tyvaluerecord *vreturned ) {

	boolean fl;
	hdlfilenum fnum;
	hdllistrecord list = nil;
	tyfilespec fs;
	unsigned short height, width;

	flnextparamislast = true;
	
	if ( ! getfilespecvalue ( hparam1, 1, &fs ) )
		return ( false );

	fl = openfile ( &fs, &fnum, true );

	if ( fl ) {
		
		fl = ScanJPEGHeader ( fnum, &height, &width );
		
		closefile ( fnum );
		
		if ( fl ) {
		
			#ifdef xxxoplanglists
			
				if ( opnewlist ( &list, false ) ) {
				
					if ( langpushlistlong ( list, ( long ) height ) ) {
					
						if ( langpushlistlong ( list, ( long ) width ) )
						
							return ( setheapvalue ( ( Handle ) list, listvaluetype, vreturned ) );
							
						}
					}
			#else
				Point pt;
				
				pt.v = width;
				pt.h = height;
				
				return ( setpointvalue ( pt, vreturned ) && coercetolist ( vreturned, listvaluetype ) );
				
			#endif
			
			}
		}
	
	if ( list != nil )
		opdisposelist ( list );
	
	
	if ( ! fl ) // JES: 10/28/2002, 9.1b1 -- ScriptError if the file couldn't be parsed for JPEG height/width instead of a silent failure
	
		langerrormessage ( BIGSTRING ( "\x49""Can't get JPEG height and width because the file isn't a valid JPEG file." ) );
	
	return ( false );
	
	} // getjpegheightwidthverb


#pragma mark === build pagtable ===

static tyvaluetype langgetextendedvaluetype (const tyvaluerecord *val) {
	
	/*
	5.0.2 dmb: another missing bit of langexternal functionality
	*/

	if ((*val).valuetype != externalvaluetype)
		return ((*val).valuetype);
	
	return (tyvaluetype) (outlinevaluetype + langexternalgettype (*val));
	} /*langgetextendedvaluetype*/


static boolean additemtopagetable (hdlhashtable htable, hdlhashnode hnode, hdlhashtable hpagetable) {

	/*
	on addItemToPageTable (adr) { Çadr points to an attribute
	*/
	
	tyaddress adr;
	tyvaluetype objecttype;
	tyvaluerecord val;
	bigstring name;
	boolean fl;
	hdldatabaserecord hdb;
	
	adr.ht = htable;
	gethashkey (hnode, adr.bs);
	
	//	local (name = nameOf (adr^));
	copystring (adr.bs, name);
	
	//	if name beginsWith "#" {
	//		name = string.delete (name, 1, 1)};
	
	if (getstringcharacter (name, 0) == '#')
		deletestring (name, 1, 1);
	
	//	if not defined (adrpagetable^.[name]) {
	if (!hashtablesymbolexists (hpagetable, name)) {
	
		//	local (lowername = string.lower (name));
		bigstring lowername;
		copystring (name, lowername);
		alllower (lowername);
		
		//	local (objecttype = typeOf (adr^));
		//	case objecttype {
		//		outlinetype;
		//		tabletype {
		//			if lowername == "prefs" {
		//				local (i);
		//				for i = 1 to sizeOf (adr^) {
		//					addItemToPageTable (@adr^ [i])}}
		//			else {
		//				adrpagetable^.[name] = adr}}}
		//	else {
		//		adrpagetable^.[name] = string (adr^)};
		
		val = (**hnode).val;
		objecttype = langgetextendedvaluetype (&val);
		
		switch (objecttype) {
			
			case tablevaluetype:
				if (equalstrings (lowername, BIGSTRING ("\x05" "prefs"))) {
					hdlhashtable ht;
					hdlhashnode hn;
					
					if (langexternalvaltotable (val, &ht, hnode)) { // should never fail
						
						for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink)
							if (!additemtopagetable (ht, hn, hpagetable))
								return (false);
						
						return (true);
						}
					}
				// else fall through
			case outlinevaluetype: 
				if (!langassignaddressvalue (hpagetable, name, &adr))
					return (false);
				
				break;
			
			default:
				hdb = tablegetdatabase (htable);

				if (hdb)
					dbpushdatabase (hdb);

				fl = copyvaluerecord (val, &val);

				if (hdb)
					dbpopdatabase ();

				if (!fl || !strongcoercetostring (&val))
					return (false);
				
				if (!hashtableassign (hpagetable, name, val))
					return (false);
				
				exemptfromtmpstack (&val);
				
				break;
			}
		
		//	if lowername == "ftpsite" { // dmb: move this into buildpagetableverb
		//		adrpagetable^.subdirectoryPath = subdirpath;
		//		adrpagetable^.adrSiteRootTable = nomad}; Ç4.2
		
		//	if lowername == "template" { Ç4.1, 4.2
		//		if (objecttype == wptexttype) or (objecttype == outlinetype) {
		//			ÇWed, Nov 20, 1996 at 7:58:21 AM by DW -- allow outlines to be templates
		//			adrpagetable^.indirectTemplate = false;
		
		if (equalstrings (lowername, str_template)) {
			
			if ((objecttype == wordvaluetype) || (objecttype == outlinevaluetype)) {
				
				if (!langassignbooleanvalue (hpagetable, str_indirecttemplate, false))
					return (false);
					
				//	if objecttype == outlinetype {
				//		adrpagetable^.[name] = adr^}}}};
				
				if (objecttype == outlinevaluetype) {
				
					if (!copyvaluerecord (val, &val))
						return (false);
					
					if (!hashtableassign (hpagetable, name, val))
						return (false);
					
					exemptfromtmpstack (&val);
					}
				}
			}
		}
				
	return (true);
	} /*additemtopagetable*/


static boolean buildpagetableverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	/*
	on buildTable (adrobject, adrpagetable) {
	*/

	hdlhashtable hpagetable;
	tyaddress nomad;
	hdlhashtable nomadtable;
	hdlhashnode hn;
	bigstring subdirpath;
	byte pc = ':';
	
	if (!getvarparam (hparam1, 1, &nomad.ht, nomad.bs))
		return (false);
	
	flnextparamislast = true;
	
	if (!gettablevalue (hparam1, 2, &hpagetable))
		return (false);

	//	local (nomad = parentOf (adrobject^), subdirpath = "");
	setemptystring (subdirpath);
	
	//	loop { Çpop out to the root looking for #directives
	while (true) {
			
		//	local (i);
		//	if nomad == nil or nomad == @root {
		//		break};
		nomadtable = nomad.ht;
		if ((nomadtable == roottable) || !findinparenttable (nomadtable, &nomad.ht, nomad.bs))
			break;
		
		//	for i = 1 to sizeOf (nomad^) {
		for (hn = (**nomadtable).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
			
			bigstring name;
			
			gethashkey (hn, name);
			
			//	if nameOf (nomad^ [i]) beginsWith "#" {
			//		addItemToPageTable (@nomad^ [i])}
			if (getstringcharacter (name, 0) == '#') {
				
				//	if lowername == "ftpsite" {	 // dmb: pulled this out of additemtopagetable
					//	adrpagetable^.subdirectoryPath = subdirpath;
					//	adrpagetable^.adrSiteRootTable = nomad}; Ç4.2
				if (equalidentifiers (name, BIGSTRING ("\x08" "#ftpsite")) && !hashtablesymbolexists (hpagetable, str_ftpsite)) {
					
					if (!langassignstringvalue (hpagetable, BIGSTRING ("\x10" "subdirectoryPath"), subdirpath))
						return (false);
					
					if (!langassignaddressvalue (hpagetable, BIGSTRING ("\x10" "adrSiteRootTable"), &nomad))
						return (false);
					}
				
				if (!additemtopagetable (nomadtable, hn, hpagetable))
					return (false);
				}
			
			//	else {
			//		break}};
			//else
			//	break;
			}
		
		//	if defined (nomad^.tools) {
		//		addItemToPageTable (@nomad^.tools)};
		
		if (hashtablelookupnode (nomadtable, BIGSTRING ("\x05" "tools"), &hn))
			additemtopagetable (nomadtable, hn, hpagetable);
		
		//	if defined (nomad^.glossary) {
		//		addItemToPageTable (@nomad^.glossary)};
		if (hashtablelookupnode (nomadtable, BIGSTRING ("\x08" "glossary"), &hn))
			additemtopagetable (nomadtable, hn, hpagetable);
		
		//	subdirpath = nameOf (nomad^) + pc + subdirpath;
		//	nomad = parentOf (nomad^)}
		insertchar (pc, subdirpath);
		insertstring (nomad.bs, subdirpath);
		}
	
	return (setbooleanvalue (true, vreturned));
	} /*buildpagetableverb*/


#if 0

static boolean refglossaryverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on refGlossary (name)
		Ç5.0 DW -- html.buildObject sets up a global for us, html.data.adrPageTable
			Çit points to the pagetable for the page currently being built
			Çwe're called from inside html.processMacros, which is protected by a semaphore
			Çif you're calling this routine directly, you must set up html.data.adrPageTable yourself.
			Ç11/12/97 at 8:34:30 AM by DW		
	*/
	
	typrocessmacrosinfo pageinfo;
	Handle href, hresult;
	bigstring lerrorstring;
	handlestream s;
	
	// get all of the prefs and tables that we need
	
	if (!htmlgetpagetable (&pageinfo.hpagetable) || !htmlgetprefstable (&pageinfo.huserprefs))
		return (false);
	
	return (htmlrefglossary (&pageinfo, href, lerrorstring, &hresult));
	} /*refglossaryverb*/

#endif


#pragma mark === verbs ===

static boolean getprefverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on getPref (prefName, adrpagedata=@websites.["#data"]) { Çnew in 4.1
		ÇLook for a preference directive, a global pref, or return a default
	*/
	
	bigstring prefname;
	typrocessmacrosinfo pageinfo;
	
	if (!getstringvalue (hp1, 1, prefname))
		return (false);
	
	if (!getoptionalpagetablevalue (hp1, 2, &pageinfo.hpagetable))
		return (false);
	
	if (!htmlgetprefstable (&pageinfo.huserprefs))
		return (false);
	
	return (htmlgetpref (&pageinfo, prefname, v));
	} /*getprefverb*/


#if 0

static boolean getonedirectiveverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on getOneDirective (directiveName, s) { Çnew in 4.0.1
		ÇRevised for ContentServer.
			ÇFriday, March 13, 1998 at 9:47:37 PM by PBS
			ÇNow case-insensitive.
			ÇRespects directivesOnlyAtBeginning pref.
		ÇOld code
			Çlocal (ix = string.patternMatch (string.lower (directivename), string.lower (s)))
			Çif ix > 0
				Çs = string.delete (s, 1, ix + sizeof (directivename))
				Çs = string.delete (s, string.patternmatch (cr, s), infinity)
				Çreturn (evaluate (s))
			Çelse
				Çreturn ("")
		local (value = "");
		local (flDirectivesOnlyAtBeginning = html.getPref ("directivesOnlyAtBeginning"));
		if typeOf (s) == outlineType {
			flDirectivesOnlyAtBeginning = false};
		table.assign (@s, string.replaceAll (string (s), "\n", ""));
		if directiveName beginsWith "#" { //pop off leading # character
			directiveName = string.mid (directiveName, 2, infinity)};
		loop { //loop through directives
			if sizeof (s) == 0 {
				break};
			local (line = string.nthField (s, "\r", 1));
			if line beginsWith "#" {
				local (name);
				name = string.nthField (line, ' ', 1); //get the name of the directive
				name = string.mid (name, 2, infinity); //pop off leading # character
				if string.lower (name) == string.lower (directiveName) { //is this the directive asked for?
					local (ix = string.patternMatch (" ", line));
					value = string.mid (line, ix + 1, infinity);
					return (evaluate (value))}}
			else {
				if flDirectivesOnlyAtBeginning {
					break}};
			s = string.delete (s, 1, sizeof (line) + 1);
			if sizeOf (s) < 3 {
				break}};
		return (value)}
	*/
	
	} /*getonedirectiveverb*/

#endif


static boolean htmlrundirective (typrocessmacrosinfo *pmi, Handle s, bigstring fieldname) {
	
	/*
	run the directive, consuming it.
	
	the only field we look at in pmi is the pagetable
	*/
	
	bigstring lerrorstring;
	tyvaluerecord val;
	
	textfirstword ((ptrbyte)(*s), gethandlesize (s), chspace, fieldname);
	
	pullfromhandle (s, 0, stringlength (fieldname) + 1, nil);
	
	if (langruntraperror (s, &val, lerrorstring)) {
		
		if (!hashtableassign ((*pmi).hpagetable, fieldname, val))
			return (false);
		
		exemptfromtmpstack (&val);
		}
	else {
		lang2paramerror (evaldirectiveerror, fieldname, lerrorstring);
		
		return (false);
		}
	
	alllower (fieldname);
	
	if (equalstrings (fieldname, str_template))
		langassignbooleanvalue ((*pmi).hpagetable, str_indirecttemplate, true);
	
	return (true);
	} /*htmlrundirective*/


static boolean rundirectiveverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on runDirective (linetext, adrpagetable=@websites.["#data"]) { Ç4.2 -- extracted from renderObject macro
		Çlinetext contains a #directive line
			Çprocess the directive and return the name of the directive
		Ç11/12/97 at 8:44:54 AM by DW -- adrpagetable is an optional param
			ÇHad to change the implementation.
			ÇOld method: build a script that's an assignment statement.
			ÇNew method: evaluate the expression and assign into the pagetable field.
			ÇThe old method for constructing an address won't work for non-global tables.
			ÇI don't believe this will break anything.
		
		local (s = string.commentDelete (linetext));
		local (fieldname = string.nthField (s, ' ', 1));
		s = string.delete (s, 1, sizeof (fieldname) + 1); Çdelete name and space
		try {delete (@adrpagetable^.[fieldname])}; Çavoid Can't Assign Over error
		
		try {
			adrpagetable^.[fieldname] = evaluate (s)}
		else {
			scriptError ("Error evaluating #" + linetext + ": " + tryError)};
		
		local (lastdirective = string.lower (fieldname)); Ç4.2
		if lastdirective == "template" { Ç4.0.2
			adrpagetable^.indirectTemplate = true};
		return (lastdirective)}
	*/
	
	Handle s;
	typrocessmacrosinfo pageinfo;
	bigstring fieldname;
	
	if (!getexempttextvalue (hp1, 1, &s))
		return (false);
	
	if (!textcommentdelete (s))
		goto error;
	
	if (!getoptionalpagetablevalue (hp1, 2, &pageinfo.hpagetable))
		goto error;

	if (!htmlrundirective (&pageinfo, s, fieldname))
		goto error;
	
	return (setstringvalue (fieldname, v));
	
	error:
		disposehandle (s);
		
		return (false);
	} /*rundirectiveverb*/


static boolean rundirectivesverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	on runDirectives (wpstring, adrpagetable=@websites.["#data"]) { Ç4.2 -- extracted from renderObject macro
		Çwpstring contains a page of text
			Çrun all the #directives and return the cleaned up text
		
		wpstring = string.replaceAll (wpstring, "\n", ""); Çwork around Windows problem -- 11/11/97 DW
		local (s = "");
		loop { Çprocess #directives
			if sizeof (wpstring) == 0 {
				break};
			local (line = string.nthField (wpstring, "\r", 1));
			if line beginsWith "#" {
				html.runDirective (string.delete (line, 1, 1), adrpagetable)}
			else {
				if html.getPref ("directivesOnlyAtBeginning", adrpagetable) {
					s = s + wpstring;
					break};
				s = s + line + "\r"};
			wpstring = string.delete (wpstring, 1, sizeof (line) + 1)};
		return (s)}
	
	5.0.2b16 dmb: finished kernelization; fixed hang when not fldirectivesonlyatbeginning
	*/
	
	Handle wpstring;
	handlestream s;
	long ct, pos;
	Handle line;
	boolean fldirectivesonlyatbeginning;
	typrocessmacrosinfo pageinfo;
	bigstring bsdirective;
	
	if (!getoptionalpagetablevalue (hp1, 2, &pageinfo.hpagetable))
		return (false);
	
	if (!htmlgetprefstable (&pageinfo.huserprefs))
		return (false);
	
	if (!htmlgetbooleanpref (&pageinfo, str_directivesonlyatbeginning, &fldirectivesonlyatbeginning))
		return (false);
	
	if (!getexempttextvalue (hp1, 1, &wpstring))
		return (false);
	
	openhandlestream (wpstring, &s);
	
	bundle { // wpstring = string.replaceAll (wpstring, "\n", ""); Çwork around Windows problem -- 11/11/97 DW
		for (s.pos = 0; s.pos < s.eof; ++s.pos) {
			
			if ((*wpstring) [s.pos] == '\n')
				pullfromhandlestream (&s, 1, nil);
			}
		
		s.pos = 0; // reset
		}
	
	while (!athandlestreameof (&s)) { // process #directives
		
		for (pos = s.pos; pos < s.eof; ++pos)
			if ((*wpstring) [pos] == '\r')
				break;
		
		// pos is at eof, or a return
		ct = pos - s.pos;
		
		if ((ct > 0) && ((*wpstring) [s.pos] == '#')) { // found a directive line
			
			if (!newhandle (ct, &line))
				break;
			
			pullfromhandlestream (&s, ct, *line); // pull directive out of wptext
			
			if (!athandlestreameof (&s))
				pullfromhandlestream (&s, 1, nil); // strip cr of directive line
			
			pullfromhandle (line, 0, 1, nil); // strip # from directive
			
			if (!htmlrundirective (&pageinfo, line, bsdirective)) // run the directive
				break;
			}
		else {
			
			if (fldirectivesonlyatbeginning)
				break;
			
			s.pos = pos + 1; // just past cr, or eof
			}
		}
	
	closehandlestream (&s);
	
	return (!fllangerror && setheapvalue (wpstring, stringvaluetype, v));
	} /*rundirectivesverb*/


static boolean runoutlinedirectivesverb (hdltreenode hp1, tyvaluerecord *v) {
#pragma unused(v)

	/*
	on runOutlineDirectives (adroutline, adrpagetable=@websites.["#data"]) { Ç4.2
		Çthe outline can contain #directives
			Çrun all the #directives and return the outline with the directives deleted
			Çplease send us a *COPY* of your outline. thanks!
	*/
	
	typrocessmacrosinfo pageinfo;
	hdloutlinerecord ho;
	hdlheadrecord nomad, nextnomad, hsummit;
	bigstring lastdirective;
	bigstring objectname;
	tyvaluerecord val;
	tyexternalid newtype;
	Handle hdirective;
	boolean fl = false;
	
	if (!getoutlinevalue (hp1, 1, &ho))
		return (false);
	
	if (!getoptionalpagetablevalue (hp1, 2, &pageinfo.hpagetable))
		return (false);
	
	//	local (oldtarget = target.get ());
	//	target.set (adroutline);
	//	op.firstSummit ();
	//	loop {
	oppushoutline (ho);
	
	nomad = (**ho).hsummit;
	
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;

		//	local (s = op.getLineText ());
		//	if s beginsWith "#" {

		/*
		opgetheadstring (nomad, bs);
		
		if (stringlength (bs) > 0 && getstringcharacter (bs, 0) == '#')
		*/

		hdirective = (**nomad).headstring; /*be sure to make a copy before modifying*/

		if ((hdirective != nil) && (gethandlesize (hdirective) > 0) && (**hdirective == '#')) {

			//	s = string.delete (s, 1, 1); Çpop off the #
			//	local (lastdirective = html.runDirective (s, adrpagetable));
			
			/*
			deletestring (bs, 1, 1); // pop off the #
			
			if (!newtexthandle (bs, &hdirective))
				goto error;
			*/
			
			long ixload = 1; /*don't copy leading pound sign*/
			long ctload = gethandlesize (hdirective) - 1;

			if (!loadfromhandletohandle (hdirective, &ixload, ctload, false, &hdirective))
				goto error;

			if (!htmlrundirective (&pageinfo, hdirective, lastdirective))
				goto error;
			
			opsetoutline (ho);
			
			//	local (newtype = nil); Ç4.2
			//	case lastdirective { Ç4.2
			//		"define" {
			//			newtype = outlinetype};
			//		"definescript" {
			//			newtype = scripttype}};
			//	if newtype != nil {
			
			newtype = (tyexternalid) -1;
			
			if (equalstrings (lastdirective, BIGSTRING ("\x06" "define")))
				newtype = idoutlineprocessor;
			
			else if (equalstrings (lastdirective, BIGSTRING ("\x0c" "definescript")))
				newtype = idscriptprocessor;
			
			if (newtype != (tyexternalid) -1) {
			
				//	local (theList);
				//	local (objectname = adrpagetable^.[lastdirective]);
				//	delete (@adrpagetable^.[lastdirective]);
				
				if (!langlookupstringvalue (pageinfo.hpagetable, lastdirective, objectname))
					goto error;
				
				hashtabledelete (pageinfo.hpagetable, lastdirective);
				
				//	op.expand (1);
				//	if op.go (right, 1) {
				//		theList = op.outlineToList (adroutline)}
				//	else {
				//		scriptError ("Empty sub-outline in \"" + objectname + "\"#define directive.")};
				//	local (adrnewoutline = @adrpagetable^.[objectname]);
				//	new (newtype, adrnewoutline);
				//	op.listToOutline (theList, adrnewoutline);
				//	target.set (adrnewoutline);
				//	op.firstsummit ();
				//	op.deleteline ();
				
				if (!opnavigate (right, &nomad)) {
					
					langparamerror (emptydefinedirective, objectname);
					
					goto error;
					}
				
				hsummit = (**ho).hsummit;
				
				(**ho).hsummit = nomad; // temp, while outline gets copied
				
				fl = langexternalnewvalue (newtype, (Handle) ho, &val);
				
				(**ho).hsummit = hsummit;
				
				if (!fl)
					goto error;
				
				if (!hashtableassign (pageinfo.hpagetable, objectname, val))
					goto error;
				
				//	target.set (adroutline);
				//	op.go (left, 1)};
				
				opnavigate (left, &nomad);
				}
			
			//	op.deleteline ()}
			opdeletenode (nomad);
			}
		else {
			//	if script.isComment () { Çnew in 4.0b7 -- omit top-level comment lines
			//		op.deleteline ()}
			//	else {
			//		if not op.go (down, 1) {
			//			break}}}};
			if ((**nomad).flcomment)
				opdeletenode (nomad);
			}
		
		if (nextnomad == nomad)
			break;
		
		nomad = nextnomad;
		} 
	
	//	try {target.set (oldtarget)}}
	oppopoutline ();
	
	return (true);
	
	error:
		oppopoutline ();
		
		return (false);
	} /*runoutlinedirectivesverb*/


static boolean cleanforexportverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
on cleanForExport (text) { Çprepare text to leave Mac environment
	Ç10/31/97 at 6:45:58 AM by DW -- moved from toys.cleanForExport.
	if sys.os () == "MacOS" {
		on replace (searchfor, replacewith) {
			if string.patternMatch (searchfor, text) != 0 { Çoptimization
				text = string.replaceAll (text, searchfor, replacewith)}};
		
		replace ("Õ", "'");
		replace ("Ô", "'");
		replace ("Ò", Ò"Ó);
		replace ("Ó", Ò"Ó);
		replace ("¥", "o");
		replace ("...", "...");
		replace ("Ê", " "); Ça funny space that Word produces
		replace ("Ð", "--");
		replace ("Ç", "&lt;&lt;");
		replace ("È", "&gt;&gt;")};
	return (text)}
	*/
	
	Handle x;
	
	flnextparamislast = true;
	
	if (!getexempttextvalue (hp1, 1, &x))
		return (false);
	
	if (!htmlcleanforexport (x))
		return (false);
	
	return (setheapvalue (x, stringvaluetype, v));
	} /*cleanforexportverb*/


#if 0

static boolean normalizenameverb (hdltreenode hp1, tyvaluerecord *v) {

	/*
	on normalizeName (name, pageTable=nil, adrObject=nil) {
		Ç2/12/98 at 3:26:44 PM by PBS
			ÇSupport for normalizing folder names.
			ÇURL-encode returned name.
			ÇSupport for normalizing a name that isn't the page being generated.
		Ç4/13/98 PBS
			ÇThere are three contexts in which this operates:
				Ç1) Normalizing a name of a new page that has no prefs.
				Ç2) Normalizing a name based on the prefs of the page being rendered.
				Ç3) Normalizing a name based on the prefs of a remote page.
		
		local (flDropNonAlphas, flLowerCaseFileNames);
		local (maxLength);
		local (extension = "");
		local (flFolder = false);
		
		if pageTable == nil and adrObject == nil { //it's a new page without prefs
			flDropNonAlphas = html.getPref ("dropNonAlphas");
			flLowerCaseFileNames = html.getPref ("lowerCaseFileNames");
			maxLength = number (html.getPref ("maxFileNameLength"));
			extension = html.getPref ("fileExtension")}
		else { //it's an existing page with prefs
			if pageTable == nil {
				pageTable = @websites.["#data"]};
			if adrObject == nil {
				adrObject = pageTable^.adrObject};
			if adrObject == pageTable^.adrObject { //it's the current page
				flDropNonAlphas = html.getPref ("dropNonAlphas", pageTable);
				flLowerCaseFileNames = html.getPref ("lowerCaseFileNames", pageTable);
				maxLength = number (html.getPref ("maxFileNameLength", pageTable));
				extension = html.getPref ("fileExtension", pageTable)}
			else { //it's a remote page
				flDropNonAlphas = html.getPagePref ("dropNonAlphas", adrObject, pageTable);
				flLowerCaseFileNames = html.getPagePref ("lowerCaseFileNames", adrObject, pageTable);
				maxLength = number (html.getPagePref ("maxFileNameLength", adrObject, pageTable));
				extension = html.getPagePref ("fileExtension", adrObject, pageTable)};
			if typeOf (adrObject^) == tableType {
				flFolder = true}};
		
		if flDropNonAlphas {
			name = string.dropNonAlphas (name)};
		if flLowerCaseFileNames {
			name = string.lower (name)};
		if flFolder {
			extension = ""};
		maxLength = maxLength - sizeOf (extension);
		if sizeof (name) > maxLength {
			name = string.mid (name, 1, maxLength)};
		return (name)}
	*/
	
	} /*normalizenameverb*/

#endif


static boolean glossarypatcherverb (hdltreenode hp1, tyvaluerecord *v) {

	/*
	on glossaryPatcher (adrpagedata=@websites.["#data"]) {
		Çscan the fully rendered page for [[#glossPatch xxx|yyy]]
			Çgenerate relative href's for these references
			ÇSun, Nov 3, 1996: if xxx is empty, just generate the URL, not an href
				ÇThis supports the JavaScript popup menu in the DaveNet website
			ÇAuthor: Dave Winer, dwiner@well.com
			ÇMon, Jan 20, 1997 at 3:30:04 PM by PH
				ÇOnly do the work if the pref is set to true
	*/
	
	typrocessmacrosinfo pageinfo;
	boolean fluseglosspatcher;
	tyaddress nomad;
	tyaddress ftpsite;
	tyvaluerecord vrenderedtext;
	handlestream s;
	long ix, ixstart, ixend, ixload, lenpath;
	long ixword = 0;
	long lenword =  0;
	Handle hurl = nil;
	Handle h = nil;
	Handle hpath = nil;
	Handle hlinetext = nil;
	hdlhashnode hnode;
	
	if (!getoptionalpagetablevalue (hp1, 1, &pageinfo.hpagetable))
		return (false);
	
	if (!htmlgetprefstable (&pageinfo.huserprefs))
		return (false);
	
	//	if not html.getPref ("useGlossPatcher", adrpagedata) {
	//		return};
	
	if (!htmlgetbooleanpref (&pageinfo, str_useglosspatcher, &fluseglosspatcher))
		return (false);
	
	if (!fluseglosspatcher)
		return (setbooleanvalue (true, v));
	
	//	with adrpagedata^ {
	//		local (pattern = "[[#glossPatch ");
	//		local (ix, ixstart, ixend);
	//		loop {
	
	if (!hashtablelookup (pageinfo.hpagetable, str_renderedtext, &vrenderedtext, &hnode)) {
		
		langparamerror (unknownidentifiererror, str_renderedtext);
		
		return (false);
		}

	//	local (url = "", nomad = adrobject);
	//	loop {
	//		nomad = parentof (nomad^);
	//		if nomad == parentOf (ftpsite^) {
	//			break};
	//		url = "../" + url};
		
	if (!langlookupaddressvalue (pageinfo.hpagetable, str_adrobject, &nomad))
		return (false);
		
	if (!langlookupaddressvalue (pageinfo.hpagetable, str_ftpsite, &ftpsite))
		return (false);

	/*findinparenttable (ftpsite.ht, &ftpsite.ht, ftpsite.bs);*/

	if (!newemptyhandle (&hurl))
		goto exit;
		
	while (true) {
			
		if (nomad.ht == ftpsite.ht || nomad.ht == nil) /*6.1b10 AR: Added nil check to prevent crashes*/
			break;
			
		if (!findinparenttable (nomad.ht, &nomad.ht, nomad.bs))
			break;

		if (!pushtexthandle (BIGSTRING ("\x03" "../"), hurl))
			goto exit;
		}/*while*/

	openhandlestream (vrenderedtext.data.stringvalue, &s);
	
	while (true) {

		//	ixstart = string.patternMatch (pattern, renderedtext);
		//	if ixstart == 0 {
		//		break};
		
		ixstart = textpatternmatch ((byte *)(*s.data + s.pos), s.eof - s.pos, str_glosspatch, false);
		
		if (ixstart < 0)
			break;
		
		ixstart += s.pos;
		
		//	ix = ixstart + sizeof (pattern);
		//	ixend = string.patternMatch ("]]", string.mid (renderedtext, ix, infinity)) + ix;
		//	ix = ixend + 1;
		//	if ix == 1 {
		//		ix = sizeOf (renderedtext)};
		
		ix = ixstart + stringlength (str_glosspatch);
		
		ixend = textpatternmatch ((byte *)(*s.data + ix), s.eof - ix, BIGSTRING ("\x02" "]]"), false);
		
		if (ixend < 0)
			ixend = s.eof;
		else
			ixend += ix + 2;
				
		//	local (s, linetext, patch);
		//	s = string.mid (renderedtext, ixstart, ixend - ixstart + 1);
		//	s = string.delete (s, 1, sizeof (pattern));

		ixload = ixstart + stringlength (str_glosspatch);

		if (!loadfromhandletohandle (s.data, &ixload, ixend - ixstart - stringlength (str_glosspatch), false, &h))
			goto exit;

		//	Çurl = url + string.nthField (s, '|', 2) + html.getPref ("fileExtension", adrpagedata)
		//	bundle { //add the path to url, it's more complicated because the extension may already be there
		//		local (path = string.nthField (s, '|', 2));
		//		local (extension = string.nthField (path, '.', 2));
		//		if extension == "" { //no extension
		//			path = path + html.getPref ("fileExtension", adrpagedata)};
		//		url = url + path};
		
		if (!nthfieldhandle (h, '|', 2, &hpath))
			goto exit;

		lenpath = gethandlesize (hpath);

		textnthword ((ptrbyte)(*hpath), lenpath, 2, '.', true, &ixword, &lenword);
		
		if ((lenword == 0) && (lenpath > 0) && ((*hpath)[lenpath - 1] != '/')) {

			bigstring extension;
		
			if (!htmlgetstringpref (&pageinfo, str_fileextension, extension))
				goto exit;
			
			if (!pushtexthandle (extension, hpath))
				goto exit;
			}
		
		if (!inserthandleinhandle (hurl, hpath, 0))
			goto exit;
		
		//	linetext = string.nthField (s, '|', 1);
		//	if linetext == "" {
		//		patch = url}
		//	else {
		//		patch = "<a href=\"" + url + "\">" + linetext + "</a>"};
		//	renderedtext = string.delete (renderedtext, ixstart, ixend - ixstart + 1);
		//	renderedtext = string.insert (patch, renderedtext, ixstart)}}}
			
		if (!nthfieldhandle (h, '|', 1, &hlinetext))
			goto exit;

		s.pos = ixstart;

		if (gethandlesize (hlinetext) == 0) {

			boolean fl;

			fl = mergehandlestreamhandle (&s, ixend - ixstart, hpath);
			
			hpath = nil; /*already disposed*/
			
			if (!fl)
				goto exit;
			}

		else {

			Handle hlink;
			
			if (!newtexthandle (str_hotlink, &hlink))
				goto exit;

			if (!parsedialoghandle (hlink, hpath, hlinetext, nil, nil)) {
				disposehandle (hlink);
				goto exit;
				}

			if (!mergehandlestreamhandle (&s, ixend - ixstart, hlink))
				goto exit;
			}

		disposehandle (hpath);

		hpath = nil;

		disposehandle (h);

		h = nil;

		disposehandle (hlinetext);
		
		hlinetext = nil;
		}/*while*/
	
	disposehandle (hurl);

	closehandlestream (&s);

	// we changed string value in place, so notify
	langsymbolchanged (pageinfo.hpagetable, str_renderedtext, hnode, true);
	
	return (setbooleanvalue (true, v));

exit:

	disposehandle (hurl);

	disposehandle (hpath);

	disposehandle (h);

	disposehandle (hlinetext);

	return (false);
	} /*glossarypatcherverb*/


static boolean expandurlsverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.0.2b18 dmb: new verb, a very abridged version of processhtmlmacros
	*/

	typrocessmacrosinfo pageinfo;
	Handle htext = nil;
	handlestream s;
	
	clearbytes (&pageinfo, sizeof (pageinfo));
	
	pageinfo.flactiveurls = true;
	
	if (!getexempttextvalue (hp1, 1, &htext))
		return (false);
	
	openhandlestream (htext, &s);
	
	if (!processhtmltext (&s, &pageinfo))
		return (false);
	
	closehandlestream (&s);
	
	return (setheapvalue (htext, stringvaluetype, v));
	} /*expandurlsverb*/


/*on traversalSkip (adr) { Ç4.2
	Çreturn true if this object should be omitted from a traversal
	local (name = nameof (adr^));
	if name beginsWith '#' { Ç4.0.1
		return (true)};
	case string.lower (name) {
		"glossary";
		"images"; Ç4.2
		"tools" {
			return (true)}};
	return (false)}
*/

static boolean traversalskipverb (hdltreenode hp1, tyvaluerecord *v) {

	tyvaluerecord val;
	hdlhashtable ht;
	bigstring bsname;
	
	flnextparamislast = true;
	
	if (!getaddressparam (hp1, 1, &val))
		return (false);
	
	if (!getaddressvalue (val, &ht, bsname))
		return (false);
	
	if (getstringcharacter (bsname, 0) == '#') {
	
		setbooleanvalue (true, v);

		return (true);
		}

	alllower (bsname);
	
	if (equalstrings (bsname, str_glossary)
			|| equalstrings (bsname, str_images)
			|| equalstrings (bsname, str_tools)) {
	
		setbooleanvalue (true, v);
		
		return (true);
		}
	
	setbooleanvalue (false, v);
		
	return (true);	
	} /*traversalskipverb*/


static boolean getpagetableaddressverb (hdltreenode hp1, tyvaluerecord *v) {

	/*
	6.1d7 AR: Implemented.
	*/

	tyvaluerecord val;
	hdlhashtable ptatable;
	bigstring bsname;
	hdlhashnode hnode;
	
	if (!langcheckparamcount (hp1, 0))
		return (false);

	if (!langfastaddresstotable (systemtable, BIGSTRING ("\x17" "temp.pageTableAddresses"), &ptatable))
		return (false);
	
	numbertostring (getthreadid (getcurrentthread ()), bsname);

	if (!langhashtablelookup (ptatable, bsname, &val, &hnode))
		return (false);
	
	if (!copyvaluerecord (val, v) || !coercetoaddress (v))
		return (false);

	return (true);	
	} /*getpagetableaddressverb*/


#pragma mark === search indexing ===

static boolean stripmarkup (handlestream *s) {

	char *p;
	byte ch;
	byte bsskipthru [8];
	long ix;
	boolean fldidspace = true; // so leading spaces are omitted
	
	p = *(*s).data;
	
	for ((*s).pos = 0; (*s).pos < (*s).eof; ++(*s).pos) {
		
		ch = p [(*s).pos];
		
		setemptystring (bsskipthru);
		
		switch (ch) { // set chreplace or bsreplace
		
			case '{': // scan to matching close
				setstringwithchar ('}', bsskipthru);
				
				break;
			
			case '<': // scan to matching close
				setstringwithchar ('>', bsskipthru);
				
				break;
			
			case '\r':
			case '\n':
			case '\t':
				ch = chspace;
				
				break;
			} /*switch*/
		
		if (!isemptystring (bsskipthru)) {
		
			ix = textpatternmatch ((byte *)(p + (*s).pos), (*s).eof - (*s).pos, bsskipthru, false);
			
			if (ix < 0)
				break;
			
			pullfromhandlestream (s, ix + stringlength (bsskipthru) - 1, nil);
			
			ch = chspace; // replace leading char with space
			
			if (athandlestreameof (s))
				break;
			}
		
		if (ch == chspace) { // the only possible replacement character
			
			if (fldidspace) { // kill this char instead of adding another
			
				pullfromhandlestream (s, 1, nil);
				
				--(*s).pos;
				}
			else {
			
				p [(*s).pos] = ch;
				
				fldidspace = true;
				}
			}
		else
			fldidspace = false;
		}
	
	return (true);
	} /*stripmarkup*/


static boolean parsepageaddress (bigstring bsaddress, bigstring bspage, bigstring bsparent) {
	
	/*
	5.1.4 dmb: bsaddress might be an odb path, a file path, or an url. figure it out.
	*/
	
	hdlhashtable hparent;
	long ctwords;
	tyfilespec fs;
	boolean fl;
	
	disablelangerror ();
	
	fl = langexpandtodotparams (bsaddress, &hparent, bspage);
	
	enablelangerror ();
	
	if (fl) { // an odb address
		
		if (!findinparenttable (hparent, &hparent, bsparent))
			setemptystring (bsparent);
		
		return (true);
		}
	
	if (patternmatch (BIGSTRING ("\x03" "://"), bsaddress)) { // an url
	
		ctwords = countwords (bsaddress, '/');
		
		nthword (bsaddress, ctwords, '/', bspage);
		
		if (ctwords > 1)
			nthword (bsaddress, ctwords - 1, '/', bsparent);
		
		return (true);
		}
	
	disablelangerror ();
	
	fl = pathtofilespec (bsaddress, &fs);
	
	if (fl) { // maybe a valid file
		
		filefrompath (bsaddress, bspage);
		
		folderfrompath (bsaddress, bsparent);
		
		cleanendoffilename (bsparent);
		
		filefrompath (bsparent, bsparent);
		}
	
	enablelangerror ();
	
	return (fl);
	} /*parsepageaddress*/
	

static boolean isstalepageaddress (bigstring bsaddress) {
	
	/*
	5.1.4 dmb: try to determine if the page address is stale. don't do 
	urls since they require a net connection. (ideally, we should handle 
	file:// urls at least)
	*/
	
	hdlhashtable hparent;
	bigstring bspage;
	tyfilespec fs;
	boolean flfolder;
	boolean fl;
	
	disablelangerror ();
	
	fl = langexpandtodotparams (bsaddress, &hparent, bspage);
	
	enablelangerror ();
	
	if (fl) { // an odb address
	
		return (hashtablesymbolexists (hparent, bspage));
		}
	
	if (patternmatch (BIGSTRING ("\x03" "://"), bsaddress)) { // an url
	
		return (false); // not handled now
		}
	
	disablelangerror ();
	
	fl = pathtofilespec (bsaddress, &fs);
	
	enablelangerror ();
	
	if (fl) { // maybe a valid file
	
		return (fileexists (&fs, &flfolder));
		}
	
	return (false);
	} /*isstalepageaddress*/
	

static boolean cleanindextable (hdlhashtable hpages) {
	
	/*
	5.1.4 dmb: hpages is an index table for a specific word, containing 
	some number of pagekey/values. loop through the table and delete 
	any value whose key doesn't appear to refer to a valid item. right now, 
	we only test for files and odb items; we don't want to assume a net connection
	for url validation.
	*/
	
	hdlhashnode hkey, hnext;
	bigstring bsaddress;
	
	for (hkey = (**hpages).hfirstsort; hkey != nil; hkey = hnext) {
		
		hnext = (**hkey).sortedlink;
		
		gethashkey (hkey, bsaddress);
		
		if (isstalepageaddress (bsaddress))
			if (!hashtabledelete (hpages, bsaddress))
				return (false);
		}
	
	return (true);
	} /*cleanindextable*/


static boolean deindexpage (hdlhashtable hindex, bigstring bspagekey) {

	/*
	5.1.4 dmb: Loop through the entire index, removing all references to this page.
	*/
	
	hdlhashtable hwords, hpages;
	hdlhashnode hletter, hword;
	hdlhashnode hnextletter, hnextword;
	bigstring bsletter, bsword;
	
	for (hletter = (**hindex).hfirstsort; hletter != nil; hletter = hnextletter) {
		
		hnextletter = (**hletter).sortedlink;
		
		gethashkey (hletter, bsletter);
		
		if (langexternalvaltotable ((**hletter).val, &hwords, hletter)) {
			
			for (hword = (**hwords).hfirstsort; hword != nil; hword = hnextword) {
				
				hnextword = (**hword).sortedlink;
				
				gethashkey (hword, bsword);
				
				if (langexternalvaltotable ((**hword).val, &hpages, hword)) {
					
					if (bspagekey == nil) {
					
						if (!cleanindextable (hpages))
							return (false);
						}
					else {
					
						if (hashtablesymbolexists (hpages, bspagekey))
							if (!hashtabledelete (hpages, bspagekey))
								return (false);
						}
					
					if ((**hpages).hfirstsort == nil) // the table is empty
						if (!hashtabledelete (hwords, bsword))
							return (false);
					}
				else if (equalstrings (bsword, BIGSTRING ("\x07" "item #1"))) {
					
					if (!hashtabledelete (hwords, bsword))
						return (false);
					}
				}
			
			if ((**hwords).hfirstsort == nil) // the table is empty
				if (!hashtabledelete (hindex, bsletter))
					return (false);
			}
		else if (equalstrings (bsletter, BIGSTRING ("\x07" "item #1"))) {
	
			if (!hashtabledelete (hindex, bsletter))
				return (false);
			}
		}
	
	return (true);
	} /*deindexpage*/


static boolean indexpage (bigstring bsaddress, bigstring bsurl, bigstring bstitle, handlestream *pagetext, 
		hdlhashtable hindex, hdlhashtable hstopwords, bigstring bsparent, bigstring bspage) {
#pragma unused (bsurl)

	/*
	Prepare text to be indexed.
		Replace whitespace characters and punctuation with spaces.
		(Don't replace #, <, >, {, and } characters.)
		Then strip HTML tags and macros.
	*/
	
	handlestream *s = pagetext;
	byte *p;
	hdlhashtable hwords, hpages;
	tyvaluerecord vcount;
	long ctwords, ixword;
	long len, offset;
	bigstring bsword;
	byte bsletter [4];
	hdlhashnode hnode;
	
	if (!deindexpage (hindex, bsaddress))
		return (false);
	
	for ((*s).pos = 0; (*s).pos < (*s).eof; ++(*s).pos) {
		
		p = (byte *)(*(*s).data + (*s).pos);
		
		switch (*p) { // set chreplace or bsreplace
		
			case '\r':
			case '\n':
			case '\t':
			case '.':
			case '-':
			case '\\':
			case '/':
			case ':':
			case '?':
			case '!':
			case ';':
			case '@':
			case '$':
			case '%':
			case '^':
			case '&':
			case '*':
			case ')':
			case '(':
			case '[':
			case ']':
				*p = chspace;
			
			default:
				break;
			}
		}
	
	if (!stripmarkup (s))
		return (false);
	
	p = (unsigned char *)(*(*s).data);
	
	ctwords = textcountwords (p, (*s).eof, chspace, true);
	
	for (ixword = 1; ixword <= ctwords; ++ixword) {
		
		textnthword (p, (*s).eof, ixword, chspace, true, &offset, &len);
		
		texttostring (p + offset, len, bsword);
	
		if (getstringcharacter (bsword, 0) == '#') //it's a directive
			continue;
		
		dropnonalphas (bsword);
		
		alllower (bsword);
		
		poptrailingchars (bsword, 's'); //pop off trailing s's
		
		if (stringlength (bsword) < 3) //we don't index 1- and 2-letter words, except for "op" and "wp"
			if (!equalstrings (bsword, BIGSTRING ("\x02" "op")) && !equalstrings (bsword, BIGSTRING ("\x02" "wp")))
				continue;
		
		if (hashtablesymbolexists (hstopwords, bsword)) //check if this is on the list of words not to index
			continue; //don't index
	
		setstringwithchar (getstringcharacter (bsword, 0), bsletter);
		
		if (!isalpha (getstringcharacter (bsletter, 0)))
			continue;
		
		// find the count for this page
		if (!langsuretablevalue (hindex, bsletter, &hwords)) // each letter table contains word tables
			return (false);
		
		if (!langsuretablevalue (hwords, bsword, &hpages)) // each word table contains addresses
			return (false);
		
		if (!hashtablelookup (hpages, bsaddress, &vcount, &hnode)) // this objects count for this word
			initvalue (&vcount, longvaluetype); //first occurence of this word in the page
		
		if (!coercetolong (&vcount)) //make sure it's a number
			return (false);

		++vcount.data.longvalue; //one more occurrance
		
		bundle { //do relevancy ranking
		
			alllower (bsaddress);
			alllower (bspage);
			alllower (bsparent);
			alllower (bstitle);
			
			if (patternmatch (bsword, bsaddress) > 0)
				if (vcount.data.longvalue < 100)
					vcount.data.longvalue += 100;
			
			if (patternmatch (bsword, bspage) > 0)
				if (vcount.data.longvalue < 500)
					vcount.data.longvalue += 500;
			
			if (equalstrings (bsword, bsparent)) {
				
				firstword (bspage, '.', bspage);
				
				if (equalstrings (bspage, str_default) || equalstrings (bspage, str_index))
					if (vcount.data.longvalue < 1000)
						vcount.data.longvalue += 1000;
				}
			
			if (patternmatch (bsword, bstitle) > 0)
				if (vcount.data.longvalue < 2000)
					vcount.data.longvalue += 2000;
			}
		
		if (!hashtableassign (hpages, bsaddress, vcount))
			return (false);
		}
	
	return (true);
	} /*indexpage*/


static boolean stripmarkupverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.1.4 dmb
	*/
	
	Handle x;
	handlestream s;
	
	flnextparamislast = true;
	
	if (!getexempttextvalue (hp1, 1, &x))
		return (false);
	
	openhandlestream (x, &s);
	
	if (!stripmarkup (&s))
		return (false);
	
	closehandlestream (&s);
	
	return (setheapvalue (x, stringvaluetype, v));
	} /*stripmarkupverb*/


static boolean deindexpageverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.1.4 dmb
	*/
	
	hdlhashtable hindex;
	bigstring bsaddress;
	
	if (!getstringvalue (hp1, 1, bsaddress))
		return (false);
	
	flnextparamislast = true;
	
	if (!gettablevalue (hp1, 2, &hindex))
		return (false);
	
	if (!deindexpage (hindex, bsaddress))
		return (false);
	
	return (setbooleanvalue (true, v));
	} /*deindexpageverb*/


static boolean indexpageverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.1.4 dmb: kernelization of Brent's indexing code
	
	ÇadrPage is the address of the page.
	Çurl is the url of the page on the web.
	Çtitle is the title of the page.
	ÇpageText is the unprocessed text of the page.
	ÇadrIndex is the address of the inverted index.
	ÇadrStopWordsTable is a table of stop words.
		ÇThe name of each entry is a stop word -- a word *not* to index.
		ÇThe values are ignored, they can be nil.
	ÇflMessages -- true to report progress in the About Window.
	*/
	
	hdlhashtable hindex, hstopwords;
	Handle hpagetext;
	bigstring bsurl, bstitle, bsaddress, bspage, bsparent;
	handlestream s;
	
	if (!getstringvalue (hp1, 1, bsaddress))
		return (false);
	
	if (!getstringvalue (hp1, 2, bsurl))
		return (false);
	
	if (!getstringvalue (hp1, 3, bstitle))
		return (false);
	
	if (!getexempttextvalue (hp1, 4, &hpagetext))
		return (false);
	
	if (!gettablevalue (hp1, 5, &hindex))
		goto error;
	
	flnextparamislast = true;
	
	if (!gettablevalue (hp1, 6, &hstopwords))
		goto error;
	
	parsepageaddress (bsaddress, bspage, bsparent);
	
	openhandlestream (hpagetext, &s);
	
	if (!indexpage (bsaddress, bsurl, bstitle, &s, hindex, hstopwords, bsparent, bspage)) // consumes hpagetext
		goto error;
	
	disposehandle (hpagetext);
	
	return (setbooleanvalue (true, v));
	
	error:
		disposehandle (hpagetext);
		
		return (false);
	} /*indexpageverb*/


static boolean cleanindexverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.1.4 dmb: clean out stale items from the index
	*/
	
	hdlhashtable hindex;
	
	flnextparamislast = true;
	
	if (!gettablevalue (hp1, 1, &hindex))
		return (false);
	
	if (!deindexpage (hindex, nil))
		return (false);
	
	return (setbooleanvalue (true, v));
	} /*cleanindexverb*/


static boolean unionmatchesverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.1.4 dmb: merge the results of the list of tables into an output table
	
	the first parameter is a list of addresses of tables.
	
	the second parameter is a table, where the results are stored.
	*/
	
	tyvaluerecord vtablelist, adrtable, vtable;
	hdlhashtable htable, hunion;
	hdlhashnode hmatch, hitem;
	bigstring bsname;
	long ix, ctitems;
	tyvaluerecord vmatch, vunion;
	hdlhashnode hnode;
	
	if (!getparamvalue (hp1, 1, &vtablelist) || !coercetolist (&vtablelist, listvaluetype)) 
		return (false);
	
	flnextparamislast = true;
	
	if (!gettablevalue (hp1, 2, &hunion))
		return (false);
	
	if (!langgetlistsize (&vtablelist, &ctitems))
		return (false);
	
	for (ix = 1; ix <= ctitems; ++ix) {
		
		if (!langgetlistitem (&vtablelist, ix, nil, &adrtable))
			return (false);
		
		if (!coercetoaddress (&adrtable))
			return (false);
		
		if (!getaddressvalue (adrtable, &htable, bsname))
			return (false);
		
		if (!hashtablelookup (htable, bsname, &vtable, &hnode))
			return (false);
		
		if (langexternalvaltotable (vtable, &htable, hnode)) {
			
			for (hmatch = (**htable).hfirstsort; hmatch != nil; hmatch = (**hmatch).sortedlink) {
			
				gethashkey (hmatch, bsname);
				
				if (!copyvaluerecord ((**hmatch).val, &vmatch) || !coercetolong (&vmatch))
					return (false);
				
				if (hashtablelookupnode (hunion, bsname, &hitem)) {
					
					vmatch.data.longvalue += 4000;
					
					if (!addvalue ((**hitem).val, vmatch, &vunion))
						return (false);
					
					exemptfromtmpstack (&vunion);
					
					(**hitem).val = vunion;
					
					langsymbolchanged (hunion, bsname, hitem, true); /*value changed*/
					}
				else {
				
					if (!hashtableassign (hunion, bsname, vmatch))
						return (false);
					}
				}
			}
		}
	
	return (setbooleanvalue (true, v));
	} /*unionmatchesverb*/


#pragma mark === webserver utility functions ===


static boolean followaddressvalue (tyvaluerecord *v) {

	/*
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/
	hdlhashnode hnode;

	if ((*v).valuetype == addressvaluetype) {
		hdlhashtable htable;
		bigstring bstemp;

		if (!getaddressvalue ((*v), &htable, bstemp))
			return (false);

		if (!langhashtablelookup (htable, bstemp, v, &hnode))
			return (false);
		}
		
	return (true);
	} /*followaddressvalue*/


static boolean followaddress (tyaddress *adr) {

	/*
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!langhashtablelookup ((*adr).ht, (*adr).bs, &val, &hnode))
		return (false);
	
	if (val.valuetype == addressvaluetype
			&& !getaddressvalue (val, &(*adr).ht, (*adr).bs))
		return (false);
		
	return (true);
	} /*followaddress*/


static boolean langlookupvaluefollow (hdlhashtable ht, bigstring bs, tyvaluerecord *v, hdlhashnode * hnode) {

	if (!langhashtablelookup (ht, bs, v, hnode))
		return (false);
	
	if ((*v).valuetype == addressvaluetype) {
		bigstring bstemp;
		hdlhashtable httemp;
		
		if (!getaddressvalue (*v, &httemp, bstemp))
			return (false);
			
		if (!langhashtablelookup (httemp, bstemp, v, hnode))
			return (false);
		}

	return (true);
	} /*langlookupvaluefollow*/


static boolean langcallscriptwithaddress (tyaddress *adrscript, tyaddress *adrparamtable, hdlhashtable hcontext, tyvaluerecord *vreturned) {
	
	/*
	6.1d2 AR: New script based on langrunscript.
	Takes an address instead of a script name.
	Also assumes a single address as a parameter instead of a parameter list.
	*/
	
	boolean flchained = false, fltmpval;
	boolean fl = false;
	tyvaluerecord val;
	hdltreenode hfunctioncall;
	hdltreenode hparamlist;
	hdltreenode hcode;
	hdltreenode hscriptcode;
	tyvaluerecord vhandler;
	hdlhashnode handlernode;
		
	if (!hashtablelookupnode ((*adrscript).ht, (*adrscript).bs, &handlernode)) {
		
		langparamerror (unknownfunctionerror, (*adrscript).bs);
		
		return (false);
		}
	
	vhandler = (**handlernode).val;
	
	/* make sure the script we're going to call is compiled */
	
	if ((**(*adrscript).ht).valueroutine == nil) { /*not a kernel table*/
		
		if (!langexternalvaltocode (vhandler, &hscriptcode)) {

			langparamerror (notfunctionerror, (*adrscript).bs);

			return (false);
			}
		
		if (hscriptcode == nil) { /*needs compilation*/
			
			if (!langcompilescript (handlernode, &hscriptcode))
				return (false);
			}
		}

	/*build a code tree and call the handler, with our error hook in place*/
	
	if (!setaddressvalue ((*adrscript).ht, (*adrscript).bs, &val))
		return (false);
	
	if (!pushfunctionreference (val, &hfunctioncall))
		return (false);
	
	if (!setaddressvalue ((*adrparamtable).ht, (*adrparamtable).bs, &val)
			|| !exemptfromtmpstack (&val)
			|| !newconstnode (val, &hparamlist)) {
	
		langdisposetree (hfunctioncall);
		
		return (false);
		}

	if (!pushfunctioncall (hfunctioncall, hparamlist, &hcode)) /*consumes input parameters*/
		return (false);
	
	if (hcontext != nil) {
		
		flchained = (**hcontext).flchained;
		
		if (flchained)
			pushhashtable (hcontext);
		else
			chainhashtable (hcontext); /*establishes outer local context*/
		}
	
	fl = evaluatelist (hcode, vreturned);
	
	fltmpval = exemptfromtmpstack (vreturned); /*must survive disposing of local scope chain*/
	
	if (hcontext != nil) {
		
		if (flchained)
			pophashtable ();
		else
			unchainhashtable ();
		}
	
	if (fltmpval) /*insert into the next-most-global tmpstack, if one exists*/
		pushvalueontmpstack (vreturned);

	langdisposetree (hcode);
		
	return (fl);
	} /*langcallscriptwithaddress*/



static boolean langruntextwithcontext (Handle htext, hdlhashtable hcontext, tyvaluerecord *v) {
				
	boolean fl = false;
	boolean fltmpval, flchained = false;
		
	if (hcontext != nil) {
		
		flchained = (**hcontext).flchained;
		
		if (flchained)
			pushhashtable (hcontext);
		else
			chainhashtable (hcontext); /*establishes outer local context*/
		}
	
	fl = langrun (htext, v);

	fltmpval = exemptfromtmpstack (v); /*must survive disposing of local scope chain*/
	
	if (hcontext != nil) {
		
		if (flchained)
			pophashtable ();
		else
			unchainhashtable ();
		}
	
	if (fltmpval) /*insert into the next-most-global tmpstack, if one exists*/
		pushvalueontmpstack (v);
		
	return (fl);
	} /*langruntextwithcontext*/



#pragma mark === webserver.util verbs ===



static boolean addHeaderToTable (Handle htext, long ix1, long len1, long ix2, long len2, hdlhashtable htable) {

	/*
	6.1d1 AR: Utility function for webserverparseheaders.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	bigstring bs;
	Handle h = nil;
	tyvaluerecord vexists;
	hdlhashnode hnode;

	texttostring (&((*htext) [ix1]), len1, bs);

	if (!loadfromhandletohandle (htext, &ix2, len2, false, &h))
		return (false);

	if (hashtablelookup (htable, bs, &vexists, &hnode)) {  /*a symbol named bs already exists in htable*/

		if (vexists.valuetype == listvaluetype) { /*the existing value is a list: append*/

			if (!langpushlisttext (vexists.data.listvalue, h)) /*consumes h*/
				return (false);

			langsymbolchanged (htable, bs, hnode, true);
			}	

		else {  /*the existing value is not a list: create a list from it and append*/

			hdllistrecord hlist;
			tyvaluerecord vlist;

			/*create empty list*/

			if (!opnewlist (&hlist, false)) {
				
				disposehandle (h);
				
				return (false);
				}

			/*push vexists and vnew on list and add list to hashtable*/

			if (!langpushlistval (hlist, nil, &vexists)) {
				
				disposehandle (h);
				
				opdisposelist (hlist);
				
				return (false);
				}
			
			if (!langpushlisttext (hlist, h)) {

				opdisposelist (hlist);
				
				return (false);
				}

			initvalue (&vlist, listvaluetype);
			
			vlist.data.listvalue = hlist;
			
			if (!hashtableassign (htable, bs, vlist)) {
					
				opdisposelist (hlist);
				
				return (false);
				}
			}
		}
	else {  /*a symbol named bs doesn't exist in htable yet*/

		if (!langassigntextvalue (htable, bs, h)) {

			disposehandle (h);

			return (false);
			}
		}

	return (true);
	}/*addHeaderToTable*/


static boolean webserverparseheaders (Handle htext, hdlhashtable hheadertable, Handle *hptr) {

	/*
	6.1d1 AR: Parse the headers of an HTTP request or response into the given table.
	We return the first line of the request/response.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	register long pos;
	long lenfirstline;
	long beginlabel, lenlabel, beginvalue, lenvalue;
	long hsize = gethandlesize (htext);
	Handle h;

	/*locate end of first line*/

	lenfirstline = textpatternmatch ((byte *)(*htext), hsize, STR_P_CRLF, false);

	if (lenfirstline == -1 || hheadertable == nil)
		goto done; /*there's no line terminator, we are done*/

	/*loop thru 2nd to nth line of headers to build the table*/

	pos = beginlabel = lenfirstline + stringlength (STR_P_CRLF);

	lenlabel = beginvalue = lenvalue = -1;

	while (pos < hsize) {

		switch ((*htext) [pos]) {

			case ':':

				if (lenlabel == -1) { /*only pick up position of first colon on a line*/

					long i = pos + 1;

					lenlabel = pos - beginlabel;

					while ((*htext) [i] == ' ' || (*htext) [i] == '\t')
						i++; /*skip over any whitespace following the colon*/

					pos = beginvalue = i;

					continue; /*bypass additional incrementation of pos*/
					}

				break;

			case '\r':

				if (beginvalue == -1 || lenlabel <= 0) { /*there was no colon on this line*/

					lenlabel = pos - beginlabel; /*the label is the full text of the line*/

					beginvalue = pos; /*the value will be empty*/
					}

				if (lenlabel == 0)
					goto done;

				lenvalue = pos - beginvalue;

				if (!addHeaderToTable (htext, beginlabel, lenlabel, beginvalue, lenvalue, hheadertable))
					return (false);

				pos++;

				if ((*htext) [pos] == '\n') /*skip over '\n' following '\r'*/
					pos++;

				if ((*htext) [pos] == '\r')
					goto done; /*end of headers, leave while-loop*/

				beginlabel = pos;

				lenlabel = beginvalue = lenvalue = -1;

				continue; /*bypass additional incrementation of pos*/			
			}

		pos++;
		}/*while*/

done:	/*set up return string: first line of headers*/

	if (lenfirstline <= 0) {

		if (!newemptyhandle (&h))
			return (false);
		}
	else {

		beginlabel = 0;

		if (!loadfromhandletohandle (htext, &beginlabel, lenfirstline, false, &h))
			return (false);
		}
			
	*hptr = h;
	
	return (true);
	}/*webserverparseheaders*/


static boolean webserverparsecookies (hdlhashtable hparamtable, tyvaluerecord *vreturn) {

	/*
	6.1d2 AR: Utility string for parsing the request's Cookie header.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	hdlhashtable hheaderstable, hcookiestable;
	tyvaluerecord val;
	Handle hcookie, hfirstpart;
	long ix, len, lencookie;
	bigstring bsname;
	boolean fl;
	hdlhashnode hnode;
	
	disablelangerror ();
	
	fl = langhashtablelookup (hparamtable, STR_P_REQUESTHEADERS, &val, &hnode);
	
	fl = fl && langexternalvaltotable (val, &hheaderstable, hnode);
	
	fl = fl && langhashtablelookup (hheaderstable, STR_P_COOKIE, &val, &hnode);
	
	fl = fl && copyvaluerecord (val, &val);
	
	fl = fl && coercetostring (&val);
	
	enablelangerror ();
	
	if (!fl) {

		setbooleanvalue (false, vreturn);	

		return (true);
		}

	hcookie = val.data.stringvalue; /*val is on the tmpstack*/

	if (!langassignnewtablevalue (hheaderstable, STR_P_COOKIES, &hcookiestable))
		return (false);

	lencookie = gethandlesize (hcookie);

	while (lencookie > 0) {
	
		tyvaluerecord vcookie;

		if (!textnthword ((ptrbyte)(*hcookie), lencookie, 1, ';', /*flstrict:*/ true, &ix, &len))
			break;

		if (!loadfromhandletohandle (hcookie, &ix, len, false, &hfirstpart))
			return (false);

		setheapvalue (hfirstpart, stringvaluetype, &vcookie);

		if (len < lencookie) {

			if (!pullfromhandle (hcookie, 0, len + 1, nil))
				return (false);
			
			handlepopleadingchars (hcookie, ' ');
			}
		else
			sethandlesize (hcookie, 0L);

		if (!textnthword ((ptrbyte)(*hfirstpart), gethandlesize (hfirstpart), 1, '=', /*flstrict:*/ true, &ix, &len))
			break;
		
		if (len > 0) {

			texttostring (*hfirstpart, len, bsname);

			if (len >= gethandlesize (hfirstpart))
				sethandlesize (hfirstpart, 0L);
			else
				if (!pullfromhandle (hfirstpart, 0, len + 1, nil))
					return (false);

			if (!hashtableassign (hcookiestable, bsname, vcookie))
				return (false);

			exemptfromtmpstack (&vcookie);
			}

		lencookie = gethandlesize (hcookie);
		}

	setbooleanvalue (true, vreturn);

	return (true);	
	}/*webserverparsecookies*/


static boolean webservergetpref (bigstring bsprefname, tyvaluerecord *vreturn) {
	
	/*
	6.1d2 AR: A utility function for getting a pref from user.webserver.prefs.
	If no value is found in that table, we return false in vreturn.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	
	2007-06-02 aradke: Don't set *vreturn to false if the requested pref doesn't exist.
	return false instead. This makes it possible for the caller to differentiate
	between a non-existant pref and one that is actually set to false.
	*/
	
	hdlhashtable hprefstable;
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;
	
	disablelangerror ();
	
	fl = langfastaddresstotable (roottable, STR_P_USERWEBSERVERPREFS, &hprefstable)
	     && langhashtablelookup (hprefstable, bsprefname, &val, &hnode);
	
	if (fl)
		fl = copyvaluerecord (val, vreturn);
	
	if (fl)	
		if ((*vreturn).valuetype == externalvaluetype)
			if (langexternalgettype (*vreturn) == idwordprocessor)
				fl = coercetostring (vreturn);
		
	enablelangerror ();
	
	return (fl);
	} /*webservergetpref*/


static boolean webservergetserverstring ( tyvaluerecord *vreturn ) {

	//
	// 2007-06-02 creedon: call webservergetpref to grab value at
	//				   user.webserver.prefs.headerFieldServer if defined
	//
	// 6.1d2 AR: Return a string identifying the server software, i.e.
	//		   Frontier/6.1d2-WinNT
	//
	// 6.1d4 AR: Reviewed for proper error handling and reporting.
	//
	
	Handle h = nil;
	tyvaluerecord vversion, vos;

	if ( webservergetpref ( STR_P_USERWEBSERVERSTRING, vreturn ) )
		return ( true );
	
	if ( ! newtexthandle ( STR_P_SERVERSTRING, &h ) )
		return ( false );

	if ( ! frontierversion ( &vversion ) )
		goto exit;

	if ( ! sysos ( &vos ) )
		goto exit;

	if ( ! parsedialoghandle ( h, vversion.data.stringvalue, vos.data.stringvalue, nil, nil ) )
		goto exit;
		
	return ( setheapvalue ( h, stringvaluetype, vreturn ) );

	exit:

		disposehandle ( h );

		return ( false );
		
	} // webservergetserverstring


static boolean webserverbuilderrorpage (Handle hshort, Handle hlong, Handle *hpage) {

	/*
	6.1d2 AR: Build an error page.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	Handle h;

	if (!newtexthandle (STR_P_ERRORPAGETEMPLATE, &h))
		return (false);

	if (!parsedialoghandle (h, hshort, hlong, nil, nil)) {

		disposehandle (h);

		return (false);
		}

	*hpage = h;

	return (true);	

	}/*webserverbuilderrorpage*/


static boolean webserverbuilderrorpagefromstrings (bigstring bsshort, bigstring bslong, Handle *hpage) {

	/*
	6.1d2 AR: Build an error page.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	hdlhashtable hcodestable;
	Handle hshort = nil;
	Handle hlong = nil;
	boolean returnval = false;
	boolean fl;
	tyvaluerecord val;
	hdlhashnode hnode;

	if (!newtexthandle (bsshort, &hshort))
		goto exit;

	if (!newtexthandle (bslong, &hlong))
		goto exit;
	
	disablelangerror ();
	
	fl = langfastaddresstotable (builtinstable, STR_P_WEBSERVERDATARESPONSES, &hcodestable);
	
	fl = fl && langhashtablelookup (hcodestable, bsshort, &val, &hnode);
	
	if (fl && val.valuetype != stringvaluetype) {
	
		fl = fl && copyvaluerecord (val, &val);
		
		fl = fl && coercevalue (&val, stringvaluetype);
		}
	
	enablelangerror ();
	
	if (fl) {
		
		if (!pushtexthandle (STR_P_SPACE, hshort))
			goto exit;
			
		if (!pushhandle (val.data.stringvalue, hshort))
			goto exit;
		}

	if (!webserverbuilderrorpage (hshort, hlong, hpage))
		goto exit;

	returnval = true;

exit:

	disposehandle (hshort);

	disposehandle (hlong);

	return (returnval);	

	}/*webserverbuilderrorpage*/


static boolean writetableitemtostreamvisit (tyvaluerecord val, ptrhandlestream sptr, bigstring bs) {

	/*
	6.1d2 AR: Utility function for webserverbuildresponse.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	if (val.valuetype == listvaluetype) { /*recurse*/
		long ix = 1;
		tyvaluerecord vitem;

		while (getnthlistval (val.data.listvalue, ix++, nil, &vitem))
			if (!writetableitemtostreamvisit (vitem, sptr, bs))
				return (false);
		}
	
	else {

		if (val.valuetype != stringvaluetype)
			if (!copyvaluerecord (val, &val) || !coercetostring (&val))
				return (false);

		if (!writehandlestreamstring (sptr, bs))
			return (false);

		if (!writehandlestreamstring (sptr, STR_P_COLON))
			return (false);
		
		if (!writehandlestreamhandle (sptr, val.data.stringvalue))
			return (false);
		
		if (!writehandlestreamstring (sptr, STR_P_CRLF))
			return (false);
		}

	return (true);
	} /*writetableitemtostreamvisit*/
	
	
/*	system.verbs.builtins.webserver.util.buildResponse */

static boolean webserverbuildresponse (bigstring bscode, hdlhashtable hheaderstable, Handle hbody, tyvaluerecord *vreturn) {

	/*
	6.1d2 AR: Build the HTTP response headers and optionally body.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	handlestream s;
	hdlhashtable hcodestable;
	tyvaluerecord val;
	long bodysize = gethandlesize (hbody);
	boolean fldisposetable = false;
	boolean fl;
	long ix;
	hdlhashnode hn;
	hdlhashnode hnode;

	openhandlestream (nil, &s);

	if (!writehandlestreamstring (&s, STR_P_HTTP11))
		goto exit;

	if (!writehandlestreamstring (&s, bscode))
		goto exit;
	
	if (!writehandlestreamchar (&s, chspace))
		goto exit;
	
	disablelangerror ();
	
	fl = langfastaddresstotable (builtinstable, STR_P_WEBSERVERDATARESPONSES, &hcodestable);
	
	fl = fl && langhashtablelookup (hcodestable, bscode, &val, &hnode);
	
	fl = fl && copyvaluerecord (val, &val);
	
	fl = fl && coercevalue (&val, stringvaluetype);
	
	enablelangerror ();
	
	if (fl) {		
		if (!writehandlestreamhandle (&s, val.data.stringvalue))
			goto exit;
		}
	else
		if (!writehandlestreamstring (&s, STR_P_UNKNOWN))
			goto exit;
	
	if (!writehandlestreamstring (&s, STR_P_CRLF))
		goto exit;
	
	/* create headers table if it doesn't exist yet */

	if (hheaderstable == nil) {
	
		if (!tablenewtablevalue (&hheaderstable, &val))
			goto exit;
		
		fldisposetable = true;
		}
	
	/* add Connection: close to header table */

	if (!langassignstringvalue (hheaderstable, STR_P_CONNECTION, STR_P_CLOSE))
		goto exit;
	
	/* add Date: Sat, 29 Nov 1997 00:51:47 GMT to header table */
	
	if (!datenetstandardstring (timenow (), &val))
		goto exit;
	
	if (!hashtableassign (hheaderstable, STR_P_DATE, val))
		goto exit;

	exemptfromtmpstack (&val);

	/* add Server: Frontier/6.1d1-NT to header table */
	
	if (!webservergetserverstring (&val))
		goto exit;
	
	if (!hashtableassign (hheaderstable, STR_P_SERVER, val))
		goto exit;

	exemptfromtmpstack (&val);

	/* add Content-Length header if there's a response body */
	
	if (bodysize > 0)
		if (!langassignlongvalue (hheaderstable, STR_P_CONTENT_LENGTH, bodysize))
			goto exit;
	
	/* loop thru the headers table and append header lines */
	
	ix = 0;
	
	/***NEED TO OPTIMIZE*** RAB 1/3/00 */
	/* This code using while (hashgetnthnode) is actually going from 0 to n, But because it is using 
	   hashgetnthnode, it is actually looping through the link list n! times.  The first is just 0,
	   then 0, 1, then 0,1,2 then 0,1,2,3, etc all the way to 0,1,2,...,n for walking the linked list.
	   This whould just use the nomad pointer and walk the list itself one time only n not n! !! */
	/* AR: It's actually looping (n+1)(n+2)/2 times which is a lot less bad than n! would have been. */
	while (hashgetnthnode (hheaderstable, ix++, &hn))
		if (!writetableitemtostreamvisit ((**hn).val, &s, (**hn).hashkey))
			goto exit;

	/* terminate the response headers with additional CRLF sequence*/
	
	if (!writehandlestreamstring (&s, STR_P_CRLF))
		goto exit;

	/* append response body if specified */
	
	if (bodysize > 0)
		if (!writehandlestreamhandle (&s, hbody))
			goto exit;

	/* done, return response */
	
	if (fldisposetable)
		tabledisposetable (hheaderstable, false);

	return (setheapvalue (closehandlestream (&s), stringvaluetype, vreturn));

exit: /* there was an error! */

	disposehandlestream (&s);

	if (fldisposetable)
		tabledisposetable (hheaderstable, false);
	
	return (false);
	} /*webserverbuildresponse*/


static boolean webserveraddtoerrorlog (tyaddress *adrmethod, bigstring bstype, bigstring bserror);


static boolean webservercallfilters (tyaddress *pta, bigstring bstable, bigstring bserrortype) {

	/*
	6.1d2 AR: A utility function for calling all scripts in a given Frontier.root table.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.

	6.1d7 AR: Modified to walk the table in sorted order.
	*/

	tyvaluerecord val;
	tyaddress adrscript;
	hdlhashtable ht;
	long i = 0;
	hdlhashnode x;
	bigstring bserror;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	boolean fl;

	
	if (!langfastaddresstotable (roottable, bstable, &ht))
		return (false);
	
	while (hashgetnthnode (ht, i++, &x)) {

		setemptystring (bserror);
	
		langtraperrors (bserror, &savecallback, &saverefcon);

		gethashkey (x, adrscript.bs);

		adrscript.ht = ht;

		fl = followaddress (&adrscript); /*6.1b5 AR: don't ignore return value of this call*/
			
		fl = fl && langcallscriptwithaddress (&adrscript, pta, nil, &val);

		languntraperrors (savecallback, saverefcon, !fl);

		fllangerror = false;

		if (!ingoodthread ())
			return (false);

		if (!fl && !webserveraddtoerrorlog (&adrscript, bserrortype, bserror))
			return (false);
		} /*while*/
	
	return (true);
	} /*webservercallfilters*/



#pragma mark === kernelized webserver ===


/*	For reference: HTTP/1.1 status codes

	Informational 1xx
		100 Continue
		101 Switching Protocols
		
	Successful 2xx
		200 OK
		201 Created
		202 Accepted
		203 Non-Authoritative Information
		204 No Content
		205 Reset Content
		206 Partial Content
		
	Redirection 3xx
		300 Multiple Choices
		301 Moved Permanently
		302 Moved Temporarily
		303 See Other
		304 Not Modified
		305 Use Proxy
		
	Client Error 4xx
		400 Bad Request
		401 Unauthorized
		402 Payment Required
		403 Forbidden
		404 Not Found
		405 Method Not Allowed
		406 Not Acceptable
		407 Proxy Authentication Required
		408 Request Timeout
		409 Conflict
		410 Gone
		411 Length Required
		412 Precondition Failed
		413 Request Entity Too Large
		414 Request-URI Too Long
		415 Unsupported Media Type
		
	Server Error 5xx
		500 Internal Server Error
		501 Not Implemented
		502 Bad Gateway
		503 Service Unavailable
		504 Gateway Timeout
		505 HTTP Version Not Supported
*/


static boolean webservergetrespondertableaddress (bigstring bsname, tyaddress *adr) {
	
	/*
	6.1d2 AR: Determine the address of the responder table based on the given name.

	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	hdlhashtable hrespondertable;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!langfastaddresstotable (roottable, STR_P_USERWEBSERVERRESPONDERS, &hrespondertable))
		return (false);
	
	if (!langhashtablelookup (hrespondertable, bsname, &val, &hnode))
		return (false);
	
	if (val.valuetype == addressvaluetype) { //follow the address
		
		if (!getaddressvalue (val, &(*adr).ht, (*adr).bs))
			return (false);
		}
	else {
		(*adr).ht = hrespondertable;
		
		copystring (bsname, (*adr).bs);
		}
	
	return (true);
	} /*webservergetrespondertableaddress*/


static boolean webserverlocaterespondercontextbuilder (hdlhashtable hpt, hdlhashtable *hnew) {

	/*
	6.1d2 AR: Build context for responder search.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/
		
	hdlhashtable ht, hheadertable;
	
	if (!langfastaddresstotable (hpt, STR_P_REQUESTHEADERS, &hheadertable))
		return (false);
	
	if (!newhashtable (&ht))
		return (false);
	
	(**ht).fllocaltable = true;

	(**ht).lexicalrefcon = 0L;
	
	//	with adrParamTable^, adrParamTable^.requestHeaders

	if (!langpushwithtable (ht, hpt))
		goto error;
	
	if (!langpushwithtable (ht, hheadertable))
		goto error;

	*hnew = ht;

	return (true);

error:
	disposehashtable (ht, false);
	
	return (false);	
	} /*webserverlocaterespondercontextbuilder*/


static boolean webserverlocateresponder (hdlhashtable hparamtable, bigstring bs, tyaddress *adrrespondertable) {

	/*
	6.1d2 AR: Determine which responder should handle this request.
	Return true if we found it.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.

	7.0.1 PBS: Fixed a bug when a responder is the address of a responder table. If, for instance, there is an address
	at user.webserver.responders.foo, and it points to a table named fooResponder, the responder would not get
	called because "foo" != "fooResponder".
	*/
	
	hdlhashtable ht, hresponderstable, hcontext;
	hdlhashnode hnode;
	register short i = 0;
	boolean flfound = false;
	boolean flenabled;
	bigstring bstemp, bskey;
	tyvaluerecord v, val, vcondition;
	tyaddress adrscript, adrparamtable;
	hdlhashnode hnode2;
	
	if (!findinparenttable (hparamtable, &adrparamtable.ht, adrparamtable.bs))
		return (false);
	
	if (!langfastaddresstotable (roottable, STR_P_USERWEBSERVERRESPONDERS, &hresponderstable))
		return (false);

	if (!webserverlocaterespondercontextbuilder (hparamtable, &hcontext))
		return (false);
	
	disablelangerror ();
	
	while (hashgetnthnode (hresponderstable, i++, &hnode)) {

		val = (**hnode).val;

		gethashkey (hnode, bskey); /*7.0.1 PBS: save the key before we resolve addresses.*/
						
		if (val.valuetype == addressvaluetype) {

			if (!getaddressvalue (val, &ht, bstemp))
				continue;

			if (!langsymbolreference (ht, bstemp, &val, &hnode))
				continue;
			}
			
		if (!langexternalvaltotable (val, &ht, hnode))
			continue;
			
		if (!langlookupbooleanvalue (ht, STR_P_ENABLED, &flenabled) || !flenabled)
			continue;
			
		if (!hashtablelookup (ht, STR_P_CONDITION, &vcondition, &hnode2))
			continue;
			
		if ((vcondition.valuetype == codevaluetype)
				|| (vcondition.valuetype == externalvaluetype && langexternalgettype (vcondition) == idscriptprocessor)) {
				
			copystring (STR_P_CONDITION, adrscript.bs);
				
			adrscript.ht = ht;
			
			if (!langcallscriptwithaddress (&adrscript, &adrparamtable, hcontext, &v)) {

				if (!ingoodthread ()) {/*unwind quickly*/
					enablelangerror ();
					goto exit;
					}

				continue;
				}
			}
		else {
			Handle htext;
				
			if (!copyvaluerecord (vcondition, &val)
					|| !coercetostring (&val)
					|| !copyhandle (val.data.stringvalue, &htext))
				continue;

			if (!langruntextwithcontext (htext, hcontext, &v)) { /* consumes htext */

				if (!ingoodthread ()) { /*unwind quickly*/
					enablelangerror ();
					goto exit;
					}

				continue;
				}
			}
				
		if (coercetoboolean (&v) && v.data.flvalue) { /* found it */
				
			//gethashkey (hnode, bs); /*7.0.1 PBS: commented out. We may have resolved one or more addresses.*/

			copystring (bskey, bs); /*7.0.1 PBS: use the key we got before resolving addresses.*/
				
			flfound = true;
				
			break;
			}	
		} /*while*/
	
	enablelangerror ();

	/* fall back to default responder if neccessary */

	if (!flfound) {
		hdlhashtable hprefstable;
		
		if (langfastaddresstotable (roottable, STR_P_USERWEBSERVERPREFS, &hprefstable)
				&& langlookupstringvalue (hprefstable, STR_P_DEFAULTRESPONDER, bs))
			flfound = true;
		}
	
	if (flfound)
		flfound = webservergetrespondertableaddress (bs, adrrespondertable);

exit:

	disposehashtable (hcontext, false);

	return (flfound);	
	} /*webserverlocateresponder*/


static boolean visitmethods (hdlhashnode hnode , Handle *h) {
	
	/*
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	bigstring bs;
	
	if (gethandlesize (*h) > 0)
		if (!pushtexthandle (BIGSTRING ("\x02" ", "), *h))
			return (false);
	
	gethashkey (hnode, bs);
	
	if (!pushtexthandle (bs, *h))
		return (false);
	
	return (true);
	} /*visitmethods*/


static boolean webservermethodnotallowed (hdlhashtable hmethodstable, bigstring bsmethod, tyvaluerecord *vreturn) {
	
	/*
	6.1d4 AR: Reviewed for proper error handling and reporting.

	6.1d15 AR: Fixed crashing bug, we were disposing of h even though it had been assigned to ht.
	*/

	bigstring bscode, bslong;
	Handle h = nil;
	Handle hpage = nil;
	hdlhashtable ht = nil;
	hdlhashnode hn;
	long ix = 0;
	boolean flresult = false;

	if (!newemptyhandle (&h))
		goto done;

	while (hashgetnthnode (hmethodstable, ix++, &hn))
		if (!visitmethods (hn, &h))
			goto done;

	if (!newhashtable (&ht))
		goto done;

	if (!langassigntextvalue (ht, STR_P_ALLOW, h))
		goto done;

	h = nil; /*it's now part of ht, make sure we don't dispose it twice*/
	
	numbertostring (405, bscode);

	parsedialogstring (STR_P_METHOD_NOT_ALLOWED, bsmethod, nil, nil, nil, bslong);

	if (!webserverbuilderrorpagefromstrings (bscode, bslong, &hpage))
		goto done;

	if (!webserverbuildresponse (bscode, ht, hpage, vreturn))
		goto done;

	flresult = true;

done:

	disposehandle (h);
	
	disposehandle (hpage);
	
	disposehashtable (ht, false);
	
	return (flresult);
	} /*webservermethodnotallowed*/
	
		
static boolean webservergetmethod (hdlhashtable hpt, tyaddress *adrresponder, tyaddress *adrmethod, tyvaluerecord *vreturn) {

	/*
	6.1d2 AR: Determine which responder method should handle this request.
	Build an error message if the specified method is not allowed.
		
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	hdlhashtable hrespondertable, hmethodstable;
	bigstring bsmethod;
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;

	if (!langlookupstringvalue (hpt, STR_P_METHOD, bsmethod))
		return (false);
	
	if (!langhashtablelookup ((*adrresponder).ht, (*adrresponder).bs, &val, &hnode)
			|| !langexternalvaltotable (val, &hrespondertable, hnode))
		return (false);
	
	if (!langlookupvaluefollow (hrespondertable, STR_P_METHODS, &val, &hnode)
			|| !langexternalvaltotable (val, &hmethodstable, hnode))
		return (false);

	copystring (bsmethod, (*adrmethod).bs);
	
	(*adrmethod).ht = hmethodstable;

	disablelangerror ();
	
	fl = followaddress (adrmethod);
	
	enablelangerror ();

	if (fl)
		return (true);

	copystring (STR_P_ANY, (*adrmethod).bs);
	
	(*adrmethod).ht = hmethodstable;

	disablelangerror ();
	
	fl = followaddress (adrmethod);

	enablelangerror ();
	
	if (fl)
		return (true);

	return (webservermethodnotallowed (hmethodstable, bsmethod, vreturn));
	} /*webservergetmethod*/


static boolean webserveraddtoerrorlog (tyaddress *adrmethod, bigstring bstype, bigstring bserror) {

	/*
	6.1d16 AR: Report webserver method errors.
	*/

	Handle htext = nil;
	Handle htype = nil;
	Handle herror = nil;
	Handle haddress = nil;
	bigstring bsaddress;
	tyvaluerecord val, vparams, vaddress;
	hdllistrecord hlist;

	disablelangerror ();

	if (!opnewlist (&hlist, false))
		goto exit;

	if (!setheapvalue ((Handle) hlist, listvaluetype, &vparams))
		goto exit;

	if (!setaddressvalue ((*adrmethod).ht, (*adrmethod).bs, &vaddress))
		goto exit;

	if (!getaddresspath (vaddress, bsaddress))
		goto exit;

	if (!newtexthandle (BIGSTRING ("\x0A" "^0: ^1: ^2"), &htext)
			|| !newtexthandle (bstype, &htype)
			|| !newtexthandle (bserror, &herror)
			|| !newtexthandle (bsaddress, &haddress)
			|| !parsedialoghandle (htext, htype, haddress, herror, nil)) {

		disposehandle (htext);

		disposehandle (htype);

		disposehandle (herror);

		disposehandle (haddress);

		goto exit;
		}
	
	disposehandle (htype);

	disposehandle (herror);

	disposehandle (haddress);

	if (!langpushlisttext (hlist, htext)) /*consumes htext*/
		goto exit;

	if (!langpushliststring (hlist, BIGSTRING ("\x09" "webserver")))
		goto exit;

	if (!langrunscript (BIGSTRING ("\x1D" "system.verbs.builtins.log.add"), &vparams, nil, &val))
		goto exit;

exit:
	enablelangerror ();

	return (true);
	} /*webservererrorlog*/


static boolean webservercallresponder (tyaddress *pta, tyaddress *adrresponder, tyvaluerecord *vreturn) {
	
	/*
	6.1d2 AR: Call responder, run postfilters, and build response.
		
	6.1d4 AR: Reviewed for proper error handling and reporting.
	
	6.2b10 AR: Implemented writing of file to stream.
	*/

	tyaddress adrscript;
	hdlhashtable hparamtable;
	bigstring bserror;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	hdlhashnode hnode;

	if (!langsuretablevalue ((*pta).ht, (*pta).bs, &hparamtable))
		return (false);

	/* locate method, possibly switch to the ANY method */
	
	setbooleanvalue (false, vreturn);

	if (!webservergetmethod (hparamtable, adrresponder, &adrscript, vreturn))
		return (false);
	
	if ((*vreturn).valuetype == stringvaluetype) /* webservermethod created an error response */
		return (true);

	/* set up our own error reporting */

	setemptystring (bserror);
	
	langtraperrors (bserror, &savecallback, &saverefcon);

	/* call the responder method */

	if (!langcallscriptwithaddress (&adrscript, pta, nil, vreturn))
		goto internal_error;
	
	/* build response if neccessary */
	
	if (((*vreturn).valuetype == booleanvaluetype) && ((*vreturn).data.flvalue == true)) {
		
		tyvaluerecord val;
		bigstring bscode;
		hdlhashtable hresponseheaderstable;
		
		/* run post-filters */
		
		if (!webservercallfilters (pta, STR_P_USERWEBSERVERPOSTFILTERS, STR_P_POSTFILTERERROR))
			goto internal_error;

		/* build response */
		
		if (!langlookupstringvalue (hparamtable, STR_P_CODE, bscode))
			goto internal_error;

		if (!langsuretablevalue (hparamtable, STR_P_RESPONSEHEADERS, &hresponseheaderstable))
			goto internal_error;

		if (!langhashtablelookup (hparamtable, STR_P_RESPONSEBODY, &val, &hnode))
			goto internal_error;

#if 0		
		if (val.valuetype == filespecvaluetype) { /* 6.2b10 AR: write the file itself to the stream */
			
			tyvaluerecord vserve;
			
			if (webservergetpref (BIGSTRING ("\x19" "flEnableDirectFileServing"), &vserve)) {
			
				if (!coercetoboolean (&vserve))
					goto internal_error;
				
				if (vserve.data.flvalue) {
					
					unsigned long stream;
					unsigned long fsize;
					tyvaluerecord vheader;
					tyfilespec fs = **val.data.filespecvalue;
					
					if (!filesize (&fs, &fsize))
						goto internal_error;	

					if (!langassignlongvalue (hresponseheaderstable, STR_P_CONTENT_LENGTH, fsize))
						goto internal_error;	

					if (!webserverbuildresponse (bscode, hresponseheaderstable, nil, &vheader))
						goto internal_error;	

					if (!langlookuplongvalue (hparamtable, STR_P_STREAM, &stream))
						goto internal_error;	
					
					if (!fwsNetEventWriteFileToStream (stream, vheader.data.stringvalue, nil, &fs))
						goto internal_error;	
					
					if (!setbooleanvalue (true, vreturn))
						goto internal_error;
					
					goto done;
					}
				}
			}
#endif

		if (!copyvaluerecord (val, &val) || !coercetostring (&val))
			goto internal_error;

		if (!webserverbuildresponse (bscode, hresponseheaderstable, val.data.stringvalue, vreturn))
			goto internal_error;
		}

/*done:*/

	languntraperrors (savecallback, saverefcon, false);

	fllangerror = false;

	return (true);

internal_error: {

		Handle hpage;
		bigstring bscode;

		/* tear down our error handling */

		languntraperrors (savecallback, saverefcon, true);

		fllangerror = false;

		if (!ingoodthread ()) /*unwind quickly*/
			return (false);

		/* log webserver error */

		webserveraddtoerrorlog (&adrscript, STR_P_RESPONDERERROR, bserror);

		/* unlock our semaphores */

		langreleasesemaphores (nil);

		/* webserver.util.buildResponse (500, nil, webserver.util.buildErrorPage ("500 Server Error", tryError)) */

		numbertostring (500L, bscode);
	
		if (!webserverbuilderrorpagefromstrings (bscode, bserror, &hpage))
			return (false);

		if (!webserverbuildresponse (bscode, nil, hpage, vreturn)) {

			disposehandle (hpage);

			return (false);
			}

		disposehandle (hpage);
		}

	return (true);
	} /*webservercallresponder*/


static boolean webserverdispatch (tyaddress *pta, tyvaluerecord *vreturn) {

	/*
	6.1d2 AR: Dispatch the request to the appropriate responder.
		
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	hdlhashtable hparamtable, hresponseheaderstable;
	tyaddress adrresponder;
	bigstring bsrespondername;
	Handle h = nil;
	
	if (!langsuretablevalue ((*pta).ht, (*pta).bs, &hparamtable))
		return (false);
	
	/* call pre-filters */
	
	if (!webservercallfilters (pta, STR_P_USERWEBSERVERPREFILTERS, STR_P_PREFILTERERROR))
		return (false);
	
	/* determine name of responder and address of responder table */
	
	if (!webserverlocateresponder (hparamtable, bsrespondername, &adrresponder))
		return (false);
	
	/* initialize paramtable fields */
	
	if (!langassignstringvalue (hparamtable, STR_P_RESPONDER, bsrespondername))
		return (false);
	
	if (!langassignaddressvalue (hparamtable, STR_P_RESPONDERTABLEADR, &adrresponder))
		return (false);

	if (!langassignlongvalue (hparamtable, STR_P_CODE, 200))
		return (false);

	if (!newemptyhandle (&h))
		return (false);

	if (!langassigntextvalue (hparamtable, STR_P_RESPONSEBODY, h)) {
	
		disposehandle (h);
		
		return (false);
		}
	
	if (!langassignnewtablevalue (hparamtable, STR_P_RESPONSEHEADERS, &hresponseheaderstable))
		return (false);

	/* call responder, run postfilters, and build response */		

	return (webservercallresponder (pta, &adrresponder, vreturn));
	} /*webserverdispatch*/
	

static boolean webserverreadrequest (hdlhashtable ht, Handle h, long *errorcode, bigstring bserror) {

	/*
	6.1d2 AR: Make sure we have the whole request, reading from the stream as neccessary.
	We assume that we are allowed to modify h.
		
	6.1d4 AR: Reviewed for proper error handling and reporting.

	6.1b9 AR: Adapted to fwsNetEventsReadStreamUntil changes.
	*/

	hdlhashtable hheaderstable;
	Handle hrequestbody = nil;
	Handle hpattern = nil;
	Handle hfirstline = nil;
	long contentlength;
	long stream = -1;
	long timeout = 30;
	boolean flresult = false;
	boolean fl;

	/* Try to lookup stream and timeout in paramtable */

	disablelangerror (); /* ignoring errors since we have default values */

	langlookuplongvalue (ht, STR_P_STREAM, &stream);

	langlookuplongvalue (ht, STR_P_TIMEOUT, &timeout);

	enablelangerror ();

	/* Make sure we have the request headers */

	if (!newtexthandle (STR_P_CRLFCRLF, &hpattern))
		goto exit;

	if (h != nil) { /* Assume we don't own the handle, so make a copy */
		if (!copyhandle (h, &h))
			goto exit;
		}
	else { /* Read headers*/

		if (!newemptyhandle (&h))
			goto exit;

		if (!fwsNetEventReadStreamUntil (stream, h, hpattern, timeout))
			goto exit;
		}

	/* For robustness remove leading cr/lf's and spaces from request */
	/*{
	
		long i, hsize = gethandlesize (h);
		byte ch;

		for (i = 0; i < hsize; i++) {

			ch = (*(byte **)h) [i];

			if ((ch != '\r') && (ch != '\n') && (ch != chspace))
				break;
			}
		
		if (i > 0)
			pullfromhandle (h, 0, i, nil);	
		}
*/	
	/* Create requestHeaders sub-table and parse headers */

	if (!langassignnewtablevalue (ht, STR_P_REQUESTHEADERS, &hheaderstable))
		goto exit;

	if (!webserverparseheaders (h, hheaderstable, &hfirstline))
		goto exit;


	/* Check for Expect header -- if expecting a Continue response, send it, otherwise fail*/

	if (hashtablesymbolexists (hheaderstable, STR_P_EXPECT)) { /*PBS 7.0b43: handle before reading body*/

		bigstring bsexpectheader;

		langlookupstringvalue (hheaderstable, STR_P_EXPECT, bsexpectheader);

		if (!equalidentifiers (bsexpectheader, STR_P_100CONTINUE)) { /*DotNet compatibility*/

			//fwsNetEventWriteStream (stream, sizestatuscontinue, STR_STATUSCONTINUE);
			//} /*if*/

	//	else { /*Can't fulfill expectation*/

			*errorcode = 417;
					
			flresult = true;
					
			goto exit;
			} /*else*/
		} /*if*/


	/* Now that the request headers are parsed, make sure we have the whole body */

	disablelangerror ();

	fl = langlookuplongvalue (hheaderstable, STR_P_CONTENT_LENGTH, &contentlength);

	enablelangerror ();

	if (fl && (contentlength > 0)) { /* there's a body, read it */

		long ctpattern = gethandlesize (hpattern);
		long ctrequest = gethandlesize (h);
		long ctheaders = searchhandle (h, hpattern, 0, ctrequest); /* length of headers */
		long ctfullrequest;
		long ixbodystart;
		//long bytestoread;

		//assert (ctheaders != -1);

		ctfullrequest = ctheaders + ctpattern + contentlength;

		//bytestoread = ctfullrequest - ctrequest;

		if (/*bytestoread > 0 &&*/ !fwsNetEventReadStreamBytes (stream, h, ctfullrequest, timeout)) {
			
			*errorcode = 400;
			
			copystring (STR_P_BODY_NOT_READ, bserror);
			
			flresult = true;
			
			goto exit;
			}

		ctrequest = gethandlesize (h);

		if (ctrequest > ctfullrequest && !sethandlesize (h, ctfullrequest)) /* remove trailing junk */
			goto exit;
		
		ixbodystart = ctheaders + ctpattern;

		if (!loadfromhandletohandle (h, &ixbodystart, contentlength, false, &hrequestbody))
			goto exit;
		}

	/* Check for Expect header -- we may not be able to live up to the client's expectations */

/*	if (hashtablesymbolexists (hheaderstable, STR_P_EXPECT)) { /% PBS 7.0b43: handle before reading body %/
	
		*errorcode = 417;
		
		flresult = true;
		
		goto exit;
		}*/

	/* Add cells to paramtable: firstLine, request, requestBody, host */
	
	disablelangerror ();
	
	langtablecopyvalue (hheaderstable, ht, STR_P_HOST);
	
	enablelangerror ();
	
	if(!langassigntextvalue (ht, STR_P_FIRSTLINE, hfirstline))
		goto exit;
	
	hfirstline = nil; /*make sure it won't be nuked anymore*/

	if (hrequestbody == nil)
		if (!newemptyhandle (&hrequestbody))
			goto exit;

	if(!langassigntextvalue (ht, STR_P_REQUESTBODY, hrequestbody))
		goto exit;

	hrequestbody = nil; /*make sure it won't be nuked anymore*/

	if(!langassigntextvalue (ht, STR_P_REQUEST, h))
		goto exit;

	h = nil; /*make sure it won't be nuked anymore*/

	flresult = true;

exit:

	disposehandle (hpattern);

	disposehandle (hfirstline);
	
	disposehandle (hrequestbody);

	disposehandle (h);
	
	return (flresult);
	} /*webserverreadrequest*/


static boolean webserverprocessfirstline (hdlhashtable ht, long *errorcode, bigstring bserror) {

	/*
	6.1d1 AR: Process the first line of a HTTP request.
	The request is already in the paramtable.
		
	6.1d4 AR: Reviewed for proper error handling and reporting.
	
	6.2a9 AR: Future-style request lines of the form "POST http://localhost/RPC2 HTTP/1.0"
	weren't handled gracefully. hpath would end up being "localhost/RPC2" instead of "/RPC2".
	*/
	
	Handle h = nil;
	Handle hmethod = nil;
	Handle hpath = nil;
	Handle huri = nil;
	Handle hversion = nil;	
	Handle hsearchargs = nil;	
	Handle hpathargs = nil;	
	long hsize;
	long wdstart, wdlength;
	boolean flresult = false;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	/* Look up the request line in the paramtable */
	
	if (!langhashtablelookup (ht, STR_P_FIRSTLINE, &val, &hnode))
		goto exit;
	
	if (val.valuetype != stringvaluetype)
		if (!copyvaluerecord (val, &val) || !coercetostring (&val))
			goto exit;
	
	h = val.data.stringvalue; /*still on tmpstack or in table*/
	
	/* Scan the request line and set up hmethod, hpath, and hversion:
	   GET /index.html HTTP/1.1  */

	hsize = gethandlesize (h);
	
	if (!textnthword ((ptrbyte)(*h), hsize, 1, chspace, false, &wdstart, &wdlength)) {
		
		*errorcode = 400;
		
		copystring (STR_P_INVALID_REQUEST_LINE, bserror);
		
		flresult = true;
		
		goto exit;
		}

	if (!loadfromhandletohandle (h, &wdstart, wdlength, false, &hmethod))
		goto exit;

	if (!textnthword ((ptrbyte)(*h), hsize, 2, chspace, false, &wdstart, &wdlength)) {
		
		*errorcode = 400;
		
		copystring (STR_P_INVALID_REQUEST_LINE, bserror);
		
		flresult = true;
		
		goto exit;
		}
	
	if (!loadfromhandletohandle (h, &wdstart, wdlength, false, &hpath))
		goto exit;

	if (!textnthword ((ptrbyte)(*h), hsize, 3, chspace, false, &wdstart, &wdlength)) {
		
		*errorcode = 400;
		
		copystring (STR_P_INVALID_REQUEST_LINE, bserror);
		
		flresult = true;
		
		goto exit;
		}
	
	if (!loadfromhandletohandle (h, &wdstart, wdlength, false, &hversion))
		goto exit;

	/* Extract HTTP version number: "HTTP/1.1" --> "1.1" */
	
	if (!textnthword ((ptrbyte)(*hversion), gethandlesize (hversion), 2, '/', false, &wdstart, &wdlength)) {
		
		*errorcode = 400;
		
		copystring (STR_P_INVALID_REQUEST_LINE, bserror);
		
		flresult = true;
		
		goto exit;
		}

	if (!pullfromhandle (hversion, 0, wdstart, nil))
		goto exit;
	
	/* Check HTTP version */
	
	if (memcmp (*hversion, "1.1", 3) == 0) { /* Host header is obligatory in HTTP/1.1 */

		if (!hashtablesymbolexists (ht, STR_P_HOST)) {
		
			*errorcode = 400;
			
			copystring (STR_P_MISSING_HOST_HEADER, bserror);
			
			flresult = true;

			goto exit;			
			}
		}
	else { /* We don't know how to handle HTTP versions greater than 1.x */

		bigstring bsmainversion;
		long mainversion = 1;
	
		if (!textnthword ((ptrbyte)(*hversion), gethandlesize (hversion), 1, '.', false, &wdstart, &wdlength)) {
			
			*errorcode = 400;
			
			copystring (STR_P_INVALID_REQUEST_LINE, bserror);
			
			flresult = true;
			
			goto exit;
			}
		
		texttostring (*hversion, wdlength, bsmainversion);
		
		stringtonumber (bsmainversion, &mainversion);
		
		if (mainversion > 1) {
			bigstring bsversion;

			*errorcode = 505;
			
			texthandletostring (hversion, bsversion);
			
			parsedialogstring (STR_P_UNSUPPORTED_VERSION, bsversion, nil, nil, nil, bserror);
			
			flresult = true;

			goto exit;			
			}
		}
	
	/* Check path */
	
	if ((getlower((*hpath)[0]) == 'h')
		&& (getlower((*hpath)[1]) == 't')
		&& (getlower((*hpath)[2]) == 't')
		&& (getlower((*hpath)[3]) == 'p')
		&& (getlower((*hpath)[4]) == ':')) { /* It's a future-style full URL, remove scheme and host name */
		
		if (!textnthword ((ptrbyte)(*hpath), gethandlesize (hpath), 3, '/', true, &wdstart, &wdlength)
				|| !pullfromhandle (hpath, 0, wdstart + wdlength, nil)) {
			
			*errorcode = 400;
			
			copystring (STR_P_INVALID_URI, bserror);
			
			flresult = true;
			
			goto exit;
			}
		}
	else if (((*hpath)[0] != '/') && (((*hpath)[0] != '*') || (gethandlesize (hpath) != 1))) {
		
		*errorcode = 400;
		
		copystring (STR_P_INVALID_URI, bserror);
		
		flresult = true;
		
		goto exit;
		}

	/* Extract searchArgs from path -- the stuff after the '?' */

	if (textnthword ((ptrbyte)(*hpath), gethandlesize (hpath), 2, '\?', true, &wdstart, &wdlength))
		if (!loadfromhandletohandle (hpath, &wdstart, wdlength, false, &hsearchargs))
			goto exit;

	if (textnthword ((ptrbyte)(*hpath), gethandlesize (hpath), 1, '\?', true, &wdstart, &wdlength))
		sethandlesize (hpath, wdlength); /* keep only stuff before the question mark */

	/* Extract pathArgs from path -- the stuff after the '$' */

	wdstart = textpatternmatch ((byte *)(*hpath), gethandlesize (hpath), STR_P_DOLLAR, false);

	if (wdstart == -1) { /*fix for proxies which encode "$" as "%24"*/

		wdstart = textpatternmatch ((byte *)(*hpath), gethandlesize (hpath), STR_P_DOLLAR_ENCODED, false);

		if (wdstart != -1)
			pullfromhandle (hpath, wdstart + 1, 2, nil);
		}

	if (wdstart != -1) {

		long temp = wdstart + 1;

		if (!loadfromhandletohandle (hpath, &temp, gethandlesize (hpath) - temp, false, &hpathargs))
			goto exit;

		sethandlesize (hpath, wdstart);
		}

	/* Add paramtable field: method */

	if(!langassigntextvalue (ht, STR_P_METHOD, hmethod))
		goto exit;

	hmethod = nil;
	
	/* Add paramtable fields: path, URI */

	if (!copyhandle (hpath, &huri))
		goto exit;

	if(!langassigntextvalue (ht, STR_P_PATH, hpath))
		goto exit;

	hpath = nil;

	if(!langassigntextvalue (ht, STR_P_URI, huri))
		goto exit;

	huri = nil;
	
	/* Add paramtable fields: searchArgs, pathArgs */

	if (hsearchargs == nil)
		if (!newemptyhandle (&hsearchargs))
			goto exit;
	
	if(!langassigntextvalue (ht, STR_P_SEARCHARGS, hsearchargs))
		goto exit;

	hsearchargs = nil;

	if (hpathargs == nil)
		if (!newemptyhandle (&hpathargs))
			goto exit;

	if(!langassigntextvalue (ht, STR_P_PATHARGS, hpathargs))
		goto exit;

	hpathargs = nil;
	
	flresult = true;

exit: /* clean up memory */

	disposehandle (hmethod);
	
	disposehandle (hpath);

	disposehandle (huri);
	
	disposehandle (hversion);
	
	disposehandle (hsearchargs);
	
	disposehandle (hpathargs);
	
	return (flresult);
	} /*webserverprocessfirstline*/


static boolean webservermaintainstats (void) {

	/*
	6.1d1 AR: called from webserverserver to maintain
	the stats in the user.webserver.stats table.
		
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	hdlhashtable hstatstable;
	
	disablelangerror (); /*ignore all errors*/
	
	if (langfastaddresstotable (roottable, STR_P_USERWEBSERVERSTATS, &hstatstable)) {

		long hits, maxconn, maxbytes, minbytes;
		long ctthreads = processthreadcount ();
		long ctbytes = FreeMem ();
		
		/*	if threads > user.webserver.stats.maxConnections {
				user.webserver.stats.maxConnections = threads}; */

		if (langlookuplongvalue (hstatstable, STR_P_MAXCONNECTIONS, &maxconn) && (ctthreads > maxconn))
			langassignlongvalue (hstatstable, STR_P_MAXCONNECTIONS, ctthreads);

		/* if memavail > user.webserver.stats.maxmemavail {
				user.webserver.stats.maxmemavail = memavail}; */

		if (langlookuplongvalue (hstatstable, STR_P_MAXMEMAVAIL, &maxbytes) && (ctbytes > maxbytes))
			langassignlongvalue (hstatstable, STR_P_MAXMEMAVAIL, ctbytes);

		/* if memavail < user.webserver.stats.minmemavail {
				user.webserver.stats.minmemavail = memavail}; */
				
		if (langlookuplongvalue (hstatstable, STR_P_MINMEMAVAIL, &minbytes) && (ctbytes < minbytes))
			langassignlongvalue (hstatstable, STR_P_MINMEMAVAIL, ctbytes);

		/* user.webserver.stats.hits++ */
		
		if (langlookuplongvalue (hstatstable, STR_P_HITS, &hits))
			langassignlongvalue (hstatstable, STR_P_HITS, ++hits);
		}
	
	enablelangerror ();
	
	return (true);
	} /*webservermaintainstats*/


static boolean webserverserver (tyaddress *pta, Handle hrequest, tyvaluerecord *vreturn) {

	/*
	6.1d1 AR: Kernelized system.verbs.builtins.webserver.server.
	Pass in nil for hrequest if you want us to read the request from the stream.
			
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	hdlhashtable hstatstable, hparamtable;
	tyvaluerecord val;
	long statuscode = 200;
	bigstring bsexplanation;

	setemptystring (bsexplanation);
	
	/* Wait here until Frontier has finished starting up */

	while (langgetuserflag (idfrontierstartup, false)) {

		if (!langgetuserflag (idflwaitduringstartup, true))
			break;

		processsleep (getcurrentthread (), 5 * 60);
		}

	/* Make sure we have a valid param table */
	
	if (!langsuretablevalue ((*pta).ht, (*pta).bs, &hparamtable))
		goto exit;

	/* Create stats sub-table in param table and record current tick count */

	if (!langassignnewtablevalue (hparamtable, STR_P_STATS, &hstatstable))
		goto exit;

	if (!langassignlongvalue (hstatstable, STR_P_PROCESSING_STARTED, gettickcount ()))
		goto exit;

	/* Read the full request and parse the headers, write everything to paramtable */

	if (!webserverreadrequest (hparamtable, hrequest, &statuscode, bsexplanation))
		goto exit;
	
	if (statuscode != 200)
		goto internal_error;

	/* Parse the first line of the request and add the following cells to the param table:
	   method, path, URI, pathArgs, searchArgs */

	if (!webserverprocessfirstline (hparamtable, &statuscode, bsexplanation))
		goto exit;
	
	if (statuscode != 200)
		goto internal_error;

	/* Parse Cookie header line and add Cookies sub-table to param table */

	if (!webserverparsecookies (hparamtable, &val))
		goto exit;

	/* Dispatch the request */

	if (!webserverdispatch (pta, vreturn))
		goto exit;

	/* Maintain stats, if the feature is turned on (our default is off) */
	
	if (langgetuserflag (idwebserverstats, false))
		if (!webservermaintainstats ()) {

			assert (false);
	
			goto exit;
			}
	
	return (true);

internal_error: {
		
		bigstring bscode;
		Handle hbody = nil;
		boolean fl = true;
		
		numbertostring (statuscode, bscode);
		
		if (!isemptystring (bsexplanation))
			if (!webserverbuilderrorpagefromstrings (bscode, bsexplanation, &hbody))
				return (false);
		
		fl = webserverbuildresponse (bscode, nil, hbody, vreturn);
		
		disposehandle (hbody);

		return (fl);
		}

exit:

	return (false);
	}/*webserverserver*/


static unsigned char * bswhatwerewedoing [] = {
	BIGSTRING ("\x21" "Checking user.inetd.shutdown flag"),
	BIGSTRING ("\x13" "Creating paramtable"),
	BIGSTRING ("\x1e" "Adding client IP to paramtable"),
	BIGSTRING ("\x17" "Initializing paramtable"),
	BIGSTRING ("\x29" "Looking up timeout and chunksize settings"),
	BIGSTRING ("\x10" "Waiting for data"),
	BIGSTRING ("\x12" "Calling the daemon"),
	BIGSTRING ("\x24" "Returning the response to the client"),
	BIGSTRING ("\x0B" "Cleaning up")
	};

static boolean inetdaddtoerrorlog (long code, bigstring bserror, hdlhashtable hparamtable) {

	/*
	6.1d1 AR: Record errors in the WebserverErrors sub-table of the daily log GDB.
			
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/

	hdlhashtable hparenttable, hlogtable;
	tyvaluerecord vlogtable;
	bigstring bstablename;
	Handle hscript = nil;
	
	if (hparamtable == nil) /*crash prevention*/
		return (true);

	/* get address of new table in log GDB */

	if (!newtexthandle (BIGSTRING ("\x33""log.addToGuestDatabase (\"inetd\", flHourlyRoll:true)"), &hscript))
		return (false);

	if (!langrun (hscript, &vlogtable) || !coercetoaddress (&vlogtable)) /*htext is always consumed*/
		return (false);
	
	if (!getaddressvalue (vlogtable, &hparenttable, bstablename))
		return (false);
	
	if (!langsuretablevalue (hparenttable, bstablename, &hlogtable))
		return (false);

	/* fill log table */
	
	disablelangerror ();

	langassignstringvalue (hlogtable, STR_P_WHATWEREWEDOING, bswhatwerewedoing[code]);

	langassignstringvalue (hlogtable, STR_P_WHATWENTWRONG, bserror);

	langassignlongvalue (hlogtable, STR_P_THREAD, getthreadid (getcurrentthread ()));

	langtablecopyvalue (hparamtable, hlogtable, STR_P_STREAM);
	
	langtablecopyvalue (hparamtable, hlogtable, STR_P_PORT);

	langtablecopyvalue (hparamtable, hlogtable, STR_P_CLIENT);
	
	enablelangerror ();

	return (true);
	} /*inetdaddtoerrorlog*/


static boolean inetdsupervisor (long stream, long refcon, tyvaluerecord * vreturn) {

	/*
	6.1d1 AR: The entry point for the kernelized webserver.
	
	6.1d4 AR: Reviewed for proper error handling and reporting.
	*/
	
	hdlhashtable hparamtable = nil;
	hdlhashtable hconfigtable;
	Handle hrequest = nil;
	tyaddress adrparamtable;
	tyvaluerecord vparamtable, vreturndata;
	bigstring bspeeraddress, bserror;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	unsigned long peeraddress, peerport;
	long timeout = 30; /*seconds*/
	long whatarewedoing = 0;
	long chunksize = 8192; /*bytes*/
	hdlhashtable ht, hprefstable;
	bigstring bsrefcon;
	tyvaluerecord vconfigtable;
	tyaddress adrconfigtable;
	tyaddress adrscript;
	boolean flGotTimeout, flGotChunksize;
	boolean flNoWait = false;
	hdlhashnode hnode;

	/* check for valid stream id */

	if (stream < 0) //return immediately, can't close listen from here
		return (true);
	
	/* set up error trapping so that we can log any errors */

	setemptystring (bserror);
	
	langtraperrors (bserror, &savecallback, &saverefcon);

	/* check user.inetd.shutdown flag */

	if (langgetuserflag (idinetdshutdown, false)) {

		bigstring bsresult;
	
		if (!fwsNetEventCloseStream (stream))
			goto exit;

		if (!langrunstring (BIGSTRING ("\x0C" "inetd.stop()"), bsresult))
			goto exit;

		goto done;
		}

	/* new (tableType, @paramTable) */
	/* local (adrparamtable = @paramtable) */

	whatarewedoing++;

	if (!tablenewtablevalue (&hparamtable, &vparamtable))
		goto exit;

	if (!langsetsymbolval (STR_P_PARAMTABLE, vparamtable)) { 

		tabledisposetable (hparamtable, false);

		goto exit;
		}

	findinparenttable (hparamtable, &adrparamtable.ht, adrparamtable.bs);

	/* get the client IP and add it to the paramtable */
	
	whatarewedoing++;

	if (!fwsNetEventGetPeerAddress (stream, &peeraddress, &peerport))
		goto exit;

	if (!fwsNetEventAddressDecode (peeraddress, bspeeraddress))
		goto exit;

	if (!langassignstringvalue (hparamtable, STR_P_CLIENT, bspeeraddress))
		goto exit;

	/* init more paramtable values: ready, refcon, stream */
	
	whatarewedoing++;

	if (!langassignbooleanvalue (hparamtable, STR_P_READY, true))
		goto exit;

	if (!langassignlongvalue (hparamtable, STR_P_REFCON, refcon))
		goto exit;

	if (!langassignlongvalue (hparamtable, STR_P_STREAM, stream))
		goto exit;

	/*  init more paramtable values: inetdConfigTableAdr */

	if (!langfastaddresstotable (roottable, STR_P_USERINETDLISTENS, &ht))
		goto exit;
	
	numbertostring (refcon, bsrefcon);

	if (!langhashtablelookup (ht, bsrefcon, &vconfigtable, &hnode)
		|| !langexternalvaltotable (vconfigtable, &ht, hnode))
		goto exit;

	if (!langlookupaddressvalue (ht, STR_P_ADRTABLE, &adrconfigtable))
		goto exit;

	if (!langassignaddressvalue (hparamtable, STR_P_INETDCONFIGTABLEADR, &adrconfigtable))
		goto exit;

	/*  init more paramtable values: port */

	if (!langhashtablelookup (adrconfigtable.ht, adrconfigtable.bs, &vconfigtable, &hnode)
		|| !langexternalvaltotable (vconfigtable, &hconfigtable, hnode))
		goto exit;
	
	if (!langtablecopyvalue (hconfigtable, hparamtable, STR_P_PORT))
		goto exit;

	/* get timeout and chunksize from daemon config table */

	whatarewedoing++;
	
	disablelangerror ();

	flGotTimeout = langlookuplongvalue (hconfigtable, STR_P_TIMEOUT, &timeout);

	flGotChunksize = langlookuplongvalue (hconfigtable, STR_P_CHUNKSIZE, &chunksize);

	enablelangerror ();

	if (!flGotTimeout || !flGotChunksize) {

		if (!langfastaddresstotable (roottable, STR_P_USERINETDPREFS, &hprefstable))
			goto exit;

		if (!flGotTimeout)
			if (!langlookuplongvalue (hprefstable, STR_P_DEFAULTTIMEOUTSECS, &timeout))
				goto exit;

		if (!flGotChunksize)
			if (!langlookuplongvalue (hprefstable, STR_P_RETURNCHUNKSIZE, &chunksize))
				goto exit;
		}


	if (!langassignlongvalue (hparamtable, STR_P_TIMEOUT, timeout))
		goto exit;

	/*  waiting for data & init more paramtable values: request */

	whatarewedoing++;
	
	if (!newemptyhandle (&hrequest))
		goto exit;

	if (!langassigntextvalue (hparamtable, STR_P_REQUEST, hrequest)) {

		disposehandle (hrequest);

		goto exit;
		}

//#if 0  //6.1b2 AR: last-minute addition for full backward compatibility with the obsolete script

	disablelangerror ();

	langlookupbooleanvalue (hconfigtable, STR_P_NOWAIT, &flNoWait);

	enablelangerror ();

	if (!flNoWait) {

		if (!fwsNetEventInetdRead (stream, hrequest, timeout))
			goto exit;

		langsymbolchanged (hparamtable, STR_P_REQUEST, nil, true);
		}

//#endif
	
	/* call the daemon */
	
	whatarewedoing++;

	adrscript.ht = hconfigtable;

	copystring (STR_P_DAEMON, adrscript.bs);

	if (!followaddress (&adrscript))
		goto exit;

	if (!langcallscriptwithaddress (&adrscript, &adrparamtable, nil, &vreturndata))
		goto exit;

	/* write response */
	
	whatarewedoing++;

	/* return a string */

	if (vreturndata.valuetype == stringvaluetype)
		if (!fwsNetEventWriteHandleToStream (stream, vreturndata.data.stringvalue, chunksize, timeout))
			goto exit;

	/* it's the end of the world as we know it */
	
	whatarewedoing++;

	if (!fwsNetEventCloseStream (stream))
		goto exit; /* On Mac OS: error -51 (bad refnum) -- WHY??? */

done:

	languntraperrors (savecallback, saverefcon, false);

	setbooleanvalue (true, vreturn);
	
	return (true);

exit:

	languntraperrors (savecallback, saverefcon, true);

	fllangerror = false; /*6.1b12 AR*/

	if (!ingoodthread ()) /*unwind quickly*/
		return (false);

	//disablelangerror ();	//can't do that because langerrordisable
							//is not preserved when a context switch occurs

	fwsNetEventAbortStream (stream);

	inetdaddtoerrorlog (whatarewedoing, bserror, hparamtable);

	//enablelangerror ();

	return (false);

}/*inetdsupervisor*/



#pragma mark === mainResponder: Calendar ===

/*
on getAddressDay (adr) {
	local (alist = string.parseaddress (adr));
	local (sizelist = sizeof (alist));
	local (day = number (alist [sizelist]));
	local (month = number (alist [sizelist - 1]));
	local (year = number (alist [sizelist - 2]));
	return (date.set (day, month, year, 0, 0, 0))}
*/

extern boolean parseaddress (Handle htext, tyvaluerecord *v); /*stringverbs.c*/

static boolean mrcalendargetaddressday (Handle htext, unsigned long *date) {
	
	tyvaluerecord vlist, val;
	long size, day, month, year;
	
	if (!parseaddress (htext, &vlist))
		return (false);

	assert (vlist.valuetype == listvaluetype);
	
	if (!langgetlistsize (&vlist, &size))
		return (false);

	if (!langgetlistitem (&vlist, size - 2, nil, &val))
		return (false);
	
	if (!coercetolong (&val))
		return (false);
	
	year = val.data.longvalue;

	if (!langgetlistitem (&vlist, size - 1, nil, &val))
		return (false);
	
	if (!coercetolong (&val))
		return (false);
	
	month = val.data.longvalue;
	
	if (!langgetlistitem (&vlist, size, nil, &val))
		return (false);
	
	if (!coercetolong (&val))
		return (false);
	
	day = val.data.longvalue;
	
	*date = datetimetoseconds (day, month, year, 0, 0, 0);
			
	return (true);
	}/*mrcalendargetaddressday*/


static boolean mrcalendargetaddressdayverb (hdltreenode hp1, tyvaluerecord *v) {
	
	Handle htext;
	unsigned long date;
	
	flnextparamislast = true;
	
	if(!getexempttextvalue (hp1, 1, &htext))
		return (false);
				
	if (!mrcalendargetaddressday (htext, &date))
		return (false);

	return (setdatevalue (date, v));
	}/*mrcalendargetaddressdayverb*/


/*
on getDayAddress (adrcalendar, d, flcreate=true, objtype=tabletype) { //turn a date into an address
	Çthis is the bottleneck
	
	local (day, month, year, hour, minute, second);
	date.get (d, @day, @month, @year, @hour, @minute, @second);
	year = string (year);
	month = string.padwithzeros (month, 2);
	day = string.padwithzeros (day, 2);
	
	local (adr = adrcalendar);
	on diveinto (name, type=tabletype) {
		adr = @adr^.[name];
		if (not defined (adr^)) and flcreate and (type != nil) {
			new (type, adr)}};
	diveinto (year);
	diveinto (month);
	diveinto (day, objtype);
	return (adr)}
*/

static boolean mrcalendargetdayaddressdive (short num, tyaddress *adr, boolean flcreate, OSType idtype) {
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!langhashtablelookup ((*adr).ht, (*adr).bs, &val, &hnode))
		return (false);

	if (!langexternalvaltotable (val, &(*adr).ht, hnode))
		return (false);
	
	shorttostring (num, (*adr).bs);
	
	while (stringlength ((*adr).bs) < 2)
		insertchar ('0', (*adr).bs);
	
	if (!langtablelookup ((*adr).ht, (*adr).bs, &(*adr).ht) && flcreate && idtype != nil) {

		tyvaluetype type = langgetvaluetype (idtype);
		
		if ((type >= outlinevaluetype) && (type <= pictvaluetype)) {
			
			hdlhashtable newtable;
			
			if (!langexternalnewvalue ((tyexternalid) (type - outlinevaluetype), nil, &val))
				return (false);

			if ((type == tablevaluetype) && langexternalvaltotable (val, &newtable, HNoNode))
				(**newtable).fllocaltable = (**(*adr).ht).fllocaltable;
			}
		else {
			initvalue (&val, novaluetype); /*nil all data*/
			
			if (!coercevalue (&val, type)) /*should only fail on low-mem*/
				return (false);
			}
		
		if (!langsetsymboltableval ((*adr).ht, (*adr).bs, val)) {
			disposevaluerecord (val, true);			
			return (false);
			}
		else
			exemptfromtmpstack (&val);
		}
	
	return (true);
	}/*mrcalendargetdayaddressdive*/
	

static boolean mrcalendargetdayaddress (tyaddress adrcalendar, unsigned long secs, boolean flcreate, OSType idtype, tyaddress *adr) {
	
	short day, month, year, hour, minute, second;
	
	*adr = adrcalendar;
	
	secondstodatetime (secs, &day, &month, &year, &hour, &minute, &second);
	
	if (!mrcalendargetdayaddressdive (year, adr, flcreate, 'tabl'))
		return (false);

	if (!mrcalendargetdayaddressdive (month, adr, flcreate, 'tabl'))
		return (false);
	
	if (!mrcalendargetdayaddressdive (day, adr, flcreate, idtype))
		return (false);
	
	return (true);
	}/*mrcalendargetdayaddress*/


static boolean mrcalendargetdayaddressverb (hdltreenode hp1, tyvaluerecord *v) {
	
	short ctconsumed = 2, ctpositional = 2;
	unsigned long secs;
	tyvaluerecord val;
	tyaddress adrcalendar, adr;	
	boolean flcreate;
	OSType idtype;
	
	if (!getaddressparam (hp1, 1, &val) || !getaddressvalue (val, &adrcalendar.ht, adrcalendar.bs))
		return (false);
	
	if (!getdatevalue (hp1, 2, &secs))
		return (false);
		
	setbooleanvalue (true, &val);
	
	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x08""flcreate"), &val))
		return (false);
	
	flcreate = val.data.flvalue;

	setostypevalue ('tabl', &val);
	
	flnextparamislast = true;
	
	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x07""objtype"), &val))
		return (false);
	
	idtype = val.data.ostypevalue;
	
	if (!mrcalendargetdayaddress (adrcalendar, secs, flcreate, idtype, &adr))
		return (false);
	
	return (setaddressvalue (adr.ht, adr.bs, v));
	}/*mrcalendargetdayaddressverb*/


/*
on getFirstAddress (adrcalendar) {
	local (adryear, adrmonth, adrday, ix);
	on diveinto (parent, adrsibling) {
		for ix = 1 to sizeof (parent^) {
			try {
				number (nameOf (parent^[ix]));
				adrsibling^ = @parent^[ix];
				return (true)}};
		return (false)};
	if diveinto (adrcalendar, @adryear) {
		if diveinto (adryear, @adrmonth) {
			if diveinto (adrmonth, @adrday) {
				return (adrday)}}};
	scripterror ("Can't get the first address in the calendar because adrcalendar doesn't point to a valid non-empty calendar structure.")}
*/


static boolean findfirstnumericnodevisit (bigstring bsname, hdlhashnode nomad, tyvaluerecord val, ptrvoid refcon) {
#pragma unused(val)

	if (isallnumeric (bsname)) {

		hdlhashnode *hnode = (hdlhashnode *) refcon;
		
		*hnode = nomad;
		
		return (true);
		}

	return (false);
	}/*findfirstnumericnodevisit*/
		

static boolean mrcalendargetfirstaddressverb (hdltreenode hp1, tyvaluerecord *v) {
	
	hdlhashtable ht;
	bigstring bs;
	hdlhashnode hn;

	flnextparamislast = true;
				
	if (!gettablevalue (hp1, 1, &ht))
		return (false);

	disablelangerror ();
	
	if (!hashsortedinversesearch (ht, &findfirstnumericnodevisit, &hn)) /*year*/
		goto error;
		
	if (!langexternalvaltotable ((**hn).val, &ht, hn))
		goto error;
	
	if (!hashsortedinversesearch (ht, &findfirstnumericnodevisit, &hn)) /*month*/
		goto error;
		
	if (!langexternalvaltotable ((**hn).val, &ht, hn))
		goto error;
	
	if (!hashsortedinversesearch (ht, &findfirstnumericnodevisit, &hn)) /*day*/
		goto error;

	enablelangerror ();
	
	gethashkey (hn, bs);
		
	return (setaddressvalue (ht, bs, v));

error:

	enablelangerror ();

	langerrormessage (BIGSTRING ("\x76""Can't get the first address in the calendar because adrcalendar doesn't point to a valid non-empty calendar structure."));

	return (false);
	}/*mrcalendargetfirstaddressverb*/

/*
on getFirstDay (adrcalendar) {
	local (adrday);
	try {
		adrday = mainResponder.calendar.getFirstAddress (adrcalendar);
		return (mainResponder.calendar.getAddressDay (adrday))}
	else {
		scripterror ("Can't get the first day in the calendar because adrcalendar doesn't point to a valid non-empty calendar structure.")}}
*/

static boolean mrcalendargetfirstday (hdlhashtable ht, tyvaluerecord *v) {

	hdlhashnode hn;
	short year, month, day;

	disablelangerror ();
	
	if (!hashsortedinversesearch (ht, &findfirstnumericnodevisit, &hn))
		goto error;
	
	stringtoshort ((**hn).hashkey, &year);
	
	if (!langexternalvaltotable ((**hn).val, &ht, hn))
		goto error;

	if (!hashsortedinversesearch (ht, &findfirstnumericnodevisit, &hn))
		goto error;
	
	stringtoshort ((**hn).hashkey, &month);
	
	if (!langexternalvaltotable ((**hn).val, &ht, hn))
		goto error;

	if (!hashsortedinversesearch (ht, &findfirstnumericnodevisit, &hn))
		goto error;
	
	stringtoshort ((**hn).hashkey, &day);

	enablelangerror ();
		
	return (setdatevalue (datetimetoseconds (day, month, year, 0, 0, 0), v));

error:

	enablelangerror ();

	langerrormessage (BIGSTRING ("\x72""Can't get the first day in the calendar because adrcalendar doesn't point to a valid non-empty calendar structure."));

	return (false);
	}/*mrcalendargetfirstday*/


static boolean mrcalendargetfirstdayverb (hdltreenode hp1, tyvaluerecord *v) {

	hdlhashtable ht;

	flnextparamislast = true;
				
	if (!gettablevalue (hp1, 1, &ht))
		return (false);

	return (mrcalendargetfirstday (ht, v));
	}/*mrcalendargetfirstdayverb*/

/*
on getLastAddress (adrcalendar) {
	local (adryear, adrmonth, adrday, ix);
	on diveinto (parent, adrsibling) {
		for ix = sizeof (parent^) downto 1 {
			try {
				number (nameOf (parent^[ix]));
				adrsibling^ = @parent^[ix];
				return (true)}};
		return (false)};
	if diveinto (adrcalendar, @adryear) {
		if diveinto (adryear, @adrmonth) {
			if diveinto (adrmonth, @adrday) {
				return (adrday)}}};
	scripterror ("Can't get the last address in the calendar because adrcalendar doesn't point to a valid non-empty calendar structure.")}
*/

static boolean findlastnumericnodevisit (bigstring bsname, hdlhashnode nomad, tyvaluerecord val, ptrvoid refcon) {
#pragma unused(val)

	if (isallnumeric (bsname)) {
	
		hdlhashnode *hnode = (hdlhashnode *) refcon;
	
		*hnode = nomad;
		}
		
	return (false);
	}/*findlastnumericnodevisit*/
		

static boolean mrcalendargetlastaddressverb (hdltreenode hp1, tyvaluerecord *v) {
	
	hdlhashtable ht;
	bigstring bs;
	hdlhashnode hn;

	flnextparamislast = true;
				
	if (!gettablevalue (hp1, 1, &ht))
		return (false);

	disablelangerror ();
	
	hn = nil;
	
	hashsortedinversesearch (ht, &findlastnumericnodevisit, &hn); /*year*/

	if (hn == nil)
		goto error;
	
	if (!langexternalvaltotable ((**hn).val, &ht, hn))
		goto error;
	
	hn = nil;
	
	hashsortedinversesearch (ht, &findlastnumericnodevisit, &hn); /*month*/

	if (hn == nil)
		goto error;
		
	if (!langexternalvaltotable ((**hn).val, &ht, hn))
		goto error;
	
	hn = nil;
		
	hashsortedinversesearch (ht, &findlastnumericnodevisit, &hn); /*day*/

	if (hn == nil)
		goto error;

	enablelangerror ();
	
	gethashkey (hn, bs);
		
	return (setaddressvalue (ht, bs, v));

error:

	enablelangerror ();

	langerrormessage (BIGSTRING ("\x75""Can't get the last address in the calendar because adrcalendar doesn't point to a valid non-empty calendar structure."));

	return (false);
	}/*mrcalendargetlastaddressverb*/

/*
on getLastDay (adrcalendar) {
	local (adrday);
	try {
		adrday = mainResponder.calendar.getLastAddress (adrcalendar);
		return (mainResponder.calendar.getAddressDay (adrday))}
	else {
		scripterror ("Can't get the last day in the calendar because adrcalendar does not point to a valid non-empty calendar structure.")}}
*/


static boolean mrcalendargetlastday (hdlhashtable ht, tyvaluerecord *v) {

	hdlhashnode hn;
	short year, month, day;

	disablelangerror ();
	
	hn = nil;
	
	hashsortedinversesearch (ht, &findlastnumericnodevisit, &hn); /*year*/

	if (hn == nil)
		goto error;
	
	stringtoshort ((**hn).hashkey, &year);
	
	if (!langexternalvaltotable ((**hn).val, &ht, hn))
		goto error;
	
	hn = nil;

	hashsortedinversesearch (ht, &findlastnumericnodevisit, &hn); /*month*/

	if (hn == nil)
		goto error;
	
	stringtoshort ((**hn).hashkey, &month);
	
	if (!langexternalvaltotable ((**hn).val, &ht, hn))
		goto error;
	
	hn = nil;

	hashsortedinversesearch (ht, &findlastnumericnodevisit, &hn); /*day*/

	if (hn == nil)
		goto error;
	
	stringtoshort ((**hn).hashkey, &day);

	enablelangerror ();
		
	return (setdatevalue (datetimetoseconds (day, month, year, 0, 0, 0), v));

error:

	enablelangerror ();

	langerrormessage (BIGSTRING ("\x71""Can't get the last day in the calendar because adrcalendar doesn't point to a valid non-empty calendar structure."));

	return (false);
	}/*mrcalendargetlastday*/


static boolean mrcalendargetlastdayverb (hdltreenode hp1, tyvaluerecord *v) {

	hdlhashtable ht;

	flnextparamislast = true;
				
	if (!gettablevalue (hp1, 1, &ht))
		return (false);

	return (mrcalendargetlastday (ht, v));
	}/*mrcalendargetlastdayverb*/

/*
on getMostRecentAddress (adrCalendar, d=clock.now ()) {
	local (adrDay, dmin = mainResponder.calendar.getFirstDay (adrCalendar));
	while (dmin <= d) {
		try {
			adrDay = mainResponder.calendar.getDayAddress (adrCalendar, d, flcreate:false);
			if defined (adrDay^) {
				return (adrDay)}};
		d = date.yesterday (d)};
	scripterror ("Can't get the most recent address in the calendar because the calendar doesn't contain any elements before " + d + ".")}
*/

static boolean mrcalendargetmostrecentaddress (tyaddress adrcalendar, unsigned long date, tyvaluerecord *v) {

	tyaddress adr;
	tyvaluerecord val;
	unsigned long mindate;
	const unsigned long oneday = 24 * 60 * 60;
	hdlhashnode hnode;
	hdlhashtable ht;
	bigstring bserror, bsdate;
		
	if (!langhashtablelookup (adrcalendar.ht, adrcalendar.bs, &val, &hnode) || !langexternalvaltotable (val, &ht, hnode) || !mrcalendargetfirstday (ht, &val))
		return (false);
	
	mindate = val.data.datevalue;
		
	disablelangerror ();
	
	while (mindate <= date) {
	
		if (mrcalendargetdayaddress (adrcalendar, date, false, 'tabl', &adr) && hashtablesymbolexists (adr.ht, adr.bs)) {
			
			enablelangerror ();
			
			return (setaddressvalue (adr.ht, adr.bs, v));
			}
	
		date -= oneday;
		}/*while*/

	enablelangerror ();

	/*
	scripterror ("Can't get the most recent address in the calendar because the calendar doesn't contain any elements before " + d + ".")}
	*/
	
	timedatestring (date, bsdate);
	
	parsedialogstring (BIGSTRING ("\x6e""Can't get the most recent address in the calendar because the calendar doesn't contain any elements before ^0."), bsdate, nil, nil, nil, bserror);
	
	langerrormessage (bserror);
	
	return (false);
	}/*mrcalendargetmostrecentaddress*/
	

static boolean mrcalendargetmostrecentaddressverb (hdltreenode hp1, tyvaluerecord *v) {

	tyaddress adrcalendar;
	tyvaluerecord val;
	short ctconsumed = 1;
	short ctpositional = 1;
	unsigned long date;
	
	if (!getaddressparam (hp1, 1, &val) || !getaddressvalue (val, &adrcalendar.ht, adrcalendar.bs))
		return (false);
	
	flnextparamislast = true;
		
	setdatevalue (timenow (), &val);

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x01""d"), &val))
		return (false);

	date = val.data.datevalue;
		
	return (mrcalendargetmostrecentaddress (adrcalendar, date, v));
	}/*mrcalendargetmostrecentaddressverb*/

/*
on getMostRecentDay (adrCalendar, d=clock.now ()) {
	local (adrDay, dmin = mainResponder.calendar.getFirstDay (adrCalendar));
	while (dmin <= d) {
		try {
			adrDay = mainResponder.calendar.getDayAddress (adrCalendar, d, flcreate:false);
			if defined (adrDay^) {
				local (day, month, year, hour, minute, second);
				date.get (d, @day, @month, @year, @hour, @minute, @second);
				d = date.set (day, month, year, 0, 0, 0);
				return (d)}};
		d = date.yesterday (d)};
	scripterror ("Can't get the most recent day in the calendar because the calendar doesn't contain any elements before " + d + ".")}
*/

static boolean mrcalendargetmostrecentdayverb (hdltreenode hp1, tyvaluerecord *v) {
	
	tyaddress adrcalendar, adr;
	tyvaluerecord val;
	short ctconsumed = 1;
	short ctpositional = 1;
	unsigned long mindate, date;
	const unsigned long oneday = 24 * 60 * 60;
	hdlhashnode hnode;
	hdlhashtable ht;
	bigstring bserror, bsdate;
	
	if (!getaddressparam (hp1, 1, &val) || !getaddressvalue (val, &adrcalendar.ht, adrcalendar.bs))
		return (false);
	
	flnextparamislast = true;
		
	setdatevalue (timenow (), &val);

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x01""d"), &val))
		return (false);

	date = val.data.datevalue;
	
	if (!langhashtablelookup (adrcalendar.ht, adrcalendar.bs, &val, &hnode) || !langexternalvaltotable (val, &ht, hnode) || !mrcalendargetfirstday (ht, &val))
		return (false);
	
	mindate = val.data.datevalue;
		
	disablelangerror ();
	
	while (mindate <= date) {
	
		if (mrcalendargetdayaddress (adrcalendar, date, false, 'tabl', &adr) && hashtablesymbolexists (adr.ht, adr.bs)) {
			
			short day, month, year, hour, minute, second;
			
			enablelangerror ();

			secondstodatetime (date, &day, &month, &year, &hour, &minute, &second);
			
			return (setdatevalue (datetimetoseconds (day, month, year, 0, 0, 0), v));
			}
	
		date -= oneday;
		}/*while*/

	enablelangerror ();

	/*
	scripterror ("Can't get the most recent day in the calendar because the calendar doesn't contain any elements before " + d + ".")}
	*/
	
	timedatestring (date, bsdate);
	
	parsedialogstring (BIGSTRING ("\x6a""Can't get the most recent day in the calendar because the calendar doesn't contain any elements before ^0."), bsdate, nil, nil, nil, bserror);
	
	langerrormessage (bserror);
	
	return (false);
	}/*mrcalendargetmostrecentdayverb*/

/*
on getNextAddress (adrCalendar, d=clock.now ()) {
	local (adrDay, dmax = mainResponder.calendar.getLastDay (adrCalendar));
	dmax = date (date.tomorrow (dmax) - 1);
	while (d <= dmax) {
		try {
			adrDay = mainResponder.calendar.getDayAddress (adrCalendar, d, flcreate:false);
			if defined (adrDay^) {
				return (adrDay)}};
		d = date.tomorrow (d)};
	scripterror ("Can't get the next address in the calendar because the calendar doesn't contain any elements after " + d + ".")}
*/

static boolean mrcalendargetnextaddress (tyaddress adrcalendar, unsigned long date, tyvaluerecord *v) {

	tyaddress adr;
	tyvaluerecord val;
	unsigned long maxdate;
	const unsigned long oneday = 24 * 60 * 60;
	hdlhashnode hnode;
	hdlhashtable ht;
	bigstring bserror, bsdate;
	
	if (!langhashtablelookup (adrcalendar.ht, adrcalendar.bs, &val, &hnode) || !langexternalvaltotable (val, &ht, hnode) || !mrcalendargetlastday (ht, &val))
		return (false);
	
	maxdate = val.data.datevalue + oneday - 1;
		
	disablelangerror ();
	
	while (date <= maxdate) {
	
		if (mrcalendargetdayaddress (adrcalendar, date, false, 'tabl', &adr) && hashtablesymbolexists (adr.ht, adr.bs)) {
			
			enablelangerror ();
			
			return (setaddressvalue (adr.ht, adr.bs, v));
			}
	
		date += oneday;
		}/*while*/

	enablelangerror ();

	/*
	scripterror ("Can't get the next address in the calendar because the calendar doesn't contain any elements after " + d + ".")}
	*/
	
	timedatestring (date, bsdate);
	
	parsedialogstring (BIGSTRING ("\x66""Can't get the next address in the calendar because the calendar doesn't contain any elements after ^0."), bsdate, nil, nil, nil, bserror);
	
	langerrormessage (bserror);
	
	return (false);
	}/*mrcalendargetnextaddress*/


static boolean mrcalendargetnextaddressverb (hdltreenode hp1, tyvaluerecord *v) {

	tyaddress adrcalendar;
	tyvaluerecord val;
	short ctconsumed = 1;
	short ctpositional = 1;
	unsigned long date;
	
	if (!getaddressparam (hp1, 1, &val) || !getaddressvalue (val, &adrcalendar.ht, adrcalendar.bs))
		return (false);
	
	flnextparamislast = true;
		
	setdatevalue (timenow (), &val);

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x01""d"), &val))
		return (false);

	date = val.data.datevalue;
		
	return (mrcalendargetnextaddress (adrcalendar, date, v));
	}/*mrcalendargetnextaddressverb*/

/*
on getNextDay (adrCalendar, d=clock.now ()) {
	local (adrDay, dmax = mainResponder.calendar.getLastDay (adrCalendar));
	dmax = date (date.tomorrow (dmax) - 1);
	while (d <= dmax) {
		try {
			adrDay = mainResponder.calendar.getDayAddress (adrCalendar, d, flcreate:false);
			if defined (adrDay^) {
				local (day, month, year, hour, minute, second);
				date.get (d, @day, @month, @year, @hour, @minute, @second);
				d = date.set (day, month, year, 0, 0, 0);
				return (d)}};
		d = date.tomorrow (d)};
	scripterror ("Can't get the next day in the calendar because the calendar doesn't contain any elements after " + d + ".")}
*/

static boolean mrcalendargetnextdayverb (hdltreenode hp1, tyvaluerecord *v) {

	tyaddress adrcalendar, adr;
	tyvaluerecord val;
	short ctconsumed = 1;
	short ctpositional = 1;
	unsigned long maxdate, date;
	const unsigned long oneday = 24 * 60 * 60;
	hdlhashnode hnode;
	hdlhashtable ht;
	bigstring bserror, bsdate;
	
	if (!getaddressparam (hp1, 1, &val) || !getaddressvalue (val, &adrcalendar.ht, adrcalendar.bs))
		return (false);
	
	flnextparamislast = true;
		
	setdatevalue (timenow (), &val);

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x01""d"), &val))
		return (false);

	date = val.data.datevalue;
	
	if (!langhashtablelookup (adrcalendar.ht, adrcalendar.bs, &val, &hnode) || !langexternalvaltotable (val, &ht, hnode) || !mrcalendargetlastday (ht, &val))
		return (false);
	
	maxdate = val.data.datevalue + oneday - 1;
		
	disablelangerror ();
	
	while (date <= maxdate) {
	
		if (mrcalendargetdayaddress (adrcalendar, date, false, 'tabl', &adr) && hashtablesymbolexists (adr.ht, adr.bs)) {
			
			short day, month, year, hour, minute, second;
			
			enablelangerror ();

			secondstodatetime (date, &day, &month, &year, &hour, &minute, &second);
			
			return (setdatevalue (datetimetoseconds (day, month, year, 0, 0, 0), v));
			}
	
		date += oneday;
		}/*while*/

	enablelangerror ();

	/*
	scripterror ("Can't get the next address in the calendar because the calendar doesn't contain any elements after " + d + ".")}
	*/
	
	timedatestring (date, bsdate);
	
	parsedialogstring (BIGSTRING ("\x62""Can't get the next day in the calendar because the calendar doesn't contain any elements after ^0."), bsdate, nil, nil, nil, bserror);
	
	langerrormessage (bserror);
	
	return (false);
	}/*mrcalendargetnextdayverb*/


/*
on navigate (adrCalendar, adrDay, flForwardInTime) {
	if not defined (adrDay^) { //consistency check
		scriptError ("Can't navigate the calendar structure because adrDay does not point to an existing calendar element.")};
	local (d = mainResponder.calendar.getAddressDay (adrDay));
	if flForwardInTime {
		return (mainResponder.calendar.getNextAddress (adrCalendar,date.tomorrow (d)))}
	else {
		return (mainResponder.calendar.getMostRecentAddress (adrCalendar, date.yesterday (d)))}}
*/

static boolean mrcalendarnavigateverb (hdltreenode hp1, tyvaluerecord *v) {
	
	tyvaluerecord val, valday;
	tyaddress adrcalendar, adrday;
	boolean flforwardintime;
	unsigned long date;
	const unsigned long oneday = 24 * 60 * 60;
	bigstring bs;
	Handle htext;

	if (!getaddressparam (hp1, 1, &val) || !getaddressvalue (val, &adrcalendar.ht, adrcalendar.bs))
		return (false);

	if (!getaddressparam (hp1, 2, &valday))
		return (false);

	flnextparamislast = true;
	
	if (!getbooleanvalue (hp1, 3, &flforwardintime))
		return (false);
	
	if (!getaddressvalue (valday, &adrday.ht, adrday.bs) || !hashtablesymbolexists (adrday.ht, adrday.bs)) { /*consistency check*/
		
		langerrormessage (BIGSTRING ("\x64""Can't navigate the calendar structure because adrDay does not point to an existing calendar element."));
		
		return (false);
		}
	
	if (!getaddresspath (valday, bs) || !newtexthandle (bs, &htext) || !mrcalendargetaddressday (htext, &date))
		return (false);
	
	if (flforwardintime)
		return (mrcalendargetnextaddress (adrcalendar, date + oneday, v));
	else
		return (mrcalendargetmostrecentaddress (adrcalendar, date - oneday, v));
	}/*mrcalendarnavigateverb*/
	
	
/*

on draw ( adrcalendar=nil, urlprefix="", colwidth=32, rowheight=22, tableborder=0, bgcolor=nil, monthYearTemplate="<font size=\"+2\" face=\"helvetica,arial\" color=\"black\"><b><center>***</center></b></font>", dayNameTemplate="<font size=\"-1\" color=\"gray\"><center>***</center></font>", dayTemplate="<font size=\"+0\" color=\"black\"><center><b>***</b></center></font>", curdate=clock.now ()) {

	local (bgcolorstring = "");
	if bgcolor != nil {
		bgcolorstring = "bgcolor=\"" + bgcolor + "\""};

	local (htmltext = "", indentlevel = 0);
	on add (s) {
		htmltext = htmltext + string.filledString ("\t", indentlevel) + s + "\r"};

	local (day, month, year, hour, minute, second);
	date.get (curdate, @day, @month, @year, @hour, @minute, @second);
	local (monthyear = date.monthToString (month) + " " + year);
	add ("<table cellspacing=\"0\" border=\"" + tableborder + "\">"); indentlevel++;
	add ("<tr>"); indentlevel++;
	add ("<td " + bgcolorstring + " colspan=\"7\">"); indentlevel++;
	add (string.replaceAll (string (monthYearTemplate), "***", monthyear));
	add ("</td>"); indentlevel--;
	add ("</tr>"); indentlevel--;
	bundle { //add days of week
		local (i);
		add ("<tr height=\"" + rowheight + "\">"); indentlevel++;
		for i = 1 to 7 {
			local (dayname = string.mid (date.dayOfWeekToString (i), 1, 3));
			add ("<td " + bgcolorstring + " width=\"" + colwidth + "\">"); indentlevel++;
			add (string.replaceAll (string (dayNameTemplate), "***", dayname));
			add ("</td>"); indentlevel--};
		add ("</tr>"); indentlevel--};

	bundle { //add the days of the month, 1, 2, 3, etc.
		on addday (daynum) {
			local (link = daynum);
			if daynum == day {
				link = "<b>" + daynum + "</b>"}
			else {
				try {
					local (yearstring = string (year));
					local (monthstring = string.padwithzeros (month, 2));
					local (daystring = string.padwithzeros (daynum, 2));
					local (adritem = @adrcalendar^.[yearstring].[monthstring].[daystring]);
					if defined (adritem^) {
						local (url = yearstring + "/" + monthstring + "/" + daystring);
						link = "<a href=\"" + urlprefix + url + "\">" + daynum + "</a>"}}};
			add ("<td " + bgcolorstring + " height=\"" + rowheight + "\">" + string.replaceAll (string (dayTemplate), "***", link) + "</td>")};
		local (startday = date.dayOfWeek (date.set (1, month, year, 0, 0, 0)), i, daynum = 1);
		local (daysinmonth = date.daysInMonth (curdate));
		add ("<tr>"); indentlevel++;
		if startday > 1 {
			add ("<td " + bgcolorstring + " colspan=\"" + (startday - 1) + "\">&nbsp;</td>")};
		for i = startday to 7 {
			addday (daynum++)};
		add ("</tr>"); indentlevel--;
		while daynum <= daysinmonth {
			add ("<tr>"); indentlevel++;
			for i = 1 to 7 {
				if daynum > daysinmonth {
					add ("<td " + bgcolorstring + " colspan=\"" + (8 - i) + "\">&nbsp;</td>");
					break};
				addday (daynum++)};
			add ("</tr>"); indentlevel--}};
	add ("</table>"); indentlevel--;
	return (htmltext)}

*/

#define str_calendartdwithinfo BIGSTRING ("\x1f""\t\t<td^0 width=\"^1\" height=\"^2\">")
#define str_calendartdspacer BIGSTRING ("\x21""\t\t<td^0 colspan=\"^1\">&nbsp;</td>\r")

#define str_calendartodaytemplate BIGSTRING ("\x09""<b>^0</b>")

#define dayofweektocolumn(i,f) ((((i) - ((f) - 1) + 6) % 7) + 1)
#define columntodayofweek(i,f) ((((i) + ((f) - 1) - 1) % 7) + 1)

#if 0

static boolean addhandle (handlestream *s, Handle h, short cttabs) {

	if (cttabs > 0) { /*indent*/

		bigstring bs;

		filledstring ('\t', cttabs, bs);

		if (!writehandlestreamstring (s, bs))
			return (false);
		}

	if (!writehandlestreamhandle (s, h))
		return (false);

	return (writehandlestreamchar (s, '\r'));
	}/*addhandle*/


static boolean addstring (handlestream *s, bigstring bstring, short cttabs) {

	if (cttabs > 0) { /*indent*/

		bigstring bs;

		filledstring ('\t', cttabs, bs);

		if (!writehandlestreamstring (s, bs))
			return (false);
		}

	if (!writehandlestreamstring (s, bstring))
		return (false);

	return (writehandlestreamchar (s, '\r'));
	}/*addstring*/

#endif


static boolean addclassattribute (handlestream *s, Handle hcssprefix, bigstring bsname) {

	if (hcssprefix == nil)
		return (true);
			
	if (!writehandlestreamstring (s, BIGSTRING ("\x08" " class=\"")))
		return (false);
	
	if (!writehandlestreamhandle (s, hcssprefix))
		return (false);

	if (!writehandlestreamstring (s, bsname))
		return (false);

	if (!writehandlestreamchar (s, '"'))
		return (false);

	return (true);
	}/*addclassattribute*/


static boolean fillintemplate (Handle h, Handle hinsert) {
	
	Handle hplaceholder;
	boolean fl;
	
	if (h == nil || hinsert == nil)
		return (true);
	
	if (!newtexthandle (BIGSTRING ("\x03""***"), &hplaceholder))
		return (false);
		
	fl = textfindreplace (hplaceholder, hinsert, h, true, false);	
	
	disposehandle (hplaceholder);
	
	return (fl);
	}/*fillintemplate*/

/*
on addday (daynum) {
	local (link = daynum);
	if daynum == day {
		link = "<b>" + daynum + "</b>"}
	else {
		try {
			local (yearstring = string (year));
			local (monthstring = string.padwithzeros (month, 2));
			local (daystring = string.padwithzeros (daynum, 2));
			local (adritem = @adrcalendar^.[yearstring].[monthstring].[daystring]);
			if defined (adritem^) {
				local (url = yearstring + "/" + monthstring + "/" + daystring);
				link = "<a href=\"" + urlprefix + url + "\">" + daynum + "</a>"}}};
	add ("<td " + bgcolorstring + " height=\"" + rowheight + "\">" + string.replaceAll (string (dayTemplate), "***", link) + "</td>")};
*/

static boolean
addday (
		handlestream *s,
		long daynum,
		boolean fltoday,
		hdlhashtable hmonthtable,
		Handle hurlprefix,
		Handle hbgcolorattrib,
		Handle hcolwidth,
		Handle hrowheight,
		Handle hday,
		Handle hcssprefix,
		boolean flfirstcolumn) {
#pragma unused (hcolwidth)

	Handle hlink= nil;
	bigstring daystring;
	bigstring paddeddaystring;
	boolean fldaylinked;

	numbertostring (daynum, daystring);

	copystring (daystring, paddeddaystring);

	while (stringlength (paddeddaystring) < 2)
		insertchar ('0', paddeddaystring);
	
	fldaylinked = (hmonthtable != nil) && hashtablesymbolexists (hmonthtable, paddeddaystring);

	if (!writehandlestreamstring (s, BIGSTRING ("\x05""\t\t<td")))
		return (false);
	
	if (fltoday) {
		if (!addclassattribute (s, hcssprefix, BIGSTRING ("\x12" "CalendarDayCurrent")))
			return (false);
		}
	else {		
		if (fldaylinked) {
			if (!addclassattribute (s, hcssprefix, BIGSTRING ("\x11" "CalendarDayLinked")))
				return (false);
			}
		else {
			if (!addclassattribute (s, hcssprefix, BIGSTRING ("\x0b" "CalendarDay")))
				return (false);
			}
		}

	if (0 < gethandlesize (hbgcolorattrib)) {

		if (!writehandlestreamhandle (s, hbgcolorattrib))
			return (false);
		}

	if (flfirstcolumn) {

		if (!writehandlestreamstring (s, BIGSTRING ("\x09"" height=\"")))
			return (false);

		if (!writehandlestreamhandle (s, hrowheight))
			return (false);

		if (!writehandlestreamchar (s, '\"'))
			return (false);	
		}

	if (!writehandlestreamchar (s, '>'))
		return (false);	

	if (fltoday) {

		bigstring daylink;

		copystring (str_calendartodaytemplate, daylink);

		parsedialogstring (daylink, daystring, nil, nil, nil, daylink);

		if (!newtexthandle (daylink, &hlink))
			goto error;
		}

	else {

		if (fldaylinked) {

			if (!newtexthandle (BIGSTRING ("\x09""<a href=\""), &hlink))
				goto error;

			if (!pushhandle (hurlprefix, hlink))
				goto error;

			if (!pushtexthandle (paddeddaystring, hlink))
				goto error;

			if (!pushtexthandle (BIGSTRING ("\x02""\">"), hlink))
				goto error;

			if (!pushtexthandle (daystring, hlink))
				goto error;

			if (!pushtexthandle (BIGSTRING ("\x04""</a>"), hlink))
				goto error;
			}

		else if (!newtexthandle (daystring, &hlink))
			goto error;
		}

	if (!copyhandle (hday, &hday)) /*don't modify original*/
		goto error;
	
	if (!fillintemplate (hday, hlink))
		goto error;

	if (!writehandlestreamhandle (s, hday))
		goto error;

	if (!writehandlestreamstring (s, BIGSTRING ("\x05""</td>")))
		goto error;

	if (!writehandlestreamchar (s, '\r'))
		goto error;

	disposehandle (hday);

	disposehandle (hlink);

	return (true);

error:

	disposehandle (hday);

	disposehandle (hlink);

	return (false);
	}

static boolean getmonthstring (long ix, tyvaluerecord vlist, Handle *h) {

	tyvaluerecord val;

	if (vlist.valuetype == novaluetype) {
		if (!datemonthtostring (ix, &val))
			return (false);
		}
	else {
		if (!langgetlistitem (&vlist, ix, nil, &val))
			return (false);
		}

	if (!coercetostring (&val))
		return (false);

	if (!copyhandle (val.data.stringvalue, h))
		return (false);

	return (true);
	}/*getmonthstring*/


static boolean getdayofweekstring (long ix, tyvaluerecord vlist, Handle *h) {

	tyvaluerecord val;

	if (vlist.valuetype == novaluetype) {

		if (!datedayofweektostring (ix, &val))
			return (false);

		if (!sethandlesize (val.data.stringvalue, 3))
			return (false);
		}
	else {
		if (!langgetlistitem (&vlist, ix, nil, &val))
			return (false);

		if (!coercetostring (&val))
			return (false);
		}

	if (!copyhandle (val.data.stringvalue, h))
		return (false);

	return (true);
	}/*getdayofweekstring*/


static boolean getmonthurl (unsigned long curdate, Handle hurldelimiter, Handle hurlprefix, hdlhashtable hcalendartable, hdlhashtable *hmonthtable) {

	/*other variable definitions*/
	hdlhashtable ht;
	short day, month, year, hour, minute, second;
	bigstring bsmonth, bsyear;

	/*try to locate monthly table in calendar*/
	/*it's not an error, if we can't find it - we'll just display a calendar without links*/

	secondstodatetime (curdate, &day, &month, &year, &hour, &minute, &second);

	numbertostring ((long) year, bsyear);

	numbertostring ((long) month, bsmonth);

	while (stringlength (bsmonth) < 2)
		insertchar ('0', bsmonth);

	*hmonthtable = nil;
	
	if (findnamedtable (hcalendartable, bsyear, &ht))
		findnamedtable (ht, bsmonth, hmonthtable);

	/*build urlprefix for this month:  "http://news.userland.com/" + "1999" + "/" + "06" + "/" */

	if (!pushtexthandle (bsyear, hurlprefix))
		return (false);

	if (!pushhandle (hurldelimiter, hurlprefix))
		return (false);

	if (!pushtexthandle (bsmonth, hurlprefix))
		return (false);

	if (!pushhandle (hurldelimiter, hurlprefix))
		return (false);
	
	return (true);
	}/*getmonthurl*/


static boolean openoutertable (handlestream *s, Handle hcssprefix, Handle htableborder) {

	if (!writehandlestreamstring (s, BIGSTRING ("\x06" "<table")))
		return (false);
	
	if (!addclassattribute (s, hcssprefix, BIGSTRING ("\x0d" "CalendarTable")))
		return (false);

	if (!writehandlestreamstring (s, BIGSTRING ("\x19" " cellspacing=\"0\" border=\"")))
		return (false);

	if (!writehandlestreamhandle (s, htableborder))
		return (false);

	if (!writehandlestreamstring (s, BIGSTRING ("\x03" "\">\r")))
		return (false);

	return (true);
	}/*addoutertable*/


static boolean addmonthyearrow (handlestream *s, tyvaluerecord vmonthlist, unsigned long curdate, Handle hcssprefix, Handle hbgcolor, Handle hmonthyeartemplate) {

	short day, month, year, hour, minute, second;
	bigstring bsyear;
	Handle h;
	boolean fl;
	
	secondstodatetime (curdate, &day, &month, &year, &hour, &minute, &second);
	
	numbertostring ((long) year, bsyear);
	
	if (!getmonthstring ((long) month, vmonthlist, &h))
		return (false);
 
	fl = enlargehandle (h, sizeof(char), " ") && pushtexthandle (bsyear, h) && fillintemplate (hmonthyeartemplate, h);
	
	disposehandle (h);
	
	if (!fl)
		return (false);
	
	if (!writehandlestreamstring (s, BIGSTRING ("\x04" "\t<tr")))
		return (false);

	if (!addclassattribute (s, hcssprefix, BIGSTRING ("\x14" "CalendarMonthYearRow")))
		return (false);

	if (!writehandlestreamstring (s, BIGSTRING ("\x04" "><td")))
		return (false);

	if (!writehandlestreamhandle (s, hbgcolor))
		return (false);

	if (!writehandlestreamstring (s, BIGSTRING ("\x0d" " colspan=\"7\">")))
		return (false);

	if (!writehandlestreamhandle (s, hmonthyeartemplate))
		return (false);

	if (!writehandlestreamstring (s, BIGSTRING ("\x0b" "</td></tr>\r")))
		return (false);
	
	return (true);
	}/*addmonthyearrow*/

/*
<tr class="[hcssprefix]CalendarDayNameRow">
	<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Sun")]</td>
	<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Mon")]</td>
	<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Tue")]</td>
	<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Wed")]</td>
	<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Thu")]</td>
	<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Fri")]</td>
	<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Sat")]</td>
	</tr>
*/

static boolean adddaynamesrow (handlestream *s, Handle hcssprefix, Handle hbgcolor, Handle hcolwidth, Handle hrowheight, Handle hdaynametemplate, long firstdayofweek, tyvaluerecord vdayofweeklist) {

	Handle htd = nil;
	long i;

	if (!writehandlestreamstring (s, BIGSTRING ("\x04" "\t<tr")))
		goto error;

	if (!addclassattribute (s, hcssprefix, BIGSTRING ("\x12" "CalendarDayNameRow")))
		goto error;

	if (!writehandlestreamstring (s, BIGSTRING ("\x02" ">\r")))
		goto error;

	if (!newtexthandle (str_calendartdwithinfo, &htd))
		goto error;

	if (!parsedialoghandle (htd, hbgcolor, hcolwidth, hrowheight, nil))
		goto error;

	for (i = 1; i <= 7; i++) {

		Handle hdayname = nil;
		Handle hday = nil;
		boolean fl = true;
		
		if (!writehandlestreamhandle (s, htd))
			goto error;

		fl = fl && copyhandle (hdaynametemplate, &hday);

		fl = fl && getdayofweekstring (columntodayofweek (i, firstdayofweek), vdayofweeklist, &hdayname);

		fl = fl && fillintemplate (hday, hdayname);
		
		fl = fl && writehandlestreamhandle (s, hday);

		disposehandle (hdayname);
		
		disposehandle (hday);

		if (!fl)
			goto error;
			
		if (!writehandlestreamstring (s, BIGSTRING ("\x06" "</td>\r")))
			goto error;
		}/*for*/	

	if (!writehandlestreamstring (s, BIGSTRING ("\x08" "\t\t</tr>\r")))
		goto error;
	
	disposehandle (htd);

	return (true);

error:
	disposehandle (htd);
	
	return (false);
	}/*adddaynamesrow*/

/*
<tr class="[hcssprefix]CalendarDayRow">
	<td colspan="1">&nbsp;</td>
	<td class="[hcssprefix]CalendarDayLinked" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/01">1</a>")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "2")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/03">3</a>")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/04">4</a>")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "5")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "6")]</td>
	</tr>
<tr class="[hcssprefix]CalendarDayRow">
	<td class="[hcssprefix]CalendarDay" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "7")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/08">8</a>")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/09">9</a>")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "10")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "11")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "12")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/13">13</a>")]</td>
	</tr>
<tr class="[hcssprefix]CalendarDayRow">
	<td class="[hcssprefix]CalendarDayLinked" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/14">14</a>")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/15">15</a>")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/16">16</a>")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/17">17</a>")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/18">18</a>")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "19")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/20">20</a>")]</td>
	</tr>
<tr class="[hcssprefix]CalendarDayRow">
	<td class="[hcssprefix]CalendarDayLinked" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/21">21</a>")]</td>
	<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/22">22</a>")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "23")]</td>
	<td class="[hcssprefix]CalendarDayCurrent">[string.replaceAll (daytemplate, "***", "24")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "25")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "26")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "27")]</td>
	</tr>
<tr class="[hcssprefix]CalendarDayRow">
	<td class="[hcssprefix]CalendarDay" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "28")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "29")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "30")]</td>
	<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "31")]</td>
	<td colspan="3">&nbsp;</td>
	</tr>
*/

static boolean adddayrows (handlestream *s, unsigned long curdate, long firstdayofweek, hdlhashtable hmonthtable,
							Handle hurlprefix,  Handle hbgcolor, Handle hcolwidth, Handle hrowheight, Handle hdaytemplate, Handle hcssprefix) {

	short day, month, year, hour, minute, second;
	short dayofweek, maxdays, firstcolumn;
	long daynum = 1;
	short i;

	/*compute dayofweek, firstcolumn, and maxdays*/
	
	secondstodatetime (curdate, &day, &month, &year, &hour, &minute, &second);

	secondstodayofweek (datetimetoseconds (1, month, year, 0, 0, 0), &dayofweek);

	firstcolumn = dayofweektocolumn (dayofweek, firstdayofweek);

	maxdays = daysInMonth (month, year);
	
	/*begin writing table*/

	if (!writehandlestreamstring (s, BIGSTRING ("\x04" "\t<tr")))
		return (false);

	if (!addclassattribute (s, hcssprefix, BIGSTRING ("\x0e" "CalendarDayRow")))
		return (false);

	if (!writehandlestreamstring (s, BIGSTRING ("\x02" ">\r")))
		return (false);

	if (firstcolumn > 1) {

		bigstring bscolspan;
		Handle hcolspan = nil;
		Handle hspacer = nil;
		boolean fl = true;

		numbertostring (firstcolumn - 1, bscolspan);

		fl = fl && newtexthandle (str_calendartdspacer, &hspacer);

		fl = fl && newtexthandle (bscolspan, &hcolspan);

		fl = fl && parsedialoghandle (hspacer, hbgcolor, hcolspan, nil, nil);
		
		fl = fl && writehandlestreamhandle (s, hspacer);

		disposehandle (hcolspan);
		
		disposehandle (hspacer);
		
		if (!fl)
			return (false);
		}

	for (i = firstcolumn; i <= 7; i++) {

		if (!addday (s, daynum, daynum == day, hmonthtable, hurlprefix,
					hbgcolor, hcolwidth, hrowheight, hdaytemplate, hcssprefix, (i == firstcolumn)))
			return (false);

		daynum++;
		}

	if (!writehandlestreamstring (s, BIGSTRING ("\x08" "\t\t</tr>\r")))
		return (false);

	/*
	while daynum <= daysinmonth {
		add ("<tr>"); indentlevel++;
		for i = 1 to 7 {
			if daynum > daysinmonth {
				add ("<td " + bgcolorstring + " colspan=\"" + (8 - i) + "\">&nbsp;</td>");
				break};
			addday (daynum++)};
		add ("</tr>"); indentlevel--};
	*/	

	while (daynum <= maxdays) {

		if (!writehandlestreamstring (s, BIGSTRING ("\x04" "\t<tr")))
			return (false);

		if (!addclassattribute (s, hcssprefix, BIGSTRING ("\x0e" "CalendarDayRow")))
			return (false);

		if (!writehandlestreamstring (s, BIGSTRING ("\x02" ">\r")))
			return (false);

		for (i = 1; i <= 7; i++) {

			if (daynum > maxdays) {

				bigstring bscolspan;
				Handle hcolspan = nil;
				Handle hspacer = nil;
				boolean fl = true;

				numbertostring (8 - i, bscolspan);

				fl = fl && newtexthandle (str_calendartdspacer, &hspacer);

				fl = fl && newtexthandle (bscolspan, &hcolspan);

				fl = fl && parsedialoghandle (hspacer, hbgcolor, hcolspan, nil, nil);
				
				fl = fl && writehandlestreamhandle (s, hspacer);

				disposehandle (hcolspan);
				
				disposehandle (hspacer);
				
				if (!fl)
					return (false);

				break;
				}

			if (!addday (s, daynum, daynum == day, hmonthtable, hurlprefix,
							hbgcolor, hcolwidth, hrowheight, hdaytemplate, hcssprefix, (i == 1)))
				return (false);

			daynum++;
			}

		if (!writehandlestreamstring (s, BIGSTRING ("\x08" "\t\t</tr>\r")))
			return (false);
		}
	
	return (true);
	}/*adddayrows*/


static boolean htmlcalendardraw (handlestream *s,
									hdlhashtable hcalendartable,
									Handle hurlprefix,
									Handle hcolwidth,
									Handle hrowheight,
									Handle htableborder,
									Handle hbgcolor,
									Handle hmonthyeartemplate,
									Handle hdaynametemplate,
									Handle hdaytemplate,
									unsigned long curdate,
									Handle hurldelimiter,
									long firstdayofweek,
									tyvaluerecord vmonthlist,
									tyvaluerecord vdayofweeklist,
									Handle hcssprefix) {

	/*other variable definitions*/
	hdlhashtable hmonthtable;

	/*try to locate monthly table in calendar*/
	/*it's not an error, if we can't find it - we'll just display a calendar without links*/
	
	if (!getmonthurl (curdate, hurldelimiter, hurlprefix, hcalendartable, &hmonthtable))
		return (false);
	
	/*
	if bgcolor != "" {
		bgcolor = " bgcolor=\"" + bgcolor + "\""};
	*/

	if (gethandlesize (hbgcolor) > 0) { /*it's not empty*/

		if (!insertinhandle (hbgcolor, 0, " bgcolor=\"", 10))
			return (false);

		if (!enlargehandle (hbgcolor, 1, "\""))
			return (false);
		}
	
	/*
	<table class="[hcssprefix]CalendarTable" cellspacing="0" border="[htableborder]">
	*/
	
	if (!openoutertable (s, hcssprefix, htableborder))
		return (false);
	
	/*
		<tr class="[hcssprefix]CalendarMonthYearRow"><td[hbgcolor] colspan="7">[string.replaceAll (hmonthYearTemplate, "***", date.monthToString (month) + " " + year)]</td></tr>
	*/

	if (!addmonthyearrow (s, vmonthlist, curdate, hcssprefix, hbgcolor, hmonthyeartemplate))
		return (false);
	
	/*
		<tr class="[hcssprefix]CalendarDayNameRow">
			<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Sun")]</td>
			<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Mon")]</td>
			<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Tue")]</td>
			<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Wed")]</td>
			<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Thu")]</td>
			<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Fri")]</td>
			<td[hbgcolor] width="[hcolwidth]" height="[hrowheight]">[string.replaceAll (string (dayNameTemplate), "***", "Sat")]</td>
			</tr>
	*/
	
	if (!adddaynamesrow (s, hcssprefix, hbgcolor, hcolwidth, hrowheight, hdaynametemplate, firstdayofweek, vdayofweeklist))
		return (false);

	/*
		<tr class="[hcssprefix]CalendarDayRow">
			<td colspan="1">&nbsp;</td>
			<td class="[hcssprefix]CalendarDayLinked" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/01">1</a>")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "2")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/03">3</a>")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/04">4</a>")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "5")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "6")]</td>
			</tr>
		<tr class="[hcssprefix]CalendarDayRow">
			<td class="[hcssprefix]CalendarDay" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "7")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/08">8</a>")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/09">9</a>")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "10")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "11")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "12")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/13">13</a>")]</td>
			</tr>
		<tr class="[hcssprefix]CalendarDayRow">
			<td class="[hcssprefix]CalendarDayLinked" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/14">14</a>")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/15">15</a>")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/16">16</a>")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/17">17</a>")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/18">18</a>")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "19")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/20">20</a>")]</td>
			</tr>
		<tr class="[hcssprefix]CalendarDayRow">
			<td class="[hcssprefix]CalendarDayLinked" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/21">21</a>")]</td>
			<td class="[hcssprefix]CalendarDayLinked">[string.replaceAll (daytemplate, "***", "<a href="[hurlprefix]2000/05/22">22</a>")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "23")]</td>
			<td class="[hcssprefix]CalendarDayCurrent">[string.replaceAll (daytemplate, "***", "24")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "25")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "26")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "27")]</td>
			</tr>
		<tr class="[hcssprefix]CalendarDayRow">
			<td class="[hcssprefix]CalendarDay" height="[hrowheight]">[string.replaceAll (daytemplate, "***", "28")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "29")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "30")]</td>
			<td class="[hcssprefix]CalendarDay">[string.replaceAll (daytemplate, "***", "31")]</td>
			<td colspan="3">&nbsp;</td>
			</tr>
	*/

	if (!adddayrows (s, curdate, firstdayofweek, hmonthtable, hurlprefix, hbgcolor, hcolwidth, hrowheight, hdaytemplate, hcssprefix))
		return (false);

	/*
		</table>
	*/

	if (!writehandlestreamstring (s, BIGSTRING ("\x0A""\t</table>\r")))
		return (false);

	return (true);
	}/*mrcalendardraw*/


static boolean htmlcalendardrawverb (hdltreenode hp1, tyvaluerecord *v) {

	/*
		on draw (adrcalendar=nil,
				urlprefix="",
				colwidth=32,
				rowheight=22,
				tableborder=0,
				bgcolor=nil,
				monthYearTemplate="<font size=\"+2\" face=\"helvetica,arial\" color=\"black\"><b><center>***</center></b></font>",
				dayNameTemplate="<font size=\"-1\" color=\"gray\"><center>***</center></font>",
				dayTemplate="<font size=\"+0\" color=\"black\"><center><b>***</b></center></font>",
				curdate=clock.now (),
				urldelimiter="/",
				firstDayOfWeek=1,
				adrMonthNamesList=nil,
				adrAbbrevDayOfWeekNamesList=nil,
				cssPrefix=nil)
	*/

	/*verb params*/
	hdlhashtable hcalendartable; 
	Handle hcolwidth, hrowheight, htableborder; /*readonlytext*/
	Handle hdaynametemplate, hdaytemplate, hurldelimiter; /*readonlytext*/
	Handle hmonthyeartemplate = nil; /*exempt*/
	Handle hurlprefix = nil; /*exempt*/
	Handle hbgcolor = nil; /*exempt*/
	Handle hcssprefix = nil;
	unsigned long curdate; /*date*/
	long firstdayofweek; /*first day of the week, 1 == Sunday*/
	tyvaluerecord vmonthlist, vdayofweeklist, vcssprefix;

	/*other variable definitions*/
	handlestream sreturn;
	boolean fl = false;

	/*init*/

	openhandlestream (nil, &sreturn);

	initvalue (&vmonthlist, novaluetype);

	initvalue (&vdayofweeklist, novaluetype);
	
	initvalue (&vcssprefix, novaluetype);

	/*get read-only parameters*/

	if (!gettablevalue (hp1, 1, &hcalendartable))
		return (false);

	if (!getreadonlytextvalue (hp1, 3, &hcolwidth))
		return (false);

	if (!getreadonlytextvalue (hp1, 4, &hrowheight))
		return (false);

	if (!getreadonlytextvalue (hp1, 5, &htableborder))
		return (false);

	if (!getreadonlytextvalue (hp1, 8, &hdaynametemplate))
		return (false);

	if (!getreadonlytextvalue (hp1, 9, &hdaytemplate))
		return (false);

	if (!getdatevalue (hp1, 10, &curdate))
		return (false);

	if (!getreadonlytextvalue (hp1, 11, &hurldelimiter))
		return (false);

	if (!getlongvalue (hp1, 12, &firstdayofweek))
		return (false);

	if (!getparamvalue (hp1, 13, &vmonthlist))
		return (false);

	if (vmonthlist.valuetype != novaluetype) {

		if (!coercetoaddress (&vmonthlist))
			return (false);

		if (!followaddressvalue (&vmonthlist))
			return (false);

		if (!copyvaluerecord (vmonthlist, &vmonthlist))
			return (false);

		if (!coercetolist (&vmonthlist, listvaluetype))
			return (false);
		}

	if (!getparamvalue (hp1, 14, &vdayofweeklist))
		return (false);

	if (vdayofweeklist.valuetype != novaluetype) {

		if (!coercetoaddress (&vdayofweeklist))
			return (false);

		if (!followaddressvalue (&vdayofweeklist))
			return (false);

		if (!copyvaluerecord (vdayofweeklist, &vdayofweeklist))
			return (false);

		if (!coercetolist (&vdayofweeklist, listvaluetype))
			return (false);
		}

	flnextparamislast = true;

	if (!getparamvalue (hp1, 15, &vcssprefix))
		return (false);
	
	if (vcssprefix.valuetype != novaluetype) {

		if (!coercetostring (&vcssprefix))
			return (false);

		hcssprefix = vcssprefix.data.stringvalue;
		}	

	/*get exempt parameters*/
	
	if (!getexempttextvalue (hp1, 2, &hurlprefix))
		goto exit;

	if (!getexempttextvalue (hp1, 6, &hbgcolor))
		goto exit;

	if (!getexempttextvalue (hp1, 7, &hmonthyeartemplate))
		goto exit;
	
	/*do our thing*/
	
	fl = htmlcalendardraw (&sreturn, hcalendartable, hurlprefix, hcolwidth, hrowheight, htableborder, hbgcolor,
									hmonthyeartemplate, hdaynametemplate, hdaytemplate, curdate,
									hurldelimiter, firstdayofweek, vmonthlist, vdayofweeklist, hcssprefix);

	/* return (htmltext) */

exit:
	disposehandle (hurlprefix);

	disposehandle (hbgcolor);

	disposehandle (hmonthyeartemplate);
	
	if (!fl) {
		disposehandlestream (&sreturn); /*checks for nil handle*/
		return (false);
		}

	return (setheapvalue (closehandlestream (&sreturn), stringvaluetype, v));
	}/*htmlcalendardrawverb*/



#pragma mark === mainResponder: Safe macros ===

/*
	About the legalMacros table:

	1) If an item in that table is a boolean, and it's false, it's not legal.

	2) If an item in that table is a boolean, and it's true, then it supports indentifiers only, no parameters.

	3) If an item in that table is a subtable, but it has no flLegal sub-item, then it's not legal.

	4) If an item in that table is a subtable, and it has an flLegal sub-item, and flLegal is false, then it's not legal.

	5) If an item in that table is a subtable, and flLegal is true, then it's legal.

	6) If an item in that table is a subtable, and it has an flParams sub-item, and flParams is true, then it must not be an identifier, it takes zero or more params. If flParams is false, then it must be an idenfifier.

	It works very much like config.mainResponder.prefs.legalTags.

	Examples (where "now" is the name of the macro):

		now   true  //legal identifier, illegal with params

		now   false //illegal

		now
		   flLegal    false   //illegal

		now
		   flLegal    true    //legal

		now                   
		   flLegal    true    //legal as identifier, illegal with params

		now
		   flLegal    true
		   flParams   true    //legal with params, illegal as identifier

		now
		   flLegal    true
		   flParams   false   //legal as identifier, illegal with params
*/

#define modern_safe_macros 1

#ifdef modern_safe_macros

static boolean macrostablelookup (hdlhashtable ht, bigstring bs, boolean *ptrflparams);

static boolean getidentifierstring (hdltreenode hnode, bigstring bs);

static boolean issafeconstop (hdltreenode hnode);

static boolean issafeidentifier (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafefieldop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafefunctionop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafeifop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafetryop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafebinaryop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafeunaryop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafearrayop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafelistop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issaferecordop (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafestatement (hdltreenode hnode, hdlhashtable hmacrostable);

static boolean issafestatementlist (hdltreenode hnode, hdlhashtable hmacrostable);


static boolean macrostablelookup (hdlhashtable ht, bigstring bs, boolean *ptrflparams) {

	/*
	6.2a9 AR: Look up identifier in legalmacros table, return false if it's illegal
	*/
	
	tyvaluerecord val;
	hdlhashnode hdummy;
	
	*ptrflparams = false;

	if (!hashtablelookup (ht, bs, &val, &hdummy))
		return (false); /*must be illegal*/

	if (langexternalvaltotable (val, &ht, hdummy)) {

		boolean fl, fllegal;

		disablelangerror ();

		fl = langlookupbooleanvalue (ht, STR_P_FLLEGAL, &fllegal);
		
		langlookupbooleanvalue (ht, STR_P_FLPARAMS, ptrflparams);

		enablelangerror ();

		if (!fl || !fllegal)
			return (false); /*must be illegal*/
		}
	
	else {

		if (val.valuetype != booleanvaluetype) /*6.1b7 AR: coercion*/
			if (!copyvaluerecord (val, &val) || !coercetoboolean (&val))
				return (false);

		if (!val.data.flvalue)
			return (false); /*must be illegal*/
		}
	
	return (true);
	}/*macrostablelookup*/


static boolean getidentifierstring (hdltreenode hnode, bigstring bs) {

	setemptystring (bs);
	
	switch ((**hnode).nodetype) {
	
		case identifierop: {
			tyvaluerecord val = (**hnode).nodeval;

			if (val.valuetype != stringvaluetype)
				return (false); /*6.2b13 AR: don't crash*/

			pullstringvalue (&(**hnode).nodeval, bs);
			break;
			}
		
		case dotop: {
			bigstring bs1, bs2;
			
			if (!getidentifierstring ((**hnode).param1, bs1))
				return (false);
			
			if (!getidentifierstring ((**hnode).param2, bs2))
				return (false);
			
			pushstring (bs1, bs);
			
			pushchar ('.', bs);
			
			pushstring (bs2, bs);			
			break;
			}
		
		default:
			return (false);	
		}/*switch*/
	
	return (true);
	}/*getidentifierstring*/


static boolean issafeconstop (hdltreenode hnode) {
	
	tyvaluetype valtype = (**hnode).nodeval.valuetype;

	assert ((**hnode).nodetype == constop);

	return (valtype == stringvaluetype || valtype == longvaluetype || valtype == doublevaluetype || valtype == booleanvaluetype || valtype == charvaluetype);
	}/*issafeconstop*/


static boolean issafeidentifier (hdltreenode hnode, hdlhashtable hmacrostable) {

	bigstring bs;
	boolean flparams;

	//assert ((**hnode).nodetype == identifierop || (**hnode).nodetype == dotop); /*why would you call us if it's not?*/
	
	if (!getidentifierstring (hnode, bs))
		return (false);
	
	if (!macrostablelookup (hmacrostable, bs, &flparams))
		return (false);
	
	return (!flparams);
	}/*issafeidentifier*/


static boolean issafefieldop (hdltreenode hnode, hdlhashtable hmacrostable) {

	//assert ((**hnode).nodetype == fieldop); /*why would you call us if it's not?*/
		
	switch ((**(**hnode).param1).nodetype) {

		case identifierop:
			return (true);
		
		case constop:
			return (issafeconstop (hnode));

		default:
			return (false);
		}/*switch*/

	if (!issafestatement ((**hnode).param2, hmacrostable)) /*named parameter*/
		return (false);
			
	return (!(**hnode).param3 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issafefieldop*/


typedef struct {
	short token;
	char *str;
	} tytokeninfo;

static boolean findtokencallback (hdlhashnode hn, ptrvoid p) {

	tytokeninfo* ptrinfo = (tytokeninfo*) p;

	if ((**hn).val.valuetype == tokenvaluetype)
		if ((**hn).val.data.tokenvalue == (*ptrinfo).token) {

			copystring ((**hn).hashkey, BIGSTRING ((*ptrinfo).str));

			return (false);
			}

	return (true); /*keep searching*/
	}/*findtokencallback*/

static boolean issafefunctionop (hdltreenode hnode, hdlhashtable hmacrostable) {

	bigstring bs;
	boolean flparams;
	register hdltreenode nomad = (**hnode).param1;

	//assert ((**hnode).nodetype == functionop); /*why would you call us if it's not?*/
	
	/*make sure the functionname is legal*/

	setemptystring (bs);

	if (((**nomad).nodetype == identifierop) && ((**nomad).nodeval.valuetype == tokenvaluetype)) {
		/*
		6.2b13 AR: special case for optimization in pushfunctioncall in langtree.c
		*/
		tytokeninfo info;

		info.token = (**nomad).nodeval.data.tokenvalue;
		info.str = (char *) bs;

		hashtablevisit (hbuiltinfunctions, &findtokencallback, (ptrvoid) &info);
		}
	else if (!getidentifierstring (nomad, bs))
		return (false);
	
	if (isemptystring (bs))
		return (false);

	if (!macrostablelookup (hmacrostable, bs, &flparams))
		return (false);
	
	if (!flparams)
		return (false);
	
	/*chain thru the linked list of params and make sure they are legal too*/
		
	nomad = (**hnode).param2;

	while (nomad != nil) {
	
		if ((**nomad).nodetype == fieldop) {
			
			if (!issafefieldop (nomad, hmacrostable))
				return (false);
			}
		
		else if (!issafestatement (nomad, hmacrostable))
			return (false);
					
		nomad = (**nomad).link;
		}/*while*/
		
	return (!(**hnode).param3 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issafefunctionop*/


static boolean issafereturnop (hdltreenode hnode, hdlhashtable hmacrostable) {

	//assert ((**hnode).nodetype == returnop); /*why would you call us if it's not?*/
	
	if (!issafestatement ((**hnode).param1, hmacrostable))
		return (false);
	
	return (!(**hnode).param2 && !(**hnode).param3 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issafereturnop*/
	

static boolean issafeifop (hdltreenode hnode, hdlhashtable hmacrostable) {

	//assert ((**hnode).nodetype == ifop); /*why would you call us if it's not?*/
	
	if (!issafestatement ((**hnode).param1, hmacrostable)) /*condition*/
		return (false);

	if (!issafestatementlist ((**hnode).param2, hmacrostable)) /*if-block*/
		return (false);
	
	if (!issafestatementlist ((**hnode).param3, hmacrostable)) /*else-block*/
		return (false);
	
	return (!(**hnode).param4); /*params are supposed to be nil*/
	}/*issafeifop*/


static boolean issafetryop (hdltreenode hnode, hdlhashtable hmacrostable) {

	//assert ((**hnode).nodetype == tryop); /*why would you call us if it's not?*/
	
	if (!issafestatementlist ((**hnode).param2, hmacrostable)) /*try-block*/
		return (false);
	
	if (!issafestatementlist ((**hnode).param3, hmacrostable)) /*else-block*/
		return (false);
	
	return (!(**hnode).param1 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issafetryop*/
	

static boolean issafebinaryop (hdltreenode hnode, hdlhashtable hmacrostable) {

	//assert ((**hnode).ctparams <= 2); /*it's 1 for ororop and andandop*/
	
	if (!issafestatement ((**hnode).param1, hmacrostable)) /*first operand*/
		return (false);
	
	if (!issafestatement ((**hnode).param2, hmacrostable)) /*second operand*/
		return (false);
	
	return (!(**hnode).param3 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issafebinaryop*/


static boolean issafeunaryop (hdltreenode hnode, hdlhashtable hmacrostable) {

	//assert ((**hnode).nodetype == notop || (**hnode).nodetype == unaryop);
	
	if (!issafestatement ((**hnode).param1, hmacrostable)) /*operand*/
		return (false);
	
	return (!(**hnode).param2 && !(**hnode).param3 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issafeunaryop*/
	
	
static boolean issafearrayop (hdltreenode hnode, hdlhashtable hmacrostable) {

	if (!issafestatementlist ((**hnode).param1, hmacrostable)) /*not sure if it can be a linked list, but better safe than sorry*/
		return (false);

	if (!issafestatement ((**hnode).param2, hmacrostable)) /*index*/
		return (false);
	
	return (!(**hnode).param3 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issafearrayop*/
	
	
static boolean issafelistop (hdltreenode hnode, hdlhashtable hmacrostable) {

	//assert ((**hnode).nodetype == listop);
	
	if (!issafestatementlist ((**hnode).param1, hmacrostable))
		return (false);

	return (!(**hnode).param2 && !(**hnode).param3 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issafelistop*/
	
	
static boolean issaferecordop (hdltreenode hnode, hdlhashtable hmacrostable) {

	register hdltreenode nomad = (**hnode).param1;

	//assert ((**nomad).nodetype == recordop); /*why would you call us if it's not?*/
	
	/*chain thru the linked list of record elements and make sure they are legal*/

	while (nomad != nil) {
	
		if (!issafefieldop (nomad, hmacrostable))
			return (false);
					
		nomad = (**nomad).link;
		}/*while*/

	return (!(**hnode).param2 && !(**hnode).param3 && !(**hnode).param4); /*params are supposed to be nil*/
	}/*issaferecordop*/
	
	
static boolean issafestatement (hdltreenode hnode, hdlhashtable hmacrostable) {

	if (hnode == nil)
		return (false); //something is clearly wrong
	
	switch ((**hnode).nodetype) {
		
		case constop:
			return (issafeconstop (hnode));

		case dotop:
		case identifierop:
			return (issafeidentifier (hnode, hmacrostable));
			
		case arrayop:
			return (issafearrayop (hnode, hmacrostable));
		
		case functionop:
			return (issafefunctionop (hnode, hmacrostable));
		
		case returnop:
			return (issafereturnop (hnode, hmacrostable));
		
		case ifop:
			return (issafeifop (hnode, hmacrostable));

		case tryop:
			return (issafetryop (hnode, hmacrostable));

		case unaryop:
		case notop:
			return (issafeunaryop (hnode, hmacrostable));
		
		case listop:
			return (issafelistop (hnode, hmacrostable));
			
		case recordop:
			return (issaferecordop (hnode, hmacrostable));

		case addop:
		case subtractop:
		case multiplyop:
		case divideop:
		case modop:
		case EQop:
		case NEop:
		case GTop:
		case LTop:
		case GEop:
		case LEop:
		case beginswithop:
		case endswithop:
		case containsop:
		case ororop:
		case andandop:
			return (issafebinaryop (hnode, hmacrostable));

		case noop:
			return (true); /*noop's are very agreeable*/
		
		default:
			return (false);
		}/*switch*/
		
	}/*issafestatement*/
	

static boolean issafestatementlist (hdltreenode hnode, hdlhashtable hmacrostable) {
	
	/*
	6.2a9 AR: Chain thru the linked list of statements
	*/
	
	register hdltreenode nomad = hnode;
	
	while (nomad != nil) {
	
		if (!issafestatement (nomad, hmacrostable))
			return (false);
					
		nomad = (**nomad).link;
		}/*while*/
	
	return (true);
	}/*issafestatementlist*/


static boolean issafemacro (Handle h, long ixstart, long len, hdlhashtable hmacrostable) {

	/*
	6.2a9 AR: Run the text thru the UserTalk parser. Walk the resulting code tree
				to determine whether this is a legal macro.
	*/
	
	hdltreenode hcode;
	Handle htext;
	boolean fl;
	unsigned long savescanlines; /*number of lines that have been scanned, for error reporting*/
	unsigned short savescanchars; /*number of chars passed over on current line, for error reporting*/
	bigstring bserror;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;

	if (!loadfromhandletohandle (h, &ixstart, len, true, &htext))
		return (false); /*probably invalid boundaries*/
	
	langtraperrors (bserror, &savecallback, &saverefcon);
	
	savescanlines = ctscanlines;
	
	savescanchars = ctscanchars;
	
	fl = langbuildtree (htext, false, &hcode); /*disposes of htext*/
			
	ctscanlines = savescanlines;
	
	ctscanchars = savescanchars;
	
	languntraperrors (savecallback, saverefcon, false);
	
	fllangerror = false;
	
	if (!fl)
		return (false); /*probably a syntax error, not legal*/
	
	if (((**hcode).link != nil) || ((**hcode).nodetype != moduleop))
		goto error;
	
	if (((**hcode).param2 != nil) || ((**hcode).param3 != nil) || ((**hcode).param4 != nil))
		goto error;
	
	if (!issafestatementlist ((**hcode).param1, hmacrostable))
		goto error;
	
	langdisposetree (hcode);
	
	return (true);

error:
	langdisposetree (hcode);
	
	return (false);
	}/*issafemacro*/

#else

static boolean issafemacro (Handle h, long ixstart, long len, hdlhashtable hmacrostable) {

	long ix;
	long ixend = ixstart + len;
	long lenidentifier;
	bigstring bsidentifier;
	boolean flparams = false;
	tyvaluerecord val;
	hdlhashtable ht;
	hdlhashnode hnode;

	/*trim whitespace*/

	while (ixstart < ixend && whitespacechar((*h)[ixstart]))
		++ixstart;

	while (ixstart < ixend && whitespacechar((*h)[ixend-1]))
		--ixend;

	if (ixstart == ixend)
		return (true); /*whitespace is considered harmless*/

	len = ixend - ixstart;

	ix = ixstart; /*first non-whitespace character*/

	/*scan identifier*/

	if (!isfirstidentifierchar ((*h)[ix]))
		return (false);

	++ix;

	while (ix < ixend && (isidentifierchar ((*h)[ix]) || (*h)[ix] == '.'))
		++ix;

	lenidentifier = ix - ixstart;

	if (lenidentifier == 0)
		return (false);

	setstringlength (bsidentifier, lenidentifier);
	
	memmove (stringbaseaddress (bsidentifier), &((*h)[ixstart]), lenidentifier);

	/*look up identifier in legalMacros table*/

	if (!hashtablelookup (hmacrostable, bsidentifier, &val, &hnode))
		return (false); /*must be illegal*/

	if (langexternalvaltotable (val, &ht, hnode)) {

		boolean fl, fllegal;

		disablelangerror ();

		fl = langlookupbooleanvalue (ht, STR_P_FLLEGAL, &fllegal);
		
		langlookupbooleanvalue (ht, STR_P_FLPARAMS, &flparams);

		enablelangerror ();

		if (!fl || !fllegal)
			return (false); /*must be illegal*/
		}
	
	else {

		if (val.valuetype != booleanvaluetype) /*6.1b7 AR: coercion*/
			if (!copyvaluerecord (val, &val) || !coercetoboolean (&val))
				return (false);

		if (!val.data.flvalue)
			return (false); /*must be illegal*/
		}

	if (!flparams)
		return (ix == ixend);

	/*skip over whitespace*/

	while (ix < ixend && whitespacechar((*h)[ix]))
		++ix;
	
	/*check params*/
	
	if ((*h)[ix] != '(')
		return (false);

	++ix;

	while (ix < ixend) {

		switch ((*h)[ix++]) {

			case chspace:
			case ',':
				break;

			case '"': {

				byte ch;

				while (ix < ixend) {

					ch = (*h)[ix++];

					if (ch == '\\') {

						++ix;
						
						continue;
						}

					if (ch == '"')
						break;
					}/*while*/

				break;
				}

			case ')':
				return (ix == ixend);

			default:
				return (false);
			}/*switch*/

		}/*while*/

	return (false);
	}/*issafemacro*/

#endif

static boolean neutermacros (Handle h, hdlhashtable hmacrostable) {

	handlestream s;
	long ixstart = 0;
	long ixsafeto = -1;
	long balance = 0;


	openhandlestream (h, &s);

	while (!athandlestreameof (&s)) {

		switch (nexthandlestreamcharacter (&s)) {

		case '{':

			if (balance == 0)
				ixstart = s.pos;

			balance++;

			break;

		case '}':

			if (balance > 0) {

				balance--;

				if (balance == 0) {

					if (!issafemacro (s.data, ixstart + 1, s.pos - ixstart - 1, hmacrostable)) {

						Handle hrepl;
						long ixload = ixstart;
						long len = s.pos - ixstart + 1;
						boolean fl;

						if (!loadfromhandletohandle (s.data, &ixload, len, true, &hrepl))
							goto exit;

						fl = stringfindreplace (BIGSTRING ("\x01" "{"), BIGSTRING ("\x06" "&#123;"), hrepl, true, false);

						fl = fl && stringfindreplace (BIGSTRING ("\x01" "}"), BIGSTRING ("\x06" "&#125;"), hrepl, true, false);
						
						if (!fl) {

							disposehandle (hrepl);

							goto exit;
							}

						s.pos = ixstart;

						if (!mergehandlestreamhandle (&s, len, hrepl))
							goto exit;

						s.pos--;
						}
					
					ixsafeto = s.pos;
					}
				}

			else {

				if (!mergehandlestreamstring (&s, 1, BIGSTRING ("\x06" "&#125;")))
					goto exit;

				s.pos--;
				}

			}/*switch*/

		s.pos++;

		}/*while*/

	//assert (s.pos == s.eof);

	if (balance > 0) { /*need to do some cleaning up*/

		Handle hrepl;
		long ixload = ixsafeto + 1;
		long len = s.eof - ixload;
		boolean fl;

		if (!loadfromhandletohandle (s.data, &ixload, len, true, &hrepl))
			goto exit;

		fl = stringfindreplace (BIGSTRING ("\x01" "{"), BIGSTRING ("\x06" "&#123;"), hrepl, true, false);

		fl = fl && stringfindreplace (BIGSTRING ("\x01" "}"), BIGSTRING ("\x06" "&#125;"), hrepl, true, false);
						
		if (!fl) {

			disposehandle (hrepl);

			goto exit;
			}

		s.pos = ixsafeto + 1;

		if (!mergehandlestreamhandle (&s, len, hrepl))
			goto exit;
		}

	closehandlestream (&s);

	return (true);

exit:

	closehandlestream (&s);

	return (false);
	}/*neutermacros*/


static boolean htmlneutermacrosverb (hdltreenode hp1, tyvaluerecord *v) {

	Handle htext;
	hdlhashtable hsafemacrostable;

	flnextparamislast = true;

	if (!gettablevalue (hp1, 2, &hsafemacrostable))
		return (false);

	if (!getexempttextvalue (hp1, 1, &htext))
		return (false);

	if (!neutermacros (htext, hsafemacrostable)) {

		disposehandle (htext);

		return (false);
		}

	return (setheapvalue (htext, stringvaluetype, v));
	}/*htmlneutermacrosverb*/


static boolean incrementcounter (hdlhashtable ht, boolean flincr) {

	long count;

	if (!langlookuplongvalue (ht, STR_P_COUNT, &count))
		return (false);

	if (flincr)
		count++;
	else
		count--;

	if (!langassignlongvalue (ht, STR_P_COUNT, count))
		return (false);

	return (true);
	}/*incrementcounter*/


static boolean neutertags (Handle h, hdlhashtable htagstable) {

	Handle hpacked;
	hdlhashnode hnode;
	hdlhashtable ht;
	handlestream s;
	long n, count;
	boolean fllegal;
	boolean fldoclose = false;
	hdlhashnode hnode2;
	boolean fldummy;

	/*copy legalTags table so we can modify it*/

	if (!tablepacktable (htagstable, true, &hpacked, &fldummy))
		return (false);

	if (!tableunpacktable (hpacked, true, &htagstable))
		return (false);

	/*initialize legalTags table*/

	hashcountitems (htagstable, &n);

	n--;

	while (n >= 0) {

		if (!hashgetnthnode (htagstable, n--, &hnode))
			break;

		if (langexternalvaltotable ((**hnode).val, &ht, hnode)) {

			if (!langlookupbooleanvalue (ht, STR_P_FLLEGAL, &fllegal))
				goto exit;
			}
		else {

			tyvaluerecord val = (**hnode).val;

			if (val.valuetype != booleanvaluetype)
				if (!copyvaluerecord (val, &val) || !coercetoboolean (&val))
					goto exit;

			fllegal = val.data.flvalue;

			if (!langsuretablevalue (htagstable, (**hnode).hashkey, &ht))
				goto exit;

			if (!langassignbooleanvalue (ht, STR_P_FLCLOSE, true))
				goto exit;

			if (!langassignbooleanvalue (ht, STR_P_FLLEGAL, fllegal))
				goto exit;
			}

		if (!fllegal) {

			if (!hashtabledelete (htagstable, (**hnode).hashkey))
				goto exit;

			continue;
			}
		
		if (!langassignlongvalue (ht, STR_P_COUNT, 0L))
			goto exit;

		}/*while*/

	/*loop thru string*/

	openhandlestream (h, &s);

	while (!athandlestreameof (&s)) {

		if (nexthandlestreamcharacter (&s) == '<') {

			long ix, ixstart = s.pos, ixend = 0;
			tyvaluerecord val;

			for (ix = ixstart + 1; ix < s.eof; ix++) {

				if ((gethandlestreamcharacter (&s, ix) == '>')) {

					ixend = ix;

					break;
					}
				}

			if (ixend == 0) { /*hit end of stream, tag doesn't end*/

				if (!mergehandlestreamstring (&s, 1, BIGSTRING ("\x04" "&lt;")))
					goto exit;
				
				continue; //don't increment our position any further
				}

			else {

				bigstring bstag;
				boolean flclose, flisclosetag = false;

				texttostring (&((*s.data)[s.pos+1]), ix - s.pos - 1, bstag);

				if ((stringlength(bstag) > 0) && (getstringcharacter(bstag, 0) == '/')) {
					
					deletefirstchar (bstag);

					flisclosetag = true;
					}

				if (stringfindchar (chspace, bstag))
					nthfield (bstag, 1, chspace, bstag);

				if (hashtablelookup (htagstable, bstag, &val, &hnode2)) {

					if (!langexternalvaltotable (val, &ht, hnode2))
						goto exit;

					if (!langlookupbooleanvalue (ht, STR_P_FLCLOSE, &flclose))
						goto exit;

					if (flclose) {
						fldoclose = true;
						if (!incrementcounter (ht, !flisclosetag))
							goto exit;
						}
					}
				else
					if (!mergehandlestreamstring (&s, 1, BIGSTRING ("\x04" "&lt;")))
						goto exit;

				s.pos += ix - ixstart - 1;
				}
			}

		s.pos++;

		}/*while*/


	/*emit closing tags*/

	if (fldoclose) { /*optimization: only loop over table if we need to close tags*/

		s.pos = s.eof;

		hashcountitems (htagstable, &n);

		n--;

		while (n >= 0) {

			if (!hashgetnthnode (htagstable, n--, &hnode))
				break;

			if (!langexternalvaltotable ((**hnode).val, &ht, hnode))
				goto exit;

			if (!langlookuplongvalue (ht, STR_P_COUNT, &count))
				goto exit;

			if (count > 0) {

				bigstring bs;

				parsedialogstring (str_closetag, (**hnode).hashkey, nil, nil, nil, bs);

				while (count-- > 0)
					if (!writehandlestreamstring (&s, bs))
						goto exit;
				}
			}/*while*/
		}

	/*clean up*/
	
	closehandlestream (&s);

	disposehashtable (htagstable, true);

	return (true);

exit:

	disposehashtable (htagstable, true);

	return (false);
	}/*neutertags*/


static boolean htmlneutertagsverb (hdltreenode hp1, tyvaluerecord *v) {

	Handle htext;
	hdlhashtable htagstable;

	flnextparamislast = true;

	if (!gettablevalue (hp1, 2, &htagstable))
		return (false);

	if (!getexempttextvalue (hp1, 1, &htext))
		return (false);

	if (!neutertags (htext, htagstable)) {

		disposehandle (htext);

		return (false);
		}

	return (setheapvalue (htext, stringvaluetype, v));
	}/*htmlneutertagsverb*/



#pragma mark === html, searchengine, webserver, inetd EFP ===

static boolean htmlfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	6.1d16 AR: Integrated webserver and inetd verbs.
	*/
	
	hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	hdlhashnode hnode;
	
	setbooleanvalue (false, v); /*by default, string functions return false*/
	
	switch (token) {
		
		case processmacrosfunc:
			return (processhtmlmacrosverb (hp1, v));
		
		case urldecodefunc:
			return (urldecodeverb (hp1, v));
		
		case urlencodefunc:
			return (urlencodeverb (hp1, v));
		
		case parseargsfunc:
			return (parseargsverb (hp1, v));
		
		case iso8859encodefunc:
			return (iso8859encodeverb (hp1, v));

		case getgifheightwidthfunc:
			return (getgifheightwidthverb (hp1, v));

		case getjpegheightwidthfunc:
			return (getjpegheightwidthverb (hp1, v));

		case buildpagetablefunc:
			return (buildpagetableverb (hp1, v));
		
		//case refglossaryfunc:
		//	return (refglossaryverb (hp1, v));
		
		case getpreffunc:
			return (getprefverb (hp1, v));
		
		//case getonedirectivefunc:
		//	return (getonedirectiveverb (hp1, v));
		
		case rundirectivefunc:
			return (rundirectiveverb (hp1, v));
		
		case rundirectivesfunc:
			return (rundirectivesverb (hp1, v));
		
		case runoutlinedirectivesfunc:
			return (runoutlinedirectivesverb (hp1, v));
		
		case cleanforexportfunc:
			return (cleanforexportverb (hp1, v));
		
		//case normalizenamefunc:
		//	return (normalizenameverb (hp1, v));
		
		case glossarypatcherfunc:
			return (glossarypatcherverb (hp1, v));
		
		case expandurlsfunc:
			return (expandurlsverb (hp1, v));
		
		case traversalskipfunc:
			return (traversalskipverb (hp1, v));

		case getpagetableaddressfunc:
			return (getpagetableaddressverb (hp1, v));

		case htmlneutermacrosfunc:
			return (htmlneutermacrosverb (hp1, v));

		case htmlneutertagsfunc:
			return (htmlneutertagsverb (hp1, v));

		/*searchengine*/

		case stripmarkupfunc:
			return (stripmarkupverb (hp1, v));
		
		case deindexpagefunc:
			return (deindexpageverb (hp1, v));
		
		case indexpagefunc:
			return (indexpageverb (hp1, v));
		
		case cleanindexfunc:
			return (cleanindexverb (hp1, v));
		
		case unionmatchesfunc:
			return (unionmatchesverb (hp1, v));
			
		/* mainResponder.calendar */

		case mrcalendargetaddressdayfunc:
			return (mrcalendargetaddressdayverb (hp1, v));
	
		case mrcalendargetdayaddressfunc:
			return (mrcalendargetdayaddressverb (hp1, v));
	
		case mrcalendargetfirstaddressfunc:
			return (mrcalendargetfirstaddressverb (hp1, v));
	
		case mrcalendargetfirstdayfunc:
			return (mrcalendargetfirstdayverb (hp1, v));
	
		case mrcalendargetlastaddressfunc:
			return (mrcalendargetlastaddressverb (hp1, v));
	
		case mrcalendargetlastdayfunc:
			return (mrcalendargetlastdayverb (hp1, v));
	
		case mrcalendargetmostrecentaddressfunc:
			return (mrcalendargetmostrecentaddressverb (hp1, v));
	
		case mrcalendargetmostrecentdayfunc:
			return (mrcalendargetmostrecentdayverb (hp1, v));
	
		case mrcalendargetnextaddressfunc:
			return (mrcalendargetnextaddressverb (hp1, v));
	
		case mrcalendargetnextdayfunc:
			return (mrcalendargetnextdayverb (hp1, v));
	
		case mrcalendarnavigatefunc:
			return (mrcalendarnavigateverb (hp1, v));
		
		case htmlcalendardrawfunc:
			return (htmlcalendardrawverb (hp1, v));

		/*webserver*/

		case webserverserverfunc: {
			tyaddress adrparamtable;
			Handle hrequest = nil;

			if (!getvarparam (hp1, 1, &adrparamtable.ht, adrparamtable.bs)) //address of param table
				return (false);

			if (langgetparamcount (hp1) > 1) {

				flnextparamislast = true;

				if (!getreadonlytextvalue (hp1, 2, &hrequest)) //full request (optionally)
					return (false);
				}
		
			return (webserverserver (&adrparamtable, hrequest, v));
			}

		case webserverdispatchfunc: {
			tyaddress adrparamtable;
			
			flnextparamislast = true;

			if (!getvarparam (hp1, 1, &adrparamtable.ht, adrparamtable.bs)) //address of param table
				return (false);
		
			return (webserverdispatch (&adrparamtable, v));
			}

		case webserverparseheadersfunc: {
			Handle response, h;
			hdlhashtable htable, hheadertable;
			bigstring bs;

			if (!getreadonlytextvalue (hp1, 1, &response))
				return (false);

			flnextparamislast = true;

			if (!getvarparam (hp1, 2, &htable, bs))
				return (false);

			if (!langsuretablevalue (htable, bs, &hheadertable))
				return (false);

			if (!webserverparseheaders (response, hheadertable, &h))
				return (false);
			
			return (setheapvalue (h, stringvaluetype, v));
			}

		case webserverparsecookiesfunc: {
			hdlhashtable ht;

			flnextparamislast = true;

			if (!gettablevalue (hp1, 1, &ht))  //6.1b8 AR: We assume the table already exists
				return (false);

			return (webserverparsecookies (ht, v));
			}

		case webserverbuildresponsefunc: {
			hdlhashtable hheadertable = nil;
			hdlhashtable hparenttable = nil;
			bigstring bscode, bstablename;
			tyvaluerecord vtable, vresponse, vadrtable;
			short ctconsumed = 1;
			short ctpositional = 1;
			
			if (!getstringvalue (hp1, 1, bscode))
				return (false);

			initvalue (&vadrtable, addressvaluetype);
			
			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_P_ADRHEADERTABLE, &vadrtable)) 
				return (false);

			if (vadrtable.data.addressvalue != nil) { /*6.1b12 AR*/

				if (!getaddressvalue (vadrtable, &hparenttable, bstablename))
					return (false);

				if (hparenttable != nil) {

					if (!langhashtablelookup (hparenttable, bstablename, &vtable, &hnode))
						return (false);

					if (!langexternalvaltotable (vtable, &hheadertable, hnode))
						return (false);
					}
				}

			initvalue (&vresponse, stringvaluetype);
			
			flnextparamislast = true;

			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_P_RESPONSEBODY, &vresponse)) 
				return (false);
				
			return (webserverbuildresponse (bscode, hheadertable, vresponse.data.stringvalue, v));
			}
			
		case webserverbuilderrorpagefunc: {
			Handle h, hshort, hlong;

			if (!getreadonlytextvalue (hp1, 1, &hshort))
				return (false);

			flnextparamislast = true;

			if (!getreadonlytextvalue (hp1, 2, &hlong))
				return (false);

			if (!webserverbuilderrorpage (hshort, hlong, &h))
				return (false);

			return (setheapvalue (h, stringvaluetype, v));
			}

		case webservergetserverstringfunc: {

			if (!langcheckparamcount (hp1, 0))
				return (false);

			return (webservergetserverstring (v));
			}

		/*inetd*/
		
		case inetdsupervisorfunc: {
			long stream, refcon;

			if (!getlongvalue (hp1, 1, &stream)) //stream ID
				return (false);

			flnextparamislast = true;

			if (!getlongvalue (hp1, 2, &refcon)) //port number
				return (false);

			return (inetdsupervisor (stream, refcon, v));
			}


		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			return (false);
		} /*switch*/
	} /*htmlfunctionvalue*/


boolean htmlinitverbs (void) {
	
	/*
	5.0.2 dmb: new verbs
	*/
	
	return (loadfunctionprocessor (idhtmlverbs, &htmlfunctionvalue));
	} /*htmlinitverbs*/
	
