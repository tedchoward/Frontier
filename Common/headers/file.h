
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


typedef struct tyfileinfo { // flattens interface for getting file information

	OSErr errcode; // if there was an error retrieving info about the file, this is the error number
	
	short vnum; // the volume that the file is on
	
	long dirid; // the folder that contains the file
	
	boolean flfolder; // true if it's a folder not a file
	
	boolean fllocked; // true if the file is locked
	
	boolean flbundle; // true if the file has a BNDL resource
	
	boolean flbusy; // true if the file is open in an application
	
	boolean flalias; // true if the file is an alias
	
	boolean flinvisible; // true if the file is invisible
	
	boolean flvolume; // true if it's a disk volume
	
	boolean flejectable; // true if it's an ejectable volume
	
	boolean flstationery; // true if the file is a stationery pad
	
	boolean flshared; // true if the file can the file be shared
	
	boolean flnamelocked; // true if the file's name is locked
	
	boolean flcustomicon; // true if the file has a custom icon
	
	boolean flhardwarelock; // for volumes, if true, the device is readonly
	
	boolean flremotevolume; // for volumes, if true, it's a remote volume, accessed over the network
	
	boolean flsystem; // Windows "system" file attribute
	
	boolean flarchive; // Windows "srchive" file attribute
	
	boolean flcompressed; // Windows "compressed" file attribute
	
	boolean fltemp; // Windows "temp" file attribute
	
	OSType filecreator, filetype; // the creator ID and file type
	
	long timecreated, timemodified, timeaccessed; // the creation and modification date for the file
	
	unsigned long long sizedataforkhigh, sizedatafork, sizeresourcefork; // the size of the file's two forks
	
	short ixlabel; // the file's label, as displayed in the Finder
	
	Point iconposition; // the position of the file's icon, when viewing by icon in Finder
	
	unsigned long ctfiles; // for folders, the number of files in the folder
	
	unsigned long ctfolders; // for volumes, the number of folders on the disk
	
	tyfolderview folderview; // for folders, view by name, by date, etc.
	
	unsigned long ctfreebytes; // for volumes
	
	unsigned long cttotalbytes; // for volumes
	
	unsigned long blocksize; // for volumes
	
	} tyfileinfo;


extern boolean flsupportslargevolumes; /*6.1b15 AR: fileverbs.c*/


/*prototypes*/

#ifdef WIN95VERSION

	typedef int (WINAPI * tyGetDiskFreeSpaceEx) (unsigned short *, ULARGE_INTEGER *, ULARGE_INTEGER *, ULARGE_INTEGER *);

	extern tyGetDiskFreeSpaceEx adrGetDiskFreeSpaceEx;

	extern void winsetfileinfo (WIN32_FIND_DATA * fileinfo, tyfileinfo *info);

	extern boolean winfileerror (const ptrfilespec );
	
#endif

extern void setfserrorparam ( const ptrfilespec );

extern boolean endswithpathsep (bigstring bs);

extern boolean cleanendoffilename (bigstring bs);

extern boolean getmachinename (bigstring bsname);

extern boolean filegetinfo (const ptrfilespec, tyfileinfo *info); // 1/28/97 dmb for clay

extern boolean filegetvolumeinfo (short vnum, tyfileinfo *info);

extern boolean filegetvolumename (short vnum, bigstring volname);

extern boolean filemakespec (short vnum, long dirid, bigstring fname, ptrfilespec pfs);

extern boolean getfsfile (const ptrfilespec pfs, bigstring name); // 1/28/97 dmb end for clay

extern void filenotfounderror (bigstring);

extern boolean surefile (const ptrfilespec );

extern boolean filegeteof (hdlfilenum, long *);

extern boolean fileseteof (hdlfilenum, long);

extern boolean filesetposition (hdlfilenum, long);

extern long filegetsize (hdlfilenum);

extern boolean filewrite (hdlfilenum, long, void *);

