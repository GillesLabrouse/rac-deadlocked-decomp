# Dreadzone startup dependencies

This is an address-based audit of the locally extracted NTSC-U Dreadzone Station
overlay. It describes actual calls on the path toward a first game frame. It does
not assign menu, mission or character names to states whose meanings remain
unknown. The scenery viewer does not execute these functions.

## Executable entry and overlay ownership

The Dreadzone overlay's ELF entry is `00528098`. Its text range begins at
`003C7C80`; calls inside that text belong to L01 even when their addresses are
below the entry. The verified entry extent is 4,584 bytes, including a final
padding word. It calls 105 distinct functions at 150 direct call sites:

- 98 targets are physically inside L01's text.
- Six are resident core targets: `00138D70`, `00157F98`, `00159120`, `0015ADA8`,
  `0015EF00`, and `00161CF0`.
- One is resident network code at `01E9E270`.

The existing normalization algorithm finds resident instruction shapes for 51 of
the 98 overlay callees, representing 43 distinct resident shapes. That grouping
is weaker than an exact match: it masks call targets and many non-stack
immediates. Several empty functions and short wrappers consequently share a
shape while performing different calls. Physically resident code and
resident-shaped overlay copies are different ownership categories.

For example, `0052A628` and `0052A648` both group with resident `0054B5A8`, but
call different L01 targets, `005431C8` and `00544418`. A host entry table must
resolve actual L01 calls and data bindings before using recovered C.

The call count and addresses were checked against the locally generated
`campaign-entry-l01-bindings.json` and independently against L01's text and
function boundaries. Private diagnostic artifacts are not distributed.

## Initialization before the loop

With the level scalar at `0021DE10` equal to one, the entry takes the campaign
branch: it clears `0021E824`, selects `0021DDA4`, `0021E818` and `0021E828`
through the result of `00666600`, assigns the state pointer at `0021DFE8`, and
later initializes the state scalar `0021DDB4` to 15. This is a branch condition,
not a claim that state 15 has a known gameplay name.

Important ordered calls before the first loop iteration include:

| L01 target | Explicit arguments | Evidence and remaining dependency |
| --- | --- | --- |
| `006C1D68` | None | Installs L01 function addresses in the callback block at `00212148`. A native port needs host function pointers, not copied guest integers. |
| `00529280` | None | Clears the range `00222900..003A0B40`; the aligned path calls `0055D2D0`, otherwise it clears bytes. Recovered C now executes with an explicitly bound host arena; all 1,565,248 bytes and guards are tested. It remains nonmatching. |
| `00574730` | None | Calls core `0011EF60`; subsystem behavior has not been recovered here. |
| `0055DFE8` | None | Actual state helper/selector execute with bounded guest storage; both 207-record fixed tables and conditional slot2 are tested. |
| `00665D58` | None | Campaign initialization audited below; eight callees remain necessary. |
| `00628020` | None | Copies the pointer at `001DFA70` into its field at offset `30`, clears its count at `34`, and clears two fields in each of 64 sixteen-byte records at `0034D320`. |
| `005742C8` | None | Sets up several memory regions through core and L01 calls, including `00633248`, `00633360` and `0055D2A8`. |
| `005D6CB0` | None | Fills `0x40000` bytes through the pointer at `002437C4`, using core `0011A518`. The literal argument is `DEADBEEF`; its low byte reaches byte filling. |
| `005D6CE8` | None | Calls `0065B890`, `00656620(0)` and `00655D38` before further setup; its full behavior remains unresolved. |
| `00578940` | `1, 0` | Large setup routine (5,800 bytes) with allocation, copying, synchronization and many subsystem calls. This is an essential recovery target before claiming initialized game data. |
| `00584038`, `00533810` | None | Additional setup precedes the later transition and first frame. |
| `0052A448` | None | Calls `005B57B8`; it remains an unresolved prerequisite. |
| `005B5C38` | Campaign path: `4, 13, 0x400` only if `0021E824 == 1` | The level-one campaign branch has `0021E824 == 0`, so this conditional call is skipped there. |
| `005B6128` | `0` | Called immediately before the transition consumer. |
| `00159120` | None | The supplied retail function is an empty return. It is not an invented host stub. |
| `005A9030` | None | L01 pending-request consumer, also reached from the frame tail. It has the normalized shape of resident `0047EFA8`. L01 bindings must still be checked independently. |

Some branches call core `00161CF0` and `0015ADA8` repeatedly between setup
operations. Their actual synchronization and subsystem behavior cannot be
replaced by empty callbacks and still count as an initialized game.

