# Simba asset generator library

This directory is a vendored, standalone Simba library. It contains the full
OSRS cache reader, definition decoders, renderers and asset dumpers needed by
[`../generator.simba`](../generator.simba). It does not include or
reference WaspLib at runtime.

Run from the `asset-generator` directory:

```text
Simba --run generator.simba
```

assets go to `out/`. To use a cache already unpacked on disk:
The first run discovers the latest valid Old School live OpenRS2 cache and
downloads `disk.zip` into `cache/<revision>/`; all generated assets go to
`out/`. To use a cache already unpacked on disk:
assets go to `out/`. To use a cache already unpacked on disk:

```text
Simba --cache=/path/to/main_file_cache_directory --run generator.simba
```

`out/` and `cache/` are intentionally ignored by Git. A normal full run
recreates `out/` before dumping; `--only=static`, `--only=items`, and the other
dumper names are for quick development runs and preserve it.

NPC spawn coordinates are the one artifact not present in the game cache. The
generator retries the live source three times; if it remains unavailable, it
uses this generator's packaged static fallback and prints a clear warning.
