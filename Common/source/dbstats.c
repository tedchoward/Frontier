
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

#include "memory.h"
#include "cursor.h"
#include "font.h"
#include "kb.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "frontierwindows.h"
#include "shell.h"
#include "db.h"
#include "dbinternal.h"


 
#define idstatsstringlist 140


enum { /*statsmessage indexes*/
	
	idwindowtitle = 1,
	
	idnodesinreleaselist,
	idbytesinreleaselist,
	idnodesinavaillist,
	idbytesinavaillist,
	idlargestfreenode,
	idnumberinconsistent,
	idnodesfreesequentially,
	idbytesfreesequentially,
	idnodesusedsequentially,
	idbytesusedsequentially,
	idnodesusedlogically,
	idbytesusedlogically,
	idnumberbytesorphaned,
	
	idreadheaderfailed,
	idreadusedheaderfailed,
	idfreeblockinuse,
	idreadavailnodefailed,
	idgeteoffailed,
	idreadheaderfailed1,
	idreadtrailerfailed2,
	idheadertrailerdisagree,
	idavailshadowoutofsynch
	};


typedef struct tystatsrecord {
	
	boolean flstatscomputed;

	long ctreleasenodes, ctreleasebytes;
	
	long ctavail, cttotalbytes, ctlargestblock;
	
	long seqctfree, seqctinuse, seqsizefree, seqsizeinuse;
	
	long logctinuse, logsizeinuse;
	
	long ctinconsistencies;
	
	bigstring bsmessage;
	
	short vstats; /*vertical offset next line is drawn at*/

	short hstats; /*horizontal offset*/

	short statslineheight;
	} tystatsrecord, *ptrstatsrecord, **hdlstatsrecord;
	

static WindowPtr statswindow = nil;

static hdlwindowinfo statswindowinfo = nil;

static hdlstatsrecord statsdata = nil;




static void getstatsstring (short id, bigstring bs) {
	
	getstringlist (idstatsstringlist, id, bs);
	} /*getstatsstring*/


static void setstatsmessage (short id) {
	
	bigstring bs;
	
	getstatsstring (id, bs);
	
	copystring (bs, (**statsdata).bsmessage);
	} /*setstatsmessage*/


static void statsline (bigstring bs) {
	
	register hdlstatsrecord hs = statsdata;
	
	movepento ((**hs).hstats, (**hs).vstats);
	
	pendrawstring (bs);
	
	(**hs).vstats += (**hs).statslineheight;
	
	//Timothy Paustian 10/5/00
	//We need to flush the result to the stats window everytime to see it
	//If this is not called the window remains blank.
	#if TARGET_API_MAC_CARBON == 1
	QDFlushPortBuffer(GetWindowPort(statswindow), nil);
	#endif
	} /*statsline*/
	

static void statsdisplay (short idstat, long n) {
	
	bigstring bs;
	
	getstatsstring (idstat, bs);
	
	pushstring ((ptrstring) "\x03" " = ", bs);
	
	pushlong (n, bs);
	
	statsline (bs);
	} /*statsdisplay*/


static void statsupdate (void) {
	
	register hdlstatsrecord hs = statsdata;
	register hdlwindowinfo hw = statswindowinfo;
	Rect r;
	
	pushstyle ((**hw).defaultfont, (**hw).defaultsize, (**hw).defaultstyle);
	
	(**hs).statslineheight = globalfontinfo.ascent + globalfontinfo.descent + 3;
	
	r = (**hw).contentrect;
	
	if (!(**hs).flstatscomputed)
		eraserect (r);

	else {

		(**hs).vstats = r.top + (**hs).statslineheight;
		
		(**hs).hstats = r.left + 8;
		
		statsdisplay (idnodesinreleaselist, (**hs).ctreleasenodes);
		
		statsdisplay (idbytesinreleaselist, (**hs).ctreleasebytes);
		
		statsline (zerostring);
		
		if ((**hs).ctavail >= 0) {
			
			statsdisplay (idnodesinavaillist, (**hs).ctavail);
			
			statsdisplay (idbytesinavaillist, (**hs).cttotalbytes);
			
			statsdisplay (idlargestfreenode, (**hs).ctlargestblock);
			
			statsdisplay (idnumberinconsistent, (**hs).ctinconsistencies);
			
			statsline (zerostring);
			}
		
		if ((**hs).seqctinuse > 0) {
		
			statsdisplay (idnodesfreesequentially, (**hs).seqctfree);
			
			statsdisplay (idbytesfreesequentially, (**hs).seqsizefree);
			
			statsdisplay (idnodesusedsequentially, (**hs).seqctinuse);
			
			statsdisplay (idbytesusedsequentially, (**hs).seqsizeinuse);
			
			statsline (zerostring);
			}
		
		if ((**hs).logctinuse > 0) {
		
			statsdisplay (idnodesusedlogically, (**hs).logctinuse);
			
			statsdisplay (idbytesusedlogically, (**hs).logsizeinuse);
			
			statsdisplay (idnumberbytesorphaned, (**hs).seqsizeinuse - (**hs).logsizeinuse - (**hs).ctreleasebytes);
			}
		}
	
	popstyle ();
	} /*statsupdate*/
	
	