## Recovered initialization draft

`nonmatching/L01_00665D58.c` is an independently written typed C draft of the
campaign initializer reached before the loop. Its canonical normalized copy is
L00 `006A95F8`; the draft deliberately uses actual L01 addresses.

It sets fields `1C`, `258`, and `25C` of the block at `00172160` to one, clears
field `228` and byte `6C9`, calls core `0015A170(0)` and `0015AA58`, clears byte
`00171DF4` when field `20` is at least two, and then invokes:

1. `006C1C00()`
2. `006669D8()`
3. `00666A28(0, 0)`
4. `006AF588()`
5. `006A9640()`
6. `006B41D0()`

The structure has explicit padding and compile-time checks for every accessed
field. No guest pointer is embedded in it. The draft does not implement its
eight dependencies or pretend their effects occur on PC.

Inspection of these immediate dependencies establishes more specific work:

- `006C1C00` installs L01 callback addresses in `00212148`; these differ from
  those installed by the entry's earlier `006C1D68` call.
- `006669D8` writes `-1` to `00223E00` and `00223E04`, then calls six routines:
  `00667668()`, `006670B8(2)`, `006671D0(3)`, `006672E8(4)`, `00667400(5)`, and
  `00667518(6)`. The arguments two through six are explicit in the call sites.
- `00666A28` stores its first argument at `00223E04`, its second at `00223E00`,
  then calls `00666F08`. Both arguments are explicitly zero at this call site.
- `006AF588` calls `0055D2A8(0037B320, 0, 0x4100)`, which directly fills words,
  then initializes twenty records of stride `340`. Nonzero overrides are word
  `0 = FFFFFFFF`, shorts `2DC` and `2E4 = -1`, and floats `2E8` and `2EC = 40`.
  It also clears `00220844` and seven words at `0037F428`. The complete reset is
  now recovered in `nonmatching/L01_006AF588.c` and executes in an isolated native
  test with a byte oracle for all records. Default compilation has the correct
  232-byte extent but still differs in instruction scheduling and registers.
- `006A9640` calls `006A9AE0` on 256 records of stride `5C` at `00375720`,
  links records 0 through 254 through their 32-bit field at offset `58`, and
  stores the first record as the guest-address list head at `00220828`.
  Record 255's field `58` remains untouched: a null-terminated list therefore
  depends on that field already being zero. The entry's preceding bulk clear
  covers this pool. `006A9AE0` uses `0055D2A8` to fill each record's 64-byte
  range at offset `C` with `CDCDCDCD`, rather than filling bytes through memset.
- `006B41D0` calls `00666B40(6, value_at_00223E04, -2)`, stores the returned
  pointer at `00220890`, and clears the returned object's byte at offset `2C`.
  Its initializer dependencies must therefore produce a valid object pointer.
- Core `0015A170` accesses the same `00172160` block and level scalar. Core
  `0015AA58` tests field `258`, which this initializer has just set to one.

With the default SN GCC flags, the draft emits 148 bytes and needs four bytes of
zero padding to cover the 152-byte retail extent. Actual-relocation comparison
fails on three instructions: the load, signed comparison and branch for field
`20` use register `v0` instead of retail's `v1`. The full original linker resolves
all calls and data addresses; the only differing bytes are at offsets `3E`,
`42`, and `46`. This is still a nonmatch, excluded from accepted sources,
coverage and the native runtime.

Compile this draft independently:

```sh
bash tools/cc.sh nonmatching/L01_00665D58.c build/drafts/dreadzone-startup.o
```

Using `-mno-split-addresses` changes this routine's base-address selection and
instruction scheduling considerably; the default flags are closer to retail.
The three remaining register differences were not hidden by broad masking,
hard-coded register constraints or inline assembly.

Further inspection of the core calls narrows the campaign branch. Core
`0015A170` clears byte `1681` even when block word `0` already exceeds its state
gate. If that word is below seven, the level-one path skips the level-zero reset
calls; it initializes ten repeated fields and, with field `258 == 1`, assigns
word `0 = 7`, clears word `8` and returns zero. Core `0015AA58` then initializes
its two ten-word ranges and flags when word `0 < 10`. With `258 == 1`, it assigns
word `0 = 10`, clears word `10` and calls `0015B498`. This is the core block's
own state, separate from the overlay frame state at `0021DDB4`.

