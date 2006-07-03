
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

#include "landinternal.h"



pascal boolean landpushparam (hdlverbrecord hverb, typaramtype type, Handle hval, void *pval, long len, typaramkeyword key) {
	
	/*
	2.1b3 dmb: no more typaramrecords
	*/
	
	register hdlverbrecord hv = hverb;
	AppleEvent event, reply;
	AERecord *evt;
	
	landsystem7geteventrecords (hv, &event, &reply);
	
	if ((**hv).verbtoken == returntoken)
		evt = &reply;
	else
		evt = &event;
	
	if (!landsystem7pushparam (evt, type, hval, pval, len, key))
		return (false);
	
	(**hv).ctparams++; /*added room for another param*/
	
	return (true);
	} /*landpushparam*/

	
pascal boolean landpushintparam (hdlverbrecord hverb, short x, typaramkeyword key) {
	
	return (landpushparam (hverb, inttype, nil, &x, longsizeof (x), key));
	} /*landpushintparam*/
	
	
pascal boolean landpushlongparam (hdlverbrecord hverb, long x, typaramkeyword key) {
	
	return (landpushparam (hverb, longtype, nil, &x, longsizeof (x), key));
	} /*landpushlongparam*/


pascal boolean landpushstringparam (hdlverbrecord hverb, bigstring bs, typaramkeyword key) {
	
	return (landpushparam (hverb, texttype, nil, bs + 1, stringlength (bs), key));
	} /*landpushstringparam*/




