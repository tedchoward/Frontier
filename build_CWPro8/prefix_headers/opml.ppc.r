
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
	2004-12-11 aradke: Generalized CodeWarrior prefix file for Classic Mac OS
*/

#define PIKE					1
#define OPMLEDITOR				1

#undef	WIN95VERSION

#define	MACVERSION				1

#define TARGET_API_MAC_OS8		1	/* see ConditionalMacros.h */
#define TARGET_API_MAC_CARBON	0
#define TARGET_API_MAC_OSX		0

#undef FRONTIER_FRAMEWORK_INCLUDES
