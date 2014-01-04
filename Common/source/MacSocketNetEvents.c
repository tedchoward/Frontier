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

/* Created 13 Aug 2011 Brent Simmons
 Many of the functions come from WinSockNetEvents.c, since both systems
 use sockets. But, since WinSockNetEvents is already crufty, I decided
 it would be easier to maintain this code in a separate file rather than
 add yet more #ifdefs and so on.
 
 TODO: ipv6 support
 It's probably best just to port it all to CFSocket and CFStream.
 But this was the easiest thing that would work. (Where "work" is defined
 as "working the same as the PowerPC build.")
 */


#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <utime.h>
#include <sys/errno.h>
#include <pthread.h>
#include "frontier.h"
#include "standard.h"
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
#include "WinSockNetEvents.h"
#include "about.h"
#include "mutex.h"


#define FRONTIER_MAX_STREAM 256

static const unsigned int INTNETERROR_INVALIDSTREAM = -1;

enum {
	SOCKTYPE_INVALID = -1,
	SOCKTYPE_UNKNOWN = 0,
	SOCKTYPE_OPEN = 1,
	SOCKTYPE_DATA,
	SOCKTYPE_LISTENING,
	SOCKTYPE_CLOSED,
	SOCKTYPE_LISTENSTOPPED,
	SOCKTYPE_INACTIVE
};


static unsigned int frontierWinSockCount = 0;
static boolean frontierWinSockLoaded = false;

long maxconnections = longinfinity;

typedef unsigned int SOCKET;
#define INVALID_SOCKET (SOCKET)(0)

#define SOCKET_ERROR -1

typedef short tysocktypeid;

typedef struct tysockRecord {
	SOCKET sockID;
	tysocktypeid typeID;
	long refcon;
	bigstring callback;
	long maxdepth;
	long listenReference;
	long currentListenDepth;
	Handle hcallbacktree;
	ThreadID idthread;
	hdldatabaserecord hdatabase;
	boolean flNotification;
} sockRecord;

static sockRecord sockstack[FRONTIER_MAX_STREAM];

static short sockListenCount = 0;
static short sockListenList[FRONTIER_MAX_STREAM];

typedef struct sockaddr_in SOCKADDR_IN;
static unsigned long maxlistendepth = 0;

boolean fwsNetEventWriteBufferToStream(unsigned long stream, char *buffer, unsigned long numberOfBytes, unsigned long chunkSize, unsigned long timeoutSecs);
static void yield(void);

pthread_mutex_t dataLock;

#define STR_P_ERROR_CLOSED_PREMATURELY	BIGSTRING ("\x45" "Can't read stream because the TCP connection was closed unexpectedly.")

#pragma mark -
#pragma mark TCPTracker

/*
 To disable the tcp tracker, don't define TCPTRACKER.
 To enable tcp tracker ouput in the about window, define TCPTRACKER == 1.
 To enable tcp tracker error ouput to a file, define TCPTRACKER == 2.
 To enable full tcp tracker output to a file, define TCPTRACKER == 3.
 */

#undef TCPTRACKER
//#define TCPTRACKER 1
//#define TCPTRACKER 2
//#define TCPTRACKER 3

#define TCPTRACKER_CONSOLE 0 //writes to console

static FILE *tcpfile = nil;
static unsigned char TCPmsg[400];

static void closeTCPTrackerFile(void) {
	if (tcpfile != nil)
		fclose(tcpfile);
}


#if TCP_TRACKER || TCPTRACKER_CONSOLE
static char *TCPGETTYPE (tysocktypeid typeID) {
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
}
#endif

static void TCPWRITEMSG() {
	
#if TCPTRACKER == 1
	convertcstring(TCPmsg);
	aboutsetmiscstring(TCPmsg);
#endif
	
#if TCPTRACKER == 3 || TCPTRACKER_CONSOLE == 1
	ThreadID idthread = nil;
	MacGetCurrentThread(&idthread);
	static long idlastthread = 0;
	
	if (tcpfile == nil) {
		tcpfile = fopen("tcpfile.txt", "w+");
		if (tcpfile == nil) {
			
			printf("Error opening tcpfile. %d\n", errno);
			return;
		}
	}
	
	if (idthread != idlastthread) {
		fprintf(tcpfile, "\n");
		idlastthread = idthread;
	}
	
	unsigned long ticks = gettickcount ();
	static unsigned long lastticks = 0;
#if TCPTRACKER_CONSOLE
	printf("%08X (%04ld) | %04X | %s", (unsigned int) ticks, (ticks - lastticks), (unsigned int)idthread, TCPmsg);
#endif
#if TCPTRACKER == 3
	int writeResult = fprintf(tcpfile, "%08X (%04ld) | %04X | %s", (unsigned int) ticks, (ticks - lastticks), (unsigned int)idthread, TCPmsg);
	if (writeResult < 0) {
		printf("Error opening tcpfile. %d\n", errno);
	}
#endif
	lastticks = ticks;
	fflush(tcpfile);
#endif
}


static void TCPTrackerTraceIn(const char *functionName, int linenumber) {
#if TCPTRACKER || TCPTRACKER_CONSOLE
	sprintf ((char *)TCPmsg, "Entering %s at line %d\n", functionName, linenumber);
	TCPWRITEMSG();	
#endif
}

static void TCPTrackerTraceOut(const char *functionName, int linenumber) {
#if TCPTRACKER || TCPTRACKER_CONSOLE
	sprintf ((char *)TCPmsg, "Exiting %s at line %d\n", functionName, linenumber);
	TCPWRITEMSG();		
#endif
}

static void TCPERRORWRITEMSG(void);

#define TCPIN() TCPTrackerTraceIn(__FUNCTION__, __LINE__)
#define TCPOUT() TCPTrackerTraceOut(__FUNCTION__, __LINE__)
#define TCPINSTREAM(streamID) TCPTRACKERIN(__FUNCTION__, __LINE__, streamID)
#define TCPOUTSTREAM(streamID) TCPTRACKEROUT(__FUNCTION__, __LINE__, streamID)



static void TCPTRACKERIN(const char *functionName, int linenumber, unsigned long streamID) {
	
#if TCPTRACKER || TCPTRACKER_CONSOLE
	if ((streamID < 1) || (streamID >= FRONTIER_MAX_STREAM)) {
		sprintf ((char *)TCPmsg, "Entering %s at line %d, Stream = %ld - INVALID STREAM.\n", functionName, linenumber, streamID);
		TCPWRITEMSG();
		return;
	}
	
	sprintf((char *)TCPmsg, "Entering %s at line %d, Stream = %ld, Socket = %ld, Type is %s, Max Depth is %d, Current Depth is %d, Listen Ref is %08lX, Refcon = %08lX.\n",
			functionName, linenumber, streamID, (long)sockstack[streamID].sockID, TCPGETTYPE (sockstack[streamID].typeID), (int)sockstack[streamID].maxdepth,
			(int)sockstack[streamID].currentListenDepth, sockstack[streamID].listenReference, sockstack[streamID].refcon);
	TCPWRITEMSG();
#endif
}


static void TCPTRACKEROUT(const char * functionName, int linenumber, unsigned long streamID) {
	
#if TCPTRACKER || TCPTRACKER_CONSOLE
	if ((streamID < 1) || (streamID >= FRONTIER_MAX_STREAM)) {
		sprintf ((char *)TCPmsg, "Exiting %s at line %d, Stream = %08lX - INVALID STREAM.\n", functionName, linenumber, streamID);
		TCPWRITEMSG();
		return;
	}
	
	sprintf ((char *)TCPmsg, "Exiting %s at line %d, Stream = %ld, Socket = %ld, Type is %s, Max Depth is %d, Current Depth is %d, Listen Ref is %08lX, Refcon = %08lX.\n",
			 functionName, linenumber, streamID, (long)sockstack[streamID].sockID, TCPGETTYPE (sockstack[streamID].typeID), (int)sockstack[streamID].maxdepth,
			 (int)sockstack[streamID].currentListenDepth, sockstack[streamID].listenReference, sockstack[streamID].refcon);
	TCPWRITEMSG();
#endif
}


