# Parallel function recovery

The native milestone is an initialized Ratchet in the first playable scene,
with actual input, movement, terrain contact and live animation. Exact PS2
recovery and native behavior are separate acceptance tracks.

## Ownership

- Root owns startup, player/scene composition, accepted sources, generated
  configuration, shared reports, integration and final validation.
- `luna_memory` owns the first resident memory/initialization batch:
  `0011C9C0` and `00127020`.
- `luna_commands` owns the first resident queue/calculation batch:
  `00127DF0` and `00127E30`.
- The existing collision agent finishes its already running wall-contact lot.
  A new third recovery agent is conditional on measured integration capacity.

Agents write only their assigned draft files and private receipts. Source and
object basenames carry the agent prefix, including the intermediates produced
by `cc.sh`. They never run the shared build, audit, report generator or linker
entry point. Every object is compiled and linked into its own draft output.
Saved ground/idle work remains preserved for root; it is not counted as accepted.

## Acceptance

1. Establish the real entry, returns, delay slots, extent, structures and calls
   from the correct local reference. Splitter metadata alone is insufficient.
2. Recover C with the established m2c/SN workflow and inspect inferred types,
   argument registers and pointer strides.
3. Validate build provenance and compare only actual relocation fields at the
   object stage. Resolve symbols with the real linker and compare exact bytes.
4. Root repeats the frozen candidate build and exact check, then performs the
   required shared build, audit, exact linking and progress checks.
5. Only successful candidates enter `src/` and matching coverage. Failed or
   deferred sources remain drafts with their blocker and attempt count.

Cross-level grouping never supplies a caller's local address. A future common
function batch must independently locate each level's entry and dependencies,
and exact-link against that level's ELF. Behavioral tests and simulated calls
do not establish an exact match.

## Raw evidence before interpretation

Before each new lot, run `tools/check_recovery_candidates.py --native-evidence`
for its actual address names. Search accepted native headers, drafts and earlier
receipts before recovering another copy or declaring a missing producer. The
guard reports unrecovered resident library hints as
`library_mapping_requires_review`, including libc rows absent from source
coverage. Inspect the archive object's actual relocations and unmasked structure
offsets before deciding ownership. A library hint is not an exact match or a
reason to assign the same address in another overlay to that library.
`map_archive.py` preserves constants, requires actual relocation compatibility
and refuses ambiguous candidates. Full resolved matching checks remain required.
The optional native mentions are navigation hints only: inspect the referenced
level, ABI, supported domain and proof before reuse. They do not change matching
availability, prove function coverage or establish shared overlay addresses.
For global writers, also inspect direct LUI plus displaced stores; searching
only LUI/ADDIU address formations misses normal global store sequences.

Record branch opcode, signed displacement, computed target and executed delay
slot directly from the reference word. Generate effective addresses from the
actual base and sign-extended immediate; do not manually transcribe a decoded
mnemonic as a raw word. Distinguish wrapping ADDIU/ADDU from trapping ADDI.
For small predicates, execute the original integer CFG across its bounded input
domain before compiling a second variant. This catches a wrong branch meaning;
it still does not replace the exact relocation/resolved-byte acceptance gates.
Inferred m2c prototypes and residue registers must be checked against callees.

Keep a failed report and add a separate correction with raw evidence and hashes.
Infrastructure failures, semantic errors and genuine compiler nonmatches are
different outcomes. Two low-yield or error-prone lots trigger a bounded review
before further variants. Root must verify every candidate and material startup
claim independently; adding agents does not compensate for an integration or
validation bottleneck. Independent reviews can use a stronger model when the
user has authorized model selection. No agent writes shared configuration.

## Throughput

Record the UTC start/end, attempted functions, compile failures, relocation
comparison failures, exact-link failures, candidates delivered, root-accepted
functions and root integration seconds for each batch in `PROGRESS_LOG.md`.
Acceptance rate is newly accepted distinct functions divided by elapsed hours
from batch dispatch through final root validation. Report agent elapsed time
and root integration time separately. Repeated variants are validation attempts,
not additional functions. An unfinished window has a provisional rate only.

The first two-agent window starts at 2026-10-11 10:19:14 UTC with 280 accepted
functions. Native modules and saved NONMATCHING drafts are excluded from this
denominator. Keep two recovery agents until final validation shows useful
throughput; add a third only when integration is keeping up. Complex blocked
functions are saved and replaced by a small independent batch, preferably a
dependency of the first scene.
