
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
#include "kb.h"
#include "memory.h"
#include "strings.h"
#include "lang.h"
#include "langexternal.h"
#include "tablestructure.h"
#include "menubar.h"
#include "menuverbs.h"
#include "meprograms.h"
#include "process.h"
#include "launch.h"
#include "langipc.h"



typedef struct tymenulistrecord {
	
	long id; /*this list's key is the program id*/
	
	hdloutlinerecord houtline; /*another way to look up a stack*/
	
	hdlmenubarstack hstack;
	
	struct tymenulistrecord **nextlist;
	} tymenulistrecord, **hdlmenulistrecord;


static hdlmenulistrecord hmenulist = nil;

static boolean flshuttingmenusdown = false;



static boolean langipcfindmenus (long id, hdloutlinerecord houtline, hdlmenulistrecord *hmenus) {
	
	register hdlmenulistrecord x = hmenulist;
	register boolean fl;
	
	while (x != nil) {
		
		if (houtline == nil)
			fl = (**x).id == id;
		else
			fl = (**x).houtline == houtline;
		
		if (fl) {
			
			*hmenus = x;
			
			return (true);
			}
		
		x = (**x).nextlist;
		} /*while*/
	
	*hmenus = nil;
	
	return (false);
	} /*langipcfindmenus*/
	

static boolean langipcfindmenubarstack (long id, hdlmenubarstack *hstack) {
	
	hdlmenulistrecord hmenus;
	
	*hstack = nil; /*default return*/
	
	if (!langipcfindmenus (id, nil, &hmenus))
		return (false);
	
	*hstack = (**hmenus).hstack;
	
	return (true);
	} /*langipcfindmenubarstack*/


boolean langipcgetmenuhandle (OSType id, short ixarray, Handle *hmenu) {
	
	hdlmenubarstack hstack;
	
	if (!langipcfindmenubarstack (id, &hstack))
		return (false);
	
	return (copyhandle ((Handle) (**hstack).stack [ixarray].hmenu, hmenu));
	} /*langipcgetmenuhandle*/


static boolean pushmenulist (long id, hdloutlinerecord houtline, hdlmenubarstack hstack) {
	
	hdlmenulistrecord hlistrecord;
	register hdlmenulistrecord h;
	
	if (!newclearhandle (sizeof (tymenulistrecord), (Handle *) &hlistrecord))
		return (false);
	
	h = hlistrecord; /*move into register*/
	
	(**h).id = id;
	
	(**h).houtline = houtline;
	
	(**h).hstack = hstack;
	
	(**h).nextlist = hmenulist;
	
	hmenulist = h;
	
	return (true);
	} /*pushmenulist*/


static void disposemenubarstack (hdlmenubarstack hstack) {
	
	hdlmenubarlist hsave = menubarlist;
	
	menubarlist = nil; /*so menu deallocation doesn't touch current bitmap*/
	
	medisposemenubar (hstack);
	
	menubarlist = hsave;
	} /*disposemenubarstack*/


static boolean disposemenulist (long id) {
	
	/*
	dispose the menulist structure with the indicated id and remove 
	it from the linked list.
	*/
	
	register hdlmenulistrecord x = hmenulist;
	register hdlmenulistrecord hnext = nil;
	register hdlmenulistrecord hprev = nil;
	
	while (x != nil) {
		
		hnext = (**x).nextlist;
		
		if ((**x).id == id) {
			
			disposemenubarstack ((**x).hstack);
			
			disposehandle ((Handle) x);
			
			if (hprev == nil)
				hmenulist = hnext;
			else
				(**hprev).nextlist = hnext;
			
			return (true);
			}
		
		hprev = x;
		
		x = hnext;
		} /*while*/
	
	return (false);
	} /*disposemenulist*/


static void disposeallmenulists (void) {
	
	while (hmenulist != nil) /*list isn't empty*/
		disposemenulist ((**hmenulist).id);
	} /*disposeallmenulists*/


static boolean getmenubartable (hdlhashtable *htable) {
	
	*htable = menubartable;
	
	return (menubartable != nil);
	} /*getmenubartable*/


