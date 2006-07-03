
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define frontierinclude


#define editTableVerb 	'edtt' /*commands that can be sent with an embedded table*/
#define startupVerb		'init'
#define deleteVerb		'dele'


typedef struct tyembeddedinfo { /*allows appletfrontier.c to not use the applet toolkit*/
	
	FSSpec filespec;
	
	short fnum;
	
	Handle h;
	
	OSType creator;
	
	bigstring windowtitle;
	} tyembeddedinfo;
	

extern tyembeddedinfo embeddedinfo; /*set this up before calling one of the embedded routines*/


typedef boolean (*tyalertcallback) (bigstring);


boolean FrontierDoScript (bigstring, bigstring);

boolean FrontierDoHandleScript (Handle, boolean, boolean, bigstring, Handle *);

boolean FrontierFastDoScript (bigstring, boolean, bigstring, bigstring);

boolean FrontierOpenObject (bigstring);

boolean FrontierIsRunning (void);

boolean getFrontierVersion (short *, short *, short *, boolean *);

boolean tableVerb (OSType, tyalertcallback, boolean);

boolean getEmbeddedTable (tyalertcallback);

boolean renameEmbeddedTable (bigstring, bigstring, tyalertcallback);

boolean runEmbeddedScript (Handle, Handle *);

boolean FrontierGetObject (Handle, bigstring, Handle *, OSType *);

boolean editFrontierObject (bigstring);
