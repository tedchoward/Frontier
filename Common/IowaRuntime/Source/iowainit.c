
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

#include "iowacore.h"
#include "iowaruntime.h"
#include "iowascript.h"
#include "iowainit.h"


#ifdef IOAinsideApp
	
	extern boolean IOAregistercomponents (void); /*prototype*/

	extern void IOAunregistercomponents (void); /*prototype*/
	
#endif


boolean iowaInit (void) {

	Handle htoss;
	
	#ifdef iowaRuntime
		initIowaRuntime (); /*1.0b24 dmb*/
	#endif
	
	#ifdef IOAinsideApp
		IOAregistercomponents ();
	#endif
	
	initIOAcomponents (&htoss);
	
	initlangcomponents ();
	
	return (true);
	} /*iowaInit*/
	
	
void iowaClose (void) {
	
	closelangcomponents ();
	
	closeIOAcomponents ();
	
	#ifdef IOAinsideApp
		IOAunregistercomponents ();
	#endif
	} /*iowaClose*/