static boolean statscountreleasestack (long *ctreleasenodes, long *ctreleasebytes) {
	
	/*
	release all the chunks accumulated in the database's releasestack.
	*/
	
	Handle h = (**databasedata).releasestack;
	register long ct;
	register long ctreleased = 0;
	long ctbytes;
	boolean flfree;
	tyvariance variance;
	#ifdef DATABASE_DEBUG
		register tydbreleasestackframe *p;
	#else
		register ptrdbaddress p;
	#endif
	
	*ctreleasenodes = 0;
	
	*ctreleasebytes = 0;
	
	if (h == nil) /*nothing released*/
		return (true);
	
	#ifdef DATABASE_DEBUG
		ct = *ctreleasenodes = gethandlesize (h) / sizeof (tydbreleasestackframe);

		p = (tydbreleasestackframe*) *h;
		
		while (--ct >= 0) {
			
			if (!dbreadheader ((p++)->adr, &flfree, &ctbytes, &variance)) {
				
				setstatsmessage (idreadheaderfailed);
				
				return (false);
				}
			
			rollbeachball ();
			
			ctreleased += ctbytes;
			}
	#else
		ct = *ctreleasenodes = gethandlesize (h) / sizeof (dbaddress);

		p = (ptrdbaddress) *h;
		
		while (--ct >= 0) {
			
			if (!dbreadheader (*p++, &flfree, &ctbytes, &variance)) {
				
				setstatsmessage (idreadheaderfailed);
				
				return (false);
				}
			
			rollbeachball ();
			
			ctreleased += ctbytes;
			}
	#endif
	
	*ctreleasebytes = ctreleased;
	
	return (true);
	} /*statscountreleasestack*/


boolean statsblockinuse (dbaddress adr, bigstring bsitem) {
	
	/*
	called from the EFPs directly as we're doing our content-based 
	database scan.

	5.0a25 dmb: added bsitem parameter, keyboardescape check
	*/
	
	register hdlstatsrecord hs = statsdata;
	boolean flfree;
	long ctbytes;
	tyvariance variance;
	
	if (adr == nildbaddress) /*ok to pass nil*/
		return (true);
	
	if (keyboardescape ())
		return (false);
	
	/*
	if (bsitem != nil) {
		
		shellsetwindowmessage (statswindowinfo, bsitem);

		shelldrawwindowmessage (statswindowinfo);
		}
	*/
	
	if (!dbreadheader (adr, &flfree, &ctbytes, &variance)) {
		
		setstatsmessage (idreadusedheaderfailed);
		
		return (false);
		}
	
	if (flfree) {
		
		setstatsmessage (idfreeblockinuse);
		
		return (false);
		}
	
	(**hs).logctinuse++;
	
	(**hs).logsizeinuse += ctbytes;
	
	return (true);
	} /*statsblockinuse*/


