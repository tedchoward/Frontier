
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

#include "shell.h"
#include "shellhooks.h"




#define maxkeyboardhooks 2

static short ctkeyboardhooks = 0;

callback keyboardhooks [maxkeyboardhooks];


boolean shellpushkeyboardhook (callback routine) {
	
	if (ctkeyboardhooks >= maxkeyboardhooks) {
	
		shellinternalerror (idtoomanykeyboardhooks, STR_increase_maxkeyboardhooks);
		
		return (false);
		}
		
	keyboardhooks [ctkeyboardhooks++] = routine;
	
	return (true);
	} /*shellpushkeyboardhook*/


boolean shellcallkeyboardhooks (void) {
	
	/*
	we call all the routines that are hooked into the keyboard.
	
	kb.h global keyboardstatus is filled with information about the keystroke.
	
	if any of the hooks return false, we execute no more -- and we return false.
	the caller is expected to return, thereby giving a hook a way of consuming
	a keystroke.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	*/
	
	register short i;
	
	for (i = 0; i < ctkeyboardhooks; i++) {
		
		register callback cb;
		
		cb = keyboardhooks [i];
		
		if (!(*cb) ())
			return (false);
		} /*for*/
		
	return (true);
	} /*shellcallkeyboardhooks*/



#define maxdirtyhooks 2

static short ctdirtyhooks = 0;

callback dirtyhooks [maxdirtyhooks];


boolean shellpushdirtyhook (callback routine) {
	
	if (ctdirtyhooks >= maxdirtyhooks) {
	
		shellinternalerror (idtoomanydirtyhooks, STR_increase_maxdirtyhooks);
		
		return (false);
		}
		
	dirtyhooks [ctdirtyhooks++] = routine;
	
	return (true);
	} /*shellpushdirtyhook*/


boolean shellcalldirtyhooks (void) {
	
	/*
	we call all the routines that are hooked into the dirty event.  that is, 
	if a user action resulted in **anything** becoming dirty, we call all 
	the routines in the dirtyhooks array.
	
	if any of the hooks return false, we execute no more -- and we return false.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	*/
	
	register short i;
	
	for (i = 0; i < ctdirtyhooks; i++) {
		
		register callback cb;
		
		cb = dirtyhooks [i];
		
		if (!(*cb) ())
			return (false);
		} /*for*/
		
	return (true);
	} /*shellcalldirtyhooks*/



#define maxmenuhooks 3

static short ctmenuhooks = 0;

menuhookcallback menuhooks [maxmenuhooks];


boolean shellpushmenuhook (menuhookcallback routine) {
	
	if (ctmenuhooks >= maxmenuhooks) {
	
		shellinternalerror (idtoomanymenuhooks, STR_increase_maxmenuhooks);
		
		return (false);
		}
		
	menuhooks [ctmenuhooks++] = routine;
	
	return (true);
	} /*shellpushmenuhook*/


boolean shellcallmenuhooks (short idmenu, short iditem) {
	
	/*
	when the user selects a menu command, we call all the routines in
	the menuhooks array.  if none of the hooks handle the command, 
	the shell will attempt to handle it.
	
	if any of the hooks return false, we execute no more -- and we return false.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	*/
	
	register short i;
	
	for (i = 0; i < ctmenuhooks; i++) {
		
		register menuhookcallback cb;
		
		cb = menuhooks [i];
		
		if (!(*cb) (idmenu, iditem))
			return (false);
		} /*for*/
		
	return (true);
	} /*shellcallmenuhooks*/



#define maxeventhooks 5

static short cteventhooks = 0;

eventhookcallback eventhooks [maxeventhooks];


boolean shellpusheventhook (eventhookcallback routine) {
	
	if (cteventhooks >= maxeventhooks) {
	
		shellinternalerror (idtoomanyeventhooks, STR_increase_maxeventhooks);
		
		return (false);
		}
		
	eventhooks [cteventhooks++] = routine;
	
	return (true);
	} /*shellpusheventhook*/


boolean shellpopeventhook (void) {
	
	assert (cteventhooks > 0);
	
	--cteventhooks;
	
	return (true);
	} /*shellpopeventhook*/


boolean shellcalleventhooks (EventRecord *ev, WindowPtr w) {
	
	/*
	when any event is processed that could affect a window owned by the 
	application, we give the event hooks a chance to intercept the event
	before normal processing occurs.  in some cases, such as a suspend or 
	resume event, normal processing will occur whether or not any hook 
	returns false.
	
	if any of the hooks return false, we execute no more -- and we return false.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	*/
	
	register short i;
	
	for (i = 0; i < cteventhooks; i++) {
		
		register eventhookcallback cb;
		
		cb = eventhooks [i];
		
		if (!(*cb) (ev, w))
			return (false);
		} /*for*/
		
	return (true);
	} /*shellcalleventhooks*/



short cterrorhooks = 0;

errorhookcallback errorhooks [maxerrorhooks];


boolean shellpusherrorhook (errorhookcallback routine) {
	
	if (cterrorhooks >= maxerrorhooks) {
	
		shellinternalerror (idtoomanyerrorhooks, STR_increase_maxerrorhooks);		
		return (false);
		}
		
	errorhooks [cterrorhooks++] = routine;
	
	return (true);
	} /*shellpusherrorhook*/


