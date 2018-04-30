# Userland Frontier Kernel #

## Installation Instructions

*I apologize for how convoluted this is. Unfortunately Apple is making us jump
through a series of hoops in the [name of security][app-translocation] in order
to run this software.*

- Download the OPML Editor distribution flle from: [static.tedchoward.com][opml-dl]
  - This image contains the root database file (`opml.root`) and other files
    necessary to run the OPML Editor.
- Double-click `OPML.dmg`. This mounts the "disk image".
- Drag the `OPML` folder to `/Applications`
- Download an OPML Editor application binary (`OPML.app.zip`) from the
  [GitHub Releases Page][dev].
- Double-click `OPML.app.zip`. This extracts `OPML.app`
- Drag `OPML.app` to `/Applications/OPML`
- `CTRL`-click `OPML.app` to view the context menu, and click *Open* in the
  context menu.

## Notes on the Source Code

### The `develop` branch

- Latest Release: [OPML Editor v10.2d4][dev].
- Builds and runs in Xcode 9 on macOS 10.13, but requires the 10.6 sdk.

### The `master` branch

- Latest Release: [OPML Editor v10.1b21][mas].
- Builds in Xcode 4 on Mac OS X 10.6, but requires the 10.4 sdk.
- Runs on macOS 10.13

[opml-dl]: http://static.tedchoward.com/frontier/opml-editor/OPML.dmg
[mas]: https://github.com/tedchoward/Frontier/releases/tag/v10.1b21
[dev]:https://github.com/tedchoward/Frontier/releases/tag/10.2d4
[app-translocation]: https://weblog.rogueamoeba.com/2016/06/29/sierra-and-gatekeeper-path-randomization/
