
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define mouseinclude /*so other modules can tell if we've been included*/


typedef struct tymouserecord {
	
	boolean fldoubleclick;
	
	Point localpt;
	
	long mouseuptime; 
	
	long mousedowntime; 
	
	Point mouseuppoint;
	
	Point mousedownpoint;
	
	boolean fldoubleclickdisabled;
	} tymouserecord;
	
	
extern tymouserecord mousestatus;

typedef void (*tymousetrackcallback) (boolean);

boolean mousetrack (Rect, tymousetrackcallback);

boolean mousedoubleclick (Point);

boolean mousecheckautoscroll (Point, Rect, boolean, tydirection *);
