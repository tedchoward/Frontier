
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

#define scriptsinclude

#ifndef opinclude

	#include "op.h"

#endif


#define typeLAND 'LAND'


/*prototypes*/

extern boolean scriptbuildtree (Handle, long, hdltreenode *);

extern boolean scriptrunstartupscripts (void);

extern boolean scriptrunsuspendscripts (void);

extern boolean scriptrunresumescripts (void);

extern long specialoneshotscriptsrunning (void);

//extern boolean scriptinstallscripts (hdlhashtable);

extern boolean scriptinstallagent (hdlhashnode);

extern boolean scriptremoveagent (hdlhashnode);

extern boolean loadsystemscripts (void);

extern boolean runshutdownscripts (void);

extern boolean scriptinmenubar (void);

extern boolean scriptdebugger (hdltreenode);

extern boolean scriptkilled (void);

extern boolean scriptpushsourcecode (hdlhashtable, hdlhashnode, bigstring);

extern boolean scriptpopsourcecode (void);

extern boolean scriptpushtable (hdlhashtable *);

extern boolean scriptpoptable (hdlhashtable);

extern boolean scriptgetdebuggingcontext (hdlhashtable *);

extern void scriptunlockdebuggingcontext (void);

extern boolean scriptzoomwindow (Rect, Rect, hdlheadrecord, WindowPtr *);

extern void scriptsetcallbacks (hdloutlinerecord);

extern boolean scriptsetdata (WindowPtr, hdlheadrecord, hdloutlinerecord);

extern boolean scriptbackgroundtask (boolean);

extern boolean scriptgettypename (long, bigstring);

extern boolean scriptgetnametype (bigstring, long *);

extern boolean scriptstart (void);

extern boolean initscripts (void);




