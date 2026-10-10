# Dreadzone controller input path

This records observations from the user's NTSC-U retail image and its campaign
L01 overlay. Addresses below are actual L01 addresses unless marked core.
The independently written [decoder draft](../nonmatching/L01_005BDA40.c)
and its two recovered math helpers compile for both PS2 and Windows, but remain
nonmatching. Native unit tests execute these three actual implementations,
the selective reset helpers, and the separately recovered actor packet callback
`006B5CB8` with its real bit writer. They remain outside matching coverage.
The controller driver, complete polling state machine, callback-context startup,
and actor integration are still pending; no playable gameplay frame has been
completed by this work.

## Frame to packet

The Dreadzone entry at `00528098` calls `005BE7B0` from several frame-state
branches, including state zero and the initial state 15. `005BE7B0` walks eight
input records starting at `001EE600`, with stride `5C0` and end `00201400`.
It passes each record to `005BE338`, then calls `00614EE8`.

| Function | Observed role | Retail extent |
| --- | --- | --- |
| `005BE7B0` | Poll all eight input records | `58` bytes |
| `005BE338` | Connection/configuration state machine and packet dispatch | `478` bytes |
| `005BDA40` | Decode buttons, axes, pressure, delay, history, repeat | `888` bytes |
| core `0013BA98` | Query controller state using port and slot | Core boundary |
| core `0013BA20` | Copy the selected controller packet to the supplied destination | Core boundary |

The three L01 functions have uniquely identified resident code-shape
counterparts `00490878`, `00490400`, and `0048FB70`. This grouping was used to
find them; actual L01 instructions were then inspected for calls, constants,
record offsets, and global addresses. Grouping with masked immediates is not a
proof of data bindings or C equivalence.

`005BE338(record)` reads the port byte at `record+54C` and slot byte at `+558`.
It saves the previous logical held mask from `+348` to `+1AC`, saves the previous
raw mask from `+1B0` to `+1BC`, and clears `+1A0` and `+1B0`. It stores the queried
connection state at `+19C`; a zero state also clears `+198` and sets byte `+551`.
The configuration switch has 79 entries and remains unrecovered in C.

For accepted connection states 6 or 2, the state machine calls
`0013BA20(port, slot, record+574)`. A nonzero result and zero first packet byte
permit dispatch. It clears byte `+551` and computes
`length = min(2 + 2 * (packet[1] & 15), 20)`.
If the callback pointer at `+56C` is nonzero, it calls
`callback(record, packet, length, context_at_570)` **before** the decoder.
It then calls `005BDA40(record, record+574, length)` and stores packet byte 1
at `+1DC`. This callback may modify input; a native adapter cannot simply bypass
it without investigating its actual binding. The state machine also invokes
core `0013BF40` using `record+188` for a separate feedback operation.

The core packet-copy function invokes `0013B8C8` to choose a controller buffer
and copies its selected length with core `0011A370`. These are real platform
boundaries, not a host input implementation. Their observed binary behavior
does not establish a complete native driver or a protocol for every device.
`00633178`, previously a candidate by proximity, belongs to DMA and is not
evidence of controller input.

## Actor linkage

The `5C0` input record is distinct from the `31F0` actor record. The actor
accessor `006658C0(index)` returns `00368F60 + index * 31F0`.

In actual L01 code, `00529310` obtains actor index 1, loads its input pointer at
actor offset `2F00`, and checks the pointed record's byte `551`. For pointer
`001EEBC0` (input-pool index 1), it checks alternative `001EFD00` (index 4) and
stores that pointer to actor `+2F00` if the alternative's byte `551` is zero.
The arithmetic is `001EEBC0 + 1140 = 001EFD00`.

There is a direct consumer inside `00630438`: instructions starting at
`006304B0` load actor `+2F00`, read input `+1A4`, and test bit `0800`.
This proves that the recovered pressed mask reaches an actual actor path.
It does not prove that the rest of the actor update, movement, camera, or
rendering runs natively. No physical button name is assigned to bit `0800`
without further mapping evidence.

