# Repository Reorganization - Phase 2 Complete

## Date: 2025-10-31

## Summary

Additional reorganization completed, moving classic applications and obsolete libraries to appropriate locations.

## Changes Made in Phase 2

### 1. Moved Classic Applications to src_classic/

**VSIM_OGL** → `src_classic/VSIM_OGL/`
- Visual simulation with OpenGL (GUI application)
- Uses OGL_BASE, GUI_BASE, GEOM_BASE libraries
- DevStudio 6.0 project format

**DivFluence** → `src_classic/DivFluence/`
- Divergent fluence calculation utilities
- Two versions: DivFluence.cpp and OrigDivFluence.cpp
- Console applications

### 2. Moved Library to lib_classic/

**VSIM_MODEL** → `lib_classic/VSIM_MODEL/`
- Visual simulation model library
- Contains test applications
- Part of classic simulation framework

### 3. Moved 7 Obsolete Libraries to lib_classic/

All these libraries use **DevStudio 6.0 (.dsp) format** and are **obsolete**:

**GEOM_BASE** → `lib_classic/GEOM_BASE/`
- Basic geometry classes (Vector, Matrix, Polygon)
- **Superseded by** GEOM_MODEL
- No active solution references

**MODEL_BASE** → `lib_classic/MODEL_BASE/`
- Base model classes (ModelObject, Observer)
- **Superseded by** GEOM_MODEL
- No active solution references

**GUI_BASE** → `lib_classic/GUI_BASE/`
- GUI framework (Dib, Graph, TabControlBar)
- **Obsolete** - modern Brimstone uses different GUI
- No active solution references

**OGL_BASE** → `lib_classic/OGL_BASE/`
- OpenGL abstraction layer
- Only used by VSIM_OGL (also obsolete)
- No active solution references

**GEOM_VIEW** → `lib_classic/GEOM_VIEW/`
- 3D rendering and view classes
- **Obsolete** visualization layer
- No active solution references

**RT_VIEW** → `lib_classic/RT_VIEW/`
- Radiotherapy-specific view classes
- **Obsolete** - integrated into modern Brimstone
- No active solution references

**OPTIMIZER_BASE** (root directory) → `lib_classic/OPTIMIZER_BASE/`
- Original optimizer prototype (header-only)
- **Superseded by** OptimizeN/OPTIMIZER_BASE
- Contains only 2 .cpp files vs OptimizeN's 9 files
- No active solution references

## Current Directory Structure

```
dH/
├── src/                      → Modern production stack
│   ├── Brimstone/
│   ├── RtModel/
│   ├── Graph/
│   ├── GenImaging/
│   └── OptimizeN/
│
├── src_classic/              → Classic applications
│   ├── Brimstone_original/
│   ├── RT_MODEL/
│   ├── Graph_original/
│   ├── VSIM_OGL/             ← NEW
│   └── DivFluence/           ← NEW
│
└── lib_classic/              → Classic libraries
    ├── MTL/                  (active)
    ├── GEOM_MODEL/           (active)
    ├── XMLLogging/           (active)
    ├── VSIM_MODEL/           ← NEW (active)
    ├── GEOM_BASE/            ← NEW (obsolete)
    ├── MODEL_BASE/           ← NEW (obsolete)
    ├── GUI_BASE/             ← NEW (obsolete)
    ├── OGL_BASE/             ← NEW (obsolete)
    ├── GEOM_VIEW/            ← NEW (obsolete)
    ├── RT_VIEW/              ← NEW (obsolete)
    └── OPTIMIZER_BASE/       ← NEW (obsolete)
```

## Analysis Results

### Classic Architecture Layers (All in lib_classic/)

The obsolete libraries reveal the original layered architecture:

```
Application Layer (src_classic/):
  VSIM_OGL, Brimstone_original, RT_MODEL

View Layer (lib_classic/ - OBSOLETE):
  RT_VIEW → GEOM_VIEW → GUI_BASE → OGL_BASE

Model Layer (lib_classic/):
  GEOM_MODEL (active), GEOM_BASE (obsolete), MODEL_BASE (obsolete)

Foundation Layer (lib_classic/):
  MTL (active)
  OPTIMIZER_BASE (obsolete) - superseded by OptimizeN
```

### Active vs Obsolete in lib_classic/

**Active Libraries (4):**
- MTL - Math Template Library (used by classic stack)
- GEOM_MODEL - Geometry modeling (used by classic stack)
- XMLLogging - Logging framework (used by classic stack)
- VSIM_MODEL - Simulation model (used by VSIM_OGL)

**Obsolete Libraries (7):**
- GEOM_BASE, MODEL_BASE - Superseded by GEOM_MODEL
- GUI_BASE, OGL_BASE - Obsolete GUI/OpenGL frameworks
- GEOM_VIEW, RT_VIEW - Obsolete view layers
- OPTIMIZER_BASE - Superseded by OptimizeN

All obsolete libraries use **DevStudio 6.0 format (.dsp files)** dating to ~1998-2002.

