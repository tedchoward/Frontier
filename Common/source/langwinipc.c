
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

#ifdef MACVERSION
#include "langxcmd.h"
#endif

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
#include "base64.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "process.h"
#include "processinternal.h"
#include "kernelverbdefs.h"
#include "winsocknetevents.h"
#include "notify.h"
#include "timedate.h"


boolean langwinipcerrorroutine (bigstring bs, ptrvoid refcon) {
	
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


#ifdef WIN95VERSION

static void langwinipchookerrors (langerrormessagecallback *savecallback) {
	
	*savecallback = langcallbacks.errormessagecallback;
	
	langcallbacks.errormessagecallback = &langwinipcerrorroutine;
	
	shellpusherrorhook ((errorhookcallback) &langwinipcerrorroutine);
	
//	hipcverb = hverb; /*make visible to errorroutine*/
	} /*langwinipchookerrors*/


static boolean langwinipcunhookerrors (langerrormessagecallback savecallback) {
	
	shellpoperrorhook ();
	
	langcallbacks.errormessagecallback = savecallback;
	
	fllangerror = false; /*make sure error flag is cleared*/
	
//	hipcverb = nil;
	
	return (true);
	} /*langwinipcunhookerrors*/


static boolean langwinipccoerceparam (VARIANTARG * var, tyvaluerecord * val);

boolean convertBSTRVariantToValue (VARIANTARG * var, tyvaluerecord * val) {
	int cSize;
	int cOut;
	Handle h;

	if (V_VT(var) == VT_BSTR) {  /*just make sure it is a string */
		if (V_BSTR(var) != NULL) {
			cSize = WideCharToMultiByte(CP_ACP,0,V_BSTR(var),-1,NULL,0,NULL,NULL);

			if (cSize != 0) {
				if (newhandle ((cSize+1), &h)) {
					cOut = WideCharToMultiByte(CP_ACP,0,V_BSTR(var),-1,*h,cSize,NULL,NULL);

					if (cOut != 0) {
						sethandlesize (h, cOut-1);

						setheapvalue(h, stringvaluetype, val);

						return (true);
						}

					disposehandle (h);
					}
				}
			}
		}

	return (false);
	} /*convertBSTRVariantToValue*/

static boolean insertToList (hdllistrecord hlist, tyvaluerecord * val) {
	boolean res;

	res = langpushlistval (hlist, nil, val);

	return (res);
	} /*insertToList*/

static boolean insertToTable (hdlhashtable htable, tyvaluerecord * val2, tyvaluerecord * val1) {
	bigstring name;
	boolean res;
	long ct;
	bigstring serializedname;
	
	if (val2->valuetype != stringvaluetype)
		return (false);

	hashcountitems (htable, &ct);
	
	numbertostring (ct + 1, serializedname);
	
	while (stringlength (serializedname) < 4)
		insertchar ('0', serializedname);
	
	pushchar ('\t', serializedname);

	pullstringvalue (val2, name);

	//uncomment out the line below if we think that matrix (tables) should be order dependent.
	//I'm mixed, right now I think not.  It is the name that matters.
		
//	insertstring (serializedname, name);

	pushhashtable (htable);

	res = hashinsert (name, *val1);

	pophashtable();

	return (res);
	} /*insertToTable*/

static boolean getArrayElement (VARTYPE elementType, SAFEARRAY *psa, long * index, tyvaluerecord * val) {
	boolean res;
	HRESULT hres;
	VARIANTARG var;

	res = true;

	VariantInit (&var);

	switch (elementType) {
		case VT_EMPTY:
		case VT_NULL:
			setlongvalue (0, val);
			break;

		case VT_I2: {
			short fooI2;

			hres = SafeArrayGetElement (psa, index, &fooI2);

			if (FAILED(hres))
				return (false);

			setintvalue (fooI2, val);
			break;
			}

		case VT_I4: {
			long fooI4;

			hres = SafeArrayGetElement (psa, index, &fooI4);

			if (FAILED(hres))
				return (false);

			setlongvalue (fooI4, val);
			break;
			}

		case VT_R4: {
			float fooR4;

			hres = SafeArrayGetElement (psa, index, &fooR4);

			if (FAILED(hres))
				return (false);

			setsinglevalue (fooR4, val);
			break;
			}

		case VT_R8: {
			double fooR8;

			hres = SafeArrayGetElement (psa, index, &fooR8);

			if (FAILED(hres))
				return (false);

			setdoublevalue (fooR8, val);
			break;
			}

		case VT_BOOL: {
			short fooBOOL;

			hres = SafeArrayGetElement (psa, index, &fooBOOL);

			if (FAILED(hres))
				return (false);

			setbooleanvalue (fooBOOL, val);
			break;
			}

		case VT_UI1: {
			unsigned char fooUI1;

			hres = SafeArrayGetElement (psa, index, &fooUI1);

			if (FAILED(hres))
				return (false);

			setcharvalue (fooUI1, val);
			break;
			}

		case VT_BSTR: {
			BSTR fooBSTR;

			hres = SafeArrayGetElement (psa, index, &fooBSTR);

			if (FAILED(hres))
				return (false);

			V_VT(&var) = VT_BSTR;

			V_BSTR(&var) = fooBSTR;

			return (convertBSTRVariantToValue (&var, val));
			break;
			}

		case VT_DATE: {
			DATE fooDATE;

			hres = SafeArrayGetElement (psa, index, &fooDATE);

			if (FAILED(hres))
				return (false);

			V_VT(&var) = VT_DATE;

			V_DATE(&var) = fooDATE;

			return (langwinipccoerceparam (&var, val));
			break;
			}

		case VT_CY: {
			CY fooCY;

			hres = SafeArrayGetElement (psa, index, &fooCY);

			if (FAILED(hres))
				return (false);

			V_VT(&var) = VT_CY;

			V_CY(&var) = fooCY;

			return (langwinipccoerceparam (&var, val));
			break;
			}

		case VT_VARIANT: {
			hres = SafeArrayGetElement (psa, index, &var);

			if (FAILED(hres))
				return (false);

			return (langwinipccoerceparam (&var, val));
			break;
			}


		case VT_ERROR:
		case VT_DISPATCH:
		case VT_UNKNOWN:
		default:
			res = false;
		}

	return (res);
	} /*getArrayElement*/


static boolean langwinipccoercearray (VARIANTARG * var, tyvaluerecord * val) {
	SAFEARRAY FAR *psa;
	HRESULT hres;
	unsigned int dimCount, elementSize;
	long ub1, lb1, ub2, lb2, k;
	long index[2];		/*we only support upto 2 dimensions */
	tyvaluerecord val1, val2;
	hdllistrecord hlist;
	hdlhashtable htable;
	VARTYPE elementType;

	hlist = NULL;
	htable = NULL;

	if (V_ISBYREF(var))
		psa = *V_ARRAYREF(var);
	else
		psa=V_ARRAY(var);

	dimCount = SafeArrayGetDim(psa);

	elementSize = SafeArrayGetElemsize (psa);

	elementType = V_VT(var) & VT_TYPEMASK;

	if (dimCount > 2)
		return (false);		/*to many dimensions, we only handle 1 for lists and 2 for records */

	hres = SafeArrayGetLBound(psa, 1, &lb1);

	if (FAILED(hres))
		return (false);		/* Failure here not good, but safe exit */

	hres = SafeArrayGetUBound(psa, 1, &ub1);

	if (FAILED(hres))
		return (false);		/* Failure here not good, but safe exit */
	
	if (dimCount == 2) {
		hres = SafeArrayGetLBound(psa, 2, &lb2);

		if (FAILED(hres))
			return (false);		/* Failure here not good, but safe exit */

		hres = SafeArrayGetUBound(psa, 2, &ub2);

		if (FAILED(hres))
			return (false);		/* Failure here not good, but safe exit */

		if ((ub2-lb2) != 1)
			return (false);		/*only support matrix of 2 by n */
		}
	else {
		lb2 = 0;
		ub2 = 0;
		}

	for (k = lb1; k <= ub1; k++) {
		index[0] = k;
		index[1] = lb2;

		if (! getArrayElement (elementType, psa, index, &val1))
			goto arrayerrorexit;

		if (dimCount == 2) {
			index[1] = lb2+1;

			if (! getArrayElement (elementType, psa, index, &val2)) {
				disposevaluerecord (val1, false);

				goto arrayerrorexit;
				}

			if (htable == NULL) {
				if (!langexternalnewvalue (idtableprocessor, nil, val)) {
					disposevaluerecord (val1, false);

					disposevaluerecord (val2, false);

					goto arrayerrorexit;
					}
				
				langexternalvaltotable (*val, &htable, HNoNode);
				}

			if (htable != NULL)
				if (! insertToTable (htable, &val1, &val2)) {
					disposevaluerecord (val1, false);

					disposevaluerecord (val2, false);

					goto arrayerrorexit;  /*needs to cleanup*/
					}
			}
		else {
			if (hlist == NULL) {
				if (! opnewlist (&hlist, false)) {
					disposevaluerecord (val1, false);

					goto arrayerrorexit;
					}
				}

			if (hlist != NULL)
				if (! insertToList (hlist, &val1)) {
					disposevaluerecord (val1, false);

					goto arrayerrorexit;  /*needs to cleanup*/
					}
			}
		}

	if (dimCount == 2) {
//			initvalue (val, tablevaluetype);
//			val->data.binaryvalue = (Handle) htable;
		}
	else {
		initvalue (val, listvaluetype);
		val->data.listvalue = hlist;
		}

	//there is nothing to clean up here since everything is contained in the val record.
	return (true);

arrayerrorexit:
	if (hlist != NULL)
		opdisposelist (hlist);

	if (htable != NULL)
		disposevaluerecord (*val, false);

	setnilvalue (val);		//safety

	return (false);
	} /*langwinipccoercearray*/


static boolean langwinipccoerceparam (VARIANTARG * var, tyvaluerecord * val) {
	VARIANTARG var2;	/*used for conversion*/
	VARIANT var3;
	boolean res, fl;
	HRESULT hr;

	res = true;

	VariantInit (&var2);
	VariantInit (&var3);

	if ((V_ISBYREF (var)) && (! V_ISARRAY (var))) {
		fl = false;

		hr = VariantCopyInd (&var3, var);

		if (SUCCEEDED(hr)) {
			fl = langwinipccoerceparam ((VARIANTARG *)&var3, val);

			VariantClear (&var3);	  /*we need to delete this copy */
			}

		return (fl);
		}

	if (V_ISVECTOR (var)) {
		return (false);
		}

	if (V_ISARRAY (var)) {
		return (langwinipccoercearray (var, val));
		}

	switch (V_VT(var) & VT_TYPEMASK) {
		case VT_EMPTY:
		case VT_NULL:
			setlongvalue (0, val);
			break;

		case VT_I2:
			setintvalue (V_I2(var), val);
			break;

		case VT_I4:
			setlongvalue (V_I4(var), val);
			break;

		case VT_R4:
			setsinglevalue (V_R4(var), val);
			break;

		case VT_R8:
			setdoublevalue (V_R8(var), val);
			break;

		case VT_BSTR:
			return (convertBSTRVariantToValue (var, val));
			break;

		case VT_BOOL:
			setbooleanvalue (V_BOOL(var), val);
			break;

		case VT_UI1:
			setcharvalue (V_UI1(var), val);
			break;


		case VT_DATE:
		case VT_CY:
			{
			hr = VariantChangeType (&var2, var, 0, VT_BSTR);

			if (SUCCEEDED(hr)) {
				fl = convertBSTRVariantToValue (&var2, val);

				VariantClear (&var2);  /*we need to delete this copy */

				res = fl;
				}
			else
				res = false;
			break;
			}

		case VT_ERROR:
		case VT_DISPATCH:
		case VT_UNKNOWN:
		case VT_VARIANT:
		default:
			res = false;
		}

	return (res);
	} /*langwinipccoerceparam*/


static boolean langwinipcbuildparamlist (hdltreenode hcode, DISPPARAMS* pDispParams, hdltreenode *hparams, unsigned int * errarg, boolean paramOrder) {
	
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
	
	ctparams = pDispParams->cArgs;
	
	if (paramOrder == true) {  //use fixed foward order
		for (i = ctparams; i >= 1; i--) {
			
			if (!langwinipccoerceparam (&pDispParams->rgvarg[i-1], &val)) {
				if (errarg != NULL)
					*errarg = i;

				goto exit;
				}

			exemptfromtmpstack (&val); /*its data is about to belong to code tree*/
			
			if (!newconstnode (val, &hparam))
				goto exit;
						
			if (hlist == nil)
				hlist = hparam;
			else
				pushlastlink (hparam, hlist);
			} /*for*/
		}
	else {
		for (i = 1; i <= ctparams; i++) {
			if (!langwinipccoerceparam (&pDispParams->rgvarg[i-1], &val)) {
				if (errarg != NULL)
					*errarg = i;

				goto exit;
				}

			exemptfromtmpstack (&val); /*its data is about to belong to code tree*/
			
			if (!newconstnode (val, &hparam))
				goto exit;
		
#ifdef SUPPORT_NAMED_PARAMS_IN_FRONTIER_COM
//			if (flnamedparams && i > 1) { // 5.0d14 dmb
//			
//				ostypetostring (param.key, bskey);
//			
//				if (!findnamedparam (hnamelist, bskey)) { // trap isn't expecting this param
//				
//					langdisposetree (hparam);
//				
//					continue;
//					}
//			
//				if (!setstringvalue (bskey, &vkey) || !newidnode (vkey, &hname)) {
//				
//					langdisposetree (hparam);
//				
//					goto exit;
//					}
//			
//				exemptfromtmpstack (&vkey);
//			
//				if (!pushbinaryoperation (fieldop, hname, hparam, &hparam))
//					goto exit;
//				}	
#endif
		
			if (hlist == nil)
				hlist = hparam;
			else
				pushlastlink (hparam, hlist);
			} /*for*/
		} /*else use parameter old backwards way*/

	fl = true;
	
	exit:
	
	if (flpushedroot)
		pophashtable ();
	
	if (fl)
		*hparams = hlist; /*nil if there weren't any params*/
	else
		langdisposetree (hlist); /*checks for nil*/
	
	return (fl);
	} /*langwinipcbuildparamlist*/

#endif


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


#ifdef WIN95VERSION

boolean langwinipchandleCOM (bigstring bsscriptname, void* pDispParams, tyvaluerecord * retval, boolean *flfoundhandler, unsigned int * errarg) {
	
	/*
	6.0a14 dmb: fixed potential memory leak in error case.
	*/

	bigstring bsverb;
	register boolean fl = false;
	tyvaluerecord val;
	langerrormessagecallback savecallback;
	hdltreenode hfunctioncall;
	hdltreenode hparamlist;
	hdltreenode hcode;
	hdlhashtable htable;
	hdlhashtable hcontext = nil;
	tyvaluerecord vhandler;
	Handle hthread = nil;
	hdlhashnode handlernode;
	boolean paramOrder = false;

	*flfoundhandler = false;
	
	if (retval->data.longvalue == 1)
		paramOrder = true;					//rab 9/3/98 fix for parameter order.

	disablelangerror();

	pushhashtable (roottable);

	fl = langexpandtodotparams (bsscriptname, &htable, bsverb);

	if (fl) {
	
		if (htable == nil)
			langsearchpathlookup (bsverb, &htable);
		}
	
	pophashtable();
	
	enablelangerror();

	if (! fl) {
		/*generate an error message*/
		return (false);
		}

	if (!hashtablelookupnode (htable, bsverb, &handlernode))
		return (false);
	
	vhandler = (**handlernode).val;
	
	*flfoundhandler = true;
			
	/*build a code tree and call the handler, with our error hook in place*/
	
	langwinipchookerrors (&savecallback);
	
	hcode = nil;
	
	if ((**htable).valueroutine == nil) { /*not a kernel table*/
		
		if (!langexternalvaltocode (vhandler, &hcode))
			goto exit;
		
		if (hcode == nil) { /*needs compilation*/
			
			if (!langcompilescript (handlernode, &hcode))
				goto exit;
			}
		}
	
//	if (iskernelverb (hv)) { /*special case -- kernel verb specifies context*/
//		
//		if (!landgetlongparam (hv, keycurrenttable, (long *) &hcontext))
//			goto exit;
//		}
	
	setaddressvalue (htable, bsverb, &val);
	
	if (!pushfunctionreference (val, &hfunctioncall))
		goto exit;
	
//	if (hcontext != nil)
//		pushhashtable (hcontext);
	
	fl = langwinipcbuildparamlist (hcode,  (DISPPARAMS*)pDispParams, &hparamlist, errarg, paramOrder);
	
//	if (hcontext != nil)
//		pophashtable ();
	
	if (!fl) {
		setlongvalue ((long) ResultFromScode(DISP_E_TYPEMISMATCH), retval);

		langdisposetree (hfunctioncall);
		
		goto exit;
		}
	
	if (!pushfunctioncall (hfunctioncall, hparamlist, &hcode)) /*consumes input parameters*/
		goto exit;
	
	if (!pushbinaryoperation (moduleop, hcode, nil, &hcode)) /*needs this level*/
		goto exit;
	
	fl = langwinipcruncode (hcode, hcontext, langwinipcerrorroutine, retval);
	
	exit:
	
	langwinipcunhookerrors (savecallback);
	
	return (fl);
	} /*langwinipchandleCOM*/

#endif


static boolean langkernelbuildparamlist (hdltreenode hcode, tyvaluerecord * listval, hdltreenode *hparams, unsigned int * errarg) {
	
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
		
		setstringvalue ("0x31" "Can't complete call because of a parameter error.", vreturned);
		
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
