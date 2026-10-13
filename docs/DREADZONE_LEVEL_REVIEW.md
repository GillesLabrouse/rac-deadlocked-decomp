# Dreadzone Mission 0 Geometry Review

Run `run-dreadzone-mission-0.cmd` from the repository root. The script prepares
`build/native-assets/dreadzone-mission-0-review.dlscene` from the local
`private/overlays` extraction when needed, then opens it in the native scene
viewer. Pass viewer arguments after the script name to capture an image or set
a camera, for example:

```powershell
.\run-dreadzone-mission-0.cmd --capture build/native-assets/dreadzone-mission-0-review.png --software
```

The scene combines mission 0's static geometry and vegetation with available
Moby editor meshes and recoverable native bind-pose previews. The current
review scene contains 3,235 placements, including 683 Moby placements, and
uses 87,160 prototype triangles. These are editor geometry previews; the
viewer does not run mission behavior, visibility rules, lighting, AI, or
animation.

The adjacent JSON report indexes all 743 Moby placements in the selected scene
variant. Each row preserves its source instance ID, position, rotation, scale,
pvars, draw distance, occlusion value, mesh availability, and whether the
preview rendered or was filtered or omitted.

The current extraction has no drawable mesh for 60 mission Moby placements
across 17 classes. Those placements are omitted from this review scene rather
than represented with guessed object shapes. To inspect their source positions,
transforms, and omission reasons, build the optional diagnostic marker scene
with `--include-unresolved-moby-markers`; those markers show positions only.
See `docs/NATIVE_PORT.md` for the asset audit and converter details.

For a ready-to-open placement audit, run `run-dreadzone-mission-0-placement-audit.cmd`.
It builds a separate scene with the static level geometry, available Moby
previews, native bind-pose previews, and fixed-size markers for the 60 omitted
Moby placements. The markers use source positions only and do not imply object
shape, orientation, scale, or visibility. Moby preview rotations and in-game
visibility remain unverified. The adjacent JSON report retains the original
per-instance metadata and omission reasons.

Two omitted classes have known gameplay-control layouts in Wrench's Deadlocked
overlay definitions: class 3950 stores objective and cutscene state, and class
8355 stores mission and cutscene data. Their 26 markers are source-position
diagnostics, not evidence of missing physical meshes. See the upstream
[class 3950 layout](https://github.com/chaoticgd/wrench/blob/master/data/overlay/src/game_dl/update/moby3950.h)
and [class 8355 layout](https://github.com/chaoticgd/wrench/blob/master/data/overlay/src/game_dl/update/moby8355_mission_control.h).

The placement inventory now reports native-core availability separately from
editor-mesh and reusable-geometry availability. Of the 60 omitted placements,
only class 9207 has an indexed native core; its source has zero high-detail
packets and its editor GLB has no drawable triangles. The other 59 have no
native core source in the indexed exports. Wrench's level unpacker creates a
class core only when its level-core entry has a nonzero asset offset; see its
[Moby class extraction code](https://github.com/chaoticgd/wrench/blob/master/src/wrenchbuild/level/level_classes.cpp).

The placement-audit launcher prefers native core bind-pose geometry for class
8554. Its mission-local GLB contains nontrivial skin weights, so the static
editor-mesh path cannot safely treat its vertices as an unskinned model. The
native core produces a 6,550-triangle diagnostic preview with 107 joints and
67 packets. This is the bind pose with no selected animation; it does not
confirm the in-game pose, local axes, or mission visibility. The launcher keeps
the other native bind-pose previews enabled and retains position-only markers
for unresolved placements.
