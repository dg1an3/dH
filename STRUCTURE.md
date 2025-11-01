# Repository Structure After Reorganization

## Directory Tree (New Organization)

```
dH/
│
├── Brimstone_src.sln ················· Main production solution (updated to src/)
├── README.md ························· Quick start guide
├── REORGANIZATION.md ················· Reorganization details
├── STRUCTURE.md ······················ This file
│
├── src/ ······························ MODERN STACK (Production)
│   ├── Brimstone/ ···················· MFC GUI application
│   │   ├── Brimstone.vcxproj ········· Project file (refs: RtModel, Graph)
│   │   ├── Brimstone.cpp ············· Application entry
│   │   ├── BrimstoneDoc.cpp ·········· Document (owns Series, Plan)
│   │   ├── BrimstoneView.cpp ········· Main view (CT, contours, dose)
│   │   ├── OptThread.cpp ············· Optimization thread
│   │   └── SeriesDicomImporter.cpp ··· DICOM import
│   │
│   ├── RtModel/ ······················ Core RT optimization library
│   │   ├── RtModel.vcxproj ··········· Project file (self-contained)
│   │   ├── ConjGradOptimizer.cpp ····· Conjugate gradient (variational Bayes)
│   │   ├── Prescription.cpp ·········· Objective function
│   │   ├── KLDivTerm.cpp ············· KL divergence for DVH matching
│   │   ├── BeamDoseCalc.cpp ·········· Dose calculation
│   │   ├── Series.cpp ················ CT + structures
│   │   ├── Plan.cpp ··················· Beam setup + optimization
│   │   └── include/ ··················· Headers (VectorN, MatrixNxM, etc.)
│   │
│   ├── Graph/ ························ DVH visualization
│   │   ├── Graph.vcxproj ············· Project file (refs: RtModel)
│   │   └── (graphing components)
│   │
│   ├── GenImaging/ ··················· ITK wrapper utilities
│   │   ├── GenImaging.vcproj
│   │   └── (ITK filter wrappers)
│   │
│   └── OptimizeN/ ···················· N-dimensional optimization
│       ├── OPTIMIZER_BASE.vcproj
│       ├── ConjGradOptimizer.h/cpp ··· Generic conjugate gradient
│       ├── BrentOptimizer.h/cpp ······ Line search
│       ├── PowellOptimizer.h/cpp ····· Powell's method
│       └── DFPOptimizer.h/cpp ········ Davidon-Fletcher-Powell
│
├── src_classic/ ······················ CLASSIC STACK (Legacy Reference)
│   ├── Brimstone_original/ ··········· Legacy application
│   │   ├── Brimstone.vcproj ·········· VS 2005 project
│   │   ├── Brimstone.cpp
│   │   ├── BrimstoneDoc.cpp ·········· (larger, different impl)
│   │   └── BrimstoneView.cpp
│   │
│   ├── RT_MODEL/ ····················· Original modular RT library
│   │   ├── RT_MODEL.vcproj
│   │   ├── DicomImEx/ ················ DICOM I/O subproject
│   │   ├── GenBeamlets/ ·············· Beamlet generator utility
│   │   ├── GenDens/ ··················· Density generator utility
│   │   └── TestHisto/ ················ Histogram tests
│   │
│   ├── Graph_original/ ··············· Legacy DVH visualization
│   │   ├── Graph.vcproj ·············· (refs: RT_MODEL)
│   │   └── (legacy graphing)
│   │
│   ├── VSIM_OGL/ ····················· Visual simulation with OpenGL (GUI app)
│   │   ├── VSIM_OGL.vcproj ··········· Uses OGL_BASE, GUI_BASE
│   │   └── (OpenGL visualization)
│   │
│   ├── DivFluence/ ··················· Divergent fluence calculations
│   │   ├── DivFluence.cpp ············ Console utility
│   │   └── OrigDivFluence.cpp ········ Original version
│   │
│   └── PenBeamEdit/ ·················· Pencil beam editor (GUI app)
│       ├── PenBeamEdit.dsp ··········· DevStudio 6.0 format (2004)
│       └── (MFC application using MTL)
│
├── lib_classic/ ······················ CLASSIC FOUNDATION LIBRARIES
│   ├── MTL/ ·························· Math Template Library
│   │   ├── MTL.vcproj
│   │   ├── VectorD.h ················· Dynamic-size vectors
│   │   ├── MatrixD.h ················· Dynamic-size matrices
│   │   ├── VectorN.h ················· Fixed-size vectors
│   │   └── MatrixNxM.h ··············· Fixed-size matrices
│   │
│   ├── GEOM_MODEL/ ··················· Geometry modeling (active)
│   │   ├── GEOM_MODEL.vcproj
│   │   ├── Polygon.h/cpp ············· 2D polygons
│   │   ├── Mesh.h/cpp ················ 3D meshes
│   │   ├── Pyramid.h/cpp ············· Multi-resolution structures
│   │   └── ClusterTest/ ·············· Clustering test app
│   │
│   ├── XMLLogging/ ··················· XML-based logging
│   │   ├── XMLLogging.vcproj
│   │   ├── XMLConsole/ ··············· Console app for viewing logs
│   │   └── TestXMLLogging/ ··········· Unit tests
│   │
│   ├── VSIM_MODEL/ ··················· Visual simulation model
│   │   ├── VSIM_MODEL.vcproj
│   │   └── Test/ ····················· Model tests
│   │
│   ├── GEOM_BASE/ ···················· Base geometry (OBSOLETE - superseded by GEOM_MODEL)
│   │   ├── GEOM_BASE.dsp ············· DevStudio 6.0 format
│   │   └── (Vector, Matrix, Polygon)
│   │
│   ├── MODEL_BASE/ ··················· Base model classes (OBSOLETE - superseded by GEOM_MODEL)
│   │   ├── MODEL_BASE.dsp ············ DevStudio 6.0 format
│   │   └── (ModelObject, Observer)
│   │
│   ├── GUI_BASE/ ····················· GUI framework (OBSOLETE)
│   │   ├── GUI_BASE.dsp ·············· DevStudio 6.0 format
│   │   └── (Dib, Graph, TabControlBar)
│   │
│   ├── OGL_BASE/ ····················· OpenGL abstraction (OBSOLETE)
│   │   ├── OGL_BASE.dsp ·············· DevStudio 6.0 format
│   │   └── (OpenGLView, OpenGLCamera)
│   │
│   ├── GEOM_VIEW/ ···················· 3D rendering/view (OBSOLETE)
│   │   ├── GEOM_VIEW.dsp ············· DevStudio 6.0 format
│   │   └── (Camera, SceneView, Light)
│   │
│   ├── RT_VIEW/ ·························· RT-specific views (OBSOLETE)
│   │   ├── RT_VIEW.dsp ··············· DevStudio 6.0 format
│   │   └── (BeamRenderable, MachineRenderable)
│   │
│   ├── OPTIMIZER_BASE/ ··············· Original optimizer prototype (OBSOLETE)
│   │   ├── OPTIMIZER_BASE.dsp ········ DevStudio 6.0 format (superseded by OptimizeN)
│   │   └── (Header-only prototypes)
│   │
│   ├── VecMat/ ······················· Vector/matrix math (OBSOLETE - superseded by MTL)
│   │   ├── VecMat.dsp ················ DevStudio 6.0 format
│   │   ├── TestVec/ ·················· Test application
│   │   └── (CVectorN, CMatrixNxM) ···· Used by VSIM_OGL, RT_VIEW, VSIM_MODEL
│   │
│   └── VecMat_original/ ·············· Original VecMat (OBSOLETE - superseded by VecMat then MTL)
│       ├── VecMat.dsp ················ DevStudio 6.0 format (~2002)
│       ├── TestVec/ ·················· Test application
│       └── (Earlier vector/matrix implementation)
│
├── deprecated/ ······················· DEPRECATED CODE (empty - pending other items)
│
├── docs/ ····························· DOCUMENTATION
│   ├── CLAUDE.md ····················· Claude Code project instructions
│   ├── repository_structure.md ······· Structure analysis
│   ├── brimstone_comparison.md ······· Modern vs classic comparison
│   ├── proposed_reorganization.md ···· Reorganization proposal
│   └── brimstone_dependency_evolution.md · Historical dependency analysis
│
├── python/ ··························· Python utilities
│   └── read_process_series.py ········ ITK-based CT processing
│
├── notebook_zoo/ ····················· Jupyter notebooks
│   └── entropy_max.sln ··············· Entropy maximization experiments
│
├── utils/ ···························· Utilities (to be organized)
├── tests/ ···························· Tests (to be organized)
└── deprecated/ ······················· Deprecated code (to be organized)

## Original Directories (Still in Root - Pending Cleanup)

These directories still exist and should be removed after build verification:

├── Brimstone/ ························ → copied to src/Brimstone/
├── RtModel/ ·························· → copied to src/RtModel/
├── Graph/ ···························· → copied to src/Graph/
├── GenImaging/ ······················· → copied to src/GenImaging/
├── OptimizeN/ ························ → copied to src/OptimizeN/
├── Brimstone_original/ ··············· → copied to src_classic/
├── RT_MODEL/ ························· → copied to src_classic/
├── Graph_original/ ··················· → copied to src_classic/
├── VSIM_OGL/ ························· → copied to src_classic/
├── DivFluence/ ······················· → copied to src_classic/
├── MTL/ ······························ → copied to lib_classic/
├── GEOM_MODEL/ ······················· → copied to lib_classic/
├── XMLLogging/ ······················· → copied to lib_classic/
├── VSIM_MODEL/ ······················· → copied to lib_classic/
├── GEOM_BASE/ ························ → copied to lib_classic/ (obsolete)
├── MODEL_BASE/ ······················· → copied to lib_classic/ (obsolete)
├── GUI_BASE/ ························· → copied to lib_classic/ (obsolete)
├── OGL_BASE/ ························· → copied to lib_classic/ (obsolete)
├── GEOM_VIEW/ ························ → copied to lib_classic/ (obsolete)
├── RT_VIEW/ ·························· → copied to lib_classic/ (obsolete)
├── OPTIMIZER_BASE/ ··················· → copied to lib_classic/ (obsolete)
├── VecMat/ ··························· → copied to lib_classic/ (obsolete)
├── VecMat_original/ ·················· → copied to lib_classic/ (obsolete)
└── PenBeamEdit/ ······················ → copied to src_classic/

**Action Required:** Delete these after verifying build works

## Other Directories (Not Yet Organized)

├── FTL/ ······························ Field Template Library
├── FieldCOM/ ························· COM wrapper (not active)
└── CVSROOT/ ·························· CVS metadata (obsolete)

**Proposed:** Move to deprecated/ or utils/ as appropriate

## Dependency Graph

### Modern Stack (src/)
```
Brimstone.exe
  ├─→ RtModel.lib (self-contained)
  └─→ Graph.lib
      └─→ RtModel.lib

