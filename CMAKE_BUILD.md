# CMake Build Guide for GenericUpdater

This project now supports building with CMake in addition to Visual Studio.

## Prerequisites

- CMake 3.15 or higher
- A C++ compiler supporting C++23 (MSVC, GCC, or Clang)
- Windows SDK (for Windows builds)

## Building with CMake

### Using Command Line

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Generate build files:
```bash
cmake ..
```

For a specific generator (e.g., Visual Studio):
```bash
cmake -G "Visual Studio 17 2022" ..
```

3. Build the project:
```bash
cmake --build . --config Release
```

Or for Debug:
```bash
cmake --build . --config Debug
```

### Using CMake GUI

1. Open CMake GUI
2. Set "Where is the source code" to the project root directory
3. Set "Where to build the binaries" to a build subdirectory
4. Click "Configure" and select your generator
5. Click "Generate"
6. Click "Open Project" or build from command line

## Project Structure

- **genUp4win**: Shared library (DLL) providing update checking functionality
- **DemoApp**: Windows application demonstrating the use of genUp4win library

## Installing

After building, you can install the binaries and headers:

```bash
cmake --install . --prefix /path/to/install
```

## Build Configurations

CMake supports the following configurations:
- **Debug**: Includes debug symbols, no optimizations
- **Release**: Optimized build without debug symbols

## Notes

- The project uses precompiled headers for faster compilation
- Resource files (.rc) are automatically included on Windows builds
- The genUp4win library exports symbols using `GENUP4WIN_EXPORTS` definition
