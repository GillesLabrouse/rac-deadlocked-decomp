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
