
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define cursorinclude /*so other includes can tell if we've been loaded*/



typedef enum tycursortype {

	cursorisdirty = -1,
	
	cursorisarrow = -2,
	
	cursorisibeam = iBeamCursor,
	
	cursoriswatch = watchCursor,
	
	cursorishorizontalrail = 256,
	
	cursorisbeachball1 = 257, 
	
	cursorisbeachball2 = 258, 
	
	cursorisbeachball3 = 259, 
	
	cursorisbeachball4 = 260,
	
	cursorispopup = 261,
	
	cursorisearth1 = 262, 
	
	cursorisearth2 = 263, 
	
	cursorisearth3 = 264, 
	
	cursorisearth4 = 265, 
	
	cursorisearth5 = 266,
	
	cursorisearth6 = 267, 
	
	cursorisearth7 = 268,
	
	cursorishollowarrow = 269,
	
	cursorfordraggingmove = 270,
	
	cursorissmallquestionmark = 271,
	
	cursorisno = 272,
	
	cursorisverticalrails = 130,
	
	cursorishorizontalrails = 131,
	
	cursorisslantedrails = 132,
	
	cursorisotherslantedrails = 133,
	
	cursorisbuttonhand = 273,
	
	cursorisgo = 274,
	
	cursorisrightwedge = 275
	} tycursortype;

	
void setcursortype (tycursortype);

void obscurecursor (void);
	
void initearth (void);

void rollearth (void);

void initbeachball (void);

void rollbeachball (void);

void watchcursor (void);

void arrowcursor (void);

