
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
	2004-10-26 aradke: New header file, to be included from all *.r files.
*/

#ifndef __FRONTIER_R__
#define __FRONTIER_R__


#if TARGET_API_MAC_CARBON
	#ifdef FRONTIER_FRAMEWORK_INCLUDES
		#include <Carbon/Carbon.r>
	#else
		#include <Carbon.r>
	#endif
#else
	#include <MacTypes.r>
	#include <AEUserTermTypes.r>
	#include <Controls.r>
	#include <Dialogs.r>
	#include <Icons.r>
	#include <Menus.r>
#endif


#include "frontierdefs.h"	/* global pre-processor defines */

#include "config.r"
#include "versions.h"


#endif /*__FRONTIER_R__*/
