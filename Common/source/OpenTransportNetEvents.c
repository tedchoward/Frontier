
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


static unsigned char * xtierrorstrings [] = {
	"",									 					/* 3149 */
	"\x1B" "A Bad address was specified",	 				/* 3150 */
	"\x1A" "A Bad option was specified",	 				/* 3151 */
	"\x19" "Missing access permission",	 					/* 3152 */
	"\x16" "Bad provider reference",	 					/* 3153 */
	"\x18" "No address was specified",	 					/* 3154 */
	"\x1A" "Call issued in wrong state",	 				/* 3155 */
	"\x21" "Sequence specified does not exist",	 			/* 3156 */
	"\x17" "A system error occurred",						/* 3157 */
	"\x21" "An event occurred - call OTLook()",				/* 3158 */
	"\x27" "An illegal amount of data was specified",		/* 3159 */
	"\x1C" "Passed buffer not big enough",					/* 3160 */
	"\x1B" "Provider is flow-controlled",					/* 3161 */
	"\x1D" "No data available for reading",					/* 3162 */
	"\x22" "No disconnect indication available",			/* 3163 */
	"\x27" "No Unit Data Error indication available",		/* 3164 */
	"\x1D" "A Bad flag value was supplied",					/* 3165 */
	"\x27" "No orderly release indication available",		/* 3166 */
	"\x18" "Command is not supported",						/* 3167 */
	"\x23" "State is changing - try again later",			/* 3168 */
	"\x28" "Bad structure type requested for OTAlloc",		/* 3169 */
	"\x1A" "Host not found (DNS error)",					/* 3170 */
	"\x29" "A Bind to an in-use address with qlen > 0",		/* 3171 */
	"\x23" "Address requested is already in use",			/* 3172 */
	"\x27" "Accept failed because of pending listen",		/* 3173 */
	"\x28" "Tried to accept on incompatible endpoint",		/* 3174 */
	"",														/* 3175 */
	"",														/* 3176 */
	"",														/* 3177 */
	"\x26" "An unspecified provider error occurred",		/* 3178 */
	"\x24" "A synchronous call at interrupt time",			/* 3179 */
	"\x19" "The command was cancelled"						/* 3180 */
	};

#if 0

static unsigned char * dnserrorstrings [5] = {
	"",
	"\x1b" "Host not found. (DNS error)",								/* 1 */
	"\x37" "Non-authoritative host not found. (Temporary DNS error)",	/* 2 */
	"\x22" "Non-recoverable error. (DNS error)",						/* 3 */
	"\x39" "Valid name, no data record of requested type. (DNS error)",	/* 4 */
	};
	
#endif

static unsigned char * stdcliberrorstrings [80] = {
	"",
	"\x1b" "Permission denied",						/* 1 */
	"\x37" "No such file or directory",				/* 2 */
	"\x22" "No such resource",						/* 3 */
	"\x39" "Interrupted system service",			/* 4 */
	"\x12" "Input/output error",					/* 5 */
	"\x15" "Device not configured",					/* 6 */
	"\x16" "Argument list too long",				/* 7 */
	"\x11" "Exec format error",						/* 8 */
	"\x13" "Bad file descriptor",					/* 9 */
	"\x12" "No child processes",					/* 10 */
	"\x19" "Resource deadlock avoided",				/* 11 */
	"\x16" "Cannot allocate memory",				/* 12 */
	"\x11" "Permission denied",						/* 13 */
	"\x0b" "Bad address",							/* 14 */
	"\x15" "Block device required",					/* 15 */
	"\x0b" "Device busy",							/* 16 */
	"\x0b" "File exists",							/* 17 */
	"\x11" "Cross-device link",						/* 18 */
	"\x21" "Operation not supported by device",		/* 19 */
	"\x0f" "Not a directory",						/* 20 */
	"\x0e" "Is a directory",						/* 21 */
	"\x10" "Invalid argument",				/* 22 */
	"\x1d" "Too many open files in system",	/* 23 */
	"\x15" "Too many open sockets",			/* 24 */
	"\x1e" "Inappropriate ioctl for device",	/* 25 */
	"\x0e" "Text file busy",					/* 26 */
	"\x0e" "File too large",					/* 27 */
	"\x17" "No space left on device",		/* 28 */
	"\x0d" "Illegal seek",					/* 29 */
	"\x15" "Read-only file system",			/* 30 */
	"\x0e" "Too many links",					/* 31 */
	"\x0b" "Broken pipe",					/* 32 */
	"",
	"",

/* non-blocking and interrupt i/o */
	"\x20" "Resource temporarily unavailable",	/* 35 */

/* ipc/network software -- argument errors */
	"\x23" "A blocking operation is in progress",			/* 36 */
	"\x20" "Operation is already in progress",		/* 37 */

/* ipc/network software -- argument errors */
	"\x20" "Socket operation on a non-socket",	/* 38 */
	"\x1f" "Destination address is required",		/* 39 */
	"\x10" "Message too long",					/* 40 */
	"\x1e" "Protocol wrong type for socket",	/* 41 */
	"\x16" "Protocol not available",			/* 42 */
	"\x16" "Protocol not supported",			/* 43 */
	"\x19" "Socket type not supported",			/* 44 */
	"\x21" "Operation not supported on socket",	/* 45 */
	"\x1d" "Protocol family not supported",		/* 46 */
	"\x2f" "Address family not supported by protocol family",	/* 47 */
	"\x16" "Address already in use",			/* 48 */
	"\x1e" "Can't assign requested address",	/* 49 */

/* ipc/network software -- operational errors */
	"\x0f" "Network is down",					/* 50 */
	"\x16" "Network is unreachable",			/* 51 */
	"\x23" "Network dropped connection on reset",	/* 52 */
	"\x20" "Software caused connection abort",	/* 53 */
	"\x18" "Connection reset by peer",			/* 54 */
	"\x19" "No buffer space available",			/* 55 */
	"\x1b" "Socket is already connected",		/* 56 */
	"\x17" "Socket is not connected",			/* 57 */
	"\x20" "Can't send after socket shutdown",	/* 58 */
	"\x21" "Too many references: can't splice",	/* 59 */
	"\x14" "Connection timed out",				/* 60 */
	"\x12" "Connection refused",				/* 61 */

	"\x21" "Too many levels of symbolic links",	/* 62 */
	"\x12" "File name too long",				/* 63 */

	"\x0c" "Host is down",						/* 64 */
	"\x10" "No route to host",					/* 65 */
	"\x13" "Directory not empty",				/* 66 */
	"\x12" "Too many processes",				/* 67 */
	"\x0e" "Too many users",					/* 68 */
	"\x13" "Disc quota exceeded",				/* 69 */

/* Network File System */
	"\x15" "Stale NFS file handle",				/* 70 */
	"\x21" "Too many levels of remote in path",	/* 71 */
	"\x11" "RPC struct is bad",					/* 72 */
	"\x11" "RPC version wrong",					/* 73 */
	"\x13" "RPC prog. not avail",				/* 74 */
	"\x15" "Program version wrong",				/* 75 */
	"\x19" "Bad procedure for program",			/* 76 */
	"\x12" "No locks available",				/* 77 */
	"\x18" "Function not implemented",			/* 78 */
	"\x21" "Inappropriate file type or format",	/* 79 */
	}; //tcperrorstrings

#define wsprintf sprintf

#include "mac.h"
#include "error.h"
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
#include "timedate.h"
#include "winsocknetevents.h"
#include "frontierdebug.h"
#include "file.h"


#ifndef OTAssert
	#define OTAssert(name, cond)	((cond) ? ((void) 0) : (DebugStr( __FILE__ ": " #name ": " #cond )))
#endif

#define NO_HOST_SERVICES NULL

#define SOCKTYPE_INVALID -1
#define SOCKTYPE_UNKNOWN 0
#define SOCKTYPE_OPEN 1
#define SOCKTYPE_DATA 2
#define SOCKTYPE_LISTENING 3
#define SOCKTYPE_CLOSED 4
#define SOCKTYPE_LISTENSTOPPED 5
#define SOCKTYPE_INACTIVE 6

#define INTNETERROR_INVALIDSTREAM -1

static const long kPacketSize = 8192L;

typedef short tysocktypeid;

typedef struct tylistenrecord * ListenRecordRef;

typedef struct tyendpointrecord * EndpointRecordRef;

typedef struct tyepstatsrecord {
	long	cttotal;
	long	ctidle;
	long	ctbroken;
	long	ctworking;
	long	ctwaiting;
	} tyepstatsrecord;

/*
	2004-10-28 aradke: OTLink and OTLIFO must be aligned on 32-bit boundaries!
*/
	
typedef struct tylistenrecord {
	EndpointRef			ep;
	OTLink				validationlink;		/* Link into an OT FIFO (not atomic) */
	OTLIFO				idleEPs;			/* Endpoints ready to accept a new incoming connection */
	OTLIFO				brokenEPs;			/* Collect failed endpoints for recycling by worker thread */
	OTLIFO				readyEPs;			/* Ready to be picked up by the worker thread which will spawn a thread to call the daemon */
	OTLIFO				waitingEPs;			/* Endpoints waiting to complete an orderly disconnect */
	UInt8				stateflags;
	OTConfigurationRef	masterconfig;
	EndpointRecordRef	acceptors;
	long				maxdepth;
	long				refcon;
	long				idthread;
	hdldatabaserecord	hdatabase;
	Handle				hcallbacktree;
	tyepstatsrecord		stats;
	tysocktypeid		typeID;
	boolean				fllistenpending;
	bigstring 			callback;
	} tylistenrecord;

typedef struct tyendpointrecord {
	EndpointRef			ep;
	OTLink				link;			/* Used for linking into TCP low-level lists (atomic) */
	OTLink				validationlink; /* Used for linking into worker thread lists (atomic) */
	UInt8				stateflags;
	UInt8				completionflags;
	OSStatus			result;
	tysocktypeid		typeID;
	long				timestamp;		/* System time when we started waiting for an orderly disconnect */
	ListenRecordRef		listener;		/* Nil if not operated by a listener */
	TCall*				sendCall;		/* Not nil if we initiate a connection */
	} tyendpointrecord;

#if TARGET_API_MAC_CARBON == 1
	/*Code change by Timothy Paustian Monday, September 25, 2000 8:34:30 PM
	I added these globals to keep the UPPs for carbon.*/
	ThreadEntryTPP	gThreadEntryCallback;
	OTNotifyUPP		gListenNotifierUPP;
	OTListSearchUPP	gListSearchUPP;
	OTNotifyUPP		gNotifierUPP;
	OTListSearchUPP	gEndListSearchUPP;
	OTNotifyUPP		gDNRNotifierUPP;
#endif

static tyepstatsrecord epstats;

static OTList sListenList;		/* List of valid listen streams to verify against when receiving params from scripts */
static OTList sEndpointList;	/* List of valid active streams to verify against when receiving params from scripts */

static OTLIFO sIdleEPs;			/* Endpoints ready to accept a new incoming connection -- Listeners have their own private list */
static OTLIFO sBrokenEPs;		/* Collect failed endpoints for recycling by main thread -- Listeners have their own private list */
static OTLIFO sWaitingEPs;		/* Endpoints waiting to complete an orderly disconnect */

static OTConfigurationRef sMasterConfig;

static OSType sOTVersionSelector = 'otvr';
static long sOTVersion;

static short frontierWinSockCount = 0;
static boolean frontierWinSockLoaded = false;


enum {
	kBrokenBit						= 0, // Bit numbers in stateflags fields
	kOpenInProgressBit				= 1,
	kPassconBit						= 2,
	kBoundBit						= 3,
	kDontDisposeBit					= 4,
	kWaitingForConnectBit			= 5,
	kWaitingForDisconnectBit		= 6,
	
	kConnectCompleteBit				= 0, // Bit numbers in completionflags fields
	kGetProtAddressBit				= 1,
	kRcvdDisconnectBit				= 2,
	kSntDisconnectBit				= 3,
	kRcvdOrderlyDisconnectBit		= 4,
	kIPReuseAddrBit					= 5,
	kTCPKeepAliveBit				= 6,
	
	kOTVersion111	= 0x01110000, // Minimum version of Open Transport required
	
	kTCPKeepAliveInMinutes				= 1,
	kTCPWaitSecsForConnect				= 30, // seconds allowed for connect to complete, when elapsed we return an OT error (-3260 or -3259)

	kTCPWaitSecsForOrderlyDisconnect	= 60, // seconds allowed for orderly disconnect to complete, when elapsed we initiate an abortive disconnect		
	
	kDontQueueIt						= 0,
	kQueueIt							= 1
	};

//
//	Option structure 
//	
//	This is used to pass down both IP_REUSEADDR and TCP_KEEPALIVE in the
//	same option message
//

struct TKeepAliveOpt {
	UInt32		len;
	OTXTILevel	level;
	OTXTIName	name;
	UInt32		status;
	UInt32		tcpKeepAliveOn;
	UInt32		tcpKeepAliveTimer;
};

typedef struct TKeepAliveOpt TKeepAliveOpt;


/* DNS queries structure */

const unsigned long fcomplete = 1L;

typedef struct {
	OTResult result;
	unsigned long flags;
	} dnsquery;


/* Templates for static functions */

static void InitEndpoint (EndpointRecordRef epref, EndpointRef ep, ListenRecordRef listenref);
static pascal void ReturnEndpoint (EndpointRecordRef epref, OTResult result, int completionbit);
static boolean CheckEndpointList (EndpointRecordRef epref);
static boolean CheckListenList (ListenRecordRef listenref);
static void CheckUnbind (EndpointRecordRef epref, OTResult result, Boolean queueIt);
static OSStatus DoBind (EndpointRecordRef epref);
static void DoRcvDisconnect (EndpointRecordRef epref);
static void EnterRcvDisconnect (EndpointRecordRef epref);
static void DoListenRcvDisconnect (ListenRecordRef listenref);
static void EnterListenAccept (ListenRecordRef listenref);
static OSStatus DoSndOrderlyDisconnect (EndpointRecordRef epref, boolean flrecurse);
//static OSStatus EnterSndOrderlyDisconnect (EndpointRecordRef epref);
static OSStatus DoSndDisconnect (EndpointRecordRef epref);
static OSStatus EnterSndDisconnect (EndpointRecordRef epref);
static void ReadAllAndClose (EndpointRecordRef epref);
//static void DoWaitList (ListenRecordRef listenref);
static pascal void DNRNotifier (void *context, OTEventCode event, OTResult result, void *cookie);
static pascal void ListenNotifier (void *context, OTEventCode code, OTResult result, void *cookie);
static pascal void Notifier (void *context, OTEventCode code, OTResult result, void *cookie);
static OSStatus EPOpen (EndpointRecordRef epref, OTConfigurationRef cfg);
static boolean EPClose (EndpointRecordRef epref);
static void Recycle (ListenRecordRef listenref);

/*Code change by Timothy Paustian Monday, September 25, 2000 8:58:07 PM
needed to add a declaration to get the new carbon stuff to compile.*/
static void *fwsacceptingthreadmain (void *param);


/* TCPTRACKER stuff */

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

#ifdef WIN95VERSION
extern 	DWORD ixthreadglobalsgrabcount;			// Tls index of counter for nest globals grabbing
#endif

static FILE * tcpfile = NULL;
static char TCPmsg[400];

#define TCPprintf(msg) msg
#define TCPERRORprintf(msg) msg

static void TCPWRITEMSG () {
	unsigned long ticks = gettickcount ();
	static unsigned long lastticks = 0;
	#ifdef WIN95VERSION
		DWORD idthread;
		static DWORD idlastthread = 0;
		long grabcount = (long) TlsGetValue (ixthreadglobalsgrabcount);
	#endif
	#ifdef MACVERSION
		static ThreadID idlastthread = 0;
		ThreadID idthread;
	#endif
	
	if (fllogger) {
		if (tcpfile == NULL) {
			tcpfile = fopen ("tcpfile.txt", "w+");
			}
		#ifdef WIN95VERSION
			idthread = GetCurrentThreadId();
		#endif
		#ifdef MACVERSION
			MacGetCurrentThread (&idthread);
		#endif

		if (idthread != idlastthread) {
			fprintf (tcpfile, "\n");
			idlastthread = idthread;
			}

	#ifdef WIN95VERSION
		fprintf (tcpfile, "%08X (%04ld) | %04X (%02ld) | %s\n", (unsigned long) ticks, (ticks - lastticks), idthread, grabcount, TCPmsg);
	#endif

	#ifdef MACVERSION
		fprintf (tcpfile, "%08X (%04ld) | %04X | %s\n", (unsigned long) ticks, (ticks - lastticks), idthread, TCPmsg);
	#endif

		lastticks = ticks;

		fflush (tcpfile);
		}
	} /*TCPWRITEMSG*/

#define TCPERRORWRITEMSG TCPWRITEMSG

static void TCPTRACKERCLOSE () {
	if (fllogger) {
		if (tcpfile != NULL)
			fclose (tcpfile);
		}
	}


#elif (TCPTRACKER == 2)
#pragma message ("*********************** TCPTRACKER is ON: Error output to tcpfile.txt **********************")

static boolean fllogger = true;

#ifdef WIN95VERSION
extern 	DWORD ixthreadglobalsgrabcount;			// Tls index of counter for nest globals grabbing
#endif

static FILE * tcpfile = NULL;
static char TCPmsg[400];

#define TCPprintf(msg)
#define TCPWRITEMSG()

#define TCPERRORprintf(msg) msg

static void TCPERRORWRITEMSG () {
	unsigned long ticks = gettickcount ();
	static unsigned long lastticks = 0;
	#ifdef WIN95VERSION
		DWORD idthread;
		static DWORD idlastthread = 0;
		long grabcount = (long) TlsGetValue (ixthreadglobalsgrabcount);
	#endif
	#ifdef MACVERSION
		long idthread = (long) (**getcurrentthread ()).idthread;
		static long idlastthread = 0;
	#endif
	
	if (fllogger) {
	#ifdef WIN95VERSION
		idthread = GetCurrentThreadId();
	#endif

		if (tcpfile == NULL) {
			tcpfile = fopen ("tcpfile.txt", "w+");
			}

		if (idthread != idlastthread) {
			fprintf (tcpfile, "\n");
			idlastthread = idthread;
			}

	#ifdef WIN95VERSION
		fprintf (tcpfile, "%08X (%04ld) | %04X (%02ld) | %s\n", (unsigned long) ticks, (ticks - lastticks), idthread, grabcount, TCPmsg);
	#endif

	#ifdef MACVERSION
		fprintf (tcpfile, "%08X (%04ld) | %04X | %s\n", (unsigned long) ticks, (ticks - lastticks), idthread, TCPmsg);
	#endif

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

#if (defined (TCPTRACKER) && (TCPTRACKER==1 || TCPTRACKER==3))

static void TCPTRACKERIN (char * functionName, int linenumber, EndpointRecordRef epref, ListenRecordRef listenref) {
	if (fllogger) {
	
		if (epref) {
			if (!CheckEndpointList (epref)) {
				wsprintf (TCPmsg, "Entering %s at line %d, EndpointRecordRef = %08lx - INVALID ENPOINT.", functionName, linenumber, (long) epref);
				TCPWRITEMSG ();
				return;
				}

			wsprintf (TCPmsg, "Entering %s at line %d, EndpointRecordRef = %08lx, Endpoint = %08lx, Type = %s, State Flags = %02x, Completion Flags = %02x, Listen Ref = %08lX.",
				functionName, linenumber, (long) epref, (long) epref->ep, TCPGETTYPE (epref->typeID), epref->stateflags, epref->completionflags, (long) epref->listener);
		
			TCPWRITEMSG ();
			}

		if (listenref) {
			if (!CheckListenList (listenref)) {
				wsprintf (TCPmsg, "Entering %s at line %d, ListenRecordRef = %08lx - INVALID ENPOINT.", functionName, linenumber, (long) listenref);
				TCPWRITEMSG ();
				return;
				}

			wsprintf (TCPmsg, "Entering %s at line %d, ListenRecordRef = %08lx, Endpoint = %08lx, Type = %s, Max Depth = %ld, Refcon = %08lX, Thread = %ld",
				functionName, linenumber, (long) listenref, (long) listenref->ep, TCPGETTYPE (listenref->typeID), listenref->maxdepth, listenref->refcon, listenref->idthread);
		
			TCPWRITEMSG ();
			}
		}
	} /*TCPTRACKERIN*/


static void TCPTRACKEROUT (char * functionName, int linenumber, EndpointRecordRef epref, ListenRecordRef listenref) {
	if (fllogger) {
	
		if (epref) {
			if (!CheckEndpointList (epref)) {
				wsprintf (TCPmsg, "Exiting %s at line %d, EndpointRecordRef = %08lx - INVALID ENPOINT.", functionName, linenumber, (long) epref);
				TCPWRITEMSG ();
				return;
				}

			wsprintf (TCPmsg, "Exiting %s at line %d, EndpointRecordRef = %08lx, Endpoint = %08lx, Type = %s, State Flags = %02x, Completion Flags = %02x, Listen Ref = %08lX.",
				functionName, linenumber, (long) epref, (long) epref->ep, TCPGETTYPE (epref->typeID), epref->stateflags, epref->completionflags, (long) epref->listener);
		
			TCPWRITEMSG ();
			}

		if (listenref) {
			if (!CheckListenList (listenref)) {
				wsprintf (TCPmsg, "Exiting %s at line %d, ListenRecordRef = %08lx - INVALID ENPOINT.", functionName, linenumber, (long) listenref);
				TCPWRITEMSG ();
				return;
				}

			wsprintf (TCPmsg, "Exiting %s at line %d, ListenRecordRef = %08lx, Endpoint = %08lx, Type = %s, Max Depth = %ld, Refcon = %08lX, Thread = %ld",
				functionName, linenumber, (long) listenref, (long) listenref->ep, TCPGETTYPE (listenref->typeID), listenref->maxdepth, listenref->refcon, listenref->idthread);
		
			TCPWRITEMSG ();
			}
		}
	} /*TCPTRACKEROUT*/

#else

#define TCPTRACKERIN(functionName, linenumber, epref, listenref)
#define TCPTRACKEROUT(functionName, linenumber, epref, listenref)

#endif


#ifdef PIKE /*7.1b3 PBS: maxconnections is now variable.*/

	/*extern const*/ /*long maxconnections = 5;*/
	
	long maxconnections = longinfinity; /*7.1b5 PBS: no more connection limit*/

#else

	/*extern const*/ long maxconnections = longinfinity; /*7.0b37 PBS: reported in system.environment table in Frontier*/

#endif

static long ctconnections = 0;


long fwsNetEventGetConnectionCount (void) {

	/*7.0b37 PBS: return current count of TCP connections.
	Used by tcp.countConnections verb.
	*/

	return (ctconnections);
	} /*fwsNetEventGetConnectionCount*/


static boolean incrementconnectioncounter (void) {

	if (ctconnections >= maxconnections)
		return (false);

	ctconnections++;

	return (true);	
	}/*incrementconnectioncounter*/

static void decrementconnectioncounter (void) {
	
	ctconnections--;
	
	if (ctconnections < 0)	/*7.0b53 PBS: a script can abort or close a stream multiple times.*/
							/*Instead we should get this info from the OS, but until then we do this.*/
		ctconnections = 0;

	//assert (ctconnections >= 0);

	}/*decrementconnectioncounter*/
	


//#else
//	#define incrementconnectioncounter() (true)
//	#define decrementconnectioncounter() ((void *)0L);
//#endif


static pascal boolean listendatabasevisit (const void* ref, OTLink* linkToCheck) {
	
	hdldatabaserecord hdatabase = (hdldatabaserecord) ref;

	return (hdatabase == (OTGetLinkObject (linkToCheck, tylistenrecord, validationlink))->hdatabase);
	}/*listendatabasevisit*/


static pascal boolean listenlinkvisit (const void* ref, OTLink* linkToCheck) {
	
	ListenRecordRef listenref = (ListenRecordRef) ref;

	return (listenref == OTGetLinkObject (linkToCheck, tylistenrecord, validationlink));
	}/*listenlinkvisit*/


static pascal boolean endpointlinkvisit (const void* ref, OTLink* linkToCheck) {
	
	EndpointRecordRef epref = (EndpointRecordRef) ref;

	return (epref == OTGetLinkObject (linkToCheck, tyendpointrecord, validationlink));
	}/*endpointlinkvisit*/


static boolean CheckEndpointList (EndpointRecordRef epref) {
	
	OTLink *eplink;
	
	#if TARGET_API_MAC_CARBON == 1
		eplink = OTFindLink (&sEndpointList, gEndListSearchUPP, (void *) epref);
	#else
		eplink = OTFindLink (&sEndpointList, &endpointlinkvisit, (void *) epref);
	#endif
	
	return (nil != eplink);
	}/*CheckEndpointList*/


static boolean CheckListenList (ListenRecordRef listenref) {
	
	OTLink *listenlink;
	
	#if TARGET_API_MAC_CARBON == 1
		listenlink = OTFindLink (&sListenList, gListSearchUPP, (void *) listenref);
	#else
		listenlink = OTFindLink (&sListenList, &listenlinkvisit, (void *) listenref);
	#endif
	
	return (nil != listenlink);
	}/*CheckListenList*/


static void InitEndpoint (EndpointRecordRef epref, EndpointRef ep, ListenRecordRef listenref) {
	
	OTMemzero (epref, sizeof (tyendpointrecord));
	
	epref->ep = ep;
	
	epref->listener = listenref;
	
	epref->typeID = SOCKTYPE_INVALID;

	return;
	}/*InitEndpoint*/


static pascal void ReturnEndpoint (EndpointRecordRef epref, OTResult result, int completionbit) {

	if (OTAtomicTestBit (&epref->stateflags, kDontDisposeBit)) {

		epref->result = result;

		OTAtomicClearBit (&epref->stateflags, kWaitingForConnectBit);

		OTAtomicSetBit (&epref->completionflags, completionbit);
		}
	else {
		boolean flbound = OTAtomicTestBit (&epref->stateflags, kBoundBit);
		
		if (flbound)
			result = OTUnbind (epref->ep);
		
		CheckUnbind (epref, result, !flbound);
		}
	}/*ReturnEndpoint*/


//
//	CheckUnbind
//
//	This routine checks the results of an unbind.   Due to various problems
//	in OpenTransport, an OTUnbind can fail for a number of reasons.  This problem
//	is timing related so you usually won't hit it.   When an OTUnbind fails,
//	we assume the best way to recover is to throw the endpoint on the broken
//	list to be recycled.   Later, in the recycle routine, it will be closed
//	and a new endpoint will be opened to replace it.  If the OTUnbind is
//	successful, the endpoint is put back on the free list to be reused.
//
//	Since the unbind failure is timing related, a more efficient solution
//	would probably be to wait and retry the unbind in a few seconds, 
//	expecting that the call would not fail on the next try.
//
static void CheckUnbind (EndpointRecordRef epref, OTResult result, Boolean queueIt) {
	
	ListenRecordRef listenref = epref->listener;

	if (kOTNoError != result) {
	
		if (0 == OTAtomicSetBit (&epref->stateflags, kBrokenBit)) {
			/*
			The OTAtomicSetBit guarantee's that the EPInfo won't be
			enqueued twice.   We only enqueue the EPInfo if the previous
			state of the bit was 0.
			*/
			if (nil != listenref) {

				OTLIFOEnqueue (&listenref->brokenEPs, &epref->link);
				
				listenref->stats.ctworking--;
				
				listenref->stats.ctbroken++;
				}
			else {
			
				OTLIFOEnqueue (&sBrokenEPs, &epref->link);
				
				epstats.ctworking--;
				
				epstats.ctbroken++;
				}
			}
		}
	else {
		if (queueIt) {
			
			TCP_ASSERT_1 (nil != epref->ep);
		
			if (nil != listenref) {
		
				OTLIFOEnqueue (&listenref->idleEPs, &epref->link);
				
				listenref->stats.ctworking--;
				
				listenref->stats.ctidle++;
				
				if (listenref->fllistenpending)
					EnterListenAccept (listenref);
				}
			else {
			
				#if TARGET_API_MAC_CARBON == 1
				
					EPClose (epref);
					
					epstats.ctworking--;
					
				#else
			
					OTLIFOEnqueue (&sIdleEPs, &epref->link);
				
					epstats.ctworking--;
				
					epstats.ctidle++;
				#endif
				}
			}
		}
	}/*CheckUnbind*/


//
//	DoBind
//
//	This routine requests a wildcard port binding from the transport protocol.
//	Since the program doesn't care what port is returned, it passes in NULL
//	for the bind return parameter.  The bind request structure is ephemeral
//	and can be a local stack variable since OT is done with it when the call returns.
//	The bind is done when the notifier receives a T_BINDCOMPLETE event.
//
static OSStatus DoBind (EndpointRecordRef epref) {

	OSStatus err;
	TBind bindReq;
	InetAddress	inAddr;
	
	/*
	Bind the endpoint to a wildcard address (assign us a port, we don't care which one).
	*/
	OTInitInetAddress(&inAddr, 0, 0);
	
	bindReq.addr.len = sizeof (InetAddress);
	bindReq.addr.buf = (unsigned char *) &inAddr;
	bindReq.qlen = 0;
	
	err = OTBind (epref->ep, &bindReq, NULL);
	
	if (err != kOTNoError)
		TCP_MSG_1 ("DoBind: OTBind returned an error");

	return (err);
	}/*DoBind*/


static OSStatus EnterBind (EndpointRecordRef epref) {

	OSStatus err;
	boolean doLeave = OTEnterNotifier (epref->ep);

	err = DoBind (epref);

	if (doLeave)
		OTLeaveNotifier (epref->ep);
	
	return (err);
	}/*EnterBind*/


static void DoRcvOrderlyDisconnect (EndpointRecordRef epref) {

	OSStatus err;
	
	err = OTRcvOrderlyDisconnect (epref->ep);
	
	if (kOTNoError != err)
		return;
	
	OTAtomicSetBit (&epref->completionflags, kRcvdOrderlyDisconnectBit);
	
	if (!OTAtomicTestBit (&epref->stateflags, kDontDisposeBit))
		if (T_IDLE == OTGetEndpointState (epref->ep))
			CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
	}/*DoRcvOrderlyDisconnect*/


static void EnterRcvOrderlyDisconnect (EndpointRecordRef epref) {

	boolean doLeave = OTEnterNotifier (epref->ep);
	
	DoRcvOrderlyDisconnect (epref);
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);
	}/*EnterRcvOrderlyDisconnect*/



static void DoRcvDisconnect (EndpointRecordRef epref) {

	OSStatus err;
	TDiscon discon;
	
	discon.udata.len = nil;
	
	err = OTRcvDisconnect (epref->ep, &discon);
	
	
	switch (err) {
	
		case kOTNoError:
			epref->result = E2OSStatus (discon.reason);
			OTAtomicSetBit (&epref->completionflags, kRcvdDisconnectBit);
			break;
		
		case kOTBufferOverflowErr:
			epref->result = kECONNREFUSEDErr; /*most likely cause*/
			OTAtomicSetBit (&epref->completionflags, kRcvdDisconnectBit);
			break;
		
		default:
			if (err != kOTNoDisconnectErr)
				TCP_MSG_1 ("DoRcvDisconnect: OTRcvDisconnect on endpoint returned an error");
			epref->result = err;
			break;	
		}
		
	if (!OTAtomicTestBit (&epref->stateflags, kDontDisposeBit))
		CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
	}/*DoRcvDisconnect*/


static void EnterRcvDisconnect (EndpointRecordRef epref) {

	boolean doLeave = OTEnterNotifier (epref->ep);
	
	DoRcvDisconnect (epref);
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);
	}/*EnterRcvDisconnect*/


//
//	DoListenRcvDisconnect
//
//	This routine is called from ListenNotifier in T_LISTEN handling
//	upon getting a kOTLookErr back indicating a T_DISCONNECT needs to be handled.
//
static void DoListenRcvDisconnect (ListenRecordRef listenref) {

	OSStatus err;
	
	err = OTRcvDisconnect (listenref->ep, NULL);
	
	if (kOTNoError != err)
		TCP_MSG_1 ("DoListenRcvDisconnect: OTRcvDisconnect on listener returned an error");
	
	return;
	}/*DoListenRcvDisconnect*/


//
//	DoListenAccept
//
//	The handling of a T_LISTEN is greatly simplified by use
//	of the tilisten module, which serializes inbound connections.
//	This means that when doing an OTAccept we won't get a kOTLookErr
//	because another inbound connection arrived and created a T_LISTEN.
//	Without the tilisten module, we have to use the "8 step 
//	listen/accept/disconnect method", which is documented elsewhere.
//	At this point, if we have a free endpoint, accept the connection.
//	If we don't, assume we are overloaded and reject the connection.
//
//	When we are called from inside the notifier due to a T_LISTEN, 
//	DoListenAccept() is called directly.
//
//	When we restart delayed handling of a T_LISTEN, either because of
//	doing a throttle-back or because the program ran out of free endpoints,
//	EnterListenAccept() is called for synchronization on the listener endpoint.
//

