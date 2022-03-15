
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

#define NEEDMOREWIN 1

#include "standard.h"

#include "htmlcontrol.h"

#include "WinLand.h"
#include "dialogs.h"
#include "file.h"
#include "font.h"
#include "kb.h"
#include "menu.h"
#include "mouse.h"
#include "quickdraw.h"
#include "scrap.h"
#include "frontier_strings.h"
#include "frontierwindows.h"
#include "cancoon.h"
#include "cancooninternal.h"
#include "shell.h"
#include "shellprivate.h"
#include "shellmenu.h"
#include "lang.h"
#include "wininet.h"

extern HWND hwndHTMLControl;

#if (FRONTIERWEB==1)
tywebappInfo gwebappInfo = {false, NULL};

HANDLE webappStartup () {

	gwebappInfo.flactive = false;

	gwebappInfo.hwebappModule = LoadLibrary ("WEBAPPDLL.DLL");
	
	if (gwebappInfo.hwebappModule == NULL)
		gwebappInfo.hwebappModule = LoadLibrary ("DLLS\\WEBAPPDLL.DLL");
		
	if (gwebappInfo.hwebappModule == NULL)
		gwebappInfo.hwebappModule = LoadLibrary ("..\\DLLS\\WEBAPPDLL.DLL");

	if (gwebappInfo.hwebappModule != NULL) {
		gwebappInfo.webappSetup = (tywebappSetup) GetProcAddress (gwebappInfo.hwebappModule, "webappSetup");
		gwebappInfo.webappWndProc = (tywebappWndProc) GetProcAddress (gwebappInfo.hwebappModule, "webappWndProc");
		gwebappInfo.webappBack = (tywebappNoParam) GetProcAddress (gwebappInfo.hwebappModule, "webappBack");
		gwebappInfo.webappForward = (tywebappNoParam) GetProcAddress (gwebappInfo.hwebappModule, "webappForward");
		gwebappInfo.webappHome = (tywebappNoParam) GetProcAddress (gwebappInfo.hwebappModule, "webappHome");
		gwebappInfo.webappStop = (tywebappNoParam) GetProcAddress (gwebappInfo.hwebappModule, "webappStop");
		gwebappInfo.webappRefresh = (tywebappNoParam) GetProcAddress (gwebappInfo.hwebappModule, "webappRefresh");
		gwebappInfo.webappNavigate = (tywebappNavigate) GetProcAddress (gwebappInfo.hwebappModule, "webappNavigate");
		gwebappInfo.webappVersion = (tywebappVersion) GetProcAddress (gwebappInfo.hwebappModule, "webappVersion");
		gwebappInfo.webappIsOffline = (tywebappIsOffline) GetProcAddress (gwebappInfo.hwebappModule, "webappIsOffline");
		gwebappInfo.webappSetOffline = (tywebappSetOffline) GetProcAddress (gwebappInfo.hwebappModule, "webappSetOffline");
		}

	return (gwebappInfo.hwebappModule);
	} /*webappStartup*/

boolean webappShutdown () {
//	if ((gwebappInfo.webappSetup != NULL) && (gwebappInfo.webappWndProc != NULL))
//		(*(gcomServerInfo.comclear)) ();

	if (gwebappInfo.hwebappModule != NULL)
		FreeLibrary (gwebappInfo.hwebappModule);

	gwebappInfo.webappSetup = NULL;
	gwebappInfo.webappWndProc = NULL;
	gwebappInfo.hwebappModule = NULL;
	return (TRUE);
	} /*webappShutdown*/

long doweb (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//	long MDI;

//	MDI = GetWindowLong(hwndMDIClient, GWL_USERDATA);

//	if ((MDI != 0L) && (MDI == GetWindowLong(hwnd, GWL_USERDATA)))
		if (htmlcontrolactive())
			return ((*(gwebappInfo.webappWndProc))(hwnd, msg, wParam, lParam));
	return (1);
}

#endif

void htmlcontrolback () {
	#if (FRONTIERWEB==1)
		if (gwebappInfo.webappBack == NULL)
			return;

		//(*(gwebappInfo.webappBack))();

		releasethreadglobals ();

		SendMessage (hwndHTMLControl, WM_USER+500, 0, 0);

		grabthreadglobals ();
	#endif
	}

