
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

/*
	2004-10-26 aradke: New global header file, to be included from frontier.h and frontier.r.
*/

#ifndef __FRONTIERDEFS_H__
#define __FRONTIERDEFS_H__

#if   defined(__GNUC__)
    #define noextended 1
#else
    #define noextended 0
#endif

#undef MEMTRACKER		/* define as 1 to enable tracking of memory allocations */
#undef DATABASE_DEBUG	/* define as 1 to enable database debugging and logging code */

#define fldebug 1

#ifndef OPMLEDITOR				/*2008-09-08 aradke: keep opml editor lean and mean*/
	#define FRONTIER_MYSQL	1	/*include mysql database client library*/
	#define FRONTIER_SQLITE	1	/*include sqlite database code*/
#endif

#endif /*__FRONTIERDEFS_H__*/