static void TCPERRORWRITEMSG(void) {
#if TCPTRACKER || TCPTRACKER_CONSOLE
	unsigned long ticks = gettickcount();
	static unsigned long lastticks = 0;
	long idthread = (long) (**getcurrentthread ()).idthread;
	static long idlastthread = 0;
	
#if TCPTRACKER == 2 || TCPTRACKER == 3
	if (tcpfile == NULL) {
		tcpfile = fopen("tcpfile.txt", "w+");
	}
	
	if (idthread != idlastthread) {
		fprintf(tcpfile, "\n");
		idlastthread = idthread;
	}
	
	fprintf(tcpfile, "%08X (%04ld) | %04X | %s", (unsigned int) ticks, (long)(ticks - lastticks), (unsigned int)idthread, TCPmsg);
	lastticks = ticks;
	fflush(tcpfile);
#endif
	
#if TCPTRACKER_CONSOLE
	printf("%08X (%04ld) | %04X | %s\n", (unsigned int) ticks, (long)(ticks - lastticks), (unsigned int)idthread, TCPmsg);
#endif
	
#endif
}


static void TCPTrackerTraceOutWithError(char *errorString, const char *functionName, int linenumber) {
#if TCPTRACKER || TCPTRACKER_CONSOLE
	sprintf((char *)TCPmsg, "Exiting %s at line %d with error %s\n", functionName, linenumber, errorString);
	TCPERRORWRITEMSG();		
#endif
}

#if TCPTRACKER || TCPTRACKER_CONSOLE
#define TCPprintf(msg) msg
#define TCPERRORprintf(msg) msg
#else
#define TCPprintf(msg)
#define TCPERRORprintf(msg)
#endif


#pragma mark -
#pragma mark Locking

static boolean lockData(void) {
	return pthread_mutex_lock(&dataLock) == 0;	
}


static boolean unlockData(void) {
	return pthread_mutex_unlock(&dataLock) == 0;
}


#pragma mark Startup


static void clearsockstack(void) {
	TCPIN();
	lockData();
	long i;
	
	for (i = 0; i < FRONTIER_MAX_STREAM; i++) {
		sockstack[i].sockID = INVALID_SOCKET;
		sockstack[i].typeID = SOCKTYPE_INVALID;
	}
	
	sockListenCount = 0;
	unlockData();
	TCPOUT();
}


static ThreadEntryTPP gThreadEntryCallback = nil;
static void *acceptingthreadmain(void *param);

static boolean netEventLaunch(void) {
	
	if (!frontierWinSockLoaded) {
		TCPIN();
		frontierWinSockLoaded = true;
		
		if (mutexCreateRecursive(&dataLock) != 0) {
			TCPTrackerTraceOutWithError("Error creating lock", __FUNCTION__, __LINE__);
			return false;
		}
		
		gThreadEntryCallback = NewThreadEntryUPP(acceptingthreadmain);
		if(gThreadEntryCallback == nil) {
			memoryerror();
			TCPTrackerTraceOutWithError("Error creating gThreadEntryCallback", __FUNCTION__, __LINE__);
			return false;
		}
		
		clearsockstack();
		TCPOUT();
	}
	
	++frontierWinSockCount;
	return true;
}


#pragma mark Utilities

static struct timeval timevalWithSeconds(long seconds) {
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	return tv;
}


static fd_set fileDescriptorSetWithSocket(SOCKET aSocket) {
	fd_set fileDescriptorSet;
	FD_ZERO(&fileDescriptorSet);
	FD_SET(aSocket, &fileDescriptorSet);
	return fileDescriptorSet;
}


static SOCKET socketForStream(unsigned long stream) {
	lockData();
	SOCKET sock = sockstack[stream].sockID;
	unlockData();
	return sock;
}


static void setStreamStatus(unsigned long stream, int status) {	
	lockData();
	sockstack[stream].typeID = status;
	unlockData();
}


static void markStreamSocketAsInvalid(unsigned long stream) {
	lockData();
	sockstack[stream].sockID = INVALID_SOCKET;
	unlockData();
}


static void markStreamAsClosed(unsigned long stream) {
	lockData();
	setStreamStatus(stream, SOCKTYPE_CLOSED);
	markStreamSocketAsInvalid(stream);	
	unlockData();
}


#pragma mark Errors


static void intneterror(long errcode) {
	TCPIN();
	bigstring bs;	
	if (errcode == INTNETERROR_INVALIDSTREAM)
		copyctopstring ("Invalid stream", bs);
	else
		copyctopstring ("Unknown stream error", bs);	
	langerrormessage(bs);
	TCPOUT();
}


static boolean streamIsValid(unsigned long stream) {
	
	/*Has the side effect of calling intneterror(INTNETERROR_INVALIDSTREAM) if invalid.*/
	
	if ((stream < 1) || (stream >= FRONTIER_MAX_STREAM)) {
		intneterror(INTNETERROR_INVALIDSTREAM);
		return false;
	}
	
	return true;
}


static boolean socketIsValid(SOCKET aSocket) {
	return (aSocket != INVALID_SOCKET);
}


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
};


static void gettcperrorstring (int errcode, bigstring bs) {
	
	int ixtcperr = errcode;
	
	if (ixtcperr > 0 && ixtcperr < 80) {
		copystring(tcperrorstrings[ixtcperr], bs); //handles nil source
		
		if (!isemptystring(bs)) {
			pushchar ('.', bs);
			return;
		}
	}
	
	getsystemerrorstring(errcode, bs);
}


static void neterror(char *cannot, long errcode) {
	
	TCPIN();
	bigstring errbs;
	char prestring[256];
	sprintf(prestring, "Can't %s because TCP/IP error code %ld", cannot, (long)errcode);
	copyctopstring(prestring, errbs);
	
	bigstring bs;
	gettcperrorstring(errcode, bs);
	
	if (!isemptystring(bs)) {
		pushstring(BIGSTRING ("\x03" " - "), errbs);
		pushstring(bs, errbs);
	}
	else {
		pushchar('.', errbs);
	}
	
	nullterminate(errbs);
	
	langerrormessage(errbs);
	TCPOUT();
}


static void plainneterror(bigstring bs) {
	TCPIN();
	bigstring errbs;
	copystring(bs, errbs);
	nullterminate(errbs);
	langerrormessage(bs);
	TCPOUT();
}


#pragma mark -
#pragma mark API

boolean fwsNetEventQuit(void) {	
	TCPIN();
	while(sockListenCount > 0)
		fwsNetEventCloseListen(sockListenList[0]);	
	DisposeThreadEntryUPP(gThreadEntryCallback);
	gThreadEntryCallback = nil;
	TCPTrackerTraceOut(__FUNCTION__, __LINE__);
	return true;
}


boolean fwsNetEventShutDown(void) {	
	TCPIN();
	frontierWinSockCount = 0;
	closeTCPTrackerFile();
	TCPTrackerTraceOut(__FUNCTION__, __LINE__);
	return true;
}


void fwsNetEventShutdownDependentListeners(long hdatabase) {
	TCPIN();
	lockData();
	long i = 1;	
	for (i = 1; i < FRONTIER_MAX_STREAM; i++) {
		if ((sockstack[i].typeID == SOCKTYPE_LISTENING) && (sockstack[i].hdatabase == (hdldatabaserecord)hdatabase))
			setStreamStatus(i, SOCKTYPE_LISTENSTOPPED);
	}
	unlockData();
	TCPTrackerTraceOut(__FUNCTION__, __LINE__);
}


#pragma mark Name/Address

boolean fwsNetEventAddressDecode(unsigned long addr, bigstring IPaddr) {
	
	/* Convert an address (4 byte) into a dotted IP address */
	
	TCPIN();
	if (!netEventLaunch())
		return false;
	
	struct in_addr in;
	in.s_addr = htonl(addr);
	
	char *sysstring = inet_ntoa(in);
	
	setstringlength(IPaddr, 0);
	
	if (sysstring == NULL) {
		TCPTrackerTraceOutWithError("can't decode address", __FUNCTION__, __LINE__);
		langlongparamerror(cantdecodeaddress, addr);	
		return false;
	}
	
	copyctopstring(sysstring, IPaddr);
	TCPOUT();
	return true;
}