static boolean getappmenurecord (long id, hdlmenurecord *hrecord) {
	
	/*
	get the menurecord for the application with the indicated id.
	*/
	
	hdlhashtable lmenubartable;
	bigstring bs;
	
	if (!getmenubartable (&lmenubartable))
		return (false);
	
	ostypetostring (id, bs);
	
	return (menugetmenubar (lmenubartable, bs, false, hrecord));
	} /*getappmenurecord*/


static boolean buildmenubarstack (hdlmenurecord hmenurecord, short firstmenuid, hdlmenubarstack *hstack) {
	
	/*
	build a menubar stack for the given menu record's outline.
	
	set up an empty menubarlist with the indicated baseid so that the 
	full range of menu ids is available, independent of the menus in 
	Frontier's menu bar.
	*/
	
	hdlmenubarlist hlist, savemenubarlist;
	register boolean fl;
	
	if (!newmenubarlist (&hlist))
		return (false);
	
	if (!menewmenubar ((**hmenurecord).menuoutline, hstack)) {
		
		disposemenubarlist (hlist);
		
		return (false);
		}
	
	(**hlist).basemenuid = firstmenuid;
	
	savemenubarlist = menubarlist;
	
	menubarlist = hlist;
	
	fl = mebuildmenubar (*hstack);
	
	if (!fl)
		medisposemenubar (*hstack);
	
	disposemenubarlist (hlist);
	
	menubarlist = savemenubarlist;
	
	return (fl);
	} /*buildmenubarstack*/


static boolean buildmenuarray (hdlmenubarstack hstack, hdlmenuarray *hmenuarray) {
	
	/*
	fill the menuarray with menus from the menubar stack.
	
	4.1b2 dmb: menu array in the (client) application zone. actually, let the client
	do it. if *hmenuarray is non-nil, just resize it.
	*/
	
	register tymenubarstackelement *pstack;
	register tyruntimemenurecord *pruntime;
	short ctmenus = (**hstack).topstack;
	
	#if 0 //def flnewfeatures
	
		/*
		THz savezone = GetZone ();
		
		SetZone (ApplicationZone ());
		
		*hmenuarray = (hdlmenuarray) NewHandleClear (ctmenus * sizeof (tyruntimemenurecord));
		
		SetZone (savezone);
		
		if (*hmenuarray == nil) {
		
			memoryerror ();
			
			return (false);
			}
		*/
		
	#else
		
		if (!newclearhandle (ctmenus * sizeof (tyruntimemenurecord), (Handle *) hmenuarray))
			return (false);
		
	#endif
	
	pstack = (**hstack).stack;
	
	pruntime = ***hmenuarray;
	
	while (--ctmenus >= 0) { /*copy the fields we care about into the runtime array*/
		
		(*pruntime).idmenu = (*pstack).idmenu;
		
		(*pruntime).flhierarchic = (*pstack).flhierarchic;
		
		(*pruntime++).hmenu = (*pstack++).hmenu;
		}
	
	return (true);
	} /*buildmenuarray*/


static boolean langipcsendmenumessage (tyapplicationid id, tyverbtoken message) {
	
	/*
	11/26/91 dmb: since this is just a notification, don't wait for a 
	reply.  these calls can be happening at odd times (while a script is 
	running; when quitting; etc.), so it may also prevent bugs like the one 
	just fixed in langipcmenubarchanged by avoiding the thread swapping that 
	occurs during a wait.
	*/
	
	hdlverbrecord hverb;
	typaramrecord param;
	bigstring bserror;
	short iderror;
	
	if (!landnewverb (id, nil, id, message, 0, &hverb))
		return (false);
	
	landverbgetsnoreply (hverb); /*no point in waiting around*/
	
	if (!landsendverb (hverb, &param, bserror, &iderror))
		return (false);
	
	landdisposeparamrecord (&param);
	
	return (true);
	} /*langipcsendmenumessage*/


#if !flruntime

static boolean langipcmenuprocessstarted (void) {
	
	/*
	we don't want Frontier's menus to dim when serving another application's 
	menu selection.
	*/
	
	processnotbusy ();
	
	return (true);
	} /*langipcmenuprocessstarted*/

#endif


