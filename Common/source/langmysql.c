
/*	$Id: langmysql.c,v 2007/04/08 18:32:00 davidgewirtz Exp $    */

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
#include "oplist.h"
#include "langsystem7.h"

// #ifdef WIN95VERSION

//	#include "my_global.h"

// #endif // WIN95VERSION

#include "mysql.h"
#include "langmysql.h"


/*

Building MySQL into Frontier

See the Frontier Lab Notebook article:

http://manila.zatz.com/frontierkernel/stories/storyReader$33

-- DG

*/


typedef enum tymysqlverbtoken { /* verbs that are processed by langmysql.c */

	initfunc,
	endfunc,
	connectfunc,
	compilequeryfunc,
	clearqueryfunc,
	getrowfunc,
	geterrornumberfunc,
	geterrormessagefunc,
	getclientinfofunc,
	getclientversionfunc, 
	gethostinfofunc,
	getserverversionfunc, 
	getprotocolinfofunc,
	getserverinfofunc,
	getqueryinfofunc,
	getaffectedrowcountfunc,
	getselectedrowcountfunc,
	getcolumncountfunc,
	getserverstatusfunc,
	getquerywarningcountfunc,
	pingserverfunc,
	seekrowfunc,
	selectdatabasefunc,
	getsqlstatefunc,
	escapestringfunc,
	isthreadsafefunc,
	closefunc,
	ctmysqlverbs
	} tymysqlverbtoken;


static boolean mysqlfunctionvalue ( short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror ) {

	//
	// 2007-04-11 gewirtz: created
	//
	
	hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	
	setbooleanvalue ( false, v ); // by default, sqlite functions return false
	
	switch ( token ) {
		
		case initfunc: // initialize MySQL
		
			return ( mysqlinitverb ( hp1, v, bserror ) );
			
		case endfunc:
		
			return ( mysqlendverb ( hp1, v, bserror ) );
			
		case connectfunc:
		
			return ( mysqlconnectverb ( hp1, v, bserror ) );
			
		case compilequeryfunc: // prepare a MySQL query
		
			return ( mysqlcompilequeryverb ( hp1, v, bserror ) );		
			
		case clearqueryfunc: // clear a MySQL query from memory
		
			return ( mysqlclearqueryverb ( hp1, v, bserror ) );		
			
		case getrowfunc: // get a full row from a query
		
			return ( mysqlgetrowverb ( hp1, v, bserror ) );		
			
 		case geterrornumberfunc: // get the error number from the most recent call
		
			return ( mysqlgeterrornumberverb ( hp1, v, bserror ) );		
			
		case geterrormessagefunc: // get the error message from the most recent call
		
			return ( mysqlgeterrormessageverb ( hp1, v, bserror ) );		
			
		case getclientinfofunc: // get the MySQL client info
		
			return ( mysqlgetclientinfoverb ( hp1, v, bserror ) );		
			
		case getclientversionfunc: // get the MySQL client version
		
			return ( mysqlgetclientversionverb ( hp1, v, bserror ) );		
			
		case gethostinfofunc: // get the MySQL host info
		
			return ( mysqlgethostinfoverb ( hp1, v, bserror ) );		
			
		case getserverversionfunc: // get the MySQL server version
		
			return ( mysqlgetserverversionverb ( hp1, v, bserror ) );		
			
		case getprotocolinfofunc: // get the MySQL protocol info
		
			return ( mysqlgetprotocolinfoverb ( hp1, v, bserror ) );		
			
		case getserverinfofunc: // get the MySQL server info
		
			return ( mysqlgetserverinfoverb ( hp1, v, bserror ) );		
			
		case getqueryinfofunc: // get the results messages from the last query
		
			return ( mysqlgetqueryinfoverb ( hp1, v, bserror ) );		
			
		case getaffectedrowcountfunc: // get the rows added, deleted, updated
		
			return ( mysqlgetaffectedrowcountverb ( hp1, v, bserror ) );		
			
		case getselectedrowcountfunc: // get the rows selected in last query
		
			return ( mysqlgetselectedrowcountverb ( hp1, v, bserror ) );		
			
		case getcolumncountfunc: // get the columns returned in last query
		
			return ( mysqlgetcolumncountverb ( hp1, v, bserror ) );		
			
		case getserverstatusfunc: // get the MySQL server status
		
			return ( mysqlgetserverstatusverb ( hp1, v, bserror ) );		
			
		case getquerywarningcountfunc: // get the number of warnings returned by the last query
		
			return ( mysqlgetquerywarningcountverb ( hp1, v, bserror ) );		
			
		case pingserverfunc: // check to see if there's still a connection with the server
		
			return ( mysqlpingserververb ( hp1, v, bserror ) );		
			
		case seekrowfunc: // move to specific row in query result set
		
			return ( mysqlseekrowverb ( hp1, v, bserror ) );		
			
		case selectdatabasefunc: // choose a MySQL database as current
		
			return ( mysqlselectdatabaseverb ( hp1, v, bserror ) );		
			
 		case getsqlstatefunc:
		
			return ( mysqlgetsqlstateverb ( hp1, v, bserror ) );		
			
 		case escapestringfunc:
		
			return ( mysqlescapestringverb ( hp1, v, bserror ) );		
			
 		case isthreadsafefunc:
		
			return ( mysqlisthreadsafeverb ( hp1, v, bserror ) );		
			
		case closefunc: // close a MySQL db
		
			return ( mysqlcloseverb ( hp1, v, bserror ) );
			
		default:
		
			getstringlist ( langerrorlist, unimplementedverberror, bserror );
			
			return ( false );
			
		} // switch
		
	} // mysqlfunctionvalue