boolean fwsNetEventAddressEncode(bigstring IPaddr, unsigned long *addr) {
	
	/* Convert a dotted IP address into an address (4 byte) */
	
	TCPIN();
	if (!netEventLaunch())
		return false;
	
	char sysstring[256];
	copyptocstring(IPaddr, sysstring);
	unsigned long netaddr = inet_addr(sysstring);
	
	if (netaddr == INADDR_NONE) {
		TCPTrackerTraceOutWithError("can't encode address", __FUNCTION__, __LINE__);
		langparamerror(cantencodeaddress, IPaddr);
		return false;
	}
	
	*addr = ntohl(netaddr);
	TCPOUT();
	
	return true;
}


boolean fwsNetEventAddressToName(unsigned long addr, bigstring domainName) {
	
	/* Convert an address (4 byte) into a domain name. */
	
	if (!netEventLaunch())
		return false;
	
	TCPprintf(sprintf((char *)TCPmsg, "Entering fwsNetEventAddressToName at line %d. Address: %ld.\n", __LINE__, addr));
	TCPWRITEMSG();
	
	unsigned long netaddr = htonl(addr);	
	struct hostent *h = gethostbyaddr((char *)&netaddr, 4, PF_INET);
	
	if (h == NULL) {
		TCPTrackerTraceOutWithError("can't convert address", __FUNCTION__, __LINE__);
		neterror("convert address", h_errno);
		return false;
	}
	
	copyctopstring(h->h_name, domainName);	
	
	TCPprintf(sprintf((char *)TCPmsg, "Leaving fwsNetEventAddressToName at line %d. Domain name: %s.\n", __LINE__, h->h_name));
	TCPWRITEMSG();
	
	return true;
}


boolean fwsNetEventNameToAddress(bigstring domainName, unsigned long *addr) {
	
	/* Convert a domain name into an address (4 byte) */
	
	if (!netEventLaunch())
		return false;
	
	char sysstring[256];
	copyptocstring(domainName, sysstring);
	
	TCPprintf(sprintf((char *)TCPmsg, "Entering fwsNetEventNameToAddress at line %d. Domain name: %s.\n", __LINE__, sysstring));
	TCPWRITEMSG();
	
	struct hostent *h = gethostbyname(sysstring);
	
	if (h == nil) {
		TCPTrackerTraceOutWithError("can't convert name", __FUNCTION__, __LINE__);
		neterror("convert name", h_errno);
		return false;
	}
	
	*addr = ntohl(*((long *)h->h_addr_list[0]));	
	
	TCPprintf (sprintf((char *)TCPmsg, "Leaving fwsNetEventNameToAddress at line %d. Address: %ld.\n", __LINE__, (long)addr));
	TCPWRITEMSG();
	
	return true;
}


boolean fwsNetEventMyAddress(unsigned long *addr) {
	
	TCPIN();
	if (!netEventLaunch())
		return false;
	
	struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    
    if (getifaddrs(&myaddrs) != 0) {
		TCPTrackerTraceOutWithError("can't get if addresses", __FUNCTION__, __LINE__);
		neterror("getifaddrs", errno);
        return false;
    }
    
    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }
        
        if (!(ifa->ifa_flags & IFF_UP)) {
            continue;
        }
        
        if (!strstr(ifa->ifa_name, "lo") && ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
            in_addr = &s4->sin_addr;
            *addr = ntohl(((struct in_addr *)in_addr)->s_addr);
            break;
        }
		
    }
    
    freeifaddrs(myaddrs);
    
    if (*addr == 0) {
        *addr = 0x7F000001;
    }
	
	TCPOUT();
	return true;
}


#pragma mark Opening/Closing Streams

static boolean checkAccepter(unsigned long stream) {
	
	TCPIN();
	lockData();
	if (sockstack[stream].listenReference != 0) {
		long listenstream = sockstack[stream].listenReference;		
		--sockstack[listenstream].currentListenDepth;
	}
	unlockData();
	TCPOUT();
	
	return true;
}


boolean fwsNetEventAbortStream(unsigned long stream) {
	
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	
	TCPINSTREAM(stream);
	
	SOCKET sock = socketForStream(stream);
	struct linger l;
	l.l_onoff = 1;
	l.l_linger = 0;	
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&l, sizeof(l));
	
	int res = close(sock);
	int errcode = errno;
	
	checkAccepter(stream);
	markStreamAsClosed(stream);
	
	if (res == SOCKET_ERROR) {
		TCPTrackerTraceOutWithError("res == SOCKET_ERROR", __FUNCTION__, __LINE__);
		neterror("abort stream", errcode);
		return false;
	}
	
	TCPOUTSTREAM(stream);
	return true;
}


boolean fwsNetEventCloseStream(unsigned long stream) {
	
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	
	TCPINSTREAM(stream);
	
	SOCKET sock = socketForStream(stream);
	
	if ((shutdown(sock, SHUT_WR) == SOCKET_ERROR) || (close(sock) == SOCKET_ERROR)) {
		long err = errno;
		markStreamAsClosed(stream);
		neterror("close stream", err);
		return false;
	}
	
	checkAccepter(stream);
	markStreamAsClosed(stream);
	
	TCPprintf (sprintf((char *)TCPmsg, "Leaving closeStream at line %d.  Thread duration: %ld ticks.\n", __LINE__, gettickcount()-(**getcurrentthreadglobals()).timestarted));
	TCPWRITEMSG();
	
	return true;
}


static void initsockrecord(long i) {	
	//	TCPIN();
	lockData();
	sockstack[i].sockID = INVALID_SOCKET;
	sockstack[i].typeID = SOCKTYPE_UNKNOWN;
	sockstack[i].maxdepth = 0;
	sockstack[i].listenReference = 0;
	sockstack[i].currentListenDepth = 0;
	sockstack[i].refcon = 0;
	copystring(emptystring, sockstack[i].callback);
	sockstack[i].flNotification = false;
	sockstack[i].hcallbacktree = nil;
	sockstack[i].hdatabase = nil;
	unlockData();
	//	TCPOUT();
}



static int indexOfSockRecordWithType(tysocktypeid sockRecordType) {
	
	//	TCPIN();
	lockData();
	int indexOfFoundRecord = -1;
	long i = 1;
	for (i = 1; i < FRONTIER_MAX_STREAM; i++) {
		if (sockstack[i].typeID == sockRecordType) {
			indexOfFoundRecord = i;
			break;
		}
	}
	unlockData();
	//	TCPOUT();
	return indexOfFoundRecord;	
} 


static boolean addsockrecord(long *stream) {
	
	//	TCPIN();
	lockData();
	long indexOfAvailableSockRecord = indexOfSockRecordWithType(SOCKTYPE_INVALID);
	if (indexOfAvailableSockRecord == -1)
		indexOfAvailableSockRecord = indexOfSockRecordWithType(SOCKTYPE_CLOSED);
	if (indexOfAvailableSockRecord == -1)
		indexOfAvailableSockRecord = indexOfSockRecordWithType(SOCKTYPE_INACTIVE);
	
	if (indexOfAvailableSockRecord == -1)
		*stream = -1;	
	else {
		initsockrecord(indexOfAvailableSockRecord);
		*stream = indexOfAvailableSockRecord;
	}
	unlockData();
	
	//	TCPOUT();
	return indexOfAvailableSockRecord != -1;
}


static boolean openStream(struct sockaddr_in *sa, unsigned long *stream) {
	
	TCPIN();
	long streamref;
	if (!addsockrecord(&streamref)) {
		neterror("open stream", EMFILE); /*Too many open sockets*/
		return false;
	}		
	
	int errcode = 0;
	SOCKET sock = socket(PF_INET, SOCK_STREAM, 0);
	if (!socketIsValid(sock)) {
		errcode = errno;
		goto exit;
	}
	
	/*Non-blocking sockets. See: http://www.scottklement.com/rpg/socktut/nonblocking.html */
	
	fcntl(sock, F_SETFL, O_NONBLOCK);
	while (true) {
		errcode = 0;
		int connectResult = connect(sock, (struct sockaddr *)sa, sizeof(*sa));
		errcode = errno;
#if TCPTRACKER_CONSOLE
		printf("connectResult %d errcode %d\n", connectResult, errcode);
#endif
		if (connectResult == 0 || errcode == EISCONN)
			break;
		if (errcode != EINPROGRESS && errcode != EALREADY) {
			close(sock);
			goto exit;
		}
		yield();
	}
	
	//	if (connect(sock, (struct sockaddr *)sa, sizeof(*sa)) == SOCKET_ERROR) {
	//		errcode = errno;
	//		close(sock);
	//		goto exit;
	//	}
	
	lockData();
	sockstack[streamref].typeID = SOCKTYPE_OPEN;
	sockstack[streamref].sockID = sock;
	unlockData();
	*stream = streamref;
	TCPOUTSTREAM(streamref);
	return true;
	
exit:
	lockData();
	sockstack[streamref].typeID = SOCKTYPE_INVALID;
	unlockData();
	neterror("open stream", errcode);
	return false;
}



