# ObjectDB User Guide

`objectdb` is a desktop GUI for browsing and editing Frontier `.root` files.

## Open A Database

1. Launch `objectdb`.
2. Choose `File -> Open...`.
3. Select a `.root` file (for example `databases/new.root`).

## Browse The Table Tree

- The left column (`Name`) shows entries.
- The middle column (`Value`) shows the current value.
- The right column (`Kind`) shows the value type.
- Expand table rows to view child entries.

## Edit A Value

1. Double-click a row’s `Value` cell.
2. Type a new value.
3. Press `Return` to apply the in-memory change.

Notes:
- Changes are **not** saved to disk until you explicitly save.
- Some kinds have specific formats (for example `rect` as `top,left,bottom,right`).

## Change A Value Type (Kind)

1. Click the `Kind` cell on a non-table row.
2. Select a type from the dropdown list.

The app assigns a default value for the selected type and updates both `Kind` and `Value`.

## Add A New Entry

Use:

`Table -> New -> Scalar -> <Type>`

Available scalar types:

- `Address`
- `Boolean`
- `Character`
- `Date`
- `Direction`
- `Float`
- `Long`
- `String`

Flow:

1. Select any row in the table area.
2. Choose scalar type from the menu.
3. Enter the new entry name in the dialog.
4. Click `OK`.

Behavior:

- If the selected row is a table, new entry is added there.
- If the selected row is not a table, new entry is added to the containing table.

## Delete An Entry

1. Select the row to remove.
2. Choose `Edit -> Clear`.

The row is removed from the current view and marked as unsaved.

## Save Changes

- `File -> Save`: saves changes to the current file.
- `File -> Save As...`: saves to a new file path and keeps that file open.

## Unsaved Changes Warning

If there are unsaved changes, ObjectDB prompts when you:

- close the window,
- quit the app,
- open another file.

Options:

- `Save` to write changes,
- `Discard` to continue without saving,
- `Cancel` to stay in the current file.

## Current Editing Scope

The app currently supports interactive editing for core scalar/struct types and type changes through the `Kind` dropdown. Some advanced binary/external handle-style types are still limited in UI editing.
