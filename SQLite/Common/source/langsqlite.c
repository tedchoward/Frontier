
/*	$Id: langsqlite.c,v 1.1.2.1 2006/03/24 01:32:31 davidgewirtz Exp $    */

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

#include "frontier.h"
#include "standard.h"

#include "frontierconfig.h"

#include "error.h"
#include "ops.h"
#include "langinternal.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "resources.h"
#include "strings.h"
#include "tablestructure.h"
#include "lang.h"
#include "langexternal.h"

#include "langsqlite.h"
#include <sqlite3.h>

/* Building SQLite into Frontier

The SQLite source distribution is compiled into the Frontier code as part of this
module. A key goal in this implementation was to avoid touching any of the SQLite
source files. SQLite is pretty complex in its coding, and I didn't want to take a 
chance that any changes to the original SQLite code would muck up the database
functionality.

To include the latest SQLite distribution (we're implementing SQLite 3 here), 
download the SQLite source code from http://www.sqlite.org/download.html.

Move all the SQLite source files into the \Common\sqlite3 folder. If you need to
include new files in the SQLite project, be sure that shell.c and tclsqlite.c are
NOT included in the Frontier project. 

That's it. For further tips, see the Frontier Lab Notebook article at:

http://manila.zatz.com/frontierkernel/stories/storyReader$29

-- DG

*/

typedef enum tysqliteverbtoken { /* verbs that are processed by langsqlite.c */

	openfunc,
	compilequeryfunc,
	clearqueryfunc,
	executequeryfunc,
	getcolumncountfunc,
	getcolumntypefunc,
	getcolumnintfunc,
	getcolumndoublefunc,
	getcolumntextfunc,
	getcolumnnamefunc,
	getcolumnfunc,
	geterrormessagefunc,
	closefunc,
	ctsqliteverbs
	} tysqliteverbtoken;


static boolean sqlitefunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	2006-03-15 gewirtz: created, cribbed from langcrypt, itself cribbed from htmlfunctionvalue
	*/
	
	hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v);			/* by default, sqlite functions return false */
	
	switch (token) {
		
		case openfunc: {				/* 2006-03-14 gewirtz: open an SQLite db */

			return (sqliteopenverb (hp1, v, bserror));
			} /* openfunc */

		case compilequeryfunc: {		/* 2006-04-18 gewirtz: prepare an SQLite statement */

			return (sqlitecompilequeryverb (hp1, v, bserror));		
			} /* compilequeryfunc */

		case clearqueryfunc: {			/* 2006-04-18 gewirtz: finalize an SQLite statement */

			return (sqliteclearqueryverb (hp1, v, bserror));		
			} /* clearqueryfunc */

		case executequeryfunc: {			/* 2006-04-18 gewirtz: move the cursor one step after the query */

			return (sqliteexecutequeryverb (hp1, v, bserror));		
			} /* executequeryfunc */

		case getcolumncountfunc: {			/* 2006-04-18 gewirtz: get the number of columns */

			return (sqlitegetcolumncountverb (hp1, v, bserror));		
			} /* getcolumncountfunc */

		case getcolumntypefunc: {			/* 2006-04-18 gewirtz: get the type of the referenced column */
										/*                     this version returns Frontier strings */
			return (sqlitegetcolumntypeverb (hp1, v, bserror));		
			} /* getcolumntypefunc */

		case getcolumnintfunc: {			/* 2006-04-18 gewirtz: get an int value from the column */

			return (sqlitegetcolumnintverb (hp1, v, bserror));		
			} /* getcolumnintfunc */

		case getcolumndoublefunc: {		/* 2006-04-18 gewirtz: get a double value from the column */

			return (sqlitegetcolumndoubleverb (hp1, v, bserror));		
			} /* getcolumndoublefunc */

		case getcolumntextfunc: {			/* 2006-04-18 gewirtz: get a text value from the column */

			return (sqlitegetcolumntextverb (hp1, v, bserror));		
			} /* getcolumntextfunc */

 		case getcolumnnamefunc: {			/* 2006-04-18 gewirtz: get the field name of the column */

			return (sqlitegetcolumnnameverb (hp1, v, bserror));		
			} /* getcolumnnamefunc */

 		case getcolumnfunc: {				/* 2006-04-20 gewirtz: get the value of the column */

			return (sqlitegetcolumnverb (hp1, v, bserror));		
			} /* getcolumnfunc */

 		case geterrormessagefunc: {		/* 2006-04-20 gewirtz: get the SQLite error message */

			return (sqlitegeterrormessageverb (hp1, v, bserror));		
			} /* geterrormessagefunc */

		case closefunc: {				/* 2006-03-15 gewirtz: close an SQLite db */
			
			return (sqlitecloseverb (hp1, v, bserror));
			} /* closefunc */

		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			return (false);
		} /* switch */
	} /* sqlitefunctionvalue */