boolean fwsNetEventOpenAddrStream(unsigned long addr, unsigned long port, unsigned long *stream) {
	
	if (!netEventLaunch())
		return false;
	
	TCPprintf (sprintf((char *)TCPmsg, "Entering fwsNetEventOpenAddrStream at line %d.  Address: %ld.\n", __LINE__, addr));
	TCPWRITEMSG();
	
	unsigned long netaddr = htonl(addr);
	struct sockaddr_in sa;
	
	memset(&sa, 0, sizeof(sa));
	memcpy(&(sa.sin_addr), &netaddr, sizeof(netaddr));
	sa.sin_family = AF_INET;
	sa.sin_port = htons((unsigned short)port);
	TCPOUT();
	
	return openStream(&sa, stream);
}


static boolean is_ipaddress(char *name) {
	
	//	TCPIN();
	short ctfields = 1;
	short fieldlen = 0;
	char *p = name;
	
	for (p = name; *p; ++p) {
		
		if (*p == '.') {
			
			if (fieldlen == 0) //leading dot, or consecutive dots
				return false;
			
			++ctfields;
			
			fieldlen = 0;
		}
		else {
			if (!isdigit(*p))
				return false;
			
			if (++fieldlen > 3) // four consecutive digits
				return false;
		}
	}
	//	TCPOUT();
	
	return ctfields == 4;
}


boolean fwsNetEventOpenNameStream(bigstring name, unsigned long port, unsigned long *stream) {
	
	if (!netEventLaunch())
		return false;
	
	char sysstring[256];
	copyptocstring (name, sysstring);
	
	TCPprintf (sprintf((char *)TCPmsg, "Entering fwsNetEventOpenNameStream at line %d.  Domain name: %s.\n", __LINE__, sysstring));
	TCPWRITEMSG();
	
	if (is_ipaddress(sysstring)) {		
		unsigned long addr;
		if (!fwsNetEventAddressEncode(name, &addr))
			return false;
		return fwsNetEventOpenAddrStream(addr, port, stream);
	}
	
	struct hostent *hp = gethostbyname(sysstring);
	long errcode = h_errno;
	
	if (hp == NULL) {
		neterror("open named stream", errcode);
		return false;
	}
	
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));	
	memcpy(&(sa.sin_addr), hp->h_addr_list[0], hp->h_length);	
	sa.sin_family = hp->h_addrtype;	
	sa.sin_port = htons((unsigned short)port);
	TCPOUT();
	return openStream(&sa, stream);
}


#pragma mark Script Calling

static boolean runBackgroundTask(void) {
	
	boolean fl = true;
	
	if (inmainthread()) {
		EventRecord ev;
		short mask = osMask|activMask|mDownMask|keyDownMask; //|highLevelEventMask|updateMask
		long sleepTime = 6;	// 1/10 of a second by default
		
		if (WaitNextEvent(mask, &ev, sleepTime, nil)) /*migh return false to indicate a null event, but that's not an error*/
			fl = shellprocessevent(&ev);
	}
	else
		fl = langbackgroundtask(true);
	
	return fl;
}


static void yield(void) {
	YieldToAnyThread();
	runBackgroundTask();
}


static boolean getcallbackcodetree(bigstring bs, Handle *htree) {
	
	TCPIN();
	Handle htext = nil;
	if (!newtexthandle(bs, &htext))
		return false;
	
	unsigned long savelines = ctscanlines;	
	unsigned short savechars = ctscanchars;
	
	hdltreenode hmodule = nil;
	boolean fl = langcompiletext(htext, false, &hmodule);
	
	ctscanlines = savelines;
	ctscanchars = savechars;
	
	if (!fl)
		return false; 
	
	fl = langpacktree((**hmodule).param1, htree); /*make a copy of the sub-tree*/
	langdisposetree(hmodule);
	TCPOUT();
	
	return fl;
}


static boolean _newProcess(hdltreenode hcode, bigstring bsname, hdlprocessrecord *hprocess) {
	
	TCPIN();
	hdlerrorstack herrorstack;
	hdltablestack htablestack;
	tyerrorrecord item;
	
	if (!newclearhandle(sizeof(typrocessrecord), (Handle *)hprocess))
		return false;
	
	hdlprocessrecord hp = *hprocess;
	
	if (!newclearhandle(sizeof(tyerrorstack), (Handle *)&herrorstack)) {
		disposehandle((Handle)hp);
		return false;
	}
	
	if (!newclearhandle(sizeof(tytablestack), (Handle *)&htablestack)) {
		disposehandle((Handle)hp);
		disposehandle((Handle)herrorstack);
		return false;
	}
	
	(**hp).hcode = hcode;
	(**hp).floneshot = true;
	(**hp).errormessagecallback = &langerrordialog;
	(**hp).debugerrormessagecallback = (langerrormessagecallback)&truenoop;
	(**hp).htablestack = htablestack;
	(**hp).herrorstack = herrorstack;
	(**hp).processstartedroutine = (langvoidcallback)&truenoop;
	(**hp).processkilledroutine = (langvoidcallback)&truenoop;
	item.errorcallback = nil;
	item.errorline = 0;
	item.errorchar = 0;	
	item.errorrefcon = 0;
	item.profilebase = 0;		
	item.profiletotal = 0;
	
	(**herrorstack).stack[(**herrorstack).toperror++] = item;
	
	copystring(bsname, (**hp).bsname);
	TCPOUT();
	return true;
}


static boolean runcallback (long listenstream, long acceptstream, long refcon) {
	
	TCPIN();
	Handle h = nil;
	if (!copyhandle(sockstack[listenstream].hcallbacktree, &h))
		return false;
	
	hdltreenode hcallbackaddress = nil;
	if (!langunpacktree(h, &hcallbackaddress))
		return false;
	
	tyvaluerecord val;
	setlongvalue(acceptstream, &val);
	
	hdltreenode hparam1 = nil;
	if (!newconstnode(val, &hparam1)) {
		langdisposetree(hcallbackaddress);
		return false;
	}
	
	setlongvalue(refcon, &val);
	
	hdltreenode hparam2 = nil;
	if (!newconstnode(val, &hparam2)) {
		langdisposetree(hcallbackaddress);
		langdisposetree(hparam1);
		return false;
	}
	
	pushlastlink(hparam2, hparam1);
	
	hdltreenode hfunctioncall = nil;
	if (!pushbinaryoperation(functionop, hcallbackaddress, hparam1, &hfunctioncall))
		return false;
	
	hdltreenode hcode = nil;
	if (!pushbinaryoperation(moduleop, hfunctioncall, nil, &hcode))
		return false;
	
	hdlprocessrecord hprocess = nil;
	if (!_newProcess(hcode, sockstack[listenstream].callback, &hprocess)) {
		langdisposetree(hcode);
		return false;
	}
	TCPOUT();
	return addprocess(hprocess);
}


#pragma mark Writing

static const unsigned long kDefaultChunkSize = 8192;
static const unsigned long kDefaultTimeoutSecs = 60;

