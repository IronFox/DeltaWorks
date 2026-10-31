# DeltaWorks

DeltaWorks is a C++ framework of containers, math, I/O, networking, and an OpenGL/OpenAL engine layer. It is the foundation of the game [Hexaverse](https://github.com/IronFox/Hexaverse), and it is published mainly because Hexaverse depends on it.

## Should you use it?

Probably not for a new project. DeltaWorks grew over many years, starting before C++11, and much of it re-implements things the standard library and Boost now do well (and better tested):

| DeltaWorks | Today you would usually use |
|---|---|
| `Container::Vector`, `Array`, `GenericHashTable`, `Queue`, `BinaryHeap` | `std::vector`, `std::array`, `std::unordered_map`, `std::deque`, `std::priority_queue` |
| `StringType::Template`, `string/` tokenizers and converters | `std::string`, `std::string_view`, `std::format`, `std::from_chars` |
| `general/thread_system`, `parallel_for` | `std::thread`, `std::jthread`, `<execution>`, `<atomic>` |
| `general/timer`, `io/date` | `<chrono>` |
| `io/file_system`, `io/xml`, `io/json` | `std::filesystem`, pugixml, nlohmann/json |
| `net/tcp` | Boost.Asio / standalone Asio |
| `math/` vectors and matrices | GLM, Eigen |

Those parts stay because Hexaverse is built on them, and replacing them is a large refactor with little payoff for the game.

What may still be interesting:

- **Engine layer:** windowing and input (Win32, partly X11), an OpenGL renderer with extension handling, text output, a profiler, and OpenAL audio with an Ogg/WAV music player.
- **Geometry:** the CGS mesh/scene structure, BVH, Collada import, and the fractal terrain code in `Terrain/` (CPU kernels; a CUDA variant sits in `Engine/cuda` but is not built).
- **Math helpers:** generated fixed-size vector/matrix operations (`math/*.template.php` generate the `*_operations.h` headers), polynomials, simplex noise.

## Status

- **Windows + MSVC (Visual Studio 2022 or newer) only.** The code uses MSVC extensions (`__declspec(nothrow)`, `__fastcall`, ...). GCC/Clang do not compile it yet.
- C++20 (or `/std:c++latest`).
- No tests and no API stability guarantees. Interfaces change when Hexaverse needs them to.
- Documentation is mostly source comments (Doxygen configs: `Core/doxygen.cfg`, `Core/windoxy.cfg`). The RIFF-like container format in `Core/io/riff_handler.*` is described in [docs/riff.md](docs/riff.md).

## Layout

| Library | Folder | Contents |
|---|---|---|
| `DeltaWorksCore` | `Core/` | `container/`, `math/`, `geometry/`, `io/`, `string/`, `net/`, `image/` (PNG/JPEG/BMP/TGA), `general/` (threads, timers, system), `interface/` (serialization), `structure/` (events, observers) |
| `DeltaWorksEngine` | `Engine/` | `engine/` (display, input, renderer, console, profiler), `gl/`, `al/` (OpenAL), `X11/` |
| `DeltaWorksTerrain` | `Terrain/` | Fractal terrain generation |

Engine depends on Core; Terrain depends on both. All three are static libraries. Code includes headers relative to the library roots, for example `#include <container/vectorList.h>` or `#include <engine/display.h>`, and everything lives in `namespace DeltaWorks`.

## Building

Two ways, both compiling the same sources. See [BUILDING.md](BUILDING.md) for details.

**CMake** (recommended):

```
cmake --preset vs-x64
cmake --build --preset vs-x64-release
```

This uses [vcpkg](https://vcpkg.io) for zlib, libpng, libjpeg-turbo and bzip2 (listed in `vcpkg.json`). The `vs-x64-bundled` preset builds without vcpkg, using the copies bundled in `Core/third_party`.

**Projector:** the `.project` files work with [Projector](https://github.com/IronFox/Projector) 1.5 or newer, which generates Visual Studio solutions and also pulls the packages through vcpkg.

Using DeltaWorks from your own CMake project:

```cmake
add_subdirectory(path/to/DeltaWorks DeltaWorks)
target_link_libraries(MyApp PRIVATE DeltaWorks::Engine)   # or DeltaWorks::Core
```

The Engine also needs the OpenAL SDK and OpenGL at link time in the application.

## Third-party code

- zlib, libpng, libjpeg-turbo, bzip2: from vcpkg (their own licenses apply). Current upstream copies of zlib, libpng, IJG libjpeg and bzip2 are bundled in `Core/third_party` as a fallback, with their license files (see [Core/third_party/VERSIONS.md](Core/third_party/VERSIONS.md)). This software is based in part on the work of the Independent JPEG Group.
- open-simplex-noise (public domain), in `Core/third_party/open-simplex-noise`.
- SHA-2 implementation by Aaron D. Gifford (BSD-3-Clause), in `Core/io/cryptographic_hash.*`.
- OpenGL / OpenAL / Creative EFX headers in `Engine/gl` and `Engine/al`, under their respective licenses.

## License

MIT, see [LICENSE](LICENSE). The third-party code listed above keeps its own license.
