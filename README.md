# Simba Asset Generator

A self-contained OSRS asset generator written entirely in Simba. The repository
contains the cache reader, renderer, static inputs, and generator entrypoint;
it does not use Java, RuneLite, Python, or WaspLib at runtime.

Run it with no arguments to discover and download the latest valid Old School
OpenRS2 cache:

```text
Simba --run generator.simba
```

For a reproducible full dump, pin the input cache:

```text
Simba --revision=2625 --run generator.simba
```

`cache/` holds downloaded caches and `out/` holds the generated asset tree.
Both are ignored by Git. `--only=itemfinder` is useful during renderer work.

## What `static/` is

`static/` is checked-in **generator input**, not a comparison directory and not
previously generated output. It vendors the historical files that the generator
currently stages unchanged into `out/`. Their provenance is deliberately
mixed; “static” means *copied by the current flow*, not “impossible to derive
from the cache”.

- Eleven paths (fonts, UI images, layouts, consumables, monsters and weapons)
  have no known automated producer in this project history; they are retained
  curated inputs until their source is specified or a producer is implemented.
- `finders/spells.zip` is partly cache-derived (276 of 376 historical entries
  match cache sprites), but its name-to-sprite mapping is not recorded and the
  remaining entries are not explained by a cache revision. `prayer.zip` and
  `overheads.zip` are historical screen-capture/processed assets rather than
  direct cache extracts.
- `jsons/gear.json` *is* cache-derivable from item definitions. It remains
  copied only because that small Simba dumper has not yet been ported.

`static/map/npcs.zip` has a different role: it is a packaged fallback. The
generator normally creates NPC data from the cache plus live spawn coordinates;
spawn coordinates do not exist in the OSRS cache, so this one output cannot be
made from cache bytes alone.

## Cross-platform proof

[`cross-platform.yml`](.github/workflows/cross-platform.yml) executes a full
revision-2625 generation on official Ubuntu, macOS, and Windows GitHub-hosted
runners. Each job checks the 24-entry manifest and uploads the manifest plus
item data as an artifact. Before generating, each runner also executes the
pure-Simba latest-cache lookup used by argumentless runs.

The pure-Simba PNG encoder deliberately uses Simba's public zlib level. PNG
pixels are stable, but this does not promise byte-for-byte equivalence to a
Java encoder with a different compression level.
