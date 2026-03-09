# UserTalk Phase A Dependency Inventory

## Purpose

This document defines the Phase 0 dependency inventory for Phase A of `usertalk`: parser/core extraction only.

Goals:

- identify the minimum parse/build subset that must move into `usertalk/`
- define the cut line between Phase A parser/core work and Phase B evaluator/runtime work
- identify bootstrap and compatibility blockers before implementation begins

This inventory is based on the current Frontier parser path in:

- `Common/source/lang.c`
- `Common/source/langscan.c`
- `Common/source/langparser.y`
- `Common/source/langtree.c`
- parse-time bootstrap in `Common/source/langstartup.c`
- builtin bootstrap in `Common/source/langverbs.c`

## Executive Summary

The parser/build path is not self-contained. It depends on four major support layers:

1. legacy core types and macros
2. value/Handle/string infrastructure
3. hash-table/bootstrap infrastructure for keywords, constants, and builtins
4. tree construction, disposal, and pack/unpack support

The biggest finding is that `lang.c` itself should not be copied wholesale into Phase A. Its parse entry point is usable as a model, but the file is too entangled with evaluation and broader runtime/global state.

The most likely Phase A extraction strategy is:

- do not copy `lang.c` as-is
- instead, recreate a smaller parse driver in `usertalk/src/`
- copy or adapt `langscan.c`, `langparser.y`, and `langtree.c`
- copy the minimum value/hash/string/memory support they require
- recreate parser bootstrap tables locally in `usertalk`

## Phase A / Phase B Cut Line

### In Scope For Phase A

- scanner setup and tokenization
- grammar generation and parse driver
- AST node allocation, disposal, pack, unpack, copy
- parse-time value creation for identifiers/constants/token nodes
- parse-time keyword/constant/builtin lookup tables
- parse-time error capture and source position tracking
- compile-time parser rewrites:
  - assignment optimizations
  - builtin token substitution
  - kernel call token/address resolution, if retained in Phase A semantics

### Out Of Scope For Phase A

- AST evaluation
- variable lookup and symbol resolution during execution
- handler invocation
- builtin verb execution
- runtime stacks and temp-stack execution semantics beyond what parse-time value handling strictly needs
- database integration
- debugger/modeless/dialog/card subsystems

## Primary Entry Path Findings

### Current Entry Path

Current parse entry flow:

1. `langcompiletext` in `Common/source/lang.c`
2. `parsesetscanstring` in `Common/source/langscan.c`
3. `yyparse()` from `langparser.y`
4. AST builder calls into `langtree.c`
5. parse error capture via `langcompileerror` / `parseerror`

### Recommendation

`langcompiletext` should be used as a behavioral reference, not as a source file to copy intact.

Reasons:

- `lang.c` owns many globals unrelated to parser/core extraction
- `lang.c` includes broad application/runtime headers
- `lang.c` contains evaluation-adjacent helpers and runtime glue that Phase A does not want

Phase A should instead reimplement a small parse driver with equivalent behavior for:

- scanner initialization
- temporary error callback installation
- `yyparse()` invocation
- AST handoff/disposal on success/failure
- line/character preservation

## Dependency Classes

### 1. Core Types And Macros

Phase A needs local copies or equivalents for:

- `boolean`, `byte`, `Handle`, `bigstring`, `hdlstring`
- `tytreetype`
- `tyvaluetype`
- `tyvaluerecord`
- `tytreenode`
- `hdltreenode`
- basic utility macros:
  - `nil`
  - `longsizeof`
  - `clearbytes`
  - string length helpers
  - endian/packing helpers used by tree packing

Likely source origins:

- `frontier.h`
- `standard.h`
- `lang.h`
- low-level memory/string headers

Phase A action:

- copy a minimal, parser-core-only compatibility header set into `usertalk/src/compat_types.h` and related local headers

### 2. Scanner Layer

Primary source:

- `Common/source/langscan.c`

Phase A-relevant exported functions:

- `parsesetscanstring`
- `parsegetscanoffset`
- `parsesetscanoffset`
- `parsegettoken`
- `isfirstidentifierchar`
- `isidentifierchar`
- `langisidentifier`
- `langdeparsestring`

Observed scanner dependencies:

- token defines from generated parser header
- string helpers
- Handle/text storage helpers
- value creation helpers
- hash lookup for keywords/constants
- parse-time globals:
  - `ctscanlines`
  - `ctscanchars`
  - scanner input globals
  - `bstoken`

Phase A action:

- copy scanner logic substantially intact
- move scanner globals under `usertalk` ownership where practical
- preserve tokenization behavior exactly

### 3. Grammar Layer

Primary source:

- `Common/source/langparser.y`

Grammar action dependencies:

- `newconstnode`
- `newidnode`
- `pushquadruplet`
- `pushtriplet`
- `pushbinaryoperation`
- `pushunaryoperation`
- `pushoperation`
- `pushloop`
- `pushloopbody`
- `pushunarystatementlist`
- `pushtripletstatementlists`
- `pushfunctioncall`
- `pushkernelcall`
- `pushlastlink`
- `pushlastoptionallink`
- `langdisposetree`
- `parseerror`

