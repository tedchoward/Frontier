
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

typedef struct odb_ * odbref;

typedef enum odbValueType  {
	
	unknownT = '\?\?\?\?',
	
	charT = 'char',
	
	shortT = 'shor',
	
	longT = 'long',
	
	binaryT = 'data',
	
	booleanT = 'bool',
	
	tokenT = 'tokn',
	
	dateT = 'date',
	
	addressT = 'addr',
	
	codeT = 'code',
	
	extendedT = 'exte',
	
	stringT = 'TEXT',
	
	externalT = 'xtrn',
	
	directionT = 'dir ',
	
	string4T = 'type',
	
	pointT = 'QDpt',
	
	rectT = 'qdrt',
	
	patternT = 'tptn',
	
	rgbT = 'cRGB',
	
	fixedT = 'fixd',
	
	singleT = 'sing',
	
	doubleT = 'doub',
	
	objspecT = 'obj ',
	
	filespecT = 'fss ',
	
	aliasT = 'alis',
	
	enumeratorT = 'enum',
	
	listT = 'list',
	
	recordT = 'reco',
	
	outlineT = 'optx',
	
	wptextT = 'wptx',
	
	tableT = 'tabl',
	
	scriptT = 'scpt',
	
	menubarT = 'mbar',
	
	pictureT = 'pict'
	
	} odbValueType;


typedef union odbValueData {
	
	boolean flvalue;
	
	unsigned char chvalue;
	
	short intvalue;
	
	long longvalue;
	
	unsigned long datevalue;
	
#ifdef MACVERSION
	tydirection dirvalue;
#endif
#ifdef WIN95VERSION
	byte dirvalue;
#endif
	
	OSType ostypevalue;
	
	Handle stringvalue;
	
	Handle addressvalue;
	
	Handle binaryvalue;
	
	Handle externalvalue;
	
	Point pointvalue;
	
	Rect **rectvalue;
	
	Pattern **patternvalue;
	
	RGBColor **rgbvalue;
	
	Fixed fixedvalue;
	
	float singlevalue;
	
	double **doublevalue;
	
	Handle objspecvalue;
	
	FSSpec **filespecvalue;
	
	Handle aliasvalue; /*AliasHandle*/
	
	OSType enumvalue;
	
	Handle listvalue;
	
	Handle recordvalue;
	} odbValueData;


/*note: must have 68k struct alignment compiler option set*/

typedef struct odbValueRecord {
	
	odbValueType valuetype;
	
	odbValueData data;
	} odbValueRecord;

extern pascal boolean odbUpdateOdbref (WindowPtr w, odbref odb);

extern pascal boolean odbAccessWindow (WindowPtr w, odbref *odb);

extern pascal boolean odbNewFile (hdlfilenum);

extern pascal boolean odbOpenFile (hdlfilenum, odbref *odb, boolean flreadonly);

extern pascal boolean odbSaveFile (odbref odb);

extern pascal boolean odbCloseFile (odbref odb);

extern pascal boolean odbDefined (odbref odb, bigstring bspath);

extern pascal boolean odbDelete (odbref odb, bigstring bspath);

extern pascal boolean odbGetType (odbref odb, bigstring bspath, OSType *type);

extern pascal boolean odbGetValue (odbref odb, bigstring bspath, odbValueRecord *value);

extern pascal boolean odbSetValue (odbref odb, bigstring bspath, odbValueRecord *value);

extern pascal boolean odbNewTable (odbref odb, bigstring bspath);

extern pascal boolean odbCountItems (odbref odb, bigstring bspath, long *count);

extern pascal boolean odbGetNthItem (odbref odb, bigstring bspath, long n, bigstring bsname);

extern pascal boolean odbGetModDate (odbref odb, bigstring bspath, unsigned long *date);

extern pascal void odbInitValue (odbValueRecord *value);

extern pascal void odbDisposeValue (odbref odb, odbValueRecord *value);

extern pascal void odbGetError (bigstring bs);

extern pascal boolean odbInstallEventHandler (void);