static boolean langipcmenuprocesskilled (void) {
	
	/*
	send a message to the client application that asked for this script 
	to be run to let it know that the script has completed execution
	
	10/29/91 dmb: we now create the done verb when process is created, 
	so that errors can be hooked and reported back to caller.
	*/
	
	hdlverbrecord hverb;
	typaramrecord param;
	bigstring bserror;
	short iderror;
	
	hverb = (hdlverbrecord) (**currentprocess).processrefcon; /*the pre-created 'done' message*/
	
	(**hverb).verbtoken = idscriptcompleted; /*landreturnerror changes token; restore*/
	
	landverbgetsnoreply (hverb); /*no point in waiting around*/
	
	if (landsendverb (hverb, &param, bserror, &iderror))
		landdisposeparamrecord (&param);
	
	return (true);
	} /*langipcmenuprocesskilled*/


boolean langipcgetitemlangtext (long id, short idmenu, short iditem, Handle *htext, long *signature) {
	
	/*
	2/15/93 dmb: broke out this code to share w/osamenus.c
	*/
	
	hdlmenubarstack hstack;
	hdlheadrecord hnode;
	boolean fl;
	
	*htext = nil;
	
	if (!langipcfindmenubarstack (id, &hstack))
		return (false);
	
	pushmenubarglobals (hstack);
	
	#if flruntime
		
		if (memenuhit (idmenu, iditem, &hnode))
			fl = megetnodelangtext (hnode, htext, signature);
		else
			fl = false;
	
	#else
		
		menudata = (hdlmenurecord) (**outlinedata).outlinerefcon;
		
		if (memenuhit (idmenu, iditem, &hnode)) {
			
			if (optionkeydown ())
				fl = langipcshowmenunode ((long) hnode);
			else
				fl = megetnodelangtext (hnode, htext, signature);
			}
		else
			fl = false;
		
	#endif
	
	popmenubarglobals ();

	return (fl);
	} /*langipcgetitemlangtext*/


boolean langipccheckformulas (long id) {
	
	hdlmenubarstack hstack;
	register short i;
	register short topstack;
	
	if (!langipcfindmenubarstack (id, &hstack))
		return (false);
	
	pushmenubarglobals (hstack);
	
	topstack = (**hstack).topstack;
	
	for (i = 0; i < topstack; i++) /*search for the menu*/
		mecheckformulas (i);
	
	popmenubarglobals ();
	
	return (true);
	} /*langipccheckformulas*/


boolean langipcrunitem (long id, short idmenu, short iditem, long *refcon) {
	
	/*
	9/27/91 dmb: when the option key is down, bring ourself to the front 
	so that the user will see the selected menu item (a feature of memenuhit)
	
	10/29/91 dmb: always set keyboardstatus.floptionkey, not just when option 
	key is now down
	
	2/10/92 dmb: use new shellactivate instead of activateprogram
	
	2/16/93 dmb: decision of what to do w/selected node now rests with us, not memenuhit
	
	3.0a dmb: no more special case for runtime's execution of the process
	*/
	
	hdlmenubarstack hstack;
	boolean fl = false;
	register hdlprocessrecord hp;
	hdlverbrecord hverb;
	boolean fljustshownode;
	hdlheadrecord hnode;
	
	*refcon = 0; /*default return*/
	
	if (!langipcfindmenubarstack (id, &hstack))
		return (false);
	
	#if !flruntime
	
	fljustshownode = optionkeydown ();
	
	if (fljustshownode) /*option key is down -- we'll show item instead of running it*/
		shellactivate ();
	
	#endif
	
	pushmenubarglobals (hstack);
	
	newlyaddedprocess = nil; /*process manager global*/
	
	#if flruntime
	
		if (memenuhit (idmenu, iditem, &hnode))
			fl = meuserselected (hnode);
		
	#else
		
		if (memenuhit (idmenu, iditem, &hnode)) {
			
			if (fljustshownode)
				fl = meshownode (hnode);
			else
				fl = meuserselected (hnode);
			}
		
	#endif
	
	popmenubarglobals ();
	
	hp = newlyaddedprocess; /*process.c global; will be nil if a process wasn't just added*/
	
	if (hp != nil) {
		
		if (landnewverb (id, nil, id, idscriptcompleted, 0, &hverb)) { /*will send upon completion*/
			
			(**hp).processrefcon = (long) hverb;
			
			(**hp).errormessagecallback = &langipcerrorroutine;
			
			#if !flruntime
			
				(**hp).processstartedroutine = &langipcmenuprocessstarted;
			
			#endif
			
			(**hp).processkilledroutine = &langipcmenuprocesskilled;
			
			*refcon = (long) (**hp).hcode;
			}
		else { /*ran out of memory*/
			
			processkill (hp); /*don't want it to run*/
			
			fl = false; /*must return false to caller so it won't expect 'done' message*/
			}
		}
	
	#if 0 /*flruntime*/
		
		if (!fl)
			return (false);
		
		addprocess (hp); /*run it*/
		
	#else
		
		newlyaddedprocess = nil; /*clean up*/
		
		return (fl);
	
	#endif
	
	} /*langipcrunitem*/


