
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
#include "quickdraw.h"
#include "font.h"
#include "resources.h"
#include "strings.h"
#include "dialogs.h"
#include "shell.rsrc.h"
#include "frontierconfig.h"
#include "shell.h"


#ifdef MACVERSION
#define configresourcetype 'cnfg'
#endif

#ifdef WIN95VERSION
//#define configresourcetype 1000
#define configresourcetype 'cnfg'
#endif

tyconfigrecord config;

short iddefaultconfig = idtableconfig;




static void initconfigrecord (tyconfigrecord *configrecord) {
	
	register ptrconfigrecord p = configrecord;
	
	clearbytes (p, longsizeof (tyconfigrecord)); /*initalize all fields to 0*/
	
	(*p).flhorizscroll = true;
	
	(*p).flvertscroll = true;
	
	(*p).flmessagearea = true;
	
	(*p).flgrowable = true;
	
	(*p).messageareafraction = 3;
	
	(*p).filecreator = typeunknown;
	
	(*p).filetype = typeunknown;

#ifdef MACVERSION
	(*p).defaultfont = systemFont;
#endif
#ifdef WIN95VERSION
	(*p).defaultfont = 0;
#endif

	(*p).defaultsize = 12;
	} /*initconfigrecord*/


void loadconfigresource (short configresnum, tyconfigrecord *cr) {
	
	/*
	2.1b5 dmb: release the config resource when done with it
	*/
	
	register Handle h;
	bigstring bs;
	
	h = getresourcehandle (configresourcetype, configresnum);

	if (h != nil) {
	
		moveleft (*h, cr, sizeof (tyconfigrecord)); 
		
		getstringlist (fontnamelistnumber, (*cr).defaultfont, bs);
		
		fontgetnumber (bs, &(*cr).defaultfont);
		
		centerrectondesktop (&(*cr).defaultwindowrect);
	
		releaseresourcehandle (h);
		}
	else
		initconfigrecord (cr);
	} /*loadconfigresource*/
	
	
/*
boolean saveconfigresource (short configresnum, tyconfigrecord *cr) {

	Handle h;
	
	h = GetResource (configresourcetype, configresnum);
	
	if (h != nil) { /%resource already exists%/
	
		moveleft ((ptrchar) cr, (ptrchar) *h, longsizeof (tyconfigrecord)); 
		
		ChangedResource (h);
		}
	else {
		if (!newfilledhandle ((ptrchar) cr, longsizeof (tyconfigrecord), &h)) 
			return (false);
		
		AddResource (h, configresourcetype, configresnum, emptystring);
		}
		
	return (ResError () == noErr);
	} /%saveconfigresource*/
	
	
boolean getprogramname (bigstring bs) {
	
	return (getstringlist (defaultlistnumber, programname, bs));
	} /*getprogramname*/
	
	
boolean getuntitledfilename (bigstring bs) {
	
	/*
	5.0d6 dmb: added numeric sequencing
	*/
	
	static long untitledsequencer = 0;
	
	if (!getstringlist (defaultlistnumber, untitledfilename, bs))
		return (false);
	
	if (++untitledsequencer > 1) {
	
		pushchar ('-', bs);
		
		pushlong (untitledsequencer, bs);
		}
	
	return (true);
	} /*getuntitledfilename*/
	
	
boolean getdefaultfilename (bigstring bs) {
	
	return (getstringlist (defaultlistnumber, defaultfilename, bs));
	} /*getdefaultfilename*/
	
/*
boolean getusername (bigstring bs) {
	
	return (getstringlist (defaultlistnumber, username, bs));
	} /%getusername*/
	
	
void initconfig (void) {
	
	initconfigrecord (&config);
	} /*initconfig*/
	
	
	