
/*	$Id$    */

/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

#ifndef fileinclude
#define fileinclude


#ifndef shelltypesinclude

	#include "shelltypes.h"

#endif


//universal error codes
#ifdef MACVERSION
#define errorVolume					nsvErr
#define errorParam					paramErr
#define errorFileNotFound			fnfErr
#define errorDirNotFound			dirNFErr
#define errorNone					noErr
#ifndef __NAVIGATION__
#include <Navigation.h>
#endif
#endif

#ifdef WIN95VERSION
#define errorVolume					ERROR_PATH_NOT_FOUND
#define errorParam					ERROR_INVALID_PARAMETER
#define errorFileNotFound			ERROR_FILE_NOT_FOUND
#define errorDirNotFound			ERROR_PATH_NOT_FOUND
#define errorNone					NO_ERROR
#endif



#ifdef MACVERSION
	#define chpathseparator ':'
#endif

#ifdef WIN95VERSION
	#define chpathseparator '\\'
#endif

	
/*selectors for sfdialog*/

typedef enum {
	
	sfputfileverb = 1,
	
	sfgetfileverb = 2,
	
	sfgetfolderverb = 3,
	
	sfgetdiskverb = 4
	} tysfverb;


#define maxsftypelist 20 /*up to ten file types for sf getfile*/


typedef struct tysftypelist { /*our version of sftypelist, includes type count*/
	
	short cttypes;
	
	OSType types [maxsftypelist];
	} tysftypelist, *ptrsftypelist;


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
	
	boolean flfolder; /*true if it's a folder not a file*/
	
	boolean fllocked; /*true if the file is locked*/
	
	boolean flbundle; /*true if the file has a BNDL resource*/
	
	boolean flbusy; /*true if the file is open in an application*/
	
	boolean flalias; /*true if the file is an alias*/
	
	boolean flinvisible;	/*true if the file is invisible*/
	
	boolean flvolume; /*true if it's a disk volume*/
	
	boolean flejectable; /*true if it's an ejectable volume*/
	
	boolean flstationery; /*true if the file is a stationery pad*/
	
	boolean flshared; /*true if the file can the file be shared*/
	
	boolean flnamelocked; /*true if the file's name is locked*/
	
	boolean flcustomicon; /*true if the file has a custom icon*/
	
	boolean flhardwarelock; /*for volumes, if true, the device is readonly*/
	
	boolean flremotevolume; /*for volumes, if true, it's a remote volume, accessed over the network*/
	
	boolean flsystem; /*Windows "system" file attribute*/
	
	boolean flarchive; /*Windows "srchive" file attribute*/
	
	boolean flcompressed; /*Windows "compressed" file attribute*/
	
	boolean fltemp; /*Windows "temp" file attribute*/
	
	OSType filecreator, filetype; /*the creator ID and file type*/
	
	long timecreated, timemodified, timeaccessed; /*the creation and modification date for the file*/
	
	long sizedataforkhigh, sizedatafork, sizeresourcefork; /*the size of the file's two forks*/
	
	short ixlabel; /*the file's label, as displayed in the Finder*/
	
	Point iconposition; /*the position of the file's icon, when viewing by icon in Finder*/
	
	unsigned short ctfiles; /*for folders, the number of files in the folder*/
	
	unsigned short ctfolders; /*for volumes, the number of folders on the disk*/
	
	tyfolderview folderview; /*for folders, view by name, by date, etc.*/
	
	unsigned long ctfreebytes; /*for volumes*/
	
	unsigned long cttotalbytes; /*for volumes*/
	
	unsigned long blocksize;
	} tyfileinfo;


extern boolean flsupportslargevolumes; /*6.1b15 AR: fileverbs.c*/


/*prototypes*/
#ifdef WIN95VERSION
	typedef int (WINAPI * tyGetDiskFreeSpaceEx) (unsigned short *, ULARGE_INTEGER *, ULARGE_INTEGER *, ULARGE_INTEGER *);

	extern tyGetDiskFreeSpaceEx adrGetDiskFreeSpaceEx;

	extern void winsetfileinfo (WIN32_FIND_DATA * fileinfo, tyfileinfo *info);

	extern boolean winfileerror (const tyfilespec *);
#endif

extern void setfserrorparam (const tyfilespec *);

extern boolean endswithpathsep (bigstring bs);

extern boolean cleanendoffilename (bigstring bs);

extern boolean getmachinename (bigstring bsname);

extern boolean equalfilespecs (const tyfilespec *, const tyfilespec *);

