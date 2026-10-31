# Building DeltaWorks

DeltaWorks can be built two ways. Both compile the same sources.

## CMake

Requires CMake 3.21+ and Visual Studio 2022 (MSVC). Other compilers are not supported yet: the code relies on MSVC extensions.

```
cmake --preset vs-x64
cmake --build --preset vs-x64-release
```

Targets: `DeltaWorks::Core`, `DeltaWorks::Engine`, `DeltaWorks::Terrain` (static libraries).
Options: `DELTAWORKS_BUILD_ENGINE`, `DELTAWORKS_BUILD_TERRAIN`, `DELTAWORKS_STATIC_RUNTIME` (default ON, `/MT` like Projector), `DELTAWORKS_CXX_STANDARD` (default 20).

Using it from another CMake project:

```cmake
add_subdirectory(path/to/DeltaWorks DeltaWorks)   # or FetchContent
target_link_libraries(MyApp PRIVATE DeltaWorks::Engine)
```

Visual Studio generators also get Projector's `OptimizedDebug` configuration (optimized with symbols, defines `_OPTIMIZED_DEBUG`).

## Projector

The `.project` files (`Core/DeltaWorksCore.project`, `Engine/DeltaWorksEngine.project`, `Terrain/DeltaWorksTerrain.project`) still work unchanged with [Projector](https://github.com/IronFox/Projector).

The CMake lists mirror the `<source>` / `<exclude>` rules of the `.project` files. When you change one, change the other (`Core/CMakeLists.txt`, `Engine/CMakeLists.txt`, `Terrain/CMakeLists.txt`).