## Decoder fields and ordering

The draft's generated `fXXX` names preserve byte offsets. Compile-time layout
checks cover the full `5C0` size and the major arrays and scalar offsets. The
native interface is [dreadzone_input.h](../native/dreadzone_input.h).

| Offset | Type / extent | Observed use |
| --- | --- | --- |
| `100` | 16 floats | Selected delayed channels, later subject to mode masks |
| `140` | 16 floats | Copy made before mode masking, with flip correction |
| `18E`, `190` | signed 16-bit index, signed word count | History index `0..29`, count capped at 30 |
| `1A0`, `1A4`, `1A8`, `1AC` | four words | Logical held, pressed, released, previous held |
| `1B0`, `1B4`, `1B8`, `1BC` | four words | Raw packet mask and edge/history variants |
| `1C0`, `1C4`, `1C8` | three words | Snapshots before mode/history/repeat processing |
| `1CC` | word | Mode mask selector |
| `1D0`, `1D4`, `1D8` | three words | No held bits, no directional bits, nonzero axes 2/3 |
| `1E0`, `258`, `2D0` | three arrays of 30 | Pressed history, angle history, magnitude history |
| `348` | word | Logical held mask saved for the next poll |
| `34C`, `350` | two words | Sample-ring write index `0..6`, delay warmup count |
| `354`, `370` | two arrays of seven words | Sampled logical and raw button masks |
| `38C` | seven arrays of 16 floats | Sampled channels |
| `54D`, `54E`, `54F` | three bytes | First-repeat delay, repeat period, countdown |
| `568` | word | Sample delay; draft requires the observed domain `0..6` |

The decoder first reads packet bytes 2 and 3 as a big-endian mask and inverts
the low 16 bits: `((packet[2] << 8) | packet[3]) ^ FFFF`. The caller must supply
these bytes; the decoder has no short-packet guard. This draft therefore
requires a readable packet of at least four bytes even when `length < 8`.

For `length >= 8`, bytes 4 through 7 become channels 0 through 3. Each byte is
centered on 127. Absolute distance below 48 gives zero; distance at least 48
gives `(distance - 48) / 76`, capped at 1 and negated for bytes below 127.
Thus bytes 79 and 175 produce signed zero, and bytes 78 and 176 are the first
nonzero samples. For `length >= 20`, bytes 8 through 19 become channels 4 through
15, multiplied by the observed float constant with value
`0.0039215688593685626983642578125`.

Global byte `0021DE31` negates sampled channels 0 and 2. State word `0021DDB4`
equal to 7 augments the logical sample ring with directional bits from channels
2 and 3, while the raw sample ring retains the original packet mask. The delay
selects `(write_index - delay + 7) % 7` after warmup. Before warmup finishes,
previous masks and existing selected channels are retained. The write index
then advances modulo seven.

The selected channels are copied to `+140`; directional bits are added to held
`+1A0` from channels 2 and 3. Pressed and released are respectively
`~previous & held` and `~held & previous`. Raw edge fields deliberately preserve
the actual mixed-mask formulas: `1B4 = ~1AC & 1B0` and
`1B8 = ~1A0 & 1BC`. They must not be rewritten using an assumed raw-only history.
Snapshots and `+348` are stored before the following mode processing.

Mode 1 clears mask `5030` and channels 0/1. Mode 2 keeps only mask `0900`, also
applies it to raw held, and clears channels 2/3. Global word `0021E8A0` resets
the mode when nonzero; using resident address `0021EE50` here would be incorrect.

Magnitude is calculated by a VU sequence from channels 2/3. Actual helper
`0055D708(float x, float y)` returns their angle; `0055D910(float old, float now)`
returns a wrapped absolute angular difference. Magnitude greater than 0.9 can
generate pressed bit `10000` from up to three preceding magnitudes or four
preceding angles. The magnitude threshold is 0.25 for the weak-to-strong history
transition, and the angle threshold is `0.959931075572967529296875`. History
advances unless state `0021DDB4` is -1. Repeat processing occurs last and can
replace pressed with held, including when its byte countdown wraps to 255.
Consequently `+1C4` and the pressed-history array can differ from final `+1A4`.