boolean langipcgetmenuarray (long id, short firstmenuid, boolean flclientowned, Handle *hmenuarray) {
	
	/*
	6/1/93 dmb: make sure yielding is disabled when we build menus in case we 
	end up reducing a formula while servicing a fast event
	
	2.1b8 dmb: return false if we're in the process of shutting down
	
	2.1b13 dmb: about the flclientedowned mechanism... the issue: 
	with component menu sharing, there's only one copy of the menus. The 
	main benefit, aside from being more efficient, is that it makes it easier 
	to support recalculating formulas. but it also means that either Frontier 
	or the client has to refer to data structures in the other's heap. When 
	Frontier or the client quits, it's essential that this be managed carefully. 
	In particular, disposing a menu that is in the client's menu list will lead 
	to a crash, so we need to get the client to dispose its menus before we 
	finish quitting. (this is done by osacomponentshutdown).
	
	so crashing bugs led me to believe that the entire mechanism was inadequte, 
	but since the langipcmenu menulist structure is private, and only used to 
	serve shared menus, it's really pretty safe. the problem, I think, are some 
	timing issues that hadn't been considered. the new langipcdisposemenus 
	routine below should (I hope) clear this up.
	*/
	
	hdlmenubarstack hstack;
	hdlmenurecord hmenurecord;
	boolean fl;
	
	if (flshuttingmenusdown)
		return (false);
	
	if (!langipcfindmenubarstack (id, &hstack)) {
		
		if (!getappmenurecord (id, &hmenurecord))
			return (false);
		
		++fldisableyield;
		
		fl = buildmenubarstack (hmenurecord, firstmenuid, &hstack);
		
		--fldisableyield;
		
		if (!fl)
			return (false);
		
		if (!pushmenulist (id, (**hstack).menubaroutline, hstack)) {
			
			disposemenubarstack (hstack);
			
			return (false);
			}
		}
	
	if (!buildmenuarray (hstack, (hdlmenuarray *) hmenuarray))
		return (false);
	
	(**hstack).flclientowned = flclientowned;
	
	return (true);
	} /*langipcgetmenuarray*/


void langipcdisposemenuarray (long id, Handle hmenusarray) {
	
	/*
	2.1b13 dmb: with apple event menu sharing, a client will never dispose
	a menu array unless we've already disposed the corresponding menulist
	and sent it an 'updm' message. except when it's quitting, but we're not 
	informed when that happens.
	
	with component menu sharing, a client may actually get menus before 
	it receives an 'updm' message (on launch). it's important that we 
	not assume that the menulist isn't there when it disposes a menu array, 
	'cause if it is there its handles are invalid. on top of that, since 
	the component manager informs us when the client quits, this gives us 
	a chance to release unneeded memory.
	*/
	
	disposemenulist (id); /*release associated menu list*/
	
	disposehandle (hmenusarray);
	} /*langipcdisposemenuarray*/



#if !flruntime

