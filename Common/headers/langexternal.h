
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

#ifndef langexternalinclude
#define langexternalinclude


#ifndef langinclude

	#include "lang.h"

#endif

#ifndef dbinclude

	#include "db.h"

#endif

#ifndef shellinclude

	#include "shell.h"

#endif


#define idophashresource 134
#define idwordhashresource 135
#define idtablehashresource 136
#define idmenuhashresource 137
#define idpicthashresource 138


typedef enum tyexternalid { /*11/17/90 DW: order determines sort order when viewing by kind*/
	
	idoutlineprocessor,
	
	idwordprocessor,
	
	idheadrecord,
	
	idtableprocessor,
	
	idscriptprocessor,
	
	idmenuprocessor,
	
	idpictprocessor,
	
	#ifdef flnewfeatures
	
	idappleprocessor,
	
	#endif
	
	/*new types must be added at end of list, these get saved on disk*/
	
	idcardprocessor,
	
	ctexternalprocessors
	} tyexternalid;


typedef struct typrocessorcallbacks { /*this structure isn't currently used*/
	
	callback loadroutine;
	
	callback unloadroutine;
	
	callback packroutine;
	
	callback unpackroutine;
	
	callback editroutine;
	
	callback newroutine;
	
	callback disposeroutine;
	
	callback converttotextroutine;
	
	callback getdisplaystringroutine;
	
	callback gettypestringroutine;
	
	callback functionvalueroutine;
	
	hdlhashtable functiontable;
	
	short tableresourceid;
	
	long processorrefcon;
	} typrocessorcallbacks, *ptrprocessorcallbacks;

/*
	11/15/01 dmb: added flags for various external types so they can use this record,
	without having to add a bunch of macros to preserve existing code.
*/
typedef struct tyexternalvariable {
	
	unsigned short id;	/*tyexternalid*/
	
	unsigned short flinmemory: 1; /*if true, variabledata is in a handle, else a dbaddress*/
	
	unsigned short flmayaffectdisplay: 1; /*not in memory, but being displayed in a table window*/
	
	unsigned short flpacked: 1; /* for wp doc; it isn't being edited, so we store it packed*/

	unsigned short flscript: 1; /* for outlines and scripts; they're identical, except for this bit*/

	unsigned short flsystemtable: 1; /*for tables: was it created by the system, or by a user script?*/

	#ifdef xmlfeatures
		unsigned short flxml: 1; /*preserve for tables; is it an xml table?*/
	#endif

//	unsigned short variableflags: 3; /*these can be defined independently for each variable*/
	
	long variabledata; /*either a handle to data record or a dbaddress*/
	
	hdldatabaserecord hdatabase; // 5.0a18 dmb

	dbaddress oldaddress; /*last place this variable was stored in db*/

#if langexternalfind_optimization

	hdlhashtable hexternaltable;

	hdlhashnode hexternalnode;

#endif
	} tyexternalvariable, *ptrexternalvariable, **hdlexternalvariable;


typedef hdlexternalvariable hdlexternalhandle;

/*
typedef struct tyexternalhandle {
	
	tyexternalid id; 
	
	Handle hdata; 
	} tyexternalhandle, *ptrexternalhandle, **hdlexternalhandle;
*/


typedef struct tydiskexternalhandle {
	
	short versionnumber; /*this structure is stored on disk*/
	
	#ifdef MACVERSION
		tyexternalid id;
		//byte unused;
	#endif
	
	#ifdef WIN95VERSION
		byte id;
		byte unused;
	#endif

	} tydiskexternalhandle;

#define externaldiskversionnumber 1


/*prototypes*/

extern tyexternalid langexternalgettype (tyvaluerecord);

extern boolean langexternalgettable (bigstring, hdlhashtable *);

extern boolean langexternalvaltotable (tyvaluerecord, hdlhashtable *, hdlhashnode);

extern boolean langexternalfindvariable (hdlexternalvariable, hdlhashtable *, bigstring);

extern boolean langexternalgettablevalue (hdltreenode, short, hdlhashtable *);

extern hdldatabaserecord langexternalgetdatabase (hdlexternalvariable);

extern void langexternalsetdatabase (hdlexternalvariable, hdldatabaserecord);

extern boolean langsetexternalsymbol (hdlhashtable, bigstring, tyexternalid, Handle);