extern boolean filereaddata (hdlfilenum, long, long *, void *);

extern boolean fileread (hdlfilenum, long, void *);

extern boolean filegetchar (hdlfilenum, char *);

extern boolean fileputchar (hdlfilenum, char);

extern boolean filewritehandle (hdlfilenum, Handle);

extern boolean filereadhandle (hdlfilenum, Handle *);

extern boolean fileparsevolname (bigstring, ptrfilespec);

extern boolean fileresolvealias ( ptrfilespec );

extern boolean filefrompath (bigstring, bigstring);

extern boolean folderfrompath (bigstring, bigstring);

extern boolean deletefile ( const ptrfilespec );

extern boolean renamefile (const ptrfilespec , bigstring);

extern boolean movefile (const ptrfilespec , const ptrfilespec );

extern boolean lockfile (const ptrfilespec );

extern boolean unlockfile (const ptrfilespec );

extern boolean newfolder (const ptrfilespec );

extern boolean newfile ( const ptrfilespec , OSType, OSType );

extern void fileinit (void);

extern void fileshutdown (void);

extern boolean opennewfile ( ptrfilespec , OSType, OSType, hdlfilenum *);

extern boolean openfile ( const ptrfilespec , hdlfilenum *, boolean );

extern boolean closefile (hdlfilenum);

extern boolean flushvolumechanges (const ptrfilespec  fsspec, hdlfilenum fnum);

extern boolean getfullfilepath (bigstring);

extern boolean foldertest ( FSRefParamPtr );

extern boolean foldertestcipbr ( CInfoPBRec * );

extern boolean getfiletype (const ptrfilespec , OSType *);

extern boolean getfilecreator (const ptrfilespec , OSType *);

extern boolean filesize (const ptrfilespec , long *);

extern boolean fileisfolder ( const ptrfilespec , boolean * );

extern boolean fileisvolume (const ptrfilespec );

extern boolean fileislocked (const ptrfilespec , boolean *);

extern boolean fileisbusy (const ptrfilespec , boolean *);

extern boolean filehasbundle (const ptrfilespec , boolean *);

extern boolean filesetbundle (const ptrfilespec , boolean);

extern boolean fileisalias (const ptrfilespec , boolean *);

extern boolean fileisvisible (const ptrfilespec , boolean *);

extern boolean filesetvisible (const ptrfilespec , boolean);

extern boolean getfiledates (const ptrfilespec , unsigned long *, unsigned long *);

extern boolean setfiledates (const ptrfilespec , unsigned long, unsigned long);
	
extern boolean getfilepos (const ptrfilespec , Point *);

extern boolean setfilepos (const ptrfilespec , Point);

extern boolean setfilecreated (const ptrfilespec , long);
	
extern boolean setfilemodified (const ptrfilespec , long);

extern boolean setfiletype (const ptrfilespec , OSType);

extern boolean setfilecreator (const ptrfilespec , OSType);

extern boolean largefilebuffer (Handle *);

extern boolean copyfile (const ptrfilespec , const ptrfilespec , boolean, boolean);

extern short filegetapplicationvnum (void);

extern short filegetapplicationrnum (void);

extern short filegetsystemvnum (void);

extern boolean getspecialfolderpath (bigstring, bigstring, boolean, ptrfilespec );

extern boolean ejectvol (const ptrfilespec );

extern boolean isejectable (const ptrfilespec , boolean *);

extern boolean langgetextendedvolumeinfo (const ptrfilespec , double *, double *);

extern boolean getfreespace (const ptrfilespec , long *);

extern boolean getvolumesize (const ptrfilespec , long *);

extern boolean getvolumeblocksize (const ptrfilespec , long *);

extern boolean filesonvolume (const ptrfilespec , long *);

extern boolean foldersonvolume (const ptrfilespec , long *);

extern boolean isvolumelocked (const ptrfilespec , boolean *);

