# CMake Build System

This document describes the CMake-based build system that has replaced the legacy Visual Studio 6.0 project files (.dsp/.dsw).

## Overview

The project has been converted from Visual Studio 6.0 project files to a modern CMake build system. This provides:
- Cross-platform build capability (primarily Windows with MFC)
- Modern IDE support (Visual Studio 2017+, CLion, etc.)
- Better dependency management
- Cleaner project structure

## Requirements

- CMake 3.15 or higher
- Visual Studio 2017 or later (for MFC support)
- Windows SDK

## Build Instructions

### Using CMake GUI

1. Open CMake GUI
2. Set source directory to the root of this repository
3. Set build directory to `build/` (or any preferred location)
4. Click "Configure" and select your Visual Studio version
5. Click "Generate"
6. Open the generated `.sln` file in Visual Studio

### Using Command Line

```bash
# Create build directory
mkdir build
cd build

# Configure with Visual Studio generator
cmake .. -G "Visual Studio 16 2019" -A Win32

# Build
cmake --build . --config Release

# Or open the generated solution file
start dH_Project.sln
```

## Project Structure

The CMake build system follows this hierarchy:

```
dH/
├── CMakeLists.txt (root)
├── MTL/
│   └── CMakeLists.txt
├── XMLLogging/
│   └── CMakeLists.txt
├── MODEL_BASE/
│   └── CMakeLists.txt
├── GEOM_BASE/
│   └── CMakeLists.txt
├── GUI_BASE/
│   └── CMakeLists.txt
├── OPTIMIZER_BASE/
│   └── CMakeLists.txt
├── GEOM_MODEL/
│   └── CMakeLists.txt
├── GEOM_VIEW/
│   └── CMakeLists.txt
├── RT_MODEL/
│   └── CMakeLists.txt
└── PenBeamEdit/
    └── CMakeLists.txt
```

## Library Dependencies

The dependency graph is as follows:

```
PenBeamEdit (executable)
├── RT_MODEL
│   ├── GEOM_MODEL
│   │   ├── MTL
│   │   └── XMLLogging
│   ├── GEOM_VIEW
│   │   ├── GEOM_MODEL
│   │   ├── MTL
│   │   ├── XMLLogging
│   │   └── OpenGL32
│   ├── OPTIMIZER_BASE
│   │   └── GEOM_MODEL
│   ├── MTL
│   └── XMLLogging
├── GEOM_VIEW
├── GEOM_MODEL
├── GEOM_BASE
│   └── MODEL_BASE
├── GUI_BASE
│   ├── GEOM_BASE
│   └── MODEL_BASE
├── MODEL_BASE
├── OPTIMIZER_BASE
├── XMLLogging
│   └── MTL
└── MTL
```

## Configuration Options

### MFC Support
All projects use MFC in shared DLL mode (`CMAKE_MFC_FLAG = 2`).

### Build Configurations
- **Debug**: Full debug info, no optimizations
- **Release**: Full optimizations, no debug info

### Preprocessor Definitions

**Debug builds:**
- `_DEBUG`
- `WIN32`
- `_WINDOWS`
- `_AFXDLL`
- `_MBCS`
- `USE_XMLLOGGING`
- `XMLLOGGING_ON`

**Release builds:**
- `NDEBUG`
- `WIN32`
- `_WINDOWS`
- `_AFXDLL`
- `_MBCS`
- `USE_XMLLOGGING`

## Targets

### Libraries (Static)
- **MTL** - Mathematical Template Library (interface/header-only)
- **XMLLogging** - XML logging framework
- **MODEL_BASE** - Base model classes
- **GEOM_BASE** - Geometric primitives
- **GUI_BASE** - GUI base components
- **OPTIMIZER_BASE** - Optimization algorithms
- **GEOM_MODEL** - Geometric modeling
- **GEOM_VIEW** - Geometric visualization
- **RT_MODEL** - Radiotherapy model

### Executables
- **PenBeamEdit** - Pencil beam editor application

## Output Directories

Built artifacts are placed in:
- Libraries: `build/lib/`
- Executables: `build/bin/`

## Precompiled Headers

Precompiled header support is commented out in each CMakeLists.txt but can be enabled by uncommenting the `target_precompile_headers()` lines.

## Notes

1. **MFC Requirement**: This project requires MFC (Microsoft Foundation Classes), which is Windows-only
2. **Intel Performance Primitives**: Optional IPP support can be enabled via `USE_IPP` definition
3. **OpenGL**: GEOM_VIEW requires OpenGL for 3D rendering
4. **MSChart ActiveX**: PenBeamEdit may require MSChart ActiveX control to be registered

## Migration from VS6 Projects

All Visual Studio 6.0 project files (.dsp, .dsw) have been removed. The CMake system replicates the original build configuration while providing modern build tool support.

### Changes from Original
- Build output paths simplified to `build/bin` and `build/lib`
- Precompiled headers optional (can be enabled per project)
- Compiler-specific settings abstracted through CMake
- Better cross-version Visual Studio support

## Troubleshooting

**Issue**: CMake can't find MFC
- **Solution**: Install "C++ MFC for latest build tools" in Visual Studio Installer

**Issue**: Link errors for OpenGL
- **Solution**: Ensure Windows SDK is installed

**Issue**: Missing include directories
- **Solution**: Check that all dependencies are built before dependent targets

## Further Customization

Each subdirectory's `CMakeLists.txt` can be customized independently. Common modifications:
- Enable/disable precompiled headers
- Add additional source files
- Modify compile definitions
- Adjust include paths
