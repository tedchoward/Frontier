
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define paletteinclude


#define maxpaletteitems 50


typedef struct typaletteitemrecord {
	
	boolean selected: 1; /*is the item selected or not?*/
	
	boolean breakafter: 1; /*start a new sub-palette after this one*/
	} typaletteitemrecord;
	
	
typedef boolean (*typalettecallback) (void);


typedef struct typaletterecord { /*one of these in each app window record*/
	
	WindowPtr macwindow;
	
	Rect r;
	
	short sicnresource;
	
	short cticons;
	
	short palettewidth;
	
	typalettecallback itemhitcallback;

	short itemselected;
	
	typaletteitemrecord item [maxpaletteitems];
	} typaletterecord, **hdlpaletterecord;
	
	
hdlpaletterecord newpalette (WindowPtr, short);

void disposepalette (hdlpaletterecord);

void invalpalette (hdlpaletterecord);

void invalpaletteitem (hdlpaletterecord, short);

void palettemousedown (hdlpaletterecord);

void paletteupdate (hdlpaletterecord);

void paletteactivate (hdlpaletterecord, boolean);

void paletteselectitem (hdlpaletterecord, short);

void getpalettesize (hdlpaletterecord, short *, short *);
