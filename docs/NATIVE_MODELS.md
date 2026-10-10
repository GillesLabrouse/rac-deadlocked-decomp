# Native model inspection

## Original animated body diagnostic

`run-model-animation.cmd` opens the latest local animation diagnostic. The
native preparation evaluates original animation index 0: 30 frames and their
half-frame samples, 107 joints, 46 selected packets and 3,326 triangles. This
produces an articulated body rather than the previous static outstretched pose.
The small motion is shown at an explicit diagnostic presentation rate; the
animation has no inferred state name and no recovered actor clock. There are
no character controls, world placement, collision or gameplay in this viewer.

From the repository root, double-click the launcher or run:

```powershell
.\run-model-animation.cmd
.\run-model-animation.cmd --software --pose-frame 30 --capture build/native-assets/animation-frame15.png
```

`--pose-frame` freezes a **sample index**, so 30 selects original frame15 at
fraction 0. Without that option, the viewer loops 60 samples at 60 samples/second.
W/A/S/D, Q/E and arrows operate the inspection camera. No emulator or BIOS is
used in preparation or playback. The GIF in `build/native-assets` contains
actual software captures of every sample, not an inferred animation.

To regenerate using the current local direct-ISO validation inputs and PNG:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/check-native.ps1
venv/Scripts/python.exe tools/prepare_native_animation.py `
  build/drafts/root-animation-original-armor-container.bin `
  build/drafts/root-animation-original-ratchet-animation-class.bin `
  --texture private/overlays/globals/armor/armors/0/0.png `
  -o build/native-assets/armor-0-native-animation.dlscene
```

The recovered native stages initialize the real skeleton16 records through
589368, decode the real fixed/current/next banks through 5894F0 and VU 160/250,
generate matrices through 589950 and VU 738/858, apply the real bind 48 records
through 589B60 and A40/AA8, then execute selected vertex blend and GS FIFO
topology. Host arithmetic remains an ordered binary32 approximation of VU
arithmetic. The preparation selects explicit banks/fractions; it does not run
the DMA loader, actor update, secondary blending or original rendering clock.

An authored cache-write dependency proof ensures that every inherited unknown
word reaches only ignored scale.w/translation.w lanes. Quaternion, XYZ and
skeleton parent fields are independent of any invented initial VU image.
Independent original EE/VU replay of the entire matrix chain agrees with all
60 prepared samples: 497,400 EE instructions, 394,440 VU pairs and 410,880 matrix
bytes, with zero differences. The replay models only the bounded bind-copy
memory effect of actual DMA registers, not hardware timing. Original MPG words
are verified against local resident data. Native decoder, blend and topology
have separate original-instruction differential validation.

The separate local `.dlposes` file carries evaluated positions, linked to the
base scene's complete ordered vertex bytes by FNV-1a. Its 64-byte little-endian
header contains magic `DLPOSE01`, version 1, sample/vertex counts, presentation
rate, fingerprint, six bounds and a zero reserved 64 word. Payload is sample
order, vertex order, three binary32 positions. The loader limits files to 256 MiB,
requires one identity instance, finite bounded positions and exact extents, and
preserves UV/color/topology while updating a mutable Direct3D vertex buffer.
The viewer performs no host joint interpolation. All packages, disc-derived
inputs, reports and captures remain local and ignored.

## Original packet and computed vertex diagnostic

`run-model-topology.cmd` opens the latest native selected-body diagnostic.
Preparation executes the recovered packet selection, local vertex/blend and raw GS FIFO topology
stages instead of importing an editor GLB. The viewer displays the resulting
static package. With the locally prepared Armor0 class and 107 pose/binding
matrices and explicit mask `0x1002`, preparation computes 4,238 output vertices
and 3,326 triangles from 46 original packets. The primary body is followed by
bangle12 and bangle1 in the real selection order. The model has a static pose
and no animation, actor update, collision or gameplay.

Build using `tools/check-native.ps1`, then regenerate from the current local
validation inputs:

```powershell
venv/Scripts/python.exe tools/prepare_native_topology.py `
  build/drafts/startup-agent-original-armor-container.bin `
  --joint-matrices build/drafts/startup-agent-armor0-bound-matrices.bin `
  --actor-mask 0x1002 `
  --texture private/overlays/globals/armor/armors/0/0.png `
  -o build/native-assets/armor-0-native-topology.dlscene
run-model-topology.cmd
```