boolean mysqlinitverbs (void) {
	
	return (loadfunctionprocessor (idmysqlverbs, &mysqlfunctionvalue));
	
	} /* mysqlinitverbs */


boolean mysqlinitverb ( hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror ) {

	int resultCode;
	static char *server_args[] = {
		"this_program"				// this string is not used
		// "--datadir=.",			// these arguments aren't, either, since we're not using
		// "--key_buffer_size=32M"	// the embedded server
	};

	/*
		mysql.init()
 
		Action:		initializes the mySQL library.
		Params:		none
		Returns:	0 if successful, otherwise an error
		Usage:		call in a try statement, call before any other MySQL call

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-library-init.html
	*/
	
	// Initialize the MySQL object
	resultCode = mysql_library_init(-1, server_args, NULL); // -1 means don't start embedded server
	if (resultCode != 0) {
		langerrormessage ("\x1B""MySQL could not initialize.");	
		return (false);
	}

	return (setlongvalue ((long) resultCode, vreturned));
	
} // mysqlinitverb


boolean mysqlendverb ( hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror ) {

	/*
		mysql.end()

		Action:		closes out the MySQL library
		Params:		none
		Returns:	true
		Usage:		call in a try statement, call after any other MySQL call

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-library-end.html
	*/
	
	// Close out the library
	mysql_library_end();

	return(setbooleanvalue (true, vreturned));
	
	} // mysqlendverb


boolean mysqlconnectverb ( hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror ) {

	//
	// 2007-05-29 gewirtz: created
	//
	
	/*
		mysql.connect (host, user, password, database, port)

		Action:  connects to a MySQL server and database, initializing the mySQL connection object
		Params:  the hostname, user id, password, existing database name, and port
		Returns: a MySQL object handle or an error
		Usage:   call in a try statement

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-init.html
		            http://dev.mysql.com/doc/refman/5.1/en/mysql-real-connect.html

		If problems are encountered connecting to a Windows-based MySQL server, see:
		http://dev.mysql.com/doc/refman/5.1/en/can-not-connect-to-server.html#can-not-connect-to-server-on-windows
	*/
	
	MYSQL *dbid;	// the MySQL object handle
	Handle host;	// hostname of the MySQL server
	Handle user;	// valid username on the MySQL server
	Handle passwd; // password for user on MySQL server
	Handle dbname; // existing database on MySQL server
	long port;	// TCP/IP port for the connection

	if ( ! gettextvalue ( hparam1, 1, &host ) ) // get the host param
		return ( false );

	if ( ! gettextvalue ( hparam1, 2, &user ) ) // get the user param
		return ( false );

	if ( ! gettextvalue ( hparam1, 3, &passwd ) ) // get the passwd param
		return ( false );

	if ( ! gettextvalue ( hparam1, 4, &dbname ) ) // get the dbname param
		return ( false );

	flnextparamislast = true; // makes sure Frontier throws an error if more than one param is passed

	if ( ! getlongvalue ( hparam1, 5, &port ) ) // get the port param
		return ( false );

	if ( ! pushcharhandle ( 0x00, host ) ) // Null terminate strings
		return ( false );
		
	if ( ! pushcharhandle ( 0x00, user ) )
		return ( false );
		
	if ( ! pushcharhandle ( 0x00, passwd ) )
		return ( false );
		
	if ( ! pushcharhandle ( 0x00, dbname ) )
		return ( false );
		
	dbid = mysql_init ( ( MYSQL* ) 0 ); // Initialize the MySQL object
	
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x2D" "MySQL could not allocate a connection object." );
		
		return ( false );
		
		}
		
	dbid = mysql_real_connect ( dbid, *host, *user, *passwd, *dbname, ( unsigned int ) port, NULL, 0 ); // Attempt the connection
	
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x22" "Could not connect to MySQL server." );
		
		return ( false );
		
		}
		
	return ( setlongvalue ( ( long ) dbid, vreturned ) );
	
	} // mysqlconnectverb


