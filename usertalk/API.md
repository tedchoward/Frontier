# UserTalk Phase A API Design

## Purpose

This document defines the proposed public C API and object model for Phase A of `usertalk`: parser/core extraction only.

Phase A includes:

- parse from string
- parse from file
- error retrieval
- AST ownership and disposal through the `usertalk` object
- AST traversal and introspection
- optional packed-tree export/import for compatibility work

Phase A does not include:

- evaluation
- runtime execution context
- variable injection
- handler invocation
- value marshaling for execution

The API is designed so Phase B can add runtime support without breaking the Phase A parser/core model.

## Design Goals

- Plain C23 API
- Stable opaque top-level object
- No public dependency on Frontier internal headers
- No direct public exposure of `hdltreenode`
- Sufficient AST inspection for corpus shape assertions
- Explicit ownership and lifetime rules
- Future-compatible with a later runtime/evaluator layer

## Opaque Types

Planned opaque types:

- `usertalk_t`
- `usertalk_node_t`

Notes:

- `usertalk_t` is the top-level owner for parser/core state and the current AST
- `usertalk_node_t` is an opaque node reference used only for inspection
- Node references are borrowed views into the AST owned by `usertalk_t`
- Node references become invalid after `usertalk_reset` or `usertalk_destroy`

## Status Model

Phase A should use explicit status returns rather than exposing legacy booleans directly.

Proposed status enum:

```c
typedef enum usertalk_status_t {
	USERTALK_STATUS_OK = 0,
	USERTALK_STATUS_INVALID_ARGUMENT,
	USERTALK_STATUS_IO_ERROR,
	USERTALK_STATUS_PARSE_ERROR,
	USERTALK_STATUS_MEMORY_ERROR,
	USERTALK_STATUS_INTERNAL_ERROR
} usertalk_status_t;
```

Notes:

- parse syntax failures should return `USERTALK_STATUS_PARSE_ERROR`
- the detailed legacy-style message and location are retrieved separately
- status values should describe API outcome, not AST node kind or script semantics

## Source Location Model

Phase A should preserve the current parser location behavior, including the line-based mode already present in the legacy implementation.

Proposed location struct:

```c
typedef struct usertalk_location_t {
	unsigned long line;
	unsigned short column;
	unsigned long offset;
	int is_line_based;
} usertalk_location_t;
```

Notes:

- `line` and `column` mirror the current parse reporting model where possible
- `offset` exists so callers can still use a position in non-line-based mode
- `is_line_based` tells the caller which interpretation applies

## Parser/Core Lifecycle

Proposed API:

```c
usertalk_t *usertalk_create(void);
void usertalk_destroy(usertalk_t *ut);
void usertalk_reset(usertalk_t *ut);
```

Rules:

- `usertalk_create` returns `NULL` on allocation failure
- `usertalk_reset` clears the current AST and current error state
- `usertalk_destroy` releases all parser/core resources, including the current AST

## Parse API

Phase A parses text, not raw bytes.

Proposed API:

```c
usertalk_status_t usertalk_parse_string(
	usertalk_t *ut,
	const char *source_text,
	int is_line_based);

usertalk_status_t usertalk_parse_file(
	usertalk_t *ut,
	const char *path,
	int is_line_based);
```

Rules:

- `source_text` must be NUL-terminated text
- `path` must be a NUL-terminated filesystem path
- the `usertalk_t` object owns the resulting AST on success
- parsing replaces any previously owned AST
- if parsing fails, no old AST remains

Open question to resolve before implementation:

- whether `parse_file` reads the file as text verbatim into memory, or whether newline normalization is required to match historical behavior

## Parse Result Queries

Proposed API:

```c
int usertalk_has_tree(const usertalk_t *ut);
const usertalk_node_t *usertalk_root(const usertalk_t *ut);
```

Rules:

- `usertalk_has_tree` returns non-zero only when the current object owns a valid AST
- `usertalk_root` returns `NULL` when no AST is present
- returned node pointers are borrowed and invalidated by `reset` or `destroy`

## Error API

Phase A should preserve first-error behavior but expose it through a stable API.

Proposed API:

```c
int usertalk_has_error(const usertalk_t *ut);
usertalk_status_t usertalk_get_error_status(const usertalk_t *ut);
const char *usertalk_get_error_message(const usertalk_t *ut);
int usertalk_get_error_location(
	const usertalk_t *ut,
	usertalk_location_t *out_location);
```

Rules:

- the error message pointer is owned by `usertalk_t`
- callers must not free or retain it beyond the lifetime of the owning object state
- `usertalk_reset`, a subsequent parse call, or `usertalk_destroy` invalidates the message pointer
- `usertalk_get_error_location` returns non-zero only when location data is available

## Node Kind Model

