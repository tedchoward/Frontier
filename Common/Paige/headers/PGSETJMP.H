
/*	$Id$    */

/* Paige Exceptions Handler header.  Original source created by Tom Shaw, OITC.
Modified by Gar for DataPak Software, Inc. to be more consistent with Paige
syntax and parameter naming conventions.   Copyright 1993-1994    */

/* July 19, 1994 Changed to add support calls to aid in interfacing Paige and Paige's
	failure mechanisms with C++ and other languages failure mechanisms by Tom Shaw, OITC 
	Also, restructured for inlines. */

/*
	Change summary:
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


/* Dec 22, 1994 Changed to fix some compiler optimization problems - TR Shaw, OITC */
 
#ifndef PGSETJMP_H
#define PGSETJMP_H

#include "CPUDefs.h"
#include "pgErrors.h"

/* PG_VOLATILE - Any variable or parameter that is modified in a PG_TRY block and 
	used in the PG_CATCH block should be declared as volatile or it may have an incorrect 
	value in the PG_CATCH block. Since not all compilers support the 'volatile' keyword, 
	you can define PG_VOLATILE and have an alternate definition used which will confound
	the compiler's optimizer like volatile (eg PGVolatile).
*/

//define PG_VOLATILE
#define PGVolatile(x)	((void) &x)

#define BOOLEAN_EXCEPTION	0x1000

/* Forward declaration of mem mgr globals required:  */

typedef struct pgm_globals pgm_globals;
typedef pgm_globals PG_FAR *pgm_globals_ptr;

typedef struct pg_fail_info pg_fail_info;
typedef pg_fail_info PG_FAR *pg_fail_info_ptr;

/* Exception handler function */

typedef PG_FN_PASCAL (void, pg_error_handler) (pgm_globals_ptr globals);

#ifdef __cplusplus
extern "C" {
#endif

extern PG_PASCAL (void) pgPushHandler (pgm_globals_ptr globals, pg_fail_info_ptr fail_info);
extern PG_PASCAL (void) pgFailure (pgm_globals_ptr globals, pg_error error, long message);
extern PG_PASCAL (void) pgSuccess (pgm_globals_ptr globals);
extern PG_PASCAL (void) pgSetFailInfo( pgm_globals_ptr globals, pg_error new_error, long new_msg, void PG_FAR *new_ref);
extern PG_PASCAL (pg_error) pgProcessError (pg_error error);
extern PG_PASCAL (void) pgDebugProc (pg_error message, unsigned long what_ref);
extern PG_PASCAL (void) pgFailNotError (pgm_globals_ptr globals, pg_error acceptable_error,
		pg_error actual_error);


#ifdef NO_C_INLINE
extern PG_PASCAL (pg_error) pgGetError( pgm_globals_ptr globals );
extern PG_PASCAL (long) pgGetMessage( pgm_globals_ptr globals );
extern PG_PASCAL (void PG_FAR *) pgGetReference( pgm_globals_ptr globals );
extern PG_PASCAL (void) pgSetReference( pgm_globals_ptr globals, void PG_FAR * last_ref);
extern PG_PASCAL (void) pgReRaise (pgm_globals_ptr globals);
extern PG_PASCAL (void) pgFailNIL (pgm_globals_ptr globals, void PG_FAR *allocation);
extern PG_PASCAL (void) pgFailZero (pgm_globals_ptr globals, generic_var value);
extern PG_PASCAL (void) pgFailBoolean( pgm_globals_ptr pgm_globals_p, pg_boolean b);
extern PG_PASCAL (void) pgFailError (pgm_globals_ptr globals, pg_error error);
extern PG_PASCAL (void) pgSetFailureHandler( pgm_globals_ptr globals, pg_error_handler handler);
#else
#define pgGetError(globals) (globals->last_error)
#define pgGetMessage(globals) (globals->last_message)
#define pgGetReference(globals) (globals->last_ref)
#define pgSetReference(globals,ref) (globals->last_ref = ref)
#define pgReRaise(globals)						\
		pgFailure(globals, pgGetError( globals ), pgGetMessage( globals ))
#define pgFailNIL(globals,allocation)			\
		if (!(allocation)) pgFailure(globals, NO_MEMORY_ERR, 0)		
#define pgFailZero(globals,allocation)			\
		if (!(allocation)) pgFailure(globals, NO_MEMORY_ERR, 0)		
#define pgFailBoolean(globals,b)				\
		if ((b)) pgFailure( globals, BOOLEAN_EXCEPTION, 0)
#define pgFailError(globals,error)			\
		if (error != NO_ERROR) pgFailure( globals, error, 0)
#define pgSetFailureHandler(globals,handler)	\
		globals->last_handler = handler
#endif

#ifdef __cplusplus
}
#endif

#endif