## Key Findings

### 1. OPTIMIZER_BASE Confusion Resolved

There are **TWO different OPTIMIZER_BASE** libraries:

**Root OPTIMIZER_BASE** (now in lib_classic/):
- DevStudio 6.0 format (.dsp)
- Header-only prototype with minimal implementation
- Only 2 .cpp files (BrentOptimizer.cpp 12KB, StdAfx.cpp)
- **Obsolete** - superseded by OptimizeN

**OptimizeN/OPTIMIZER_BASE**:
- Modern Visual Studio format (.vcproj)
- Full implementation with 9 .cpp files
- **Active** - used by classic Brimstone_original
- Located in OptimizeN/ directory

### 2. Evolution Timeline

**Phase 1 (DevStudio 6.0 era - ~1998-2002):**
- GEOM_BASE, MODEL_BASE, GUI_BASE, OGL_BASE
- GEOM_VIEW, RT_VIEW
- OPTIMIZER_BASE (root - prototype)

**Phase 2 (Visual Studio 2005 era - ~2005-2008):**
- GEOM_MODEL supersedes GEOM_BASE, MODEL_BASE
- OptimizeN supersedes OPTIMIZER_BASE
- RT_MODEL, Graph_original, Brimstone_original
- MTL, XMLLogging

**Phase 3 (Visual Studio 2010+ era - ~2010-present):**
- RtModel supersedes RT_MODEL (self-contained)
- Graph supersedes Graph_original
- Modern Brimstone supersedes Brimstone_original
- Simplified architecture (2 libraries vs 7+)

## Original Directories Still in Root

These directories remain and should be deleted after verification:

**Now in src_classic/:**
- VSIM_OGL/
- DivFluence/

**Now in lib_classic/:**
- VSIM_MODEL/
- GEOM_BASE/
- MODEL_BASE/
- GUI_BASE/
- OGL_BASE/
- GEOM_VIEW/
- RT_VIEW/
- OPTIMIZER_BASE/

**From Phase 1 (already tracked):**
- Brimstone/, RtModel/, Graph/, GenImaging/, OptimizeN/
- Brimstone_original/, RT_MODEL/, Graph_original/
- MTL/, GEOM_MODEL/, XMLLogging/

## Build Impact

### Modern Stack (src/)
- **No impact** - modern stack doesn't use any moved libraries
- Build should work exactly as before

### Classic Stack (src_classic/ + lib_classic/)
- **VSIM_OGL** may need path updates to find lib_classic/ libraries
- **DivFluence** likely standalone (no significant dependencies)
- **Brimstone_original** already references lib_classic/ (needs path updates from Phase 1)

## Documentation Updated

- ✅ STRUCTURE.md - Updated with all new locations
- ✅ Shows obsolete libraries clearly marked in lib_classic/
- ✅ Updated file counts and dependency graphs

## Remaining Tasks

### Not Yet Organized
- VecMat/ - Old math library (superseded by MTL)
- VecMat_original/ - Even older version
- FTL/ - Field Template Library
- PenBeamEdit/ - Pencil beam editor app
- FieldCOM/ - COM wrapper
- CVSROOT/ - CVS metadata (obsolete)

### Proposed Next Steps
1. **Test modern build** - Verify Brimstone_src.sln works
2. **Delete original directories** - Clean up root after verification
3. **Organize remaining** - Move VecMat, FTL, PenBeamEdit to appropriate locations
4. **Update classic paths** - Fix src_classic/ project references if needed

## Benefits of Phase 2 Reorganization

1. **Clear separation** - Classic apps in src_classic/, classic libs in lib_classic/
2. **Obsolete code identified** - 7 obsolete libraries clearly marked
3. **Historical context preserved** - DevStudio 6.0 libraries show evolution
4. **Reduced root clutter** - 10 more directories organized
5. **Better understanding** - Architecture layers now visible

## Summary Statistics

**Total directories reorganized:** 10
- Applications to src_classic/: 2 (VSIM_OGL, DivFluence)
- Libraries to lib_classic/: 8 (VSIM_MODEL + 7 obsolete)

**lib_classic/ now contains:**
- Active libraries: 4 (MTL, GEOM_MODEL, XMLLogging, VSIM_MODEL)
- Obsolete libraries: 7 (all DevStudio 6.0 format)
- Total: 11 libraries

**src_classic/ now contains:**
- Applications: 5 (Brimstone_original, RT_MODEL with utils, Graph_original, VSIM_OGL, DivFluence)

## Verification Checklist

Before deleting original directories:
- [ ] Build modern stack: `msbuild Brimstone_src.sln`
- [ ] Verify Brimstone.exe runs
- [ ] Check that no broken references exist
- [ ] Review git status
- [ ] Create backup if needed

After verification:
- [ ] Delete original directories from root
- [ ] Update git to track new locations
- [ ] Optional: Update classic stack paths
- [ ] Optional: Organize remaining directories (VecMat, FTL, etc.)
