# Microsoft Developer Studio Project File - Name="Frontier" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Frontier - Win32 DbTracker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Frontier.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Frontier.mak" CFG="Frontier - Win32 DbTracker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Frontier - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Frontier - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Frontier - Win32 MemTracker" (based on "Win32 (x86) Application")
!MESSAGE "Frontier - Win32 DbTracker" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Frontier/Projects/Winland", CNAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Frontier - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\_build\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Zp2 /MT /W3 /GX /Zi /Ob2 /I "..\Common\headers" /I "..\Common\SystemHeaders" /I "..\Common\Paige\headers" /I "..\Common\PCRE" /D "NDEBUG" /D FRONTIERCOM=1 /D "WIN95VERSION" /D "_WINDOWS" /D "WIN32" /D FRONTIERWEB=0 /D "WIN32_COMPILE" /FR /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\Common\headers" /i "..\Common\SystemHeaders" /i "..\Common\resources\Win32\res" /d "NDEBUG"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 odbc32.lib odbccp32.lib winmm.lib libcmt.lib oldnames.lib comctl32.lib wininet.lib PaigeWinMSCRT.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib /libpath:"..\Common\Paige"
# SUBTRACT LINK32 /profile /incremental:yes

!ELSEIF  "$(CFG)" == "Frontier - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\_build\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Zp2 /MTd /W3 /GX /Zi /Od /Ob2 /I "..\Common\headers" /I "..\Common\SystemHeaders" /I "..\Common\Paige\headers" /I "..\Common\PCRE" /D "_DEBUG" /D FRONTIERCOM=1 /D "WIN95VERSION" /D "_WINDOWS" /D "WIN32" /D "WIN32_COMPILE" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\Common\headers" /i "..\Common\SystemHeaders" /i "..\Common\resources\Win32\res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 PaigeWinMSCRT.lib winmm.lib libcmtd.lib oldnames.lib comctl32.lib wininet.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib /nologo /subsystem:windows /incremental:no /map /debug /machine:I386 /nodefaultlib /libpath:"..\Common\Paige"
# SUBTRACT LINK32 /verbose /profile /pdb:none

!ELSEIF  "$(CFG)" == "Frontier - Win32 MemTracker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Frontier"
# PROP BASE Intermediate_Dir "Frontier"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\MemTrack"
# PROP Intermediate_Dir ".\_build\MemTrack"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp2 /W3 /Gm /GX /Zi /I "..\..\PaigeSrc.2pf\pgHeader" /D "_DEBUG" /D "WIN95VERSION" /D "_WINDOWS" /D FRONTIERCOM=1 /D "WIN32" /FR /FD /c
# ADD CPP /nologo /Zp2 /MTd /W3 /GX /Zi /Od /I "..\Common\headers" /I "..\Common\SystemHeaders" /I "..\Common\Paige\headers" /I "..\Common\PCRE" /D "_DEBUG" /D MEMTRACKER=1 /D FRONTIERCOM=1 /D "WIN95VERSION" /D "_WINDOWS" /D "WIN32" /D "WIN32_COMPILE" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\Common\headers" /i "..\Common\SystemHeaders" /i "..\Common\resources\Win32\res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\..\paigeSrc.2pf\pgLib32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /map
# ADD LINK32 PaigeWinMSCRT.lib winmm.lib libcmtd.lib oldnames.lib comctl32.lib wininet.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib /nologo /subsystem:windows /incremental:no /map /debug /machine:I386 /nodefaultlib /libpath:"..\Common\Paige"

