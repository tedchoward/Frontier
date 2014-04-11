
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
#include "file.h"
#include "ops.h"
#include "strings.h"
#include "shellhooks.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langtokens.h"
#include "tableinternal.h"
#include "tableverbs.h"
#include "tablestructure.h"
#include "resources.h"
#include "WinSockNetEvents.h"
#include "sysshellcall.h" /* 2006-03-09 aradke: unixshellcall moved from CallMachOFramework.h */
#include "byteorder.h"	/* 2006-04-16 aradke: swap byte-order in loadfunctionprocessor */


#define str_isPike				BIGSTRING ("\x06" "isPike")
#define str_isOpmlEditor        BIGSTRING ("\x0c" "isOpmlEditor")    /* 2005-04-06 dluebbert */
#define str_isFrontier          BIGSTRING ("\x0a" "isFrontier")      /* 2005-04-06 dluebbert */
#define str_isRadio				BIGSTRING ("\x07" "isRadio")
#define str_isMac				BIGSTRING ("\x05" "isMac")
#define str_isMacOsClassic		BIGSTRING ("\x0e" "isMacOsClassic") /* 2004-11-19 creedon */
#define str_isServer			BIGSTRING ("\x08" "isServer") /* 2004-11-19 creedon */
#define str_isWindows			BIGSTRING ("\x09" "isWindows")
#define str_osFlavor			BIGSTRING ("\x08" "osFlavor")
#define str_osMajorVersion		BIGSTRING ("\x0e" "osMajorVersion")
#define str_osMinorVersion		BIGSTRING ("\x0e" "osMinorVersion")
#define str_osPointVersion		BIGSTRING ("\x0e" "osPointVersion") /* 2004-11-19 creedon */
#define str_osBuildNumber		BIGSTRING ("\x0d" "osBuildNumber")
#define str_osVersionString		BIGSTRING ("\x0f" "osVersionString")
#define str_osFullNameForDisplay	BIGSTRING ("\x14" "osFullNameForDisplay")
#define str_winServicePackNumber	BIGSTRING ("\x14" "winServicePackNumber")
#define str_isCarbon				BIGSTRING ("\x08" "isCarbon")
#define str_maxTcpConnections		BIGSTRING ("\x11" "maxTcpConnections")


void initsegment (void) {
	
	} /*initsegment*/


static boolean newfunctionprocessor (bigstring bsname, langvaluecallback valuecallback, boolean flwindow, hdlhashtable *htable) {
	
	/*
	each of the external function processors register with the system by calling
	this routine.  we create a new hashtable in the EFP table, and return a
	handle to the new table.  we also link in a callback routine that processes the
	verbs for this EFP.  flwindow is also recorded -- it says whether or not
	the EFP requires a window be open in order for one of its verbs to be
	executed.
	
	6/1/93 dmb: flwindow parameter is now real. when true, the valuecallback must
	respond to being called with a nil parameter list by determining whether or not 
	a specific verb requires Frontier to be the current process.
	*/
	
	register hdlhashtable ht;
	
	if (!tablenewsystemtable (efptable, bsname, htable))
		return (false);
	
	ht = *htable; /*copy into register*/
	
	(**ht).flverbsrequirewindow = flwindow;
	
	(**ht).valueroutine = valuecallback;
	
	return (true);
	} /*newfunctionprocessor*/


static boolean hashinsertcstring (bigstring bs, const tyvaluerecord *v) {

	convertcstring (bs);

	return (hashinsert (bs, *v));
	} /*hashinsertcstring*/


static boolean langaddcstringkeyword (bigstring bs, short tokennumber) {
	
	tyvaluerecord val;
	
	initvalue (&val, tokenvaluetype);
	
	val.data.tokenvalue = (short) tokennumber;
	
	return (hashinsertcstring (bs, &val));
	} /*langaddcstringkeyword*/
	

boolean langaddkeyword (bigstring bs, short tokennumber) {
	
	tyvaluerecord val;
	
	initvalue (&val, tokenvaluetype);
	
	val.data.tokenvalue = (short) tokennumber;
	
	return (hashinsert (bs, val));
	} /*langaddkeyword*/
	
