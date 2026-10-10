# Dreadzone state-zero record orchestration

`nonmatching/L01_00630438.c` reconstructs the actual Dreadzone overlay routine
at `00630438..00630930` (exclusive end): 1272 bytes, 318 instructions, 34 local
branch targets, 43 direct call sites and 38 distinct callees. Its input ABI is
`void func_L01_00630438(void)`. It is reached from the state-zero branch of the
level entry, but also from other entry branches; its reachability alone does
not establish that an actor was constructed or a game frame completed.

The routine coordinates existing records, a conditional state snapshot,
subsystem updates and dispatch. It does not allocate or initialize the record
array or the pointed-to actors. Those prerequisites must be supplied by the
real pre-loop and level-load path before executing it on the host.

## Independently verified L01 data

The following bindings were read from the L01 retail overlay, not copied from
the resident or multiplayer versions. Address meanings beyond the observed
operations remain provisional.

| Binding | Observed use |
| --- | --- |
| `0021DDA4` | Pointer to a structure whose word at offset zero supplies the record count; reloaded after calls. |
| `00368F60 + index * 31F0` | Record returned by `006658C0(index)`; the accessor performs no bounds check. |
| Record `2EDC` / `2EDD` | Activation byte and countdown byte; countdown is set to 15 and passed by address to `0055D1E0`. |
| Record `2F00 -> 1A4` | Flags word tested against `800`; this first loop assumes a valid pointer when the earlier conditions hold. |
| Record `2F28 -> 4` | Optional child payload pointer; copied and later rebound during the snapshot path. |
| `002121A0 + index * C0` | Destination for each existing child payload snapshot. |
| `002124F0` | Destination for a separate `1C4`-byte copy from the pointer at `0021DFE8`; that pointer is then rebound to the destination. |
| `00172160` fields `10`, `250`, `254`, `258`, `2C4`, `2D0`, `1681` | Snapshot selection, identifier and completion fields. |
| `0021EE00` | Pointer receiving a 16-byte zero store after subsystem dispatch; it must be valid and suitably aligned. |
| `00171B40` | Signed value capped at `05F5E0FF`; subsequently stored before a no-argument predicate call. |
| `0021E8FC` | Counter decremented and clamped to zero; the draft preserves the retail 32-bit subtraction. |

The draft checks record size `31F0`, pointer-field offsets, flags offset `1A4`
and the 16-byte quad type at compile time. Its pointer layouts are PS2 layouts;
it is not directly compiled into the 64-bit Windows runtime. The safe count
domain and initialization of all nested pointers still require evidence from
their producers. The spacing between staging addresses does not prove a
permitted count or establish that these records represent every game actor.

## Observed execution gates

`00630430()` currently returns zero. When either `0021E824` or `0021E828` is
nonzero, the routine visits the counted records. With `00222B84 >= 8`, record
flags `800`, and state `0021DDB4 == 0`, it calls `00665CB0(index, 1)`, sets the
countdown to 15, and decrements it while activation remains nonzero.

After `005A0AD8`, flag `00222BEC` can trigger the real request preflight
`005A9608(6, 1, 0, 0, NULL)`. Acceptance records a boolean read through
`00527B00`, clears `00222BEC`, and bypasses the remainder of this update.

Otherwise the larger path begins with `00630360`. A level value of zero or
at least 40 enables the core synchronization and conditional snapshot path;
other values reach `00653028`. Snapshot copying additionally requires the
integer at `00172430`, converted to float, to exceed 1500, or byte `001737E1`
to be nonzero, and field `001723B8` to be zero. A nonzero selection at
`00172424` performs the copy/rebind sequence around core calls and
`01E9E798`. This is an observed state-preservation operation, not proof of
actor creation.

The subsequent update sequence reaches `00637588`, `005BA3B8`, `00586950`,
`005B5940`, `006272C8`, `00573C58`, `006666A8`, conditional `006A8340`, and
`005BEC10`. Selection between `00624C88` and `00625DA8` depends on the updated
fields at `0021E818`, `0021E824` and `0021E8A0`. Later calls include
`00574B28`, `00587E20`, record-dependent `005D7058(index)` plus the explicit
index 4, `0056F440` and `00630388`.

