# libodb

## Overview
`libodb` is the cross-platform ODB engine extracted from Frontier. It preserves the Frontier 10 database file format and provides a C API compatible with the legacy ODB Engine header (`odb_engine.h`). The focus is read/write correctness with minimal dependencies.

## Build & Test
Configure and build with CMake:

```sh
cmake -S . -B build_libodb
cmake --build build_libodb --target odb_cli
```

Run tests:

```sh
ctest --test-dir build_libodb
```

Tests use `databases/new.root` as the fixture.

## API Surface
Public entry points live in `libodb/include/odb_engine.h`. Key functions include:
- Open/create: `odbOpenPath`, `odbNewPath`, `odbOpenFile`, `odbNewFile`
- Read: `odbGetType`, `odbGetValue`, `odbCountItems`, `odbGetNthItem`, `odbGetModDate`, `odbDefined`
- Write: `odbSetValue`, `odbNewTable`, `odbDelete`, `odbSaveFile`
- Cleanup: `odbDisposeValue`

### Value Types
Currently, `odbSetValue` supports:
- `stringvaluetype` (Pascal-style string `Handle`)
- `longvaluetype`
- `booleanvaluetype`

Other types are read-only for now. Non-table externals are returned as opaque packed blobs (`externalvaluetype`).

## Paths & Naming
Paths are dot-separated and case-insensitive (e.g., `root.workspace.firstName`). Each segment must be a valid identifier:
- First character: `[A-Za-z]`
- Remaining characters: `[A-Za-z0-9]`

## Errors & Ownership
- Functions return `odbBool` (1 = success, 0 = failure). Use `odbGetError` and `odbGetLastErrorCode` for diagnostics.
- When `odbGetValue` succeeds, call `odbDisposeValue` to release owned memory.
- `Handle` is an internal abstraction; strings are Pascal-style (`len` byte + data).

## CLI
`odb_cli` is a minimal harness for manual inspection and regression testing.

Examples:

```sh
odb_cli databases/new.root list root.workspace
odb_cli databases/new.root get root.workspace.firstName
odb_cli /tmp/test.root newfile
odb_cli /tmp/test.root newtable root.workspace
odb_cli /tmp/test.root set root.workspace.age long 43
odb_cli /tmp/test.root moddate root.workspace
```

`set`, `delete`, and `newtable` auto-save on success; `save` persists explicit changes.
