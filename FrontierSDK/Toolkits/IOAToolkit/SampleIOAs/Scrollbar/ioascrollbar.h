
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define ioascrollbarinclude


typedef struct tyscrollbardata {
	
	short versionnumber;
	
	short controlresourceid;
	
	short controlmin;
	
	short controlmax;
	
	ControlHandle hcontrol;
	
	char waste [16];
	} tyscrollbardata, **hdlscrollbardata;


