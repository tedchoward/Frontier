
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

#define shellundoinclude /*so other includes can tell if we've been loaded*/



#pragma pack(2)
typedef struct tystack {

	short topstack;

	short basesize;

	short elemsize;
	
			
		byte stack [1];
	
	} tystack, *ptrstack, **hdlstack;


typedef boolean (*undocallback) (Handle, boolean);


typedef struct tyundorecord {
	
	undocallback undoroutine;
	
	Handle hundodata;
	
	boolean flactionstep; /*is this step an action record?*/
	} tyundorecord;


typedef struct tyactionrecord {
	
	long ixaction;
	
	long globaldata;
	
	boolean flaction; /*always true for actionrecords*/
	} tyactionrecord;


typedef struct tyundostack {

	short topundo;
	
	short basesize;
	
	short elemsize;
	
	/*
	union {
		tyundorecord undostep;
		
		tyactionrecord actionstep;
		} u;
	*/
		
	short ixaction;
	
	long globaldata;
	
	
		tyundorecord undostep [1];
		
	} tyundostack, *ptrundostack, **hdlundostack;
#pragma options align=reset


/*function prototypes*/

extern boolean pushundostep (undocallback, Handle);

extern boolean pushundoaction (short);

extern boolean popundoaction (void);

extern boolean undolastaction (boolean);

extern boolean redolastaction (boolean);

extern boolean getundoaction (short *);

extern boolean getredoaction (short *);

extern void killundo (void);

extern boolean newundostack (hdlundostack *);

extern boolean disposeundostack (hdlundostack);

extern void initundo (void);


/*global variables*/

extern hdlundostack shellundostack;

extern hdlundostack shellredostack;