!ELSEIF  "$(CFG)" == "Frontier - Win32 DbTracker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Frontier___Win32_DbTracker"
# PROP BASE Intermediate_Dir "Frontier___Win32_DbTracker"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\DbTracker"
# PROP Intermediate_Dir ".\_build\DbTracker"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp2 /MTd /W3 /Gm /GX /Zi /I "..\Paigesrc.2pf\pgHeader\\" /D "_DEBUG" /D FRONTIERCOM=0 /D "WIN95VERSION" /D "_WINDOWS" /D "WIN32" /FR /FD /c
# ADD CPP /nologo /Zp2 /MTd /W3 /GX /Zi /Od /I "..\Common\headers" /I "..\Common\SystemHeaders" /I "..\Common\Paige\headers" /I "..\Common\PCRE" /D "_DEBUG" /D FRONTIERCOM=1 /D "WIN95VERSION" /D "_WINDOWS" /D "WIN32" /D DATABASE_DEBUG=1 /D "WIN32_COMPILE" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\Common\headers" /i "..\Common\SystemHeaders" /i "..\Common\resources\Win32\res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\paigeSrc.2pf\pgLib32.lib winmm.lib libcmtd.lib oldnames.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib /nologo /subsystem:windows /map /debug /debugtype:both /machine:I386 /nodefaultlib
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 PaigeWinMSCRT.lib winmm.lib libcmtd.lib oldnames.lib comctl32.lib wininet.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib /nologo /subsystem:windows /incremental:no /map /debug /machine:I386 /nodefaultlib /libpath:"..\Common\Paige"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Frontier - Win32 Release"
# Name "Frontier - Win32 Debug"
# Name "Frontier - Win32 MemTracker"
# Name "Frontier - Win32 DbTracker"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "file"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\file.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\filedialog.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\fileloop.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\filemp3.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\fileops.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\filepath.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\fileverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\findinfile.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\resources.c
# End Source File
# End Group
# Begin Group "pcre"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\PCRE\chartables.c
# End Source File
# Begin Source File

SOURCE=..\Common\PCRE\get.c
# End Source File
# Begin Source File

SOURCE=..\Common\PCRE\pcre.c
# End Source File
# Begin Source File

SOURCE=..\Common\PCRE\study.c
# End Source File
# End Group
# Begin Group "pict"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\pict.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\pictverbs.c
# End Source File
# End Group
# Begin Group "stubs_win"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\stubs\megastubs.c
# End Source File
# Begin Source File

SOURCE=..\Common\stubs\megastubs2.c
# End Source File
# Begin Source File

SOURCE=..\Common\stubs\minneed.c
# End Source File
# End Group
# Begin Group "tool"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\about.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\assert.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\base64.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\cursor.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\dockmenu.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\error.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\font.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\frontierconfig.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\frontierdebug.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\frontierstart.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\frontierwindows.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\htmlcontrol.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\icon.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\kb.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\launch.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\main.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\md5.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\memory.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\menu.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\mouse.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\notify.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\ops.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\popup.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\process.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\quickdraw.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\scrap.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\scripts.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\scrollbar.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\search.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\serialnumber.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\smallicon.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\sounds.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\textedit.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\threads.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\timedate.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\winregistry.c
# End Source File
# End Group
# Begin Group "_cancoon"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\cancoon.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\cancoonpopup.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\cancoonverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\cancoonwindow.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\command.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\miniwindow.c
# End Source File
# End Group
# Begin Group "_clay"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\claybrowserexpand.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\claybrowserstruc.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\claybrowservalidate.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\claycallbacks.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\claylinelayout.c
# End Source File
# End Group
# Begin Group "_db"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\db.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\dbstats.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\dbverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\odbengine.c
# End Source File
# End Group
# Begin Group "_lang"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Common\source\lang.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langcallbacks.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langdate.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langdll.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langerror.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langerrorwindow.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langevaluate.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langexternal.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langhash.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langhtml.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langlist.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langmath.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langops.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langpack.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langparser.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langpython.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langregexp.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langscan.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langstartup.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langsystypes.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langtmpstack.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langtrace.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langtree.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langvalue.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langwinipc.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\langxml.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\WinSockNetEvents.c
# End Source File
# End Group
# Begin Group "_menubar"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\menubar.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\menueditor.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\menufind.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\menupack.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\menuresize.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\menuverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\meprograms.c
# End Source File
# End Group
# Begin Group "_op"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\op.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opbuttons.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opdisplay.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opdraggingmove.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opedit.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opexpand.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\ophoist.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opicons.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opinit.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\oplangtext.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\oplineheight.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\oplist.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opops.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\oppack.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\oppopup.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opprint.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\oprefcon.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opscreenmap.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opscrollbar.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opstructure.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opvalidate.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opvisit.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\opxml.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\textdisplay.c
# End Source File
# End Group
# Begin Group "_shell"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\shell.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellactivate.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellblocker.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellbuttons.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellcallbacks.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellfile.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellhooks.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shelljuggler.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellkb.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellmenu.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellmouse.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellops.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellprint.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellscrap.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellscroll.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellsysverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellundo.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellupdate.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellwindow.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellwindowmenu.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\shellwindowverbs.c
# End Source File
# End Group
# Begin Group "_string"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\strings.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\stringverbs.c
# End Source File
# End Group
# Begin Group "_table"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\tablecompare.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tabledisplay.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tableedit.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tableexternal.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tablefind.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tableformats.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tableops.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tablepack.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tablepopup.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tablerunbutton.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tablescrap.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tablestructure.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tablevalidate.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tableverbs.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\tablewindow.c
# End Source File
# End Group
# Begin Group "_wp"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\Common\source\wpengine.c
# End Source File
# Begin Source File