## ABI and decompiler corrections

`006658C0(s32)` returns a record pointer. `00665CB0(s32, s32)` writes the first
argument's record activation byte and conditionally calls `00570470`.
`0055D1E0(u8 *)` updates the pointed-to countdown byte.
`0055D3B8(void *, const void *, u32)` performs a memory copy; its result is
discarded here. `005A9608` has five arguments, with an optional completion
byte pointer as its fifth argument. The indexed getters `005D8228(s32)` and
`005D8248(s32)` read separate words of `54`-byte records at `0031B2F8`.

`0053A470` has no input argument: it iterates counted records and calls
`0053A4D0(index)`. The leftover clamped value in the caller's argument register
is not a parameter. The copy helpers use byte strides `C0`, not element
strides inferred from an unknown type. The 128-bit zero store is expressed
using GCC `mode(TI)`, which produces the observed `por`/`sq` operation without
inline assembly or a substitute host effect.

The initial decompiler output moved count-pointer reads before calls that
the retail code follows with a fresh read. It also cached `0021E824` before
`00538610` even on the path that reloads the value after that call. The draft
reads these values after the relevant calls. Ignored callee return types are
still provisional; they are not used to infer subsystem behavior.

## Remaining dependencies and useful next work

| Dependency | Why it prevents a real first frame | Next evidence to recover |
| --- | --- | --- |
| Record and nested-pointer producers | This routine assumes the array, count structure, definitions and child payloads already exist. | Trace writes to `00368F60`, `0021DDA4`, record `2F00`/`2F28`, and the pre-loop `00665D58` chain. |
| `00665CB0 -> 00570470` | Activating a record has a downstream effect beyond changing one byte. | Recover `00570470` and its data dependencies before treating activation as complete. |
| `00653028` | The common level-range path advances an asynchronous resource request and decompresses two sections. | Recover the request callbacks, I/O completion and decompressor behavior; see the loader reconstruction below. |
| `005B5940 -> 00633178` | Reaches the verified MMIO transfer boundary during the update sequence. | Connect command construction and synchronization to a real host renderer. |
| `005BEC10` | Iterates `40`-byte records and dispatches through callback table `00298700`, with handlers receiving a record pointer. | Recover table entries, record producers and handler ABIs. Direct-call recovery alone cannot execute this path. |
| `00624C88` / `00625DA8` | Large selected update routines read live object/subsystem state. | Trace the applicable initial flags and recover the branch actually taken at first frame. |
| `00574B28` | Contains vector-unit operations and consumes global vector state. | Recover layouts and numerical behavior or a documented equivalent host implementation. |
| Core snapshot calls and `01E9E798` | Copy/rebind depends on core and network operations, not just memory copying. | Implement or prove the applicable initial path's effects using the supplied retail data. |

The next high-value actor work is the virtual constructor `00668070`, its
actor pool and payload allocator, and the child-link producer `0015FA68`.
Small accessors describe existing storage; recovering more of them does not
by itself provide actor initialization or gameplay.

## Resource loader: `00653028`

`nonmatching/L01_00653028.c` reconstructs the 696-byte routine
`00653028..006532E0`, with nine direct call sites and six direct callees.
Its ABI is `void func_L01_00653028(void)`. All seven table words at `003AFD70`
were read from L01 and point inside this function:

| State | Retail target | Effect |
| --- | --- | --- |
| 1 | `0065309C` | Start the section request, or report immediate completion for an absent entry. |
| 2 | `0065314C` | Start a second asynchronous operation; advance to state 4. |
| 3 | `006532BC` | Return without an operation. |
| 4 | `00653180` | Decompress the section referenced by the first header word; advance to 5. |
| 5 | `00653200` | Decompress the section referenced by header offset 8; advance to 6. |
| 6 | `0065327C` | Wait for the completion byte, then advance to 7. |
| 7 | `00653294` | Record success when status is zero, clear state and the byte at offset `12`. |

