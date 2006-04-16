
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

#ifndef byteorderinclude
#define byteorderinclude

/*
	2006-04-08 aradke: determine native byte order and define byte swapping macros
*/

#if (defined(WIN32) || defined(__i386) || defined(__i386__)) && !defined(__LITTLE_ENDIAN__)
	#define __LITTLE_ENDIAN__	1
#endif


#if defined(__LITTLE_ENDIAN__) && ((__LITTLE_ENDIAN__ == 1) || !defined (__BIG_ENDIAN__))

	#define SWAP_BYTE_ORDER	1

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

	#undef SWAP_BYTE_ORDER

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


/*
	2006-04-16 aradke: Carbon resource manager resources are in big endian format.
		If we are running on an Intel Mac, we need to perform byte order conversion.
*/

#if (defined(TARGET_API_MAC_CARBON) && (TARGET_API_MAC_CARBON == 1) && defined(SWAP_BYTE_ORDER))

	#define SWAP_REZ_BYTE_ORDER	1

	#define reztomemshort(x)	shortswap(x)
	#define reztomemlong(x)		longswap(x)
	#define reztomemrect(x)		do {shortswap((x).top); shortswap((x).left); shortswap((x).bottom); shortswap((x).right);} while(0)
	#define memtorezshort(x)	shortswap(x)
	#define memtorezlong(x)		longswap(x)
	#define memtorezrect(x)		do {shortswap((x).top); shortswap((x).left); shortswap((x).bottom); shortswap((x).right);} while(0)

#else

	#undef SWAP_REZ_BYTE_ORDER

	#define reztomemshort(x)
	#define reztomemlong(x)
	#define reztomemrect(x)
	#define memtorezshort(x)
	#define memtorezlong(x)
	#define memtorezrect(x)

#endif


#define longswap(foo)	do {foo = dolongswap(foo);} while (0)
#define shortswap(foo)	do {foo = doshortswap(foo);} while (0)


#if (defined(TARGET_API_MAC_CARBON) && (TARGET_API_MAC_CARBON == 1) && defined(__GNUC__))

	/* using system sdk functions from CFByteOrder.h */

	#define dolongswap(foo)			CFSwapInt32(foo)
	#define doshortswap(foo)		CFSwapInt16(foo)
	
#elif (defined(__i386__) && defined(__GNUC__))

	/* using AT&T x86 assembly code syntax */

	inline long dolongswap (long foo) {

		__asm__("mov foo,%eax\nbswap %eax\nmov %eax,foo\n");

		return (foo);
		} /*dolongswap*/

	inline short doshortswap (short foo) {

		__asm__("mov foo,%ax\n mov %al,%bh\nmov %ah,%bl\nmov %bx,foo");

		return (foo);
		} /*doshortswap*/

#elif defined(WIN32)

	/* using Intel x86 assembly code syntax */
	
	__inline long dolongswap (long foo) {

		_asm
			{
			mov eax,foo
			bswap eax
			mov foo,eax
			}

		return (foo);
		} /*dolongswap*/

	__inline short doshortswap (short foo) {

		_asm
			{
			mov ax,foo
			mov bh,al
			mov bl,ah
			mov foo,bx
			}
		
		return (foo);
		} /*doshortswap*/

#else

	/* portable code using only C operators */
	
	inline long dolongswap (long foo) {

		foo = ((((foo) >> 24) & 0x000000ff)
				| (((foo) & 0x00ff0000) >> 8)
				| (((foo) & 0x0000ff00) << 8)
				| (((foo) & 0x000000ff) << 24));
		
		return (foo);
		} /*dolongswap*/

	inline short doshortswap (short foo) {

		foo = ((((foo) >> 8) & 0x00ff)
				| (((foo) << 8) & 0xff00));

		return (foo);
		} /*doshortswap*/

#endif


#endif /* byteorderinclude */
