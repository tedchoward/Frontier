
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define undoinclude


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif



#define undolistnumber 134 /*operations that can be undone, in string format*/
#define cantundoitem 1
#define undostring 2
#define redostring 3



typedef struct tystack {

	short topstack;

	short basesize;

	short elemsize;

	byte stack [];
	} tystack, *ptrstack, **hdlstack;


typedef boolean (*undocallback) (Handle, boolean);


typedef struct tyundorecord {
	
	undocallback undoroutine;
	
	Handle hundodata;
	
	boolean flactionstep; /*is this step an action record?*/
	} tyundorecord;


typedef struct tyactionrecord {
	
	long ixaction;
	
	long globaldata;
	
	boolean flaction; /*always true for actionrecords*/
	} tyactionrecord;


typedef struct tyundostack {

	short topundo;
	
	short basesize;
	
	short elemsize;
	
	short ixaction;
	
	long globaldata;
	
	tyundorecord undostep [];
	} tyundostack, *ptrundostack, **hdlundostack;


extern hdlundostack undostack;

extern hdlundostack redostack;


extern boolean pushundostep (undocallback, Handle);

extern boolean pushundoaction (short);

extern boolean popundoaction (void);

extern boolean undolastaction (boolean);

extern boolean redolastaction (boolean);

extern boolean getundoaction (short *);

extern boolean getredoaction (short *);

extern boolean getundomenuitem (bigstring, boolean *);

extern void killundo (void);

extern boolean newundostack (hdlundostack *);

extern boolean disposeundostack (hdlundostack);



