
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



typedef struct tyqueuerecord {
	
	Handle hdata;
	
	struct tyqueuerecord **hnext;
	} tyqueuerecord, *ptrqueuerecord, **hdlqueuerecord;


boolean landpushqueue (Handle h) {
	
	/*
	implement a first-in-first-out queue.  push a new element at the end
	of the list headed bu by (**landglobals).hqueue.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlqueuerecord x;
	register hdlqueuerecord hlast;
	register short queuedepth = 0;
	hdlqueuerecord hnew;
	tyqueuerecord q;
	
	q.hdata = h;
	
	q.hnext = nil;
	
	if (!landnewfilledhandle (&q, longsizeof (tyqueuerecord), (Handle *) &hnew))
		return (false);
	
	x = (hdlqueuerecord) (**hg).hqueue;
	
	if (x == nil) { /*inserting into empty queue*/
		
		(**hg).hqueue = (Handle) hnew;
		
		return (true);
		}
		
	while (x != nil) { /*find the last guy in the queue*/
		
		hlast = x;
		
		x = (**x).hnext;
		
		queuedepth++;
		} /*while*/
	
	(**hlast).hnext = hnew;
	
	if (queuedepth > (**hg).maxqueuedepth) /*for display in stats window*/
		(**hg).maxqueuedepth = queuedepth;
	
	return (true);
	} /*landpushqueue*/


boolean landpopqueue (Handle *h) {
	
	/*
	pop the first guy off the queue, returning the data saved in h.
	
	return false if the queue is empty.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlqueuerecord x = (hdlqueuerecord) (**hg).hqueue;
	
	if (x == nil) /*empty queue*/
		return (false);
	
	*h = (**x).hdata; /*return the first guy on the queue*/
	
	(**hg).hqueue = (Handle) (**x).hnext;
	
	landdisposehandle ((Handle) x);
	
	return (true);
	} /*landpopqueue*/


boolean landpopqueueitem (landqueuepopcallback cb, long refcon, Handle *h) {
	
	/*
	scan the queue for the item that satisfies the callback
	
	if found, pop it out and return true; else return false.  
	*/
	
	register hdlqueuerecord x;
	register hdlqueuerecord hnext;
	register hdlqueuerecord hprev = nil;
	register hdllandglobals hg = landgetglobals ();
	
	x = (hdlqueuerecord) (**hg).hqueue;
	
	while (x != nil) { /*haven't reached end of queue*/
		
		hnext = (**x).hnext;
		
		if ((*cb) ((**x).hdata, refcon)) { /*found it*/
			
			*h = (**x).hdata;
			
			if (hprev == nil) /*was first in queue*/
				(**hg).hqueue = (Handle) hnext;
			else
				(**hprev).hnext = hnext;
			
			landdisposehandle ((Handle) x);
			
			return (true);
			}
		
		hprev = x;
		
		x = hnext;
		}
	
	return (false); /*didn't find it*/
	} /*landpopqueueitem*/


boolean landemptyqueue (void) {
	
	/*
	empty out the queue, return true if it was non-empty.
	*/
	
	register boolean fl = false;
	Handle h;
	
	while (landpopqueue (&h))
		fl = true;
		
	return (fl);
	} /*landemptyqueue*/



