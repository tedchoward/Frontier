
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
#include "dialogs.h"
#include "error.h"
#include "file.h"
#include "resources.h"
#include "strings.h"
#include "shell.h"
#include "db.h"
#include "dbinternal.h"
#include "ops.h" //6.2b3 AR: for numbertostring

#include "frontierdebug.h" //6.2b7 AR

#define dberrorlist 256

#define setdirty(hdb) 		((**hdb).flags |= dbdirtymask)
#define cleardirty(hdb)		((**hdb).flags &= ~dbdirtymask)
#define isdirty(hdb) 		((**hdb).flags & dbdirtymask)

#define majorversion(v)		(v & 0x00f0)
#define minorversion(v)		(v & 0x000f)

typedef enum {
	
	dbnoerror,
	
	dbwrongversionerror,
	
	dbfreeblockerror,

	dbfreelisterror,

	dbinconsistentavaillisterror,

	dbassignfreeblockerror,

	dbfilesizeerror,

	dbreleasefreeblockerror,

	dbreleaseinvalidblockerror,

	dbmergeinvalidblockerror
	} tydberror;


hdldatabaserecord databasedata; /*the global database handle*/

#if flruntime

	#define fldatabasesaveas false

#else

	boolean fldatabasesaveas = false; /*only true during Save As operation*/

#endif


static hdldatabaserecord databasedestination; /*for Save As*/

#if fldebug

static long leftmerges = 0, rightmerges = 0; /*statistics*/

static long splits = 0, nonsplits = 0; /*more statistics*/

static long allocs = 0, newallocs = 0, allocloops = 0; /*more statistics*/

#endif


#ifdef DATABASE_DEBUG

#pragma message ("*********************** DATABASE_DEBUG is ON: output to dblog.txt ***********************")

#define dberror(num) debug_dberror(num, __LINE__, true)
#define dblogerror(num) debug_dberror(num, __LINE__, false)
#define dbseteof(eof) debug_dbseteof(eof, __LINE__)


static boolean DBTRACKERGETROOTVISIT (WindowPtr w, bigstring bsfile) {

	hdlwindowinfo hinfo;

	if (getwindowinfo (w, &hinfo))
		if ((long) (**hinfo).fnum == (**databasedata).fnumdatabase) {

			copystring (fsname(&(**hinfo).fspec), bsfile);

			return (false); /*terminate visiting*/
			}

	return (true);
	}/*getrootnamevisit*/


static void DBTRACKERGETFILENAME (bigstring bsfile) {

	if (shellvisittypedwindows (idcancoonconfig, &DBTRACKERGETROOTVISIT, bsfile))
		setemptystring (bsfile); /*nothing found*/
	
	}/*DBTRACKERGETFILENAME*/


static void debug_dberror (short errnum, int line, boolean flthrow) {
	
	bigstring bs, bsfile;
	char str[1024];
	
	getstringlist (dberrorlist, errnum, bs);

	DBTRACKERGETFILENAME (bsfile);
	
	sprintf (str, "%s | %s [db.c,%ld]", stringbaseaddress (bsfile), stringbaseaddress (bs), line);

	DB_MSG_1 (str);

	if (flthrow)
		shellerrormessage (bs);
	} /*debug_dberror*/


static boolean debug_dbseteof (long eof, long line) {

	const long onegigabyte = 1024L * 1024L * 1024L;

	if (eof >= onegigabyte) {

		long oldeof = nil;

		if (dbgeteof (&oldeof) && (oldeof < onegigabyte)) {

			char str[1024];
			bigstring bsfile;
			
			DBTRACKERGETFILENAME (bsfile);
			
			sprintf (str, "%s | WARNING -- Growing database from %ld to %ld bytes. [db.c,%ld]", stringbaseaddress (bsfile), oldeof, eof, line);

			DB_MSG_1 (str);
			}
		}

	if ((eof & 0x80000000L) != 0x00000000L) {

		dberror (dbfilesizeerror);

		return (false); //trying to grow the file beyond 2 GB
		}

	return (fileseteof ((hdlfilenum)((**databasedata).fnumdatabase), eof));
	} /*dbseteof*/

#else

#define dblogerror(errnum)

static void dberror (short errnum) {
	
	bigstring bs;
	
	getstringlist (dberrorlist, errnum, bs);
	
	shellerrormessage (bs);
	} /*dberror*/

static boolean dbseteof (long eof) {

	if ((eof & 0x80000000L) != 0x00000000L) {

		dberror (dbfilesizeerror);

		return (false); //trying to grow the file beyond 2 GB
		}

	return (fileseteof ((hdlfilenum)((**databasedata).fnumdatabase), eof));
	} /*dbseteof*/

#endif


#define ctdatabasestack 10

short topdatabasestack = 0;

hdldatabaserecord databasestack [ctdatabasestack];


static boolean dbrelease (dbaddress); /*6.2b2: Dropped from db.h and declared static*/
static boolean dballocate (long databytes, ptrvoid pdata, dbaddress *paddress); /*6.2b14 AR: forward declaration for dbwriteshadowavaillist*/

boolean dbpushdatabase (hdldatabaserecord hdatabase) {
	
	/*
	when you want to temporarily work with a different databaserecord, call this
	routine, do your stuff and then call dbpopdatabase.
	*/
	
	if (topdatabasestack >= ctdatabasestack) {
		
		DebugStr (STR_database_stack_overflow);
		
		return (false);
		}
	
	databasestack [topdatabasestack++] = databasedata;
	
	if (hdatabase != nil)
		databasedata = hdatabase;
	
	return (true);
	} /*dbpushdatabase*/
		

boolean dbpopdatabase (void) {
	
	if (topdatabasestack <= 0)
		return (false);
	
	databasedata = databasestack [--topdatabasestack];
	
	return (true);
	} /*dbpopdatabase*/


static void dbswapglobals (void) {
	
	/*
	used to temporarily set and unset databasedestination as the databasehandle
	*/
	
	if (fldatabasesaveas) {
		
		hdldatabaserecord htemp = databasedata;
		
		databasedata = databasedestination;
		
		databasedestination = htemp;
		}
	} /*dbswapglobals*/


static boolean dbseek (dbaddress adr) {
	
	return (filesetposition((hdlfilenum)((**databasedata).fnumdatabase), adr));
	} /*dbseek*/
		
	
static boolean dbwrite (dbaddress adr, long ctbytes, ptrvoid pdata) {
	
	if (!dbseek (adr))
		return (false);
		
	return (filewrite ((hdlfilenum)((**databasedata).fnumdatabase), ctbytes, pdata)); 
	} /*dbwrite*/
	

static boolean dbread (dbaddress adr, long ctbytes, ptrvoid pdata) {

	if (!dbseek (adr))
		return (false);
				
	return (fileread ((hdlfilenum)((**databasedata).fnumdatabase), ctbytes, pdata)); 
	} /*dbread*/
	

static boolean dbwriteswap (dbaddress adr, long ctbytes, ptrvoid pdata) {
	boolean res;

	if (!dbseek (adr))
		return (false);

#ifdef WIN32
	if (ctbytes == sizeof (long))
		{
		longswap (*((long *)pdata));
		}

	if (ctbytes == sizeof(short))
		{
		shortswap (*((short*)pdata));
		}
#endif
	
	res = filewrite ((hdlfilenum)((**databasedata).fnumdatabase), ctbytes, pdata);

#ifdef WIN32
	if (ctbytes == sizeof (long))
		{
		longswap (*((long *)pdata));
		}

	if (ctbytes == sizeof(short))
		{
		shortswap (*((short*)pdata));
		}
#endif

	return (res);
	} /*dbwrite*/
	

static boolean dbreadswap (dbaddress adr, long ctbytes, ptrvoid pdata) {
	boolean res;

	if (!dbseek (adr))
		return (false);
				
	res = fileread ((hdlfilenum)((**databasedata).fnumdatabase), ctbytes, pdata); 

#ifdef WIN32
	if (ctbytes == sizeof (long))
		{
		longswap (*((long *)pdata));
		}
	else
		{
		if (ctbytes == sizeof(short))
			{
			shortswap (*((short*)pdata));
			}
		}
#endif

	return (res);
	} /*dbread*/
	

boolean dbgeteof (long *eof) {

	return (filegeteof ((hdlfilenum)((**databasedata).fnumdatabase), eof));
	} /*dbgeteof*/

		
static void dbheaderdirty (void) {
	
	/*
	5.0.1 dmb: was ^= instead of |=. Caused avail list database corruption
	*/
	
	(**databasedata).flags |= dbdirtymask;
	} /*dbheaderdirty*/