static void DoListenAccept (ListenRecordRef listenref) {

	OTLink* acceptor_link;
	EndpointRecordRef acceptor;
	TCall call;
	InetAddress clientaddr;
	OTResult lookResult;
	OSStatus err;
	
/*	if (!incrementconnectioncounter ()) {
		OTSndDisconnect (listenref->ep, nil); /%6.2b13 AR: reject connection (PIKE) %/
		return;
		}*/
	
	if (!incrementconnectioncounter ()) {
		
		listenref->fllistenpending = true;
		
		return;	
		} /*if*/

	//
	//	Get an EPInfo & endpoint.   If none are available, defer handling the T_LISTEN.
	//
	acceptor_link = OTLIFODequeue (&listenref->idleEPs);
	
	if (acceptor_link == NULL) {
		decrementconnectioncounter();
		listenref->fllistenpending = true;
		return;
		}
	
	listenref->fllistenpending = false;

	listenref->stats.ctidle--;
	
	listenref->stats.ctworking++;
	
	acceptor = OTGetLinkObject (acceptor_link, tyendpointrecord, link);
	
	InitEndpoint (acceptor, acceptor->ep, listenref);
		
	call.addr.maxlen = sizeof(InetAddress);
	call.addr.buf = (unsigned char*) &clientaddr;
	call.opt.maxlen = 0;
	call.opt.buf = NULL;
	call.udata.maxlen = 0;
	call.udata.buf = NULL;
		
	err = OTListen (listenref->ep, &call);

	if (kOTNoError != err) {
		//
		//	Only two errors are expected at this point.
		//	One would be a kOTNoDataErr, indicating the inbound connection
		//	was unavailable, temporarily hidden by a higher priority streams
		//	message, etc.   The more likely error is a kOTLookErr, 
		//	which indicates a T_DISCONNECT on the OTLook()
		//	happens when the call we were going to process disconnected.
		//	In that case, go away and wait for the next T_LISTEN event.
		//
		OTLIFOEnqueue (&listenref->idleEPs, &acceptor->link);
		
		listenref->stats.ctworking--;
		
		listenref->stats.ctidle++;

		if (kOTNoDataErr != err) {
			lookResult = OTLook (listenref->ep);
			
			if (kOTLookErr == err && T_DISCONNECT == lookResult)
				DoListenRcvDisconnect (listenref);
			else	
				TCP_MSG_1 ("Notifier: T_LISTEN - OTListen returned an error");
			}
						
		decrementconnectioncounter();
		
		return;	
		}
	
	err = OTAccept (listenref->ep, acceptor->ep, &call);
	
	if (kOTNoError != err) {
		//
		//	Again, we have to be able to handle the connection being disconnected
		//	while we were trying to accept it.
		//
		OTLIFOEnqueue (&listenref->idleEPs, &acceptor->link);
		
		listenref->stats.ctworking--;
		
		listenref->stats.ctidle++;
		
		lookResult = OTLook (listenref->ep);
		
		if (kOTLookErr == err && T_DISCONNECT == lookResult)
			DoListenRcvDisconnect (listenref);
		else
			TCP_MSG_1 ("Notifier: T_LISTEN - OTAccept returned an error");

		decrementconnectioncounter();
		}
	}/*DoListenAcceept*/


//
//	EnterListenAccept
//
//	This is a front end to DoListenAccept() which is used whenever 
//	it is not being called from inside the listener endpoint's notifier.
//	We do this for synchronization.   If we were processing an OTListen()
//	or an OTAccept() and we were interrupted at the listener endpoint's
//	notifier with a T_LISTEN, etc, it would be inconvenient and would require
//	some more sophisticated synchronization code to deal with the problem.
//	The easy way to avoid this is to do an OTEnterNotifier() on the listener's
//	endpoint.   
//
//	Important note - doing OTEnterNotifier on one endpoint only prevents that
//	endpoint's notifier for interrupting us.   Since the same notifier code
//	is used for lots of endpoints here, remember that the one endpoint's 
//	notifier can interrupt another.   Doing an OTEnterNotifier() on the
//	listener endpoint prevents the listener from interrupting us, but it
//	does not prevent the Notifier() routine from interrupting us via 
//	another endpoint which also uses the same routine.
//
//	Important note #2 - Don't ever do an OTEnterNotifier on an acceptor endpoint
//	before doing the OTAccept().   This confuses OT and creates problems.
//
static void EnterListenAccept (ListenRecordRef listenref) {

	boolean doLeave = OTEnterNotifier (listenref->ep);
	
	DoListenAccept (listenref);

	if (doLeave)
		OTLeaveNotifier(listenref->ep);
	}


//
//	DoSndOrderlyDisconnect
//
//	This routine is a front end to OTSndOrderlyDisconnect(). It's only called
//	from fwsNetEventCloseStream but not from the Notifier.
//
static OSStatus DoSndOrderlyDisconnect (EndpointRecordRef epref, boolean flrecurse) {

	OSStatus err;

	/* Indicate to the notifier that it's now responsible for recycling this endpoint */

	err = OTSndOrderlyDisconnect (epref->ep);
	
	if (kOTNoError != err) {
		
		if (kOTLookErr == err) {
			
			OTResult result = OTLook (epref->ep);
			
			switch (result) {
				
				case T_DISCONNECT:
					DoRcvDisconnect (epref);
					break;
				
				case T_ORDREL:
					DoRcvOrderlyDisconnect (epref);
					if (flrecurse)
						err = DoSndOrderlyDisconnect (epref, false); // try again, but only once
					break;
				
				default: {
					TCP_MSG_1 ("DoSndOrderlyDisconnect: OTLook returned an unhandled event");
					CheckUnbind (epref, kOTLookErr, kDontQueueIt);
					}
				}
			}
		else {
			TCP_MSG_1 ("DoSndOrderlyDisconnect: OTSndOrderlyDisconnect returned an error");
			CheckUnbind (epref, err, kDontQueueIt);
			}
		}
	else
		ReadAllAndClose (epref);

	return (err);
	}/*DoSndOrderlyDisconnect*/

#if 0

static OSStatus EnterSndOrderlyDisconnect (EndpointRecordRef epref) {

	OSStatus err;
	boolean doLeave = OTEnterNotifier (epref->ep);

	err = DoSndOrderlyDisconnect (epref);
		
	if (doLeave)
		OTLeaveNotifier(epref->ep);

	return (err);
	}/*EnterSndOrderlyDisconnect*/

#endif

//
//	DoSndDisconnect
//
//	This routine is a front end to OTSndDisconnect().  It's only called from
//	EnterSndDisconnect and DoSndOrderlyDisconnect but not from the Notifier.
//  We give up control of the endpoint at this point.  The Notifier is supposed to do the clean up.
//
static OSStatus DoSndDisconnect (EndpointRecordRef epref) {

	OSStatus err;

	/* Indicate to the notifier that it's now responsible for recycling this endpoint */
	
	OTAtomicClearBit (&epref->stateflags, kDontDisposeBit);
	
	/* Initiate disconnect */

	err = OTSndDisconnect (epref->ep, nil);
	
	if (kOTNoError != err) {
	
		if (kOTLookErr == err) {
			
			OTResult result = OTLook (epref->ep);
			
			switch (result) {
				
				case T_DISCONNECT:
					DoRcvDisconnect (epref);
					break;
				
				default:
					TCP_MSG_1 ("DoSndDisconnect: OTLook returned an unhandled event");
					CheckUnbind (epref, kOTLookErr, kDontQueueIt);
					break;
				}
			}
		else {
			CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
			TCP_MSG_1 ("DoSndDisconnect: OTSndOrderlyDisconnect returned an error");
			}
		}
	
	return (err);
	}/*DoSndDisconnect*/


//
//	EnterSndDisconnect
//
//	A thin wrapper around DoSndDisconnect.
//	Called from fwsNetEventAbortStream to initiate an abortive disconnect.
//
static OSStatus EnterSndDisconnect (EndpointRecordRef epref) {
	
	OSStatus err;
	boolean doLeave = OTEnterNotifier (epref->ep);
	
	err = DoSndDisconnect (epref);
	
	if (doLeave)
		OTLeaveNotifier(epref->ep);
	
	return (err);
	}/*EnterSndDisconnect*/


//
//	ReadAndClose:
//
//	This routine attempts to read all available data from an endpoint.
//	Check the endpoint state to see if there's any data left to be read.
//	If so, read until we get an error -- most likely a kOTNoDataErr.
//	
//	Then check the endpoint state to see if we are in T_IDLE.  If so,
//	the connection is fully broken down and we can unbind and requeue
//	the endpoint for reuse.
//
//	Otherwise, the notifier will eventually get a T_ORDREL, T_DISCONNECT,
//	or another T_DATA event and take care of the rest.
//
static void ReadAllAndClose (EndpointRecordRef epref) {

	OTResult epstate = OTGetEndpointState (epref->ep);
	
	switch (epstate) {
		
		case T_INREL:
			DoRcvOrderlyDisconnect (epref);
			return;
	
		case T_DATAXFER: {
			char buffer[kPacketSize];
			OTResult result;
			OTFlags junkFlags;
			
			do {
				result = OTRcv (epref->ep, buffer, kPacketSize, &junkFlags);
				} while (result >= 0);
		
			epstate = OTGetEndpointState (epref->ep);
			}
		}/*switch*/

	if (T_IDLE == epstate && 0 == OTAtomicTestBit (&epref->stateflags, kDontDisposeBit)) {
		CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
		}
	}/*ReadAllAndClose*/


static pascal void DNRNotifier (void *context, OTEventCode event, OTResult result, void *cookie) {

	#pragma unused (cookie)
	
	switch (event) {

		case T_DNRADDRTONAMECOMPLETE:
		case T_DNRSTRINGTOADDRCOMPLETE: {
			dnsquery *q = (dnsquery *) context;
			
			q->result = result;
			
			q->flags |= fcomplete;
			
			break;
			}
		}

	return;
	}/*dnsnotifier*/

static pascal void ListenNotifier (void* context, OTEventCode code, OTResult result, void* cookie) {

	#pragma unused (cookie)
	
	ListenRecordRef listenref = (ListenRecordRef) context;
		
	switch (code) {

		/*
		T_DISCONNECT:
		
		Call DoListenRcvDisconnect to do all the work
		*/
		case T_DISCONNECT: {
			DoListenRcvDisconnect (listenref);
			return;
			}

		/*
		T_DISCONNECTCOMPLETE:
		*/
		case T_DISCONNECTCOMPLETE: {
			if (kOTNoError != result)
				TCP_MSG_1 ("ListenNotifier: T_DISCONNECT_COMPLETE returned an error");
			return;
			}
		
		/*
		T_LISTEN:
		
		Call DoListenAccept() to do all the work.
		*/
		case T_LISTEN: {
			DoListenAccept (listenref);
			break;
			}

		/*
		T_ACCEPTCOMPLETE:

		This event is received by the listener endpoint only.   
		The acceptor endpoint will get a T_PASSCON event instead.
		*/
		case T_ACCEPTCOMPLETE:
			OTAssert ("ListenNotifier: T_ACCEPTCOMPLETE returned an error.", result == kOTNoError);
			return;
			
		/*
		default:

		There are events which we don't handle, but we don't expect to see
		any of them.   When running in debugging mode while developing a program,
		we exit with an informational alert.   Later, in the production version
		of the program, we ignore the event and try to keep running.
		*/
		default: {
			TCP_MSG_1 ("ListenNotifier: unknown event");
			return;
			}
			
		}/*switch (event)*/
		
	}/*ListenNotifier*/

static OSStatus SetIpReuseAddrOption (EndpointRecordRef epref) {
		
	TOptMgmt 		optReq;
	TOption			opt;

	optReq.flags = T_NEGOTIATE;
	optReq.opt.len = kOTFourByteOptionSize;
	optReq.opt.buf = (unsigned char *) &opt;
	
	opt.len = sizeof(TOption);
	opt.level = INET_IP;
	opt.name = kIP_REUSEADDR;
	opt.status = 0;
	opt.value[0] = 1;
	
	return (OTOptionManagement (epref->ep, &optReq, nil));
	}/*SetIpReuseAddrOption*/


static OSStatus SetTcpKeepAliveOption (EndpointRecordRef epref) {

	TOptMgmt 			optReq;
	TKeepAliveOpt		opt;

	optReq.flags			= T_NEGOTIATE;
	optReq.opt.len			= sizeof (TKeepAliveOpt);
	optReq.opt.buf			= (unsigned char *) &opt;
	
	opt.len					= sizeof (TKeepAliveOpt);
	opt.level				= INET_TCP;
	opt.name				= TCP_KEEPALIVE;
	opt.status				= 0;
	opt.tcpKeepAliveOn		= 1;
	opt.tcpKeepAliveTimer	= kTCPKeepAliveInMinutes;	
	
	return (OTOptionManagement (epref->ep, &optReq, nil));
	}/*SetTcpKeepAliveOption*/


static OSStatus SetTcpConnAbortThresholdOption (EndpointRecordRef epref) {

	TOptMgmt 		optReq;
	TOption			opt;

	optReq.flags = T_NEGOTIATE;
	optReq.opt.len = kOTFourByteOptionSize;
	optReq.opt.buf = (unsigned char *) &opt;
	
	opt.len = sizeof(TOption);
	opt.level = INET_TCP;
	opt.name = TCP_CONN_ABORT_THRESHOLD;
	opt.status = 0;
	opt.value[0] = kTCPWaitSecsForConnect * 1000; /*milliseconds*/
	
	return (OTOptionManagement (epref->ep, &optReq, nil));
	}/*SetTcpConnAbortThresholdOption*/
	

