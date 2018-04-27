
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

#include <standard.h>
#include "config.h"
#include "dialogs.h"
#include "file.h"
#include "strings.h"
#include "db.h" /*stats*/
#include "shell.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "tableverbs.h"
#include "opverbs.h"
#include "scripts.h"
#include "menuverbs.h"
#include "pictverbs.h"
#include "wpverbs.h"
#include "cancoon.h"
#include "command.h"
#ifdef flcomponent
	#include "osacomponent.h"
#endif
#include "odbinternal.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"


static boolean ccstart (void) {
	
	tyfilespec fs;
	hdlfilenum fnum;
	odbref odb;
	
	fs.vRefNum = 0;
	fs.parID = 0;
	copystring ("\podbTester.root", fs.name);
	
	if (!fileopenorcreate  (&fs, 'CLNG', 'ROOT', &fnum)) {
		
		alertdialog ("\pError creating file.\n");
		
		goto error;
		}

	if (!odbNewFile (fnum)) {
	
		alertdialog ("\pError creating new database\n");
		
		goto error;
		}
	
	if (!odbOpenFile (fnum, &odb)) {
	
		alertdialog ("\pError opening database\n");
		
		goto error;
		}
	
	return true;
	
	error:
		return false;
	} /*ccstart*/


void main (void) {
	
	/*
	3.0.4b6 dmb: call osacomponentstart from here, after the shell
	is more thoroughly initialized.
	*/
	
	iddefaultconfig = idscriptconfig;
	
	Debugger ();
	
	assert (sizeof (tyvaluerecord) == 8);
	
	assert (sizeof (tyhashnode) == 18);
	
	if (!shellstart ())
		return;
	
	if (!opstart ())
		shellquit ();
	
	/*
	if (!menustart ())
		shellquit ();
	
	if (!tablestart ())
		shellquit ();
	
	if (!scriptstart ())
		shellquit ();
	
	if (!wpstart ())
		shellquit ();
	
	if (!cmdstart ())
		shellquit ();
	
	if (!langdialogstart ())
		shellquit ();
	
	if (!langerrorstart ())
		shellquit ();
	
	#ifdef iowaRuntime
	
	if (!langcardstart ())
		shellquit ();
	
	#endif
		
	if (!statsstart ())
		shellquit (); 
		
	if (!pictstart ())
		shellquit ();
	*/

	if (!ccstart ())
		shellquit ();
	
	#ifdef flcomponent
	
	if (!osacomponentstart ())
		;	// don't quit if this doesn't work
	
	#endif
	
	shellmaineventloop ();
	} /*main*/