static boolean dbflushheader (void) {
	
	/*
	5.1.5 dmb: copy databasedata to local record for writing
	*/
	
	register hdldatabaserecord hdb = databasedata;
	boolean fl;
	tydatabaserecord diskrec;
	
	assert (sizeof (diskrec.u.growthspace) >= sizeof (diskrec.u.extensions));
	
	if (isdirty (hdb)) { /*changes made to header*/
		
		cleardirty (hdb); /*clear it*/
		
		diskrec = **hdb;

		#ifdef SMART_DB_OPENING		
			clearbytes (&diskrec.u.extensions.availlistshadow, sizeof (diskrec.u.extensions.availlistshadow)); /*in-memory structure only*/
			
			clearbytes (&diskrec.u.extensions.flreadonly, sizeof (diskrec.u.extensions.flreadonly)); /*in-memory structure only*/
		#else
			clearbytes (&diskrec.u.growthspace, sizeof (diskrec.u.growthspace)); /*in-memory structure only*/
		#endif

		#ifdef WIN32
			{
			short i;
			longswap (diskrec.availlist);
			longswap (diskrec.u.extensions.availlistblock);
			shortswap (diskrec.flags);
			for (i = 0; i < ctviews; i++)
				{
				longswap (diskrec.views[i]);
				}
		//	longswap (diskrec.fnumdatabase);
			longswap (diskrec.headerLength);
			shortswap (diskrec.longversionMajor);
			shortswap (diskrec.longversionMinor);
			}
		#endif
		
		fl = dbwrite ((dbaddress) 0, sizeof (tydatabaserecord), &diskrec);
		
		#ifdef WIN95VERSION
			if (fl)
				flushvolumechanges (nil, (hdlfilenum)((**databasedata).fnumdatabase));
		#else
		/*flush file buffers*/ {
			IOParam pb;
			
			clearbytes (&pb, sizeof (pb));
			
			pb.ioRefNum = (hdlfilenum)((**databasedata).fnumdatabase);
			
			PBFlushFile ((ParmBlkPtr) &pb, false);
			}
		#endif

		return (fl);
		} /*changes made to header*/
		
	return (true);
	} /*dbflushheader*/
	

boolean dbreadheader (dbaddress adr, boolean *flfree, long *ctbytes, tyvariance *variance) {
	
	tyheader header;
	
	if (!dbread (adr, sizeheader, &header))
		return (false);
	
#ifdef WIN32
	longswap(header.variance);
	longswap(header.sizefreeword.size);
#endif

	*flfree = (header.sizefreeword.size & 0x80000000L) == 0x80000000L ? true : false;
	
	*ctbytes = header.sizefreeword.size & 0x7FFFFFFFL;
	
	*variance = header.variance;
	
	return (true);
	} /*dbreadheader*/
	

boolean dbreadtrailer (dbaddress adr, boolean *flfree, long *ctbytes) {

	tytrailer trailer;
	
	if (!dbread (adr, sizetrailer, &trailer))
		return (false);
		
#ifdef WIN32
	longswap(trailer.sizefreeword.size);
#endif

	*flfree = (trailer.sizefreeword.size & 0x80000000L) == 0x80000000L ? true : false;
	
	*ctbytes = trailer.sizefreeword.size & 0x7FFFFFFFL;
	
	return (true);
	} /*dbreadtrailer*/
	

static boolean dbwriteheader (dbaddress adr, boolean flfree, long ctbytes, tyvariance variance) {

	tyheader header;
	
	header.sizefreeword.size = ctbytes;

	if (flfree)
		header.sizefreeword.size |= 0x80000000L;
	
	header.variance = variance;
	
#ifdef WIN32
	longswap(header.variance);
	longswap(header.sizefreeword.size);
#endif

	return (dbwrite (adr, sizeheader, &header));
	} /*dbwriteheader*/
	
	
static boolean dbwritetrailer (dbaddress adr, boolean flfree, long ctbytes) {

	tytrailer trailer;
	
	trailer.sizefreeword.size = ctbytes; 

	if (flfree)
		trailer.sizefreeword.size |= 0x80000000L;
	
#ifdef WIN32
	longswap(trailer.sizefreeword.size);
#endif

	return (dbwrite (adr, sizetrailer, &trailer));
	} /*dbwritetrailer*/


static boolean dbwriteheaderandtrailer (dbaddress adr, boolean flfree, long ctbytes, tyvariance variance) {
	
	if (!dbwriteheader (adr, flfree, ctbytes, variance))
		return (false);
	
	return (dbwritetrailer (adr + sizeheader + ctbytes, flfree, ctbytes));
	} /*dbwriteheaderandtrailer*/
	
		
boolean dbreadavailnode (dbaddress adr, boolean *flfree, long *ctbytes, dbaddress *link) {

	/*
	each node on the avail list has a link stored in its data field, we get
	all the usual data from the node and return that link in the nextnomad
	parameter.
	*/

	tyvariance variance; /*variance is irrelevent in avail nodes*/
	
	if (!dbreadheader (adr, flfree, ctbytes, &variance))
		return (false);
			
	return (dbreadswap (adr + sizeheader, sizeof (dbaddress), link));
	} /*dbreadavailnode*/
	

static boolean dbwriteavailnode (dbaddress adr, long ctbytes, dbaddress nextlink) {

	assert (adr != nil);
	
	assert ((**databasedata).u.extensions.availlistblock == nildbaddress);
	
	if (!dbwriteheader (adr, true, ctbytes, 0L))
		return (false);
	
	if (!dbwriteswap (adr + sizeheader, sizeof (dbaddress), &nextlink))
		return (false);

	if (!dbwritetrailer (adr + sizeheader + ctbytes, true, ctbytes))
		return (false);
	
	return (true);
	} /*dbwriteavailnode*/
	

static boolean dbsetavaillink (dbaddress adr, dbaddress link) {

	/*
	adr points to a record in the database file.  move past the header and
	write the link address in the first four bytes of the block's space.
	*/
	
	assert ((**databasedata).u.extensions.availlistblock == nildbaddress);
	
	if (adr == nildbaddress) { /*special case, set link in file header*/
		
		(**databasedata).availlist = link;
		
		dbheaderdirty ();
			
		return (true);
		}
		
	return (dbwriteswap (adr + sizeheader, sizeof (dbaddress), &link));
	} /*dbsetavaillink*/
	
	
static boolean dbwritedatablock (dbaddress adr, long databytes, long nodebytes, ptrvoid pdata) {

	/*
	there might be less data to write than there is room in the 
	block, so we only write as much as is necessary, but we size 
	the block according to its logical size.
	*/
	
	if (!dbwriteheader (adr, false, nodebytes, (tyvariance) nodebytes - databytes))
		return (false);
		
	if (pdata != nil) /*write data bytes*/
	
		if (!dbwrite (adr + sizeheader, databytes, pdata)) 
			return (false);
	
	return (dbwritetrailer (adr + nodebytes + sizeheader, false, nodebytes));
	} /*dbwritedatablock*/
	

static boolean dbfindpreviousavail (dbaddress adr, dbaddress *prev, long *ixshadow) {
	
	/*
	the available list is not doubly-linked.  this is where we pay the price.
	
	the caller wants to know which node in the avail list points at it.  if
	its the list header, we return nildbaddress.
	
	returns true if *prev was correctly set, false otherwise.
	
	5.1.5 dmb: use in-memory shadow
	*/
	
#ifdef dbshadow
	hdldatabaserecord hdb = databasedata;
	hdlavaillistshadow havailshadow = (hdlavaillistshadow) (**hdb).u.extensions.availlistshadow.data;
	long i, ctavail = (**hdb).u.extensions.availlistshadow.eof / sizeof (tyavailnodeshadow);
	
	for (i = 0; i < ctavail; ++i) {
		
		if ((*havailshadow) [i].adr == adr) {
			
			if (i > 0)
				*prev = (*havailshadow) [i - 1].adr;
			else
				*prev = nildbaddress;
			
			*ixshadow = i;
			
			return (true);
			}
		}
	
	dblogerror (dbfreelisterror); /*something fishy is going on*/
	
	return (false);
#else
	dbaddress nomad;
	boolean flfree;
	long ctbytes;
	dbaddress nextnomad;
	
	nomad = (**databasedata).availlist;
	
	if (nomad == adr) { /*he's the first guy on the list*/
	
		*prev = nildbaddress;
		
		return (true);
		}
		
	while (true) {
		
		if (nomad == nildbaddress) /*reached end of list, no one points at the node*/
			return (false);
			
		if (!dbreadavailnode (nomad, &flfree, &ctbytes, &nextnomad))
			return (false);
			
		if (nextnomad == adr) { /*found the guy that points at our friend*/
			
			*prev = nomad;
			
			return (true);
			}
		
		nomad = nextnomad; /*advance to next node*/
		} /*while*/
#endif
	} /*dbfindpreviousavail*/



