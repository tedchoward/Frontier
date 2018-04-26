
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

#include "langxcmd.h"

#include "memory.h"
#include "frontierconfig.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "font.h"
#include "kb.h"
#include "mouse.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "sounds.h"
#include "strings.h"
#include "frontierwindows.h"
#include "shell.h"
#include "shellhooks.h"
#include "oplist.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langipc.h"
#include "langdll.h"
#include "langsystem7.h"
#include "langtokens.h"
#include "langwinipc.h"
#include "BASE64.H"
#include "tablestructure.h"
#include "tableverbs.h"
#include "process.h"
#include "processinternal.h"
#include "kernelverbdefs.h"
#include "WinSockNetEvents.h"
#include "notify.h"
#include "timedate.h"


boolean langwinipcerrorroutine (bigstring bs, ptrvoid refcon) {
#pragma unused (refcon)

	/*
	if an error occurs while processing a runscript verb, we want to 
	return the text that would normally go into the langerror window as 
	an error string to our caller.
	*/
	
	tyvaluerecord * val = nil;
	
	if (currentprocess != nil)
		val = (tyvaluerecord *) (**currentprocess).processrefcon;
	
	if (val == nil)
		return (false);		//should never be....
	
	setstringvalue (bs, val);

	exemptfromtmpstack (val);

	return (false); /*consume the error*/
	} /*langwinipcerrorroutine*/




static boolean langwinipcprocessstarted (void) {
	
	/*
	we don't want Frontier's menus to dim when serving another application's 
	doscript or trap call.
	*/
	
	processnotbusy ();
	
	return (true);
	} /*langwinipcprocessstarted*/


static boolean langwinipcruncode (hdltreenode hcode, hdlhashtable hcontext, langerrormessagecallback errorcallback, tyvaluerecord * vreturned) {
	
	/*
	2.1b12 dmb: shared code between trap and doscript verbs.
	
	we always consume hcode
	
	2.1b14 dmb: take hcontext parameter for special kernel call case
	
	4.0b7 dmb: fixed double-dispose memory bug when
	*/
	
	hdlprocessrecord hprocess;
	register boolean fl;
	
	if (!newprocess (hcode, true, nil, 0L, &hprocess)) {
		
		langdisposetree (hcode);
		
		return (false);
		}
		
	(**hprocess).processrefcon = (long) vreturned;
	
	if (errorcallback != NULL)
		(**hprocess).errormessagecallback = errorcallback;
	
	(**hprocess).processstartedroutine = &langwinipcprocessstarted;
	
	(**hprocess).hcontext = hcontext;
	
	fl = processruncode (hprocess, vreturned);
	
	disposeprocess (hprocess);
		
	return (fl);
	} /*langwinipcruncode*/




static boolean
langkernelbuildparamlist (
		hdltreenode		 hcode,
		tyvaluerecord	*listval,
		hdltreenode		*hparams,
		unsigned int	*errarg)
{
#pragma unused (hcode)
	
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
	tyvaluerecord val;
	hdltreenode hparam;
	boolean flpushedroot;
#ifdef SUPPORT_NAMED_PARAMS_IN_FRONTIER_COM
	boolean flnamedparams;
	byte bskey [6];
	tyvaluerecord vkey;
	hdltreenode hname, hnamelist;
	OSErr err;
#endif
	register boolean fl = false;
	
	if (currenthashtable == nil)
		flpushedroot = pushhashtable (roottable);
	else
		flpushedroot = false;
	
	if (!langgetlistsize (listval, &ctparams))
		goto exit;
	
	for (i = 1; i <= ctparams; i++) {

		if (!langgetlistitem (listval, i, NULL, &val)) {

			if (errarg != NULL)
				*errarg = i;

			goto exit;
			}

		exemptfromtmpstack (&val); /*its data is about to belong to code tree*/
		
		if (!newconstnode (val, &hparam))
			goto exit;
		
#ifdef SUPPORT_NAMED_PARAMS_IN_FRONTIER_COM
//		if (flnamedparams && i > 1) { // 5.0d14 dmb
//			
//			ostypetostring (param.key, bskey);
//			
//			if (!findnamedparam (hnamelist, bskey)) { // trap isn't expecting this param
//				
//				langdisposetree (hparam);
//				
//				continue;
//				}
//			
//			if (!setstringvalue (bskey, &vkey) || !newidnode (vkey, &hname)) {
//				
//				langdisposetree (hparam);
//				
//				goto exit;
//				}
//			
//			exemptfromtmpstack (&vkey);
//			
//			if (!pushbinaryoperation (fieldop, hname, hparam, &hparam))
//				goto exit;
//			}
#endif
		
		if (hlist == nil)
			hlist = hparam;
		else
			pushlastlink (hparam, hlist);
		} /*for*/
	
	fl = true;
	
	exit:
	
	if (flpushedroot)
		pophashtable ();
	
	if (fl)
		*hparams = hlist; /*nil if there weren't any params*/
	else
		langdisposetree (hlist); /*checks for nil*/
	
	return (fl);
	} /*langkernelbuildparamlist*/