Before dispatch it calls `00157F98`, `001634A8(0)` and the busy predicate
`00163928()`. The loader block is at `001711A8`: index at `4`, status at `8`,
result byte at `10`, byte at `12`, section-present bytes at `19`/`1A`, waited
byte at `1B`, destination pointer at `20`, and state at `24`. Its entries at
`001CF050` have two words and an eight-byte stride. The buffer formula is
`destination + [001CEC04] - (entry.word4 << 11)`; the request's offset uses
`entry.word0 + [001CEBF4]`. Sector and file-offset interpretations remain
provisional; the observed shifts and arithmetic are retained.

`00163840` consumes seven argument slots, including a two-argument completion
callback and its context pointer. It records the request in the core block
at `0021D940` and calls `001634F0`. Callback `00653940(block, status)` changes
state to 2 for status zero, or to 7 and records status 1/2; status 2 also
re-enters the loader. Callback `006539B8(block, status)` sets byte `1B` for
status 0/1/2, records nonzero status, and re-enters for status 2. Other status
values have no observed effect in these callbacks.

`00583980(const u8 *source, u8 *destination)` is a WAD decompressor, not an
actor constructor. The retail implementation checks the `WAD` signature,
reads an unaligned compressed length, and uses DMA register `1000D400` and
scratchpad `70000000`. The decompiler's additional argument-register values
are not consumed inputs. The two calls decode to `[0024375C]` and the loader
block's destination. This routine writes no `31F0` record or `2F00`/`2F28`
pointer. It cannot be used as evidence that the actors have been initialized.
The normalized resident counterpart is `004FE9A0`; all bindings above were
verified independently against L01.

## Actual record initializer: `006A8420`

`nonmatching/L01_006A8420.c` reconstructs `006A8420..006A8758`, 824 bytes,
seven direct call sites, six direct callees and one virtual call. Its input
is a full-width `s32 index`; only the byte stores truncate it. The initial
untyped decompiler incorrectly inferred a byte input.

The function selects `00368F60 + index * 31F0`, resets it through `006A93A0`,
stores the byte index at `2FEA`, assigns vtable `003B10C0` at offset `14`, and
calls that table's offset `2C` with five arguments:
`(record, NULL, &record.positionA0, &record.rotationB0, index)`. The actual
word at `003B10EC` is **`00668070`**. It is not the separate `006AC140`
record path.

After the virtual constructor, the initializer fills these fields:

| Record field | Value or observed effect |
| --- | --- |
| `2EFC` | `00230280 + index * 470`. |
| `2F00` | `001EE600 + index * 1700`, with a conditional substitution by `001EEBC0`. |
| `2EEC` | Pointer `003BD930`. |
| `2F00 -> 56C` / `570` | Callback `006B5CB8` and pointer to the record byte at `2FEC`; callback inputs are not established here. |
| `2F04`, `2F08`, `2F0C` | Bit-preserving copies of the float components at `A0`, `A4`, `A8`. Retail uses `lq`, `pextuw`, `prot3w` and `mtc1`, not integer-to-float conversions. |
| `2F28` | Output of the core child-link operation `0015FA68`. |
| `2F28 -> 4 -> 38` | Input index; both nested pointers are assumed valid by retail. |
| `0` | Initially index, then the identifier returned by `0015C6E8(0)`. |
| `2F24`, `2E20` | Signed halfword `[00171B50]` converted to float, or 50.0 when zero. |
| `2EF0 -> AC -> E0` | Back-reference to this record; actor and payload must already exist. |
| `003660F8[index]` | Published record pointer after setup calls. |

Three iterations copy words from the tables `00171D50`, `00171D80`,
`00171DB0` (row stride `10`) into `00171E00`, `00171E78`, `00171EF0`
(row stride `28`), using the stored byte index as the four-byte column.
A changed selection between byte table `00171DE0` and word table `001711D0`
calls `0068E3B0(record, selection)` and clears `1AE0`. Initialization then
calls `00669078(record)` and conditionally `00688578(record, selection)`.
Their unused caller argument-register contents are not parameters. The
special branch requires `[0021E824] == 0`, `[0021E828] == 1` and
`index == [0021E828]`; it also adjusts actor fields `74` and `36`.

