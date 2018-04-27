
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


#include "frontier.h"
#include "standard.h"


#ifdef NeverDefine_For_Reference
For reference I am listing the error codes from the windows winsock.h file here

/*
 * All Windows Sockets error constants are biased by WSABASEERR from
 * the "normal"
 */
#define WSABASEERR              10000
/*
 * Windows Sockets definitions of regular Microsoft C error constants
 */
#define WSAEINTR                (WSABASEERR+4)
#define WSAEBADF                (WSABASEERR+9)
#define WSAEACCES               (WSABASEERR+13)
#define WSAEFAULT               (WSABASEERR+14)
#define WSAEINVAL               (WSABASEERR+22)
#define WSAEMFILE               (WSABASEERR+24)

/*
 * Windows Sockets definitions of regular Berkeley error constants
 */
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAEDESTADDRREQ         (WSABASEERR+39)
#define WSAEMSGSIZE             (WSABASEERR+40)
#define WSAEPROTOTYPE           (WSABASEERR+41)
#define WSAENOPROTOOPT          (WSABASEERR+42)
#define WSAEPROTONOSUPPORT      (WSABASEERR+43)
#define WSAESOCKTNOSUPPORT      (WSABASEERR+44)
#define WSAEOPNOTSUPP           (WSABASEERR+45)
#define WSAEPFNOSUPPORT         (WSABASEERR+46)
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)

#define WSAEDISCON              (WSABASEERR+101)

/*
 * Extended Windows Sockets error constant definitions
 */
#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (when using the resolver). Note that these errors are
 * retrieved via WSAGetLastError() and must therefore follow
 * the rules for avoiding clashes with error numbers from
 * specific implementations or language run-time systems.
 * For this reason the codes are based at WSABASEERR+1001.
 * Note also that [WSA]NO_ADDRESS is defined only for
 * compatibility purposes.
 */

#define h_errno         WSAGetLastError()

/* Authoritative Answer: Host not found */
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND

/* Non-Authoritative: Host not found, or SERVERFAIL */
#define WSATRY_AGAIN            (WSABASEERR+1002)
#define TRY_AGAIN               WSATRY_AGAIN

/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define WSANO_RECOVERY          (WSABASEERR+1003)
#define NO_RECOVERY             WSANO_RECOVERY

/* Valid name, no data record of requested type */
#define WSANO_DATA              (WSABASEERR+1004)
#define NO_DATA                 WSANO_DATA

/* no address, look for MX record */
#define WSANO_ADDRESS           WSANO_DATA
#define NO_ADDRESS              WSANO_ADDRESS

#endif
//This ends the Reference Section


static unsigned char * tcperrorstrings [80] = {
	BIGSTRING (""),
	BIGSTRING ("\x1b" "Host not found. (DNS error)"),					/* 1 */
	BIGSTRING ("\x37" "Non-authoritative host not found. (Temporary DNS error)"),	/* 2 */
	BIGSTRING ("\x22" "Non-recoverable error. (DNS error)"),						/* 3 */
	BIGSTRING ("\x39" "Valid name, no data record of requested type. (DNS error)"),	/* 4 */
	BIGSTRING ("\x12" "Input/output error"),				/* 5 */
	BIGSTRING ("\x15" "Device not configured"),			/* 6 */
	BIGSTRING ("\x16" "Argument list too long"),			/* 7 */
	BIGSTRING ("\x11" "Exec format error"),				/* 8 */
	BIGSTRING ("\x13" "Bad file descriptor"),			/* 9 */
	BIGSTRING ("\x12" "No child processes"),				/* 10 */
	BIGSTRING ("\x19" "Resource deadlock avoided"),		/* 11 */
	BIGSTRING ("\x16" "Cannot allocate memory"),			/* 12 */
	BIGSTRING ("\x11" "Permission denied"),				/* 13 */
	BIGSTRING ("\x0b" "Bad address"),					/* 14 */
	BIGSTRING ("\x15" "Block device required"),			/* 15 */
	BIGSTRING ("\x0b" "Device busy"),					/* 16 */
	BIGSTRING ("\x0b" "File exists"),					/* 17 */
	BIGSTRING ("\x11" "Cross-device link"),				/* 18 */
	BIGSTRING ("\x21" "Operation not supported by device"),	/* 19 */
	BIGSTRING ("\x0f" "Not a directory"),				/* 20 */
	BIGSTRING ("\x0e" "Is a directory"),					/* 21 */
	BIGSTRING ("\x10" "Invalid argument"),				/* 22 */
	BIGSTRING ("\x1d" "Too many open files in system"),	/* 23 */
	BIGSTRING ("\x15" "Too many open sockets"),			/* 24 */
	BIGSTRING ("\x1e" "Inappropriate ioctl for device"),	/* 25 */
	BIGSTRING ("\x0e" "Text file busy"),					/* 26 */
	BIGSTRING ("\x0e" "File too large"),					/* 27 */
	BIGSTRING ("\x17" "No space left on device"),		/* 28 */
	BIGSTRING ("\x0d" "Illegal seek"),					/* 29 */
	BIGSTRING ("\x15" "Read-only file system"),			/* 30 */
	BIGSTRING ("\x0e" "Too many links"),					/* 31 */
	BIGSTRING ("\x0b" "Broken pipe"),					/* 32 */
	BIGSTRING (""),
	BIGSTRING (""),

/* non-blocking and interrupt i/o */
	BIGSTRING ("\x20" "Resource temporarily unavailable"),	/* 35 */

/* ipc/network software -- argument errors */
	BIGSTRING ("\x23" "A blocking operation is in progress"),			/* 36 */
	BIGSTRING ("\x20" "Operation is already in progress"),		/* 37 */

/* ipc/network software -- argument errors */
	BIGSTRING ("\x20" "Socket operation on a non-socket"),	/* 38 */
	BIGSTRING ("\x1f" "Destination address is required"),		/* 39 */
	BIGSTRING ("\x10" "Message too long"),					/* 40 */
	BIGSTRING ("\x1e" "Protocol wrong type for socket"),	/* 41 */
	BIGSTRING ("\x16" "Protocol not available"),			/* 42 */
	BIGSTRING ("\x16" "Protocol not supported"),			/* 43 */
	BIGSTRING ("\x19" "Socket type not supported"),			/* 44 */
	BIGSTRING ("\x21" "Operation not supported on socket"),	/* 45 */
	BIGSTRING ("\x1d" "Protocol family not supported"),		/* 46 */
	BIGSTRING ("\x2f" "Address family not supported by protocol family"),	/* 47 */
	BIGSTRING ("\x16" "Address already in use"),			/* 48 */
	BIGSTRING ("\x1e" "Can't assign requested address"),	/* 49 */

/* ipc/network software -- operational errors */
	BIGSTRING ("\x0f" "Network is down"),					/* 50 */
	BIGSTRING ("\x16" "Network is unreachable"),			/* 51 */
	BIGSTRING ("\x23" "Network dropped connection on reset"),	/* 52 */
	BIGSTRING ("\x20" "Software caused connection abort"),	/* 53 */
	BIGSTRING ("\x18" "Connection reset by peer"),			/* 54 */
	BIGSTRING ("\x19" "No buffer space available"),			/* 55 */
	BIGSTRING ("\x1b" "Socket is already connected"),		/* 56 */
	BIGSTRING ("\x17" "Socket is not connected"),			/* 57 */
	BIGSTRING ("\x20" "Can't send after socket shutdown"),	/* 58 */
	BIGSTRING ("\x21" "Too many references: can't splice"),	/* 59 */
	BIGSTRING ("\x14" "Connection timed out"),				/* 60 */
	BIGSTRING ("\x12" "Connection refused"),				/* 61 */

	BIGSTRING ("\x21" "Too many levels of symbolic links"),	/* 62 */
	BIGSTRING ("\x12" "File name too long"),				/* 63 */

	BIGSTRING ("\x0c" "Host is down"),						/* 64 */
	BIGSTRING ("\x10" "No route to host"),					/* 65 */
	BIGSTRING ("\x13" "Directory not empty"),				/* 66 */
	BIGSTRING ("\x12" "Too many processes"),				/* 67 */
	BIGSTRING ("\x0e" "Too many users"),					/* 68 */
	BIGSTRING ("\x13" "Disc quota exceeded"),				/* 69 */

/* Network File System */
	BIGSTRING ("\x15" "Stale NFS file handle"),				/* 70 */
	BIGSTRING ("\x21" "Too many levels of remote in path"),	/* 71 */
	BIGSTRING ("\x11" "RPC struct is bad"),					/* 72 */
	BIGSTRING ("\x11" "RPC version wrong"),					/* 73 */
	BIGSTRING ("\x13" "RPC prog. not avail"),				/* 74 */
	BIGSTRING ("\x15" "Program version wrong"),				/* 75 */
	BIGSTRING ("\x19" "Bad procedure for program"),			/* 76 */
	BIGSTRING ("\x12" "No locks available"),				/* 77 */
	BIGSTRING ("\x18" "Function not implemented"),			/* 78 */
	BIGSTRING ("\x45" "Can't read stream because the TCP connection was closed unexpectedly."),	/* 79 */
	}; //tcperrorstrings


#define ACCEPT_CONN_WITHOUT_GLOBALS		1


	#ifdef FRONTIER_GUSI_2
		#include <compat.h>
		#include <fcntl.h>
		#include <inttypes.h>
		#include <netdb.h>
		#include <netinet/in.h>
		#include <arpa/inet.h>
		#include <sys/ioctl.h>
		#include <sys/socket.h>
		#include <sys/stat.h>
		#include <sys/time.h>
		#include <sys/types.h>
		#include <sys/un.h>
		#include <unistd.h>
		#include <utime.h>
		
		#include <pthread.h>

		#define GUSI_SpinHook	'spin'

		typedef void (*GUSIHook)(void);
		
		void GUSISetHook (OSType code, GUSIHook hook);

		#define ACCEPT_IN_SEPARATE_THREAD		1

	#else

		#include <GUSI.h>
		
		#undef ACCEPT_IN_SEPARATE_THREAD

	#endif

	#include <sys/errno.h>
	#include "mac.h"

#include "error.h"
#include "file.h"
#include "kb.h"
#include "ops.h"
#include "memory.h"
#include "threads.h"
#include "strings.h"
#include "lang.h"
#include "langinternal.h"
#include "process.h"
#include "processinternal.h"
#include "shell.h"
#include "shellhooks.h"

#include "winsocknetevents.h"


#define NO_HOST_SERVICES NULL



	#ifdef FRONTIER_GUSI_2
	typedef struct hostData {
		
		void * dummy; // strucure not needed under Windows
		} hostData;
	#endif
	
	extern int h_errno;
	
	extern long sHostID = 0; //cached value in GUSINetDB.cp
	
	#define WSAEWOULDBLOCK EAGAIN
	#define WSAENOTCONN ENOTCONN
	#define WSAETIMEDOUT ETIMEDOUT
	#define WSAECONNABORTED ECONNABORTED
	#define WSAENOTSOCK ENOTSOCK
	#define WSAEMFILE EMFILE
	#define WSAGetLastError() (errno == EINTR? userCanceledErr : errno)
	#define WSAGetHostError() h_errno

	#define wsprintf sprintf

	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)

	#define SD_READ		0
	#define SD_SEND		1
	#define SD_BOTH		2

	#define closesocket(foo) close(foo)
	#define ioctlsocket(d,request,argp) ioctl(d,request,argp)
	typedef int SOCKET;

	/* Microsoft Windows Extended data types */
	#define FAR

	typedef struct sockaddr SOCKADDR;
	typedef struct sockaddr *PSOCKADDR;
	typedef struct sockaddr FAR *LPSOCKADDR;

	typedef struct sockaddr_in SOCKADDR_IN;
	typedef struct sockaddr_in *PSOCKADDR_IN;
	typedef struct sockaddr_in FAR *LPSOCKADDR_IN;

	typedef struct linger LINGER;
	typedef struct linger *PLINGER;
	typedef struct linger FAR *LPLINGER;

	typedef struct in_addr IN_ADDR;
	typedef struct in_addr *PIN_ADDR;
	typedef struct in_addr FAR *LPIN_ADDR;

#define SOCKTYPE_INVALID -1
#define SOCKTYPE_UNKNOWN 0
#define SOCKTYPE_OPEN 1
#define SOCKTYPE_DATA 2
#define SOCKTYPE_LISTENING 3
#define SOCKTYPE_CLOSED 4
#define SOCKTYPE_LISTENSTOPPED 5
#define SOCKTYPE_INACTIVE 6

#define FRONTIER_MAX_STREAM 256
#define INTNETERROR_INVALIDSTREAM -1

typedef short tysocktypeid;

