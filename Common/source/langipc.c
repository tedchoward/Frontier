
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

#include <land.h>
#include "error.h"
#include "memory.h"
#include "ops.h"
#include "strings.h"
#include "file.h"
#include "launch.h"
#include "resources.h"
#include "shell.h"
#include "shellhooks.h"
#include "oplist.h"
#include "lang.h"
#include "langinternal.h"
#include "langipc.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "tableinternal.h" /*for error string numbers -- see langipctablemessage*/
#include "tablestructure.h"
#include "tableverbs.h"
#include "op.h"
#include "meprograms.h"
#include "process.h"
#include "processinternal.h"
#include "byteorder.h"
#ifdef flcomponent
	#include <uisharing.h>
	#include <uisinternal.h>
	#include "osacomponent.h"
	#include <SetUpA5.h>
#endif

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h"
#endif


// Subroutine Events:

/*
enum {
	kASSubroutineEvent			= 'psbr',
	keyASSubroutineName			= 'snam'
};
*/

#define keykerneltable 'ktbl'
#define keycurrenttable 'ctbl'


#define landnofileopenerror (lastinternalerror + 1)

#define idwho 'who?'
#define isruntime	0x0000
#define isfrontier	0x0001

#define bcdversion	0x05000000


#if flruntime

	#define iswho (bcdversion + isruntime)

#else
	
	#define iswho (bcdversion + isfrontier)

#endif
	


typedef struct tyipcaddress {
	
	OSType appid;
	
	Str32 appname;
	
	typrocessid appprocessid;
	
	boolean flcodeval; /*set if val contains address of direct handler*/
	
	boolean flrootval; /*set if val contains filespec of root handler*/
	
	tyvaluerecord val;
	
	/*remaining fields for network addresses*/
	
	boolean flnetwork; /*set if these fields are meaningful*/
	
	tynetworkaddress netaddress; /*will be null desciptor if not on network*/
	} tyipcaddress;


typrocessid langipcself;

static boolean fltoolkitinitialized = false;

static hdlverbrecord hipcverb = nil; /*the verb currently being handled*/

//Code change by Timothy Paustian Wednesday, June 14, 2000 8:59:44 PM
//Changed to Opaque call for Carbon
//This is never used except to get the zone which is never
//used anywhere else in the application
//static THz langipczone;

//static Handle langipcresmap;

static short langipcresfile;



boolean langipcerrorroutine (bigstring bs, ptrvoid refcon) {
#pragma unused (refcon)

	/*
	if an error occurs while processing a runscript verb, we want to 
	return the text that would normally go into the langerror window as 
	an error string to our caller.
	
	we can't call landreturnerror because our error string doesn't have 
	anything to do with our stringlist.  so we push our error values 
	manually.
	
	7/2/91 dmb: we can only get called when a process is running that 
	was dispatched from this file, or when an error occurs while no 
	process is running.  to support multiple threads, we get the verb handle 
	from the process refcon if one is running.
	
	2/4/92 dmb: don't call langerrorenabled or set fllangerror here; now 
	handled by langerrormessage
	
	4/1/93 dmb: allow for the possibility that the process is non-nil, but 
	doesn't have the verbrecord attached
	*/
	
	register hdlverbrecord hv = nil;
	
	if (currentprocess != nil)
		hv = (hdlverbrecord) (**currentprocess).processrefcon;
	
	if (hv == nil)
		hv = hipcverb;
	
	if ((**hv).verbtoken != idscriptcompleted) { /*this isn't a menu sharing return message*/
		
		if (!landstartreturn (hv)) /*error returning error!*/
			return (true);
		}
	
	if (!landpushstringparam (hv, bs, errorstringkey))
		return (true);
	
	landpushintparam (hv, errAEEventFailed, errornumberkey); /*non-zero.  descriptiveness comes from string*/
	
	return (false); /*consume the error*/
	} /*langipcerrorroutine*/


static void langipchookerrors (hdlverbrecord hverb, callback *savecallback) {
	
	*savecallback = (callback) langcallbacks.errormessagecallback;
	
	langcallbacks.errormessagecallback = &langipcerrorroutine;
	
	shellpusherrorhook ((errorhookcallback) &langipcerrorroutine);
	
	hipcverb = hverb; /*make visible to errorroutine*/
	} /*langipchookerrors*/


static boolean langipcunhookerrors (callback savecallback) {
	
	shellpoperrorhook ();
	
	langcallbacks.errormessagecallback = (langerrormessagecallback) savecallback;
	
	fllangerror = false; /*make sure error flag is cleared*/
	
	hipcverb = nil;
	
	return (true);
	} /*langipcunhookerrors*/


boolean langipcpushparam (tyvaluerecord *valparam, typaramkeyword key, hdlverbrecord hverb) {
	
	/*
	convert our internal value format into the format used for interprocess
	communications and push the result on the accumulating packet.
	
	4/29/92 dmb: added fldontexempt flag to fix memory leak when sending rgb, pattern 
	and rect values
	
	2.1b2 dmb: don't push values w/no type
	
	2.1b3 dmb: rewrote; no longer use land paramrecords
	
	2.1b9 dmb: need special case for addressvaluetype
	
	2.1b12 dmb: take valparam by reference so we can zero it when when we 
	steal the handle.
	*/
	
	register tyvaluerecord *v = valparam;
	OSType type;
	void *pval = nil;
	Handle hval = nil;
	long len = 0;
	byte flval;
	bigstring bsval;
	AEDesc aelist;
	
	#if __powerc
	extended80 x80;
	#endif
	
	type = langgettypeid ((*v).valuetype);
	
	switch ((*v).valuetype) { /*set hval or pval/len*/
		
		case novaluetype: /*2.1b2*/
			return (true);
		
		case booleanvaluetype: /*our booleans are 2 bytes, but AE Booleans are 1 byte*/
			flval = bitboolean ((*v).data.flvalue);
			
			pval = &flval;
			
			len = sizeof (flval);
			
			break;
		
		case charvaluetype:
			pval = &(*v).data.chvalue;
			
			len = sizeof ((*v).data.chvalue);
			
			break;
		
		case intvaluetype:
			pval = &(*v).data.intvalue;
			
			len = sizeof ((*v).data.intvalue);
			
			break;
		
		case directionvaluetype:
			pval = &(*v).data.dirvalue;
			
			len = sizeof ((*v).data.dirvalue);
			
			break;
		
		case longvaluetype: /*all of these are normal, four-byte types	*/
		case datevaluetype: /*whose langtypeid matches their AE DescType*/
		case ostypevaluetype:
		case enumvaluetype:
		case fixedvaluetype:
		case pointvaluetype:
		case singlevaluetype:
			pval = &(*v).data.longvalue;
			
			len = sizeof ((*v).data.longvalue);
			
			break;
		
		case binaryvaluetype: /*binary values need their AE type extracted*/
			if (!copyvaluedata (v))
				return (false);
			
			hval = (*v).data.binaryvalue;
			
			pullfromhandle (hval, 0L, sizeof (OSType), &type);
			
			break;
		
		case objspecvaluetype: /*special case of nil objects*/
			hval = (*v).data.objspecvalue;
			
			if (hval == nil)
				type = typeNull;
			
			break;
		
		#if __powerc
		
			case doublevaluetype: {
				long double x = **(*v).data.doublevalue;
				 
				safeldtox80 (&x, &x80);
				 
				pval = &x80;
				 
				len = sizeof (x80);
				 
				break;
				}
		#else
		
			case doublevaluetype:
			
		#endif
		
		case stringvaluetype: 	/*all of these are normal, handle-based types */
		case rectvaluetype:		/*whose langtypeid matches their AE DescType  */
		case patternvaluetype:
		case rgbvaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
	#ifndef oplanglists
		case listvaluetype:
		case recordvaluetype:
	#endif
			hval = (*v).data.stringvalue;
			
			break;
		
	#ifdef oplanglists
		case listvaluetype:
		case recordvaluetype:
			if (!langipcconvertoplist (v, &aelist))
				return (false);
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				copydatahandle (&aelist, &hval);
				
			#else
			
				hval = aelist.dataHandle;
			
			#endif
			
			disposevaluerecord (*v, false); /*override exempt, below*/
		
			setnilvalue (v); /*clear out value*/
			
			break;
	
	#endif
	
		case addressvaluetype:	/*addresses need to be sent as strings for safety*/
			getaddresspath (*v, bsval);
			
			pval = bsval + 1;
			
			len = stringlength (bsval);
			
			break;
		
		default: /*not a supported type for ipc*/
			langerror (badoutgoingipctypeerror);
			
			return (false);
		
		} /*switch*/
	
	if (!landpushparam (hverb, (typaramtype) type, hval, pval, len, key))
		return (false);
	
	if (hval != nil) { /*handle was merged into hverb*/
		
		exemptfromtmpstack (v);
		
		initvalue (v, novaluetype); /*clear out value*/
		}
	
	return (true);
	} /*langipcpushparam*/


static boolean langipccoerceparam (AEDesc *param, tyvaluerecord *vreturned) {
	
	/*
	convert a value returned by UserLand IPC into lang.c's a valuerecord,
	consuming it.  for binary types, we save memory by stealing the binary 
	handle itself and setting the param record's handle to nil (instead of 
	making a copy of it).  if the caller is working with a copy of a parmater 
	record, then it must update the original after this call
	
	4/28/92 dmb: consume param
	
	8/12/92 dmb: special case for international text ('itxt')
	
	2.1b3 dmb: rewrote
	
	2.1b6 dmb: map a typeNull descriptor into a nil value, not a null objspec
	
	2.1b9 dmb: need special case for addressvaluetype
	
	4.1b6 dmb: handling incomine double floating values by asking the AE 
	manager to convert to extended for us.
	
	03/14/2002 PBS: AE OS X fix
	
	2002-11-15 AR: rewritten to make sure we _always_ consume param and also the data handle,
	but don't modify, i.e. set to nil, the dataHandle field of any AEDesc param record
	*/
	
	register AEDesc *p = param;
	register DescType desctype = (*p).descriptorType;
	register tyvaluerecord *v = vreturned;
	Handle hdata;	
	bigstring bs;
	tyvaluetype vtype;
	boolean fl = true;

	if (desctype == typeLongFloat) {  /*4.1b6 dmb: convert to extended so we understand it*/
		
		AEDesc pnew;
		OSErr errorcode;
		
		errorcode = AECoerceDesc (p, typeExtended, &pnew);
		
		AEDisposeDesc (p);
	
		if (oserror (errorcode))
			return (false);
		
		*p = pnew;
		
		desctype = (*p).descriptorType;
		}
	
	vtype = langgetvaluetype (desctype);

	#ifdef oplanglists
		
		if (vtype == listvaluetype || vtype == recordvaluetype) {	/*only case that actually needs AEDesc record*/
		
			fl = langipcconvertaelist (p, v);
			
			AEDisposeDesc (p);
			
			return (fl);
			}

	#endif
	
	#if TARGET_API_MAC_CARBON == 1
		
		copydatahandle (p, &hdata);	/*make a copy of the opaque data handle*/

	#else
	
		hdata = (*p).dataHandle;	/*get a reference to the data handle*/
		
		(*p).dataHandle = nil;	/*we own the original now, make sure AE manager won't dispose*/

	#endif
	
	AEDisposeDesc (p);
	
	initvalue (v, vtype); /*don't leave garbage in unassigned fields*/
	
	switch (vtype) {
		
		case booleanvaluetype:
			(*v).data.flvalue = numberfromhandle (hdata) != 0;
			
			break;
		
		case charvaluetype:
			(*v).data.chvalue = numberfromhandle (hdata);
			
			break;
		
		case intvaluetype:
			(*v).data.intvalue = numberfromhandle (hdata);
			
			break;
		
		case directionvaluetype:
			(*v).data.dirvalue = (tydirection) numberfromhandle (hdata);
			
			break;
		
		case longvaluetype: /*all of these are normal, four-byte types	*/
		case datevaluetype: /*whose langtypeid matches their AE DescType*/
		case ostypevaluetype:
		case enumvaluetype:
		case fixedvaluetype:
		case pointvaluetype:
		case singlevaluetype:
			(*v).data.longvalue = numberfromhandle (hdata);
			
			break;
		
		case objspecvaluetype:
			
			fl = setheapvalue (hdata, objspecvaluetype, v);	/*consumes hdata*/
			
			hdata = nil;
			
			break;
		
		#if __powerc
		
			case doublevaluetype: {
				long double ld;
				
				assert (gethandlesize (hdata) == sizeof (extended80));
				
				safex80told (*(extended80 **) hdata, &ld);
				
				fl = setdoublevalue (ld, v);
				
				break;
				}
		
		#else
		
			case doublevaluetype:
		
		#endif
		
		case stringvaluetype: 	/*all of these are normal, handle-based types */
		case rectvaluetype:		/*whose langtypeid matches their AE DescType  */
		case patternvaluetype:
		case rgbvaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
	#ifndef oplanglists
		case listvaluetype:
		case recordvaluetype:
	#endif
		case binaryvaluetype:
			
			fl = setheapvalue (hdata, vtype, v);	/*consumes hdata*/
			
			hdata = nil;
			
			break;
	
	/*
	#ifdef oplanglists	//now handled as a special case above
		case listvaluetype:
		case recordvaluetype:
			if (!langipcconvertaelist (p, v))
				return (false);
			
			break;
	#endif
	*/
		case addressvaluetype: /*addresses must be converted back from strings*/
			
			fl = setheapvalue (hdata, stringvaluetype, v);	/*consumes hdata*/
			
			hdata = nil;
			
			break;
		
		default:
			switch (desctype) {
				
				case pathtype: /*these are so ancient they're almost certain not to come up*/
				case stringtype:
					copyheapstring ((hdlstring) hdata, bs);
					
					fl = setstringvalue (bs, v);
					
					break;
				
				case typeNull:
					/*
					if (!setheapvalue (nil, objspecvaluetype, v))
						return  (false);
					*/
					
					(*v).valuetype = novaluetype;
					
					break;
				
				case typeTrue:
					setbooleanvalue (true, v);
					
					break;
				
				case typeFalse:
					setbooleanvalue (false, v);
					
					break;
				
				case typeIntlText:
					
					if (**(short **) hdata == smRoman) {
						
						pullfromhandle (hdata, 0, 4, nil); /*strip off script & language codes*/
						
						fl = setheapvalue (hdata, stringvaluetype, v);	/*consumes hdata*/
						}
					else {
						
						fl = setbinaryvalue (hdata, desctype, v);	/*consumes hdata*/
						}
			
					hdata = nil;
					
					break;
				
				default:
					
					fl = setbinaryvalue (hdata, desctype, v);	/*consumes hdata*/
			
					hdata = nil;

					break;
				}
			
			break;
		
		} /*switch*/

	disposehandle (hdata);	/*checks for nil*/
	
	return (fl);
	} /*langipccoerceparam*/


