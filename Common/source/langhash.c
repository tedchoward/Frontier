
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

#include "error.h"
#include "memory.h"
#include "strings.h"
#include "font.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langipc.h"
#include "langsystem7.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "oplist.h"
#include "timedate.h"
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */

	#include "aeutils.h" /*PBS 03/14/02: AE OS X fix.*/

#pragma pack(2)
typedef struct tydisksymbolrecord {
	
	long ixkey; /*in the string handle, where is this symbol's name?*/
	
	unsigned char valuetype; /*copied from the symbol's value record*/
	
	unsigned char version;
	
	tyvaluedata data; /*if a string, this stores an index into the string handle*/
	} tydisksymbolrecord, *ptrdisksymbolrecord, **hdldisksymbolrecord;

typedef struct tyOLD42disksymbolrecord {
	
	long ixkey; /*in the string handle, where is this symbol's name?*/
	
	unsigned char valuetype; /*copied from the symbol's value record*/
	
	unsigned char version : 4;
	
	unsigned char unused : 3;
	
	unsigned char flsorted : 1; /*were these records packed in sort order?*/
	
	tyvaluedata data; /*if a string, this stores an index into the string handle*/
	} tyOLD42disksymbolrecord, *ptrOLD42disksymbolrecord, **hdlOLD42disksymbolrecord;


// 5.0.1: bumped version number so we can clear uninitialized flags
#define tablediskversion 0x03


typedef struct tydisktablerecord { /*new in 5.0, a header for each table*/
	
	short version; /*in the string handle, where is this symbol's name?*/
	
	short sortorder;
	
	unsigned long timecreated, timelastsave; /*number of seconds since 1/1/04*/
	
	long flags;
	} tydisktablerecord, *ptrdisktablerecord, **hdldisktablerecord;


typedef enum tylinetableitemflags {

	flxml = 0x8000

	} tylinetableitemflags;

#define	maxinlinescalarsize	1023
#define	diskvalsizeflag		(-1)

typedef struct tydiskvaluerecord {		/*4.0.1b1 dmb*/
	
	long sizeflag;	/*always -1*/
	
	dbaddress adr;	/*addres of actual scalar value*/
	} tydiskvaluerecord;
#pragma options align=reset


hdlhashtable currenthashtable = nil;

hdltablestack hashtablestack = nil;

boolean fllanghashassignprotect = false;

boolean fllangexternalvalueprotect = false;	/*4.1b4 dmb: new global, disable protection*/


static boolean flunpackingtable = 0;

//static Handle h1; /*global for packing and unpacking routines -- holds the binary info*/

//static Handle h2; /*global for packing and unpacking routines -- holds the text info*/

static hdlhashnode hnewnode; /*global for hashinsertaddress*/

static boolean flexternalmemorypack = false;

static hdldatabaserecord hexternalpackdatabase;



static hdlhashtable hfirstfreetable = nil; /*private free list for hash tables*/


#ifdef fldebug

static long cthashtablesallocated = 0;

#endif




boolean newhashtable (hdlhashtable *htable) {
	
	/*
	all fields are initialized to 0, no initialization code needed here.
	
	9/23/91 dmb: now look for magic table.  this allows a table to be stuffed 
	full of values in a location that is removed from and/or independent of 
	our caller.  in particular, langfunccall can populate a locals table without 
	adding it to the local chain; evaluatelist picks up the table indirectly by 
	pushing a local chain, which eventually tries to allocate a hash table.
	
	5.0d15 dmb: preserve new cttmpstack field. We're assuming that the reused 
	table pool is mostly for local tables that actually need temp stacks.
	*/
	
	if (hmagictable != nil) {
		
		*htable = hmagictable;
		
		hmagictable = nil;
		
		return (true);
		}
	
	#ifdef fldebug
	
	++cthashtablesallocated;
	
	#endif
	
	if (hfirstfreetable != nil) { /*let's reuse one that's been allocated*/
		
		hdlhashtable ht = hfirstfreetable;
		short ct;
		
		hfirstfreetable = (**hfirstfreetable).prevhashtable;
		
		ct = (**ht).cttmpstack;
		
		clearhandle ((Handle) ht);
		
		(**ht).cttmpstack = ct;
		
		*htable = ht;
		
		return (true);
		}
	
	if (!newclearhandle (sizeof (tyhashtable), (Handle *) htable))
		return (false);
	
	(***htable).timecreated = timenow ();
	
	return (true);
	} /*newhashtable*/
	
	
short hashgetstackdepth (void) {
	
	/*
	if there are 12 tables in the current chain, return 12.
	*/
	
	register hdlhashtable x = currenthashtable;
	register short ct = 0;
	
	while (x != nil) {
		
		ct++;
		
		x = (**x).prevhashtable;
		} /*while*/
		
	return (ct);
	} /*hashgetstackdepth*/


void chainhashtable (hdlhashtable htable) {
	
	/*
	chain and unchain implement a stack of symbol tables.  the newest table is
	pointed to by currenthashtable, a global.  the global symbol table, the last in
	the list, points to nil.
	*/
	
	register hdlhashtable ht = htable;
	
	(**ht).prevhashtable = currenthashtable;
	
	(**ht).flchained = true;
	
	currenthashtable = ht;
	} /*chainhashtable*/


void unchainhashtable (void) {
	
	/*
	5.1.5b14 dmb: reset prevhashtable to nil
	*/
	
	register hdlhashtable ht = currenthashtable;
	register hdlhashtable hprev = (**ht).prevhashtable;
	
	(**ht).prevhashtable = nil;
	
	(**ht).flchained = false;
	
	currenthashtable = hprev;
	} /*unchainhashtable*/


	

hdlhashtable sethashtable (hdlhashtable hset) {
	
	/*
	5.0.2b6 dmb: utility routine for pushing using local storage
	*/
	
	hdlhashtable hprev = currenthashtable;
	
	currenthashtable = hset;
	
	return (hprev);
	} /*sethashtable*/


boolean pushhashtable (hdlhashtable h) {
	
	/*
	5.1.2 dmb: handle nil hs instead of asserting that it's not, If it's nil,
	the process globals have been disposed.
	*/
	
	register hdltablestack hs = hashtablestack;
	
	if (hs == nil)
		return (false);
	
	if (!langcheckstacklimit (idtablestack, (**hs).toptables, cthashtables)) /*overflow!*/
		return (false);
	
	(**hs).stack [(**hs).toptables++] = currenthashtable;
	
	currenthashtable = h;
	
	/*stacktracer (toptables);*/
	
	return (true);
	} /*pushhashtable*/


boolean pophashtable (void) {
	
	register hdltablestack hs = hashtablestack;
	
	if ((**hs).toptables <= 0) {
		
		shellinternalerror (idtoomanypophashtables, STR_too_many_pophashtables);
		
		return (false);
		}
	
	currenthashtable = (**hs).stack [--(**hs).toptables];
	
	/*stacktracer (toptables);*/
	
	return (true);
	} /*pophashtable*/


boolean pushouterlocaltable (void) {
	
	/*
	push the local table with the most global scope -- the only table that 
	is global to the current process, but unique to it.
	*/
	
	register hdlhashtable ht = currenthashtable;
	register hdlhashtable hprev;
	
	assert (ht != nil);
	
	assert ((**ht).fllocaltable); /*current hash table should be a local table*/
	
	while (true) {
		
		hprev = (**ht).prevhashtable;
		
		if ((hprev == nil) || !(**hprev).fllocaltable)
			return (pushhashtable (ht));
		
		ht = hprev;
		} /*while*/
	} /*pushouterlocaltable*/



#ifdef smartmemory


#include "tableverbs.h"

/******/

static boolean hashtablevisitall (hdlhashtable htable, boolean (*visit) (hdlhashnode)) {
	
	/*
	7/25/92 dmb: weird version of table visitation needed for purging tables:
	
	always visit all kids; return true if all kids return true, else false
	*/
	
	register hdlhashnode x;
	register short i;
	register boolean fl = true;
	
	for (i = 0; i < ctbuckets; i++) {
		
		x = (**htable).hashbucket [i];
		
		while (x != nil) {
			
			hdlhashnode nextx = (**x).hashlink;
			
			if (!(*visit) (x)) 
				fl = false;
			
			x = nextx;
			} /*while*/
		} /*for*/
	
	return (fl);
	} /*hashtablevisitall*/	


static boolean checkaddressvisit (hdlhashnode hnode) {
	
	/*
	7/25/92 dmb: weird version of table visitation needed for purging tables:
	
	always visit all kids; return true if all kids return true, else false
	*/
	
	register hdlexternalvariable hv;
	tyvaluerecord val;
	hdlhashtable htable;
	bigstring bs;
	hdltreenode hcode;
	langerrorcallback errorcallback;
	
	val = (**hnode).val;
	
	switch (val.valuetype) {
		
		case addressvaluetype:
			getaddressvalue (val, &htable, bs);
			
			(**htable).flnopurge = true;
			
			break;
		
		case externalvaluetype:
			hv = (hdlexternalvariable) val.data.externalvalue;
			
			if (!(**hv).flinmemory)
				break;
			
			if (langexternalvaltotable (val, &htable)) {
				
				hashtablevisitall (htable, checkaddressvisit);
				
				break;
				}
			
			if (langexternalvaltocode (val, &hcode)) {
				
				if (langfinderrorrefcon ((long) hnode, &errorcallback)) /***/
					(**htable).flnopurge = true;
				
				break;
				}
			
			break;
		}
	
	return (true);
	} /*checkaddressvisit*/	


static boolean purgetablevisit (hdlhashnode hnode) {
	
	/*
	7/25/92 dmb: weird version of table visitation needed for purging tables:
	
	always visit all kids; return true if all kids return true, else false
	*/
	
	register hdlexternalvariable hv;
	register hdlhashtable ht;
	tyvaluerecord val;
	hdlhashtable htable;
	boolean flnopurge;
	
	val = (**hnode).val;
	
	if (val.valuetype != externalvaluetype) 
		return (true);
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if (!(**hv).flinmemory)
		return (true);
	
	if (!langexternalvaltotable (val, &htable))
		return (true);
	
	ht = htable;
	
	flnopurge = (**ht).flnopurge;
	
	(**ht).flnopurge = false; /*must reset every time*/
	
	if ((**ht).fldirty)
		return (false);
	
	
	if ((**ht).flwindowopen)
		return (false);
	
	
	assert (!(**ht).fllocaltable);
	
	if ((**hnode).fldontsave)
		return (false);
	
	if (!hashtablevisitall (ht, purgetablevisit))
		return (false);
	
	if ((**ht).fllocked)
		return (false);
	
	if (flnopurge)
		return (false);
	
	tableverbunload (hv);
	
	return (true);
	} /*purgetablevisit*/	

#endif