boolean shellpoperrorhook (void) {
	
	assert (cterrorhooks > 0);
	
	--cterrorhooks;
	
	return (true);
	} /*shellpoperrorhook*/


boolean shellcallerrorhooks (bigstring bs) {
	
	/*
	we call all the routines that are hooked into the error event.  that is, 
	if a user action resulted in any error, we call all the routines in the 
	errorhooks array.
	
	if any of the hooks return false, we execute no more -- and we return false.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	
	1/31/91: we call the most recently pushed hooks first
	*/
	
	register short i = cterrorhooks;
	
	while (--i >= 0) {
		
		register errorhookcallback cb;
		
		cb = errorhooks [i];
		
		if (!(*cb) (bs))
			return (false);
		} /*for*/
		
	return (true);
	} /*shellcallerrorhooks*/



#define maxscraphooks 7

static short ctscraphooks = 0;

scraphookcallback scraphooks [maxscraphooks];


boolean shellpushscraphook (scraphookcallback routine) {
	
	if (ctscraphooks >= maxscraphooks) {
	
		shellinternalerror (idtoomanyscraphooks, STR_increase_maxscraphooks);
		
		return (false);
		}
		
	scraphooks [ctscraphooks++] = routine;
	
	return (true);
	} /*shellpushscraphook*/


boolean shellcallscraphooks (Handle hscrap) {
	
	/*
	we call all the routines that are hooked into the readscrap event.  that is, 
	if we're about it import the scrap, we call all the routines in the 
	scraphooks array so that they can look for their own type.
	
	if any of the hooks return false, we execute no more -- and we return false.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	
	1/31/91: we call the most recently pushed hooks first
	*/
	
	register short i = ctscraphooks;
	
	while (--i >= 0) {
		
		register scraphookcallback cb;
		
		cb = scraphooks [i];
		
		if (!(*cb) (hscrap))
			return (false);
		} /*for*/
		
	return (true);
	} /*shellcallscraphooks*/


#define maxmemoryhooks 3

static short ctmemoryhooks = 0;

memoryhookcallback memoryhooks [maxmemoryhooks];


boolean shellpushmemoryhook (memoryhookcallback routine) {
	
	if (ctmemoryhooks >= maxmemoryhooks) {
	
		shellinternalerror (idtoomanymemoryhooks, STR_increase_maxmemoryhooks);
		
		return (false);
		}
	
	memoryhooks [ctmemoryhooks++] = routine;
	
	return (true);
	} /*shellpushmemoryhook*/


boolean shellcallmemoryhooks (long *ctbytesneeded) {
	
	/*
	we call all the routines that are hooked into the low memory mechanism.
	
	each hook has the opportunity to release some memory, and reduce the 
	ctbytesneeded value accordingly.  any hook is free to release more or 
	less than the amound needed.
	
	if any of the hooks return false, we execute no more -- and we return false.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	*/
	
	register short i;
	
	for (i = 0; i < ctmemoryhooks; i++) {
		
		register memoryhookcallback cb;
		
		cb = memoryhooks [i];
		
		if (!(*cb) (ctbytesneeded))
			return (false);
		} /*for*/
	
	return (true);
	} /*shellcallmemoryhooks*/


#define maxfilehooks 1

static short ctfilehooks = 0;

static callback filehooks [maxfilehooks];


boolean shellpushfilehook (callback routine) {
	
	if (ctfilehooks >= maxfilehooks) {
	
		shellinternalerror (idtoomanyfilehooks, STR_increase_maxfilehooks);
		
		return (false);
		}
		
	filehooks [ctfilehooks++] = routine;
	
	return (true);
	} /*shellpushfilehook*/


boolean shellcallfilehooks (void) {
	
	/*
	we call all the routines that are hooked into file opening & closing.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	*/
	
	register short i;
	
	for (i = 0; i < ctfilehooks; i++) {
		
		register callback cb;
		
		cb = filehooks [i];
		
		(*cb) ();
		} /*for*/
	
	return (true);
	} /*shellcallfilehooks*/



#define maxwakeuphooks 2

static short ctwakeuphooks = 0;

static wakeuphookcallback wakeuphooks [maxwakeuphooks];


boolean shellpushwakeuphook (wakeuphookcallback routine) {
	
	if (ctwakeuphooks >= maxwakeuphooks) {
	
		shellinternalerror (idtoomanyfilehooks, STR_increase_maxfilehooks);
		
		return (false);
		}
		
	wakeuphooks [ctwakeuphooks++] = routine;
	
	return (true);
	} /*shellpushwakeuphook*/


boolean shellcallwakeuphooks (hdlprocessthread hthread) {
	
	/*
	we call all the routines that are hooked into wakeup opening & closing.
	
	note -- the hook routine can assume absolutely nothing about its environment.
	*/
	
	register short i;
	
	for (i = 0; i < ctwakeuphooks; i++) {
		
		register wakeuphookcallback cb;
		
		cb = wakeuphooks [i];
		
		if (!(*cb) (hthread))
			return (false);
		} /*for*/
	
	return (true);
	} /*shellcallwakeuphooks*/