boolean setdescriptorvalue (AEDesc desc, tyvaluerecord *val) {
	
	/*
	if the descriptor corresponds to one of our types, set val to that type; 
	otherwise, set val to a binary value.
	
	the descriptor handle ends up on the temp stack, or is disposed 
	on error.
	
	11/6/92 dmb: use new langgoodbinarytype
	
	2.1b3 dmb: finally, we can just call langipccoerceparam -- cool!
	*/
	
	return (langipccoerceparam (&desc, val));
	} /*setdescriptorvalue*/


boolean valuetodescriptor (tyvaluerecord *val, AEDesc *desc) {
	
	/*
	2.1b3 dmb: create a descriptor corresponding to the given value 
	record, consuming it.  the resulting descriptor handle is not in
	the temp stack, and must be disposed by the caller.
	
	this routine could be made more efficient by extracting & using 
	langipcpushparam logic. note, however, that charactertypes are 
	treated differently. (this routine results in a 'TEXT' descriptor)
	
	2.1b6 dmb: special case for novaluetype -> typeNull
	
	2.1b14 dmb: special case for addressvaluetype, pass as string
	*/
	
	boolean fl;
	
	switch ((*val).valuetype) {
		
		case novaluetype:
			(*desc).descriptorType = typeNull;
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				newdescnull (desc, typeNull);
				
			#else
	
				(*desc).descriptorType = typeNull;
								
			#endif

			return (true);
	
		case addressvaluetype: { /*2.1b14*/
			bigstring bspath;
			OSType typeid = langgettypeid (addressvaluetype);
			
			getaddresspath (*val, bspath);
			
			return (!oserror (AECreateDesc (typeid, bspath + 1, stringlength (bspath), desc)));
			}
		
	#ifdef oplanglists
		case listvaluetype:
		case recordvaluetype:
			if (!langipcconvertoplist (val, desc))
				return (false);
			
			disposevaluerecord (*val, false);
			
			setnilvalue (val);
			
			return (true);
	#endif
		
		default:
			fl = coercetobinary (val);
			
			if (fl) {
				
				binarytodesc ((*val).data.binaryvalue, desc);
				
				exemptfromtmpstack (val);
				}
			else
				disposevaluerecord (*val, true);
			
			(*val).data.binaryvalue = nil; /*handle has been consumed*/
			
			return (fl);
		}
	} /*valuetodescriptor*/


static boolean langipcfileopen (hdlverbrecord hverb) {
	
	/*
	6/26/92 dmb: don't call shellsetsuperglobals; require that globals 
	already be properly set -- which they should be.  this avoids a dependency 
	on the Mac WindowList, which isn't valid when system handlers are running
	*/
	
	/*
	if (!shellsetsuperglobals ()) {
	*/
	
	if (roottable == nil) {
		
		landreturnerror (hverb, landnofileopenerror);
		
		return (false);
		}
	
	return (true);
	} /*langipcfileopen*/


static boolean langipcprocessstarted (void) {
	
	/*
	we don't want Frontier's menus to dim when serving another application's 
	doscript or trap call.
	*/
	
	processnotbusy ();
	
	return (true);
	} /*langipcprocessstarted*/


static boolean langipcruncode (hdlverbrecord hverb, hdltreenode hcode, hdlhashtable hcontext, boolean flstringresult) {
	
	/*
	2.1b12 dmb: shared code between trap and doscript verbs.
	
	we always consume hcode
	
	2.1b14 dmb: take hcontext parameter for special kernel call case
	
	4.0b7 dmb: fixed double-dispose memory bug when
	*/
	
	hdlprocessrecord hprocess;
	register hdlprocessrecord hp;
	register boolean fl;
	tyvaluerecord val;
	
	if (!newprocess (hcode, true, nil, 0L, &hprocess)) {
		
		langdisposetree (hcode);
		
		return (false);
		}
	
	hp = hprocess; /*copy into register*/
	
	(**hp).processrefcon = (long) hverb;
	
	(**hp).errormessagecallback = &langipcerrorroutine;
	
	(**hp).processstartedroutine = &langipcprocessstarted;
	
	(**hp).hcontext = hcontext;
	
	fl = processruncode (hprocess, &val);
	
	disposeprocess (hprocess);
	
	/*if no error occurred, return value to caller.  error hook handles error values*/
	
	if (fl) {
		
		if (flstringresult)
			coercetostring (&val);
		
		landstartreturn (hverb);
		
		if (!langipcpushparam (&val, returnedvaluekey, hverb)) {	// grabs handle unless it fails
		
			exemptfromtmpstack (&val);	// may or may not be a temp value now
			
			disposetmpvalue (&val);		// get rid of it
			}
		}
	
	return (fl);
	} /*langipcruncode*/


static boolean langipchandlerunscript (hdlverbrecord hverb) {
	
	/*
	6/26/91 dmb: instead of blocking events, block ourselves until 
	the previous script is done
	
	2/5/92 dmb: restoring errormessagecallback must occur _after_ exit label.
	
	2/15/93 dmb: don't fork if verb is direct
	*/
	
	Handle htext;
	callback savecallback;
	hdltreenode hcode;
	register boolean fl = false;
	
	if (!langipcfileopen (hverb))
		return (false);
	
	if (!landgettextparam (hverb, idscriptvalue, &htext))
		return (false);
	
	langipchookerrors (hverb, &savecallback);
	
	if (!langbuildtree (htext, false, &hcode)) /*syntax error*/
		goto exit;
	
	fl = langipcruncode (hverb, hcode, nil, true);
	
	exit:
	
	langipcunhookerrors (savecallback);
	
	return (fl); /*we will get here & return false if new thread didn't fork*/
	} /*langipchandlerunscript*/


static boolean issubroutineverb (hdlverbrecord hverb) {
	
	register hdlverbrecord hv = hverb;
	
	return ( /*((**hv).verbclass == kOSASuite) && */ ((**hv).verbtoken == kASSubroutineEvent));
	} /*issubroutineverb*/


static boolean iskernelverb (hdlverbrecord hverb) {
	
	register hdlverbrecord hv = hverb;
	
	return (issubroutineverb (hv) && ((**hv).verbclass == 'LAND'));
	} /*iskernelverb*/


static boolean handlerexpectsnames (hdltreenode hcode, AERecord *event, hdltreenode *hparams) {

	/*
	5.0d14 dmb: look at the parameter list of the script in hcode, and 
	decide of it should be called with named parameters. we answer true
	if it takes more than one parameter, and all but the first parameter 
	have 4-character names that match keys of parameters in the event
	*/
	
	hdltreenode hnames;
	hdltreenode nomad;
	tyvaluerecord osacode;
	bigstring bs;
	OSType key;
	DescType type;
	long size;
	
	if (hcode == nil)
		return (false);
	
	if ((**hcode).nodetype != moduleop)
		return (false);
	
	hcode = (**hcode).param1;
	
	if ((hcode == nil) || ((**hcode).nodetype == kernelop))
		return (false);
	
	if (isosascriptnode (hcode, &osacode))
		return (false);
	
	hcode = (**hcode).param2;
	
	if ((**hcode).nodetype != procop)
		return (false);
	
	hnames = (**hcode).param2;
	
	if (hnames == nil) // no formal params
		return (false);
	
	nomad = (**hnames).link; //advance to next name in list*/
	
	if (nomad == nil) // there was only one param
		return (false);
	
	while (true) { //step through remaining names in list
		
		if ((**nomad).nodetype == assignlocalop) {
			
			if (!langgetidentifier ((**nomad).param1, bs))
				break;
			}
		else {
			
			if (!langgetidentifier (nomad, bs))
				break;
			}
		
		if (stringlength (bs) != 4)
			break;
		
		if ((**nomad).nodetype != assignlocalop) { // not an optional param
			
			stringtoostype (bs, &key);
			
			if (AESizeOfKeyDesc (event, key, &type, &size) == errAEDescNotFound)
				break;
			}
		
		nomad = (**nomad).link; //advance to next name in list
		
		if (nomad == nil) { //reached the end of list
			
			*hparams = hnames;
			
			return (true);
			}
		} /*while*/
	
	return (false);
	} /*handlerexpectsnames*/


static boolean findnamedparam (hdltreenode hnames, bigstring bsname) {
	
	/*
	given the function parameter list, see if it contains the given (string4) name
	*/
	
	hdltreenode nomad;
	bigstring bs;
	
	for (nomad = (**hnames).link; nomad != nil; nomad = (**nomad).link) {
	
		if ((**nomad).nodetype == assignlocalop) {
			
			if (!langgetidentifier ((**nomad).param1, bs))
				continue;
			}
		else {
			
			if (!langgetidentifier (nomad, bs))
				continue;
			}
		
		if (equalidentifiers (bs, bsname)) //5.0a8 dmb: case-insensitive
			return (true);
		}
	
	return (false);
	} /*findnamedparam*/


boolean langipcbuildparamlist (hdltreenode hcode, hdlverbrecord hverb, hdltreenode *hparams) {
	
	/*
	take all of the parameters in the incoming verb hverb and build a code 
	tree for the corresponding lang paramter list
	
	2.1b5 dmb: special case for subroutine events
	
	2.1b12 dmb: push the root table to make sure address values will work
	
	3.0.1b2 dmb: for subroutine events, the direct parameter is optional
	
	5.0d14 dmb: take hcode parameter, so we can see of trap script takes 
	parameters by name. the first (direct) parameter can have any name. if
	all others are 4 characters long, and appear in the event, we use names.
	*/
	
	register short i;
	long ctparams;
	hdltreenode hlist = nil;
	AEDescList directparam;
	typaramrecord param;
	tyvaluerecord val;
	hdltreenode hparam;
	AERecord event, reply;
	AERecord *list;
	boolean flpushedroot;
	boolean flnamedparams;
	byte bskey [6];
	tyvaluerecord vkey;
	hdltreenode hname, hnamelist;
	OSErr err;
	register boolean fl = false;
	
	landsystem7geteventrecords (hverb, &event, &reply);
	
	if (issubroutineverb (hverb)) {
		
		err = AEGetKeyDesc (&event, keyDirectObject, typeAEList, &directparam);
		
		if (err == errAEDescNotFound) {
			
			*hparams = nil;
			
			return (true);
			}
		
		if (oserror (err))
			return (false);
		
		flnamedparams = false;
		
		list = &directparam;
		}
	else {
		
		flnamedparams = handlerexpectsnames (hcode, &event, &hnamelist);
		
		list = &event;
		}
	
	if (currenthashtable == nil)
		flpushedroot = pushhashtable (roottable);
	else
		flpushedroot = false;
	
	if (oserror (AECountItems (list, &ctparams)))
		goto exit;
	
	for (i = 1; i <= ctparams; i++) {
		
		if (!landsystem7getnthparam (list, i, &param))
			goto exit;
		
		if (!langipccoerceparam (&param.desc, &val))
			goto exit;
		
		exemptfromtmpstack (&val); /*its data is about to belong to code tree*/
		
		if (!newconstnode (val, &hparam))
			goto exit;
		
		if (flnamedparams && i > 1) { // 5.0d14 dmb
			
			ostypetostring (param.key, bskey);
			
			if (!findnamedparam (hnamelist, bskey)) { // trap isn't expecting this param
				
				langdisposetree (hparam);
				
				continue;
				}
			
			if (!setstringvalue (bskey, &vkey) || !newidnode (vkey, &hname)) {
				
				langdisposetree (hparam);
				
				goto exit;
				}
			
			exemptfromtmpstack (&vkey);
			
			if (!pushbinaryoperation (fieldop, hname, hparam, &hparam))
				goto exit;
			}
		
		if (hlist == nil)
			hlist = hparam;
		else
			pushlastlink (hparam, hlist);
		} /*for*/
	
	fl = true;
	
	exit:
	
	if (flpushedroot)
		pophashtable ();
	
	if (issubroutineverb (hverb)) /*need to dispose of parameter list record*/
		AEDisposeDesc (list);
	
	if (fl)
		*hparams = hlist; /*nil if there weren't any params*/
	else
		langdisposetree (hlist); /*checks for nil*/
	
	return (fl);
	} /*langipcbuildparamlist*/


static boolean langipcvalhascode (const tyvaluerecord *val) {
	
	/*
	4/30/92 dmb: created as part of new code extension support
	*/
	
	register const tyvaluerecord *v = val;
	hdltreenode hcode;
	
	switch ((*v).valuetype) {
		
		case externalvaluetype:
			return (langexternalvaltocode (*v, &hcode));
		
		case binaryvaluetype: /*maybe should qualify by binarytype*/
			return (true);
		
		default:
			return (false);
		}
	} /*langipcvalhascode*/


static boolean langipcfindtraphandler (hdlverbrecord hverb, tyvaluerecord *adr) {
	
	/*
	search the traps table for a handler for the given verb.
	
	if there's a script named after the verb at the top level of the traps table, use it.
	
	otherwise, look for a subtable named after the verb class, and see if it contains 
	the desired handler.
	
	2.1b1 dmb: don't leave trap address value in the temp stack.  langipchandletrapverb 
	now takes care of disposal
	
	2.1b5 dmb: added support for subroutine traps
	*/
	
	register hdlverbrecord hv = hverb;
	register hdlhashtable ht;
	hdlhashtable htable;
	tyvaluerecord val;
	bigstring bsclass;
	bigstring bstrap;
	boolean fl;
	hdlhashnode hnode;
	
	if (issubroutineverb (hv)) {
		
		if (!landgetstringparam (hv, keyASSubroutineName, bstrap)) 
			return (false);
		
		if (iskernelverb (hv)) { /*special case -- kernel verb specifies table*/
			
			if (!landgetlongparam (hv, keykerneltable, (long *) &htable))
				return (false);
			
			ht = htable;
			
			goto exit;
			}
		
		disablelangerror ();
		
		fl = langexpandtodotparams (bstrap, &htable, bstrap);
		
		enablelangerror ();
		
		if (!fl)
			return (false);
		
		if ((htable == nil) && !langsearchpathlookup (bstrap, &htable))
			return (false);
		
		if (!landpushstringparam (hv, bstrap, keyASSubroutineName)) /*substitute name only*/
			return (false);
		
		ht = htable;
		
		goto exit;
		}
	
	if (iachandlertable == nil) /*no traps table*/
		return (false);
	
	ostypetostring ((**hv).verbtoken, bstrap);
	
	if (hashtablelookup (iachandlertable, bstrap, &val, &hnode)) {
		
		if (langipcvalhascode (&val)) {
			
			ht = iachandlertable;
			
			goto exit;
			}
		}
	
	ostypetostring ((**hv).verbclass, bsclass);
	
	if (!hashtablelookup (iachandlertable, bsclass, &val, &hnode))
		return (false);
	
	if (!langexternalvaltotable (val, &htable, hnode))
		return (false);
	
	ht = htable; /*copy into register*/
	
	if (!hashtablelookup (ht, bstrap, &val, &hnode)) { /*not found literally; check for spaces*/
		
		if (!poptrailingwhitespace (bstrap)) /*nothing to pop off*/
			return (false);
		
		if (!hashtablelookup (ht, bstrap, &val, &hnode)) /*oh well*/
			return (false);
		}
	
	if (!langipcvalhascode (&val)) { /*make sure it can be called*/
		
		langparamerror (notfunctionerror, bstrap);
		
		return (false);
		}
	
	exit:
	
	if (!setaddressvalue (ht, bstrap, adr))
		return (false);
	
	exemptfromtmpstack (adr);
	
	return (true);
	} /*langipcfindtraphandler*/

