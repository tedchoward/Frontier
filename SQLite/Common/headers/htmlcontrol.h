
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

#if (FRONTIERWEB==1)
typedef long (WINAPI * tywebappSetup) (HINSTANCE hInstance, HWND hwnd, HWND hwndStatus, char * initialURL);
typedef void (WINAPI * tywebappNoParam) ();
typedef void (WINAPI * tywebappNavigate) (char * nameto);
typedef long (WINAPI * tywebappVersion) ();
typedef short (WINAPI * tywebappIsOffline) (short * fl);
typedef short (WINAPI * tywebappSetOffline) (short fl);

typedef LRESULT (WINAPI * tywebappWndProc) (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct tywebappInfo {
	boolean flactive;
	HMODULE hwebappModule;
	tywebappSetup webappSetup;
	tywebappWndProc webappWndProc;
	tywebappNoParam webappBack;
	tywebappNoParam webappForward;
	tywebappNoParam webappHome;
	tywebappNoParam webappStop;
	tywebappNoParam webappRefresh;
	tywebappNavigate webappNavigate;
	tywebappVersion webappVersion;
	tywebappIsOffline webappIsOffline;
	tywebappSetOffline webappSetOffline;
	} tywebappInfo;

extern tywebappInfo gwebappInfo;


HANDLE webappStartup ();
boolean webappShutdown ();
long doweb (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
long CALLBACK htmlControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void htmlcontrolback ();
void htmlcontrolforward ();
void htmlcontrolrefresh ();
void htmlcontrolhome ();
void htmlcontrolstop ();
void htmlcontrolnavigate (Handle htext);
boolean htmlcontrolversion (unsigned short * majorVersion, unsigned short * minorVersion);
boolean htmlcontrolpresent ();
boolean htmlcontrolactive ();
boolean htmlcontrolisoffline (boolean * fl);
boolean htmlcontrolsetoffline (boolean fl);