Phase A needs a stable public node-kind enum for AST shape tests.

Planned enum:

```c
typedef enum usertalk_node_kind_t {
	USERTALK_NODE_UNKNOWN = 0,
	USERTALK_NODE_NOOP,
	USERTALK_NODE_ADD,
	USERTALK_NODE_SUBTRACT,
	USERTALK_NODE_MULTIPLY,
	USERTALK_NODE_DIVIDE,
	USERTALK_NODE_MOD,
	USERTALK_NODE_IDENTIFIER,
	USERTALK_NODE_CONSTANT,
	USERTALK_NODE_UNARY_MINUS,
	USERTALK_NODE_NOT,
	USERTALK_NODE_ASSIGN,
	USERTALK_NODE_FUNCTION_CALL,
	USERTALK_NODE_EQ,
	USERTALK_NODE_NE,
	USERTALK_NODE_GT,
	USERTALK_NODE_LT,
	USERTALK_NODE_GE,
	USERTALK_NODE_LE,
	USERTALK_NODE_OR,
	USERTALK_NODE_AND,
	USERTALK_NODE_INCREMENT_PRE,
	USERTALK_NODE_INCREMENT_POST,
	USERTALK_NODE_DECREMENT_PRE,
	USERTALK_NODE_DECREMENT_POST,
	USERTALK_NODE_LOOP,
	USERTALK_NODE_FILE_LOOP,
	USERTALK_NODE_FOR_LOOP,
	USERTALK_NODE_BREAK,
	USERTALK_NODE_RETURN,
	USERTALK_NODE_BUNDLE,
	USERTALK_NODE_IF,
	USERTALK_NODE_PROC,
	USERTALK_NODE_LOCAL,
	USERTALK_NODE_MODULE,
	USERTALK_NODE_DOT,
	USERTALK_NODE_ARRAY,
	USERTALK_NODE_ADDRESS_OF,
	USERTALK_NODE_DEREFERENCE,
	USERTALK_NODE_ASSIGN_LOCAL,
	USERTALK_NODE_BRACKET,
	USERTALK_NODE_CASE,
	USERTALK_NODE_CASE_ITEM,
	USERTALK_NODE_CASE_BODY,
	USERTALK_NODE_KERNEL,
	USERTALK_NODE_CONTINUE,
	USERTALK_NODE_WITH,
	USERTALK_NODE_FOR_DOWN_LOOP,
	USERTALK_NODE_TRY,
	USERTALK_NODE_BEGINS_WITH,
	USERTALK_NODE_ENDS_WITH,
	USERTALK_NODE_CONTAINS,
	USERTALK_NODE_RANGE,
	USERTALK_NODE_LIST,
	USERTALK_NODE_FIELD,
	USERTALK_NODE_RECORD,
	USERTALK_NODE_FOR_IN_LOOP,
	USERTALK_NODE_GLOBAL,
	USERTALK_NODE_OSA_SCRIPT,
	USERTALK_NODE_ADD_VALUE,
	USERTALK_NODE_SUBTRACT_VALUE,
	USERTALK_NODE_MULTIPLY_VALUE,
	USERTALK_NODE_DIVIDE_VALUE
} usertalk_node_kind_t;
```

Notes:

- this is a public semantic mirror of the legacy `tytreetype`
- numeric values do not need to match legacy internal enum values unless compatibility testing proves that useful

## Node Introspection API

Proposed API:

```c
usertalk_node_kind_t usertalk_node_kind(const usertalk_node_t *node);
int usertalk_node_location(
	const usertalk_node_t *node,
	usertalk_location_t *out_location);
unsigned int usertalk_node_child_count(const usertalk_node_t *node);
const usertalk_node_t *usertalk_node_child(
	const usertalk_node_t *node,
	unsigned int index);
const usertalk_node_t *usertalk_node_link(const usertalk_node_t *node);
```

Index mapping:

- child `0` -> legacy `param1`
- child `1` -> legacy `param2`
- child `2` -> legacy `param3`
- child `3` -> legacy `param4`

Rules:

- missing children return `NULL`
- `child_count` reflects the number of allocated child slots, not the number of non-NULL children
- `link` exposes the legacy statement-list / parameter-list chaining directly

## Value Introspection API

AST shape tests need limited visibility into identifiers and constants.

Phase A should expose read-only value inspection only.

Proposed value-kind enum:

