# Building dH with CMake

This document provides detailed instructions for building the dH (Brimstone) radiotherapy treatment planning system using CMake.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start](#quick-start)
3. [Build Options](#build-options)
4. [Platform-Specific Instructions](#platform-specific-instructions)
5. [ITK Dependency Management](#itk-dependency-management)
6. [Troubleshooting](#troubleshooting)
7. [Migration from Visual Studio Solution Files](#migration-from-visual-studio-solution-files)

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
  - **Installation options:**
    - **Option A (Manual Build):** Download and build from https://itk.org/
    - **Option B (vcpkg - Recommended for Windows):** Install pre-built binaries via vcpkg package manager
    - **Option C (System Package):** Linux/macOS package managers (may have older versions)
  - See [ITK Dependency Management](#itk-dependency-management) for detailed instructions

- **MFC (Microsoft Foundation Classes)** - Required for:
  - Brimstone GUI application (Windows only)
  - Included with Visual Studio (select "MFC and ATL support" during installation)

## Quick Start

### Windows with Visual Studio (Manual ITK Build)

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

### Windows with vcpkg (Recommended - Faster Setup)

1. **Install vcpkg (one-time setup):**
   ```cmd
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   bootstrap-vcpkg.bat
   ```

2. **Install ITK:**
   ```cmd
   vcpkg install itk:x64-windows
   ```

3. **Build the project:**
   ```cmd
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
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
3. Install ITK (choose one method below)

#### Method 1: Using vcpkg (Recommended)

**Advantages:** Pre-built binaries, faster setup, no manual build required.

1. **Install vcpkg:**
   ```cmd
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   bootstrap-vcpkg.bat
   ```

2. **Install ITK:**
   ```cmd
   vcpkg install itk:x64-windows
   ```
   Note: This may take 20-30 minutes on first install but provides pre-built binaries.

3. **Configure and build:**
   ```cmd
   mkdir build && cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 ^
       -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
   cmake --build . --config Release
   ```

4. **Or open in Visual Studio:**
   ```cmd
   start dH.sln
   ```

#### Method 2: Manual ITK Build

**Advantages:** Full control over ITK configuration and build options.

1. **Build ITK from source:**
   ```cmd
   git clone https://github.com/InsightSoftwareConsortium/ITK.git C:\ITK-src
   mkdir C:\ITK-build && cd C:\ITK-build
   cmake ..\ITK-src -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```
   Note: This can take 1-2 hours. ITK is a large library (~1-2 GB built).

2. **Configure and build dH:**
   ```cmd
   set ITK_DIR=C:\ITK-build
   mkdir build && cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 -DITK_DIR=%ITK_DIR%
   cmake --build . --config Release
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

## ITK Dependency Management

The Insight Toolkit (ITK) is a large dependency (~1-2 GB built) required for medical image processing in dH. This section compares different methods for obtaining ITK.

### Comparison of ITK Installation Methods

| Method | Setup Time | Build Time | Disk Space | Best For | CMake Configuration |
|--------|------------|------------|------------|----------|---------------------|
| **vcpkg (Recommended)** | 5 min | 20-30 min (first time) | ~2 GB | Windows developers, quick setup | `-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake` |
| **Manual Build** | 10 min | 1-2 hours | ~3 GB | Custom ITK configuration, cross-platform | `-DITK_DIR=/path/to/ITK-build` |
| **System Package** | 2 min | None (pre-built) | ~500 MB | Linux/macOS, may have older version | Auto-detected or `-DITK_DIR=/usr/lib/cmake/ITK` |
| **FetchContent** | 0 min | 1-2 hours (every clean build) | ~3 GB | CI/CD, reproducible builds | Automatic (requires CMake modification) |

### Option 1: vcpkg (Recommended for Windows)

**Pros:**
- Pre-built binaries save 1-2 hours of build time
- Handles dependencies automatically
- Easy updates: `vcpkg upgrade`
- Integrates seamlessly with Visual Studio
- Works well with multiple projects

**Cons:**
- Requires ~2 GB disk space
- ITK version controlled by vcpkg (may lag behind latest)
- Additional tool to install

**Setup:**
```cmd
REM One-time vcpkg installation
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
bootstrap-vcpkg.bat

REM Install ITK
vcpkg install itk:x64-windows

REM Configure dH
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### Option 2: Manual Build from Source

**Pros:**
- Full control over ITK modules and build options
- Can use latest ITK version from git
- Can optimize for specific hardware
- Works on all platforms

**Cons:**
- Long initial build time (1-2 hours)
- Requires manual dependency management
- More complex setup

**Setup:**
```bash
# Clone ITK
git clone https://github.com/InsightSoftwareConsortium/ITK.git
cd ITK && mkdir build && cd build

# Configure (minimal build - only needed modules)
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DModule_ITKCommon=ON \
    -DModule_ITKIOImageBase=ON \
    -DModule_ITKIOXML=ON

# Build (this takes time)
cmake --build . --config Release -j

# Use in dH
cmake /path/to/dH -DITK_DIR=/path/to/ITK/build
```

### Option 3: System Package Manager (Linux/macOS)

**Pros:**
- Fastest setup (pre-built)
- Managed by system package manager
- Smallest disk footprint

**Cons:**
- Often older ITK versions
- Limited module selection
- Platform-specific

**Setup:**
```bash
# Ubuntu/Debian
sudo apt-get install libinsighttoolkit5-dev

# macOS (Homebrew)
brew install itk

# Configure dH (usually auto-detected)
cmake .. -DBUILD_BRIMSTONE_APP=OFF
```

### Option 4: FetchContent (Advanced - CI/CD)

**Pros:**
- Fully automatic, no manual steps
- Reproducible builds with pinned versions
- Good for continuous integration

**Cons:**
- Very long configure time on clean builds
- Rebuilds ITK if build directory is deleted
- Requires modifying CMakeLists.txt

**Setup:**
This requires modifying the root `CMakeLists.txt`. Contact maintainers if you need this option for CI/CD pipelines.

### Recommended Workflow by Platform

- **Windows Development:** Use vcpkg (Option 1)
- **Linux Development:** System package if available, otherwise manual build (Option 3 or 2)
- **macOS Development:** Homebrew if available, otherwise manual build (Option 3 or 2)
- **CI/CD Pipelines:** FetchContent or cached manual build (Option 4 or 2)
- **Maximum Control:** Manual build from source (Option 2)

## Troubleshooting

### ITK Not Found

**Error:**
```
CMake Error: By not providing "FindITK.cmake" this project has asked CMake to find a package configuration file provided by "ITK"
```

**Solutions:**

**If using vcpkg:**
```cmd
REM Ensure you're using the vcpkg toolchain file
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

REM Verify ITK is installed
vcpkg list | findstr itk
```

**If using manual build:**
```bash
# Set the ITK_DIR variable to point to your ITK build directory
cmake .. -DITK_DIR=/path/to/ITK-build
```

### vcpkg Issues

**Error: vcpkg not found or ITK not installed**

**Solution:**
```cmd
REM Check vcpkg installation
where vcpkg

REM If not found, ensure vcpkg is bootstrapped
cd C:\vcpkg
bootstrap-vcpkg.bat

REM Install ITK if missing
vcpkg install itk:x64-windows

REM List installed packages
vcpkg list
```

**Error: Wrong architecture (x86 vs x64)**

**Solution:**
Ensure you install the correct architecture that matches your build:
```cmd
REM For 64-bit builds (most common)
vcpkg install itk:x64-windows

REM For 32-bit builds
vcpkg install itk:x86-windows
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