typedef struct tysockRecord {
	SOCKET			sockID;
	tysocktypeid	typeID;
	long			refcon;
	bigstring 		callback;
	long			maxdepth;
	long			listenReference;
	long			currentListenDepth;
	boolean			flNotification;
	#ifdef ACCEPT_CONN_WITHOUT_GLOBALS
		Handle			hcallbacktree;
	#endif
	#ifdef ACCEPT_IN_SEPARATE_THREAD
		long				idthread;
		hdldatabaserecord	hdatabase;
	#endif
	} sockRecord;


static short frontierWinSockCount = 0;
static boolean frontierWinSockLoaded = false;
static sockRecord sockstack[FRONTIER_MAX_STREAM];


	static short sockListenCount = 0;
	static short sockListenList[FRONTIER_MAX_STREAM];



#define _entercriticalsockstacksection()

#define _leavecriticalsockstacksection()



static char * TCPGETTYPE (tysocktypeid typeID) {
	switch (typeID) {
		case SOCKTYPE_INVALID:
			return ("INVALID");

		case SOCKTYPE_UNKNOWN:
			return ("UNKNOWN");

		case SOCKTYPE_OPEN:
			return ("OPEN");

		case SOCKTYPE_DATA:
			return ("DATA");

		case SOCKTYPE_LISTENING:
			return ("LISTENING");

		case SOCKTYPE_CLOSED:
			return ("CLOSED");

		case SOCKTYPE_LISTENSTOPPED:
			return ("LISTEN-STOPPED");

		case SOCKTYPE_INACTIVE:
			return ("INACTIVE");

		default:
			break;
		}

	return ("BAD Type value");
	} /*TCPGETTYPE*/



#define STR_P_ERROR_CLOSED_PREMATURELY	BIGSTRING ("\x45" "Can't read stream because the TCP connection was closed unexpectedly.")

/*
	To disable the tcp tracker, don't define TCPTRACKER.

	To enable tcp tracker ouput in the about window, define TCPTRACKER == 1.

	To enable tcp tracker error ouput to a file, define TCPTRACKER == 2.

	To enable full tcp tracker output to a file, define TCPTRACKER == 3.
*/


//#undef TCPTRACKER
#define TCPTRACKER 1
//#define TCPTRACKER 2
//#define TCPTRACKER 3

#if (TCPTRACKER == 3)
#pragma message ("*********************** TCPTRACKER is ON: Full output to tcpfile.txt ***********************")

static boolean fllogger = true;


static FILE * tcpfile = NULL;
static char TCPmsg[400];

#define TCPprintf(msg) msg
#define TCPERRORprintf(msg) msg

static void TCPWRITEMSG () {
	unsigned long ticks = gettickcount ();
	static unsigned long lastticks = 0;
		long idthread = (long) (**getcurrentthread ()).idthread;
		static long idlastthread = 0;
	
	if (fllogger) {

		if (tcpfile == NULL) {
			tcpfile = fopen ("tcpfile.txt", "w+");
			}

		if (idthread != idlastthread) {
			fprintf (tcpfile, "\n");
			idlastthread = idthread;
			}


		fprintf (tcpfile, "%08X (%04ld) | %04X | %s", (unsigned long) ticks, (ticks - lastticks), idthread, TCPmsg);

		lastticks = ticks;

		fflush (tcpfile);
		}
	} /*TCPWRITEMSG*/

#define TCPERRORWRITEMSG TCPWRITEMSG

void TCPTRACKERIN (char * functionName, int linenumber, unsigned long streamID) {
	if (fllogger) {
		if ((streamID < 1) || (streamID >= FRONTIER_MAX_STREAM)) {
			wsprintf (TCPmsg, "Entering %s at line %d, Stream = %ld - INVALID STREAM.\n", functionName, linenumber, streamID);
			TCPWRITEMSG ();
			return;
			}

			wsprintf (TCPmsg, "Entering %s at line %d, Stream = %ld, Socket = %ld, Type is %s, Max Depth is %d, Current Depth is %d, Listen Ref is %08lX, Refcon = %08lX.\n",
				functionName, linenumber, streamID, sockstack[streamID].sockID, TCPGETTYPE (sockstack[streamID].typeID), sockstack[streamID].maxdepth,
				sockstack[streamID].currentListenDepth, sockstack[streamID].listenReference, sockstack[streamID].refcon);

		TCPWRITEMSG ();
		}
	} /*TCPTRACKERIN*/


void TCPTRACKEROUT (char * functionName, int linenumber, unsigned long streamID) {
	if (fllogger) {
		if ((streamID < 1) || (streamID >= FRONTIER_MAX_STREAM)) {
			wsprintf (TCPmsg, "Exiting %s at line %d, Stream = %08lX - INVALID STREAM.\n", functionName, linenumber, streamID);
			TCPWRITEMSG ();
			return;
			}

			wsprintf (TCPmsg, "Exiting %s at line %d, Stream = %ld, Socket = %ld, Type is %s, Max Depth is %d, Current Depth is %d, Listen Ref is %08lX, Refcon = %08lX.\n",
				functionName, linenumber, streamID, sockstack[streamID].sockID, TCPGETTYPE (sockstack[streamID].typeID), sockstack[streamID].maxdepth,
				sockstack[streamID].currentListenDepth, sockstack[streamID].listenReference, sockstack[streamID].refcon);

		TCPWRITEMSG ();
		}
	} /*TCPTRACKEROUT*/

static void TCPTRACKERCLOSE () {
	if (fllogger) {
		if (tcpfile != NULL)
			fclose (tcpfile);
		}
	}


#elif (TCPTRACKER == 2)
#pragma message ("*********************** TCPTRACKER is ON: Error output to tcpfile.txt **********************")

static boolean fllogger = true;


static FILE * tcpfile = NULL;
static char TCPmsg[400];

#define TCPprintf(msg)
#define TCPWRITEMSG()
#define TCPTRACKERIN(functionName, linenumber, streamID)
#define TCPTRACKEROUT(functionName, linenumber, streamID)

#define TCPERRORprintf(msg) msg

static void TCPERRORWRITEMSG () {
	unsigned long ticks = gettickcount ();
	static unsigned long lastticks = 0;
		long idthread = (long) (**getcurrentthread ()).idthread;
		static long idlastthread = 0;
	
	if (fllogger) {

		if (tcpfile == NULL) {
			tcpfile = fopen ("tcpfile.txt", "w+");
			}

		if (idthread != idlastthread) {
			fprintf (tcpfile, "\n");
			idlastthread = idthread;
			}


		fprintf (tcpfile, "%08X (%04ld) | %04X | %s", (unsigned long) ticks, (ticks - lastticks), idthread, TCPmsg);

		lastticks = ticks;

		fflush (tcpfile);
		}
	} /*TCPWRITEMSG*/


static void TCPTRACKERCLOSE () {
	if (fllogger) {
		if (tcpfile != NULL)
			fclose (tcpfile);
		}
	}

#elif (TCPTRACKER == 1)
#pragma message ("*********************** TCPTRACKER is ON: Full output to About window **********************")

#include "about.h"
#define fllogger (aboutstatsshowing())
#define TCPprintf(msg) msg
#define TCPERRORprintf(msg) msg

static char TCPmsg[400];

static void TCPWRITEMSG () {
	
	if (fllogger) {
		
		convertcstring (TCPmsg);
		
		aboutsetmiscstring (TCPmsg);
		}
	} /*TCPWRITEMSG*/

#define TCPERRORWRITEMSG TCPWRITEMSG

static void TCPTRACKERIN (char * functionName, int linenumber, unsigned long streamID) {
	if (fllogger) {
		if ((streamID < 1) || (streamID >= FRONTIER_MAX_STREAM)) {
			wsprintf (TCPmsg, "Entering %s at line %d, Stream = %ld - INVALID STREAM.\n", functionName, linenumber, streamID);
			TCPWRITEMSG ();
			return;
			}

		wsprintf (TCPmsg, "Entering %s at line %d, Stream = %ld, Socket = %ld, Type is %s, Max Depth is %d, Current Depth is %d, Listen Ref is %08lX, Refcon = %08lX.\n",
			functionName, linenumber, streamID, sockstack[streamID].sockID, TCPGETTYPE (sockstack[streamID].typeID), sockstack[streamID].maxdepth,
			sockstack[streamID].currentListenDepth, sockstack[streamID].listenReference, sockstack[streamID].refcon);

		TCPWRITEMSG ();
		}
	} /*TCPTRACKERIN*/


static void TCPTRACKEROUT (char * functionName, int linenumber, unsigned long streamID) {
	if (fllogger) {
		if ((streamID < 1) || (streamID >= FRONTIER_MAX_STREAM)) {
			wsprintf (TCPmsg, "Exiting %s at line %d, Stream = %08lX - INVALID STREAM.\n", functionName, linenumber, streamID);
			TCPWRITEMSG ();
			return;
			}

		wsprintf (TCPmsg, "Exiting %s at line %d, Stream = %ld, Socket = %ld, Type is %s, Max Depth is %d, Current Depth is %d, Listen Ref is %08lX, Refcon = %08lX.\n",
			functionName, linenumber, streamID, sockstack[streamID].sockID, TCPGETTYPE (sockstack[streamID].typeID), sockstack[streamID].maxdepth,
			sockstack[streamID].currentListenDepth, sockstack[streamID].listenReference, sockstack[streamID].refcon);

		TCPWRITEMSG ();
		}
	} /*TCPTRACKEROUT*/

#define TCPTRACKERCLOSE()

#else

#define TCPprintf(msg)
#define TCPERRORprintf(msg)
#define TCPWRITEMSG()
#define TCPERRORWRITEMSG()
#define TCPTRACKERIN(functionName, linenumber, streamID)
#define TCPTRACKEROUT(functionName, linenumber, streamID)
#define TCPTRACKERCLOSE()

#endif


static boolean getsockrecord (SOCKET sock, long *stream) {
	long i;

	for (i = 1; i < FRONTIER_MAX_STREAM; i++) {
		if (sockstack[i].sockID == sock) {
			*stream = i;
			return (true);
			}
		}

	*stream = -1;

	return (false);
	} /*getsockrecord*/


static void initsockrecord (long i) {

	/*
	5.0.2b21 dmb: share repeated code
	*/
	
	sockstack[i].sockID = INVALID_SOCKET;
	sockstack[i].typeID = SOCKTYPE_UNKNOWN;
	sockstack[i].maxdepth = 0;
	sockstack[i].listenReference = 0;
	sockstack[i].currentListenDepth = 0;
	sockstack[i].refcon = 0;
	copystring (emptystring, sockstack[i].callback);
	sockstack[i].flNotification = false;
	#ifdef ACCEPT_CONN_WITHOUT_GLOBALS
		sockstack[i].hcallbacktree = nil;
	#endif
	#ifdef ACCEPT_IN_SEPARATE_THREAD
		sockstack[i].idthread = nil;
		sockstack[i].hdatabase = nil;
	#endif
	} /*initsockrecord*/


static boolean addsockrecord (long *stream) {
	
	/*
	5.1.5 dmb: reuse closed sockets before inactive ones.

	6.2a12 AR: This is the bottleneck for grabbing a new or used socket record.
	On Win32, protect the socket stack by declaring a critical section.
	*/

	long i;

	_entercriticalsockstacksection();

	for (i = 1; i < FRONTIER_MAX_STREAM; i++) {
		if (sockstack[i].typeID == SOCKTYPE_INVALID) {
			initsockrecord (i);
			*stream = i;
			_leavecriticalsockstacksection();
			return (true);
			}
		}
	
	/* If none are inactive, reuse those that are closed */
	
	for (i = 1; i < FRONTIER_MAX_STREAM; i++) {
		if (sockstack[i].typeID == SOCKTYPE_CLOSED) {
			initsockrecord (i);
			*stream = i;
			_leavecriticalsockstacksection();
			return (true);
			}
		}

	/* If we used up all the stack, reuse those that are inactive */

	for (i = 1; i < FRONTIER_MAX_STREAM; i++) {
		if (sockstack[i].typeID == SOCKTYPE_INACTIVE) {
			initsockrecord (i);
			*stream = i;
			_leavecriticalsockstacksection();
			return (true);
			}
		}

	_leavecriticalsockstacksection();

	*stream = -1;

	return (false);
	} /*addsockrecord*/


static void clearsockstack () {
	long i;

	_entercriticalsockstacksection();

	for (i = 0; i < FRONTIER_MAX_STREAM; i++) {
		sockstack[i].sockID = INVALID_SOCKET;
		sockstack[i].typeID = SOCKTYPE_INVALID;
		}

	_leavecriticalsockstacksection();

		sockListenCount = 0;
	} /*clearsockstack*/


static void gettcperrorstring (int errcode, bigstring bs) {

	int ixtcperr = errcode;
	
	
	if (ixtcperr > 0 && ixtcperr < 80) {
		
		copystring (tcperrorstrings [ixtcperr], bs); //handles nil source
		
		if (!isemptystring (bs)) {
			
			pushchar ('.', bs);

			return;
			}
		}
	
		getsystemerrorstring (errcode, bs);
	
	
	} /*gettcperrorstring*/


