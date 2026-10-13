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

These are bytes available from each data pointer to the end of its decoded
chunk, not proven resource lengths. In all 28 chunks, a 9314 candidate
descriptor appears exactly after the repeated leading rows at the 9227 data
pointer; the gap is 49 rows in chunks 5–31 and 10 rows in chunk 32. The 9314
data pointer follows that descriptor and points to the repeated-row run. The
audit records the nearest following candidate descriptor and its aligned row
distance as structural evidence only; it does not link the IDs to model assets.

In chunk 32, the 9314 pointer is followed by 512 additional 16-byte rows
(8,192 bytes) after the 10 identical leading rows. This tail has no recognized
`PIF2`, `WAD`, or `VIF` signature and has not been identified as texture,
geometry, or another resource type. The other 27 occurrences end at the repeated
row block or have smaller tails (64 rows in three chunks). These boundaries
remain provisional because the resource format is unknown.

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

## Scene 0 PIF2 textures and repeated coordinate rows

Scene record 0 has a `moby_load` stream of 180,512 decoded bytes and one index
record with ID 8333 spanning offsets 32 through 45,680. The stream contains two
128-by-128 PSMT8 PIF2 images at offsets `0x30` and `0x5950`, each declaring four
mip levels. The images decode as an armored character texture and an orange
character texture. This supports that the stream contains character-related
texture data; it does not identify a mesh, Moby class, or placement.

In scene 0 chunks 0 through 22, a provisional chunk resource entry with ID 8333
points to 16-byte rows. For chunks 0 through 21, 49 rows are available before
the decoded chunk ends. Their first-to-last points form a continuous sequence:
each chunk's last row exactly matches the next chunk's first row (22 matching
boundaries). Chunk 22 has 153 rows available to the end of its chunk, so its
resource boundary is unknown. The other two entries recurring beside ID 8333
hold repeated points. Although the 8333 sequence is coordinate-shaped, its role
and coordinate frame are unverified. It is not treated as a path, vertices, or
renderable geometry.

Export supported base mips for inspection with:

```powershell
python tools/audit_dreadzone_scene_wad.py --extract-pif-dir build/native-assets/dreadzone-scene-pif --output build/native-assets/dreadzone-scene-wad-audit.json
```

Only PSMT8 and PSMT4 base mips are exported. These PNGs are diagnostic textures,
not scene geometry. No exact pixel match was found for the two scene 0 images in
the currently extracted Moby-class and Dreadzone-mission PNG collections; this
does not rule out resized, cropped, or otherwise transformed variants.

## Correlating chunk resources with mission placements

The optional `--correlate-mission-root` audit compares up to 49 finite
`(x, y, z, 0)` rows after each direct chunk resource entry and aligned nested
candidate with positions in the extracted `mission.instances` files, using a
0.01-unit tolerance. This is coordinate evidence only; unmatched rows and
resource IDs remain provisional.

For resource ID 9387, the audit compares 3,286 rows from 68 chunk entries and
all 3,286 match one coordinate, `(207.998, 467.993, 126.891)`. That coordinate
matches 15 extracted Moby 9387 placement records across mission variants. This
confirms the class-and-position relationship for this resource pattern, but it
does not establish that other IDs share the same meaning or reveal any
orientation. No new placement is emitted: these records already exist in the
mission data.

The same check finds a second relationship among nested candidates: ID 9277
has 121 records with 5,841 comparable rows. Of these, 4,656 match 45 Moby 9277
placement records across mission variants at three coordinates:
`(186.644, 468.004, 132.906)`, `(192.764, 482.803, 132.906)`, and
`(207.416, 488.882, 132.906)`. This supports a class-and-position link for
these ID 9277 records. It does not recover rotations or add geometry; the
matching objects are already represented in mission data.

Run the coordinate cross-check with:

```powershell
python tools/audit_dreadzone_scene_wad.py --correlate-mission-root private/overlays/levels/campaign/1_dreadzone_station/missions --output build/native-assets/dreadzone-scene-wad-correlated-audit.json
```

## Matching scene PIF textures against extracted Moby textures

The audit exports 105 supported PIF base mips. An exact RGBA pixel comparison
against 1,421 Moby and TIE PNGs within the Dreadzone level extraction found 27
PIF matches. Expanding the comparison to 12,336 Moby and TIE PNGs under
`private/overlays` found 30 unique matched PIFs. The matches include one PIF
for class 10, two for class 8297, thirteen for class 9387, three for class 9419,
and four for class 9575. One of the class 9387 texture matches is also present
in the global export for class 5745. Seven more PIFs match a texture shared by
classes 8554, 9870, 9872, 9873, and 9875, so those images alone cannot
distinguish the five classes.

Classes 5745 and 9419 have no placements in the extracted Dreadzone
`mission.instances` files; their global texture matches alone do not show that
they are objects in this level. None of the PIF matches corresponds to the 17
mission Moby classes with no drawable mesh in the current review scene. Exact
pixel identity does not recover geometry, placements, rotations, or visibility.

Recreate the PIF images with `--extract-pif-dir` and compare them with exported
class textures before assigning any new resource semantics.

## Scene 20 resource 9228 coordinate stream

The 28 direct resource entries with candidate ID 9228 in scene record 20,
chunks 5–32, contain 1,333 bounded 16-byte rows when each payload is capped at
the next direct resource descriptor. Chunks 5–31 each contribute 49 rows and
chunk 32 contributes 10. Each chunk's final row exactly repeats the first row
of the next chunk, producing a continuous ordered sequence. Every row decodes
as four little-endian floats, with the fourth component zero. The first three
components range approximately over X 10.127–11.188, Y 9.253–11.019, and Z
1.226–2.691.

The 9228 coordinates vary along all three axes and have many short steps; they
do not form a simple monotonic path. Comparing these rows with mission
placements finds no coordinate matches. Direct candidate IDs 9207 and 8484 in
the same chunks also have no direct coordinate matches to their similarly
numbered Moby placements. These checks do not establish whether 9228 holds
vertices, another local coordinate buffer, or unrelated data; no associated
index stream, packet header, transform, or triangle topology has been verified.
The sequence is not rendered or assigned to an object.

Candidate IDs 9227 and 9314 also recur as nested candidates in these chunks.
The bounded 9227 region is a repeated point block in each chunk (truncated to
10 rows in chunk 32). The 9314 region begins with a second repeated point
block and has varied trailing rows in some chunks. These regularities link the
buffers structurally to the same repeated chunk set, but do not identify their
object or role.

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

## Scene 20 direct resource 9207 row pattern

The direct resource descriptor with candidate ID 9207 appears in each of
chunks 5–32. Bounded by the next direct or nested candidate descriptor, the
payload regions contain 1,333 aligned 16-byte rows: 49 rows in chunks 5–31 and
10 rows in chunk 32. Every row across all 28 chunks is byte-identical and
decodes as `(11.305635, 11.062163, 1.188676, 0)`. None matches a mission Moby
placement coordinate. This repeated point does not establish that the region
is geometry or that the descriptor identifies class 9207; the resource and
coordinate namespaces remain provisional.

The WAD audit JSON now includes a bounded `payload_layout` summary for each
direct resource entry. It reports the next descriptor offset, aligned row
count, unique-row count, identical leading run, and first row bytes without
interpreting row semantics.

