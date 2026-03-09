# UserTalk Extraction Plan

## Goal

Extract the Frontier language parser into a new top-level module at `usertalk/` that builds as a static library with CMake on Linux using C23.

This plan covers Phase A only: parser/core extraction. Evaluation/runtime extraction is intentionally deferred to a later phase so the parser boundary, AST compatibility, and Bison migration can be completed and validated first.

V1 scope:

- Parsing only
- No evaluation
- C API only
- Public API uses standard C types and opaque `usertalk` objects
- Internal behavior remains syntax-compatible with the current Frontier parser
- Packed tree output must remain byte-for-byte compatible
- Existing script/database artifacts must continue to be supported
- The new module must not depend on `Common/headers` or other shared Frontier code
- The new module must preserve current parser behavior, error behavior, builtin/keyword resolution behavior, and compile-time parser rewrites
- The existing in-tree parser remains in place and is not migrated in this effort
- The extracted parser/core should be structured so evaluation/runtime can be layered in later without breaking AST or packed-tree compatibility

## Non-Goals

- No language changes
- No evaluator extraction
- No database format changes
- No runtime behavior improvements outside parsing
- No changes to the existing Frontier parser integration path beyond optional test reuse

## Future Phase

Evaluation is expected to be a follow-on effort after Phase A is complete.

That later phase is expected to add:

- evaluator/runtime dependency extraction
- execution APIs
- value marshaling APIs
- runtime/bootstrap state for execution
- execution behavior tests

Phase A should not implement evaluation, but it should avoid design choices that block that later work.

## Key Constraints

### Public API

- Opaque `usertalk` object owns the parse tree
- Input API must support:
  - `const char *` text input
  - file path based parsing
- Callers must use accessors, helpers, and visitor/introspection APIs rather than direct `hdltreenode` access
- Public API should include:
  - `usertalk` create/destroy
  - parse from string
  - parse from file
  - success/failure query
  - first error retrieval and location
  - tree access through opaque ownership
  - tree disposal through `usertalk` destruction or explicit reset
  - pack/unpack helpers
  - AST introspection and visitor helpers

Design constraint for future evaluation layering:

- The `usertalk` object and internal core types should be designed so a future runtime object can reuse the same AST and compatibility layer without forcing an incompatible public API break

### Internal Compatibility

- Keep the grammar source in `langparser.y`
- Generate parser C from that grammar with GNU Bison on Linux
- Preserve current scanner and parser semantics
- Preserve current line/character tracking behavior, including the current line-based mode behavior
- Preserve keyword table, constant table, and builtin function resolution behavior
- Preserve existing AST node layout and packing behavior needed for byte-for-byte packed tree compatibility
- Preserve legacy internal data structures where needed, including Handle- and Pascal-string-based internals

### Build/Test

- New top-level module name: `usertalk`
- CMake only
- Build on Linux with both `clang` and `gcc`
- Tests run via CTest
- Validation corpus is a new hand-written corpus added to the repo
- Success criteria focus on parse success, AST shape, and packed-tree compatibility

## Current Parser Boundary

The current parser is not a single file. The minimum extraction target includes at least these responsibilities:

- Scanner setup and tokenization
- Grammar and parse driver
- AST node allocation and disposal
- Value construction for identifiers, constants, and builtins
- Parse-time error reporting
- Source position tracking
- Tree pack/unpack support
- Table/bootstrap logic for keywords, constants, and builtins

The main parser entry path today is:

1. `langcompiletext` in `Common/source/lang.c`
2. scanner setup in `langscan.c`
3. grammar in `langparser.y`
4. AST construction helpers in `langtree.c`
5. error plumbing in `langerror.c` plus callbacks/global state

The extraction must isolate only the parse/build path and stop before evaluation.

## Expected Dependency Inventory

The parser depends on more than just `langscan.c` and `langparser.y`. The extraction plan must inventory and copy the minimum parse-time subset of legacy support code into `usertalk/`.

