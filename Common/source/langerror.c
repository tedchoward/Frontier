
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

#include "ops.h"
#include "resources.h"
#include "strings.h"
#include "langinternal.h"



boolean fllangerror = false;  /*if true, the langerror dialog has already appeared*/

unsigned short langerrordisable = 0; /*it's possible to temporarily disable lang errors*/




void disablelangerror (void) {
	
	++langerrordisable;
	} /*disablelangerror*/


void enablelangerror (void) {
	
	--langerrordisable;
	} /*enablelangerror*/


boolean langerrorenabled (void) {
	
	return (langerrordisable == 0);
	} /*langerrorenabled*/


/*
static boolean fllocalerrorhook = false;

langerrordispatch (bigstring bs) { /%finish this someday%/
	
	hdltreenode hcode;
	
	if (!fllocalerrorhook && langgetlocalhandlercode ((ptrstring) "\perror", &hcode)) {
		
		fllocalerrorhook = true;
		
		fllocalerrorhook = false;
		
		return;
		}
	
	langerrormessage (bs);
	} /%langerrordispatch%/
*/

void langerror (short stringnum) {
	
	bigstring bs;
	
	getstringlist (langerrorlist, stringnum, bs);
	
	langerrormessage (bs);
	} /*langerror*/


void lang3paramerror (short stringnum, const bigstring bs1, const bigstring bs2, const bigstring bs3) {
	
	bigstring bs;
	
	getstringlist (langerrorlist, stringnum, bs);
	
	parsedialogstring (bs, (ptrstring) bs1, (ptrstring) bs2, (ptrstring) bs3, nil, bs);
	
	langerrormessage (bs);
	} /*lang3paramerror*/


void langparamerror (short stringnum, const bigstring bsparam) {
	
	lang3paramerror (stringnum, bsparam, nil, nil);
	} /*langparamerror*/


void lang2paramerror (short stringnum, const bigstring bs1, const bigstring bs2) {
	
	lang3paramerror (stringnum, bs1, bs2, nil);
	} /*lang2paramerror*/


void langlongparamerror (short stringnum, long x) {
	
	byte bslong [64];
	
	numbertostring (x, bslong);
	
	lang3paramerror (stringnum, bslong, nil, nil);
	} /*langlongparamerror*/


void langostypeparamerror (short stringnum, OSType x) {
	
	byte bsid [6];
	
	ostypetostring (x, bsid);
	
	lang3paramerror (stringnum, bsid, nil, nil);
	} /*langostypeparamerror*/


void parseerror (bigstring bs) {

	bigstring bscopy; /*must work on a copy!*/
	
	copystring (bs, bscopy); 
	
	convertcstring (bscopy); /*convert a c string to pascal format*/
	
	langparamerror (parsererror, bscopy);
	} /*parseerror*/



