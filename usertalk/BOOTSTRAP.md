# UserTalk Phase A Bootstrap Design

## Purpose

Define how `usertalk` Phase A will initialize the parse-time tables and state required for syntax-compatible AST construction without depending on Frontier startup, `Common/headers`, classic resources, or application globals.

This document covers parse/core bootstrap only.

It does not define:

- evaluator/runtime bootstrap
- execution-time symbol tables
- database-backed startup state

## Bootstrap Requirements

Phase A parsing depends on four categories of bootstrap data/state:

1. keyword token table
2. predefined constant table
3. builtin function token table
4. kernel EFP table used by `pushkernelcall`

It also needs:

- a parser-owned error state
- scanner line/character state
- parser-owned top-level bootstrap lifecycle

## Current Frontier Sources

### Keywords, Constants, Basic Builtins

Current sources:

- `Common/source/langstartup.c`

Relevant functions:

- `langinitconsttable`
- `langinitbuiltintable`
- `langinitkeywordtable`

Findings:

- these tables are explicit in source
- they do not fundamentally require resource loading
- they currently populate Frontier hashtables and rely on broader system-table infrastructure

### Resource-Driven EFP Bootstrap

Current sources:

- `Common/source/langstartup.c`
- `Common/source/langverbs.c`
- other `loadfunctionprocessor(...)` users across the tree

Relevant functions:

- `loadfunctionprocessor`
- `newfunctionprocessor`
- `langinitbuiltins`

Findings:

- Frontier builds an `efptable`
- each EFP gets a subtable of verbs
- those EFP subtables carry:
  - verb names
  - token values
  - a non-NULL `valueroutine`
- `pushkernelcall` validates `kernel(EFP.verb)` against this table at parse time

## Design Principle

Phase A bootstrap should be split into:

1. mandatory parser-core bootstrap
2. optional or separately-scoped kernel/EFP bootstrap

The mandatory parser-core bootstrap must be fully self-contained and source-defined.

## Phase A Bootstrap Architecture

Recommended new internal file:

- `usertalk/src/bootstrap.c`

Recommended responsibilities:

- build parser-local keyword table
- build parser-local constant table
- build parser-local builtin-function token table
- initialize any parse-time EFP/kernel tables needed for compile-time validation
- expose a single internal bootstrap entry point used by `usertalk_create`

Suggested internal entry points:

- `usertalk_bootstrap_init`
- `usertalk_bootstrap_dispose`
- `usertalk_bootstrap_reset`

## 1. Keyword Bootstrap

### Required Behavior

The scanner must convert reserved words into the same token values as the current parser.

Current token mapping source:

- `Common/source/langstartup.c`
- `Common/headers/langtokens.h`
- `Common/headers/yytab.h`

### Recommendation

Define the keyword table statically in `usertalk/src/bootstrap.c` using source literals and token constants.

Suggested representation:

```c
typedef struct usertalk_keyword_def_t {
	const char *name;
	unsigned short token;
} usertalk_keyword_def_t;
```

Populate from the current explicit list:

- `equals`
- `notequals`
- `greaterthan`
- `lessthan`
- `not`
- `and`
- `or`
- `beginswith`
- `endswith`
- `contains`
- `loop`
- `fileloop`
- `while`
- `in`
- `break`
- `continue`
- `return`
- `if`
- `then`
- `else`
- `bundle`
- `local`
- `on`
- `case`
- `kernel`
- `for`
- `to`
- `downto`
- `with`
- `try`

### Decision

- do not reproduce this through `tablenewsystemtable` or resource loading
- materialize it directly in `usertalk`

## 2. Constant Bootstrap

### Required Behavior

The scanner must resolve predefined constants exactly as current Frontier does.

Current source:

- `langinitconsttable` in `Common/source/langstartup.c`

Categories observed:

- `nil`
- `infinity`
- directions
- boolean constants
- generated `*Type` constants
- machine constants
- OS constants

### Recommendation

Build a source-defined constant table in `usertalk`.

Suggested representation:

```c
typedef struct usertalk_const_def_t {
	const char *name;
	usertalk_const_kind_t kind;
	long long_value;
	double double_value;
	unsigned short token_or_enum;
	const char *string_value;
} usertalk_const_def_t;
```

Implementation note:

- the public representation above is illustrative
- the actual internal bootstrap should populate real `tyvaluerecord`-compatible values

### Special Case

Type constants currently come from iterating `tyvaluetype` and applying `langgettypestring`.

Recommendation:

- do not preserve this as a generated startup behavior in Phase A
- instead, define the resulting type-constant names explicitly in the bootstrap table once the compatible type set is frozen

Reason:

- it reduces hidden dependencies on type-string helpers
- it makes the bootstrap deterministic and reviewable

## 3. Builtin Function Token Bootstrap

### Required Behavior

`pushfunctioncall` rewrites simple identifier function calls into builtin-token calls by looking them up in `hbuiltinfunctions`.

Current explicit bootstrap source:

- `langinitbuiltintable` in `Common/source/langstartup.c`

Current explicit builtin names:

- `appleevent`
- `complexevent`
- `finderevent`
- `tableevent`
- `objspec`
- `setobj`
- `pack`
- `unpack`
- `defined`
- `typeof`
- `sizeof`
- `nameof`
- `parentof`
- `indexof`
- `gestalt`
- `syscrash`
- `myMoof`

### Recommendation

Define this builtin-function token table directly in `usertalk/src/bootstrap.c` as source data, not via resource loading.

Suggested representation:

