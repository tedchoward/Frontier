
*** UserLand Frontier(tm)  --  Version 10.0a1 Open Source Release

Building the Frontier kernel currently requires Metrowerks CodeWarrior
Pro 8 for Mac or Windows with the 8.3 updates. Other versions might
work, too, but have not been tested.


*** Getting Started

1. Locate the Frontier.mcp project file in the build_CW8 directory and
open it with CodeWarrior.

2. Select a target from the popup menu at the top of the project window.
It is a good idea to start with the Debug build for your current
platform, e.g. Frontier OSX Debug if you are running CodeWarrior on Mac
OS X.

3. Select the Make command from the Project menu.

4. When CodeWarrior has finished compiling and linking, locate the
resulting application in the sub-directory of the build_CW8 directory
whose name corresponds to the target name you chose under step #2. Copy
the application to a directory containing a Frontier installation with
at least a Frontier.root file and launch it.

5. Report problems and ask questions on the Frontier Kernel mailing
list:

	http://groups.yahoo.com/groups/frontierkernel


*** Notes

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

This source code download does not come with a Frontier installation
for testing the applications compiled from the code. For this purpose,
minimal installations for each platform are available for donwload
from the Frontier Kernel website at:

	http://kernel.scripting.com/

