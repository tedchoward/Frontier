
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

#include "frontier.h"
#include "standard.h"

#include "byteorder.h"

//#if (defined(TARGET_API_MAC_CARBON) && (TARGET_API_MAC_CARBON == 1))
//#include <CoreFoundation/CoreFoundation.h>
//#include <CFByteOrder.h>
//#endif


long dolongswap (long foo) {

	/*
	2006-04-09 aradke: utility function for swapping the byte order
		of 32-bit integers from big endian to little endian and vice versa.
	*/

#if (defined(TARGET_API_MAC_CARBON) && (TARGET_API_MAC_CARBON == 1))

	foo = CFSwapInt32 (foo);

#elif (defined(__i386__) && defined(__GNUC__))

	/*
		10.0a5 - TRT - 29 Dec 2004
		GNU by default uses the AT&T assembly code syntax;
		MSC/MWERKS uses the Intel assembly code syntax;
		some versions of 'gcc' accept -masm=att|intel to
		specify which syntax to use; but for those that don't...
	*/
	__asm__("mov foo,%eax\nbswap %eax\nmov %eax,foo\n");

#elif defined(WIN32)	/* intel x86 assembler code */

	_asm
		{
		mov eax,foo
		bswap eax
		mov foo,eax
		}

#else	/* default implementation using just C operators, should work with all compilers */

	#pragma warning "Using default implementation of dolongswap, edit byteorder.c to implement faster native implementation"

	foo = ((((foo) >> 24) & 0x000000ff)		/* if all else fails, do it the slow way */
			| (((foo) & 0x00ff0000) >> 8)
			| (((foo) & 0x0000ff00) << 8)
			| (((foo) & 0x000000ff) << 24));

#endif

	return (foo);
	} /*dolongswap*/


short doshortswap (short foo) {

	/*
	2006-04-09 aradke: utility function for swapping the byte order
		of 16-bit integers from big endian to little endian and vice versa.
	*/

#if (defined(TARGET_API_MAC_CARBON) && (TARGET_API_MAC_CARBON == 1))

	foo = CFSwapInt16 (foo);

#elif (defined(__i386__) && defined(__GNUC__))

	/*
		10.0a5 - TRT - 29 Dec 2004
		GNU by default uses the AT&T assembly code syntax;
		MSC/MWERKS uses the Intel assembly code syntax;
		some versions of 'gcc' accept -masm=att|intel to
		specify which syntax to use; but for those that don't...
	*/
	__asm__("mov foo,%ax\n mov %al,%bh\nmov %ah,%bl\nmov %bx,foo");

#elif defined(WIN32)	/* intel x86 assembler code */

	_asm
		{
		mov ax,foo
		mov bh,al
		mov bl,ah
		mov foo,bx
		}

#else	/* default implementation using just C operators, should work with all compilers */

	#pragma warning "Using default implementation of doshortswap, edit byteorder.c to implement faster native implementation"

	foo = ((((foo) >> 8) & 0x00ff)		/* if all else fails, do it the slow way */
			| (((foo) << 8) & 0xff00));

#endif

	return (foo);
	} /*doshortswap*/

