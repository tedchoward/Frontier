
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

/*
7.0b4 PBS 08/09/00: QuickTime verbs.
*/

#include "frontier.h"
#include "standard.h"

#ifdef MACVERSION /*7.0b4 PBS: QuickTime verbs are Mac-only.*/

#include "memory.h"
#include "strings.h"
#include "ops.h"
#include "resources.h"
#include "timedate.h"
#include "lang.h"
#include "langinternal.h"
#include "langhtml.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "oplist.h"
#include "player.h"


typedef enum tyquicktimeverbtoken { /*verbs that are processed by langquicktime.c*/
	
	quicktimeopenfunc,
	quicktimeplayfunc,
	quicktimestopfunc,
	quicktimeisplayingfunc,
	
	ctquicktimeverbs
	} tyquicktimeverbtoken;


/*Functions*/


static boolean quicktimeisplayingverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	7.0b4 PBS: Is the QuickTime Player window currently playing a movie?
	*/
	
	if (!langcheckparamcount (hp1, 0))
		return (false);
			
	(*v).data.flvalue = playerisplaying ();
			
	return (true);
	} /*quicktimeisplayingverb*/


static boolean quicktimeopenverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	7.0b4 PBS: Open a movie in the QuickTime player window.
	*/
	
	tyfilespec fs;

	flnextparamislast = true;
	
	if (!getfilespecvalue (hp1, 1, &fs))
		return (false);

	if (!playeropenmovieinwindow (&fs)) {
		
		langerrormessage ("\x31" "Can't open the file because of a QuickTime error.");
		
		return (false);
		} /*if*/
	
	return (true);		
	} /*quicktimeopenverb*/


static boolean quicktimeplayverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	7.0b4 PBS: play the movie in the QuickTime Player window.
	*/
	
	if (!langcheckparamcount (hp1, 0))
		return (false);
			
	(*v).data.flvalue = playerplaymovie ();
			
	return (true);
	} /*quicktimeplayverb*/
	

static boolean quicktimestopverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	7.0b4 PBS: stop the movie in the QuickTime Player window.
	*/
	
	if (!langcheckparamcount (hp1, 0))
		return (false);
			
	(*v).data.flvalue = playerstopmovie ();
			
	return (true);
	} /*quicktimestopverb*/


static boolean quicktimefunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	7.0b4 PBS: switch statement for QuickTime verbs.
	*/
	
	hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /*by default, string functions return false*/
	
	switch (token) {
		
		case quicktimeopenfunc:
			return (quicktimeopenverb (hp1, v));
		
		case quicktimeplayfunc:
			return (quicktimeplayverb (hp1, v));
		
		case quicktimestopfunc:
			return (quicktimestopverb (hp1, v));
		
		case quicktimeisplayingfunc:
			return (quicktimeisplayingverb (hp1, v));
		
		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			return (false);
		} /*switch*/
	} /*quicktimefunctionvalue*/
	
	
boolean quicktimeinitverbs (void) {
	
	/*
	7.0b4 PBS: new QuickTime verbs.
	*/
		
	return (loadfunctionprocessor (idquicktimeverbs, &quicktimefunctionvalue));
	} /*quicktimeinitverbs*/


#endif