extern boolean volumecreated (const ptrfilespec , unsigned long *);

extern boolean lockvolume (const ptrfilespec , boolean);

extern boolean unmountvolume (const ptrfilespec );

extern boolean drivenumtovolname (short, bigstring);

extern boolean findapplication (OSType, ptrfilespec );

extern boolean getfilecomment (const ptrfilespec , bigstring);

extern boolean setfilecomment (const ptrfilespec , bigstring);

extern boolean getfilelabel (const ptrfilespec , bigstring);

extern boolean setfilelabel (const ptrfilespec , bigstring);

extern boolean mountvolume (bigstring, bigstring, bigstring);

extern boolean initfile (void);


extern boolean fiffindinfile (const ptrfilespec , bigstring, long *); /*findinfile.c*/

extern boolean fifcomparefiles (const ptrfilespec , const ptrfilespec );

extern boolean fifcharcounter (const ptrfilespec , char, long *);

extern boolean fifclosefile (const ptrfilespec );

extern boolean fifcloseallfiles (long);

extern boolean fifopenfile (const ptrfilespec , long);

extern boolean fifendoffile (const ptrfilespec );

extern boolean fifreadline (const ptrfilespec , Handle *);

extern boolean fifwriteline (const ptrfilespec , Handle);

extern boolean fifreadhandle (const ptrfilespec , long, Handle *);

extern boolean fifreadfile (const ptrfilespec fs, Handle *x);	/* 2006-04-11 aradke */

extern boolean fifwritehandle (const ptrfilespec , Handle);

extern boolean fifsetposition (const ptrfilespec , long);

extern boolean fifgetposition (const ptrfilespec , long *);

extern boolean fifsetendoffile (const ptrfilespec fs, long eof);

extern boolean fifgetendoffile (const ptrfilespec fs, long *eof);


#pragma mark === file.c ===

extern boolean equalfilespecs ( const ptrfilespec, const ptrfilespec );

extern boolean filegetposition (hdlfilenum, long *);

extern boolean fileexists ( const ptrfilespec , boolean * );

extern boolean filetruncate (hdlfilenum);

#ifdef MACVERSION

	extern void filegetinfofrompb ( FSRefParam *, tyfileinfo * );

	extern void filegetinfofrompbcipbr ( CInfoPBRec *, tyfileinfo * );

	extern boolean getmacfileinfo ( const ptrfilespec, FSRefParamPtr, FSCatalogInfoPtr );

	extern boolean getmacfileinfocipbr ( const FSSpecPtr, CInfoPBRec * );

#endif


#pragma mark === filedialog.c ===

extern boolean sfdialog (tysfverb, bigstring, ptrsftypelist, ptrfilespec , OSType); /* 2005-10-06 creedon - added OSType */

extern boolean initfiledialog (void);

#ifdef MACVERSION

	//Code change by Timothy Paustian Tuesday, June 20, 2000 2:22:02 PM
	//Nav services code for Frontier.
	
	extern OSErr	TimsPutFile( bigstring, Str255, StandardFileReply *, OSType );

	extern OSErr	TimsGetFolderOrVolume(bigstring prompt, SInt16 dialogType, StandardFileReply * outReply);

	extern pascal void		NavEventProc(NavEventCallbackMessage callBackSelector, 
					    NavCBRecPtr callBackParms, 
					    NavCallBackUserData callBackUD);

	extern OSErr getafile (bigstring prompt, ptrsftypelist filetypes, StandardFileReply * outReply, OSType); /* 2005-09-23 creedon */ 

#endif
                        
#endif


#pragma mark === filemp3.c ===

boolean getmp3info (const ptrfilespec , long *, long *, long *, long *, boolean *);


#pragma mark === fileops.c ===

extern void clearfilespec(ptrfilespec);	// 2009-09-05 aradke

extern boolean setfilelabelindex (const ptrfilespec , short, boolean); // 2006-04-23 creedon */

