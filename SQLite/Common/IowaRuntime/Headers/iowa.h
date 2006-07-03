
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

#define iowainclude


#ifndef appletpopupinclude

	#include <appletpopup.h>
	
#endif

#ifndef iowacoreinclude

	#include "iowacore.h"

#endif

#ifndef iowaruntimeinclude

	#include "iowaruntime.h"

#endif

#ifndef iowaprefsinclude

	#include "iowaprefs.h"

#endif


typedef struct tyeditinfo { /*linked into refcon field of card record*/

	short mode; /*indicates which palette item is selected*/
	
	typopuprecord stylingpopup;
	
	short stylingarrowmode;
	
	Rect rname, rscript, rcolorpopup, rpopup; /*rectangles for object name, linked script, popups*/
	
	boolean flnameempty, flscriptempty; /*used to optimize display of edit rectangles*/
	
	boolean flstatusbaredit;
	
	hdlobject hobjectinstatusbar;
	
	boolean fleditname;
	
	Rect redit;
	
	hdleditrecord heditbuffer;
	
	boolean initcalculatedobjects;
	
	Point lastclickpoint;
	
	boolean flnubonborder;
	} tyeditinfo, **hdleditinfo;
	

#undef flstatusbar /*DW 12/4/93 -- I'm turning off the status bar*/

#define crosshaircursor 500

#define nubsize 5 /*the size of the nub box on a selected object*/

#define vertdamagecontrol /*doctitlebarheight*/0

#define draggingpix 3 /*distance to initiate drag*/
#define draggingticks 10 /*time before drag initiation*/


#define undocutstring 4 /*first app-specific slot in undo stringlist*/
#define undocopystring 5
#define undopastestring 6
#define undoclearstring 7
#define undotypingstring 8
#define undomovestring 9
#define undocleanstring 10
#define undoalignstring 11
#define undoresizestring 12
#define undodeletionstring 13
#define undoformatstring 14
#define undogroupstring 15
#define undoungroupstring 16



boolean packlist (hdlobject firstobject, Handle hpacked);

void disposeobject (hdlobject h);

boolean inselection (hdlobject x);

boolean editobjectdialog (tyobject *);
	
void madechanges (void);

void selectiondirty (void);

void putchildobjectlist (hdlobject, hdlobject);

boolean newobject (tyobject, hdlobject *);

boolean newgroupobject (hdlobject *);

boolean getnthsortedobject (short, hdlobject *);

boolean nothingselected (void);

void makeselectionempty (void);

void makefrontobject (hdlobject);

void sendobjecttoback (hdlobject);

void addtoselection (hdlobject);

boolean visitselectedobjects (tyobjectvisitroutine);

boolean flatvisitselectedobjects (tyobjectvisitroutine);

boolean firstselectedobject (hdlobject *);

boolean caneditobjecttext (hdlobject);

hdlobject findeditableobject (void);

boolean alignselectedobjects (tydirection);

boolean groupselectedobjects (void);

boolean ungroupselectedobjects (void);

boolean iowaiacmessage (void);

boolean iowafastiacmessage (void);

boolean getsingleselectedobject (hdlobject *);

hdleditrecord getanyactiveeditrecord (void);

void setselectionto (hdlobject);

void drawselectionframe (hdlobject, Rect, boolean);

boolean pointinnub (hdlobject, Point);

void invalnub (hdlobject);

boolean iowaedittable (void);

boolean setcardsize (short, short);

void zapcardwindow (void);

void enterarrowmode (void);
