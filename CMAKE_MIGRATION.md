# CMake Migration Checklist

This document tracks the migration from Visual Studio solution files to CMake build system.

## Implementation Status

### Core CMake Files
- [x] Root CMakeLists.txt created
- [x] RtModel/CMakeLists.txt created
- [x] Graph/CMakeLists.txt created
- [x] Brimstone/CMakeLists.txt created
- [x] cmake/BrimstoneConfig.cmake.in created
- [x] Build options implemented (modular configuration)
- [x] Installation rules configured
- [x] Package config files configured

### Documentation
- [x] README.md updated with CMake instructions
- [x] BUILDING.md created with comprehensive build guide
- [x] DEPRECATION.md created with migration timeline
- [x] Build scripts created (validate_cmake.sh, build_windows.bat)

### Testing and Validation

#### Automated Validation (Completed)
- [x] CMake syntax validation
- [x] Configuration without ITK (minimal build)
- [x] File structure verification
- [x] Build file generation

#### Manual Validation Required (Windows with ITK + MFC)
- [ ] CMake configuration succeeds with ITK
- [ ] RtModel library builds successfully
- [ ] Graph library builds successfully
- [ ] Brimstone application builds successfully
- [ ] All kernel data files copied correctly
- [ ] Application runs and loads
- [ ] Basic functionality testing
- [ ] No build warnings or errors

## Validation Steps for Users

To verify CMake build on Windows with Visual Studio and ITK:

### Step 1: Configure
```cmd
set ITK_DIR=C:\path\to\ITK-build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DITK_DIR=%ITK_DIR%
```

**Verify:** Configuration completes without errors

### Step 2: Build
```cmd
cmake --build . --config Release
```

**Verify:** 
- RtModel.lib created in build/lib/Release/
- Graph.lib created in build/lib/Release/
- Brimstone.exe created in build/bin/Release/
- Kernel .dat files copied to build/bin/Release/

### Step 3: Run
```cmd
cd bin\Release
Brimstone.exe
```

**Verify:**
- Application launches
- No missing DLL errors
- Can create new plan
- Can load existing data (if test data available)

### Step 4: Compare with Visual Studio Build

Build the same project using Brimstone_src.sln:

1. Open Brimstone_src.sln in Visual Studio
2. Build → Build Solution
3. Compare output sizes and functionality

**Verify:**
- Libraries are similar size
- Application has same functionality
- No regressions in behavior

## Removal Checklist

Before removing Visual Studio solution files (.sln, .vcxproj, .vcproj):

### Prerequisites
- [x] CMake build system fully implemented
- [x] Documentation complete
- [x] Migration guide provided
- [ ] Manual validation completed (requires Windows + ITK + MFC)
- [ ] No critical issues reported
- [ ] Community feedback period elapsed (recommended: 1-2 releases)

### Files to Remove

When all prerequisites are met, remove:

#### Main Solution Files
```
Brimstone_src.sln
RtModel/RtModel.vcxproj
RtModel/RtModel.vcxproj.filters
RtModel/RtModel.vcproj
Graph/Graph.vcxproj
Graph/Graph.vcxproj.filters
Graph/Graph.vcproj
Brimstone/Brimstone.vcxproj
Brimstone/Brimstone.vcxproj.filters
Brimstone/Brimstone.vcproj
```

#### Component Solution Files
```
Brimstone/Brimstone.sln
Brimstone_original/Brimstone.sln
Brimstone_original/Brimstone.vcproj
Graph_original/Graph.vcproj
MTL/TestMTL/TestMTL.sln
MTL/MTL.vcproj
RT_MODEL/TestHisto/TestHisto.sln
RT_MODEL/RT_MODEL.vcproj
FTL/FTL.sln
FTL/FTL.vcproj
GEOM_MODEL/TestRegion/Test_GEOM_MODEL.sln
GEOM_MODEL/GEOM_MODEL.vcproj
OptimizeN/Test/OPTIMIZER_BASE_Test.sln
OptimizeN/OPTIMIZER_BASE.vcproj
GenImaging/GenImaging.vcproj
XMLLogging/XMLLogging.vcproj
```

#### Supporting Files
```
*.vcxproj.user
*.vcproj.vspscc
*.vssscc
```

## Current Decision

**Status:** ✅ CMake implementation complete, ⏳ Awaiting validation

**Recommendation:** Keep Visual Studio solution files until:
1. Manual validation completed on Windows with ITK and MFC
2. At least one release cycle with CMake as primary build system
3. No critical issues reported

**Rationale:**
- Cannot fully validate CMake build without ITK and MFC (Windows-only)
- Responsible software engineering suggests deprecation period
- Maintains backward compatibility during transition
- Allows community to report issues

## Next Steps

1. **For Users:** Test CMake build on Windows with ITK and MFC
2. **For Maintainers:** Collect feedback on CMake builds
3. **For Future:** Remove Visual Studio files after successful validation period

## Reporting Validation Results

If you successfully build with CMake, please report:
- Windows version
- Visual Studio version
- CMake version
- ITK version
- Any issues encountered
- Comparison with Visual Studio solution build

This information helps determine when safe to remove legacy build files.