extern boolean filegetinfo (const tyfilespec *fs, tyfileinfo *info); // 1/28/97 dmb for clay

extern void filegetinfofrompb (CInfoPBRec *pb, tyfileinfo *info);

extern boolean filegetvolumeinfo (short vnum, tyfileinfo *info);

extern boolean filegetvolumename (short vnum, bigstring volname);

extern boolean getmacfileinfo (const tyfilespec *fs, CInfoPBRec *pb);

extern boolean filemakespec (short vnum, long dirid, bigstring fname, ptrfilespec pfs);

extern boolean filegetfilename (const tyfilespec *pfs, bigstring name); // 1/28/97 dmb end for clay

extern void filenotfounderror (bigstring);

extern boolean surefile (const tyfilespec *);

extern boolean filegeteof (hdlfilenum, long *);

extern boolean fileseteof (hdlfilenum, long);

extern boolean filesetposition (hdlfilenum, long);

extern boolean filegetposition (hdlfilenum, long *);

extern long filegetsize (hdlfilenum);

extern boolean filetruncate (hdlfilenum);

extern boolean filewrite (hdlfilenum, long, void *);

extern boolean filereaddata (hdlfilenum, long, long *, void *);

extern boolean fileread (hdlfilenum, long, void *);

extern boolean filegetchar (hdlfilenum, char *);

extern boolean fileputchar (hdlfilenum, char);

extern boolean filewritehandle (hdlfilenum, Handle);

extern boolean filereadhandle (hdlfilenum, Handle *);

#ifdef NEWFILESPECTYPE
extern boolean fileparsevolname (bigstring, long *, bigstring);
#else
extern boolean fileparsevolname (bigstring, short *, bigstring);
#endif

extern boolean fileresolvealias (tyfilespec *);

extern boolean filefrompath (bigstring, bigstring);

extern boolean folderfrompath (bigstring, bigstring);

extern boolean getfileparentfolder (const tyfilespec *, tyfilespec *);

extern boolean deletefile (const tyfilespec *);

extern boolean renamefile (const tyfilespec *, bigstring);

extern boolean movefile (const tyfilespec *, const tyfilespec *);

extern boolean lockfile (const tyfilespec *);

extern boolean unlockfile (const tyfilespec *);

extern boolean newfolder (const tyfilespec *);

extern boolean newfile (const tyfilespec *, OSType, OSType);

extern void fileinit (void);

extern void fileshutdown (void);

extern boolean fileopenorcreate (const tyfilespec *, OSType, OSType, hdlfilenum *);

extern boolean opennewfile (const tyfilespec *, OSType, OSType, hdlfilenum *);

extern boolean openfile (const tyfilespec *, hdlfilenum *, boolean);

extern boolean closefile (hdlfilenum);

extern boolean flushvolumechanges (const tyfilespec * fsspec, hdlfilenum fnum);

extern boolean getfullfilepath (bigstring);

extern boolean foldertest (CInfoPBRec *);

extern boolean getfiletype (const tyfilespec *, OSType *);

extern boolean getfilecreator (const tyfilespec *, OSType *);

extern boolean fileexists (const tyfilespec *, boolean *);

extern boolean filesize (const tyfilespec *, long *);

extern boolean fileisfolder (const tyfilespec *, boolean *);

extern boolean fileisvolume (const tyfilespec *);

extern boolean fileislocked (const tyfilespec *, boolean *);

extern boolean fileisbusy (const tyfilespec *, boolean *);

extern boolean filehasbundle (const tyfilespec *, boolean *);

extern boolean filesetbundle (const tyfilespec *, boolean);

extern boolean fileisalias (const tyfilespec *, boolean *);

extern boolean fileisvisible (const tyfilespec *, boolean *);

extern boolean filesetvisible (const tyfilespec *, boolean);

extern boolean getfiledates (const tyfilespec *, unsigned long *, unsigned long *);

extern boolean setfiledates (const tyfilespec *, unsigned long, unsigned long);

extern boolean getfilepos (const tyfilespec *, Point *);

extern boolean setfilepos (const tyfilespec *, Point);

extern boolean setfilecreated (const tyfilespec *, long);

extern boolean setfilemodified (const tyfilespec *, long);

extern boolean setfiletype (const tyfilespec *, OSType);

extern boolean setfilecreator (const tyfilespec *, OSType);

extern boolean largefilebuffer (Handle *);

extern boolean copyfile (const tyfilespec *, const tyfilespec *, boolean, boolean);