static pascal void Notifier (void* context, OTEventCode code, OTResult result, void* cookie) {

	/*
	2002-10-30 AR: Fixed a possible connection leak in the T_PASSCON handler
	where we neglected to return the endpoint to the broken queue
	in case of an error.
	*/
	
	EndpointRecordRef epref = (EndpointRecordRef) context;
	OSStatus err = noErr;
		
	switch (code) {

		/*
		T_OPENCOMPLETE:
		
		This event occurs when an OTAsyncOpenEndpoint() completes.   Note that this event,
		just like any other async call made from outside the notifier, can occur during
		the call to OTAsyncOpenEndpoint().  That is, in the main thread the program did
		the OTAsyncOpenEndpoint(), and the notifier is invoked before control is returned
		to the line of code following the call to OTAsyncOpenEndpoint().   This is one
		event we need to keep track of even if we are shutting down the program since there
		is no way to cancel outstanding OTAsyncOpenEndpoint() calls.
		*/
		case T_OPENCOMPLETE: {
			
			/* Clear the OpenInProgress bit and check result of OTAsyncOpenEndpoint */
				
			OTAtomicClearBit (&epref->stateflags, kOpenInProgressBit);
			
			if (kOTNoError == result)
				epref->ep = (EndpointRef) cookie;
			else {
				TCP_MSG_1 ("Notifier: T_OPENCOMPLETE returned an error");
				ReturnEndpoint (epref, result, kConnectCompleteBit);
				return;
				}

			/* We should insert a test here to break out if we're in the process of shutting down the listener or the app */
			
			/* Set endpoint to blocking mode */
			
			err = OTSetBlocking (epref->ep);
			
			if (kOTNoError != err) {
				TCP_MSG_1 ("Notifier: T_OPENCOMPLETE - OTSetBlocking returned an error");
				ReturnEndpoint (epref, err, kConnectCompleteBit);
				return;
				}
						
			/* Option Management - Turn on ip_reuseaddr so we don't have port conflicts in general. */
			
			if (kOTNoError != SetIpReuseAddrOption (epref)) {
				TCP_MSG_2 ("Notifier: T_OPENCOMPLETE - OTOptionManagement returned an error");
				ReturnEndpoint (epref, err, kConnectCompleteBit);
				return;
				}
			
			/* Code path resumes at T_OPTMGMTCOMPLETE */
			
			return;
			}

		/*
		T_OPTMGMTCOMPLETE:
		
		An OTOptionManagement() call has completed.  We set the ip_reuseaddr option
		for all endpoints right after they have been opened. We set the TCP_KEEPALIVE
		option for all endpoints every time we accept or intiate a new connection.
		We also set the TCP_CONN_ABORT_THRESHOLD option for all active endpoints
		whenever we we initiate a new connection.
		*/
		case T_OPTMGMTCOMPLETE: {

			ListenRecordRef	listenref = epref->listener;
			
			if (result != kOTNoError) {
				TCP_MSG_2 ("Notifier: T_OPTMGMTCOMPLETE returned an error");
				ReturnEndpoint (epref, result, kConnectCompleteBit);
				return;
				}
			
			if (nil != listenref) {

				/*
				This is the branch where we deal with passive endpoints that are used by
				the listener to accept incoming connections. If we just opened the endpoint,
				the T_OPENCOMPLETE handler just negotiated the ip_reuseaddr option.
				In this case, the next thing to do is to add the endpoint to the list of idle endpoints.
				If there is a connection pending, we accept it right away.
				*/
				if (0 == OTAtomicSetBit (&epref->completionflags, kIPReuseAddrBit)) {

					OTLIFOEnqueue (&listenref->idleEPs, &epref->link);
					
					listenref->stats.ctworking--;

					listenref->stats.ctidle++;
				
					if (listenref->fllistenpending)
						EnterListenAccept (listenref);
					
					return;
					}
				
				/*
				If we get here, the T_PASSCON handler just negotiated the TCP_KEEPALIVE option.
				After setting the appropriate completion flag, we are ready to go to work.
				*/
				if (0 == OTAtomicSetBit (&epref->completionflags, kTCPKeepAliveBit)) {

					OTAtomicSetBit (&epref->stateflags, kDontDisposeBit);

					OTLIFOEnqueue (&epref->listener->readyEPs, &epref->link);
					
					return;
					}
				
				/*
				We should never ever manage to get here, so post a debug message.
				*/
				TCP_MSG_1 ("Notifier: T_OPTMGMTCOMPLETE got triggered unexpectedly");
				return;
				}
			
			else {				

				/*
				This is the branch where we deal with active endpoints. If we just opened the endpoint,
				the last thing we did was to negotiate the ip_reuseaddr option. In this case,
				the next thing to do is to bind the endpoint if it was opened by fwsOpenStream,
				or to add it to the list of idle endpoints if it was re-opened by Recycle.
				*/
				if (0 == OTAtomicSetBit (&epref->completionflags, kIPReuseAddrBit)) {

					if (nil != epref->sendCall) {

						/*
						The endpoint was just opened by fwsOpenStream. The next thing to do is to bind it.
						If the DoBind call succeeds, the code path will resume at T_BINDCOMPLETE.
						*/
						err = DoBind (epref); 
						
						if (kOTNoError != err) {
							TCP_MSG_2 ("Notifier: T_OPTMGMTCOMPLETE - DoBind returned an error");
							ReturnEndpoint (epref, err, kConnectCompleteBit);
							}
						}
					else {
						
						/*
						The endpoint was just re-opened by Recycle. Now it's ready to go on the list of idle endpoints.
						Eventually, it will be dequeued by fwsOpenStream in order to be bound again.
						If that DoBind call succeeds, the code path will resume at T_BINDCOMPLETE.
						*/
						OTLIFOEnqueue (&sIdleEPs, &epref->link);
						
						epstats.ctworking--;
						
						epstats.ctidle++;
						}
					
					return;
					}
				
				/*
				If the TCP_KEEPALIVE flag hasn't been set yet, T_BINDCOMPLETE just negotiated the TCP_KEEPALIVE option.
				In that case, the next thing to do is to also negotiate the TCP_CONN_ABORT_THRESHOLD option.
				*/
				if (0 == OTAtomicSetBit (&epref->completionflags, kTCPKeepAliveBit)) {
					
					if (kOTNoError != SetTcpConnAbortThresholdOption (epref)) {
						TCP_MSG_2 ("Notifier: T_OPTMGMTCOMPLETE - OTOptionManagement returned an error");
						ReturnEndpoint (epref, err, kConnectCompleteBit);
						}
					
					return;
					}
				
				/*
				We get here if both the TCP_KEEPALIVE and TCP_CONN_ABORT_THRESHOLD options have been negotiated.
				Now we are ready to connect to the peer.
				*/
				err = OTConnect (epref->ep, epref->sendCall, nil);
				
				if (err != kOTNoDataErr) {
					/* anything other than kOTNoDataErr means that a connection has not been initiated */
					TCP_MSG_1 ("Notifier: T_OPTMGMTCOMPLETE - OTConnect returned an error");
					ReturnEndpoint (epref, err, kConnectCompleteBit);
					return;
					}
				/*
				If OTConnect returned the expected kOTNoDataErr, the code path will resume at T_CONNECT.
				*/
				}

			return;
			}

		/*
		T_PASSCON:

		This event happens on the accepting endpoint, not the listening endpoint.
		At this point the connection is fully established and we can begin the
		process of downloading data.  Note that due to a problem in OT it is 
		possible for a T_DATA to beat a T_PASSCON to the notifier.  When this
		happens we note it in the T_DATA case and then start processing the 
		data here.
		*/
		case T_PASSCON: {
					
			if (kOTNoError != result) {
				TCP_MSG_1 ("Notifier: T_PASSCON returned an error");
				decrementconnectioncounter();
				ReturnEndpoint (epref, result, kConnectCompleteBit); /* 2002-10-30 AR: don't leak endpoint */
				return;
				}	
			
			if (0 != OTAtomicSetBit (&epref->stateflags, kPassconBit)) {

				/*
				A T_DATA previously beat the T_PASSCON to our notifier.
				Here we help OT out by having deferred data processing until now.
				*/
				if (OTAtomicTestBit (&epref->stateflags, kWaitingForDisconnectBit)) {
					ReadAllAndClose (epref);
					return;
					}				
				}
			
			/*
			Before we can get to work, we need to negotiate the TCP_KEEPALIVE option.
			First, we set the ip_reuseaddr bit to make sure that
			the T_OPTMGMTCOMPLETE handler does the right thing.
			*/
			OTAtomicSetBit (&epref->completionflags, kIPReuseAddrBit);
			
			if (kOTNoError != SetTcpKeepAliveOption (epref)) {
				TCP_MSG_2 ("Notifier: T_PASSCON - OTOptionManagement returned an error");
				ReturnEndpoint (epref, err, kConnectCompleteBit);
				}

			/*
			If SetTcpKeepAliveOption succeeded, the code path will resume at T_OPTMGMTCOMPLETE.
			*/
			return;
			}

		/*
		T_BINDCOMPLETE:

		This event is returned when an endpoint has been bound to a wildcard address.
		No errors are expected. We should only get here if the endpoint belongs
		to an active connection, initiated by fwsOpenStream rather than by a listener.
		We set the bound bit and the ip_reuseaddr bit so that the Notifier will do the right thing.
		Then we negotiate the TCP_KEEPALIVE option.
		*/
		case T_BINDCOMPLETE: {
		
			if (kOTNoError != result) {
				TCP_MSG_1 ("Notifier: T_BINDCOMPLETE returned an error");
				ReturnEndpoint (epref, result, kConnectCompleteBit);
				return;
				}
			
			OTAtomicSetBit (&epref->stateflags, kBoundBit); /* so we know to unbind... */
			
			OTAtomicSetBit (&epref->completionflags, kIPReuseAddrBit); /* make sure we do the right thing in the notifier*/
			
			if (kOTNoError != SetTcpKeepAliveOption (epref)) {
				TCP_MSG_2 ("Notifier: T_BINDCOMPLETE - OTOptionManagement returned an error");
				ReturnEndpoint (epref, err, kConnectCompleteBit);
				}
			
			/*
			If SetTcpKeepAliveOption succeeded, the code path will resume at T_OPTMGMTCOMPLETE.
			*/
			return;
			}
		
		/*
		T_CONNECT:

		This event is returned when a connection is established to the server.
		The program must call OTRcvConnect() to get the connection information
		and clear the T_CONNECT event from the stream.  Since OTRcvConnect()
		returns immediately (rather than via a completion event to the notifier)
		we can use local stack structures for parameters.
		*/
		case T_CONNECT: {
					
			if (result != kOTNoError) {
				TCP_MSG_1 ("Notifier: T_CONNECT returned an error");
				ReturnEndpoint (epref, result, kConnectCompleteBit);
				return;
				}
			
			err = OTRcvConnect (epref->ep, nil);

			if (err != kOTNoError)
				TCP_MSG_1 ("Notifier: T_CONNECT - OTRcvConnect returned an error");
			
			/* We're done, one way or another... */
			
			ReturnEndpoint (epref, err, kConnectCompleteBit);
			
			return;
			}

		/*
		T_DATA:

		The main rule for processing T_DATA's is to remember that once you have
		a T_DATA, you won't get another one until you have read to a kOTNoDataErr.
		The advanced rule is to remember that you could get another T_DATA
		during an OTRcv() which will eventually return kOTNoDataErr, presenting
		the application with a synchronization issue to be most careful about.
		*/
		case T_DATA: {
			/*
			Here we work around a small OpenTransport bug.
			It turns out, since this program does almost everything from inside the notifier,
			that during a T_UNBINDCOMPLETE we can put an EPInfo back into the idle list.
			If that notification is interrupted by a T_LISTEN at the notifier, we could
			end up starting a new connection on the endpoint before OT unwinds the stack
			out of the code which delivered the T_UNBINDCOMPLETE.   OT has some specific
			code to protect against a T_DATA arriving before the T_PASSCON, but in this
			case it gets confused and the events arrive out of order.   If we try to
			do an OTRcv() at this point we will get a kOTStateChangeErr because the endpoint
			is still locked by the earlier OTAccept call until the T_PASSCON is delivered
			to us.   This is fairly benign and can be worked around easily.  What we do
			is note that the T_PASSCON hasn't arrived yet and defer the call to ReadData()
			until it does.
			*/
			if (0 != OTAtomicSetBit (&epref->stateflags, kPassconBit)) {
				/*
				Because are are running completely inside notifiers,
				it is possible for a T_DATA to beat a T_PASSCON to us.
				We need to help OT out when this occurs and defer the
				data read until the T_PASSCON arrives.
				*/				
				if (OTAtomicTestBit (&epref->stateflags, kWaitingForDisconnectBit))
					ReadAllAndClose (epref);				
				}
			return;
			}
		
		/*
		T_GODATA:
		*/
		case T_GODATA: {
			return;
			}

		/*
		T_ORDREL:

		This event occurs when an orderly release has been received on the stream.
		*/
		case T_ORDREL: {
			
			DoRcvOrderlyDisconnect (epref);
						
			return;
			}
		
		/*
		T_DISCONNECT:
		
		Call DoRcvDisconnect to do all the work
		*/
		case T_DISCONNECT: {
			/*
			If the kWaitingForConnectBit is set, it means that the owning thread
			sits in a loop in fwsOpenStream, waiting for the connection request
			to complete. However, the peer didn't accept the connection, so we receive
			the disconnection request and return the reason for the disconnect
			to the owning thread in the epref->result field by calling DoRcvDisconnect.
			*/
			
			if (OTAtomicTestBit (&epref->stateflags, kDontDisposeBit))
				if (!OTAtomicTestBit (&epref->stateflags, kWaitingForConnectBit)
					&& !OTAtomicTestBit (&epref->stateflags, kWaitingForDisconnectBit)) {	
						
					/*
					Deal with it in the worker thread.  We will get a kOTLookErr
					when we try to read or write.  In that case we call OTLook,
					and proceed to process the event.
					*/
					return;
					}
		
			DoRcvDisconnect (epref);
			
			if (OTAtomicTestBit (&epref->stateflags, kWaitingForConnectBit)) {
			
				OTAtomicClearBit (&epref->stateflags, kWaitingForConnectBit);
				
				OTAtomicSetBit (&epref->completionflags, kConnectCompleteBit);
				}
				
			return;
			}

		/*
		T_DISCONNECTCOMPLETE:
		*/
		case T_DISCONNECTCOMPLETE: {
			
			TCP_ASSERT_1 (!OTAtomicTestBit (&epref->stateflags, kDontDisposeBit));
				
			if (result != kOTNoError)
				TCP_MSG_1 ("Notifier: T_DISCONNECT_COMPLETE returned an error");
			
			CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
			
			return;
			}

		/*
		T_UNBINDCOMPLETE:

		This event occurs on completion of an OTUnbind().
		The endpoint is ready for reuse on a new inbound connection.
		Put it back into the queue of idle endpoints.
		Note that the OTLIFO structure has atomic queue and dequeue,
		which can be helpful for synchronization protection.  
		*/
		case T_UNBINDCOMPLETE: {
			
			TCP_ASSERT_1 (!OTAtomicTestBit (&epref->stateflags, kDontDisposeBit));
		
			if (kOTNoError == result)
				OTAtomicClearBit (&epref->stateflags, kBoundBit);
				
			CheckUnbind (epref, result, kQueueIt);
			
			return;
			}
		
		/*
		T_GETPROTADDRCOMPLETE:
		
		GetProtAddress call initiated by fwsNetEventsGetPeerAddress is complete,
		so go ahead and set the completion flag.
		*/
		case T_GETPROTADDRCOMPLETE: {
			OTAtomicSetBit (&epref->completionflags, kGetProtAddressBit);
			return;
			}
			
		/*
		default:

		There are events which we don't handle, but we don't expect to see
		any of them.   When running in debugging mode while developing a program,
		we exit with an informational alert.   Later, in the production version
		of the program, we ignore the event and try to keep running.
		*/
		default: {
			//char str[256];
			//sprintf (str, "Notifier: unknown event (%08lx)", code);
			//TCP_MSG_1 (str);
			return;
			}
			
		}/*switch (code)*/
		
	}/*Notifier*/

//
//	EPOpen:
//
//	A front end to OTAsyncOpenEndpoint.
//	A status bit is set so we know there is an open in progress.
//	It is cleared when the notifier gets a T_OPENCOMPLETE where the context
//	pointer is this EPInfo.  Until that happens, this EPInfo can't be cleaned
//	up and released.
//
static OSStatus EPOpen (EndpointRecordRef epref, OTConfigurationRef cfg) {

	OSStatus err;
	
	//
	//	Clear all old state bits and set the open in progress bit.
	//	This doesn't need to be done atomicly because we are 
	//	single threaded on this endpoint at this point.
	//
	
	OTAtomicSetBit (&epref->stateflags, kOpenInProgressBit);
	
	#if TARGET_API_MAC_CARBON == 1
	err = OTAsyncOpenEndpointInContext (cfg, 0, nil, gNotifierUPP, epref, nil); //see OpenTransport.h line 1939
	
	#else	
	err = OTAsyncOpenEndpoint (cfg, 0, nil, &Notifier, epref);
	#endif
		
	if (kOTNoError != err)
		OTAtomicClearBit (&epref->stateflags, kOpenInProgressBit);
	
	return (err);
	}/*EPOpen*/


//
//	EPClose
//
//	This routine is a front end to OTCloseProvider.   Centralizing closing of
//	endpoints makes debugging and instrumentation easier.   Also, since this
//	program uses Ack Sends to avoid data copies when doing OTSnd(), some special
//	care is required at close time.   
//
static boolean EPClose (EndpointRecordRef epref) {

	OSStatus err;
	
	/*
	If an endpoint is still being opened, we can't close it yet.
	There is no way to cancel an OTAsyncOpenEndpoint, so we just
	have to wait for the T_OPENCOMPLETE event at the notifier.
	*/
	if (0 != OTAtomicTestBit (&epref->stateflags, kOpenInProgressBit))
		return (false);
	
	/*
	If the OTAsyncOpenEndpoint failed, the endpoint ref will be NULL,
	and we don't need to close it now.
	*/
	if (nil == epref->ep)
		return true;
	
	err = OTCloseProvider (epref->ep);
	
	epref->ep = NULL;
		
	if (kOTNoError != noErr)
		TCP_MSG_1 ("EPClose: OTCloseProvider returned an error");

	return (true);
	}/*EPClose*/


//
//	Recycle:
//
//	This routine shouldn't be necessary, but it is helpful to work around both
//	problems in OpenTransport and bugs in this program.   Basically, whenever an
//	unexpected error occurs which shouldn't be fatal to the program, the EPInfo
//	is queued on the BrokenEP queue.  When recycle is called, once per pass around
//	the event loop, it will attempt to close the associated endpoint and open
//	a new one to replace it using the same EPInfo structure.   This process of
//	closing an errant endpoint and opening a replacement is probably the most
//	reliable way to make sure that this program and OpenTransport can recover
//	from unexpected happenings in a clean manner.
//
static void Recycle (ListenRecordRef listenref) {

	OTLink* nomad;
	OTLIFO* brokenlist;
	EndpointRecordRef epref;
	OTConfigurationRef cfg;

	if (nil != listenref) {
		brokenlist = &listenref->brokenEPs;
		cfg = listenref->masterconfig;
		}
	else {
		brokenlist = &sBrokenEPs;
		cfg = sMasterConfig;
		}
	
	nomad = OTLIFOStealList (brokenlist);
	
	while (nil != nomad) {
		
		epref = OTGetLinkObject (nomad, tyendpointrecord, link);
		
		nomad = nomad->fNext;
		
		if (!EPClose (epref)) {

			OTLIFOEnqueue (brokenlist, &epref->link);
							
			continue;
			}

		OTAtomicClearBit (&epref->stateflags, kBrokenBit);
		
		InitEndpoint (epref, nil, listenref);

		if (kOTNoError != EPOpen (epref, OTCloneConfiguration (cfg))) {
		
			OTLIFOEnqueue (brokenlist, &epref->link);
		
			continue;
			}
		
		if (nil != listenref) {
			listenref->stats.ctbroken--;
			
			listenref->stats.ctworking++;
			}
		else {
			epstats.ctbroken--;
			
			epstats.ctworking++;
			}
		}/*while*/
	}/*Recycle*/


static void ProcessWaitingEndpoints (ListenRecordRef listenref) {

	boolean doLeave;
	OTLink* nomad;
	EndpointRecordRef epref;
	OTLIFO* waitinglist = (listenref != nil) ? &listenref->waitingEPs : &sWaitingEPs;
	
	nomad = OTLIFOStealList (waitinglist);
	
	while (nil != nomad) {
		
		epref = OTGetLinkObject (nomad, tyendpointrecord, link);
		
		nomad = nomad->fNext;
		
		doLeave = OTEnterNotifier (epref->ep);
		
		/*bundle*/ {
		
			if (listenref != nil) { /*required to keep the stats accurate if CheckUnbind is called*/
				listenref->stats.ctwaiting--;
				listenref->stats.ctworking++;
				}
			else {
				epstats.ctwaiting--;
				epstats.ctworking++;
				}
			
			if (T_IDLE == OTGetEndpointState (epref->ep)) {
				
				OTAtomicClearBit (&epref->stateflags, kDontDisposeBit); /*pass on responsibility to the Notifier*/
				
				CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
				}
			else if (epref->timestamp + kTCPWaitSecsForOrderlyDisconnect < timenow())
				DoSndDisconnect (epref);
			else {
			
				if (listenref != nil) {
					listenref->stats.ctworking--;
					listenref->stats.ctwaiting++;
					}
				else {
					epstats.ctworking--;
					epstats.ctwaiting++;
					}

				OTLIFOEnqueue (waitinglist, &epref->link); /*try again later*/
				}
			}
		
		if (doLeave)
			OTLeaveNotifier (epref->ep);
		}/*while*/

	}/*ProcessWaitingEndpoints*/


