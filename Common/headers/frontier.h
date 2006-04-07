
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
	2004-10-23 aradke: New global header file, to be included from all source files.

	2006-03-04 aradke: disable MS Visual C++ warning about unknown pragmas
	so it won't complain about "#pragma unused(foo)"
*/

#ifndef __FRONTIER_H__
#define __FRONTIER_H__


/* sanity check of prefix headers and build environment */

#if !defined(MACVERSION) && !defined(WIN95VERSION)
	#error Either MACVERSION or WIN95VERSION must always be defined!
#endif

#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
	#error Either __LITTLE_ENDIAN__ or __BIG_ENDIAN__ must be defined!
#endif


#ifdef WIN95VERSION
	#if (defined(_MSC_VER) && !defined(__MWERKS__))
		/*
		2006-03-04 aradke: Key to MS Visual C++ warning numbers
			4244: conversion from [e.g.] 'long' to 'short', possible loss of data
			4761: integral size mismatch in argument; conversion supplied
			4068: unknown pragma
			4013: 'fooBarFunction' undefined; assuming extern returning int
		*/
		#pragma warning (disable: 4244 4761 4068; error: 4013)
	#endif
#endif /* WIN95VERSION */


#include "osincludes.h"		/* operating system headers */


#include "frontierdefs.h"	/* global pre-processor defines */


#endif /*__FRONTIER_H__*/