static void plainneterror (bigstring bs) {

	/*
	6.1b15 AR
	*/

	bigstring errbs;
	
	copystring (bs, errbs);

	nullterminate (errbs);
	
	TCPERRORprintf (wsprintf(TCPmsg, "NET ERROR - %s.\n", stringbaseaddress(errbs)));
	TCPERRORWRITEMSG ();
	
	langerrormessage (bs);
	} /*neterror*/


static void neterror (char * cannot, long errcode) {

	bigstring bs;
	bigstring errbs;
	char prestring[256];

	wsprintf (prestring, "Can't %s because TCP/IP error code %ld", cannot, (long)errcode);
	copyctopstring (prestring, errbs);

	gettcperrorstring (errcode, bs);

	//if (equaltextidentifiers (stringbaseaddress(bs), "No information available for error number", (short)strlen("No information available for error number")) != true) {
	if (!isemptystring (bs)) {
		
		pushstring (BIGSTRING ("\x03" " - "), errbs);
		
		pushstring (bs, errbs);
		}
	else {
		pushchar ('.', errbs);
		}
	
	nullterminate (errbs);
	
	TCPERRORprintf (wsprintf(TCPmsg, "NET ERROR - %s.\n", stringbaseaddress(errbs)));
	TCPERRORWRITEMSG ();
	
	langerrormessage (errbs);
	} /*neterror*/


static void intneterror (long errcode) {
	bigstring bs;

	if (errcode == INTNETERROR_INVALIDSTREAM)
		copyctopstring ("Invalid stream", bs);
	else
		copyctopstring ("Unknown stream error", bs);

	langerrormessage (bs);
	} /*intneterror*/



	#ifdef FRONTIER_GUSI_2
	
	static void fwsGUSI2Spin (boolean flresting) {
		short	mask = osMask|activMask|mDownMask|keyDownMask;

		if (inmainthread ()) {
				
			if (flresting) {	
				EventRecord		ev;
				
				if (WaitNextEvent (mask, &ev, 1L, nil))
					shellprocessevent (&ev);
				}
			else
				shellyield (false);
			}
		else {
/*			if (flresting) {

				boolean fl;			
				hdlthreadglobals hthread = getcurrentthreadglobals ();
				ThreadID idthread = (ThreadID) (**hthread).idthread;
				const long sleepTime = 6L;
				//hdlthreadqueue hq;
				
				//if (!newclearhandle (sizeof (tythreadqueuerecord), (Handle *) &hq))
				//	return (memFullErr);
				
				//(**hq).idthread = idthread;
				
				TCPprintf (wsprintf(TCPmsg, "Going to sleep for %d ticks.\n", sleepTime));
				TCPWRITEMSG();
				
				//listlink ((hdllinkedlist) gusisleepqueue, (hdllinkedlist) hq);
				
				fl = processsleep (hthread, sleepTime);	
				
				if (!fl) {
					
					//if (listunlink ((hdllinkedlist) gusisleepqueue, (hdllinkedlist) hq))
						//disposehandle ((Handle) hq);
					
					fl = langbackgroundtask (true);
					}
				
				TCPprintf (wsprintf(TCPmsg, "Awake! at line %d.\n", __LINE__));
				TCPWRITEMSG();
				
			//	gusiwakethread = 0;
				}
			else */
				langbackgroundtask (true);
			}
		}/*fwsGUSI2Spin*/

	#else
	
		void ROTATECURSOR (short);
		void ROTATECURSOR (short i) {} // for GUSI
	
	typedef struct tythreadqueuerecord {
		
		struct tythreadqueuerecord **hnext;
		
		ThreadID idthread;
		} tythreadqueuerecord, *ptrthreadqueue, **hdlthreadqueue;


	static hdlthreadqueue gusisleepqueue = nil;
	static hdlthreadqueue gusiwakequeue = nil;

	
	static int fwsGUSISpin (spin_msg msg, long arg) {
	
		long			sleepTime =	6;	// 1/10 of a second by default
		boolean			flresting = true;
		short			mask = osMask|activMask|mDownMask|keyDownMask; //|highLevelEventMask|updateMask
		extern int GUSI_error (int);
		boolean			fl = true;

		if (languserescaped (false))
			return GUSI_error (-128);

		/*
		switch (msg) {
			case SP_SLEEP:
			case SP_SELECT:
				if (arg >= sleepTime)				// Only sleep if patience guaranteed
					break;
				// Otherwise, fall through	
			case SP_AUTO_SPIN:
				sleepTime = 0;
				flresting = false;
				break;
			//case SP_STREAM_WRITE:
			//	if (arg >= sleepTime)
			//		sleepTime = 120;
			//	break;
			case SP_MISC:
				sleepTime = 0;
				break;
			
			default:
				break;
		}
		*/
		
		switch (msg) {
			
			case SP_AUTO_SPIN:
				return noErr;
			
			case SP_MISC:
				if (arg < 0) {
					sleepTime = 0;
					flresting = false;
					}
				else
					sleepTime = 6;
				
				break;
			
			case SP_NAME:
			case SP_ADDR:
			case SP_STREAM_READ:
			case SP_STREAM_WRITE:
				if (arg < 0)
					sleepTime = 0;
				else
					sleepTime = 120;
				break;
			
			default:
				flresting = false;
			}
		
		if (inmainthread ()) {
			
			if (flresting) {	
				EventRecord		ev;
				
				if (WaitNextEvent (mask, &ev, sleepTime, nil))
					shellprocessevent (&ev);
				}
		//	else
		//		fl = shellyield (false);
			}
		
		else {
			if (flresting) {
			
				hdlthreadglobals hthread = getcurrentthreadglobals ();
				ThreadID idthread = (ThreadID) (**hthread).idthread;
				hdlthreadqueue hq;
				
				if (!newclearhandle (sizeof (tythreadqueuerecord), (Handle *) &hq))
					return (memFullErr);
				
				(**hq).idthread = idthread;
				
				TCPprintf (wsprintf(TCPmsg, "Going to sleep for %d ticks.\n", sleepTime));
				TCPWRITEMSG();
				
				listlink ((hdllinkedlist) gusisleepqueue, (hdllinkedlist) hq);
				
				fl = processsleep (hthread, sleepTime);	
				
				if (!fl) {
					
					if (listunlink ((hdllinkedlist) gusisleepqueue, (hdllinkedlist) hq))
						disposehandle ((Handle) hq);
					
					fl = langbackgroundtask (true);
					}
				
				TCPprintf (wsprintf(TCPmsg, "Awake! at line %d.\n", __LINE__));
				TCPWRITEMSG();
				
			//	gusiwakethread = 0;
				}
			else
				fl = langbackgroundtask (true);
			}
		
		if (fl)
			return noErr;
		else
			return -128;
	} /*fwsGUSISpin*/


//#define GUSISLEEPQUEUESIZE 128
//static ThreadID gusisleepqueue [GUSISLEEPQUEUESIZE];

//static long ixlastcompleted = -1;
//static long ixlastwoken = -1;

static void wakecompletedthreads (void) {
	
	/*
	walk through the circular gusisleepqueue and wake any newly-completed threads
	*/
	
	/*
	for ( ; ixlastwoken != ixlastcompleted; ixlastwoken = (ixlastwoken + 1) % GUSISLEEPQUEUESIZE) {
		
		ThreadID id = gusisleepqueue [ixlastwoken];
	*/
	hdlthreadqueue hq, hnext;
	
	for (hq = (**gusiwakequeue).hnext; hq != nil; hq = hnext) {
		
		ThreadID id = (**hq).idthread;
		hdlthreadglobals hg = getprocessthread (id);
		
		hnext = (**hq).hnext;
		
		listunlink ((hdllinkedlist) gusiwakequeue, (hdllinkedlist) hq);
		
		disposehandle ((Handle) hq);
		
		if (hg && processwake (hg)) {
			TCPprintf (wsprintf(TCPmsg, "Waking thread at line %d.  Thread ID: %04X.\n", __LINE__, id));
			TCPWRITEMSG();
			}
		else {
			TCPERRORprintf (wsprintf(TCPmsg, "Error waking thread at line %d.  Thread ID: %04X.\n", __LINE__, id));
			TCPERRORWRITEMSG();
			}
		}
	} /*wakecompletedthreads*/


static OSErr fwsGUSIWakeThread (ThreadID idthread) {
	
	/*
	gusisleepqueue [ixlastcompleted] = idthread;
	
	ixlastcompleted = (ixlastcompleted + 1) % GUSISLEEPQUEUESIZE;
	*/
	
	/*
	5.1.5b10 dmb: can't do much in a completion routine. just move thread 
	from sleepqueue to wakequeue.
	*/
	
	hdlthreadqueue hq;
	
	for (hq = (**gusisleepqueue).hnext; hq != nil; hq = (**hq).hnext) {
		
		if ((**hq).idthread == idthread) {
			
			listunlink ((hdllinkedlist) gusisleepqueue, (hdllinkedlist) hq);
			
			listlink ((hdllinkedlist) gusiwakequeue, (hdllinkedlist) hq);
			
			break;
			}
		}
	
	return (noErr);
	} /*fwsGUSIWakeThread*/


static boolean fwsGUSIWakeupHook (hdlprocessthread hthread) {
	
	/*
	if this thread is in our sleep queue, wake it up and return false
	
	just to be extra careful, handle the case where the thread is already
	in the wake queue
	*/
	
	hdlthreadqueue hq;
	ThreadID idthread = (ThreadID) (**hthread).idthread;
	
	for (hq = (**gusisleepqueue).hnext; hq != nil; hq = (**hq).hnext) {
		
		if ((**hq).idthread == idthread) { //move it to wake queue
			
			listunlink ((hdllinkedlist) gusisleepqueue, (hdllinkedlist) hq);
			
			listlink ((hdllinkedlist) gusiwakequeue, (hdllinkedlist) hq);
			
			break;
			}
		}
	
	for (hq = (**gusiwakequeue).hnext; hq != nil; hq = (**hq).hnext) {
		
		if ((**hq).idthread == idthread) { //found it in wake queue, wake it
			
			listunlink ((hdllinkedlist) gusiwakequeue, (hdllinkedlist) hq);
			
			disposehandle ((Handle) hq);
			
			if (processwake (hthread)) {
				TCPprintf (wsprintf(TCPmsg, "Hook: waking thread at line %d.  Thread ID: %04X.\n", __LINE__, idthread));
				TCPWRITEMSG();
				}
			else {
				TCPERRORprintf (wsprintf(TCPmsg, "Hook: error waking thread at line %d.  Thread ID: %04X.\n", __LINE__, idthread));
				TCPERRORWRITEMSG();
				}
			
			return (false); //hooked
			}
		}
	
	return (true); //not hooked
	} /*fwsGUSIWakeupHook*/


#endif


#ifdef PIKE

	/*extern const*/ /*long maxconnections = 5;*/

	long maxconnections = longinfinity; /*7.1b5 PBS: no more connection limit*/

#else

	/*extern const*/ long maxconnections = longinfinity; /*7.0b37 PBS: reported in system.environment table in Frontier*/
															/*7.1b2 RAB: made global variables*/

#endif

/*7.0b37 PBS: Count connections in both Radio and Frontier.*/

extern long ctconnections = 0;


static boolean incrementconnectioncounter (void) {

	if (ctconnections >= maxconnections)
		return (false);

	ctconnections++;

	return (true);	
	} /*incrementconnectioncounter*/

static void decrementconnectioncounter (void) {
	
	ctconnections--;

	assert (ctconnections >= 0);

	} /*decrementconnectioncounter*/


long fwsNetEventGetConnectionCount (void) {

	/*7.0b37 PBS: return current count of TCP connections.
	Used by tcp.countConnections verb.
	*/

	return (ctconnections);
	} /*fwsNetEventGetConnectionCount*/


static boolean fwsNetEventLaunch (struct hostData *data) {

	/*
	Initialize the NetEvents system
	
	5.0.2b5 dmb: added hostData parameter and GUSI support to handle threading
	*/


	if (! frontierWinSockLoaded) {

		
			#ifdef FRONTIER_GUSI_2
				//GUSISetHook (GUSI_SpinHook, (GUSIHook) fwsGUSI2Spin);
			#else
				GUSISetup (GUSIwithInternetSockets);
				
				GUSISetHook (GUSI_SpinHook, (GUSIHook) fwsGUSISpin);
				
				GUSISetHook (GUSI_WakeThreadHook, (GUSIHook) fwsGUSIWakeThread);
			
				if (!newclearhandle (sizeof (tythreadqueuerecord), (Handle *) &gusisleepqueue))
					return (false);
				
				if (!newclearhandle (sizeof (tythreadqueuerecord), (Handle *) &gusiwakequeue))
					return (false);
				
				shellpushwakeuphook (&fwsGUSIWakeupHook);
			#endif


		clearsockstack();
		}
	
		#ifndef FRONTIER_GUSI_2
			sethostdata (data);
		#endif
	
	++frontierWinSockCount;
	frontierWinSockLoaded = true;

	return (true);
	} /*fwsNetEventLaunch*/