extern short getfilelabelindex (const ptrfilespec , short *);	// 2006-04-23 creedon */

#ifdef MACVERSION

	extern boolean macfilespecisvalid(const ptrfilespec fs);	// 2009-09-05 aradke
	
	extern boolean macfilespecisresolvable(const ptrfilespec fs);	// 2009-09-13 aradke
	
	extern OSErr macgetfsref(const ptrfilespec fs, FSRef* fsref);	// 2009-09-05 aradke
	
	extern OSErr macgetfsspec(const ptrfilespec fs, FSSpec *fss);	// 2009-09-06 aradke
	
	extern OSErr macgetfilespecparent(const ptrfilespec fs, ptrfilespec fsparent);	// 2009-09-05 aradke
	
	extern OSErr macgetfilespecchild(const ptrfilespec fs, tyfsnameptr name, ptrfilespec fschild);	// 2009-09-13 aradke
	
	extern OSErr macgetfilespecchildfrombigstring(const ptrfilespec fs, bigstring bsname, ptrfilespec fschild);	// 2009-09-13 aradke

	extern OSErr macmakefilespec(const FSRef *fsref, ptrfilespec fs);	// 2009-09-06 aradke
	
	extern boolean macgetfsrefname(const FSRef *fsref, tyfsnameptr fsname);	// 2009-09-05 aradke
	
	extern boolean macgetfsrefnameasbigstring(const FSRef *fsref, bigstring bs);	// 2009-09-05 aradke
	
	extern void macgetfilespecname(const ptrfilespec fs, tyfsnameptr fsname);	// 2009-09-05 aradke
	
	extern void macgetfilespecnameasbigstring(const ptrfilespec fs, bigstring bs);	// 2009-09-05 aradke
	
	extern void clearfsname(tyfsnameptr fsname);	// 2009-09-05 aradke
	
	extern void copyfsname(const tyfsnameptr fssource, tyfsnameptr fsdest);	// 2009-09-05 aradke
		
	unsigned short fsnamelength(const tyfsnameptr fsname);	// 2009-09-05 aradke

	extern void bigstringtofsname(const bigstring bs, tyfsnameptr fsname);	// 2009-09-05 aradke
	
	extern void fsnametobigstring(const tyfsnameptr fsname, bigstring bs);	// 2009-09-05 aradke
	
	extern boolean CFStringRefToStr255 (CFStringRef input, StringPtr output); // 2006-08-08 creedon
	
	extern OSStatus GetApplicationIconRef ( const ProcessSerialNumber * , const FSSpec* , IconRef * ); // 2006-06-04 creedon

	extern OSStatus LSIsApplication( const FSRef *, Boolean *, Boolean * ); // 2006-05-25

#endif // MACVERSION


#pragma mark === filepath.c ===
 
extern boolean directorytopath ( const ptrfilespec, bigstring);

extern boolean volumerefnumtopath (short, bigstring);

extern boolean filesetdefaultpath ( const ptrfilespec );

extern boolean filegetdefaultpath (ptrfilespec );

extern boolean filespectopath (const ptrfilespec , bigstring);

extern boolean pathtofilespec ( bigstring, ptrfilespec );

extern boolean filegetpath (const ptrfilespec , bigstring);

extern boolean setfsfile (ptrfilespec , bigstring);

extern boolean getfsfile (const ptrfilespec , bigstring);

extern boolean getfsvolume (const ptrfilespec, long *);

extern void initfsdefault (void); /* 2005-07-18 creedon */

#ifdef MACVERSION

	extern OSStatus pathtofsref (bigstring, FSRef *);

#endif // MACVERSION


#pragma mark === fileverbs.c ===
 
 //extern filecheckdefaultpath (bigstring);

extern boolean filegetprogramversion (bigstring);

extern boolean filestart (void); /*6.1b15 AR*/