boolean hashflushcache (long *ctbytesneeded) {
	
	register hdlhashtable hfreetable;
	
	#ifdef smartmemory
	
	hashtablevisitall (roottable, checkaddressvisit);
	
	hashtablevisitall (roottable, purgetablevisit);
	
	#endif
	
	while (hfirstfreetable != nil) {
		
		hfreetable = hfirstfreetable;
		
		hfirstfreetable = (**hfreetable).prevhashtable;
		
		*ctbytesneeded -= gethandlesize ((Handle) hfreetable);
		
		disposehandle ((Handle) hfreetable);
		}
	
	return (true);
	} /*hashflushcache*/


boolean disposehashnode (hdlhashtable ht, hdlhashnode hnode, boolean fldisposevalue, boolean fldisk) {
	
	/*
	5.1.4 dmb: take htable parameter for database setting for disk scalars
	
	2003-05-22 AR: Call hashunregisteraddressnode at the lowest level
	possible to ensure that we don't encounter invalid hdlhashnodes
	later when we dispose a hashtable.
	*/

	register hdlhashnode hn = hnode;
	
	/*
	if ((**hn).ctlocks > 0) {
		
		(**hn).fldisposewhenunlocked = true;
		
		return (false);
		}
	*/


	if (fldisposevalue) {
		
		boolean flneeddatabase = (fldisk && (**hn).val.fldiskval);
		hdldatabaserecord hdb = nil;

		if (flneeddatabase) {
			
			hdb = tablegetdatabase (ht);

			if (hdb)
				dbpushdatabase (hdb);
			}

		disposevaluerecord ((**hn).val, fldisk);
		
		if (flneeddatabase && hdb)
			dbpopdatabase ();
		}

	disposehandle ((Handle) hn);
	
	return (true);
	} /*disposehashnode*/


void dirtyhashtable (hdlhashtable ht) {
	
	(**ht).fldirty = true;
	
	(**ht).timelastsave = timenow ();
	} /*dirtyhashtable*/

	
static short smashhashtable (hdlhashtable htable, boolean fldisk, boolean flcallback) {
	
	/*
	4.0b7 4/25/96 dmb: pulled this code out of disposehashtable
	so we could make a verb out of it. had to add the flcallback parameter,
	since disposehashtable doesn't want to.
	*/
	
	register hdlhashtable ht = htable;
	register hdlhashnode nomad, nextnomad;
	register short i;
	short ctdisposed = 0;
	bigstring bs;
	
	if (ht == nil) /*easy to dispose of nil table*/
		return (0);
	
	(**ht).hfirstsort = nil;	/*disconnect now so table is valid during disposal*/
	
	for (i = 0; i < ctbuckets; i++) {
		
		nomad = (**ht).hashbucket [i];
		
		(**ht).hashbucket [i] = nil; /*disconnect list so table is valid during disposal*/
		
		while (nomad != nil) {
			
			nextnomad = (**nomad).hashlink;
			
			if (flcallback)
				gethashkey (nomad, bs);
			
			if (flcallback)
				langsymbolunlinking (ht, nomad);
			
			disposehashnode (ht, nomad, true, fldisk);
			
			if (flcallback)
				langsymboldeleted (ht, bs);
			
			++ctdisposed;
			
			nomad = nextnomad;
			} /*while*/
		} /*for*/
	
	dirtyhashtable (ht);

	return (ctdisposed); 
	} /*smashhashtable*/


short emptyhashtable (hdlhashtable htable, boolean fldisk) {

	return (smashhashtable (htable, fldisk, true));
	} /*emptyhashtable*/

boolean disposehashtable (hdlhashtable htable, boolean fldisk) {
	
	/*
	7/10/90 DW: if it's a local table, don't dispose of any code trees linked
	in as values.
	
	1/8/90 dmb: check new flchained flag to postpone disposal
	
	6/10/92 dmb: disconnect bucket list during disposal so table remains valid
	
	9/24/92 dmb: removed special case for code node value disposal.
	disposevaluerecord now knows that it should never dispose code values
	*/
	
	register hdlhashtable ht = htable;
	
	if (ht == nil) /*easy to dispose of nil table*/
		return (true);
	
	#ifdef fldebug
	
	--cthashtablesallocated;
	
	#endif
	
	if (ht == roottable) { /*very serious internal error*/
	
		shellinternalerror (iddisposingsystemtable, STR_trying_to_dispose_global_symbol_table);
		
		return (false);
		}
	
	if ((**ht).flchained) { /*table is in local chain; can't dispose now*/
		
		(**ht).fldisposewhenunchained = true; /*we'll do it later*/
		
		return (true);
		}
	
	pushhashtable (ht);
	
	cleartmpstack ();
	
	pophashtable ();
	
	smashhashtable (ht, fldisk, false);
	/*
	for (i = 0; i < ctbuckets; i++) {
		
		nomad = (**ht).hashbucket [i];
		
		(**ht).hashbucket [i] = nil; /%disconnect list so table is valid during disposal%/
		
		while (nomad != nil) {
			
			/%
			boolean fldisposevalue;
			
			fldisposevalue = (!(**ht).fllocaltable) || ((**nomad).val.valuetype != codevaluetype);
			%/
			
			nextnomad = (**nomad).hashlink;
			
			disposehashnode (nomad, true /%fldisposevalue%/, fldisk);
			
			nomad = nextnomad;
			}
		}
	*/
	
	
	(**ht).prevhashtable = hfirstfreetable;
	
	hfirstfreetable = ht;
	
	/*
	disposehandle ((Handle) ht);
	*/
	
	return (true); 
	} /*disposehashtable*/


short hashfunction (const bigstring bs) {
	
	/*
		3.0.4b8 dmb: need to make locals unsigned to protect against ctype's int's
	*/

//	register unsigned short c;
	register unsigned short len;
//	register ptrstring p = (ptrstring) bs;
	register unsigned short val;

	len = stringlength (bs);
	
	if (len == 0)
		return (0);
	
//	c = p [1];
	
	val = getlower(getstringcharacter(bs,0));
	
//	c = p [len];
	
	val += getlower(getstringcharacter(bs,len-1));
	
	return (val % ctbuckets);
	} /*hashfunction*/


static boolean hashsortedinsert (hdlhashnode hnode) {

	register hdlhashnode hn = hnode;
	register hdlhashtable ht = currenthashtable;
	register hdlhashnode nomad = (**ht).hfirstsort;
	register hdlhashnode nomadprev = nil;
	short comparison;
	
	if (nomad == nil) { /*first guy in sorted list*/
		
		(**ht).hfirstsort = hn;
		
		(**hn).sortedlink = nil;
		
		return (true);
		}
	
	while (true) {
		
		comparison = (*langcallbacks.comparenodescallback) (ht, hn, nomad);
		
		if (comparison < 0) {
			
			if (nomadprev == nil) { /*he's the new first element*/	
				
				(**hn).sortedlink = (**ht).hfirstsort;
				
				(**ht).hfirstsort = hn;
				
				return (true);
				}
				
			(**hn).sortedlink = nomad; /*insert in before nomad, middle of list*/
			
			(**nomadprev).sortedlink = hn;
			
			return (true);
			}
		
		nomadprev = nomad; /*advance to next node in list*/
		
		nomad = (**nomad).sortedlink; /*advance to next node in sorted list*/
		
		if (nomad == nil) { /*insert at end of list*/
			
			(**hn).sortedlink = nil;
			
			(**nomadprev).sortedlink = hn;
			
			return (true);
			}
		} /*while*/
	} /*hashsortedinsert*/
	

static void hashsorteddelete (hdlhashnode hnodedelete) {
	
	register hdlhashtable htable = currenthashtable;
	register hdlhashnode nomad = (**htable).hfirstsort;
	register hdlhashnode nomadprev = nil;
	register hdlhashnode hnode = hnodedelete;
	
	while (nomad != nil) {
		
		if (nomad == hnode) {
			
			if (nomadprev == nil) { /*unlinking first in list*/
			
				(**htable).hfirstsort = (**nomad).sortedlink;
				
				return;
				}
				
			(**nomadprev).sortedlink = (**nomad).sortedlink;
			
			return;
			}
		
		nomadprev = nomad;
		
		nomad = (**nomad).sortedlink;
		} /*while*/
	} /*hashsorteddelete*/
	
	
static boolean hashlinknode (hdlhashtable htable, hdlhashnode hnode) {
	
	register hdlhashnode hn = hnode;
	register hdlhashtable ht = htable;
	register short ixbucket;
	register hdlhashnode hnext;
	
	ixbucket = hashfunction ((**hn).hashkey);
	
	hnext = (**ht).hashbucket [ixbucket];
	
	(**ht).hashbucket [ixbucket] = hnode; /*link new guy at head of list*/
	
	(**hn).hashlink = hnext;
	
	return (true);
	} /*hashlinknode*/
	

boolean hashinsertnode (hdlhashnode hnode, hdlhashtable htable) {
	
	/*
	3/23/93 dmb: don't invoke callback when flunpackingtable flag is set
	*/
	
	register hdlhashnode hn = hnode;
	register hdlhashtable ht = htable;
	bigstring bs;
	
	hashlinknode (ht, hn);
	
	if (flunpackingtable) /*tableunpack will take care of sort links*/
		return (true);
	
	pushhashtable (ht);
	
	hashsortedinsert (hn);
	
	pophashtable ();
	
	dirtyhashtable (ht);
	
	gethashkey (hn, bs);
	
	langsymbolinserted (ht, bs, hn);
	
	return (true);
	} /*hashinsertnode*/
	

boolean hashunlinknode (hdlhashtable htable, hdlhashnode hnode) {
	
	/*
	a sure-fire hash-algorithm-independent way to unlink a node.
	*/
	
	register short i;
	register hdlhashnode nomad, prev;
	
	for (i = 0; i < ctbuckets; i++) {
		
		nomad = (**htable).hashbucket [i];
		
		prev = nil;
		
		while (nomad != nil) {
			
			if (nomad == hnode) /*found it*/
				goto afterloop;
				
			prev = nomad;
			
			nomad = (**nomad).hashlink;
			} /*while*/
		} /*for*/
	
	return (false); /*not found*/
	
	afterloop:
	
	if (prev == nil) 
		(**htable).hashbucket [i] = (**nomad).hashlink;
	else
		(**prev).hashlink = (**nomad).hashlink;
	
	return (true);
	} /*hashunlinknode*/


boolean hashsetnodekey (hdlhashtable htable, hdlhashnode hnode, const bigstring bs) {
	
	if (!sethandlesize ((Handle) hnode, sizeof (tyhashnode) + stringsize (bs)))
		return (false);
	
	hashunlinknode (htable, hnode);
	
	copystring (bs, (**hnode).hashkey);
	
	hashlinknode (htable, hnode);
	
	(**htable).flneedsort = true;
	
	langsymbolchanged (htable, bs, hnode, false); /*value didn't change*/
	
	return (true);
	} /*hashsetnodekey*/


static boolean newhashnode (hdlhashnode *hnode, const bigstring bskey) {
	
	if (!newclearhandle (sizeof (tyhashnode) + stringsize (bskey), (Handle *) hnode))
		return (false);
	
	copystring (bskey, (***hnode).hashkey);
	
	return (true);
	} /*newhashnode*/


