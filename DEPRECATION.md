# DEPRECATION NOTICE

## Visual Studio Solution Files

The following Visual Studio solution files are **deprecated** and will be removed in a future release:

- `Brimstone_src.sln` - Main production system solution
- `Brimstone/Brimstone.sln` - Alternate Brimstone solution  
- `Brimstone_original/Brimstone.sln` - Original Brimstone solution
- Other component-specific .sln files

## Migration to CMake

The project has transitioned to **CMake** as the primary build system. CMake provides:

- Cross-platform support (Windows, Linux, macOS)
- Better dependency management
- Modern build tooling
- Easier integration with other projects

### Timeline

- **Current Release:** Both CMake and Visual Studio solutions supported
- **Next Release:** Visual Studio solutions marked as deprecated (current status)
- **Future Release:** Visual Studio solutions will be removed

### How to Migrate

Replace usage of Visual Studio solution files with CMake:

**Old approach:**
```
Open Brimstone_src.sln in Visual Studio
Build → Build Solution
```

**New approach (recommended):**
```cmd
mkdir build && cd build
cmake .. -DITK_DIR=C:\path\to\ITK-build
cmake --build . --config Release
```

Or use the provided helper script:
```cmd
build_windows.bat
```

### Documentation

- See [BUILDING.md](BUILDING.md) for comprehensive build instructions
- See [README.md](README.md) for quick start guide
- Run `validate_cmake.sh` to verify your CMake setup

### Support

If you encounter issues with the CMake build system:

1. Check [BUILDING.md](BUILDING.md) troubleshooting section
2. Verify prerequisites are installed (CMake, ITK, MFC)
3. Run the validation script: `./validate_cmake.sh`
4. Report issues with CMake configuration details

### Rationale

The migration to CMake offers several advantages:

1. **Portability:** Build on any platform with CMake support
2. **Modularity:** Optional components can be enabled/disabled
3. **Maintainability:** Simpler build configuration
4. **Integration:** Easier to use as a dependency in other projects
5. **Modern Tooling:** Better IDE support across platforms

## Project Files to be Removed

### Main Solution
- [ ] `Brimstone_src.sln`
- [ ] `RtModel/RtModel.vcxproj`
- [ ] `RtModel/RtModel.vcproj`
- [ ] `Graph/Graph.vcxproj`
- [ ] `Graph/Graph.vcproj`
- [ ] `Brimstone/Brimstone.vcxproj`
- [ ] `Brimstone/Brimstone.vcproj`

### Component Solutions
- [ ] `Brimstone/Brimstone.sln`
- [ ] `Brimstone_original/Brimstone.sln`
- [ ] `MTL/TestMTL/TestMTL.sln`
- [ ] `RT_MODEL/TestHisto/TestHisto.sln`
- [ ] `FTL/FTL.sln`
- [ ] `GEOM_MODEL/TestRegion/Test_GEOM_MODEL.sln`
- [ ] `OptimizeN/Test/OPTIMIZER_BASE_Test.sln`
- [ ] Other .vcxproj and .vcproj files

### When Removal Will Occur

These files will be removed once:

1. ✅ CMake build system is fully implemented
2. ✅ Documentation is updated
3. ✅ Validation scripts are in place
4. ⏳ Community feedback period has elapsed (recommended: 1-2 releases)
5. ⏳ No critical issues reported with CMake builds

## Current Status

✅ **CMake implementation complete**
- Main production system (RtModel, Graph, Brimstone) fully supported
- Modular options for foundation and component libraries
- Comprehensive documentation and validation scripts

⏳ **Transition period**
- Both build systems currently supported
- Users encouraged to migrate to CMake
- Report any issues with CMake builds

## Questions?

If you have questions or concerns about this migration, please open an issue on the project repository.
