
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

/* targets: none, about window, log file, modal dialog, debugger */

#define LAND_LOGTARGET_NONE			0
#define LAND_LOGTARGET_ABOUT		1
#define LAND_LOGTARGET_FILE			2
#define LAND_LOGTARGET_DIALOG		4
#define LAND_LOGTARGET_DEBUGGER		8


/* levels: off, everything, medium, low */

#define LAND_LOGLEVEL_OFF	0
#define LAND_LOGLEVEL_1		1
#define LAND_LOGLEVEL_2		2
#define LAND_LOGLEVEL_3		3


/* configuration for categories */

#define LAND_TCPLOG_NAME	"tcp"
#define LAND_TCPLOG_LEVEL	LAND_LOGLEVEL_OFF
#define LAND_TCPLOG_TARGET	LAND_LOGTARGET_NONE

#define LAND_DBLOG_NAME		"db"
#define LAND_DBLOG_LEVEL	LAND_LOGLEVEL_OFF
#define LAND_DBLOG_TARGET	LAND_LOGTARGET_NONE

#define LAND_THREADSLOG_NAME		"threads"
#define LAND_THREADSLOG_LEVEL		LAND_LOGLEVEL_OFF
#define LAND_THREADSLOG_TARGET		LAND_LOGTARGET_NONE

#define LAND_GENERALLOG_NAME		"general"
#define LAND_GENERALLOG_LEVEL		LAND_LOGLEVEL_OFF
#define LAND_GENERALLOG_TARGET		LAND_LOGTARGET_NONE


/* basic definitions */

#define LAND_MSG(m, t, n)		logmessage ((m), __FILE__, __LINE__, (t), (n))
#define LAND_ASSERT(e, t, n)	((void) ((e) ? 0 : (logassert ((#e), __FILE__, __LINE__, (t), (n))))


/* definitions for GENERAL category */

#if (LAND_GENERALLOG_LEVEL >= LAND_LOGLEVEL_1)
	#define MSG_1(x)		LAND_MSG((x), LAND_GENERALLOG_TARGET, LAND_GENERALLOG_NAME)
	#define ASSERT_1(x)		LAND_ASSERT((x), LAND_GENERALLOG_TARGET, LAND_GENERALLOG_NAME))
#else
	#define MSG_1(x)
	#define ASSERT_1(x)
#endif

#if (LAND_GENERALLOG_LEVEL >= LAND_LOGLEVEL_2)
	#define MSG_2(x)		LAND_MSG((x), LAND_GENERALLOG_TARGET, LAND_GENERALLOG_NAME)
	#define ASSERT_2(x)		LAND_ASSERT((x), LAND_GENERALLOG_TARGET, LAND_GENERALLOG_NAME))
#else
	#define MSG_2(x)
	#define ASSERT_2(x)
#endif

#if (LAND_GENERALLOG_LEVEL >= LAND_LOGLEVEL_3)
	#define MSG_3(x)		LAND_MSG((x), LAND_GENERALLOG_TARGET, LAND_GENERALLOG_NAME)
	#define ASSERT_3(x)		LAND_ASSERT((x), LAND_GENERALLOG_TARGET, LAND_GENERALLOG_NAME))
#else
	#define MSG_3(x)
	#define ASSERT_3(x)
#endif


/* definitions for TCP category */

#if (LAND_TCPLOG_LEVEL >= LAND_LOGLEVEL_1)
	#define TCP_MSG_1(x)		LAND_MSG((x), LAND_TCPLOG_TARGET, LAND_TCPLOG_NAME)
	#define TCP_ASSERT_1(x)		LAND_ASSERT((x), LAND_TCPLOG_TARGET, LAND_TCPLOG_NAME))
#else
	#define TCP_MSG_1(x)
	#define TCP_ASSERT_1(x)
#endif

#if (LAND_TCPLOG_LEVEL >= LAND_LOGLEVEL_2)
	#define TCP_MSG_2(x)		LAND_MSG((x), LAND_TCPLOG_TARGET, LAND_TCPLOG_NAME)
	#define TCP_ASSERT_2(x)		LAND_ASSERT((x), LAND_TCPLOG_TARGET, LAND_TCPLOG_NAME))