extern boolean langexternaldontsave (hdlhashtable, bigstring);

extern boolean langexternalpleasesave (hdlhashtable, bigstring);

extern boolean langexternaltypestring (hdlexternalhandle, bigstring);

extern boolean langexternalgetdisplaystring (hdlexternalhandle, bigstring);

extern boolean langexternalisdirty (hdlexternalhandle);

extern boolean langexternalsetdirty (hdlexternalhandle, boolean);

extern boolean langexternalpack (hdlexternalhandle, Handle *, boolean *);

extern boolean langexternalunpack (Handle, hdlexternalhandle *);

extern boolean langexternalmemorypack (hdlexternalhandle, Handle *, hdlhashnode);

extern boolean langexternalmemoryunpack (Handle, hdlexternalhandle *);

extern boolean langexternalcopyvalue (const tyvaluerecord *, tyvaluerecord *);

extern boolean langexternalcoercetostring (tyvaluerecord *);

extern boolean langexternalgetowningwindow (hdlwindowinfo *);

extern void langexternalquotename (bigstring);

extern void langexternalbracketname (bigstring);

extern boolean langexternalgetfullpath (hdlhashtable, bigstring, bigstring, hdlwindowinfo *);

extern boolean langexternalgetquotedpath (hdlhashtable, bigstring, bigstring);

extern boolean langexternalgetexternalparam (hdltreenode, short, short *, hdlexternalvariable *);

extern boolean langexternalzoomfrom (tyvaluerecord, hdlhashtable, bigstring, rectparam);

extern boolean langexternalzoom (tyvaluerecord, hdlhashtable, bigstring);

extern boolean langexternalzoomfilewindow (const tyvaluerecord *, tyfilespec *, boolean);

extern boolean langexternalwindowopen (tyvaluerecord, hdlwindowinfo *);

extern boolean langexternalwindowclosed (hdlexternalvariable);

extern boolean langexternaldisposevariable (hdlexternalvariable, boolean, boolean (*) (hdlexternalvariable, boolean));

extern boolean langexternaldisposevalue (tyvaluerecord, boolean);

extern boolean langexternalgetconfig (tyvaluetype, short, tyconfigrecord *);

extern boolean langexternalnewvalue (tyexternalid, Handle, tyvaluerecord *);

extern boolean langexternalvaltocode (tyvaluerecord, hdltreenode *);

extern boolean langexternalgetvalsize (tyvaluerecord, long *);

extern boolean langnewexternalvariable (boolean, long, hdlexternalvariable *);

extern short langexternalcomparetypes (tyexternalid, tyexternalid);

extern boolean langexternalsurfacekey (hdlexternalvariable);

extern boolean langexternalpacktotext (hdlexternalhandle, Handle);

extern boolean langexternalsearch (tyvaluerecord, boolean *);

extern boolean langexternalcontinuesearch (hdlexternalvariable);

extern boolean langexternalgettimes (hdlexternalhandle, long *, long *, hdlhashnode);

extern boolean langexternalsettimes (hdlexternalhandle, long, long, hdlhashnode);

extern boolean langexternalfindusedblocks (hdlexternalhandle, bigstring);

extern boolean langexternaltitleclick (Point, hdlexternalvariable);

extern OSType langexternalgettypeid (tyvaluerecord);

extern tyvaluetype langexternalgetvaluetype (OSType);

extern boolean langexternalregisterwindow (hdlexternalvariable);

extern boolean langexternalunregisterwindow (hdlwindowinfo);

extern boolean langexternalcloseregisteredwindows (boolean);

extern boolean langexternalrefdata (hdlexternalvariable, Handle *);

boolean langexternalsetreadonly (hdlexternalvariable hv, boolean flreadonly); /*7.0b6 PBS*/

boolean langexternalgetreadonly (hdlexternalvariable hv); /*7.0b6 PBS*/

extern boolean fullpathstats (hdlhashtable intable, boolean flfirst);

extern boolean hashstatsverb (tyvaluerecord *v);

extern boolean langexternalsymbolchanged (hdlhashtable htable, const bigstring bsname, hdlhashnode hnode, boolean flvalue);

extern boolean langexternalsymbolinserted (hdlhashtable htable, const bigstring bsname, hdlhashnode hnode);

#endif



