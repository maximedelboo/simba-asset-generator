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

## Cross-platform proof

[`cross-platform.yml`](.github/workflows/cross-platform.yml) executes a full
revision-2625 generation on official Ubuntu, macOS, and Windows GitHub-hosted
runners. Each job checks the 24-entry manifest and uploads the manifest plus
item data as an artifact. Before generating, each runner also executes the
pure-Simba latest-cache lookup used by argumentless runs.

The pure-Simba PNG encoder deliberately uses Simba's public zlib level. PNG
pixels are stable, but this does not promise byte-for-byte equivalence to a
Java encoder with a different compression level.