boolean sqliteinitverbs (void) {
	
	/*	2006-03-15 gewirtz: created, cribbed from cryptinitverbs */
	
	return (loadfunctionprocessor (idsqliteverbs, &sqlitefunctionvalue));
	} /* sqliteinitverbs */

static void sqliteOpenError (const char *errmsg, bigstring bserror) {
	bigstring bserrmsg;

	copyctopstring (errmsg, bserrmsg); /* This creates a pstring at bserrmsg */
	getstringlist (langerrorlist, sqliteopenerror, bserror); 
	parsedialogstring (bserror, bserrmsg, nil, nil, nil, bserror);

	return;
	} /*sqliteOpenError*/

static void sqliteDatabaseError (const char *errmsg, bigstring bserror) {
	bigstring bserrmsg;

	copyctopstring (errmsg, bserrmsg); /* This creates a pstring at bserrmsg */
	getstringlist (langerrorlist, sqlitedberror, bserror); 
	parsedialogstring (bserror, bserrmsg, nil, nil, nil, bserror);

	return;
	} /*sqliteDatabaseError*/

static void sqliteScriptError (const char *errmsg, bigstring bserror) {
	bigstring bserrmsg;

	copyctopstring (errmsg, bserrmsg); /* This creates a pstring at bserrmsg */
	getstringlist (langerrorlist, notfunctionerror, bserror); 
	parsedialogstring (bserror, bserrmsg, nil, nil, nil, bserror);

	return;
	} /*sqliteScriptError*/


boolean sqliteopenverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	Handle dbfile;	/* path to database file		*/
	sqlite3 *dbid;	/* the SQLite database handle	*/
	int returnCode;	/* return code from SQLite call */

	/*	
		sqlite.open(dbfile)

		Action:  opens a SQLite database
		Params:  a file path to the database file
		Returns: a value that corresponds to the opened database ID
		Usage:   call in a try statement
		Notes:   do not pass the opened database ID across threads

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_open.
	*/

	flnextparamislast = true; /* makes sure Frontier throws an error if more than one param is passed */

	if (!gettextvalue (hparam1, 1, &dbfile)) /* get the param */
		return (false);

	pushcharhandle (0x00, dbfile); /* convert *dbfile to a string */

	/* Process the SQLite sequence */
	returnCode = sqlite3_open(*dbfile, &dbid);
	if(returnCode) {
		sqliteOpenError(sqlite3_errmsg(dbid), bserror); /* send database open error */
		sqlite3_close(dbid);
		return (false);
	}

	/* return the db address to the scripter */
	return (setheapvalue ((Handle) dbid, longvaluetype, vreturned)); 

} /* sqliteopenverb */

boolean sqlitecloseverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	sqlite3 *dbid;			/* the SQLite database handle */
	int returnCode;			/* return code from SQLite call */

	/*	
		sqlite.close(dbid)

		Action:  close an SQLite database
		Params:  a database id
		Returns: an SQLite result code
		Usage:   check result code for success or failure
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_close.
	*/

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) /* Get the long value, which becomes the db pointer */
		return (false);

	/* Process the SQLite sequence */
	returnCode = sqlite3_close(dbid); 

	return setlongvalue (returnCode, vreturned);

} /* sqlitecloseverb */

