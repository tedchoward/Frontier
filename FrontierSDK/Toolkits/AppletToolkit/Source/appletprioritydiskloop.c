
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletfiles.h"
#include "appletfilesinternal.h"
#include "appletfolder.h"
#include "appletprioritydiskloop.h"


static tyfileloopcallback usercallback = nil;

static short userfavoritevnum;

static short passnum;



static boolean priorityvisit (bigstring fname, tyfileinfo *info, long refcon) {
	
	boolean flslow = (*info).flhardwarelock || (*info).flremotevolume;
	boolean flfave = (*info).vnum == userfavoritevnum;
	
	switch (passnum) {
		
		case 0:
			if (!flfave) /*only interested in the favorite disk on pass 0*/
				return (true);
				
			if (flslow) /*we're going to ignore the favorite, it's a slow disk*/
				return (true);
				
			break;
		
		case 1: 
			if (flfave) /*already visited in pass 0*/
				return (true);
				
			if (flslow) /*save for pass 2*/
				return (true);
				
			break;
			
		case 2: 
			if (flfave) /*already visited in pass 0*/
				return (true);
				
			if (!flslow) /*already visited in pass 1*/
				return (true);
				
			break;
		} /*switch*/
	
	return ((*usercallback) (fname, info, refcon));
	} /*priorityvisit*/
	

boolean prioritydiskloop (tyfileloopcallback callback, long refcon, short vnumfavorite) {
	
	/*
	same as diskloop, except we process the volumes 
	according to a sensible priority scheme.
	
	if vnumfavorite is non-zero, we look on that
	volume first.
	
	then we look on all fast volumes except the
	favorite one.
	
	finally we look at all slow volumes, ones that
	are remote, or hardware locked (probably CD's).
	
	note: these can't be nested because the callback 
	is stored in a static, it's not stacked.
	*/
	
	short firstpassnum;
	
	usercallback = callback;
	
	userfavoritevnum = vnumfavorite;
	
	if (vnumfavorite == 0)
		firstpassnum = 1;
	else
		firstpassnum = 0;
	
	for (passnum = firstpassnum; passnum <= 2; passnum++) {
	
		if (!diskloop (&priorityvisit, refcon))
			return (false);
		} /*for*/
	
	return (true);
	} /*prioritydiskloop*/
	

