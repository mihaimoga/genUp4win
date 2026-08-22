# Repository Guidelines

## Overview

This is a Windows C++23 solution for `genUp4win`, a shared updater library, and `DemoApp`, a Win32 application that demonstrates the library.

- `genUp4win/` contains the DLL target configuration. Its source, headers, and resource file are currently kept at the repository root.
- `DemoApp/` contains the demo application's source, headers, and resources.
- `CMakeLists.txt` configures both targets and sets `DemoApp` as the Visual Studio startup project.
- `*.vcxproj` files support opening and building the solution directly in Visual Studio.

## Build

Requirements: CMake 3.15+, a C++23-capable compiler, and the Windows SDK.

Configure and build from the repository root:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
```

Use `Release` instead of `Debug` for an optimized build. CMake target definitions enable Unicode and build `genUp4win` as a Windows DLL; `DemoApp` links against that target.

## Code Conventions

- Keep all production code compatible with C++23 and Windows APIs used by the existing code.
- Follow the formatting, naming, include ordering, and brace style in the file being changed.
- Preserve precompiled-header usage: source files that depend on it should include the corresponding `pch.h` first.
- Do not change warning levels, compiler options, linker subsystem settings, or preprocessor definitions unless the change requires it.
- Keep public library declarations in root-level headers synchronized with implementation changes in the root-level `.cpp` files.
- Preserve Unicode (`UNICODE` and `_UNICODE`) support and use the existing `CString`/Win32 conventions where applicable.

## CMake and Resources

- When adding or removing source, header, or `.rc` files, update the appropriate target list in `genUp4win/CMakeLists.txt` or `DemoApp/CMakeLists.txt`.
- Retain `target_precompile_headers` configuration when reorganizing target sources.
- Keep resource identifiers and generated resource headers consistent with their `.rc` files.

## Validation

- Build the affected configuration after code or build changes:

```powershell
cmake --build build --config Debug
```

- No automated test project is currently configured. For behavior changes, manually verify the affected updater or DemoApp workflow where practical.
- Do not commit generated build directories, binaries, or intermediate Visual Studio/CMake artifacts.
