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

The remaining 60 marker placements belong to these classes:

| Class | Placements | Class | Placements |
| --- | ---: | --- | ---: |
| 3107 | 1 | 3168 | 1 |
| 3174 | 1 | 3305 | 7 |
| 3950 | 25 | 8226 | 1 |
| 8246 | 1 | 8247 | 1 |
| 8321 | 5 | 8355 | 1 |
| 8520 | 1 | 9207 | 1 |
| 9213 | 8 | 9271 | 2 |
| 9340 | 2 | 9453 | 1 |
| 9918 | 1 | | |

The extracted class definitions and the checked ISO-derived Wrench exports have
no drawable GLB or native core for these 59 placements, except class 9207's
core. That core has 107 joints but a zero primary packet count, so the native
topology stage has no primary geometry to render. The empty class stubs for
8321, 8520, 9213, and 9271 do not reference any mesh or binary resources.
Class 8321's update structure includes `ArmorVars`, but Wrench defines that
block as armor percentages, flags, prior hit points, and armor bits. It does
not identify a model resource or prove that the generic Armor0 mesh belongs to
these placements; see the upstream
[ArmorVars definition](https://github.com/chaoticgd/wrench/blob/75f6d86ad055dad5955cf9d771dd8146cbf7d6f6/data/overlay/src/game_dl/mobyutil.h).

A focused inspection of the arena cluster rendered class 8432's four placements
as a separate fixture and class 8519's nearby placement as a separate console
prop. Their proximity to the five class 8321 placements does not establish that
either mesh is class 8321's body. Class 8321 pvars contain words that numerically
match some placed Moby instance IDs, but their field schema is unverified; the
matches are not treated as model references. Keep class 8321 as unresolved
position-only markers until its model link or geometry is recovered.

The class IDs 8246 and 8247 also exist in the separate TIE asset namespace, with
TIE meshes and textures present in the extraction. Their Moby class definitions
have no materials or exported mesh. Numeric ID equality across TIE and Moby
classes does not establish asset equivalence, so the TIE meshes are not used for
the mission's unresolved Moby placements.