Expected dependency classes:

- Fundamental types and macros
- Memory and Handle utilities
- Pascal string and text-handle utilities
- Value record definitions and value-copy/dispose helpers
- Tree node definitions and tree helpers
- Hash table support required to populate and query keyword/constant/builtin tables
- Error state and callback plumbing used during parse
- Byte-order/packing helpers used by pack/unpack
- Any parser-required initialization tables or resources currently loaded elsewhere

This inventory must be completed before implementation starts. No code movement should begin before each copied dependency is categorized as one of:

- required in V1
- replaceable by local shim without behavior change
- not needed for parsing

## Proposed Module Layout

```text
usertalk/
  CMakeLists.txt
  PLAN.md
  README.md
  include/
    usertalk.h
  src/
    api.c
    api_internal.h
    parser_driver.c
    parser_driver.h
    scan.c
    scan.h
    grammar.y
    tree.c
    tree.h
    value.c
    value.h
    memory.c
    memory.h
    strings.c
    strings.h
    handles.c
    handles.h
    hash.c
    hash.h
    errors.c
    errors.h
    pack.c
    pack.h
    bootstrap.c
    bootstrap.h
    compat_types.h
  tests/
    corpus/
    parse_success_tests.c
    ast_shape_tests.c
    packed_tree_tests.c
    file_api_tests.c
```

Notes:

- File names above are planning placeholders, not final commitments.
- `grammar.y` should stay close to the original `langparser.y` to minimize divergence.
- The extracted copy should preserve legacy internal layout where compatibility depends on it.

## Public API Direction

The public API should present a modern C surface while hiding legacy internals.

Planned API categories:

- `usertalk` lifecycle
- Parse from memory
- Parse from file
- Parse status and error retrieval
- Tree packing/unpacking
- Tree traversal
- Tree/node introspection

Representative API shape:

- `usertalk_create`
- `usertalk_destroy`
- `usertalk_reset`
- `usertalk_parse_string`
- `usertalk_parse_file`
- `usertalk_has_tree`
- `usertalk_get_error`
- `usertalk_visit`
- `usertalk_node_type`
- `usertalk_node_child`
- `usertalk_node_link`
- `usertalk_pack_tree`
- `usertalk_unpack_tree`

The `usertalk` object remains the sole owner of the tree unless an explicit transfer API is added later.

Phase A note:

- This API is intentionally parser-only in Phase A
- Future evaluator support should be layered as an additional runtime API rather than by changing the `usertalk` object's ownership and introspection model

## Compatibility Strategy

Compatibility is the main risk. The extraction should therefore optimize for behavioral sameness over cleanup.

Rules:

- Preserve the grammar structure as closely as possible
- Preserve scanner tokenization order and special cases exactly
- Preserve builtins/keywords/constants resolution behavior
- Preserve node layout and field sizes
- Preserve tree link/param layout semantics
- Preserve tree packing version and byte layout
- Preserve parse-time rewrites such as assignment optimization and builtin function token substitution
- Preserve current error callback semantics internally even if the public API wraps them

Preferred technique:

- Copy the parse-time code path first with minimal edits
- Build a wrapper C API around it
- Only then introduce localized shims where build isolation requires them

Phase sequencing note:

- Parser/core compatibility is the prerequisite for any future evaluator extraction
- No execution-layer changes should be attempted until parser compatibility is demonstrated over the test corpus

## Bison Migration Plan

The existing grammar source says `langparser.y` was historically generated with MacYACC. V1 requires GNU Bison generation on Linux.

This must be handled as a dedicated sub-phase, not folded into general extraction.

Steps:

1. Copy the grammar into `usertalk/src/grammar.y`
2. Establish a Bison generation target in CMake
3. Identify and patch only the minimum generator-compatibility deltas needed for Bison output
4. Keep semantic actions unchanged unless a Bison compatibility fix is strictly required
5. Verify that generated parser behavior matches the current parser over the test corpus

