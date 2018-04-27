
/*	$Id$    */

/*
 *	MacHeaders.h
 *
 *	Processor independant interface to the MacHeaders<xxx> files ...
 */

#ifdef __cplusplus

#if   __CFM68K__
	#include "shellheadersCFM68K++"
#else
	#include "shellheaders68K++"
#endif

#else

#if   __CFM68K__
	#include "shellheadersCFM68K"
#else
	#include "shellheaders68K"
#endif

#endif
