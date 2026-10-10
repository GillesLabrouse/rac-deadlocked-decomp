# Startup path toward native execution

These observations concern the supplied NTSC-U disc and locally decoded resident
and overlay ELFs. Names are derived from addresses; no retail instruction bytes
or assets are included here. Identifying a boundary or a call does not recover its
callee's behavior. The native scenery viewer does not execute this startup path.

## Resident entry and first loop

The resident ELF enters at `001574E8`. This handwritten platform entry sets up
PS2 execution and reaches `00157C58`. The latter initializes a subsystem, parses
the observed `BOPT=`, `gooey`, and `multi` arguments, calls recovered initializer
`0045CB88`, and invokes `00437EB0` through a function pointer. After that entry
returns, it obtains the next entry from `001579A0` and repeats.

The draft loaders at `00157738` and `001579A0` copy executable sections to guest
addresses. A native port must select compiled host entry points and load their
data; it cannot call those copied MIPS bytes. These drafts remain outside `src/`
and native build targets.

The initial entry has a verified extent `00437EB0..00438684` (exclusive end),
2,004 bytes, followed by one padding word. All 33 direct branches and all 18
targets of the table at `003CFA80` stay within it. Its only indirect jump is the
table dispatch; its terminal return has a stack-restoring delay slot. There are
63 distinct direct callees across initialization, per-frame work, and cleanup.

The earlier `00437EA8` label is an alternate fall-through entry: it stores an
incoming argument before reaching this shared body. It is not a standalone leaf
helper. The observed resident startup call deliberately enters eight bytes later.
The split configuration exposes the body without treating either early returns
or this prelude as unrelated complete functions.

## Observed state dispatch

The loop reads the state at `0021DDB4` and indexes the table with `state + 1`
after an unsigned range check. These are branch groups, not assigned menu names:

| State | Branch target | Observed work |
| --- | --- | --- |
| -1, 0 | `0043830C` | Alternate initial branch or a sequence of update/draw calls; an idle path queues request 23 or 24. |
| 1 | `00438488` | Calls `004E5070` with zero scalar arguments. |
| 3, 4, 16 | `004384AC` | Calls `00490878`, `00489C28`, and `00489740`. |
| 2, 5..15 | `004384C4` | Uses the shared frame tail. |
| Outside -1..16 | `004384C4` | Uses the same shared tail. |

The loop uses `0021EDBC` to decide whether to exit. It also directly reads and
clears the device address `10000800` and accumulates a 64-bit counter. Timing and
device access therefore require a host interface before this body can run on PC.
The semantics of its update/draw callees, input structures, and state transitions
still need recovery; the branch table alone cannot provide a working menu.

## Recovered queue boundary

The idle path passes the queue at `0023A800` to the recovered enqueue and dispatch
routines. The queue has ten eight-byte items, read/write indices at offsets
`5C`/`58`, a count at `60`, mode at `68`, and lock byte at `6C`.

| Routine | Verified behavior |
| --- | --- |
| `0045C768` | Resets indices/count/lock, fills items with a diagnostic byte, sets mode to one. |
| `0045C7B0` | Removes an item in FIFO order and clears the lock when the final item is removed. |
| `0045C850` | Rejects full or locked queues, stores a value and its `value >= 100` classification. |
| `0045C8C8` | Resets through `0045C768`. |
| `0045C990` | Removes one request, derives mode from state, calls `0047F568`, and resets the queue on a negative result. |

All five functions match exactly after linking and execute as recovered C in a
native test. The queue-only fixture maps byte filling to CRT `memset` and captures
the request handler. It does not assign gameplay meanings to requests
or simulate their effects. The handler's fifth parameter is an optional pointer;
this caller supplies null.

The experimental `nonmatching/0047F568.c` now implements observed request
preflight, object gating, previous-state snapshots and parameter assignment.
Its only call is the exactly matched `00465C08`, which reads `0021DAE0` without
arguments. Rejection codes `-1`, `-2`, `-3` and `-4` can override earlier failures
in that order. Only successful preflight changes the pending request and payload;
the optional completion byte is cleared even when the request fails.

