
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

#define iowaverbsinclude

#ifdef fliowa
	
	/*temp -- use archaic "headline" external type as slot for iowa*/
	
	#define idcardprocessor idheadrecord
	#define cardvaluetype headvaluetype
	
	
	/*prototypes*/
	
	#ifdef langexternalinclude
	
	extern boolean cardverbgetdisplaystring (hdlexternalvariable, bigstring);
	
	extern boolean cardverbgettypestring (hdlexternalvariable, bigstring);
	
	extern boolean cardverbdispose (hdlexternalvariable, boolean);
	
	extern boolean cardverbnew (Handle, hdlexternalvariable *);
	
	extern boolean cardverbisdirty (hdlexternalvariable);
	
	extern boolean cardverbmemorypack (hdlexternalvariable, Handle *);
	
	extern boolean cardverbmemoryunpack (Handle, long *, hdlexternalvariable *);
	
	extern boolean cardverbpack (hdlexternalvariable, Handle *);
	
	extern boolean cardverbunpack (Handle, long *, hdlexternalvariable *);
	
	extern boolean cardverbpacktotext (hdlexternalvariable, Handle);
	
	extern boolean cardverbgetsize (hdlexternalvariable, long *);
	
	extern boolean cardverbgettimes (hdlexternalvariable, long *, long *);
	
	extern boolean cardwindowopen (hdlexternalvariable, hdlwindowinfo *);
	
	extern boolean cardedit (hdlexternalvariable, bigstring, Rect);
	
	extern boolean cardverbfind (hdlexternalvariable, boolean *);
	
	#endif
	
	extern boolean cardinitverbs (void);
	
	extern boolean cardstart (void);

#endif

