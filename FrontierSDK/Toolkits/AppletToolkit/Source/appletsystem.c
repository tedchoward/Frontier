
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletmemory.h"
#include "appletstrings.h"
#include "appletsystem.h"


boolean getsystemname (bigstring systemname) {
	
	StringHandle h;
	
	h = GetString (-16413);
	
	if (ResError () != noErr)
		return (false);
		
	copyheapstring (h, systemname);
	
	if (stringlength (systemname) == 0)
		copystring ("\pUntitled System", systemname);
	
	return (true);
	} /*getsystemname*/
