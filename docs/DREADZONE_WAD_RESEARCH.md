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

| Candidate ID | Available rows | Occurrences | Unique rows | Identical leading rows |
| --- | ---: | ---: | ---: | ---: |
| 9227 | 117 | 24 | 21 | 49 |
| 9227 | 181 | 3 | 85 | 49 |
| 9227 | 541 | 1 | 523 | 10 |
| 9314 | 49 | 24 | 1 | 49 |
| 9314 | 113 | 3 | 65 | 49 |
| 9314 | 522 | 1 | 513 | 10 |

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
