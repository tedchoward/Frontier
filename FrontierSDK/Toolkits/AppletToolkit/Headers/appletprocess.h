
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define appletprocessinclude


#ifndef __PROCESSES__

	#include <Processes.h>
	
#endif


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif


typedef struct typrocessinfo {
	
	char name [32]; /*depends on how the app was launched*/
	
    ProcessSerialNumber serialnumber, launcherserialnumber;
     
    FSSpec file; /*the file the app was launched from*/
   
    OSType filetype; /*type of the file*/
    
    OSType filecreator; /*creator ID of the file*/
    
    unsigned long sizerecourceflags;
    
    Ptr partition; /*where the app is loaded in memory*/
    
    unsigned long partitionsize; /*how much memory the app occupies*/
    
    unsigned long freebytesinheap; /*how many bytes are free in its application heap*/
    
    unsigned long launchticks; /*the value of TickCount when the app was launched*/
    
    unsigned long activeticks; /*the number of ticks it has consumed*/
    
    unsigned long refcon; /*you can add your own info to this record*/
	} typrocessinfo, **hdlprocessinfo;
	

typedef boolean (*typrocessloopcallback) (typrocessinfo *, long);

boolean processloop (typrocessloopcallback, long);

boolean findrunningapp (OSType, typrocessinfo *);

boolean bringapptofront (OSType);

boolean launchappwithdoc (FSSpec *, FSSpec *, boolean);

boolean sendodoc (OSType, FSSpec *);

boolean launchusingfinder (FSSpec *);

boolean revealinfinder (FSSpec *);

boolean launchdeskaccessoryfile (FSSpec *);

boolean fileisrunning (FSSpec *);

boolean equalprocesses (typrocessinfo *, typrocessinfo *);

boolean quitapplication (OSType);

boolean launchHelperApp (OSType, void *);

boolean getcurrentappfilespec (FSSpec *);

boolean getcurrentappfolder (FSSpec *);



