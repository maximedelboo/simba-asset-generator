# Simba Asset Generator

A standalone OSRS asset generator written entirely in Simba. It reads an OSRS
cache, renders and extracts the available assets, and writes the results to
`out/`. It has no Java, RuneLite, Python, or WaspLib runtime dependencies.

## Usage

Run without arguments to look up, download, and process the latest valid Old
School cache from OpenRS2:

```text
Simba --run generator.simba
```

For a reproducible run with a fixed cache revision:

```text
Simba --revision=2625 --run generator.simba
```

An unpacked cache can also be used directly:

```text
Simba --cache=/path/to/cache --run generator.simba
```

`cache/` contains downloaded caches and `out/` contains generated assets. Both
directories are ignored by Git. A normal run clears `out/` and runs every
dumper. Use `--only=<dumper>` for targeted development, for example
`--only=itemfinder` or `--only=gear`.

## What it does

The generator:

1. Obtains a specified or current OSRS cache.
2. Reads cache definitions, sprites, models, and map data.
3. Generates asset files and `hashes.json` in `out/`.
4. Verifies that all 24 expected manifest paths are present.

The `gear` dumper generates `jsons/gear.json` from cache item definitions. For
cache 2625, its output is byte-identical to the previously downloaded asset.

## `static/`

`static/` contains generator input that is currently copied unchanged to
`out/`. It is not an output or comparison directory. It supplies 15 of the 24
manifest paths, including fonts, UI images, layouts, and some JSON and ZIP
files.

## Current limitations

- Twelve static paths do not yet have a generation step in this repository.
- `finders/spells.zip` uses cache sprites in part, but the complete
  name-to-sprite mapping is not recorded.
- `finders/prayer.zip` and `finders/overheads.zip` are processed screen/sprite
  assets rather than direct cache extracts.
- NPC spawn coordinates are not stored in the OSRS cache. The generator
  normally obtains them externally; `static/map/npcs.zip` is the fallback when
  that source is unavailable.
- PNG pixels are stable, but PNG files are not necessarily byte-identical to
  files made by an encoder using a different zlib configuration.

## Cross-platform

[`cross-platform.yml`](.github/workflows/cross-platform.yml) runs a complete
revision-2625 generation on GitHub-hosted Ubuntu, macOS, and Windows runners.
Each job verifies the 24-entry manifest and uploads the manifest and item data
as artifacts.