/*
boolean fwsNetEventIsRunning (void) {

	/* Indicate if we are between a fwsNetEventLaunch and a fwsNetEventQuit * /

	return (frontierWinSockLoaded);
	} /*fwsNetEventIsRunning*/



boolean fwsNetEventQuit (void) {

	/*
	5.0.2b10: this function is being reborn to close all listeners on the Mac
	*/
	
	//	for (i = 0; i < sockListenCount; i++) {
	//		listenstream = sockListenList[i];
	
	while (sockListenCount > 0)
		fwsNetEventCloseListen (sockListenList [0]);
	
	return (true);
	} /*fwsNetEventQuit*/



boolean fwsNetEventShutDown (void) {

	/*
	shut down the NetEvents system
	This is for Frontier Internal use ONLY
	*/

	frontierWinSockCount = 0;

	TCPTRACKERCLOSE();

	if (frontierWinSockLoaded) {

		return (true);
		}

	return (false);
	} /*fwsNetEventShutDown*/


boolean fwsNetEventAddressDecode (unsigned long addr, bigstring IPaddr) {
	
	/* Convert an address (4 byte) into a dotted IP address */
	
	char * sysstring;
	struct in_addr in;
	struct hostData hostdata;

	if (!fwsNetEventLaunch (&hostdata))
		return (false);
	
	in.s_addr = htonl(addr);

	sysstring = inet_ntoa (in);

	setstringlength (IPaddr,0);
	
	if (sysstring == NULL) {
		
		langlongparamerror (cantdecodeaddress, addr);
		
		return (false);
		}

	copyctopstring (sysstring, IPaddr);

	return (true);
	} /*fwsNetEventAddressDecode*/


boolean fwsNetEventAddressEncode (bigstring IPaddr, unsigned long  * addr) {
	
	/* Convert a dotted IP address into an address (4 byte) */
	
	unsigned long netaddr;
	char sysstring[256];
		#ifndef FRONTIER_GUSI_2
			struct in_addr foo;
		#endif
	struct hostData hostdata;

	if (!fwsNetEventLaunch (&hostdata))
		return (false);

	copyptocstring (IPaddr, sysstring);

		#ifdef FRONTIER_GUSI_2
			netaddr = inet_addr (sysstring);
		#else
			foo = inet_addr (sysstring);
			netaddr = foo.s_addr;
		#endif


	if (netaddr == INADDR_NONE) {
		
		langparamerror (cantencodeaddress, IPaddr);
		
		return (false);
		}

	*addr = ntohl (netaddr);

	return (true);
	} /*fwsNetEventAddressEncode*/


boolean fwsNetEventAddressToName (unsigned long addr, bigstring domainName) {

	/*
	Convert an address (4 byte) into a domain name
	
	5.1.5 dmb: release thread globals!
	*/
	
	struct hostent * h;
	unsigned long netaddr;
	struct hostData hostdata;
	long errcode;
	

	if (!fwsNetEventLaunch (&hostdata))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Entering fwsNetEventAddressToName at line %d.  Address: %ld.\n", __LINE__, addr));
	TCPWRITEMSG();

	releasethreadglobalsnopriority();

	netaddr = htonl(addr);

	h = gethostbyaddr ((char *) &netaddr, 4, PF_INET);

	errcode = WSAGetHostError ();

	grabthreadglobalsnopriority();

	if (h == NULL) {
		neterror("convert address", errcode);
		return (false);
		}

	copyctopstring (h->h_name, domainName);

	TCPprintf (wsprintf(TCPmsg, "Leaving fwsNetEventAddressToName at line %d.  Domain name: %s.\n", __LINE__, h->h_name));
	TCPWRITEMSG();

	return (true);
	} /*fwsNetEventAddressToName*/


boolean fwsNetEventNameToAddress (bigstring domainName, unsigned long * addr) {
	
	/*
	Convert a domain name into an address (4 byte)
	
	5.1.5 dmb: release thread globals!
	*/
	
	struct hostent * h;
	char sysstring[256];
	struct hostData hostdata;
	long errcode;
	
	if (!fwsNetEventLaunch (&hostdata))
		return (false);

	copyptocstring (domainName, sysstring);

	TCPprintf (wsprintf(TCPmsg, "Entering fwsNetEventNameToAddress at line %d.  Domain name: %s.\n", __LINE__, sysstring));
	TCPWRITEMSG();

	releasethreadglobalsnopriority();

	h = gethostbyname (sysstring);

	errcode = WSAGetHostError ();

	grabthreadglobalsnopriority();
	
	if (h == NULL) {
		neterror("convert name", errcode);
		return (false);
		}
	
	*addr = ntohl (*((long *)h->h_addr_list[0]));
	
	TCPprintf (wsprintf(TCPmsg, "Leaving fwsNetEventNameToAddress at line %d.  Address: %ld.\n", __LINE__, addr));
	TCPWRITEMSG();
	
	return (true);
	} /*fwsNetEventNameToAddress*/


boolean fwsNetEventMyAddress (unsigned long * addr) {
	
	/* Get the hosts address */

	struct hostData hostdata;
	
		
		if (!fwsNetEventLaunch (&hostdata))
			return (false);
		
		sHostID = 0; //clear cached value
		
		*addr = (unsigned long) gethostid ();
		
		return (true);
	} /*fwsNetEventMyAddress*/



#ifdef ACCEPT_CONN_WITHOUT_GLOBALS

static boolean fwsgetcallbackcodetree (bigstring bs, Handle *htree) {
		
	Handle htext;
	boolean fl;
	unsigned long savelines;
	unsigned short savechars;
	hdltreenode hmodule = nil;
	
	if (!newtexthandle (bs, &htext))
		return (false);
	
	savelines = ctscanlines;
	
	savechars = ctscanchars;
	
	fl = langcompiletext (htext, false, &hmodule); /*always disposes htext*/
		
	ctscanlines = savelines;
	
	ctscanchars = savechars;
	
	if (!fl)
		return (false); 
	
	fl = langpacktree ((**hmodule).param1, htree); /*make a copy of the sub-tree*/

	langdisposetree (hmodule);
	
	return (fl);
	} /*fwsgetcallbackcodetree*/


static boolean fwsnewprocess (hdltreenode hcode, bigstring bsname, hdlprocessrecord *hprocess) {
		
	register hdlprocessrecord hp;
	hdlerrorstack herrorstack;
	hdltablestack htablestack;
	tyerrorrecord item;
	
	if (!newclearhandle (sizeof (typrocessrecord), (Handle *) hprocess))
		return (false);
	
	hp = *hprocess; /*copy into register*/
	
	if (!newclearhandle (sizeof (tyerrorstack), (Handle *) &herrorstack)) {
		
		disposehandle ((Handle) hp);
		
		return (false);
		}
	
	if (!newclearhandle (sizeof (tytablestack), (Handle *) &htablestack)) {
		
		disposehandle ((Handle) hp);
		
		disposehandle ((Handle) herrorstack);
		
		return (false);
		}
	
	(**hp).hcode = hcode;
	
	(**hp).floneshot = true;
	
	(**hp).errormessagecallback = &langerrordialog;

	(**hp).debugerrormessagecallback = (langerrormessagecallback) &truenoop;
	
	(**hp).htablestack = htablestack;
	
	(**hp).herrorstack = herrorstack;
	
	(**hp).processstartedroutine = (langvoidcallback) &truenoop;
	
	(**hp).processkilledroutine = (langvoidcallback) &truenoop;
		
	item.errorcallback = nil;
	
	item.errorline = 0;
	
	item.errorchar = 0;	
	
	item.errorrefcon = 0;

	item.profilebase = 0;		
	
	(**herrorstack).stack [(**herrorstack).toperror++] = item;

	copystring (bsname, (**hp).bsname);

	return (true);
	} /*newprocess*/
	

static boolean fwsruncallback (long listenstream, long acceptstream, long refcon) {

	hdltreenode hcallbackaddress;
	hdltreenode hfunctioncall;
	hdltreenode hcode;
	hdltreenode hparam1;
	hdltreenode hparam2;
	tyvaluerecord val;
	hdlprocessrecord hprocess;
	Handle h;
	
	//build code tree
	
	if (!copyhandle (sockstack[listenstream].hcallbacktree, &h))
		return (false);
		
	if (!langunpacktree (h, &hcallbackaddress))
		return (false);
	
	setlongvalue (acceptstream, &val);
	
	if (!newconstnode (val, &hparam1)) {
		langdisposetree (hcallbackaddress);
		return (false);
		}
	
	setlongvalue (refcon, &val);
	
	if (!newconstnode (val, &hparam2)) {
		langdisposetree (hcallbackaddress);
		langdisposetree (hparam1);
		return (false);
		}
	
	pushlastlink (hparam2, hparam1);
	
	if (!pushbinaryoperation (functionop, hcallbackaddress, hparam1, &hfunctioncall))
		return (false);
		
	if (!pushbinaryoperation (moduleop, hfunctioncall, nil, &hcode))
		return (false);

	//create new process
	
	if (!fwsnewprocess (hcode, sockstack[listenstream].callback, &hprocess)) {
		langdisposetree (hcode);
		return (false);
		}

	//add new process

	return (addprocess (hprocess));
	}/*fwsruncallback*/

#endif


static void parsecallbackstring (long stream, long p1, long p2, bigstring bs) {

	/*
	5.1.5 dmb: common code for all callbacks
	*/

	copystring (sockstack[stream].callback, bs);
	pushchar ('(', bs);
	pushlong (p1, bs);
	pushchar (',', bs);
	pushlong (p2, bs);
	pushchar (')', bs);

	nullterminate(bs); //for debug display
	} /*parsecallbackstring*/


static boolean fwsrunstring (bigstring bs) {
	
	/*
	5.1.5 dmb: make sure we have thread globals for the compiler and processlist, 
	then run the string as a new process.

	note: we might special case each call and build a code tree by hand, so we
	don't need globals. I'm not sure if adding a new process really needs globals.
	*/

	boolean fl;
	
	grabthreadglobalsnopriority ();

	fl = processrunstringnoerrorclear (bs);
	
	releasethreadglobalsnopriority ();

	return (fl);
	} /*fwsrunstring*/


static boolean restartAccepter (SOCKET s, short listenstream) {
	boolean fl = true;

	return (fl);
	} /*restartAccepter*/


static boolean checkAccepter (unsigned long stream) {

	if (sockstack[stream].listenReference != 0) {

		long listenstream = sockstack[stream].listenReference;

		--sockstack[listenstream].currentListenDepth;

		}

	return (true);
	} /*checkAccepter*/


#ifdef ACCEPT_IN_SEPARATE_THREAD

static boolean fwsacceptsocket (long listenstream) {

	/*
	Process an accept pending message on a socket
	*/

	int sasize;
	struct sockaddr_in sa;
	SOCKET acceptsock;
	long err;
	boolean fl = false;
	long newstream;
	long dummy = 0; /*need a pointer to nil for ioctlsocket call*/

	TCPTRACKERIN ("fwsacceptsocket", __LINE__, listenstream);
	
	/*Accept connection*/

	sasize = sizeof(sa);
	
	acceptsock = accept (sockstack[listenstream].sockID, (struct sockaddr *)&sa, &sasize);

	if (acceptsock != INVALID_SOCKET) {

#ifdef PIKE
		if (incrementconnectioncounter ()) {
#endif

			/*Increment listen depth*/

			++sockstack[listenstream].currentListenDepth;

			/*get socket record for accepted socket*/

			if (!addsockrecord (&newstream)) {
				
				struct linger l;

				l.l_onoff = 1;
				l.l_linger = 0;

				TCPERRORprintf (wsprintf(TCPmsg, "In fwsacceptsocket at line %d.  Error addding new socket record %s (%ld, %ld).\n", __LINE__, stringbaseaddress (sockstack[listenstream].callback), WSAEMFILE * -1L, sockstack[listenstream].refcon));
				TCPERRORWRITEMSG();

				setsockopt (acceptsock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof(l)); /*abort*/

				fl = closesocket (acceptsock);

				#ifdef PIKE
					decrementconnectioncounter ();
				#endif

				goto exit;
				}
	
			/*Add Socket to list*/		
			sockstack[newstream].sockID = acceptsock;
			sockstack[newstream].typeID = SOCKTYPE_OPEN;
			sockstack[newstream].listenReference = listenstream;
			sockstack[newstream].refcon = 0;

			/*Pass "stream" and "refcon" to callback*/

			TCPprintf (wsprintf(TCPmsg, "In fwsacceptsocket at line %d.  Accepted new socket %ld: %s (%ld, %ld).\n", __LINE__, acceptsock, stringbaseaddress (sockstack[listenstream].callback), newstream, sockstack[listenstream].refcon));
			TCPWRITEMSG();

			fl = fwsruncallback (listenstream, newstream, sockstack[listenstream].refcon);

#ifdef PIKE
			}
		else {

			struct linger l;

			l.l_onoff = 1;
			l.l_linger = 0;

			TCPERRORprintf (wsprintf(TCPmsg, "In fwsacceptsocket at line %d.  Exceeded number of maximum connections: %ld of %ld.\n", __LINE__, ctconnections, maxconnections));
			TCPERRORWRITEMSG();

			setsockopt (acceptsock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof(l));

			fl = closesocket (acceptsock);
			}
#endif

		}
	else
		{
		/*if an error - pass this on to callback*/
		err = WSAGetLastError();

		TCPERRORprintf (wsprintf(TCPmsg, "In fwsacceptsocket at line %d.  Error processing accept message %s (%ld, %ld).\n", __LINE__, stringbaseaddress (sockstack[listenstream].callback), err * -1L, sockstack[listenstream].refcon));
		TCPWRITEMSG();

		fl = fwsruncallback (listenstream, err * -1L, sockstack[listenstream].refcon);
		
		}

exit:

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsacceptsocket at line %d.  Return value is %s.\n", __LINE__, fl?"True":"False"));
	TCPWRITEMSG();
	
	return (fl);
	} /*fwsacceptsocket*/


