
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

#ifndef FDLLCALL_H
#define FDLLCALL_H


typedef struct odb_ * odbRef;

typedef unsigned char odbBool;

typedef unsigned char odbString [256];

#ifndef isFrontier

#ifdef MACVERSION
typedef short hdlfilenum;
#endif

#ifdef WIN32
#define Handle HANDLE
typedef unsigned long FourCharCode;

typedef FourCharCode OSType;

typedef struct tyPoint
	{
	short	v;
	short h;
	} Point, *PointPtr;

typedef RECT Rect;

typedef HANDLE hdlfilenum;
#endif

#endif

typedef enum odbDirection {
	
	dir_nodirection = 0, 
	
	dir_up = 1, 
	
	dir_down = 2, 
	
	dir_left = 3,
	
	dir_right = 4, 
	
	dir_flatup = 5, 
	
	dir_flatdown = 6, 
	
	dir_sorted = 8,
	
	dir_pageup = 9,
	
	dir_pagedown = 10,
	
	dir_pageleft = 11,
	
	dir_pageright = 12
	} odbDirection;


typedef enum odbValueType  {
	
	odb_unknownvaluetype = '\?\?\?\?',
	
	odb_charvaluetype = 'char',
	
	odb_shortvaluetype = 'shor',
	
	odb_longvaluetype = 'long',
	
	odb_binaryvaluetype = 'data',
	
	odb_booleanvaluetype = 'bool',
	
	odb_tokenvaluetype = 'tokn',
	
	odb_datevaluetype = 'date',
	
	odb_addressvaluetype = 'addr',
	
	odb_codevaluetype = 'code',
	
	odb_extendedvaluetype = 'exte',
	
	odb_stringvaluetype = 'TEXT',
	
	odb_externalvaluetype = 'xtrn',
	
	odb_directionvaluetype = 'dir ',
	
	odb_string4valuetype = 'type',
	
	odb_pointvaluetype = 'QDpt',
	
	odb_rectvaluetype = 'qdrt',
	
	odb_patternvaluetype = 'tptn',
	
	odb_rgbvaluetype = 'cRGB',
	
	odb_fixedvaluetype = 'fixd',
	
	odb_singlevaluetype = 'sing',
	
//	doublevaluetype = 'exte',
	odb_doublevaluetype = 'doub',
	
	odb_objspecvaluetype = 'obj ',
	
	odb_filespecvaluetype = 'fss ',
	
	odb_aliasvaluetype = 'alis',
	
	odb_enumeratorvaluetype = 'enum',
	
	odb_listvaluetype = 'list',
	
	odb_recordvaluetype = 'reco',
	
	odb_outlinevaluetype = 'optx',
	
	odb_wptextvaluetype = 'wptx',
	
	odb_tablevaluetype = 'tabl',
	
	odb_scriptvaluetype = 'scpt',
	
	odb_menubarvaluetype = 'mbar',
	
	odb_picturevaluetype = 'pict'
	
	} odbValueType;


typedef union odbValueData {
	
	odbBool flvalue;
	
	unsigned char chvalue;
	
	short intvalue;
	
	long longvalue;
	
	unsigned long datevalue;
	
	odbDirection dirvalue;
	
	OSType ostypevalue;
	
	Handle stringvalue;
	
	Handle addressvalue;
	
	Handle binaryvalue;
	
	Handle externalvalue;
	
	Point pointvalue;
	
	Rect **rectvalue;

#ifdef MACVERSION
	Pattern **patternvalue;
	
	RGBColor **rgbvalue;
	
	Fixed fixedvalue;
	
	FSSpec **filespecvalue;
#endif
	
	float singlevalue;
	
//	extended80 **doublevalue;
	double **doublevalue;
	
	Handle objspecvalue;
	
	Handle aliasvalue; /*AliasHandle*/
	
	OSType enumvalue;
	
	Handle listvalue;
	
	Handle recordvalue;
	} odbValueData;


typedef struct odbValueRecord {
	
	odbValueType valuetype;
	
	odbValueData data;
	} odbValueRecord;

#ifdef MACVERSION
#define xCALLBACK
#endif

#ifdef WIN32
#define xCALLBACK CALLBACK
#endif