boolean mysqlcloseverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.close(dbid)

		Action:  close a MySQL session
		Params:  a database connection id, the MySQL object
		Returns: nothing
		Usage:   there doesn't appear to be a way to check for success or failure

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-close.html
	*/

	MYSQL *dbid;	// the MySQL object handle
	int returnCode;	// return code from close call

	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the db pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL sequence
	mysql_close(dbid); 
	returnCode = 0; // placeholder for debugging. mysql_close doesn't return anything.

	return setlongvalue (returnCode, vreturned);

	} /* mysqlcloseverb */


boolean mysqlcompilequeryverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.compileQuery(dbid, query)

		Action:  compile a MySQL query
		Params:  a database connection id and a query string
		Returns: a query id
		Usage:   call in a try statement
		Notes:   there are a lot of possible approaches to queries in MySQL. We're using the
		         mysql_store_result version, which is more flexible, but which could have 
				 some issues with large dataset returns.

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-query.html
		            http://dev.mysql.com/doc/refman/5.1/en/mysql-store-result.html
	*/
	
	Handle query;				// the SQL query passed to the MySQL server
	MYSQL *dbid;				// the MySQL object handle
	MYSQL_RES *queryid;			// the returned MySQL query id
	int returnCode;				// return code from MySQL query
	
	if (!getlongvalue (hparam1, 1, (long *) &dbid)) /* Get the long value, which becomes the db pointer */
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */
	
	if (!gettextvalue (hparam1, 2, &query)) // get the query param
		return (false);

	// Null terminate strings
	if (!pushcharhandle (0x00, query))
		return (false);

	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the initial query sequence
	returnCode = mysql_query(dbid, *query);
	if(returnCode != 0) {
		langerrormessage ("\x21""Could not initialize MySQL query.");	
		return (false);
	}

	queryid = mysql_store_result(dbid);
	if(mysql_errno(dbid) != 0) {
		langerrormessage ("\x21""Could not initialize MySQL query.");	
		return (false);
	}

	if (mysql_field_count(dbid) == 0)
		return (setlongvalue ((long) 0, vreturned)); // return 0 to the scripter
	else
		return (setlongvalue ((long) queryid, vreturned)); // return the db address to the scripter
		
	} /* mysqlcompilequeryverb */


boolean mysqlclearqueryverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	MYSQL_RES *queryid;			// the returned MySQL query id
	int returnCode;

	/*	
		mysql.clearQuery(queryid)

		Action:  clear a MySQL query
		Params:  a query id
		Returns: 0 (MySQL doesn't return anything)

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-free-result.html
	*/

	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) // Get the long value, which becomes the queryid pointer
		return (false);

	// Process the MySQL sequence
	mysql_free_result(queryid);

	returnCode = 0; // placeholder. mysql_free_result doesn't return anything.
	return setlongvalue (returnCode, vreturned);

	} /* mysqlclearqueryverb */