A separate native target executes this draft together with the actual queue,
including snapshot capacity and rejection/reset cases. The 508-byte output still
differs from retail's 512 bytes, so it is excluded from accepted matching sources.
Those preflight-only tests do not execute the consumer or enter a level.

## Pending-request consumer draft

`nonmatching/0047EFA8.c` reconstructs the resident consumer. Its two jump tables
at `003D0E80` and `003D0ED0` each contain 17 entries, recovered from the resident
image rather than an overlay. The first table routes cleanup by the previous
state; the second routes setup by the pending request. Calls may change globals,
so the draft reloads the pending request after cleanup.

The consumer records the previous state even when no request is pending. A
positive counter at `0021F2E4` decrements and postpones cleanup/setup. Normal
completion sets the optional byte, installs the pending state, resets the request
to `-2` and clears its completion pointer. Request two can instead remain pending
while a finite float progresses toward `1.1`, leaving the current state untouched.
An additional `0021EDC8` gate can retain the request even at that threshold.

The native consumer fixture exercises these transitions with the actual queue,
preflight draft and matching progression helper. Unrecovered effects are captured
at their call boundaries, so the test is not a runnable game loop. The consumer's
PS2 output is 1,460 bytes against 1,416 in retail and remains outside accepted
coverage. Its decimal immediate adaptation is explicit in the source's assembler
flags; no literal-pool relocation is hidden from the comparator.

For Dreadzone the corresponding consumer is L01 `005A9030`, and its preflight
is `005A9608`. These normalized copies still require independent L01 bindings.
The first-frame dependency audit is in
[`DREADZONE_DEPENDENCIES.md`](DREADZONE_DEPENDENCIES.md).

## Campaign entry draft and level bindings

The extracted Dreadzone Station overlay ELF specifies entry `00528098`. The
existing overlay scanner groups its 4,584-byte extent under canonical common
function `func_L00_00594458`, originating in the multiplayer-menu overlay. This
classification uses normalized instruction shapes; it does not prove that every
callee, global or guest pointer has the same meaning across levels. Each overlay's
addresses must be resolved independently before executing recovered host code.

`nonmatching/L01_00528098.c` now holds a compilable draft of Dreadzone's actual
entry, using L01 addresses. The entry has 150 direct call sites reaching 105
distinct callees. Its 21-entry state table at `003AC550` contains only targets
inside the verified entry extent. Relative branch targets agree with L00, but
32 of 87 observed constant data bindings change between the two overlays.
This binding comparison is positional evidence, not proof of callee equivalence.

For example, the state scalar at `0021DDB4` and level scalar at `0021DE10` retain
their addresses. The loop-exit scalar changes from L00's `0021E69C` to Dreadzone's
`0021E80C`; the object pointer list changes from `00343E78` to `003660F8`, and the
record-array base from `00346BE0` to `00368F60`. Reusing L00 addresses would therefore
access different storage even when the surrounding instruction shapes agree.

The draft corrects byte-versus-element strides for records of size `AD0`, `31F0`
and `470`, and for the four-byte guest pointer list. Compile-time checks validate
those guest layouts, the state fields, and a 32-byte request payload. The payload
initializes a scalar at offset zero and the byte value eight at offset six; its remaining
bytes stay unspecified. An observed call passing six floating-point and six
integer arguments also requires explicit ABI handling in the decompiler input.

SN GCC now produces 4,424 bytes against the entry's 4,584 retail bytes. Expressing
observed scalar fields through their shared base at `00172160` restores the
original 112-byte stack frame; compile-time checks preserve every field offset.
The comparison still fails: register allocation, mixed GP/absolute global accesses, block
scheduling and provisional callee signatures still need work. The draft is outside
`src/`, excluded from progress coverage, and unsuitable for a native build.

Compile it separately, without adding it to accepted build objects:

```sh
bash tools/cc.sh nonmatching/L01_00528098.c build/drafts/dreadzone-entry.o -mno-split-addresses
```

The first-scene port still needs this entry's initialization and frame loop,
dynamic objects and characters, collision, input, timing and rendering interfaces.
The existing extracted Dreadzone scenery provides assets for that work, without
supplying the missing game logic.
