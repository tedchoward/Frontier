
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

#define WIN32
#define _WIN32
#define _WINDOWS
#define _X86_

#define _WIN32_WINNT 	0x0400		/* Windows NT 4.0 or later */
#define _WIN32_WINDOWS	0x0410		/* Windows 98 or later */

#define WIN95VERSION 1

#define PACKFLIPPED		/* enable little endian / big endian conversion for database file functions */

#undef PIKE				/* define as 1 to build Radio.exe (previously known as Pike) */

#define FRONTIERCOM 1
#define FRONTIERWEB 0

#undef MEMTRACKER		/* define as 1 to enable tracking of memory allocations */
#undef fltrialsize		/* define as 1 to build trial version with expiration logic */
#undef DATABASE_DEBUG	/* define as 1 to enable database debugging and logging code */


#undef lazythis_optimization
#undef langexternalfind_optimization

#undef winhybrid
#define fljustpacking 0
#undef flruntime
#define fldebug 1
#define flnewfeatures 1
#define version42orgreater 1
#define version5orgreater 1
#undef flcomponent
#define isFrontier 1
#undef dropletcomponent
#undef fliowa
#define threadverbs 1
#define oplanglists 1
#define flregexpverbs 1
#define gray3Dlook 1
#define noextended 1

#undef macBirdRuntime
#undef appRunsCards /*for Applet Toolkit, Iowa Runtime is baked in*/
#undef iowaRuntimeInApp /*iowa code knows it's in an app*/
#undef iowaRuntime /*iowa code knows it's not compiling in Card Editor*/
#undef cmdPeriodKillsCard
#undef IOAinsideApp /*all the IOA's are baked into the app*/
#undef coderesource /*we're not running inside a code resource*/

#define Rez true
#define DeRez false
#define SystemSevenOrLater 1