/*
static setglobaltransactionid (long idtransaction) {
	
	tyvaluerecord val;
	
	setlongvalue (idtransaction, &val);
	
	if (iachandlertable != nil)
		hashtableassign (iachandlertable, "\ptrapTransaction", val);
	} /%setglobaltransactionid%/
*/

#if TARGET_API_MAC_CARBON == 0
#if GENERATINGCFM

#define BUILD_68K_ROUTINE_DESCRIPTOR(procInfo, m68kProcPtr)  \
	{								\
	_MixedModeMagic,				\
	kRoutineDescriptorVersion,		\
	kSelectorsAreNotIndexable,		\
	0,								\
	0,								\
	0,								\
	0,								\
	{								\
	{								\
	(procInfo),						\
	0,								\
	kM68kISA,						\
	kProcDescriptorIsAbsolute |		\
	kUseCurrentISA,					\
	(ProcPtr)(m68kProcPtr),			\
	0,								\
	0,								\
	},								\
	},								\
	}

static RoutineDescriptor UCMDDesc = BUILD_68K_ROUTINE_DESCRIPTOR (uppAEEventHandlerProcInfo, nil);

#endif
#endif


static boolean langipchandletrapverb (hdlverbrecord hverb, boolean *flfoundhandler) {
	
	/*
	10/24/91 dmb: support 2-level traps table using langipcfindtraphandler routine
	
	12/2/91 dmb: hooked up new langipcprocessstarted to prevent menu dimming 
	and recalculation
	
	9/22/92 dmb: removed call to setglobaltransactionid. this was never documented, 
	and has been superceeded by the getEventAttribute verb
	
	2/15/93 dmb: don't fork if verb is direct
	
	4/1/93 dmb: added code to handle executing osa scripts. also moved landstartreturn
	block to before the exit rather than after it.
	
	2.1b13 dmb: oops, forgot to set last parameter to new langipcruncode to false.
	
	2.1b14 dmb: a special case for the kernelverb -- actual context is passed along. 
	the context must also be used while evaluating address parameters.
	*/
	
	register hdlverbrecord hv = hverb;
	bigstring bsverb;
	register boolean fl = false;
	tyvaluerecord val;
	callback savecallback;
	hdltreenode hfunctioncall;
	hdltreenode hparamlist;
	hdltreenode hcode;
	hdlhashtable htable;
	hdlhashtable hcontext = nil;
	long refcon;
	tyvaluerecord vhandler;
	tyvaluerecord osacode;
	//Handle hthread = nil;
	hdlhashnode handlernode;
	AppleEvent event, reply;
	
	*flfoundhandler = false;
	
	if (!langipcfileopen (hv))
		return (false);
	
	refcon = (**hv).landrefcon;
	
	if (refcon == 0) { /*haven't already looked it up*/
		
		if (!langipcfindtraphandler (hv, &val)) /*no trap handler exists for this verb*/
			return (false);
		}
	else {
		initvalue (&val, addressvaluetype);
		
		val.data.addressvalue = (hdlstring) refcon;
		}
	
	pushvalueontmpstack (&val); /*make sure it's cleaned up in the end*/
	
	if (!getaddressvalue (val, &htable, bsverb))
		return (false);
	
	if (!hashtablelookupnode (htable, bsverb, &handlernode))
		return (false);
	
	vhandler = (**handlernode).val;
	
	*flfoundhandler = true;
	
	landsystem7geteventrecords (hv, &event, &reply);
	
	if (vhandler.valuetype == binaryvaluetype) {
		
		Handle h = vhandler.data.binaryvalue;
		
	#ifdef flcomponent
		if (**(OSType **) h != 'UCMD') {
			
			/*
			langparamerror (notucmderror, bsverb);
			*/
			
			fl = evaluateosascriptevent (&vhandler, &event, &reply);
			
			return (fl);
			}
	#endif
		
		loadhandleremains (sizeof (OSType), h, &h);  //skip binarytype
		
		lockhandle ((Handle) h);
		
		/*
		bundle {
			RoutineDescriptor ucmddesc = BUILD_ROUTINE_DESCRIPTOR (uppAEEventHandlerProcInfo, nil);
			ucmddesc.routineRecords [0].procDescriptor = (ProcPtr) *h;	// fill in the blank
			fl = !oserror (CallAEEventHandlerProc (&ucmddesc, &event, &reply, 0L));
			}
		*/
		
		//MakeDataExecutable (*h, gethandlesize (h)); //5.0.2 dmb
		
		/*
		fl = !oserror ((*(AEEventHandlerProcPtr) *h) (&event, &reply, 0L));
		*/
		
		
		#if TARGET_API_MAC_CARBON == 1
		
			langerrormessage ("\pUCMDs are not supported in the Carbon version.");
			
		#else
		
			#if __powerc
				fl = !oserror (CallAEEventHandlerProc ((UniversalProcPtr) (*h), &event, &reply, 0L));
			#else
				fl = !oserror (CallAEEventHandlerProc ((tyeventhandler) (*h), &event, &reply, 0L));
			#endif
		
		#endif
		
		unlockhandle ((Handle) h);
		
		disposehandle (h);
		
		return (fl);
		}
	
	/*
	setglobaltransactionid ((**hv).idtransaction);
	*/
	
	/*build a code tree and call the handler, with our error hook in place*/
	
	langipchookerrors (hv, &savecallback);
	
	hcode = nil;
	
	if ((**htable).valueroutine == nil) { /*not a kernel table*/
		
		if (!langexternalvaltocode (vhandler, &hcode))
			goto exit;
		
		if (hcode == nil) { /*needs compilation*/
			
			if (!langcompilescript (handlernode, &hcode))
				goto exit;
			}
		
		#ifdef flcomponent
	
			if (isosascriptnode (hcode, &osacode)) {
				
				fl = evaluateosascriptevent (&osacode, &event, &reply);
				
				goto exit;
				}
			
		#endif
		}
	
	if (iskernelverb (hv)) { /*special case -- kernel verb specifies context*/
		
		if (!landgetlongparam (hv, keycurrenttable, (long *) &hcontext))
			goto exit;
		}
	
	if (!pushfunctionreference (val, &hfunctioncall))
		goto exit;
	
	if (hcontext != nil)
		pushhashtable (hcontext);
	
	fl = langipcbuildparamlist (hcode, hv, &hparamlist);
	
	if (hcontext != nil)
		pophashtable ();
	
	if (!fl) {
		
		langdisposetree (hfunctioncall);
		
		goto exit;
		}
	
	if (!pushfunctioncall (hfunctioncall, hparamlist, &hcode)) /*consumes input parameters*/
		goto exit;
	
	if (!pushbinaryoperation (moduleop, hcode, nil, &hcode)) /*needs this level*/
		goto exit;
	
	fl = langipcruncode (hv, hcode, hcontext, false);
	
	exit:
	
	langipcunhookerrors (savecallback);
	
	return (fl);
	} /*langipchandletrapverb*/


#if TARGET_API_MAC_OS8

static boolean langipcgetmenuarrayverb (hdlverbrecord hverb) {
	
	long id;
	Handle h = nil; /*4.1b2 dmb*/
	short firstmenuresource;
	
	if (!langipcfileopen (hverb))
		return (false);
	
	if (!landgetlongparam (hverb, idmenuprogram, &id))
		return (false);
	
	if (!landgetintparam (hverb, idstartingresource, &firstmenuresource))
		return (false);
	
	if (!langipcgetmenuarray (id, firstmenuresource, false, &h)) {
		
		landreturnerror (hverb, outofmemoryerror); /*ran out of memory in menu server*/
		
		return (false);
		}
	
	return (landreturnbinary (hverb, h));
	} /*langipcgetmenuarrayverb*/


static boolean langipcgetmenuhandleverb (hdlverbrecord hverb) {
	
	long id;
	short ixarray;
	Handle h;
	
	if (!langipcfileopen (hverb))
		return (false);
	
	if (!landgetlongparam (hverb, idmenuprogram, &id))
		return (false);
	
	if (!landgetintparam (hverb, idarrayindex, &ixarray))
		return (false);
	
	if (!langipcgetmenuhandle (id, ixarray, &h)) {
		
		landreturnerror (hverb, outofmemoryerror);
		
		return (false);
		}
	
	return (landreturnbinary (hverb, h));
	} /*langipcgetmenuhandleverb*/


static boolean langipcrunmenuitemverb (hdlverbrecord hverb) {
	
	long id;
	short idmenu, iditem;
	long refcon;
	register hdlverbrecord hv = hverb;
	
	if (!langipcfileopen (hverb))
		return (false);
	
	if (!landgetlongparam (hv, idmenuprogram, &id))
		return (false);
	
	if (!landgetintparam (hv, idmenuidvalue, &idmenu))
		return (false);
	
	if (!landgetintparam (hv, idmenuitemvalue, &iditem))
		return (false);
	
	if (!langipcrunitem (id, idmenu, iditem, &refcon)) {
		
		landreturnerror (hv, outofmemoryerror);
		
		return (false);
		}
	
	return (landreturnlong (hv, refcon));
	} /*langipcrunmenuitemverb*/


static boolean langipckillscriptverb (hdlverbrecord hverb) {
	
	/*
	the long parameter shoud be the refcon returned by runmenuitem.  since 
	we count count on perfect synchonization, we shouldn't assume that the 
	process still exists, so we're using the code instead of the process 
	handle itself.  the down side is that it won't find the process if the 
	current process list isn't the original one.  we can make this more 
	robust if necessary.
	
	7/17/92 dmb: refcon parameter uses directparamkey, not idmenuprogram
	*/
	
	long refcon;
	boolean fl;
	
	if (!langipcfileopen (hverb))
		return (false);
	
	if (!landgetlongparam (hverb, directparamkey, &refcon))
		return (false);
	
	fl = processdisposecode ((hdltreenode) refcon);
	
	return (landreturnboolean (hverb, fl));
	} /*langipckillscriptverb*/

#endif


static pascal boolean langipchandleverb (hdlverbrecord hverb) {
	
	//
	// 2006-07-25 creedon: FSRef-ized
	//
	// 4.1b7 dmb: allow a specific root file to be targed with the "subject" 
	// event attribute. Bug: right now, langipcfindhandler doesn't have this 
	// logic. So if an event doesn't have a handler in this root, landsystem7
	// will return eventnothandled even if the target root does have a handler.
	//
	// 1991-10-29 dmb: let traps have a chance at runscripttoken before executing built-in code
	//
	
	boolean fl;
	boolean flfoundhandler;
	AEDesc subject;
	WindowPtr wroot = nil;
	
	oserror ( noErr ); // make sure it's clear
	
	if ( landgetverbattr ( hverb, 'subj', typeFSS, &subject ) == noErr ) {
		
		FSSpec fs;
		FSRef fsref;
		Handle h;
		tyfilespec fst;
		
		clearbytes ( &fst, sizeof ( fst ) );
		
		copydatahandle (&subject, &h);
		
		fs = **(FSSpec **) h;
		
		disposehandle (h);
		
		FSpMakeFSRef ( &fs, &fsref );
		
		(void) macmakefilespec (&fsref, &fst);
		
		wroot = shellfindfilewindow ( &fst );
		
		if (wroot == nil) {
		
			// landreturnerror 
			
			}
		else {
			
			(**hverb).landrefcon = 0; // don't use trap script found in other root
			
			shellpushglobals (wroot);
			
			(*shellglobals.setsuperglobalsroutine) ();
			}
		}
	
	switch ((**hverb).verbtoken) {
		
		case idrunning:
			fl = landreturnboolean (hverb, true); // for old IACTK compatibility
			
			break;
		
		case idwho:
			fl = landreturnlong (hverb, iswho);
			
			break;
		
		case runscripttoken:
			fl = langipchandletrapverb (hverb, &flfoundhandler); // let traps have a shot at it first
			
			if (!flfoundhandler)
				fl = langipchandlerunscript (hverb);
			
			break;
		
		// case schedulescripttoken:
		// 	fl = langipchandleschedulescript (hverb);
		//	
		// 	break;
		
		default:
			fl = langipchandletrapverb (hverb, &flfoundhandler);
			
			if (!flfoundhandler) // getoserror () == noErr
				landreturnerror (hverb, undefinedverberror);
			
			break;
		}
	
	if (!fl) {
		
		if (landgeterror () == noErr)
			landseterror (getoserror ());
		}
	
	if (wroot != nil) {
		
		shellpopglobals ();
		
		(*shellglobals.setsuperglobalsroutine) ();
		}

	return (fl);
	
	} // langipchandleverb


static pascal boolean langipcfindhandler (hdlverbrecord hverb) {
	
	/*
	4/29/92 dmb: make sure we can actually handle this wildcard verb
	*/
	
	tyvaluerecord val;
	
	if (!langipcfindtraphandler (hverb, &val)) /*no trap handler exists for this verb*/
		return (false);
	
	(**hverb).landrefcon = (long) val.data.addressvalue;
	
	return (true);
	} /*langipcfindhandler*/