static boolean statscompute (boolean flincludeusedblocks) {

	/*
	the first three statistics are numbers produced by traversing the 
	avail list.
	
	the other numbers are determined by doing a sequential scan of the
	database, record by record.
	
	ctinconsistencies is the result of comparing the trailer with the balancing
	header in each block in the database.  if they all agree with each other
	then ctinconsistencies is 0, otherwise the number of errors is reported.
	
	7/8/91 dmb: roll the beach ball
	*/
	
	register hdlstatsrecord hs = statsdata;
	long ctnodes;
	long totalbytes;
	long largestblock;
	dbaddress nomad;
	boolean flfree, fl2;
	long ctbytes, ct2;
	dbaddress nextnomad;
	long ctfree, ctinuse, sizefree, sizeinuse, eof;
	long ctincon;
	tyvariance variance;
	hdlavaillistshadow havailshadow = (hdlavaillistshadow) (**databasedata).u.extensions.availlistshadow.data;
	
	initbeachball (right);
	
	clearhandle ((Handle) hs);

	statsupdate ();
	
	if (!statscountreleasestack (&ctnodes, &totalbytes))
		return (false);
	
	if (keyboardescape ())
		return (false);
	
	(**hs).ctreleasenodes = ctnodes; /*copy returned values*/
	
	(**hs).ctreleasebytes = totalbytes; /*copy returned values*/
	
	(**hs).flstatscomputed = true; // something has been computed

	(**hs).ctavail = -1; // not yet calced
	
	statsupdate ();
	
	nomad = (**databasedata).availlist;
	
	ctnodes = 0;
	
	totalbytes = 0;
	
	largestblock = 0;
	
	while (nomad != nildbaddress) {
	
		if (!dbreadavailnode (nomad, &flfree, &ctbytes, &nextnomad)) {
			
			setstatsmessage (idreadavailnodefailed);
			
			return (false);
			}
		
		if ((*havailshadow) [ctnodes].adr != nomad)
			setstatsmessage (idavailshadowoutofsynch);
		
		if ((*havailshadow) [ctnodes].size != ctbytes)
			setstatsmessage (idavailshadowoutofsynch);
		
		ctnodes++;
		
		totalbytes += ctbytes;
		
		nomad = nextnomad;
		
		rollbeachball ();
		
		if (ctbytes > largestblock)
			largestblock = ctbytes;
		} /*while*/	
		
	(**hs).ctavail = ctnodes; /*copy returned values*/
	
	(**hs).cttotalbytes = totalbytes;
	
	(**hs).ctlargestblock = largestblock;
	
	statsupdate ();
	
	if (!dbgeteof (&eof)) {
	
		setstatsmessage (idgeteoffailed);
		
		return (false);
		}
	
	ctfree = 0; /*initialize loop variables*/
	
	ctinuse = 0;
	
	sizefree = 0;
	
	sizeinuse = 0;
	
	ctincon = 0;
	
	(**hs).seqctfree = 0; /*initialize returned values*/
	
	(**hs).seqctinuse = 0;
	
	(**hs).seqsizefree = 0;
	
	(**hs).seqsizeinuse = 0;
	
	(**hs).ctinconsistencies = 0;
	
	(**hs).logctinuse = 0;
	
	(**hs).logsizeinuse = 0;
	
	nomad = firstphysicaladdress; /*point at first physical record in the database*/
	
	initbeachball (left); /*reverse direction*/
	
	while (nomad < eof) { /*scan the database sequentially*/
		
		if (!dbreadheader (nomad, &flfree, &ctbytes, &variance)) {
	
			setstatsmessage (idreadheaderfailed1);
			
			return (false);
			}
			
		if (flfree) {
		
			ctfree++;
			
			sizefree += ctbytes;
			}
		else {
			ctinuse++;
			
			sizeinuse += ctbytes;
			}
			
		nomad += sizeheader + ctbytes; /*point at the trailer*/
		
		if (!dbreadtrailer (nomad, &fl2, &ct2)) {
		
			setstatsmessage (idreadtrailerfailed2);
						
			return (false);
			}
			
		if ((fl2 != flfree) || (ct2 != ctbytes)) {
			
			setstatsmessage (idheadertrailerdisagree);
							
			ctincon++;
			}
		
		rollbeachball ();
		
		nomad += sizetrailer; /*advance to next sequential record*/
		} /*while*/
	
	/*finished the sequential scan*/
	
	(**hs).seqctfree = ctfree; /*copy returned values*/
	
	(**hs).seqctinuse = ctinuse;
	
	(**hs).seqsizefree = sizefree;
	
	(**hs).seqsizeinuse = sizeinuse;
	
	(**hs).ctinconsistencies = ctincon;
	
	statsupdate ();
	
	/*calculate logical block usage*/

	if (!flincludeusedblocks)
		return (true);
	
	#ifdef SMART_DB_OPENING
		statsblockinuse ((**databasedata).u.extensions.availlistblock, nil);
	#endif
	
	shellpushrootglobals (statswindow);
	
	initbeachball (right); /*reverse direction again*/
	
	(*shellglobals.findusedblocksroutine) (); /*traverses all database structures*/
	
	shellpopglobals ();
	
	return (true);
	} /*statscompute*/