SOURCE=..\Common\source\wpverbs.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\Common\source\FrontierWinMain.c
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ".rc,.bmp,.ico,.cur"
# Begin Source File

SOURCE=..\Common\resources\Win32\WinLand.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\Common\headers\about.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\BASE64.H
# End Source File
# Begin Source File

SOURCE=..\Common\headers\bitmaps.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\cancoon.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\cancooninternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\claybrowser.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\claybrowserexpand.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\claybrowserstruc.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\claybrowservalidate.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\claycallbacks.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\claylinelayout.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\command.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\cursor.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\db.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\dbinternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\dialogs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\dockmenu.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\error.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\FDllCall.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\file.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\filealias.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\fileloop.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\filesystem7.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\font.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\frontier.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\frontierconfig.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\frontierdebug.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\frontierdefs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\frontierstart.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\frontierwindows.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\FrontierWinMain.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\htmlcontrol.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\icon.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\iowaverbs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\kb.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\kernelverbdefs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\kernelverbs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\lang.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langdll.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langexternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langhtml.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langinternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langipc.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langmath.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langparser.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langpython.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langregexp.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langsystem7.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langtokens.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langwinipc.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langxcmd.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\langxml.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\launch.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\mac.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\macconv.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\md5.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\memory.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\menu.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\menubar.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\menueditor.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\menuinternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\menuverbs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\meprograms.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\miniwindow.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\mouse.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\notify.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\odbinternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\op.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\opbuttons.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\opdisplay.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\opdraggingmove.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\opicons.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\opinternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\oplineheight.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\oplist.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\ops.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\opscreenmap.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\opverbs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\opxml.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\osacomponent.h
# End Source File
# Begin Source File

SOURCE=..\Common\PCRE\pcre.h
# End Source File
# Begin Source File

SOURCE=..\Common\PCRE\pcre_config.h
# End Source File
# Begin Source File

SOURCE=..\Common\PCRE\pcre_internal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\pict.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\pictverbs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\popup.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\process.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\processinternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\quickdraw.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\resources.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\scrap.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\scripts.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\scrollbar.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\search.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\serialnumber.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\services.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shell.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shell.msvs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shell.rsrc.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shellbuttons.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shellhooks.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shellmenu.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shellprint.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shellprivate.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shelltypes.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\shellundo.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\smallicon.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\sounds.h
# End Source File
# Begin Source File

SOURCE=..\Common\SystemHeaders\standard.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\stdwin.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\stringdefs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\strings.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\tabledisplay.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\tableformats.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\tableinternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\tablestructure.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\tableverbs.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\textdisplay.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\textedit.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\threads.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\timedate.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\versions.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\windowlayout.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\WinLand.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\winregistry.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\WinSockNetEvents.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\wpengine.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\wpinternal.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\wpverbs.h
# End Source File
# Begin Source File

SOURCE=..\Common\SystemHeaders\WSE.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\yytab.h
# End Source File
# Begin Source File

SOURCE=..\Common\headers\zoom.h
# End Source File
# End Group
# End Target
# End Project
