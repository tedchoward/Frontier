
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

#include "kb.h"
#include "mouse.h"
#include "strings.h"
#include "shell.h"
#include "langinternal.h"




/*
static boolean fllangtrace = false; 
*/


#ifdef fldebug
	#define fllangtrace 0
#else
	#define fllangtrace 0
#endif

extern bigstring bstoken; /*text of last token, see langscan.c*/

/*
#if fllangtrace
	#ifdef MACVERSION
		#define messageset(bs) shellfrontwindowmessage (bs)
	#endif

	#ifdef WIN95VERSION
		static FILE * tracefile = NULL;
		#define messageset(bs)	\
				do {if (tracefile == NULL) \
					tracefile = fopen ("langtrace.txt", "w+"); \
					fprintf (tracefile, "%s\n", bs);} while (0)
	#endif
#endif
*/


#if fllangtrace
	

	#define STR_lineterminator "\r\n"


	static FILE * tracefile = NULL;


	void langstarttrace (void) {

		if (tracefile == NULL)
			tracefile = fopen ("frontier_langtrace.txt", "a+"); /* append */

		fprintf (tracefile, STR_lineterminator);
		} /*langstarttrace*/
		
		
	void langendtrace (void) {

		if (tracefile)
			fclose (tracefile);
		
		tracefile = NULL;
		} /*langendtrace*/


	void langtrace (bigstring bs) {
		
		nullterminate (bstoken);

		nullterminate (bs);

		fprintf (tracefile, ("'%s' ==> %s" STR_lineterminator), &bstoken[1], &bs[1]);
		
		if (shiftkeydown ())
			waitmouseclick ();
		} /*langtrace*/


#else


	void langstarttrace (void) { }
		
	void langendtrace (void) { }

	void langtrace (bigstring bs) { }


#endif

/*
void langsyntaxtrace (boolean fl) {
	
	#ifdef fldebug
	
	fllangtrace = fl; /*set global that turns the syntax trace on or off%/
	
	if (fl) 
		langstarttrace ();
	
	#endif
	} /*langsyntaxtrace*/
	