static void gettcperrorstring (int errcode, bigstring bs) {

	int ixtcperr = errcode;
		
	if (IsEError (errcode)) {
	
		ixtcperr = OSStatus2E (errcode);
		
		copystring (stdcliberrorstrings [ixtcperr], bs); //handles nil source
		
		if (!isemptystring (bs)) {
			
			pushchar ('.', bs);

			return;
			}
		}
	
	if (IsXTIError (errcode)) {
	
		ixtcperr = OSStatus2XTI (errcode);
		
		copystring (xtierrorstrings [ixtcperr], bs); //handles nil source
		
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
	
	TCPERRORprintf (wsprintf(TCPmsg, "NET ERROR - %s", stringbaseaddress(errbs)));
	TCPERRORWRITEMSG ();
	
	langerrormessage (bs);
	} /*neterror*/


static void closedunexpectedlyerror (char * cannot) {

	/*
	6.2b17 AR
	*/

	bigstring bs;
	char prestring[256];
	
	wsprintf (prestring, "Can't %s because the TCP connection was closed unexpectedly.", cannot);
	copyctopstring (prestring, bs);

	plainneterror (bs);
	} /*neterror*/


static void neterror (char * cannot, long errcode) {

	bigstring bs;
	bigstring errbs;
	char prestring[256];

	wsprintf (prestring, "Can't %s because TCP/IP error code %ld", cannot, (long)errcode);
	copyctopstring (prestring, errbs);

	gettcperrorstring (errcode, bs);

	if (!isemptystring (bs)) {
		
		pushstring ("\x03" " - ", errbs);
		
		pushstring (bs, errbs);
		}
	else {
		pushchar ('.', errbs);
		}
	
	nullterminate (errbs);
	
	TCPERRORprintf (wsprintf(TCPmsg, "NET ERROR - %s", stringbaseaddress(errbs)));
	TCPERRORWRITEMSG ();
	
	langerrormessage (errbs);
	} /*neterror*/

#if 0

static void dnserror (char *cannot, long errcode) {
	
	if (errcode == kOTBadNameErr) {  /* Host not found */
	
		bigstring bs;
		bigstring errbs;
		char prestring[256];

		wsprintf (prestring, "Can't %s because TCP/IP error code %ld", cannot, (long)errcode);
		copyctopstring (prestring, errbs);

		copystring (dnserrorstrings [1], bs); //handles nil source
					
		pushstring ("\x03" " - ", errbs);
			
		pushstring (bs, errbs);

		pushchar ('.', errbs);
		
		nullterminate (errbs);
		
		TCPERRORprintf (wsprintf(TCPmsg, "NET ERROR - %s", stringbaseaddress(errbs)));
		TCPERRORWRITEMSG ();
		
		langerrormessage (errbs);
		
		return;
		}

	neterror (cannot, errcode);
	}

#endif


static void intneterror (long errcode) {
	bigstring bs;

	if (errcode == INTNETERROR_INVALIDSTREAM)
		copyctopstring ("Invalid stream", bs);
	else
		copyctopstring ("Unknown stream error", bs);

	langerrormessage (bs);
	} /*intneterror*/


static boolean fwsbackgroundtask (void) {

	boolean fl = true;

	if (inmainthread ()) {
		EventRecord		ev;
		short mask = osMask|activMask|mDownMask|keyDownMask; //|highLevelEventMask|updateMask
		long sleepTime = 6;	// 1/10 of a second by default
		
		if (WaitNextEvent (mask, &ev, sleepTime, nil)) /*migh return false to indicate a null event, but that's not an error*/
			fl = shellprocessevent (&ev);
		}
	else
		fl = langbackgroundtask (true);
	
	return (fl);
	}/*fwsbackgroundtask*/


static boolean fwsNetEventLaunch (struct hostData *data) {

	/*
	Initialize the NetEvents system
	
	5.0.2b5 dmb: added hostData parameter and GUSI support to handle threading
	*/
	
	#pragma unused (data)
	
	if (!frontierWinSockLoaded) {
		
		//Code change by Timothy Paustian Monday, June 26, 2000 3:55:47 PM
		//OpenTransport takes a context for all of its calls. Applications
		//do not need this, so pass nil
		OSStatus err = noErr;
		
		#if TARGET_API_MAC_CARBON ==1
			err = InitOpenTransportInContext(kInitOTForApplicationMask, nil);
		#else
			err = InitOpenTransport ();
		#endif
	
		if (err != kOTNoError) {
			neterror ("initialize Open Transport", err);
			return (false);
			}

		err = Gestalt (sOTVersionSelector, (long*) &sOTVersion);
		
		if (err || (sOTVersion < kOTVersion111)) {
			plainneterror ("\x23" "Please install Open Transport 1.1.1 or later.");
			return (false);
			}
		
		clearbytes (&epstats, sizeof (epstats));
		
		sListenList.fHead = nil;
		sEndpointList.fHead = nil;

		sIdleEPs.fHead = nil;
		sBrokenEPs.fHead = nil;
		sWaitingEPs.fHead = nil;
		
		sMasterConfig = OTCreateConfiguration ("tcp");
		
		#if TARGET_API_MAC_CARBON == 1
			gThreadEntryCallback = NewThreadEntryUPP (fwsacceptingthreadmain);
			
			if(gThreadEntryCallback == nil) {
				memoryerror ();
				return (false);
				}
				
			gListenNotifierUPP = NewOTNotifyUPP(ListenNotifier);
			
			if(gListenNotifierUPP == nil) {
				memoryerror ();
				return (false);
				}
			
			gListSearchUPP = NewOTListSearchUPP(listenlinkvisit);
			
			if(gListSearchUPP == nil) {
				memoryerror ();
				return (false);
				}
			
			gNotifierUPP = NewOTNotifyUPP(Notifier);
			
			if(gNotifierUPP == nil) {
				memoryerror ();
				return (false);
				}
			
			gDNRNotifierUPP = NewOTNotifyUPP (DNRNotifier);
			
			if (gDNRNotifierUPP == nil) {
				memoryerror ();
				return (false);
				}
				
			gEndListSearchUPP = NewOTListSearchUPP(endpointlinkvisit);
			
			if(gEndListSearchUPP == nil) {
				memoryerror ();
				return (false);
				}
				
		#endif
		}
		
	frontierWinSockLoaded = true;
	
	++frontierWinSockCount;

	return (true);
	} /*fwsNetEventLaunch*/


boolean fwsNetEventQuit (void) {

	frontierWinSockCount = 0;

	TCPTRACKERCLOSE();

	if (frontierWinSockLoaded) {
		frontierWinSockLoaded = false;

		//Code change by Timothy Paustian Sunday, May 7, 2000 9:29:38 PM
		//Changed to Opaque call for Carbon
		#if TARGET_API_MAC_CARBON == 1

			//get rid of our UPPs
			DisposeOTNotifyUPP(gNotifierUPP);
			gNotifierUPP = nil;
			DisposeOTNotifyUPP (gDNRNotifierUPP);
			DisposeOTListSearchUPP(gEndListSearchUPP);
			gEndListSearchUPP = nil;
			DisposeOTListSearchUPP(gListSearchUPP);
			gListSearchUPP = nil;
			DisposeOTNotifyUPP(gListenNotifierUPP);
			gListenNotifierUPP = nil;
			DisposeThreadEntryUPP(gThreadEntryCallback);
			gThreadEntryCallback = nil;

			//This has to be switched to NULL when we link against the carbon lib.
			CloseOpenTransportInContext(nil);
		#else
			CloseOpenTransport ();
		#endif

		return (true);
		}

	return (false);
	} /*fwsNetEventShutDown*/


boolean fwsNetEventAddressDecode (unsigned long addr, bigstring IPaddr) {
	
	/* Convert an address (4 byte) into a dotted IP address */

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);
	
	setstringlength (IPaddr, 0);
	
	OTInetHostToString (addr, stringbaseaddress (IPaddr));
	
	setstringlength (IPaddr, strlen (stringbaseaddress (IPaddr)));

	return (true);
	} /*fwsNetEventAddressDecode*/


boolean fwsNetEventAddressEncode (bigstring IPaddr, unsigned long  * addr) {
	
	/* Convert a dotted IP address into an address (4 byte) */

	InetHost host;
	OSStatus err;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);
	
	nullterminate (IPaddr);
	
	err = OTInetStringToHost (stringbaseaddress (IPaddr), &host);
	
	if (err != kOTNoError) {
	
		langparamerror (cantencodeaddress, IPaddr);
	
		return (false);
		}

	*addr = host;
	
	return (true);
	} /*fwsNetEventAddressEncode*/
	

boolean fwsNetEventAddressToName (unsigned long addr, bigstring domainName) {

	/*
	Convert an address (4 byte) into a domain name
	*/

	dnsquery query;
	InetSvcRef inetservice;
	OSStatus err;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);
	
	TCPprintf (wsprintf(TCPmsg, "Entering fwsNetEventAddressToName at line %d.  Address: %ld.", __LINE__, addr));
	TCPWRITEMSG();

	/*open dns service provider*/

	#if TARGET_API_MAC_CARBON == 1
		inetservice = OTOpenInternetServicesInContext (kDefaultInternetServicesPath, 0L, &err, nil);
	#else
		inetservice = OTOpenInternetServices (kDefaultInternetServicesPath, 0L, &err);
	#endif
	
	if (err != kOTNoError) {
		neterror("convert address", err);
		return (false);
		}
	
	/*install the notifier*/
	
	OTMemzero (&query, sizeof (query));
	
	#if TARGET_API_MAC_CARBON == 1
		err = OTInstallNotifier (inetservice, gDNRNotifierUPP, &query);
	#else
		err = OTInstallNotifier (inetservice, &DNRNotifier, &query);
	#endif
	
	if (err != kOTNoError) 
		goto exit;

	/*switch to async mode*/
		
	err = OTSetAsynchronous (inetservice);

	if (err != kOTNoError) 
		goto exit2;
	
	/*fire off query*/
	
	err = OTInetAddressToName (inetservice, (InetHost) addr, stringbaseaddress (domainName));

	if (err != kOTNoError) 
		goto exit2;

	/*spin until query completes*/
	
	while (0 == (query.flags & fcomplete))
		YieldToAnyThread ();

	/*clean up*/
	
	#if TARGET_API_MAC_CARBON == 1
		OTRemoveNotifier (inetservice);
	#endif

	OTCloseProvider (inetservice);
	
	/*retrieve query result and return*/
	
	if (query.result != kOTNoError) {
		neterror ("convert address", query.result);
		return (false);
		}
	
	setstringlength (domainName, strlen (stringbaseaddress (domainName)));
	
	poptrailingchars (domainName, '.');

	TCPprintf (wsprintf(TCPmsg, "Leaving fwsNetEventAddressToName at line %d.  Domain name: %s.", __LINE__, stringbaseaddress (domainName)));
	TCPWRITEMSG();

	return (true);
	
	/*error handling*/
	
exit2:

	#if TARGET_API_MAC_CARBON == 1
		OTRemoveNotifier (inetservice);
	#endif

exit:
	
	OTCloseProvider (inetservice);

	neterror ("convert name", err);
	
	return (false);
	} /*fwsNetEventAddressToName*/


boolean fwsNetEventNameToAddress (bigstring domainName, unsigned long * addr) {
	
	/*
	Convert a domain name into an address (4 byte)
	
	7.0b44 PBS: OS X -- make the domain name all lower to prevent an error.
	*/

	char sysstring[256];
	dnsquery query;
	InetHostInfo hostinfo;
	InetSvcRef inetservice;
	OSStatus err;
	
	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Entering fwsNetEventNameToAddress at line %d.  Domain name: %s.", __LINE__, stringbaseaddress (domainName)));
	TCPWRITEMSG();
	
	#if TARGET_API_MAC_CARBON == 1
	
		alllower (domainName); /*7.0b44 PBS: OS X may return an error otherwise.*/
		
	#endif
	   
	copyptocstring (domainName, sysstring);
	
	/*open dns service provider*/

	#if TARGET_API_MAC_CARBON == 1
		inetservice = OTOpenInternetServicesInContext (kDefaultInternetServicesPath, 0L, &err, nil);
	#else
		inetservice = OTOpenInternetServices (kDefaultInternetServicesPath, 0L, &err);
	#endif
	
	if (err != kOTNoError) {
		neterror("convert name", err);
		return (false);
		}
	
	/*install the notifier*/
	
	OTMemzero (&query, sizeof (query));
	
	#if TARGET_API_MAC_CARBON == 1
		err = OTInstallNotifier (inetservice, gDNRNotifierUPP, &query);
	#else
		err = OTInstallNotifier (inetservice, &DNRNotifier, &query);
	#endif
	
	if (err != kOTNoError) 
		goto exit;

	/*switch to async mode*/
		
	err = OTSetAsynchronous (inetservice);
		
	if (err != kOTNoError) 
		goto exit;

	/*fire off query*/
	
	err = OTInetStringToAddress (inetservice, sysstring, &hostinfo);
	
	if (err != kOTNoError) 
		goto exit;

	/*spin until query completes*/
	
	while (0 == (query.flags & fcomplete))
		YieldToAnyThread ();

	/*clean up*/
	
	OTCloseProvider (inetservice);
	
	/*retrieve query result and return*/
	
	if (query.result != kOTNoError) {
		neterror ("convert name", query.result);
		return (false);
		}
			
	*addr = hostinfo.addrs[0];

	TCPprintf (wsprintf(TCPmsg, "Leaving fwsNetEventNameToAddress at line %d.  Address: %08lx.", __LINE__, *addr));
	TCPWRITEMSG();

	return (true);
	
	/*error handling*/

exit:
	
	OTCloseProvider (inetservice);

	neterror ("convert name", err);
	
	return (false);
	} /*fwsNetEventNameToAddress*/


boolean fwsNetEventMyAddress (unsigned long * addr) {
	
	/* Get the hosts address */

	OSStatus err;
	OSStatus junk;
	EndpointRef dummyEP;
	InetInterfaceInfo info;
	Boolean fl = false;
	TEndpointInfo epInfo;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	//Code change by Timothy Paustian Monday, June 26, 2000 4:04:28 PM
	//
	#if TARGET_API_MAC_CARBON == 1
		dummyEP = OTOpenEndpointInContext (OTCreateConfiguration ("tcp"), 0, &epInfo, &err, nil);
	#else	
		dummyEP = OTOpenEndpoint (OTCreateConfiguration ("tcp"), 0, &epInfo, &err);
	#endif
	
	if (err != kOTNoError) {
		neterror("get local address", err);
		return (false);
		}
	
	err = OTInetGetInterfaceInfo (&info, 0);
	
	if (err == kOTNoError) {
		
		*addr = info.fAddress;
		
		fl = true;
		}
	else
		neterror("get local address", err);

	if (dummyEP != nil) {
		junk = OTCloseProvider(dummyEP);
		OTAssert("fwsNetEventMyAddress: Failed closing dummy endpoint", junk == kOTNoError);
		}
	
	return (fl);
	} /*fwsNetEventMyAddress*/


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

#ifdef flnewfeatures
	item.profilebase = 0;		
#endif
	
	(**herrorstack).stack [(**herrorstack).toperror++] = item;

	copystring (bsname, (**hp).bsname);

	return (true);
	} /*newprocess*/
	

static boolean fwsruncallback (EndpointRecordRef epref) {

	hdltreenode hcallbackaddress;
	hdltreenode hfunctioncall;
	hdltreenode hcode;
	hdltreenode hparam1;
	hdltreenode hparam2;
	tyvaluerecord val;
	hdlprocessrecord hprocess;
	Handle h;
	
	ListenRecordRef listenref = epref->listener;
	long refcon = listenref->refcon;
	Handle hcallbacktree = listenref->hcallbacktree;
	bigstring bsname;
	
	copystring (listenref->callback, bsname);
	
	//build code tree
	
	if (!copyhandle (hcallbacktree, &h))
		return (false);
		
	if (!langunpacktree (h, &hcallbackaddress))
		return (false);
	
	setlongvalue ((long) epref, &val);
	
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
	
	if (!fwsnewprocess (hcode, bsname, &hprocess)) {
		langdisposetree (hcode);
		return (false);
		}

	//add new process

	return (addprocess (hprocess));
	}/*fwsruncallback*/


static long fwsprocesspendingconnections (ListenRecordRef listenref) {

	OTLink* nomad = OTLIFOStealList (&listenref->readyEPs);
	OTLink* next;
	EndpointRecordRef epref;
	long ct = 0;
	boolean fl;
	
	while (nil != nomad) {
		
		next = nomad->fNext;

		epref = OTGetLinkObject (nomad, tyendpointrecord, link);
					
		OTAddFirst (&sEndpointList, &epref->validationlink);
		
		epref->typeID = SOCKTYPE_OPEN;

		TCPprintf (wsprintf (TCPmsg, "In fwsacceptpendingconnections at line %d.  Accepted new connection #%08ld on listener %08lx: %s (%08lx, %ld).",
										__LINE__, ct, (long) listenref, stringbaseaddress (listenref->callback), (long) epref, listenref->refcon));
		TCPWRITEMSG();

		fl = fwsruncallback (epref);
	
		TCP_ASSERT_1 (fl);	

		nomad = next;
		
		ct++;
		}/*while*/
		
	return (ct);
	}/*fwsprocesspendingconnections*/