#ifdef SMART_DB_OPENING	

static void dbclearshadowavaillist (void) {
	
	/*
	6.2b12 AR: This function MUST be called before modifying the linked list
	of available blocks on disk. We reset the pointer to the cached shadow avail list
	in the database header, set the header's dirty bit, and flush the header to disk.
	
	Finally, we also release the block allocated for the cached shadow avail list.
	
	Do nothing if we're performing a Save A Copy or if the db was opened read-only.
	*/

	register hdldatabaserecord hdb = databasedata;
	
	if (!fldatabasesaveas && !(**hdb).u.extensions.flreadonly)

		if ((**hdb).u.extensions.availlistblock != nildbaddress) {
	
			dbaddress adrblock = (**hdb).u.extensions.availlistblock;
			
			(**hdb).u.extensions.availlistblock = nildbaddress;
			
			dbheaderdirty ();
			
			dbflushheader ();		
			
			if (!dbrelease (adrblock)) {
				#ifdef DATABASE_DEBUG
					char str[256];

					sprintf (str, "dbrelease failed for address %ld.", (**hdb).u.extensions.availlistblock);

					DB_MSG_2 (str);
				#endif
				}
			}
			
	return;
	} /*dbclearshadowavaillist*/


static void dbdisposeshadowavaillist (void) {
	
	register hdldatabaserecord hdb = databasedata;
	handlestream s;
	
	assert (hdb != nil);

	s = (**hdb).u.extensions.availlistshadow;

	disposehandlestream (&s);
		
	clearbytes (&s, sizeof (s));
		
	(**hdb).u.extensions.availlistshadow = s;

	return;
	} /*dbdisposeshadowavaillist*/

#endif


static boolean dbwriteshadowavaillist (void) {

	/*
	6.2a9 AR: If there's an in-memory shadow avail list, write it to a faked new data block
	at the end of the database and store a pointer to the block in the file header.
	
	What happens if the database is opened by a version of Frontier that doesn't know about
	this convention? When it first saves the database, it will write the db header, thereby
	destroying the pointer to the on-disk shadow avail list. We end up with an orphaned block
	in the database, typically a few dozen kB in size. Next time the user saves a copy,
	the orphaned block is dropped from the database. Not a big deal.
	
	Also see dbreadshadowavaillist.

	If we don't have write permission, just dispose of everything. Don't even flush the db header.
	
	6.2b14 AR: Before we call dbwritedatablock, we have to determine the actual size
	of the allocated block. It could be larger than what we asked for. So we call
	dbreadheader to update the value of nodebytes to the real thing.
	*/

	register hdldatabaserecord hdb = databasedata;
	dbaddress adrblock = nil;
	boolean fl = false;
	boolean flfree;
	
	assert (hdb != nil);

	if ((**hdb).u.extensions.flreadonly || fldatabasesaveas)
		return (true); /*we're done already*/
	
	dbclearshadowavaillist ();
		
	if ((**hdb).u.extensions.availlistshadow.data == nil)
		return (true); /*we're done already*/
	
	if ((**hdb).u.extensions.availlistshadow.eof > 0) { /*there's something to be saved*/
	
		long nodebytes = (**hdb).u.extensions.availlistshadow.eof;
		long databytes, dummy;
		Handle h = nil;
		
		if (!dballocate (nodebytes, nil, &adrblock))
			goto error;
		
		assert (adrblock != nil);

		closehandlestream (&(**hdb).u.extensions.availlistshadow);
		
		if (!copyhandle ((**hdb).u.extensions.availlistshadow.data, &h))
			goto error;
		
		databytes = gethandlesize (h);
		
		assert (databytes == nodebytes || databytes == nodebytes - (long) sizeof (tyavailnodeshadow));

		#if 0 //DATABASE_DEBUG //6.2b7 AR: debugging check disabled

			/*verify cached version of avail list*/ {

				tyavailnodeshadow diskavailrec;
				tyavailnodeshadow memavailrec;
				dbaddress nextavail;
				boolean flfree;
				long dbeof;
				long ix = 0;
			
				if (!dbgeteof (&dbeof))
					goto error;

				diskavailrec.adr = (**hdb).availlist;

				while (diskavailrec.adr != nildbaddress) {
					
					if (!dbreadavailnode (diskavailrec.adr, &flfree, &diskavailrec.size, &nextavail) ||
						!flfree ||
						diskavailrec.adr + diskavailrec.size > dbeof) {

						diskavailrec.adr = nildbaddress;
						
						assert (false);

						break;
						}
					
					memavailrec = ((tyavailnodeshadow*)(*h)) [ix++];

					assert (diskavailrec.adr == memavailrec.adr);
					
					assert (diskavailrec.size == memavailrec.size);

					assert (ix * sizeof (tyavailnodeshadow) < databytes);

					diskavailrec.adr = nextavail;
					
					rollbeachball ();
					} /*while*/	
				
				diskavailrec.size = 0;

				memavailrec = ((tyavailnodeshadow*)(*h)) [ix++];

				assert (diskavailrec.adr == memavailrec.adr);
					
				assert (diskavailrec.size == memavailrec.size);

				assert (ix * sizeof (tyavailnodeshadow) == databytes);
				}
		#endif
		
		#ifdef WIN32
			/*switch byte order*/ {
			long ix;
			long ct = databytes / sizeof (tyavailnodeshadow);
			register tyavailnodeshadow* p = (tyavailnodeshadow *) *h;

			for (ix = 0; ix < ct; ix++) {
				longswap (p[ix].adr);
				longswap (p[ix].size);
				}
			}
		#endif

		lockhandle (h);
		
		fl = dbreadheader (adrblock, &flfree, &nodebytes, &dummy);
		
		assert (databytes <= nodebytes);
		
		fl = fl && dbwritedatablock (adrblock, databytes, nodebytes, *h);
		
		unlockhandle (h);

		disposehandle (h);
		
		if (!fl)
			goto error;
		}
	
	fl = true;

error:

	(**hdb).u.extensions.availlistblock = fl ? adrblock : nildbaddress;

	setdirty (hdb);
	
	dbflushheader ();

	return (fl);
	}/*dbwriteshadowavaillist*/


static boolean dbreadshadowavaillist (void) {

	/*
	6.2a9 AR: If the availlistblock was set in the database header, go in and
	read the avail list block from the end of the database instead of chaining
	thru the linked list of free blocks. Nuke the reference to the availlistblock
	in the db header asap, so if we crash somewhere down the road, we won't read
	an inconsistent availlistblock the next time we open the db.
	
	Also see dbwriteshadowavaillist.
	*/

	register hdldatabaserecord hdb = databasedata;
	dbaddress adrblock;
	hdlavaillistshadow h;
	handlestream s;
	long dbeof;

	assert (hdb != nil);
	
	if (!dbgeteof (&dbeof))
		return (false);
	
	adrblock = (**hdb).u.extensions.availlistblock;

	if (adrblock == nildbaddress) /*for safety*/
		return (false);

	if (!dbrefhandle (adrblock, (Handle*) &h))
		return (false);

#ifdef WIN32
	/*switch byte order*/ {
		long ix;
		long ct = gethandlesize ((Handle) h) / sizeof (tyavailnodeshadow);
		register tyavailnodeshadow* p = *h;

		for (ix = 0; ix < ct; ix++) {
			longswap (p[ix].adr);
			longswap (p[ix].size);
			}
		}
#endif

	/*Test consistency of cached shadow avail list*/
	
	if ((**h).adr != (**hdb).availlist) {
		
		dblogerror (dbinconsistentavaillisterror);
		
		disposehandle ((Handle) h);
	
		return (false);
		}

	if ((**h).adr != nildbaddress) {

		long availbytes;
		dbaddress firstavail = (**h).adr;
		dbaddress nextavail;
		boolean flfree;
	
		if (!dbreadavailnode (firstavail, &flfree, &availbytes, &nextavail)
				|| !flfree || firstavail + availbytes > dbeof) {
			
			dblogerror (dbinconsistentavaillisterror);
			
			disposehandle ((Handle) h);
		
			return (false);
			}
		}

	openhandlestream ((Handle)h, &s);

#if 0 //DATABASE_DEBUG //6.2b7 AR: debugging code disabled

	/*verify cached version of avail list*/ {

		tyavailnodeshadow diskavailrec;
		tyavailnodeshadow memavailrec;
		dbaddress nextavail;
		boolean flfree;
		long ix = 0;
	
		diskavailrec.adr = (**hdb).availlist;

		while (diskavailrec.adr != nildbaddress) {
			
			if (!dbreadavailnode (diskavailrec.adr, &flfree, &diskavailrec.size, &nextavail) ||
				!flfree ||
				diskavailrec.adr + diskavailrec.size > dbeof) {

				diskavailrec.adr = nildbaddress;
				
				assert (false);

				break;
				}
			
			memavailrec = ((tyavailnodeshadow*)(*s.data)) [ix++];

			assert (diskavailrec.adr == memavailrec.adr);
			
			assert (diskavailrec.size == memavailrec.size);

			assert (ix * sizeof (tyavailnodeshadow) < s.eof);

			diskavailrec.adr = nextavail;
			
			rollbeachball ();
			} /*while*/	
		
		diskavailrec.size = 0;

		memavailrec = ((tyavailnodeshadow*)(*s.data)) [ix++];

		assert (diskavailrec.adr == memavailrec.adr);
			
		assert (diskavailrec.size == memavailrec.size);

		assert (ix * sizeof (tyavailnodeshadow) == s.eof);
		}
#endif
		
	(**hdb).u.extensions.availlistshadow = s;
	
	return (true);
	}/*dbreadshadowavaillist*/


