
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

 

#ifndef dbinclude /*so other includes can tell if we've been loaded*/
#define dbinclude /*so other includes can tell if we've been loaded*/


#ifdef HEADERTRACE
#pragma message( "**Compiling " __FILE__ )
#endif


#ifndef memoryinclude
	#include "memory.h"
#endif

#define nildbaddress 0L


#define ctviews 3 /*leave room for 3 different views in a database header*/
#define cancoonview 0

typedef long dbaddress, *ptrdbaddress, **hdldbaddress;

#pragma pack(2)
typedef struct tydatabaserecord { /*stored at offset 0 in the db file*/
	

	unsigned char systemid;		/* 0 = MAC (compatiblity)  */


	unsigned char versionnumber; /*which version created this file?*/
	
	dbaddress availlist; /*avail list is singly-linked, nil terminated*/
	
	short oldfnumdatabase; /*only applies when database record is in memory*/
	
	short flags; /*any changes to header since it was last flushed?*/
	
	dbaddress views [ctviews]; /*addresses of the root of each view*/
	
	Handle releasestack; /*holds addresses of nodes waiting to be released*/
	

						/* RAB 9/2/96 - fnumdatabase is delibritly declared

						as a long not a hdlfilenum. This is to allow for a consistent header

						size between Mac and Win platforms even though hdlfilenum is 

						system dependent.

						It has been moved to this position to allow view and fldirty

						to remain the same as version 3 files.

						*/

	long fnumdatabase; /* file handle while in memory - New to version 5*/

	long headerLength;		/*size of header - New to version 5*/

	short longversionMajor;	/*new extended version id - new to version 5*/

	short longversionMinor;	/*new extended version id - new to version 5*/
	
	union {
		char growthspace [50]; /*room for new fields without format change*/
		
		struct {
			dbaddress availlistblock; /*6.2a9 AR: on-disk structure mirroring availlist, a contiguous block*/
			
			handlestream availlistshadow; /*never saved to disk; in-memory structure mirroring availlist*/

			boolean flreadonly; /*6.2a9 AR: never saved to disk; if this is true, don't write to the file*/
			} extensions;
		} u;
	} tydatabaserecord, *ptrdatabaserecord, **hdldatabaserecord;
#pragma options align=reset
	
extern hdldatabaserecord databasedata; /*can be set by external user*/

extern boolean fldatabasesaveas;


#ifdef DATABASE_DEBUG

typedef struct {
	dbaddress	adr;
	long		id;
	long		line;
	Handle		file;
	} tydbreleasestackframe;

#define dbpushreleasestack(adr, valtype) \
	debug_dbpushreleasestack(adr, valtype, __LINE__, __FILE__);

extern boolean debug_dbpushreleasestack (dbaddress adr, long valtype, long line, char *sourcefile);

#else

extern boolean dbpushreleasestack (dbaddress, long);

#endif

/*prototypes*/

extern boolean dbpushdatabase (hdldatabaserecord);

extern boolean dbpopdatabase (void);

extern boolean dbreference (dbaddress, long, ptrvoid);

extern boolean dbrefhandle (dbaddress, Handle *);

extern boolean dbassign (dbaddress *, long, ptrvoid);

extern boolean dbcopy (dbaddress, dbaddress *);

extern boolean dballochandle (Handle, dbaddress *);

extern boolean dbassignhandle (Handle, dbaddress *);

extern boolean dbsavehandle (Handle, dbaddress *);

extern boolean dbrefheapstring (dbaddress, hdlstring *);

extern boolean dbassignheapstring (dbaddress *, hdlstring);

extern void dbsetview (short, dbaddress);

extern void dbgetview (short, dbaddress *);

extern void dbcurrentdatabase (hdldatabaserecord);

extern void dbgetcurrentdatabase (hdldatabaserecord *);

extern boolean dbflushreleasestack (void);

extern boolean dbfnumchanged (hdlfilenum);

extern boolean dbdispose (void);

extern boolean dbnew (hdlfilenum);

extern boolean dbopenfile (hdlfilenum, boolean);

extern boolean dbclose (void);

extern boolean dbstartsaveas (hdlfilenum);

extern boolean dbendsaveas (void);

extern boolean statsblockinuse (dbaddress, bigstring); /*dbstats.c*/

extern boolean dbstatsmessage (hdldatabaserecord, boolean); /*6.2a8 AR*/

extern boolean statsstart (void);

#endif



