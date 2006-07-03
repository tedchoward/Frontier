
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

/* file:		IPCLib.c
-- desc:		Implements Interface to the IPC driver.
-- version:		V1.0
-- by:			Don Park
-- when:		November 1988

-- implementation notes:
--
-- 1.	Only Macintosh applications are allowed to be a Process.  A Process is
-- 		identified by an unformatted Pascal String and by a unique PID (Process
-- 		ID).  PID is allocated during runtime.
-- 2.	To allow variable length Messages, Handles are used to store Messages.
-- 3.	All Message handles are initially allocated in the source application
-- 		heapzone and passed to a device driver named ".IPC" which will create
-- 		a copy in System heap.  System heap will expand and contract so there
-- 		is no danger in congesting the System heap with message handles.
-- 4.	Message handles can be deallocated right after calling ipcSend() or it
-- 		can be kept around and reused later.
-- 5.	IPC Manager routines should NOT be called from Completion Routines, VBL
--		tasks, Time Manager tasks, and Interrupt Service Routines.  Even if the
--		application global area access is controled via imbedding register A5,
--		ipcReceive relies on the current application heapzone to be that of the
--		receiver's.  Other IPC routines are currently safe but this might change.
--
-- terms:	Following terms apply only within the context of IPC communication.
--
-- 		Process:	an application identified by a unique Pascal string.  No
-- 					desk accessory, driver, or code resources are allowed.
-- 		Message:	a packet of unformatted data stored in a handle.
--
-- data types:
--
-- 		PIN:	Process Identifier Name (Pascal String)
-- 		PID:	Process ID (long)
--
-- routines:
--
-- 		ipcOpen()		- Register a Process using its PIN and get its PID
-- 		ipcClose()		- Unregister a Process using its PID.
-- 		ipcFind()		- Find a Process using its PIN.
-- 		ipcWho()		- Return a Process's name.
--
-- 		ipcSend()		- Send a Message Handle.
-- 		ipcReceive()	- Receive a Message Handle.
-- 		ipcClear()		- Flush all Messages sent to a specific PID.
-- 		ipcError()		- Return current error code.
--
-- see:		Inside Switcher (on accessing another application's heapzone)
-- hist:	30Nov88 V1.0 DDP	New.
*/

#include <land.h>

#ifdef MPWC

#include <devices.h>

#endif

#include "landtransport.h"
#include "ipcdriver.h"

/*------------------------- PRIVATE Structures -------------------------------*/
/*--------------------------- PRIVATE Globals --------------------------------*/
#ifdef GLOBALSOK
OSErr	_ipcErr;		/* last IPC error code */
#endif


#define	_ipcRefNum	(**(landgetglobals ())).ipcrefnum		/* IPC driver's refnum */


pascal char *landgetipcdrivername (void);

/*------------------------- Function Prototypes ------------------------------*/

static OSErr	_ipcControl			( short, _IPCParam* );

/*---------------------------- PUBLIC Routines ------------------------------*/

/* name:	ipcOpen()
-- desc:	Register a named Process, installing the IPC driver if is not open
-- 			already.
-- impl:	Locate the IPC driver and remember its refnum.
-- 			Send csOpnIPCPrc control call to IPC driver.
-- 			Return process's PID.
*/
	OSErr				/* <- Error Code */
ipcOpen (
	PIN*	pin,		/* -> Name of Client Process Name (Pascal string) */
	PID*	_pid)		/* <- New Client Process ID */
{
	register OSErr	err;		/* error code */
	_IPCParam		ipcPB;		/* IPC Parameter Block for _ipcControl() */
	short			refnum;

	/* Locate the IPC driver and remember its refnum */
	if (!(err = OpenDriver((ConstStr255Param)landgetipcdrivername(), &refnum)))	{
		
		(**(landgetglobals ())).ipcrefnum = refnum;

		/* send csOpnIPCPrc control call to IPC driver to register self */
		ipcPB.pin = pin;
		if (!(err = _ipcControl(csOpnIPCPrc, &ipcPB)))
			if (_pid)				/* return process's PID */
				*_pid = ipcPB.pid;
	}
	
#ifdef GLOBALSOK
	return (_ipcErr = err);
#else
	return (err);
#endif
}

