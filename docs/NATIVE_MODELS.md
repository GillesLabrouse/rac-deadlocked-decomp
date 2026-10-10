# Native editor-mesh inspection

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
