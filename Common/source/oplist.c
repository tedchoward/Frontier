
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
#include "strings.h"
#include "resources.h"
#include "op.h"
#include "opinternal.h"
#include "oplist.h"



/*
created 3/20/90 DW:

you can build a very simple list manager on top of the outline processor
implemented in op.c.  this is such a thing.

the idea is that any list built on top of op, could someday easily be
displayed in a window, and be interacted with by the user.  both noble
goals.  so here goes!
*/


#define fldebugging false 

#define oplistversionnumber 1


#ifdef MACVERSION
	#define unconditionallongswap(x) ((((x) >> 24) & 0x000000ff) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))
	#define unconditionalshortswap(x) ((((x) >> 8) & 0x00ff) | (((x) << 8) & 0xff00))
	#define unconditionallyswaplong(x) do {(x) = unconditionallongswap(x);} while (0)
	#define unconditionallyswapshort(x) do {(x) = unconditionalshortswap(x);} while (0)
#endif

#ifdef WIN95VERSION
	#define unconditionallongswap(x) dolongswap(x)
	#define unconditionalshortswap(x) doshortswap(x)
	#define unconditionallyswaplong(x) longswap(x)
	#define unconditionallyswapshort(x) shortswap(x)
#endif


typedef struct tydisklistrecord {
	
	short recordsize; /*number of bytes in this header*/
	
	short versionnumber; /*this structure is saved on disk*/
	
	unsigned long ctoutlinebytes; /*size of the packed outline record*/
	
	short ctitems; /*save this instead of recomputing every time we load*/
	
	boolean flunused; /*just maintaining the byte alignment we want*/
	
	boolean isrecord; /*are list items named?*/
	
	// short ctitems_hiword; // 5.1.3 - support more than 32K items
	
	/*packed outline record is stored at end of this record*/
	} tydisklistrecord;


static hdllistrecord hcurrentlist = nil;


/*static boolean oplistreleaserefcon (hdlheadrecord hnode, boolean fl) {
	
	cb = (**hcurrentlist).releaseitemcallback;
	
	if (cb)
		return ((*cb) ((**hnode).hrefcon));
	} /*oplistreleaserefcon*/


boolean opnewlist (hdllistrecord *hlist, boolean isrecord) {
	
	/*
	7.31.97 dmb: expanded implementation to serve as UserTalk's list
	and record datatype.
	*/

	register hdllistrecord h;
	hdloutlinerecord houtline;
	
	if (!newclearhandle (sizeof (tylistrecord), (Handle *) hlist))
		return (false);
	
	h = *hlist; /*copy into register*/

	if (!newoutlinerecord (&houtline)) { 
		
		disposehandle ((Handle) h);
		
		return (false);
		}
		
	(**houtline).flbuildundo = false;
	
	(**houtline).flinhibitdisplay = true;

	(**h).houtline = houtline;
	
	(**h).ctitems = 0;
	
	(**h).isrecord = isrecord;
	
	/*
	(**houtline).releaserefconcallback = &oplistreleaserefcon;
	*/

	return (true);
	} /*opnewlist*/
	
	
void opdisposelist (hdllistrecord hlist) {
	
	register hdloutlinerecord ho;
	
	if (hlist == nil) /*defensive driving*/
		return;
		
	hcurrentlist = hlist; /*for our callback routines*/
	
	ho = (hdloutlinerecord) (**hlist).houtline;
	
	opdisposeoutline (ho, false);
	
	hcurrentlist = nil;
	
	disposehandle ((Handle) hlist);
	} /*opdisposelist*/
	
	
boolean oppushhandle (hdllistrecord hlist, ptrstring pname, Handle hdata) {
	
	/*
	add a new item at the end of the list.  link in the indicated handle in the
	refcon field of the allocated headrecord.  return false if there's an allocation
	error.

	8.11.97 dmb: dispose hdata on error.
	*/
	
	register hdllistrecord h = hlist;
	register hdlheadrecord hlast;
	register hdloutlinerecord ho;
	register long ctitems;
	hdlheadrecord hnew;
	bigstring bs;
	
	if ((**h).isrecord && (pname == nil))
		goto error;

	ho = (hdloutlinerecord) (**h).houtline;
	
	if (!oppushoutline (ho))
		goto error;
	
	ctitems = (**h).ctitems;
	
	copystring (pname, bs); /*checks for nil*/
	
	if (ctitems == 0) { /*adding to an empty list*/
		
		hnew = (**ho).hsummit; /*set this guy's refcon handle*/
		
		opsetheadstring (hnew, bs);
		}
		
	else {
		hlast = oprepeatedbump (down, ctitems - 1, (**ho).hsummit, false);
		
	//	setstringlength (bs, 0);

		if (!opaddheadline (hlast, down, bs, &hnew)) {

			oppopoutline ();

			goto error;
			}
		
		(**ho).ctexpanded++;
		
		(**hnew).flexpanded = true;
		}
	
	#ifdef fldebug
	
	if (fldebugging) {
		
		copystring ((ptrstring) "\x0A" "headline #", bs);
		
		pushint (ctitems + 1, bs);
		
		opsetheadstring (hnew, bs);
		}
	
	#endif
	
	(**hnew).hrefcon = hdata; /*link in the user's data structure*/
	
	(**h).ctitems++;

	oppopoutline ();

	return (true);

	error:
		
		disposehandle (hdata);

		return (false);
	} /*oppushhandle*/
	

