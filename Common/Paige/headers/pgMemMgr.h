/* pgMemMgr.h defines the API for Paige Memory Manager. This header
file also #includes "pgMachineDefs."  */

/* Updated by TR Shaw, OITC, Inc. 12 Feb/20 Apr 1994 for C/C++/Pascal linkage and PPC control
	and for size and speed */

/* New support routines provided by TR Shaw, OITC, Inc. 6 Feb 1995 */

#ifndef PGMEMMGR_H
#define PGMEMMGR_H

#include "CPUDefs.h"


#ifdef C_LIBRARY
#include <String.h>
#include <stdlib.h>
#endif

#include "pgMTraps.h"
#include "pgSetJmp.h"

#ifdef PG_DEBUG
#define PG_DEBUG_STATICS
	/* Static globals are OK in debug mode */
#endif

//#define PG_BIG_CHECKSUMS				
	/* Make huge checksums for memory trash detect */

#define BIG_CHECKSUM_BYTE	0xAA		/* Byte placed into big checksum buffer */

#ifdef PG_BIG_CHECKSUMS
#define CHECKSUM_SIZE		2048
#else
#define CHECKSUM_SIZE		sizeof(long)
#endif

/* Debugging flags: */

#define NO_DEBUG			0x0000		/* No debugging		*/
#define BAD_REF_CHECK		0x0001		/* Report bugus refs */
#define OVERWRITE_CHECK		0x0002		/* Do a checksum at end of block */
#define ACCESS_CTR_CHECK	0x0004		/* Test access counter */
#define NOT_DISPOSED_CHECK	0x0008		/* Check ref is not disposed */
#define ALL_MEMORY_CHECK	0x0010		/* Check ALL memory_refs each call */
#define FILL_ONES_ENABLE	0x0020		/* Fill with 1's into mem_rec after dispose */
#define ALL_ONES_ENABLE		0x0040		/* Fill ALL disposed memory with 1's */
#define RANGE_CHECK			0x0080		/* Check range on appropriate calls */
#define CURSOR_DEBUG		0x0100		/* Debug cursor out-of-sync */

#define DEF_DEBUG_BITS		(BAD_REF_CHECK | OVERWRITE_CHECK | ACCESS_CTR_CHECK | NOT_DISPOSED_CHECK | RANGE_CHECK)

#define MINIMUM_ID			1			/* Minimum mem_id */

#define USE_ALL_RECS		-1			/* Use whole ref in UseMemoryRecord */
#define REASONABLE_REC_SIZE 0x00200000	/* Don't want to go much past this */
#define PURGE_VALUE_MASK	0x00FF		/* Purge value mask (strips extra flags) */
#define PURGED_FLAG			0x8000		/* Flag that tells me reference is purged */
#define PARTIAL_LOAD_FLAG	0x4000		/* Ref has been partially loaded */
#define NO_DATA_SAVE_FLAG	0x2000		/* Data doesn't need to be saved when purged */
#define PURGE_LOCK_FLAG		0x1000		/* Prevent from purging temporarily */

#define CLEAR_PURGED_FLAG	0x7FFF		/* Clears the above */

#define NO_PURGING_STATUS	0x00FF		/* Do not purge if purge = this */
#define PURGE_NO_CHANGE		0xFFFF		/* Do not change purge status */

#define PURGE_THRESHOLD		0x00010000	/* Extra amount purged for safety */
#define PURGE_WORTH_IT		0x00000064	/* Amount memory has to be to be worth purging */

/* "Checksum" bytes put at end of block */

#define CHECKSUM_BYTE1		0x12
#define CHECKSUM_BYTE2		0x34
#define CHECKSUM_BYTE3		0x56
#define CHECKSUM_BYTE4		0x78

#define CHECKSUM_SIG		0x7777

#ifdef PG_DEBUG
#define NON_APP_SIZE	sizeof(mem_rec) + CHECKSUM_SIZE
#else
#define NON_APP_SIZE	sizeof(mem_rec)
#endif

#ifdef MAC_PLATFORM

#define BEGIN_TIMER_DEBUG(msg) \
		{   unsigned long	debug_ticks = TickCount();  \
			char	 num[14];	\
			num[13] = msg;

#define END_TIMER_DEBUG \
			NumToString(TickCount() - debug_ticks, (StringPtr)num); \
			++num[0];	\
			num[num[0]] = num[13];  \
			DebugStr((StringPtr)num);	}

#endif

#define MEM_NULL	0L

typedef unsigned long memory_ref;	/* Basic memory reference */
typedef memory_ref PG_FAR *memory_ref_ptr;

