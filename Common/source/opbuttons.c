
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

/*Synthetic buttons for outline windows.*/

#include "frontier.h"
#include "standard.h"

#include "shell.h"
#include "lang.h"
#include "oplist.h"
#include "frontier_strings.h"
#include "langxml.h"
#include "processinternal.h"
#include "ops.h"
#include "opbuttons.h"


void opbuttonsattach (hdlwindowinfo hinfo, hdlhashtable htable) {

	/*
	7.1b18 PBS: attach buttons to this outline.
	*/

	hdlhashnode hn;
	short ct = 0;
	hdllistrecord hlist;

	(**hinfo).flhidebuttons = false;

	(**hinfo).flsyntheticbuttons = true;

	opnewlist (&hlist, false);
	
	(**hinfo).buttonlist = (Handle) hlist;
	
	/*loop through all of the items in the table*/
	
	for (hn = (**htable).hfirstsort; hn != nil; hn = (**hn).sortedlink) {

		bigstring bsname;

		ct++;

		if (ct > 16) /*16 is max number of buttons*/
			break;

		gethashkey (hn, bsname);
		
		xmlgetname (bsname);

		oppushstring ((hdllistrecord) (**hinfo).buttonlist, nil, bsname);
		} /*for*/

	(**hinfo).buttonscripttable = (Handle) htable;
	} /*opbuttonsattach*/


boolean opbuttonstatus (short buttonnum, tybuttonstatus *status) {
#pragma unused (buttonnum)

	/*
	7.1b18 PBS: buttons are always displayed and enabled.
	*/
	
	if (outlinedata == NULL)
		return (false);

	(*status).flenabled = true;
	
	(*status).fldisplay = true;
	
	(*status).flbold = false; /*our buttons are never bold*/
	
	return (true);
	} /*opbuttonstatus*/


boolean opbutton (short buttonnum) {
	
	/*
	7.1b18 PBS: Handle a click in an outline button. Run
	the associated script.
	*/
	
	hdlhashtable ht = (hdlhashtable) (**shellwindowinfo).buttonscripttable;
	hdlhashnode hn;
	hdltreenode hcode;
	tyvaluerecord v;
	short ct = 0;

	/*Find the item in the table and call the script.*/
	
	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) {

		ct++;

		if (ct > 16) /*16 is max number of buttons*/
			break;

		if (ct == buttonnum) {
		
			langcompilescript (hn, &hcode);

			langexternalvaltocode ((**hn).val, &hcode);

			langruncode (hcode, nil, &v);

			disposevaluerecord (v, false);

			break;
			} /*if*/		
		} /*for*/

	return (true);
	} /*scriptbutton*/
