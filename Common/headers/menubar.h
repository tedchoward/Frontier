
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

#define menubarinclude


#ifndef shelltypesinclude
	
	#include "shelltypes.h"

#endif


#ifndef opinclude

	#include "op.h"

#endif


/*types*/

#define maxmenus 96 /*maximum number of user-defined menus for each menu list*/

#define ctmenubarstack 50 /*maximum number of menus stacked in each menubar outline*/


typedef struct tymenubarstackelement {

	hdlmenu hmenu; /*the menu data structure itself*/
	
	short idmenu; /*Menu Manager id of this menu*/
	
	hdlheadrecord hnode; /*the structure that created this menu*/

	boolean flhierarchic; /*is it a sub-menu?*/
	
	boolean flenabled; /*is it enabled?*/
	
	boolean flbuiltin; /*does menu belong to the Frontier app?*/
	
	short ctbaseitems; /*if flbuiltin, number of items that were already there*/
	} tymenubarstackelement;


typedef struct tymenubarstack {
	
	struct tymenubarstack **hnext; /*linked list of these*/
	
	hdloutlinerecord menubaroutline; /*the outline that is at one w/the menubar structure*/
	
	boolean flactive: 1;
	
	boolean flclientowned: 1; /*actual menus owned by menu sharing client?*/
	
	short ixdeletedmenu; /*for rebuilding menu bar*/
	
	short topstack;
	
	long refcon;
	
	tymenubarstackelement stack [ctmenubarstack];
	} tymenubarstack, *ptrmenubarstack, **hdlmenubarstack;


typedef struct tymenubarlist {
	
	hdlmenubarstack hfirst; /*linked list of these*/
	
	boolean flactive: 1; /*is this the active menubar?*/
	
	short basemenuid;
	
	byte menubitmap [(maxmenus / 8) + 1]; /*assume eight bits per byte*/
	} tymenubarlist, *ptrmenubarlist, **hdlmenubarlist;


typedef boolean (*menubarchangedcallback)(hdloutlinerecord);

typedef byte (*menubarcmdkeycallback)(hdlheadrecord);

typedef struct tymenubarcallbacks {
	
	menubarchangedcallback menubarchangedroutine;
	
	menubarcmdkeycallback getcmdkeyroutine;
	} tymenubarcallbacks;


/*globals*/

extern hdlmenubarlist menubarlist;

extern tymenubarcallbacks menubarcallbacks;

	
/*prototypes*/

extern boolean pushmenubarglobals (hdlmenubarstack);

extern boolean popmenubarglobals (void);

extern boolean newmenubarlist (hdlmenubarlist *);

extern void setcurrentmenubarlist (hdlmenubarlist);

extern boolean activatemenubarlist (hdlmenubarlist, boolean);

extern boolean disposemenubarlist (hdlmenubarlist);

extern void medirtymenubar (void);

extern void meupdatemenubar (void);

extern void mecheckmenubar (void);

extern boolean medisposemenubar (hdlmenubarstack);

extern boolean menewmenubar (hdloutlinerecord, hdlmenubarstack *);

extern boolean mebuildmenubar (hdlmenubarstack);

extern boolean memenuitemchanged (hdlmenubarstack, hdlheadrecord);

extern short mecheckdeletedmenu (short, boolean);

extern boolean memenuitemadded (hdlmenubarstack, hdlheadrecord);

extern boolean memenuitemdeleted (hdlmenubarstack, hdlheadrecord);

extern boolean meinsertmenubar (hdlmenubarstack);

extern boolean medeletemenubar (hdlmenubarstack);

extern boolean purgefrommenubarlist (long);

extern boolean rebuildmenubarlist (void);

extern boolean melocatemenubarnode (hdlheadrecord, hdloutlinerecord *);

extern boolean mecheckformulas (short);

extern boolean memenuhit (short, short, hdlheadrecord *);

extern boolean memenu (short, short);

extern void menubarinit (void);

extern boolean mereduceformula (bigstring bs); /*7.0b12 PBS: used by oppopup.c*/

extern void mereducemenucodes (bigstring bs, boolean *flenabled, boolean *flchecked); /*7.0b23 PBS: used by oppopup.c*/