GenImaging.lib (standalone ITK wrapper)
OptimizeN.lib (may need MTL from lib_classic/)
```

### Classic Stack (src_classic/ + lib_classic/)
```
Brimstone_original.exe
  ├─→ RT_MODEL.lib (src_classic/)
  │   ├─→ GEOM_MODEL.lib (lib_classic/)
  │   ├─→ MTL.lib (lib_classic/)
  │   ├─→ OptimizeN.lib (root - needs path update)
  │   └─→ XMLLogging.lib (lib_classic/)
  ├─→ Graph_original.lib (src_classic/)
  │   ├─→ RT_MODEL.lib
  │   ├─→ GEOM_MODEL.lib (lib_classic/)
  │   ├─→ MTL.lib (lib_classic/)
  │   └─→ XMLLogging.lib (lib_classic/)
  └─→ DicomImEx.lib (in RT_MODEL/)
      └─→ (same as RT_MODEL)
```

## File Counts by Directory

### src/ (Modern Stack)
- Brimstone: 27 files (.cpp + .h)
- RtModel: ~60+ files (library)
- Graph: ~20+ files
- GenImaging: ~10+ files
- OptimizeN: ~15+ files

### src_classic/ (Classic Stack)
- Brimstone_original: 23 files (.cpp + .h)
- RT_MODEL: ~80+ files (modular)
- Graph_original: ~20+ files
- VSIM_OGL: OpenGL visualization app
- DivFluence: Fluence calculation utilities
- PenBeamEdit: Pencil beam editor app (MFC)

### lib_classic/ (Foundation)
**Active Libraries:**
- MTL: ~20+ files (math - current)
- GEOM_MODEL: ~40+ files (geometry - current)
- XMLLogging: ~10+ files (logging)
- VSIM_MODEL: Visual simulation model

**Obsolete Libraries (DevStudio 6.0 format):**
- GEOM_BASE: Superseded by GEOM_MODEL
- MODEL_BASE: Superseded by GEOM_MODEL
- GUI_BASE: Obsolete GUI framework
- OGL_BASE: Obsolete OpenGL abstraction
- GEOM_VIEW: Obsolete 3D rendering
- RT_VIEW: Obsolete RT visualization
- OPTIMIZER_BASE: Superseded by OptimizeN
- VecMat: Superseded by MTL (~2003)
- VecMat_original: Superseded by VecMat then MTL (~2002)

## Build Status

✅ Main solution updated: Brimstone_src.sln → src/
✅ Modern stack copied to src/
✅ Classic stack copied to src_classic/
✅ Classic libs copied to lib_classic/
✅ Documentation moved to docs/
⏳ Build verification pending
⏳ OptimizeN may need MTL path fix
⏳ Original directories pending deletion
⏳ Classic stack paths need updates

## Next Actions

1. **Verify build** - Test Brimstone_src.sln
2. **Check OptimizeN** - May need `..\..\lib_classic\MTL\` path
3. **Delete originals** - Remove root directories after verification
4. **Update classic** - Fix src_classic/ paths to lib_classic/
5. **Organize remaining** - Move deprecated, utils, tests

## Questions to Resolve

1. **OptimizeN dependency:** Should MTL be copied to src/ for OptimizeN?
2. **Original directories:** When to delete from root?
3. **Classic stack:** How important is maintaining build-ability?
4. **Further organization:** Continue with utils/, tests/, deprecated/?