boolean langipcrunscript (bigstring bsscriptname, tyvaluerecord *vparams, hdlhashtable hcontext, tyvaluerecord *vreturned) {
	
	/*
	5.0.2b6 rab/dmb: new verb
	
	5.0.2b7 dmb: preserve errormessagecallback through the call

	6.0a14 dmb: fixed potential memory leak in error case.
	*/
	
	bigstring bsverb;
	register boolean fl = false;
	tyvaluerecord val;
	hdltreenode hfunctioncall;
	hdltreenode hparamlist;
	hdltreenode hcode;
	hdlhashtable htable;
	tyvaluerecord vhandler;
	hdlhashnode handlernode;
	
	pushhashtable (roottable);

	fl = langexpandtodotparams (bsscriptname, &htable, bsverb);

	if (fl) {
	
		if (htable == nil)
			langsearchpathlookup (bsverb, &htable);
		}
	
	pophashtable();

	if (! fl)
		return (false);
	
	if (!hashtablelookupnode (htable, bsverb, &handlernode)) {
		
		langparamerror (unknownfunctionerror, bsverb);
		
		return (false);
		}
	
	vhandler = (**handlernode).val;
	
	/*build a code tree and call the handler, with our error hook in place*/
	
	hcode = nil;
	
	if ((**htable).valueroutine == nil) { /*not a kernel table*/
		
		if (!langexternalvaltocode (vhandler, &hcode)) {

			langparamerror (notfunctionerror, bsverb);

			goto exit;
			}
		
		if (hcode == nil) { /*needs compilation*/
			
			if (!langcompilescript (handlernode, &hcode))
				goto exit;
			}
		}
		
	setaddressvalue (htable, bsverb, &val);
	
	if (!pushfunctionreference (val, &hfunctioncall))
		goto exit;
	
	if (hcontext != nil)
		pushhashtable (hcontext);
	
	fl = langkernelbuildparamlist (hcode, vparams, &hparamlist, NULL);
	
	if (hcontext != nil)
		pophashtable ();
	
	if (!fl) {
		
		setstringvalue (BIGSTRING ("0x31" "Can't complete call because of a parameter error."), vreturned);
		
		langdisposetree (hfunctioncall);
		
		goto exit;
		}
	
	if (!pushfunctioncall (hfunctioncall, hparamlist, &hcode)) /*consumes input parameters*/
		goto exit;
	
	if (!pushbinaryoperation (moduleop, hcode, nil, &hcode)) /*needs this level*/
		goto exit;
	
	fl = langwinipcruncode (hcode, hcontext, langcallbacks.errormessagecallback, vreturned);
	
	if (fl)
		pushvalueontmpstack (vreturned);
	
	exit:
	
	return (fl);
	} /*langipcrunscript*/