static boolean dbshadowavaillist (void) {
	
	/*
	5.1.5 dmb: read the entire avail list into memory. the last record in 
	the shadow array is {0, 0}
	
	6.2a9 AR: streamlined usage of handlestream, no longer keep separate
	local havaillist handle around which would interfere with the new
	shadow avail list caching in the db. (see dbwriteshadowavaillist)
	
	If dbreadschadowaviallist doesn't succeed, we try the old-fashioned way.
	*/
	
	handlestream s;
	tyavailnodeshadow availrec;
	dbaddress nextavail;
	boolean flfree;
	long dbeof;

#ifdef SMART_DB_OPENING	
	if ((**databasedata).u.extensions.availlistblock != nildbaddress)
		if (dbreadshadowavaillist ())
			return (true);
#endif

	if (!dbgeteof (&dbeof))
		return (false);
	
	openhandlestream (nil, &s);
	
	availrec.adr = (**databasedata).availlist;
	
	while (availrec.adr != nildbaddress) {
		
		if (!dbreadavailnode (availrec.adr, &flfree, &availrec.size, &nextavail) ||
			!flfree ||
			availrec.adr + availrec.size > dbeof) {

			availrec.adr = nildbaddress;
			
			dberror (dbfreelisterror);

			break;
			}
		
		if (!writehandlestream (&s, &availrec, sizeof (availrec)))
			goto error;
		
		availrec.adr = nextavail;
		
		rollbeachball ();
		} /*while*/	
	
	availrec.size = 0;
	
	if (!writehandlestream (&s, &availrec, sizeof (availrec)))
		goto error;
	
	(**databasedata).u.extensions.availlistshadow = s;
	
	return (true);
	
	error:
		disposehandlestream (&s);
		
		return (false);
	} /*dbshadowavaillist*/


static boolean dbinsertavailshadow (long ixshadow, dbaddress adr, long ctbytes) {
	
	handlestream s = (**databasedata).u.extensions.availlistshadow;
	tyavailnodeshadow avail;
	
	assert ((ixshadow >= 0) && (ixshadow <= s.eof / (long) sizeof (tyavailnodeshadow)));
	
	avail.adr = adr;
	
	avail.size = ctbytes;
	
	s.pos = ixshadow * sizeof (tyavailnodeshadow);
	
	if (!mergehandlestreamdata (&s, 0L, &avail, sizeof (avail)))
		return (false);
	
	(**databasedata).u.extensions.availlistshadow = s;
	
	return (true);
	} /*dbinsertavailshadow*/


static boolean dbdeleteavailshadow (long ixshadow) {

	handlestream s = (**databasedata).u.extensions.availlistshadow;
	
	assert ((ixshadow >= 0) && (ixshadow < s.eof / (long) sizeof (tyavailnodeshadow)));
	
	s.pos = ixshadow * sizeof (tyavailnodeshadow);
	
	if (!pullfromhandlestream (&s, sizeof (tyavailnodeshadow), nil))
		return (false);
	
	(**databasedata).u.extensions.availlistshadow = s;
	
	return (true);
	} /*dbdeleteavailshadow*/


static boolean dbsetavailshadow (long ixshadow, dbaddress adr, long ctbytes) {

	handlestream s = (**databasedata).u.extensions.availlistshadow;
	tyavailnodeshadow avail;
	
	assert ((ixshadow >= 0) && (ixshadow < s.eof / (long) sizeof (tyavailnodeshadow)));
	
	avail.adr = adr;
	
	avail.size = ctbytes;
	
	s.pos = ixshadow * sizeof (tyavailnodeshadow);
	
	if (!mergehandlestreamdata (&s, sizeof (avail), &avail, sizeof (avail)))
		return (false);
	
	(**databasedata).u.extensions.availlistshadow = s;
	
	return (true);
	} /*dbsetavailshadow*/


static boolean dbgetsizeandvariance (dbaddress adr, long *size, tyvariance *variance) {

	/*
	give me the address of a database block and I'll return the number
	of bytes it has reserved.  the variance is the number of unused bytes
	that are the result of block-splitting in allocate.
	*/
	
	boolean flfree;
	
	return (dbreadheader (adr, &flfree, size, variance));
	} /*dbgetsizeandvariance*/
	

static boolean dbsetsize (dbaddress adr, long size, tyvariance variance) {
	
	return (dbwriteheader (adr, false, size, variance));
	} /*dbsetsize*/
	

boolean dbreference (dbaddress adr, long maxbytes, ptrvoid pdata) {

	/*
	copy into pdata the database block located at address.  the number
	of bytes is found in the header/trailer word at the beginning of
	the block.
	
	under no circumstances will we read in more than maxbytes.  the caller
	should supply us with the size of pdata in this argument, it prevents
	disastrous overwriting of memory.
	
	each block also records a variance -- the number of extra bytes 
	due to block-splitting in allocate.  we only copy the number of
	bytes that actually hold the caller's data, probably saves a little
	time, and keeps us from overwriting other important stuff!
	*/
	
	long ctbytes;
	boolean flfree;
	tyvariance variance;
	
	if (!dbreadheader (adr, &flfree, &ctbytes, &variance))
		return (false);
		
	if (flfree || (ctbytes < 0)) { /*referencing a free node -- probably a bad address*/
		
		dberror (dbfreeblockerror);
		
		return (false);
		}
	
	return (dbread (adr + sizeheader, min (maxbytes, ctbytes - (long) variance), pdata));
	} /*dbreference*/
	

boolean dbrefhandle (dbaddress adr, Handle *h) {

	/*
	copy a block from the database into a handle which we allocate.
	
	the caller must dispose of the handle.
	
	5.0.1 dmb: added freeblock error; don't fail silently
	*/
	
	register dbaddress a = adr;
	register boolean fl;
	register Handle hregister;
	register long ct;
	long ctbytes;
	boolean flfree;
	tyvariance variance;
		
	*h = nil;
		
	if (a == nildbaddress) /*defensive driving*/
		return (false);
	
	if (!dbreadheader (a, &flfree, &ctbytes, &variance))
		return (false);
		
	ct = ctbytes - (long) variance;
	
	if (flfree || (ct < 0)) { /*probably a bad address*/
		
		dberror (dbfreeblockerror);
		
		return (false);
		}
		
	if (!newclearhandle (ct, h))
		return (false);
	
	hregister = *h;
	
	lockhandle (hregister);
	
	fl = dbread (a + sizeheader, ct, *hregister);
	
	unlockhandle (hregister);
	
	return (fl);
	} /*dbrefhandle*/
	

#if 0

static boolean dbrefbytes (dbaddress adr, long ctwanted, ptrvoid pdata) {

	/*
	copy into pdata the database block located at address.  this call
	is used when you want fewer than the "natural" number of bytes that
	dbreference returns.
	
	5.0.1 dmb: added freeblock error; don't fail silently
	*/
	
	long ctbytes;
	boolean flfree;
	tyvariance variance;
	
	if (!dbreadheader (adr, &flfree, &ctbytes, &variance))
		return (false);
		
	if (flfree || (ctbytes < 0)) { /*referencing a free node -- probably a bad address*/
		
		dberror (dbfreeblockerror);
		
		return (false);
		}
	
	ctwanted = min (ctwanted, ctbytes - (long) variance);
	
	return (dbread (adr + sizeheader, ctwanted, pdata));
	} /*dbrefbytes*/

#endif	