typedef struct tyXDLLProcTable {
	Handle (xCALLBACK *xMemAlloc) (long sz);
	Handle (xCALLBACK *xMemResize)(Handle m, long sz);
	char * (xCALLBACK *xMemLock) (Handle m);
	void (xCALLBACK *xMemUnlock) (Handle  m);
	void (xCALLBACK *xMemFree) (Handle m);
	long (xCALLBACK *xMemGetSize) (Handle m);

	odbRef (xCALLBACK *xOdbGetCurrentRoot) ();
	odbBool (xCALLBACK *xOdbNewFile) (hdlfilenum);
	odbBool (xCALLBACK *xOdbOpenFile) (hdlfilenum, odbRef *odb);
	odbBool (xCALLBACK *xOdbSaveFile) (odbRef odb);
	odbBool (xCALLBACK *xOdbCloseFile) (odbRef odb);
	odbBool (xCALLBACK *xOdbDefined) (odbRef odb, odbString bspath);
	odbBool (xCALLBACK *xOdbDelete) (odbRef odb, odbString bspath);
	odbBool (xCALLBACK *xOdbGetType) (odbRef odb, odbString bspath, OSType *type);
	odbBool (xCALLBACK *xOdbCountItems) (odbRef odb, odbString bspath, long *count);
	odbBool (xCALLBACK *xOdbGetNthItem) (odbRef odb, odbString bspath, long n, odbString bsname);
	odbBool (xCALLBACK *xOdbGetValue) (odbRef odb, odbString bspath, odbValueRecord *value);
	odbBool (xCALLBACK *xOdbSetValue) (odbRef odb, odbString bspath, odbValueRecord *value);
	odbBool (xCALLBACK *xOdbNewTable) (odbRef odb, odbString bspath);
	odbBool (xCALLBACK *xOdbGetModDate) (odbRef odb, odbString bspath, unsigned long *date);
	void (xCALLBACK *xOdbDisposeValue) (odbRef odb, odbValueRecord *value);
	void (xCALLBACK *xOdbGetError) (odbString bs);

	odbBool (xCALLBACK *xDoScript) (char * script, long len, odbValueRecord *value);
	odbBool (xCALLBACK *xDoScriptText) (char * script, long len, Handle *text);

	odbBool (xCALLBACK *xOdbNewListValue) (odbRef odb, odbValueRecord *value, odbBool flRecord);
	odbBool (xCALLBACK *xOdbGetListCount) (odbRef odb, odbValueRecord *value, long * cnt);

	// 2006-04-04 - kw --- removed parameter names
	// odbBool (xCALLBACK *xOdbDeleteListValue) (odbRef odb, odbValueRecord *value, long index, char * recordname);
	odbBool (xCALLBACK *xOdbDeleteListValue) (odbRef, odbValueRecord *, long, char *);
	// odbBool (xCALLBACK *xOdbSetListValue) (odbRef odb, odbValueRecord *value, long index, char * recordname, odbValueRecord *valueData);
	odbBool (xCALLBACK *xOdbSetListValue) (odbRef, odbValueRecord *, long, char *, odbValueRecord *);
	// odbBool (xCALLBACK *xOdbGetListValue) (odbRef odb, odbValueRecord *value, long index, char * recordname, odbValueRecord *valueReturn);
	odbBool (xCALLBACK *xOdbGetListValue) (odbRef, odbValueRecord *, long, char *, odbValueRecord *);

	odbBool (xCALLBACK *xOdbAddListValue) (odbRef odb, odbValueRecord *value, char * recordname, odbValueRecord *valueData);

	odbBool (xCALLBACK *xInvoke) (odbString bsscriptname, void * params, odbValueRecord * value, odbBool *flfound, unsigned int * errarg);
	odbBool (xCALLBACK *xCoerce) (odbValueRecord * odbval, odbValueType newtype);

	odbBool (xCALLBACK *xCallScript) (odbString bspath, odbValueRecord *vparams, odbValueRecord *value); /* 2002-10-13 AR */
	odbBool (xCALLBACK *xCallScriptText) (odbString bspath, odbValueRecord *vparams, Handle * text); /* 2002-10-13 AR */

	odbBool (xCALLBACK *xThreadYield) (); /* 2003-04-22 AR */
	odbBool (xCALLBACK *xThreadSleep) (long sleepticks); /* 2003-04-22 AR */
	} XDLLProcTable;


#define maxdllparams 16

typedef struct tydllparamblock {
	
	long ctparams;
	
	long paramdata [maxdllparams];
	
	long paramsize [maxdllparams];
	
	long resultdata;
	
	long resultsize;
	
//	XDLLProcTable * proctable;
	
	unsigned char errormessage [256]; // c string on pc, p string on mac
	} tydllparamblock;


typedef odbBool (xCALLBACK * tyDLLEXTROUTINE) (tydllparamblock * data, XDLLProcTable * proctable);


#endif

