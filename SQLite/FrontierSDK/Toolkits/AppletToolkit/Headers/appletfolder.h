
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define appletfolderinclude


#ifndef filesinclude

	#include "appletfiles.h"
	
#endif


#ifndef __FOLDERS__

	#include <Folders.h>
	
#endif


typedef boolean (*tyfileloopcallback) (bigstring, tyfileinfo *, long);


boolean getfolderfilespec (short, long, FSSpec *);

boolean filegetparentfolderspec (FSSpec *, FSSpec *);

boolean getsystemfolderfilespec (OSType, FSSpec *);

boolean diskloop (tyfileloopcallback, long);

boolean folderloop (ptrfilespec, boolean, tyfileloopcallback, long);

boolean foldercontainsalias (ptrfilespec, ptrfilespec, ptrfilespec);

boolean systemfoldercontainsalias (OSType, FSSpec *);

boolean filecreatefolder (ptrfilespec);

boolean filegetfolderid (ptrfilespec, long *);

boolean addaliastospecialfolder (FSSpec *, OSType);

boolean deletealiasesfromfolder (FSSpec *, FSSpec *);

boolean deletealiasesfromspecialfolder (FSSpec *, OSType);

boolean filegetsubitemspec (FSSpec *, bigstring, FSSpec *);

boolean filesetfolderview (FSSpec *, short);

boolean fileemptyfolder (FSSpec *);

boolean foldercontainsfile (FSSpec *, FSSpec *);

boolean fileflushvolumes (void);

boolean filesurefolder (ptrfilespec);

boolean filesuresubfolder (ptrfilespec, bigstring, ptrfilespec);