boolean hashinsert (const bigstring bs, tyvaluerecord val) {
	
	/*
	5.0.2b10 dmb: make sure we don't put a value with the tmp flag set.
	*/
	
	register hdlhashtable ht = currenthashtable;
	register hdlhashnode h;
	hdlhashnode hnode;
	
	if (isemptystring (bs)) {
		bigstring bspath;
		
		langexternalgetfullpath (currenthashtable, (ptrstring) bs, bspath, nil);
		
		lang2paramerror (illegalnameerror, bspath, bs);
		
		return (false);
		}
	
	if (!newhashnode (&hnode, bs))
		return (false);
	
	h = hnode; /*copy into register*/
	
	hnewnode = h; /*copy into global for hashinsertaddress*/
	
	val.fltmpstack = false; // 5.0.2: caller is responsible for actually removing it
	
	(**h).val = val;
	
	hashinsertnode (h, ht);
	
	return (true);
	} /*hashinsert*/
	

/*
hashmerge (hdlhashtable hsource, hdlhashtable hdest) {
	
	/%
	merge hsource into hdest, leaving hsource empty.  since it consumes 
	no memory (we just unlink nodes and deposit them) it can't fail.
	%/
	
	register short i;
	
	for (i = 0; i < ctbuckets; i++) {
		
		register hdlhashnode x;
		
		x = (**hsource).hashbucket [i];
		
		while (x != nil) { /%chain through the hash list%/
			
			register hdlhashnode nextx; 
			
			nextx = (**x).hashlink;
			
			hashinsertnode (x, hdest);
			
			x = nextx;
			} /%while%/
			
		(**hsource).hashbucket [i] = nil; /%we leave the source table empty%/
		} /%for%/
	} /%hashmerge%/
*/

/** 2/7/91 dmb: new implementation of array references resolves them
	immediately, so we don't have to handlel them here

boolean hashlocatearray (short arrayindex, hdlhashnode *hnode, hdlhashnode *hprev) {
	
	tyvaluerecord val;
	bigstring bsvarname;
	
	if (!hashgetiteminfo (currenthashtable, arrayindex - 1, bsvarname, &val)) {
		
		langlongparamerror (tabletoosmallerror, (long) arrayindex);
		
		return (false);
		}
		
	return (hashlocate (bsvarname, hnode, hprev));
	} /%hashlocatearray%/

	
boolean hashstringtoarrayindex (bigstring bs, short *arrayindex) {

	bigstring bscopy;
		
	if (stringlength (bs) == 0) /%empty names not allowed, defensive driving%/
		return (false);
	
	if (bs [1] != '$') 
		return (false);
		
	copystring (bs, bscopy);
		
	deletefirstchar (bscopy);
		
	if (!stringtoshort (bscopy, arrayindex)) {
			
		langparamerror (badindexname, bs);
			
		return (false);
		}
	
	return (true);
	} /%hashstringtoarrayindex%/
*/

boolean hashlocate (const bigstring bs, hdlhashnode *hnode, hdlhashnode *hprev) {

	/*
	7/15/90 DW: add support for table array-style references.  if the string
	begins with a $, we return the node and prev for the nth guy in the sorted
	list of the table.
	*/
	
	register short ixbucket;
	register hdlhashnode nomad, nomadprev;
	
	/*
	short arrayindex;
	
	if (hashstringtoarrayindex (bs, &arrayindex)) {
	
		return (hashlocatearray (arrayindex, hnode, hprev));
		}
	*/
	
	ixbucket = hashfunction (bs);

	//assert (currenthashtable != nil);

	//assert (validhandle ((Handle) currenthashtable));
	
	nomad = (**currenthashtable).hashbucket [ixbucket];
	
	nomadprev = nil;
	
	while (nomad != nil) {
		
		if (equalidentifiers (bs, (**nomad).hashkey)) {
		
			*hnode = nomad;
			
			*hprev = nomadprev;
			
			return (true);
			}
		
		nomadprev = nomad;
		
		nomad = (**nomad).hashlink;
		} /*while*/
		
	return (false); /*loop terminated, not found*/
	} /*hashlocate*/


boolean hashunlink (const bigstring bs, hdlhashnode *hnode) {
	
	hdlhashnode hprev;
	register hdlhashnode hn;
	
	if (!hashlocate (bs, hnode, &hprev)) {
	
		langparamerror (cantdeleteerror, bs);
		
		return (false);
		}
	
	hn = *hnode; /*copy into register*/
	
	langsymbolunlinking (currenthashtable, hn);
	
	if (hprev == nil) 
		(**currenthashtable).hashbucket [hashfunction (bs)] = (**hn).hashlink;
	else 
		(**hprev).hashlink = (**hn).hashlink;
	
	hashsorteddelete (hn);
	
	dirtyhashtable (currenthashtable);
	
	langsymboldeleted (currenthashtable, bs);
	
	return (true);
	} /*hashunlink*/

/*
boolean hashdeletenode (hdlhashnode *hnode) {
	
	hdlhashnode hprev;
	register hdlhashnode hn;
	
	hashunlinknode (currenthashtable, hnode);
	
	hashsorteddelete (hnode);
	
	dirtyhashtable (currenthashtable);
	
	return (true);
	} /%hashdeletenode%/
*/


boolean hashdelete (const bigstring bs, boolean fldisposevalue, boolean fldisk) {
	
	hdlhashnode hnode, hprev;
	register hdlhashnode hn;
	
	if (!hashlocate (bs, &hnode, &hprev)) {
	
		langparamerror (cantdeleteerror, bs);
		
		return (false);
		}
	
	hn = hnode; /*copy into register*/
	
	langsymbolunlinking (currenthashtable, hn);
	
	if (hprev == nil) 
		(**currenthashtable).hashbucket [hashfunction (bs)] = (**hn).hashlink;
	else 
		(**hprev).hashlink = (**hn).hashlink;
	
	hashsorteddelete (hn);
	
	disposehashnode (currenthashtable, hn, fldisposevalue, fldisk);
	
	dirtyhashtable (currenthashtable);
	
	langsymboldeleted (currenthashtable, bs);
	
	return (true);
	} /*hashdelete*/


boolean hashtabledelete (hdlhashtable htable, bigstring bs) {
	
	boolean fl;

	if (!pushhashtable (htable))
		return (false);
	
	fl = hashdelete (bs, true, true);
	
	pophashtable ();

	return (fl);
	} /*hashtabledelete*/


boolean hashsymbolexists (const bigstring bs) {
	
	hdlhashnode hnode, hprev;
	
	return (hashlocate (bs, &hnode, &hprev));
	} /*hashsymbolexists*/


boolean hashtablesymbolexists (hdlhashtable htable, const bigstring bs) {
	
	boolean fl;
	
	pushhashtable (htable);
	
	fl = hashsymbolexists (bs);
	
	pophashtable ();
	
	return (fl);
	} /*hashtablesymbolexists*/


typedef struct localityinfo {
	
	boolean fllocal;

	hdldatabaserecord hdb;
	} tylocalityinfo, *ptrlocalityinfo;


static boolean hashsetlocalityvisit (hdlhashnode hnode, ptrvoid refcon) {
	
	hdlhashtable ht;
	hdlexternalvariable hv;
	tyvaluerecord val = (**hnode).val;
	ptrlocalityinfo info = (ptrlocalityinfo) refcon;
	
	if (val.valuetype == externalvaluetype) {
		
		hv = (hdlexternalvariable) val.data.externalvalue;
		
		if (currenthashtable != filewindowtable)
			langexternalsetdatabase (hv, (*info).hdb);
		
		if ((**hv).flinmemory && langexternalvaltotable (val, &ht, hnode)) {
			
			(**ht).fllocaltable = (*info).fllocal;
			
			hashtablevisit (ht, &hashsetlocalityvisit, info);
			}
		}
	
	return (true); /*always continue traversal*/
	} /*hashsetlocalityvisit*/


void hashsetlocality (tyvaluerecord *val, boolean fllocal) {
	
	/*
	5.0.2b10 dmb: new routine. when we assign a table value to a local
	table, it and all of its subtables must be local too. Or the converse.
	
	5.0.2b13 dmb: set the table's parent link. we now maintain it strictly.
	
	5.1.4 dmb: deal with database ownership for newly-created externals

	encode address value according to locality
	
	6.2b16 AR: No longer static so it can be called externally (langaddlocals, langevaluate.c)
	*/
	
	hdlhashtable ht;
	hdlexternalvariable hv;
	
	tylocalityinfo info;

	switch ((*val).valuetype) {
		
		case addressvaluetype:

			disablelangerror (); /*08/02/2000 AR*/
			
			setaddressencoding (val, !fllocal);

			enablelangerror ();

			break;
	
		case externalvaluetype:
			
			hv = (hdlexternalvariable) (*val).data.externalvalue;
			
			info.fllocal = fllocal;
			
			if (currenthashtable != filewindowtable) {
				
				info.hdb = tablegetdatabase (currenthashtable);
				
				langexternalsetdatabase (hv, info.hdb);
				}
			
			if (!(**hv).flinmemory || !langexternalvaltotable (*val, &ht, nil))
				break;
			
			(**ht).parenthashtable = currenthashtable;
			
			(**ht).fllocaltable = fllocal;
			
			hashtablevisit (ht, &hashsetlocalityvisit, &info);

			break;

		default:
			break;
		}
	} /*hashsetlocality*/


boolean hashassign (const bigstring bs, tyvaluerecord val) {
	
	/*
	9/23/91 dmb: no longer clear fllangerror, or look at it when 
	hashlocate returns false.  array references are implemented differently 
	now, and hashlocate never generates errors.  clearing fllangerror can 
	have the side effect of hiding an error condition unexpectedly.
	
	5.0b17 dmb: if we're assigning a tmp external, claim the data like 
	a normal tmp. don't copy the data, clean fltmpdata instead. really, our
	caller should be exempting from the tmp stack, but this close to shipping
	let's not assume more than we have to

	5.0.1b1 dmb: the b17 change broke stuff, because the object may be in 
	another table's temp stack. Our caller is responsible for exempting 
	anything assinged into a table. we just need to make sure that the 
	fltmpstack flag is clear for _any_ object we assign to a hashnode

	5.0.1b2 dmb: when disposing a value, set fldisk false for local table items
	
	5.0.2b13 dmb: set fltmpdata false & call hashsetlocality before hashinsert case
	*/
	
	hdlhashnode hnode, hprev;
	tyvaluerecord existingval;
	boolean fllocal = (**currenthashtable).fllocaltable;
	
	/*
	fllangerror = false;
	*/
	
	if (val.fltmpdata) { /*val doesn't own it's data*/
		
		if (val.fltmpstack)
			val.fltmpdata = false;
		else
			if (!copyvaluedata (&val))
				return (false);
		}
	
	val.fltmpstack = false; // 5.0.1: caller is responsible for actually removing it
	
	//if (val.valuetype == externalvaluetype) // 5.0.2: localness of tables must match parent
		hashsetlocality (&val, fllocal);
	
	if (!hashlocate (bs, &hnode, &hprev)) { /*the name doesn't exist or is invalid*/
		
		/*just an undefined variable*/
		
		return (hashinsert (bs, val));
		}
	
	existingval = (**hnode).val;
	
	if (fllanghashassignprotect) { /*protect externals from being smashed by assignment*/
		
		if ((existingval.valuetype == externalvaluetype) && (val.valuetype != externalvaluetype)) {
			bigstring bstype;
			
			langexternaltypestring ((hdlexternalhandle) existingval.data.externalvalue, bstype);
			
			lang2paramerror (badexternalassignmenterror, bstype, bs);
			
			return (false);
			}
		}
	
	/*carefully nuke existing value*/ {
		
		boolean flneeddatabase = (!fllocal && existingval.fldiskval);
		hdldatabaserecord hdb = nil;

		if (flneeddatabase) {
			
			hdb = tablegetdatabase (currenthashtable);

			if (hdb)
				dbpushdatabase (hdb);
			}

		disposevaluerecord (existingval, !fllocal);
		
		if (flneeddatabase && hdb)
			dbpopdatabase ();
		}
	
	(**hnode).val = val;
	
	langsymbolchanged (currenthashtable, bs, hnode, true); /*value changed*/
	
	return (true);
	} /*hashassign*/