static boolean dballocate (long databytes, ptrvoid pdata, dbaddress *paddress) {

	/*
	allocate databytes space in the database.  return the database address of the
	allocated space in paddress.  if allocation error, paddress == nildbaddress.
	
	the caller can supply the address of data to be saved in the database, if its
	not nil, we will copy the data into the file before returning.

	4.1b9 dmb: removed special case check for nil prevnomad; dbsetavaillink handles
	that case.
	
	5.1.5b1 dmb: use and maintain availlist shadow
	*/

	long origeof;
	long nodebytes, newnodebytes;
	//boolean flfree;
	dbaddress nomad, prevnomad, nextnomad;
	tyvariance variance;
	long smallestinterestingblock;
	long ctalloc;
	
#if fldebug
	allocs++;
#endif

#ifdef SMART_DB_OPENING	
	dbclearshadowavaillist (); /*6.2b12 AR*/
#endif

	dbswapglobals (); /*use databasedestination*/
	
	smallestinterestingblock = max (databytes, (long) minblocksize);
	
#ifdef dbshadow
	{
	hdldatabaserecord hdb = databasedata;
	hdlavaillistshadow havailshadow = (hdlavaillistshadow) (**hdb).u.extensions.availlistshadow.data;
	long i, ctavail = (**hdb).u.extensions.availlistshadow.eof / sizeof (tyavailnodeshadow);
	
	for (i = 0, prevnomad = nildbaddress; i < ctavail; ++i, prevnomad = nomad) {

#if	fldebug
		allocloops++;
#endif

		nomad = (*havailshadow) [i].adr;
		
		if (nomad == nildbaddress)
			break;
		
		nodebytes = (*havailshadow) [i].size;
		
		if (nodebytes < smallestinterestingblock) //too small to be of interest
			continue;

		/*found a block to allocate off avail list*/
		
		//if (!dbreadavailnode (nomad, &flfree, &nodebytes, &nextnomad))
		//	goto failure;
		
		nextnomad = (*havailshadow) [i + 1].adr;
		
		//assert (nodebytes == (*havailshadow) [i].size);
		
		variance = nodebytes - databytes; //how much more we got than what we asked for
		
		if (variance >= (minblocksize + sizeheader + sizetrailer)) { //split into two blocks
			
			newnodebytes = nodebytes - (databytes + sizeheader + sizetrailer);
			
			if (!dbwriteheaderandtrailer (nomad, true, newnodebytes, (tyvariance) 0))
				goto failure;
			
			dbsetavailshadow (i, nomad, newnodebytes);
			
			nomad += sizeheader + newnodebytes + sizetrailer;
			
			if (!dbwritedatablock (nomad, databytes, databytes, pdata))
				goto failure;
			
			*paddress = nomad; /*use the newly split off block*/
			

#if fldebug
			splits++;
#endif

			goto success;
			} /*splitting into two blocks*/
		
		if (!dbwritedatablock (nomad, databytes, nodebytes, pdata))
			goto failure;
		

#if fldebug
		nonsplits++;
#endif		

		*paddress = nomad;
		
		dbdeleteavailshadow (i);
		
		if (!dbsetavaillink (prevnomad, nextnomad)) /*unlink node from avail list*/
			goto failure;
		
		goto success;
		}
	}
#else
	nomad = (**databasedata).availlist;
	
	prevnomad = nildbaddress; /*no previous node*/
	
	while (nomad != nildbaddress) { /*look at each element on the avail list, first-fit*/
		
		if (!dbreadavailnode (nomad, &flfree, &nodebytes, &nextnomad))
			goto failure;
		
		if (nodebytes < smallestinterestingblock) /*too small to be of interest*/
			goto nextloop;
		
		/*found a block to allocate off avail list*/
		
		variance = nodebytes - databytes; /*how much more we got than what we asked for*/
		
		if (variance >= (minblocksize + sizeheader + sizetrailer)) { /*split into two blocks*/
			
			newnodebytes = nodebytes - (databytes + sizeheader + sizetrailer);
			
			if (!dbwriteheaderandtrailer (nomad, true, newnodebytes, (tyvariance) 0))
				goto failure;
				
			nomad += sizeheader + newnodebytes + sizetrailer;
			
			if (!dbwritedatablock (nomad, databytes, databytes, pdata))
				goto failure;
				
			*paddress = nomad; /*use the newly split off block*/
			

#if fldebug
			splits++;
#endif			

			goto success;
			} /*splitting into two blocks*/
			
		if (!dbwritedatablock (nomad, databytes, nodebytes, pdata))
			goto failure;

#if fldebug		
		nonsplits++;
#endif

		*paddress = nomad;
		
		if (!dbsetavaillink (prevnomad, nextnomad)) /*unlink node from avail list*/
			goto failure;
		
		goto success;
		
		nextloop:
		
		prevnomad = nomad; /*remember in case we have to unlink the next one*/
		
		nomad = nextnomad; /*advance to next node in the avail list*/
		} /*while*/
#endif

#if fldebug	
	newallocs++;
#endif

	if (!dbgeteof (&origeof))
		goto failure;
	
	if (databytes < minblocksize) { /*we never alloc a block smaller than minblocksize*/
		
		ctalloc = minblocksize;
		
		variance = minblocksize - databytes;
		}
	else {
	
		ctalloc = databytes;
		
		variance = 0;
		}
		
	if (!dbseteof (origeof + sizeheader + ctalloc + sizetrailer))
		goto failure;
		
	if (!dbwritedatablock (origeof, databytes, ctalloc, pdata)) 	
		goto failure;
	
 	*paddress = origeof; /*this is the address of the block we allocated*/
	
	
	success:
	
	dbswapglobals (); /*restore*/
	
	return (true); /*the allocation was successful*/
	
	
	failure:
	
	dbswapglobals (); /*restore*/
	
	return (false);
	} /*dballocate*/


static boolean dbmergeleft (boolean flmerged, dbaddress adr, boolean* ptrflmergedleft) {

	/*
	try to merge the database block pointed to by adr with the block to its
	left.  this often may not be possible because the block to the left may
	or may not be free, or even may not exist.  return true if it worked, 
	false otherwise.
	
	we do nothing to the avail list if we merge.  we assume that the free
	block to the left is already on the avail list.
	
	flmerged tells us whether a right-merge has already been performed.  if
	so, the node at adr is on the available list and must be popped off the
	list if a left-merge takes place.
	
	5.1.5b1 dmb: maintain availlist shadow
	*/
	
	dbaddress newadr;
	long newsize;
	boolean flfree, flleftfree;
	long ctbytes, ctleftbytes;
	dbaddress nextavail, prevavail;
	long ixshadow;
	
	*ptrflmergedleft = false; /*default return value*/
	
	if (adr == firstphysicaladdress) /*nothing to the left, other than the header!*/
		return (true);

	if (adr < firstphysicaladdress) { /*nothing to the left, other than the header!*/

		dblogerror (dbmergeinvalidblockerror); /*illegal address*/

		return (false);
		}
	
	if (!dbreadtrailer (adr - sizetrailer, &flleftfree, &ctleftbytes))
		return (false);

	if (ctleftbytes < minblocksize) { /*probably an invalid block*/

		dblogerror (dbmergeinvalidblockerror); /*illegal address*/

		return (false);
		}

	if (!flleftfree) /*can't merge if block to left is not free*/
		return (true);

#ifdef fldebug //DATABASE_DEBUG
	{
		long leftblockadr = adr - sizetrailer - ctleftbytes - sizeheader;
		long dbeof;
		boolean flfreeheader;
		long ctbytesheader;
		tyvariance variance;

		if (!dbgeteof (&dbeof))
			return (false);

		if (leftblockadr < firstphysicaladdress) {

			dblogerror (dbmergeinvalidblockerror); /*illegal address*/

			return (false);
			}

		if (leftblockadr > dbeof) {

			dblogerror (dbmergeinvalidblockerror); /*illegal address*/

			return (false);
			}

		if (!dbreadheader (leftblockadr, &flfreeheader, &ctbytesheader, &variance))
			return (false);

		if (!flfreeheader) { /*the trailer said otherwise!*/

			dblogerror (dbmergeinvalidblockerror); /*illegal address*/

			return (false);
			}
		
		if (ctbytesheader != ctleftbytes) {

			dblogerror (dbmergeinvalidblockerror); /*illegal address*/

			return (false);
			}
	}
#endif

	if (!dbreadavailnode (adr, &flfree, &ctbytes, &nextavail)) /*get data about our block*/
		return (false);
	
	if (flmerged) { /*the node we're releasing is already on the avail list, pop him!*/
		
		if (!dbfindpreviousavail (adr, &prevavail, &ixshadow))
			return (false); /*damaged free list*/
		
		assert ((*(hdlavaillistshadow)(**databasedata).u.extensions.availlistshadow.data) [ixshadow + 1].adr == nextavail);
		
		if (!dbsetavaillink (prevavail, nextavail)) /*point around the soon-to-be-defunct node*/
			return (false);
			
		if (!dbdeleteavailshadow (ixshadow))
			return (false);
		}
	
	newsize = ctleftbytes + ctbytes + sizeheader + sizetrailer; /*start merging*/
	
	newadr = adr - sizetrailer - ctleftbytes - sizeheader;
	
	if (!dbwriteheaderandtrailer (newadr, true, newsize, 0L)) //avail link already set
		return (false);
	
	if (!dbfindpreviousavail (newadr, &prevavail, &ixshadow)) // don't need prev, just index
		return (false);

	dbsetavailshadow (ixshadow, newadr, newsize);
	
#if fldebug
	leftmerges++;
#endif

	*ptrflmergedleft = true; /*actually merged*/
	
	return (true);
	} /*dbmergeleft*/


