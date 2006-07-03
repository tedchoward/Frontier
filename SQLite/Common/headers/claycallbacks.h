
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

#if filebrowser

boolean claycreatefile(fs), newfile (fs, 'LAND', 'TEXT')
#define clayfileexists(fs,isfolder) fileexists (fs, isfolder)
#define claycopyfile(x, y) copyfile(x, y, true, true)
#define filedelete deletefile
#define filemove movefile

#endif

#ifdef odbbrowser

#define clayfileexists(fs,isfolder) hashtablesymbolexists ((hdlhashtable) (*fs).parID, (*fs).name)

boolean claycopyfile (const tybrowserspec *, const tybrowserspec *);

boolean claydeletefile (const tybrowserspec *);

boolean claymovefile (const tybrowserspec *, const tybrowserspec *);

boolean claylookupvalue (const tybrowserspec *, tyvaluerecord *, hdlhashnode *);

boolean claycreatefile (const tybrowserspec *);

#endif

boolean claygetspecialfolder (void *, boolean, tybrowserspec *);

boolean clayemptyfilefolder (tybrowserspec *);

boolean claygetdirid (const tybrowserspec * , tybrowserdir *);

boolean claygetinmemorydirid (const tybrowserspec *, tybrowserdir *);

boolean claygetsubitemspec (tybrowserspec *, bigstring fname, tybrowserspec *);
	
boolean browsertextchanged (hdlheadrecord, bigstring);
