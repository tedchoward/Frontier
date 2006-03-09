# $Id$
#
# Makefile - for Frontier
#
# UserLand Frontier(tm) -- High performance Web content management,
# object database, system-level and Internet scripting environment,
# including source code editing and debugging.
#
# Copyright (C) 1992-2004 UserLand Software, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

#================================================================================================
# System Definitions
#================================================================================================

SHELL = /bin/sh

PROJECT = Frontier

CC = gcc

RANLIB = ranlib

LD = ld

REZ = /Developer/Tools/Rez

RESMERGER = /Developer/Tools/ResMerger

RC = windres

#================================================================================================
# Directory Definitions
#================================================================================================

# Installation variables.  Spaces OK, only dir create and file copy operations.
runinst_prefix = /usr/local

bininst = ${runinst_prefix}/bin

# Internal variables. - No spaces allowed: libtool chokes on spaces in directory names.
TOPDIR = ..
SRCDIR = $(TOPDIR)
#OBJDIR = ./obj
OBJDIR = $(SRCDIR)
#BINDIR = $(TOPDIR)
BINDIR = .

INCLUDE_PATHS = \
	-I $(SRCDIR)/Common/headers/ \
	-I $(SRCDIR)/Common/IOAToolkit/ \
	-I $(SRCDIR)/Common/MoreFiles/ \
	-I $(SRCDIR)/Common/PCRE/ \
	-I $(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/ \
	-I $(SRCDIR)/FrontierSDK/Toolkits/IACTools/Headers/ \
	-I $(SRCDIR)/Common/SystemHeaders/ \
	-I $(SRCDIR)/Common/IowaRuntime/Headers/ \
	-I $(SRCDIR)/Common/UserLandIACToolkit/ \
	-I $(SRCDIR)/Common/Paige/headers/ \
	-I $(SRCDIR)/Common/source/

#MAC_INCLUDE_PATHS = \
#	-I /Developer/Headers/FlatCarbon

# Mac OS X Mach-O Application bundle
SRCROOT = .
BUILT_PRODUCTS_DIR = $(BINDIR)
CONTENTS_FOLDER_PATH = $(PROJECT).app/Contents
EXECUTABLE_FOLDER_PATH = $(PROJECT).app/Contents/MacOS
UNLOCALIZED_RESOURCES_FOLDER_PATH = $(PROJECT).app/Contents/Resources

#MS_SDK_INCLUDE_PATHS = \
#	-isystem c:/Program\ Files/Microsoft\ SDK/include/ \
#	-isystem c:/Program\ Files/Microsoft\ SDK/include/Win64/atl/ \
#	-isystem c:/Program\ Files/Microsoft\ SDK/include/Win64/crt/ \
#	-isystem c:/Program\ Files/Microsoft\ SDK/include/Win64/crt/sys/ \
#	-isystem c:/Program\ Files/Microsoft\ SDK/include/Win64/mfc/ \
#	-isystem c:/Program\ Files/Microsoft\ SDK/include/gl/ \
#	-isystem c:/Program\ Files/Microsoft\ SDK/include/mfc/

MS_SDK_INCLUDE_PATHS = \
	-I c:/Program\ Files/Microsoft\ SDK/include/ \
	-I c:/Program\ Files/Microsoft\ SDK/include/Win64/atl/ \
	-I c:/Program\ Files/Microsoft\ SDK/include/Win64/crt/ \
	-I c:/Program\ Files/Microsoft\ SDK/include/Win64/crt/sys/ \
	-I c:/Program\ Files/Microsoft\ SDK/include/Win64/mfc/ \
	-I c:/Program\ Files/Microsoft\ SDK/include/gl/ \
	-I c:/Program\ Files/Microsoft\ SDK/include/mfc/

WINE_INCLUDE_PATHS = \
	-I y:/Library/Darwine/include/wine/ \
	-I y:/Library/Darwine/include/wine/windows/ \
	-I y:/Library/Darwine/include/wine/msvcrt/ \
	-I y:/Library/Darwine/include/wine/msvcrt/sys

WIN_INCLUDE_PATHS = \
	-I ./ \
	$(MS_SDK_INCLUDE_PATHS)
	
#WIN_INCLUDE_PATHS = \
#	-I ./ \
#	-I /usr/include/sys/ \
#	-I /usr/include/w32api/

#WIN_INCLUDE_PATHS = \
#	-I ./ \
#	$(WINE_INCLUDE_PATHS)
	
#================================================================================================
# Compiler/Linker/Rez Options
#================================================================================================

# Prefix file - don't really need this - can set options directly in CFLAGS
PREFIX_FILE = -include frontier.gnu.h

# CFLAGS etc.
#CFLAGS = -Wall -Wno-switch -Wno-parentheses -Wno-unused -fpascal-strings -Wno-long-double $(PREFIX_FILE) $(INCLUDES)

MAC_CFLAGS = \
	-DMACVERSION=1 \
	-x c -arch ppc -pipe -std=c99 -Wno-trigraphs -fasm-blocks -fpascal-strings \
	-fshort-enums -fwritable-strings -mtune=G4 -Wmissing-prototypes -Wunknown-pragmas \
	-Wmost -Wno-four-char-constants -Wno-unknown-pragmas -Wno-long-double -fmessage-length=0 \
	-fno-inline -malign-mac68k $(PREFIX_FILE) $(INCLUDE_PATHS)

MAC_LDFLAGS = \
	-arch ppc -prebind -Wl,-no_arch_warnings -multiply_defined warning

MAC_REZ_OPTIONS = \
	-d SystemSevenOrLater=1 -d oldTemp \
	-append \
	-d TARGET_API_MAC_CARBON=1 -d FRONTIER_FRAMEWORK_INCLUDES=1 -i $(SRCDIR)/Common/headers

WIN_CFLAGS = \
	-DWIN95VERSION=1 \
	-mwindows -mno-cygwin -masm=intel \
	-x c -pipe -std=c99 -Wno-trigraphs \
	-fshort-enums -fwritable-strings -Wmissing-prototypes -Wunknown-pragmas \
	-Wall -Wno-parentheses -Wno-unknown-pragmas -fmessage-length=0 \
	$(PREFIX_FILE) $(INCLUDE_PATHS) $(WIN_INCLUDE_PATHS)
	
WIN_LDFLAGS = \
	-mwindows -mno-cygwin

WIN_REZ_OPTIONS = \
	-I $(SRCDIR)/Common/resources/Win32/ \
	-I $(SRCDIR)/Common/resources/Win32/res \
	$(WIN_INCLUDE_PATHS) $(INCLUDE_PATHS) \
	-O coff

DEBUGFLAGS = -g -O0
ifdef DMALLOC
DEBUGFLAGS += -DDMALLOC
endif

MAC_PAIGE_LIB_AR = \
	$(SRCDIR)/Common/Paige/libpaigemacho.a

MAC_PAIGE_LIB = \
	-L$(SRCDIR)/Common/Paige/ -lpaigemacho

WIN_PAIGE_LIB = \
	-L$(SRCDIR)/Common/Paige/ -lPaigeWin.lib

MAC_FRAMEWORKS = \
	-framework Carbon \
	-framework System \
	-framework ApplicationServices \
	-framework QuickTime

MAC_LIBS = $(MAC_FRAMEWORKS) $(MAC_PAIGE_LIB)

WIN_LIBS = -lc $(WIN_PAIGE_LIB)

DEBUGLIBS = -ldmalloc

#================================================================================================
# Headers Definitions
#================================================================================================

INCLUDE_FILES = \
	$(SRCDIR)/Common/IOAToolkit/ioa.h \
	$(SRCDIR)/Common/IOAToolkit/ioacolorpopup.h \
	$(SRCDIR)/Common/IOAToolkit/ioaedittext.h \
	$(SRCDIR)/Common/IOAToolkit/ioaicon.h \
	$(SRCDIR)/Common/IOAToolkit/ioapopup.h \
	$(SRCDIR)/Common/IOAToolkit/ioascrollbar.h \
	$(SRCDIR)/Common/MoreFiles/MoreFilesX.h \
	$(SRCDIR)/Common/PCRE/pcre.h \
	$(SRCDIR)/Common/headers/frontier.h \
	$(SRCDIR)/Common/headers/font.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletdefs.h \
	$(SRCDIR)/Common/SystemHeaders/standard.h \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Headers/iac.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletmemory.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletstrings.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletfiles.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletprocess.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletfrontier.h \
	$(SRCDIR)/Common/headers/aeutils.h \
	$(SRCDIR)/Common/SystemHeaders/uisinternal.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletbits.h \
	$(SRCDIR)/Common/headers/quickdraw.h \
	$(SRCDIR)/Common/headers/memory.h \
	$(SRCDIR)/Common/headers/file.h \
	$(SRCDIR)/Common/headers/launch.h \
	$(SRCDIR)/Common/headers/shelltypes.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletfont.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletquickdraw.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/applettextedit.h \
	$(SRCDIR)/Common/headers/error.h \
	$(SRCDIR)/Common/headers/strings.h \
	$(SRCDIR)/Common/headers/shell.rsrc.h \
	$(SRCDIR)/Common/headers/resources.h \
	$(SRCDIR)/Common/headers/lang.h \
	$(SRCDIR)/Common/headers/langinternal.h \
	$(SRCDIR)/Common/headers/ops.h \
	$(SRCDIR)/Common/headers/langexternal.h \
	$(SRCDIR)/Common/headers/langsystem7.h \
	$(SRCDIR)/Common/headers/process.h \
	$(SRCDIR)/Common/headers/kernelverbs.h \
	$(SRCDIR)/Common/headers/filesystem7.h \
	$(SRCDIR)/Common/headers/filealias.h \
	$(SRCDIR)/Common/headers/tablestructure.h \
	$(SRCDIR)/Common/headers/kernelverbdefs.h \
	$(SRCDIR)/Common/headers/winregistry.h \
	$(SRCDIR)/Common/headers/cursor.h \
	$(SRCDIR)/Common/headers/dialogs.h \
	$(SRCDIR)/Common/headers/timedate.h \
	$(SRCDIR)/Common/headers/frontierwindows.h \
	$(SRCDIR)/Common/headers/shell.h \
	$(SRCDIR)/Common/SystemHeaders/SetUpA5.h \
	$(SRCDIR)/Common/headers/oplist.h \
	$(SRCDIR)/Common/headers/fileloop.h \
	$(SRCDIR)/Common/headers/mac.h \
	$(SRCDIR)/Common/headers/scrap.h \
	$(SRCDIR)/Common/headers/threads.h \
	$(SRCDIR)/Common/MoreFiles/MoreFilesExtras.h \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Headers/iacinternal.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appleticons.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletkb.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletops.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletmenuops.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/applet.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowacore.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowascript.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowaparser.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowafrontier.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowaattributes.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowaruntime.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletresource.h \
	$(SRCDIR)/Common/SystemHeaders/UserTalk.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletfiledesktop.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowainit.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletmouse.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletcursor.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletcardops.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletzoom.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletcard.h \
	$(SRCDIR)/Common/UserLandIACToolkit/landinternal.h \
	$(SRCDIR)/Common/headers/search.h \
	$(SRCDIR)/Common/headers/shellprint.h \
	$(SRCDIR)/Common/headers/shellundo.h \
	$(SRCDIR)/Common/headers/cancoon.h \
	$(SRCDIR)/Common/headers/pict.h \
	$(SRCDIR)/Common/headers/pictverbs.h \
	$(SRCDIR)/Common/headers/bitmaps.h \
	$(SRCDIR)/Common/headers/mouse.h \
	$(SRCDIR)/Common/headers/scrollbar.h \
	$(SRCDIR)/Common/headers/zoom.h \
	$(SRCDIR)/Common/headers/shellhooks.h \
	$(SRCDIR)/Common/headers/menu.h \
	$(SRCDIR)/Common/headers/popup.h \
	$(SRCDIR)/Common/headers/meprograms.h \
	$(SRCDIR)/Common/headers/dockmenu.h \
	$(SRCDIR)/Common/headers/opinternal.h \
	$(SRCDIR)/Common/headers/menuverbs.h \
	$(SRCDIR)/Common/headers/kb.h \
	$(SRCDIR)/Common/headers/shellmenu.h \
	$(SRCDIR)/Common/SystemHeaders/land.h \
	$(SRCDIR)/Common/headers/BASE64.h \
	$(SRCDIR)/Common/headers/versions.h \
	$(SRCDIR)/Common/headers/icon.h \
	$(SRCDIR)/Common/headers/smallicon.h \
	$(SRCDIR)/Common/headers/textedit.h \
	$(SRCDIR)/Common/headers/windowlayout.h \
	$(SRCDIR)/Common/headers/shellprivate.h \
	$(SRCDIR)/Common/headers/about.h \
	$(SRCDIR)/Common/headers/cancooninternal.h \
	$(SRCDIR)/Common/headers/processinternal.h \
	$(SRCDIR)/Common/headers/WinLand.h \
	$(SRCDIR)/Common/headers/OSXSpecifics.h \
	$(SRCDIR)/Common/headers/player.h \
	$(SRCDIR)/Common/headers/frontierconfig.h \
	$(SRCDIR)/Common/headers/db.h \
	$(SRCDIR)/Common/headers/tableverbs.h \
	$(SRCDIR)/Common/headers/opverbs.h \
	$(SRCDIR)/Common/headers/scripts.h \
	$(SRCDIR)/Common/headers/wpverbs.h \
	$(SRCDIR)/Common/headers/command.h \
	$(SRCDIR)/Common/headers/osacomponent.h \
	$(SRCDIR)/Common/headers/iowaverbs.h \
	$(SRCDIR)/Common/headers/frontierstart.h \
	$(SRCDIR)/Common/headers/frontierdebug.h \
	$(SRCDIR)/Common/Paige/headers/PAIGE.h \
	$(SRCDIR)/Common/Paige/headers/PGDEFTBL.h \
	$(SRCDIR)/Common/headers/notify.h \
	$(SRCDIR)/Common/headers/md5.h \
	$(SRCDIR)/Common/headers/services.h \
	$(SRCDIR)/Common/headers/serialnumber.h \
	$(SRCDIR)/Common/headers/langregexp.h \
	$(SRCDIR)/Common/headers/shellbuttons.h \
	$(SRCDIR)/Common/headers/op.h \
	$(SRCDIR)/Common/headers/menueditor.h \
	$(SRCDIR)/Common/headers/tableinternal.h \
	$(SRCDIR)/Common/headers/sounds.h \
	$(SRCDIR)/Common/SystemHeaders/uisharing.h \
	$(SRCDIR)/Common/headers/miniwindow.h \
	$(SRCDIR)/Common/headers/wpengine.h \
	$(SRCDIR)/Common/headers/tableformats.h \
	$(SRCDIR)/Common/headers/langipc.h \
	$(SRCDIR)/Common/headers/dbinternal.h \
	$(SRCDIR)/Common/headers/WinSockNetEvents.h \
	$(SRCDIR)/Common/headers/opdisplay.h \
	$(SRCDIR)/Common/headers/opicons.h \
	$(SRCDIR)/Common/headers/claybrowser.h \
	$(SRCDIR)/Common/headers/claybrowserstruc.h \
	$(SRCDIR)/Common/headers/claybrowserexpand.h \
	$(SRCDIR)/Common/headers/claycallbacks.h \
	$(SRCDIR)/Common/headers/claybrowservalidate.h \
	$(SRCDIR)/Common/headers/oplineheight.h \
	$(SRCDIR)/Common/headers/odbinternal.h \
	$(SRCDIR)/Common/headers/CallMachOFrameWork.h \
	$(SRCDIR)/Common/headers/sysshellcall.h \
	$(SRCDIR)/Common/headers/langhtml.h \
	$(SRCDIR)/Common/headers/langxml.h \
	$(SRCDIR)/Common/headers/langxcmd.h \
	$(SRCDIR)/Common/headers/langdll.h \
	$(SRCDIR)/Common/headers/langtokens.h \
	$(SRCDIR)/Common/headers/langwinipc.h \
	$(SRCDIR)/Common/headers/htmlcontrol.h \
	$(SRCDIR)/Common/headers/langpython.h \
	$(SRCDIR)/Common/PCRE/pcre_internal.h \
	$(SRCDIR)/Common/headers/yytab.h \
	$(SRCDIR)/Common/headers/langparser.h \
	$(SRCDIR)/Common/headers/menubar.h \
	$(SRCDIR)/Common/source/iso8859.c \
	$(SRCDIR)/Common/headers/FrontierWinMain.h \
	$(SRCDIR)/Common/headers/menuinternal.h \
	$(SRCDIR)/Common/headers/textdisplay.h \
	$(SRCDIR)/Common/headers/opxml.h \
	$(SRCDIR)/Common/headers/opbuttons.h \
	$(SRCDIR)/Common/headers/opdraggingmove.h \
	$(SRCDIR)/Common/headers/osadroplet.h \
	$(SRCDIR)/Common/headers/osainternal.h \
	$(SRCDIR)/Common/headers/osamenus.h \
	$(SRCDIR)/Common/headers/osaparseaete.h \
	$(SRCDIR)/Common/headers/osawindows.h \
	$(SRCDIR)/Common/headers/tabledisplay.h \
	$(SRCDIR)/Common/headers/claylinelayout.h \
	$(SRCDIR)/Common/Paige/headers/PGTRAPS.h \
	$(SRCDIR)/Common/Paige/headers/PGEXCEPS.h \
	$(SRCDIR)/Common/Paige/headers/DEFPROCS.h \
	$(SRCDIR)/Common/Paige/headers/MACHINE.h \
	$(SRCDIR)/Common/Paige/headers/pgHLevel.h \
	$(SRCDIR)/Common/headers/wpinternal.h \
	$(SRCDIR)/Common/SystemHeaders/WSE.h \
	$(SRCDIR)/Common/PCRE/chartables.c \
	$(SRCDIR)/Common/headers/osincludes.h \
	$(SRCDIR)/Common/headers/macconv.h \
	$(SRCDIR)/Common/headers/stringdefs.h \
	$(SRCDIR)/Common/MoreFiles/Optimization.h \
	$(SRCDIR)/Common/MoreFiles/OptimizationEnd.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletundo.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletpalette.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletwires.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletscrollbar.h \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Headers/appletpopup.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowavalidate.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowa.h \
	$(SRCDIR)/Common/IowaRuntime/Headers/iowaprefs.h \
	$(SRCDIR)/Common/headers/opscreenmap.h \
	$(SRCDIR)/Common/Paige/headers/pgMemMgr.h \
	$(SRCDIR)/Common/Paige/headers/Userdefs.h \
	$(SRCDIR)/Common/Paige/headers/CPUDEFS.h \
	$(SRCDIR)/Common/Paige/headers/pgTxr.h \
	$(SRCDIR)/Common/headers/FDllCall.h \
	$(SRCDIR)/Common/PCRE/pcre_config.h \
	$(SRCDIR)/Common/Paige/headers/PGSETJMP.h \
	$(SRCDIR)/Common/Paige/headers/pgMTraps.h \
	$(SRCDIR)/Common/Paige/headers/PGERRORS.h \
	$(SRCDIR)/Common/headers/frontierdefs.h \
	$(SRCDIR)/Common/headers/langmath.h \
	$(SRCDIR)/Common/headers/FastTimes.h

#================================================================================================
# Sources and Objects Definitions
#================================================================================================

MAC_APPLET_SRC = \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Source/appletbits.c \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Source/appletfrontier.c \
	$(SRCDIR)/Common/source/appletglue.c \
	$(SRCDIR)/FrontierSDK/Toolkits/AppletToolkit/Source/applettextedit.c

MAC_APPLET_OBJS = \
	$(OBJDIR)/FrontierSDK/Toolkits/AppletToolkit/Source/appletbits.o \
	$(OBJDIR)/FrontierSDK/Toolkits/AppletToolkit/Source/appletfrontier.o \
	$(OBJDIR)/Common/source/appletglue.o \
	$(OBJDIR)/FrontierSDK/Toolkits/AppletToolkit/Source/applettextedit.o

MAC_FILE_SRC = \
	$(SRCDIR)/Common/source/filealias.c \
	$(SRCDIR)/Common/source/filelaunch.c

MAC_FILE_OBJS = \
	$(OBJDIR)/Common/source/filealias.o \
	$(OBJDIR)/Common/source/filelaunch.o

FILE_SRC = \
	$(SRCDIR)/Common/source/file.c \
	$(SRCDIR)/Common/source/filedialog.c \
	$(SRCDIR)/Common/source/fileloop.c \
	$(SRCDIR)/Common/source/filemp3.c \
	$(SRCDIR)/Common/source/fileops.c \
	$(SRCDIR)/Common/source/filepath.c \
	$(SRCDIR)/Common/source/fileverbs.c \
	$(SRCDIR)/Common/source/findinfile.c \
	$(SRCDIR)/Common/source/resources.c

FILE_OBJS = \
	$(OBJDIR)/Common/source/file.o \
	$(OBJDIR)/Common/source/filedialog.o \
	$(OBJDIR)/Common/source/fileloop.o \
	$(OBJDIR)/Common/source/filemp3.o \
	$(OBJDIR)/Common/source/fileops.o \
	$(OBJDIR)/Common/source/filepath.o \
	$(OBJDIR)/Common/source/fileverbs.o \
	$(OBJDIR)/Common/source/findinfile.o \
	$(OBJDIR)/Common/source/resources.o

MAC_IAC_SRC = \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iac.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacapps.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacasynch.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacbinary.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacboolean.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacdate.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacdouble.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacfilespec.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iachandler.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iaclist.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iaclong.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacnetwork.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacnewsystemverb.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacops.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacpoint.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacreceive.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacrect.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacrecord.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacrgb.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacsend.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacshort.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacstring.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacstring4.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iacsysevents.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iactable.c \
	$(SRCDIR)/FrontierSDK/Toolkits/IACTools/Source/iactext.c

MAC_IAC_OBJS = \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iac.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacapps.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacasynch.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacbinary.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacboolean.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacdate.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacdouble.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacfilespec.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iachandler.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iaclist.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iaclong.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacnetwork.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacnewsystemverb.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacops.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacpoint.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacreceive.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacrect.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacrecord.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacrgb.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacsend.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacshort.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacstring.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacstring4.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iacsysevents.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iactable.o \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/iactext.o

MAC_IOA_SRC = \
	$(SRCDIR)/Common/IOAToolkit/ioa.c \
	$(SRCDIR)/Common/IOAToolkit/ioabutton.c \
	$(SRCDIR)/Common/IOAToolkit/ioacheckbox.c \
	$(SRCDIR)/Common/IOAToolkit/ioacolorpopup.c \
	$(SRCDIR)/Common/IOAToolkit/ioaedittext.c \
	$(SRCDIR)/Common/IOAToolkit/ioaframe.c \
	$(SRCDIR)/Common/IOAToolkit/ioaicon.c \
	$(SRCDIR)/Common/IOAToolkit/ioapicture.c \
	$(SRCDIR)/Common/IOAToolkit/ioapopup.c \
	$(SRCDIR)/Common/IOAToolkit/ioaradio.c \
	$(SRCDIR)/Common/IOAToolkit/ioarect.c \
	$(SRCDIR)/Common/IOAToolkit/ioascrollbar.c \
	$(SRCDIR)/Common/IOAToolkit/ioastatic.c

MAC_IOA_OBJS = \
	$(OBJDIR)/Common/IOAToolkit/ioa.o \
	$(OBJDIR)/Common/IOAToolkit/ioabutton.o \
	$(OBJDIR)/Common/IOAToolkit/ioacheckbox.o \
	$(OBJDIR)/Common/IOAToolkit/ioacolorpopup.o \
	$(OBJDIR)/Common/IOAToolkit/ioaedittext.o \
	$(OBJDIR)/Common/IOAToolkit/ioaframe.o \
	$(OBJDIR)/Common/IOAToolkit/ioaicon.o \
	$(OBJDIR)/Common/IOAToolkit/ioapicture.o \
	$(OBJDIR)/Common/IOAToolkit/ioapopup.o \
	$(OBJDIR)/Common/IOAToolkit/ioaradio.o \
	$(OBJDIR)/Common/IOAToolkit/ioarect.o \
	$(OBJDIR)/Common/IOAToolkit/ioascrollbar.o \
	$(OBJDIR)/Common/IOAToolkit/ioastatic.o

MAC_IOWA_SRC = \
	$(SRCDIR)/Common/IowaRuntime/Source/iowaattributes.c \
	$(SRCDIR)/Common/IowaRuntime/Source/iowacomponent.c \
	$(SRCDIR)/Common/IowaRuntime/Source/iowacore.c \
	$(SRCDIR)/Common/IowaRuntime/Source/iowafrontier.c \
	$(SRCDIR)/Common/IowaRuntime/Source/iowainit.c \
	$(SRCDIR)/Common/IowaRuntime/Source/iowaparser.c \
	$(SRCDIR)/Common/IowaRuntime/Source/iowaruntime.c \
	$(SRCDIR)/Common/IowaRuntime/Source/iowascript.c \
	$(SRCDIR)/Common/IowaRuntime/Source/iowavalidate.c

MAC_IOWA_OBJS = \
	$(OBJDIR)/Common/IowaRuntime/Source/iowaattributes.o \
	$(OBJDIR)/Common/IowaRuntime/Source/iowacomponent.o \
	$(OBJDIR)/Common/IowaRuntime/Source/iowacore.o \
	$(OBJDIR)/Common/IowaRuntime/Source/iowafrontier.o \
	$(OBJDIR)/Common/IowaRuntime/Source/iowainit.o \
	$(OBJDIR)/Common/IowaRuntime/Source/iowaparser.o \
	$(OBJDIR)/Common/IowaRuntime/Source/iowaruntime.o \
	$(OBJDIR)/Common/IowaRuntime/Source/iowascript.o \
	$(OBJDIR)/Common/IowaRuntime/Source/iowavalidate.o

MAC_LAND_SRC = \
	$(SRCDIR)/Common/UserLandIACToolkit/land.c \
	$(SRCDIR)/Common/UserLandIACToolkit/landgetparam.c \
	$(SRCDIR)/Common/UserLandIACToolkit/landops.c \
	$(SRCDIR)/Common/UserLandIACToolkit/landpush.c \
	$(SRCDIR)/Common/UserLandIACToolkit/landqueue.c \
	$(SRCDIR)/Common/UserLandIACToolkit/landreturn.c \
	$(SRCDIR)/Common/UserLandIACToolkit/landsystem7.c \
	$(SRCDIR)/Common/UserLandIACToolkit/landverbarray.c

MAC_LAND_OBJS = \
	$(OBJDIR)/Common/UserLandIACToolkit/land.o \
	$(OBJDIR)/Common/UserLandIACToolkit/landgetparam.o \
	$(OBJDIR)/Common/UserLandIACToolkit/landops.o \
	$(OBJDIR)/Common/UserLandIACToolkit/landpush.o \
	$(OBJDIR)/Common/UserLandIACToolkit/landqueue.o \
	$(OBJDIR)/Common/UserLandIACToolkit/landreturn.o \
	$(OBJDIR)/Common/UserLandIACToolkit/landsystem7.o \
	$(OBJDIR)/Common/UserLandIACToolkit/landverbarray.o

MAC_MOREFILES_CLASSIC_SRC = \
	$(SRCDIR)/Common/MoreFiles/MoreFilesExtras.c

MAC_MOREFILES_CLASSIC_OBJS = \
	$(OBJDIR)/Common/MoreFiles/MoreFilesExtras.o

MAC_MOREFILES_SRC = \
	$(SRCDIR)/Common/MoreFiles/MoreFilesX.c

MAC_MOREFILES_OBJS = \
	$(OBJDIR)/Common/MoreFiles/MoreFilesX.o

PCRE_SRC = \
	$(SRCDIR)/Common/PCRE/get.c \
	$(SRCDIR)/Common/PCRE/maketables.c \
	$(SRCDIR)/Common/PCRE/pcre.c \
	$(SRCDIR)/Common/PCRE/study.c

PCRE_OBJS = \
	$(OBJDIR)/Common/PCRE/get.o \
	$(OBJDIR)/Common/PCRE/maketables.o \
	$(OBJDIR)/Common/PCRE/pcre.o \
	$(OBJDIR)/Common/PCRE/study.o

PICT_SRC = \
	$(SRCDIR)/Common/source/pict.c \
	$(SRCDIR)/Common/source/pictverbs.c

PICT_OBJS = \
	$(OBJDIR)/Common/source/pict.o \
	$(OBJDIR)/Common/source/pictverbs.o

WIN_STUBS_SRC = \
	$(SRCDIR)/Common/stubs/minneed.c \
	$(SRCDIR)/Common/stubs/megastubs2.c \
	$(SRCDIR)/Common/stubs/megastubs.c

WIN_STUBS_OBJS = \
	$(OBJDIR)/Common/stubs/minneed.o \
	$(OBJDIR)/Common/stubs/megastubs2.o \
	$(OBJDIR)/Common/stubs/megastubs.o

MAC_TOOL_SRC = \
	$(SRCDIR)/Common/source/aeutils.c \
	$(SRCDIR)/Common/source/bitmaps.c \
	$(SRCDIR)/Common/source/dialogs.c \
	$(SRCDIR)/Common/source/FastTimes.c \
	$(SRCDIR)/Common/source/mac.c \
	$(SRCDIR)/Common/source/player.c \
	$(SRCDIR)/Common/source/services.c \
	$(SRCDIR)/Common/source/uisharing.c \
	$(SRCDIR)/Common/source/zoom.c

MAC_TOOL_OBJS = \
	$(OBJDIR)/Common/source/aeutils.o \
	$(OBJDIR)/Common/source/bitmaps.o \
	$(OBJDIR)/Common/source/dialogs.o \
	$(OBJDIR)/Common/source/FastTimes.o \
	$(OBJDIR)/Common/source/mac.o \
	$(OBJDIR)/Common/source/player.o \
	$(OBJDIR)/Common/source/services.o \
	$(OBJDIR)/Common/source/uisharing.o \
	$(OBJDIR)/Common/source/zoom.o

WIN_TOOL_SRC = \
	$(SRCDIR)/Common/source/htmlcontrol.c \
	$(SRCDIR)/Common/source/winregistry.c

WIN_TOOL_OBJS = \
	$(OBJDIR)/Common/source/htmlcontrol.o \
	$(OBJDIR)/Common/source/winregistry.o

TOOL_SRC = \
	$(SRCDIR)/Common/source/about.c \
	$(SRCDIR)/Common/source/assert.c \
	$(SRCDIR)/Common/source/base64.c \
	$(SRCDIR)/Common/source/cursor.c \
	$(SRCDIR)/Common/source/dockmenu.c \
	$(SRCDIR)/Common/source/error.c \
	$(SRCDIR)/Common/source/font.c \
	$(SRCDIR)/Common/source/frontierconfig.c \
	$(SRCDIR)/Common/source/frontierdebug.c \
	$(SRCDIR)/Common/source/frontierstart.c \
	$(SRCDIR)/Common/source/frontierwindows.c \
	$(SRCDIR)/Common/source/icon.c \
	$(SRCDIR)/Common/source/kb.c \
	$(SRCDIR)/Common/source/launch.c \
	$(SRCDIR)/Common/source/main.c \
	$(SRCDIR)/Common/source/md5.c \
	$(SRCDIR)/Common/source/memory.c \
	$(SRCDIR)/Common/source/menu.c \
	$(SRCDIR)/Common/source/mouse.c \
	$(SRCDIR)/Common/source/notify.c \
	$(SRCDIR)/Common/source/ops.c \
	$(SRCDIR)/Common/source/popup.c \
	$(SRCDIR)/Common/source/process.c \
	$(SRCDIR)/Common/source/quickdraw.c \
	$(SRCDIR)/Common/source/scrap.c \
	$(SRCDIR)/Common/source/scripts.c \
	$(SRCDIR)/Common/source/scrollbar.c \
	$(SRCDIR)/Common/source/search.c \
	$(SRCDIR)/Common/source/serialnumber.c \
	$(SRCDIR)/Common/source/smallicon.c \
	$(SRCDIR)/Common/source/sounds.c \
	$(SRCDIR)/Common/source/sysshellcall.c \
	$(SRCDIR)/Common/source/textedit.c \
	$(SRCDIR)/Common/source/threads.c \
	$(SRCDIR)/Common/source/timedate.c

TOOL_OBJS = \
	$(OBJDIR)/Common/source/about.o \
	$(OBJDIR)/Common/source/assert.o \
	$(OBJDIR)/Common/source/base64.o \
	$(OBJDIR)/Common/source/cursor.o \
	$(OBJDIR)/Common/source/dockmenu.o \
	$(OBJDIR)/Common/source/error.o \
	$(OBJDIR)/Common/source/font.o \
	$(OBJDIR)/Common/source/frontierconfig.o \
	$(OBJDIR)/Common/source/frontierdebug.o \
	$(OBJDIR)/Common/source/frontierstart.o \
	$(OBJDIR)/Common/source/frontierwindows.o \
	$(OBJDIR)/Common/source/icon.o \
	$(OBJDIR)/Common/source/kb.o \
	$(OBJDIR)/Common/source/launch.o \
	$(OBJDIR)/Common/source/main.o \
	$(OBJDIR)/Common/source/md5.o \
	$(OBJDIR)/Common/source/memory.o \
	$(OBJDIR)/Common/source/menu.o \
	$(OBJDIR)/Common/source/mouse.o \
	$(OBJDIR)/Common/source/notify.o \
	$(OBJDIR)/Common/source/ops.o \
	$(OBJDIR)/Common/source/popup.o \
	$(OBJDIR)/Common/source/process.o \
	$(OBJDIR)/Common/source/quickdraw.o \
	$(OBJDIR)/Common/source/scrap.o \
	$(OBJDIR)/Common/source/scripts.o \
	$(OBJDIR)/Common/source/scrollbar.o \
	$(OBJDIR)/Common/source/search.o \
	$(OBJDIR)/Common/source/serialnumber.o \
	$(OBJDIR)/Common/source/smallicon.o \
	$(OBJDIR)/Common/source/sounds.o \
	$(OBJDIR)/Common/source/sysshellcall.o \
	$(OBJDIR)/Common/source/textedit.o \
	$(OBJDIR)/Common/source/threads.o \
	$(OBJDIR)/Common/source/timedate.o

CANCOON_SRC = \
	$(SRCDIR)/Common/source/cancoon.c \
	$(SRCDIR)/Common/source/cancoonpopup.c \
	$(SRCDIR)/Common/source/cancoonverbs.c \
	$(SRCDIR)/Common/source/cancoonwindow.c \
	$(SRCDIR)/Common/source/command.c \
	$(SRCDIR)/Common/source/miniwindow.c

CANCOON_OBJS = \
	$(OBJDIR)/Common/source/cancoon.o \
	$(OBJDIR)/Common/source/cancoonpopup.o \
	$(OBJDIR)/Common/source/cancoonverbs.o \
	$(OBJDIR)/Common/source/cancoonwindow.o \
	$(OBJDIR)/Common/source/command.o \
	$(OBJDIR)/Common/source/miniwindow.o

CLAY_SRC = \
	$(SRCDIR)/Common/source/claybrowserexpand.c \
	$(SRCDIR)/Common/source/claybrowserstruc.c \
	$(SRCDIR)/Common/source/claybrowservalidate.c \
	$(SRCDIR)/Common/source/claycallbacks.c \
	$(SRCDIR)/Common/source/claylinelayout.c

CLAY_OBJS = \
	$(OBJDIR)/Common/source/claybrowserexpand.o \
	$(OBJDIR)/Common/source/claybrowserstruc.o \
	$(OBJDIR)/Common/source/claybrowservalidate.o \
	$(OBJDIR)/Common/source/claycallbacks.o \
	$(OBJDIR)/Common/source/claylinelayout.o

DB_SRC = \
	$(SRCDIR)/Common/source/db.c \
	$(SRCDIR)/Common/source/dbstats.c \
	$(SRCDIR)/Common/source/dbverbs.c \
	$(SRCDIR)/Common/source/odbengine.c

DB_OBJS = \
	$(OBJDIR)/Common/source/db.o \
	$(OBJDIR)/Common/source/dbstats.o \
	$(OBJDIR)/Common/source/dbverbs.o \
	$(OBJDIR)/Common/source/odbengine.o

MAC_LAMG_CLASSIC_SRC = \
	$(SRCDIR)/Common/source/langxcmd.c

MAC_LAMG_CLASSIC_OBJS = \
	$(OBJDIR)/Common/source/langxcmd.o

MAC_LANG_SRC = \
	$(SRCDIR)/Common/source/CallMachOFrameWork.c \
	$(SRCDIR)/Common/source/langcard.c \
	$(SRCDIR)/Common/source/langdialog.c \
	$(SRCDIR)/Common/source/langipc.c \
	$(SRCDIR)/Common/source/langipcmenus.c \
	$(SRCDIR)/Common/source/langmodeless.c \
	$(SRCDIR)/Common/source/langquicktime.c \
	$(SRCDIR)/Common/source/OpenTransportNetEvents.c

MAC_LANG_OBJS = \
	$(OBJDIR)/Common/source/CallMachOFrameWork.o \
	$(OBJDIR)/Common/source/langcard.o \
	$(OBJDIR)/Common/source/langdialog.o \
	$(OBJDIR)/Common/source/langipc.o \
	$(OBJDIR)/Common/source/langipcmenus.o \
	$(OBJDIR)/Common/source/langmodeless.o \
	$(OBJDIR)/Common/source/langquicktime.o \
	$(OBJDIR)/Common/source/OpenTransportNetEvents.o

WIN_LANG_SRC = \
	$(SRCDIR)/Common/source/WinSockNetEvents.c

WIN_LANG_OBJS = \
	$(OBJDIR)/Common/source/WinSockNetEvents.o

LANG_SRC = \
	$(SRCDIR)/Common/source/lang.c \
	$(SRCDIR)/Common/source/langcallbacks.c \
	$(SRCDIR)/Common/source/langdate.c \
	$(SRCDIR)/Common/source/langdll.c \
	$(SRCDIR)/Common/source/langerror.c \
	$(SRCDIR)/Common/source/langerrorwindow.c \
	$(SRCDIR)/Common/source/langevaluate.c \
	$(SRCDIR)/Common/source/langexternal.c \
	$(SRCDIR)/Common/source/langhash.c \
	$(SRCDIR)/Common/source/langhtml.c \
	$(SRCDIR)/Common/source/langlist.c \
	$(SRCDIR)/Common/source/langmath.c \
	$(SRCDIR)/Common/source/langops.c \
	$(SRCDIR)/Common/source/langpack.c \
	$(SRCDIR)/Common/source/langparser.c \
	$(SRCDIR)/Common/source/langregexp.c \
	$(SRCDIR)/Common/source/langpython.c \
	$(SRCDIR)/Common/source/langscan.c \
	$(SRCDIR)/Common/source/langstartup.c \
	$(SRCDIR)/Common/source/langsystypes.c \
	$(SRCDIR)/Common/source/langtmpstack.c \
	$(SRCDIR)/Common/source/langtrace.c \
	$(SRCDIR)/Common/source/langtree.c \
	$(SRCDIR)/Common/source/langvalue.c \
	$(SRCDIR)/Common/source/langverbs.c \
	$(SRCDIR)/Common/source/langwinipc.c \
	$(SRCDIR)/Common/source/langxml.c

LANG_OBJS = \
	$(OBJDIR)/Common/source/lang.o \
	$(OBJDIR)/Common/source/langcallbacks.o \
	$(OBJDIR)/Common/source/langdate.o \
	$(OBJDIR)/Common/source/langdll.o \
	$(OBJDIR)/Common/source/langerror.o \
	$(OBJDIR)/Common/source/langerrorwindow.o \
	$(OBJDIR)/Common/source/langevaluate.o \
	$(OBJDIR)/Common/source/langexternal.o \
	$(OBJDIR)/Common/source/langhash.o \
	$(OBJDIR)/Common/source/langhtml.o \
	$(OBJDIR)/Common/source/langlist.o \
	$(OBJDIR)/Common/source/langmath.o \
	$(OBJDIR)/Common/source/langops.o \
	$(OBJDIR)/Common/source/langpack.o \
	$(OBJDIR)/Common/source/langparser.o \
	$(OBJDIR)/Common/source/langregexp.o \
	$(OBJDIR)/Common/source/langpython.o \
	$(OBJDIR)/Common/source/langscan.o \
	$(OBJDIR)/Common/source/langstartup.o \
	$(OBJDIR)/Common/source/langsystypes.o \
	$(OBJDIR)/Common/source/langtmpstack.o \
	$(OBJDIR)/Common/source/langtrace.o \
	$(OBJDIR)/Common/source/langtree.o \
	$(OBJDIR)/Common/source/langvalue.o \
	$(OBJDIR)/Common/source/langverbs.o \
	$(OBJDIR)/Common/source/langwinipc.o \
	$(OBJDIR)/Common/source/langxml.o

MENUBAR_SRC = \
	$(SRCDIR)/Common/source/menubar.c \
	$(SRCDIR)/Common/source/menueditor.c \
	$(SRCDIR)/Common/source/menufind.c \
	$(SRCDIR)/Common/source/menupack.c \
	$(SRCDIR)/Common/source/menuresize.c \
	$(SRCDIR)/Common/source/menuverbs.c \
	$(SRCDIR)/Common/source/meprograms.c

MENUBAR_OBJS = \
	$(OBJDIR)/Common/source/menubar.o \
	$(OBJDIR)/Common/source/menueditor.o \
	$(OBJDIR)/Common/source/menufind.o \
	$(OBJDIR)/Common/source/menupack.o \
	$(OBJDIR)/Common/source/menuresize.o \
	$(OBJDIR)/Common/source/menuverbs.o \
	$(OBJDIR)/Common/source/meprograms.o

OP_SRC = \
	$(SRCDIR)/Common/source/op.c \
	$(SRCDIR)/Common/source/opbuttons.c \
	$(SRCDIR)/Common/source/opdisplay.c \
	$(SRCDIR)/Common/source/opdraggingmove.c \
	$(SRCDIR)/Common/source/opedit.c \
	$(SRCDIR)/Common/source/opexpand.c \
	$(SRCDIR)/Common/source/ophoist.c \
	$(SRCDIR)/Common/source/opicons.c \
	$(SRCDIR)/Common/source/opinit.c \
	$(SRCDIR)/Common/source/oplangtext.c \
	$(SRCDIR)/Common/source/oplineheight.c \
	$(SRCDIR)/Common/source/oplist.c \
	$(SRCDIR)/Common/source/opops.c \
	$(SRCDIR)/Common/source/oppack.c \
	$(SRCDIR)/Common/source/oppopup.c \
	$(SRCDIR)/Common/source/opprint.c \
	$(SRCDIR)/Common/source/oprefcon.c \
	$(SRCDIR)/Common/source/opscreenmap.c \
	$(SRCDIR)/Common/source/opscrollbar.c \
	$(SRCDIR)/Common/source/opstructure.c \
	$(SRCDIR)/Common/source/opvalidate.c \
	$(SRCDIR)/Common/source/opverbs.c \
	$(SRCDIR)/Common/source/opvisit.c \
	$(SRCDIR)/Common/source/opxml.c \
	$(SRCDIR)/Common/source/textdisplay.c

OP_OBJS = \
	$(OBJDIR)/Common/source/op.o \
	$(OBJDIR)/Common/source/opbuttons.o \
	$(OBJDIR)/Common/source/opdisplay.o \
	$(OBJDIR)/Common/source/opdraggingmove.o \
	$(OBJDIR)/Common/source/opedit.o \
	$(OBJDIR)/Common/source/opexpand.o \
	$(OBJDIR)/Common/source/ophoist.o \
	$(OBJDIR)/Common/source/opicons.o \
	$(OBJDIR)/Common/source/opinit.o \
	$(OBJDIR)/Common/source/oplangtext.o \
	$(OBJDIR)/Common/source/oplineheight.o \
	$(OBJDIR)/Common/source/oplist.o \
	$(OBJDIR)/Common/source/opops.o \
	$(OBJDIR)/Common/source/oppack.o \
	$(OBJDIR)/Common/source/oppopup.o \
	$(OBJDIR)/Common/source/opprint.o \
	$(OBJDIR)/Common/source/oprefcon.o \
	$(OBJDIR)/Common/source/opscreenmap.o \
	$(OBJDIR)/Common/source/opscrollbar.o \
	$(OBJDIR)/Common/source/opstructure.o \
	$(OBJDIR)/Common/source/opvalidate.o \
	$(OBJDIR)/Common/source/opverbs.o \
	$(OBJDIR)/Common/source/opvisit.o \
	$(OBJDIR)/Common/source/opxml.o \
	$(OBJDIR)/Common/source/textdisplay.o

MAC_OSA_CLASSIC_SRC = \
	$(SRCDIR)/Common/source/osamenus.c \
	$(SRCDIR)/Common/source/osadroplet.c

MAC_OSA_CLASSIC_OBJS = \
	$(OBJDIR)/Common/source/osamenus.o \
	$(OBJDIR)/Common/source/osadroplet.o

MAC_OSA_SRC = \
	$(SRCDIR)/Common/source/osacomponent.c \
	$(SRCDIR)/Common/source/osaparseaete.c \
	$(SRCDIR)/Common/source/osawindows.c

MAC_OSA_OBJS = \
	$(OBJDIR)/Common/source/osacomponent.o \
	$(OBJDIR)/Common/source/osaparseaete.o \
	$(OBJDIR)/Common/source/osawindows.o

SHELL_SRC = \
	$(SRCDIR)/Common/source/shell.c \
	$(SRCDIR)/Common/source/shellactivate.c \
	$(SRCDIR)/Common/source/shellblocker.c \
	$(SRCDIR)/Common/source/shellbuttons.c \
	$(SRCDIR)/Common/source/shellcallbacks.c \
	$(SRCDIR)/Common/source/shellfile.c \
	$(SRCDIR)/Common/source/shellhooks.c \
	$(SRCDIR)/Common/source/shelljuggler.c \
	$(SRCDIR)/Common/source/shellkb.c \
	$(SRCDIR)/Common/source/shellmenu.c \
	$(SRCDIR)/Common/source/shellmouse.c \
	$(SRCDIR)/Common/source/shellops.c \
	$(SRCDIR)/Common/source/shellprint.c \
	$(SRCDIR)/Common/source/shellscrap.c \
	$(SRCDIR)/Common/source/shellscroll.c \
	$(SRCDIR)/Common/source/shellsysverbs.c \
	$(SRCDIR)/Common/source/shellundo.c \
	$(SRCDIR)/Common/source/shellupdate.c \
	$(SRCDIR)/Common/source/shellverbs.c \
	$(SRCDIR)/Common/source/shellwindow.c \
	$(SRCDIR)/Common/source/shellwindowmenu.c \
	$(SRCDIR)/Common/source/shellwindowverbs.c

SHELL_OBJS = \
	$(OBJDIR)/Common/source/shell.o \
	$(OBJDIR)/Common/source/shellactivate.o \
	$(OBJDIR)/Common/source/shellblocker.o \
	$(OBJDIR)/Common/source/shellbuttons.o \
	$(OBJDIR)/Common/source/shellcallbacks.o \
	$(OBJDIR)/Common/source/shellfile.o \
	$(OBJDIR)/Common/source/shellhooks.o \
	$(OBJDIR)/Common/source/shelljuggler.o \
	$(OBJDIR)/Common/source/shellkb.o \
	$(OBJDIR)/Common/source/shellmenu.o \
	$(OBJDIR)/Common/source/shellmouse.o \
	$(OBJDIR)/Common/source/shellops.o \
	$(OBJDIR)/Common/source/shellprint.o \
	$(OBJDIR)/Common/source/shellscrap.o \
	$(OBJDIR)/Common/source/shellscroll.o \
	$(OBJDIR)/Common/source/shellsysverbs.o \
	$(OBJDIR)/Common/source/shellundo.o \
	$(OBJDIR)/Common/source/shellupdate.o \
	$(OBJDIR)/Common/source/shellverbs.o \
	$(OBJDIR)/Common/source/shellwindow.o \
	$(OBJDIR)/Common/source/shellwindowmenu.o \
	$(OBJDIR)/Common/source/shellwindowverbs.o

STRING_SRC = \
	$(SRCDIR)/Common/source/strings.c \
	$(SRCDIR)/Common/source/stringverbs.c

STRING_OBJS = \
	$(OBJDIR)/Common/source/strings.o \
	$(OBJDIR)/Common/source/stringverbs.o

TABLE_SRC = \
	$(SRCDIR)/Common/source/tablecompare.c \
	$(SRCDIR)/Common/source/tabledisplay.c \
	$(SRCDIR)/Common/source/tableedit.c \
	$(SRCDIR)/Common/source/tableexternal.c \
	$(SRCDIR)/Common/source/tablefind.c \
	$(SRCDIR)/Common/source/tableformats.c \
	$(SRCDIR)/Common/source/tableops.c \
	$(SRCDIR)/Common/source/tablepack.c \
	$(SRCDIR)/Common/source/tablepopup.c \
	$(SRCDIR)/Common/source/tablerunbutton.c \
	$(SRCDIR)/Common/source/tablescrap.c \
	$(SRCDIR)/Common/source/tablestructure.c \
	$(SRCDIR)/Common/source/tablevalidate.c \
	$(SRCDIR)/Common/source/tableverbs.c \
	$(SRCDIR)/Common/source/tablewindow.c

TABLE_OBJS = \
	$(OBJDIR)/Common/source/tablecompare.o \
	$(OBJDIR)/Common/source/tabledisplay.o \
	$(OBJDIR)/Common/source/tableedit.o \
	$(OBJDIR)/Common/source/tableexternal.o \
	$(OBJDIR)/Common/source/tablefind.o \
	$(OBJDIR)/Common/source/tableformats.o \
	$(OBJDIR)/Common/source/tableops.o \
	$(OBJDIR)/Common/source/tablepack.o \
	$(OBJDIR)/Common/source/tablepopup.o \
	$(OBJDIR)/Common/source/tablerunbutton.o \
	$(OBJDIR)/Common/source/tablescrap.o \
	$(OBJDIR)/Common/source/tablestructure.o \
	$(OBJDIR)/Common/source/tablevalidate.o \
	$(OBJDIR)/Common/source/tableverbs.o \
	$(OBJDIR)/Common/source/tablewindow.o

WP_SRC = \
	$(SRCDIR)/Common/source/wpengine.c \
	$(SRCDIR)/Common/source/wpverbs.c

WP_OBJS = \
	$(OBJDIR)/Common/source/wpengine.o \
	$(OBJDIR)/Common/source/wpverbs.o

WIN_MAIN_SRC = \
	$(SRCDIR)/Common/source/FrontierWinMain.c

WIN_MAIN_OBJS = \
	$(OBJDIR)/Common/source/FrontierWinMain.o

COMMON_SOURCES = \
	$(FILE_SRC) \
	$(PCRE_SRC) \
	$(PICT_SRC) \
	$(TOOL_SRC) \
	$(CANCOON_SRC) \
	$(CLAY_SRC) \
	$(DB_SRC) \
	$(LANG_SRC) \
	$(MENUBAR_SRC) \
	$(OP_SRC) \
	$(SHELL_SRC) \
	$(STRING_SRC) \
	$(TABLE_SRC) \
	$(WP_SRC)

COMMON_OBJECTS = \
	$(FILE_OBJS) \
	$(PCRE_OBJS) \
	$(PICT_OBJS) \
	$(TOOL_OBJS) \
	$(CANCOON_OBJS) \
	$(CLAY_OBJS) \
	$(DB_OBJS) \
	$(LANG_OBJS) \
	$(MENUBAR_OBJS) \
	$(OP_OBJS) \
	$(SHELL_OBJS) \
	$(STRING_OBJS) \
	$(TABLE_OBJS) \
	$(WP_OBJS)

MAC_CLASSIC_SOURCES = \
	$(MAC_MOREFILES_CLASSIC_SRC) \
	$(MAC_LAMG_CLASSIC_SRC) \
	$(MAC_OSA_CLASSIC_SRC)

MAC_CLASSIC_OBJECTS = \
	$(MAC_MOREFILES_CLASSIC_OBJS) \
	$(MAC_LAMG_CLASSIC_OBJS) \
	$(MAC_OSA_CLASSIC_OBJS)

MAC_SOURCES = \
	$(MAC_APPLET_SRC) \
	$(MAC_FILE_SRC) \
	$(MAC_IAC_SRC) \
	$(MAC_IOA_SRC) \
	$(MAC_IOWA_SRC) \
	$(MAC_LAND_SRC) \
	$(MAC_MOREFILES_SRC) \
	$(MAC_TOOL_SRC) \
	$(MAC_LANG_SRC) \
	$(MAC_OSA_SRC) \
	$(COMMON_SOURCES)

MAC_OBJECTS = \
	$(MAC_APPLET_OBJS) \
	$(MAC_FILE_OBJS) \
	$(MAC_IAC_OBJS) \
	$(MAC_IOA_OBJS) \
	$(MAC_IOWA_OBJS) \
	$(MAC_LAND_OBJS) \
	$(MAC_MOREFILES_OBJS) \
	$(MAC_TOOL_OBJS) \
	$(MAC_LANG_OBJS) \
	$(MAC_OSA_OBJS) \
	$(COMMON_OBJECTS)

WIN_SOURCES = \
	$(WIN_MAIN_SRC) \
	$(WIN_STUBS_SRC) \
	$(WIN_TOOL_SRC) \
	$(WIN_LANG_SRC) \
	$(COMMON_SOURCES)

WIN_OBJECTS = \
	$(WIN_MAIN_OBJS) \
	$(WIN_STUBS_OBJS) \
	$(WIN_TOOL_OBJS) \
	$(WIN_LANG_OBJS) \
	$(COMMON_OBJECTS)


#================================================================================================
# Resources Definitions
#================================================================================================

REZ_INCLUDES = \
	$(SRCDIR)/Common/headers/config.r \
	$(SRCDIR)/Common/headers/frontier.r \
	$(SRCDIR)/Common/headers/shell.mcp.r

FRONTIER_REZ = \
	$(SRCDIR)/resources/Frontier/BNDL.r \
	$(SRCDIR)/resources/Frontier/icns.r

RADIO_REZ = \
	$(SRCDIR)/resources/Radio/BNDL.r \
	$(SRCDIR)/resources/Radio/icns.r

CARBON_REZ = \
	$(SRCDIR)/Common/resources/Mac/carb.r \
	$(SRCDIR)/Common/resources/Mac/osxdialogs.r

MAC_REZ_TEMPLATES = \
	$(SRCDIR)/Common/resources/Mac/ResEditTMPLsForShell.r

MAC_REZ_PLIST = \
	$(SRCDIR)/resources/Frontier/plist.r

MAC_REZ = \
	$(SRCDIR)/Common/resources/Mac/aeut.r \
	$(SRCDIR)/Common/resources/Mac/clut.r \
	$(SRCDIR)/Common/resources/Mac/ioaapp.r \
	$(SRCDIR)/Common/resources/Mac/iowaruntime.r \
	$(SRCDIR)/Common/resources/Mac/kernelverbs.r \
	$(SRCDIR)/Common/resources/Mac/lang.r \
	$(SRCDIR)/Common/resources/Mac/opicons.r \
	$(SRCDIR)/Common/resources/Mac/osacomponent.r \
	$(SRCDIR)/Common/resources/Mac/ppat.r \
	$(SRCDIR)/Common/resources/Mac/SHELL.R \
	$(SRCDIR)/Common/resources/Mac/sicn.r \
	$(SRCDIR)/Common/windoidWDEF/windoidWDEF.r

MAC_RESOURCES = \
	$(MAC_REZ) \
	$(CARBON_REZ) \
	$(FRONTIER_REZ)

WIN_RESOURCES = \
	$(SRCDIR)/Common/resources/Win32/WinLand.rc

# DEPENDENCIES
#DEPS = $(OBJS:$(OBJDIR)/%.o=%)

EXES = $(BINDIR)/$(PROJECT)

#================================================================================================
# Make rules
#================================================================================================

.PHONY: all clean dirs MAC_$(PROJECT) WIN_$(PROJECT)

all: dirs
	@case "`uname -s`" in \
	Darwin*) \
		if [ ! -x $(REZ) ]; then \
			echo "The Mac OS X Developer Tools need to be installed"; \
		else \
			$(MAKE) EXES='$(BINDIR)/$(PROJECT).app' CFLAGS='$(MAC_CFLAGS)' MAC_$(PROJECT); \
  		fi \
  		;; \
	MINGW*|CYGWIN*) \
		if [ true ]; then \
			echo "Not yet! Try building with WINE."; \
		else \
			$(MAKE) EXES='$(BINDIR)/$(PROJECT).exe' CFLAGS='$(WIN_CFLAGS)' WIN_$(PROJECT); \
  		fi \
  		;; \
	*) \
		echo "Not a supported platform" \
		;; \
	esac

