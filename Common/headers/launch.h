
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

#define launchinclude

#ifndef shelltypesinclude

	#include "shelltypes.h"

#endif

/*typedefs*/

#pragma pack(2)
typedef struct tylaunchcallbacks {
	
	callback waitcallback; /*called while waiting for process manager change to take effect*/
	} tylaunchcallbacks;
#pragma options align=reset


/*globals*/

extern tylaunchcallbacks launchcallbacks;


/*prototypes*/

extern boolean launchapplication (const ptrfilespec, const ptrfilespec, boolean flbringtofront);

extern boolean findrunningapplication (OSType *, bigstring, typrocessid *);

extern boolean activateapplication (bigstring);

extern boolean getfrontapplication (bigstring, boolean);

extern short countapplications (void);

extern boolean getnthapplication (short, bigstring);

extern boolean getapplicationfilespec ( bigstring, ptrfilespec );

extern boolean executeresource (ResType, short, bigstring);


extern OSType getprocesscreator (void);

extern typrocessid getcurrentprocessid (void);

extern boolean iscurrentapplication (typrocessid);

extern boolean isfrontapplication (typrocessid);

extern boolean activateapplicationwindow (typrocessid, WindowPtr);




