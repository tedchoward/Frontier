
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define appletfilesinternalinclude


extern boolean flerrordialogdisabled;

void setfileerrorfile (ptrfilespec);

boolean fileerror (ptrfilespec, OSErr);

boolean getmacfileinfo (ptrfilespec, CInfoPBRec *);

boolean setmacfileinfo (ptrfilespec, CInfoPBRec *);

boolean isvolumefilespec (ptrfilespec);

void filegetinfofrompb (CInfoPBRec *, tyfileinfo *);

