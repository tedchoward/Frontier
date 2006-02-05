
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
	2004-10-26 aradke: New global header file, to be included from frontier.h and frontier.r.
*/

#ifndef __FRONTIERDEFS_H__
#define __FRONTIERDEFS_H__


#ifdef WIN95VERSION
	#define PACKFLIPPED		/* enable little endian / big endian conversion for database file functions */
	#define FRONTIERCOM 1
#ifndef OPMLEDITOR
	#define FRONTIERWEB 0
#else //OPMLEDITOR
	#define FRONTIERWEB 1	/* 2006-02-05 aradke: enable HTML display in main window background */
#endif // OPMLEDITOR
	#undef winhybrid
	#define fljustpacking 0
	#undef flcomponent
	#define gray3Dlook 1
	#define noextended 1
	#define NEWFILESPECTYPE 1	
#endif /* WIN95VERSION */


#ifdef MACVERSION
	#define flcomponent 1
	#ifdef __powerc
		#define noextended 1
	#elif defined(__GNUC__)
		#define noextended 1
	#else
		#define noextended 0
	#endif
#endif /* MACVERSION */


#undef MEMTRACKER		/* define as 1 to enable tracking of memory allocations */
#undef fltrialsize		/* define as 1 to build trial version with expiration logic */
#undef DATABASE_DEBUG	/* define as 1 to enable database debugging and logging code */


#undef flruntime
#define fldebug 1
#define flnewfeatures 1
#define version42orgreater 1
#define version5orgreater 1
#define isFrontier 1
#undef dropletcomponent
#undef fliowa
#define threadverbs 1
#define oplanglists 1
#define flregexpverbs 1
#undef lazythis_optimization
#undef langexternalfind_optimization
#define PASCALSTRINGVERSION 1
#define SPEED 1
#undef fltracklocaladdresses		/*2004-12-08 aradke: disable [buggy] code for tracking deleted local addresses*/


#ifdef MACVERSION
	#define macBirdRuntime	1
	#undef appRunsCards			/*for Applet Toolkit, Iowa Runtime is baked in*/
	#define iowaRuntimeInApp	/*iowa code knows it's in an app*/
	#define iowaRuntime			/*iowa code knows it's not compiling in Card Editor*/
	#define cmdPeriodKillsCard
	#define IOAinsideApp		/*all the IOA's are baked into the app*/
	#undef coderesource			/*we're not running inside a code resource*/
#endif

#ifdef WIN95VERSION
	#undef macBirdRuntime
	#undef appRunsCards			/*for Applet Toolkit, Iowa Runtime is baked in*/
	#undef iowaRuntimeInApp		/*iowa code knows it's in an app*/
	#undef iowaRuntime			/*iowa code knows it's not compiling in Card Editor*/
	#undef cmdPeriodKillsCard
	#undef IOAinsideApp			/*all the IOA's are baked into the app*/
	#undef coderesource			/*we're not running inside a code resource*/
#endif


#define Rez true
#define DeRez false


#define SystemSevenOrLater 1


#endif /*__FRONTIERDEFS_H__*/
