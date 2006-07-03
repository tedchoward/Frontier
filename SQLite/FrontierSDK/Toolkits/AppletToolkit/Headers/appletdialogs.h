
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define appletdialogsinclude


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif


extern boolean fldialogsenabled;

extern bigstring bsalertstring;


DialogPtr newmodaldialog (short, short);

void boldenbutton (DialogPtr, short);

short runmodaldialog (void);

void parsedialogstring (bigstring);

void setdialogtext (DialogPtr, short, bigstring);

void getdialogtext (DialogPtr, short, bigstring);

void selectdialogtext (DialogPtr, short);

void disabledialogitem (DialogPtr, short);

void enabledialogitem (DialogPtr, short);

void dialoggetobjectrect (DialogPtr, short, Rect *);

boolean alertdialog (bigstring);

boolean confirmdialog (bigstring);

boolean askdialog (bigstring, bigstring);

boolean yesnodialog (bigstring);

short threewaydialog (bigstring, bigstring, bigstring, bigstring);

short savedialog (bigstring);

boolean oserror (OSErr);