static boolean statsdispose (void) {
	
	disposehandle ((Handle) statsdata);
	
	return (true);
	} /*statsdispose*/


static boolean statsmousedown (Point pt, tyclickflags flags) {
	
	/*
	smashrect ((**statswindowinfo).contentrect);
	
	statscompute ();
	*/
	
	return (true);
	
	} /*statsmousedown*/


boolean statsstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks statscallbacks;
	register ptrcallbacks cb;
	
	shellnewcallbacks (&statscallbacks);
	
	cb = statscallbacks; /*copy into register*/
	
	loadconfigresource (idstatsconfig, &(*cb).config);
		
	(*cb).configresnum = idstatsconfig;
		
	(*cb).windowholder = &statswindow;
	
	(*cb).dataholder = (Handle *) &statsdata;
	
	(*cb).infoholder = &statswindowinfo; 
	
	(*cb).updateroutine = &statsupdate;
	
	(*cb).disposerecordroutine = &statsdispose;
	
	(*cb).mouseroutine = &statsmousedown;
	
	return (true);
	} /*statsstart*/


static boolean statsfindwindow (void) {
	
	register boolean fl;
	
	fl = shellfindwindow (
		
		idstatsconfig, &statswindow, &statswindowinfo, 
		
		(Handle *) &statsdata);
		
	return (fl);
	} /*statsfindwindow*/


static boolean statsnewwindow (void) {
	
	WindowPtr w;
	hdlwindowinfo hw;
	bigstring bstitle;
	Rect rzoom, rwindow;
	hdlwindowinfo hparent;
	Handle hdata;
	
	if (!shellgetfrontrootinfo (&hparent)) /*our parent is the frontmost root window*/
		return (false);
	
	shellgetwindowcenter (hparent, &rzoom);
	
	if (!newclearhandle (sizeof (tystatsrecord), &hdata))
		return (false);
	
	rwindow.top = -1; /*accept default*/
	
	getstatsstring (idwindowtitle, bstitle);
	
	if (!newchildwindow (idstatsconfig, hparent, &rwindow, &rzoom, bstitle, &w)) {
		
		disposehandle ((Handle) hdata);
		
		return (false);
		}
		
	getwindowinfo (w, &hw);
	
	(**hw).hdata = hdata;
	
	/*ccnewsubwindow (messagewindowinfo, ixmsginfo);*/
	
//	shellpushglobals (statswindow); 
	
	#if TARGET_API_MAC_CARBON == 1
	
		SetThemeWindowBackground (w, kThemeBrushModelessDialogBackgroundActive, false);
		
	#endif

	windowzoom (w);
	
//	shellpopglobals ();
	
	statswindow = w;

	return (true);
	} /*statsnewwindow*/
		 
	
boolean dbstatsmessage (hdldatabaserecord hdb, boolean flincludeusedblocks) {
	
	if (!statsfindwindow ()) {
		
		if (!statsnewwindow ()) /*sets dbstats.c globals if it worked*/
			return (false);
		}
	
	if (optionkeydown ())	
		windowbringtofront (statswindow);
	
	shellpushglobals (statswindow);
	
	smashrect ((**statswindowinfo).contentrect);
	
	if (hdb != nil)
		dbpushdatabase (hdb);

	statscompute (flincludeusedblocks);
	
	if (hdb != nil)
		dbpopdatabase ();

	shellpopglobals ();
	
	return (true);
	} /*dbstatsmessage*/
	
	
