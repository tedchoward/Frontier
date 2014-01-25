
/*	$Id: langsqlite.h,v 1.1.2.1 2006/03/24 01:31:35 davidgewirtz Exp $    */

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


// prototypes

extern boolean sqliteopenverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-03-15 gewirtz

extern boolean sqlitecloseverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-03-17 gewirtz

extern boolean sqlitecompilequeryverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-18 gewirtz

extern boolean sqliteclearqueryverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-18 gewirtz

extern boolean sqliteresetqueryverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-08-31 gewirtz

extern boolean sqlitestepqueryverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-08-31 gewirtz

extern boolean sqlitegetcolumncountverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-18 gewirtz

extern boolean sqlitegetcolumntypeverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-18 gewirtz

extern boolean sqlitegetcolumnintverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-18 gewirtz

extern boolean sqlitegetcolumndoubleverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-18 gewirtz

extern boolean sqlitegetcolumntextverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-18 gewirtz

extern boolean sqlitegetcolumnnameverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-18 gewirtz

extern boolean sqlitegetcolumnverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-20 gewirtz

extern boolean sqlitegetrowverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-08-31 gewirtz

extern boolean sqlitegeterrormessageverb (hdltreenode, tyvaluerecord *, bigstring); // 2006-04-20 gewirtz

extern boolean sqliteinitverbs (void); // 2006-03-15 gewirtz

extern boolean sqlitesetcolumnblobverb ( hdltreenode, tyvaluerecord *, bigstring ); // 2007-08-25 creedon

extern boolean getlastinsertrowidverb ( hdltreenode, tyvaluerecord *, bigstring );  // 2007-08-28 creedon


// SQLITE MESSAGES

#define SQLITE_COLUMN_ERROR_0 BIGSTRING("\xe9""SQLite column error. Frontier uses a base-1 index. Columns must be specified with 1 indicating the first column. This is different from the SQLite API, which requires that columns must be specified with 0 indicating the first column.")
#define SQLITE_COLUMN_ERROR_MAX BIGSTRING("\x50""SQLite column error. An attempt was made to access a column that does not exist.")
#define SQLITE_COLUMN_ERROR_UNDEFINED BIGSTRING("\x29""SQLite returned an undefined column type.")
#define SQLITE_COLUMN_ERROR_ROW_OR_COLUMN BIGSTRING("\x31" "SQLite invalid row or column number out of range.") // 2007-08-25 creedon
#define SQLITE_PARAMETER_ERROR_COUNT BIGSTRING("\x1b" "SQLite parameter error. An attempt was made to access a parameter that does not exist.") // 2007-08-26 creedon

