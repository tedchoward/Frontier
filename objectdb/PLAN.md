# ObjectDB GTK3 Browser Plan

## Goal

Build a Linux-only GTK3 desktop application for browsing Frontier `.root` files.

V1 scope:

- Plain C
- Top-level project folder: `objectdb/`
- Read-only
- Open files via `File -> Open`
- Main view is a hierarchical table with columns:
  - `Name`
  - `Value`
  - `Kind`
- Uses `libodb` for all database access

This should be designed so it can later grow into a full editor without requiring a major rewrite.

## Architecture

The app should be split into three layers.

### 1. Document Layer

Purpose:

- Own the currently opened `.root` file path and `odbRef`
- Isolate all `libodb` interactions
- Translate raw ODB data into UI-friendly entry records

Responsibilities:

- Open a `.root` file
- Close the current document
- Report whether a document is open
- Enumerate children for any table path
- Format database values into display text
- Convert `libodb` errors into application-facing error strings

Suggested API:

- `objectdb_document_new`
- `objectdb_document_free`
- `objectdb_document_open`
- `objectdb_document_close`
- `objectdb_document_is_open`
- `objectdb_document_get_children`

Suggested entry shape:

- `name`
- `full_path`
- `kind_text`
- `value_text`
- `is_table`

Design note:

Even in read-only mode, this layer should be the single owner of document state so write support can later be added here.

### 2. Tree/Model Adapter Layer

Purpose:

- Bridge document data into GTK's hierarchical model APIs

Implementation:

- Backed by `GtkTreeStore`

Visible columns:

- `Name`
- `Value`
- `Kind`

Hidden columns:

- Full ODB path
- `is_table`
- `children_loaded`

Behavior:

- Populate only root children when a file is opened
- For table rows, insert a dummy child so the row is expandable
- On row expansion, lazily load real children and mark the row as loaded

Design note:

Lazy loading keeps startup fast and avoids recursively materializing the whole database.

### 3. GTK UI Layer

Purpose:

- Own the GTK application and main window
- Wire menus, dialogs, and tree view behavior

Responsibilities:

- Create `GtkApplication`
- Create main window
- Build `File` menu with `Open` and `Quit`
- Create `GtkTreeView` inside a scrolled window
- Handle row expansion events
- Show error dialogs
- Update window title when a document is opened

The UI layer should not call raw `libodb` APIs directly. It should work through the document and tree-model layers.

## UI Design

Main window content:

- Menu bar
- Scrolled tree view

Tree view:

- Tree column: `Name`
- Secondary columns: `Value`, `Kind`

Open flow:

- `File -> Open`
- `GtkFileChooserDialog`
- Filter to `*.root`
- On success:
  - Close any open document
  - Open selected file
  - Rebuild tree model from `root`
  - Update window title with the selected file name

Failure flow:

- Show a GTK error dialog with a friendly message

## Value Formatting Rules

`Kind` should be human-readable, for example:

- `table`
- `string`
- `long`
- `short`
- `char`
- `bool`
- `date`
- `direction`
- `string4`
- `single`
- `enum`
- `binary`
- `external`
- `unknown`

`Value` should be formatted for display:

- table: blank or `<table>`
- string: text
- bool: `true` or `false`
- numbers: decimal text
- direction: symbolic name where possible
- string4 / enum: FourCC text
- binary / external: placeholder such as `<binary N bytes>`
- unsupported complex types: placeholder text, never a crash

## Build System Integration

Top-level:

- Add `add_subdirectory(objectdb)` to the root `CMakeLists.txt`

`objectdb/CMakeLists.txt`:

- Define an `objectdb` executable target
- Use `pkg-config` to find GTK3 (`gtk+-3.0`)
- Link against:
  - `libodb`
  - GTK3

Preferred include usage:

- Depend on `libodb/include`
- Avoid depending on `libodb/src` unless there is no public alternative

## Folder Layout

```text
objectdb/
  CMakeLists.txt
  PLAN.md
  README.md
  src/
    main.c
    app.c
    app.h
    window.c
    window.h
    document.c
    document.h
    tree_model.c
    tree_model.h
    value_format.c
    value_format.h
```

## V1 Acceptance Criteria

- Builds on Linux with CMake
- Launches a GTK3 window
- `File -> Open` opens a `.root` file
- Tree view shows `Name`, `Value`, `Kind`
- Table rows expand to show child entries
- Uses `libodb` for all database access
- Read-only only
- Open failures are handled with dialogs instead of crashes

## Concrete Implementation Sequence

### Phase 1: Scaffold The New App

1. Create `objectdb/`, `objectdb/src/`, and `objectdb/CMakeLists.txt`
2. Update the root `CMakeLists.txt` to include `add_subdirectory(objectdb)`
3. Add a minimal `main.c` and `app.c` that launch an empty `GtkApplicationWindow`
4. Verify the new app builds on Linux and starts successfully

Deliverable:

- A compilable GTK3 application target wired into the existing CMake project

### Phase 2: Build The Window Shell

1. Add `window.c` / `window.h`
2. Create the main window, menu bar, scrolled container, and placeholder tree view
3. Add `File -> Open` and `Quit` actions
4. Add a file chooser filtered to `*.root`

Deliverable:

- A working shell application with basic window and menu plumbing

### Phase 3: Implement The Document Layer

1. Add `document.c` / `document.h`
2. Define an opaque `ObjectDBDocument` struct
3. Implement open/close logic around `odbOpenPath` / `odbCloseFile`
4. Implement child enumeration using:
   - `odbCountItems`
   - `odbGetNthItem`
   - `odbGetType`
   - `odbGetValue`
5. Centralize error extraction from `libodb`

Deliverable:

- A reusable, UI-independent document API that can enumerate entries for a table path

### Phase 4: Implement Value Formatting

1. Add `value_format.c` / `value_format.h`
2. Map `odbValueType` values to human-readable `Kind` strings
3. Convert `odbValueRecord` values into display strings
4. Handle tables, binary, and externals with placeholder output
5. Ensure all owned `libodb` values are disposed correctly

Deliverable:

- A formatting module used by the document layer for stable display text

### Phase 5: Implement The Tree Model Adapter

1. Add `tree_model.c` / `tree_model.h`
2. Define the `GtkTreeStore` schema with visible and hidden columns
3. Implement model reset / clear
4. Implement root population after document open
5. Implement lazy child loading for expandable table rows
6. Use dummy child rows so GTK shows expanders for tables

Deliverable:

- A hierarchical model that can represent the ODB tree efficiently

### Phase 6: Connect UI To Data

1. Connect `File -> Open` to document open and tree model rebuild
2. Connect tree row expansion to lazy child loading
3. Update the window title after a file opens
4. Show GTK error dialogs for failed document operations

Deliverable:

- A usable read-only browser for `.root` files

### Phase 7: Manual Validation Pass

1. Open `databases/new.root`
2. Verify root children appear
3. Expand `root.workspace`
4. Confirm `Name`, `Value`, and `Kind` render correctly
5. Confirm table rows expand and scalar rows do not
6. Try an invalid file path and verify failure is graceful

Deliverable:

- A validated v1 browser with known-good behavior on a real fixture

## Follow-On Work After V1

Once the browser is stable, the next logical extensions are:

- Command-line open of an initial `.root` file
- Multiple document windows
- Refresh / reload support
- Selection detail pane
- Editing, dirty-state tracking, and save flows
- Create / delete / rename operations

