
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
*/

#ifndef __FRONTIER_H__
#define __FRONTIER_H__


#if !defined(MACVERSION) && !defined(WIN95VERSION)
	#error Either MACVERSION or WIN95VERSION must always be defined!
#endif


#ifdef WIN95VERSION
	#ifndef __MWERKS__
		#pragma warning (disable: 4244 4761; error: 4013)
	#endif
#endif /* WIN95VERSION */


#include "osincludes.h"		/* operating system headers */


#include "frontierdefs.h"	/* global pre-processor defines */


#endif /*__FRONTIER_H__*/
