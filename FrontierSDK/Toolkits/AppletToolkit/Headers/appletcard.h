
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define appletcardinclude


#ifndef iowaruntimeinclude

	#include <iowaruntime.h>

#endif

extern boolean fldialogsenabled;

extern bigstring bsalertstring;


boolean twowaydialog (bigstring, bigstring, bigstring);

short threewaydialog (bigstring, bigstring, bigstring, bigstring);

boolean alertdialog (bigstring);

boolean infodialog (bigstring);

boolean confirmdialog (bigstring);

boolean yesnodialog (bigstring bs);

boolean askdialog (bigstring, bigstring);

boolean passworddialog (bigstring, bigstring);

boolean replaceexistingdialog (bigstring);
