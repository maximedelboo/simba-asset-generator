# Performance analysis (cache 2639)

This is the measured performance record for the pure-Simba generator on macOS,
using the unpacked OpenRS2 cache revision 2639. Timings are wall-clock values;
they are useful for ranking work, not as cross-machine promises.

## How to iterate

Use `tests/profile_map_shard.simba` for map-image changes. It is read-only by
default and measures one 256-region shard without rebuilding `out/`:

```text
Simba --root=/path/to/simba-asset-generator --cache=/path/to/cache \
  --shard=0 --run tests/profile_map_shard.simba
```

Compare the same shard before and after a change, then check a geographically
separate shard (currently 0 and 6). `--save=/tmp/name.bin` writes the resource
bundle as an optional parity artifact. A full generation remains the final
manifest and cross-platform check, not the inner-loop benchmark.

## Baseline

The complete mapdata baseline took 719.7 s:

| family | time |
| --- | ---: |
| map images | 430.8 s |
| collision | 98.0 s |
| heightmap | 184.8 s |
| packing and manifest | ~6 s |

The non-map dumpers were measured independently:

| dumper | time |
| --- | ---: |
| static | <1 s |
| gear | 5.2 s |
| item definitions | 10.0 s |
| item finder | 64.7 s |
| objects | 109.4 s |
| NPCs | 60.9 s |

That puts mapdata at roughly three quarters of the local full generation.

## Implemented wins

### Keep a real map working set

The previous 24-region LRU cache caused 8,209 region loads for 2,933 regions
in a full map-image run. The renderer walks raster-order columns, so it must
retain the preceding columns rather than only the current 3x3 neighbourhood.
`MAPIMAGE_REGION_CACHE` is now 512.

Two representative 256-region shards measured as follows:

| shard | cache 24 | cache 512 | change |
| --- | ---: | ---: | ---: |
| 0 | 38.4 s, 721 misses | 29.4 s, 304 misses | -23% |
| 6 | 41.3 s, 742 misses | 32.3 s, 392 misses | -22% |

The two cache settings produced byte-identical resource bundles for both
shards. SHA-256 values were `fc3b271c…aa91c8b7` for shard 0 and
`887158de…5447d411` for shard 6 under both settings. Extrapolating the sampled
23% improvement to the 416.9 s map-image build suggests about 95 s saved per
full generation. That extrapolation is deliberately labelled as an estimate;
the next release run is the end-to-end confirmation.

### Native map-composite plugin

`plugins/mapnative/` contains the bundled C plugin which replaces precisely one hot loop:
the terrain composite that selects an already-built plane buffer and copies its
non-zero ARGB pixels into the output BGRA image. It does not decode cache data,
build terrain planes, draw objects/icons or encode bundles. The generator loads
the platform-appropriate bundled binary by default.

The native loop is genuinely an order-of-magnitude faster and its output was
proven against full resource bundles, not sampled pixels:

| shard | pure total | native total | pure composite | native composite | result |
| --- | ---: | ---: | ---: | ---: | --- |
| 0 | 24.448 s | 19.808 s | 6.848 s | 0.128 s | byte-identical, SHA-256 `fc3b271c…aa91c8b7` |
| 6 | 30.240 s | 25.472 s | 6.816 s | 0.032 s | byte-identical, SHA-256 `887158de…5447d411` |

That is 53x–213x for the extracted composite stage, but 16%–19% per complete
shard. It is direct evidence that a custom native kernel works and is safe at
this boundary; it is equally direct evidence that this single kernel cannot
make the whole mapdata dump 10x faster. The remaining cost is region/cache
decode, plane construction, object/icon drawing, collision and heightmap.
The required macOS, Linux and Windows binaries ship under
`plugins/mapnative/`; a missing or incompatible package is therefore a clear
startup error rather than a silent pure-Simba fallback.

The normal `generator.simba` entry point, with its packaged arm64 binary and
cache 2639, completed a full 24-file verified generation in 455.3 s (7:35).
Its mapdata phase was 333.1 s: 144.8 s map images, 64.7 s collision and 118.6
s heightmaps. This is the production-path measurement, not a spike entry point.

### Memoise NPC model statistics, not NPC entities

`TModelStats.OfModel` is pure for a model archive and the immutable texture
table. The NPC helper now memoises that lower-level result by model id, while
still executing the entity-level rule literally: first model supplies height,
and each model palette is appended in order with duplicates preserved.

