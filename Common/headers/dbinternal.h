
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

#define dbinternalinclude


#define SMART_DB_OPENING	1
//#undef SMART_DB_OPENING


/***RAB #define dbversionnumber 3 **VERSION used in 4.x**/

/***AR	#define dbversionnumber 5 /**VERSION used up to 6.2a10**/

#ifdef SMART_DB_OPENING
	#define dbversionnumber 6 /* 6.2a9 AR: added caching of shadow avail list in last block of database */
	#define dbfirstversionwithcachedshadowavaillist 6 /* 6.2a11 AR */
#else
	#define dbversionnumber 5 /**VERSION used up to 6.2a10**/
#endif

#define dbversionnumberminor 0


#define dbsystemidMac	0

#define dbsystemidWin32	1


#define minblocksize 32L
#define sizeheader (long)sizeof(tyheader)
#define sizetrailer (long)sizeof(tytrailer)
#define firstphysicaladdress (long)sizeof(tydatabaserecord)


typedef enum {
	dbdirtymask		= 0x0001
	} tydbflagmask;


typedef long tyvariance;


typedef struct tysizefreeword {
	
//	unsigned long flfree: 1;
	
//	unsigned long size: 31;
	long size;
	} tysizefreeword;


typedef struct tyheader {

	tysizefreeword sizefreeword;
	
	tyvariance variance;
	} tyheader, *ptrheader, **hdlheader;
	
	
typedef struct tytrailer {

	tysizefreeword sizefreeword;
	} tytrailer, *ptrtrailer, **hdltrailer;


#define dbshadow

typedef struct availnodeshadow {
	
	dbaddress adr;
	
	long size;
	// next record in this array is the next free block
	} tyavailnodeshadow, ** hdlavaillistshadow;


/*prototypes*/

extern boolean dbgeteof (long *);

extern boolean dbreadtrailer (dbaddress, boolean *, long *);

extern boolean dbreadheader (dbaddress, boolean *, long *, tyvariance *);

extern boolean dbreadavailnode (dbaddress, boolean *, long *, dbaddress *);



