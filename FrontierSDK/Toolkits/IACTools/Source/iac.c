
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"


/*
8/24/92 DW: IAC Tools has been factored and optimized for the third time. At this point
there is no code that all different code types include.

The net effect -- if you use THINK C, you can safely include all the iacxxx.c files in
your project, and with the Smart Link feature turned on, you'll get a reasonably small
size link.

We stopped short of going all the way, putting each routine in its own file. We felt that
that would be too unwieldy, and would get back very small amounts of memory.
*/


tyIACglobals IACglobals;




