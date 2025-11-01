# Brimstone vs Brimstone_original: Structural Comparison

## Executive Summary

**Brimstone** and **Brimstone_original** are two distinct versions of the radiotherapy planning GUI application with significant structural differences. They share similar functionality but depend on different library implementations and have evolved separately.

## Key Differences

### 1. Project Format and Visual Studio Version

| Aspect | Brimstone | Brimstone_original |
|--------|-----------|-------------------|
| Project Format | .vcxproj (VS 2010+) | .vcproj (VS 2005/2008) |
| Toolset | v143 (VS 2022) | Version 8.00 (VS 2005) |
| Platforms | Win32, x64 | Win32 only |

### 2. Solution Structure

**Brimstone** (Brimstone/Brimstone.sln):
```
Brimstone.sln (VS 2005 format, but references newer projects)
├─ Brimstone (app)
├─ DicomImEx (from ..\RT_MODEL\)
├─ GEOM_MODEL (from ..\GEOM_MODEL\)
├─ MTL (from ..\MTL\)
├─ OPTIMIZER_BASE (from ..\OptimizeN\)
├─ RT_MODEL (from ..\RT_MODEL\)     ← OLD library structure
├─ XMLLogging (from ..\XMLLogging\)
├─ Graph (from ..\Graph\)
└─ DCMTK libraries (external: dcmdata, dcmimgle, ofstd)
```

**Brimstone_original** (Brimstone_original/Brimstone.sln):
```
Brimstone_original.sln (identical to Brimstone/Brimstone.sln!)
├─ Brimstone (app)
├─ DicomImEx
├─ GEOM_MODEL
├─ MTL
├─ OPTIMIZER_BASE
├─ RT_MODEL                          ← OLD library structure
├─ XMLLogging
├─ Graph
└─ DCMTK libraries (external)
```

**Main Production** (Brimstone_src.sln - at repo root):
```
Brimstone_src.sln (VS 2010+ format)
├─ RtModel (from .\RtModel\)        ← NEW consolidated library
├─ Brimstone (from .\Brimstone\)
└─ Graph (from .\Graph\)
```

### 3. Core Library Dependencies