Acceptance criteria for this phase:

- Grammar generates on Linux with GNU Bison
- Generated parser compiles with both `clang` and `gcc`
- Corpus parse results match the legacy parser path

## Phased Implementation Plan

### Phase 0: Dependency Inventory And Boundary Definition

Deliverable:

- A concrete file/function/type inventory for the parse-only subset

Tasks:

1. Enumerate every function, type, macro, and global referenced directly or transitively from:
   - `lang.c` parse entry path
   - `langscan.c`
   - `langparser.y`
   - `langtree.c`
   - pack/unpack code used for compatibility
2. Mark each dependency as:
   - copy as-is
   - wrap
   - locally shim
   - exclude
3. Define the exact cut line between parse-time and evaluation-time code
4. Identify all global state that must move into a parser-owned context versus what can remain process-global inside the module

Gate:

- No implementation work starts until this inventory is complete

### Phase 1: Scaffold The New Module

Deliverable:

- `usertalk/` builds as a static library with a placeholder public header and test target wiring

Tasks:

1. Create `usertalk/`
2. Add `CMakeLists.txt`
3. Add top-level `add_subdirectory(usertalk)` to the root CMake file
4. Add skeleton public header and empty library target
5. Add CTest wiring for parser tests

### Phase 2: Copy The Internal Parse-Time Subset

Deliverable:

- Internal sources compile without depending on `Common/headers`

Tasks:

1. Copy the minimum required types, macros, and helpers into module-local headers/sources
2. Copy scanner, tree, value, error, and pack/unpack logic required for parsing
3. Remove direct include dependencies on `Common/headers`
4. Keep internal representations compatible with the legacy code

Gate:

- Internal code compiles in isolation, even if the public API is still incomplete

### Phase 3: Make The Grammar Generate With Bison

Deliverable:

- `grammar.y` generates parser C on Linux under CMake

Tasks:

1. Import grammar source
2. Update generation rules for GNU Bison
3. Resolve generator-specific incompatibilities
4. Verify generated parser links against the copied scanner/tree support

Gate:

- Parser generation is deterministic in the Linux build

### Phase 4: Recreate Parser Bootstrap State

Deliverable:

- Parser module can initialize and own the parse-time tables it needs

Tasks:

1. Recreate keyword table initialization
2. Recreate predefined constant table initialization
3. Recreate builtin function table initialization
4. Recreate any parser-required error or callback defaults
5. Ensure parser lifecycle handles bootstrap/teardown safely

Gate:

- `parse_string` can run without any dependency on the host Frontier runtime

### Phase 5: Build The Public C API

Deliverable:

- Opaque `usertalk` object with stable parse APIs and introspection helpers

Tasks:

1. Define `usertalk_t`
2. Wrap parse entry behavior currently exposed through `langcompiletext`
3. Add memory-buffer and file-path parse APIs
4. Preserve current line/character tracking modes in the API
5. Add error retrieval APIs that expose current behavior without exposing internal globals
6. Add tree accessors and visitors
7. Add pack/unpack public helpers

Gate:

- Tests can consume the module without touching internal headers

### Phase 6: Add AST Introspection And Visitor Support

Deliverable:

- Stable public tree inspection API sufficient for shape assertions

Tasks:

1. Expose node kind enumeration
2. Expose child/link traversal
3. Expose source location access
4. Expose limited value inspection needed for constants, identifiers, and builtin-token nodes
5. Add a visitor API for whole-tree traversal

Gate:

- AST shape tests can be written entirely through the public API

### Phase 7: Build The Test Corpus And Compatibility Suite

Deliverable:

- Hand-written corpus plus automated compatibility checks under CTest

Corpus categories:

- empty statements
- identifiers and bracketed identifiers
- strings, chars, numbers, escapes
- assignments and optimized assignments
- dotted references
- array and range references
- function calls with positional and named params
- handlers
- local/global declarations
- loops, while, for, for-in, fileloop
- if/else
- case
- with
- try/else
- lists and records
- syntax errors and location reporting