boolean mysqlgetrowverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-10 creedon: convert some numeric MYSQL_TYPE to numbers, see
	//				   "Formats supported and returned as number"
	//
	// 2007-06-04 gewirtz: fix for returning TEXT type variant of BLOB
	//
	// 2007-05-31 creedon, asseily: fix for crash when column_text is NULL, return nil
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*
	
	mysql.getRow(queryid)
	
	Action:  return the next row from the query result set
	Params:  a query id
	Returns: 0 if no more rows, or a Frontier list containing data
	
	MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-fetch-row.html
			  http://dev.mysql.com/doc/refman/5.1/en/mysql-num-fields.html
			  http://dev.mysql.com/doc/refman/5.1/en/mysql-field-seek.html
			  http://dev.mysql.com/doc/refman/5.1/en/mysql-fetch-field.html
	
	Notes:
	
	MySQL might also return a 0 if there's an error. It's important to check
	for an error if a zero value is returned to differentiate between end of
	dataset and last row.
	
	The MySQL fetch_row function is interesting, in that it looks like
	each field returned is returned as a string. This is great for Frontier,
	since Frontier can't handle some of MySQL's bigger number types (like double),
	but it can handle strings. So everything that's supported is returned as
	a string to Frontier. The Frontier programmer can then do whatever 
	coercing is necessary.
	
	The only formats we explicitly don't support are:
	
		MYSQL_TYPE_BIT
		MYSQL_TYPE_BLOB (although we do return TEXT, which is a BLOB variant)
		MYSQL_TYPE_ENUM
		MYSQL_TYPE_GEOMETRY
		MYSQL_TYPE_NULL
		MYSQL_TYPE_SET
		
	Formats supported and returned as string:
	
		MYSQL_TYPE_DATE
		MYSQL_TYPE_DATETIME
		MYSQL_TYPE_DECIMAL
		MYSQL_TYPE_DOUBLE
		MYSQL_TYPE_FLOAT
		MYSQL_TYPE_NEWDECIMAL
		MYSQL_TYPE_STRING
		MYSQL_TYPE_TIME
		MYSQL_TYPE_TIMESTAMP
		MYSQL_TYPE_VAR_STRING
		MYSQL_TYPE_YEAR
		
	Formats supported and returned as number:
	
		MYSQL_TYPE_INT24
		MYSQL_TYPE_LONG
		MYSQL_TYPE_LONGLONG
		MYSQL_TYPE_SHORT
		MYSQL_TYPE_TINY
		
	More info: http://dev.mysql.com/doc/refman/5.1/en/c-api-datatypes.html
	
	*/
	
	unsigned int fieldNumber; // field number
	unsigned int fieldCount;	 // number of fields
	MYSQL_RES *queryid;		 // query id
	MYSQL_ROW row;
	MYSQL_FIELD *field;
	hdllistrecord hlist;
	Handle h = NULL;
	const unsigned char *column_text;
	tyvaluerecord val;
	
	flnextparamislast = true; // makes sure Frontier throws an error if more than one param is passed
	
	if (!getlongvalue (hparam1, 1, (long *) &queryid)) // Get the long value, which becomes the queryid pointer
		return (false);
		
	row = mysql_fetch_row(queryid);
	
	if (row == NULL) {
		// return 0, meaning no more rows
		// **** NOTE THIS COULD ALSO MEAN AN ERROR, MIGHT BE GOOD TO CHECK
		return (setlongvalue ((long) 0, vreturned));
		
		}
		
	fieldCount = mysql_num_fields(queryid);
	
	if (fieldCount == 0) {
	
		langerrormessage ("\x2D""MySQL.getRow requires a minimum of one field.");
			
		return (false);
		
		}
	
	mysql_field_seek(queryid, 0); // restart gathering field data from the first field
	
	if (!opnewlist (&hlist, false)) // fail out if we can't create the list
		return (false);
		
	for (fieldNumber=0; fieldNumber<fieldCount; ++fieldNumber) {
	
		field = mysql_fetch_field(queryid);
		
		switch ( field->type ) {
		
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_LONGLONG:
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_NEWDECIMAL:
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
			case MYSQL_TYPE_TIMESTAMP:
			case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_TIME:
			case MYSQL_TYPE_DATETIME:
			case MYSQL_TYPE_YEAR:
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_BLOB: {
			
				if (field->type == MYSQL_TYPE_BLOB) {
					// This is a special case, to see if the type is really TEXT
					if (field->charsetnr == 63) {
						// binary data, see http://dev.mysql.com/doc/refman/5.0/en/c-api-datatypes.html
						if (!langpushlistlong (hlist, (long) 0))
							goto error;
							
						return (setheapvalue ((Handle) hlist, listvaluetype, vreturned));
					}
				}
				
				column_text = row[fieldNumber];
				
				if ( column_text == NULL )
				
					setnilvalue ( &val );
					
				else {

					// Exit the verb, converting column_name back to Frontier handle
					
					if ( ! newfilledhandle ( ( ptrvoid ) column_text, strlen ( column_text ), &h ) )
						return ( false ); // Allocation failed
						
					if ( ! setheapvalue ( h, stringvaluetype, &val ) ) // convert handle to value
						goto error;
						
					bundle { // coerce certain numeric MYSQL_TYPE_ to kernel number types
					
						tyvaluetype type;
						
						switch ( field -> type ) {
						
							case MYSQL_TYPE_LONG:
							case MYSQL_TYPE_LONGLONG:
							
								type = doublevaluetype;
								
								break;
								
							case MYSQL_TYPE_INT24:
							case MYSQL_TYPE_SHORT:
							
								type = longvaluetype;
								
								break;
								
							case MYSQL_TYPE_TINY:
							
								type = intvaluetype;
								
								break;
								
							default:
							
								type = novaluetype;
								
							} // switch
						
						if ( type != novaluetype )
							if ( ! coercevalue ( &val, type ) )
								goto error;
								
						} // bundle
						
					} // if
					
				if (!langpushlistval (hlist, NULL, &val))
					goto error;
					
				break;
				
				}
				
			default: {
			
				if (!langpushlistlong (hlist, (long) 0))
					goto error;
				}
				
			} // switch
		
		} // for

	return (setheapvalue ((Handle) hlist, listvaluetype, vreturned));

	error:
	
		opdisposelist ( hlist );
		
		return ( false );

	} // mysqlgetrow


