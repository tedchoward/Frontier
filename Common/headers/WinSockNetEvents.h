
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

/*************************************************************************

This file contains the Frontier NetEvents interface for the Windows
version.  It uses the WinSock 1.1 specification.  Extensions should be 
added for the 2.0 specification at a later date.  The primary advantage
of the 2.x specification is that more then just the TCP/IP protocol is
supported.  In fact many protocols are supported including the Apple
Share protocol.


Created 7/20/97 Robert Bierman

  General Note:  All addresses are in HOST format.  We convert to network
  format internally.

**************************************************************************/

/* Represent version 1.1 of the WinSock interface */

#define WINSOCK_VERSION_MAJOR 1
#define WINSOCK_VERSION_MINOR 1


extern /*const*/ long maxconnections; /*7.0b37 PBS: referenced by system.environment set-up in langstartup.c*/
										/*7.1b2 RAB: removed const */

/* Initialize the NetEvents system */
//boolean fwsNetEventLaunch (void);

/* Indicate if we are between a fwsNetEventLaunch and a fwsNetEventQuit */
//boolean fwsNetEventIsRunning (void);

/* shut down the NetEvents system */
extern boolean fwsNetEventQuit (void);

/* shut down the NetEvents system */
/* This is for Frontier Internal use ONLY */
extern boolean fwsNetEventShutDown (void);

/* shutdown listeners whose daemon script lives in hdatabase */
extern void fwsNetEventShutdownDependentListeners (long hdatabase);

/* Convert an address (4 byte) into a dotted IP address */
extern boolean fwsNetEventAddressDecode (unsigned long addr, bigstring IPaddr);

/* Convert a dotted IP address into an address (4 byte) */
extern boolean fwsNetEventAddressEncode (bigstring IPaddr, unsigned long  * addr);

/* Convert an address (4 byte) into a domain name */
extern boolean fwsNetEventAddressToName (unsigned long addr, bigstring domainName);

/* Convert a domain name into an address (4 byte) */
extern boolean fwsNetEventNameToAddress (bigstring domainName, unsigned long * addr);

/* Get the hosts address */
extern boolean fwsNetEventMyAddress (unsigned long * addr);


/* Abort a stream and delete associated data */
extern boolean fwsNetEventAbortStream (unsigned long stream);

/* Close a stream and delete associated data */
extern boolean fwsNetEventCloseStream (unsigned long stream);

/* Open a stream and create associated data */
extern boolean fwsNetEventOpenAddrStream (unsigned long addr, unsigned long port, unsigned long * stream);

/* Open a stream and create associated data */
extern boolean fwsNetEventOpenNameStream (bigstring name, unsigned long port, unsigned long * stream);

/* Read from a stream */
extern boolean fwsNetEventReadStream (unsigned long stream, unsigned long * bytesToRead, char * buffer);

/* Write to a Stream */
extern boolean fwsNetEventWriteStream (unsigned long stream, unsigned long bytesToWrite, char * buffer);

/* Set up a listner on a port */
extern boolean fwsNetEventListenStream (unsigned long port, long depth, bigstring callback, unsigned long refcon, unsigned long * stream, unsigned long ipaddr, long hdatabase);

/* get the status of a stream */
extern boolean fwsNetEventStatusStream (unsigned long stream, bigstring status, unsigned long * bytesPending);

#ifdef WIN95VERSION
/* Process an accept pending message on a socket */
extern boolean fwsNetEventAcceptSocket (WPARAM wParam, LPARAM lParam);
#endif

#ifdef MACVERSION
extern boolean fwsNetEventCheckAndAcceptSocket (void);
#endif

/* Close a listen and delete associated data */
extern boolean fwsNetEventCloseListen (unsigned long stream);

/* Get the peers address and port info */
extern boolean fwsNetEventGetPeerAddress (unsigned long stream, unsigned long * peeraddress, unsigned long * peerport);

/* Read from stream until pattern is found or timeout */
extern boolean fwsNetEventReadStreamUntil (unsigned long stream, Handle hbuffer, Handle hpattern, unsigned long timeoutsecs);

/* Read ctbytes bytes from stream or timeout */
extern boolean fwsNetEventReadStreamBytes (unsigned long stream, Handle hbuffer, long ctbytes, unsigned long timeoutsecs);

/* Read from stream until connection is closed or timeout */
extern boolean fwsNetEventReadStreamUntilClosed (unsigned long stream, Handle hbuffer, unsigned long timeoutsecs);

/* Write to stream using a blocking socket, retry until done */
extern boolean fwsNetEventWriteHandleToStream (unsigned long stream, Handle hbuffer, unsigned long chunksize, unsigned long timeoutsecs);

/* Write a file to stream using a blocking socket, retry until done */
//#if 0
extern boolean fwsNetEventWriteFileToStream (unsigned long stream, Handle hprefix, Handle hsuffix, ptrfilespec fs);
//#endif

/* Special read function for inetdsupervisor */
extern boolean fwsNetEventInetdRead (unsigned long stream, Handle hbuffer, unsigned long timeoutsecs);

extern boolean fwsNetEventGetStats (unsigned long stream, bigstring bs);

extern long fwsNetEventGetConnectionCount (void); /*7.0b37 PBS*/
