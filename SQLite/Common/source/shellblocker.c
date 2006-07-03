
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
#include "shellprivate.h"




#define ctblocks 5 /*we can remember blocked events up to 5 levels deep*/

static short topblocks = 0;

static short blockstack [ctblocks];

static short blockedevents = 0;



short shellblockedevents (void) {
	
	return (blockedevents);
	} /*shellblockedevents*/


boolean shellblocked (short mask) {
	
	return ((blockedevents & mask) != 0);
	} /*shellblocked*/


boolean shellpushblock (short mask, boolean flblock) {
	
	if (topblocks >= ctblocks)
		return (false);
	
	blockstack [topblocks++] = blockedevents;
	
	if (flblock)
		blockedevents |= mask;
	else
		blockedevents &= ~mask;
	
	return (true);
	} /*shellpushblock*/


boolean shellpopblock (void) {
	
	if (topblocks <= 0)
		return (false);
	
	blockedevents = blockstack [--topblocks];
	
	return (true);
	} /*shellpopblock*/
	


static short shelleventsdisable = 0;


boolean shelleventsblocked (void) {
	
	return (shelleventsdisable > 0);
	} /*shelleventsblocked*/
	
	
boolean shellblockevents (void) {
	
	++shelleventsdisable;
	
	return (true);
	} /*shellblockevents*/
	

boolean shellpopevents (void) {
	
	if (shelleventsdisable <= 0)
		return (false);
	
	--shelleventsdisable;
	
	return (true);
	} /*shellpopevents*/
	
	