Test classes:

- parse success/failure expectations
- AST shape checks through visitor/introspection APIs
- packed-tree byte comparison
- file API behavior
- compiler matrix runs under `clang` and `gcc`

Gate:

- New parser passes the full corpus

### Phase 8: Document The Module

Deliverable:

- Module-level documentation sufficient for build and API use

Docs:

- `README.md`
- build instructions
- public API overview
- compatibility guarantees
- testing instructions
- known limitations of V1

## Follow-On Phase B: Evaluator/Runtime Extraction

This is out of scope for the current implementation plan, but the parser/core extraction should leave room for it.

Expected scope of Phase B:

- extract evaluation/runtime dependencies
- define runtime lifecycle and execution APIs
- expose value input/output APIs
- bootstrap symbol tables and runtime state needed for execution
- validate execution behavior against a dedicated corpus

Expected prerequisites:

- parser/core extraction complete
- AST introspection API stable
- packed-tree compatibility proven for the Phase A corpus
- Bison-based parser generation stable on Linux

## Test Strategy

### Primary Success Criteria

- The library builds on Linux with CMake using both `clang` and `gcc`
- The parser accepts the same language syntax as the current Frontier parser over the corpus
- AST shape matches expected structure for the corpus via public traversal APIs
- Packed trees remain byte-for-byte compatible for corpus cases

### Comparison Strategy

For each corpus item:

1. Parse with the legacy parser path
2. Parse with the extracted parser
3. Compare:
   - success/failure result
   - first error location and message behavior where practical
   - AST shape through normalized traversal
   - packed tree bytes

The extracted parser is not accepted unless packed tree bytes match for the compatibility subset.

## Main Risks

### 1. Hidden Parse-Time Dependencies

Risk:

- Parser code may rely on globals or utility behavior that appears unrelated at first glance

Mitigation:

- Complete the dependency inventory before code movement
- Port globals into a parser-owned context only when safe

### 2. Bison Compatibility

Risk:

- The grammar was written for MacYACC and may need careful adaptation

Mitigation:

- Isolate generator changes in a dedicated phase
- Keep semantic actions stable
- Validate against the corpus after every grammar delta

### 3. Packed Tree Compatibility

Risk:

- Small changes to struct layout, value representation, or packing code can break byte identity

Mitigation:

- Preserve legacy internal types where compatibility requires them
- Add packed-tree byte tests early

### 4. Bootstrap Tables

Risk:

- Builtin/keyword/constant resolution may currently rely on broader runtime startup paths

Mitigation:

- Rebuild those tables inside the module with explicit initialization code
- Test builtin token substitution directly

### 5. Over-Refactoring

Risk:

- Cleanup work can easily change behavior in a legacy parser

Mitigation:

- Prefer copy-first, wrap-second, refactor-last
- Keep V1 invasive changes to the minimum required for isolation and Linux buildability

## Recommended First Execution Steps

1. Add `usertalk/` with `PLAN.md`, `README.md`, and `CMakeLists.txt` scaffolding
2. Perform the dependency inventory and publish the results in a follow-up design note
3. Prototype Bison generation from a copied `langparser.y` before moving large amounts of support code
4. Build the initial hand-written corpus in parallel with extraction work
5. Only after those steps, begin copying the parse-time support layer

## Acceptance Criteria For The Extraction Effort

- `usertalk` builds as a static library via CMake on Linux
- No dependency remains on `Common/headers` or shared Frontier source
- Public API is C-only and uses standard C types plus opaque parser ownership
- Parser behavior matches existing syntax and parse-time semantics over the corpus
- ASTs are inspectable through the public visitor/introspection API
- Packed trees are byte-for-byte compatible for the validated corpus
- Tests run under CTest with both `clang` and `gcc`
- Existing in-tree parser remains untouched as the production path