MAC_$(PROJECT):	$(MAC_OBJECTS) $(BINDIR)/$(PROJECT).rsrc $(MAC_PAIGE_LIB_AR) $(MAC_INFOPLIST_OBJS)
	if [ ! -f ".ranlib" ]; then \
		$(RANLIB) $(MAC_PAIGE_LIB_AR); \
		touch ".ranlib"; \
	fi
	$(CC) $(MAC_LDFLAGS) -o $(BINDIR)/$(PROJECT) $(MAC_OBJECTS) $(MAC_LIBS)
	
	#
	# Create application bundle
	#

	mkdir -p $(BUILT_PRODUCTS_DIR)/$(EXECUTABLE_FOLDER_PATH)
	mkdir -p $(BUILT_PRODUCTS_DIR)/$(UNLOCALIZED_RESOURCES_FOLDER_PATH)
	
	# Copy Mach-O binary
	mv $(PROJECT) $(BUILT_PRODUCTS_DIR)/$(EXECUTABLE_FOLDER_PATH)
	
	# Build Frontier.icns file - there is a bug in that this file is not built correctly from the
	# raw input, so just copy a pre-made file
	#$(REZ) $(MAC_REZ_OPTIONS) $(SRCDIR)/resources/Frontier/icns.r -useDF -o $(BUILT_PRODUCTS_DIR)/$(UNLOCALIZED_RESOURCES_FOLDER_PATH)/Frontier.icns
	cp $(SRCDIR)/resources/Frontier/Frontier.icns $(BUILT_PRODUCTS_DIR)/$(UNLOCALIZED_RESOURCES_FOLDER_PATH)

	echo "APPLLAND" > $(BUILT_PRODUCTS_DIR)/$(CONTENTS_FOLDER_PATH)/PkgInfo

	# Move resources from resource fork to data fork
	$(RESMERGER) $(BINDIR)/$(PROJECT).rsrc -dstIs DF -o $(BUILT_PRODUCTS_DIR)/$(UNLOCALIZED_RESOURCES_FOLDER_PATH)/$(PROJECT).rsrc
	
	#
	# Extract info from Common/headers/versions.h for Info.plist file
	#
	
	$(CC) $(MAC_CFLAGS) -o $(SRCROOT)/tools/appversion.o $(SRCROOT)/tools/appversion.c
	$(CC) $(MAC_CFLAGS) -o $(SRCROOT)/tools/appcopyright.o $(SRCROOT)/tools/appcopyright.c

	( \
		product_str=$(PROJECT); \
		\
		version_str=`$(SRCROOT)/tools/appversion.o`; \
		copyright_str=`$(SRCROOT)/tools/appcopyright.o`; \
		\
		sed -e "s/\$${PRODUCT_NAME_STR}/$${product_str}/g" \
			-e "s/\$${PRODUCT_VERSION_STR}/$${version_str}/g" \
			-e "s/\$${PRODUCT_COPYRIGHT_STR}/$${copyright_str}/g" \
			$(SRCROOT)/Info.plist > $(BUILT_PRODUCTS_DIR)/$(CONTENTS_FOLDER_PATH)/Info.plist; \
	)
	
