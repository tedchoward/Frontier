
/*	$Id$    */

/* This PAIGE file is separate to maintain compatibility with MS Foundation Class
development for Windows (when users use pure MFC their compiler complains about
<setjmp> code and it is not supported!  Copyright ©1994    */

/* July 19, 1994 Changed to add support calls to aid in interfacing Paige and Paige's
	failure mechanisms with C++ and other languages failure mechanisms by Tom Shaw, OITC 
	Also, restructured for inlines. */

/*
	TRS/OITC Change summary:
		last_ref 			A void* to pgm_globals to hold object's reference (actually can be used 
							for anything). This was added as Paige uses last_message to pass various 
							messages to support debug
		pgGetReference		Macro or procedure (depending on compilation option) to access the last_ref
		PG_GET_REFERENCE	Macro to access the last_ref in a PG_TRY-PG_CATCH-PG_ENDTRY sequence
		pgSetReference		Macro or procedure (depending on compilation option) to set the last_ref
		PG_SET_REFERENCE	Macro to set the last_ref in a PG_TRY-PG_CATCH-PG_ENDTRY sequence
	Added for ease of use:
		pgReRaise			Function to pass a failure up the failure chain
		PG_RERAISE			Macro to pass a failure up the chain in a PG_TRY-PG_CATCH-PG_ENDTRY sequence
	Changed:
		pgSetFailInfo		To optionally set last_ref
		pg_error_handler	Cut down on parameter passing as all info is in pgm_globals
		pgGetError			Made a macro or procedure depending on compilation option for linkages
		pgGetMessage		Made a macro or procedure depending on compilation option for linkages
*/

#ifndef PGEXCEPS_H
#define PGEXCEPS_H

#include "CPUDEFS.H"
#include "PGSETJMP.H"

#include <setjmp.h>
#ifdef _INC_SETJMP
#ifdef WIN16_COMPILE
#ifdef __cplusplus
#include "win16jmp.h"
#endif
#endif
#endif

struct pg_fail_info
{
	pg_fail_info_ptr	next;			/* Pointer to next handler	*/
	pg_error_handler	handler;		/* Optional error handler proc */
	jmp_buf				regs;			/* This is machine and environment specific but C takes care of it */
};


#define PG_TRY(pg_globals_p)								\
		{ pg_fail_info __fi;								\
		  pgm_globals_ptr __pg_globals_p = pg_globals_p;	\
		  pgPushHandler( __pg_globals_p, &__fi);			\
		  if (!setjmp( __fi.regs)) {

#define PG_CATCH											\
			pgSuccess(__pg_globals_p); }					\
		else {
		
#define PG_ENDTRY											\
		}	}


/* Some useful macros */

/* These following macros can only be used within a TRY-CATCH-ENDTRY sequence */
		
#define PG_GET_ERROR()										\
		pgGetError( __pg_globals_p)
		
#define PG_GET_MESSAGE()									\
		pgGetMessage( __pg_globals_p)

#define PG_GET_REFERENCE()									\
		pgGetReference( __pg_globals_p)

#define PG_SET_REFERENCE(ref)								\
		pgSetReference( __pg_globals_p, ref)

#define PG_FAIL_NIL(p)										\
		pgFailNIL( __pg_globals_p, (void PG_FAR *)(p))
		
#define PG_FAIL_BOOLEAN(b)									\
		pgFailBoolean( __pg_globals_p, (pg_boolean)(b))
		
#define PG_FAIL_ERROR(err)									\
		pgFailError( __pg_globals_p, (pg_error)(err))
		
#define PG_FAIL_NOT_ERROR(acceptable_error,actual_error)	\
		pgFailNotError( __pg_globals_p, (pg_error)(acceptable_error), (pg_error)(actual_error))
		
#define PG_FAILURE(err,message)								\
		pgFailure( __pg_globals_p, (pg_error)(err), (long)(message))
		
#define PG_SUCCESS()										\
		pgSuccess( __pg_globals_p)
		
#define PG_SET_FAIL_INFO(err,message,ref)					\
		pgSetFailInfo( __pg_globals_p, (pg_error)(err), (long)(message), (void *)(ref))
		
#define PG_GET_MEM_GLOBALS()								\
		(__pg_globals_p)

#define PG_RERAISE()										\
		pgReRaise(__pg_globals_p)


#endif