boolean oppushdata (hdllistrecord hlist, ptrstring pname, ptrvoid pdata, long ctbytes) {
	
	/*
	normally we have a statically allocated thing to link into a new list item.
	
	if you already have a handle, use oppushhandle to add an item to the list.
	*/
	
	Handle h;
	
	if (!newfilledhandle (pdata, ctbytes, &h))
		return (false);
	
	return (oppushhandle (hlist, pname, h));
	} /*oppushdata*/


boolean oppushstring (hdllistrecord hlist, ptrstring pname, bigstring bs) {
	
	/*
	add a new item to a list that's just storing strings.
	*/

	return (oppushdata (hlist, pname, bs, (long) stringsize (bs)));
	} /*oppushstring*/
	

static boolean opgetlistnode (hdllistrecord hlist, long ix, ptrstring pname, hdlheadrecord *hnode) {
	
	/*
	should be an internal routine -- this guy returns the outline headrecord for
	the indicated index into the list.
	*/
	
	hdloutlinerecord ho;
	hdlheadrecord nomad;
	bigstring bshead;
	
	*hnode = nil;
	
	if (hlist == nil) /*defensive driving*/
		return (false);
	
	ho = (hdloutlinerecord) (**hlist).houtline;

	if (ix == -1 && pname != nil) { /*looking up by name*/
		
		if (!(**hlist).isrecord)
			return (false);
		
		for (nomad = (**ho).hsummit; ; ) {
			
			opgetheadstring (nomad, bshead);
			
			if (equalstrings (bshead, pname)) {
				
				*hnode = nomad;

				return (true);
				}

			if (!opchasedown (&nomad))
				return (false);
			}
		}
	else {
		if ((ix < 1) || (ix > (**hlist).ctitems))
			return (false);
		
		*hnode = oprepeatedbump (down, ix - 1, (**ho).hsummit, false);
		
		return (true);
		}
	} /*opgetlistnode*/


boolean opgetlisthandle (hdllistrecord hlist, long ix, ptrstring pname, Handle *hdata) {
	
	/*
	the user's data is stored in the refcon handle in each headrecord.
	
	we index through the list, to the ixth item, and return the refcon
	handle for that list item.
	
	ix is 1-based.  the first item is item #1 and so on.
	
	return false if there aren't ix items in the list.
	*/
	
	hdlheadrecord hnode;
	
	if (!opgetlistnode (hlist, ix, pname, &hnode))
		return (false);
	
	if (pname != nil)
		opgetheadstring (hnode, pname);
	
	*hdata = (**hnode).hrefcon;
	
	return (true);
	} /*opgetlisthandle*/
	
	
boolean opgetliststring (hdllistrecord hlist, long ix, ptrstring pname, bigstring bs) {
	
	Handle h;
	
	if (!opgetlisthandle (hlist, ix, pname, &h)) {
		
		setstringlength (bs, 0);
		
		return (false);
		}
		
	copyheapstring ((hdlstring) h, bs);
	
	return (true);
	} /*opgetliststring*/


boolean opsetlisthandle (hdllistrecord hlist, long ix, ptrstring pname, Handle hdata) {
	
	hdlheadrecord hnode;
	hdloutlinerecord ho = (hdloutlinerecord) (**hlist).houtline;
	boolean flpush;
	
	if (!opgetlistnode (hlist, ix, pname, &hnode)) {
		
		if (ix == -1 && pname != nil) //looking up by name*/
			flpush = (**hlist).isrecord;
		
		else
			flpush = (ix == (**hlist).ctitems + 1);
		
		if (flpush)
			return (oppushhandle (hlist, pname, hdata));
		
		disposehandle (hdata);

		return (false);
		}
	
	(*(**ho).releaserefconcallback) (hnode, true);
	
	disposehandle ((**hnode).hrefcon); /*get rid of the old handle*/
	
	(**hnode).hrefcon = hdata; /*link in the new one*/
	
	return (true);
	} /*opsetlistdata*/