**Brimstone** references:
- `RtModel` (NEW consolidated library at `..\RtModel\`)
- `Graph` (NEW version at `..\Graph\`)
- Paths: `..\RtModel\include`, `..\Graph\include`

**Brimstone_original** references:
- `RT_MODEL` (OLD library at `..\RT_MODEL\`)
- `Graph` (OLD version - references `..\Graph\` but may use Graph_original)
- Paths: `..\RT_MODEL\include`, `..\Graph\include`

### 4. Source File Differences

**File Count:**
- Brimstone: 27 files (.cpp + .h)
- Brimstone_original: 23 files (.cpp + .h)

**Unique to Brimstone:**
- `SeriesDicomImporter.cpp/h` - DICOM import functionality
- `PrescriptionBar.cpp/h/resx` - UI component
- `stdafx.cpp/h` (lowercase)

**Unique to Brimstone_original:**
- `StdAfx.cpp/h` (uppercase - older convention)

**Shared but Modified:**
Most core files exist in both but have been modified independently:
- `Brimstone.cpp` (different sizes: 4223 vs 4139 bytes)
- `BrimstoneDoc.cpp` (7538 vs 15263 bytes - significant difference!)
- `BrimstoneView.cpp` (13303 vs 16027 bytes)
- `MainFrm.cpp` (2828 vs 3305 bytes)
- `OptThread.cpp` (3874 vs 2998 bytes)
- `PlanarView.cpp` (35191 vs 24838 bytes - major refactoring!)
- `PlanSetupDlg.cpp` (4667 vs 1114 bytes)
- `PrescriptionToolbar.cpp` (11846 vs 9771 bytes)

### 5. Dependency Architecture

```
Brimstone (MODERN):
    Brimstone.exe
    ├─→ RtModel.lib (consolidated, modern)
    │   └─→ ITK, VNL
    └─→ Graph.lib (modern)
        └─→ RtModel.lib

Brimstone_original (CLASSIC):
    Brimstone.exe
    ├─→ RT_MODEL.lib (old modular structure)
    ├─→ GEOM_MODEL.lib
    ├─→ OPTIMIZER_BASE.lib
    ├─→ MTL.lib
    ├─→ XMLLogging.lib
    ├─→ DicomImEx.lib
    └─→ Graph.lib (old version)
        └─→ RT_MODEL.lib
    └─→ DCMTK (dcmdata, dcmimgle, ofstd)
```

## Clean Separation Feasibility Analysis

### Can They Be Separated into src_classic/ and src/?

**Answer: NO - Not Cleanly**

### Reasons:

#### 1. Shared Library Dependencies
Both versions depend on libraries in the same parent directory structure:
- `Graph/` (but different versions - Graph vs Graph_original exists)
- `RT_MODEL/` vs `RtModel/` (different libraries!)
- `GEOM_MODEL/`, `MTL/`, `OptimizeN/`, `XMLLogging/`

Moving Brimstone folders would break relative paths like `..\Graph\`, `..\RtModel\`, etc.

#### 2. Library Evolution Creates Conflicts
- **Brimstone_original** uses the OLD modular library structure (RT_MODEL, GEOM_MODEL, etc.)
- **Brimstone** uses the NEW consolidated RtModel library
- Both reference `..\Graph\` but may need different versions (Graph vs Graph_original)

#### 3. Source Code Divergence
The source files are NOT identical - they've evolved independently:
- BrimstoneDoc.cpp: 2x size difference (15KB vs 7.5KB)
- PlanarView.cpp: Major refactoring (35KB vs 24KB)
- Different features: SeriesDicomImporter only in modern version

#### 4. External Dependencies Differ
- DCMTK paths differ: `..\..\..\dcmtk-3.5.3\` vs `$(DCMTK_DIR)\`
- ITK integration differs between versions

## Recommended Approach

### Option 1: Keep Current Structure (RECOMMENDED)
```
dH/
├─ Brimstone/              (modern version - depends on RtModel)
├─ Brimstone_original/     (classic version - depends on RT_MODEL)
├─ RtModel/                (modern consolidated library)
├─ RT_MODEL/               (classic modular library)
├─ Graph/                  (modern version)
├─ Graph_original/         (classic version)
└─ [other shared libraries]
```

**Advantages:**
- No path changes needed
- Both versions can coexist
- Easy to compare differences
- Maintains historical context

**Disadvantages:**
- Flat directory structure less organized
- Not immediately clear which is "production"

### Option 2: Archive Classic Version
```
dH/
├─ src/                    (modern Brimstone + RtModel + Graph)
├─ archive_classic/
│   ├─ Brimstone_original/
│   ├─ RT_MODEL/
│   ├─ Graph_original/
│   └─ [related classic libs]
└─ [shared foundation libraries]
```

**Advantages:**
- Clearer which version is active
- Classic version preserved for reference

**Disadvantages:**
- Breaks relative paths in classic version
- Would require project file updates to build classic
- More complex to maintain both

### Option 3: Document-Only Separation
Keep physical structure, but add clear documentation:
- Update README to designate Brimstone as production
- Mark Brimstone_original as legacy/reference
- Add DEPRECATED.md in Brimstone_original/

**Advantages:**
- No file structure changes
- Both versions remain buildable
- Clear intent without disruption

**Disadvantages:**
- Directory structure remains flat

## Migration Path Analysis

If you want to consolidate to a single modern version:

### Step 1: Identify Unique Features
- Compare functionality between versions
- Find features in Brimstone_original not in Brimstone
- Document behavioral differences

### Step 2: Port Missing Features
- Port unique features from Brimstone_original to Brimstone
- Test thoroughly

### Step 3: Deprecate Original
- Mark Brimstone_original as deprecated
- Eventually move to archive/

## Conclusion

**Clean separation into src_classic/ and src/ is NOT feasible without significant refactoring** due to:
1. Shared library dependencies with relative paths
2. Different underlying library architectures (RT_MODEL vs RtModel)
3. Divergent source code evolution
4. Both need access to common foundation libraries

**Best approach:**
- Keep current structure
- Add clear documentation marking Brimstone as production
- Consider Brimstone_original as legacy/reference
- If needed, selectively port unique features from original to modern version
- Eventually archive classic version once modern version is feature-complete

## Key Questions to Answer

1. **Is Brimstone_original still actively used/built?**
   - If no: Move to archive/ with documentation
   - If yes: Identify why and what features are missing in Brimstone

2. **What unique functionality exists in Brimstone_original?**
   - Larger BrimstoneDoc.cpp suggests different document model
   - Different PlanarView implementation
   - Missing SeriesDicomImporter

3. **Which version is production?**
   - Brimstone_src.sln (root level) suggests Brimstone is production
   - Brimstone_original may be frozen legacy code

4. **Are both actively maintained?**
   - File timestamps would indicate this
   - Git history would show which receives updates