Phase A action:

- copy grammar source with minimum changes needed for GNU Bison
- keep semantic actions stable
- provide a local parser header exposing `YYSTYPE`, `yylval`, `yyval`, and scanner callback glue

### 4. Tree Construction And Compatibility Layer

Primary source:

- `Common/source/langtree.c`

Required exported functionality:

- `langvisitcodetree`
- `langcounttreenodes`
- `langdisposetree`
- `newconstnode`
- `newidnode`
- `pushquadruplet`
- `pushtriplet`
- `pushbinaryoperation`
- `pushunaryoperation`
- `pushoperation`
- `pushloop`
- `pushloopbody`
- `pushunarystatementlist`
- `pushtripletstatementlists`
- `pushfunctioncall`
- `pushkernelcall`
- `pushfunctionreference`
- `pushlastlink`
- `pushlastoptionallink`
- `langpacktree`
- `langunpacktree`
- `langcopytree`

Key findings:

- tree allocation is custom and localizable
- `pushbinaryoperation` performs required parse-time assignment rewrites
- `pushfunctioncall` depends on builtin lookup in `hbuiltinfunctions`
- `pushkernelcall` depends on address construction and table lookup, which is one of the most Phase A-sensitive dependencies
- packed-tree support depends on `langpackvalue` / `langunpackvalue` and exact legacy struct layout

Phase A action:

- copy tree layer largely intact
- preserve struct packing and node layout exactly
- explicitly isolate `pushkernelcall` dependencies during extraction design

### 5. Value Layer

Primary sources:

- `Common/source/langvalue.c`
- `Common/headers/lang.h`

Minimum parse/build-facing functions observed:

- `initvalue`
- `setbooleanvalue`
- `setcharvalue`
- `setintvalue`
- `setlongvalue`
- `setdirectionvalue`
- `setostypevalue`
- `setstringvalue`
- `setaddressvalue`
- `copyvaluedata`
- `copyvaluerecord`
- `disposevaluerecord`
- `exemptfromtmpstack`

Plus pack/unpack compatibility:

- `langpackvalue`
- `langunpackvalue`

Key finding:

- The tree and scanner do not need the full runtime value system, but they do need a real subset of value creation/copy/disposal semantics
- temp-stack interaction appears even during parsing because predefined constants and address values are exempted before being attached to AST nodes

Phase A action:

- inventory and copy the minimum value operations needed by scanner, tree, bootstrap, and pack/unpack
- do not attempt to simplify value semantics until compatibility is proven

### 6. Memory And Handle Layer

Observed dependencies:

- `newtexthandle`
- `newheapstring`
- generic Handle allocation/disposal/resize/read helpers
- `malloc`-based local tree allocation inside `langtree.c`
- byte/handle copying helpers used by pack/unpack and strings

Likely source origins:

- `memory.h`
- `memory.track.c`
- low-level utility code

Phase A action:

- build a local Handle/memory support layer in `usertalk`
- keep public API free of `Handle`
- preserve internal Handle semantics where AST/value compatibility depends on them

### 7. String Layer

Observed dependencies:

- Pascal string manipulation
- `copystring`
- `pushstring`
- `lastword`
- `convertcstring`
- `setstringwithchar`
- `pushchar`
- `midinsertstring`
- `stringlength`, `stringsize`
- resource-string decoding helpers if resource-based bootstrap were retained

Key finding:

- parse/build path fundamentally uses Frontier Pascal strings internally

Phase A action:

- copy the minimal Pascal-string support required by scanner, bootstrap, value, and tree code

### 8. Hash Table Layer

Observed parser/build-facing functions:

- `hashtableassign`
- `hashtablelookup`
- `hashtablelookupnode`
- `hashinsert`
- `pushhashtable`
- `pophashtable`
- `tablenewsystemtable`

Observed parser/build-facing globals:

- `hkeywordtable`
- `hconsttable`
- `hbuiltinfunctions`

Key findings:

- scanner depends directly on `hkeywordtable` and `hconsttable`
- tree builder depends directly on `hbuiltinfunctions`
- current bootstrap uses full table infrastructure, not a parser-local flat map

Phase A action:

- either:
  - copy the minimal hash/table layer needed for parser bootstrap, or
  - replace it with a behavior-compatible parser-local table implementation behind the same local API

Recommendation:

- keep names and lookup semantics close to current code in Phase A
- defer any internal table redesign until after compatibility is established

### 9. Error And Trace Layer

Observed parse/build dependencies:

- `parseerror`
- `langstarttrace`
- `langendtrace`
- `langcallbacks.errormessagecallback`
- `ctscanlines`
- `ctscanchars`
- first-error capture behavior in `langcompiletext`

Key finding:

- current parser error flow is split across `lang.c`, `langerror.c`, and callback state
- tracing can likely be stubbed or made no-op in Phase A if outward behavior is preserved

Phase A action:

- implement a local `usertalk` error capture path modeled on `langcompiletext`
- preserve first-error semantics
- preserve line/character behavior
- make tracing a local no-op unless needed for debug builds

