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

The current extraction has no drawable mesh for 60 mission Moby placements
across 17 classes. Those placements are omitted from this review scene rather
than represented with guessed object shapes. To inspect their source positions,
transforms, and omission reasons, build the optional diagnostic marker scene
with `--include-unresolved-moby-markers`; those markers show positions only.
See `docs/NATIVE_PORT.md` for the asset audit and converter details.