static boolean setbinarysymbolval (hdlhashtable htable, bigstring bs, Handle x, OSType typeid) {
	
	/*
	encapsulate some code that appears in the file more than once
	*/
	
	tyvaluerecord val;
	
	if (!setbinaryvalue (x, typeid, &val))
		return (false);
	
	if (!langsetsymboltableval (htable, bs, val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	return (true);
	} /*setbinarysymbolval*/


static void langipcexpandnetaddress (tyipcaddress *ipcaddress) {
	
	/*
	fill out all fields in the ipc network address, seeded from netaddress
	*/
	
	register tyipcaddress *adr = ipcaddress;
	
	(*adr).flnetwork = true;
	
	copystring ((*adr).netaddress.target.name.name, (*adr).appname); /*macnetglobals.location;*/
	
	if ((*adr).netaddress.target.name.portKindSelector == ppcByString)
		moveleft ((*adr).netaddress.target.name.u.portTypeStr + 1, &(*adr).appid, sizeof (OSType));
	else
		(*adr).appid = (*adr).netaddress.target.name.u.port.portCreator;
	} /*langipcexpandnetaddress*/


static boolean getnetaddress (hdlhashtable htable, bigstring bsvarname, tyipcaddress *ipcaddress) {
	
	tyvaluerecord val;
	long ix = sizeof (OSType);
	hdlhashnode hnode;
	
	if (!hashtablelookup (htable, bsvarname, &val, &hnode))
		return (false);
	
	if (val.valuetype != binaryvaluetype)
		return (false);
	
	if (getbinarytypeid (val.data.binaryvalue) != 'targ')
		return (false);
	
	return (loadfromhandle (val.data.binaryvalue, &ix, sizeof (tynetworkaddress), &(*ipcaddress).netaddress));
	} /*getnetaddress*/
	
	
boolean langipcbrowsenetwork (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	2/11/92 dmb: added support for prompt
	
	6/8/92 dmb: use landgeterror to make sure that user cancel doesn't terminate 
	the script
	*/
	
	register hdltreenode hp1 = hparam1;
	tyapplicationid appid;
	tyipcaddress ipcaddress;
	hdlhashtable htable;
	bigstring bs;
	Handle x;
	bigstring bsprompt;
	short pnum = 1;
	
	setbooleanvalue (false, vreturned);
	
	setemptystring (bsprompt);
	
	if (langgetparamcount (hp1) != 2) { /*new calling conventions: require prompt*/
		
		if (!getstringvalue (hp1, pnum++, bsprompt))
			return (false);
		}
	
	if (!getostypevalue (hp1, pnum++, &appid))
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hp1, pnum++, &htable, bs))
		return (false);
	
	if (!getnetaddress (htable, bs, &ipcaddress))
		clearbytes (&ipcaddress, sizeof (ipcaddress));
	
	if (!landbrowsenetworkapps (bsprompt, &ipcaddress.netaddress, appid))
		return (landgeterror () == userCanceledErr);
	
	/*
	langipcexpandnetaddress (&ipcaddress);
	*/
	
	if (!newfilledhandle (&ipcaddress.netaddress, sizeof (tynetworkaddress), &x))
		return (false);
	
	if (!setbinarysymbolval (htable, bs, x, 'targ')) /*'netx'*/
		return (false);
	
	(*vreturned).data.flvalue = true;
	
	return (true);
	} /*langipcbrowsenetwork*/


static boolean getipcaddressvalue (hdltreenode hparam1, short pnum, tyipcaddress *ipcaddress) {
	
	//
	// 2006-06-25 creedon: FSRef-ized
	//
	// 5.0d19 dmb: handle 'psn ' binary address values
	//
	// 1993-04-09 dmb: if the address parameter is zero, send to self
	// 
	// 1993-02-12 dmb: always use landglobals for find our own application id
	//
	
	register tyipcaddress *padr = ipcaddress;
	tyvaluerecord val;
	
	if (!getparamvalue (hparam1, pnum, &val))
		return (false);
	
	if (val.valuetype == binaryvaluetype) {
		
		long ix = sizeof (OSType); // skip the binary type
		
		switch (getbinarytypeid (val.data.binaryvalue)) {
			
			case 'psn ':
				(*padr).appid = 0;
				
				return (loadfromhandle (val.data.binaryvalue, &ix, sizeof (ProcessSerialNumber), &(*padr).appprocessid));
			
			case 'targ':
				if (!loadfromhandle (val.data.binaryvalue, &ix, sizeof (tynetworkaddress), &(*padr).netaddress))
					return (false);
				
				langipcexpandnetaddress (padr);
				
				return (true);
			}
		}
	
	if (val.valuetype == addressvaluetype) {
		/*
		hdlhashtable htable;
		bigstring bs;
		*/
		
		(*padr).appid = (**landgetglobals ()).applicationid;
		
		(*padr).flcodeval = true;
		
		/*
		(*padr).addressval = val;
		
		if (!getaddressvalue (val, &htable, bs))
			return (false);
		
		if (!hashtablelookup (htable, bs, &val))
			return (false);
		*/
		
		(*padr).val = val;
		
		return (true);
		}
	
	if (val.valuetype == stringvaluetype) {
		bigstring bsaddress;
		bigstring bs;
		OSErr errcode;
		
		texthandletostring ((Handle) val.data.stringvalue, bsaddress);
		
		if (stringfindchar (':', bsaddress)) { /*maybe a network address*/
			
			if (landstring2networkaddress (bsaddress, &(*padr).netaddress)) {
				
				langipcexpandnetaddress (padr);
				
				return (true);
				}
			
			errcode = landgeterror ();
			
			if (errcode < 0) {
				
				langgetstringlist (onthisnetstring, bs);
				
				lang2paramerror (iacaddresserror, bsaddress, bs); /*can't make sense out of network address*/
				
				return (false);
				}
			}
		
		(*padr).appid = 0;
		
		if (findrunningapplication (&(*padr).appid, bsaddress, &(*padr).appprocessid)) { /*got an application name*/
			
			copystring (bsaddress, (*padr).appname);
			
			return (true);
			}
		
		if (stringlength (bsaddress) != 4) { /*can't be coerced into a string4*/
			
			langgetstringlist (onthismacstring, bs);
			
			lang2paramerror (iacaddresserror, bsaddress, bs); /*can't make sense out of local address*/
			
			return (false);
			}
		}
	
	if (val.valuetype == filespecvaluetype) { /*4.1b7 dmb: send to another open root*/
		
		tyfilespec fs = **val.data.filespecvalue;
		
		if (shellfindfilewindow (&fs) == nil) {
		
			bigstring bs;
			
			getfsfile ( &fs, bs );
			
			langparamerror (dbnotopenederror, bs );
			
			return (false);
			}
		
		(*padr).flrootval = true;
		
		(*padr).val = val;
		
		val.data.ostypevalue = 0; /*pick up app signature as event target*/
		}
	else {
	
		if (!coercetoostype (&val))
			return (false);
		}
	
	if (val.data.ostypevalue == 0)
		(*padr).appid = (**landgetglobals ()).applicationid;
	else
		(*padr).appid = val.data.ostypevalue;
	
	findrunningapplication (&val.data.ostypevalue, (*padr).appname, nil); /*set if running*/
	
	return (true);
	} /*getipcaddressvalue*/


boolean langipcgetaddressvalue (hdltreenode hparam1, tyvaluerecord *vreturned) {
#pragma unused (hparam1)

	return (setbooleanvalue (false, vreturned));
	} /*langipcgetaddressvalue*/


static boolean getlistpositionvalue (hdltreenode hparam1, short pnum, tyvaluerecord *posval) {
	
	/*
	get a list position parameter -- either an index (number) or a keyword (string4)
	
	a new ae list is created. the original oplist is untouched.
	*/
	
	tyvaluerecord *v = posval;
	
	if (!getparamvalue (hparam1, pnum, v))
		return (false);
	
	return (coercetolistposition (v));
	} /*getlistpositionvalue*/


boolean langipcconvertoplist (const tyvaluerecord *vlist, AEDesc *list) {
	
	/*
	5.0a24 dmb: convert an op list or record to an AE list. the op list 
	is left intact
	*/
	
	boolean flrecord = (*vlist).valuetype == recordvaluetype;
	tyvaluerecord vitem;
	AEDesc item;
	long ix, ctitems;
	bigstring bskey;
	OSType key;
	OSErr err;
	
	if (!langgetlistsize (vlist, &ctitems))
		return (false);
	
	if (oserror (AECreateList (nil, 0, flrecord, list)))
		return (false);
	
	for (ix = 1; ix <= ctitems; ++ix) { /*process each item in the list*/
		
		if (!langgetlistitem (vlist, ix, bskey, &vitem))
			goto error;
		
		stringtoostype (bskey, &key);
		
		if (!valuetodescriptor (&vitem, &item)) // consumes value
			goto error;
		
		if (flrecord)
			err = AEPutKeyDesc (list, key, &item);
		else
			err = AEPutDesc (list, 0, &item);
		
		AEDisposeDesc (&item);
		
		if (oserror (err))
			goto error;
		}
	
	return (true);
		
	error: {
		
		AEDisposeDesc (list);
		
		return (false);
		}
	} /*langipcconvertoplist*/


boolean langipcconvertaelist (const AEDesc *list, tyvaluerecord *vlist) {
	
	/*
	5.0a24 dmb: convert an AE list or record to a usertalk oplist.
	
	a new oplist is created on the temp stack. the original aelist is untouched.
	
	5.0b7 dmb: disposevaluerecord, fix leak
	*/
	
	boolean flrecord = (*list).descriptorType == typeAERecord;
	hdllistrecord hlist;
	AEDesc item;
	OSType key;
	bigstring bskey;
	tyvaluerecord vitem;
	long ix, ctitems;
	boolean fl;
	
	if (oserror (AECountItems (list, &ctitems)))
		return (false);
	
	if (!opnewlist (&hlist, flrecord))
		return (false);
	
	for (ix = 1; ix <= ctitems; ++ix) { // process each item in the list*/
	
		if (oserror (AEGetNthDesc (list, ix, typeWildCard, &key, &item)))
			goto error;
		
		if (!setdescriptorvalue (item, &vitem)) // consumes item
			goto error;
		
		ostypetostring (key, bskey);
		
		if (flrecord)
			fl = langpushlistval (hlist, bskey, &vitem);
		else
			fl = langpushlistval (hlist, nil, &vitem);
		
		disposevaluerecord (vitem, true);
		
		if (!fl)
			goto error;
		}
	
	return (setheapvalue ((Handle) hlist, flrecord? recordvaluetype: listvaluetype, vlist));
	
	error: {
	
		opdisposelist (hlist);
		
		return (false);
		}
	} /*langipcconvertaelist*/


void binarytodesc (Handle hbinary, AEDesc *desc) {
	
	/*
	6/24/92 dmb: need special-case for boolean values; must extract extra 
	(high order) byte
	
	8/11/92 dmb: also special-case character values; they must go out as standard 
	typeChar descriptors.
	
	2006-04-17 aradke: Must byte-swap valtype id extracted from binary value on Intel Macs.
	*/
	
	register AEDesc *d = desc;

	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/	
		
		Handle hcopy;
		DescType dtype;
		
		copyhandle (hbinary, &hcopy);
		
		pullfromhandle (hcopy, 0L, sizeof (DescType), &dtype);
		
		disktomemlong (dtype);
		
		newdescwithhandle (d, dtype, hcopy);
		
		disposehandle (hcopy);
	
	#else
	
		(*d).dataHandle = hbinary;
	
		pullfromhandle ((*d).dataHandle, 0L, sizeof (DescType), &(*d).descriptorType);
		
		disktomemlong ((*d).descriptorType);
	
	#endif
	
	
	switch ((*d).descriptorType) {
		
	#ifndef version42orgreater
	
		case 'bool': {
		
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				Handle hcopy;
				
				copydatahandle (d, &hcopy);
				
				pullfromhandle (hcopy, 0L, 1L, nil);
				
				putdeschandle (d, (*d).descriptorType, hcopy);
				
				disposehandle (hcopy);
			
			#else
	
				pullfromhandle ((*d).dataHandle, 0L, 1L, nil);
			
			#endif
			
			break;
			}
	
	#endif
		
		case 'char':
			(*d).descriptorType = 'TEXT';
			
			break;
		}
	} /*binarytodesc*/


static boolean stealbinaryhandle (tyvaluerecord *val, Handle *x) {
	
	if (exemptfromtmpstack (val)) {
		
		*x = (*val).data.binaryvalue;
		
		return (true);
		}
	
	return (copyhandle ((*val).data.binaryvalue, x));
	} /*stealbinaryhandle*/

			
static boolean getbinarylistdesc (boolean flrecord, tyvaluerecord val, AEDescList *listdesc) {
	
	/*
	5.0a25 dmb: closer to 4.x code, must let binary list values stay as such
	*/
	
	Handle x;
	
	switch (val.valuetype) {
		
		case binaryvaluetype:
			
			if (!stealbinaryhandle (&val, &x))
				return (false);
			
			binarytodesc (x, listdesc);
			
			return (true);
		
		case listvaluetype:
		case recordvaluetype:
			#ifdef version5orgreater
				return (langipcconvertoplist (&val, listdesc));
			#else
			
				#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
				
					{
					Handle hcopy;
					
					copydatahandle (listdesc, &hcopy);
					
					if (!stealbinaryhandle (&val, &hcopy))
						return (false);
						
					disposehandle (hcopy);
					}
				
				#else
				
					if (!stealbinaryhandle (&val, &(*listdesc).dataHandle))
						return (false);
								
				#endif
					
				(*listdesc).descriptorType = typeAERecord;
				
				return (true);
			#endif
		
		case novaluetype:
		case longvaluetype:
			
			if (val.data.longvalue == 0) { /*uninitialized variable - create empty list*/
				
				return (!oserror (AECreateList (nil, 0, flrecord, listdesc)));
				}
			
			break;
		
		default:
			/* do nothing */
			break;
		}
	
	langerror (badipclistvalerror);
	
	return (false);
	} /*getbinarylistdesc*/


boolean langipcputlistitem (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	10/14/91 dmb: created.
	
	verb: putlistitem (value, key/index, @list); returns boolean.  adds an 
	item to an AEDescList or an AERecord, creating the list if it doesn't 
	already exist.
	
	value is any database value.
	
	list is the address of a binary value that should be an AEList.  if no 
	object exists at list, a new binary value is created.
	
	if second parameter is an index, list is assumed to be (or created as) a 
	normal descriptor list.  if the second parameter is a key, list 
	is assumed to be (or created as) an AE record.
	*/
	
	register hdltreenode hp1 = hparam1;
	tyvaluerecord posval;
	tyvaluerecord itemval;
	tyvaluerecord listval;
	hdlhashtable htable;
	bigstring bsname;
	boolean flrecord;
	AEDesc itemdesc;
	AEDescList listdesc;
	OSErr errcode;
	hdlhashnode hnode;
	
	setbooleanvalue (false, vreturned); /*default return*/
	
	if (!getbinaryparam (hp1, 1, &itemval))
		return (false);
	
	if (!getlistpositionvalue (hp1, 2, &posval))
		return (false);
	
	#ifdef oplanglists
		if (posval.valuetype == stringvaluetype)
			coercetoostype (&posval);
	#endif
	
	flnextparamislast = true;
	
	if (!getvarparam (hp1, 3, &htable, bsname)) /*this list address*/
		return (false);
	
	if (isemptystring (bsname)) {
		
		langerror (binaryrequirederror);
		
		return (false);
		}
	
	flrecord = posval.valuetype == ostypevaluetype; /*was passed a key*/
		
	if (hashtablelookup (htable, bsname, &listval, &hnode)) { /*something there; make sure it's a list*/
		
		if (!getbinarylistdesc (flrecord, listval, &listdesc))
			return (false);
		}
	else {
		if (oserror (AECreateList (nil, 0, flrecord, &listdesc)))
			return (false);
		}
	
	binarytodesc (itemval.data.binaryvalue, &itemdesc); /*steal the handle -- it's a temp*/
	
	if (flrecord)
		errcode = AEPutKeyDesc (&listdesc, posval.data.ostypevalue, &itemdesc);
	else
		errcode = AEPutDesc (&listdesc, posval.data.longvalue, &itemdesc);
	
	if (oserror (errcode)) {
		
		AEDisposeDesc (&listdesc);
		
		return (false);
		}
	
	/*don't dispose itemdesc handle -- it's in the temp stack*/
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle hcopy;
		boolean fl = false;
		
		copydatahandle (&listdesc, &hcopy);
		
		fl = setbinarysymbolval (htable, bsname, hcopy, listdesc.descriptorType);
		
		//disposehandle (hcopy);
		
		if (!fl)
			return (false);
		}
	
	#else
	
		if (!setbinarysymbolval (htable, bsname, listdesc.dataHandle, listdesc.descriptorType))
			return (false);
	
	#endif
	
	(*vreturned).data.flvalue = true;
	
	return (true);
	} /*langipcputlistitem*/