`0015B498` copies word `10` to `002120F4`, assigns fields `254` and byte `250`,
calls `0015F4A0`, copies field `218` to `220`, and invokes the optional callback
at `0021DB94`. `0015F4A0` immediately returns one when pointer `0021DFE8` is
already nonzero; its large creation/network path is skipped in that case.
The entry's selected state pointer and callback binding must be verified when
porting these calls. These are inspected branch facts, not implemented native
core initialization or permission to substitute their unresolved effects.

## First frame and platform boundaries

Each entry-loop iteration saves the current state, reads and clears timer MMIO
`10000800`, updates the 64-bit total at `0021DA00`, and calls `0054D080` with that
total. It then calls `00633178(001170D0)`, `00633430`, `006333B0`, `00564DE0`,
`00565170(1)`, `00564B30`, and core `00157F98` before the state dispatch.

`00633178` is **not an input polling function**. Direct inspection shows it
waits on bit `100` of MMIO `10008000`, calls `0055D280(16)` while busy, clears
`10008020`, writes the masked source pointer to `10008030`, starts a transfer
with control `145`, and waits for completion. Its normalized resident copy is
`004E7450`. Both calls per frame need a real host rendering/transfer boundary.

The three `00564xxx`/`00565170` calls also belong to command construction:
they write words such as `30000009`, `50000009`, `30000002` and `50000002` through
the pointer at `00223790`, advance pointers, and reference command data. They
cannot be treated as controller polling just because they occur at frame start.
Core `00138D70`, used in frame-tail branches, directly accesses display MMIO
`12001000`; it also needs a host presentation/synchronization boundary.

The initial state-15 branch eventually reaches `00630388`, `00538610`, and
`00549918`. The state-zero branch instead reaches `005BE7B0`, `00630438`,
`00549918` and `00527CE0`, with core calls between them. These are different
observed sequences; their state names and game effects remain to be recovered.
`00630438` calls record accessor `006658C0`, several initialization/update
routines, network code `01E9E798`, and subsystem operations. It is an essential
next target for object/character setup, rather than a replaceable scene demo.

The frame tail reaches `005800D0` (10,088 bytes), `0057EAC0`, several additional
subsystems, command synchronization and finally the request consumer. In
`0057EAC0`, a function pointer is selected from the table at `003B75B8`; simply
translating direct calls is insufficient to cover that path.

## Independently bound consumer draft

`nonmatching/L01_005A9030.c` now expresses the actual L01 consumer rather than
using resident addresses. Its 35 direct call sites reach 31 overlay callees and
one resident core callee, `00163840`. The two switch tables are at `003AD710`
and `003AD760`; all 17 relative targets in each agree with the resident copy.

There are 67 observed constant-address references and 32 distinct bindings,
of which 22 change. In particular, the pending request is at `0021E814`, the
delay counter at `0021ED24`, parameters at `0021ED28`/`0021ED2C`, completion pointer
at `0021ED30` and progress at `0021E85C`. State `0021DDB4` stays at its address.
The reviewed transfer record expressions remain `001CFB30 + index * 14C`,
shared word `001CFAFC` and payload `001CFAF8`. Their valid index domain is still
unresolved. Neither the positional binding check nor these expressions establish
all callee signatures or complete record layouts.

The L01 draft compiles to 1,460 bytes against 1,416 in retail and remains outside
accepted coverage and native builds. The native consumer fixture currently tests
the resident draft; it must not be presented as execution of Dreadzone's loop.

## Object and input work still required

The draft entry already uses the L01 pointer list at `003660F8`, records based
at `00368F60` with stride `31F0`, and the child pointer at record offset `2F28`.
Actual accessor `006658C0(index)` returns `00368F60 + index * 31F0`. The separate
callee `00529310` obtains record one through that accessor, reads its pointer at
`2F00`, and tests a byte at pointee offset `551`. These are concrete object
dependencies; they do not yet establish a fully initialized Ratchet object,
animation, collision or player update.

The polling path, packet decoder, edge processing and actor-side input bindings
are now identified in `DREADZONE_INPUT_PATH.md`. Actual packet, reset and callback
algorithms execute in native fixtures. The driver/configuration state machine and
actor integration remain unfinished; the free-camera controls do not implement
gameplay. Likewise,
the verified transfer and presentation boundaries need host implementations
that consume the recovered game's output. The existing terrain/Tie renderer
provides assets and a rendering experiment, not those game interfaces.

The next useful integration checkpoint is to recover the setup and update
dependencies above, resolve all L01 callbacks and globals, and observe the actual
entry changing state and producing a frame with initialized objects. Neither
the compilation of this initializer nor passing callback fixtures proves that
checkpoint has been reached.
