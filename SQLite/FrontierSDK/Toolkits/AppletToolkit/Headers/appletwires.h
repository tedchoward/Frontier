
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define appletwiresinclude

extern bigstring bstargetwindowname; /*title of window that IAC verbs apply to*/



boolean initappletwires (void);

pascal short appwaitroutine (EventRecord *);

boolean settargetglobals (void);

boolean appruncard (Handle);