/*
boolean langaddkeywordlist (hdlhashtable htable, byte * bskeywords[], short ctkeywords) {
	
	register short i;
	register boolean fl = true;
	
	pushhashtable (htable); 
	
	for (i = 0;  fl && (i < ctkeywords);  i++)
		fl = langaddkeyword ((ptrstring) bskeywords [i], i);
	
	pophashtable ();
	
	return (fl);
	} /%langaddkeywordlist%/
*/

boolean loadfunctionprocessor (short id, langvaluecallback valuecallback) {
	
	/*
	2.1b5 dmb: having learning that our use of lots of string literals, and 
	hence the "Seperate STRS" option in Think C was making us victim of an 
	'040 instruction cache bug in Think C's run-time implementation of same, 
	it's time to create the kernel tables directly from resources.  that's 
	what this routine does.
	
	2006-04-16 aradke: swap byte-order on Intel Macs
	*/
	
	bigstring bsname;
	hdlhashtable htable;
	short flwindow;
	long ix = 0;
	Handle hefps;
	short ctefps;
	short ctverbs;
	short ixverb = 0;
	boolean fl = true;
	
#ifdef MACVERSION
	hefps = getresourcehandle ('EFP#', id);
#endif
#ifdef WIN95VERSION
	hefps = getresourcehandle ('EFP_', id);
#endif
	
	assert (hefps != nil);
	
	if (hefps == nil)
		return (false);
	
	if (!loadfromhandle (hefps, &ix, 2, &ctefps))
		return (false);
	
	reztomemshort (ctefps);
	
	while (--ctefps >= 0) {
		
		copyrezstring (BIGSTRING (*hefps + ix), bsname);
		
		ix += stringsize (bsname);
		
		if (!loadfromhandle (hefps, &ix, 2, &flwindow))
			return (false);
		
		reztomemshort (flwindow);
		
		if (!newfunctionprocessor (bsname, valuecallback, (boolean) flwindow, &htable))
			return (false);
		
		if (!loadfromhandle (hefps, &ix, 2, &ctverbs))
			return (false);
		
		reztomemshort (ctverbs);
		
		pushhashtable (htable); 
		
		while (--ctverbs >= 0 && fl) {
			
			copyrezstring (BIGSTRING (*hefps + ix), bsname);
			
			ix += stringsize (bsname);
			
			fl = langaddkeyword ((ptrstring) bsname, ixverb++);
			}
		
		pophashtable ();
		}

	releaseresourcehandle (hefps);
	
	return (fl);
	} /*loadfunctionprocessor*/


