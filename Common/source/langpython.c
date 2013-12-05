
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

#include "frontier.h"
#include "standard.h"

#define xxxFRONTIER_PYTHON 1

#ifdef FRONTIER_PYTHON

#include "C:\\Python16\\include\\Python.h"

#endif

#include "error.h"
#include "file.h"
#include "memory.h"
#include "ops.h"
#include "resources.h"
#include "strings.h"
#include "lang.h"
#include "langipc.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "langhtml.h"
#include "langwinipc.h"
#include "process.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "op.h"
#include "opinternal.h"
#include "oplist.h"
#include "opverbs.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "shell.rsrc.h"
#include "timedate.h"
#include "WinSockNetEvents.h"
#ifdef flcomponent
#include "osacomponent.h"
#endif
#include "langpython.h"

#ifdef FRONTIER_PYTHON

static boolean flpythoninitialized = false;

static void initpython (void) {

	if (!flpythoninitialized) {

		flpythoninitialized = true;

		Py_Initialize ();
		}

	return;
	}/*initpython*/


boolean langrunpythonscript (hdltreenode hp1, tyvaluerecord *v) {

	Handle h;
	boolean fl = false;
	char nilchar = '\0';

	flnextparamislast = true;

	if (!getexempttextvalue (hp1, 1, &h))
		return (false);

	if (!enlargehandle (h, 1, &nilchar)) {

		disposehandle (h);

		return (false);
		}

	initpython ();

	lockhandle (h);

	//PyObject* PyRun_String (char *str, int start, PyObject *globals, PyObject *locals);

	if (PyRun_SimpleString (*h) == 0)
		fl = setbooleanvalue (true, v);
	
	unlockhandle (h);

	disposehandle (h);

	return (fl);
	}/*langrunpythonscript*/

#else

boolean langrunpythonscript (hdltreenode hp1, tyvaluerecord *v) {
#pragma unused(hp1, v)

	langerror (unimplementedverberror);

	return (false);
	}

#endif
