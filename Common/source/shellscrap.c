
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

  
#ifdef MACVERSION
	#include <standard.h>
#endif

#ifdef WIN95VERSION
	#include "standard.h"
#endif

#include "kb.h"
#include "memory.h"
#include "scrap.h"
#include "strings.h"
#include "shell.h"
#include "shellhooks.h"




typedef struct tyscraprecord {

	short count; /*for comparison to toolbox scrapcount*/
	
	boolean floptionkey; /*4.1b7 dmb: was scrap read w/Option key down?*/
	
	boolean fldirty; /*needs to be put into the desktop scrap*/
	
	short fllocked; /*for Win reentrancy*/

	Handle hscrap; /*if not nil, points to a chunk of text*/
	
	tyscraptype type; /*one type at a time*/
	
	shelldisposescrapcallback disposeroutine;
	
	shellexportscrapcallback exportroutine;
	} tyscraprecord;


tyscraprecord shellscrap;




static void shelldisposeinternalscrap (void) {
	
	/*
	5.0a16 dmb: this used to be shelldisposescrap
	*/

	if (shellscrap.hscrap)
		(*shellscrap.disposeroutine) (shellscrap.hscrap);
	
	shellscrap.hscrap = nil;
	
	shellscrap.fldirty = false;
	} /*shelldisposeinternalscrap*/


void shelldisposescrap (void) {
	
	if (!shellscrap.fllocked) {
		
		shelldisposeinternalscrap ();
		
		handlescrapdisposed ();
		}
	} /*shelldisposescrap*/


static boolean shellsetinternalscrap (void * hscrap, tyscraptype type, shelldisposescrapcallback disposeroutine, shellexportscrapcallback exportroutine) {
	
	/*
	5.0a16 dmb: this used to be shellsetscrap
	*/

	if (hscrap == nil)
		return (false);
	
	shelldisposeinternalscrap (); /*get rid of whatever's there*/
	
	shellscrap.hscrap = hscrap;
	
	shellscrap.fldirty = true;
	
	shellscrap.type = type;
	
	shellscrap.disposeroutine = disposeroutine;
	
	shellscrap.exportroutine = exportroutine;
	
	shellscrap.count = getscrapcount (); /*we're newer than desk scrap*/

	return (true);
	} /*shellsetinternalscrap*/


boolean shellsetscrap (void * hscrap, tyscraptype type, shelldisposescrapcallback disposeroutine, shellexportscrapcallback exportroutine) {
	
	/*
	5.0a16 dmb: do it right, use delayed rendering for Win. Calling 
	putscrap should result in the disposal of the current private clipboard
	*/
	
	#ifdef WIN95VERSION
		if (openclipboard ()) { // set this scrap type for delayed rendering
			
			if (resetscrap ()) {

				putscrap (type, NULL);
				
				if ((type != textscraptype) && (type != pictscraptype))
					putscrap (textscraptype, NULL);
				}

			closeclipboard ();
			}
	#endif
	
	return (shellsetinternalscrap (hscrap,type, disposeroutine, exportroutine));
	} /*shellsetscrap*/


boolean shellgetscrap (Handle *hscrap, tyscraptype *type) {
		
	*hscrap = shellscrap.hscrap;
	
	*type = shellscrap.type;
	
	return (shellscrap.hscrap != nil);
	} /*shellgetscrap*/


static boolean exportshellscrap (tyscraptype type, Handle *hscrap, boolean *flconverted) {
	
	/*
	5.0b10 dmb: if exportroutine is nil, but script is right type, 
	return true with unconverted data

	5.0.2 dmb: if hscrap is nil, return false
	*/
	
	if (shellscrap.hscrap == nil)
		return (false);

	if (shellscrap.exportroutine)
		return ((*shellscrap.exportroutine) (shellscrap.hscrap, type, hscrap, flconverted));
	
	if (shellscrap.type == type) {
		
		*hscrap = shellscrap.hscrap;
		
		*flconverted = false; /*we're returning a handle to the actual scrap*/
		
		return (true);
		}
	
	return (false);
	} /*exportshellscrap*/


boolean shellconvertscrap (tyscraptype type, Handle *hscrap, boolean *flconverted) {
	
	/*
	if the scrap is of the given type, simply return a handle to it with
	no conversion.
	otherwise, return a handle to a converted version of the scrap contents.
	*/
	
	if (!shellscrap.hscrap)
		return (false);
	
	if (shellscrap.type == type) {
		
		*hscrap = shellscrap.hscrap;
		
		*flconverted = false; /*we're returning a handle to the actual scrap*/
		
		return (true);
		}
	
	return (exportshellscrap (type, hscrap, flconverted));
	} /*shellconvertscrap*/