boolean mysqlgeterrornumberverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getErrorNumber(dbid)

		Action:  get the most recent MySQL error code
		Params:  a database connection id
		Returns: the error number, 0 if no error

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-errno.html
					http://dev.mysql.com/doc/refman/5.1/en/errors-handling.html
	*/

	MYSQL *dbid;				// the MySQL object handle
	unsigned int returnCode;		// return code from MySQL call

	flnextparamislast = true;	/* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) /* Get the long value, which becomes the pline pointer */
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL call
	returnCode = mysql_errno(dbid);

	return (setlongvalue ((long) returnCode, vreturned));

	} /* mysqlgeterrornumberverb */


boolean mysqlgeterrormessageverb ( hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror ) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	//				   formatting tweaks
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getErrorMessage(dbid)

		Action:  get the most recent MySQL error code
		Params:  a database connection id
		Returns: a string containing the error message

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-error.html
				  http://dev.mysql.com/doc/refman/5.1/en/errors-handling.html
	*/
	
	MYSQL *dbid;		    // the MySQL object handle
	const char *mysqlError; // pointer to the error message
	Handle h = NULL;	    // The handle being returned back to Frontier
	
	flnextparamislast = true; // makes sure Frontier throws an error if more than one param is passed
	
	if ( ! getlongvalue ( hparam1, 1, ( long * ) &dbid ) ) // Get the long value, which becomes the db pointer
		return ( false );
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	if ( mysql_ping ( dbid ) != 0 ) { // Check that server's still alive
	
		langerrormessage ( "\x20" "Lost connection to MySQL server." );
		
		return ( false );
		
		}
		
	mysqlError = mysql_error ( dbid ); // Process the MySQL sequence
	
	// Exit the verb, converting errMsg back to Frontier handle
	
	if ( ! newfilledhandle ( ( ptrvoid ) mysqlError, strlen ( mysqlError ), &h ) )
		return ( false ); // Allocation failed
		
	return ( setheapvalue ( h, stringvaluetype, vreturned ) ); 
	
	} // mysqlgeterrormessageverb


boolean mysqlgetclientinfoverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	const char *mysqlMsg;		// pointer to the message
	Handle h = nil;		// The handle being returned back to Frontier

	/*	
		mysql.getClientInfo()

		Action:  get the version of the MySQL client
		Params:  none
		Returns: a string containing the version of the MySQL client in human-recognizable form

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-get-client-info.html
	*/

	// Process the MySQL sequence
	mysqlMsg = mysql_get_client_info();

	// Exit the verb, converting errMsg back to Frontier handle
	if(!newfilledhandle ((ptrvoid) mysqlMsg, strlen (mysqlMsg), &h))
		return false; // Allocation failed

	return (setheapvalue (h, stringvaluetype, vreturned)); 

	} /* mysqlgetclientinfoverb */


boolean mysqlgetclientversionverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	unsigned long returnCode;	// The number being returned back to Frontier


	/*	
		mysql.getClientVersion()

		Action:  get the version of the MySQL client as an integer
		Params:  none
		Returns: an integer containing the version of the MySQL client

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-get-client-version.html

		Notes:

		Returns an integer that represents the client library version.
		The value has the format XYYZZ where X is the major version, YY  is the release
		level, and ZZ is the version number within the release level.
		For example, a value of 40102 represents a client library version of 4.1.2.
	*/

	// Process the MySQL call
	returnCode = mysql_get_client_version();

	return (setlongvalue ((long) returnCode, vreturned));

	} /* mysqlgetclientversionverb */


boolean mysqlgethostinfoverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getHostInfo(dbid)

		Action:  get information about connection to the host
		Params:  a database connection id
		Returns: a string containing information about the host in human-recognizable form

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-get-host-info.html
	*/

	MYSQL *dbid;				// the MySQL object handle
	const char *mysqlMsg;		// pointer to the message
	Handle h = nil;		// The handle being returned back to Frontier

	flnextparamislast = true; /* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) /* Get the long value, which becomes the db pointer */
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL call
	mysqlMsg = mysql_get_host_info(dbid);

	// Exit the verb, converting errMsg back to Frontier handle
	if(!newfilledhandle ((ptrvoid) mysqlMsg, strlen (mysqlMsg), &h))
		return false; // Allocation failed

	return (setheapvalue (h, stringvaluetype, vreturned)); 

	} /* mysqlgethostinfoverb */