typedef long PG_FAR *available_ptr;		/* Used for virtual memory map */
typedef pg_handle PG_FAR *master_list_ptr;

/* Purge function verbs:  */

enum {
	purge_init,				/* Initialize VM */
	purge_memory,			/* Purge the reference */
	unpurge_memory,			/* Unpurge the reference */
	dispose_purge			/* Purged ref will be disposed */
};


/* Debugger proc definitions */

typedef PG_FN_PASCAL (void, mem_debug_proc) (pg_error message, unsigned long what_ref);
typedef PG_FN_PASCAL (pg_error, purge_proc) (memory_ref ref_to_purge, pgm_globals_ptr mem_globals, short verb);
typedef PG_FN_PASCAL (long, free_memory_proc) (pgm_globals_ptr mem_globals, memory_ref dont_free, long desired_free);


/* Memory Manager Global Vars (app keeps these) */

struct pgm_globals {
	short					signature;			/* Used for checking/debugging */
	pg_short_t				debug_flags;		/* Debug mode, if any */
	pg_handle				master_handle;		/* HANDLE for master list (Windows only) */
	pg_handle				spare_tire;			/* Used to free up some memory in tight situations */
	master_list_ptr			master_list;		/* Contains list of all active memory_refs */
	long					next_master;		/* Next available space in master_list */
	long					total_unpurged;		/* Total # of bytes allocated not purged */
	long					max_memory;			/* Maximum memory (set by app) */
	long					purge_threshold;	/* Amount extra to purge */
	void PG_FAR				*machine_var;		/* Machine-specific generic ptr */
	mem_debug_proc			debug_proc;			/* Called when a bug is detected */
	purge_proc				purge;				/* Called to purge/unpurge memory */
	free_memory_proc		free_memory;		/* Called to free up miscellaneous memory */
	long					purge_ref_con;		/* Reference for purge proc */
	memory_ref				purge_info;			/* Machine-based purge information */
	memory_ref				freemem_info;		/* List of pg_ref(s) for cache feature (2.0) */
	long					next_mem_id;		/* Used for unique ID's assigned to refs */
	long					current_id;			/* ID to use for MemoryAlloc's */
	long					active_id;			/* Which ID to suppress, if any, for purging */
	long					last_message;		/* Last message in exception handling */
	pg_fail_info_ptr		top_fail_info;		/* Current exception in linked list */
	void PG_FAR *			last_ref;			/* Last reference - used by external failure processing TRS/OITC */
	pg_error_handler		last_handler;		/* Last app handler before Paige */
	pg_error				last_error;			/* Last reported error */
#ifdef PG_DEBUG
	memory_ref				debug_check;		/* Used for special-case debugging */
	memory_ref				dispose_check;		/* Used for special-case debugging on DisposeMemory */
	short					debug_access;		/* Used with above field */
#endif
	void PG_FAR				*app_globals;		/* Ptr to globals for PAIGE, etc. */
	long					creator;			/* For Mac file I/O */
	long					fileType;			/* For Mac file I/O */
};


#ifdef PG_DEBUG
#ifdef PG_DEBUG_STATICS
extern pgm_globals_ptr	debug_globals;
#endif
#endif


/* Memory ref's internal struct (hidden from app) which is placed in the block: */

struct mem_rec {
	short				rec_size;		/* Record size */
	short				extend_size;	/* Amount of extra slop to extend */
	long				num_recs;		/* Current number of records */
	long				real_num_recs;	/* Actual number of records */
	long				master_index;	/* Where it is stored in master_list */
	short				access;			/* Access counter */
	pg_short_t			purge;			/* Purge priority and status */
	long				mem_id;			/* Special ID assigned to memory_ref */
	unsigned long		qty_used;		/* Nbr. of times accessed */
	pgm_globals_ptr 	globals;		/* Globals refs */
};
typedef struct mem_rec mem_rec;
typedef mem_rec PG_FAR *mem_rec_ptr;


