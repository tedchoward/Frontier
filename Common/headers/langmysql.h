
/*	$Id: langmysql.h,v 1.1.2.1 2007/04/09 18:30:00 davidgewirtz Exp $    */

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

/* prototypes */

extern boolean mysqlinitverb (hdltreenode, tyvaluerecord *, bigstring);				/* 2007-04-08 gewirtz */

extern boolean mysqlendverb (hdltreenode, tyvaluerecord *, bigstring);				/* 2007-04-15 gewirtz */

extern boolean mysqlconnectverb (hdltreenode, tyvaluerecord *, bigstring);			/* 2007-04-08 gewirtz */

extern boolean mysqlcompilequeryverb (hdltreenode, tyvaluerecord *, bigstring);		/* 2007-04-09 gewirtz */

extern boolean mysqlclearqueryverb (hdltreenode, tyvaluerecord *, bigstring);		/* 2007-04-09 gewirtz */

extern boolean mysqlgetrowverb (hdltreenode, tyvaluerecord *, bigstring);			/* 2007-04-09 gewirtz */

extern boolean mysqlgeterrornumberverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-10 gewirtz */

extern boolean mysqlgeterrormessageverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-10 gewirtz */

extern boolean mysqlgetclientinfoverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-10 gewirtz */

extern boolean mysqlgetclientversionverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-10 gewirtz */

extern boolean mysqlgethostinfoverb (hdltreenode, tyvaluerecord *, bigstring);		/* 2007-04-10 gewirtz */

extern boolean mysqlgetserverversionverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-10 gewirtz */

extern boolean mysqlgetprotocolinfoverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-10 gewirtz */

extern boolean mysqlgetserverinfoverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-10 gewirtz */

extern boolean mysqlgetqueryinfoverb (hdltreenode, tyvaluerecord *, bigstring);		/* 2007-04-10 gewirtz */

extern boolean mysqlgetaffectedrowcountverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-11 gewirtz */

extern boolean mysqlgetselectedrowcountverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-11 gewirtz */

extern boolean mysqlgetcolumncountverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-11 gewirtz */

extern boolean mysqlgetserverstatusverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-11 gewirtz */

extern boolean mysqlgetquerywarningcountverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-11 gewirtz */

extern boolean mysqlpingserververb (hdltreenode, tyvaluerecord *, bigstring);		/* 2007-04-11 gewirtz */

extern boolean mysqlseekrowverb (hdltreenode, tyvaluerecord *, bigstring);			/* 2007-04-11 gewirtz */

extern boolean mysqlselectdatabaseverb (hdltreenode, tyvaluerecord *, bigstring);	/* 2007-04-11 gewirtz */

extern boolean mysqlgetsqlstateverb (hdltreenode, tyvaluerecord *, bigstring);		/* 2007-04-12 gewirtz */

extern boolean mysqlescapestringverb (hdltreenode, tyvaluerecord *, bigstring);		/* 2007-04-15 gewirtz */

extern boolean mysqlisthreadsafeverb (hdltreenode, tyvaluerecord *, bigstring);		/* 2007-04-15 gewirtz */

extern boolean mysqlcloseverb (hdltreenode, tyvaluerecord *, bigstring);			/* 2007-04-08 gewirtz */



extern boolean mysqlinitverbs (void); /* 2007-04-08 gewirtz */

/* boolean hmacmd5 (unsigned char *, int, unsigned char *, int, unsigned char *);*/ /* 2006-03-05 creedon */

/* boolean hmacsha1 (unsigned char *, int, unsigned char *, int, unsigned char *);*/ /* 2006-03-12 creedon */

/* SQLITE MESSAGES */
// #define SQLITE_COLUMN_ERROR_0 "\xe9""SQLite column error. Frontier uses a base-1 index. Columns must be specified with 1 indicating the first column. This is different from the SQLite API, which requires that columns must be specified with 0 indicating the first column."
// #define SQLITE_COLUMN_ERROR_MAX "\x50""SQLite column error. An attempt was made to access a column that does not exist."
// #define SQLITE_COLUMN_ERROR_UNDEFINED "\x29""SQLite returned an undefined column type."