## Verification and next dependencies

The independently written sources contain no inline assembly or forced register
allocation. The decoder compiled with the existing SN pipeline using its declared
`-mno-split-addresses -ffast-math` and opt-in `--float-immediates` flags:

```sh
bash tools/cc.sh nonmatching/L01_005BDA40.c \
  build/drafts/math-agent-input-decoder.o -mno-split-addresses -ffast-math
```

| Source | Compiled bytes | Retail bytes | Matching |
| --- | --- | --- | --- |
| `L01_005BDA40.c` | 2048 | 2184 | No |
| `L01_0055D708.c` | 424 | 280 including zero branch | No |
| `L01_0055D910.c` | 64 | 56 | No |

Relocation-aware comparison fails for all three, so none contributes matching
coverage. The polynomial's original entry extent is 272 bytes, with the
eight-byte zero-input return at `0055D818` included in the recovered C function.
Both math helpers use opt-in `--float-immediates` to avoid compiler literal-pool
relocations. The polynomial's pi/4 immediate form was checked at `0055D7EC`;
the delta's pi form was checked at `0055D914`. Actual-address link comparison
now reports the polynomial's full verified `118`-byte span with `--span
0055D708:118`; its scalar recovery and the delta remain nonmatching.

[dreadzone_input_tests.c](../native/tests/dreadzone_input_tests.c) executes the
decoder and both actual helpers without captured math calls or driver stubs.
MSVC C11 `/W4 /WX /O2 /fp:precise` builds and runs 4309 passing checks with
`DEADLOCKED_NATIVE=1`, `native/include`, and `native` include paths. The packet
fixture performs only the four previous-mask operations observed in the real
poll caller; it does not emulate packet transport or configuration.

Tests cover neutral packets, press/hold/release, every byte value on each of
four axes, short-packet thresholds, pressure channels, all delays `0..6` over
ring wraps, history saturation/pause, direction bits, flip/mode ordering, and
repeat timing/byte underflow. Actual helper results also exercise the strong
quarter-turn history trigger and distinguish final pressed, snapshots, and
history writes. Record guards check both storage boundaries.

An independent double-precision `atan2` oracle checks a `33 x 33` normal finite
grid in `[-1,1]`. The greatest circular angle error observed is
`3.34129247e-7` radians. A separate double-precision circular-distance oracle
checks the scalar delta leaf. Packet-domain piecewise axis and magnitude
oracles independently check thresholds and normalization. These are host C
unit tests, not a comparison with an executing PS2/VU implementation.

The draft replaces VU magnitude with scalar `sqrtf` on the host. Newly decoded
channels 0..3 are finite and in `[-1,1]`, so the arithmetic has a bounded finite
domain. Previously retained channels must also satisfy this condition.
No equivalence of VU rounding, accumulator precision, flags, or thresholds at
adjacent floating-point values has been established. Exceptional floats and
corrupted ring indices/counts are outside the current draft's verified scope.

The recovered [angle helper](../nonmatching/L01_0055D708.c) evaluates the actual
odd polynomial terms through degree 15 in the observed VU addition order. Its
host numeric coefficient and quadrant tables were independently read from the
user-supplied L01 overlay at `003B9540` and `003B9580`. The PS2 draft references
those actual addresses. The scalar recovery includes the zero-input branch
`0055D818` and preserves sign-bit quadrant selection.
The [delta helper](../nonmatching/L01_0055D910.c) preserves the actual single
wrap operation; it does not repeatedly normalize arbitrary angle differences.
Neither source uses SDK source, headers, or samples.

The focused next recovery target is the real poll state machine `005BE338`,
including the core controller boundary and execution of the recovered `+56C`
callback through the actual actor binding.
Actor consumers should then be connected through the verified `+2F00` pointer
while retaining this game's decoded state, rather than introducing unrelated
camera controls.

## Initialization producers and selective reset

The complete `001EE600..00201400` pool lies in the source image's `core.bss`
section (`001A5180`, size `78AB8`, type `SHT_NOBITS`). Its locally extracted
payload contains 11776 zero bytes for all eight input records. This establishes
the initial storage values of sample indices/counts, history index/count, delay,
mode, repeat bytes, and callback/context. It is distinct from the behavior of
any individual reset function. Host storage must provide the same zero-filled
initial pool explicitly; the selective reset cannot repair arbitrary records.

Three actual operations were recovered in
[L01_005BD950.c](../nonmatching/L01_005BD950.c), with a separate
[native interface](../native/dreadzone_input_reset.h):

| Function | Actual writes / effect | Fields deliberately preserved |
| --- | --- | --- |
| `005BD950(record)` | Clears selected/current channels, logical/raw masks and snapshots; sets `1D0` and `1D4` to 1; clears pending sample-ring entries | History arrays/index/count, `348`, `34C`, `350`, `568`, mode, repeat, previous masks, callback/context, padding |
| `005BD710(record)` | Sets history index `18E` and count `190` to zero; null selects pool record zero | History contents and every other byte |
| `005BD790(record, delay)` | Stores `568`; increasing the delay also zeros `350` and `34C` | Existing samples, history, options and other storage; decreasing/equal delay preserves both indices |

The selective reset clears the logical and raw rings and all 16 sampled
channels at circular positions `(34C - sample + 7) % 7` for
`sample = 0..350-1`. It does not clear every ring entry, advance the write
position, or restart warmup. Valid observed ring index/count domains remain
explicit preconditions. The delay setter does not clamp its input; native
tests restrict it to `0..6`. Its C function includes the real return branch at
`005BD7AC`, which is outside the extracted 28-byte entry extent.

There are actual actor-side producers beyond the isolated reset. Dispatcher
`006A83B0(index, phase)` calls `006A8420` for phase 0, `006A8758` for phase 1,
and `006A9390` for phase 2. The phase dispatch and those calls were directly
inspected, but the binding of this dispatcher to the very first gameplay frame
has not been established by this input work.

In `006A8758`, the input pointer comes from the word table at `0021DDD8` indexed
by the actor index and is stored to actor `2F00`. It calls `005BD790` with delay 2
or state word `0021DDC8`, depending on the inspected state condition. It then
sets input callback `56C` to actual function `006B5CB8`, and context `570` to
actor `2FEC`. `006A8420` also binds that callback/context after selecting a pool
input record. The callback has its own packet-history/context operations. It is
executed by the separate callback tests described below, rather than by the
decoder-only or selective-reset tests.

At the Dreadzone entry's exit path, `00666058` clears callback/context fields
for all eight records after three other subsystem calls. It does not initialize
ring indices, delay, mode, or repeat. The earlier resident controller-library
opening path (`00460138 -> 0048F960`) is distinct from these actual L01 reset
helpers. Neither that opening path nor the full polling state machine is part
of the native test fixture.

[dreadzone_input_reset_tests.c](../native/tests/dreadzone_input_reset_tests.c)
executes the three new operations together with the real packet decoder and
math helpers. Under MSVC C11 `/W4 /WX /O2 /fp:precise`, 195 checks pass. An
independent byte-range oracle checks every preserved byte for all 49 ring-index
and pending-count combinations, with guards around the record. Another 49
cases cover all delay changes in `0..6`. Tests also verify zero-filled startup
storage for all eight records, the null history-reset fallback, delayed first
input, and real press/axis/release behavior after warmup and delay decrease.
The fixture's C static storage supplies its BSS precondition; no recovered
reset is credited with whole-record zero initialization.

The SN build and strict actual-address overlay link complete successfully.
Unmasked byte comparison fails for all three functions: selective reset is
240/240 bytes with 16 differing bytes, history reset is 24/32 bytes, and delay
setter is 36/28 entry bytes including its separate return branch. These are
nonmatching drafts and remain outside matching coverage. This selective-reset
test executes no controller opening, callback, actor frame, or gameplay input
transport. The separate callback test executes the real callback algorithm,
but still does not execute controller transport or an actor frame.

## Actual actor packet callback

The independent [callback draft](../nonmatching/L01_006B5CB8.c) recovers
`006B5CB8..006B5FD8`, an 800-byte function installed by the actor-side producers
above. This indirect-only entry is absent from the current retail splitter's
function list; its prologue, return, next prologue, and stored callback pointers
were inspected independently. Its observed ABI is:

```c
void func_L01_006B5CB8(InputRecord5C0 *record, const u8 *packet,
                      s32 length, DreadzoneInputContext1E0 *context);
