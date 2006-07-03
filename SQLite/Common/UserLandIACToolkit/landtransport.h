
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

/* file:	ipc.h
-- desc:	Public header file for IPCLib
-- version:	V1.0
-- by:		Don Park
-- when:	November 1988
-- hist:	30Nov88 V1.0 DDP	New.
*/

typedef Str255	PIN;	/* Process Name */
typedef	short	PID;	/* Process ID */

/* IPC specific error codes:
-- ipcErrBase:		Start of IPC error code range (expected to be negative).
-- ipcNSProcErr:	Specified Process (either by PID or PIN) is currently NOT
--					open.
-- ipcFullErr:		Too many IPC processes are open.
-- ipcBadPIDErr:	Given PID is NOT a valid PID.
-- ipcBadPINErr:	Given PIN is NOT a valid PIN.
-- ipcBadMsgErr:	Given Message is NOT a valid Message.
*/
#define	ipcErrBase		-2000
#define	ipcNSProcErr	(ipcErrBase - 0)
#define	ipcFullErr		(ipcErrBase - 1)
#define	ipcBadPIDErr	(ipcErrBase - 2)
#define	ipcBadPINErr	(ipcErrBase - 3)
#define	ipcBadMsgErr	(ipcErrBase - 4)

extern OSErr	ipcOpen			( PIN*, PID* );
extern OSErr	ipcClose		( PID );
extern OSErr	ipcFind			( PIN*, PID* );
extern OSErr	ipcWho			( PID, PIN* );

extern OSErr	ipcSend			( PID, PID, Handle );
extern OSErr	ipcReceive		( PID, PID*, Handle* );
extern OSErr	ipcClear		( PID );

#ifdef	GLOBALSOK
extern OSErr	ipcError		( void );
#endif