static void *fwsacceptingthreadmain (long *param) {

	/*
	6.2a12 AR: we sit in a loop waiting for new connections.

	7.1b29 PBS: fix CPU usage 100% bug on Win98/ME by yielding in this loop.
	
	9.1b3 AR: Increased select timeout from 10 microseconds to 1 second.
	When select times out, it returns zero and all we do is check whether
	we need to break out of the while loop, so a higher timeout value is
	not a problem. This also makes the 7.1b29 bug fix redundant.
	
	Further, we only need to check the current listen depth if we just
	accepted another connection.
	
	Also, it's good practice to initialize the timeval struct through
	every iteration of the loop.
	*/

	long listenstream = (long) param;
	register sockRecord* sockrecptr = &sockstack[listenstream];
	register SOCKET sock = sockrecptr->sockID;
	Handle hcallback = sockrecptr->hcallbacktree; /*keep a copy in our stack so we can safely dispose it*/	
	long maxdepth = sockrecptr->maxdepth;
	fd_set readset;
	struct timeval tv;
	int res;

	TCPTRACKERIN ("fwsacceptingthreadmain", __LINE__, listenstream);
	
	attachtomainthread (sockrecptr->idthread); /*6.2b7 AR*/
	
	while (sockrecptr->typeID == SOCKTYPE_LISTENING) {

		FD_ZERO (&readset);

		FD_SET (sock, &readset);

		tv.tv_sec = 1L;		/* 1 second */
		tv.tv_usec = 0L;	/* 0 micro-seconds */

		res = select (sock+1, &readset, NULL, NULL, &tv);
		
		TCPprintf (wsprintf(TCPmsg, "In fwsacceptingthreadmain at line %d.  Select returned %ld.\n", __LINE__, (long) res));
		TCPWRITEMSG();
	
		if (sockrecptr->typeID != SOCKTYPE_LISTENING || flshellclosingall) {
			break;
			}

		if (res == 1) {

			(void) fwsacceptsocket (listenstream);

			while (sockrecptr->currentListenDepth >= maxdepth) {
				usleep (10L);	/* sleep for 10 milli-seconds */
				}
			}

/*		
#ifdef PIKE
		usleep (10L); /+7.1b29 PBS: fix CPU usage 100% bug on Win98/ME by yielding in this loop.+/
#endif
*/
		}/*while*/	

	disposehandle (hcallback);

	if (sockrecptr->typeID == SOCKTYPE_LISTENSTOPPED) {
		/*we have been asked by fwsNetEventCloseListen to take responsibility of cleaning up*/
		sockrecptr->sockID = INVALID_SOCKET;
		sockrecptr->typeID = SOCKTYPE_CLOSED;
		}

	TCPTRACKEROUT ("fwsacceptingthreadmain", __LINE__, listenstream);

	return (nil);
	}/*fwsacceptingthreadmain*/


static boolean fwslaunchacceptingthread (long stream) {
	
	
		pthread_t idthread;
		pthread_attr_t attr;
		
		pthread_attr_init (&attr);
		
		pthread_create ((pthread_t *) &sockstack[stream].idthread, &attr, fwsacceptingthreadmain, (void *)stream);
	
		pthread_attr_destroy (&attr);
		
	
	return (true);
	}/*fwslaunchacceptingthread*/

#endif

void fwsNetEventShutdownDependentListeners (long hdatabase) {

#ifdef ACCEPT_IN_SEPARATE_THREAD
	long i;

	for (i = 1; i < FRONTIER_MAX_STREAM; i++)
		if (sockstack[i].typeID == SOCKTYPE_LISTENING)
			if (sockstack[i].hdatabase == (hdldatabaserecord) hdatabase)
				sockstack[i].typeID = SOCKTYPE_LISTENSTOPPED; /*make sure the separate thread terminates before the database goes away*/
#endif	
	}/*fwsNetEventShutdownDependentListeners*/


/* Abort a stream and delete associated data */
boolean fwsNetEventAbortStream (unsigned long stream) {

	/*
	5.1.5 dmb: release thread globals while we close; don't reset 
	socket typeID until close is complete
	
	6.2a9 AR: Don't reset socket typeID and sockID until close is complete
	so addsockrecord doesn't get to hand it out before we're done.
	This bug probably sneaked back in during the 6.1 development cycle.
	*/

	SOCKET sock;
	int res, errcode;
	struct linger l;
//	struct hostData hostdata;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	TCPTRACKERIN ("fwsNetEventAbortStream", __LINE__, stream);

	sock = sockstack[stream].sockID;

	l.l_onoff = 1;
	l.l_linger = 0;

	releasethreadglobalsnopriority();

	setsockopt (sock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof(l));

	res = closesocket(sock);

	errcode = WSAGetLastError ();

	grabthreadglobalsnopriority();

#ifdef PIKE
	decrementconnectioncounter();
#endif

	checkAccepter (stream);

	sockstack[stream].sockID = INVALID_SOCKET;
	sockstack[stream].typeID = SOCKTYPE_CLOSED;

	if (res == SOCKET_ERROR) {
		neterror("abort stream", errcode);
		return (false);
		}

	TCPTRACKEROUT ("fwsNetEventAbortStream", __LINE__, stream);

	return (true);
	} /*fwsNetEventAbortStream*/


boolean fwsNetEventCloseStream (unsigned long stream) {

	/*
	Close a stream and delete associated data
	
	do we want or should the user control the SO_LINGER flag for a
	graceful verses hard socket closure.  for now it is the user.

	6.2a9 AR: Don't reset socket typeID and sockID until close is complete
	so addsockrecord doesn't get to hand it out before we're done.
	This bug probably sneaked back in during the 6.1 development cycle.
	*/

	SOCKET sock;
	long err;
	
	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	TCPTRACKERIN ("fwsNetEventCloseStream", __LINE__, stream);

	sock = sockstack[stream].sockID;

#ifdef PIKE
	decrementconnectioncounter();
#endif

	releasethreadglobalsnopriority();

	if (shutdown (sock, SD_SEND) == SOCKET_ERROR) {
		
		err = WSAGetLastError ();

		goto error;
		}

	if (closesocket(sock) == SOCKET_ERROR) {
		
		err = WSAGetLastError ();

		goto error;
		}

	grabthreadglobalsnopriority();

	checkAccepter (stream);

	sockstack[stream].sockID = INVALID_SOCKET;
	sockstack[stream].typeID = SOCKTYPE_CLOSED;

	TCPprintf (wsprintf(TCPmsg, "Leaving closeStream at line %d.  Thread duration: %ld ticks.\n", __LINE__, gettickcount()-(**getcurrentthreadglobals()).timestarted));
	TCPWRITEMSG();

	return (true);

error:
	
	grabthreadglobalsnopriority ();

	sockstack[stream].sockID = INVALID_SOCKET;
	sockstack[stream].typeID = SOCKTYPE_CLOSED;

	neterror ("close stream", err);

	return (false);
	} /*fwsNetEventCloseStream*/


boolean fwsNetEventCloseListen (unsigned long stream) {

	/*
	Close a listen and delete associated data
	
	do we want or should the user control the SO_LINGER flag for a
	graceful verses hard socket closure.  for now it is the user.
	
	5.1.5b7 dmb: set linger timeout to zero for Mac (now implemented in GUSI)

	6.2a9 AR: Don't reset socket typeID and sockID until close is complete
	so addsockrecord doesn't get to hand it out before we're done.
	*/

	SOCKET sock;
	int res, errcode;
		int i,j;
		struct linger l;
//	struct hostData hostdata;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	sock = sockstack[stream].sockID;

	/* No more messages */
	#ifndef ACCEPT_IN_SEPARATE_THREAD
		sockstack[stream].maxdepth = -100;		/*never restart select call*/
	#endif
	
	TCPTRACKERIN ("fwsNetEventCloseListen", __LINE__, stream);

	releasethreadglobalsnopriority();


		for (i = 0; i < sockListenCount; i++) {
			if (sockListenList [i] == stream) {
				if (i == (sockListenCount - 1)) {
					//easy case - end of list to remove
					--sockListenCount;
					}
				else {
					//Move remaining down in the list
					for (j = i; j < sockListenCount - 1; j++)
						sockListenList[j] = sockListenList[j+1];

					--sockListenCount;
					}

				break;		//It can only be in the list once.
				}
			}
	
		l.l_onoff = 1;
		l.l_linger = 0;
		
		setsockopt (sock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof(l));
	

	res = closesocket(sock);

	errcode = WSAGetLastError ();

	grabthreadglobalsnopriority();

#ifdef ACCEPT_IN_SEPARATE_THREAD
	sockstack[stream].typeID = SOCKTYPE_LISTENSTOPPED;
#else
	sockstack[stream].sockID = INVALID_SOCKET;
	sockstack[stream].typeID = SOCKTYPE_CLOSED;
#endif

	if (res == SOCKET_ERROR) {
		neterror("close listen", errcode);
		return (false);
		}

	TCPTRACKEROUT ("fwsNetEventCloseListen", __LINE__, stream);

	return (true);
	} /*fwsNetEventCloseListen*/


static short is_ipaddress (char *name) {
	
	short ctfields = 1;
	short fieldlen = 0;
	char *p = name;
	
	for (p = name; *p; ++p) {
		
		if (*p == '.') {
			
			if (fieldlen == 0) //leading dot, or consequtive dots
				return (false);
			
			++ctfields;
			
			fieldlen = 0;
			}
		else {
			if (!isdigit (*p))
				return (false);
			
			if (++fieldlen > 3) // four consecutive digits
				return (false);
			}
		}
	
	return (ctfields == 4);
	} /*is_ipaddress*/


static boolean fwsOpenStream (struct sockaddr_in *sa, unsigned long * stream) {
	
	/*
	5.0.2b4 dmb: the common code between openAddrStream and openNameStream, so we
	don't have to convert one to the other to open a stream of either type

	6.1d13 AR: Release thread globals.

	6.2b1 AR: For Pike, decrement connection counter if connection attempt fails.
	*/
	
	SOCKET sock;
	int errcode;
	long streamref;

	if (!addsockrecord (&streamref)) {
		neterror ("open stream", WSAEMFILE); /*Too many open sockets*/
		return (false);
		}		

#ifdef PIKE
	if (!incrementconnectioncounter ()) {

		plainneterror (BIGSTRING ("\x54" "Can't open stream because no more than five TCP connections may be open at any time."));

		return (false);
		}
#endif
	
	releasethreadglobalsnopriority ();

	sock = socket(PF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET) {
		errcode = WSAGetLastError ();
		goto exit;
		}

	if (connect (sock, (struct sockaddr *) sa, sizeof(*sa)) == SOCKET_ERROR) {
		errcode = WSAGetLastError ();
		closesocket (sock);
		goto exit;
		}

	sockstack[streamref].typeID = SOCKTYPE_OPEN;

	sockstack[streamref].sockID = sock;

	*stream = streamref;
	
	grabthreadglobalsnopriority ();

	return (true);

exit:

	sockstack[streamref].typeID = SOCKTYPE_INVALID;

	grabthreadglobalsnopriority ();

	neterror ("open stream", errcode);

#ifdef PIKE
	decrementconnectioncounter ();
#endif

	return (false);
	} /*fwsOpenStream*/