`0015FA68` consumes integer/pointer slots 0 through 5 and 7, plus float
register `f12`. Slot 6 is unused by the callee; the draft retains that slot
to place `&record.child2F28` in slot 7. The arguments are the record index,
position and rotation pointers, byte `2EE4`, words `2F14` and `2318`, the
unused slot, the child output pointer, and float `2E20`. Inspection of the
callee proves that its child record is selected from
`01F31380 + index * AD0`; its payload can come from network/core operations
`01EC93E8`, `01EC94C0`, `01EC96C0`, `01EC9530`, or the `002121A0` snapshot
path. An existing payload may be reused. This is a real prerequisite, not
just a pointer allocation that can be replaced with an arbitrary buffer.

## Actor production prerequisites

The actual virtual constructor `00668070(record, actor, position, rotation,
index)` uses all five arguments. A supplied actor is bound at `2EF0` and
receives the vectors. For the null actor input used by `006A8420`, it inspects
the existing actor range `[00222D14]..[00222D2C]` and actor class fields. It
can reuse/reclassify an existing actor through `00592588`, or call
`005869E0(class, F0)` and bind the result at `2EF0`. This constructor spans `00668070..00669078` (4104 bytes, 1026 instructions),
with 26 direct call sites, one virtual call and a `1B0`-byte stack frame.
Its branches stay inside that extent and its single return is at `00669070`.
It has vector-unit calculations, virtual dispatch through vtable offset `28`,
and further setup calls. Its full behavior still requires recovery; the
allocation observations alone do not replace that function.

`005869E0(class, payloadSize)` is a concrete 304-byte pool allocator. It
searches `100`-byte actors between `[00222D2C]` and `[00222D30]`, checks
availability and an age/limit condition, obtains the optional payload via
`0058D538`, initializes the actor through `00586B10`, sets actor `AC` to that
payload, clears the payload through `0055D2A8`, and updates the available
count at `00222D10`. It can return null. Recovering this chain and the pool's
producer is the next bounded route to a real actor, alongside the child-link
producer. The initializer's immediate dereferences make successful actor
and child setup explicit preconditions; no null guards or fake success have
been added to the draft.

A distinct 3128-byte initializer at `006A8758` also writes `2F00` and calls
`005869E0` before the same virtual constructor. Writes to `2F28` in the
separate `006AC140`/`006ACA48` paths include cleanup; a store to that offset
alone is not evidence of actor production.

## Validation of the additional drafts

Both additional sources compile with the existing SN GCC 2.95.3 pipeline;
all PS2 layout assertions pass. The loader compiles to 640 bytes with a
`0x30` (48-byte) frame, matching the retail frame but not its 696
bytes. Its switch emits a `.rodata` jump table, which the shared strict linker
currently rejects. A private experiment links the original object and places
that table at the independently verified `003AFD70`; all external symbols
resolve and unmasked comparison fails. No shared linker changes were made.

The record initializer compiles to 800 bytes with a `30`-byte frame in
hexadecimal, versus 824 bytes and a `0x20` (32-byte) frame in retail. The strict
private link resolves its L01/core symbols and unmasked comparison fails.
Compiler output confirms the eight integer/pointer slots and independent
`f12` argument. Differences include scalar float copies instead of the
retail packed vector extraction, optimized table copying, and mixed
absolute/GP accesses to `0021E828`. These drafts remain excluded from matching
coverage and the host runtime. Neither was executed as a native game frame.

## Executable payload allocation and release

The allocation path now has three recovered sources:

| Source | Independently verified retail span | SN compiled size | Role |
| --- | --- | --- | --- |
| `nonmatching/L01_005869E0.c` | `005869E0..00586B10`, 304 bytes | 296 bytes | Select an actor slot, allocate its payload, call the real class initializer and clear the requested payload bytes. |
| `nonmatching/L01_0058D538.c` | `0058D538..0058D6A8`, 368 bytes | 568 bytes | First-fit allocation of 64-byte payload blocks using packed descriptors. |
| `nonmatching/L01_0058D6A8.c` | `0058D6A8..0058D780`, 216 bytes | 332 bytes | Unlink a payload allocation and reset the descriptor state when the last allocation is released. |