static void fwscleanuplistener (ListenRecordRef listenref) {

	EndpointRecordRef epref;
	OTLink* nomad;
	OTLink* next;
	long ctepsclosed = 0;
	
	/* Process pending connections for one last time */
	
	fwsprocesspendingconnections (listenref); /* listenref->readyEPs is now empty and remains empty */
	
	/* Now enter a loop in which we close idle and broken eps until we've closed all dependent acceptors */

	while (ctepsclosed < listenref->maxdepth) {
	
		YieldToAnyThread ();

		nomad = OTLIFOStealList (&listenref->idleEPs);
		
		while (nomad != nil) {
					
			next = nomad->fNext;
			
			epref = OTGetLinkObject (nomad, tyendpointrecord, link);
			
			nomad = next;
			
			if (EPClose (epref)) {
			
				++ctepsclosed;

				listenref->stats.ctidle--;
				}
			else {
				OTLIFOEnqueue (&listenref->brokenEPs, &epref->link);

				listenref->stats.ctidle--;
				
				listenref->stats.ctbroken++;
				}
			}/*while*/
			
		YieldToAnyThread ();

		nomad = OTLIFOStealList (&listenref->brokenEPs);
		
		while (nomad != nil) {
					
			next = nomad->fNext;
			
			epref = OTGetLinkObject (nomad, tyendpointrecord, link);
			
			nomad = next;
			
			if (EPClose (epref)) {
			
				++ctepsclosed;
				
				listenref->stats.ctbroken--;
				}
			else
				OTLIFOEnqueue (&listenref->brokenEPs, &epref->link);
			}/*while*/
			
		}/*while*/
	
	assert (listenref->stats.ctbroken == 0);
	
	assert (listenref->stats.ctidle == 0);
	
	assert (listenref->stats.ctworking == 0);
	
	/* Finally, dispose all allocated memory structures */
	
	OTDestroyConfiguration (listenref->masterconfig);

	disposehandle (listenref->hcallbacktree);
	
	DisposePtr ((Ptr) listenref->acceptors);
	
	DisposePtr ((Ptr) listenref);
	
	return;
	}/*fwscleanuplistener*/


static void *fwsacceptingthreadmain (void *param) {

	/*
	We sit in a loop waiting for connections that are ready to be picked up and sent to the daemon.
	We are also responsible for shutting down the listener and all dependent endpoints.
	*/

	ListenRecordRef listenref = (ListenRecordRef) param;
	OSStatus err;
	long ct = 0;

	TCPTRACKERIN ("fwsacceptingthreadmain", __LINE__, nil, listenref);
	
	while (SOCKTYPE_LISTENSTOPPED != listenref->typeID) {
		
		ct += fwsprocesspendingconnections (listenref);
				
		ProcessWaitingEndpoints (listenref);
		
		Recycle (listenref);

		if (listenref->fllistenpending)
			EnterListenAccept (listenref);
		
		err = YieldToAnyThread ();
		
		TCP_ASSERT_1 (err == kOTNoError);	
		}/*while*/
	
	TCPprintf (wsprintf (TCPmsg, "In fwsacceptingthreadmain at line %d, broke out of loop after accepting %ld connections on listener %08lx. Now starting clean-up.",
							__LINE__, ct, (long) listenref));
	TCPWRITEMSG();

	fwscleanuplistener (listenref);	

	TCPprintf (wsprintf (TCPmsg, "Exiting fwsacceptingthreadmain at line %d.", __LINE__));
	TCPWRITEMSG();

	return (nil);
	}/*fwsacceptingthreadmain*/


static boolean fwslaunchacceptingthread (ListenRecordRef listenref) {

	OSStatus err;

	#if TARGET_API_MAC_CARBON == 1
		err = NewThread (kCooperativeThread, gThreadEntryCallback, (void *)listenref, 0, kUsePremadeThread + kCreateIfNeeded + kFPUNotNeeded, nil, &listenref->idthread);	
	#else
		err = NewThread (kCooperativeThread, fwsacceptingthreadmain, (void *)listenref, 0, kUsePremadeThread + kCreateIfNeeded + kFPUNotNeeded, nil, &listenref->idthread);
	#endif

	if (kOTNoError != err) {
		oserror (err);
		return (false);	
		}

	return (true);
	}/*fwslaunchacceptingthread*/


boolean fwsNetEventListenStream (unsigned long port, long depth, bigstring callback, unsigned long refcon, unsigned long * stream, unsigned long ipaddr, long hdatabase) {

	/* Set up a listner on a port */

	Handle hcallbacktree = nil;
	OSStatus err;
	TBind bindReq;
	InetAddress ipAddress;
	ListenRecordRef listenref = nil;
	EndpointRecordRef epref = nil;
	TOptMgmt optReq;
	TOption opt;
	long i;
	
	nullterminate (callback);
	TCPprintf(wsprintf(TCPmsg, "Entering fwsNetEventListenStream at line %d. Port = %ld, Depth = %ld, Refcon = %ld, Callback = %s.", __LINE__, port, depth, refcon, stringbaseaddress(callback)));
	TCPWRITEMSG ();

	/* Initialize Open Transport and static data structures */
	
	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);
	
	/* Compile and pack a code tree for the address of the daemon script */

	if (!fwsgetcallbackcodetree (callback, &hcallbacktree))
		return (false);
	
	/* Allocate and clear memory for the listener */
	
	listenref = (ListenRecordRef) NewPtr (sizeof (tylistenrecord));

	if (listenref == nil) {
		memoryerror ();
		goto exit;
		}
	
	OTMemzero (listenref, sizeof (tylistenrecord));
	
	/* Initialize further status fields */
	copystring (callback, listenref->callback);

	listenref->maxdepth = depth;

	listenref->refcon = refcon;
	
	listenref->hcallbacktree = hcallbacktree;

	listenref->hdatabase = (hdldatabaserecord) hdatabase;
	
	/* Open synchronous listener and set to blocking mode */

	#if TARGET_API_MAC_CARBON == 1
		listenref->ep = OTOpenEndpointInContext (OTCreateConfiguration ("tilisten,tcp"), nil, nil, &err, nil);
	#else
		listenref->ep = OTOpenEndpoint (OTCreateConfiguration ("tilisten,tcp"), nil, nil, &err);
	#endif
	
	if (kOTInvalidEndpointRef == listenref->ep || kOTNoError != err) {
		neterror ("create listen stream", err);
		goto exit;
		}
	
	err = OTSetBlocking (listenref->ep);
	
	if (kOTNoError != err) {
		OSStatus status;
		status = OTCloseProvider (listenref->ep);	
		OTAssert ("fwsNetEventListenStream: Could not close listener", status == kOTNoError);
		neterror ("set listen stream to blocking mode", err);
		goto exit;
		}
	
	/* Option Management - Turn on ip_reuseaddr so we don't have port conflicts in general. */

	optReq.flags = T_NEGOTIATE;
	optReq.opt.len = kOTFourByteOptionSize;
	optReq.opt.buf = (unsigned char *) &opt;
	
	opt.len = sizeof (TOption);
	opt.level = INET_IP;
	opt.name = kIP_REUSEADDR;
	opt.status = 0;
	opt.value[0] = 1;

	err = OTOptionManagement (listenref->ep, &optReq, nil);

	if (kOTNoError != err) {
		OSStatus status;
		status = OTCloseProvider (listenref->ep);	
		OTAssert ("fwsNetEventListenStream: Could not close listener", status == kOTNoError);
		neterror ("set IP_REUSEADDR option on listen stream", err);
		goto exit;
		}

	/* Bind listener to specified address or default address if ipaddr is nil */

	OTInitInetAddress (&ipAddress, port, ipaddr);
	
	bindReq.addr.buf = (UInt8 *) &ipAddress;
	
	bindReq.addr.len = sizeof (ipAddress);
	
	bindReq.qlen = depth;
	
	err = OTBind (listenref->ep, &bindReq, nil);

	if (kOTNoError != err) {
		OSStatus status;
		status = OTCloseProvider (listenref->ep);	
		OTAssert ("fwsNetEventListenStream: Could not close listener", status == kOTNoError);
		neterror ("bind listen stream", err);
		goto exit;
		}	

	/* Install a notifier for the listener and switch to async mode */
	#if TARGET_API_MAC_CARBON == 1
		err = OTInstallNotifier (listenref->ep, gListenNotifierUPP, (void *) listenref);
	#else
		err = OTInstallNotifier (listenref->ep, &ListenNotifier, (void *) listenref);
	#endif
		
	if (kOTNoError != err) {
		OSStatus status;
		status = OTCloseProvider (listenref->ep);	
		OTAssert ("fwsNetEventListenStream: Could not close listener", status == kOTNoError);
		neterror ("install notifier for listen stream", err);
		goto exit;
		}

	err = OTSetAsynchronous (listenref->ep);
	
	if (kOTNoError != err) {
		OSStatus status;
		status = OTCloseProvider (listenref->ep);	
		OTAssert ("fwsNetEventListenStream: Could not close listener", status == kOTNoError);
		neterror ("set listen stream to async mode", err);
		goto exit;
		}
	
	/* Push the listener on the global list of listeners, so we can close it on shutdown */
	
	OTAddFirst (&sListenList, &listenref->validationlink);

	/* Launch the worker thread that manages incoming connections */
	
	if (!fwslaunchacceptingthread (listenref)) { /* takes care of closing ep and cleaning up, even if we fail further down */
		OSStatus status;
		status = OTCloseProvider (listenref->ep);	
		OTAssert ("fwsNetEventListenStream: Could not close listener", status == kOTNoError);
		goto exit;
		}

	/* Create configuration to be cloned for accepters */
	
	listenref->masterconfig = OTCreateConfiguration ("tcp");

	if (kOTInvalidConfigurationPtr == listenref->masterconfig || kOTNoMemoryConfigurationPtr == listenref->masterconfig) {
		memoryerror ();
		goto exit;
		}
	
	/* Allocate and clear memory for block of acceptors */
	
	listenref->acceptors = (EndpointRecordRef) NewPtr (depth * sizeof (tyendpointrecord));

	if (listenref->acceptors == nil) {
		memoryerror ();
		goto exit;
		}
	
	listenref->stats.cttotal = depth;
	
	OTMemzero (listenref->acceptors, depth * sizeof (tyendpointrecord));

	/* Loop over acceptors and open them asynchronously one by one */
	for (epref = listenref->acceptors, i = 0; i < depth; epref++, i++) {
		
		epref->listener = listenref;
	
		err = EPOpen (epref, OTCloneConfiguration (listenref->masterconfig));
		
		if (kOTNoError != err) {
			neterror ("create worker streams for listen stream", err);	
			goto exit;
			}

		listenref->stats.ctworking++;
		}

	/* Signal to the worker thread that we are ready to go */

	listenref->typeID = SOCKTYPE_LISTENING;

	/* Return to caller */

	*stream = (long) listenref;

	TCPTRACKEROUT ("fwsNetEventListenStream", __LINE__, nil, listenref);

	return (true);

exit:

	//disposehandle (hcallbacktree);
		
	//neterror ("set up listen stream", err);

	return (false);
	} /*fwsNetEventListenStream*/


boolean fwsNetEventCloseListen (unsigned long stream) {

	/*
	Close a listen and delete associated data
	
	7.0b35 PBS: pass gListSearchUPP to OTFindAndRemoveLink.
	*/

	ListenRecordRef listenref = (ListenRecordRef) stream;
	OSStatus err;
	
	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);
	
	TCPTRACKERIN ("fwsNetEventCloseListen", __LINE__, nil, listenref);

	/* Find and remove from listener list */
	
	#if TARGET_API_MAC_CARBON == 1
		if (!OTFindAndRemoveLink (&sListenList, gListSearchUPP, (void *)listenref)) {
			intneterror (INTNETERROR_INVALIDSTREAM);
			return (false);
			}
	#else
		if (!OTFindAndRemoveLink (&sListenList, &listenlinkvisit, (void *)listenref)) {
			intneterror (INTNETERROR_INVALIDSTREAM);
			return (false);
			}
	#endif	

	/* Remove the notifier function */

	OTRemoveNotifier (listenref->ep);
	
	/* Set the listener to synchronous mode */
	
	err = OTSetSynchronous (listenref->ep);
	
	if (kOTNoError != err) {
		TCPERRORprintf (wsprintf (TCPmsg, "In fwscleanuplistener at line %d, error %ld setting listener %08lx to synchronous mode.", __LINE__, err, (long) listenref));
		TCPERRORWRITEMSG();
		}
	
	/* Close the listening endpoint itself */
	
	err = OTCloseProvider (listenref->ep);
	
	if (kOTNoError != err) {
		TCPERRORprintf (wsprintf (TCPmsg, "In fwscleanuplistener at line %d, error %ld closing listener %08lx.", __LINE__, err, (long) listenref));
		TCPERRORWRITEMSG();
		}
	
	/* Refer the actual clean up to the worker thread associated with the listener */
	
	listenref->typeID = SOCKTYPE_LISTENSTOPPED;

	TCPTRACKEROUT ("fwsNetEventCloseListen", __LINE__, nil, listenref);

	return (true);
	} /*fwsNetEventCloseListen*/


void fwsNetEventShutdownDependentListeners (long hdatabase) {

	OTLink* listitem;
	ListenRecordRef listenref;

	#if TARGET_API_MAC_CARBON == 1
		OTListSearchUPP listendatabaseUPP = NewOTListSearchUPP(listendatabasevisit);
	#endif
	
	if (!frontierWinSockLoaded)
		return;
	
	
	while (true) {
		
		#if TARGET_API_MAC_CARBON == 1
			listitem = OTFindLink (&sListenList, listendatabaseUPP, (void *) hdatabase);
		#else
			listitem = OTFindLink (&sListenList, &listendatabasevisit, (void *) hdatabase);
		#endif
		
		if (listitem == nil)
			break;
	
		listenref = OTGetLinkObject (listitem, tylistenrecord, validationlink);

		OTRemoveLink (&sListenList, &listenref->validationlink);
		
		listenref->typeID = SOCKTYPE_LISTENSTOPPED; /* Leave clean up to worker thread */
		}
		
		#if TARGET_API_MAC_CARBON == 1
			DisposeOTListSearchUPP(listendatabaseUPP);
		#endif
	}/*fwsNetEventShutdownDependentListeners*/


boolean fwsNetEventCheckAndAcceptSocket (void) {
	
	/*
	This is called from the main event loop.
	We don't have to do any accepting here,
	but use the opportunity to recycle broken EPs.
	*/
	
	if (frontierWinSockLoaded)

		ProcessWaitingEndpoints (nil);

		Recycle (nil);

	return (true);
	}


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


static boolean fwsOpenStream (TCall *ptrSndCall, unsigned long * stream) {
	
	/*
	Open a new endpoint asynchronously.
	*/
	
	EndpointRecordRef epref;
	OTLink *epref_link = nil;
	OSStatus err;

	if (!incrementconnectioncounter ()) {

		plainneterror ("\x54" "Can't open stream because no more than five TCP connections may be open at any time.");

		return (false);
		}

	/* Try to obtain a new endpoint from the static list of idle endpoints */
	
	#if TARGET_API_MAC_CARBON != 1
		
		/*Always create new endpoints on OS X.*/
	
		epref_link = OTLIFODequeue (&sIdleEPs);
		
	#endif
	
	if (nil == epref_link) { /* No endpoints available, need to allocate and open a new one */
	
		epref = (EndpointRecordRef) NewPtr (sizeof (tyendpointrecord));
		
		if (nil == epref) {
			neterror ("open stream", memFullErr);
			decrementconnectioncounter();
			return (false);
			}

		InitEndpoint (epref, nil, nil);
		
		epref->sendCall = ptrSndCall;
		
		OTAtomicSetBit (&epref->stateflags, kWaitingForConnectBit);

		OTAtomicSetBit (&epref->stateflags, kDontDisposeBit);

		err = EPOpen (epref, OTCloneConfiguration (sMasterConfig));
		
		if (kOTNoError != err) {
			DisposePtr ((Ptr) epref);
			neterror ("open stream", err);
			decrementconnectioncounter();
			return (false);
			}

		epstats.cttotal++;
		
		epstats.ctworking++;
		}
	else {
	
		epstats.ctidle--;
		
		epstats.ctworking++;
		
		epref = OTGetLinkObject (epref_link, tyendpointrecord, link);
		
		InitEndpoint (epref, epref->ep, nil);

		epref->sendCall = ptrSndCall;

		OTAtomicSetBit (&epref->stateflags, kWaitingForConnectBit);

		OTAtomicSetBit (&epref->stateflags, kDontDisposeBit);

		err = EnterBind (epref);

		if (kOTNoError != err) {
			neterror ("open stream", err);
			CheckUnbind (epref, err, kQueueIt);
			decrementconnectioncounter();	
			return (false);
			}
		}
	
	/* Loop until open, bind, and connect have completed */
	
	while (!OTAtomicTestBit (&epref->completionflags, kConnectCompleteBit))
		if (!fwsbackgroundtask ()) {
			if (OTAtomicTestBit (&epref->stateflags, kBoundBit))
				CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
			else
				CheckUnbind (epref, userCanceledErr, kQueueIt);	
			decrementconnectioncounter(); 
			return (false);
			}
	
	err = epref->result;
	
	if (kOTNoError != err) {
		neterror ("open stream", err);
		if (OTAtomicTestBit (&epref->stateflags, kBoundBit))
			CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
		else
			CheckUnbind (epref, err, kQueueIt);	
		decrementconnectioncounter();	
		return (false);
		}
	
	/* Clean up and return */
	
	epref->typeID = SOCKTYPE_OPEN;
	
	OTAddFirst (&sEndpointList, &epref->validationlink);
	
	*stream = (long) epref;
	
	return (true);
	}/*fwsOpenStream*/


