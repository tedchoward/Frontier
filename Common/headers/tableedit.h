
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

#define tableeditinclude


/*globals*/

extern boolean flmustexiteditmode;


/*prototypes*/

extern boolean tableeditsetglobals (void);

extern boolean tableeditingemptycell (void);

extern boolean tablecelliseditable (short, short);

extern boolean tableeditdrawcell (short, short);

extern boolean tableeditactivate (boolean);

extern boolean tableeditsetbufferrect (void);

extern boolean tableeditleavecell (void);

extern boolean tableeditentercell (short, short);

extern boolean tableeditmousedown (Point, short, short);

extern boolean tableeditgo (tydirection);

extern boolean tableeditkeystroke (void);

extern boolean tableeditidle (void);

extern boolean tableeditsetselect (short, short);

extern boolean tableeditpoststylechange (void);

extern boolean tableeditadjustcursor (short, short);

extern boolean tableeditgetundoglobals (long *);

extern boolean tableeditsetundoglobals (long, boolean);