The handwritten payload routines have multiple returns. The splitter's initial
allocation span stops at `0058D678`, omitting its failure and empty-list exits;
the release span stops at `0058D750`, omitting its head and empty-list exits.
All conditional branches and internal jumps were checked against the complete
ranges above. Allocation returns at `0058D670`, `0058D678`, `0058D6A0`;
release returns at `0058D748`, `0058D750`, `0058D768`, `0058D778`. Their
shared-table operations have no direct callees. Explicit span overrides in
private draft checks do not alter accepted function metadata.

### Serialized payload state

The following are live values read through the L01 GP, not invented host
pointers or inferred fixed capacities:

| Binding | Observed meaning |
| --- | --- |
| `00222D3C` | Guest base of the payload byte arena. |
| `00222D40` | Payload capacity in 64-byte blocks. |
| `00222D44` | Guest base of the four-byte descriptor array. |
| `00222D48` | Guest pointer to the first active descriptor, zero when empty. |
| `00222D4C` | Guest pointer to the final active descriptor, zero when empty. |
| `00222D50` | Guest pointer to the next unused descriptor slot. |
| `00222D54` | Exclusive guest end of the descriptor array. |

Each descriptor packs the block count in bits `0..7`, the starting block
in bits `8..19`, and a signed next-descriptor index in bits `20..31`.
`FFF` terminates the chain. Requests round up by `(bytes + 63) >> 6`.
Allocation searches the active chain in payload-offset order for the first
adequate gap, inserts a descriptor there or appends within capacity, and
advances the metadata cursor past occupied descriptor words. Payload bytes
are untouched by the allocator. Freeing clears the selected descriptor,
lowers the metadata cursor, updates links, and resets head/tail to zero and
cursor to the descriptor base on the last release. It leaves payload bytes
untouched.

`native/dreadzone_actor_pool.h` exposes the host arena and state descriptors.
Serialized links remain `u32`; host pointers exist only in the arena interface.
`DEADLOCKED_NATIVE` builds the actual shared allocation/release algorithms in
`0058D538.c` and `0058D6A8.c`, with bounded guest translation and validation.
The supported host domain is a valid sorted chain, up to 2048 descriptors,
4096 payload blocks, and nonzero requests of at most 255 blocks. Bounds,
cycles, overlapping active allocations, mismatched tail, unusable cursor,
unknown free addresses and pointer/extent overflow fail before mutation.
This validation is a host adaptation, not additional retail behavior.

Zero-sized payloads bypass `0058D538` in the actor allocator. The host payload
API rejects a direct zero request because the packed representation can lose
that allocation's identity. Invalid release addresses return failure on the
host and in the current C draft; retail executes a trap for an unknown block.
Valid release behavior was compared against retail. The original direct
allocator's empty-list path assumes sufficient payload capacity; the host
checks it. The retail metadata-cursor loop probes one word at the exclusive
end before testing it; the host avoids this unnecessary out-of-range read.

`dreadzone_payload_allocate_zeroed` applies the actor allocator's observed
payload-clear step after a successful allocation. The real `0055D2A8` callee
writes complete four-byte words, rounding positive byte counts upward to four:
1/3/4 bytes clear 4, 5 clears 8, and 65 clears 68. The remaining padding to the
64-byte allocation boundary stays untouched. This helper completes
the payload memory operation without claiming completion of actor setup.

### Actor allocation still requires class initialization

The actor allocator searches the guest range `[00222D2C]..[00222D30]` at
stride `100`. A candidate requires unsigned byte `20 >= FE` and unsigned
word `A0 <= [0021DDB8]`. A nonzero requested payload must allocate successfully;
failure exits the search and may call diagnostics `00588CC8`/`00588D08` when
byte `[0021EC28]` is nonzero. Success calls `00586B10(actor, class, 1)`, writes
the payload guest pointer at `AC`, clears the word-rounded payload footprint, and
decrements `[00222D10]` only when that count is nonzero.

The retail `FF` candidate path writes byte `120` before the initializer.
That address belongs to the following `100`-byte slot. This cross-slot effect
is retained in the PS2 draft; it must not be silently replaced by an actor
field within a larger assumed stride. Accessible backing for that write and
the actual range/count producers remain prerequisites for host actor setup.

