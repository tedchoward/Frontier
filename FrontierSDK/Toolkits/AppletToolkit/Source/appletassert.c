
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifdef fldebug
	
	#include "applet.h"
	#include "appletassert.h"

	static void convertcstring (bigstring bs) {
		
		/*
		convert a c string to a pascal string.
		*/
		
		ptrbyte p;
		short len = 0;
		
		p = &bs [0]; /*point at the first byte in c string*/
		
		while (*p++) len++;
		
		moveright (&bs [0], &bs [1], (long) len);
		
		bs [0] = (byte) len;
		} /*convertcstring*/
		
	
	short __assert (char *expr, char *file, short line) {
		
		bigstring bsexpr, bsfile, bsline, bsmessage;
		static boolean flnorentry = false;
		
		if (flnorentry)
			return (0);
		
		flnorentry = true;
		
		moveleft (expr, bsexpr, (long) lenbigstring);
		
		moveleft (file, bsfile, (long) lenbigstring);
		
		convertcstring (bsexpr);
		
		convertcstring (bsfile);
		
		NumToString ((long) line, bsline);
		
		copystring ("\pAssertion failed in file ", bsmessage);
		
		pushstring (bsfile, bsmessage);
		
		pushstring ("\p, at line ", bsmessage);
		
		pushstring (bsline, bsmessage);
		
		pushstring ("\p. Expression is “", bsmessage);
		
		pushstring (bsexpr, bsmessage);
		
		pushstring ("\p”", bsmessage);
		
		DebugStr (bsmessage);
		
		flnorentry = false;
		
		return (0);
	} /*__assert*/

#endif