
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

#define opentrypointsinclude

/*
function prototypes for op.c routines that like to be linked into callback 
structures.  these are in their own .h file because some files only need to
have these names defined and don't need any detailed knowledge of op.h's
data structures.
*/

extern boolean opupdate (void);

extern boolean opactivate (boolean);

extern boolean opidle (void);

extern boolean opsetcursor (void);

extern boolean opgetscrollbarinfo (void);

extern boolean opresetscrollbars (void);

extern boolean opscroll (tydirection, boolean, int);

extern boolean opscrollto (boolean, int);

extern boolean opsetselectioninfo (void);

extern boolean opmousedown (Point);

extern boolean opkeystroke (void);

extern boolean opcmdkeyfilter (char);

extern boolean opundo (void);

extern boolean opcut (void);

extern boolean opcopy (void);

extern boolean oppaste (void);

extern boolean opclear (void);

extern boolean opselectall (void);

extern boolean opreadscrap (void);

extern boolean opwritescrap (void);

extern opdisposeoutline (hdloutlinerecord, boolean);