.PHONY: WIN_$(PROJECT)
WIN_$(PROJECT):	$(WIN_OBJECTS) $(BINDIR)/$(PROJECT).res
	$(CC) $(WIN_LDFLAGS) -o $(BINDIR)/$(PROJECT).exe $(WIN_OBJECTS) $(WIN_LIBS) $(BINDIR)/$(PROJECT).res

.SUFFIXES: .r .rc .res .rsrc

#$(OBJDIR)/%.c.o:	%.c
%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $<

# do nothing with langparser.y - at present
%.c: %.y

$(BINDIR)/$(PROJECT).rsrc: 	$(MAC_RESOURCES)
	$(REZ) $(MAC_REZ_OPTIONS) -t RSRC -c RSED $(MAC_RESOURCES) -o $(BINDIR)/$(PROJECT).rsrc

$(BINDIR)/$(PROJECT).res: 	$(WIN_RESOURCES)
	$(RC) $(WIN_RESOURCES) $(WIN_REZ_OPTIONS) -o $(BINDIR)/$(PROJECT).res

debug:
	@$(MAKE) CFLAGS='$(CFLAGS) $(DEBUGFLAGS)' LIBS='$(LIBS) $(DEBUGLIBS)' all

#
# clean: Get rid of executable, dependencies, and all object files
#
clean:
	rm -f $(MAC_OBJECTS) $(WIN_OBJECTS) $(MAC_INFOPLIST_OBJS)
	rm -f $(BINDIR)/$(PROJECT).rsrc $(BINDIR)/$(PROJECT).res
	rm -rf $(BINDIR)/$(PROJECT).app $(BINDIR)/$(PROJECT).exe
	rm -f $(SRCROOT)/tools/appversion.o $(SRCROOT)/tools/appcopyright.o