boolean fwsNetEventWriteBufferToStream(unsigned long stream, char *buffer, unsigned long numberOfBytes, unsigned long chunkSize, unsigned long timeoutSecs) {
	
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	if (numberOfBytes < 1)
		return true;
	
	TCPINSTREAM(stream);
	SOCKET sock = socketForStream(stream);
	if (!socketIsValid(sock)) {
		neterror ("write stream", ENOTSOCK);
		return false;
	}
	
	UInt32 lastSendTickCount = TickCount();
	
	long ix = 0;
	unsigned long bytesRemaining = numberOfBytes;
	int errorCode = 0;
	
	if (chunkSize < 1)
		chunkSize = kDefaultChunkSize;
	if (timeoutSecs < 1)
		timeoutSecs = kDefaultTimeoutSecs;
	
	do {
		
		if ((TickCount() - lastSendTickCount) > (timeoutSecs * 60)) {
			errorCode = ETIMEDOUT;
			goto exit;
		}
		
		yield();
		
		fd_set writeSet = fileDescriptorSetWithSocket(sock);
		struct timeval timeout = timevalWithSeconds(0);
		int selectResult = select(sock + 1, nil, &writeSet, nil, &timeout);
		errorCode = 0;
		
		switch (selectResult) {
				
			case 1: {
				
				unsigned long bytesToWrite = (bytesRemaining < chunkSize) ? bytesRemaining : chunkSize;
				ssize_t sendResult = send(sock, buffer + ix, bytesToWrite, 0);
				errorCode = errno;
				
				if (sendResult == SOCKET_ERROR && errorCode != EWOULDBLOCK) {
#if TCPTRACKER_CONSOLE
					printf("res == SOCKET_ERROR");
#endif
					goto exit; /*unconditionally throw a script error*/
				}
				
				if (sendResult > 0) {
					lastSendTickCount = TickCount();
					TCPprintf(sprintf((char *)TCPmsg, "In fwsNetEventWriteBufferToStream at line %d, requested writing %ld bytes, wrote %ld bytes.\n", __LINE__, bytesToWrite, sendResult));
					TCPWRITEMSG();
					
					bytesRemaining -= sendResult;
					ix += sendResult;
				}
			}
				break;
				
			case 0: /*select timed out*/
				if (errno == EINPROGRESS || errno == EALREADY || errno == EINTR) {
#if TCPTRACKER_CONSOLE
					printf("fwsNetEventWriteBufferToStream EINPROGRESS || EALREADY || EINTR");
#endif
				}
				continue; //will check for timeout at top of loop
				
				
			case SOCKET_ERROR:
				if (errno == EWOULDBLOCK) {
#if TCPTRACKER_CONSOLE
					printf("fwsNetEventWriteBufferToStream EWOULDBLOCK");
#endif
					continue;
				}
				errorCode = errno;
				setStreamStatus(stream, SOCKTYPE_INACTIVE);
				goto exit;
				
			default: /*should never happen*/
				errorCode = -1;
				goto exit;
		}
		
	} while (bytesRemaining > 0);
	
	TCPprintf(sprintf((char *)TCPmsg, "Exiting fwsNetEventWriteBufferToStream at line %d. Total bytes written = %ld.\n", __LINE__, ix));
	TCPWRITEMSG();
	return true;
	
exit:
	neterror("write stream", errorCode);
	return false;
}


boolean fwsNetEventWriteHandleToStream(unsigned long stream, Handle hbuffer, unsigned long chunksize, unsigned long timeoutsecs) {
	lockhandle(hbuffer);
	boolean success = fwsNetEventWriteBufferToStream(stream, *hbuffer, gethandlesize(hbuffer), chunksize, timeoutsecs);
	unlockhandle(hbuffer);
	return success;
}


boolean fwsNetEventWriteStream(unsigned long stream, unsigned long bytesToWrite, char *buffer) {	
	return fwsNetEventWriteBufferToStream(stream, buffer, bytesToWrite, 0, 0);
}


static boolean transmitfile(unsigned long stream, ptrfilespec fs) {
	
	TCPINSTREAM(stream);
	static const long kFileBufferSize = 32767L;
	char *buffer = malloc(kFileBufferSize);
	if (buffer == nil) {
		memoryerror();
		return false;
	}
	
	hdlfilenum fnum;
	if (!openfile(fs, &fnum, true))
		return false;
	
	boolean didTransmitEntireFile = false;
	long ctread = 0;
	long ix = 0;
	
	while(true) {
		
		if (!filesetposition(fnum, ix))
			break;
		if (!filereaddata(fnum, kFileBufferSize, &ctread, buffer))
			break;
		
		if (ctread == 0) {
			didTransmitEntireFile = true;
			break;
		}
		
		ix += ctread;
		if (!fwsNetEventWriteStream(stream, ctread, buffer))
			break;
	}
	
	if (buffer != nil)
		free(buffer);
	
	return (closefile(fnum) && didTransmitEntireFile);
}


boolean fwsNetEventWriteFileToStream(unsigned long stream, Handle hprefix, Handle hsuffix, ptrfilespec fs) {
	
	TCPINSTREAM(stream);
	if (!netEventLaunch())
		return false;
	
	if (hprefix != nil && !fwsNetEventWriteHandleToStream(stream, hprefix, 0, 0))
		return false;
	
	if (!transmitfile(stream, fs))
		return false;
	
	if (hsuffix != nil && !fwsNetEventWriteHandleToStream(stream, hsuffix, 0, 0))
		return false;
	
	TCPOUTSTREAM(stream);
	return true; 
}


#pragma mark Reading

boolean fwsNetEventReadStream(unsigned long stream, unsigned long *bytesToRead, char *buffer) {
	
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	
	TCPINSTREAM(stream);
	SOCKET sock = socketForStream(stream);
	int res = recv(sock, buffer, *bytesToRead, 0);
	int errcode = errno;
	
	if (res == SOCKET_ERROR) {
		neterror("read stream", errcode);
		return false;
	}
	
	TCPprintf (if (*bytesToRead > 0) {bigstring bs; texttostring(buffer, *bytesToRead, bs); firstword (bs, '\r', bs); convertpstring (bs); sprintf ((char *)TCPmsg, "In ReadStream, read: %s\n", bs); TCPWRITEMSG ();})
	TCPprintf (sprintf((char *)TCPmsg, "Exiting fwsNetEventReadStream at line %d. Bytes requested = %ld, read = %ld.\n", __LINE__, *bytesToRead, (unsigned long) res));
	TCPWRITEMSG ();
	
	*bytesToRead = res;
	
	return true;
}



static boolean acceptsocket(long listenstream) {
	
	TCPINSTREAM(listenstream);
	
	struct sockaddr_in sa;
	boolean fl = false;
	long newstream;
	unsigned int sasize = sizeof(sa);
	SOCKET acceptsock = accept(socketForStream(listenstream), (struct sockaddr *)&sa, &sasize);
	
	if (acceptsock != INVALID_SOCKET) {
		
		lockData();
		++sockstack[listenstream].currentListenDepth;
		unlockData();
		
		if (!addsockrecord(&newstream)) {
			
			struct linger l;
			l.l_onoff = 1;
			l.l_linger = 0;
			
			sprintf((char *)TCPmsg, "In fwsacceptsocket at line %d.  Error addding new socket record %s (%ld, %ld).\n", __LINE__, stringbaseaddress (sockstack[listenstream].callback), EMFILE * -1L, sockstack[listenstream].refcon);
			TCPERRORWRITEMSG();
			
			setsockopt(acceptsock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof(l));
			fl = close(acceptsock);
			return false;
		}
		
		lockData();
		sockstack[newstream].sockID = acceptsock;
		sockstack[newstream].typeID = SOCKTYPE_OPEN;
		sockstack[newstream].listenReference = listenstream;
		sockstack[newstream].refcon = 0;
		unlockData();
		
		long refcon = sockstack[listenstream].refcon;
		TCPprintf (sprintf((char *)TCPmsg, "In fwsacceptsocket at line %d.  Accepted new socket %ld: %s (%ld, %ld).\n", __LINE__, (long)acceptsock, stringbaseaddress (sockstack[listenstream].callback), newstream, sockstack[listenstream].refcon));
		TCPWRITEMSG();
		fl = runcallback(listenstream, newstream, refcon);
	}
	else {
		/*if an error - pass this on to callback*/
		long err = errno;
		lockData();
		long refcon = sockstack[listenstream].refcon;
		unlockData();
		sprintf((char *)TCPmsg, "In fwsacceptsocket at line %d.  Error processing accept message %s (%ld, %ld).\n", __LINE__, stringbaseaddress (sockstack[listenstream].callback), err * -1L, sockstack[listenstream].refcon);
		TCPWRITEMSG();
		fl = runcallback(listenstream, err * -1L, refcon);
	}
	
	TCPprintf (sprintf((char *)TCPmsg, "Exiting fwsacceptsocket at line %d.  Return value is %s.\n", __LINE__, fl?"True":"False"));
	TCPWRITEMSG();
	return fl;
}


