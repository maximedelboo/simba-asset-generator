# Map-native compositor

The map renderer calls this small C plugin by default. It performs the exact
terrain-plane composite used by `TMapImage.DrawTerrain`: select the effective
plane for a tile, copy its non-zero ARGB pixels to the BGRA chunk, and preserve
the bridge and upper-plane ordering. Cache decoding, terrain-plane creation,
objects, icons and PNG encoding remain Simba code.

The plugin is packaged with the repository. Running `generator.simba` needs no
plugin installation or command-line option: Simba loads the binary relative to
the map renderer source.

## Bundled binaries

| Runtime | Binary |
| --- | --- |
| macOS x86_64 | `mapnative.dylib` |
| macOS arm64 | `mapnative64.dylib.aarch64` |
| Linux x86_64 | `mapnative.so` |
| Linux arm64 | `mapnative64.so.aarch64` |
| Windows x86_64 | `mapnative.dll` |

These are the architectures distributed by the generator's supported Simba
launchers. The GitHub workflow loads the packaged binary before its full Linux,
macOS and Windows generation jobs.

## Rebuilding

macOS builds can be made directly with the system compiler:

```text
make -C plugins/mapnative macos-x64 macos-arm64
```

The release binaries are deliberately committed. `mapnative.c` has no external
dependencies; Linux and Windows binaries can be rebuilt on their native
runners with the matching `make` target, or with a cross compiler.

## Correctness boundary

The FFI takes only four `Int32` plane buffers, region settings and the existing
Simba image buffer. It allocates nothing and has no cache state. Representative
256-region pure-Simba and plugin bundles were compared byte-for-byte (including
their serialized resource bundle); the normal full generator then verifies its
24-file output manifest.
