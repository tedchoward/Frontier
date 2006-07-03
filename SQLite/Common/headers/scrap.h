
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

#define scrapinclude /*so other includes can tell if we've been loaded*/

#ifndef shelltypesinclude

	#include "shelltypes.h"

#endif



#define noscraptype 0

#define anyscraptype '****'

#define allscraptypes '++++'

#define textscraptype 'TEXT'

#define pictscraptype 'PICT'

#define hashscraptype 'HASH'

#define opscraptype 'OP  '

#define wpscraptype 'WPTX'

#define menuscraptype 'MNBR'

#define scriptscraptype 'SCPT'


/*function prototypes*/

extern boolean resetscrap (void);

extern short getscrapcount (void);

extern short handlescrapdisposed (void);

#ifdef WIN95VERSION
	extern tyscraptype win2shellscraptype (UINT);

	extern UINT shell2winscraptype (tyscraptype);
#endif

extern tyscraptype getscraptype (void);

extern boolean getscrap (tyscraptype, Handle);

extern boolean putscrap (tyscraptype, Handle);

extern boolean openclipboard (void);

extern boolean closeclipboard (void);

extern void initclipboard (void);

