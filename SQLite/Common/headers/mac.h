
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

#define macinclude


/*constants*/

#define idgestaltselector 'LAND'
#define idgestaltfunction 128


/*typedefs*/

typedef struct tymemoryconfig {
	
	long minstacksize; /*minimum stack space required*/
	
	long minheapsize; /*minimum heap space to run program*/
	
	long avghandlesize; /*to determine # of master blocks; use zero for none*/
	
	long reserveforcode; /*approx. size of not-yet-loaded CODE resources*/
	
	long reserved;
	} tymemoryconfig, **hdlmemoryconfig;


/*globals*/

extern long	gSystemVersion;
extern boolean	gHasColorQD;
extern boolean	gCanUseNavServ;

//extern SysEnvRec macworld; /*the machine environment*/

extern tymemoryconfig macmemoryconfig;


/*prototypes*/

extern boolean initmacintosh (void);

extern short countinitialfiles (void);

extern void getinitialfile (short, bigstring, short *);

extern boolean installgestaltfunction (void);

#ifdef MACVERSION //JES 2004.03.29: MacOS Specific function declarations
	extern void WriteToConsole (char *s);
	extern void DoErrorAlert(OSStatus status, CFStringRef errorFormatString);
#endif