extern short filegetapplicationvnum (void);

extern short filegetapplicationrnum (void);

extern short filegetsystemvnum (void);

extern boolean getspecialfolderpath (bigstring, bigstring, boolean, tyfilespec *);

extern boolean ejectvol (const tyfilespec *);

extern boolean isejectable (const tyfilespec *, boolean *);

extern boolean langgetextendedvolumeinfo (const tyfilespec *, double *, double *);

extern boolean getfreespace (const tyfilespec *, long *);

extern boolean getvolumesize (const tyfilespec *, long *);

extern boolean getvolumeblocksize (const tyfilespec *, long *);

extern boolean filesonvolume (const tyfilespec *, long *);

extern boolean foldersonvolume (const tyfilespec *, long *);

extern boolean isvolumelocked (const tyfilespec *, boolean *);

extern boolean volumecreated (const tyfilespec *, unsigned long *);

extern boolean lockvolume (const tyfilespec *, boolean);

extern boolean unmountvolume (const tyfilespec *);

extern boolean drivenumtovolname (short, bigstring);

extern boolean findapplication (OSType, tyfilespec *);

extern boolean getfilecomment (const tyfilespec *, bigstring);

extern boolean setfilecomment (const tyfilespec *, bigstring);

extern boolean getfilelabel (const tyfilespec *, bigstring);

extern boolean setfilelabel (const tyfilespec *, bigstring);

extern boolean mountvolume (bigstring, bigstring, bigstring);

extern boolean initfile (void);


extern boolean fiffindinfile (const tyfilespec *, bigstring, long *); /*findinfile.c*/

extern boolean fifcomparefiles (const tyfilespec *, const tyfilespec *);

extern boolean fifcharcounter (const tyfilespec *, char, long *);

extern boolean fifclosefile (const tyfilespec *);

extern boolean fifcloseallfiles (long);

extern boolean fifopenfile (const tyfilespec *, long);

extern boolean fifendoffile (const tyfilespec *);

extern boolean fifreadline (const tyfilespec *, Handle *);

extern boolean fifwriteline (const tyfilespec *, Handle);

extern boolean fifreadhandle (const tyfilespec *, long, Handle *);

extern boolean fifwritehandle (const tyfilespec *, Handle);

extern boolean fifsetposition (const tyfilespec *, long);

extern boolean fifgetposition (const tyfilespec *, long *);

extern boolean fifsetendoffile (const tyfilespec *fs, long eof);

extern boolean fifgetendoffile (const tyfilespec *fs, long *eof);


extern boolean directorytopath (long, short, bigstring); /*filepath.c*/

extern boolean volumerefnumtopath (short, bigstring);

extern boolean filesetdefaultpath (const tyfilespec *);

extern boolean filegetdefaultpath (tyfilespec *);

extern boolean filespectopath (const tyfilespec *, bigstring);

extern boolean pathtofilespec (bigstring, tyfilespec *);

extern boolean filegetpath (const tyfilespec *, bigstring);

extern boolean setfsfile (tyfilespec *, bigstring);

extern boolean getfsfile (const tyfilespec *, bigstring);

extern boolean getfsvolume (const tyfilespec *, long *);

//extern filecheckdefaultpath (bigstring); /*fileverbs.c*/

extern boolean filegetprogramversion (bigstring);

extern boolean filestart (void); /*6.1b15 AR*/

extern OSErr GetApplicationPackageFSSpecFromBundle (tyfilespec *); /* 2005-07-17 creedon */

// filedialog.c

extern boolean sfdialog (tysfverb, bigstring, ptrsftypelist, tyfilespec *);

extern boolean initfiledialog (void);

#ifdef MACVERSION
	//Code change by Timothy Paustian Tuesday, June 20, 2000 2:22:02 PM
	//Nav services code for Frontier.
	extern OSErr	TimsPutFile(bigstring prompt, Str255 fileName, StandardFileReply * outReply);

	extern OSErr	TimsGetFile(bigstring prompt, ptrsftypelist filetypes, StandardFileReply * 	outReply);

	extern OSErr	TimsGetFolderOrVolume(bigstring prompt, SInt16 dialogType, StandardFileReply * 	outReply);

	extern pascal void		NavEventProc(NavEventCallbackMessage callBackSelector, 
	                        NavCBRecPtr callBackParms, 
	                        NavCallBackUserData callBackUD);
#endif
                        
#endif

// filemp3.c

boolean getmp3info (const tyfilespec *, long *, long *, long *, long *, boolean *);



