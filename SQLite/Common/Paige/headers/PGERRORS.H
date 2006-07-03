
/*	$Id$    */

/* This header file defines "generic" errors.

12/10/93 -- Codes have been #idef'd for platform-specific errors.

DataPak Software, Inc.  Copyright 1993-1994. 		*/

/* Some compilers (Symantic) treat hex constants as unsigned thus making certain error comparisons fail
	corrected by hard typing 6 Jan 1994 by T.R. Shaw, OITC, Paige Board Member */


#ifndef PGERRORS_H
#define PGERRORS_H

#include "CPUDEFS.H"

#ifndef NO_ERROR
#define NO_ERROR				0x0000		/* No error */
#endif

#ifdef MAC_PLATFORM

#if !TARGET_API_MAC_CARBON
#include <MacErrors.h>		/* 2004-09-29 AR: renamed from errors.h */
#endif

/* Memory errors */

#define NO_MEMORY_ERR			memFullErr	 /* Insufficient memory */
#define NIL_ADDRESS_ERR			nilHandleErr /* Address is NIL (not valid) */

/* Paige-specific memory management and exception handler errors:  */

#define NOT_ENOUGH_PURGED_ERR	(pg_error)0x0101		/* Cannot purge enough space */
#define NO_PURGE_FILE_ERR		(pg_error)0x0102		/* Purge file not available */
#define LOCKED_BLOCK_ERR		(pg_error)0x0103		/* Cannot resize locked block */
#define BAD_ADDRESS_ERR			(pg_error)0x0104		/* Address is bogus (not valid) */
#define BAD_LINK_ERR			(pg_error)0x0105		/* Something wrong with internal ref */
#define CHECKSUM_ERR			(pg_error)0x0200		/* Memory_ref checksum error */
#define ACCESS_ERR				(pg_error)0x0201		/* Access failed on memory_ref */
#define BAD_REF_ERR				(pg_error)0x0202		/* Bogus memory_ref */
#define REF_DISPOSED_ERR		(pg_error)0x0203		/* Memory_ref has been disposed */
#define FILE_PURGE_ERR			(pg_error)0x0204		/* Error on file when purging */
#define FILE_UNPURGE_ERR		(pg_error)0x0205		/* Error on reading purged file */
#define RANGE_ERR				(pg_error)0x0206		/* Access out of range */
#define PURGED_MEMORY_ERR		(pg_error)0x0207		/* Attempt to operate on a purged block */
#define DEBUG_ZERO_ERR			(pg_error)0x0208		/* Access is zero debug check */
#define DEBUG_NZ_ERR			(pg_error)0x0209		/* Access is non-zero debug check */
#define NO_ERR_HANDLER_ERR		(pg_error)0x020A		/* No exception handler */
#define PG_PSTRING_TOO_BIG_ERR	(pg_error)0x020B		/* Memory ref conversion to Pascal string error */
#define BAD_PAIGE_VERSION_ERR	(pg_error)0x0210		/* Incorrect PAIGE version for operaration */
#define PROGRAM_BUG_ASSERT		(pg_error)0x0211		/* Used for general assertion */

/* File I/O Errors */

#define NO_SPACE_ERR			dskFulErr				/* File mediem has insufficient space */
#define NOT_OPEN_ERR			fnOpnErr				/* Requested file not open */
#define FILE_LOCK_ERR			fLckdErr				/* File is locked (somehow) */
#define WRITE_PROTECT_ERR		wPrErr					/* Medium is write protected */
#define ACCESS_DENIED_ERR		permErr					/* Access permission denied */
#define EOF_ERR					eofErr					/* Attempt to go past end of file */
#define IO_ERR					ioErr					/* Hard I/O Error */
#define NO_FILE_ERR				fnfErr					/* File not found */

/* Paige-specific file errors: */

#define NO_HANDLER_ERR			(pg_error)0x0300		/* A key handler not found */
#define BAD_TYPE_ERR			(pg_error)0x0301		/* File not of appropriate type */
#define UNICODE_ERR				(pg_error)0x0309		/* File contains unicode and this platform cannot handle */
#define SOFT_EOF_ERR			(pg_error)0x03FF		/* Logical end of file "error" */

/* Other Paige errors: */

#define LLIST_ERR				(pg_error)0x03FC		/* Linked List is invalid */
#define DUP_PG_REF_ERR			(pg_error)0x03FD		/* Duplicate entry on pgAddSection */

#define USER_CANCELLED_ERR		userCanceledErr

#define RESOURCE_NOT_FOUND		resNotFound				/* Attempt to access a non existant resource */

#define UNKNOWN_ERR				(pg_error)0x7FFF		/* Unknown error that Paige cannot understand */

#endif

#ifdef WINDOWS_PLATFORM

#include <stdio.h>
#include <errno.h>


#ifndef EACCES
#define EACCES					(pg_error)0x0305
#endif

#ifndef EIO
#define EIO						(pg_error)0x0307
#endif

/* CHANGE APPROPRIATE TYPES TO OS-SPECIFIC ERROR CODES  */