static boolean initenvironment ( hdlhashtable ht ) {

	//
	// 2007-06-29 creedon: isMacOsClassic is always false now that we don't
	//				   support the Mac Classic envrionment any longer
	//
	//				   for Mac, bug fix for OS version numbers like 10.4.10
	//
	// 2004-11-19 creedon: added isServer
	//
	//				   for Mac, added osBuildNumber, osFullNameForDisplay
	//				   ( now a calculated value )
	//
	
	bigstring bsos, bsversion;
	boolean isServer;
	
	#ifdef MACVERSION
	
		Handle hcommand, hreturn;
		bigstring bs;
    
		newemptyhandle ( &hreturn );
		
		bundle { // system version
		
			getsystemversionstring ( bsversion, NULL );
			
			bundle { // major
                SInt32 majorVersion;
                Gestalt(gestaltSystemVersionMajor, &majorVersion);
                langassignlongvalue(ht, str_osMajorVersion, majorVersion);
				}
					
			bundle { // minor
                SInt32 minorVersion;
                Gestalt(gestaltSystemVersionMinor, &minorVersion);
                langassignlongvalue(ht, str_osMinorVersion, minorVersion);
				}
					
			bundle { // bug fix
                SInt32 bugfixVersion;
                Gestalt(gestaltSystemVersionBugFix, &bugfixVersion);
                langassignlongvalue(ht, str_osPointVersion, bugfixVersion);
				}
					
			}
			
		langassignbooleanvalue (ht, str_isMac, true);
		
		langassignbooleanvalue (ht, str_isWindows, false);
		
		// langassignstringvalue (ht, str_osFlavor, zerostring);
		
		langassignbooleanvalue (ht, str_isCarbon, true);
		
		bundle { // get mac os build number
		
			newtexthandle ( "\psw_vers -buildVersion", &hcommand );
			
			unixshellcall ( hcommand, hreturn );
			
			texthandletostring ( hreturn, bs );
			
			sethandlesize ( hreturn, 0 );
			
			setstringlength ( bs, stringlength ( bs ) - 1 );
			
			langassignstringvalue ( ht, str_osBuildNumber, bs );
			
			}
			
		bundle { // get os full display name
		
			copystring ( "\psw_vers -productName", bs ); 
			
			sethandlecontents ( stringbaseaddress ( bs ), stringlength ( bs ), hcommand );
			
			unixshellcall ( hcommand, hreturn );
			
			texthandletostring ( hreturn, bsos );
			
			setstringlength ( bsos, stringlength ( bsos ) - 1 );
			
			}
			
		disposehandle ( hcommand );
		
		disposehandle ( hreturn );
		
		bundle { // is server
		
			if ( equalstrings ( bsos, "\pMac OS X Server" ) )
				isServer = true;
			else
				isServer = false;
			}
			
	#endif

	#ifdef WIN95VERSION
	
		bigstring bsservicepack;
		byte bsflavor [4];
		OSVERSIONINFO osinfo;

		osinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		
		GetVersionEx (&osinfo);
		
		copystring (BIGSTRING ("\x08" "Windows "), bsos);
		
		langassignbooleanvalue (ht, str_isMac, false);
		
		langassignbooleanvalue (ht, str_isWindows, true);
		
		if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		
			copystring (BIGSTRING ("\x02" "NT"), bsflavor);
			
			isServer = true;
			
			}
		else {
			
			if (osinfo.dwBuildNumber == 0)
				copystring (BIGSTRING ("\x02" "95"), bsflavor);
			else
				copystring (BIGSTRING ("\x02" "98"), bsflavor);
			
			isServer = false;
			
			}
			
		pushstring (bsflavor, bsos);
		
		langassignstringvalue (ht, str_osFlavor, bsflavor);
		
		langassignlongvalue (ht, str_osMajorVersion, osinfo.dwMajorVersion);
		
		langassignlongvalue (ht, str_osMinorVersion, LOWORD(osinfo.dwMinorVersion));
		
		langassignlongvalue (ht, str_osBuildNumber, LOWORD(osinfo.dwBuildNumber));
		
		getsystemversionstring (bsversion, bsservicepack);
		
		langassignstringvalue (ht, str_winServicePackNumber, bsservicepack);
		
		langassignbooleanvalue (ht, str_isCarbon, false); // 7.0b28: isCarbon is false on Windows.
		
	#endif
	
	langassignbooleanvalue ( ht, str_isMacOsClassic, false );
	
	langassignbooleanvalue ( ht, str_isServer, isServer );
		
	langassignlongvalue ( ht, str_maxTcpConnections, maxconnections ); // 7.0b37 PBS: max TCP connections
	
	langassignstringvalue ( ht, str_osFullNameForDisplay, bsos );
	
	langassignstringvalue ( ht, str_osVersionString, bsversion );
	
	#ifdef PIKE
	
		#ifndef OPMLEDITOR
		
			langassignbooleanvalue (ht, str_isPike, true);
			langassignbooleanvalue (ht, str_isRadio, true); /*7.0b37 PBS: system.environment.isRadio*/
			langassignbooleanvalue (ht, str_isOpmlEditor, false); /*2005-04-06 dluebbert: system.environment.isOPML*/
			langassignbooleanvalue (ht, str_isFrontier, false); /*2005-04-06 dluebbert: system.environment.isFrontier*/
			
		#else // OPMLEDITOR
		
			langassignbooleanvalue (ht, str_isPike, false);
			langassignbooleanvalue (ht, str_isRadio, false); /*7.0b37 PBS: system.environment.isRadio*/
			langassignbooleanvalue (ht, str_isOpmlEditor, true); /*2005-04-06 dluebbert: system.environment.isOPML*/
			langassignbooleanvalue (ht, str_isFrontier, false); /*2005-04-06 dluebbert: system.environment.isFrontier*/
			
		#endif // OPMLEDITOR
		
	#else //!PIKE
	
		langassignbooleanvalue (ht, str_isPike, false);
		langassignbooleanvalue (ht, str_isRadio, false); /*7.0b37 PBS: system.environment.isRadio*/
		langassignbooleanvalue (ht, str_isOpmlEditor, false); /*2005-04-06 dluebbert: system.environment.isOPML*/
		langassignbooleanvalue (ht, str_isFrontier, true); /*2005-04-06 dluebbert: system.environment.isFrontier*/
		
	#endif //!PIKE

	return ( true );
		
	} // initenvironment