/* name:	ipcClose()
-- desc:	Close a Process and remove its name from the IPC process table.
-- impl:	Call IPC driver with csClsIPCPrc control code.
*/
	OSErr				/* <- Error Code */
ipcClose (
	PID		pid)		/* -> PID of Process to be closed */
{
	_IPCParam	ipcPB;		/* IPC Parameter Block for _ipcControl() */

	ipcPB.pid = pid;

#ifdef GLOBALSOK
	return (_ipcErr = _ipcControl(csClsIPCPrc, &ipcPB));
#else
	 return (_ipcControl(csClsIPCPrc, &ipcPB));
#endif
}

/* name:	ipcFind()
-- 			Find a named Process and return its PID.
-- impl:	Call IPC driver with csFndIPCPrc control code.
*/
	OSErr				/* <- Error Code */
ipcFind (
	PIN*	pin,		/* -> Name of Target Process Name (Pascal string) */
	PID*	_pid)		/* <- Target Process ID */
{
	OSErr		err;		/* error code */
	_IPCParam	ipcPB;		/* IPC Parameter Block for _ipcControl() */

	ipcPB.pin = pin;
	if (!(err = _ipcControl(csFndIPCPrc, &ipcPB)) && _pid)
		*_pid = ipcPB.pid;

#ifdef GLOBALSOK
	return (_ipcErr = err);
#else
	return (err);
#endif
}

/* name:	ipcWho()
-- desc:	Return a Process's name.
-- impl:	Call IPC driver with csFndIPCPrc control code.
*/
	OSErr				/* <- Error Code */
ipcWho (
	PID		pid,		/* -> Process ID */
	PIN*	_pin)		/* <- Name of Target Process Name (Pascal string) */
{
	_IPCParam	ipcPB;		/* IPC Parameter Block for _ipcControl() */

	ipcPB.pid = pid;
	ipcPB.pin = _pin;

#ifdef	GLOBALSOK
	return (_ipcErr = _ipcControl(csWhoIPCPrc, &ipcPB));
#else
	return (_ipcControl(csWhoIPCPrc, &ipcPB));
#endif
}

/* name:	ipcSend()
-- desc:	Send a Message handle to another Process.
-- note:	Caller is responsible for allocating the Message handle.  Allocate
-- 				it in your application heap.  The handle can be deallocated using
-- 				DisposHandle() when this function returns.
-- impl:	Setup _IPCParam fields
-- 				Call csSndIPCMsg to the IPC driver
*/
	OSErr				/* <- Error Code */
ipcSend (
	PID		sender,		/* -> PID of Process sending Message handle */
	PID		receiver,	/* -> PID of Process receiving Message handle */
	Handle	message)	/* -> Message handle (can not be 0L) */
{
	_IPCParam	ipcPB;		/* IPC Parameter Block for _ipcControl() */

	ipcPB.sender = sender;
	ipcPB.receiver = receiver;
	ipcPB.message = message;

#ifdef GLOBALSOK
	return (_ipcErr = _ipcControl(csSndIPCMsg, &ipcPB));
#else
	return (_ipcControl(csSndIPCMsg, &ipcPB));
#endif
}

/* name:	ipcReceive()
-- desc:	Receive a Message handle from another Process.
-- note:	Receiver is responsible for deallocating the Message handle.
-- 			If returned message handle is NIL then nothing was received.  
-- impl:	Setup _IPCParam fields
-- 			Call csRcvIPCMsg to the IPC driver
*/
	OSErr				/* <- Error Code */