boolean langipcgetlistitem (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	11/6/92 dmb: use new langgoodbinarytype
	*/
	
	register hdltreenode hp1 = hparam1;
	tyvaluerecord posval;
	tyvaluerecord itemval;
	tyvaluerecord listval;
	hdlhashtable htable;
	bigstring bsname;
	boolean flrecord;
	AEDesc itemdesc;
	AEDescList listdesc;
	OSErr errcode;
	DescType key;
	
	setbooleanvalue (false, vreturned); /*default return*/
	
	if (!getbinaryparam (hp1, 1, &listval))
		return (false);
	
	if (!getlistpositionvalue (hp1, 2, &posval))
		return (false);
	
	#ifdef oplanglists
		if (posval.valuetype == stringvaluetype)
			coercetoostype (&posval);
	#endif
	
	flnextparamislast = true;
	
	if (!getvarparam (hp1, 3, &htable, bsname)) /*this item address*/
		return (false);
	
	if (isemptystring (bsname)) {
		
		langerror (binaryrequirederror);
		
		return (false);
		}
	
	flrecord = posval.valuetype == ostypevaluetype; /*was passed a key*/
	
	if (!getbinarylistdesc (flrecord, listval, &listdesc))
		return (false);
	
	if (flrecord)
		errcode = AEGetKeyDesc (&listdesc, posval.data.ostypevalue, typeWildCard, &itemdesc);
	else
		errcode = AEGetNthDesc (&listdesc, posval.data.longvalue, typeWildCard, &key, &itemdesc);
	
	AEDisposeDesc (&listdesc);
	
	if (errcode == errAEDescNotFound) /*not fatal; just return false value*/
		return (true);
	
	if (oserror (errcode))
		return (false);
	
	if (!setdescriptorvalue (itemdesc, &itemval))
		return (false);
	
	/*
	if (!setbinaryvalue (itemdesc.dataHandle, itemdesc.descriptorType, &itemval))
		return (false);
	
	valuetype = langgetvaluetype (itemdesc.descriptorType);
	
	if (langgoodbinarytype (valuetype)) { /%unpackable%/
		
		if (!coercevalue (&itemval, valuetype))
			return (false);
		}
	*/
	
	if (!langsetsymboltableval (htable, bsname, itemval))
		return (false);
	
	exemptfromtmpstack (&itemval);
	
	(*vreturned).data.flvalue = true;
	
	return (true);
	} /*langipcgetlistitem*/


boolean langipccountlistitems (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	4/15/92 dmb: created.
	
	6/24/92 dmb: resurrected.
	
	12/21/92 dmb: don't special-case binarytype; we have real list types now.
	*/
	
	register hdltreenode hp1 = hparam1;
	tyvaluerecord listval;
	hdlhashtable htable;
	bigstring bsname;
	AEDescList listdesc;
	OSErr errcode;
	hdlhashnode hnode;
	
	setlongvalue (0, vreturned); /*default return*/
	
	flnextparamislast = true;
	
	if (!getvarvalue (hp1, 1, &htable, bsname, &listval, &hnode)) /*the list*/
		return (false);
	
	if (!getbinarylistdesc (false, listval, &listdesc))
		return (false);
	
	errcode = AECountItems (&listdesc, &(*vreturned).data.longvalue);
	
	AEDisposeDesc (&listdesc);
	
	return (!oserror (errcode));
	} /*langipccountlistitems*/


boolean langipcsettimeout (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	set the timeout for all outgoing events in this thread
	*/
	
	long timeout;
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 1, &timeout))
		return (false);
	
	setbooleanvalue (landsettimeout (timeout), vreturned);
	
	return (true);
	} /*langipcsettimeout*/


boolean langipcsettransactionid (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	set the transaction id for all outgoing events in this thread
	*/
	
	long id;
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 1, &id))
		return (false);
	
	setbooleanvalue (landsettransactionid (id), vreturned);
	
	return (true);
	} /*langipcsettransactionid*/


boolean langipcsetinteractionlevel (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	set the interaction level for all outgoing events in this thread
	*/
	
	short level;
	
	flnextparamislast = true;
	
	if (!getintvalue (hparam1, 1, &level))
		return (false);
	
	setbooleanvalue (landsetinteractionlevel (level), vreturned);
	
	return (true);
	} /*langipcsetinteractionlevel*/


boolean langipcgeteventattr (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	register hdlverbrecord hv;
	OSType attr;
	AEDesc attrdesc;
	OSErr errcode;
	
	setbooleanvalue (false, vreturned);
	
	flnextparamislast = true;
	
	if (!getostypevalue (hparam1, 1, &attr))
		return (false);
	
	if (currentprocess == nil)
		goto error;
	
	hv = (hdlverbrecord) (**currentprocess).processrefcon;
	
	if (hv == nil)
		goto error;
	
	errcode = landgetverbattr (hv, attr, typeWildCard, &attrdesc);
	
	if (oserror (errcode))
		return (false);
	
	return (setdescriptorvalue (attrdesc, vreturned));
	
	error: {
		
		langparamerror (trapnotrunningerror, bsfunctionname);
		
		return (false);
		}
	} /*langipcgeteventattr*/


boolean langipccoerceappleitem (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	5.0b1 dmb: use valuetodescriptor to handle oplists
	*/
	
	tyvaluerecord val;
	DescType type;
	AEDesc desc, result;
	OSErr err;
	
	setbooleanvalue (false, vreturned);
	
	if (!getparamvalue (hparam1, 1, &val))
		return (false);
	
	flnextparamislast = true;
	
	if (!getostypevalue (hparam1, 2, &type))
		return (false);
	
	if (!valuetodescriptor (&val, &desc))
		return (false);
	
	err = AECoerceDesc (&desc, type, &result);
	
	AEDisposeDesc (&desc);
	
	if (oserror (err)) /*old desc is in temp stack*/
		return (false);
	
	return (setdescriptorvalue (result, vreturned ));
	} /*langipccoerceappleitem*/


/*
boolean langipcapprunning (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	tyapplicationid appid;
	
	flnextparamislast = true;
	
	if (!getostypevalue (hparam1, 1, &appid))
		return (false);
	
	setbooleanvalue (landapprunning (appid), vreturned);
	
	return (true);
	} /%langipcapprunning%/
*/

static boolean langipcgetparamvisit (bigstring bs, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
#pragma unused (hnode)

	hdlverbrecord hv = (hdlverbrecord) refcon;
	OSType key;
	
	if (!stringtoostype (bs, &key)) {
		
		langparamerror (ostypecoerceerror, bs);
		
		return (true); /*stop visit*/
		}
	
	if (!copyvaluerecord (val, &val))
		return (false);
	
	return (!langipcpushparam (&val, key, hv));
	} /*langipcgetparamvisit*/


static boolean langipcpushrecordparams (const tyvaluerecord *vrecord, hdlverbrecord hverb) {
	
	/*
	2.1a7 dmb: add all of the items in the record to the apple event we're 
	building in hverb
	*/
	
	register hdlverbrecord hv = hverb;
	OSType key;
	tyvaluerecord vitem;
	long ix, ctitems;
	
	if (!langgetlistsize (vrecord, &ctitems))
		return (false);
	
	for (ix = 1; ix <= ctitems; ++ix) {
		
		bigstring bskey;
		
		if (!langgetlistitem (vrecord, ix, bskey, &vitem))
			return (false);
		
		stringtoostype (bskey, &key);
		
		if (!langipcpushparam (&vitem, key, hv)) /*clears vitem if handle is stolen*/
			return (false);
		
		disposevaluerecord (vitem, true);
		}
	
	return (true);
	} /*langipcpushrecordparams*/


static boolean langipcbuildmessage (hdltreenode hparam1, short paramnum, tyipcmessageflags msgflags, tyipcaddress *ipcadr, hdlverbrecord *hverb) {
	
	/*
	12/18/91 dmb seperated tranaction functionality from microsoft DDE functionality
	
	6/29/92 dmb: now take special cases as tyipcmessageflags. added support for systemmsg
	
	9/7/92 dmb: accept a table in the parameter list to provide a group a values, like 
	tableevent. (in fact, this makes tableevent redundant.)
	
	2/12/93 dmb: always use landglobals for find our own application id
	
	2.1a7 dmb: allow a record to replace any key/value pair, just like a table
	
	4.1b7 dmb: access rootval in address to specify another root as the event subject
	
	5.1.5 dmb: use tablesortedinversesearch for guest databases
	*/
	
	register hdltreenode hp1 = hparam1;
	register hdllandglobals hlg = landgetglobals ();
	tyverbclass classid;
	tyverbtoken verbid;
	register hdlverbrecord hv;
	register short pnum = paramnum;
	tyvaluerecord val;
	tyipcaddress ipcaddress;
	long savetransid = 0, transactionid;
	boolean fl;
	
	clearbytes (&ipcaddress, sizeof (tyipcaddress)); /*unassigned fields must be zero*/
	
	if (msgflags & systemmsg) {
		
		ipcaddress.appid = (**hlg).applicationid;
		
		findrunningapplication (&val.data.ostypevalue, ipcaddress.appname, nil);
		}
	else {
		
		if (!getipcaddressvalue (hp1, pnum++, &ipcaddress))
			return (false);
		}
	
	*ipcadr = ipcaddress; /*return to caller*/
	
	if (msgflags & microsoftmsg)
		classid = 'DDE ';
	else
		if (!getostypevalue (hp1, pnum++, &classid))
			return (false);
	
	if (!getostypevalue (hp1, pnum++, &verbid))
		return (false);
	
	if (!fltoolkitinitialized) {
		
		langerror (iactoolkitnotintializederror);
		
		return (false);
		}
	
	if (msgflags & transactionmsg) {
		
		savetransid = (**hlg).eventsettings.transactionid;
		
		if (!getlongvalue (hp1, pnum++, &transactionid))
			return (false);
		
		(**hlg).eventsettings.transactionid = transactionid;
		}
	
	if (ipcaddress.flnetwork)
		fl = landnewnetworkverb (&ipcaddress.netaddress, classid, verbid, 0, hverb);
	else
		fl = landnewverb (ipcaddress.appid, &ipcaddress.appprocessid, classid, verbid, 0, hverb);
	
	if (msgflags & transactionmsg)
		(**hlg).eventsettings.transactionid = savetransid; /*restore*/
	
	if (!fl) {
		
		langostypeparamerror (appnotloadederror, ipcaddress.appid);
		
		return (false);
		}
	
	hv = *hverb; /*move into register*/
	
	if (ipcaddress.flcodeval) { /*4/30/92 dmb*/
		
		(**hv).flverbisdirect = true;
		
		(**hv).landrefcon = (long) ipcaddress.val.data.addressvalue;
		
		exemptfromtmpstack (&ipcaddress.val);
		}
	
	if (ipcaddress.flrootval) { /*4.1b7 dmb*/
	
		tyfilespec fs = **ipcaddress.val.data.filespecvalue;
		
		landsetverbattr ( hv, 'subj', typeFSRef, &fs, sizeof ( fs ) );
		}
	
	while (true) { /*pop off pairs of key+param combos*/
	
		tyvaluerecord valparam;
		tyvaluerecord valkey;
		hdlhashtable htable;
		boolean lfl;
		
		flparamerrorenabled = false; /*no error dialog on getparamvalue call*/
		
		lfl = getparamvalue (hp1, pnum++, &valkey);
		
		flparamerrorenabled = true; /*restore*/
		
		if (!lfl) { /*didn't get another key*/
			
			if (fllangerror) /*was a bad parameter; didn't run out*/
				goto error;
			
			return (true); /*success*/
			}
		
		if (langexternalvaltotable (valkey, &htable, HNoNode)) { /*got a table; grab all values*/
			
			// hipcverb = hv; /*make visible to visit routine*/
			
			if (tablesortedinversesearch (htable, &langipcgetparamvisit, hv)) /*true means error occurred*/
				goto error;
			}
		else {
			
			if (valkey.valuetype == recordvaluetype) {
				
				if (!langipcpushrecordparams (&valkey, hv))
					goto error;
				}
			else {
				
				if (!coercetoostype (&valkey))
					goto error;
				
				flinhibitnilcoercion = true;
				
				lfl = getparamvalue (hp1, pnum++, &valparam);
				
				flinhibitnilcoercion = false;
				
				if (fllangerror) /*wasn't just nil coercion failure*/
					goto error;
				
				if (!lfl) /*must be nil coercion error*/
					continue;
				
				if (!langipcpushparam (&valparam, valkey.data.ostypevalue, hv))
					goto error;
				}
			}
		} /*while*/
	
	error: /*goto here on error before sending verb*/
	
	landdisposeverb (hv);
	
	return (false);
	} /*langipcbuildmessage*/


typedef struct appvisitinfo {

	OSType idlookfor;
	
	hdlhashtable apptablefound;
	
	boolean fluseappinfoname;
	} tyappvisitinfo, *ptrappvisitinfo;


static boolean appinfovisit (hdlhashtable happtable, bigstring bsname, tyappvisitinfo *appvisitinfo) {
	
	bigstring bs;
	tyvaluerecord val;
	hdlhashtable hinfotable;
	hdlhashnode hnode;
	
	langgetstringlist (appinfotablestring, bs); /*name of appinfo table*/
	
	if (!hashtablelookup (happtable, bs, &val, &hnode))
		return (false);
	
	if (!langexternalvaltotable (val, &hinfotable, hnode)) /*not an info table*/
		return (false);
	
	langgetstringlist (appidfieldstring, bs); /*name of id entry*/
	
	if (!hashtablelookup (hinfotable, bs, &val, &hnode))
		return (false);
	
	if (val.valuetype != ostypevaluetype)
		return (false);
	
	if (val.data.ostypevalue != (*appvisitinfo).idlookfor)
		return (false);
	
	if ((*appvisitinfo).fluseappinfoname) { /*use appinfo.name, not the name of the app table*/
		
		langgetstringlist (appnamefieldstring, bs); /*name of name entry*/
		
		if (hashtablelookup (hinfotable, bs, &val, &hnode)) {
			
			if (val.valuetype == stringvaluetype)
				texthandletostring (val.data.stringvalue, bsname);
			}
		}
	
	return (true); /*where or not we got the name, we did find the entry*/
	} /*appinfovisit*/