static void *acceptingthreadmain(void *param) {
	
	TCPINSTREAM((long)param);
	long listenstream = (long)param;
	lockData();
	sockRecord *sockrecptr = &sockstack[listenstream];
	unlockData();
	SOCKET sock = sockrecptr->sockID;
	Handle hcallback = sockrecptr->hcallbacktree; /*keep a copy in our stack so we can safely dispose it*/	
	long maxdepth = sockrecptr->maxdepth;
	
	while (sockrecptr->typeID == SOCKTYPE_LISTENING) {
		
		fd_set readset = fileDescriptorSetWithSocket(sock);
		struct timeval tv = timevalWithSeconds(0);
		int res = select(sock+1, &readset, NULL, NULL, &tv);
		
		if (sockrecptr->typeID != SOCKTYPE_LISTENING || flshellclosingall) {
			break;
		}
		
		if (res == 1) {
			(void)acceptsocket(listenstream);			
			while (sockrecptr->currentListenDepth >= maxdepth) {
				YieldToAnyThread();
			}
		}
		YieldToAnyThread();
	}	
	
	disposehandle(hcallback);
	
	lockData();
	if (sockrecptr->typeID == SOCKTYPE_LISTENSTOPPED) {
		/*we have been asked by fwsNetEventCloseListen to take responsibility of cleaning up*/
		sockrecptr->sockID = INVALID_SOCKET;
		sockrecptr->typeID = SOCKTYPE_CLOSED;
	}
	unlockData();
	TCPOUTSTREAM(listenstream);
	return nil;
}


static boolean launchacceptingthread (long stream) {	
	//pthread_create((pthread_t *)&sockstack[stream].idthread, nil, acceptingthreadmain, (void *)stream);
	TCPIN();
	OSErr err = NewThread(kCooperativeThread, gThreadEntryCallback, (void *)stream, 0, kUsePremadeThread + kCreateIfNeeded + kFPUNotNeeded, nil, (ThreadID *)(&sockstack[stream].idthread));
	TCPOUT();
	return err == noErr;
}


boolean fwsNetEventListenStream(unsigned long port, long depth, bigstring callback, unsigned long refcon, unsigned long *stream, unsigned long ipaddr, long hdatabase) {
	
	TCPIN();
	if (!netEventLaunch())
		return false;
	
	long streamref;
	if (!addsockrecord(&streamref)) {
		neterror ("initialize listen stream", EMFILE); /*Too many open sockets*/
		return false;
	}		
	
	nullterminate(callback);
	TCPprintf(sprintf((char *)TCPmsg, "Entering fwsNetEventListenStream at line %d. Port = %ld, Depth = %ld, Refcon = %ld, Callback = %s.\n", __LINE__, port, depth, refcon, stringbaseaddress(callback)));
	TCPWRITEMSG ();
	
	Handle hcallbacktree = nil;
	if (!getcallbackcodetree(callback, &hcallbacktree))
		return false;
	
	SOCKET sock = socket(PF_INET, SOCK_STREAM, 0);
	
	int errcode;
	TCPprintf(sprintf((char *)TCPmsg, "In fwsNetEventListenStream at line %d. socket call result is sock = %ld.\n", __LINE__, (long)sock));
	TCPWRITEMSG ();
	if (sock == INVALID_SOCKET) {
		errcode = h_errno;
		setStreamStatus(streamref, SOCKTYPE_INACTIVE);
		disposehandle(hcallbacktree);
		neterror("create listen stream", errcode);
		return false;
	}
	
	SOCKADDR_IN addr;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ipaddr);
	addr.sin_port = htons((unsigned short)port);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
	
	/* Advanced Mac OS X Programming 2nd ed. page 299 says to set the socket option to SO_REUSEADDR
	 so that the socket can be reused if the app crashes and is re-launched. This way there's no
	 need to wait for the previously-bound socket to timeout in the kernel (which can take a few minutes).*/
	int yes = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	
    if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		errcode = h_errno;
		neterror("bind listen stream", errcode);
		close(sock);
		setStreamStatus(streamref, SOCKTYPE_INACTIVE);
		disposehandle(hcallbacktree);
		return false;
	} 
	
	if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
		errcode = h_errno;
		neterror("setup listen stream", errcode);
		close(sock);
		setStreamStatus(streamref, SOCKTYPE_INACTIVE);
		disposehandle(hcallbacktree);
		return false;
	}
	
	lockData();
	sockstack[streamref].refcon = refcon;	
	sockstack[streamref].sockID = sock;
	setStreamStatus(streamref, SOCKTYPE_LISTENING);
	copystring (callback, sockstack[streamref].callback);
	sockstack[streamref].maxdepth = depth;
	sockstack[streamref].listenReference = 0;
	sockstack[streamref].currentListenDepth = 0;
	sockstack[streamref].hcallbacktree = hcallbacktree;
	
	*stream = streamref;
	
	sockstack[streamref].hdatabase = (hdldatabaserecord)hdatabase; /*hdldatabaserecord of the db that contains the daemon script*/
	unlockData();
	
	if (!launchacceptingthread(streamref)) {
		close(sock);
		setStreamStatus(streamref, SOCKTYPE_INACTIVE);
		disposehandle(hcallbacktree);
		return false;
	}
	
	TCPOUTSTREAM(*stream);
	return true;
}


boolean fwsNetEventStatusStream(unsigned long stream, bigstring status, unsigned long *bytesPending) {
	
	//TCPINSTREAM(stream);
	*bytesPending = 0;
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	
	lockData();
	tysocktypeid typeID = sockstack[stream].typeID;
	unlockData();
	
	switch (typeID) {
		case SOCKTYPE_INVALID:
			intneterror(INTNETERROR_INVALIDSTREAM);
			copyctopstring("INACTIVE", status);
			return false;
			
		case SOCKTYPE_UNKNOWN:
			copyctopstring("UNKNOWN", status);
			break;
			
		case SOCKTYPE_OPEN: {
			copyctopstring("OPEN", status);
			
			struct timeval tv = timevalWithSeconds(0);
			
			SOCKET sock = socketForStream(stream);
			fd_set readset = fileDescriptorSetWithSocket(sock);
			
			/* now check for data */
			
			int res = select(sock+1, &readset, NULL, NULL, &tv);
			
			if (res == SOCKET_ERROR) {
				neterror("check status on stream", h_errno);
				copyctopstring("INACTIVE", status);
				setStreamStatus(stream, SOCKTYPE_INACTIVE);
				return false;
			}
			
			if (res == 1) {  /*this means that the socket we sent has data */
				res = ioctl(sock, FIONREAD, bytesPending);
				TCPprintf (sprintf((char *)TCPmsg, "In fwsNetEventStatusStream at line %d, ioctlsocket returned %ld with the number of bytes pending as %ld.\n", __LINE__, (long)res, (long)(*bytesPending)));
				TCPWRITEMSG ();
				
				if (res == SOCKET_ERROR) {
					neterror("check status on stream", h_errno);
					copyctopstring("INACTIVE", status);
					setStreamStatus(stream, SOCKTYPE_INACTIVE);
					return false;
				}
				
				if (*bytesPending == 0) {
					copyctopstring("INACTIVE", status);
					setStreamStatus(stream, SOCKTYPE_INACTIVE);
				}
				else
					copyctopstring("DATA", status);
			}
		}
			break;
			
		case SOCKTYPE_LISTENING:
			copyctopstring("LISTENING", status);
			break;
			
		case SOCKTYPE_CLOSED:
			copyctopstring("CLOSED", status);
			break;
			
		case SOCKTYPE_LISTENSTOPPED:
			copyctopstring("STOPPED", status);
			break;
			
		case SOCKTYPE_INACTIVE:
			copyctopstring("INACTIVE", status);
			break;
			
		default:
			copyctopstring("UNKNOWN", status);
			break;
	}
	
	nullterminate(status);
	//	TCPprintf (sprintf((char *)TCPmsg, "Exiting fwsNetEventStatusStream at line %d, result is %s with bytes pending %ld.\n", __LINE__, stringbaseaddress(status), *bytesPending));
	//	TCPWRITEMSG ();
	
	return true;
}


