
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

/******************************************************************************
**
**  Project Name:	LDRProjects
**     File Name:	MAF.h
**
**   Description:	Header file for MAF.c
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**	LSR			Larry Rosenstein
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	--------	-----	------	---------------------------------------------
**	09/02/91	20:39	LDR		Moved into separate file
**						LSR		Original version
**
******************************************************************************/

#ifndef SystemSevenOrLater
	#define SystemSevenOrLater 1
#endif

#ifdef MACVERSION

#ifndef __ALIASES__
	#include <aliases.h>
#endif

#ifndef __APPLEEVENTS__
	#include <AppleEvents.h>
#endif

#ifndef __DEVICES__
	#include <Devices.h>
#endif

#ifndef __DIALOGS__
	#include <Dialogs.h>
#endif

#ifndef __ERRORS__
	#include <Errors.h>
#endif

#ifndef __FILES__
	#include <Files.h>
#endif

#ifndef __FINDER__
	#include <finder.h>
#endif

#ifndef __FOLDERS__
	#include <Folders.h>
#endif

#ifndef __FONTS__
	#include <Fonts.h>
#endif

#ifndef __ICONS__
	#include <Icons.h>
#endif

#ifndef __MENUS__
	#include <Menus.h>
#endif

#ifndef __RESOURCES__
	#include <resources.h>
#endif

#ifndef __SEGLOAD__
	#include <SegLoad.h>
#endif

#ifndef __STDIO__
	#include <StdIO.h>
#endif

#ifndef __STRING__
	#include <string.h>
#endif

#ifndef __TYPES__
	#include <types.h>
#endif

#endif

boolean MakeAliasFile (const FSSpec* srcFile, const FSSpec* destFile);



