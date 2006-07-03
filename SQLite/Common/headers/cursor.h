
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

#define cursorinclude /*so other includes can tell if we've been loaded*/



typedef enum tycursortype {

	cursorisdirty = -1,
	
	cursorisarrow = -2,
	
	cursorisibeam = iBeamCursor,
	
	cursoriswatch = watchCursor,
	
	cursorisverticalrails = 130,
	
	cursorishorizontalrails = 131,
	
	cursorisslantedrails = 132,
	
	xxxcursorisotherslantedrails = 133,
	
	xxxcursorishorizontalrail = 256,
	
	cursorisbeachball1 = 257, 
	
	cursorisbeachball2 = 258, 
	
	cursorisbeachball3 = 259, 
	
	cursorisbeachball4 = 260,
	
	cursorispopup = 261,
	
	xxxcursorisearth1 = 262, 
	
	xxxcursorisearth2 = 263, 
	
	xxxcursorisearth3 = 264, 
	
	xxxcursorisearth4 = 265, 
	
	xxxcursorisearth5 = 266,
	
	xxxcursorisearth6 = 267, 
	
	xxxcursorisearth7 = 268,
	
	cursorishollowarrow = 269,
	
	cursorfordraggingmove = 270,
	
	xxxcursorissmallquestionmark = 271,
	
	xxxcursorisno = 272,
	
	xxxcursorisbuttonhand = 273,
	
	cursorisgo = 274,
	
	xxxcursorisrightwedge = 275
	} tycursortype;

	
/*prototypes*/

extern void setcursortype (tycursortype);

extern void obscurecursor (void);

extern void initbeachball (tydirection);

extern void rollbeachball (void);

extern boolean beachballcursor (void);

extern void initearth (void);

extern void rollearth (void);