boolean hashtableassign (hdlhashtable htable, const bigstring bs, tyvaluerecord val) {
	
	boolean fl;
	
	pushhashtable (htable);
	
	fl = hashassign (bs, val);
	
	pophashtable ();
	
	return (fl);
	} /*hashtableassign*/


boolean hashresolvevalue (hdlhashtable htable, hdlhashnode hnode) {
	
	/*
	3/19/92 dmb: try to resolve an address -- it hasn't been referenced since 
	it was unpacked.
	
	4.0.2b1 dmb: handle disk-based scalar values. load the value and release
	the dbaddress. added htable parameter so we can potentially dirty it 
	
	5.0a23 dmb: on address resolution failure, reset flunresolvedaddress to true
	
	5.0b7 dmb: don't set flunresolvedaddress to true on failure. It breaks 
	the table display. don't know why exactly.

	5.1.4 dmb: no longer resolve addresses automatically. It's now a valid state.
	Exception: the paths table needs high-performance address access

	5.1.4 dmb: dbpushreleasestack must be while database is pushed
	*/
	
	register hdlhashnode hn = hnode;
	boolean fl;
	
	if (htable == pathstable && (**hn).flunresolvedaddress) {

		(**hn).flunresolvedaddress = false; /*clear now to avoid potential recursion*/
		
		lockhandle ((Handle) hn); /*08/02/2000 AR: so it's safe to pass &(**hn).val to setaddressencoding*/
		
		disablelangerror ();
		
		fl = setaddressencoding (&(**hn).val, false);
		
		enablelangerror ();
	
		unlockhandle ((Handle) hn);
		
		if (!fl)
			return (false);
		}

	if ((**hn).val.fldiskval) {
		Handle hbinary;
		hdldatabaserecord hdb = tablegetdatabase (htable);
		
		if (hdb)
			dbpushdatabase (hdb);
		
		fl = dbrefhandle ((**hn).val.data.diskvalue, &hbinary);
		
		if (fl)
			dbpushreleasestack ((**hn).val.data.diskvalue, (long) langgettype ((**hn).val));
		
		if (hdb)
			dbpopdatabase ();
		
		if (!fl)
			return (false);
		
		(**htable).fldirty = true;  /*dmb 6/18/96: we released disk value, must force table to be resaved*/
		
		(**hn).val.data.binaryvalue = hbinary;
		
		(**hn).val.fldiskval = false;
		}
	
	return (true);
	} /*hashresolvevalue*/


boolean hashlookup (const bigstring bs, tyvaluerecord *vreturned, hdlhashnode *hnode) {
	
	/*
	3/19/92 dmb: must check for unresolved addresses here
	*/
	
	hdlhashnode hprev;
	
	if (!hashlocate (bs, hnode, &hprev)) 
		return (false);
	
	if (!hashresolvevalue (currenthashtable, *hnode))
		return (false);
	
	*vreturned = (***hnode).val;
	
	return (true);
	} /*hashlookup*/
	

boolean hashtablelookup (hdlhashtable htable, const bigstring bs, tyvaluerecord *vreturned, hdlhashnode *hnode) {
	
	boolean fl;

	if (htable == nil)	/*8.0b48 PBS: it *is* nil sometimes. Too many callers would have to check it,*/
		return (false);	/*so the check is done here.*/
	
	
	pushhashtable (htable);
	
	fl = hashlookup (bs, vreturned, hnode);
	
	pophashtable ();
	
	return (fl);
	} /*hashtablelookup*/


boolean hashlookupnode (const bigstring bs, hdlhashnode *hnode) {
	
	/*
	3/19/92 dmb: must check for unresolved addresses here
	*/
	
	hdlhashnode hprev;
	
	if (!hashlocate (bs, hnode, &hprev))
		return (false);
	
	return (hashresolvevalue (currenthashtable, *hnode));
	} /*hashlookupnode*/


boolean hashtablelookupnode (hdlhashtable htable, const bigstring bs, hdlhashnode *hnode) {
	
	boolean fl;
	
	pushhashtable (htable);
	
	fl = hashlookupnode (bs, hnode);
	
	pophashtable ();
	
	return (fl);
	} /*hashtablelookupnode*/


