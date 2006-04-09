
/*	$Id: $    */

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

#ifndef byteorderinclude
#define byteorderinclude

/*
	2006-04-08 aradke: determine native byte order and define byte swapping macros
*/

#if (defined(WIN32) || defined(__i386) || defined(__i386__)) && !defined(__LITTLE_ENDIAN__)
	#define __LITTLE_ENDIAN__	1
#endif


#if defined(__LITTLE_ENDIAN__) && ((__LITTLE_ENDIAN__ == 1) || !defined (__BIG_ENDIAN__))
	#define conditionallongswap(x) dolongswap(x)
	#define conditionalshortswap(x) doshortswap(x)
	#define conditionalenumswap(x) doshortswap(x)
	#define disklong(x) dolongswap(x)
	#define memlong(x) dolongswap(x)
	#define diskshort(x) doshortswap(x)
	#define memshort(x) doshortswap(x)
	#define disktomemshort(x) shortswap(x)
	#define disktomemlong(x)  longswap(x)
	#define memtodiskshort(x) shortswap(x)
	#define memtodisklong(x) longswap(x)
#elif defined(__BIG_ENDIAN__) && ((__BIG_ENDIAN__ == 1) || !defined (__LITTLE_ENDIAN__))
	#define conditionallongswap(x) x
	#define conditionalshortswap(x) x
	#define conditionalenumswap(x) x
	#define disklong(x) x
	#define memlong(x) x
	#define diskshort(x) x
	#define memshort(x) x
	#define disktomemshort(x)
	#define disktomemlong(x)
	#define memtodiskshort(x)
	#define memtodisklong(x)
#else
	#error "Couldn't determine byte order of target architecture, update osincludes.h"
#endif


#define longswap(foo)	do {foo = dolongswap(foo);} while (0)
#define shortswap(foo)	do {foo = doshortswap(foo);} while (0)


long dolongswap (long);		/* byteorder.c */
short doshortswap (short);


#endif /* byteorderinclude */