```c
typedef enum usertalk_value_kind_t {
	USERTALK_VALUE_NONE = 0,
	USERTALK_VALUE_CHAR,
	USERTALK_VALUE_INT,
	USERTALK_VALUE_LONG,
	USERTALK_VALUE_BOOLEAN,
	USERTALK_VALUE_TOKEN,
	USERTALK_VALUE_DATE,
	USERTALK_VALUE_DOUBLE,
	USERTALK_VALUE_STRING,
	USERTALK_VALUE_DIRECTION,
	USERTALK_VALUE_OSTYPE,
	USERTALK_VALUE_POINT,
	USERTALK_VALUE_RECT,
	USERTALK_VALUE_PATTERN,
	USERTALK_VALUE_RGB,
	USERTALK_VALUE_FIXED,
	USERTALK_VALUE_SINGLE,
	USERTALK_VALUE_ENUM,
	USERTALK_VALUE_LIST,
	USERTALK_VALUE_RECORD,
	USERTALK_VALUE_EXTERNAL,
	USERTALK_VALUE_CODE,
	USERTALK_VALUE_FILESPEC,
	USERTALK_VALUE_ALIAS,
	USERTALK_VALUE_OBJSPEC
} usertalk_value_kind_t;
```

Proposed API:

```c
int usertalk_node_has_value(const usertalk_node_t *node);
usertalk_value_kind_t usertalk_node_value_kind(const usertalk_node_t *node);
const char *usertalk_node_string(const usertalk_node_t *node);
int usertalk_node_long(const usertalk_node_t *node, long *out_value);
int usertalk_node_double(const usertalk_node_t *node, double *out_value);
int usertalk_node_boolean(const usertalk_node_t *node, int *out_value);
int usertalk_node_token(const usertalk_node_t *node, unsigned short *out_value);
```

Rules:

- string access returns a borrowed NUL-terminated string owned by `usertalk_t`
- if exact legacy string bytes cannot safely be exposed as a borrowed C string, provide a separate copy-out API before implementation starts
- accessor functions return zero when the requested representation does not apply

Open question to resolve before implementation:

- whether the public API should expose only UTF-8-ish display strings, or exact legacy byte strings copied into stable C buffers

## Visitor API

The visitor API should support corpus assertions without exposing internal tree layout directly.

Proposed callback:

```c
typedef int (*usertalk_visit_fn)(
	const usertalk_node_t *node,
	void *context);
```

Proposed API:

```c
usertalk_status_t usertalk_visit(
	const usertalk_t *ut,
	usertalk_visit_fn visit_fn,
	void *context);
```

Traversal rules:

- visit order should match legacy tree visitation order where practical
- visits should include linked nodes and child nodes
- callback returning zero stops traversal successfully
- callback returning non-zero continues traversal

Open question to resolve before implementation:

- whether early termination should be reported distinctly from success

## Packed Tree API

Packed-tree compatibility is a core acceptance requirement for Phase A. The API can expose pack/unpack publicly if that helps corpus validation and later embedding.

Proposed API:

```c
usertalk_status_t usertalk_pack_tree(
	const usertalk_t *ut,
	unsigned char **out_data,
	size_t *out_size);

usertalk_status_t usertalk_unpack_tree(
	usertalk_t *ut,
	const unsigned char *data,
	size_t size);

void usertalk_free(void *ptr);
```

Rules:

- `usertalk_pack_tree` allocates a buffer owned by the caller
- the caller releases it with `usertalk_free`
- `usertalk_unpack_tree` replaces any existing AST
- unpacked trees are owned by `usertalk_t` like parsed trees

Decision point before implementation:

- if public packed-tree APIs feel too broad for Phase A, they can be kept internal to tests initially
- the compatibility requirement itself does not change

## Ownership Summary

Owned by caller:

- `usertalk_t *`
- buffers returned by `usertalk_pack_tree`

Borrowed from `usertalk_t`:

- `usertalk_node_t *`
- error message strings
- node string values

Invalidated by:

- `usertalk_reset`
- any successful or failed parse that replaces current state
- `usertalk_unpack_tree`
- `usertalk_destroy`

## Phase B Compatibility

Phase B is expected to add runtime extraction and evaluation.

To avoid breaking Phase A:

- `usertalk_t` should remain the stable core owner type
- evaluation should likely be added either as:
  - additional APIs on `usertalk_t`, or
  - a separate `usertalk_runtime_t` that references a `usertalk_t`
- AST node references should remain read-only to callers
- parser/core error APIs should remain distinct from future execution error APIs

Recommended direction:

- keep Phase A focused on parser/core ownership
- add a separate runtime object in Phase B if execution state becomes materially larger than parser/core state

## Decisions Still Required Before Coding

1. Confirm whether `parse_string` takes only NUL-terminated text, or whether a `(const char *, size_t)` overload is still needed for embedded NUL safety.
2. Confirm whether string-valued node accessors expose borrowed C strings or copy-out buffers.
3. Confirm whether packed-tree APIs are public in Phase A or test-internal only.
4. Confirm visitor early-termination semantics.
5. Confirm exact status enum naming and whether `IO` and `PARSE` errors should be distinct at the top level.
