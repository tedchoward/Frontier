
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

#define iowaprefsinclude

#define ctdialogs 50

typedef struct tydialoginfo { /*information we remember about each dialog*/
	
	short top, left;
	} tydialoginfo;
	
#define maxsavedquickscript 1024

typedef struct tyiowaprefs { /*globals that communicate between callbacks*/
	
	short versionnumber; /*this structure is saved on disk*/
	
	/*information we maintain for each iowa dialog*/
	
		tydialoginfo dialoginfo [ctdialogs];
		
	/*information for the quick script window*/
		
		short ixdefaultquickscriptlanguage;
		
		long lensavedquickscript;
		
		char savedquickscript [maxsavedquickscript];
		
	char waste [992]; /*room for growth*/
	} tyiowaprefs;
	
extern tyiowaprefs iowaprefs;


boolean iowadefaultprefs (void);

boolean iowareadprefsfile (FSSpec *, tyiowaprefs *);

boolean iowaloadprefs (void);

boolean iowawriteprefsfile (tyiowaprefs *, FSSpec *);

boolean iowasaveprefs (void);

void littleprefsdialog (void);

