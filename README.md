# 2D Platformer

A 2D platformer built in C++23 with SFML 3 and a custom Entity-Component-System.

## Status

In active development.

## Tech

- C++23
- SFML 3
- Custom ECS (sparse-set)
- Data-driven design (JSON via nlohmann/json)
- CMake + vcpkg

## Build

Requires [vcpkg](https://github.com/microsoft/vcpkg) and CMake.

1. Set the vcpkg toolchain path in `CMakePresets.json` to your local vcpkg install.
2. Open the folder in a CMake-aware IDE (e.g. Visual Studio), or configure from the command line.
3. Build the `Platformer` target.

## License

[MIT](LICENSE)