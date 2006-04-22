*** UserLand Frontier(tm)  --  Version 10.1 Open Source Release ***

Last updated 22 Apr 2006 by Andre Radke

Since the original v10.0a1 Open Source release of Frontier, additional work
has been done to allow the building of Frontier using other development
environments. The following sections describe these environments.


A. Apple Xcode Tools for Mac OS X
=================================
   
Build directory : build_XCode

Getting Started with Xcode v1.2 or v1.5 or v2.0
-----------------------------------------------

1. Open the Frontier.xcode project in Xcode.

2. Click the Build button.

Getting Started with Xcode v2.1 and higher
------------------------------------------

1. Open the FrontierFat.xcodeproj project in Xcode.

2. Click the Build button.

Notes
-----

Both the Frontier.xcode and the FrontierFat.xcodeproj projects offer a
development and a deployment build style.

In the FrontierFat.xcodeproj
project, the development build style will compile for the native
architecture only, but the deployment build style will compile a
Universal Binary of Frontier which will run on both PPC and Intel Macs.


B. Metrowerks CodeWarrior Pro for Mac or Windows
================================================
   
Build directory : build_CWPro8

Building the Frontier kernel currently requires Metrowerks CodeWarrior
Pro 8 for Mac or Windows with the 8.3 updates. Other versions might
work, too, but have not been tested.

Getting Started
---------------

1. Locate the Frontier.mcp project file in the build_CW8 directory and
open it with CodeWarrior.

2. Select a target from the popup menu at the top of the project window.
It is a good idea to start with the Debug build for your current
platform, e.g. Frontier OSX CFM Debug if you are running CodeWarrior on Mac
OS X.

3. Select the Make command from the Project menu.

4. When CodeWarrior has finished compiling and linking, locate the
resulting application in the sub-directory of the build_CW8 directory
whose name corresponds to the target name you chose under step #2. Copy
the application to a directory containing a Frontier installation with
at least a Frontier.root file and launch it.

Notes
-----

When you open the Frontier.mcp project file on Windows, CodeWarrior will
complain that the access path

	{System}Library/CFM Support/StubLibraries

cannot be found. This means that you will not be able to build the Mac
OS X version on that platforms, but it does not affect the versions for
Windows.

When you build the Frontier Win or Frontier Win Debug target, expect
CodeWarrior to report about 240 compiler warnings, mostly about
redeclared identifiers in Microsoft header files, some missing function
prototypes, and some unused variables. You can ignore these warnings.


C. Microsoft Visual Studio C++ 6.0 for Windows
==============================================
   
Build directory : build_VC6

Getting Started
---------------

1. Open the Frontier.dsp project in MS Visual Studio.

2. Choose a build configuration from the Project menu. It is a good idea
to start with the Debug build configuration.

3. Build.

Notes
-----

Needs more accurate build instructions.


D. GNU Tools (initially for Mac OS X only)
==========================================
   
Build directory : build_GNU

Building the Frontier kernel currently requires the Mac OS X Developer
Tools to be installed on Mac OS X. Older tools using Project Builder, or
newer tools using Xcode should work, using 'gcc' v3.3. Other versions of
'gcc' might work, too, but have not been tested (on Mac OS X).

Getting Started
---------------

In a shell, do the following :

1. Change directory to the "build_GNU" directory of the Frontier project.

e.g.

	cd build_GNU

2. Type :

	make

Notes
-----

The support for the GNU tools is very preliminary, and currently builds a non-Debug
version for Mac OS X (only) by default. No GUI based tools such as the Project Builder or
Xcode IDE are required.

When you build, expect the GNU tools to report some compiler warnings.
You can ignore these warnings.

Support for building the Windows version using Cygwin/MinGW or other GNU based
development environments on Windows systems, and support for building the Windows
version using WINE on non-Windows systems, may be added in the future.


*** Reporting Problems

Report problems and ask questions on the Frontier Kernel mailing list:

	http://groups.yahoo.com/groups/frontierkernel


*** Notes

This source code download does not come with a Frontier installation
for testing the applications compiled from the code. For this purpose,
minimal installations for each platform are available for donwload
from the Frontier Kernel SourceForge project website at:

	http://frontierkernel.sourceforge.net/
