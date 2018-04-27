
/*	$Id$    */

#ifndef PGMTRAPS_H
#define PGMTRAPS_H


/*************** PAIGE machine-dependent memory definitions ******************/

/* This file defines all memory-related functions such as allocation, resizing,
etc. for memory_ref types. It is controlled by CPUDefs.h that defines the
platform.  */

/* File I/O has moved to pgIO.h - TRS OITC */


#include "CPUDEFS.H"
#include "pgMemMgr.h"

#define DEF_MASTER_QTY		256

#ifdef MAC_PLATFORM


#define MASTER_ENTRY_SIZE	sizeof(memory_ref)
#define MASTER_ENTRY_NULL	MEM_NULL;
#define SPARE_TIRE_SIZE		0x00004000

#define pgMemoryPtr(ref) (mem_rec_ptr) *((Handle) ref)
#define pgFreePtr(ref) {}
#define pgAllocMemory(bytesize) (generic_var)NewHandle(bytesize)
#define pgAllocMemoryClear(bytesize) (generic_var)NewHandleClear(bytesize)
#define pgLockMemory(ref) HLock((Handle) ref)
#define pgLockMemoryHi(ref) HLockHi((Handle) ref)
#define pgUnlockMemory(ref) HUnlock((Handle) ref)
#define pgResizeMemory(globals, ref, bytesize) { \
		SetHandleSize((Handle) ref, bytesize);          \
		}
#define pgMemorySize(ref) GetHandleSize((Handle) ref)
#define pgFreeMemory(ref) DisposeHandle((Handle) ref)
#define pgMemoryError(globals) MemError()
#define pgRecoverMemory(ptr) (memory_ref)RecoverHandle((Ptr) ptr)

#endif

#ifdef WINDOWS_PLATFORM

#ifdef WINDOWS_EMULATE_COMPILE

#include "WinMac.h"

#else
#include <WINDOWS.H>
#endif

#include <string.h>


#define MASTER_ENTRY_SIZE	(sizeof(HANDLE))
#define MASTER_ENTRY_NULL	(HANDLE)NULL;
#define SPARE_TIRE_SIZE		0x00000400
#define MAX_HANDLE_ENTRIES	(DEF_MASTER_QTY - 1)

#define pgMemoryPtr(ref) (mem_rec_ptr) GlobalLock((HANDLE) *((HANDLE PG_FAR *)ref))
#define pgFreePtr(ref) GlobalUnlock(*((HANDLE PG_FAR *)ref))
#define pgAllocMemory(bytesize) GlobalAlloc(GMEM_MOVEABLE, bytesize)
#define pgAllocMemoryClear(bytesize) GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytesize)
#define pgLockMemory(ref) {}
#define pgLockMemoryHi(ref) {}
#define pgUnlockMemory(ref) {}
#define pgResizeMemory(globals, ref, bytesize) {  \
	if (*((HANDLE PG_FAR *)ref) = GlobalReAlloc((HANDLE) *((HANDLE PG_FAR *)ref), bytesize, 0)) \
		globals->last_error = NO_ERROR;	\
	else								\
		globals->last_error = NO_MEMORY_ERR; \
		}

#define pgMemorySize(ref) GlobalSize(*((HANDLE PG_FAR *)ref))
#define pgFreeMemory(ref) GlobalFree(*((HANDLE PG_FAR *)ref))
#define pgMemoryError(globals) globals->last_error


#ifdef WIN32_COMPILE
#define BlockMove(src, dest, size) MoveMemory(dest, src, size)
#define pgRecoverMemory(ptr) ((HGLOBAL)GlobalHandle(ptr))
#else
#define pgRecoverMemory(ptr) ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(ptr))))
#define BlockMove(src, dest, size) hmemcpy(dest, src, size)
#endif

/* Misc. */

#ifndef WINDOWS_EMULATE_COMPILE
#define Debugger() DebugBreak()
#define DebugStr OutputDebugString
#define ExitToShell()  PostQuitMessage(1)
#endif

#endif

#endif
