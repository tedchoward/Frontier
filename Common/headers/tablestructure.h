
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

#define tablestructureinclude

#ifndef langinclude

	#include "lang.h"

#endif

#ifndef dbinclude

	#include "db.h"

#endif


/*constants*/

#define idsystemtablescripts 139

enum  { /*indexes of system table scripts*/
	
	idmenubarscript = 1,
	
	idobjectdbscript,
	
	idquickscriptscript,
	
	idtechsupportscript,
	
	idfinder2clickscript,
	
	idfinder2frontscript,
	
	idfrontierclickers,
	
	idcontrol2clickscript,
	
	idcommand2clickscript,
	
	idoption2clickscript,
	
	idopenwindowscript,
	
	idsavewindowscript,
	
	idclosewindowscript,
	
	idcompilewindowscript,
	
	idisfirsttimescript,
	
	idopenurlscript,
	
	iduseriso8859map,
	
	iduserfontprefscript,
	
	idinexpertmodescript,
	
	idtoggleexpertmodescript,
	
	idrequiredeclarationsscript,
	
	idsuspendscript,
	
	idresumescript,
	
	idsearchparamstable,
	
	idagentsenabledscript,

	idautosave,
	
	idfrontierstartup,
	
	idflwaitduringstartup,
	
	idwebserverstats,

	idinetdshutdown,

	idpikeisfilemenuitemenabledscript,

	idpikegetmenuitemstring,

	idpikerunmenuscript,

	idopstruct2clickscript,

	idopreturnkeyscript,
	
	idopexpandscript,
	
	idopcollapsescript,
	
	idopcursormovedscript, /*7.0b6 PBS*/

	idoprightclickscript,

	idpikeruneditmenuscript,

	idpikeisfilemenuitemcheckedscript,

	idopinsertscript
	};


/*globals*/

extern Handle rootvariable;

extern hdlhashtable roottable;

extern hdlhashtable internaltable;

extern hdlhashtable systemtable;

extern hdlhashtable efptable;

extern hdlhashtable langtable;

extern hdlhashtable builtinstable;

extern hdlhashtable agentstable;

extern hdlhashtable runtimestacktable;

extern hdlhashtable semaphoretable;

extern hdlhashtable threadtable;

extern hdlhashtable filewindowtable;

extern hdlhashtable verbstable;

extern hdlhashtable resourcestable;

extern hdlhashtable pathstable;

extern hdlhashtable iacgluetable;

extern hdlhashtable iachandlertable;

extern hdlhashtable menubartable;

extern hdlhashtable objectmodeltable;

extern hdlhashtable environmenttable;


extern byte nameinternaltable []; 

extern byte namemenubar []; 

extern byte namebeginnermenus []; 

extern byte namebuiltinstable [];

extern byte nameagentstable [];

extern byte nameresourcestable [];

extern byte nameefptable [];

extern byte namelangtable [];

extern byte namestacktable [];

extern byte namesemaphoretable [];

extern byte namethreadtable [];

extern byte namefilewindowtable [];

extern byte nameroottable [];

extern byte namestartuptable [];

extern byte namesuspendtable [];

extern byte nameresumetable [];

extern byte nameshutdowntable [];

extern byte namesystembranch [];

extern byte namepathstable [];

extern byte nameiacgluetable [];

extern byte nameiachandlertable [];

extern byte namemenubartable [];

extern byte nameenvironmenttable [];


/*prototypes*/

extern boolean linksystemtablestructure (hdlhashtable); /*tablestructure.c*/

extern boolean getsystemtablescript (short, bigstring);

extern boolean unlinksystemtablestructure (void);

extern boolean tablenewsubtable (hdlhashtable, bigstring, hdlhashtable *);

extern boolean tablenewsystemtable (hdlhashtable, bigstring, hdlhashtable *);

extern boolean tableloadsystemtable (dbaddress, Handle *, hdlhashtable *, boolean);

extern boolean tablesavesystemtable (Handle, dbaddress *);

extern boolean tablesavesystemtable (Handle, dbaddress *);

extern boolean checktablestructure (boolean);

extern boolean cleartablestructureglobals (void);

extern boolean settablestructureglobals (Handle, boolean);


extern void initsegment (void); /*tablestartup.c*/

extern boolean loadfunctionprocessor (short, langvaluecallback);

extern boolean inittablestructure (void);


extern boolean tablevalidate (hdlhashtable, boolean); /*tablevalidate.c*/