static boolean dbmergeright (dbaddress adr, long ctbytes, boolean* ptrflmergedright) {

	/*
	try to merge the database block pointed to by adr with the block to its
	right.  this often may not be possible because the block to the right may
	or may not be free, or even may not exist.  return true if we merged, 
	false otherwise.
	
	we also adjust the available list if we merge.  it must point at the 
	beginning of the two merged blocks.
	
	we don't need to change the address because even if we merge, the address
	of the merged blocks is the same as adr.
	
	5.1.5b1 dmb: take ctbytes parameter so we don't need to re-read header;
	maintain availlist shadow

	6.2b5 AR: Do writes for merged block sequentially
	*/
	
	long eof;
	dbaddress rightblockadr;
	boolean flrightfree;
	long ctrightbytes;
	dbaddress prevavail, nextavail;
	long ixshadow;
	
	*ptrflmergedright = false;
	
	rightblockadr = adr + sizeheader + ctbytes + sizetrailer;
	
	if (!dbgeteof (&eof))
		return (false);

	if (rightblockadr == eof) /*there is no block to the right*/
		return (true);

	if (rightblockadr > eof) { /*reached the end of the file*/

		dblogerror (dbmergeinvalidblockerror);

		return (false);
		}

	if (!dbreadavailnode (rightblockadr, &flrightfree, &ctrightbytes, &nextavail))
		return (false);

	if (ctrightbytes < minblocksize) {

		dblogerror (dbmergeinvalidblockerror);

		return (false); //not likely to be a valid block, probably just a stream of nil bytes
		}
	
	if (!flrightfree) /*the block to the right is in use*/
		return (true);
	
#ifdef fldebug //DATABASE_DEBUG
	{
		long traileradr = rightblockadr + sizeheader + ctrightbytes;
		boolean flfreetrailer;
		long ctbytestrailer;

		if (traileradr < firstphysicaladdress) {

			dblogerror (dbmergeinvalidblockerror); /*illegal address*/

			return (false);
			}

		if (traileradr > eof) {

			dblogerror (dbmergeinvalidblockerror); /*illegal address*/

			return (false);
			}

		if (!dbreadtrailer (traileradr, &flfreetrailer, &ctbytestrailer))
			return (false);

		if (!flfreetrailer) { /*the header said otherwise!*/

			dblogerror (dbmergeinvalidblockerror); /*illegal address*/

			return (false);
			}
		
		if (ctbytestrailer != ctrightbytes) {

			dblogerror (dbmergeinvalidblockerror); /*illegal address*/

			return (false);
			}
	}
#endif

	if (!dbfindpreviousavail (rightblockadr, &prevavail, &ixshadow))
		return (false);
	
	assert ((*(hdlavaillistshadow)(**databasedata).u.extensions.availlistshadow.data) [ixshadow + 1].adr == nextavail);

	if (!dbsetavaillink (prevavail, adr)) /*point at beginning of two merged blocks*/
		return (false);
		
	ctbytes += ctrightbytes + sizeheader + sizetrailer;

	if (!dbwriteavailnode (adr, ctbytes, nextavail))
		return (false);

	if (!dbsetavailshadow (ixshadow, adr, ctbytes))
		return (false);
		
#if fldebug
	rightmerges++;
#endif

	*ptrflmergedright = true; /*actually merged*/
	
	return (true); /*actually merged*/
	} /*dbmergeright*/
	
	
static boolean dbrelease (dbaddress adr) {

	/*
	release the database block at adr.
	
	try to merge it with the block to the left and then with the block to right.
	
	push any new free block(s) on the available list.
	
	5.1.4 dmb: do all three writes sequentially (write availlink before trailer)

	6.2b3 AR: Change in our philosophy: We no longer consider it a big deal if releasing
	a block fails, but make absolutely sure we don't corrupt the database by releasing a non-existant
	block in the database. Most callers now ignore our return value.
	*/
	
	boolean flmergedleft, flmergedright;
	boolean flfree;
	long ctbytes;
	tyvariance variance;
	 
	if (adr == nildbaddress) /*its easy to release the nil node*/
		return (true);
	
#ifdef SMART_DB_OPENING	
	dbclearshadowavaillist (); /*6.2b12 AR*/
#endif

	if (!dbreadheader (adr, &flfree, &ctbytes, &variance)) 
		return (false);
	
	if (flfree) { /*nasty internal error - block is already free*/
		
		dberror (dbreleasefreeblockerror);
		
		return (false);
		}

#ifdef fldebug //DATABASE_DEBUG
	/*check header/trailer consistency*/ {

		boolean flfreetrailer;
		long ctbytestrailer;
		dbaddress traileradr = adr + sizeheader + ctbytes;
		long dbeof;

		if (!dbgeteof (&dbeof))
			return (false);

		if (traileradr > dbeof) { /*nasty internal error - probably not a valid address*/
			
			dblogerror (dbreleaseinvalidblockerror);
			
			return (false);
			}

		if (!dbreadtrailer (traileradr, &flfreetrailer, &ctbytestrailer))
			return (false);

		if (flfreetrailer) { /*nasty internal error - probably not a valid address*/
			
			dblogerror (dbreleaseinvalidblockerror);
			
			return (false);
			}

		if (ctbytes != ctbytestrailer) { /*nasty internal error - probably not a valid address*/
			
			dblogerror (dbreleaseinvalidblockerror);
			
			return (false);
			}
		}
#endif
	
	if (!dbmergeright (adr, ctbytes, &flmergedright))
		return (false);
	
	if (!dbmergeleft (flmergedright, adr, &flmergedleft))
		return (false);
		
	if (flmergedleft || flmergedright)
		return (true); /*we're done*/
	
	/*no merging -- set free bits in header & trailer, insert at head of avail list*/
	
	if (!dbwriteavailnode (adr, ctbytes, (**databasedata).availlist))
		return (false);
	
	(**databasedata).availlist = adr;
	
	if (!dbinsertavailshadow (0, adr, ctbytes))
		return (false);
	
	dbheaderdirty ();
	
	return (true);
	} /*dbrelease*/
	

#if 0

static boolean dbreadbytes (dbaddress adr, long offset, long ctbytes, char *pdata) {

	/*
	copy from the data part of the block at adr, at given offset into memory.
	
	this is useful if you want to stream a known amount of data out of a database block
	but don't want to allocate a temporary buffer to hold it all.
	*/
	
	return (dbread (adr + sizeheader + offset, ctbytes, pdata));
	} /*dbreadbytes*/
	

static boolean dbwritebytes (dbaddress adr, long offset, long ctbytes, char *pdata) {

	/*
	copy the data from memory to the data part of the block at adr, at given offset.
	
	this is useful if you want to stream a known amount of data into a database block
	but don't want to allocate a temporary buffer to hold it all.
	*/
	
	return (dbwrite (adr + sizeheader + offset, ctbytes, pdata));
	} /*dbwritebytes*/

#endif
	

static boolean dbmove (ptrvoid pdata, long ctbytes, dbaddress adr) {
	
	/*
	copy the data from memory (pdata) to the data part of the block at adr.
	
	call this when you know that the size of the object you're writing is the
	same as the object this block was created to hold.
	*/
	
	return (dbwrite (adr + sizeheader, ctbytes, pdata)); 
	} /*dbmove*/
	

