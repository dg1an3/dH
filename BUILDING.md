# Building dH with CMake

This document provides detailed instructions for building the dH (Brimstone) radiotherapy treatment planning system using CMake.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start](#quick-start)
3. [Build Options](#build-options)
4. [Platform-Specific Instructions](#platform-specific-instructions)
5. [Troubleshooting](#troubleshooting)
6. [Migration from Visual Studio Solution Files](#migration-from-visual-studio-solution-files)

## Prerequisites

### Required

- **CMake** 3.16 or newer
- **C++17 compatible compiler**
  - Windows: Visual Studio 2019 or newer
  - Linux: GCC 9+ or Clang 10+
  - macOS: Xcode 11+ or Clang 10+

### Optional Dependencies

- **ITK (Insight Segmentation and Registration Toolkit)** - Required for:
  - Main Brimstone production system (RtModel, Graph, Brimstone)
  - Component libraries
  - Download and build from: https://itk.org/

- **MFC (Microsoft Foundation Classes)** - Required for:
  - Brimstone GUI application (Windows only)
  - Included with Visual Studio (select "MFC and ATL support" during installation)

## Quick Start

### Windows with Visual Studio

1. **Set ITK_DIR environment variable:**
   ```cmd
   set ITK_DIR=C:\path\to\ITK-build
   ```

2. **Run the Windows build script:**
   ```cmd
   build_windows.bat
   ```

3. **Build the project:**
   ```cmd
   cd build
   cmake --build . --config Release
   ```

### Linux / macOS

1. **Configure:**
   ```bash
   mkdir build && cd build
   cmake .. -DITK_DIR=/path/to/ITK-build -DBUILD_BRIMSTONE_APP=OFF
   ```

2. **Build:**
   ```bash
   cmake --build . --config Release
   ```

3. **Install (optional):**
   ```bash
   sudo cmake --install . --prefix /usr/local
   ```

## Build Options

### Main Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_BRIMSTONE_APP` | `ON` | Build the Brimstone GUI application (requires MFC on Windows, ITK) |
| `BUILD_FOUNDATION_LIBS` | `OFF` | Build foundation libraries (MTL, FTL) |
| `BUILD_COMPONENT_LIBS` | `OFF` | Build component libraries (GEOM_MODEL, OPTIMIZER_BASE, XMLLogging) |
| `BUILD_TESTS` | `OFF` | Build test applications |
| `BUILD_SHARED_LIBS` | `OFF` | Build shared libraries instead of static libraries |

### Example Configurations

**Minimal build (core libraries only, no GUI):**
```bash
cmake .. \
    -DBUILD_BRIMSTONE_APP=OFF \
    -DITK_DIR=/path/to/ITK-build
```

**Full build with all components:**
```bash
cmake .. \
    -DBUILD_BRIMSTONE_APP=ON \
    -DBUILD_FOUNDATION_LIBS=ON \
    -DBUILD_COMPONENT_LIBS=ON \
    -DBUILD_TESTS=ON \
    -DITK_DIR=/path/to/ITK-build
```

**Development build (libraries only for integration):**
```bash
cmake .. \
    -DBUILD_BRIMSTONE_APP=OFF \
    -DBUILD_FOUNDATION_LIBS=ON \
    -DBUILD_COMPONENT_LIBS=ON \
    -DITK_DIR=/path/to/ITK-build
```

## Platform-Specific Instructions

### Windows with Visual Studio

#### Prerequisites
1. Install Visual Studio 2019 or newer
2. During installation, select:
   - "Desktop development with C++"
   - "MFC and ATL support"
3. Build or install ITK

#### Building
```cmd
REM Set ITK directory
set ITK_DIR=C:\ITK-build

REM Configure with Visual Studio generator
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DITK_DIR=%ITK_DIR%

REM Build
cmake --build . --config Release

REM Or open in Visual Studio
start dH.sln
```

#### Generator Options
- Visual Studio 2022: `-G "Visual Studio 17 2022"`
- Visual Studio 2019: `-G "Visual Studio 16 2019"`
- NMake: `-G "NMake Makefiles"`

### Linux

#### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake git

# Build ITK (example)
git clone https://github.com/InsightSoftwareConsortium/ITK.git
cd ITK && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### Building
```bash
mkdir build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_BRIMSTONE_APP=OFF \
    -DITK_DIR=/path/to/ITK/build
make -j$(nproc)
sudo make install
```

### macOS

#### Prerequisites
```bash
# Install Xcode command line tools
xcode-select --install

# Install CMake via Homebrew
brew install cmake

# Build ITK (similar to Linux)
```

#### Building
```bash
mkdir build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_BRIMSTONE_APP=OFF \
    -DITK_DIR=/path/to/ITK/build
make -j$(sysctl -n hw.ncpu)
sudo make install
```

## Troubleshooting

### ITK Not Found

**Error:**
```
CMake Error: By not providing "FindITK.cmake" this project has asked CMake to find a package configuration file provided by "ITK"
```

**Solution:**
Set the `ITK_DIR` variable to point to your ITK build directory:
```bash
cmake .. -DITK_DIR=/path/to/ITK-build
```

### MFC Not Found (Windows)

**Error:**
```
Cannot find MFC libraries
```

**Solution:**
1. Open Visual Studio Installer
2. Modify your installation
3. Under "Individual Components", select "C++ MFC for latest build tools"
4. Install and retry

### Cannot Build on Linux/macOS

**Note:** The Brimstone GUI application requires MFC which is Windows-only.

**Solution:**
Build without the GUI application:
```bash
cmake .. -DBUILD_BRIMSTONE_APP=OFF -DITK_DIR=/path/to/ITK-build
```

### Build Artifacts in Repository

If you see unexpected build files committed:

**Solution:**
Check `.gitignore` includes:
```
build/
cmake-build-*/
CMakeCache.txt
CMakeFiles/
```

## Migration from Visual Studio Solution Files

### Current Status

The CMake build system is now the recommended method for building dH. The legacy Visual Studio solution files are maintained for backward compatibility but will be removed in a future release.

### Main Solution Mapping

| Legacy File | CMake Equivalent |
|-------------|------------------|
| `Brimstone_src.sln` | Root `CMakeLists.txt` with `BUILD_BRIMSTONE_APP=ON` |
| `RtModel.vcxproj` | `RtModel/CMakeLists.txt` |
| `Graph.vcxproj` | `Graph/CMakeLists.txt` |
| `Brimstone.vcxproj` | `Brimstone/CMakeLists.txt` |

### Transition Plan

1. **Current:** Both CMake and Visual Studio solutions supported
2. **Next Release:** CMake primary, Visual Studio solutions deprecated
3. **Future:** Visual Studio solutions removed

### Verifying CMake Build

Before removing Visual Studio solutions, verify:

1. CMake configuration succeeds
2. All projects build without errors
3. Unit tests pass (if applicable)
4. Application runs correctly

Run the validation script:
```bash
./validate_cmake.sh
```

## Project Structure

### Main Production System

When `BUILD_BRIMSTONE_APP=ON` (default):

```
dH/
├── RtModel/          # Core algorithm library (static)
├── Graph/            # Visualization library (static)
└── Brimstone/        # GUI application (executable)
```

### Build Outputs

```
build/
├── lib/              # Static/shared libraries
│   ├── RtModel.lib
│   └── Graph.lib
└── bin/              # Executables
    └── Brimstone.exe
```

## Advanced Topics

### Cross-Compiling

CMake supports cross-compilation. Set the toolchain file:
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake
```

### Custom Install Prefix

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/custom/install/path
cmake --install .
```

### Using as a Dependency

After installation, other CMake projects can find Brimstone:
```cmake
find_package(Brimstone REQUIRED)
target_link_libraries(MyApp PRIVATE Brimstone::RtModel)
```

## Getting Help

- **Documentation:** See README.md for project overview
- **Repository Structure:** See repository_structure.md
- **Issues:** Report build problems on the project issue tracker

## License

U.S. Patent 7,369,645

Copyright (c) 2007-2021, Derek G. Lane. All rights reserved.

See LICENSE file for full terms.
