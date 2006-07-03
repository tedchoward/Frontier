
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef __APPLEEVENTS__

	#include <AppleEvents.h>

#endif

#ifndef __COMPONENTS__

	#include <Components.h> /*3.0*/ 

#endif

#ifndef __MENUS__

	#include <Menus.h> /*3.0*/
	
#endif


#define __MENUSHARING__ /*so other modules can tell that we've been included*/

//	RMS 960614
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

typedef struct tysharedmenurecord { /*must match scripting system record structure*/
	
	short idmenu; /*the resource id of the menu*/
	
	short flhierarchic: 1; /*if true it's a hiearchic menu*/
	
	short flinserted: 1; /*if true the menu has been inserted in the menu bar*/
	
	MenuHandle hmenu; /*a handle to the Mac Menu Manager's data structure*/
	} tysharedmenurecord;


typedef tysharedmenurecord tymenuarray [1];

typedef tymenuarray **hdlmenuarray;

typedef pascal void (*tyMSerrordialog) (Str255);

typedef pascal void (*tyMSeventfilter) (EventRecord *);

typedef pascal void (*tyMSmenusinstaller) (hdlmenuarray);



typedef struct tyMSglobals { /*Menu Sharing globals, all in one struct*/

	OSType serverid; /*identifier for shared menu server*/

	OSType clientid; /*id of this application*/
	
	hdlmenuarray hsharedmenus; /*data structure that holds shared menus*/
	
	Boolean fldirtysharedmenus; /*if true, menus are reloaded next time app comes to front*/
	
	Boolean flscriptcancelled; /*set true by calling CancelSharedScript*/
	
	Boolean flscriptrunning; /*true if a script is currently running*/
	
	Boolean flinitialized; /*true if InitSharedMenus was successful*/
	
	long idscript; /*the server's id for the currently running script, makes it easy to kill it*/
	
	ComponentInstance menuserver; /*3.0*/ 
	
	long serverversion; /*4.1*/
	
	tyMSerrordialog scripterrorcallback; /*3.0*/
	
	tyMSeventfilter eventfiltercallback; /*3.0*/
	
	tyMSmenusinstaller menusinsertercallback; /*4.1*/
	
	tyMSmenusinstaller menusremovercallback; /*4.1*/
	} tyMSglobals;


extern tyMSglobals MSglobals; /*menu sharing globals*/


/*basic Menu Sharing routines*/

	pascal Boolean InitSharedMenus (tyMSerrordialog, tyMSeventfilter);

	pascal Boolean SharedMenuHit (short, short);
	
	pascal Boolean SharedScriptRunning (void);
	
	pascal Boolean CancelSharedScript (void);
	
	pascal Boolean CheckSharedMenus (short);
	
	pascal Boolean SharedScriptCancelled (AppleEvent *, AppleEvent *);
	

/*special-purpose routines*/

	pascal Boolean DisposeSharedMenus (void);

	pascal Boolean IsSharedMenu (short);
	
	pascal Boolean EnableSharedMenus (Boolean);
	
	pascal Boolean RunSharedMenuItem (short, short);

	pascal Boolean SetMenusInserterCallback (tyMSmenusinstaller);
	
	pascal Boolean SetMenusRemoverCallback (tyMSmenusinstaller);
	
//	RMS 960614	
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif
