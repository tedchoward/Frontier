
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

/* OSXSpecifics.h
	
	This file and its header represent a one-stop shop for setting up OSX specific things.
	The goal is to only need to call the setup procedure once, and have the function pointers
	available for the rest of the time Frontier is running. I'll hone this down over time,
	but for right now, this is good enough.
	
*/

enum {
	kQDUseDefaultTextRendering 	= 0,			// Sets whatever is specified in system defaults.
												// Currently sets kQDUseTrueTypeScalerGlyphs if nothing is specified.
	kQDUseTrueTypeScalerGlyphs  	= (1 << 0),	// bit 0
	kQDUseCGTextRendering		= (1 << 1),	// bit 1
	kQDUseCGTextMetrics			= (1 << 2),	// bit 2
	kQDSupportedFlags			= kQDUseTrueTypeScalerGlyphs | kQDUseCGTextRendering | kQDUseCGTextMetrics,	kQDDontChangeFlags	= 0xFFFFFFFF	// to request the current state, without changing anything
};



// BSD function prototypes
/*
int	execv( const char *path, char *const argv[] );
*/
typedef int (*execvFuncPtr)( const char*, char **const );


FILE *	 popen(const char *command, const char *type);
typedef FILE *(*BSDpopenFuncPtr)( const char*, const char* );

int	pclose( FILE *stream );
typedef int (*BSDpcloseFuncPtr)( FILE* );

typedef int (*BSDfreadFuncPtr)( void *, size_t, size_t, FILE * );
typedef UInt32 (*QuartzTextPtr)( UInt32 );

void useQDText(int i);


void	InvokeTool( char *toolName );