boolean fwsNetEventOpenAddrStream (unsigned long addr, unsigned long port, unsigned long * stream) {

	/*
	Open a stream and create associated data
	
	5.1.6 dmb: don't use a hostent; construct the sockaddr from the address, avoiding DNS
	*/
	
	unsigned long netaddr;
	struct hostData hostdata;
	struct sockaddr_in sa;
	
	if (!fwsNetEventLaunch (&hostdata))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Entering fwsNetEventOpenAddrStream at line %d.  Address: %ld.\n", __LINE__, addr));
	TCPWRITEMSG();

	netaddr = htonl(addr);

		sHostID = 0; //clear cached value
	
	memset (&sa, 0, sizeof(sa));
	
	memcpy (&(sa.sin_addr), &netaddr, sizeof (netaddr));
	
	sa.sin_family = AF_INET;
	
	sa.sin_port = htons ((unsigned short) port);
	
	return (fwsOpenStream (&sa, stream));
	} /*fwsNetEventOpenAddrStream*/


boolean fwsNetEventOpenNameStream (bigstring name, unsigned long port, unsigned long * stream) {

	/*
	Open a stream and create associated data
	
	5.1.6 dmb: if passed an IP address, encode address and use that instead (avoid dns)
	*/

	char sysstring[256];
	struct hostent * hp;
	struct hostData hostdata;
	unsigned long addr;
	struct sockaddr_in sa;
	long errcode;

	if (!fwsNetEventLaunch (&hostdata))
		return (false);
	
	copyptocstring (name, sysstring);
	
		sHostID = 0; //clear cached value
	
	TCPprintf (wsprintf(TCPmsg, "Entering fwsNetEventOpenNameStream at line %d.  Domain name: %s.\n", __LINE__, sysstring));
	TCPWRITEMSG();
	
	if (is_ipaddress (sysstring)) {
		
		if (!fwsNetEventAddressEncode (name, &addr))
			return  (false);
		
		return (fwsNetEventOpenAddrStream (addr, port, stream));
		}
	
	releasethreadglobalsnopriority ();

	hp = gethostbyname (sysstring);

	errcode = WSAGetHostError ();

	grabthreadglobalsnopriority ();

	if (hp == NULL) {
		neterror("open named stream", errcode);
		return (false);
		}
	
	memset (&sa, 0, sizeof(sa));
	
	memcpy (&(sa.sin_addr), hp->h_addr_list[0], hp->h_length);
	
	sa.sin_family = hp->h_addrtype;
	
	sa.sin_port = htons ((unsigned short) port);
	
	return (fwsOpenStream (&sa, stream));
	} /*fwsNetEventOpenNameStream*/


boolean fwsNetEventReadStream (unsigned long stream, unsigned long * bytesToRead, char * buffer) {

	/* Read from a stream */

	SOCKET sock;
	int res, errcode;
//	struct hostData hostdata;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventReadStream", __LINE__, stream);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	sock = sockstack[stream].sockID;

	releasethreadglobalsnopriority();

	res = recv (sock, buffer, *bytesToRead, 0);
	
	errcode = WSAGetLastError ();

	grabthreadglobalsnopriority();

	if (res == SOCKET_ERROR) {
		neterror("read stream", errcode);
		return (false);
		}

	TCPprintf (if (*bytesToRead > 0) {bigstring bs; texttostring(buffer, *bytesToRead, bs); firstword (bs, '\r', bs); convertpstring (bs); wsprintf (TCPmsg, "In ReadStream, read: %s\n", bs); TCPWRITEMSG ();})
	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventReadStream at line %d.  Bytes requested = %ld, read = %ld.\n", __LINE__, *bytesToRead, (unsigned long) res));
	TCPWRITEMSG ();

//	if (res == (int) bytesToRead)
//		return (true);
//
//	return (false);

	*bytesToRead = res;

	return (true);
	} /*fwsNetEventReadStream*/


boolean fwsNetEventWriteStream (unsigned long stream, unsigned long bytesToWrite, char * buffer) {

	/*
	Write to a Stream
	
	5.0.2b3 dmb: if we write fewer bytes than requested, we need to retry, generate error
	*/
	
	SOCKET sock;
	int res, errcode;
//	int len = sizeof (res);

//	struct hostData hostdata;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	TCPTRACKERIN ("fwsNetEventWriteStream", __LINE__, stream);

	sock = sockstack[stream].sockID;
	
	//if (getsockopt  (sock, SOL_SOCKET, SO_SNDBUF, &res, &len) != 0)
	//	;

	releasethreadglobalsnopriority();
	
	res = send (sock, buffer, bytesToWrite, 0);
	
	errcode = WSAGetLastError ();

	grabthreadglobalsnopriority();

	if (res == SOCKET_ERROR) {
		neterror("write stream", errcode);
		return (false);
		}

	if (res < (int) bytesToWrite) {
		neterror("write stream", WSAEWOULDBLOCK);
		return (false);
		}

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventWriteStream at line %d.  Bytes requested = %ld, written = %ld.\n", __LINE__, bytesToWrite, (unsigned long) res));
	TCPWRITEMSG ();

	return (true);
	} /*fwsNetEventWriteStream*/


/*
static unsigned long selects = 0;
static unsigned long accepts = 0;
static unsigned long selecting = 0;
static unsigned long accepting = 0;
static unsigned long avgselect = 0;
static unsigned long avgaccept = 0;
*/
static unsigned long maxlistendepth = 0;

#ifdef FRONTIER_GUSI_2

boolean fwsNetEventCheckAndAcceptSocket () { return (true); }

#else

