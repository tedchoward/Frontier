/*
 *	MacHeaders.h
 *
 *	Processor independant interface to the MacHeaders<xxx> files ...
 */

#ifdef __cplusplus

#if __POWERPC__
	#include "shellheadersPPC++"
#elif __CFM68K__
	#include "shellheadersCFM68K++"
#else
	#include "shellheaders68K++"
#endif

#else

#if __POWERPC__
	#include "shellheadersPPC"
#elif __CFM68K__
	#include "shellheadersCFM68K"
#else
	#include "shellheaders68K"
#endif

#endif
