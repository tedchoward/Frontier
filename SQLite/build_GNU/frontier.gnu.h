
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
	2004-12-22 trt: Precompile prefix header for GNU tools
*/

#ifdef	MACVERSION

#define TARGET_API_MAC_OS8		0	/* see ConditionalMacros.h */
#define TARGET_API_MAC_CARBON	1
#define TARGET_API_MAC_OSX		0

#define FRONTIER_FRAMEWORK_INCLUDES	1
//#define	FRONTIER_FLAT_HEADERS		1

#undef WIN95VERSION

#else

/*
#define WIN32
#define _WIN32
#define _WINDOWS
#define _X86_
#define WINVER 0x0400
*/
#define	_M_IX86
#define _WIN32_WINNT 	0x0400		/* Windows NT 4.0 or later */
#define _WIN32_WINDOWS	0x0410		/* Windows 98 or later */

/* These keywords are built in to MSVC++ */
#ifndef _stdcall
	#define _stdcall __stdcall
#endif
#ifndef _cdecl
	#define _cdecl __cdecl
#endif
#ifndef _asm
	#define _asm asm
#endif
#ifndef _inline
	#define _inline inline
#endif
#ifndef _fastcall
	#define _fastcall __fastcall
#endif

/* these type macros are defined in MSVC++ */
#ifndef __int8
	#define __int8 char
#endif
#ifndef __int16
	#define __int16 short
#endif
#ifndef __int32
	#define __int32 long
#endif
#ifndef __int64
	#define __int64  long long
#endif

/*
 UNICODE (Wide Character) types
*/

//typedef unsigned short wchar_t;    // wc,   16-bit UNICODE character

#undef	MACVERSION
#undef	TARGET_API_MAC_CARBON
#undef	TARGET_RT_MAC_CFM
#undef	TARGET_RT_MAC_MACHO

#endif

#include "frontier.h"