`00586B10` is an 880-byte initializer. It clears the actor while preserving
selected state, assigns class-derived fields using tables at `0026B590`,
`0026B2B0`, `0026DD90`, `0026AE00`, `0026AA40`, and dispatches further setup
such as `00589210`, `0058B148` and `0058A648`. The payload allocator alone
cannot supply those asset/model effects. `005869E0.c` therefore remains a PS2
reconstruction with external real callees; it is not built into the native
payload test. The actual actor pool capacities and their initialization must
be recovered from their producers, rather than chosen from a fixture count.

### Validation evidence

All three sources compile under SN GCC. Their private links resolve every
symbol at independently observed L01 addresses and compare unmasked bytes;
all remain nonmatching. The payload helper is shared as inline C, while
retail used handwritten assembly, so the code sizes differ substantially.
No matching coverage or accepted objects are added.

`native/tests/dreadzone_actor_pool_tests.c` executes the recovered allocation
and release algorithms under 64-bit Windows. Independent packed-word and byte
oracles cover 64-byte allocation rounding, word clearing with untouched padding,
word-rounded footprints for requests 1/3/4/5/65 and unaligned host backing,
metadata exhaustion, payload exhaustion, head/middle/tail unlinking, first-fit
gap reuse, cursor skipping, last-release reset and reuse, invalid inputs,
corrupted chains, guest translation limits and untouched guard regions.

A separate private interpreter executes the actual local L01 instruction words,
including branch delay slots. Twelve deterministic traces compare **7200**
operations: 4003 allocation attempts, 3197 releases, 653 allocation failures,
and 124 last-release resets, executing 949994 retail instructions. Return
values, every descriptor word and head/tail/cursor agree with native C after
every operation. Payload bytes remain untouched in both allocators. The
interpreter's supported scope is these two bounded integer memory routines;
it does not emulate the game, its renderer or its actor constructor.

The sources and native test are reviewable progress toward the actor path.
They do not provide a playable native first frame. The next essential work
is the remaining model-dependent `00586B10` path and actor/class-table producers,
plus the larger constructor
`00668070` and child-link producer `0015FA68`.

## Validation status

The source compiles with SN GCC 2.95.3 and the repository pipeline, including
explicit `--float-immediates` adaptation for 1500.0. It produces 1240 bytes
and a `0x20` (32-byte) stack frame, versus 1272 bytes and `0x30` in retail.
Layout assertions pass. A private link places the
draft at `00630438` and resolves its actual L01 and core/network symbols;
unmasked byte comparison fails. It remains in `nonmatching/` and contributes
no matching coverage.

No game frames or native actor effects were executed for this draft. No SDK
source, assets or disassembly are included in these tracked files.

## Executed class-initialization slice

`L01_00586B10.c` now reconstructs the complete PS2 class initializer while its
bounded native interface executes actual branches needing neither cleanup nor
model setup: a mismatched class slot, or a mapped definition whose byte `8` is
zero. The former assigns the handler guest pointer and observed flags; the latter
assigns the definition/model/auxiliary guest fields and all recovered defaults.
The native path clears 256 bytes by 64 four-byte stores through `memcpy`, retaining
the real word-filler footprint with safely unaligned host backing.

The zero-count branch at `00586D38` resets the earlier float word at actor `48`
to zero and sets flag `40`. A private instruction differential caught this effect
before finalization. The actual environment initializer `00589210` and packed
64-bit operation `0058D468` also execute in C; actor pointers remain guest32.
Read-only class views map only independently supplied local tables. Every required
mapping is checked before mutation, and outstanding cleanup/model paths return
explicit dependency results without pretending to construct an actor.

The native byte-image/guard fixtures pass. A separate bounded interpreter executes
the actual initializer and its small callees for 512 cases: 254 mapped, 258
mismatched, 245 with environment data and 138 with auxiliary data. All 256 actor
bytes and unchanged class-table bytes agree after 198,060 retail instructions.
This validates the exercised slice, not the unexecuted model or cleanup paths.

All three sources compile and pass relocation resolution, but remain nonmatching:
initializer 872/880 bytes, environment 84 plus padding versus 88, and packer 32/32
with instruction differences. These are draft diagnostics outside coverage.
Real nonzero model allocation/compaction, class-table producers and vector/DMA
model processing still need recovery before this slice can initialize a player.