```

The callback reads record mode `1CC`, preserves the record and original packet,
and updates the actor context plus two statistics counters. Its context begins
at actor `2FEC` and occupies `1E0` bytes. The
[native interface](../native/dreadzone_input_callback.h) preserves fixed-width
guest fields and exact offsets; it contains no host pointers.

| Context offset | Storage | Observed use |
| --- | --- | --- |
| `0` | Four 16-bit snapshots | Packet bytes 2/3 copied bytewise, giving a little-endian raw mask |
| `8` | Four 16-bit snapshots | Packet bytes 6/7 |
| `10` | Four 16-bit snapshots | Packet bytes 4/5 |
| `18` | Word | Low nibble marks changed frames; upper bytes overlap the stream |
| `19..3B` | 35 bytes | Compact button-change stream, most-significant bit first |
| `3C` | Signed word | Accumulated stream bit count |
| `40..1D7` | Preserved storage | Not modified by this callback |
| `1D8` | Signed word | Frame count, with snapshot slot `count % 4` |
| `1DC` | Signed word | Published stream bit count every fourth frame; otherwise -1 |

The bytewise button snapshot deliberately differs from the decoder's big-endian
mask interpretation. A nonzero record mode selects the observed neutral packet
at L01 `003BDD40` for the callback's local operations. Its raw buttons are `FFFF`
and four stick bytes are 128. This selection does not replace the original
packet subsequently passed to the decoder. A length below eight still snapshots
the selected packet's button pair, but uses neutral stick pairs.

For slot zero, the comparison mask is always `FFFF`; other slots use the
preceding button snapshot. Changed bits are scanned from bit 0 to bit 15.
One, two, or three changes are encoded as four-bit indices, zero separators,
and a final one terminator, taking 5, 10, or 15 bits. Four or more changes
restore the initial cursor and encode the complete raw mask in four nibbles,
from high to low, with zero separators: 19 bits. An unchanged frame has no
payload and clears its slot flag. Updating the flags preserves the overlapping
stream bytes in word `18`.

Every fourth frame publishes the cursor to `1DC`, increments actual counter
`002208AC` by four and `002208A8` by `4 + cursor`, then resets the cursor.
These actual addresses were calculated from L01 GP-relative accesses, rather
than copied from a resident counterpart. The next slot-zero comparison still
uses `FFFF`, not the previous group's last snapshot.

The draft also recovers actual bit writer `006B59C8..006B5AF8` (304 retail bytes).
Its host C implementation writes byte-sized chunks while preserving bits outside
the requested range. The callback uses widths one and four. The native copy
dependency follows the observed forward-byte path of actual `0055D3B8` for the
callback's two-byte disjoint copies. Vector alignment, prefetch, and other copy
domains are not established by this recovery. There are no driver substitutes
or captured callback results in these implementations.

Both SN functions compile with the declared `-mno-split-addresses` flag and
link against actual L01 addresses. The strict draft diagnostic resolves every
relocation and compares bytes without masking:

```sh
bash tools/cc.sh nonmatching/L01_006B5CB8.c \
  build/drafts/math-agent-input-callback.o -mno-split-addresses