boolean opsetlistdata (hdllistrecord hlist, long ix, ptrstring pname, ptrvoid pdata, long ctbytes) {
	
	Handle hdata;
	
	if (!newfilledhandle (pdata, ctbytes, &hdata))
		return (false);
	
	return (opsetlisthandle (hlist, ix, pname, hdata));
	} /*opsetlistdata*/


long opcountlistitems (hdllistrecord hlist) {
	
	return ((**hlist).ctitems);
	} /*opcountlistitems*/


static boolean opdeletelistnode (hdloutlinerecord ho, hdlheadrecord hdelete) {
	
	hdlheadrecord hother;
	
	hother = hdelete;

	if (!opchasedown (&hother) && !opchaseup (&hother)) // deleting only summit (flat outline)
		return (false);
	
	else {
		if ((**ho).hsummit == hdelete)
			(**ho).hsummit = hother;
		
		if ((**ho).hline1 == hdelete)
			(**ho).hline1 = hother;
		
		if ((**ho).hbarcursor == hdelete)
			(**ho).hbarcursor = hother;
		
		opunlink (hdelete);

		(**ho).ctexpanded--;
		
		if ((**hdelete).flmarked)
			(**ho).ctmarked --;
		}
	
	return (true);
	} /*opdeletelistnode*/


boolean opdeletelistitem (hdllistrecord hlist, long ix, ptrstring pname) {
	
	hdlheadrecord hnode;
	hdloutlinerecord ho = (hdloutlinerecord) (**hlist).houtline;
	
	if (!opgetlistnode (hlist, ix, pname, &hnode))
		return (false);
	
	oppushoutline (ho);
	
	if (opdeletelistnode (ho, hnode))
		opreleasenode (hnode, false);
	else
		opemptyrefcon (hnode);
	
	oppopoutline ();
	
	(**hlist).ctitems--;
	
	return (true);
	} /*opdeletelistitem*/


boolean oppacklist (hdllistrecord hlist, Handle *hpacked) {
	
	/*
	5.0b13 dmb: byteswap info.ctoutlinebytes
	*/
	
	tydisklistrecord info;
	Handle hpackedoutline = nil;
	Handle hpackedlist = nil;
	boolean fl;
	
	if (!oppushoutline ((hdloutlinerecord) (**hlist).houtline))
		return (false);
		
	hpackedoutline = nil; /*allocate a new handle for packing*/
	
	fl = oppack (&hpackedoutline);
	
	oppopoutline ();
	
	if (!fl)
		return (false);
		
	clearbytes (&info, sizeof (info));
	
	info.recordsize = conditionalshortswap ((short) sizeof (info));

	info.versionnumber = conditionalshortswap (oplistversionnumber);
	
	info.isrecord = (**hlist).isrecord;
	
	// memlongtodiskwords ((**hlist).ctitems, info.ctitems, info.ctitems_hiword);
	
	if ((**hlist).ctitems > 0x7fff) // can't store it
		info.ctitems = -1;
	else
		info.ctitems = (**hlist).ctitems;
	
	memtodiskshort (info.ctitems);

	info.ctoutlinebytes = conditionallongswap (gethandlesize (hpackedoutline));
	
	if (!newfilledhandle (&info, sizeof (info), &hpackedlist))
		goto error;
	
	if (!pushhandle (hpackedoutline, hpackedlist))
		goto error;
		
	*hpacked = hpackedlist; /*returned handle*/
	
	disposehandle (hpackedoutline);
	
	return (true);
		
	error:
	
	oppopoutline ();
	
	disposehandle (hpackedoutline);
	
	disposehandle (hpackedlist);
	
	return (false);
	} /*oppacklist*/
	
	
