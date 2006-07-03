
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define ioaiconinclude





typedef boolean (*tyiconclickcallback) (hdlobject, boolean);


typedef struct tyicondata {
	
	short versionnumber;
	
	short resnum;
	
	boolean haslabel;
	
	unsigned short align: 4; /*DW 8/25/93*/
	
	unsigned short ixlabel: 3; /*DW 8/25/93*/
	
	Handle iconsuite; /*DW 8/25/93 -- if non-nil, it overrides resnum*/
	
	tyiconclickcallback iconclick; /*allows app to define what 2clicking or 1clicking does*/
	
	char waste [32];
	} tyicondata, **hdlicondata;


