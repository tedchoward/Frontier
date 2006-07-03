
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

/* file:	IPCDriver.h
-- desc:	Public header file for IPCDriver
-- version:	V1.0
-- by:		Don Park
-- when:	November 1988
-- hist:	30Nov88 V1.0 DDP	New.
*/

#ifdef GLOBALSOK
#define	_IPCDrvrName		"\p.IPC Manager © 1988 UserLand"
#define	_IPCFileName		"\pIPC Manager"
#endif

/* Control selector codes understood by IPC driver */
enum {
	csOpnIPCPrc = 256,	/* register a named Process */
	csClsIPCPrc,		/* unregister a Process */
	csFndIPCPrc,		/* find a named Process */
	csWhoIPCPrc,		/* return name of a Process */
	csSndIPCMsg,		/* send a Message from a Process to another Process */
	csRcvIPCMsg,		/* receive a Message from any Process to a Process */
	csClrIPCMsg			/* flush all Messages sent to a Process */
};

/* Structure passed through csParam field of CntrlParam block when
-- a Control code is sent.
*/
typedef struct	{		/* Process Parameter Block */
	PIN*	pin;		/* Process Name */
	PID		pid;		/* Process ID */
	PID		sender;		/* Sender Process ID */
	PID		receiver;	/* Receiver Process ID */
	Handle	message;	/* Message handle */
} _IPCParam;