boolean dbassign (dbaddress *padr, long newsize, ptrvoid pdata) {
	
	/*
	we want to move new data into the database block whose address is adr.
	
	maybe the size has changed?  think about variable length strings.  if so, the
	new size is given in newsize. 
	
	we get a pointer to the address because we might change the address if we have
	to allocate to fit new larger data.  we never re-allocate a block if the data
	got smaller.
	
	10/16/91 dmb: found longstanding bug.  the variance must we updated any time the 
	size changes, not just when the newsize is less than cttotal

	6.2b2 AR: Improved error checking based on the assumption that we should
	never assign to a free block -- except when saving a copy, of course.
	*/
	
	register dbaddress adr;
	tyvariance ctunused;
	long cttotal;
	boolean flfree;
	
	adr = *padr; /*copy into a register*/
	
	if (fldatabasesaveas || (adr == nildbaddress)) /*no previous allocation, create a new one*/
		return (dballocate (newsize, pdata, padr)); 
	
	if (!dbreadheader (adr, &flfree, &cttotal, &ctunused)) /*find out how much space we have in block*/
		return (false);

	if (flfree) { /*6.2b2 AR: here's another chance to easily detect corruption, why not use it?*/

		dberror (dbassignfreeblockerror);
	
		return (false);
		}
	
	if (newsize > cttotal) { /*there isn't enough room*/

		if (!dbrelease (adr)) { //ignore return value, don't want to abort saving
			#ifdef DATABASE_DEBUG
				char str[256];

				sprintf (str, "dbrelease failed for address %ld.", adr);

				DB_MSG_2 (str);
			#endif
			}

		return (dballocate (newsize, pdata, padr)); /*allocate the new, bigger block*/
		}
	
	if (newsize != cttotal - ctunused) /*must update the variance*/
	
		if (!dbsetsize (adr, cttotal, cttotal - newsize))
		
			return (false);
		
	return (dbmove (pdata, newsize, adr)); /*copy the data into a big-enough block*/
	} /*dbassign*/
	
	
static boolean dbgetsize (dbaddress adr, long *logicalsize) {

	/*
	give me the address of a database block and I'll return the number
	of logical bytes it is using.
	*/
	
	tyvariance size, variance;
	
	*logicalsize = 0;
	
	if (adr == nildbaddress) 
		return (false);
	
	if (!dbgetsizeandvariance (adr, &size, &variance)) 		
		return (false);
		
	*logicalsize = size - variance;
	
	return (true);
	} /*dbgetsize*/


boolean dbcopy (dbaddress adrorig, dbaddress *adrcopy) {
	
	/*
	create a copy of the database block pointed to by adrorig.  return
	true if adrcopy has the address of a new block, the same logical size
	as the original with a copy of the original's data.
	*/
	
	register boolean flreturned;
	Handle hnew;
	register Handle h;
	long size;
	
	if (adrorig == nildbaddress) { /*it's very easy to copy the nil node*/
		
		*adrcopy = nildbaddress;
		
		return (true);
		}
	
	if (!dbgetsize (adrorig, &size))
		return (false);
	
	if (!newhandle (size, &hnew)) /*not enough room in the heap*/
		return (false);
	
	h = hnew; /*copy into register*/
	
	lockhandle (h);
	
	flreturned = false; /*default*/
	
	if (dbreference (adrorig, size, *h))
	
		flreturned = dballocate (size, *h, adrcopy);
	
	unlockhandle (h);
	
	disposehandle (h);
	
	return (flreturned);
	} /*dbcopy*/
	
	
static boolean dballocstring (dbaddress *adr, bigstring bs) {
	
	return (dballocate ((long) stringlength(bs) + 1, bs, adr));
	} /*dballocstring*/
	

static boolean dbrefstring (dbaddress adr, bigstring bs) {
	
	setstringlength (bs, 0);
		
	if (adr == nildbaddress) /*nil adr represents an empty string, saves time & space*/
		return (true);
		
	return (dbreference (adr, sizeof (bigstring), bs));
	} /*dbrefstring*/
	
	
static boolean dbassignstring (dbaddress *adr, bigstring bs) {
	
	if (*adr == nildbaddress) 
		return (dballocstring (adr, bs));
	else
		return (dbassign (adr, (long) stringlength(bs) + 1, bs));
	} /*dbassignstring*/
	
	
static boolean dbreleasestring (dbaddress adr) {

	if (!dbrelease (adr)) {
		#ifdef DATABASE_DEBUG
			char str[256];

			sprintf (str, "dbrelease failed for address %ld.", adr);

			DB_MSG_2 (str);
		#endif

		return (false);
		}

	return (true);
	} /*dbreleasestring*/
	

boolean dbrefheapstring (dbaddress adr, hdlstring *hstring) {
	
	bigstring bs;
	
	if (!dbrefstring (adr, bs))
		return (false);
		
	return (newheapstring (bs, hstring));
	} /*dbrefheapstring*/
	

boolean dbassignheapstring (dbaddress *adr, hdlstring hstring) {

	bigstring bs;
	
	copyheapstring (hstring, bs); /*checks for hstring == nil*/
	
	if (isemptystring (bs)) {
	
		if (!fldatabasesaveas)
			dbreleasestring (*adr); 
		
		*adr = nildbaddress; /*default return value, indicates empty string*/
		
		return (true);
		}
	
	return (dbassignstring (adr, bs));
	} /*dbassignheapstring*/
	
	
boolean dballochandle (Handle halloc, dbaddress *adr) {
	
	register Handle h = halloc;
	register boolean fl;
	
	if (h == nil) { /*defensive driving, nil handles are represented by nil addresses*/
		
		*adr = nildbaddress;
		
		return (true);
		}
		
	lockhandle (h);
	
	fl = dballocate ((long) gethandlesize (h), *h, adr);
	
	unlockhandle (h);
	
	return (fl);
	} /*dballochandle*/
	
	
boolean dbassignhandle (Handle h, dbaddress *adr) {
	
	/*
	6/30/92 dmb: added check for nil handle
	*/
	
	register boolean fl;
	
	if (*adr == nildbaddress) /*creating a new guy*/
	
		return (dballochandle (h, adr));
	
	if (h == nil)
		return (dbassign (adr, 0, nil));
	
	lockhandle (h);
	
	fl = dbassign (adr, (long) gethandlesize (h), *h);
	
	unlockhandle (h);
	
	return (fl);
	} /*dbassignhandle*/
	
	
boolean dbsavehandle (Handle hsave, dbaddress *adr) {
	
	/*
	xxx -- not sure why this is needed, looks like dbassignhandle, above,  
	does the job fairly well.
	*/
	
	register Handle h = hsave;
	register long ctbytes;
	register boolean fl;
	dbaddress a = *adr;
	
	ctbytes = gethandlesize (h);
	
	lockhandle (h);
	
	if (a == nildbaddress) 
		fl = dballocate (ctbytes, *h, &a);
	else
		fl = dbassign (&a, ctbytes, *h);
		
	unlockhandle (h);
	
	*adr = a; /*copy into returned value*/
	
	return (fl);
	} /*dbsavehandle*/
	

/*
boolean dbnewarray (ctelements, sizeelement, pdata, adr) short ctelements, sizeelement; ptrvoid pdata; dbaddress *adr; {
	
	register long ctbytes;
	
	ctbytes = ((long) ctelements * sizeelement) + sizeof (tydbarrayheader);
	
	return (dballocate (ctbytes, pdata, adr));
	} /%dbnewarray%/
*/
	

void dbsetview (short viewnumber, dbaddress adrtext) {

	register hdldatabaserecord hdb;
	
	dbswapglobals ();
	
	hdb = databasedata; /*move into register*/
	
	(**hdb).views [viewnumber] = adrtext;
	
	setdirty (hdb);
	
	dbflushheader ();
	
	dbswapglobals ();
	} /*dbsetview*/


void dbgetview (short viewnumber, dbaddress *adrtext) {
	
	*adrtext = (**databasedata).views [viewnumber];
	} /*dbgetview*/


void dbcurrentdatabase (hdldatabaserecord hdb) {
	
	if (hdb != nil)
		databasedata = hdb; 
	} /*dbcurrentdatabase*/
	

void dbgetcurrentdatabase (hdldatabaserecord *hdb) {
	
	*hdb = databasedata;
	} /*dbgetcurrentdatabase*/
	
	
boolean dbfnumchanged (hdlfilenum newfnum) {
	
	register hdldatabaserecord hdb = databasedata;
	
	(**hdb).fnumdatabase = (long) newfnum;
	
	setdirty (hdb);
	
	return (dbflushheader ());
	} /*dbfnumchanged*/


#ifdef DATABASE_DEBUG

boolean debug_dbpushreleasestack (dbaddress adr, long valtype, long line, char *sourcefile) {
	
	/*
	the chunk of db space pointed to by adr is being logically released, but
	the caller is saying that he doesn't want to make the effects permanent
	until some time in the future.  he indicates it's time to release all these
	guys by calling dbflushreleasestack, below.
	
	if the user decides to not save changes, you should call dbzeroreleasestack.
	*/
	
	Handle hstack = (**databasedata).releasestack;
	tydbreleasestackframe info;

	if (adr == nildbaddress) /*no need to waste space on a nil address*/
		return (true);
		
	if (hstack == nil) {
		
		if (!newclearhandle (0L, &hstack))
			return (false);
			
		(**databasedata).releasestack = hstack;
		}

	clearbytes (&info, sizeof (info));

	info.adr = adr;

	info.id = valtype;

	info.line = line;

	newfilledhandle (sourcefile, strlen (sourcefile), &info.file);
		
	return (enlargehandle (hstack, sizeof (info), &info));
	} /*dbpushreleasestack*/


