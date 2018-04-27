
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

#include "resources.h"
#include "lang.h"
#include "langexternal.h"
#include "tablestructure.h"
#include "cancoon.h"
#include "cancooninternal.h"
#include "kernelverbdefs.h"
#include "about.h"




#define cancoonerrorlist 268 
#define notimplementederror 3


typedef enum tycancoontoken { 
	
	showflagfunc,
	
	hideflagfunc,
	
	showpopupfunc,
	
	hidepopupfunc,
	
	showbuttonsfunc,
	
	hidebuttonsfunc,

	showserverstatsfunc,
	
	/*
	setmsgfunc,
	*/
	
	ctcancoonverbs
	} tycancoontoken;



static boolean ccshowhideverb (hdltreenode hparam1, boolean flshow, boolean flshowing, callback toggleroutine, tyvaluerecord *vreturned) {
	
	/*
	set vreturned to false if the state of the flag is not changed.  otherwise, 
	call the toggle routine and return true.
	
	1/30/92 dmb: fixed typo; when toggling, set vreturned to true, not false!
	
	5.6.97 dmb: can no longer assume that the "root" window is cancoon. globals 
	are now set in ccfunctionvalue
	*/
	
	if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
		return (false);
	
	if (flshowing != flshow) { /*changing*/
	
	//	shellpushfrontrootglobals ();
		
		(*toggleroutine) ();
		
	//	shellpopglobals ();
		
		(*vreturned).data.flvalue = true;
		}
	
	return (true);
	} /*ccshowhideverb*/


static boolean ccshowflagverb (hdltreenode hparam1, boolean flshow, tyvaluerecord *vreturned) {
	
	/*
	set vreturned to false if the state of the flag is not changed.
	*/
	
	return (ccshowhideverb (hparam1, flshow, !(**cancoonglobals).flflagdisabled, (callback) &cctoggleflag, vreturned));
	} /*ccshowflagverb*/


static boolean ccshowpopupverb (hdltreenode hparam1, boolean flshow, tyvaluerecord *vreturned) {
	
	/*
	set vreturned to false if the state of the popup is not changed.
	*/
	
	return (ccshowhideverb (hparam1, flshow, !(**cancoonglobals).flpopupdisabled, (callback) &cctoggleagentspopup, vreturned));
	} /*ccshowpopupverb*/


static boolean ccshowbuttonsverb (hdltreenode hparam1, boolean flshow, tyvaluerecord *vreturned) {
	
	/*
	set vreturned to false if the state of the buttons is not changed.
	*/
	
	return (ccshowhideverb (hparam1, flshow, (**cancoonglobals).flbigwindow, (callback) &ccflipflag, vreturned));
	} /*ccshowbuttonsverb*/


static boolean ccmsgverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	bigstring bsmsg;
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 1, bsmsg))
		return (false);
	
	if (shellwindow != nil)
		shellpushrootglobals (shellwindow);
	else
		shellpushfrontrootglobals ();
	
	ccmsg (bsmsg, !processisoneshot (true));
	
	shellpopglobals ();
	
	return (setbooleanvalue (true, vreturned));
	} /*ccmsgverb*/


static boolean ccfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	8/23/91 dmb: don't push front globals around entire switch, since that 
	would disable background processes while the parameter to "msg" is 
	evaluated.
	
	8/25/92 dmb: commented-out setmsgfunc -- redundant & undocumented
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl = false;
	register short errornum = 0;
	
	setbooleanvalue (false, v); /*by default, cancoon functions return false*/
	
		// can no longer assume that the "root" window is cancoon
		if (!shellfindwindow (idcancoonconfig, &cancoonwindow, &cancoonwindowinfo, (Handle *) &cancoondata))
			return (true);
		
		shellpushglobals (cancoonwindow);

	switch (token) {
		
		case showflagfunc:
			fl = ccshowflagverb (hparam1, true, v);
			
			break;
		
		case hideflagfunc:
			fl = ccshowflagverb (hparam1, false, v);
			
			break;
		
		case showpopupfunc:
			fl = ccshowpopupverb (hparam1, true, v);
			
			break;
		
		case hidepopupfunc:
			fl = ccshowpopupverb (hparam1, false, v);
			
			break;
		
		case showbuttonsfunc:
			fl = ccshowbuttonsverb (hparam1, true, v);
			
			break;
		
		case hidebuttonsfunc:
			fl = ccshowbuttonsverb (hparam1, false, v);
			
			break;
		
		case showserverstatsfunc: {
			boolean flset;
			
			flnextparamislast = true;

			if (!getbooleanvalue (hparam1, 1, &flset))
				break;
												
			aboutsetstatsflag (flset);
						
			fl = setbooleanvalue (true, v);
			}
			
			break;
		
		/*
		case setmsgfunc:
			fl = ccmsgverb (hparam1, v);
			
			break;
		*/
		
		default:
			fl = false;
			
			errornum = notimplementederror;
			
			break;
		} /*switch*/
	
		shellpopglobals ();

	if (fl)
		return (true);
	
	if (errornum != 0) /*get error string*/
		getstringlist (cancoonerrorlist, errornum, bserror);
	
	return (false);
	} /*ccfunctionvalue*/


boolean ccinitverbs (void) {
	
	langcallbacks.msgverbcallback = &ccmsgverb;
	
	return (loadfunctionprocessor (idcancoonverbs, &ccfunctionvalue));
	} /*ccinitverbs*/