boolean mysqlgetserverversionverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*
		mysql.getServerVersion(dbid)

		Action:  get information about connection to the MySQL server
		Params:  a database connection id
		Returns: an integer containing the version of the MySQL server

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-get-server-version.html

		Notes:

		Return format is a number that represents the MySQL server version in
		this format:

			major_version*10000 + minor_version *100 + sub_version

		For example, 5.1.5 is returned as 50105. This function is useful in client
		programs for quickly determining whether some version-specific
		server capability exists. 
	*/
	
	MYSQL *dbid;				// the MySQL object handle
	unsigned long returnCode;	// the return code
	
	flnextparamislast = true; /* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) /* Get the long value, which becomes the db pointer */
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL call
	returnCode = mysql_get_server_version(dbid);

	// Exit the verb, converting errMsg back to Frontier handle
	return (setlongvalue ((long) returnCode, vreturned));

	} /* mysqlgetserverversionverb */


boolean mysqlgetprotocolinfoverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*
		mysql.getProtocolInfo(dbid)

		Action:  get information about the MySQL protocol version
		Params:  a database connection id
		Returns: an integer containing the protocol version used by current connection

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-get-proto-info.html

		Notes:

		Returns an integer. This call is unlikely to be used, but provided for 
		completeness and because it was easy to code. To better understand the
		MySQL protocol, read:

		http://www.redferni.uklinux.net/mysql/MySQL-Protocol.html
	*/
	
	MYSQL *dbid;			// the MySQL object handle
	unsigned int returnCode;	// the return code
	
	flnextparamislast = true; // makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the db pointer
		return (false);

	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL sequence
	returnCode = mysql_get_proto_info(dbid);

	// Exit the verb, converting errMsg back to Frontier handle
	return (setlongvalue ((long) returnCode, vreturned));

	} /* mysqlgetprotocolinfoverb */


boolean mysqlgetserverinfoverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getServerInfo(dbid)

		Action:  get information about the MySQL server version
		Params:  a database connection id
		Returns: a string containing information about the server version
			     in human-recognizable form

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-get-server-info.html

	*/
	
	MYSQL *dbid;				// the MySQL object handle
	const char *mysqlMsg;		// pointer to the message
	Handle h = nil;		// The handle being returned back to Frontier
	
	flnextparamislast = true; // makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the db pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL call
	mysqlMsg = mysql_get_server_info(dbid);

	// Exit the verb, converting errMsg back to Frontier handle
	if(!newfilledhandle ((ptrvoid) mysqlMsg, strlen (mysqlMsg), &h))
		return false; // Allocation failed

	return (setheapvalue (h, stringvaluetype, vreturned)); 

	} /* mysqlgetserverinfoverb */


boolean mysqlgetqueryinfoverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getQueryInfo(dbid)

		Action:  get information about the most recently executed MySQL query
		Params:  a database connection id
		Returns: a string containing information about the query
			     in human-recognizable form

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-info.html

		Note:

		This function returns result data only for certain queries of the form:

		INSERT INTO ... SELECT ...
		INSERT INTO ... VALUES (...),(...),(...)...
		LOAD DATA INFILE ...
		ALTER TABLE
		UPDATE 

		For all other, the routine returns ""

	*/
	
	MYSQL *dbid;				// the MySQL object handle
	const char *mysqlMsg;		// pointer to the message
	Handle h = nil;		// The handle being returned back to Frontier
	
	flnextparamislast = true; // makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the db pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL sequence
	mysqlMsg = mysql_info(dbid);

	if (mysqlMsg == NULL) {
		// Exit the verb, converting errMsg back to Frontier handle 
		if(!newfilledhandle ((ptrvoid) "", 0, &h))
			return false; // Allocation failed
	} else {
	// Exit the verb, converting errMsg back to Frontier handle
		if(!newfilledhandle ((ptrvoid) mysqlMsg, strlen (mysqlMsg), &h))
			return false; // Allocation failed
	}

	return (setheapvalue (h, stringvaluetype, vreturned)); 

	} /* mysqlgetqueryinfoverb */


boolean mysqlgetaffectedrowcountverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getAffectedRowCount(dbid)

		Action:  gets a numeric value corresponding to the number of rows changed/deleted
		Params:  a database connection id
		Returns: double-value (the number of rows could be very high)

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-affected-rows.html

		Note:

		Returns the number of rows changed (for UPDATE), deleted (for DELETE), or inserted
		(for INSERT). For SELECT  statements, works like mysql.getSelectedRowCount()

	*/

	MYSQL *dbid;				// the MySQL object handle
	my_ulonglong rowCount;		// this datastructure is too big for Frontier

	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL call
	rowCount = mysql_affected_rows(dbid);

	return (setdoublevalue ((double) rowCount, vreturned));

	} /* mysqlgetaffectedrowcountverb */


boolean mysqlgetselectedrowcountverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	MYSQL_RES *queryid;			// query id
	my_ulonglong rowCount;		// this datastructure is too big for Frontier

	/*	
		mysql.getSelectedRowCount(queryid)

		Action:  gets the number of rows in the return set from a query
		Params:  a database query id
		Returns: double-value (the number of rows could be very high)

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-num-rows.html
	*/

	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) // Get the long value, which becomes the pointer
		return (false);

	// Process the MySQL call
	rowCount = mysql_num_rows(queryid);

	return (setdoublevalue ((double) rowCount, vreturned));

	} /* mysqlgetselectedrowcountverb */


