
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

#include "strings.h"
#include "menu.h"
#include "quickdraw.h"
#include "smallicon.h"
#include "popup.h"
#include "kb.h"
#include "cursor.h"
#include "windowlayout.h"
#include "lang.h"
#include "langexternal.h"
#include "scripts.h"
#include "tablestructure.h"
#include "cancoon.h"
#include "cancooninternal.h"



typedef struct agentpopupinfo {
	
	hdlmenu hagentmenu;

	short checkedagentitem;

	short ctagentsvisited;
	} tyagentpopupinfo, *ptragentpopupinfo;


static boolean ccagentpopupvisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
	
	hdltreenode hcode;
	ptragentpopupinfo info = (ptragentpopupinfo) refcon;
	
	if (!langexternalvaltocode (val, &hcode)) /*not a scipt, or no code*/
		return (false);
	
	++(*info).ctagentsvisited;
	
	if (hcode == (**cancoondata).hprimaryagent)
		(*info).checkedagentitem = (*info).ctagentsvisited;
	
	if ((*info).hagentmenu == nil) /*not filling out a menu*/
		return (false);
	
	return (!pushpopupitem ((*info).hagentmenu, bsname, true, 0)); /*terminate visit on error*/
	} /*ccagentpopupvisit*/


static boolean ccfillagentpopup (hdlmenu hmenu, short *checkeditem) {
	
	tyagentpopupinfo info;
	
	if (agentstable == nil) /*defensive driving*/
		return (false);
	
	info.checkedagentitem = -1; /*default*/
	
	info.hagentmenu = hmenu;
	
	info.ctagentsvisited = 0;
	
	if (hashsortedinversesearch (agentstable, &ccagentpopupvisit, &info))
		return (false);
	
	*checkeditem = info.checkedagentitem;
	
	return (true);
	} /*ccfillagentpopup*/


static boolean ccagentselectvisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
	
	/*
	6/28/91 dmb: never toggle agent off; selecting current agent does nothing
	
	12/28/91 dmb: when an agent is selected and it's code isn't found, install it
	*/
	
	hdltreenode hcode;
	bigstring bs;
	ptragentpopupinfo info = (ptragentpopupinfo) refcon;
	
	if (!langexternalvaltocode (val, &hcode)) /*not a scipt*/
		return (false);
	
	if (++(*info).ctagentsvisited == (*info).checkedagentitem) {
		
		register hdlcancoonrecord hc = cancoondata;
		hdlprocessrecord hprocess;
		
		/*
		if ((**hc).hprimaryagent == hcode) /%already selected -- toggle to off, no selection%/
			(**hc).hprimaryagent = nil;
		else
		*/
		
			(**hc).hprimaryagent = hcode;
		
		if ((*info).hagentmenu != nil) { /*responding to a menu click*/
			
			if (processfindcode (hcode, &hprocess))
				copystring ((**hprocess).bsmsg, bs);
			
			else {
				
				scriptinstallagent (hnode);
				
				copystring ((ptrstring) "\x01" " ", bs);
				}
			
			ccmsg (bs, false); /*update existing message*/
			
			ccmsg (zerostring, false); /*unblock agents*/
			
			if (keyboardstatus.floptionkey || optionkeydown ())
				langexternalzoom (val, agentstable, bsname);
			}
		
		return (true); /*we're done, stop visiting*/
		}
	
	return (false); /*keep visiting*/
	} /*ccagentselectvisit*/


static boolean ccagentpopupselect (hdlmenu hmenu, short itemselected) {
	
	tyagentpopupinfo info;
	
	info.checkedagentitem = itemselected;
	
	info.hagentmenu = hmenu;
	
	info.ctagentsvisited = 0;
	
	hashsortedinversesearch (agentstable, &ccagentselectvisit, &info);
	
	return (true);
	} /*ccagentpopupselect*/


boolean ccagentpopuphit (Rect rpopup, Point pt) {
	
	return (popupmenuhit (rpopup, true, &ccfillagentpopup, &ccagentpopupselect));
	} /*ccagentpopuphit*/


void ccupdateagentpopup (Rect rpopup) {
	
	/*
	bigstring bs;
	
	copystring (zerostring, bs);
	
	drawpopup (rpopup, bs, true);
	*/
	
	short extrawidth = (rpopup.right - rpopup.left - popuparrowwidth);
	
	insetrect (&rpopup, extrawidth / 2, 0);
	
	displaypopupicon (rpopup, true);	
	} /*ccupdateagentpopup*/


boolean ccgetprimaryagent (short *ixagent) {
	
	return (ccfillagentpopup (nil, ixagent));
	} /*ccgetprimaryagent*/


boolean ccsetprimaryagent (short ixagent) {
	
	if (ixagent <= 0)
	//	return (false);
		ixagent = 1;
	
	return (ccagentpopupselect (nil, ixagent));
	} /*ccsetprimaryagent*/


boolean cccodereplaced (hdltreenode holdcode, hdltreenode hnewcode)  {
	
	register hdlcancoonrecord hc = cancoonglobals;
	
	if (hc != nil) {
		
		if ((**hc).hprimaryagent == holdcode)
			(**hc).hprimaryagent = hnewcode;
		}
	
	return (true);
	} /*cccodereplaced*/




