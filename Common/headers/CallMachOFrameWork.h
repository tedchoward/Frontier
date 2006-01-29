
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

#ifndef callmachoframework
#define callmachoframework

/* 2006-01-29 creedon - define the following for CodeWarrior compilation because it doesn't have these defined in its headers, as Xcode does */

#ifdef __MWERKS__
	#define	F_GETFL		3		/* get file status flags */
	#define	F_SETFL		4		/* set file status flags */
	#define	O_NONBLOCK	0x0004	/* no delay */
#endif


extern boolean unixshellcall (Handle hcommand, Handle hreturn);

extern OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr);

extern void *getframeworkfuncptr (CFStringRef framework, CFStringRef functionname);

extern void *convertcfmtomachofuncptr (void *);

extern void disposemachofuncptr (void *);

extern void *convertmachotocfmfuncptr (void *);

extern void disposecfmfuncptr (void *);

#endif
