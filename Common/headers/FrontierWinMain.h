
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

#ifndef frontierwinmaininclude
#define frontierwinmaininclude

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#define wm_destroycaret WM_USER+300

#define wm_processAccept WM_USER+301

extern CHAR sz_frontierstartreplace [];
extern CHAR sz_frontierstartsearch [];

extern UINT wm_startreplace;
extern UINT wm_startsearch;

extern boolean openwindowsparamlinefiles (void);

EventAvail (unsigned short mask, EventRecord * ev);

#if (FRONTIERCOM == 1)
	extern Handle COMStartup ();
	extern boolean COMShutdown();
	extern Handle COMSYSModule();
#endif

#endif
