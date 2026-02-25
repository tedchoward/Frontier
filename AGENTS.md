# Repository Guidelines

## Project Structure & Module Organization
- `Common/source`: core C/C++/Obj-C implementation for the Frontier kernel.
- `Common/headers`: public and internal headers used across build targets.
- `Common/resources`: shared resource files (classic Mac/Win resource data).
- `FrontierSDK/`: SDK toolkits and sample IOAs/cards for integrations.
- `build_Xcode/`, `build_VC2K5/`, `build_VC2K3/`, `build_VC6/`, `build_CWPro8/`: IDE-specific project files.
- `resources/`: application assets and packaging support files.

## Build, Test, and Development Commands
This repo is primarily IDE-driven rather than command-line:
- Xcode (macOS): open `build_Xcode/Frontier Universal Binary.xcodeproj` and Build.
- Visual Studio (Windows): open `build_VC2K8/Frontier.vcproj` and Build the Debug configuration.

If you need to run the app, ensure a database root file is available (for example an `OPML` installation containing `opml.root`).

## Architecture Overview
- The Frontier kernel is a cross-platform core with platform entry points in `Common/source` (for example `FrontierMacMain.c`, `FrontierWinMain.c`).
- Shared system libraries and third-party dependencies live under `Common/` (for example `PCRE/`, `sqlite3/`, `Paige/`).
- SDK toolkits and sample IOAs/cards live in `FrontierSDK/` for integrations and extensions.

## Platform Build Notes
- macOS builds should use `build_Xcode/Frontier Universal Binary.xcodeproj`; other Xcode projects are deprecated.
- Windows builds should use `build_VC2K8/Frontier.vcproj`; other Visual Studio configurations are deprecated.
- CodeWarrior builds are deprecated and should not be used for new changes.

## Coding Style & Naming Conventions
- Languages: C/C++ with some Objective-C (`.m`).
- Indentation: tabs are common in existing files; keep alignment consistent within a file.
- Braces: K&R style (`if (...) {`), with existing comment blocks preserved.
- Naming: follow established prefixes and typedef patterns already present (e.g., `ty...`, `hd...`), and avoid reformat-only changes.
- No enforced formatter or linter is configured in this repo.

## Testing Guidelines
- No automated test suite is present.
- Validate changes by building in the relevant IDE and performing a quick manual smoke test (launch the app with a valid root database such as `opml.root`).

## Commit & Pull Request Guidelines
- Commit messages are short, imperative, and specific (e.g., "Fixes Xcode warnings", "Removes `THINK_C` `#ifdef`s").
- PRs should include a clear description of the change and affected build target(s).
- PRs should state how you built and validated the change (IDE + configuration).
- PRs should include screenshots only when UI or resource changes are visible.

## Agent Notes
- Prefer minimal, targeted edits in legacy code paths.
- Keep changes scoped to the relevant platform directories when possible.
