
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

#ifndef threadsinclude
#define threadsinclude


#define	idnullthread		((hdlthread) 0)
#define	idcurrentthread		((hdlthread) 1)
#define	idapplicationthread	((hdlthread) 2)


typedef struct _thread * hdlthread;

typedef void * tythreadmainparams;

typedef pascal void * (*tythreadmaincallback) (tythreadmainparams);

typedef void (*tythreadglobalscallback) (void *);

#pragma pack(2)
typedef struct tythreadcallbacks {

	tythreadglobalscallback disposecallback;
	
	tythreadglobalscallback swapincallback;
	
	tythreadglobalscallback swapoutcallback;
	} tythreadcallbacks;
#pragma options align=reset

/*globals*/

extern tythreadcallbacks threadcallbacks;


/*prototypes*/

extern boolean canusethreads (void);

extern boolean initmainthread (void *);

extern boolean inmainthread (void);

extern boolean attachtomainthread (long); /*6.2b7 AR*/

extern boolean newthread (tythreadmaincallback, tythreadmainparams, void *, hdlthread *);

extern boolean threadstartup (void);

extern void threadshutdown (void);

extern boolean threadsleep (hdlthread hthread);

extern boolean threadissleeping (hdlthread);

extern boolean threadwake (hdlthread, boolean);

extern boolean threadiswaiting (void);

extern boolean threadyield (boolean);

extern long grabthreadglobals (void);

extern long releasethreadglobals (void);

extern long grabthreadglobalsnopriority (void);

extern long releasethreadglobalsnopriority (void);

extern boolean initthreads (void);

#ifdef fldebug
	extern void checkthreadglobals (void);
#else
	#define checkthreadglobals() ((void *)0)
#endif


#endif