static boolean initCharsetsTable (hdlhashtable cSetsTable)
{
#if MACVERSION
	OSStatus err;
	ItemCount ct, actual_ct, i;
	
	err = TECCountAvailableTextEncodings( &ct );
	if ( err != noErr ) {
		return (true);  // don't kill the whole startup
	}
	
	TextEncoding enc, encOut;
	TextEncoding availEncodings[ ct ];
	bigstring ianaName, displayName;
	unsigned long lenDisplayName;
	RegionCode reg;
	
	err = TECGetAvailableTextEncodings ( availEncodings, ct, &actual_ct );
	if ( err != noErr )
		return (true);  // we don't want to kill the whole startup here
	
	for ( i = 0; i < actual_ct; i++ ) {
		enc = availEncodings[ i ];
		
		/*
			get the internet "iana" name for the encoding
		*/
		err = TECGetTextEncodingInternetName( enc, ianaName );
		if ( err != noErr )
			continue;
		
		/*
			now convert the "iana" name back into the canonical encoding value
			(more than one encoding will point to the same iana name, 
			this makes sure that we use the right one)
		*/
		err = TECGetTextEncodingFromInternetName( &enc, ianaName );
		if ( err != noErr )
			continue;
		
		/*
			get the encoding's display name, which is the value of the table cell
		*/
		err = GetTextEncodingName( enc, kTextEncodingFullName, verUS, kTextEncodingMacRoman, 255, &lenDisplayName, &reg, &encOut, (TextPtr) displayName + 1 );
		if ( err != noErr )
			continue;
		
		displayName[ 0 ] = (unsigned char) lenDisplayName;
		
		langassignstringvalue( cSetsTable, ianaName, displayName );
	}
	
	return (true);
	
#endif

#if WIN95VERSION
	HRESULT err;
	IMultiLanguage2 * pMultiLanguage;
	IEnumCodePage * pEnumCodePage;
	UINT i, cnum = 0;
	MIMECPINFO cpInfo;
	long ccpInfo;
	bigstring ianaName, displayName;

	initCOM();

	err = CoCreateInstance(
			&CLSID_CMultiLanguage, 
			NULL,
			CLSCTX_INPROC_SERVER,
			&IID_IMultiLanguage2,
			(void **) &pMultiLanguage );

	if ( FAILED( err ) )
		goto done;

	err = pMultiLanguage->lpVtbl->EnumCodePages(
		pMultiLanguage,
		0,  // MIMECONTF_MAILNEWS | MIMECONTF_BROWSER | MIMECONTF_VALID | MIMECONTF_MIME_LATEST
		9,  /* LANGID. I'm totally guessing that 0 means "all", here. -- smd */
		(IEnumCodePage **) &pEnumCodePage );
	
	if ( FAILED( err ) )
		goto done;

	pMultiLanguage->lpVtbl->GetNumberOfCodePageInfo( pMultiLanguage, &cnum );
	
	// pcpInfo = (PMIMECPINFO)CoTaskMemAlloc( sizeof(MIMECPINFO) );

	for ( i = 0; i < cnum; i++ )
	{
		err = pEnumCodePage->lpVtbl->Next( pEnumCodePage, 1, &cpInfo, &ccpInfo );

		if ( SUCCEEDED( err ) && ( ccpInfo != 0 ) )
		{
			if ( copyWideToPString( cpInfo.wszWebCharset, ianaName )
				&& copyWideToPString( cpInfo.wszDescription, displayName ) )
				langassignstringvalue( cSetsTable, ianaName, displayName );
				//langassignlongvalue( cSetsTable, ianaName, cpInfo.uiCodePage );
		}
	}

	pEnumCodePage->lpVtbl->Release( pEnumCodePage );
	pMultiLanguage->lpVtbl->Release( pMultiLanguage );
	// CoTaskMemFree( (LPVOID) pcpInfo );

done:
	return (true);
	
#endif	
	} /* initCharsetsTable */