#define NO_MEMORY_ERR			(pg_error)0x0100		/* Insufficient memory */
#define NOT_ENOUGH_PURGED_ERR	(pg_error)0x0101		/* Cannot purge enough space */
#define NO_PURGE_FILE_ERR		(pg_error)0x0102		/* Purge file not available */
#define LOCKED_BLOCK_ERR		(pg_error)0x0103		/* Cannot resize locked block */
#define NIL_ADDRESS_ERR			(pg_error)0x0104		/* Address is NIL (not valid) */
#define BAD_ADDRESS_ERR			(pg_error)0x0105		/* Address is bogus (not valid) */
#define BAD_LINK_ERR			(pg_error)0x0106		/* Something wrong with internal ref */

/* Paige memory_ref-specific errors */

#define CHECKSUM_ERR			(pg_error)0x0200		/* Memory_ref checksum error */
#define ACCESS_ERR				(pg_error)0x0201		/* Access failed on memory_ref */
#define BAD_REF_ERR				(pg_error)0x0202		/* Bogus memory_ref */
#define REF_DISPOSED_ERR		(pg_error)0x0203		/* Memory_ref has been disposed */
#define FILE_PURGE_ERR			(pg_error)0x0204		/* Error on file when purging */
#define FILE_UNPURGE_ERR		(pg_error)0x0205		/* Error on reading purged file */
#define RANGE_ERR				(pg_error)0x0206		/* Access out of range */
#define PURGED_MEMORY_ERR		(pg_error)0x0207		/* Attempt to operate on a purged block */
#define DEBUG_ZERO_ERR			(pg_error)0x0208		/* Access is zero debug check */
#define DEBUG_NZ_ERR			(pg_error)0x0209		/* Access is non-zero debug check */
#define NO_ERR_HANDLER_ERR		(pg_error)0x020A		/* No exception handler */
#define PG_PSTRING_TOO_BIG_ERR	(pg_error)0x020B		/* Memory ref conversion to Pascal string error */
#define BAD_PAIGE_VERSION_ERR	(pg_error)0x0210		/* Incorrect PAIGE version for operaration */
#define PROGRAM_BUG_ASSERT		(pg_error)0x0211		/* Used for general assertion */

/* File I/O Errors */

#define NO_HANDLER_ERR			(pg_error)0x0300		/* A key handler not found */
#define NO_SPACE_ERR			(pg_error)0x0301		/* File has insufficient space */
#define NOT_OPEN_ERR			(pg_error)0x0302		/* Requested file not open */
#define FILE_LOCK_ERR			(pg_error)0x0303		/* File locked */
#define WRITE_PROTECT_ERR		(pg_error)0x0304		/* Medium write-protected */
#define ACCESS_DENIED_ERR		(pg_error)EACCES		/* Access permission denied */
#define EOF_ERR					(pg_error)EOF			/* Attempt to go past end of file */
#define IO_ERR					(pg_error)EIO			/* Hard I/O Error */
#define BAD_TYPE_ERR			(pg_error)0x0308		/* File not of appropriate type */
#define UNICODE_ERR				(pg_error)0x0309		/* File contains unicode and this platform cannot handle */
#define NO_FILE_ERR				(pg_error)0x03FE		/* File not found */
#define SOFT_EOF_ERR			(pg_error)0x03FF		/* Logical end of file "error" */

/* Other Paige errors: */

#define USER_CANCELLED_ERR		(pg_error)(-128)

#define LLIST_ERR				(pg_error)0x03FC		/* Linked List is invalid */
#define DUP_PG_REF_ERR			(pg_error)0x03FD		/* Duplicate entry on pgAddSection */

#ifdef WIN16_COMPILE
#define RESOURCE_NOT_FOUND		(pg_error)0x0500		/* Attempt to access a non existant resource */
#else
#define RESOURCE_NOT_FOUND		(pg_error)ERROR_RESOURCE_DATA_NOT_FOUND
#endif

#define UNKNOWN_ERR				(pg_error)0x7FFF		/* Unknown error that Paige cannot understand */

#endif

/* Runtime debugging errors (not memory manager related): */

#define LOCKED_PG_ERROR			(pg_error)0x0400		/* Attempt to change a locked pg_ref */
#define ILLEGAL_RE_ENTER_ERROR	(pg_error)0x0401		/* Illegal re-entrance */
#define BAD_PARAM_ERROR			(pg_error)0x0402		/* Bad parameter in a function */
#define GLOBALS_MISMATCH_ERROR	(pg_error)0x0403		/* Globals in doc do not match pg_globals */
#define DUP_KEY_HANDLER_ERROR	(pg_error)0x0404		/* pgWrite/Read a key that already has a handler */
#define BAD_REFCON_ID_ERROR		(pg_error)0x0405		/* Bad refcon num for exclusion/containers */
#define STRUCT_INTEGRITY_ERR	(pg_error)0x0406		/* Style structures bad */
#define USER_BREAK_ERR			(pg_error)0x0407		/* User-invoked debug break */
#define CARET_SYNC_ERR			(pg_error)0x0408		/* Caret and caret bit out of sync */

#endif