### 10. Bootstrap Tables

Current bootstrap sources:

- keywords/constants/basic builtin tokens in `Common/source/langstartup.c`
- resource-driven builtin registration in `Common/source/langverbs.c`

Relevant functions:

- `langinitconsttable`
- `langinitbuiltintable`
- `langinitkeywordtable`
- `langinitbuiltins`
- `loadfunctionprocessor`

Key findings:

- constants and keywords are explicit and easy to reproduce locally
- builtin token bootstrap is split:
  - a small explicit builtin token table in `langstartup.c`
  - additional resource-driven builtin registration in `langverbs.c`
- `langinitbuiltins` currently depends on resources and runtime callback wiring

Phase A action:

- this area needs a design decision before coding

Recommendation:

- recreate the keyword table and constant table directly in `usertalk`
- recreate the parse-time builtin token table directly in `usertalk`
- treat resource-driven builtin bootstrap as a separate compatibility question to resolve in the Bison/bootstrap spike

Open blocker:

- confirm exactly which builtin names must be present during parsing for AST compatibility, and whether any of them are only introduced by resource-driven startup

## Copy / Wrap / Shim / Exclude Classification

### Copy As-Is Or Near-As-Is

- `langscan.c` core scanner logic
- `langparser.y` grammar and semantic actions
- `langtree.c` core tree allocation/build/dispose logic
- exact tree struct layout and packing definitions

### Copy But Trim To Parse/Core Subset

- value creation/copy/dispose helpers
- Handle/memory helpers
- Pascal-string helpers
- hash/table helpers
- parse error capture logic modeled after `langcompiletext`
- tree pack/unpack value support

### Wrap Or Reimplement Locally

- top-level parse driver currently in `lang.c`
- public API surface
- parser object ownership
- error retrieval and message storage
- file loading for `parse_file`
- trace hooks

### Exclude From Phase A

- evaluator entry points
- `evaluatetree`
- `evaluatelist`
- symbol resolution during execution
- handler invocation/runtime stacks
- UI/process/dialog/debugger integrations
- database and system integration not required for parse/build/bootstrap

## Known High-Risk Areas

### 1. `pushkernelcall`

Why it is risky:

- it resolves dotted kernel references during parsing
- it depends on table/address helpers rather than just syntax construction

Implication:

- Phase A must either copy enough address/table logic to preserve current behavior, or document and validate a narrow parser-local equivalent

### 2. Packed Tree Compatibility

Why it is risky:

- exact byte compatibility depends on:
  - node layout
  - value packing
  - type IDs
  - byte-order handling

Implication:

- `langpackvalue` / `langunpackvalue` are part of the true Phase A dependency surface, not optional extras

### 3. Bootstrap Coupling

Why it is risky:

- current parse-time tables are initialized through larger runtime startup paths

Implication:

- `usertalk` needs a self-contained bootstrap layer
- Phase A cannot rely on global Frontier startup

### 4. `lang.c` Entanglement

Why it is risky:

- the obvious compile entry point lives in a file with broad runtime coupling

Implication:

- Phase A should not copy `lang.c` whole
- a clean-room parse driver based on `langcompiletext` behavior is safer

## Recommended File-Level Extraction Map

### New `usertalk` Files To Create

- `src/driver.c`
  - local replacement for `langcompiletext` behavior
- `src/scan.c`
  - copied/adapted from `langscan.c`
- `src/grammar.y`
  - copied/adapted from `langparser.y`
- `src/tree.c`
  - copied/adapted from `langtree.c`
- `src/value.c`
  - parse/core subset from `langvalue.c`
- `src/pack.c`
  - value/tree packing subset
- `src/hash.c`
  - minimal table/hash support
- `src/strings.c`
  - Pascal string/text-handle helpers
- `src/memory.c`
  - Handle/memory support
- `src/bootstrap.c`
  - keyword/constant/builtin token initialization
- `src/errors.c`
  - parser/core error capture
- `src/compat_types.h`
  - copied type subset

### Existing Frontier Files To Treat As Behavioral References Only

- `Common/source/lang.c`
- `Common/source/langstartup.c`
- `Common/source/langverbs.c`

## Concrete Preconditions Before Implementation

These items must be resolved before extraction coding starts:

1. Bison feasibility spike
   - prove `langparser.y` can generate under GNU Bison with minimal deltas
2. Bootstrap decision
   - define exactly how `usertalk` will initialize keyword/constants/builtin token tables without Frontier startup
3. Packed value subset inventory
   - determine the minimum `langpackvalue` / `langunpackvalue` support required for AST compatibility tests
4. `pushkernelcall` strategy
   - preserve current behavior by copied support, or isolate a parser-local equivalent with validation

## Immediate Next Steps

1. Run the Bison spike against a copied `langparser.y`
2. Build a symbol-level inventory for:
   - value helpers used by scanner/tree/bootstrap/pack
   - hash helpers used by parser bootstrap and `pushkernelcall`
3. Draft the bootstrap design for `usertalk`
4. Draft the corpus comparison harness so compatibility checks can begin as soon as Phase A code compiles