boolean htmlcontrolsetoffline (boolean floffline) {

	#if (FRONTIERWEB==1)

		/*releasethreadglobals ();

		SendMessage (hwndHTMLControl, WM_USER+501, floffline, 0);

		grabthreadglobals ();*/

	INTERNET_CONNECTED_INFO ci;

    memset(&ci, 0, sizeof(ci));
    if(floffline) {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    } else {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }

    InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));

	#endif

	return (true);
	}

	
void htmlcontrolforward () {
	#if (FRONTIERWEB==1)
		if (gwebappInfo.webappForward == NULL)
			return;

		(*(gwebappInfo.webappForward))();
	#endif
	}

void htmlcontrolrefresh () {
	#if (FRONTIERWEB==1)
		if (gwebappInfo.webappRefresh == NULL)
			return;

		(*(gwebappInfo.webappRefresh))();
	#endif
	}

void htmlcontrolhome () {
	#if (FRONTIERWEB==1)
		if (gwebappInfo.webappHome == NULL)
			return;

		(*(gwebappInfo.webappHome))();
	#endif
	}

void htmlcontrolstop () {
	#if (FRONTIERWEB==1)
		if (gwebappInfo.webappStop == NULL)
			return;

		(*(gwebappInfo.webappStop))();
	#endif
	}

void htmlcontrolnavigate (Handle htext) {
	#if (FRONTIERWEB==1)
		unsigned long len;
		char buff[2048];

		if (gwebappInfo.webappNavigate == NULL)
			return;

		len = gethandlesize (htext);

		memmove (buff, *htext, len);

		buff[len] = 0;

		releasethreadglobalsnopriority();

		(*(gwebappInfo.webappNavigate))(buff);

		grabthreadglobalsnopriority();
	#endif
	}

boolean htmlcontrolversion (unsigned short * majorVersion, unsigned short * minorVersion) {
	#if (FRONTIERWEB==1)

		unsigned long version;

		if (gwebappInfo.webappVersion != NULL)
			version = (*(gwebappInfo.webappVersion))();
		else
			version = 0L;

		*majorVersion = HIWORD(version);
		*minorVersion = LOWORD(version);
		return (true);

	#else

		*majorVersion = 0;
		*minorVersion = 0;
		return (false);

	#endif
	}

boolean htmlcontrolpresent () {
	#if (FRONTIERWEB==1)

		if (gwebappInfo.hwebappModule != NULL)
			return (true);
		else
			return (false);

	#else

		return (false);

	#endif
	}

boolean htmlcontrolactive () {
	#if (FRONTIERWEB==1)

		return (gwebappInfo.flactive);

	#else

		return (false);

	#endif
	}

boolean htmlcontrolisoffline (boolean * fl) {

	#if (FRONTIERWEB==1)


    DWORD   dwState = 0, dwSize = sizeof(DWORD);
    BOOL    fRet = FALSE;

    if(InternetQueryOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState,
        &dwSize))
    {
        if(dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
            fRet = TRUE;
    }

	*fl = fRet;

    return (true);


	/*	short bl;

		if (gwebappInfo.webappIsOffline == NULL)
			return (false);

		if ((*(gwebappInfo.webappIsOffline))(&bl)) {
			*fl = bl;
			return (true);
			}
		else {
			return (false);
			}*/
	#else
		return (false);
	#endif*/
	}

boolean htmlcontrolsetofflinehandler (boolean  fl) {
	#if (FRONTIERWEB==1)
		short bl;
		boolean floffline = false;

		if (gwebappInfo.webappSetOffline == NULL)
			return (false);

		bl = fl;
		releasethreadglobals ();

		floffline = (*(gwebappInfo.webappSetOffline)) (bl);
		
		grabthreadglobals ();

		return (floffline);

	#else
		return (false);
	#endif
	}


long CALLBACK htmlControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
#if (FRONTIERWEB==1)
	
	doweb (hwnd, msg, wParam, lParam);


	if (msg == WM_USER+500)
		(*(gwebappInfo).webappBack) ();

	if (msg == WM_USER+501)
		htmlcontrolsetofflinehandler (wParam);

	switch (msg) {
		case WM_WINDOWPOSCHANGING:
			{
			WINDOWPOS * wp;

			if (hwndMDIClient != NULL) {

				wp = (WINDOWPOS *) lParam;

				wp->flags |= SWP_NOZORDER;
				
				}
			break;
			}
		}
#endif

	return (DefMDIChildProc (hwnd, msg, wParam, lParam));
	}