ipcReceive (
	PID		receiver,	/* -> PID of Process receiving Message handle */
	PID*	_sender,	/* <- PID of Process sending Message handle */
	Handle*	_msg)		/* <- Message handle (can not be 0L) */
{
	OSErr		err;		/* error code */
	_IPCParam	ipcPB;		/* IPC Parameter Block for _ipcControl() */

	ipcPB.receiver = receiver;
	if (!(err = _ipcControl(csRcvIPCMsg, &ipcPB)))	{
		if (_sender)
			*_sender = ipcPB.sender;
		if (_msg)
			*_msg = ipcPB.message;
	}
#ifdef GLOBALSOK
	return (_ipcErr = err);
#else
	return (err);
#endif
}

/* name:	ipcClear()
-- desc:	Clear all Messages sent to a Process.
-- note:	Use this routine only to flush Messages sent to your own Process.
-- 			Do NOT pass other Process's PID.
-- impl:	Setup _IPCParam fields
-- 			Call csClrIPCMsg to the IPC driver
*/
	OSErr				/* <- Error Code */
ipcClear (
	PID		receiver)	/* -> PID of Process receiving Message handle */
{
	_IPCParam	ipcPB;		/* IPC Parameter Block for _ipcControl() */

	ipcPB.receiver = receiver;

#ifdef GLOBALSOK
	return (_ipcErr = _ipcControl(csClrIPCMsg, &ipcPB));
#else
	return (_ipcControl(csClrIPCMsg, &ipcPB));
#endif
}


#ifdef GLOBALSOK
/* name:	ipcError()
-- desc:	Return current error status for Dave.
-- note:	Dave, although you might prefer obtaining error codes using a
-- 			separate function, some programmers might want it returned from the
-- 			function that caused the error.  I have decided to return the error
-- 			code, because you can just ignore the returned error code or use it
-- 			as Boolean value which is TRUE if an error occured.  Don't worry,
-- 			adding '!' to each function call does not add any extra code in
-- 			both THINK C and in MPW C.
-- impl:	return error code from local global _ipcErr.
*/
	OSErr				/* <- Error Code */
ipcError ( )
{
	return _ipcErr;
}
#endif

/*--------------------------- PRIVATE Routines -------------------------------*/

/* name:	_ipcControl()
-- desc:	Send a control code and a _IPCParam to IPC driver
-- impl:	set up CntrlParam fields.
-- 			if there is a _IPCParam block to be sent, copy it into CntrlParam.
-- 			call PBControl to IPC driver.
-- 			if no error and there is a _IPCParam block, copy result into it.
*/
	OSErr					/* <- Error Code */
_ipcControl (
	short		csCode,		/* -> control selector code */
	_IPCParam*	ipcParam)	/* -> IPC Parameter Block */
{
	OSErr		err;		/* error code from PBControl() */
	CntrlParam	ctrlPB;		/* Parameter Block for PBControl() call */

	ctrlPB.ioVRefNum = 0;			/* no drive number */

#ifdef FOOBAR	
#ifdef THINKC

	ctrlPB.ioRefNum = _ipcRefNum;	/* IPC driver refnum */
	
	ctrlPB.csCode = csCode;			/* control selector code */
	if (ipcParam)
		BlockMove((Ptr) ipcParam, (Ptr) &ctrlPB.csParam, sizeof(_IPCParam));
	if (!(err = PBControl((Ptr) &ctrlPB, false)) && ipcParam)
		BlockMove((Ptr) &ctrlPB.csParam, (Ptr) ipcParam, sizeof(_IPCParam));
#endif
#endif

#ifndef MPWCX

	ctrlPB.ioCRefNum = _ipcRefNum; /* IPC driver refnum */
	
	ctrlPB.csCode = csCode;			/* control selector code */
	
	if (ipcParam)
		BlockMove((Ptr) ipcParam, (Ptr) ctrlPB.csParam, sizeof(_IPCParam));
	
	err = PBControl((ParmBlkPtr) &ctrlPB, false);
	
	if (!(err) && ipcParam)
		BlockMove((Ptr) ctrlPB.csParam, (Ptr) ipcParam, sizeof(_IPCParam));
#endif

	return err;
}