boolean mysqlgetcolumncountverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getColumnCount(dbid)

		Action:  gets the number of columns from the most recent query
		Params:  a database connection id
		Returns: number corresponding to the column count

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-field-count.html
	*/

	MYSQL *dbid;				// the MySQL object handle
	unsigned int returnCode;		// return code from MySQL call

	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the pline pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL call
	returnCode = mysql_field_count(dbid);

	return (setlongvalue ((long) returnCode, vreturned));

	} /* mysqlgetcolumncountverb */


boolean mysqlgetserverstatusverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getServerStatus(dbid)

		Action:  gets server status 
		Params:  a database connection id
		Returns: string containing detailed server status info (uptime, threads, etc.)

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-stat.html

		Note:

		Status will fail/crash if mysql.connect() hasn't previously succeeded. It'll
		be messy, so check your result codes.
	*/
	
	MYSQL *dbid;				// the MySQL object handle
	const char *mysqlStatus;	// pointer to the status message
	Handle h = nil;		// The handle being returned back to Frontier
	
	flnextparamislast = true; /* makes sure Frontier throws an error if more than one param is passed */

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) /* Get the long value, which becomes the db pointer */
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL sequence
	mysqlStatus = mysql_stat(dbid);

	if (mysqlStatus == NULL) {
		return (setlongvalue ((long) 0, vreturned));
		}
	else {
		// Exit the verb, converting errMsg back to Frontier handle
		if(!newfilledhandle ((ptrvoid) mysqlStatus, strlen (mysqlStatus), &h))
			return false; // Allocation failed
		}

	return (setheapvalue (h, stringvaluetype, vreturned)); 

	} /* mysqlgetserverstatusverb */


boolean mysqlgetquerywarningcountverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.getQueryWarningCount(dbid)

		Action:  gets warning count from a query
		Params:  a database connection id
		Returns: the number of warnings generated during execution of the previous SQL statement.

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-warning-count.html

		Note:

		Obviously, you should check this when performing queries.
	*/

	MYSQL *dbid;				// the MySQL object handle
	unsigned int returnCode;	// return code from MySQL call

	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the pline pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL call
	returnCode = mysql_warning_count(dbid);

	return (setlongvalue ((long) returnCode, vreturned));

	} /* mysqlgetquerywarningcountverb */


boolean mysqlpingserververb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*	
		mysql.pingServer(dbid)

		Action:  checks to make sure the connection to the server is up
		Params:  a database connection id
		Returns: 0, if the connection is alive

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-ping.html

		Note:

		mysql.pingServer() attempts to reconnect if the connection went down.
	*/

	MYSQL *dbid;				// the MySQL object handle
	unsigned int returnCode;	// return code from MySQL call

	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed
	
	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the pline pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Process the MySQL call
	returnCode = mysql_ping(dbid);

	return (setlongvalue ((long) returnCode, vreturned));

	} /* mysqlpingserververb */


boolean mysqlseekrowverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	MYSQL_RES *queryid;			// the returned MySQL query id
	tyvaluerecord v1;
	double row;					// row to seek

	/*	
		mysql.seekRow(queryid, row)

		Action:  seeks to the specified row number in the data set
		Params:  a database query id, the row
		Returns: 0 (MySQL doesn't return anything, so there's no meaning here)

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-data-seek.html

		Note:

		MySQL uses base-0 numbering while Frontier uses base-1. If you want MySQL
		to find the first row, pass 1 from Frontier. This routine will decrement
		the value, and see row 0 via the MySQL call.

		Because there could be a LOT of rows in a results dataset, the value
		passed from Frontier can be a double (although conversion is done from
		regular integers and longs).
	*/

	if (!getlongvalue (hparam1, 1, (long *) &queryid)) // Get the long value, which becomes the query pointer
		return (false);

	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed
	
	if (!getdoubleparam (hparam1, 2, &v1)) // get the seek row
		return (false);

	row = **v1.data.doublevalue;
	if (row > 0) {
		--row; // adjust for Frontier's base of 1
	}
	else {
		langerrormessage ("\x2F""MySQL.seekRow requires a value of 1 or greater.");	
		return (false);
	}

	mysql_data_seek(queryid, (my_ulonglong) row);

	return (setlongvalue ((long) 0, vreturned));

	} /* mysqlseekrowverb */