static boolean hashinsertaddress (bigstring bsname, bigstring bsval) {
	
	/*
	3/19/92 dmb: discovered critical bug: if we try to resolve address references 
	here, using langexpandtodotparams, an address that references the table 
	being unpacked will generate infinite recursion.  That answer is to leave 
	the address in its string format for now, and then resolve the address when 
	it's referenced through a hashlookup.  this necessitated adding a new flag 
	to the hashrecord, and introducing the hnewnode global so we know what node 
	was created by hashinsert.  also, to preserve the original path information 
	and coordinate with getaddressvalue, we adopted a new convention of using 
	a hashtable of -1 to indicate an unresvoled address value.
	*/
	
	tyvaluerecord val;
	
	if (!setaddressvalue ((hdlhashtable) -1, bsval, &val))
		return (false);
	
	if (!hashinsert (bsname, val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	(**hnewnode).flunresolvedaddress = true;
	
	return (true);
	} /*hashinsertaddress*/


/*
static boolean hashinsertaddress (bigstring bsname, bigstring bsval) {
	
	tyvaluerecord val;
	hdlhashtable htable;
	bigstring bs;
	boolean fl;
	
	pushhashtable (roottable);
	
	disablelangerror ();
	
	if (langexpandtodotparams (bsval, &htable, bs))
		fl = setaddressvalue (htable, bs, &val);
	else
		fl = setstringvalue (bsval, &val);
	
	enablelangerror ();
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	if (!hashinsert (bsname, val))
		return (false);
	
	pushhashtable (roottable);
	
	exemptfromtmpstack (val);
	
	pophashtable ();
	
	return (true);
	} /%hashinsertaddress%/
*/


boolean hashtablevisit (hdlhashtable htable, langtablevisitcallback visit, ptrvoid refcon) {
	
	/*
	###4.0.2b1 warning: scalar node values may now be on disk. callers that may 
	be examining strings values must handle this. (currently these are no such callers.)
	*/
	
	register hdlhashnode x;
	register short i;
	
	for (i = 0; i < ctbuckets; i++) {
		
		x = (**htable).hashbucket [i];
		
		while (x != nil) {
			
			hdlhashnode nextx = (**x).hashlink;
			
			if (!(*visit) (x, refcon)) 
				return (false);
				
			x = nextx;
			} /*while*/
		} /*for*/
	
	return (true);
	} /*hashtablevisit*/


static int hashcompare (const void *h1, const void *h2) {
	
	return ((*langcallbacks.comparenodescallback) (currenthashtable, *(hdlhashnode *)h1, *(hdlhashnode *)h2));
	} /*hashcompare*/


static boolean hashquicksort (hdlhashtable htable) {
	
	/*
	3/31/93 dmb: re-sort the indicated hashtable, using the standard c 
	library quicksort routine. this is really fast in general, but note 
	that it's worst-case performance is an already-sorted list.
	*/
	
	register hdlhashtable ht = htable;
	long ctitems;
	Handle hlist;
	register hdlhashnode h;
	
	hashcountitems (ht, &ctitems);
	
	if (ctitems == 0)
		return (true);
	
	if (!newhandle (ctitems * sizeof (hdlhashnode), &hlist))
		return (false);
	
	lockhandle (hlist);
	
	/*populate the array*/ {
		
		register hdlhashnode *p = (hdlhashnode *) *hlist;
		
		for (h = (**ht).hfirstsort; h != nil; h = (**h).sortedlink)
			*p++ = h;
		}
	
	/*sort it*/ {
		
		pushhashtable (ht);
		
		#ifdef THINK_C
		
			qsort (*hlist, ctitems, sizeof (hdlhashnode), (__cmp_func) &hashcompare);
		
		#else
		
			qsort (*hlist, ctitems, sizeof (hdlhashnode), &hashcompare);
		
		#endif
		
		pophashtable ();
		}
	
	/*link the list*/ {
		
		register hdlhashnode *p = (hdlhashnode *) *hlist;
		
		(**ht).hfirstsort = *p;
		
		while (--ctitems > 0) {
			
			(***p).sortedlink = *(p + 1);
			
			++p;
			}
		
		(***p).sortedlink = nil;
		}
	
	unlockhandle (hlist);

	disposehandle (hlist);
	
	(**ht).flneedsort = false;
	
	return (true);
	} /*hashquicksort*/


boolean hashresort (hdlhashtable htable, hdlhashnode hresort) {
	
	/*
	re-sort the indicated hashtable.  first empty out the sorted list, then 
	visit every node in the table re-inserting it into the sorted list.
	
	3/31/93 dmb: added hresort parameter. if it's not nil, then only that 
	node needs to be resorted. using the quicksort routine, it's especially 
	important not to resort a mostly-sorted list just to move one node.
	*/
	
	register hdlhashtable ht = htable;
	
	if (hresort == nil)
		return (hashquicksort (ht));
	
	pushhashtable (ht);
	
	hashsorteddelete (hresort);
	
	hashsortedinsert (hresort);
	
	pophashtable ();
	
	(**ht).flneedsort = false; // this may be a bug. we don't know if this was only unsorted node
	
	return (true);
	} /*hashresort*/


boolean hashinversesearch (hdlhashtable htable, langinversesearchcallback visit, ptrvoid refcon, bigstring bsname) {

	/*
	perform a relatively slow, content-based search.
	
	we call the visit routine for every node in the current hashtable.
	
	if he returns false, we keep going -- he hasn't found the thing he's looking
	for yet.  if true, we return with the value of the node we stopped on.
	
	we return false if the visit routine never returns true.
	
	###4.0.2b1 warning: scalar node values may now be on disk. callers that may 
	be examining strings values must handle this. (currently these are no such callers.)
	*/
	
	register hdlhashnode nomad;
	register short i;
	
	for (i = 0; i < ctbuckets; i++) {
		
		nomad = (**htable).hashbucket [i];
		
		while (nomad != nil) {
		
			gethashkey (nomad, bsname);
				
			if ((*visit) (bsname, nomad, (**nomad).val, refcon)) /*search is over*/
				return (true);
				
			nomad = (**nomad).hashlink;
			} /*while*/
		} /*for*/
	
	setemptystring (bsname);
	
	return (false); /*never found the node he wanted*/
	} /*hashinversesearch*/


boolean hashsortedinversesearch (hdlhashtable htable, langsortedinversesearchcallback visit, ptrvoid refcon) {

	/*
	like hashinversesearch, except that items are visited in the current 
	sort order
	
	we return false if the visit routine never returns true.
	
	###4.0.2b1 warning: scalar node values may now be on disk. the value 
	that we pass to the visit routine may be unresolved. callers that may 
	be examining strings must handle this. (currently these are langipcgetparamvisit,
	tablefind, and tableverbpacktotext

	*/
	
	register hdlhashnode nomad = (**htable).hfirstsort;
	bigstring bsname;
	
	while (nomad != nil) {
		
		gethashkey (nomad, bsname);
		
		if ((*visit) (bsname, nomad, (**nomad).val, refcon)) /*search is over*/
			return (true);
		
		nomad = (**nomad).sortedlink;
		} /*while*/
	
	return (false); /*never found the node he wanted*/
	} /*hashsortedinversesearch*/


static boolean nodeintablevisit (hdlhashnode hnode, ptrvoid refcon) {
	
	return (hnode != (hdlhashnode) refcon); /*false terminates traversal*/
	} /*nodeintablevisit*/


boolean hashnodeintable (hdlhashnode hnode, hdlhashtable htable) {
	
	/*
	search the indicated table for the node, return true if we find it.
	*/
	
	if (htable == nil) // watch your back!
		return (false);
	
	return (!hashtablevisit (htable, &nodeintablevisit, hnode));
	} /*hashnodeintable*/


static boolean hashpackstring (handlestream *s, bigstring bs, long *ix) {
	
	*ix = (*s).pos;

	return (writehandlestream (s, (ptrvoid) bs, (long) stringsize (bs)));
	} /*hashpackstring*/


static void hashunpackstring (Handle hget, bigstring bs, long ix) {
	
	/*
	5.0.1 rab: p must point to unsigned
	*/

	register ptrbyte p;
	
	p = (ptrbyte)(*hget + ix);
	
	moveleft (p, bs, (unsigned long) *p + 1);
	} /*hashunpackstring*/


static boolean hashpackdata (handlestream *s, void *pdata, long ctbytes, long *ix) {
	
	unsigned long x = ctbytes;

	*ix = (*s).pos; /*where the text item is stored*/
	
	memtodisklong (x);

	if (!writehandlestream (s, &x, sizeof (x)))
		return (false);
	
	return (writehandlestream (s, pdata, ctbytes)); /*following the 4-byte length is the data*/
	} /*hashpackdata*/
			

static boolean hashpackbinary (handlestream *s, Handle hbinary, long *ix) {
	
	unsigned long ctbytes;
	
	*ix = (*s).pos; /*where the text item is stored*/
	
	ctbytes = gethandlesize (hbinary); /*first 4 bytes holds the unsigned length*/

	memtodisklong (ctbytes);

	if (!writehandlestream (s, &ctbytes, sizeof (ctbytes)))
		return (false);
	
	return (writehandlestreamhandle (s, hbinary)); /*following the 4-byte length is the packed text*/
	} /*hashpackbinary*/


static boolean hashunpackbinary (Handle hget, Handle *hbinary, long ix) {
	
	long ctbytes;
	
	if (!loadlongfromdiskhandle (hget, &ix, &ctbytes))
		return (false);

	return (loadfromhandletohandle (hget, &ix, ctbytes, false, hbinary));
	} /*hashunpackbinary*/


static boolean hashpackscalar (handlestream *s, hdlhashnode hnode, long *ix) {
	
	/*
	4.0.2b1 dmb: new code for disk-based scalars. for "binary" values above 
	a certain size, we allocated a separate db block and pack the address into 
	the table data structure instead of the data itself. see the comment in 
	hashunpackscalar for a discussion of how this is handled at the other 
	end (reading).
	
	5.1.5b9 dmb: use new hexternalpackdatabase for diskvals
	*/
	
	tydiskvaluerecord diskvalue;
	Handle hbinary = (**hnode).val.data.binaryvalue;
	unsigned long ctbytes;
	boolean fl;
	
	*ix = (*s).pos; /*where the item is stored*/
	
	if ((**hnode).val.fldiskval) {	/*already a disk-based scalar. can be tricky*/
		
		if (flexternalmemorypack) {
			
			hdldatabaserecord hdb = hexternalpackdatabase;
			
			if (hdb)
				dbpushdatabase (hdb);
			
			fl = dbrefhandle ((dbaddress) hbinary, &hbinary);
			
			if (hdb)
				dbpopdatabase ();
			
			if (!fl)
				return (false);
			
			fl = hashpackbinary (s, hbinary, ix);
			
			disposehandle (hbinary); /*we just loaded it to pack; don't change in-memory value*/
			
			return (fl);
			}
		
		diskvalue.sizeflag = conditionallongswap(diskvalsizeflag);
		
		diskvalue.adr = (dbaddress) hbinary;
		
		if (fldatabasesaveas) {
		
			if (!dbcopy (diskvalue.adr, &diskvalue.adr))
				return (false);
			}
		
		memtodisklong (diskvalue.adr);

		return (writehandlestream (s, &diskvalue, sizeof (diskvalue)));
		}
	
	ctbytes = gethandlesize (hbinary); /*first 4 bytes holds the unsigned length*/
	
	if ((ctbytes > maxinlinescalarsize) && (!flexternalmemorypack)) {	
		
		diskvalue.sizeflag = conditionallongswap(diskvalsizeflag);
		
		diskvalue.adr = nildbaddress;
		
		if (!dbassignhandle (hbinary, &diskvalue.adr))
			return (false);
		
		if (!fldatabasesaveas) { /*must keep all data structures in sync*/
		
			disposevaluerecord ((**hnode).val, true);
			
			(**hnode).val.fldiskval = true;
			
			(**hnode).val.data.diskvalue = diskvalue.adr;
			}
		
		memtodisklong (diskvalue.adr);

		return (writehandlestream (s, &diskvalue, sizeof (diskvalue)));
		}
	else {

		memtodisklong (ctbytes);

		if (!writehandlestream (s, &ctbytes, sizeof (ctbytes)))
			return (false);
		
		return (writehandlestreamhandle (s, hbinary)); /*following the 4-byte length is the packed text*/
		}
	} /*hashpackscalar*/


static boolean hashunpackscalar (Handle hget, tyvaluerecord *val, long ix) {
	
	/*
	4.0.2b1 dmb: like langunpackbinary, but this version handles scalar values  
	that have had their own db block allocated instead of being packed inline 
	in the table data. if the binary block length is diskvalsizeflag (-1), the  
	next 4 bytes are the db address of the actual data.
	
	for such "disk-based" values, the valuerecord has the true tyvaluetype, but the 
	fl.diskval flag is set, and the valuedata is a dbaddress. once someone actually 
	refers to the data, it is loaded into memory*. to ensure full compatibility with 
	all language functions, we must make the valuerecord look totally normal at that 
	point, so there's no way that an indirect data structure can be used to preserve
	the original dbaddress. so we must release the dbaddress and reallocate the value
	when we save.
	
	if we added more indirection to the data structure -- more like tyexternalvalues -- 
	it would no longer be so easy to confine these change to this file. there would be 
	additional overhead in accessing large binary values. and for values that change
	after being accessed, nothing would be gained; in both cases the original dbaddress
	would be released and a new one allocated.
	
	it should be noted that if we did create an addition data structure for 
	indirection, values referenced on a read-only basis would no longer have to be 
	rewritten to disk each save. to do this, copyvaluerecord would have to convert 
	diskvalues to normal scalars. other language functions that directly modify the value  
	of a hashnode would require additional work, like s[3] = 'x' where s is a string. 
	it's unclear what other ramifications would surface
	
	*see hashresolvevalue
	
	*/
	
	long ctbytes;
	
	if (!loadlongfromdiskhandle (hget, &ix, &ctbytes))
		return (false);
	
	if (ctbytes == diskvalsizeflag) {
		
		(*val).fldiskval = true;
		
		return (loadlongfromdiskhandle (hget, &ix, (dbaddress *) &(*val).data.binaryvalue));
		}
	else {
		return (loadfromhandletohandle (hget, &ix, ctbytes, false, &(*val).data.binaryvalue));
		}
	} /*hashunpackscalar*/


static boolean hashpackexternal (handlestream *s, hdlexternalvariable h, long *ix, boolean *flnewdbaddress) {
	
	Handle hpacked;
	unsigned long ctbytes;
	boolean fl;
	
	*ix = (*s).pos; /*where the text item is stored*/
	
	if (flexternalmemorypack)
		fl = langexternalmemorypack (h, &hpacked, HNoNode);
	else
		fl = langexternalpack (h, &hpacked, flnewdbaddress);
	
	if (!fl)
		return (false);
	
	ctbytes = gethandlesize (hpacked); /*first 4 bytes holds the unsigned length*/
	
	memtodisklong (ctbytes);

	if (!writehandlestream (s, &ctbytes, sizeof (ctbytes)))
		goto error;
	
	if (!writehandlestreamhandle (s, hpacked)) /*following the 4-byte length is the packed text*/
		goto error;
	
	disposehandle (hpacked);
	
	return (true);
	
	error:
	
	disposehandle (hpacked);
	
	return (false);
	} /*hashpackexternal*/


static boolean hashunpackexternal (Handle hget, boolean flmemory, hdlexternalhandle *h, long ix) {
	
	long ctbytes;
	Handle hpacked;
	boolean fl;
	
	if (!loadlongfromdiskhandle (hget, &ix, &ctbytes))
		return (false);
	
	if (!loadfromhandletohandle (hget, &ix, ctbytes, true, &hpacked)) 
		return (false);
	
	if (flmemory)
		fl = langexternalmemoryunpack (hpacked, h);
	else
		fl = langexternalunpack (hpacked, h);
	
	disposehandle (hpacked);
	
	return (fl);
	} /*hashunpackexternal*/


static void hashreporterror (short iderror, bigstring bsname, bigstring bserror) {

	/*
	5.1.4 dmb: embellish the bserror, folding it and bsname into the message iderror
	
	the smart part: for recursion, see if bserror already includes iderror. In that
	case, fold bsname into the path that's already in the message
	*/
		
	bigstring bs;
	
	fllangerror = false; // make sure our error won't be ignored
	
	getstringlist (langerrorlist, iderror, bs);
	
	nthword (bs, 1, '^', bs);
	
	if (patternmatch (bs, bserror) == 1) { // it's already been parsed in
		
		pushchar ('.', bsname);
		
		midinsertstring (bsname, bserror, stringlength (bs) + 1);
		
		langerrormessage (bserror);
		}
	else
		lang2paramerror (iderror, bsname, bserror);
	} /*hashbuilderrormessage*/

#pragma pack(2)
typedef struct typackinforecord {

	handlestream s1;
	handlestream s2;
	boolean flmustsave;
	} typackinforecord;
#pragma options align=reset


static boolean hashpackvisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
	
	/*
	return true to terminate the search, false to continue.
	
	4/8/93 dmb: added support for code values
	
	2.1b2 dmb: filespecs must be saved as aliases since volume reference 
	numbers aren't persistent across boots
	
	3.0.2 dmb: power pc code for doubles
	
	3.0.2 dmb: fixed memory leak when packing filespecs and code
	
	3.0.4 dmb: must set rec.data.longvalue for PPC doublevaluetype! fixes db corruption.

	5.1b21 dmb: don't let unresolvable address kill the save
	
	5.1.3 dmb: smarter error reporting
	
	6.2a15 AR: added flmustsave parameter
	*/
	
	typackinforecord *lpi = (typackinforecord *) refcon;
	tydisksymbolrecord rec;
	bigstring bsvalue;
	long size;
	Handle hpacked;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	bigstring bspackerror;
	boolean fl;
	
	assert (sizeof (tydisksymbolrecord) == 10L);
	
	/*
	if (stringlength (bsname) == 0)
		Debugger ();
	
	ccmsg (bsname, false);
	*/
	
	if ((**hnode).fldontsave && !flexternalmemorypack) /*keep traversing the table*/
		return (false);
	
	langtraperrors (bspackerror, &savecallback, &saverefcon);
	
	clearbytes (&rec, sizeof (rec));
	
	if (!hashpackstring (&lpi->s2, bsname, &rec.ixkey))
		goto error;
	
	memtodisklong (rec.ixkey);
	
//	rec.valuetype = conditionalenumswap(val.valuetype);
	rec.valuetype = val.valuetype;
	
	rec.data = val.data;
	
	size = sizeof (long); // default size of value data
	
	switch (val.valuetype) {
		
		case oldstringvaluetype:
			copyheapstring ((hdlstring) val.data.stringvalue, bsvalue);
			
			if (!hashpackstring (&lpi->s2, bsvalue, &rec.data.longvalue))
				goto error;
			
			break;
		
		case addressvaluetype:
			disablelangerror ();

			fl = getaddresspath (val, bsvalue);
			
			enablelangerror ();
			
			if (!fl) // on error, bsvalue should at least be item's name; don't break the save
				;
			
			if (!hashpackstring (&lpi->s2, bsvalue, &rec.data.longvalue))
				goto error;
			
			break;
		
	#ifdef oldMACVERSION	
		case filespecvaluetype: { /*need to save as a (minimal) alias*/
			
			register hdlfilespec x = val.data.filespecvalue;
			tyfilespec fs = **x;
			AliasHandle halias = nil;
			
			disablelangerror ();
			
			if (filespectoalias (&fs, true, &halias)) {
				
				rec.version = 1; /*all versions were zero until now*/
				
				x = (hdlfilespec) halias;
				}
				
			enablelangerror ();
			
			if (!hashpackbinary (&lpi->s2, (Handle) x, &rec.data.longvalue))
				goto error;
			
			disposehandle ((Handle) halias); /*3.0.2*/
			
			break;
			}
	#endif
	
		case rectvaluetype: {
			diskrect rdisk;
			
			recttodiskrect (*val.data.rectvalue, &rdisk);
			
			if (!hashpackdata (&lpi->s2, &rdisk, sizeof (rdisk), &rec.data.longvalue))
				goto error;
			
			break;
			}
		
		case rgbvaluetype: {
			diskrgb rgbdisk;
			
			rgbtodiskrgb (*val.data.rgbvalue, &rgbdisk);
			
			if (!hashpackdata (&lpi->s2, &rgbdisk, sizeof (rgbdisk), &rec.data.longvalue))
				goto error;
			
			break;
			}
		
		#if noextended
		
			case doublevaluetype: {
				double x = **val.data.doublevalue;
				extended80 x80;
				
					dtox80 (&x, &x80);
				
				if (!hashpackdata (&lpi->s2, &x80, sizeof (x80), &rec.data.longvalue))
					goto error;
				
				break;
				}
		#else
		
			case doublevaluetype:
			
		#endif
		
	#ifdef oldWIN95VERSION	
		case filespecvaluetype:
		case aliasvaluetype:
	#endif
		case stringvaluetype:
		case passwordvaluetype:
		case patternvaluetype:
		case objspecvaluetype:
		case binaryvaluetype:
			if (!hashpackscalar (&lpi->s2, hnode, &rec.data.longvalue))
				goto error;
				
			break;
		
		case listvaluetype:
		case recordvaluetype:
			if (!oppacklist (val.data.listvalue, &hpacked))
				goto error;
			
			if (!hashpackbinary (&lpi->s2, hpacked, &rec.data.longvalue))
				goto error;
			
			rec.version = 2;
			
			disposehandle (hpacked);

			break;

		case filespecvaluetype:
		case aliasvaluetype:
			if (!langpackfileval (&val, &hpacked))
				goto error;
			
			if (!hashpackbinary (&lpi->s2, hpacked, &rec.data.longvalue))
				goto error;
			
			rec.version = 2;
			
			disposehandle (hpacked);
			
			break;

		case codevaluetype:
			if (!langpacktree (val.data.codevalue, &hpacked))
				goto error;
			
			if (!hashpackbinary (&lpi->s2, hpacked, &rec.data.longvalue))
				goto error;
			
			disposehandle (hpacked); /*3.0.2*/
			
			break;
		
		case externalvaluetype: {
			boolean flnewdbaddress = false;
			
			if (!hashpackexternal (&lpi->s2, (hdlexternalvariable) val.data.externalvalue, &rec.data.longvalue, &flnewdbaddress))
				goto error;
			
			lpi->flmustsave = lpi->flmustsave || flnewdbaddress;
				
			break;
			}
			
		case novaluetype:
		case booleanvaluetype:
		case charvaluetype:
			size = sizeof (char);
			
			break;
		
		case intvaluetype:
		case tokenvaluetype:
			size = sizeof (short);
			
			break;
		
		case pointvaluetype:
			memtodiskshort (val.data.pointvalue.h);
			memtodiskshort (val.data.pointvalue.v);
			
			size = 0;
			
			break;
			
		case longvaluetype:
		case ostypevaluetype:
		case enumvaluetype:
		case fixedvaluetype:
		case singlevaluetype:
			size = sizeof (long);
			
			break;
		
		case directionvaluetype:
			size = sizeof (rec.data.dirvalue); // not necessarily a tydirection
			
			break;
		
		case datevaluetype:	// *** needs xplat format
			size = sizeof (long);
			
			break;
		
		default:
			langerror (cantpackerror);
			
			goto error;
		} /*switch*/
	
	if (size == sizeof (long))
		memtodisklong (rec.data.longvalue);
	
	if (size == sizeof (short))
		memtodiskshort (rec.data.intvalue);
	
	if (!writehandlestream (&lpi->s1, &rec, sizeof (rec))) 
		goto error;
	
	languntraperrors (savecallback, saverefcon, false);
	
	return (false); /*keep going, kind of backwardsÉ*/
	
	error:
	
	disposehandlestream (&lpi->s1); 
	
	disposehandlestream (&lpi->s2); 
	
	languntraperrors (savecallback, saverefcon, true);
	
	hashreporterror (hashpackerror, bsname, bspackerror);
	
	return (true); /*stop now, this is the error return*/
	} /*hashpackvisit*/


boolean hashpacktable (hdlhashtable htable, boolean flmemory, Handle *hpackedtable, boolean *flmustsave) {
	
	/*
	traverse the current symbol table, creating two packages of information
	that can be unpacked back into an in-memory hash table.
	
	the first, hrecords, is an array of disksymbolrecords.  each record can
	have one or two indexes to strings in the hstrings package.
	
	then merge the two handles returning one packet for the caller to save.
	
	10/6/91 dmb: mergehandles now consumes both source handles
	
	2/2/93 dmb: check result of pushpackstack
	
	3/30/93 dmb: 
	*/
	
	register boolean fl = false;
	tydisktablerecord header;
	typackinforecord packrec;
	Handle h1, h2;
	
	clearbytes (&header, sizeof (header));
	
	header.version = conditionalshortswap(tablediskversion);
	
	header.timecreated = conditionallongswap((**htable).timecreated);
	
	header.timelastsave = conditionallongswap((**htable).timelastsave);
	
	header.sortorder = conditionalshortswap((**htable).sortorder);
	
	#ifdef xmlfeatures
		if ((**htable).flxml)
			header.flags |= flxml;
	#endif
	
	clearbytes (&packrec, sizeof (packrec));
	
	packrec.flmustsave = *flmustsave;

	openhandlestream (nil, &packrec.s1);

	openhandlestream (nil, &packrec.s2);

	if (!writehandlestream (&packrec.s1, &header, sizeof (header)))
		goto exit;
	
	flexternalmemorypack = flmemory;
	
	if (flexternalmemorypack)
		hexternalpackdatabase = tablegetdatabase (htable);
	
	hashsortedinversesearch (htable, &hashpackvisit, &packrec);
	
	if (packrec.s1.data == nil) /*an error while packing*/
		goto exit;

	h1 = closehandlestream (&packrec.s1);

	h2 = closehandlestream (&packrec.s2);
	
	fl = mergehandles (h1, h2, hpackedtable);
	
	*flmustsave = packrec.flmustsave;

	exit:

	return (fl);
	} /*hashpacktable*/


boolean hashunpacktable (Handle hpackedtable, boolean flmemory, hdlhashtable htable) {
	
	/*
	unpack a hashtable packed by hashpacktable.  first explode the packed handle
	into two handles.
	
	return true if everything worked.
	
	we dispose of the packed table as soon as we're finished with it and both
	of the exploded handles.
	
	9/30/91 dmb: changed all hashassign calls to hashinsert; we're starting with 
	a fresh table, so duplicate names should only exist if they were saved that 
	way, in which case we want to preserve them.
	
	3/2/92 dmb: added backward compat code for change in double type (1.0 used 
	"universal" extended doubles; now we use SANE extended
	
	8/14/92 dmb: added special case for nil objspecs
	
	4/8/93 dmb: added support for code values
	
	2.1b2 dmb: filespecs are now saved as aliases
	
	2.1b9 dmb: if filespec can't be resolved from stored alias, change 
	valuetype to alias. also, this operation no longer attempts to mount 
	volumes
	
	5.0d1 dmb: tables now have a header when packed
	
	5.0.2b6 dmb: use new sethashtable to remove stack depth limit
	
	5.1.4 dmb: use new hashreporterror

	2002-11-11 AR: Added assert to make sure the C compiler chose the
	proper byte alignment for the tydisktablerecord struct. If it did not,
	we would end up corrupting any database files we saved.
	
	2006-04-20 sethdill & aradke: convert rgb values to native byte order
	*/
	
	boolean fl;
	Handle hrecords, hstrings;
	bigstring bsname, bsvalue;
	hdlhashnode hlastnode = nil;
	boolean flsorted = true; // 6.10.97 dmb: no longer do any auto-sorting here
	tydisktablerecord header;
	long ix = 0;
	long ixstrings;
	long size;
	Handle hpacked;
	boolean fldirty;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	bigstring bsunpackerror;
	hdlhashtable prevhashtable;
	
	assert (sizeof(tydisktablerecord) == 16L);
	
	if (!unmergehandles (hpackedtable, &hrecords, &hstrings)) /*consumes hpackedtable*/
		return (false);
	
	fldirty = (**htable).fldirty; //start with current state
	
	/*see if this is a 5.0 table, with a header*/
	
	loadfromhandle (hrecords, &ix, sizeof (tydisktablerecord), &header);
	
	header.version = conditionalshortswap(header.version);
	
	if (header.version > 0) { // a header has been written
		
		(**htable).sortorder = conditionalshortswap(header.sortorder);
		
		(**htable).timecreated = conditionallongswap(header.timecreated);
		
		(**htable).timelastsave = conditionallongswap(header.timelastsave);
		
		if (header.version == 2) //5.0.1: forgot to initialize flags
			header.flags = 0;
		
		flsorted = true;
		}
	else {
		header.version = 0;
		
		header.flags = 0;
		
		(**htable).timecreated = (**htable).timelastsave = timenow (); //5.0.1
		
		ix = 0;
		}
	
	fl = false; /*default return value*/
	
	#ifdef xmlfeatures
		(**htable).flxml = (header.flags & flxml) != 0;
	#endif

	prevhashtable = sethashtable (htable); // pushhashtable (htable);
	
	++flunpackingtable;
	
	langtraperrors (bsunpackerror, &savecallback, &saverefcon); // hook errors so we can embellish
	
	while (true) {
		
		tydisksymbolrecord rec;
		tyvaluerecord val;
		
		assert (sizeof (tydisksymbolrecord) == sizeof (tyOLD42disksymbolrecord));
		
			
			if (!loadfromhandle (hrecords, &ix, sizeof (rec), &rec)) /*out of records*/
				break;
		
		disktomemlong (rec.ixkey);
//		disktomemshort (rec.valuetype);
//		disktomemshort (rec.version);
		
		if (header.version < 2) // shift down from old bitfield position
			rec.version >>= 4;

		ixstrings = conditionallongswap  (rec.data.longvalue);
		
		hashunpackstring (hstrings, bsname, rec.ixkey);
		
		if (isemptystring (bsname)) /*skip junk*/
			continue;
		
		initvalue (&val, (tyvaluetype) rec.valuetype);
		
		size = 0; // by default, we take no data directly from rec
		
		switch (val.valuetype) {
			
			case oldstringvaluetype:
				hashunpackstring (hstrings, bsvalue, ixstrings);
				
				if (!newheapvalue (bsvalue + 1, (long) stringlength (bsvalue), stringvaluetype, &val))
					goto L1;
				
				exemptfromtmpstack (&val);
				
				break;
			
			case addressvaluetype:
				hashunpackstring (hstrings, bsvalue, ixstrings);
				
				if (!hashinsertaddress (bsname, bsvalue))
					goto L1;
				
				setemptystring (bsname); /*exception -- we've already inserted it*/
				
				break;
			
			case olddoublevaluetype: {
				Handle hextended;
				
				if (!hashunpackbinary (hstrings, &hextended, ixstrings))
					goto L1;
				
				pullfromhandle (hextended, 2, 2, nil); /*universal -> SANE*/
				
				val.valuetype = doublevaluetype;
				
				val.data.binaryvalue = hextended;
				
				break;
				}
			
		#ifdef oldMACVERSION
			case filespecvaluetype: { /*need to save as a (minimal) alias*/
				tyfilespec fs;
				boolean flresolved;
				Handle hbinary;
				
				if (!hashunpackbinary (hstrings, &hbinary, ixstrings))
					goto L1;
				
				if (rec.version > 0) { /*filespec is stored as an alias*/
					
					disablelangerror ();
					
					flresolved = aliastofilespec ((AliasHandle) hbinary, &fs);
					
					enablelangerror ();
					
					if (flresolved) {
						
						if (!sethandlecontents (&fs, filespecsize (fs), hbinary))
							goto L1;
						}
					else
						val.valuetype = aliasvaluetype;
					}
				
				val.data.binaryvalue = hbinary;
				
				break;
				}
			#endif
			
			case rectvaluetype: {
				diskrect **rdisk;
				Rect r;
				 
				if (!hashunpackbinary (hstrings, (Handle *) &rdisk, ixstrings))
					goto L1;
				
				diskrecttorect (*rdisk, &r);
				
				disposehandle ((Handle) rdisk);
				
				if (!newheapvalue (&r, sizeof (r), rectvaluetype, &val))
					goto L1;
				
				exemptfromtmpstack (&val);
				
				break;
				}
			
			case rgbvaluetype: { /* 2006-04-20 sethdill & aradke */
				diskrgb **rgbdisk;
				RGBColor rgb;
				 
				if (!hashunpackbinary (hstrings, (Handle *) &rgbdisk, ixstrings))
					goto L1;
				
				diskrgbtorgb (*rgbdisk, &rgb);
				
				disposehandle ((Handle) rgbdisk);
				
				if (!newheapvalue (&rgb, sizeof (rgb), rgbvaluetype, &val))
					goto L1;
				
				exemptfromtmpstack (&val);
				
				break;
				}
				
		#if noextended
		
			case doublevaluetype: {
				double x;
				extended80 **x80;
				 
				if (!hashunpackbinary (hstrings, (Handle *) &x80, ixstrings))
					goto L1;
				
					x = x80tod (*x80);
				 
				disposehandle ((Handle) x80);	// 1/22/97 dmb: this was a leak!
				
				if (!setdoublevalue (x, &val))
					goto L1;
				
				exemptfromtmpstack (&val);
			
				break;
				}
		#else
		
			case doublevaluetype:
			
		#endif
		
		#if oldWIN95VERSION	
			case filespecvaluetype: // unpack normally
			case aliasvaluetype:	// *** needs xplat format
		#endif
			case stringvaluetype:
			case passwordvaluetype:
			case patternvaluetype:
			case binaryvaluetype: {
				if (!hashunpackscalar (hstrings, &val, ixstrings))
					goto L1;
				
				break;
				}
			
			case listvaluetype:
			case recordvaluetype:
				if (rec.version < 2) {
						AEDesc aelist;
						
						if (!hashunpackscalar (hstrings, &val, ixstrings))
							goto L1;
						
						if (val.fldiskval) { //yikes! we have to resolve before converting
							Handle hbinary;
							
							if (!dbrefhandle (val.data.diskvalue, &hbinary))
								goto L1;
							
							dbpushreleasestack (val.data.diskvalue, val.valuetype);
							
							fldirty = true; //table needs to be rewritten
							
							val.data.binaryvalue = hbinary;
							
							val.fldiskval = false;
							}
						
							
							{
							DescType typecode = typeAEList;
							
							if (val.valuetype == recordvaluetype)
								typecode = typeAERecord;
						
							newdescwithhandle (&aelist, typecode, val.data.binaryvalue);
							}
						
						
						if (!langipcconvertaelist (&aelist, &val))
							goto L1;
						
						AEDisposeDesc (&aelist);
						
						exemptfromtmpstack (&val);
					
					
					break;
					}
				
				if (!hashunpackbinary (hstrings, &hpacked, ixstrings))
					goto L1;
				
				if (!opunpacklist (hpacked, &val.data.listvalue))
					goto L1;
				
				break;

			case filespecvaluetype:
			case aliasvaluetype:

				if (rec.version < 2) {
						tyfilespec fs;
						boolean flresolved;
						Handle hbinary;
						
						if (!hashunpackbinary (hstrings, &hbinary, ixstrings))
							goto L1;
						
						if (rec.version > 0) { /*filespec is stored as an alias*/
							
							disablelangerror ();
							
							flresolved = aliastofilespec ((AliasHandle) hbinary, &fs);
							
							enablelangerror ();
							
							if (flresolved) {
								
								if (!sethandlecontents (&fs, filespecsize (fs), hbinary))
									goto L1;
								}
							else
								val.valuetype = aliasvaluetype;
							}
						
						val.data.binaryvalue = hbinary;
						
						break;
					
					}
				
				if (!hashunpackbinary (hstrings, &hpacked, ixstrings))
					goto L1;
				
				if (!langunpackfileval (hpacked, &val))
					goto L1;
				
				break;

			case objspecvaluetype: {
				Handle hobjspec;
				
				if (!hashunpackbinary (hstrings, &hobjspec, ixstrings))
					goto L1;
				
				if (gethandlesize (hobjspec) == 0) {
					
					disposehandle (hobjspec);
					
					hobjspec = nil;
					}
				
				val.data.objspecvalue = hobjspec;
				
				break;
				}
			
			case codevaluetype:
				if (!hashunpackbinary (hstrings, &hpacked, ixstrings))
					goto L1;
				
				if (!langunpacktree (hpacked, &val.data.codevalue))
					goto L1;
				
				break;
			
			case externalvaluetype: {
				hdlexternalhandle h;
				
				if (!hashunpackexternal (hstrings, flmemory, &h, ixstrings))
					goto L1;
				
				val.data.externalvalue = (Handle) h;
				
				break;
				}
				
			case directionvaluetype:

			case booleanvaluetype:
				if (header.version < 2)
					rec.data.chvalue = (byte) rec.data.intvalue;
				// fall through
			case novaluetype:
			case charvaluetype:
				val.data.chvalue = rec.data.chvalue;
				
				break;
			
			case intvaluetype:
			case tokenvaluetype:
				val.data.intvalue = rec.data.intvalue;
				
				disktomemshort (val.data.intvalue); // 5.0b9 dmb: was flipping rec.data
				break;
			
			case pointvaluetype:
				val.data.pointvalue = rec.data.pointvalue;
				
				disktomemshort (val.data.pointvalue.h);
				disktomemshort (val.data.pointvalue.v);
				
				break;
				
			case longvaluetype:
			case ostypevaluetype:
			case enumvaluetype:
			case fixedvaluetype:
			case singlevaluetype:
			case datevaluetype:
				val.data.longvalue = rec.data.longvalue;
				
				disktomemlong (val.data.longvalue);
				
				break;
			
			default:
				val.data = rec.data;
				
				break;
			} /*switch*/
		
		if (!isemptystring (bsname)) { /*needs to be inserted*/
			
			if (!hashinsert (bsname, val))
				goto L1;
			}
		
		if (hlastnode == nil)
			(**htable).hfirstsort = hnewnode;
		else
			(**hlastnode).sortedlink = hnewnode;
		
		hlastnode = hnewnode;
		} /*for*/
		
	fl = true; /*loop terminated, we will return true*/
	
	L1:
	
	languntraperrors (savecallback, saverefcon, !fl);
	
	sethashtable (prevhashtable);
	
	--flunpackingtable;
	
	disposehandle (hrecords);
	
	disposehandle (hstrings);
	
	(**htable).fldirty = fldirty;
	
	if (!flsorted)
		hashresort (htable, nil);
	
	if (!fl)
		hashreporterror (hashunpackerror, bsname, bsunpackerror);
	
	return (fl);
	} /*hashunpacktable*/


boolean hashcountitems (hdlhashtable htable, long *ctitems) {
	
	/*
	return the number of items in the indicated hash table.
	*/
	
	register hdlhashnode nomad = (**htable).hfirstsort;
	register long ct = 0;
	
	while (nomad != nil) {
		
		ct++;
		
		nomad = (**nomad).sortedlink;
		} /*while*/
		
	*ctitems = ct;
	
	return (true);
	} /*hashcountitems*/


boolean hashsortedsearch (hdlhashtable htable, const bigstring bsname, long *item) {
	
	/*
	search the sorted linked list attached to the indicated table, and return the
	index of the node having the indicated name.
	
	return false if there is no node with the name.
	
	2/6/91 dmb: handle array references
	*/
	
	register hdlhashnode nomad = (**htable).hfirstsort;
	register long ct = 0;
	
	/*
	short arrayindex;
	
	if (hashstringtoarrayindex (bsname, &arrayindex)) {
		
		*item = arrayindex - 1;
		
		return (true);
		}
	*/
	
	while (nomad != nil) {
		
		if (equalidentifiers ((**nomad).hashkey, bsname)) {
		
			*item = ct;
			
			return (true);
			}
		
		ct++;
		
		nomad = (**nomad).sortedlink;
		} /*while*/
		
	return (false);
	} /*hashsortedsearch*/
	
	
boolean hashgetnthnode (hdlhashtable htable, long n, hdlhashnode *hnode) {
	
	/*
	n is 0-based.  we return a handle to the node for the indicated item number.
	
	9/12/90 DW: add defensive driving -- don't crash when there are zero items in 
	the table and the caller is asking for info about item #0.
	
	4/18/91 dmb: fixed loop's nil test; used to crash when n was just out of range.
	*/
	
	register hdlhashnode nomad = (**htable).hfirstsort;
	register long ct = n;
	
	*hnode = nil;
	
	if (nomad == nil) /*defensive driving*/
		return (false);
	
	while (--ct >= 0) {
		
		nomad = (**nomad).sortedlink;
		
		if (nomad == nil) /*there aren't that many items in the table*/
			return (false);
		} /*for*/
	
	*hnode = nomad;
	
	return (true);
	} /*hashgetnthnode*/
	
	
boolean hashgetsortedindex (hdlhashtable htable, hdlhashnode hnode, long *idx) {
	
	/*
	traverse the sorted list for the indicated table, looking for the indicated node.
	
	if we find it, set *idx to its index, and return true.
	
	the index is 0-based.
	*/
	
	register hdlhashnode nomad = (**htable).hfirstsort;
	register long ct = 0;
	
	while (nomad != nil) {
		
		if (nomad == hnode) { /*found it*/
			
			*idx = ct;
			
			return (true);
			}
			
		nomad = (**nomad).sortedlink;
		
		ct++;
		} /*while*/
		
	return (false); /*not found*/
	} /*hashgetsortedindex*/


boolean hashgetiteminfo (hdlhashtable htable, long item, bigstring bsname, tyvaluerecord *val) {
	
	/*
	the item number is 0-based.  we return the name and value information for the
	indicated item number, returning false if there aren't that many items.
	
	4/3/92 dmb: must check for unresolved addresses here
	
	3/19/93 dmb: if bsname is nil, don't set it
	
	6/7/96 dmb: if val is nil, don't set it either. (For ODBEngine, but useful elsewhere)
	
	5.0a23 dmb: don't resolve the value if caller doesn't need it.
	*/
	
	hdlhashnode hnode;
	
	if (!hashgetnthnode (htable, item, &hnode))
		return (false);
	
	if (bsname != nil)
		gethashkey (hnode, bsname);
	
	if (val != nil) {
		
		if (!hashresolvevalue (htable, hnode))
			return (false);
		
		*val = (**hnode).val;
		}
	
	return (true);
	} /*hashgetiteminfo*/
	

#if !odbengine
boolean hashgetvaluestring (tyvaluerecord val, bigstring bs) {
	
	/*
	a special entrypoint for creating a string representation of a value, 
	something worth displaying.  you shouldn't put up an error dialog for any 
	of these coercions, and it's ok not to replicate all the info in the coercion.
	
	5/21/91 dmb: copy valuerecord before coercing, or we trash caller's value
	
	12/22/92 dmb: if an error occurs during string coercion, must clear temps
	
	2.1b2 dmb: deparse string values
	
	2.1b4 dmb: don't deparse quotes, just non-printing characters (pass chnul)

	5.0.1 dmb: deparse filespec and alias values
	*/
	
	disablelangerror ();
	
	switch (val.valuetype) {
		
		case novaluetype:
			langgetmiscstring (nilstring, bs);
			
			break;
		
		case charvaluetype:
			setstringwithchar (val.data.chvalue, bs);
			
			langdeparsestring (bs, chnul);
			
			break;
		
		case booleanvaluetype:
		case intvaluetype:
		case longvaluetype:
		case directionvaluetype:
		case datevaluetype:
		case ostypevaluetype:
		case pointvaluetype:
		case rectvaluetype:
		case rgbvaluetype:
		case patternvaluetype:
		case fixedvaluetype:
		case singlevaluetype:
		case doublevaluetype:
		case objspecvaluetype:
		case enumvaluetype:
		case listvaluetype:
		case recordvaluetype:
		
			if (copyvaluerecord (val, &val) && coercetostring (&val)) {
				
				pullstringvalue (&val, bs);
				
				releaseheaptmp ((Handle) val.data.stringvalue);
				
				break;
				}
			
			cleartmpstack (); /*clean up on error*/
			
			langgetmiscstring (errorstring, bs);
			
			break;
		
		case filespecvaluetype:
		case aliasvaluetype:
		
			if (copyvaluerecord (val, &val) && coercetostring (&val)) {
				
				pullstringvalue (&val, bs);
				
				releaseheaptmp ((Handle) val.data.stringvalue);

				langdeparsestring (bs, chnul);
				
				break;
				}
			
			cleartmpstack (); /*clean up on error*/
			
			langgetmiscstring (errorstring, bs);
			
			break;

		case addressvaluetype:
			getaddresspath (val, bs);
			
			if (!isemptystring (bs))
				insertchar ('@', bs);
			
			break;
		
		case stringvaluetype:
			pullstringvalue (&val, bs);
			
			langdeparsestring (bs, chnul);
			
			break;
		
		case binaryvaluetype: {
			register Handle h = val.data.binaryvalue;
			long cthex = gethandlesize (h) - sizeof (OSType);
			
			if (cthex == 0)
				langgetmiscstring (nilstring, bs);
			else
				bytestohexstring (*h + sizeof (OSType), cthex, bs);
			
			/*
			OSType typeid;
			bigstring bstype, bsdata;
			
			typeid = **(OSType **) h;
			
			switch (typeid) {
				
				case 'TEXT':
				case 's255':
					texttostring (*h + sizeof (OSType), gethandlesize (h) - sizeof (OSType), bs);
					
					break;
				
				default:
					bytestohexstring (*h + sizeof (OSType), gethandlesize (h) - sizeof (OSType), bs);
					
					break;
				}
			*/
			
			break;
			}
		
		case externalvaluetype:
			langexternalgetdisplaystring ((hdlexternalhandle) val.data.externalvalue, bs);
			
			break;
		
		
		case codevaluetype:
			parsenumberstring (langmiscstringlist, treesizestring, langcounttreenodes (val.data.codevalue), bs);
			
			break;
		
		case tokenvaluetype:
			langgetmiscstring (tokennumberstring, bs);
			
			pushint (val.data.tokenvalue, bs);
			
			break;
		
		
		default:
			langgetmiscstring (unknownstring, bs);
		} /*switch*/
	
	enablelangerror ();
	
	return (true);
	} /*hashgetvaluestring*/
#endif

boolean hashgettypestring (tyvaluerecord val, bigstring bstype) {
	
	switch (val.valuetype) {
		
		case novaluetype:
			langgetmiscstring (nonestring, bstype);
			
			break;
		
		case intvaluetype: case longvaluetype:
			langgetmiscstring (numberstring, bstype);
			
			break;
		
		/*
		case tokenvaluetype:
			copystring ((ptrstring) "\pverb", bstype);
			
			break;
		
		case codevaluetype:
			copystring ((ptrstring) "\phandler", bstype);
			
			break;
		*/
		
		case externalvaluetype:
			langexternaltypestring ((hdlexternalhandle) val.data.externalvalue, bstype);
			
			break;
		
		default:
			langgettypestring (val.valuetype, bstype);
			
			break;
		} /*switch*/
	
	return (true);
	} /*hashgettypestring*/


boolean hashgetsizestring (const tyvaluerecord *val, bigstring bssize) {
	
	/*
	the size string tells you how much storage is allocated for the value.  if it
	returns non-empty, you might want to display it along with the type string.
	
	we only return non-empty size strings for types where size is interesting.  not
	for chars, ints, longs, booleans, dates, etc.
	*/
	
	long size;
	
	switch ((*val).valuetype) {
		
		case stringvaluetype:
		case listvaluetype:
		case recordvaluetype:
			if (langgetvalsize (*val, &size))
				numbertostring (size, bssize);
			
			break;
		

		case binaryvaluetype: {
			register Handle h = (*val).data.binaryvalue;
			OSType typeid;
			
			typeid = getbinarytypeid (h);
			
			ostypetostring (typeid, bssize);
			
			break;
			}
		
		default:
			setemptystring (bssize); /*often this string stays empty*/
			
			break;
		} /*switch*/
	
	return (true);
	} /*hashgetsizestring*/


boolean hashvaltostrings (tyvaluerecord val, bigstring bstype, bigstring bsvalue, bigstring bssize) {
	
	/*
	give me a value record and I'll return three strings suitable for display.
	
	the first string indicates the type of the value, and the second is the value.
	
	the third string tells you how much storage is allocated for the value.  if it
	returns non-empty, you might want to display it along with the type string.
	
	we only return non-empty size strings for types where size is interesting.  not
	for chars, ints, longs, booleans, dates, etc.
	
	10/7/91 dmb: for performance, don't call getvalsize for addresses -- we can 
	do a quick calc with the value string, instead of regenerating the full path.
	*/
	
	hashgetvaluestring (val, bsvalue);
	
	hashgettypestring (val, bstype);
	
	hashgetsizestring (&val, bssize);
	
	return (true);
	} /*hashvaltostrings*/





