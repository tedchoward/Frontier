
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define kbinclude /*so other includes can tell if we've been loaded*/


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif




typedef struct tykeystrokerecord {

	char chkb;
	
	byte flshiftkey, flcmdkey, floptionkey, flalphalock, flcontrolkey;
		
	short ctmodifiers; /*the number of booleans that are on*/

	short keycode; /*see Toolbox Event Manager -- this is the hardware key code*/
	
	byte flkeypad: 1; /*if true, keystroke comes from numeric keypad*/
	
	byte flautokey: 1; /*if true, keystroke is an automatic key*/
	
	tydirection keydirection; 
	} tykeystrokerecord, *ptrkeystrokerecord, **hdlkeystrokerecord;


extern tykeystrokerecord keyboardstatus;


tydirection keystroketodirection (char ch); 

boolean arrowkey (char);

tydirection keystroketodirection (char);

void setkeyboardstatus (long, long, long);

void keyboardclearescape (void);

void keyboardsetescape (void);

boolean keyboardescape (void);

void keyboardpeek (tykeystrokerecord *);

boolean enterkeydown (void);

boolean optionkeydown (void);

boolean cmdkeydown (void);

boolean shiftkeydown (void);

boolean returnkeydown (void);