```c
typedef struct usertalk_builtin_def_t {
	const char *name;
	unsigned short token;
} usertalk_builtin_def_t;
```

Token source:

- `Common/headers/langtokens.h`

### Decision

- Phase A should preserve the explicit parse-time builtin token substitutions from `langstartup.c`
- Phase A should not depend on `langinitbuiltins()` from `langverbs.c`

Reason:

- the parser only needs token substitution behavior here
- runtime dispatch is a Phase B problem

## 4. Kernel EFP Bootstrap

This is the main design issue.

### Why It Matters

`pushkernelcall` does not merely build a syntax node. It:

1. looks up the dotted path in `efptable`
2. confirms the target table has a non-NULL `valueroutine`
3. confirms the verb exists and has `tokenvaluetype`
4. stores an address value into the AST

So Phase A cannot preserve current `kernel(...)` parse behavior unless it bootstraps enough of the EFP table structure for that validation to succeed.

### Current Frontier Model

The current `efptable` is populated through repeated `loadfunctionprocessor(...)` calls. Those calls:

- create an EFP subtable
- attach a `valueroutine`
- add verb names from classic resources

This is not usable as-is in `usertalk` because:

- it depends on Frontier resource loading
- it depends on runtime callback wiring
- it is much broader than Phase A parser-only needs

### Recommended Phase A Strategy

Split kernel bootstrap into two layers:

#### Layer A: Parse-Core Mandatory

Provide a parser-local `efptable` with parser-visible EFP subtables and verb-name entries sufficient for `pushkernelcall` validation.

Requirements:

- subtable exists for each supported EFP name
- subtable `valueroutine` field is non-NULL
- each supported verb name maps to a `tokenvaluetype`

#### Layer B: Source Of Truth

Prefer source-defined bootstrap tables in `usertalk`, not resource loading.

Suggested representation:

```c
typedef struct usertalk_efp_verb_def_t {
	const char *efp_name;
	const char *verb_name;
	unsigned short token;
} usertalk_efp_verb_def_t;
```

### Recommended Initial Scope

Phase A should support at least the language EFP set currently loaded by `langinitbuiltins()` / `loadfunctionprocessor(idlangverbs, ...)`, because that is the closest parse-time dependency we have already traced.

Open compatibility question:

- whether full syntax compatibility requires all Frontier EFP families to be present in Phase A, or only those reachable from language-core parsing tests

### Design Constraint

Even though runtime execution is out of scope, the parser-local EFP table must still preserve current parse acceptance/rejection behavior for `kernel(EFP.verb)` over the validated corpus.

### Stub Callback Strategy

Because `pushkernelcall` only checks `valueroutine != nil`, Phase A can use a parser-local non-NULL stub callback for EFP subtables.

Recommendation:

- define a no-op internal callback pointer for parser bootstrap
- do not attempt to port execution callbacks in Phase A

This preserves the structural contract `pushkernelcall` expects without dragging in runtime dispatch.

## Bootstrap Data Ownership

Bootstrap tables should be owned by `usertalk_t`, not global process state.

Recommended model:

- each `usertalk_t` owns its parse-time tables
- tables are created once in `usertalk_create`
- tables survive across `usertalk_reset`
- tables are destroyed in `usertalk_destroy`

Reason:

- avoids hidden process-global coupling
- supports test isolation
- keeps Phase B options open

## Recommended Internal APIs

Bootstrap-facing internal APIs:

- `bool usertalk_bootstrap_keywords(usertalk_t *ut);`
- `bool usertalk_bootstrap_constants(usertalk_t *ut);`
- `bool usertalk_bootstrap_builtins(usertalk_t *ut);`
- `bool usertalk_bootstrap_efp(usertalk_t *ut);`
- `bool usertalk_bootstrap_all(usertalk_t *ut);`

Table-facing internal APIs can be thin wrappers over whatever parser-local hash/table layer Phase A adopts.

## What Should Not Be Copied

Phase A bootstrap should not copy these behaviors directly:

- classic resource loading through `getresourcehandle`
- `loadfunctionprocessor(...)` as implemented today
- runtime callback wiring from `langverbs.c`
- full Frontier system-table hierarchy creation

Reason:

- those are startup/runtime concerns, not parser-core concerns
- they create unnecessary coupling to the old environment

## Compatibility Rules

Phase A bootstrap must preserve:

- keyword spelling and token numbers
- builtin function spelling and token numbers used for parse-time substitution
- constant names and resulting parse-time value identities
- parse acceptance/rejection behavior for bootstrap-driven constructs in the validated corpus

Phase A bootstrap does not need to preserve:

- original resource-loading mechanism
- original system-table topology outside the subset required by parser-core logic
- runtime callback implementations

## Open Decisions

These must be resolved before extraction coding starts:

1. EFP scope
   - Which EFP families must exist in Phase A for syntax compatibility beyond the hand-written corpus?
2. Type constants
   - Should Phase A generate `*Type` names from a copied type-string helper, or define the final names explicitly in source?
3. Kernel strategy
   - Is source-defined parser-local EFP bootstrap acceptable as the Phase A replacement for resource-driven `loadfunctionprocessor(...)`?

## Recommended Decision

Recommended path for Phase A:

1. Build keywords, constants, and builtin function token tables directly from source-defined arrays in `usertalk/src/bootstrap.c`
2. Build a parser-local `efptable` directly from source-defined arrays plus a stub non-NULL callback
3. Preserve exact token values from `langtokens.h`
4. Avoid resource loading entirely in Phase A

This is the smallest bootstrap design that preserves parse-time semantics while remaining compatible with Linux and the extraction goal.