boolean fwsNetEventCheckAndAcceptSocket () {
	int i;
	int res;
	SOCKET sock;
	fd_set readset;
	struct timeval tv;
	int sasize;
	struct sockaddr_in sa;
	SOCKET acceptsock;
	long listenstream, newstream;
	long err;
	boolean fl = true;
#ifndef	ACCEPT_CONN_WITHOUT_GLOBALS
	bigstring bs;
#endif
	
	if (!frontierWinSockLoaded)
		return (false);

#ifndef FRONTIER_GUSI_2	
	wakecompletedthreads ();
#endif

//	unsigned long ticks;

	//We could set all the sockets at one time, but I think that is harder to manage.
	//Let's just loop through each listen socket and see if it is ready.

	for (i = 0; i < sockListenCount; i++) {
		listenstream = sockListenList[i];

		if (sockstack[listenstream].currentListenDepth <= (sockstack[listenstream].maxdepth)) {
			sock = sockstack[listenstream].sockID;

			tv.tv_sec = 0;
			tv.tv_usec = 0;

			FD_ZERO (&readset);

			FD_SET(sock, &readset);

			/* now check for data */

		//	ticks = gettickcount ();
			
			res = select (sock+1, &readset, NULL, NULL, &tv);
			
		//	selecting += gettickcount () - ticks;
		//	avgselect = selecting / ++selects;

			if (res == 1) { /* we have a live one...*/

				/*Grab socket record for child socket*/
				
				if (!addsockrecord (&newstream)) {
					/* if an error - pass this on to callback */
					long err = WSAEMFILE;
					
					#ifdef ACCEPT_CONN_WITHOUT_GLOBALS
						TCPERRORprintf (wsprintf(TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d.  Error %ld addding new socket record %s (%ld, %ld).\n", __LINE__, stringbaseaddress (sockstack[listenstream].callback), err * -1L, sockstack[listenstream].refcon));
						TCPWRITEMSG();

						fl = fwsruncallback (listenstream, err * -1L, sockstack[listenstream].refcon);
					#else
						parsecallbackstring (listenstream, err * -1L, sockstack[listenstream].refcon, bs);;

						TCPERRORprintf (wsprintf(TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d.  Error addding new socket record %s.\n", __LINE__, stringbaseaddress(bs)));
						TCPERRORWRITEMSG();
						
						fl = fwsrunstring (bs);
					#endif
					
					continue;
					}

				/*Accept connection*/

				sasize = sizeof(sa);
				
			//	ticks = gettickcount ();
				
				acceptsock = accept (sock, (struct sockaddr *)&sa, &sasize);
				
			//	accepting += gettickcount () - ticks;
			//	avgaccept = accepting / ++accepts;
				
				if (acceptsock != INVALID_SOCKET) {
#ifdef PIKE
					if (incrementconnectioncounter ()) {
#endif
						/*sucessful connection -  assign this */
						++sockstack[listenstream].currentListenDepth;
					
						maxlistendepth = max (maxlistendepth, sockstack[listenstream].currentListenDepth);
					
						/*Add Socket to list*/
						sockstack[newstream].refcon = 0;
						sockstack[newstream].sockID = acceptsock;
						sockstack[newstream].typeID = SOCKTYPE_OPEN;
						sockstack[newstream].listenReference = listenstream;

						/*Pass "stream" and "refcon" to callback*/
						
						#ifdef ACCEPT_CONN_WITHOUT_GLOBALS
							TCPprintf (wsprintf(TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d.  Accepted new socket %ld, callback %s (%ld, %ld).\n", __LINE__, acceptsock, stringbaseaddress (sockstack[listenstream].callback), newstream, sockstack[listenstream].refcon));
							TCPWRITEMSG();

							fl = fwsruncallback (listenstream, newstream, sockstack[listenstream].refcon);
						#else
							parsecallbackstring (listenstream, newstream, sockstack[listenstream].refcon, bs);

							TCPprintf (sprintf(TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d.  Accepted new socket %ld, stream %ld callback message %s.\n", __LINE__, (unsigned long) acceptsock, (unsigned long) newstream, stringbaseaddress(bs)));
							TCPWRITEMSG();
							
							fl = fwsrunstring (bs);
						#endif
						
						--i; // back up so we'll check this socket again
#ifdef PIKE
						}
					else {

						struct linger l;

						l.l_onoff = 1;
						l.l_linger = 0;

						TCPERRORprintf (wsprintf(TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d.  Exceeded number of maximum connections: %ld of %ld.\n", __LINE__, ctconnections, maxconnections));
						TCPERRORWRITEMSG();

						setsockopt (acceptsock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof(l));

						fl = closesocket (acceptsock);

						sockstack[newstream].typeID = SOCKTYPE_INVALID;
						}
#endif
					}
				else
					{
					/* if an error - pass this on to callback */
					err = WSAGetLastError();

					#ifdef ACCEPT_CONN_WITHOUT_GLOBALS
						TCPERRORprintf (wsprintf(TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d.  Error processing accept message %s (%ld, %ld).\n", __LINE__, stringbaseaddress (sockstack[listenstream].callback), err * -1L, sockstack[listenstream].refcon));
						TCPWRITEMSG();

						fl = fwsruncallback (listenstream, err * -1L, sockstack[listenstream].refcon);
					#else
						parsecallbackstring (listenstream, err * -1L, sockstack[listenstream].refcon, bs);

						TCPERRORprintf (wsprintf(TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d.  Error processing accept message %s.\n", __LINE__, stringbaseaddress(bs)));
						TCPERRORWRITEMSG();
						
						fl = fwsrunstring (bs);
					#endif
					
					if (err == ENOTCONN) {
					
						// 6.0b2 dmb: this code isn't right. the listens keep succeeding, 
						// chewing up resources until they're gone; accepts still fail
						/*
						if (listen (sock, 5) == SOCKET_ERROR) {
						
							neterror("setup listen stream", WSAGetLastError ());
							
							fwsNetEventCloseListen (listenstream);
							//closesocket (sock);
							}
						*/
						}
					sockstack[newstream].typeID = SOCKTYPE_INVALID;
					}
				}
			else if (res == SOCKET_ERROR) {
				//The select on that socket had an error - What to do??
				}
			}
		}

	return (fl);
	} /*fwsNetEventCheckAndAcceptSocket*/
#endif



boolean fwsNetEventListenStream (unsigned long port, long depth, bigstring callback, unsigned long refcon, unsigned long * stream, unsigned long ipaddr, long hdatabase) {

	/* Set up a listner on a port */

	SOCKET sock;
	SOCKADDR_IN addr;
	int errcode;
	long streamref;
#ifdef ACCEPT_CONN_WITHOUT_GLOBALS
	Handle hcallbacktree = nil;
#endif
	
	nullterminate (callback);
	TCPprintf(wsprintf(TCPmsg, "Entering fwsNetEventListenStream at line %d. Port = %ld, Depth = %ld, Refcon = %ld, Callback = %s.\n", __LINE__, port, depth, refcon, stringbaseaddress(callback)));
	TCPWRITEMSG ();

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	if (!addsockrecord (&streamref)) {
		neterror ("initialize listen stream", WSAEMFILE); /*Too many open sockets*/
		return (false);
		}		

#ifdef ACCEPT_CONN_WITHOUT_GLOBALS
	if (!fwsgetcallbackcodetree (callback, &hcallbacktree))
		return (false);
#endif

	releasethreadglobalsnopriority();

	sock = socket(PF_INET, SOCK_STREAM, 0);

	TCPprintf (wsprintf(TCPmsg, "In fwsNetEventListenStream at line %d.  socket call result is sock = %ld.\n", __LINE__, (long)sock));
	TCPWRITEMSG ();

	if (sock == INVALID_SOCKET) {
		errcode = WSAGetLastError ();
		grabthreadglobalsnopriority();
		neterror("create listen stream", errcode);
		sockstack[streamref].typeID = SOCKTYPE_INACTIVE;
		disposehandle (hcallbacktree);
		return (false);
		}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ipaddr);
	addr.sin_port = htons((unsigned short) port);

    if (bind(sock, (LPSOCKADDR)&addr, sizeof (addr)) == SOCKET_ERROR) {
		errcode = WSAGetLastError ();
	  	grabthreadglobalsnopriority();
		neterror("bind listen stream", errcode);
		closesocket (sock);
		sockstack[streamref].typeID = SOCKTYPE_INACTIVE;
		disposehandle (hcallbacktree);
		return (false);
		} 

	if (listen (sock, SOMAXCONN) == SOCKET_ERROR) {
		errcode = WSAGetLastError ();
	  	grabthreadglobalsnopriority();
		neterror("setup listen stream", errcode);
		closesocket (sock);
		sockstack[streamref].typeID = SOCKTYPE_INACTIVE;
		disposehandle (hcallbacktree);
		return (false);
		}

	grabthreadglobalsnopriority();
	
	sockstack[streamref].refcon = refcon;

	sockstack[streamref].sockID = sock;

	sockstack[streamref].typeID = SOCKTYPE_LISTENING;

	copystring (callback, sockstack[streamref].callback);

	sockstack[streamref].maxdepth = depth;

	sockstack[streamref].listenReference = 0; //6.2a14 AR: was refcon, but probably doesn't make a difference (yet!)

	sockstack[streamref].currentListenDepth = 0;
	
	sockstack[streamref].hcallbacktree = hcallbacktree;
	

	*stream = streamref;

		sockListenList[sockListenCount++] = *stream;

	#ifdef ACCEPT_IN_SEPARATE_THREAD
		sockstack[streamref].hdatabase = (hdldatabaserecord) hdatabase; /*hdldatabaserecord of the db that contains the daemon script*/

		if (!fwslaunchacceptingthread (streamref)) {
			closesocket (sock);
			sockstack[streamref].typeID = SOCKTYPE_INACTIVE;
			disposehandle (hcallbacktree);
			return (false);
			}
	#else
	#endif

	TCPTRACKEROUT ("fwsNetEventListenStream", __LINE__, *stream);
	return (true);

	} /*fwsNetEventListenStream*/


boolean fwsNetEventStatusStream (unsigned long stream, bigstring status, unsigned long * bytesPending) {

	/* get the status of a stream */

	int res;
	SOCKET sock;
	fd_set readset;
	struct timeval tv;
//	struct hostData hostdata;

//#if (TCPTRACKER == 1)
	if (!inmainthread())
		TCPTRACKERIN ("fwsNetEventStatusStream", __LINE__, stream);
//#endif

	*bytesPending = 0;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	sock = sockstack[stream].sockID;

	switch (sockstack[stream].typeID) {
		case SOCKTYPE_INVALID:
			intneterror(INTNETERROR_INVALIDSTREAM);
			copyctopstring ("INACTIVE", status);
			return (false);

		case SOCKTYPE_UNKNOWN:
			copyctopstring ("UNKNOWN", status);
			break;

		case SOCKTYPE_OPEN:
			copyctopstring ("OPEN", status);

			tv.tv_sec = 0;
			tv.tv_usec = 0;

			FD_ZERO (&readset);

			FD_SET(sock, &readset);

			/* now check for data */

			res = select (sock+1, &readset, NULL, NULL, &tv);

			if (res == SOCKET_ERROR) {
				neterror("check status on stream", WSAGetLastError ());
					copyctopstring ("INACTIVE", status);
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
				return (false);
				}

			if (res == 1) {  /*this means that the socket we sent has data */
				res = ioctlsocket (sock, FIONREAD, bytesPending);

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventStatusStream at line %d, ioctlsocket returned %ld with the number of bytes pending as %ld.\n", __LINE__, res, *bytesPending));
				TCPWRITEMSG ();

				if (res == SOCKET_ERROR) {
					neterror("check status on stream", WSAGetLastError ());
					copyctopstring ("INACTIVE", status);
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					return (false);
					}

				if (*bytesPending == 0)
					{
					copyctopstring ("INACTIVE", status);

					/* this condition means that the virtual socket has been closed */

					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					}
				else
					copyctopstring ("DATA", status);
				}

			break;

		case SOCKTYPE_LISTENING:
			copyctopstring ("LISTENING", status);
			break;

		case SOCKTYPE_CLOSED:
			copyctopstring ("CLOSED", status);
			break;

		case SOCKTYPE_LISTENSTOPPED:
			copyctopstring ("STOPPED", status);
			break;

		case SOCKTYPE_INACTIVE:
			copyctopstring ("INACTIVE", status);
			break;

		default:
			copyctopstring ("UNKNOWN", status);
			break;
		}

	nullterminate (status);

//#if (TCPTRACKER == 1)
	if (!inmainthread()) {
		TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventStatusStream at line %d, result is %s with bytes pending %ld.\n", __LINE__, stringbaseaddress(status), *bytesPending));
		TCPWRITEMSG ();
		}
//#endif

	return (true);
	} /*fwsNetEventStatusStream*/


boolean fwsNetEventGetPeerAddress (unsigned long stream, unsigned long * peeraddress, unsigned long * peerport) {
	SOCKADDR_IN sockAddr;
	SOCKET sock;
	int nSockAddrLen;
	int res;

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}
	
	TCPTRACKERIN ("fwsNetEventGetPeerAddress", __LINE__, stream);

	sock = sockstack[stream].sockID;
	
	memset(&sockAddr, 0, sizeof(sockAddr));

	nSockAddrLen = sizeof(sockAddr);

	res = getpeername(sock, (SOCKADDR*)&sockAddr, &nSockAddrLen);

	if (res == SOCKET_ERROR) {
		neterror("get peer address", WSAGetLastError());
		return (false);
		}

	*peerport = (unsigned long) ntohs(sockAddr.sin_port);
	*peeraddress = ntohl(sockAddr.sin_addr.s_addr);

	TCPTRACKEROUT ("fwsNetEventGetPeerAddress", __LINE__, stream);

	return (true);
	} /*fwsNetEventGetPeerAddress*/


boolean fwsNetEventReadStreamUntil (unsigned long stream, Handle hbuffer, Handle hpattern, unsigned long timeoutsecs) {

	/*
	6.1d1 AR: Read from a stream appending to hbuffer until hpattern is found.

	6.1d13 AR: Only grab globals when growing the handle (for proper out of memory errors).

	6.1b9 AR: Check the whole buffer for the pattern -- not only the part we read.
	*/

	SOCKET sock;
	fd_set readset;
	struct timeval tv;
	int res, errcode;
	long ix = gethandlesize (hbuffer);
	long ixstart = ix;
	unsigned long bytes;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventReadStreamUntil", __LINE__, stream);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	sock = sockstack[stream].sockID;

	if (sock == INVALID_SOCKET) {
		neterror ("read stream", WSAENOTSOCK);
		return (false);
		}

	releasethreadglobalsnopriority();

	tv.tv_sec = timeoutsecs;
	tv.tv_usec = 0;

	FD_ZERO (&readset);

	FD_SET (sock, &readset); /*as long as we're dealing with a single socket,
							 doing this just once should be totally safe.*/

	while (searchhandle (hbuffer, hpattern, 0, ix) == -1L) {

		res = select (sock+1, &readset, NULL, NULL, &tv);

		switch (res) {

			case 1:  /*this means that the socket has data */
				res = ioctlsocket (sock, FIONREAD, &bytes);

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamUntil at line %d, ioctlsocket returned %ld with the number of bytes pending as %ld.\n", __LINE__, res, bytes));
				TCPWRITEMSG ();

				if (res == SOCKET_ERROR) {
					errcode = WSAGetLastError ();
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					goto exit;
					}

				if (bytes == 0) { /*closed prematurely*/
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					goto error_closedprematurely;
					}
								
				grabthreadglobalsnopriority(); /*we need the globals because sethandlesize might throw an error*/

				if (!sethandlesize (hbuffer, ix + bytes))
					return (false); /*we have the thread globals, so we can return immediately*/
				
				releasethreadglobalsnopriority();

				lockhandle (hbuffer);

				res = recv (sock, &((*hbuffer) [ix]), bytes, 0);
				
				unlockhandle (hbuffer);
				
				if (res == SOCKET_ERROR) {
					errcode = WSAGetLastError ();
					goto exit;
					}
								
				ix += res;

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamUntil at line %d, requested reading %ld bytes, read %ld bytes.\n", __LINE__, bytes, res));
				TCPWRITEMSG ();

				break;
		
			case 0: /*select timed out*/
				errcode = WSAETIMEDOUT;
				goto exit;

			case SOCKET_ERROR:
				errcode = WSAGetLastError ();
				sockstack[stream].typeID = SOCKTYPE_INACTIVE;
				goto exit;

			default:
				errcode = -1; /* should never happen*/
				goto exit;
			}
		}

	grabthreadglobalsnopriority();

	if (!sethandlesize (hbuffer, ix))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventReadStreamUntil at line %d.  Total bytes read = %ld.\n", __LINE__, ix - ixstart));
	TCPWRITEMSG ();

	return (true);

exit:
	grabthreadglobalsnopriority();

	if (!sethandlesize (hbuffer, ix))
		return (false);

	neterror ("read stream", errcode);

	return (false);

error_closedprematurely:
	grabthreadglobalsnopriority();

	if (!sethandlesize (hbuffer, ix))
		return (false);

	plainneterror (STR_P_ERROR_CLOSED_PREMATURELY);

	return (false);
	} /*fwsNetEventReadStreamUntil*/



boolean fwsNetEventReadStreamUntilClosed (unsigned long stream, Handle hbuffer, unsigned long timeoutsecs) {

	/*
	6.1b15 AR: Branched from fwsNetEventReadStreamUntil. Don't look for a pattern, just read until closed.
	*/

	SOCKET sock;
	fd_set readset;
	struct timeval tv;
	int res, errcode;
	long ix = gethandlesize (hbuffer);
	long ixstart = ix;
	unsigned long bytes;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventReadStreamUntil", __LINE__, stream);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	sock = sockstack[stream].sockID;

	if (sock == INVALID_SOCKET) {
		neterror ("read stream", WSAENOTSOCK);
		return (false);
		}

	releasethreadglobalsnopriority();

	tv.tv_sec = timeoutsecs;
	tv.tv_usec = 0;

	FD_ZERO (&readset);

	FD_SET (sock, &readset); /*as long as we're dealing with a single socket,
							 doing this just once should be totally safe.*/

	while (true) {

		res = select (sock+1, &readset, NULL, NULL, &tv);

		switch (res) {

			case 1:  /*this means that the socket has data */
				res = ioctlsocket (sock, FIONREAD, &bytes);

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamUntilClosed at line %d, ioctlsocket returned %ld with the number of bytes pending as %ld.\n", __LINE__, res, bytes));
				TCPWRITEMSG ();

				if (res == SOCKET_ERROR) {
					errcode = WSAGetLastError ();
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					goto exit;
					}

				if (bytes == 0) { /*we're done*/
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					goto done;
					}
								
				grabthreadglobalsnopriority(); /*we need the globals because sethandlesize might throw an error*/

				if (!sethandlesize (hbuffer, ix + bytes))
					return (false); /*we have the thread globals, so we can return immediately*/
				
				releasethreadglobalsnopriority();

				lockhandle (hbuffer);

				res = recv (sock, &((*hbuffer) [ix]), bytes, 0);
				
				unlockhandle (hbuffer);
				
				if (res == SOCKET_ERROR) {
					errcode = WSAGetLastError ();
					goto exit;
					}
								
				ix += res;

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamUntilClosed at line %d, requested reading %ld bytes, read %ld bytes.\n", __LINE__, bytes, res));
				TCPWRITEMSG ();

				break;
		
			case 0: /*select timed out*/
				errcode = WSAETIMEDOUT;
				goto exit;

			case SOCKET_ERROR:
				errcode = WSAGetLastError ();
				sockstack[stream].typeID = SOCKTYPE_INACTIVE;
				goto exit;

			default:
				errcode = -1; /* should never happen*/
				goto exit;
			}
		}

done:
	grabthreadglobalsnopriority();

	if (!sethandlesize (hbuffer, ix))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventReadStreamUntilClosed at line %d.  Total bytes read = %ld.\n", __LINE__, ix - ixstart));
	TCPWRITEMSG ();

	return (true);

exit:
	grabthreadglobalsnopriority();

	if (!sethandlesize (hbuffer, ix))
		return (false);

	neterror ("read stream", errcode);

	return (false);
	}/*fwsNetEventReadStreamUntilClosed*/


boolean fwsNetEventReadStreamBytes (unsigned long stream, Handle hbuffer, long ctbytes, unsigned long timeoutsecs) {

	/*
	6.1d1 AR: Read the specified number of bytes from the stream appending to hbuffer.
	
	6.1b9 AR: ctbytes applies to the whole buffer -- not only the part we read.
	*/

	SOCKET sock;
	fd_set readset;
	struct timeval tv;
	int res;
	long lenbuffer = gethandlesize (hbuffer);
	long ix = lenbuffer;
	long bytes;
	long errcode;

	if (ctbytes <= lenbuffer) /*6.1b8 AR: don't do anything*/
		return (true);

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventReadStreamBytes", __LINE__, stream);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	if (!sethandlesize (hbuffer, ctbytes)) //make enough room in one go
		return (false);
				
	sock = sockstack[stream].sockID;

	if (sock == INVALID_SOCKET) {
		neterror ("read stream", WSAENOTSOCK);
		return (false);
		}

	releasethreadglobalsnopriority();	//from now on, don't touch lang globals before
										//having called grabthreadglobalsnopriority
	tv.tv_sec = timeoutsecs;
	tv.tv_usec = 0;

	FD_ZERO (&readset);

	FD_SET (sock, &readset);

	lockhandle (hbuffer);

	while (ix < ctbytes) {

		res = select (sock+1, &readset, NULL, NULL, &tv);

		switch (res) {

			case 1:  /*this means that the socket has data */
				res = ioctlsocket (sock, FIONREAD, &bytes);

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamBytes at line %d, ioctlsocket returned %ld with the number of bytes pending as %ld.\n", __LINE__, res, bytes));
				TCPWRITEMSG ();

				if (res == SOCKET_ERROR) {
					errcode = WSAGetLastError ();
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					goto exit;
					}

				if (bytes == 0) { /*closed prematurely*/
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					goto error_closedprematurely;
					}
								
				if (bytes > (ctbytes - ix)) /* Don't read more than ctbytes */
					bytes = ctbytes - ix;				
				
				res = recv (sock, &((*hbuffer) [ix]), bytes, 0);
				
				if (res == SOCKET_ERROR) { /* might be WSAEMSGSIZE if we read less data than is available? */
					errcode = WSAGetLastError ();
					goto exit;
					}
				
				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamBytes at line %d, requested reading %ld bytes, read %ld bytes.\n", __LINE__, bytes, res));
				TCPWRITEMSG ();

				ix += res;

				break;
		
			case 0: /*select timed out*/
				errcode = WSAETIMEDOUT;
				goto exit;

			case SOCKET_ERROR:
				errcode = WSAGetLastError ();
				sockstack[stream].typeID = SOCKTYPE_INACTIVE;
				goto exit;

			default:
				errcode = -1;
				//assert (false);
				goto exit;
			}
		}

	grabthreadglobalsnopriority();

	unlockhandle (hbuffer);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventReadStreamBytes at line %d.  Total bytes requested = %ld, bytes read = %ld.\n", __LINE__, ctbytes, ix));
	TCPWRITEMSG ();
	
	return (true);

exit:
	grabthreadglobalsnopriority();

	neterror("read stream", errcode);

	unlockhandle (hbuffer);

	sethandlesize (hbuffer, ix); /*shrinking, can't fail*/

	return (false);

error_closedprematurely:
	grabthreadglobalsnopriority();

	plainneterror (STR_P_ERROR_CLOSED_PREMATURELY);

	unlockhandle (hbuffer);

	sethandlesize (hbuffer, ix); /*shrinking, can't fail*/

	return (false);
	} /*fwsNetEventReadStreamBytes*/


boolean fwsNetEventWriteHandleToStream (unsigned long stream, Handle hbuffer, unsigned long chunksize, unsigned long timeoutsecs) {

	/* Write to stream in chunks */

	SOCKET sock;
	fd_set writeset;
	struct timeval tv;
	int res;
	long ix = 0;
	unsigned long bytesremaining = gethandlesize (hbuffer);
	long bytestowrite;
	long errcode;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventWriteHandleToStream", __LINE__, stream);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	sock = sockstack[stream].sockID;

	if (sock == INVALID_SOCKET) {
		neterror ("write stream", WSAENOTSOCK);
		return (false);
		}

	tv.tv_sec = timeoutsecs;
	tv.tv_usec = 0;

	FD_ZERO (&writeset);

	FD_SET (sock, &writeset);

	lockhandle (hbuffer);

	releasethreadglobalsnopriority();	//from now on, don't touch lang globals before
										//having called grabthreadglobalsnopriority
	do {

		res = select (sock+1, NULL, &writeset, NULL, &tv);

		switch (res) {

			case 1:  /*this means that we can write to the socket */
				bytestowrite = (bytesremaining < chunksize) ? bytesremaining : chunksize;

				res = send (sock, &((*hbuffer) [ix]), bytestowrite, 0);
				
				if (res == SOCKET_ERROR) {
				
					errcode = WSAGetLastError ();


					goto exit; /*unconditionally throw a script error*/
					}
					
			continue_send:

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventWriteHandleToStream at line %d, requested writing %ld bytes, wrote %ld bytes.\n", __LINE__, bytestowrite, res));
				TCPWRITEMSG ();

				bytesremaining -= res;
				
				ix += res;
				
				break;
		
			case 0: /*select timed out*/
				errcode = WSAETIMEDOUT;
				goto exit;

			case SOCKET_ERROR:
				errcode = WSAGetLastError ();
				sockstack[stream].typeID = SOCKTYPE_INACTIVE;
				goto exit;

			default: /*should never happen*/
				errcode = -1;
				//assert (false);
				goto exit;
			}
	} while (bytesremaining > 0);

	grabthreadglobalsnopriority();
	
	unlockhandle (hbuffer);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventWriteHandleToStream at line %d.  Total bytes written = %ld.\n", __LINE__, ix));
	TCPWRITEMSG ();

	return (true);

exit:
	grabthreadglobalsnopriority();

	neterror("write stream", errcode);

	unlockhandle (hbuffer);

	return (false);
	} /*fwsNetEventWriteHandleToStream*/


//#if 0

static boolean fwstransmitfile (unsigned long stream, ptrfilespec fs) {

	char *buffer;
	static const long kFileBufferSize = 32767L;
	boolean fl = false;
	hdlfilenum fnum;
	long ctread = 0;
	long ix = 0;

	/* open file */

	if (!openfile (fs, &fnum, true))
		return (false);

	/* allocate buffer */

	buffer = malloc (kFileBufferSize);

	if (buffer == nil) {
		memoryerror ();
		goto exit;
		}
		
	while (true) {

		/* read from file */
		
		if (!filesetposition (fnum, ix))
			goto exit;

		if (!filereaddata (fnum, kFileBufferSize, &ctread, buffer))
			goto exit;

		if (ctread == 0) {
			fl = true;
			goto exit;
			}
		
		ix += ctread;

		/* write to stream */

		if(!fwsNetEventWriteStream (stream, ctread, buffer))
			goto exit;
		}

exit:
	/* free buffer */

	if (buffer != nil)
		free (buffer);

	/* close file */

	fl = closefile (fnum);

	return (fl);
	}/*fwstransmitfile*/


boolean fwsNetEventWriteFileToStream (unsigned long stream, Handle hprefix, Handle hsuffix, ptrfilespec fs) {

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventWriteFileToStream", __LINE__, stream);


	if (hprefix != nil) {
		boolean fl;

		lockhandle (hprefix);

		fl = fwsNetEventWriteStream (stream, gethandlesize (hprefix), *hprefix);

		unlockhandle (hprefix);

		if (!fl)
			return (false);
		}

	if (!fwstransmitfile (stream, fs))
		return (false);

	if (hsuffix != nil) {
		boolean fl;

		lockhandle (hsuffix);

		fl = fwsNetEventWriteStream (stream, gethandlesize (hsuffix), *hsuffix);

		unlockhandle (hsuffix);

		if (!fl)
			return (false);
		}

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventWriteFileToStream at line %d.\n", __LINE__));
	TCPWRITEMSG ();

	return (true); 
	} /*fwsNetEventWriteFileToStream*/

//#endif

/*
loop { //wait for data to read
	status = tcp.statusStream (stream, @bytespending);
	if (bytespending > 0) and (status == "DATA") {
		paramTable.request = paramTable.request + tcp.readStream (stream, bytespending);
		tcTimeout = clock.ticks () + 30}; //Now give only 1/2 second to get more data
	if status == "CLOSED" {
		break};
	if status == "INACTIVE" {
		break};
	if clock.ticks () > tcTimeout {
		break};
	if status == "OPEN" {
		sys.systemTask (); //yield a little within this thread for the system
		thread.sleepfor (1)}}} //yield this thread for a second to allow normal Frontier processing
*/


boolean fwsNetEventInetdRead (unsigned long stream, Handle hbuffer, unsigned long timeoutsecs) {

	/*
	6.1b2 AR: Wait timeoutsecs seconds for data to come in. After the first packed has been
	received, continue reading with a reduced timeout of 1 second. (For historical reasons.)
	*/

	SOCKET sock;
	fd_set readset;
	struct timeval tv;
	int res, errcode;
	long ix = gethandlesize (hbuffer);
	long ixstart = ix;
	unsigned long bytes;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventInetdRead", __LINE__, stream);

	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	sock = sockstack[stream].sockID;

	if (sock == INVALID_SOCKET) {
		neterror ("read stream", WSAENOTSOCK);
		return (false);
		}

	releasethreadglobalsnopriority();

	tv.tv_sec = timeoutsecs;
	tv.tv_usec = 0;

	FD_ZERO (&readset);

	FD_SET (sock, &readset); /*as long as we're dealing with a single socket,
							 doing this just once should be totally safe.*/
	while (true) {

		res = select (sock+1, &readset, NULL, NULL, &tv);

		switch (res) {

			case 1:  /*this means that the socket has data */
				res = ioctlsocket (sock, FIONREAD, &bytes);

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventInetdRead at line %d, ioctlsocket returned %ld with the number of bytes pending as %ld.\n", __LINE__, res, bytes));
				TCPWRITEMSG ();

				if (res == SOCKET_ERROR) {
					errcode = WSAGetLastError ();
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					goto exit;
					}

				if (bytes == 0) { /*closed prematurely*/
					sockstack[stream].typeID = SOCKTYPE_INACTIVE;
					goto done; /*not an error in this case*/
					}
								
				grabthreadglobalsnopriority(); /*we need the globals because sethandlesize might throw an error*/

				if (!sethandlesize (hbuffer, ix + bytes))
					return (false); /*we have the thread globals, so we can return immediately*/
				
				releasethreadglobalsnopriority();

				lockhandle (hbuffer);

				res = recv (sock, &((*hbuffer) [ix]), bytes, 0);
				
				unlockhandle (hbuffer);
				
				if (res == SOCKET_ERROR) {
					errcode = WSAGetLastError ();
					goto exit;
					}
								
				ix += res;
				
				tv.tv_sec = 1; /*reduce timeout to one second after the first packet has been received*/

				TCPprintf (wsprintf(TCPmsg, "In fwsNetEventInetdRead at line %d, requested reading %ld bytes, read %ld bytes.\n", __LINE__, bytes, res));
				TCPWRITEMSG ();

				break;
		
			case 0: /*select timed out*/
				goto done; /*not an error in this case*/

			case SOCKET_ERROR:
				errcode = WSAGetLastError ();
				sockstack[stream].typeID = SOCKTYPE_INACTIVE;
				goto exit;

			default:
				errcode = -1; /* should never happen*/
				goto exit;
			}
		}/*while*/

done:
	grabthreadglobalsnopriority();

	if (!sethandlesize (hbuffer, ix))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventInetdRead at line %d.  Total bytes read = %ld.\n", __LINE__, ix - ixstart));
	TCPWRITEMSG ();

	return (true);

exit:
	grabthreadglobalsnopriority();

	neterror ("read stream", errcode);

	return (false);
	} /*fwsNetEventInetdRead*/


boolean fwsNetEventGetStats (unsigned long stream, bigstring bs) {
	
	unsigned long ctopen = 0;
	unsigned long ctdata = 0;
	unsigned long ctclosed = 0;
	unsigned long ctinactive = 0;
	unsigned long i;
	
	setemptystring (bs);
	
	for (i = 1; i <= FRONTIER_MAX_STREAM; i++) {
		
		if (sockstack[i].listenReference == (long) stream) {
			
			switch (sockstack[i].typeID) {
					
				case SOCKTYPE_OPEN:
					ctopen++;
					break;
					
				case SOCKTYPE_DATA:
					ctdata++;
					break;
					
				case SOCKTYPE_CLOSED:
					ctclosed++;
					break;
					
				case SOCKTYPE_INACTIVE:
					ctinactive++;
					break;
				}/*switch*/
			}
		}/*for*/

	pushlong (ctopen, bs);

	pushchar (',', bs);

	pushlong (ctdata, bs);

	pushchar (',', bs);

	pushlong (ctclosed, bs);

	pushchar (',', bs);

	pushlong (ctinactive, bs);

	return (true);
	}/*fwsNetEventGetStats*/

