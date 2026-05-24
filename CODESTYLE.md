# Code Style

## Naming
- Types (classes, structs, enums): `PascalCase`
- Functions and methods: `PascalCase`
- Variables, parameters, fields: `camelCase`
- Constants (local, member, static, global): `UPPER_CASE`

## Files
- Headers: `.h`, sources: `.cpp`
- Header guard: `#pragma once`
- One public type per file pair; small helpers may share a file with their owner

## Formatting
- Braces: Allman (opening brace on its own line)
- Indentation: tabs
- Pointer/reference binds to type: `int* ptr`, `int& ref`
- `const` on the left: `const int`

## Includes
Order (each group sorted alphabetically):
1. Paired header first (in `Foo.cpp`, include `Foo.h` before anything else)
2. Project headers
3. Third-party libraries (SFML, nlohmann/json)
4. Standard library

## Constructors
- Prefer member initializer lists; use `this->` only when necessary

## auto
- Use for unnameable types (lambdas) or long/verbose types (~12+ chars)
- Avoid when an explicit type aids readability

## Namespaces
- No project-wide namespace (UI system may be namespaced if it grows)

## Other
- No preprocessor macros (use `constexpr`, `enum class`, templates instead)