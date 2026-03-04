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
- Expansion state is preserved during paste/rename refreshes.

## Rename An Entry

1. Click a row’s `Name` cell.
2. Type a new name.
3. Press `Return` to apply the in-memory rename.

Notes:
- Root and placeholder rows are not renamable.
- Name conflicts in the same table are rejected.

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

Keyboard shortcut:

- Press `Enter` on a selected row to create a new `String` entry in that row’s table context.

### Add A Sub-Table

Use:

`Table -> New -> Sub-Table`

Behavior:

- Uses the selected table, or the containing table if a non-table row is selected.
- Prompts for the sub-table name.

## Delete An Entry

1. Select the row to remove.
2. Choose `Edit -> Clear`.

The row is removed from the current view and marked as unsaved.

## Cut, Copy, And Paste Entries

Use the `Edit` menu:

- `Edit -> Cut`
- `Edit -> Copy`
- `Edit -> Paste`

Behavior:

- Works for normal entries and sub-tables.
- Pasting targets the selected table, or containing table for non-table selections.
- `Cut` moves the entry; `Copy` duplicates it.
- Sub-tables paste recursively with all descendants.

If a pasted name already exists in the destination table:

- The app prompts for a replacement name.
- You can enter a new name and retry, or cancel the paste.

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

The app supports interactive editing for core scalar/struct types, type changes via `Kind`, entry rename, sub-table creation, and cut/copy/paste for entries. Some advanced binary/external handle-style types are still limited in UI editing.
