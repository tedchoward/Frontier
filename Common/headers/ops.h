
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

#define opsinclude


/*typedefs*/

typedef struct tylinkedlistrecord {
	
	struct tylinkedlistrecord **hnext;
	} tylinkedlistrecord, *ptrlinkedlist, **hdllinkedlist;


/*prototypes*/

extern short minint (short, short);

extern short maxint (short, short);

extern short absint (short);

extern boolean delayticks (long);

extern boolean delayseconds (long);

extern void counttickloops (long *);

extern void burntickloops (long);

extern unsigned char uppercasechar (unsigned char);

extern unsigned char lowercasechar (unsigned char);

extern boolean textchar (unsigned char);

extern void shorttostring (short, bigstring);

extern void numbertostring (long, bigstring);

extern boolean stringtoshort (bigstring, short *);

extern boolean stringtonumber (bigstring, long *);

extern boolean stringtofloat (bigstring, double *);

extern boolean floattostring (double, bigstring);

extern long numberfromhandle (Handle);

extern void exittooperatingsystem (void);

extern short dirtoindex (tydirection);

extern tydirection indextodir (short);

extern boolean validdirection (tydirection);

extern tydirection oppositdirection (tydirection dir);

extern long divup (long, long);

extern long divround (long, long);

extern long quantumize (long, long);

extern boolean truenoop (void);

extern boolean falsenoop (void);

extern boolean gestalt (OSType, long *);

extern boolean listlink (hdllinkedlist, hdllinkedlist);

extern boolean listunlink (hdllinkedlist, hdllinkedlist);

#if __powerc

extern void safex80told (const extended80 *x80, long double *x);

extern void safeldtox80 (const long double *x, extended80 *x80);

#endif

extern void getsystemversionstring (bigstring, bigstring);

extern void getsizestring (unsigned long, bigstring);