boolean shellreadscrap (void) {
	
	/*
	return true if something was loaded from the desk scrap, false 
	otherwise.
	
	dmb: since we don't display the script (i.e. no Clipboard window), 
	this doesn't need to be called every time the desk scrap may have changed.
	we only need to check the desk scrap when we're about to do a paste.
	
	12/28/90 dmb: handle pict
	
	4.1b7 dmb: added floptionkey checking so that same scrap can be 
	imported again if the state of the Option key changes
	*/
	
	short scrapcount;
	Handle hscrap;
	
	scrapcount = getscrapcount ();
	
	if ((scrapcount <= shellscrap.count) && /*desk scrap isn't newer*/
		(keyboardstatus.floptionkey == shellscrap.floptionkey)) /*same key status*/
		return (false);
	
	shellscrap.count = scrapcount; /*remember for next time*/
	
	shellscrap.floptionkey = keyboardstatus.floptionkey; /*4.1b7*/
	
	if (!newemptyhandle (&hscrap)) /*this is where the result of GetScrap is to be put*/
		return (false);
	
	openclipboard ();

	if (shellcallscraphooks (hscrap)) { /*none of the hooks set the scrap*/
		
		register tyscraptype scraptype = noscraptype;
		
	#ifndef WIN95VERSION
		if (getscrap ('PICT', hscrap))
			scraptype = pictscraptype;
		
		else { 
	#endif
			if (getscrap ('TEXT', hscrap))
				scraptype = textscraptype;
		
			else {
				
				scraptype = getscraptype (); /*whatever's there*/
				
				if (!getscrap (scraptype, hscrap))
					scraptype = noscraptype;
				}
		#ifndef WIN95VERSION
			}
		#endif
		
		if (scraptype != noscraptype)
			shellsetinternalscrap (hscrap, scraptype, (shelldisposescrapcallback) disposehandle, nil);
		}
	
	shellscrap.fldirty = false; /*doesn't need to be exported*/
	
	closeclipboard ();

	return (true);
	} /*shellreadscrap*/


static boolean cr2crlfhandle (Handle htext) {
	
	byte bscr[] = "\x01\r";
	byte bscrlf[] = "\x02\r\n";
	Handle hcr = nil;
	Handle hcrlf = nil;
	boolean fl;
	
	if (!newtexthandle (bscr, &hcr))
		return (false);

	fl = newtexthandle (bscrlf, &hcrlf);

	if (fl)
		fl = textfindreplace (hcr, hcrlf, htext, true, false);

	disposehandle (hcr);
	
	disposehandle (hcrlf);

	return (fl);
	} /*cr2crlfhandle*/


boolean shellexportscrap (tyscraptype scraptype) {
	
	/*
	7/21/91 dmb: new convention: asking the export routine to export to its 
	own type means we want a contigous version of its structure to put on the 
	system clipboard
	
	5.0a16 dmb: add linefeeds when exporting converted text.
	
	5.0a23 dmb: call shellconvertscrap, not exportshellscrap
	
	5.0b9 dmb: no, that was wrong. we must allow the exportscrap routine to 
	create an export version of the current type, which convertscrap doesn't do
	*/
	
	Handle hscrap = nil;
	boolean flconverted = false;
	boolean fl = false;
	
	if (!exportshellscrap (scraptype, &hscrap, &flconverted)) //couldn't create this type
		return (false);
	
	#ifdef WIN95VERSION
	
		if (scraptype == textscraptype) {
			
			if (flconverted)			
				cr2crlfhandle (hscrap);
			}
		
	#endif
	
	fl = putscrap (scraptype, hscrap);
	
	if (flconverted)
		disposehandle (hscrap);
	
	return (fl);
	} /*shellexportscrap*/


boolean shellwritescrap (tyscraptype type) {
	
	/*
	export our internal scrap.  if it's a picture or text, export it as is.  
	otherwise convert to pict or text and export that.
	
	6/5/91 dmb: now put internal form of scrap onto clipboard as well 
	as converted material
	
	2.1b3 dmb: added type parameter so caller can cause scrap to be 
	written only if it's a certain type. if the caller passes textscraptype, 
	we'll accept any scrap smaller than 256 bytes.
	
	5.0d14 dmb: can't count on handle size of non-text scrap predicting the 
	size of an exported text scrap. so we don't enforce the 255-char limit 
	here. The caller must do so.

	5.0a16 dmb: Win-ized, scrap exporting occurs on demand. we only 
	do something if type is allscraptypes, meaning we're responding to a 
	WM_RENDERALLFORMATS message
	*/
	
	boolean fl = false;
	
	#ifdef WIN95VERSION
		if (type != allscraptypes)
			return (true);
	#endif

	#ifdef MACVERSION
		if (!shellscrap.fldirty)
			return (false);
		
		if ((type != anyscraptype) && (type != shellscrap.type)) {
			
			if ((type != textscraptype) /*|| (gethandlesize (shellscrap.hscrap) > lenbigstring)*/)
				return (false);
			}
	#endif
	
	shellscrap.fldirty = false;
	
	if (!shellscrap.hscrap)
		return (false);
	
	if (!openclipboard ())
		return (false);
	
	++shellscrap.fllocked; //so resetscrap won't touch it

	if (!resetscrap ())
		goto exit;
	
	shellscrap.count = getscrapcount ();
	
	switch (shellscrap.type) {
		
		#if TARGET_API_MAC_CARBON == 1
			case hashscraptype: /*7.0b48 PBS: can't export this type*/
			
				fl = false;
				
				break;
		#endif
			
		case pictscraptype:
			fl = shellexportscrap (pictscraptype);
			
			break;

		case textscraptype:
			fl = shellexportscrap (textscraptype);

			break;
		
		default:
			if (shellexportscrap (shellscrap.type))
				fl = true;
			
			if (shellexportscrap (pictscraptype))
				fl = true;
			
			if (shellexportscrap (textscraptype))
				fl = true;
			
			break;
		}

exit:
	--shellscrap.fllocked;

	closeclipboard ();

	return (fl);
	} /*shellwritescrap*/


void initscrap (void) {
	
	initclipboard ();

	clearbytes (&shellscrap, sizeof (shellscrap));
	
	shellscrap.count = getscrapcount () - 1; /*make sure they're different*/
	} /*initscrap*/