boolean inittablestructure (void) {
	
	/*
	do this just after you initialize lang.c.  
	
	we build the initial structure of hashtables for CanCoon.
	
	5.1.4 dmb: do the environmenttable
	*/
	
	hdlhashtable htable; 
	
	if (!newhashtable (&htable)) /*this is where everything starts*/
		return (false);
	
	pushhashtable (htable); /*set lang.c global*/
	
	// do the compiler table
	
	if (!tablenewsystemtable (htable, nameinternaltable, &internaltable))
		goto error;
	
	if (!tablenewsystemtable (internaltable, nameefptable, &efptable))
		goto error;
	
	if (!tablenewsystemtable (internaltable, namelangtable, &langtable))
		goto error;
	
	if (!tablenewsystemtable (internaltable, namestacktable, &runtimestacktable))
		goto error;
	
	if (!tablenewsystemtable (internaltable, namesemaphoretable, &semaphoretable))
		goto error;
	
	#if threadverbs
		if (!tablenewsystemtable (internaltable, namethreadtable, &threadtable))
			goto error;
	#endif
		
	#ifdef version42orgreater
		if (!tablenewsystemtable (internaltable, namefilewindowtable, &filewindowtable))
			goto error;
	#endif
	
	// now do the environment table
	
	if (!tablenewsystemtable (htable, nameenvironmenttable, &environmenttable))
		goto error;
	
	if (!initenvironment (environmenttable))
		goto error;
	
	// and now the charsets table
	
	if (!tablenewsystemtable (htable, namecharsetstable, &charsetstable))
		goto error;
	
	initCharsetsTable (charsetstable);  /* don't die if the charsets can't be initialized */
	
	pophashtable ();
	
	return (true);
	
	error:
	
	pophashtable ();
	
	disposehashtable (htable, false);
	
	internaltable = efptable = langtable = runtimestacktable = semaphoretable = threadtable = filewindowtable = environmenttable = nil;
	
	return (false);
	} /*inittablestructure*/


static boolean langaddnilconst (bigstring bs) {
	
	tyvaluerecord val;
	
	initvalue (&val, novaluetype);
	
	return (hashinsertcstring (bs, &val));
	} /*langaddnilconst*/


static boolean langaddlongconst (bigstring bs, long x) {
	
	tyvaluerecord val;
	
	setlongvalue (x, &val);
	
	return (hashinsertcstring (bs, &val));
	} /*langaddlongconst*/