boolean fwsNetEventOpenAddrStream (unsigned long addr, unsigned long port, unsigned long * stream) {

	/*
	6.2b6 AR: Open Transport version
	*/
	
	TCall sndCall;
	InetAddress hostAddress;
	
	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Entering fwsNetEventOpenAddrStream at line %d.  Address: %ld.", __LINE__, addr));
	TCPWRITEMSG();
	
	OTInitInetAddress (&hostAddress, port, addr);

	sndCall.addr.buf 	= (UInt8 *) &hostAddress;
	sndCall.addr.len 	= sizeof (hostAddress);
	sndCall.opt.buf 	= nil;		// no connection options
	sndCall.opt.len 	= 0;
	sndCall.udata.buf 	= nil;		// no connection data
	sndCall.udata.len 	= 0;
	sndCall.sequence 	= 0;		// ignored by OTConnect
	
	return (fwsOpenStream (&sndCall, stream));
	} /*fwsNetEventOpenAddrStream*/


boolean fwsNetEventOpenNameStream (bigstring name, unsigned long port, unsigned long * stream) {

	/*
	6.2b6 AR: Open Transport version.
	
	7.0b53 PBS: lower-case the domain name in the OS X version, else an error may be reported.
	*/

	char sysstring[256];
	unsigned long addr;
	TCall sndCall;
	DNSAddress dnsaddress;
	
	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);
	
	#if TARGET_API_MAC_CARBON == 1
	
		alllower (name); /*7.0b44 PBS: OS X may return an error otherwise.*/
		
	#endif

	copyptocstring (name, sysstring);
		
	TCPprintf (wsprintf(TCPmsg, "Entering fwsNetEventOpenNameStream at line %d.  Domain name: %s.", __LINE__, sysstring));
	TCPWRITEMSG();
	
	if (is_ipaddress (sysstring)) {
		
		if (!fwsNetEventAddressEncode (name, &addr))
			return  (false);
		
		return (fwsNetEventOpenAddrStream (addr, port, stream));
		}
	
	#if TARGET_API_MAC_CARBON == 1 /*7.0b53 PBS: check the domain name*/
	
		if (!fwsNetEventNameToAddress (name, &addr))
			return (false);
	
	#endif
	
	wsprintf (sysstring, "%s:%ld", sysstring, port);

	sndCall.addr.len 	= OTInitDNSAddress (&dnsaddress, sysstring);
	sndCall.addr.buf 	= (UInt8 *) &dnsaddress;
	sndCall.opt.buf 	= nil;		// no connection options
	sndCall.opt.len 	= 0;
	sndCall.udata.buf 	= nil;		// no connection data
	sndCall.udata.len 	= 0;
	sndCall.sequence 	= 0;		// ignored by OTConnect
		
	return (fwsOpenStream (&sndCall, stream));
	} /*fwsNetEventOpenNameStream*/


boolean fwsNetEventAbortStream (unsigned long stream) {

	/*
	Abort a stream and delete associated data

	We trust the notifier and EnterSndDisconnect
	to do the right thing regarding clean-up.
	*/
	
	EndpointRecordRef epref = (EndpointRecordRef) stream;
	OSStatus err;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventAbortStream", __LINE__, epref, nil);

	/* Validate endpoint reference */
	
	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}
	
	/* Remove from list of valid endpoint references */
	
	OTRemoveLink (&sEndpointList, &epref->validationlink);
			
	/* Initiate abortive disconnect */
	
	err = EnterSndDisconnect (epref);
	
	if (kOTNoError != err) { // Scripts don't have to care about this error, it has already been taken care off anyway
		TCPERRORprintf (wsprintf (TCPmsg, "In fwsNetEventAbortStream at line %d, error %ld disconnecting endpoint %08lx.", __LINE__, err, (long) epref));
		TCPERRORWRITEMSG();
		}

	decrementconnectioncounter();

	TCPTRACKEROUT ("fwsNetEventAbortStream", __LINE__, epref, nil);

	return (true);
	} /*fwsNetEventAbortStream*/


boolean fwsNetEventCloseStream (unsigned long stream) {

	/*
	Close a stream and delete associated data
	
	We trust the notifier and EnterSndOrderlyDisconnect
	to do the right thing regarding clean-up.
	*/

	EndpointRecordRef epref = (EndpointRecordRef) stream;
	boolean doLeave;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	/* Validate endpoint reference */
	
	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}
	
	TCPTRACKEROUT ("fwsNetEventCloseStream", __LINE__, epref, nil);
	
	/* Remove from list of valid endpoint references */
	
	OTRemoveLink (&sEndpointList, &epref->validationlink);
	
	/* Initiate graceful disconnect */
	
	doLeave = OTEnterNotifier (epref->ep);
	

	/*bundle*/ {
		ListenRecordRef listenref = epref->listener;
		
		DoSndOrderlyDisconnect (epref, true);
				
		if (0 == OTAtomicTestBit (&epref->stateflags, kBrokenBit)) {

			if (T_IDLE == OTGetEndpointState (epref->ep)) {
				
				OTAtomicClearBit (&epref->stateflags, kDontDisposeBit); /*pass on responsibility to the Notifier*/
				
				CheckUnbind (epref, OTUnbind (epref->ep), kDontQueueIt);
				}
			else {
					
				epref->timestamp = timenow ();
				
				OTAtomicSetBit (&epref->stateflags, kWaitingForDisconnectBit);

				if (listenref != nil) {
					listenref->stats.ctworking--;
					listenref->stats.ctwaiting++;
					OTLIFOEnqueue (&listenref->waitingEPs, &epref->link); /*try again later*/
					}
				else {
					epstats.ctworking--;
					epstats.ctwaiting++;
					OTLIFOEnqueue (&sWaitingEPs, &epref->link); /*try again later*/
					}
				}
			}
		}
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);

	decrementconnectioncounter();

	TCPTRACKEROUT ("fwsNetEventCloseStream", __LINE__, epref, nil);

	return (true);
	} /*fwsNetEventCloseStream*/
	
	
boolean fwsNetEventReadStream (unsigned long stream, unsigned long * bytesToRead, char * buffer) {

	/* Read from a stream */

	OTResult result;
	OTFlags junkFlags;
	EndpointRecordRef epref = (EndpointRecordRef) stream;
	long ix = 0;
	long readcount = *bytesToRead;
	boolean doLeave = false;
	
	*bytesToRead = 0;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventReadStream", __LINE__, epref, nil);

	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	doLeave = OTEnterNotifier (epref->ep);

	if (OTAtomicTestBit (&epref->completionflags, kRcvdOrderlyDisconnectBit) || OTAtomicTestBit (&epref->completionflags, kRcvdDisconnectBit)) {
		closedunexpectedlyerror ("read stream");
		goto error;
		}

	while (readcount > 0) {

		result = OTRcv (epref->ep, (void *) (buffer + ix), readcount, &junkFlags);
	
		if (result < 0) {
			
			if (kOTNoDataErr == result) { /* No data available for reading */
				if (!fwsbackgroundtask ()) {
					if (doLeave)
						OTLeaveNotifier (epref->ep);
					return (false);
					}
				continue;
				}

			if (kOTLookErr == result) {
				
				OTResult eventcode = OTLook (epref->ep);
				
				switch (eventcode) {
					
					case T_DISCONNECT: {	/* Receive and break out */
						EnterRcvDisconnect (epref);
						closedunexpectedlyerror ("read stream");
						goto error;
						}
					
					case T_ORDREL: {	/* Receive and try again */
						EnterRcvOrderlyDisconnect (epref);
						closedunexpectedlyerror ("read stream");
						goto error;
						}
					
					case T_GODATA: { /* Try again */
						TCP_MSG_2 ("OTLook after OTRcv returned T_GODATA");
						continue;
						}
					
					default: {
						neterror ("write stream", result);
						goto error;
						}
					}
				}
			else {
				neterror("read stream", result);
				goto error;
				}
			}
		
		readcount -= result;
		
		ix += result;

		*bytesToRead = ix;
		}/*while*/

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventReadStream at line %d.  Bytes requested = %ld, read = %ld.", __LINE__, *bytesToRead, (unsigned long) result));
	TCPWRITEMSG ();

	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (true);

error:

	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (false);
	} /*fwsNetEventReadStream*/


boolean fwsNetEventWriteStream (unsigned long stream, unsigned long bytesToWrite, char * buffer) {

	/*
	Write to a Stream
	
	5.0.2b3 dmb: if we write fewer bytes than requested, we need to retry, generate error
	
	6.2b9 AR: The above change doesn't really work since scripts have no way to find out
	which part of the data needs to be resend. Instead of throwing an error, keep trying.
	*/
	
	EndpointRecordRef epref = (EndpointRecordRef) stream;
	OTResult result;
	long ix = 0;
	boolean doLeave = false;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventWriteStream", __LINE__, epref, nil);

	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}
	
	doLeave = OTEnterNotifier (epref->ep);
	
	if (OTAtomicTestBit (&epref->completionflags, kRcvdDisconnectBit)) {
		closedunexpectedlyerror ("write stream");
		goto exit;
		}
	
	while (bytesToWrite > 0) {

		result = OTSnd (epref->ep, (void *) (buffer + ix), bytesToWrite, nil);
	
		if (result <= 0) {
			
			if (kOTFlowErr == result) {
				if (!fwsbackgroundtask ()) {
					if (doLeave)
						OTLeaveNotifier (epref->ep);
					return (false);
					}
				continue;
				}

			if (kOTLookErr == result) {
				
				OTResult eventcode = OTLook (epref->ep);
				
				switch (eventcode) {
					
					case T_DISCONNECT: {	/* Receive and break out */
						EnterRcvDisconnect (epref);
						closedunexpectedlyerror ("write stream");
						goto exit;
						}
					
					case T_ORDREL: {	/* Receive and try again */
						EnterRcvOrderlyDisconnect (epref);
						continue;
						}
					
					case T_GODATA: { /* Try again */
						TCP_MSG_2 ("OTLook after OTSnd returned T_GODATA");
						continue;
						}
					
					default: {
						neterror ("write stream", result);
						goto exit;
						}
					}
				}
			else {
				neterror("write stream", result);
				goto exit;
				}
			}

		TCPprintf (wsprintf(TCPmsg, "In fwsNetEventWriteHandleToStream at line %d, requested writing %ld bytes, wrote %ld bytes.", __LINE__, bytesToWrite, result));
		TCPWRITEMSG ();

		bytesToWrite -= result;
		
		ix += result;
		}

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventWriteStream at line %d.  Bytes requested = %ld, written = %ld.", __LINE__, bytesToWrite, (unsigned long) result));
	TCPWRITEMSG ();
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);
		
	return (true);

exit:

	if (doLeave)
		OTLeaveNotifier (epref->ep);
		
	return (false);
	} /*fwsNetEventWriteStream*/



boolean fwsNetEventStatusStream (unsigned long stream, bigstring status, unsigned long * bytesPending) {

	/* get the status of a stream */

	EndpointRecordRef epref = (EndpointRecordRef) stream;
	ListenRecordRef listenref = (ListenRecordRef) stream;
	tysocktypeid typeID = SOCKTYPE_INVALID;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);
		
	*bytesPending = 0;

	if (CheckEndpointList (epref)) {
		typeID = epref->typeID;
		if (!inmainthread())
			TCPTRACKERIN ("fwsNetEventStatusStream", __LINE__, epref, nil);
		}

	if (CheckListenList (listenref)) {
		typeID = listenref->typeID;
		if (!inmainthread())
			TCPTRACKERIN ("fwsNetEventStatusStream", __LINE__, nil, listenref);
		}
	
	if (!epref && !listenref && !inmainthread ()) {
		TCPTRACKERIN ("fwsNetEventStatusStream", __LINE__, epref, listenref);
		}

	switch (typeID) {
		case SOCKTYPE_INVALID:
			intneterror(INTNETERROR_INVALIDSTREAM);
			copyctopstring ("INACTIVE", status);
			return (false);

		case SOCKTYPE_UNKNOWN:
			copyctopstring ("UNKNOWN", status);
			break;

		case SOCKTYPE_OPEN: {
			OTResult result;
		
			copyctopstring ("OPEN", status);
		
			result = OTCountDataBytes (epref->ep, (OTByteCount*) bytesPending);
			
			switch (result) {
				
				case kOTNoError: {
					if (*bytesPending != 0)
						copyctopstring ("DATA", status);
					else
						copyctopstring ("OPEN", status);
					break;
					}
		
				case kOTNoDataErr: {
					copyctopstring ("OPEN", status);
					break;
					}
				
				case kOTLookErr: {
					
					OTResult eventcode = OTLook (epref->ep);
					
					switch (eventcode) {
						
						case T_DISCONNECT: {	/* Receive and break out */
							//EnterRcvDisconnect (epref); /* Don't actually disconnect here, leave that up to fwsNetEventCloseStream */
							copyctopstring ("INACTIVE", status);
							epref->typeID = SOCKTYPE_INACTIVE;
							break;
							}
						
						case T_ORDREL: {	/* Receive and try again */
							EnterRcvOrderlyDisconnect (epref); /* Don't actually disconnect here, leave that up to fwsNetEventCloseStream */
							copyctopstring ("INACTIVE", status);
							epref->typeID = SOCKTYPE_INACTIVE;
							break;
							}
						
						default: {
							copyctopstring ("OPEN", status);
							break;
							}
						}
					break;
					}
				
				case kOTOutStateErr:
					if (*bytesPending == 0)
						break;
						
				default:
					neterror("check status on stream", result);
					copyctopstring ("INACTIVE", status);
					epref->typeID = SOCKTYPE_INACTIVE;
					return (false);
				}
				
			if (*bytesPending == 0)
				if (OTAtomicTestBit (&epref->completionflags, kRcvdOrderlyDisconnectBit) || OTAtomicTestBit (&epref->completionflags, kRcvdDisconnectBit)) {
					copyctopstring ("INACTIVE", status);
					epref->typeID = SOCKTYPE_INACTIVE;
					}

			break;
			}

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

	if (!inmainthread()) {
		TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventStatusStream at line %d, result is %s with bytes pending %ld.", __LINE__, stringbaseaddress(status), *bytesPending));
		TCPWRITEMSG ();
		}

	return (true);
	} /*fwsNetEventStatusStream*/


boolean fwsNetEventGetPeerAddress (unsigned long stream, unsigned long * peeraddress, unsigned long * peerport) {

	EndpointRecordRef epref = (EndpointRecordRef) stream;
	TBind* peerAddr;
	OSStatus err;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventGetPeerAddress", __LINE__, epref, nil);

	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}
	//Code change by Timothy Paustian Monday, June 26, 2000 4:07:42 PM
	//
	#if TARGET_API_MAC_CARBON == 1
		peerAddr = OTAllocInContext (epref->ep, T_BIND, T_ADDR, &err, nil);	
	#else
		peerAddr = OTAlloc (epref->ep, T_BIND, T_ADDR, &err);
	#endif
		
	if (err != kOTNoError) {
		neterror ("get peer address", err);
		return (false);
		}
		
	OTAtomicClearBit (&epref->completionflags, kGetProtAddressBit);
	
	err = OTGetProtAddress (epref->ep, nil, peerAddr);

	if (err != kOTNoError) {
		neterror ("get peer address", err);
		OTFree (peerAddr, T_BIND);
		return (false);
		}
	
	while (!OTAtomicTestBit (&epref->completionflags, kGetProtAddressBit))
		YieldToAnyThread ();
	
	//OTAssert ("fwsNetEventGetPeerAddress: size of peer address structure doesn't match InetAddress", sizeof (InetAddress) == peerAddr->addr.len);

	//OTAssert ("fwsNetEventGetPeerAddress: size of peer address structure doesn't match InetAddress", AF_INET == ((InetAddress*) peerAddr->addr.buf)->fAddressType);
		
	*peerport = ((InetAddress*) peerAddr->addr.buf)->fPort;
	
	*peeraddress = ((InetAddress*) peerAddr->addr.buf)->fHost;	

	OTFree (peerAddr, T_BIND);

	TCPTRACKEROUT ("fwsNetEventGetPeerAddress", __LINE__, epref, nil);

	return (true);
	} /*fwsNetEventGetPeerAddress*/


