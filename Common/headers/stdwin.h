/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright (C) 1993-1995 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
*
*		stdwin.h : include file for standard system include files,
*		or project specific include files that are used frequently, but
*		are changed infrequently
\******************************************************************************/

#include <windows.h>
#include <fcntl.h>
#include <SYS\types.h>
#include <SYS\stat.h>
#include <io.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

#ifndef WIN16
    #include <commdlg.h>
#endif

#ifdef WIN16
#include "drivinit.h"
#endif