static boolean langaddstringconst (bigstring bs, bigstring x) {
	
	tyvaluerecord val;
	
	setstringvalue (x, &val);
	
	if (!hashinsertcstring (bs, &val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	return (true);
	} /*langaddstringconst*/


#if 0

static boolean langaddintconst (bigstring bs, short x) {
	
	tyvaluerecord val;
	
	setintvalue (x, &val);
	
	return (hashinsertcstring (bs, &val));
	} /*langaddintconst*/

#endif
	

static boolean langadddirectionconst (bigstring bs, tydirection x) {
	
	tyvaluerecord val;
	
	setdirectionvalue (x, &val);
	
	return (hashinsertcstring (bs, &val));
	} /*langadddirectionconst*/


static boolean langaddbooleanconst (bigstring bs, boolean x) {
	
	tyvaluerecord val;
	
	setbooleanvalue (x, &val);
	
	return (hashinsert (bs, val));
	} /*langaddbooleanconst*/


static boolean langaddtypeconst (bigstring bs, tyvaluetype x) {
	
	tyvaluerecord val;
	
	setostypevalue (langgettypeid (x), &val);
	
	return (hashinsert (bs, val));
	} /*langaddtypeconst*/


#define add(x,y) if (!langaddcstringkeyword ((ptrstring) x, y)) return (false)

#define addnil(x) if (!langaddnilconst ((ptrstring) x)) return (false)

#define addlong(x,y) if (!langaddlongconst ((ptrstring) x, y)) return (false)

#define addint(x,y) if (!langaddintconst ((ptrstring) x, y)) return (false)

#define adddirection(x,y) if (!langadddirectionconst ((ptrstring) x, y)) return (false)

#define addboolean(x,y) if (!langaddbooleanconst ((ptrstring) x, y)) return (false)

#define addtype(x,y) if (!langaddtypeconst ((ptrstring) x, y)) return (false)

#define addstring(x,y) if (!langaddstringconst ((ptrstring) x, y)) return (false)


static boolean langinitconsttable (void) {
	
	/*
	2.1b1 dmb: added shortType as a special case; redundant with intType, 
	but more consistent with user termiology
	
	2.1b2 dmb: added nil constant
	*/
	
	tyvaluetype type;
	bigstring bs;
	
	if (!tablenewsystemtable (langtable, (ptrstring) "\x09" "constants", &hconsttable))
		return (false);
		
	pushhashtable (hconsttable); /*converted to constants by the scanner*/
	
	addnil ("nil");
	
	addlong ("infinity", longinfinity);
	
	adddirection ("up", up);
	
	adddirection ("down", down);
	
	adddirection ("left", left);
	
	adddirection ("right", right);
	
	adddirection ("flatup", flatup);
	
	adddirection ("flatdown", flatdown);
	
	adddirection ("nodirection", nodirection);
	
	adddirection ("pageup", pageup);
	
	adddirection ("pagedown", pagedown);
	
	adddirection ("pageleft", pageleft);
	
	adddirection ("pageright", pageright);
	
	addboolean (bstrue, (boolean) true);
	
	addboolean (bsfalse, (boolean) false);
	
	for (type = novaluetype; type < ctvaluetypes; type++)
		
		switch (type) {
			
			case olddoublevaluetype: /*don't define constants for these types*/
			case externalvaluetype:
			
			#ifndef fliowa
			
			case headvaluetype:
			
			#endif
			
			case oldstringvaluetype: /*8/13*/
			case passwordvaluetype: /*9/17*/
			case unused2valuetype:
				break;
			
			default:
				langgettypestring (type, bs);
				
				lastword (bs, chspace, bs);
				
				pushstring ((ptrstring) "\x04Type", bs);
				
				addtype (bs, type);
				
				break;
			}
	
	addtype (BIGSTRING ("\x09shortType"), intvaluetype); /*special case to match coercion verb*/
	
	addstring ("machinePPC", machinePPC);
	addstring ("machine68K", machine68K);
	addstring ("machineX86", machinex86);
	
	addstring ("osMacOS", osMacOS);
	//Code change by Timothy Paustian Tuesday, July 11, 2000 9:41:39 PM
	//We add a detection for the carbon environment
	addstring("osMacCn", osCarbon);
	addstring ("osWin95", osWin95);
	addstring ("osWinNT", osWinNT);
	
	pophashtable ();
	
	return (true);
	} /*langinitconsttable*/


static boolean langinitbuiltintable (void) {
	
	if (!tablenewsystemtable (langtable, (ptrstring) "\x08" "builtins", &hbuiltinfunctions))
		return (false);
	
	pushhashtable (hbuiltinfunctions); /*converted to function ops by the parser*/
	
	add ("appleevent", appleeventfunc);
	
	add ("complexevent", complexeventfunc);
	
	add ("finderevent", findereventfunc);
	
	add ("tableevent", tableeventfunc);
	
	add ("objspec", objspecfunc);
	
	add ("setobj", setobjspecfunc);
	
	add ("pack", packfunc);
	
	add ("unpack", unpackfunc);
	
	add ("defined", definedfunc);
	
	add ("typeof", typeoffunc);
	
	add ("sizeof", sizeoffunc);
	
	add ("nameof", nameoffunc);
	
	add ("parentof", parentoffunc);

	add ("indexof", indexoffunc);
	
	add ("gestalt", gestaltfunc);
	
	add ("syscrash", syscrashfunc);
	
	#if !flruntime
	
	add ("myMoof", myMooffunc);
	
	#endif
	
	pophashtable ();
	
	return (true);
	} /*langinitbuiltintable*/


static boolean langinitkeywordtable (void) {
	
	/*
	3/6/92 dmb: added "with" token
	*/
	
	if (!tablenewsystemtable (langtable, (ptrstring) "\x08" "keywords", &hkeywordtable))
		return (false);
	
	pushhashtable (hkeywordtable); /*converted to tokens by the scanner*/
	
	add ("equals", equalsfunc);
	
	add ("notequals", notequalsfunc);
	
	add ("greaterthan", greaterthanfunc);
	
	add ("lessthan", lessthanfunc);
	
	add ("not", notfunc);
	
	add ("and", andfunc);
	
	add ("or", orfunc);
	
	add ("beginswith", beginswithfunc);
	
	add ("endswith", endswithfunc);
	
	add ("contains", containsfunc);
	
	add ("loop", loopfunc);
	
	add ("fileloop", fileloopfunc);
	
	add ("while", whilefunc);
	
	add ("in", infunc);
	
	add ("break", breakfunc);
	
	add ("continue", continuefunc);
	
	add ("return", returnfunc);
	
	add ("if", iffunc);
	
	add ("then", thenfunc);
	
	add ("else", elsefunc);
	
	add ("bundle", bundlefunc);
	
	add ("local", localfunc);
	
	add ("on", onfunc);
	
	add ("case", casefunc);
	
	add ("kernel", kernelfunc);
	
	add ("for", forfunc);
	
	add ("to", tofunc);
	
	add ("downto", downtofunc);
	
	add ("with", withfunc);
	
	add ("try", tryfunc);
	
	pophashtable ();
	
	return (true);
	} /*langinitkeywordtable*/


static boolean langinstallresources (void) {
	
	if (!langinitconsttable ())
		return (false);
	
	if (!langinitbuiltintable ())
		return (false);
	
	if (!langinitkeywordtable ())
		return (false);
	
	return (true);
	} /*langinstallresources*/


boolean langinitverbs (void) {
	
	if (!langinstallresources ())
		return (false);
	
	return (langinitbuiltins ());
	} /*langinitverbs*/

boolean initlang (void) {

	shellpushmemoryhook (&hashflushcache);
	
	langcallbacks.symbolchangedcallback = (langsymbolchangedcallback) &truenoop; 
	
	langcallbacks.symbolunlinkingcallback = (langtablenodecallback) &truenoop;
	
	langcallbacks.symboldeletedcallback = (langaddresscallback) &truenoop;
	
	langcallbacks.symbolinsertedcallback = (langsymbolinsertedcallback) &truenoop;
	
	langcallbacks.comparenodescallback = (langcomparenodescallback) &falsenoop;
		
	langcallbacks.debuggercallback = (langtreenodecallback) &truenoop;
	
	langcallbacks.debugerrormessagecallback = (langerrormessagecallback) &truenoop;
	
	langcallbacks.scriptkilledcallback = (langbooleancallback) &falsenoop;
	
	newclearhandle (longsizeof (tyerrorstack), (Handle *) &langcallbacks.scripterrorstack);
	
	/* 4.1b4 dmb: no longer push this be default. langrun now does it.
	langpusherrorcallback (nil, 0L);
	*/
	
	/*
	langcallbacks.scripterrorcallback = &truenoop;
	
	langcallbacks.scripterrorrefcon = (long) 0;
	*/
	
	langcallbacks.scriptcompilecallback = (langhashnodetreenodecallback) &falsenoop;
	
	langcallbacks.backgroundtaskcallback = (langbooleancallback) &truenoop;
	
	langcallbacks.pushtablecallback = (langtablerefcallback) &langdefaultpushtable;
	
	langcallbacks.poptablecallback = (langtablecallback) &langdefaultpoptable;
	
	langcallbacks.pushsourcecodecallback = (langsourcecodecallback) &truenoop;
	
	langcallbacks.popsourcecodecallback = (langvoidcallback) &truenoop;
	
	langcallbacks.saveglobalscallback = (langvoidcallback) &truenoop;
	
	langcallbacks.restoreglobalscallback = (langvoidcallback) &truenoop;
	
	langcallbacks.errormessagecallback = (langerrormessagecallback) &truenoop;
	
	langcallbacks.errormessagerefcon = nil;
	
	langcallbacks.clearerrorcallback = (langvoidcallback) &truenoop;
	
	langcallbacks.msgverbcallback = (langverbcallback) &truenoop;
	
	langcallbacks.codereplacedcallback = (langtreenodescallback) &truenoop;
	
	langcallbacks.idvaluecallback = nil;
	
	langcallbacks.partialeventloopcallback = (langshortcallback) &falsenoop;
	
	langcallbacks.processeventcallback = (langeventcallback) &falsenoop; /*4.1b13 dmb - new*/
	
	if (!newclearhandle (longsizeof (tytablestack), (Handle *) &hashtablestack))
		return (false);
	
	return (true);
	} /*initlang*/