boolean opunpacklist (Handle hpacked, hdllistrecord *hnewlist) {
	
	/*
	5.0a17 dmb: preserve outlinedata.
	
	5.0b9 dmb: consume hpacked
	
	5.0b13 dmb: byteswap info.ctoutlinebytes
		
	2002-11-11 AR: Added assert to make sure the C compiler chose the
	proper byte alignment for the tydisklistrecord struct. If it did not,
	we would end up corrupting any database files we saved.
	*/
	
	register hdllistrecord hlist = nil;
	hdloutlinerecord ho;
	long ixload = 0;
	tydisklistrecord info;
	Handle hpackedoutline = nil;
	boolean fl;
	
	assert (sizeof(tydisklistrecord) == 12L);
	
	if (!opnewlist (hnewlist, false)) /*create an empty list*/
		goto error;
	
	hlist = *hnewlist; /*copy into register*/
	
	clearbytes (&info, sizeof (info));
	
	// load first field - the record size
	if (!loadfromhandle (hpacked, &ixload, sizeof (info.recordsize), &info.recordsize))
		goto error;
	
	disktomemshort (info.recordsize);
	
	// load remaining data in record
	if (!loadfromhandle (hpacked, &ixload, min (sizeof (info), info.recordsize) - sizeof (info.recordsize), &info.versionnumber))
		goto error;
	
	disktomemshort (info.versionnumber);
	
	disktomemshort (info.ctitems);
	
	// disktomemshort (info.ctitems_hiword);
	
	disktomemlong (info.ctoutlinebytes);
	
	if (info.versionnumber == 0x01000000) // handle pre-swapped objects (unswap)
		info.versionnumber = 0x01;
	
	if (info.versionnumber != oplistversionnumber) { /*we only read version 1 format lists*/
		
		shellinternalerror (idbadopversionnumber, STR_bad_list_version_number);
		
		goto error;
		}
	
	#if 1 // faster
		hpackedoutline = hpacked;
		
		hpacked = nil;
		
		if (!pullfromhandle (hpackedoutline, 0, info.recordsize, nil))
			goto error;
		
		if (gethandlesize (hpackedoutline) != (long) info.ctoutlinebytes) { //old, unbyteswapped?
			
			unconditionallyswapshort (info.ctitems);
			
			// unconditionallyswapshort (info.ctitems_hiword);
			
			unconditionallyswaplong (info.ctoutlinebytes);
			
			info.isrecord = info.flunused;
			
			assert (gethandlesize (hpackedoutline) == (long) info.ctoutlinebytes);
			}
	
	#else // old code
		if (!loadfromhandletohandle (hpacked, &ixload, info.ctoutlinebytes, true, &hpackedoutline))
			goto error;
		
		disposehandle (hpacked);
		
		hpacked = nil;
	#endif
	
	// (**hlist).ctitems = makelong (info.ctitems, info.ctitems_hiword);
	(**hlist).ctitems = info.ctitems;
	
	(**hlist).isrecord = info.isrecord;
	
	ho = (hdloutlinerecord) (**hlist).houtline;
	
	opdisposeoutline (ho, false); /*opunpack will create a new one*/
	
	(**hlist).houtline = nil; /*don't leave it dangling*/
	
	fl = opunpackoutline (hpackedoutline, &ho);
	
	disposehandle (hpackedoutline);
	
	hpackedoutline = nil; /*so it won't get disposed in case of an error*/
	
	if (!fl)
		goto error;
		
	(**hlist).houtline = ho;
		
	if (info.ctitems == -1) { // couldn't store it; > 32K
		
		(**hlist).ctitems = opcountatlevel ((**ho).hsummit);
		}
	
	return (true);
	
	error:
	
	disposehandle (hpacked);
	
	opdisposelist (hlist);
	
	disposehandle (hpackedoutline);
	
	return (false);
	} /*opunpacklist*/


boolean opcopylist (hdllistrecord hsource, hdllistrecord *hcopy) {
	
	/*
	5.0.2b12 dmb: new routine, so we don't have to pack/unpack
	*/
	
	hdloutlinerecord ho;
	
	if (!copyhandle ((Handle) hsource, (Handle *) hcopy))
		return (false);
	
	if (!opcopyoutlinerecord ((**hsource).houtline, &ho)) {
		
		disposehandle ((Handle) *hcopy);
		
		return (false);
		}
	
	(***hcopy).houtline = ho;
	
	return (true);
	} /*opcopylist*/


boolean oploadstringlist (short resnum, hdllistrecord *hnewlist) {
	
	register hdllistrecord hlist;
	register short ixlist;
	bigstring bs;
	
	if (!opnewlist (hnewlist, false)) /*create an empty list*/
		return (false);
	
	hlist = *hnewlist; /*copy into register*/
	
	ixlist = 1; /*STR# list indices begin at 1*/
	
	while (true) {
		
		if (!getstringlist (resnum, ixlist, bs)) 
			return (true);
		
		if (!oppushstring (hlist, nil, bs)) {
			
			opdisposelist (hlist);
			
			*hnewlist = nil;
			
			return (false);
			}
		
		ixlist++;
		} /*while*/
	} /*oploadstringlist*/



