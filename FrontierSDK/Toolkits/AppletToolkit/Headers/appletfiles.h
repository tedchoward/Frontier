
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef filesinclude
#define filesinclude


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif


typedef char tyfoldername [32];

typedef char tyfilename [32];


typedef enum tyfolderview { 
	
	viewbysmallicon = 0,
	
	viewbyicon = 1,
	
	viewbyname = 2, 
	
	viewbydate = 3, 
	
	viewbysize = 4,
	
	viewbykind = 5, 
	
	viewbycomment = 6,
	
	viewbycolor = 7,
	
	viewbyversion = 8
	} tyfolderview;


typedef struct tyfileinfo { /*flattens interface for getting file information*/

	OSErr errcode; /*if there was an error retrieving info about the file, this is the error number*/
	
	short vnum; /*the volume that the file is on*/
	
	long dirid; /*the folder that contains the file*/
	
	boolean flfolder: 1; /*true if it's a folder not a file*/
	
	boolean fllocked: 1; /*true if the file is locked*/
	
	boolean flbundle: 1; /*true if the file has a BNDL resource*/
	
	boolean flbusy: 1; /*true if the file is open in an application*/
	
	boolean flalias: 1; /*true if the file is an alias*/
	
	boolean flinvisible: 1;	/*true if the file is invisible*/
	
	boolean flvolume: 1; /*true if it's a disk volume*/
	
	boolean flejectable: 1; /*true if it's an ejectable volume*/
	
	boolean flstationery: 1; /*true if the file is a stationery pad*/
	
	boolean flshared: 1; /*true if the file can the file be shared*/
	
	boolean flnamelocked: 1; /*true if the file's name is locked*/
	
	boolean flcustomicon: 1; /*true if the file has a custom icon*/
	
	boolean flhardwarelock: 1; /*for volumes, if true, the device is readonly*/
	
	boolean flremotevolume: 1; /*for volumes, if true, it's a remote volume, accessed over the network*/
	
	OSType filecreator, filetype; /*the creator ID and file type*/
	
	long timecreated, timemodified; /*the creation and modification date for the file*/
	
	long sizedatafork, sizeresourcefork; /*the size of the file's two forks*/
	
	short ixlabel; /*the file's label, as displayed in the Finder*/
	
	Point iconposition; /*the position of the file's icon, when viewing by icon in Finder*/
	
	unsigned short ctfiles; /*for folders, the number of files in the folder*/
	
	tyfolderview folderview; /*for folders, view by name, by date, etc.*/
	
	unsigned long ctfreebytes; /*for volumes*/
	
	unsigned long cttotalbytes; /*for volumes*/
	} tyfileinfo;
	
	
#ifndef shelltypesinclude
	typedef FSSpec tyfilespec, *ptrfilespec;
#endif

typedef short tyvolnum;

typedef boolean (*sfcallback) (tyfileinfo *);


boolean fileexists (ptrfilespec);

boolean filemakespec (short, long, bigstring, ptrfilespec);

boolean filegetfilename (ptrfilespec, bigstring);

boolean equalfilespecs (ptrfilespec, ptrfilespec);

boolean sfdialog (boolean, bigstring, ptrfilespec, sfcallback, OSType);

boolean fileopen (ptrfilespec, short *);

boolean fileopenforreading (ptrfilespec, short *);

boolean fileopenresourcefork (ptrfilespec, short *);

boolean filereadwholefile (short, Handle *);

boolean filedeletefile (ptrfilespec);

boolean filenew (ptrfilespec, OSType, OSType, short *);

boolean filegetinfo (ptrfilespec, tyfileinfo *);

boolean filegetvolumeinfo (tyvolnum, tyfileinfo *);

boolean filesettype (ptrfilespec, OSType);

boolean filesetcreator (ptrfilespec, OSType);

void fileclose (short);

long filegetsize (short);

boolean fileseteof (short, long);

boolean filetruncate (short);

boolean filewrite (short, long, void *);

boolean fileread (short, long, void *);

boolean filewritehandle (short, Handle);

boolean filereadhandle (short, long, Handle *);

boolean filegetchar (short, byte *);

boolean filegetvolumename (short, bigstring);

#endif