boolean sqlitecompilequeryverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	Handle query;				/* the SQL query passed to the SQLite engine */
	sqlite3 *dbid;				/* the SQLite database handle */
	sqlite3_stmt *queryid = 0;	/* the compiled query */
	int returnCode;				/* return code from SQLite call */

	/*	
		sqlite.compileQuery(dbid, query)

		Action:  compile an SQLite query
		Params:  a database id and a query string
		Returns: a query id
		Usage:   call in a try statement
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_prepare.

		Code that is a good example of prepare in use:
		http://souptonuts.sourceforge.net/code/eatblob.c.html
	*/

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) /* Get the long value, which becomes the db pointer */
		return (false);

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */
	
	if (!getreadonlytextvalue (hparam1, 2, &query)) /* get the query param */
		return (false);

	/* Process the SQLite sequence */
	returnCode = sqlite3_prepare(dbid, *query, gethandlesize(query), &queryid, 0);
	if(returnCode != SQLITE_OK || queryid == NULL) {
		sqliteDatabaseError(sqlite3_errmsg(dbid), bserror); /* send database error */
		return (false);
	}

	return (setlongvalue ((long) queryid, vreturned)); /* return the db address to the scripter */

} /* sqlitecompilequeryverb */

boolean sqliteclearqueryverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	sqlite3_stmt *queryid;	/* query id */
	int returnCode;			/* return code from SQLite call */

	/*	
		sqlite.clearQuery(queryid)

		Action:  clear an SQLite query
		Params:  a query id
		Returns: an SQLite result code
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_finalize.
	*/

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the queryid pointer */
		return (false);

	/* Process the SQLite sequence */
	returnCode = sqlite3_finalize(queryid);

	return setlongvalue (returnCode, vreturned);

} /* sqliteclearqueryverb */

boolean sqliteexecutequeryverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	sqlite3_stmt *queryid;	/* query id */
	int returnCode;			/* return code from SQLite call */

	/*	
		sqlite.executeQuery(queryid)

		Action:  perform the query, moving the query cursor to the next row of results
		Params:  a query id
		Returns: an SQLite result code
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_step.
	*/

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the pline pointer */
		return (false);

	/* Process the SQLite sequence */
	returnCode = sqlite3_step(queryid);

	return setlongvalue (returnCode, vreturned);

} /* sqliteexecutequeryverb */

boolean sqlitegetcolumntypeverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	long columnNumber;			/* column number */
	sqlite3_stmt *queryid;		/* query id */
	int returnCode;				/* return code from SQLite call */

	/*	
		sqlite.getColumnType(queryid, columnNumber)

		Action:  Retrieve the type of the specified column
		Params:  a query id and the number of the column (1 base)
		Returns: an SQLite result code
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_column_type.
	*/

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the queryid pointer */
		return (false);

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	/* Enter the verb, convert the param to null-terminated string */
	if (!getlongvalue (hparam1, 2, &columnNumber))
		return (false);

	/* Process the SQLite sequence */
	--columnNumber; /* Frontier uses base 1, but SQLite uses a base of 0 for its column numbering */
	returnCode = sqlite3_column_type(queryid, (int) columnNumber);

	switch(returnCode) {

		case SQLITE_INTEGER: {
			return setostypevalue (langgettypeid (longvaluetype), vreturned);
		}
		case SQLITE_FLOAT: {
			return setostypevalue (langgettypeid (doublevaluetype), vreturned);
		} 
		case SQLITE_TEXT: {
			return setostypevalue (langgettypeid (stringvaluetype), vreturned);
		} 
		case SQLITE_BLOB: {
			return setostypevalue (langgettypeid (binaryvaluetype), vreturned);
		} 
		case SQLITE_NULL: {
			return setostypevalue (langgettypeid (novaluetype), vreturned);
		} 
		default: {
			/* SQLite spec says sqlite3_column_type only returns the above 5 types. But,
			   just in case it lied, we'll return an error type to help Frontier avoid
			   getting a case of return value indigestion. */
			langerrormessage ("\x29""SQLite returned an undefined column type.");		
			return (false);
		}
	} /* switch */

} /* sqlitegetcolumntypeverb */

boolean sqlitegetcolumnnameverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	long columnNumber;			/* column number */
	Handle returnH;
	const char *column_name;	
	sqlite3_stmt *queryid;

	/*	
		sqlite.getColumnName(queryid, columnNumber)

		Action:  Retrieve the name of the specified column
		Params:  a query id and the number of the column (1 base)
		Returns: an SQLite result code
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_column_name.
	*/

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the pline pointer */
		return (false);

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	/* Enter the verb, convert the param to null-terminated string */
	if (!getlongvalue (hparam1, 2, &columnNumber))
		return (false);

	/* Process the SQLite sequence */
	--columnNumber; /* Frontier uses base 1, but SQLite uses a base of 0 for its column numbering */
	column_name = sqlite3_column_name(queryid, (int) columnNumber);

	/* Exit the verb, converting column_name back to Frontier handle */
	newfilledhandle ((ptrvoid) column_name, strlen (column_name), &returnH); 
	return (setheapvalue (returnH, stringvaluetype, vreturned)); 

} /* sqlitegetcolumnnameverb */

boolean sqlitegetcolumncountverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	sqlite3_stmt *queryid;
	int returnCode;			/* return code from SQLite call */

	/*	
		sqlite.getColumnCount(queryid)

		Action:  Retrieve number of columns returned in a query
		Params:  a query id
		Returns: the number of columns in the result set returned by the prepared SQL statement.
		Notes:   do not pass the opened database ID across threads.
				 returns 0 if pStmt is an SQL statement that does not return data (for example an UPDATE).

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_column_count.
	*/
	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the pline pointer */
		return (false);

	/* Process the SQLite sequence */
	returnCode = sqlite3_column_count(queryid);

	return setlongvalue (returnCode, vreturned);

} /* sqlitegetcolumncountverb */

boolean sqlitegetcolumnintverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	long columnNumber;
	sqlite3_stmt *queryid;
	long returnCode;			/* return code from SQLite call */

	/*	
		sqlite.getColumnInt(queryid, columnNumber)

		Action:  Retrieve the value of the specified integer-type column
		Params:  a query id and the number of the column (1 base)
		Returns: an SQLite result code
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_column_int.
	*/

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the pline pointer */
		return (false);

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	/* Enter the verb, convert the param to null-terminated string */
	if (!getlongvalue (hparam1, 2, &columnNumber))
		return (false);

	/* Process the SQLite sequence */
	--columnNumber; /* Frontier uses base 1, but SQLite uses a base of 0 for its column numbering */
	returnCode = sqlite3_column_int(queryid, (int) columnNumber);

	return (setlongvalue ((long) returnCode, vreturned));

} /* sqlitegetcolumnintverb */

boolean sqlitegetcolumndoubleverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	long columnNumber;
	sqlite3_stmt *queryid;
	double returnCode;			/* return code from SQLite call */

	/*	
		sqlite.getColumnDouble(queryid, columnNumber)

		Action:  Retrieve the value of the specified double-type column
		Params:  a query id and the number of the column (1 base)
		Returns: an SQLite result code
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_column_double.
	*/

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the pline pointer */
		return (false);

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	/* Enter the verb, convert the param to null-terminated string */
	if (!getlongvalue (hparam1, 2, &columnNumber))
		return (false);

	/* Process the SQLite sequence */
	--columnNumber; /* Frontier uses base 1, but SQLite uses a base of 0 for its column numbering */
	returnCode = sqlite3_column_double(queryid, (int) columnNumber);

	return (setlongvalue ((double) returnCode, vreturned));

} /* sqlitegetcolumndoubleverb */

