# Building DeltaWorks

DeltaWorks can be built with CMake or with Projector. Both compile the same sources. Visual Studio 2022 or newer (MSVC) is required: the code relies on MSVC extensions, so GCC and Clang are not supported yet.

## Third-party libraries

zlib, libpng, libjpeg and bzip2 come from [vcpkg](https://vcpkg.io) by default. They are listed in `vcpkg.json`, which pins a vcpkg baseline, and are installed automatically on the first build. vcpkg ships with Visual Studio 2022 and newer (the "vcpkg package manager" component). It can also be installed standalone, with `VCPKG_ROOT` pointing at it.

Static triplets (`x64-windows-static`, `x86-windows-static`) are used to match DeltaWorks' static runtime (`/MT`, `/MTd`).

Without vcpkg, CMake can fall back to the copies bundled in `Core/third_party` (current upstream releases, see `Core/third_party/VERSIONS.md`).

## CMake

Requires CMake 3.21 or newer. Run these from a Visual Studio developer prompt, or set `VCPKG_ROOT`:

```
cmake --preset vs-x64
cmake --build --preset vs-x64-release
```

| Preset | Platform | Third-party libraries |
|---|---|---|
| `vs-x64` | x64 | vcpkg |
| `vs-win32` | Win32 | vcpkg |
| `vs-x64-bundled` | x64 | bundled copies, no vcpkg needed |
| `ninja-msvc` | x64, Ninja | vcpkg |

Targets: `DeltaWorks::Core`, `DeltaWorks::Engine`, `DeltaWorks::Terrain` (static libraries).

| Option | Default | Meaning |
|---|---|---|
| `DELTAWORKS_THIRD_PARTY` | `AUTO` | `PACKAGES`: require `find_package()` results (vcpkg or other). `BUNDLED`: use `Core/third_party`. `AUTO`: packages if all four are found, otherwise bundled |
| `DELTAWORKS_BUILD_ENGINE` | `ON` | Build DeltaWorksEngine |
| `DELTAWORKS_BUILD_TERRAIN` | `ON` | Build DeltaWorksTerrain (needs Engine) |
| `DELTAWORKS_STATIC_RUNTIME` | `ON` | `/MT`, `/MTd`, like the Projector build |
| `DELTAWORKS_CXX_STANDARD` | `20` | C++ standard |

Using DeltaWorks from another CMake project:

```cmake
add_subdirectory(path/to/DeltaWorks DeltaWorks)   # or FetchContent
target_link_libraries(MyApp PRIVATE DeltaWorks::Engine)
```

If the parent project uses vcpkg in manifest mode, add `zlib`, `libpng`, `libjpeg-turbo` and `bzip2` to its own `vcpkg.json`.

Visual Studio generators also get Projector's `OptimizedDebug` configuration (optimized with symbols, defines `_OPTIMIZED_DEBUG`).

## Projector

The `.project` files (`Core/DeltaWorksCore.project`, `Engine/DeltaWorksEngine.project`, `Terrain/DeltaWorksTerrain.project`) need [Projector](https://github.com/IronFox/Projector) 1.5 or newer. `DeltaWorksCore.project` declares `<vcpkg />`, so the generated Visual Studio projects install and link the vcpkg packages. Projects that reference DeltaWorks, such as the Hexaverse clients, inherit this automatically. The Projector build does not use the bundled copies.

The CMake lists mirror the `<source>` / `<exclude>` rules of the `.project` files. When you change one, change the other (`Core/CMakeLists.txt`, `Engine/CMakeLists.txt`, `Terrain/CMakeLists.txt`).