boolean dbflushreleasestack (void) {
	
	/*
	release all the chunks accumulated in the database's releasestack.
	
	5.1.4 dmb: don't lock the handle
	*/
	
	Handle h = (**databasedata).releasestack;
	tydbreleasestackframe info;
	long i, ct;
	long hsize;
	
	if (h != nil) {
		
		hsize = gethandlesize (h);
		
		ct = hsize / sizeof (info);
		
		for (i = 0; i < ct; ++i) {
			
			rollbeachball (); /*dmb 4.1b9*/

			info = ((tydbreleasestackframe*)(*h)) [i];
			
			if (!dbrelease (info.adr)) {

				bigstring bsfile;
				char str[256];

				texthandletostring (info.file, bsfile);

				sprintf (str, "dbrelease failed for address %ld, type %ld, line %ld in %s.", info.adr, info.id, info.line, stringbaseaddress (bsfile));
				
				DB_MSG_2 (str);
				}
			}

		disposehandle (h);
		
		(**databasedata).releasestack = nil;
		}
	
#ifdef SMART_DB_OPENING	
	dbwriteshadowavaillist (); /*6.2b12 AR: this is a good place to do it since we're about done with saving*/
#endif
	 
	return (true);
	} /*dbflushreleasestack*/

#else
	
boolean dbpushreleasestack (dbaddress adr, long valtype) {
	
	/*
	the chunk of db space pointed to by adr is being logically released, but
	the caller is saying that he doesn't want to make the effects permanent
	until some time in the future.  he indicates it's time to release all these
	guys by calling dbflushreleasestack, below.
	
	if the user decides to not save changes, you should call dbzeroreleasestack.

	6.2b3 AR: Added valtype parameter, only used in debug version (see above).
	*/
	
	Handle hstack = (**databasedata).releasestack;
	
	if (adr == nildbaddress) /*no need to waste space on a nil address*/
		return (true);
		
	if (hstack == nil) {
		
		if (!newclearhandle (0L, &hstack))
			return (false);
			
		(**databasedata).releasestack = hstack;
		}
		
	return (enlargehandle (hstack, sizeof (adr), &adr));
	} /*dbpushreleasestack*/


boolean dbflushreleasestack (void) {
	
	/*
	release all the chunks accumulated in the database's releasestack.
	
	5.1.4 dmb: don't lock the handle
	*/
	
	Handle h = (**databasedata).releasestack;
	long i, ct;
	long hsize;
	
	if (h != nil) {
		
		hsize = gethandlesize (h);
		
		ct = hsize / sizeof (dbaddress);
		
		for (i = 0; i < ct; ++i) {
			
			rollbeachball (); /*dmb 4.1b9*/
			
			dbrelease (((ptrdbaddress) (*h)) [i]);
			}

		disposehandle (h);
		
		(**databasedata).releasestack = nil;
		}
	
#ifdef SMART_DB_OPENING	
	dbwriteshadowavaillist (); /*6.2b12 AR: this is a good place to do it since we're about done with saving*/
#endif
	
	return (true);
	} /*dbflushreleasestack*/

#endif


static void dbzeroreleasestack (void) {
	
	disposehandle ((**databasedata).releasestack);
	
	(**databasedata).releasestack = nil;
	} /*dbzeroreleasestack*/


boolean dbdispose (void) {

	dbzeroreleasestack ();

#ifdef SMART_DB_OPENING	
	dbdisposeshadowavaillist ();
#else
	disposehandle ((**databasedata).u.extensions.availlistshadow.data);
#endif
	
	disposehandle ((Handle) databasedata);
	
	databasedata = nil;
	
	return (true);
	} /*dbdispose*/


boolean dbnew (hdlfilenum fnum) {
	
	/*
	2002-11-11 AR: Added assert to make sure the C compiler chose the
	proper byte alignment for the tydatabaserecord struct. If it did not,
	we would end up corrupting any database files we saved.
	*/
	
	register hdldatabaserecord hdb;
	
	assert (sizeof (tydatabaserecord) == 88);
	
	if (!newclearhandle (sizeof (tydatabaserecord), (Handle *) &databasedata))
		return (false);
		
	hdb = databasedata; /*copy into register*/
	
	(**hdb).fnumdatabase = (long) fnum;
	
#ifdef MACVERSION
	(**hdb).systemid = dbsystemidMac;
#endif

#ifdef WIN95VERSION
	(**hdb).systemid = dbsystemidWin32;
#endif

	(**hdb).versionnumber = dbversionnumber;

	(**hdb).headerLength = firstphysicaladdress;
	(**hdb).longversionMajor = dbversionnumber;
	(**hdb).longversionMinor = dbversionnumberminor;
	
	dbshadowavaillist ();

	setdirty (hdb);
	
	if (dbflushheader ()) /*initial info written to disk*/
		return (true);
	
	dbdispose (); /*error flushing the data out to disk*/
	
	return (false);
	} /*dbnew*/
	

boolean dbopenfile (hdlfilenum fnum, boolean flreadonly) {
	
	/*
	4.1b9 dmb: allow opening of databases newer than us, as long as 
	version number change is not major.
	
	5.1.5 dmb: use diskrec instead of handle locking; shadow avail list

	6.2a9 AR: To support the builtins.db verbs we need to know whether
	we have write permission, so we introduced the flreadonly param.
	
	2002-11-11 AR: Added assert to make sure the C compiler chose the
	proper byte alignment for the tydatabaserecord struct. If it did not,
	we would end up corrupting any database files we saved.
	*/

	tydatabaserecord diskrec;
	register hdldatabaserecord hdb;
	
	assert (sizeof (tydatabaserecord) == 88);
	
	if (!newclearhandle (longsizeof (tydatabaserecord), (Handle *) &databasedata))
		return (false);
	
	hdb = databasedata; /*copy into register*/
	
	(**hdb).fnumdatabase = (long) fnum; /*set up so dbread will work*/
	
	if (!dbread ((dbaddress) 0, sizeof (tydatabaserecord), &diskrec))
		goto error;
	
	#ifdef WIN32
		{
		short i;
		longswap (diskrec.availlist);
		longswap (diskrec.u.extensions.availlistblock);
		shortswap (diskrec.flags);
		for (i = 0; i < ctviews; i++)
			{
			longswap (diskrec.views[i]);
			}
//		longswap (diskrec.fnumdatabase);
		longswap (diskrec.headerLength);
		shortswap (diskrec.longversionMajor);
		shortswap (diskrec.longversionMinor);
		}
	#endif
	
	diskrec.fnumdatabase = (long) fnum; /*this just got overwritten*/
	
	diskrec.releasestack = nil; /*this is an in-memory structure only*/
	
	diskrec.u.extensions.flreadonly = flreadonly; /*this is an in-memory structure only*/

	**hdb = diskrec;
	
	if ((**hdb).versionnumber != dbversionnumber) {

		if (majorversion ((**hdb).versionnumber) != majorversion (dbversionnumber)) {
		
			dberror (dbwrongversionerror);
			
			goto error;
			}
		
		#ifdef SMART_DB_OPENING
		if ((**hdb).versionnumber < dbfirstversionwithcachedshadowavaillist)
			(**hdb).u.extensions.availlistblock = nildbaddress; /*don't count on old version to handle this one*/
		#endif
		
		(**hdb).versionnumber = dbversionnumber; /*we can only write what we know*/
		
		setdirty (hdb);
		}
		
	if (!dbshadowavaillist ())
		goto error;
	
	return (true);
	
	error:
	
	disposehandle ((Handle) hdb);
	
	databasedata = nil;
	
	return (false); /*error loading in header*/
	} /*dbopenfile*/


boolean dbclose (void) {
	
	dbzeroreleasestack (); /*don't release chunks accumulated in release stack*/
	
	setdirty (databasedata);
	
	return (dbflushheader ());
	} /*dbclose*/


boolean dbstartsaveas (hdlfilenum fnum) {
	
	register boolean fl;
		
	fldatabasesaveas = true; /*set global; enables databasehandle swapping*/
	
	dbswapglobals ();
	
	fl = dbnew (fnum);
	
	dbswapglobals ();
	
	fldatabasesaveas = fl;
	
	return (fl);
	} /*dbstartsaveas*/


boolean dbendsaveas (void) {
	
	register boolean fl;
	
	if (!fldatabasesaveas)
		return (false);
		
	dbswapglobals ();
	
	fl = dbclose ();
	
	dbdispose ();
	
	dbswapglobals ();
	
	fldatabasesaveas = false;
	
	return (fl);
	} /*dbendsaveas*/