#else
	#define TCP_MSG_2(x)
	#define TCP_ASSERT_2(x)
#endif

#if (LAND_TCPLOG_LEVEL >= LAND_LOGLEVEL_3)
	#define TCP_MSG_3(x)		LAND_MSG((x), LAND_TCPLOG_TARGET, LAND_TCPLOG_NAME)
	#define TCP_ASSERT_3(x)		LAND_ASSERT((x), LAND_TCPLOG_TARGET, LAND_TCPLOG_NAME))
#else
	#define TCP_MSG_3(x)
	#define TCP_ASSERT_3(x)
#endif


/* definitions for DB category */

#if (LAND_DBLOG_LEVEL >= LAND_LOGLEVEL_1)
	#define DB_MSG_1(x)		LAND_MSG((x), LAND_DBLOG_TARGET, LAND_DBLOG_NAME)
	#define DB_ASSERT_1(x)		LAND_ASSERT((x), LAND_DBLOG_TARGET, LAND_DBLOG_NAME))
#else
	#define DB_MSG_1(x)
	#define DB_ASSERT_1(x)
#endif

#if (LAND_DBLOG_LEVEL >= LAND_LOGLEVEL_2)
	#define DB_MSG_2(x)		LAND_MSG((x), LAND_DBLOG_TARGET, LAND_DBLOG_NAME)
	#define DB_ASSERT_2(x)		LAND_ASSERT((x), LAND_DBLOG_TARGET, LAND_DBLOG_NAME))
#else
	#define DB_MSG_2(x)
	#define DB_ASSERT_2(x)
#endif

#if (LAND_DBLOG_LEVEL >= LAND_LOGLEVEL_3)
	#define DB_MSG_3(x)		LAND_MSG((x), LAND_DBLOG_TARGET, LAND_DBLOG_NAME)
	#define DB_ASSERT_3(x)		LAND_ASSERT((x), LAND_DBLOG_TARGET, LAND_DBLOG_NAME))
#else
	#define DB_MSG_3(x)
	#define DB_ASSERT_3(x)
#endif


/* definitions for THREADS category */

#if (LAND_THREADSLOG_LEVEL >= LAND_LOGLEVEL_1)
	#define THREADS_MSG_1(x)		LAND_MSG((x), LAND_THREADSLOG_TARGET, LAND_THREADSLOG_NAME)
	#define THREADS_ASSERT_1(x)		LAND_ASSERT((x), LAND_THREADSLOG_TARGET, LAND_THREADSLOG_NAME))
#else
	#define THREADS_MSG_1(x)
	#define THREADS_ASSERT_1(x)
#endif

#if (LAND_THREADSLOG_LEVEL >= LAND_LOGLEVEL_2)
	#define THREADS_MSG_2(x)		LAND_MSG((x), LAND_THREADSLOG_TARGET, LAND_THREADSLOG_NAME)
	#define THREADS_ASSERT_2(x)		LAND_ASSERT((x), LAND_THREADSLOG_TARGET, LAND_THREADSLOG_NAME))
#else
	#define THREADS_MSG_2(x)
	#define THREADS_ASSERT_2(x)
#endif

#if (LAND_THREADSLOG_LEVEL >= LAND_LOGLEVEL_3)
	#define THREADS_MSG_3(x)		LAND_MSG((x), LAND_THREADSLOG_TARGET, LAND_THREADSLOG_NAME)
	#define THREADS_ASSERT_3(x)		LAND_ASSERT((x), LAND_THREADSLOG_TARGET, LAND_THREADSLOG_NAME))
#else
	#define THREADS_MSG_3(x)
	#define THREADS_ASSERT_3(x)
#endif


/* function templates */

extern void logmessage (char *, char *, long, long, char *);

extern long logassert (char *, char *, long, long, char *);

extern void logstartup (void);

extern void logshutdown (void);

