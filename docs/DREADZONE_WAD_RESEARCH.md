# Dreadzone Scene WAD Research

The Dreadzone Station `scene.wad` audit inventories 30 scene records and their
69 compressed chunk references. It validates sector bounds and decodes the 608
nonempty chunks, then records the four-entry resource tables and aligned
zero-reserved nested-resource candidates. Candidate IDs remain provisional;
the format has not been mapped to Moby classes or scene-object types.

In scene record 20, chunks 5–32 each contain candidate IDs 9227 and 9314. Their
pointed-to regions contain recurring 16-byte rows. The audit summarizes the
available row count, unique row count, identical leading run, and first row in
hex without interpreting the bytes. The observed distributions are:

| Candidate ID | Available rows | Occurrences | Unique rows | Identical leading rows | Rows after that run |
| --- | ---: | ---: | ---: | ---: | ---: |
| 9227 | 117 | 24 | 21 | 49 | 68 |
| 9227 | 181 | 3 | 85 | 49 | 132 |
| 9227 | 541 | 1 | 523 | 10 | 531 |
| 9314 | 49 | 24 | 1 | 49 | 0 |
| 9314 | 113 | 3 | 65 | 49 | 64 |
| 9314 | 522 | 1 | 513 | 10 | 512 |

These row patterns do not establish vertex, index, packet, or object semantics.
No triangle topology, model-class relationship, visibility rule, or orientation
has been verified from these candidates. They are not added as geometry to the
review scene. Class 9207 therefore remains unresolved where the available
extracted assets contain no drawable mesh.

Recreate the inventory with:

```powershell
python tools/audit_dreadzone_scene_wad.py --output build/native-assets/dreadzone-scene-wad-audit.json
```

The JSON output retains raw candidate offsets and the structural summary so
later format work can be checked against the same source data.

## Scene 20 resource 9228 coordinate stream

The 28 direct resource entries with candidate ID 9228 in scene record 20,
chunks 5–32, contain 1,333 bounded 16-byte rows when each payload is capped at
the next resource descriptor. Chunks 5–31 each contribute 49 rows and chunk
32 contributes 10. The final row of each chunk exactly repeats the first row
of the next chunk, producing a continuous ordered sequence. Every row decodes
as four little-endian floats, with the fourth component zero. The first three
components range approximately over X 10.127–11.188, Y 9.253–11.019, and Z
1.226–2.691.

The 9228 coordinates vary along all three axes and have many short steps; they
do not form a simple monotonic path. Comparing these rows with mission
placements finds no coordinate matches. Neighboring candidate resource IDs
9207 and 8484 also have no direct coordinate matches to their similarly
numbered Moby placements. These checks do not establish whether 9228 holds
vertices, another local coordinate buffer, or unrelated data; no associated
index stream, packet header, transform, or triangle topology has been verified.
The sequence is not rendered or assigned to an object.

Candidate IDs 9227 and 9314 also recur in these chunks. The bounded 9227 region
is 49 repeated copies of one point per chunk (with chunk 32's final block
truncated to 10 rows). The bounded 9314 region begins with 49 repeated copies
of a second point per chunk, and continues with varied rows in some chunks.
These regularities tie the buffers structurally to the same repeated chunk set,
but do not identify their object or role.

### Inspect the raw 9228 coordinates

The raw coordinate stream can be opened in the native scene viewer as a
separate point-cloud diagnostic. The octahedra are artificial display markers;
the source rows retain their raw coordinates and identity placement. This
diagnostic does not recover an object mesh or assign a coordinate frame.

```powershell
python tools/audit_dreadzone_scene_wad.py `
  private/overlays/levels/campaign/1_dreadzone_station/scene.wad `
  --output build/native-assets/dreadzone-scene-wad-audit.json
python tools/prepare_dreadzone_wad_coordinate_diagnostic.py `
  --audit-report build/native-assets/dreadzone-scene-wad-audit.json `
  -o build/native-assets/dreadzone-scene20-9228-points.dlscene
build/native/Release/deadlocked_native_scene_viewer.exe `
  build/native-assets/dreadzone-scene20-9228-points.dlscene
```

The companion JSON report records the selected source rows and keeps the
coordinate-frame, topology, and object-mesh flags false. Keep this scene
separate from playable level packages until those properties are established.