boolean mysqlselectdatabaseverb ( hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror ) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*
		mysql.selectDatabase(dbid, database)

		Action:  changes the currently active database
		Params:  a database connection id, an existing database name
		Returns: 0 if successful, throws an error otherwise

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-select-db.html
	*/
	
	MYSQL *dbid;	// the MySQL object handle
	int resultCode;	// result code
	Handle dbname;	// existing database on MySQL server

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the db pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	flnextparamislast = true;	// makes sure Frontier throws an error if more than one param is passed

	if (!gettextvalue (hparam1, 2, &dbname)) // get the dbname param
		return (false);

	// Null terminate strings
	if (!pushcharhandle (0x00, dbname))
		return (false);

	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL request
	resultCode = mysql_select_db(dbid, *dbname);
	if (resultCode != 0) {
		langerrormessage ("\x24""Could not connect to MySQL database.");	
		return (false);
	}
	
	return (setlongvalue ((long) 0, vreturned));
	
	} /* mysqlselectdatabaseverb */


boolean mysqlgetsqlstateverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*
		mysql.getSQLState(dbid)

		Action:  a null-terminated string containing the SQLSTATE error code
		         for the most recently executed SQL statement.
		Params:  a database connection id
		Returns: a string corresponding to the defined SQLSTATE codes

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-sqlstate.html

		Notes: SQLSTATE is part of the SQL standard. These codes are likely to be used 
		       only by purists, but it's included here for completeness. Aren't we all
			   so lucky?
	*/

	MYSQL *dbid;				// the MySQL object handle
	const char *mysqlStatus;	// pointer to the status message
	Handle h = nil;		// The handle being returned back to Frontier

	flnextparamislast = true; // makes sure Frontier throws an error if more than one param is passed

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the db pointer
		return (false);
		
	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	// Check that server's still alive
	if (mysql_ping(dbid) != 0) {
		langerrormessage ("\x20""Lost connection to MySQL server.");	
		return (false);
	}

	// Process the MySQL sequence
	mysqlStatus = mysql_sqlstate(dbid);

	// Exit the verb, converting errMsg back to Frontier handle
	if(!newfilledhandle ((ptrvoid) mysqlStatus, strlen (mysqlStatus), &h))
		return false; // Allocation failed

	return (setheapvalue (h, stringvaluetype, vreturned)); 

	} /* mysqlgetsqlstateverb */


boolean mysqlescapestringverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	//
	// 2007-07-02 creedon: if dbid is NULL script error
	//
	// 2007-05-29 gewirtz: created
	//
	
	/*
		mysql.escapeString(dbid, string)

		Action:  creates a legal SQL string that you can use in an SQL statement
		Params:  a database connection id, a string containing SQL
		Returns: a cleaned up string

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-real-escape-string.html

		Notes: dbid is required because the string returned is the character set in
		use by the server
	*/

	MYSQL *dbid;				// the MySQL object handle
	Handle fromstring;
	Handle tostring;
	unsigned long size;
	unsigned long encodedSize;
	Handle h = nil;		// The handle being returned back to Frontier

	if (!getlongvalue (hparam1, 1, (long *) &dbid)) // Get the long value, which becomes the pline pointer
		return (false);

	if ( dbid == NULL ) {
	
		langerrormessage ( "\x25" "Invalid MySQL database connection ID." );
		
		return ( false );
		
		}
		
	flnextparamislast = true;	// makes sure Frontier throws an error

	if (!gettextvalue (hparam1, 2, &fromstring)) // get the from param
		return (false);

	// Null terminate strings
	if (!pushcharhandle (0x00, fromstring))
		return (false);

	size = strlen(*fromstring);

	if(!newhandle (size*2+1, &tostring)) {
		return false;
	}

	encodedSize = mysql_real_escape_string(dbid, *tostring, *fromstring, size);

	// Exit the verb, converting the allocated string back to Frontier handle
	if(!newfilledhandle ((ptrvoid) *tostring, encodedSize, &h)) {
		return false; // Allocation failed
	}

	disposehandle(tostring);

	return (setheapvalue (h, stringvaluetype, vreturned)); 

	} /* mysqlescapestringverb */


boolean mysqlisthreadsafeverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	unsigned long returnCode;	// The number being returned back to Frontier

	/*
		mysql.isThreadSafe()

		Action:  checks to see if the version of the MySQL client linked in is threadsafe
		Params:  none
		Returns: true if safe, false if not

		MySQL docs: http://dev.mysql.com/doc/refman/5.1/en/mysql-thread-safe.html

		Notes: 

		This is unlikely to be used by the Frontier scripter, but is an easy check to be sure
		the right MySQL client is being linked into the Frontier kernel.
	*/

	// Process the MySQL call
	returnCode = mysql_thread_safe();

	if (returnCode == 1)
		return(setbooleanvalue (true, vreturned));
	else
		return(setbooleanvalue (false, vreturned));

	} /* mysqlisthreadsafeverb */

