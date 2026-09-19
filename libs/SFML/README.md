# SFML Setup

This project uses **SFML 3.1.0**. vcpkg's own `sfml` port has not been
updated past 3.0.2 yet, so SFML is built from source and vendored here
instead of pulled through vcpkg (nlohmann-json still comes from vcpkg as
usual).

## Build SFML 3.1.0

SFML 3.1.0 fetches its own dependencies (FreeType, HarfBuzz, SheenBidi, Ogg,
Vorbis, FLAC) via CMake's `FetchContent`, so building it standalone needs
nothing beyond CMake, Ninja (or another generator) and a C++ compiler --
no vcpkg involved for this step.

1. Download and extract the SFML 3.1.0 source archive:
   https://github.com/SFML/SFML/archive/refs/tags/3.1.0.zip
2. Configure, build and install **both** Debug and Release as shared
   libraries, with `CMAKE_INSTALL_PREFIX` pointing at this folder
   (`libs/SFML`). From the extracted source folder:

```bash
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON ^
    -DSFML_BUILD_NETWORK=OFF -DSFML_BUILD_TEST_SUITE=OFF -DSFML_BUILD_EXAMPLES=OFF -DSFML_BUILD_DOC=OFF ^
    -DCMAKE_INSTALL_PREFIX="<path to this repo>/libs/SFML"
cmake --build build-debug --config Debug
cmake --install build-debug --config Debug

cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ^
    -DSFML_BUILD_NETWORK=OFF -DSFML_BUILD_TEST_SUITE=OFF -DSFML_BUILD_EXAMPLES=OFF -DSFML_BUILD_DOC=OFF ^
    -DCMAKE_INSTALL_PREFIX="<path to this repo>/libs/SFML"
cmake --build build-release --config Release
cmake --install build-release --config Release
```

Run these from a Visual Studio developer command prompt (or after calling
`vcvarsall.bat`) so `cmake` picks up MSVC. Building from source, rather than
using the official prebuilt binaries, keeps SFML's toolset an exact match
for whichever MSVC compiles the game.

This installs into:

```text
libs/
└── SFML/
    ├── include/
    ├── lib/     <- sfml-*.lib and sfml-*-d.lib (Release and Debug)
    └── bin/     <- the matching runtime DLLs, both configurations
```

`CMakeLists.txt` already points at `include`/`lib`, and copies the right
configuration's DLLs from `bin/` next to the executable after every build, so
nothing here needs manual copying. `libs/SFML/` is git-ignored (this file
excepted) -- each clone builds it once.

The Network module is not used by the game.
