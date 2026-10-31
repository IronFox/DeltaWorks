# Bundled third-party libraries

These copies are only used when DeltaWorks is built with CMake and
`DELTAWORKS_THIRD_PARTY=BUNDLED` (or `AUTO` without vcpkg packages). The default
builds, CMake with vcpkg and Projector, get the libraries from vcpkg (see `vcpkg.json`).

Each folder contains the library sources of an upstream release, unmodified,
with its license file. Programs, tests, build scripts and documentation were left out.

| Folder | Library | Version | Upstream | License |
|---|---|---|---|---|
| `zlib/` | zlib | 1.3.2 | https://github.com/madler/zlib (tag `v1.3.2`) | zlib, `zlib/LICENSE` |
| `png/` | libpng | 1.6.59 | https://github.com/pnggroup/libpng (tag `v1.6.59`) | libpng, `png/LICENSE` |
| `jpeg/` | IJG libjpeg | 10 (25-Jan-2026) | https://ijg.org (mirrored as tag `jpeg-10` in libjpeg-turbo) | IJG, `jpeg/README` |
| `bz2/` | bzip2 | 1.0.8 | https://gitlab.com/bzip2/bzip2 (tag `bzip2-1.0.8`) | bzip2 (BSD-style), `bz2/LICENSE` |
| `open-simplex-noise/` | open-simplex-noise-in-c | v1.1 | https://github.com/smcameron/open-simplex-noise-in-c | Unlicense, `open-simplex-noise/LICENSE` |

Files that are not taken verbatim from those tags:

- `png/pnglibconf.h` is upstream's `scripts/pnglibconf.h.prebuilt`.
- `jpeg/jconfig.h` is upstream's `jconfig.vc`.
- `bz2/bz2.h` is a DeltaWorks forwarding header to `io/bz2.h`, not part of bzip2.

This software is based in part on the work of the Independent JPEG Group.

## Updating

Copy the library sources and headers of the new release over the folder, keep the
license file, regenerate `pnglibconf.h` / `jconfig.h` as above, and update this table
and the version line in `CMakeLists.txt`.