#ifdef __cplusplus
extern	"C" {
#endif


/* Main function definitions */

extern PG_PASCAL (void) pgMemStartup (pgm_globals_ptr mem_globals, long max_memory);
extern PG_PASCAL (void) pgMemShutdown (pgm_globals_ptr mem_globals);
extern PG_PASCAL (memory_ref) DisposeMemory (memory_ref ref);
extern PG_PASCAL (memory_ref) DisposeNonNilMemory (memory_ref ref);
extern PG_PASCAL (memory_ref) MemoryAllocID (pgm_globals_ptr globals,
		pg_short_t rec_size, long num_recs, short extend_size, long mem_id);
extern PG_PASCAL (memory_ref) MemoryAllocClearID (pgm_globals_ptr globals,
		pg_short_t rec_size, long num_recs, short extend_size, long mem_id);
extern PG_PASCAL (memory_ref) MemoryDuplicate (memory_ref src_ref);
extern PG_PASCAL (memory_ref) MemoryDuplicateID (memory_ref src_ref, long mem_id);
extern PG_PASCAL (void) MemoryCopy (memory_ref src_ref, memory_ref target_ref); 
extern PG_PASCAL (memory_ref) MemoryRecover (void PG_FAR *ptr); 
extern PG_PASCAL (void PG_FAR*) UseMemory (memory_ref ref);
extern PG_PASCAL (void PG_FAR*) UseMemoryRecord (memory_ref ref, long wanted_rec,
			long seq_recs_used, pg_boolean first_use);
extern PG_PASCAL (void) GetMemoryRecord (memory_ref ref, long wanted_rec,
			void PG_FAR *record);
extern PG_PASCAL (void PG_FAR*) UseForLongTime(memory_ref ref);
extern PG_PASCAL (void) SetMemorySize (memory_ref ref_to_size, long wanted_size);
extern PG_PASCAL (void) SetMemoryRecSize (memory_ref ref, pg_short_t new_rec_size, short extend_size);
extern PG_PASCAL (void) DisposeAllMemory (pgm_globals_ptr mem_globals, long memory_id);
extern PG_PASCAL (void) PurgeAllMemory (pgm_globals_ptr mem_globals, long memory_id);
extern PG_PASCAL (void) UnuseAllMemory (pgm_globals_ptr mem_globals, long memory_id);
extern PG_PASCAL (long) GetAllMemorySize (pgm_globals_ptr mem_globals, long memory_id,
			pg_boolean return_byte_size);
extern PG_PASCAL (void PG_FAR*) InsertMemory (memory_ref ref, long offset, long insert_size);
extern PG_PASCAL (void PG_FAR*) AppendMemory (memory_ref ref, long append_size, pg_boolean zero_fill);
extern PG_PASCAL (void) DeleteMemory (memory_ref ref, long offset, long delete_size);
extern PG_PASCAL (void) SetMemoryPurge (memory_ref ref, short purge_priority,
		pg_boolean no_data_save);
extern PG_PASCAL (memory_ref) InitMemoryRef (pgm_globals_ptr mem_globals, pg_handle base_ref);
extern PG_PASCAL (memory_ref) ForceMinimumMemorySize (memory_ref ref_to_size);
extern PG_PASCAL (pg_error) MemoryPurge (pgm_globals_ptr globals, long minimum_amount,
		memory_ref mask_ref);
extern PG_PASCAL (void) InitVirtualMemory (pgm_globals_ptr globals, purge_proc purge_function,
		long ref_con);
extern PG_PASCAL (void) UnuseMemory (memory_ref ref);
extern PG_PASCAL (long) GetMemorySize (memory_ref ref);
extern PG_PASCAL (long) GetByteSize (memory_ref ref);
extern PG_PASCAL (short) GetMemoryRecSize (memory_ref ref);
extern PG_PASCAL (pgm_globals_ptr) GetGlobalsFromRef (memory_ref ref);
extern PG_PASCAL (void) UnuseAndDispose (memory_ref ref);
extern PG_PASCAL (long) GetMemoryRefID(memory_ref ref);
extern PG_PASCAL (void) SetMemoryRefID(memory_ref ref, long mem_id);
extern PG_PASCAL (short) GetAccessCtr (memory_ref ref);
extern PG_PASCAL (long) pgUniqueMemoryID (pgm_globals_ptr globals);
extern PG_PASCAL (long) GetMemoryRefID(memory_ref ref);
extern PG_PASCAL (void) ChangeAllMemoryID(pgm_globals_ptr mem_globals, long orig_id, long new_id);
extern PG_PASCAL (long) pgGetByteSize (pg_short_t rec_size, long num_recs);
extern PG_PASCAL (memory_ref) pgAllocateNewRef (pgm_globals_ptr mem_globals, pg_short_t rec_size,
		long num_recs, short extend_size, short zero_fill);
extern PG_PASCAL (void) DisposeFailedMemory (memory_ref ref);
extern PG_PASCAL (void) UnuseAllFailedMemory (pgm_globals_ptr mem_globals, long memory_id);
extern PG_PASCAL (void) UnuseFailedMemory (memory_ref ref);
extern PG_PASCAL (void) DisposeAllFailedMemory (pgm_globals_ptr mem_globals, long memory_id);
extern PG_PASCAL (pg_handle) DetachMemory (memory_ref ref);
extern PG_PASCAL (void) pgMemInsert (memory_ref ref, long offset, long byte_size, long block_size);
extern PG_PASCAL (void) pgMemDelete (memory_ref ref, long offset, long byte_size, long block_size);
extern PG_PASCAL (void) pgBlockMove (const void PG_FAR *src, void PG_FAR *dest, long block_size);
extern PG_PASCAL (short) pgCheckReference (memory_ref ref);
extern PG_PASCAL (pg_error) pgStandardPurgeProc (memory_ref ref_to_purge, pgm_globals_ptr mem_globals, short verb);
extern PG_PASCAL (long) pgStandardFreeProc (pgm_globals_ptr mem_globals, memory_ref dont_free ,long desired_free);
extern PG_PASCAL (pg_short_t) GetMemoryPurge (memory_ref ref);

#ifdef C_LIBRARY
#define pgFillBlock(block,block_size,value)	memset((block),(value),(size_t)(block_size))
#else
extern PG_PASCAL (void) pgFillBlock (void PG_FAR *block, long block_size, char value);
#endif

/* The following are available only in debug mode */

#ifdef PG_DEBUG

extern PG_PASCAL (short) pgCheckReference (memory_ref ref);

#ifdef PG_DEBUG_STATICS
extern PG_PASCAL (long) TotalPaigeMemory (void);
#endif
extern PG_PASCAL (pg_boolean) pgCheckAllMemoryRefs (void);
#endif

#ifdef NO_C_INLINE
extern PG_PASCAL (memory_ref) MemoryAlloc (pgm_globals_ptr globals, pg_short_t rec_size,
		long num_recs, short extend_size);
extern PG_PASCAL (memory_ref) MemoryAllocClear (pgm_globals_ptr globals, pg_short_t rec_size,
		long num_recs, short extend_size);
#else
#define MemoryAlloc(globals,rec_size,num_recs,extend_size)  pgAllocateNewRef((globals),rec_size,num_recs,extend_size,FALSE)
#define MemoryAllocClear(globals,rec_size,num_recs,extend_size)  pgAllocateNewRef((globals),rec_size,num_recs,extend_size,TRUE)
#endif


/* TRS/OITC additions */

extern PG_PASCAL (long) MemoryToCStr(memory_ref ref);
extern PG_PASCAL (long) MemoryToPStr(memory_ref ref);
extern PG_PASCAL (long) CStrToMemory(memory_ref ref);
extern PG_PASCAL (long) PStrToMemory(memory_ref ref);
extern PG_PASCAL (char PG_FAR *) UseMemoryToCStr(memory_ref ref);
extern PG_PASCAL (pg_char_ptr) UseMemoryToPStr(memory_ref ref);
extern PG_PASCAL (void) UnuseMemoryFromCStr(memory_ref ref);
extern PG_PASCAL (void) UnuseMemoryFromPStr(memory_ref ref);
extern PG_PASCAL (void) UnuseAndZero(memory_ref ref);
extern PG_PASCAL (memory_ref) DisposeNonNilFailedMemory (memory_ref ref);
extern PG_PASCAL (memory_ref) DuplicateNonNilMemory (memory_ref ref);
extern PG_PASCAL (pg_boolean) EqualMemory (memory_ref ref1, memory_ref ref2);
extern PG_PASCAL (memory_ref) Bytes2Memory (pgm_globals_ptr mem_globals, const pg_bits8_ptr data, short char_size, long length);
#ifdef NO_C_INLINE
extern PG_PASCAL (memory_ref) PString2Memory (pgm_globals_ptr mem_globals, const pg_p_string_ptr string);
extern PG_PASCAL (memory_ref) CString2Memory (pgm_globals_ptr mem_globals, const pg_c_string_ptr string);
#else
#define PString2Memory(mem_globals,string) Bytes2Memory (mem_globals, (pg_bits8_ptr)string, sizeof(pg_char), string[0] + 1)
#define CString2Memory(mem_globals,string) Bytes2Memory (mem_globals, (pg_bits8_ptr)string, sizeof(pg_char), pgCStrLength(string) + 1)
#endif


/* pgSourceDebugBreak is used for PAIGE source code developers. Its purpose is to break into
the high-level debugger of the development system. When MemMgr detects an error, it jumps
to this function if you have defined SOURCE_LEVEL_DEBUG in CPUDEFS.H. */

#ifdef SOURCE_LEVEL_DEBUG
extern char pgSourceDebugBreak (memory_ref offending_ref, char *debug_str);
#else
#define pgSourceDebugBreak(offending_ref,debug_str)	
#endif


#ifdef __cplusplus
	}
#endif


#endif
