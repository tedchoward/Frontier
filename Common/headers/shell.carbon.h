
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

#ifndef shell_werks_include
#define shell_werks_include
#ifdef __MWERKS__
	#if __INTEL__
		#define WIN95VERSION
	#else
		#define MACVERSION 1
	#endif
#endif

#ifdef MACVERSION
	#ifndef THINK_C
		#include "carbonheaders.h"
		#pragma once on
		#pragma syspath_once on
	#endif
#endif

#ifdef WIN95VERSION
	#include <Win32Headers>
	#define _DEBUG
	#define _WINDOWS
	#define _X86_
	#define _WIN32
#endif

#undef fltrialsize
#undef flruntime
#define fldebug 1
#define flnewfeatures 1
#define version42orgreater 1
#define version5orgreater 1
#define flcomponent 1
#define isFrontier 1
#undef dropletcomponent
#undef fliowa
#define threadverbs 1
#define oplanglists 1
#define flregexpverbs 1

#ifdef __powerc
	#define noextended 1
#else
	#define noextended 0
#endif

//#define macBirdRuntime 0
//#undef appRunsCards /*for Applet Toolkit, Iowa Runtime is baked in*/
//#define iowaRuntimeInApp /*iowa code knows it's in an app*/
//#define iowaRuntime /*iowa code knows it's not compiling in Card Editor*/
//#define cmdPeriodKillsCard
//#define IOAinsideApp /*all the IOA's are baked into the app*/
#undef coderesource /*we're not running inside a code resource*/

#define Rez true
#define DeRez false

#undef SystemSevenOrLater
#define SystemSevenOrLater 1

#endif
