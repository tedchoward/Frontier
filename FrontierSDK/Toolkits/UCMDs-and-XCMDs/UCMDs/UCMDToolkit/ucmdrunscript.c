
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


/*
if you want to run a script from within a UCMD, add this file to your
project.
*/

#include "ucmd.h"


pascal Boolean runscript (Handle hscript, Handle *hreturns) {

	tyIACglobals saveIACglobals;
	AppleEvent event, reply;
	Boolean flhavereply = false;
	Boolean fl = false;
	Str255 errorstring;
	
	*hreturns = nil;
	
	if (!IACnewsystemverb ('misc', 'dosc', &event))	// LDR - changed 'fast' -> 'misc' based on DB suggestion
		return (false);
	
	// LDR - copy for backup,
	//	otherwise you blow away future use of the original event, especially replies
	saveIACglobals = IACglobals;
	
	IACglobals.event = &event;
	
	if (!IACpushtextparam (hscript, '----'))
		goto exit;
		
	if (!IACsendverb (&event, &reply))
		goto exit;
	
	flhavereply = true;
	
	IACglobals.reply = &reply;
	
	if (IACiserrorreply (errorstring)) {
		
		Handle h;
		long len = errorstring [0];
		
		h = NewHandle (len);
		
		if (h == nil)
			goto exit;
		
		BlockMoveData (&errorstring [1], *h, len);	// LDR - changed to blockmovedata for minor speed improvement
		
		*hreturns = h;
		
		goto exit;
		}
		
	IACglobals.event = &reply; /*get the string from the reply record*/
	
	fl = IACgettextparam ('----', hreturns);
	
	// LDR - commented these two lines out since you they cause a memory leak & are unneccesary
	//		since the AEM copies data into a newhandle already, doing ANOTHER HandToHand leaves
	//		a block lying around...
	// if (fl)
	//	fl = HandToHand (hreturns) == noErr;
	
	exit:
	
	AEDisposeDesc (&event);	
	
	if (flhavereply)
		AEDisposeDesc (&reply);
	
	IACglobals = saveIACglobals;

	return (fl);
	} /*runscript*/
	
	