static boolean apptablevisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
	
	/*
	2.1b2 dmb: added this layer of tableinternal-level code to 
	prevent a bunch of memory from being consumed by an id search
	*/
	
	tyappvisitinfo *appvisitinfo = (tyappvisitinfo*) refcon;
	register hdltablevariable hv;
	register hdlhashtable ht;
	register boolean fltempload;
	hdltablevariable hvariable;
	short errorcode;
	
	if (!gettablevariable (val, &hvariable, &errorcode))
		return (false);
	
	hv = hvariable;
	
	fltempload = !(**hv).flinmemory;
	
	if (!tableverbinmemory ((hdlexternalvariable) hv, hnode))
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata; 
	
	if (appinfovisit (ht, bsname, appvisitinfo)) {
		
		(*appvisitinfo).apptablefound = ht;
		
		return (true);
		}
	
	if (fltempload)
		tableverbunload ((hdlexternalvariable) hv);
	
	return (false);
	} /*apptablevisit*/


boolean langipcfindapptable (OSType appsignature, boolean fluseinfoname, hdlhashtable *happtable, bigstring bsname) {
	
	/*
	2.1b1 dmb: search all appinfo tables for an id matching appsignature
	
	if fluseinfo is true, use appinfo.name as the result, otherwise return 
	the name of the app table itself
	
	common code shared with osacomponent.
	*/
	
	tyappvisitinfo appvisitinfo;
	
	setemptystring (bsname);
	
	if (iacgluetable == nil)
		return (false);
	
	appvisitinfo.idlookfor = appsignature;
	
	appvisitinfo.fluseappinfoname = fluseinfoname;
	
	hashinversesearch (iacgluetable, &apptablevisit, &appvisitinfo, bsname);
	
	if (isemptystring (bsname))
		return (false);
	
	*happtable = appvisitinfo.apptablefound;
	
	return (true);
	} /*langipcfindapptable*/


static void langipcerrormessage (short iderror, tyipcaddress *ipcaddress, bigstring bsparam, short idoserr) {
	
	/*
	do the best job we can at identifying the application at the ipcaddress;
	
	8/11/92 dmb: centralized dealing with error codes returned by the target 
	app here (idoserr).
	*/
	
	register tyipcaddress *adr = ipcaddress;
	bigstring bsname, bswhoseid;
	bigstring bsoserr;
	hdlhashtable happtable;
	
	/*check for os error code*/
	
	if ((bsparam == nil) && (idoserr != 0)) {
		
		langgetmiscstring (unknownstring, bsname);
		
		/*
		setoserrorparam (bsoserr);
		*/
		
		getsystemerrorstring (idoserr, bsoserr);
		
		parsedialogstring (bsoserr, bsname, nil, nil, nil, bsoserr);
		
		bsparam = bsoserr;
		}
	
	/*if message was to a code extension, just report back the error straight*/
	
	if ((*adr).flcodeval && (bsparam != nil)) {
		
		langerrormessage (bsparam);
		
		return;
		}
	
	setemptystring (bsname);
	
	setemptystring (bswhoseid);
	
	/*first, try using the appInfo table*/
	
	if (!langipcfindapptable ((*adr).appid, true, &happtable, bsname)) /*not found; see if it's part of address*/
		copystring ((*adr).appname, bsname);
	
	if (isemptystring (bsname)) { /*oh well, we'll have to use id*/
		
		ostypetostring ((*adr).appid, bsname);
		
		langgetstringlist (appwhoseidstring, bswhoseid);
		}
	
	lang3paramerror (iderror, bsname, bswhoseid, bsparam);
	} /*langipcerrormessage*/


boolean langipcmessage (hdltreenode hparam1, tyipcmessageflags msgflags, tyvaluerecord *vreturned) {
	
	/*
	1/2/91 dmb: added fltoolkitinitialized flag & corresponding error message
	
	12/18/91 dmb: added fltransaction param
	
	6/29/92 dmb: now take special cases as tyipcmessageflags
	
	3.0b15 dmb: on a userCanceledErr, we need to call oserror so that 
	getoserror will yield the appropriate result.
	*/
	
	tyipcaddress ipcaddress;
	hdlverbrecord hverb;
	typaramrecord param;
	bigstring bserror;
	short iderror;
	boolean fl = true;
	
	if (!langipcbuildmessage (hparam1, 1, msgflags, &ipcaddress, &hverb))
		return (false);
	
	if (msgflags & noreplymsg)
		landverbgetsnoreply (hverb);
	
	if (!landsendverb (hverb, &param, bserror, &iderror)) {
		
		if (iderror == 0)
			iderror = landgeterror ();
		
		switch (iderror) {
			
			case 0: /*no error returned -- assume app not running*/
			case connectionInvalid:
				
				langipcerrormessage (appnotloadederror, &ipcaddress, nil, 0);
				
				/*
				ostypetostring (ipcaddress.appid, bs);
				
				langparamerror (appnotloadederror, bs);
				*/
				
				break;
			
			case userCanceledErr:
			case usercancellederror: /*user cancelled -- don't put up an error message*/
				
				oserror (userCanceledErr); /*3.0b15: set the error code*/
				
				break;
			
			default:
				if (isemptystring (bserror)) {
					
					short idmsg;
					
					idmsg = ipcappleerror; /*"couldn't complete the message..."*/
					
					switch (iderror) {
						
						case errAEWaitCanceled:
						case errAETimeout:
							break;
						
						default:
							if ((-1700 >= iderror) && (iderror > -1800)) /*other AE Mgr error*/
								idmsg = ipcerror; /*"xxx" reported the following error...*/
							
							break;
						}
					
					langipcerrormessage (idmsg, &ipcaddress, nil, iderror);
					}
				else
					langipcerrormessage (ipcerror, &ipcaddress, bserror, 0);
				
				/*
				copystring (ipcaddress.appname, bs);
				
				if (isemptystring (bserror))
					numbertostring (iderror, bserror);
				
				lang2paramerror (ipcerror, bs, bserror);
				*/
				
				break;
			}
		
		return (false);
		}
	
	if (param.desc.descriptorType == 0) /*nothing returned to us -- assume true*/
		setbooleanvalue (true, vreturned);
		
	else {
		
		fl = langipccoerceparam (&param.desc, vreturned); /*consumes param*/
		}
	
	return (fl);
	} /*langipcmessage*/


static boolean langipccomplexsend (hdlverbrecord hverb, tyipcaddress *ipcaddress, hdlhashtable htable) {
	
	/*
	send the message in hverb, and stuff all the values in the reply into htable.
	
	consumes hverb.
	
	3.0b15 dmb: on a userCanceledErr, we need to call oserror so that 
	getoserror will yield the appropriate result.
	*/
	
	hdlverbrecord hreturned;
	register hdlverbrecord hv;
	register short i;
	boolean fl;
	
	if (!landcomplexsend (hverb, &hreturned)) {
		
		OSErr iderror;
		
		iderror = landgeterror ();
		
		if ((iderror != userCanceledErr) && (iderror != usercancellederror)) {
			
			if ((iderror == 0) || (iderror == connectionInvalid))
				langipcerrormessage (appnotloadederror, ipcaddress, nil, 0);
			else
				langipcerrormessage (ipcappleerror, ipcaddress, nil, iderror);
			}
		else
			oserror (userCanceledErr); /*3.0b15: set the error code*/
		
		return (false);
		}
	
	hv = hreturned; /*move into register*/
	
	fl = true; /*think positive*/
	
	pushhashtable (htable);
	
	for (i = 1; i <= (**hv).ctparams; i++) {
		
		typaramrecord param;
		tyvaluerecord val;
		bigstring bs;
		
		if (!landgetnthparam (hv, i, &param))
			return (false);
		
		if (!langipccoerceparam (&param.desc, &val))
			break;
		
		ostypetostring (param.key, bs);
		
		fl = hashassign (bs, val);
		
		if (!fl)
			break;
		
		exemptfromtmpstack (&val);
		} /*for*/
	
	pophashtable ();
	
	landdisposeverb (hv); /*dispose returned verb*/
	
	return (fl);
	} /*langipccomplexsend*/


static boolean langipcggettableparam (hdltreenode hparam1, short pnum, boolean flcreate, hdlhashtable *htableparam) {
	
	hdlhashtable htable;
	register hdlhashtable ht;
	bigstring bs;
	boolean fl;
	
	if (!getvarparam (hparam1, pnum, &htable, bs))
		return (false);
	
	ht = htable; /*copy into register*/
	
	assert (ht != nil);
	
	pushhashtable (ht);
	
	fl = langexternalgettable (bs, htableparam);
	
	if (!fl) {
		
		if (flcreate)
			fl = tablenewsubtable (ht, bs, htableparam);
		
		else {
			bigstring bserror;
			
			getstringlist (tableerrorlist, namenottableerror, bserror);
			
			langerrormessage (bserror);
			}
		}
	
	pophashtable ();
	
	return (fl);
	} /*langipcggettableparam*/


boolean langipccomplexmessage (hdltreenode hparam1, tyvaluerecord *vreturned) {
#pragma unused(vreturned)

	hdlhashtable htable;
	tyipcaddress ipcaddress;
	hdlverbrecord hverb;
	
	if (!langipcggettableparam (hparam1, 1, true, &htable))
		return (false);
	
	if (!langipcbuildmessage (hparam1, 2, normalmsg, &ipcaddress, &hverb))
		return (false);
	
	return (langipccomplexsend (hverb, &ipcaddress, htable));
	} /*langipccomplexmessage*/


boolean langipctablemessage (hdltreenode hparam1, tyvaluerecord *vreturned) {
#pragma unused(vreturned)

	/*
	5.1.5 dmb: use tablesortedinversesearch for guest databases
	*/
	
	hdlhashtable hsourcetable;
	hdlhashtable hdesttable;
	hdlverbrecord hverb;
	tyipcaddress ipcaddress;
	
	if (!langipcggettableparam (hparam1, 1, false, &hsourcetable))
		return (false);
	
	if (!langipcggettableparam (hparam1, 2, true, &hdesttable))
		return (false);
	
	if (!langipcbuildmessage (hparam1, 3, normalmsg, &ipcaddress, &hverb))
		return (false);
	
	// hipcverb = hverb; /*make visible to visit routine*/
	
	if (tablesortedinversesearch (hsourcetable, &langipcgetparamvisit, hverb)) { /*true means error occurred*/
		
		landdisposeverb (hverb);
		
		return (false);
		}
	
	return (langipccomplexsend (hverb, &ipcaddress, hdesttable)); /*consumes hverb*/
	} /*langipctablemessage*/


static AEDesc *fastreply;

static langerrormessagecallback fastsavecallback;


static boolean langipcfasterror (bigstring bs, ptrvoid refcon) {
#pragma unused (refcon)

	long n;
	OSErr err;
	
	n = getoserror ();
	
	if (n == noErr)
		n = -errAEEventFailed;
	
	err = AEPutKeyPtr (fastreply, 'errn', typeLongInteger, (Ptr) &n, sizeof (n));
	
	err = AEPutKeyPtr (fastreply, 'errs', typeChar, (Ptr) bs + 1, stringlength (bs));
	
	return (true);
	} /*langipcfasterror*/


static void langipchookfasterrors (AEDesc *reply) {
	
	fastsavecallback = langcallbacks.errormessagecallback;
	
	langcallbacks.errormessagecallback = &langipcfasterror;
	
	shellpusherrorhook ((errorhookcallback) &langipcfasterror);
	
	fastreply = reply;
	} /*langipchookfasterrors*/


static boolean langipcunhookfasterrors (void) {
	
	shellpoperrorhook ();
	
	langcallbacks.errormessagecallback = fastsavecallback;
	
	oserror (noErr); /*clear it out to avoid conflicts with current thread*/
	
	fllangerror = false; /*ditto*/
	
	return (true);
	} /*langipcunhookfasterrors*/