The class must be unrelocated. Joint matrices and PNGs are explicit local
inputs; this bridge does not create a constructor or supply an animation.
`--identity-joints` is an explicitly labelled comparison fixture only.
The mask comes from the fresh actor reset/constructor OR boundary; it does not
establish the final live player state. Existing actors can retain different
masks. `--low-detail` explicitly selects low-detail candidates without running
distance-based LOD or render-queue budget checks. Omitting `--actor-mask`
retains the primary-only comparison.
Its literal is directly present in the local ISO at `0x96DC9E00`. Independent
TOC/primary-section parsing and execution of the entire original `00157738`
copy publish `0x1002` to `0x2206B0` under the actual caller limit and explicit
level1 state. This verifies the data boundary, not the disk driver, complete
constructor or full native startup.
Positions use genuine byte/256 blends, persistent matrices/guest IDs,
delayed vertex associations and duplicates. Signed ST divides by 4096. Raw
FIFO winding and ADC suppression are retained without editor strip parity
normalization. The bridge presents raw axes as (y,x,z), scaled by class24/1024,
with flat double-sided shading. Actor scale/basis, live camera/view, clipping,
lighting, texture upload and DMA are outside this diagnostic.

The current computed-vertex bytes and all triangle/ID/ST/texture records
agree with independent original instruction replay. The actual VU1 replay
uses an explicit non-clipping view and valid caller GIF headers; it does not
establish live GS initialization or an original game frame. The software
capture contains 142,531 non-background pixels and has been visually inspected:
the head, torso, arms and legs are visible in a static pose with outstretched
arms. Generated assets and
captures remain local and ignored.

For a repeatable capture:

```powershell
run-model-topology.cmd --software --capture build/native-assets/armor-0-native-topology.png
```

## Editor-mesh bridge

`tools/prepare_native_moby.py` imports one explicitly selected mesh from a local
Wrench GLB export into the existing native scene package. The renderer executes
on Windows with Direct3D 11, hardware or WARP. No retail instructions are run.
This supplies geometry inspection for the future actor backend; it does not
construct an actor, skin its vertices, animate it or execute game logic.

## Inspect the current armor model

From the repository root, using the existing local extraction:

```powershell
venv/Scripts/python.exe tools/prepare_native_moby.py `
  private/overlays/globals/armor/armors/0/mesh.glb `
  -o build/native-assets/armor-0-preview.dlscene
build/native/Release/deadlocked_native_scene_viewer.exe `
  build/native-assets/armor-0-preview.dlscene
```

The default selection is mesh zero, the exported primary armor geometry. It
contains 2,714 triangles and one used texture. Its 27 alternative/component
meshes are deliberately omitted and listed by count in the generated report;
the preview is an incomplete body, with no selected head or legs. Those parts
require the game's actual model-selection and animation state. Use
`--mesh-index N` to inspect an individual alternate mesh, without combining
mutually exclusive LODs or attachments. No placement into Dreadzone is inferred.

After preparation, `run-model-preview.cmd` opens this local package. Its camera
frames the model automatically; the existing W/A/S/D, Q/E and arrow controls
remain inspection controls. For a repeatable software capture:

```powershell
run-model-preview.cmd --software --capture build/native-assets/armor-0-preview.png
```

The primary armor package rendered successfully with hardware and WARP, each
capture containing 89,815 non-background pixels. The software image was visually
inspected: the textured torso and arms are present; the remaining body is absent.
Generated packages, textures, reports and captures stay ignored and local.

## Supported format and limits

The importer follows the embedded buffers, accessors, primitive indexing and
material references of the [Khronos glTF 2.0 specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html).
It supports the observed untransformed Wrench editor-mesh subset: one selected
standalone node, float positions, float or normalized integer UV/color arrays,
local PNG base-color images, triangle lists and triangle strips. Indexed strip
connectors preserve winding parity even when degenerate triangles are skipped.
It retains exported coordinates and UV values. Normals are validated but the
current flat texture shader does not use them for lighting.

Malformed buffer/view extents, incorrect indices and attribute counts, nonfinite
values, external texture paths, parent transforms, skinning, morph targets,
required extensions and blended-alpha materials fail explicitly. The renderer's
existing alpha cutoff and flat texture shading are preview behavior, not verified
original game shading or complete glTF material reproduction. Selecting one mesh
does not establish which parts, joint matrices or LOD the game would draw.

Twelve authored tests verify interleaved accessors, normalized UVs, independent
coordinate/color expectations, strip winding and connector parity, alternate
mesh selection, and rejection of invalid layouts, transformations and paths.
An inventory of 380 primary local armor/Moby exports imported 288 meshes
(131,956 triangles); 91 contain unsupported joint/weight attributes and one
has no drawable primary triangles. Unsupported models are not silently simplified.

The next actor-rendering prerequisite is the recovered class/model allocation
path plus its actual joint/part selection. The editor export contains no recovered
game animation or player update, and no campaign frame is added by this tool.