boolean sqlitegetcolumntextverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	long columnNumber;
	sqlite3_stmt *queryid;
	Handle returnH;
	const unsigned char *column_text;

	/*	
		sqlite.getColumnText(queryid, columnNumber)

		Action:  Retrieve the value of the specified text-type column
		Params:  a query id and the number of the column (1 base)
		Returns: an SQLite result code
		Notes:   do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_column_text.
	*/

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the pline pointer */
		return (false);

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	/* Enter the verb, convert the param to null-terminated string */
	if (!getlongvalue (hparam1, 2, &columnNumber))
		return (false);

	/* Process the SQLite sequence */
	--columnNumber; /* Frontier uses base 1, but SQLite uses a base of 0 for its column numbering */
	column_text = sqlite3_column_text(queryid, (int) columnNumber);

	/* Exit the verb, converting column_name back to Frontier handle */
	newfilledhandle ((ptrvoid) column_text, strlen (column_text), &returnH); 
	return (setheapvalue (returnH, stringvaluetype, vreturned)); 

} /* sqlitegetcolumntextverb */

boolean sqlitegetcolumnverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	long columnNumber;			/* column number */
	sqlite3_stmt *queryid;		/* query id */
	int returnCode;				/* return code from SQLite call */
	Handle returnH;
	const unsigned char *column_text;

	/*	
		sqlite.getColumn(queryid, columnNumber)

		Action:  Retrieve the value of the specified column element
		Params:  a query id and the number of the column (1 base)
		Returns: an SQLite result code
		Notes:   do not pass the opened database ID across threads.
		         this implementation does not support blobs. If a blob column is requested, 0 is returned.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_column_type.
	*/

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) /* Get the long value, which becomes the queryid pointer */
		return (false);

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	/* Enter the verb, convert the param to null-terminated string */
	if (!getlongvalue (hparam1, 2, &columnNumber))
		return (false);

	/* Process the SQLite sequence */
	--columnNumber; /* Frontier uses base 1, but SQLite uses a base of 0 for its column numbering */
	returnCode = sqlite3_column_type(queryid, (int) columnNumber);

	switch(returnCode) {

		case SQLITE_INTEGER: {
			returnCode = sqlite3_column_int(queryid, (int) columnNumber);
			return (setlongvalue ((long) returnCode, vreturned));
		}
		case SQLITE_FLOAT: {
			returnCode = sqlite3_column_double(queryid, (int) columnNumber);
			return (setlongvalue ((double) returnCode, vreturned));
		} 
		case SQLITE_TEXT: {
			column_text = sqlite3_column_text(queryid, (int) columnNumber);

			/* Exit the verb, converting column_name back to Frontier handle */
			newfilledhandle ((ptrvoid) column_text, strlen (column_text), &returnH); 
			return (setheapvalue (returnH, stringvaluetype, vreturned)); 
		} 
		case SQLITE_BLOB: {
			return (setlongvalue ((long) 0, vreturned));
		} 
		case SQLITE_NULL: {
			return (setlongvalue ((long) 0, vreturned));
		} 
		default: {
			/* SQLite spec says sqlite3_column_type only returns the above 5 types. But,
			   just in case it lied, we'll return an error type to help Frontier avoid
			   getting a case of return value indigestion. */
			langerrormessage ("\x29""SQLite returned an undefined column type.");		
			return (false);
		}
	} /* switch */

} /* sqlitegetcolumntext */

boolean sqlitegeterrormessageverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	const char *sqliteError;	/* pointer to the error message */
	sqlite3 *db;				/* the SQLite database handle	*/
	Handle returnH = nil;		/* The handle being returned back to Frontier */

	/*	
		Frontier verb: sqlite.getErrorMessage(dbfile)

		Returns the error message for the most recent SQLite API call. 
		Takes a database ID.
		Returns a string containing the error message.
		NOTE: do not pass the opened database ID across threads.

		SQLite docs: http://www.sqlite.org/capi3ref.html#sqlite3_errmsg.
	*/

	flnextparamislast = true; /* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &db)) /* Get the long value, which becomes the db pointer */
		return (false);

	/* Process the SQLite sequence */
	sqliteError = sqlite3_errmsg(db);

	/* Exit the verb, converting errMsg back to Frontier handle */
	newfilledhandle ((ptrvoid) sqliteError, strlen (sqliteError), &returnH); 
	return (setheapvalue (returnH, stringvaluetype, vreturned)); 

} /* sqlitegeterrormessageverb */