
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

#include "frontier.h"
#include "standard.h"

#ifdef fldebug

#ifdef MACVERSION

	#include "memory.h"
	#include "strings.h"
	#include "ops.h"
	#include "shell.h"

#if TARGET_API_MAC_CARBON
	#include "file.h"
	#include "launch.h"
	#include "timedate.h"
#endif


#if 0 //TARGET_API_MAC_CARBON	/* 2004-10-23 aradke: rely on Debugging.h instead */

	short __assert (char *expr, char *file, short line) {
	
		/*
		On OS X, DebugStr output gets automatically rerouted to stderr
		which in turn gets logged to console.log, viewable via Console.app.
		
		2003-05-26 AR: Include date/time, app name, and app version.
		*/
		
		static boolean flnorentry = false;
		short day, month, year, hour, minute, second;
		tyfilespec myfspec;
		bigstring bs, bslogstamp, bsline, bsfile, bsmessage;
		
		if (flnorentry)
			return (0);
		
		flnorentry = true;
		
		/*get timestamp*/
		
		secondstodatetime (timenow (), &day, &month, &year, &hour, &minute, &second);
		
		numbertostring ((long) year, bs);
		
		pushstring (bs, bslogstamp);
		
		pushchar ('-', bslogstamp);
		
		numbertostring ((long) month, bs);
		
		padwithzeros (bs, 2);
		
		pushstring (bs, bslogstamp);
		
		pushchar ('-', bslogstamp);
		
		numbertostring ((long) day, bs);
		
		padwithzeros (bs, 2);
		
		pushstring (bs, bslogstamp);
		
		pushchar (' ', bslogstamp);
		
		numbertostring ((long) hour, bs);
		
		padwithzeros (bs, 2);
		
		pushstring (bs, bslogstamp);
		
		pushchar (':', bslogstamp);
		
		numbertostring ((long) minute, bs);
		
		padwithzeros (bs, 2);
		
		pushstring (bs, bslogstamp);
		
		pushchar (':', bslogstamp);
		
		numbertostring ((long) second, bs);
		
		padwithzeros (bs, 2);
		
		pushstring (bs, bslogstamp);
		
		pushchar (' ', bslogstamp);

 		/*get filespec for app*/
 
		getapplicationfilespec (nil, &myfspec);
		
		pushstring (fsname (&myfspec), bslogstamp);
		
		pushchar (' ', bslogstamp);
		
		/*get version of app*/
		
		filegetprogramversion (bs);
		
		pushchar ('(', bslogstamp);
		
		pushstring (bs, bslogstamp);
		
		pushchar (')', bslogstamp);
		
		/*get file name*/
		
		moveleft (file, bsfile, (long) lenbigstring);
		
		convertcstring (bsfile);
		
		/*get line number*/
		
		numbertostring ((long) line, bsline);
		
		/*ouput message*/
		
		parsedialogstring (
				"\p\r^0: Assertion failed in file ^1, at line ^2.\r",
				bslogstamp, bsfile, bsline, nil,
				bsmessage);
				
		DebugStr (bsmessage);
		
		/*send message to stderr*/
		
		flnorentry = false;

		return (0);
	} /*__assert*/

#endif	/*TARGET_API_MAC_CARBON*/

#if TARGET_API_MAC_OS8
		
	short __assert (char *expr, char *file, short line) {

		bigstring bsfile, bsline, bsmessage;
		static boolean flnorentry = false;
		
		if (flnorentry)
			return (0);
		
		flnorentry = true;
		
		moveleft (file, bsfile, (long) lenbigstring);
		
		convertcstring (bsfile);
		
		numbertostring ((long) line, bsline);
		
		parsedialogstring (
				"\pAssertion failed in file ^0, at line ^1.",
				bsfile, bsline, nil, nil,
				bsmessage);
		
		DebugStr (bsmessage);
		
		flnorentry = false;

		return (0);
	} /*__assert*/

#endif	/*TARGET_API_MAC_OS8*/

#endif	/*MACVERSION*/
	

#ifdef WIN95VERSION

	#include "threads.h"
	#include <stdio.h>

	#ifdef _MSC_VER		/* 2002-11-09 AR: use _assert from debug version of Microsoft C runtime library*/

		#include <assert.h>
		#include "threads.h"

		short __assert (char *expr, char *file, short line) {
			
			#ifdef _DEBUG

				static boolean flnorentry = false;
				
				if (flnorentry)
					return (0);
				
				flnorentry = true;

				releasethreadglobals ();
				
				_assert (expr, file, line);

				grabthreadglobals ();

				flnorentry = false;

			#endif	/*_DEBUG*/

			return (0);
			} /*__assert*/

	#endif	/* _MSC_VER */

	#ifdef __MWERKS__		/* 2002-11-09 AR: put up MessageBox with Cancel button to abort application */

		short __assert (char *expr, char *file, short line) {
		
			#ifdef _DEBUG

				static boolean flnorentry = false;
				char buf[4096];
				int itemnum;
				
				if (flnorentry)
					return (0);
				
				flnorentry = true;

				sprintf(buf,"Assertion failed in %s, at line %d.\n\nExpression: %s\n\n", file, line, expr);
				
				//releasethreadglobals ();	/* 2002-11-10 AR: other threads should not continue running if we hit an assertion failure */
				
				itemnum = MessageBox (hwndMDIClient, buf, "Assertion Failure", MB_ABORTRETRYIGNORE | MB_ICONSTOP | MB_APPLMODAL);

				//grabthreadglobals ();

				flnorentry = false;
				
				if (itemnum == IDABORT)
					abort();

				if (itemnum == IDRETRY) {
				
					if (IsDebuggerPresent ())
						OutputDebugString (buf);
					
					DebugBreak();
					}

			#endif	/*_DEBUG*/

			return (0);
			} /*__assert*/

	#endif	/*__MWERKS__*/

#endif	/*WIN95VERSION*/

#endif	/*fldebug*/

