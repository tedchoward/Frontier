
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

#ifndef langdllinclude
#define langdllinclude

#ifndef FDLLCALL_H
#include "FDllCall.h"
#endif


Handle xCALLBACK extfrontierReAlloc (Handle h, long sz);
Handle xCALLBACK extfrontierAlloc (long sz);
char * xCALLBACK extfrontierLock (Handle h);
void xCALLBACK extfrontierFree (Handle h);
long xCALLBACK extfrontierSize (Handle h);
void xCALLBACK extfrontierUnlock (Handle h);

odbRef xCALLBACK extOdbGetCurrentRoot (void);
odbBool xCALLBACK extOdbNewFile (hdlfilenum);
odbBool xCALLBACK extOdbOpenFile (hdlfilenum, odbRef *odb);
odbBool xCALLBACK extOdbSaveFile (odbRef odb);
odbBool xCALLBACK extOdbCloseFile (odbRef odb);
odbBool xCALLBACK extOdbDefined (odbRef odb, odbString bspath);
odbBool xCALLBACK extOdbDelete (odbRef odb, odbString bspath);
odbBool xCALLBACK extOdbGetType (odbRef odb, odbString bspath, OSType *type);
odbBool xCALLBACK extOdbCountItems (odbRef odb, odbString bspath, long *count);
odbBool xCALLBACK extOdbGetNthItem (odbRef odb, odbString bspath, long n, odbString bsname);
odbBool xCALLBACK extOdbGetValue (odbRef odb, odbString bspath, odbValueRecord *value);
odbBool xCALLBACK extOdbSetValue (odbRef odb, odbString bspath, odbValueRecord *value);
odbBool xCALLBACK extOdbNewTable (odbRef odb, odbString bspath);
odbBool xCALLBACK extOdbGetModDate (odbRef odb, odbString bspath, unsigned long *date);
void xCALLBACK extOdbDisposeValue (odbRef odb, odbValueRecord *value);
void xCALLBACK extOdbGetError (odbString bs);

odbBool xCALLBACK extDoScript (char * script, long len, odbValueRecord *value);
odbBool xCALLBACK extDoScriptText (char * script, long len, Handle * text);

odbBool xCALLBACK extOdbNewListValue (odbRef odb, odbValueRecord *valueList, odbBool flRecord);
odbBool xCALLBACK extOdbGetListCount (odbRef odb, odbValueRecord *valueList, long * cnt);
odbBool xCALLBACK extOdbDeleteListValue (odbRef odb, odbValueRecord *valueList, long index, char * recordname);
odbBool xCALLBACK extOdbSetListValue (odbRef odb, odbValueRecord *valueList, long index, char * recordname, odbValueRecord *valueData);
odbBool xCALLBACK extOdbGetListValue (odbRef odb, odbValueRecord *valueList, long index, char * recordname, odbValueRecord *valueReturn);
odbBool xCALLBACK extOdbAddListValue (odbRef odb, odbValueRecord *valueList, char * recordname, odbValueRecord *valueData);

odbBool xCALLBACK extInvoke (bigstring bsscriptname, void * pDispParams, odbValueRecord * retval, boolean *flfoundhandler, unsigned int * errarg);
odbBool xCALLBACK extCoerce (odbValueRecord * odbval, odbValueType newtype);

odbBool xCALLBACK extCallScript (odbString bspath, odbValueRecord *vparams, odbValueRecord *value); /* 2002-10-13 AR */
odbBool xCALLBACK extCallScriptText (odbString bspath, odbValueRecord *vparams, Handle * text); /* 2002-10-13 AR */

odbBool xCALLBACK extThreadYield (void); /* 2003-04-22 AR */
odbBool xCALLBACK extThreadSleep (long sleepticks); /* 2003-04-22 AR */

extern void dllinitverbs (void); /*2004-11-29 aradke*/

extern void fillcalltable (XDLLProcTable *);

extern boolean dllisloadedverb (hdltreenode hparam1, tyvaluerecord *vreturned);

extern boolean dllloadverb (hdltreenode hparam1, tyvaluerecord *vreturned);

extern boolean dllunloadverb (hdltreenode hparam1, tyvaluerecord *vreturned);

extern boolean dllcallverb (hdltreenode hparam1, tyvaluerecord *vreturned);

#endif