static boolean langipcmenubarchanged (hdloutlinerecord houtline) {
	
	/*
	called from menubar.c when a menubar change (insert, delete, 
	reorg, etc.) occurs.
	
	since we maintain our own menubar stacks, we have to use the 
	outline itself as the key to finding a match in our list.
	
	this should probably be a callback, if there were menubarcallbacks.
	
	11/26/91 dmb: callback now gets called with outline, not menubarstack. 
	also, must disposemenulist before sending update message, in case 
	new menus are requested before message routine returns.
	*/
	
	hdlmenulistrecord hmenus;
	
	if (langipcfindmenus (0L, houtline, &hmenus)) {
		
		register long id = (**hmenus).id;
		
		disposemenulist (id); /*must rebuild from scratch*/
		
		langipcsendmenumessage (id, idupdatemenus);
		}
	
	return (true);
	} /*langipcmenubarchanged*/

#endif


static boolean notifyappvisit (hdlhashnode hnode, ptrvoid refcon) {
#pragma unused (refcon)

	/*
	if hnode is a menubar that may belong to an application, send the 
	updatemenustoken message to that application.
	
	always return true so all table nodes will be visited.
	
	2.1b13 dmb: under component menusharing, the client might ask for 
	(and get) its shared menus before we get here. do don't send a 
	notification if the menubar stack is already built.
	*/
	
	register hdlhashnode h = hnode;
	bigstring bskey;
	long id;
	hdlmenubarstack hstack;
	
	if (langexternalgettype ((**h).val) != idmenuprocessor) /*not a menubar value*/
		return (true);
	
	gethashkey (h, bskey);
	
	if (!stringtoostype (bskey, (OSType *) &id)) /*name longer than 4 characters*/
		return (true);
	
	if (!langipcfindmenubarstack (id, &hstack)) /*didn't already get them*/
		langipcsendmenumessage (id, idupdatemenus);
	
	return (true);
	} /*notifyappvisit*/


boolean langipcmenustartup (void) {
	
	/*
	visit the menubar table and send a message to each application 
	to let them know their menus are available
	*/
	
	hdlhashtable lmenubartable;
	
	if (!getmenubartable (&lmenubartable))
		return (false);
	
	hashtablevisit (lmenubartable, &notifyappvisit, nil);
	
	return (true);
	} /*langipcmenustartup*/


boolean langipcmenushutdown (void) {
	
	/*
	visit the menubar table and send a message to each application 
	to let them know their menus are no longer available
	
	2.1b8 dmb: since our component may have installed the 'done' handler, 
	we need to let the event be processed before quitting. so we have to 
	wait for a reply, and set the flshuttingmenusdown flag to prevent a 
	new set of menus from being built
	*/
	
	hdlhashtable lmenubartable;
	
	disposeallmenulists ();
	
	if (!getmenubartable (&lmenubartable))
		return (false);
	
	flshuttingmenusdown = true;
	
	hashtablevisit (lmenubartable, &notifyappvisit, nil);
	
	flshuttingmenusdown = false;
	
	return (true);
	} /*langipcmenushutdown*/


static boolean getmenubarid (hdlhashtable htable, const bigstring bs, tyapplicationid *id) {
	
	hdlhashtable lmenubartable;
	
	if (!getmenubartable (&lmenubartable))
		return (false);
	
	if (htable != lmenubartable)
		return (false);
	
	return (stringtoostype ((ptrstring) bs, id)); /*false if name is longer than 4 characters*/
	} /*getmenubarid*/


boolean langipcsymbolchanged (hdlhashtable htable, const bigstring bs, boolean flvalue) {
#pragma unused(flvalue)

	tyapplicationid id;
	
	if (getmenubarid (htable, bs, &id)) {
		
		langipcsendmenumessage (id, idupdatemenus);
		
		disposemenulist (id); /*must rebuild from scratch*/
		}
	
	return (true);
	} /*langipcsymbolchanged*/


boolean langipcsymbolinserted (hdlhashtable htable, const bigstring bs) {
	
	return (langipcsymbolchanged (htable, bs, true));
	} /*langipcsymbolinserted*/


boolean langipcsymboldeleted (hdlhashtable htable, const bigstring bs) {
	
	return (langipcsymbolchanged (htable, bs, true));
	} /*langipcsymboldeleted*/


boolean langipcmenuinit (void) {
	
	#if !flruntime
	
	menubarcallbacks.menubarchangedroutine = &langipcmenubarchanged;
	
	#endif
	
	return (true);
	} /*langipcmenuinit*/





