
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"
#include "appletfilesinternal.h"
#include "appletfileunique.h"




void filepushsuffixnumber (short suffixnum, bigstring name) {
	
	if (suffixnum > 0) {
	
		pushstring ("\p #", name);
	
		pushint (suffixnum, name);
		}
	} /*filepushsuffixnumber*/
		

boolean filegetuniquename (FSSpec *fs, short *suffixnum) {
	
	bigstring origname, name;
	
	*suffixnum = 0;
	
	copystring ((*fs).name, origname);
	
	if (stringlength (origname) == 0)
		return (false);
		
	while (fileexists (fs)) {
	
		copystring (origname, name);
		
		(*suffixnum)++;
		
		filepushsuffixnumber (*suffixnum, name);
		
		if (!filemakespec ((*fs).vRefNum, (*fs).parID, name, fs))
			return (false);
		} /*while*/
	
	return (true);
	} /*filegetuniquename*/
	
	
