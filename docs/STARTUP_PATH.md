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
native test. The native fixture maps byte filling to CRT `memset` and captures
the unrecovered request handler. It does not assign gameplay meanings to requests
or simulate their effects. The handler's fifth parameter is an optional pointer;
this caller supplies null.

## Campaign entry to investigate next

The extracted Dreadzone Station overlay ELF specifies entry `00528098`. The
existing overlay scanner groups its 4,584-byte extent under canonical common
function `func_L00_00594458`, originating in the multiplayer-menu overlay. This
classification uses normalized instruction shapes; it does not prove that every
callee, global or guest pointer has the same meaning across levels. Each overlay's
addresses must be resolved independently before executing recovered host code.

The first-scene port still needs this entry's initialization and frame loop,
dynamic objects and characters, collision, input, timing and rendering interfaces.
The existing extracted Dreadzone scenery provides assets for that work, without
supplying the missing game logic.
