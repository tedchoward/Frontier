
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

	#include "OSXSpecifics.h"
	#include "player.h" /*7.0b4: QuickTime Player*/

#include "about.h"
#include "frontierconfig.h"
#include "db.h" /*stats*/
#include "file.h" /*6.1b15 AR: filestart*/
#include "shell.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "tableverbs.h"
#include "opverbs.h"
#include "scripts.h"
#include "menuverbs.h"
#include "pictverbs.h"
#include "wpverbs.h"
#include "cancoon.h"
#include "command.h"
#ifdef flcomponent
	#include "osacomponent.h"
#endif
#include "frontierstart.h"


boolean frontierstart (void) {
	
	/*
	do all initialization and starting up. on failure, exit to shell; only
	return to caller on success
	
	3.0.4b6 dmb: call osacomponentstart from here, after the shell
	is more thoroughly initialized.
	*/
	
	iddefaultconfig = idcancoonconfig;
//	iddefaultconfig = idscriptconfig;
	
	if (!filestart ()) /*6.1b16*/
		return (false);

	if (!opstart ())
		return (false);
	
	if (!menustart ())
		return (false);
	
	if (!tablestart ())
		return (false);
	
	if (!scriptstart ())
		return (false);
		
	if (!pictstart ())
		return (false);
	
	if (!wpstart ())
		return (false);
	
	
	if (!aboutstart ())
		return (false);
		

/*
		useQDText(0); // set to 1 for Quartz rendering.
*/
	
		if (!playerstart ()) /*7.0b4 PBS: QuickTime Player*/
			return (false);
	
	
	
	if (!cmdstart ())
		return (false);
	
	if (!langdialogstart ())
		return (false);
	
	if (!langerrorstart ())
		return (false);
	
	if (!statsstart ())
		return (false); 
	
	
	if (!ccstart ())
		return (false);
	
	#ifdef flcomponent
	
	//#if !TARGET_API_MAC_CARBON
	//Code change by Timothy Paustian Saturday, July 8, 2000 9:47:28 PM
	//The carbon version just does not play nice with the component manager.
	//at some point I have to figure this out.
	if (!osacomponentstart ())
		;	// don't quit if this doesn't work
	//#endif

	#endif
	
	if (!shellstart ())
		return (false);
	
	return (true);
	} /*frontierstart*/