venv/Scripts/python.exe tools/check_overlay_draft.py L01 \
  build/drafts/math-agent-input-callback.o --entry 006B5CB8:320
```

The explicit entry size is hexadecimal `320` (800 bytes). It supplies the
independently verified pointer-only boundary solely to this draft diagnostic;
it does not change accepted metadata or validate a boundary by itself. The
callback compiles to 624 bytes and the bit writer to 164 bytes versus 800 and
304 retail bytes respectively. Both are **NONMATCHING** and contribute no
matching coverage. PS2 layout definitions are local to the source, while the
native header is included only for `DEADLOCKED_NATIVE`, preserving the existing
SN source/header provenance tracking.

[dreadzone_input_callback_tests.c](../native/tests/dreadzone_input_callback_tests.c)
passes 4250 checks under MSVC C11 `/W4 /WX /O2 /fp:precise`. An independent
bit-at-a-time oracle compares the byte-chunk writer across offsets 0..15,
widths 1..32, and four value patterns, plus zero-width preservation. A separate
format oracle checks unchanged frames, one to three changes, full-mask fallback,
four-frame publication, the slot-zero sentinel, modes, short packet lengths,
and unused storage. Nonzero backing bytes and context guards verify partial-byte
preservation and bounds. Tests also check that the full input record and source
packet remain unchanged.

A combined four-frame test runs the actual callback before the actual decoder
and both math helpers in the observed poll order. It verifies press, hold,
release, history advancement, snapshots, and a completed ten-bit stream with
statistics of four frames and fourteen bits. Packet construction and the
caller's previous-mask bookkeeping are controlled fixtures; controller polling,
network transport, and actor callbacks are not emulated.

The tested callback domain requires nonnegative frame/cursor counts, no signed
counter overflow, sufficient stream storage, a readable four-byte packet
(eight bytes when stick data is selected), and nonoverlapping packet/context
storage on a little-endian host. Four frames need at most 76 payload bits.
No bounds checks absent from the retail function are invented here. The stream
pointer is derived from the complete context object at byte `19`, allowing
access across the overlapping word and padding within that object. These callback
tests supplied initialized storage. Its actual producer is now recovered below;
the complete native actor frame remains unfinished.

## Actual actor-context producer

The vtable at `003B10C0` stores `00668070` at slot `2C` and `006A6598` at slot
`28`. `006A8420` invokes the former at `006A8478`; `006A8758` does likewise at
`006A88F0`. The larger constructor calls slot `28` at `00668120`, before the
callback/context bindings at `006A8514` and `006A8A30`. Both inspected producers
therefore perform the actual full actor clear before installing the callback.

`006A6598` invokes the real word filler `0055D2A8` with 0 and `31F0` bytes,
calls base constructor `006A93A0`, sets guest vtable word `14 = 003B10C0`, byte
`266D = 1` and byte `31CF = 5`. The entire embedded context `2FEC..31CC`, including
cursor `3C` and frame count `1D8`, is genuinely zeroed by this producer. Its native
union retains 32-bit guest fields and supplies the original word-fill dependency
from the shared memory recovery, without a CRT replacement.

The base constructor writes `14 = 003B0718`, calls identity helper `006A9638`
with the opaque subobject at offset `1970`, and returns the original actor pointer.
Its retail empty loops are omitted: the draft preserves the inspected memory and
return effects, not instruction timing. Calling the base constructor alone does
not initialize the callback counters or clear the actor.

The native context-initializer fixture passes 64,509 checks. A complete byte-image
oracle covers all `31F0` bytes on four initial patterns and external guards;
base-constructor preservation is checked independently. The actual initializer,
callback and decoder then run for 24 frames with each of the seven input delays,
checking delayed press/hold/release, histories, bounded stream publication and
preserved actor fields. Packet transport and input BSS remain fixture inputs.

Strict original-address checks report reset `006A6598` nonmatching at 80/80 bytes
(`--entry 006A6598:50`), base constructor nonmatching at 56/656, and identity helper
exact at 8/8. These L01 recovery files remain outside accepted matching coverage.
The complete `00668070` constructor, actor integration and controller transport
remain prerequisites for a playable frame.
