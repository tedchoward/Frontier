*** UserLand Frontier(tm)  --  Version 10.0 Open Source Release

Last updated 02 Jan 2005 by Terry Teague

Since the original v10.0a1 Open Source release of Frontier, additional work
has been done to allow the building of Frontier using other development
environments. The following sections describe these environments.


*** Supported Development Environments

A. Metrowerks CodeWarrior Pro for Mac or Windows
================================================
   
CVS module, build directory : build_CWPro8

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

When you open the Frontier.mcp project file on Windows or on classic Mac
OS, CodeWarrior will complain that the access path

	{System}Library/CFM Support/StubLibraries

cannot be found. This means that you will not be able to build the Mac
OS X version on those platforms, but it does not affect the versions for
Windows and classic Mac OS.

When you build the Frontier PPC or Frontier PPC Debug targets, expect
CodeWarrior to report a single link warning about the errno variable
being defined in OTXTILib and MSL_ALL_PPC.Lib (or MSL_ALL_PPC_D.Lib).
You can safely ignore this warning.

When you build the Frontier Win or Frontier Win Debug target, expect
CodeWarrior to report about 240 compiler warnings, mostly about
redeclared identifiers in Microsoft header files, some missing function
prototypes, and some unused variables. You can ignore these warnings.


B. Microsoft Visual Studio C++ for Windows
==========================================
   
CVS module, build directory : build_VC6

Building the Frontier kernel currently requires Microsoft Visual Studio
C++ v6. Other versions might work, too, but have not been tested.

Getting Started
---------------

1. Open the Frontier.dsp project in MS Visual Studio.

2. Build.

Notes
-----

Needs more accurate build instructions.


C. Apple Xcode Tools for Mac OS X
=================================
   
CVS module, build directory : build_XCode

Building the Frontier kernel currently requires Xcode v1.2 or v1.5.
Other versions might work, too, but have not been tested.

Getting Started
---------------

1. Open the Frontier.xcode project in Xcode.

2. Click the Build button.

Notes
-----

The support for Xcode is preliminary, and currently builds a Debug
(Development build style) version by default.

When you build, expect Xcode to report a couple of compiler warnings.
You can ignore these warnings.


D. GNU Tools (initially for Mac OS X only)
==========================================
   
CVS module, build directory : build_GNU

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
version using WINE on non-Windows systems, is being developed.


*** Reporting Problems

Report problems and ask questions on the Frontier Kernel mailing list:

	http://groups.yahoo.com/groups/frontierkernel


*** Notes

This source code download does not come with a Frontier installation
for testing the applications compiled from the code. For this purpose,
minimal installations for each platform are available for donwload
from the Frontier Kernel SourceForge project website at:

	http://frontierkernel.sourceforge.net/