boolean fwsNetEventCheckAndAcceptSocket(void) {
	
	//TCPIN();
	if (!frontierWinSockLoaded)
		return false;
	
	boolean fl = true;
	int i;
	for (i = 0; i < sockListenCount; i++) {
		lockData();
		long listenstream = sockListenList[i];
		boolean listenDepthIsLessThanMaxDepth = (sockstack[listenstream].currentListenDepth <= (sockstack[listenstream].maxdepth));
		unlockData();
		
		if (listenDepthIsLessThanMaxDepth) {
			
			SOCKET sock = socketForStream(listenstream);
			
			fd_set readset = fileDescriptorSetWithSocket(sock);
			struct timeval tv = timevalWithSeconds(0);
			int res = select(sock+1, &readset, NULL, NULL, &tv);
			
			if (res == 1) { /* we have a live one...*/
				
				/*Grab socket record for child socket*/
				
				long newstream = 0;
				if (!addsockrecord(&newstream)) {
					/* if an error - pass this on to callback */
					long err = EMFILE;
					TCPERRORprintf (sprintf((char *)TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d.  Error addding new socket record %s.\n", __LINE__, stringbaseaddress((sockstack[listenstream].callback))));
					TCPWRITEMSG();
					fl = runcallback(listenstream, err * -1L, sockstack[listenstream].refcon);
					continue;
				}
				
				/*Accept connection*/
				
				struct sockaddr_in sa;
				unsigned int sasize = sizeof(sa);
				SOCKET acceptsock = accept(sock, (struct sockaddr *)&sa, &sasize);
				
				if (acceptsock != INVALID_SOCKET) {
					/*sucessful connection -  assign this */
					lockData();
					++sockstack[listenstream].currentListenDepth;
					
					maxlistendepth = max(maxlistendepth, sockstack[listenstream].currentListenDepth);
					
					sockstack[newstream].refcon = 0;
					sockstack[newstream].sockID = acceptsock;
					setStreamStatus(newstream, SOCKTYPE_OPEN);
					sockstack[newstream].listenReference = listenstream;
					
					TCPprintf (sprintf((char *)TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d. Accepted new socket %ld, callback %s (%ld, %ld).\n", __LINE__, (long)acceptsock, stringbaseaddress (sockstack[listenstream].callback), (long)newstream, sockstack[listenstream].refcon));
					TCPWRITEMSG();
					
					long refcon = sockstack[listenstream].refcon;
					unlockData();
					
					/*Pass "stream" and "refcon" to callback*/
					fl = runcallback(listenstream, newstream, refcon);
					
					--i; // back up so we'll check this socket again
				}
				else
				{
					/* if an error - pass this on to callback */
					long err = errno;
					
					sprintf((char *)TCPmsg, "In fwsNetEventCheckAndAcceptSocket at line %d. Error processing accept message %s (%ld, %ld).\n", __LINE__, stringbaseaddress (sockstack[listenstream].callback), err * -1L, sockstack[listenstream].refcon);
					TCPWRITEMSG();
					
					fl = runcallback(listenstream, err * -1L, sockstack[listenstream].refcon);
					
					if (err == ENOTCONN) {
						; //TODO
					}
					setStreamStatus(newstream, SOCKTYPE_INVALID);
					
				}
			}
			else if (res == SOCKET_ERROR) {
				//The select on that socket had an error - What to do?? //TODO
			}
		}
	}
	
	//TCPOUT();
	return fl;
}


boolean fwsNetEventCloseListen(unsigned long stream) {
	
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	
	TCPINSTREAM(stream);
	
	lockData();
	SOCKET sock = socketForStream(stream);
	
	int i = 0;
	for (i = 0; i < sockListenCount; i++) {
		if (sockListenList[i] == stream) {
			if (i == (sockListenCount - 1)) {
				//easy case - end of list to remove
				--sockListenCount;
			}
			else {
				//Move remaining down in the list
				int j = 0;
				for (j = i; j < sockListenCount - 1; j++)
					sockListenList[j] = sockListenList[j+1];
				
				--sockListenCount;
			}
			
			break;		//It can only be in the list once.
		}
	}
	unlockData();
	
	struct linger l;
	l.l_onoff = 1;
	l.l_linger = 0;
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&l, sizeof(l));
	
	int res = close(sock);
	int errcode = errno;
	
	setStreamStatus(stream, SOCKTYPE_LISTENSTOPPED);
	
	if (res == SOCKET_ERROR) {
		neterror("close listen", errcode);
		return false;
	}
	
	TCPOUTSTREAM(stream);
	return true;
}


boolean fwsNetEventGetPeerAddress(unsigned long stream, unsigned long *peeraddress, unsigned long *peerport) {
	
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	
	TCPINSTREAM(stream);
	
	SOCKET sock = socketForStream(stream);
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	unsigned int nSockAddrLen = sizeof(sockAddr);
	int res = getpeername(sock, (struct sockaddr *)&sockAddr, &nSockAddrLen);
	
	if (res == SOCKET_ERROR) {
		neterror("get peer address", errno);
		return false;
	}
	
	*peerport = (unsigned long)ntohs(sockAddr.sin_port);
	*peeraddress = ntohl(sockAddr.sin_addr.s_addr);
	
	TCPOUTSTREAM(stream);
	
	return true;
}


typedef enum _fwsReadStreamConditionType {
	fwsReadStreamConditionClosed,
	fwsReadStreamConditionNumberOfBytes,
	fwsReadStreamConditionPatternMatch
} fwsReadStreamConditionType;

static const unsigned long kDefaultReadChunkSize = 8192;
static const unsigned long kDefaultReadTimeoutSecs = 60;

static boolean readStreamUntilCondition(unsigned long stream, Handle hbuffer, fwsReadStreamConditionType conditionType, unsigned long numberOfBytesToRead, Handle hpattern, unsigned long timeoutSecs) {
	
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	TCPINSTREAM(stream);
	
	SOCKET sock = socketForStream(stream);
	if (sock == INVALID_SOCKET) {
		neterror("read stream", ENOTSOCK);
		return false;
	}
	
	if (timeoutSecs < 1)
		timeoutSecs = kDefaultReadTimeoutSecs;
	long currentHandleSize = gethandlesize(hbuffer);
	unsigned long numberOfBytesRead = currentHandleSize; //applies to entire buffer, which might have bytes previously-read
	UInt32 lastReadTickCount = TickCount();
	int errorCode = 0;
	
	while (true) {
		
		yield();
		if ((TickCount() - lastReadTickCount) >= (timeoutSecs * 60)) {
			errorCode = ETIMEDOUT;
			goto exit_error;
		}
		
		if (conditionType == fwsReadStreamConditionPatternMatch) {
			if (searchhandle(hbuffer, hpattern, 0, currentHandleSize) >= 0)
				break;
		}
		
		else if (conditionType == fwsReadStreamConditionNumberOfBytes) {
			if (numberOfBytesRead >= numberOfBytesToRead)
				break;
		}
		
		struct timeval timeout = timevalWithSeconds(0);	
		fd_set readset = fileDescriptorSetWithSocket(sock);
		int selectResult = select(sock + 1, &readset, nil, nil, &timeout);
		
		switch (selectResult) {
			case 1: {
				
				errorCode = 0;
				long numberOfBytesAvailable = 0;
				int ioctlResult = ioctl(sock, FIONREAD, &numberOfBytesAvailable);
				TCPprintf(sprintf((char *)TCPmsg, "In readStreamUntilCondition at line %d, ioctl returned %d with the number of bytes pending as %ld.\n", __LINE__, ioctlResult, numberOfBytesAvailable));
				TCPWRITEMSG();
				
				if (ioctlResult == SOCKET_ERROR) {
					errorCode = errno;
					if (errorCode == EWOULDBLOCK)
						continue;
					setStreamStatus(stream, SOCKTYPE_INACTIVE);
					goto exit_error;
				}
				if (numberOfBytesAvailable == 0) {
					errorCode = errno;
					setStreamStatus(stream, SOCKTYPE_INACTIVE);
					TCPprintf(sprintf((char *)TCPmsg, "In readStreamUntilCondition at line %d, ioctlResult is %d, numberOfBytesAvailable is 0, errno is %d and condition is %d.\n", __LINE__, ioctlResult, errorCode, conditionType));
					TCPWRITEMSG();
					//					if ((errorCode == EISCONN) && (conditionType == fwsReadStreamConditionNumberOfBytes))
					//						goto exit; //probably already read in previous readStreamUntilPattern call
					if (conditionType == fwsReadStreamConditionNumberOfBytes || conditionType == fwsReadStreamConditionPatternMatch)
						goto error_closedprematurely;
					goto exit; //closed
				}
				
				if (conditionType == fwsReadStreamConditionNumberOfBytes) {
					if (numberOfBytesRead + numberOfBytesAvailable > numberOfBytesToRead) {
						numberOfBytesAvailable = numberOfBytesToRead - numberOfBytesRead;
					}
				}
				
				if (!sethandlesize(hbuffer, currentHandleSize + numberOfBytesAvailable))
					return false;
				
				lockhandle(hbuffer);
				long recvResult = recv(sock, &((*hbuffer)[currentHandleSize]), numberOfBytesAvailable, 0);
				unlockhandle(hbuffer);
				
				TCPprintf(sprintf((char *)TCPmsg, "In readStreamUntilCondition at line %d, recv read %ld bytes.\n", __LINE__, recvResult));
				TCPWRITEMSG();
				
				if ((recvResult >= 0) && (recvResult < numberOfBytesAvailable))
					sethandlesize(hbuffer, currentHandleSize + recvResult);
				currentHandleSize = gethandlesize(hbuffer);
				numberOfBytesRead = currentHandleSize;
				if (recvResult > 0) {
					//numberOfBytesRead = numberOfBytesRead + recvResult;
					lastReadTickCount = TickCount();
				}
				
				if (recvResult == SOCKET_ERROR) {
					errorCode = errno;
					if (errorCode == EWOULDBLOCK)
						continue;
					goto exit_error;
				}
				
			}				
				break;
			case 0:
			case SOCKET_ERROR:
				errorCode = errno;
				if (errorCode == EWOULDBLOCK)
					continue;
				break;
			default:
				errorCode = -1;
				goto error_closedprematurely;
				break;
		}		
	}
	
exit:
	TCPprintf(sprintf((char *)TCPmsg, "Exiting readStreamUntilCondition at line %d. Total bytes read = %ld.\n", __LINE__, numberOfBytesRead));
	TCPWRITEMSG();
	return true;
	
exit_error:
	setStreamStatus(stream, SOCKTYPE_INACTIVE);
	neterror("read stream", errorCode);	
	return false;
	
error_closedprematurely:
	setStreamStatus(stream, SOCKTYPE_INACTIVE);
	plainneterror(STR_P_ERROR_CLOSED_PREMATURELY);	
	return false;
}


boolean fwsNetEventReadStreamUntil(unsigned long stream, Handle hbuffer, Handle hpattern, unsigned long timeoutsecs) {
	return readStreamUntilCondition(stream, hbuffer, fwsReadStreamConditionPatternMatch, 0, hpattern, timeoutsecs);
}


boolean fwsNetEventReadStreamBytes(unsigned long stream, Handle hbuffer, long ctbytes, unsigned long timeoutsecs) {
	return readStreamUntilCondition(stream, hbuffer, fwsReadStreamConditionNumberOfBytes, ctbytes, nil, timeoutsecs);
}


boolean fwsNetEventReadStreamUntilClosed (unsigned long stream, Handle hbuffer, unsigned long timeoutsecs) {
	return readStreamUntilCondition(stream, hbuffer, fwsReadStreamConditionClosed, 0, nil, timeoutsecs);
}


boolean fwsNetEventInetdRead(unsigned long stream, Handle hbuffer, unsigned long timeoutsecs) {
	
	TCPINSTREAM(stream);
	if (!netEventLaunch() || !streamIsValid(stream))
		return false;
	
	SOCKET sock = socketForStream(stream);
	if (sock == INVALID_SOCKET) {
		neterror("read stream", ENOTSOCK);
		return false;
	}
	
	UInt32 lastReadTickCount = TickCount();
	
	//	struct timeval tv = timevalWithSeconds(timeoutsecs);
	fd_set readset = fileDescriptorSetWithSocket(sock);
	int res, errcode;
	long ix = gethandlesize(hbuffer);
#if TCPTRACKER == 1 || TCPTRACKER == 3
	long ixstart = ix;
#endif
	unsigned long bytes;
	
	while (true) {
		
		if ((TickCount() - lastReadTickCount) > (timeoutsecs * 60)) {
			errcode = ETIMEDOUT;
			goto exit;
		}
		
		struct timeval tv = timevalWithSeconds(0);
		res = select(sock+1, &readset, NULL, NULL, &tv);
		
		switch (res) {
				
			case 1:  /*this means that the socket has data */
			{
				res = ioctl(sock, FIONREAD, &bytes);
				
				TCPprintf (sprintf((char *)TCPmsg, "In fwsNetEventInetdRead at line %d, ioctlsocket returned %ld with the number of bytes pending as %ld.\n", __LINE__, (long)res, bytes));
				TCPWRITEMSG ();
				if (res == SOCKET_ERROR) {
					errcode = errno;
					setStreamStatus(stream, SOCKTYPE_INACTIVE);
					goto exit;
				}
				
				if (bytes == 0) { /*closed prematurely*/
					setStreamStatus(stream, SOCKTYPE_INACTIVE);
					goto done; /*not an error in this case*/
				}
				
				if (!sethandlesize(hbuffer, ix + bytes))
					return false;
				
				lockhandle(hbuffer);
				res = recv(sock, &((*hbuffer)[ix]), bytes, 0);
				unlockhandle(hbuffer);
				
				if (res == SOCKET_ERROR) {
					errcode = errno;
					goto exit;
				}
				
				ix += res;
				lastReadTickCount = TickCount();
				//				tv.tv_sec = 1; /*reduce timeout to one second after the first packet has been received*/
				
				TCPprintf (sprintf((char *)TCPmsg, "In fwsNetEventInetdRead at line %d, requested reading %ld bytes, read %ld bytes.\n", __LINE__, bytes, (long)res));
				TCPWRITEMSG ();
				
			}
				break;
				
			case 0: /*select timed out*/
				goto done; /*not an error in this case*/
				
			case SOCKET_ERROR:
				errcode = errno;
				setStreamStatus(stream, SOCKTYPE_INACTIVE);
				goto exit;
				
			default:
				errcode = -1; /* should never happen*/
				goto exit;
		}
	}
	
done:
	if (!sethandlesize(hbuffer, ix))
		return false;
	TCPprintf (sprintf((char *)TCPmsg, "Exiting fwsNetEventInetdRead at line %d. Total bytes read = %ld.\n", __LINE__, ix - ixstart));
	TCPWRITEMSG ();
	return true;
	
exit:
	neterror("read stream", errcode);
	return false;
}


#pragma mark Stats

boolean fwsNetEventGetStats(unsigned long stream, bigstring bs) {
	
	TCPINSTREAM(stream);
	unsigned long ctopen = 0;
	unsigned long ctdata = 0;
	unsigned long ctclosed = 0;
	unsigned long ctinactive = 0;
	unsigned long i;
	
	setemptystring(bs);
	
	lockData();
	for (i = 1; i <= FRONTIER_MAX_STREAM; i++) {
		
		if (sockstack[i].listenReference == (long)stream) {
			
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
			}
		}
	}
	unlockData();
	
	pushlong(ctopen, bs);
	pushchar(',', bs);
	pushlong(ctdata, bs);
	pushchar(',', bs);
	pushlong(ctclosed, bs);
	pushchar(',', bs);	
	pushlong(ctinactive, bs);
	TCPOUTSTREAM(stream);
	return true;
}


long fwsNetEventGetConnectionCount(void) {
	
	unsigned int i = 0;
	unsigned int numberOfConnections = 0;
	
	lockData();
	
	for (i = 1; i <= FRONTIER_MAX_STREAM; i++) {
		
		switch (sockstack[i].typeID) {
				
			case SOCKTYPE_OPEN:
			case SOCKTYPE_DATA:
				numberOfConnections++;
				break;
			default:
				break;
		}
	}
	
	unlockData();
	
	return numberOfConnections;
}

