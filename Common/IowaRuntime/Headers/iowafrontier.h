
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

#define iowafrontierinclude


#define idfrontier 			'LAND'

boolean findfrontiercomponent ();

boolean frontStartCard (AEDesc *);

boolean frontSetRuntimeCard (boolean, boolean);

boolean frontDeleteCardTable (void);

boolean frontEditTable (bigstring);

boolean frontGetTable (void);

boolean frontRunMacro (bigstring, bigstring, Handle *);

boolean frontRunUserScript (bigstring, bigstring, bigstring, Handle *);

boolean frontAddToGlossary (void *, bigstring, short, bigstring);

boolean frontEditGlossary (bigstring);

boolean frontGlossaryReference (bigstring, bigstring, Handle *);

boolean frontEditObject (bigstring, bigstring);

boolean frontRunLinkFilter (Handle, short, bigstring, Handle *);

boolean frontCopyToPageTable (AERecord *);

boolean frontCopyFromPageTable (AERecord *);

boolean frontRunPageFilter (bigstring);

boolean frontRunFinalFilter (Handle, bigstring, Handle *);