static boolean langipcnewevent (ProcessSerialNumber *psn, AEEventClass class, AEEventID id, AppleEvent *event) {
	
	AEDesc desc;
	OSErr err;
	
	err = AECreateDesc (typeProcessSerialNumber, (Ptr) psn, sizeof (*psn), &desc);
	
	if (oserror (err))
		return (false);
	
	err = AECreateAppleEvent (class, id, &desc, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	AEDisposeDesc (&desc);
	
	return (!oserror (err));
	} /*langipcnewevent*/


boolean newselfaddressedevent (AEEventID id, AppleEvent *event) {
	
	ProcessSerialNumber psn = langipcself;
	
	if (iscurrentapplication (psn)) {
		
		psn.highLongOfPSN = 0;
		
		psn.lowLongOfPSN =  kCurrentProcess;
		}
	
	return (langipcnewevent (&psn, 'LAND', id, event));
	} /*newselfaddressedevent*/


static OSErr langipcsendevent (AppleEvent *event, AppleEvent *reply) {

	register hdllandglobals hlg;
	OSErr err;
	
	hlg = landgetglobals ();
	
	err = (*(**hlg).eventsendcallback) (event, reply, 
		
		(AESendMode) kAEWaitReply + kAEAlwaysInteract + kAECanSwitchLayer, 
		
		(AESendPriority) kAEHighPriority, (long) kNoTimeOut, nil, nil);
	
	return (err);
	} /*langipcsendevent*/


static boolean langipcgeterrorstring (const AppleEvent *reply, bigstring bserror) {
	
	AEDesc desc;
	
	if (AEGetParamDesc (reply, 'errs', typeChar, &desc) == noErr) {
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/	
		
			datahandletostring (&desc, bserror);
		
		#else
		
			texthandletostring (desc.dataHandle, bserror);
		
		#endif
		
		AEDisposeDesc (&desc);
		
		return (true);
		}
	
	return (false);
	} /*langipcgeterrorstring*/


boolean langipcbuildsubroutineevent (AppleEvent *event, bigstring bsname, hdltreenode hparam1) {
	
	/*
	2.1b3 dmb: build an OSA subroutine event given the name and parameter list
	
	2.1b4 dmb: handle named parameters. if a name is defined, try to use its value 
	as the key. otherwise, use the name itself.
	
	2.1b12 dmb: moved into langipc.c from osacomponent.c. the caller is now 
	responsible for creating the apple event itself, but we dispose of it on error.
	*/
	
	AEDesc params = {typeNull, nil};
	AEDesc desc;
	tyvaluerecord vparam;
	tyvaluerecord vkey;
	OSType key;
	register hdltreenode hp;
	register OSErr err;
	register boolean fl = false;
	hdlhashnode hnode;
	
	err = AEPutParamPtr (event, keyASSubroutineName, typeChar, (Ptr) bsname + 1, stringlength (bsname));
	
	if (err != noErr)
		goto exit;
	
	err = AECreateList (nil, 0, false, &params);
	
	if (err != noErr)
		goto exit;
	
	for (hp = hparam1; hp != nil; hp = (**hp).link) {
		
		if ((**hp).nodetype == fieldop) {
		
			if (!langgetidentifier ((**hp).param1, bsname))
				goto exit;
			
			if (langgetsymbolval (bsname, &vkey, &hnode) && (vkey.valuetype == ostypevaluetype))
				key = vkey.data.ostypevalue;
			else
				stringtoostype (bsname, &key);
			
			if (!evaluatetree ((**hp).param2, &vparam))
				goto exit;
			
			if (!valuetodescriptor (&vparam, &desc))
				goto exit;
			
			err = AEPutKeyDesc (event, key, &desc);
			}
		else {
			
			if (!evaluatetree (hp, &vparam))
				goto exit;
			
			if (!valuetodescriptor (&vparam, &desc))
				goto exit;
			
			err = AEPutDesc (&params, 0, &desc);
			}
		
		AEDisposeDesc (&desc);
		
		if (err != noErr)
			goto exit;
		}
	
	err = AEPutParamDesc (event, keyDirectObject, &params);
	
	if (err != noErr)
		goto exit;
	
	fl = true;
	
	exit:
	
	AEDisposeDesc (&params);
	
	if (!fl) {
		
		oserror (err); /*set error code, if available*/
		
		AEDisposeDesc (event);
		}
	
	return (fl);
	} /*langipcbuildsubroutineevent*/


boolean langipchandlercall (hdltreenode htree, bigstring bsverb, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	3.0a dmb: htree/hparam1 are a standard function call, but the function 
	isn't defined.  if the current process isn't frontier, and the function 
	name is a simple identifier (not a dotted id), then send a subroutine 
	event to the current process and see if it handles the call.
	
	return true if the subroutine event is handled successfully. if an 
	attempt is made to handle it and we generate an error, it will override 
	the error that langhandlercall will subsequently generate.
	*/
	
	tytreetype op;
	AppleEvent event = {typeNull, nil};
	AppleEvent reply = {typeNull, nil};
	ProcessSerialNumber psn = {0, kCurrentProcess};
	OSErr err;
	boolean fl = false;
	
	if (iscurrentapplication (langipcself)) /*if frontier could handle this, we wouldn't get here*/
		return (false);
	
	op = (**htree).nodetype;
	
	if ((op != identifierop) && (op != bracketop))
		return (false);
	 
	if (!langipcnewevent (&psn, kOSASuite, kASSubroutineEvent, &event))
		return (false);
	
	if (!langipcbuildsubroutineevent (&event, bsverb, hparam1)) /*disposes event on error*/
		return (false);
	
	err = langipcsendevent (&event, &reply);
	
	AEDisposeDesc (&event);
	
	if (err != errAEEventNotHandled) {
		
		if (err != noErr) {
			
			bigstring bs;
			
			if (langipcgeterrorstring (&reply, bs))
				langerrormessage (bs);
			else
				oserror (err);
			}
		else {
			
			AEDesc desc;
			
			err = AEGetParamDesc (&reply, '----', typeWildCard, &desc);
			
			switch (err) {
				
				case noErr:
					fl = setdescriptorvalue (desc, vreturned);
					
					break;
				
				case errAEDescNotFound:
					fl = setbooleanvalue (true, vreturned);
					
					break;
				
				default:
					oserror (err);
					
					break;
				}
			}
		}
	
	AEDisposeDesc (&reply);
	
	return (fl);
	} /*langipchandlercall*/


#if 1

/*
static boolean handlekernelfunction (hdlverbrecord hverb) {
	
	/%
	2.1b14: this is what langipchandletrapverb boils down to in 
	its handling of the kernel event. but to avoid all this code 
	duplication, we'll just add yet another special case to the trap 
	hander.
	%/
	
	register hdlverbrecord hv = hverb;
	register boolean fl = false;
	bigstring bsverb;
	tyvaluerecord val;
	callback savecallback;
	hdltreenode hfunctioncall;
	hdltreenode hparamlist;
	hdltreenode hcode;
	hdlhashtable hkernel;
	hdlhashtable hcontext;
	
	if (!landgetstringparam (hv, keyASSubroutineName, bsverb)) 
		return (false);
	
	if (!landgetlongparam (hv, keykerneltable, (long *) &hkernel))
		return (false);
	
	if (!landgetlongparam (hv, keycurrenttable, (long *) &hcontext))
		return (false);
	
	/%build a code tree and call the handler, with our error hook in place%/
	
	langipchookerrors (hv, &savecallback);
	
	if (!setaddressvalue (hkernel, bsverb, &val))
		goto exit;
	
	if (!pushfunctionreference (val, &hfunctioncall))
		goto exit;
	
	pushhashtable (hcontext); /%need context to evaluate local address params%/
	
	fl = langipcbuildparamlist (hv, &hparamlist);
	
	pophashtable ();
	
	if (!fl) {
		
		langdisposetree (hfunctioncall);
		
		goto exit;
		}
	
	if (!pushfunctioncall (hfunctioncall, hparamlist, &hcode)) /%consumes input parameters%/
		goto exit;
	
	if (!pushbinaryoperation (moduleop, hcode, nil, &hcode)) /%needs this level%/
		goto exit;
	
	fl = langipcruncode (hv, hcode, hcontext, false);
	
	exit:
	
	langipcunhookerrors (savecallback);
	
	return (fl);
	} /%handlekernelfunction%/
*/

boolean langipckernelfunction (hdlhashtable htable, bigstring bsverb, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	5/18/93 dmb: paired with handlerkernelfunction above, this routine decouples 
	kernel functions so that they can be called from client processes via an AE.
	
	2.1b11 dmb: to reliably continue execution in the event handler, we need to 
	pass the thread globals, not just the langcallbacks
	
	2.1b12 dmb: ok, so it wasn't thinking this through. passing the entire context 
	and trying to run the same code tree on the other side of an AE/process swap 
	is doomed to failure. what if one of the parameter trees sends an AE back to 
	the caller? deadlock. no, the only reliable way to do this is the slow way: 
	evaluate all the parameters now, and send the resulting values in the kernel 
	event. forunately, the subroutine event is much like this, so there's code to 
	share.
	
	3.0a dmb: fixed memory leak in failure case, getting text
	
	5.0a23 dmb: handle no value in the reply, meaning nil was returned
	*/
	
	AppleEvent event = {typeNull, nil};
	AppleEvent reply = {typeNull, nil};
	OSErr err;
	boolean fl = false;
	
	if (!newselfaddressedevent (kASSubroutineEvent, &event)) /*note: class is 'LAND'*/
		return (false);
	
	if (!langipcbuildsubroutineevent (&event, bsverb, hparam1)) /*disposes event on error*/
		return (false);
	
	err = AEPutParamPtr (&event, keykerneltable, typeLongInteger, (Ptr) &htable, sizeof (long));
	
	if (err == noErr)
		err = AEPutParamPtr (&event, keycurrenttable, typeLongInteger, (Ptr) &currenthashtable, sizeof (long));
	
	if (err == noErr)
		err = langipcsendevent (&event, &reply);
	
	AEDisposeDesc (&event);
	
	if (!oserror (err)) {
		
		bigstring bs;
		AEDesc desc;
		
		if (langipcgeterrorstring (&reply, bs))
			langerrormessage (bs);
		
		else {
			
			err = AEGetParamDesc (&reply, '----', typeWildCard, &desc);
			
			if (err == errAEDescNotFound)
				fl = setnilvalue (vreturned);
			else
				if (!oserror (err))
					fl = setdescriptorvalue (desc, vreturned);
			}
		}
	
	AEDisposeDesc (&reply);
	
	return (fl);
	} /*langipckernelfunction*/

#else

static pascal OSErr handlekernelfunction (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	/*
	2.1b11 dmb: we're now pass the thread globals of the caller, so we can 
	cleanly do our stuff.  also, since this isn't a fast event handler, we 
	don't need to push/pop a fast context (and probably shouldn't)
	*/
	
	AEDesc desc;
	OSType type;
	long size;
	hdlhashtable htable;
	bigstring bsverb;
	hdlhashtable hcontext;
	hdltreenode hparam1;
	tyvaluerecord vreturned;
	hdlthreadglobals hthreadglobals, hsaveglobals;
	/*
	tyfastverbcontext savecontext;
	tylangcallbacks savecallbacks;
	boolean flscriptwasrunning;
	*/
	OSErr err;
	
	/*
	landpushfastcontext (&savecontext);
	*/
	
	err = AEGetParamPtr (event, 'ktbl', typeLongInteger, &type, (Ptr) &htable, sizeof (long), &size);
	
	if (err != noErr)
		goto exit;
	
	err = AEGetParamPtr (event, 'kvrb', typeChar, &type, (Ptr) bsverb + 1, lenbigstring, &size);
	
	if (err != noErr)
		goto exit;
	
	setstringlength (bsverb, size);
	
	err = AEGetParamPtr (event, 'ctbl', typeLongInteger, &type, (Ptr) &hcontext, sizeof (long), &size);
	
	if (err != noErr)
		goto exit;
	
	err = AEGetParamPtr (event, 'parm', typeLongInteger, &type, (Ptr) &hparam1, sizeof (long), &size);
	
	if (err != noErr)
		goto exit;
	
	err = AEGetParamPtr (event, 'thrd', typeLongInteger, &type, (Ptr) &hthreadglobals, sizeof (hthreadglobals), &size);
	
	if (err != noErr)
		goto exit;
	
	hsaveglobals = getcurrentthreadglobals ();
	
	copythreadglobals (hsaveglobals);
	
	swapinthreadglobals (hthreadglobals);
	
	langipchookfasterrors (reply);
	
	if (kernelfunctionvalue (htable, bsverb, hparam1, &vreturned)) {
		
		if (valuetodescriptor (&vreturned, &desc)) { /*consumes val*/
			
			AEPutParamDesc (reply, '----', &desc);
			
			AEDisposeDesc (&desc);
			}
		}
	
	langipcunhookfasterrors ();
	
	copythreadglobals (hthreadglobals);
	
	swapinthreadglobals (hsaveglobals);
	
	exit:
	
	/*
	landpopfastcontext (&savecontext);
	*/
	
	return (err);
	} /*handlekernelfunction*/

boolean langipckernelfunction (hdlhashtable htable, bigstring bsverb, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	5/18/93 dmb: paired with handlerkernelfunction above, this routine decouples 
	kernel functions so that they can be called from client processes via an AE.
	
	2.1b11 dmb: to reliably continue execution in the event handler, we need to 
	pass the thread globals, not just the langcallbacks
	*/
	
	AEDesc desc;
	AppleEvent event = {typeNull, nil};
	AppleEvent reply = {typeNull, nil};
	OSErr err;
	boolean fl = false;
	hdlthreadglobals hthreadglobals;
	
	if (!newselfaddressedevent ('krnl', &event))
		return (false);
	
	err = AEPutParamPtr (&event, 'ktbl', typeLongInteger, (Ptr) &htable, sizeof (long));
	
	if (err == noErr)
		err = AEPutParamPtr (&event, 'kvrb', typeChar, (Ptr) bsverb + 1, stringlength (bsverb));
	
	if (err == noErr)
		err = AEPutParamPtr (&event, 'ctbl', typeLongInteger, (Ptr) &currenthashtable, sizeof (long));
	
	if (err == noErr)
		err = AEPutParamPtr (&event, 'parm', typeLongInteger, (Ptr) &hparam1, sizeof (long));
	
	/*
	if (err == noErr)
		err = AEPutParamPtr (&event, 'cbck', typeChar, (Ptr) &langcallbacks, sizeof (langcallbacks));
	*/
	
	if (err == noErr) {
		
		hthreadglobals = getcurrentthreadglobals ();
		
		err = AEPutParamPtr (&event, 'thrd', typeLongInteger, (Ptr) &hthreadglobals, sizeof (hthreadglobals));
		}
	
	if (err == noErr)
		err = langipcsendevent (&event, &reply);
	
	AEDisposeDesc (&event);
	
	if (!oserror (err)) {
		
		if (AEGetParamDesc (&reply, 'errs', typeChar, &desc) == noErr) {
			
			bigstring bs;
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				datahandletostring (&desc, bs);
			
			#else

				texthandletostring (desc.dataHandle, bs);
			
			#endif
			
			langerrormessage (bs);
			}
		else {
			
			err = AEGetParamDesc (&reply, '----', typeWildCard, &desc);
			
			if (!oserror (err))
				fl = setdescriptorvalue (desc, vreturned);
			}
		}
	
	AEDisposeDesc (&reply);
	
	return (fl);
	} /*langipckernelfunction*/

#endif

#if !flruntime

static pascal OSErr handleshowmenunode (const AppleEvent *event, AppleEvent *reply, long refcon) {
#pragma unused (reply, refcon)

	/*
	4.1b14 dmb: added outline parameter; can't count on outline data not changing
	*/
	
	OSType type;
	long size;
	hdloutlinerecord houtline;
	hdlheadrecord hnode;
	OSErr err;
	
	err = AEGetParamPtr (event, 'outl', typeLongInteger, &type, (Ptr) &houtline, sizeof (long), &size);
	
	if (err == noErr)
		err = AEGetParamPtr (event, 'node', typeLongInteger, &type, (Ptr) &hnode, sizeof (long), &size);
	
	if (err == noErr) {
		
		SetFrontProcess (&langipcself);
		
		oppushoutline (houtline);
		
		meshownode (hnode);
		
		oppopoutline ();
		}
	
	return (err);
	} /*handleshowmenunode*/

#endif

boolean langipcshowmenunode (long hnode) {
	
	/*
	4.1b14 dmb: added outline parameter; can't count on outline data not changing
	*/
	
	AppleEvent event, reply;
	OSErr err;
	
	if (!newselfaddressedevent ('show', &event))
		return (false);
	
	err = AEPutParamPtr (&event, 'outl', typeLongInteger, (Ptr) &outlinedata, sizeof (long));
	
	err = AEPutParamPtr (&event, 'node', typeLongInteger, (Ptr) &hnode, sizeof (long));
	
	if (err == noErr) {
		
		err = langipcsendevent (&event, &reply);
		
		AEDisposeDesc (&reply);
		}
	
	AEDisposeDesc (&event);
	
	return (!oserror (err));
	} /*langipcshowmenunode*/


boolean langipcnoop (void) {
	
	AppleEvent event, reply;
	
	if (!newselfaddressedevent ('noop', &event))
		return (false);
	
	langipcsendevent (&event, &reply);
	
	AEDisposeDesc (&event);
	
	AEDisposeDesc (&reply);
	
	return (true);
	} /*langipcshowmenunode*/


static pascal OSErr handleyield (const AppleEvent *event, AppleEvent *reply, long refcon) {
#pragma unused (event, reply, refcon)

	processyield ();
	
	return (noErr);
	} /*handleyield*/


static pascal boolean langipcbreakembrace (EventRecord *ev) {
	
	/*
	7/3/91 dmb: it turns out that try to do background processing during the 
	breakembraceroutine doesn't work out very well.  for one thing, we aren't 
	getting the event information from the Apple Event Manager, and shouldn't be
	handling any events other than those that it would be giving us.  secondly, 
	it slows things down quite a bit.
	
	to allow user input while we're waiting for a message reply, we'll have to 
	add more bandwidth to the Toolkit interface.
	*/
	
	if ((*ev).what != nullEvent) {
		
		shellblockevents (); /*just in case*/
		
		shellprocessevent (ev);
		
		shellpopevents ();
		}
	
	/*
	shellpushblock (highLevelEventMask, true); /%just in case%/
	
	fl = langbackgroundtask ();
	
	shellpopblock ();
	
	if (!fl)
		return (true);
	*/
	
	return (landefaultbreakembrace (ev));
	} /*langipcbreakembrace*/


static pascal OSErr langipcopenapproutine (void) {
	
	if (!shellopendefaultfile ())
		return (getoserror ());
	
	return (noErr);
	} /*langipcopenapproutine*/


static pascal OSErr langipcopendocroutine ( FSSpec *fs ) {

	//
	// 2006-07-25 creedon: FSRef=ized
	//

	tyfilespec fst;
	FSRef fsref;
	
	clearbytes ( &fst, sizeof ( fst ) );
	
	(void) FSpMakeFSRef ( fs, &fsref );
	
	(void) macmakefilespec (&fsref, &fst);
	
	if ( ! shellopenfile ( &fst, false, nil ) )
		return ( getoserror ( ) );
	
	return ( noErr );
	
	} // langipcopendocroutine


static pascal OSErr langipcquitapproutine (void) {
	
	/*
	8/12/91 dmb: use new shellexitmaineventloop to defer quit until after
	we've had a chance to reply to caller
	*/
	
	if (!shellcloseall (nil, true)) /*user hit Cancel button in save dialog*/
		return (userCanceledErr);
	
	shellexitmaineventloop (); /*sets flag for next iteration*/
	
	return (noErr);
	} /*langipcquitapproutine*/


static boolean getpackedobject (Handle htext, Handle *hpacked) {
	
	/*
	htext is a text handle with the name of an object that you want packed from
	the object database. if we return true, hpacked contains the packed version
	of the database object.
	
	on exit, htext is in the temp stack, and will be disposed automatically
	*/
	
	tyvaluerecord val;
	bigstring bs;
	hdlhashtable htable;
	hdlhashnode hnode;
	
	if (!setheapvalue (htext, stringvaluetype, &val))
		return (false);
	
	if (!stringtoaddress (&val))
		return (false);
		
	if (!getaddressvalue (val, &htable, bs))
		return (false);
	
	if (!langsymbolreference (htable, bs, &val, &hnode))
		return (false);
	
	return (langpackvalue (val, hpacked, hnode));
	} /*getpackedobject*/
	

static boolean setpackedobject (Handle htext, Handle hpacked) {
	
	/*
	we unpack the packed handle into the object database location indicated by
	htext.
	
	on exit, htext is in the temp stack, and hpacked is consumed
	*/
	
	tyvaluerecord val;
	bigstring bs;
	hdlhashtable htable;
	boolean fl = false;
	
	if (!setheapvalue (htext, stringvaluetype, &val))
		goto exit;
	
	if (!stringtoaddress (&val))
		goto exit;
		
	if (!getaddressvalue (val, &htable, bs))
		goto exit;
	
	if (!langunpackvalue (hpacked, &val))
		goto exit;
	
	fl = langsetsymboltableval (htable, bs, val);
	
	exit:
	
	disposehandle (hpacked);
	
	return (fl);
	} /*setpackedobject*/


static pascal OSErr langipcfastgetobject (AppleEvent *event, AppleEvent *reply, long refcon) {
#pragma unused (refcon)

	/*
	7/3/92 DW: this one will be hot!
	*/
	
	OSErr err;
	AEDesc desc;
	tyfastverbcontext savecontext;
	long curA5;
	
	#if flcomponent
		
		curA5 = SetUpAppA5 ();
	
	#endif
	
	landpushfastcontext (&savecontext);
	
	langipchookfasterrors (reply);
	
	err = noErr;
	
	if (oserror (AEGetKeyDesc (event, '----', typeChar, &desc)))
		goto exit;
	
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle hcopy;
		boolean fl = false;
		
		copydatahandle (&desc, &hcopy);
		
		fl = getpackedobject (hcopy, &hcopy);
		
		if (fl)
			
			newdescwithhandle (&desc, desc.descriptorType, hcopy);
			
		disposehandle (hcopy);
		
		if (!fl)
			goto exit;
		}
	
	#else
	
		if (!getpackedobject (desc.dataHandle, &desc.dataHandle)) /*consumes input handle*/
			goto exit;
		
	#endif
	
	desc.descriptorType = 'data';
	
	err = AEPutKeyDesc (reply, '----', &desc);
	
	AEDisposeDesc (&desc);
	
	exit:
	
	langipcunhookfasterrors ();
	
	landpopfastcontext (&savecontext);
	
	#if flcomponent
		
		RestoreA5 (curA5);
	
	#endif
	
	return (err);
	} /*langipcfastgetobject*/


