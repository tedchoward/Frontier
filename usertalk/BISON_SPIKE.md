# UserTalk GNU Bison Spike

## Goal

Determine whether the existing Frontier grammar in `Common/source/langparser.y` can be generated with GNU Bison on this Linux system, and identify the first blocking point after generation.

## Environment

- System date: 2026-03-09
- Tool: GNU Bison 3.8.2
- Compiler used for the follow-up compile probe: system `cc` with `-std=c23`

## Commands Run

Version check:

```sh
bison --version
```

Generation probe:

```sh
bison -d -o /tmp/usertalk_langparser.c Common/source/langparser.y
```

Compile probe:

```sh
cc -std=c23 -c /tmp/usertalk_langparser.c -ICommon/headers -ICommon/source -o /tmp/usertalk_langparser.o
```

## Result

### 1. Bison Generation

Result:

- success

Generated files:

- `/tmp/usertalk_langparser.c`
- `/tmp/usertalk_langparser.h`

Observed diagnostics:

- none

Conclusion:

- the grammar itself is accepted by GNU Bison 3.8.2 without immediate syntax or generation errors
- GNU Bison is a viable replacement for MacYACC at the grammar-generation step

### 2. Compile Probe

Result:

- failed

First blocking error:

```text
In file included from Common/headers/frontier.h:43,
                 from Common/source/langparser.y:50:
Common/headers/osincludes.h:51:26: fatal error: Carbon/Carbon.h: No such file or directory
```

Conclusion:

- the first blocker is not the grammar and not Bison
- the blocker is the grammar prologue's dependency on Frontier platform headers through:
  - `frontier.h`
  - `osincludes.h`
  - `Carbon/Carbon.h`

## What This Means

The spike changes the risk profile in a useful way.

Confirmed:

- GNU Bison can generate parser C and a header from the existing grammar source

Not yet solved:

- compiling the generated parser on Linux against the current Frontier include graph

Implication for Phase A:

- `langparser.y` should be copied into `usertalk/src/grammar.y`
- the grammar prologue will need to be rewritten to include `usertalk` local compatibility headers instead of Frontier platform/application headers
- this is now a support-layer extraction problem, not a grammar-generation problem

## Practical Follow-On Work

1. Create local `usertalk` compatibility headers for the grammar prologue
2. Replace these includes in the copied grammar:
   - `frontier.h`
   - `standard.h`
   - `memory.h`
   - `strings.h`
   - `lang.h`
   - `langinternal.h`
   - `langparser.h`
3. Re-run the compile probe against the copied grammar and local headers
4. Continue until the generated parser compiles cleanly on Linux

## Decision

The Bison spike is a pass.

Reason:

- it proved the key unknown: GNU Bison can generate from the legacy grammar
- the remaining work is in dependency isolation, which was already part of the extraction plan
