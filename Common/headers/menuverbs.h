
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

#define menuverbsinclude


#ifndef langexternalinclude

	#include "langexternal.h"

#endif

#ifndef menueditorinclude
	
	#include "menueditor.h"

#endif


/*prototypes*/

extern boolean menuverbgetdisplaystring (hdlexternalvariable, bigstring);

extern boolean menuverbgettypestring (hdlexternalvariable, bigstring);

extern boolean menuverbisdirty (hdlexternalvariable);

extern boolean menuverbsetdirty (hdlexternalvariable, boolean);

extern boolean menuverbmemorypack (hdlexternalvariable, Handle *);

extern boolean menuverbmemoryunpack (Handle, long *, hdlexternalvariable *);

extern boolean menuverbpack (hdlexternalvariable, Handle *, boolean *);

extern boolean menuverbunpack (Handle, long *, hdlexternalvariable *);

extern boolean menuverbpacktotext (hdlexternalvariable, Handle);

extern boolean menuverbgetsize (hdlexternalvariable, long *);

extern boolean menuverbgettimes (hdlexternalvariable, long *, long *);

extern boolean menuverbsettimes (hdlexternalvariable, long, long);

extern boolean menuverbfindusedblocks (hdlexternalvariable, bigstring);

extern boolean menuverbfind (hdlexternalvariable, boolean *);

extern boolean menuwindowopen (hdlexternalvariable, hdlwindowinfo *);

extern boolean menuedit (hdlexternalvariable, hdlwindowinfo, ptrfilespec, bigstring, rectparam);

extern boolean menuverbdispose (hdlexternalvariable, boolean);

extern boolean menuverbnew (Handle, hdlexternalvariable *);

extern boolean menunewmenubar (hdlhashtable, bigstring, hdlmenurecord *);

extern boolean menugetmenubar (hdlhashtable, bigstring, boolean, hdlmenurecord *);

extern boolean editnamedmenubar (hdlhashtable, bigstring);

extern boolean editnamedmenubaropen (hdlhashtable, bigstring);

extern boolean menustart (void);