static pascal OSErr
langipcfastsetobject (
		AppleEvent	*event,
		AppleEvent	*reply,
		long		 refcon)
{
#pragma unused(refcon)
	
	/*
	7/3/92 DW: this one will be hot!
	*/
	
	OSErr err;
	AEDesc desc1, desc2;
	tyfastverbcontext savecontext;
	Boolean fl;
	
	#if flcomponent && TARGET_API_MAC_OS8
		
		long curA5 = SetUpAppA5 ();
	
	#endif
	
	landpushfastcontext (&savecontext);
	
	langipchookfasterrors (reply);
	
	err = noErr;
	
	if (oserror (AEGetKeyDesc (event, '----', typeChar, &desc1)))
		goto exit;
	
	if (oserror (AEGetKeyDesc (event, 'pack', 'data', &desc2))) {
		
		AEDisposeDesc (&desc1);
		
		goto exit;
		}
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle hcopy1, hcopy2;
		
		copydatahandle (&desc1, &hcopy1);
		
		copydatahandle (&desc2, &hcopy2);
		
		if (!setpackedobject (hcopy1, hcopy2))
			goto exit;
		}
	
	#else
	
		if (!setpackedobject (desc1.dataHandle, desc2.dataHandle)) /*consumes handles*/
			goto exit;
	
	#endif
	
	fl = true;
	
	err = AEPutParamPtr (
		
		reply, keyDirectObject, typeBoolean, (Ptr) &fl, 
		
		(Size) sizeof (Boolean));
	
	exit:
	
	langipcunhookfasterrors ();
	
	landpopfastcontext (&savecontext);
	
	#if flcomponent && TARGET_API_MAC_OS8
		
		RestoreA5 (curA5);
	
	#endif
	
	return (err);
	} /*langipcfastsetobject*/


static pascal OSErr
langipchandlefastscript (
		AppleEvent	*event,
		AppleEvent	*reply,
		long		 refcon)
{
#pragma unused(refcon)

	/*
	3.0.2b1 dmb: reverted to original, 2.0.1 implementation that doesn't rely
	on the component manager being present.
	*/
	
	/*
	ComponentInstance comp;
	*/
	AEDesc script;
	tyfastverbcontext savecontext;
	boolean flscriptwasrunning;
	bigstring bs;
	boolean fl;
	OSErr err;
	
	#if flcomponent && TARGET_API_MAC_OS8
		
		long curA5 = SetUpThisA5 (refcon);
	
	#endif
	
	#if 1
	
		landpushfastcontext (&savecontext);
		
		langipchookfasterrors (reply);
		
		++fldisableyield;
		
		err = AEGetKeyDesc (event, '----', typeChar, &script);
		
		if (oserror (err))
			goto exit;
		
		pushhashtable (roottable); /*make sure we don't tromple current script's tmpstack*/
		
		flscriptwasrunning = flscriptrunning;
		
		flscriptrunning = false;
		
		setemptystring (bs);
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			{
			Handle h;
			
			copydatahandle (&script, &h);
			
			fl = langrunhandle (h, bs);
			}
		
		#else
		
			fl = langrunhandle (script.dataHandle, bs); /*consumes dataHandle*/
		
		#endif
		
		flscriptrunning = flscriptwasrunning;
		
		cleartmpstack (); /*make sure we're livin' clean*/
		
		pophashtable ();
		
		if (fl)
			AEPutKeyPtr (reply, '----', typeChar, (Ptr) bs + 1, stringlength (bs));
		
		exit:
		
		--fldisableyield;
		
		langipcunhookfasterrors ();
		
		landpopfastcontext (&savecontext);
		
	#else /*this code assumes that the component manger is present*/
		
		comp = OpenComponent (osacomponent);
		
		if (comp == nil)
			err = memFullErr;
		
		else {
			
			err = AEGetKeyDesc (event, '----', typeChar, &script);
			
			if (err == noErr) {
				
				err = OSADoScript (comp, &script, kOSANullScript, typeChar, kOSANullMode, &result);
				
				AEDisposeDesc (&script);
				
				if (err == noErr) {
					
					err = AEPutKeyDesc (reply, '----', &result);
					
					AEDisposeDesc (&result);
					}
				}
			
			CloseComponent (comp);
			}
	
	#endif
	
	#if flcomponent && TARGET_API_MAC_OS8
		
		RestoreA5 (curA5);
	
	#endif
	
	return (err);
	} /*langipchandlefastscript*/


static boolean langipcinstallfastscript (void) {
	
	/*
	5/7/92 dmb: install the fastscript handler into the system dispatch table.
	
	as per IM VI-64, the handler must reside in the system heap.  we assume that 
	the handler, right above us in the source, directly preceeds us in the code.
	*/
	//This is never used in the app
	//Code change by Timothy Paustian Wednesday, June 14, 2000 8:59:58 PM
	//Changed to Opaque call for Carbon
	//langipczone = GetZone ();
	
	langipcresfile = CurResFile ();
	
	//langipcresmap = LMGetTopMapHndl ();
	
	if (!landsystem7installfasthandler ('fast', 'dosc', langipchandlefastscript))
		return (false);
	
	/*
	landsystem7installfasthandler ('app0', 'clck', langipchandleapp0verb);
	*/
	
	if (!landsystem7installfasthandler ('fast', 'gobj', langipcfastgetobject))
		return (false);
	
	if (!landsystem7installfasthandler ('fast', 'sobj', langipcfastsetobject))
		return (false);
	
	return (true);
	} /*langipcinstallfastscript*/


static void langipcremovefastscript (void) {
	
	/*
	11/17/92 dmb: remove fast sojb & gobj
	*/
	
	landsystem7removefasthandler ('fast', 'dosc');
	
	landsystem7removefasthandler ('fast', 'gobj');
	
	landsystem7removefasthandler ('fast', 'sobj');
	
	landsystem7removefasthandler ('app0', 'clck');
	} /*langipcremovefastscript*/


boolean langipcstart (void) {
	
	/*
	register with the IAC toolkit, using the identifier defined in land.h, 
	and registering the verbs that scriptrunners must implement.
	*/
	
	register hdllandglobals hg;
	
	fltoolkitinitialized = landinit ();
	
	if (!fltoolkitinitialized) /*report failure, but program may still run*/
		return (false);
	
	hg = landgetglobals (); /*move into register*/
	
	GetCurrentProcess (&langipcself);
	
	(**hg).iderrorlist = langerrorlist; /*3.0.1; was 136*/
	
	(**hg).handleverbroutine = &langipchandleverb;
	
	(**hg).findhandlerroutine = &langipcfindhandler;
	
	(**hg).breakembraceroutine = &langipcbreakembrace;
	
	(**hg).macopenapproutine = &langipcopenapproutine;
	
	(**hg).macopendocroutine = &langipcopendocroutine;
	
	(**hg).macquitapproutine = &langipcquitapproutine;
	
	#ifdef flcomponent
	
	(**hg).eventcreatecallback = &landsystem7defaultcreate;
	
	(**hg).eventsendcallback = &landsystem7defaultsend;
	
	#endif
	

	/*
	AEInstallEventHandler ('LAND', 'krnl', NewAEEventHandlerProc (handlekernelfunction), 0, false);
	*/
	
	#if !flruntime
	
		#if TARGET_API_MAC_CARBON == 1
	
			AEInstallEventHandler ('LAND', 'show', NewAEEventHandlerUPP (handleshowmenunode), 0, false);
		
		#else

			AEInstallEventHandler ('LAND', 'show', NewAEEventHandlerProc (handleshowmenunode), 0, false);
		
		#endif
	
	#endif
	
	#if TARGET_API_MAC_CARBON == 1

		AEInstallEventHandler ('LAND', 'yiel', NewAEEventHandlerUPP (handleyield), 0, false);
	
	#else

		AEInstallEventHandler ('LAND', 'yiel', NewAEEventHandlerProc (handleyield), 0, false);
	
	#endif
		
	langipcinstallfastscript ();
	
	landaddverb (runscripttoken);
	
	/*
	landaddverb (schedulescripttoken);
	*/
	
	landaddfastverb (getmenuarraytoken);
	
	landaddfastverb (getmenuhandletoken);
	
	landaddfastverb (idwho);
	
	landaddverb (idrunmenuitem); /*if option key is down, this may open the scipt*/
	
	landaddfastverb (killscripttoken);
	
	landacceptanyverb (true); /*although we only register above two, we'll try anything*/
	
	#if flcomponent
	
	wsGlobals.windowserver = nil;
	
	#endif
	
	return (langipcmenuinit ());
	} /*langipcstart*/


void langipcshutdown (void) {
	
	langipcremovefastscript ();
	
	landclose ();
	
	#if flcomponent
		uisClose ();
	#endif
	} /*langipcshutdown*/


#if flruntime

boolean langipcinit (void) {
	
	#ifdef flcomponent
	
		#if !TARGET_API_MAC_CARBON
		RememberA5 ();
		#endif
	
	#endif
	
	return (true);
	} /*langipcinit*/

#else

static boolean langipceventhook (EventRecord *ev, WindowPtr w) {
#pragma unused(w)

	/*
	7/3/91 dmb: under system 7, we fork off a new thread for every 
	incoming IAC event we handle.  under system 6, handling IAC events 
	blocks normal event handling until completion.
	*/
	
	if (!fltoolkitinitialized)
		return (true);
	
	return (!landeventfilter (ev));
	} /*langipceventhook*/

boolean langipcinit (void) {
	
	#ifdef flcomponent
		//Code change by Timothy Paustian Wednesday, June 14, 2000 9:02:24 PM
		//Changed to Opaque call for Carbon
		//we don't need this in carbon
		#if !TARGET_CARBON
		RememberA5 ();
		#endif
	#endif
	shellpusheventhook (&langipceventhook);
	
	return (true);
	} /*langipcinit*/

#endif