boolean fwsNetEventReadStreamUntil (unsigned long stream, Handle hbuffer, Handle hpattern, unsigned long timeoutsecs) {

	/*
	Read data from stream. Don't return until we found the pattern or until we timed out.
	*/

	EndpointRecordRef epref = (EndpointRecordRef) stream;
	long ix = gethandlesize (hbuffer);
	long ixstart = ix;
	OTFlags junkFlags;
	OTResult result;
	long timeoutticks;
	boolean doLeave = false;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventReadStreamUntil", __LINE__, epref, nil);

	if (searchhandle (hbuffer, hpattern, 0, ix) != -1L)
		return (true);

	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}
	
	doLeave = OTEnterNotifier (epref->ep);
		
	if (OTAtomicTestBit (&epref->completionflags, kRcvdOrderlyDisconnectBit) || OTAtomicTestBit (&epref->completionflags, kRcvdDisconnectBit))
		goto error_closedprematurely;

	if (!sethandlesize (hbuffer, ix + kPacketSize)) {
		if (doLeave)
			OTLeaveNotifier (epref->ep);
		return (false);
		}
	
	timeoutticks = gettickcount () + (timeoutsecs * 60);

	while (true) {

		lockhandle (hbuffer);

		result = OTRcv (epref->ep, &((*hbuffer) [ix]), kPacketSize, &junkFlags);

		unlockhandle (hbuffer);

		if (result < 0) {

			if (kOTNoDataErr == result) {
				
				if (gettickcount () > timeoutticks) {
					result = kETIMEDOUTErr;
					goto exit;
					}
				
				if (!fwsbackgroundtask ()) {
					if (doLeave)
						OTLeaveNotifier (epref->ep);
					return (false);
					}
				
				continue;
				}
			
			if (kOTLookErr == result) {
			
				result = OTLook (epref->ep);
				
				switch (result) {
					
					case T_ORDREL:
						EnterRcvOrderlyDisconnect (epref);
						goto error_closedprematurely;
					
					case T_DISCONNECT:
						EnterRcvDisconnect (epref);
						goto error_closedprematurely;
					
					case T_GODATA:
						TCP_MSG_2 ("OTLook after OTRcv returned T_GODATA");
						continue; /*ignore it*/
					
					default:
						result = kOTLookErr;
						goto exit;
					}
				}
			else
				goto exit;
			}

		TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamUntil at line %d, read %ld bytes.", __LINE__, result));
		TCPWRITEMSG ();

		ix += result;
		
		if (searchhandle (hbuffer, hpattern, 0, ix) != -1L)
			break;
		
		if (!sethandlesize (hbuffer, ix + kPacketSize)) { /* Make room for another 8k */
			if (doLeave)
				OTLeaveNotifier (epref->ep);
			return (false);
			}
	
		timeoutticks = gettickcount () + (timeoutsecs * 60);
		}/*while*/

	if (!sethandlesize (hbuffer, ix)) {
		if (doLeave)
			OTLeaveNotifier (epref->ep);
		return (false);
		}

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventReadStreamUntil at line %d.  Total bytes read = %ld.", __LINE__, ix - ixstart));
	TCPWRITEMSG ();

	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (true);

exit:

	sethandlesize (hbuffer, ix);

	neterror ("read stream", result);
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);
		
	return (false);

error_closedprematurely:

	sethandlesize (hbuffer, ix);

	closedunexpectedlyerror ("read stream");

	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (false);
	} /*fwsNetEventReadStreamUntil*/



boolean fwsNetEventReadStreamUntilClosed (unsigned long stream, Handle hbuffer, unsigned long timeoutsecs) {

	/*
	Read data from stream until closed.
	*/

	EndpointRecordRef epref = (EndpointRecordRef) stream;
	long ix = gethandlesize (hbuffer);
	long ixstart = ix;
	OTFlags junkFlags;
	OTResult result;
	long timeoutticks;
	boolean doLeave = false;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventReadStreamUntilClosed", __LINE__, epref, nil);

	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}
	
	doLeave = OTEnterNotifier (epref->ep);
	
	if (OTAtomicTestBit (&epref->completionflags, kRcvdOrderlyDisconnectBit) || OTAtomicTestBit (&epref->completionflags, kRcvdDisconnectBit))
		goto done; /* 08/08/2000 AR: we're done already */
	
	if (!sethandlesize (hbuffer, ix + kPacketSize)) {
		if (doLeave)
			OTLeaveNotifier (epref->ep);
		return (false);
		}

	timeoutticks = gettickcount () + (timeoutsecs * 60);

	while (true) {
		
		lockhandle (hbuffer);

		result = OTRcv (epref->ep, &((*hbuffer) [ix]), kPacketSize, &junkFlags);

		unlockhandle (hbuffer);

		if (result < 0) {
		
			if (kOTNoDataErr == result) {
				
				if (gettickcount () > timeoutticks) {
					neterror ("read stream", kETIMEDOUTErr);
					goto exit;
					}

				if (!fwsbackgroundtask ()) {
					if (doLeave)
						OTLeaveNotifier (epref->ep);
					return (false);
					}
				
				continue;
				}
			
			if (kOTLookErr == result) {
			
				result = OTLook (epref->ep);
				
				switch (result) {
					
					case T_ORDREL:
						EnterRcvOrderlyDisconnect (epref);
						goto done;
					
					case T_DISCONNECT:
						EnterRcvDisconnect (epref);
						goto done;
					
					case T_GODATA:
						TCP_MSG_2 ("OTLook after OTRcv returned T_GODATA");
						continue; /*ignore it*/
					
					default:
						neterror ("read stream", kOTLookErr);
						goto exit;
					}
				}
			else {
				neterror ("read stream", result);
				goto exit;
				}
			}

		TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamUntilClosed at line %d, read %ld bytes.", __LINE__, result));
		TCPWRITEMSG ();

		ix += result;

		if (!sethandlesize (hbuffer, ix + kPacketSize))
			return (false); /*we have the thread globals, so we can return immediately*/

		timeoutticks = gettickcount () + (timeoutsecs * 60);
		}/*while*/

done:

	if (!sethandlesize (hbuffer, ix))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventReadStreamUntilClosed at line %d.  Total bytes read = %ld.", __LINE__, ix - ixstart));
	TCPWRITEMSG ();
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (true);

exit:

	if (!sethandlesize (hbuffer, ix))
		return (false);
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (false);
	}/*fwsNetEventReadStreamUntilClosed*/


boolean fwsNetEventReadStreamBytes (unsigned long stream, Handle hbuffer, long ctbytes, unsigned long timeoutsecs) {

	/*
	Read the specified number of bytes from the stream appending to hbuffer.
	*/

	EndpointRecordRef epref = (EndpointRecordRef) stream;
	long lenbuffer = gethandlesize (hbuffer);
	long ix = lenbuffer;
	OTResult result;
	OTFlags junkFlags;
	long timeoutticks;
	boolean doLeave = false;

	if (ix >= ctbytes) /*6.1b8 AR: don't do anything*/
		return (true);

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventReadStreamBytes", __LINE__, epref, nil);

	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	doLeave = OTEnterNotifier (epref->ep);

	if (!sethandlesize (hbuffer, ctbytes)) { //make enough room in one go
		if (doLeave)
			OTLeaveNotifier (epref->ep);
		return (false);
		}

	lockhandle (hbuffer);
	
	if (OTAtomicTestBit (&epref->completionflags, kRcvdOrderlyDisconnectBit) || OTAtomicTestBit (&epref->completionflags, kRcvdDisconnectBit))
		goto error_closedprematurely;
		
	timeoutticks = gettickcount () + (timeoutsecs * 60);

	while (ix < ctbytes) {

		result = OTRcv (epref->ep, (void *) &((*hbuffer)[ix]), ctbytes - ix, &junkFlags);
		
		if (result < 0) {
		
			if (kOTNoDataErr == result) {
				
				if (gettickcount () > timeoutticks) {
					result = kETIMEDOUTErr;
					goto exit;
					}

				if (!fwsbackgroundtask ()) {
					unlockhandle (hbuffer);
					if (doLeave)
						OTLeaveNotifier (epref->ep);
					return (false);
					}
				
				continue;
				}
			
			if (kOTLookErr == result) {
			
				result = OTLook (epref->ep);
				
				switch (result) {
					
					case T_ORDREL:
						EnterRcvOrderlyDisconnect (epref);
						goto error_closedprematurely;
					
					case T_DISCONNECT:
						EnterRcvDisconnect (epref);
						goto error_closedprematurely;
					
					case T_GODATA:
						TCP_MSG_2 ("OTLook after OTRcv returned T_GODATA");
						continue; /*ignore it*/
					
					default:
						result = kOTLookErr;
						goto exit;
					}
				}
			else
				goto exit;
			}

		TCPprintf (wsprintf(TCPmsg, "In fwsNetEventReadStreamBytes at line %d, read %ld bytes.", __LINE__, result));
		TCPWRITEMSG ();

		ix += result;

		timeoutticks = gettickcount () + (timeoutsecs * 60);
		}/*while*/

	unlockhandle (hbuffer);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventReadStreamBytes at line %d.  Total bytes requested = %ld, bytes read = %ld.", __LINE__, ctbytes, ix));
	TCPWRITEMSG ();

	if (doLeave)
		OTLeaveNotifier (epref->ep);
	
	return (true);

exit:

	unlockhandle (hbuffer);

	sethandlesize (hbuffer, ix); /*shrinking, can't fail*/

	neterror ("read stream", result);

	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (false);

error_closedprematurely:

	closedunexpectedlyerror ("read stream");

	unlockhandle (hbuffer);

	sethandlesize (hbuffer, ix); /*shrinking, can't fail*/

	if (doLeave)
		OTLeaveNotifier (epref->ep);
		
	return (false);
	} /*fwsNetEventReadStreamBytes*/


boolean fwsNetEventWriteHandleToStream (unsigned long stream, Handle hbuffer, unsigned long chunksize, unsigned long timeoutsecs) {

	/* Write to stream in chunks */

	EndpointRecordRef epref = (EndpointRecordRef) stream;
	unsigned long bytesremaining = gethandlesize (hbuffer);
	long ix = 0;
	long bytestowrite;
	OTResult result;
	long timeoutticks;
	boolean doLeave = false;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventWriteHandleToStream", __LINE__, epref, nil);

	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}
	
	doLeave = OTEnterNotifier (epref->ep);

	lockhandle (hbuffer);
	
	if (OTAtomicTestBit (&epref->completionflags, kRcvdDisconnectBit))
		goto error_closedprematurely;

	timeoutticks = gettickcount () + (timeoutsecs * 60);

	do {
	
		bytestowrite = (bytesremaining < chunksize) ? bytesremaining : chunksize;
	
		result = OTSnd (epref->ep, &((*hbuffer) [ix]), bytestowrite, nil);
	
		if (result <= 0) {
		
			if (kOTFlowErr == result) {
				
				if (gettickcount () > timeoutticks) {
					result = kETIMEDOUTErr;
					goto exit;
					}

				if (!fwsbackgroundtask ()) {
					unlockhandle (hbuffer);
					if (doLeave)
						OTLeaveNotifier (epref->ep);
					return (false);
					}
				
				continue;
				}
			
			if (kOTLookErr == result) {
			
				result = OTLook (epref->ep);
				
				switch (result) {
					
					case T_ORDREL:
						EnterRcvOrderlyDisconnect (epref);
						goto error_closedprematurely;
					
					case T_DISCONNECT:
						EnterRcvDisconnect (epref);
						goto error_closedprematurely;
					
					case T_GODATA:
						continue; /*ignore it*/
					
					default:
						result = kOTLookErr;
						goto exit;
					}
				}
			else
				goto exit;
			}

		TCPprintf (wsprintf(TCPmsg, "In fwsNetEventWriteHandleToStream at line %d, requested writing %ld bytes, wrote %ld bytes.", __LINE__, bytestowrite, result));
		TCPWRITEMSG ();

		bytesremaining -= result;
		
		ix += result;

		timeoutticks = gettickcount () + (timeoutsecs * 60);

	} while (bytesremaining > 0);
	
	unlockhandle (hbuffer);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventWriteHandleToStream at line %d.  Total bytes written = %ld.", __LINE__, ix));
	TCPWRITEMSG ();
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);
		
	return (true);

exit:

	unlockhandle (hbuffer);

	neterror("write stream", result);
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (false);

error_closedprematurely:

	unlockhandle (hbuffer);

	closedunexpectedlyerror ("write stream");
	
	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (false);
	} /*fwsNetEventWriteHandleToStream*/


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

	TCPTRACKERIN ("fwsNetEventWriteFileToStream", __LINE__, (EndpointRecordRef) stream, nil);

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


boolean fwsNetEventInetdRead (unsigned long stream, Handle hbuffer, unsigned long timeoutsecs) {

	/*
	6.1b2 AR: Wait timeoutsecs seconds for data to come in. After the first packet has been
	received, continue reading with a reduced timeout of .5 second. (For historical reasons.)
	*/

	EndpointRecordRef epref = (EndpointRecordRef) stream;
	OTResult result;
	OTFlags junkFlags;
	long ix = gethandlesize (hbuffer);
	long ixstart = ix;
	long timeoutticks;
	boolean doLeave = false;

	if (!fwsNetEventLaunch (NO_HOST_SERVICES))
		return (false);

	TCPTRACKERIN ("fwsNetEventInetdRead", __LINE__, epref, nil);

	if (!CheckEndpointList (epref)) {
		intneterror (INTNETERROR_INVALIDSTREAM);
		return (false);
		}

	doLeave = OTEnterNotifier (epref->ep);
	
	if (OTAtomicTestBit (&epref->completionflags, kRcvdOrderlyDisconnectBit) || OTAtomicTestBit (&epref->completionflags, kRcvdDisconnectBit))
		goto done;

	timeoutticks = gettickcount () + (timeoutsecs * 60);

	while (true) {
		
		lockhandle (hbuffer);

		result = OTRcv (epref->ep, &((*hbuffer) [ix]), kPacketSize, &junkFlags);

		unlockhandle (hbuffer);

		if (result < 0) {
		
			if (kOTNoDataErr == result) {
				
				if (gettickcount () > timeoutticks)
					goto done; /* simply stop reading, don't throw an error */

				YieldToAnyThread ();
				
				continue;
				}
			
			if (kOTLookErr == result) {
			
				result = OTLook (epref->ep);
				
				switch (result) {
					
					case T_ORDREL:
						EnterRcvOrderlyDisconnect (epref);
						goto done;
					
					case T_DISCONNECT:
						EnterRcvDisconnect (epref);
						goto done;
					
					case T_GODATA:
						TCP_MSG_2("OTLook after OTRcv returned T_GODATA");
						continue; /*ignore it*/
					
					default:
						result = kOTLookErr;
						goto exit;
					}
				}
			else
				goto exit;
			}

		TCPprintf (wsprintf(TCPmsg, "In fwsNetEventInetdRead at line %d, requested reading %ld bytes, read %ld bytes.", __LINE__, kPacketSize, result));
		TCPWRITEMSG ();

		ix += result;

		if (!sethandlesize (hbuffer, ix + kPacketSize)) {
			if (doLeave)
				OTLeaveNotifier (epref->ep);
			return (false);
			}

		timeoutticks = gettickcount () + 30;
		}/*while*/

done:
	if (!sethandlesize (hbuffer, ix))
		return (false);

	TCPprintf (wsprintf(TCPmsg, "Exiting fwsNetEventInetdRead at line %d.  Total bytes read = %ld.", __LINE__, ix - ixstart));
	TCPWRITEMSG ();

	if (doLeave)
		OTLeaveNotifier (epref->ep);
		
	return (true);

exit:
	neterror ("read stream", result);

	if (doLeave)
		OTLeaveNotifier (epref->ep);

	return (false);
	} /*fwsNetEventInetdRead*/


boolean fwsNetEventGetStats (unsigned long stream, bigstring bs) {
	
	setemptystring (bs);
	
	if (nil != stream) {
		
		ListenRecordRef listenref = (ListenRecordRef) stream;
	
		if (!CheckListenList (listenref)) {
			intneterror (INTNETERROR_INVALIDSTREAM);
			return (false);
			}

		pushlong (listenref->stats.cttotal, bs);
	
		pushchar (',', bs);
	
		pushlong (listenref->stats.ctidle, bs);
	
		pushchar (',', bs);
	
		pushlong (listenref->stats.ctworking, bs);
	
		pushchar (',', bs);
	
		pushlong (listenref->stats.ctwaiting, bs);
	
		pushchar (',', bs);
	
		pushlong (listenref->stats.ctbroken, bs);
		}
	else {
	
		pushlong (epstats.cttotal, bs);
	
		pushchar (',', bs);
	
		pushlong (epstats.ctidle, bs);
	
		pushchar (',', bs);
	
		pushlong (epstats.ctbroken, bs);
	
		pushchar (',', bs);
	
		pushlong (epstats.ctworking, bs);
		}

	return (true);
	}/*fwsNetEventGetStats*/