On all 16,292 NPCs, per-NPC decode/stats/JSON fell from 47.0 s to 13.3 s; the
whole NPC dumper fell from 53.4 s to 19.9 s. The resulting
`map/zips/npcs_helper.zip` is byte-identical to the uncached baseline (SHA-256
`4f42ee5f…b8770926`).
`tests/npc_model_memo.simba` additionally compared cached and uncached JSON for
all 16,292 NPC definitions.

### Preallocate object tile indices

Object-region projection builds two compact arrays for each populated tile:
the tile position and five plane-boundary offsets. It previously appended every
one dynamically, repeatedly copying progressively larger arrays. The dumper
now counts populated tiles, allocates both final arrays once, and fills them in
the same ascending order.

On the complete 2,933-region object dump, region projection fell from 83.1 s
to 69.1 s and the full objects dumper from 107.9 s to 92.2 s (about 15%). The
pre- and post-change `map/objects.zip` files were byte-identical, SHA-256
`e1b73d96…c00e76f10`. This is a structural allocation reduction, not a change
to plane-major traversal: that traversal remains necessary for the observable
morph-category mutation order.

### Index item duplicates with native `Hash32`

The item finder previously searched all retained rows linearly for every
rendered `(name, raw CRC32)` pair, and repeated the same pattern for border
masks. It now uses a collision-safe, open-addressed string set. Simba's
compiled `Hash32` (xxHash32) chooses a slot; the full string comparison is kept
so a hash collision cannot alter the first-row-wins rule.

The complete itemfinder fell from 64.7 s to 59.2 s (about 8.5%). Before keeping
the change, `cmp` verified every emitted item artifact: `items.bin`,
`items-imgs.zip`, `items.zip`, and `data/id.txt`, `item.txt`, `hash.txt`. All
six were byte-identical. Their post-change SHA-256 values begin respectively
`01da774b`, `aadcc2b5`, `ee5328c5`, `9886c7d4`, `39b8f6b4`, and `d0008acb`.

## Measured hotspots still worth work

| hotspot | measured cost | next action |
| --- | ---: | --- |
| map terrain | plane build plus composite | native composite is proven exact and 53x–213x faster; next native spike should include plane construction, not another image primitive |
| object region projection | 69.1 s | its allocation-heavy tile index is fixed; measure cache decoding only before attempting a specialised reader |
| object chunk/JSON loop | 19.8 s | profile string allocation only after region work; zip output is only 0.3 s |
| item sprite rendering | 36.8 s | already a pure-Simba exact rasteriser; optimise only against a pixel oracle |
| item PNG/zlib | 6.3 s | already calls Simba's native ZLIB encoder; not a custom-code target |

## Simba-native audit

The generator already uses compiled Simba code for the expensive generic
operations:

- `TResourceWriter.AddImage` and `Save` for map and item bundles;
- `CompressBytes`/`CompressData` for ZLIB and GZip;
- `TImage.Fill`, `ReplaceColor`, `FindColor`, `DrawTPA` and PNG/image storage
  operations in item post-processing.

`TImage.DrawImage` is compiled but is not an interchangeable map primitive. It
uses alpha-zero transparency, clips per source pixel, and forces copied pixels
opaque. Map terrain instead uses fixed Jagex blend buffers with `integer 0`
meaning “do not paint”; icons use a different `ARGB integer 0` rule and cache
sprite offsets. Replacing those loops with `DrawImage` would change pixels or
first require conversion/caching work that dominates the tiny icon phase.

Simba exposes no BZip2 compression algorithm. A prior native BZip2 plugin was
byte-identical but saved about four seconds on the real dump and cannot have a
runtime fallback: a missing `{$loadlib}` is a compile error. It remains a poor
default dependency. The earlier rasteriser-plugin proposal has the same
distribution problem; the measured pure-Simba rasteriser is already in the
right performance class.

## Rejected spike: minimal object-region reader

An experimental reader consumed the same terrain/location bytes as `TRegion`
but allocated only settings and locations. It produced the same complete
`objects.zip` byte-for-byte, yet measured 90.7 s for regions/JSON versus 87.4 s
for the retained two-pass-index version. Cache decompression and the remaining
stream work dominate enough that duplicating the mature region decoder is not a
win on this workload, so that spike was removed rather than adding a second
decoder to maintain.

## What not to optimise

- Map bundle writing is ~9–10 s per family and already native.
- Icons are ~13 s over all map images.
- Generic `DrawImage` substitutions are not pixel-safe for this pipeline.
- Entity-level model caching is not allowed: it would alter first-model and
  palette-concatenation semantics. The implemented cache is strictly per model.