installexes: $(EXES)
	if [ ! -d "$(bininst)" ]; then mkdir -p "$(bininst)"; fi
	cp -f $(EXES) "$(bininst)/"

install: installexes

#
# We stick object files in separate directories for convenience
#
DIRS = \
	$(OBJDIR) \
	$(OBJDIR)/Common/source/ \
	$(OBJDIR)/FrontierSDK/Toolkits/AppletToolkit/Source/ \
	$(OBJDIR)/FrontierSDK/Toolkits/IACTools/Source/ \
	$(OBJDIR)/Common/IOAToolkit/ \
	$(OBJDIR)/Common/IowaRuntime/Source/ \
	$(OBJDIR)/Common/UserLandIACToolkit/ \
	$(OBJDIR)/Common/MoreFiles/ \
	$(OBJDIR)/Common/PCRE/

dirs: $(DIRS)

$(DIRS): %: 
	mkdir -p $@

#
# You can optionally "make dep" to make dependencies.
# voodoo magic in the form of a perl script turns
# Foo.o: Foo.cpp into obj/Foo.cpp.o: Foo.cpp
#
# I'm not brave enough to try and translate that regex to sed
#dep:
#	$(CCC) -M $(CFLAGS) $(DEPS) | \
#		perl -pe 's/^(.*)\.o: ([^\/]*\/)??([^ \/]+\.(\w+))/$(OBJDIR)\/$$2$$1.$$4.o: $$2$$3/;' \
#		> .depend
#depend:
#       mkdep ${CFLAGS} ${SRCS}

#
# Include ".depend" if it exists (run "make dep" to generate it)
#
#ifeq (.depend,$(wildcard .depend))
#include .depend
#endif

