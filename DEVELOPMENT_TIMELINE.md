# dH/Brimstone Repository: Development History

## Overview

This document traces the evolution of a radiotherapy treatment planning system from its origins in 1988 Fortran code through to the present day. The development spans **three decades** and involves **two parallel evolution paths** that converge in the final Brimstone application.

**Key Facts:**
- **Patent:** U.S. Patent 7,369,645
- **Primary Author:** Derek G. Lane (2nd Messenger Systems)
- **Languages:** Fortran → C++ → Python
- **Core Innovation:** Variational inverse planning using KL divergence minimization

---

## Part 1: Algorithm Origins (1988-2000)

### The Wisconsin Convolution/Superposition Method

The dose calculation algorithm originates from **Rock Mackie's group at the University of Wisconsin** (1988). The key insight: radiation dose can be computed by:

1. **TERMA calculation** - Ray-trace through patient to compute Total Energy Released per unit MAss
2. **Kernel convolution** - Spread energy using pre-computed deposition kernels

**Original Fortran Code** (`PenBeam_indens/code/`):
```
ray_trace_set_up.for  - Build ray-trace lookup tables
make_vector.for       - Direction cosines → voxel locations
new_sphere_convolve.for - Spherical kernel convolution
```

The algorithm uses a **plane-crossing** technique: for each ray direction, precompute which voxel boundaries are crossed and in what order. This avoids expensive per-ray calculations.

### C++ Translation: DivFluence (~2000)

The Fortran code was translated to C++ in `DivFluence/OrigDivFluence.cpp` (~980 lines):

| Fortran | C++ | Purpose |
|---------|-----|---------|
| `mydiv_fluence_calc.for` | `div_fluence_calc()` | TERMA calculation |
| `ray_trace_set_up.for` | `ray_trace_set_up()` | Ray-trace tables |
| `make_vector.for` | `make_vector()` | Voxel traversal |
| `mynew_sphere_convolve.for` | `new_sphere_convolve()` | Kernel convolution |

This translation preserved the algorithm exactly while enabling integration with Windows/MFC applications.

---

## Part 2: Visualization Framework (2000-2002)

### Commercial Context: Siemens VSim Prototype

Around 2000-2001, development began on a **prototype for Siemens' VSim** (Virtual Simulation) product. This commercial context explains the sophisticated visualization architecture.

**Evolution of rendering:**
```
OGL_BASE (2000)     - OpenGL immediate-mode rendering
    ↓
GEOM_VIEW (2001)    - Migration to Direct3D 8 (better Windows drivers)
    ↓
RT_VIEW (2002)      - Radiotherapy-specific: beams, machines, contours
```

### The DRR Renderer: Bridging Visualization and Dose Calculation

**Key Discovery:** The DRR (Digitally Reconstructed Radiograph) renderer in `VSIM_OGL/DRRRenderer.cpp` uses the **same ray-tracing algorithm** as TERMA calculation, but for visualization:

| DRR Renderer | TERMA Calculator |
|--------------|------------------|
| Synthetic X-ray image | Dose deposition volume |
| Sum CT values along ray | Integrate attenuation along ray |
| 2D output (image) | 3D output (TERMA volume) |
| Fixed-point arithmetic (fast) | Floating-point (precise) |

**The `ClipRaster()` function** in DRRRenderer.cpp performs the same plane-crossing voxel traversal as the Fortran `make_vector()`:

```cpp
// DRRRenderer.cpp - ClipRaster()
// Uses 16.16 fixed-point for speed
nPixelValue += pppVoxels[viStart[2] >> 16][viStart[1] >> 16][viStart[0] >> 16];
viStart += viStep;
```

The DRR files were removed ~2001 when the VSim prototype transitioned to Siemens' proprietary codebase. They have been **restored from commit 40e1350a** (June 7, 2002) as historical reference:
- `VSIM_OGL/DRRRenderer.cpp` / `.h`
- `GEOM_VIEW/DRRRenderable.cpp` / `include/DRRRenderable.h`

---

## Part 3: Optimization Framework Development (2003-2006)

### PenBeamEdit (~2004)

Before the full optimization system existed, `PenBeamEdit` provided a standalone tool for:
- Visualizing beamlet intensity maps
- Manual editing of beamlet weights
- Testing TERMA and convolution calculations

This served as a **testbed** for the dose calculation pipeline.

### Core Algorithm Crystallization (2003-2006)

The variational inverse planning algorithm took shape:

1. **Objective Function:** Minimize KL divergence between calculated and target DVHs
2. **Optimization:** Conjugate gradient with adaptive variance
3. **Multi-scale:** Coarse-to-fine pyramid (4 levels: 8mm → 0.5mm)
4. **Parameterization:** Sigmoid transform for bounded beamlet weights

**April 2006: CVS/Git Migration**

The first git commits (April 1-2, 2006) represent migration from CVS, not project start:
- 41 commits in 2 days
- Thread-safety fixes for multi-core optimization
- VS2005 secure CRT migration
- 4-level pyramid finalized

---

## Part 4: Production System (2006-2009)

### RtModel Library

The production dose calculation lives in `RtModel/`:

| File | CVS Rev | Function |
|------|---------|----------|
| `BeamDoseCalc.cpp` | r602 | TERMA ray-tracing |
| `SphereConvolve.cpp` | - | Kernel convolution |
| `KLDivTerm.cpp` | r647 | KL divergence computation |
| `ConjGradOptimizer.cpp` | r644 | Conjugate gradient optimizer |
| `PlanPyramid.cpp` | r647 | Multi-scale optimization |

**Key improvement over DivFluence:** `BeamDoseCalc::TraceRayTerma()` uses **trilinear interpolation** with neighborhood updates, vs. nearest-neighbor in the original:

```cpp
// BeamDoseCalc.cpp - TraceRayTerma()
REAL fluenceInc = fluence0 * exp(-mu * path) * mu * deltaPath;
UpdateTermaNeighborhood(nNdx, weights, fluenceInc);  // Trilinear splatting
```

### Brimstone Application

MFC-based GUI integrating all components:
- DICOM CT import
- Structure (ROI) definition
- Prescription/constraint setup
- Multi-threaded optimization
- Real-time DVH visualization

---

## Part 5: Algorithm Lineage Summary

```
1988  Wisconsin Fortran (Rock Mackie)
      │
      │   ray_trace_set_up.for
      │   make_vector.for
      │   new_sphere_convolve.for
      │
      ▼
~2000 DivFluence (C++ translation)
      │
      │   div_fluence_calc()      ─────────────────┐
      │   ray_trace_set_up()                       │
      │   new_sphere_convolve()                    │
      │                                            │
      ▼                                            │ Same algorithm,
2001  DRRRenderer.cpp (visualization variant)      │ different purpose
      │                                            │
      │   ClipRaster() - fixed-point ray-trace     │
      │   ComputeDRR() - accumulate CT values      │
      │                                            │
      ▼                                            │
2004  PenBeamEdit (beamlet editing testbed)        │
      │                                            │
      ▼                                            │
2008  RtModel/BeamDoseCalc.cpp (production)  ◄─────┘
      │
      │   TraceRayTerma() - floating-point
      │   TrilinearInterpDensity() - interpolation
      │   UpdateTermaNeighborhood() - splatting
      │
      ▼
2025  python/read_process_series.py (ITK port)
```

---

## Part 6: Later Development (2010-2025)

### Maintenance Phase (2010-2015)
- **2010:** Code reorganization, moved files to RtModel
- **2014:** ITK 4.3 upgrade, VS2013 support
- **2015:** Open source publication on GitHub, BSD license

### Machine Learning Experiments (2019-2020)
- TensorFlow 2.0 exploration of variational approaches
- `entropy_max` notebooks investigating entropy gradients
- Connection to variational Bayes formalization

### Modern Era (2025)
- **January:** Python/ITK port of TERMA calculation
- **October:** Repository consolidation (pheonixrt merge)
- **October:** Variational Bayes documentation, optional free energy calculation
- **November:** Docker builds for Fortran code, EGSnrc integration

---

## Key Insights

### 1. The Algorithm is Older Than the Repository
The git history starts in 2006, but the algorithm dates to 1988 (Fortran) with C++ translation around 2000.

### 2. Visualization and Dose Calculation Share Common Roots
DRRRenderer (2001) and BeamDoseCalc (2008) use the same ray-tracing technique. The DRR code is the "missing link" showing how visualization expertise informed dose calculation.

### 3. Commercial Origins Explain the Architecture
The Siemens VSim prototype work (2001) drove professional-grade visualization that seems over-engineered for a research project.

### 4. DivFluence is the Rosetta Stone
`DivFluence/OrigDivFluence.cpp` is a direct C++ translation of the Fortran, making it the key reference for understanding the algorithm's structure.

---

## Intellectual Property

- **U.S. Patent 7,369,645** - Radiotherapy treatment planning
- **Copyright:** 2000-2021, Derek G. Lane
- **License:** Modified BSD with Hippocratic and Anti-996 clauses
- **Company:** 2nd Messenger Systems

---

## Source Attribution

This history is based on:
1. Git commit history (2006-2025)
2. CVS $Id$ tags in source files (r602-r647, 2008-2009)
3. Copyright headers in source files (2000-2002)
4. Oral history from the original developer
5. Fortran code comments (Rock Mackie, 1988)

---

*Document revised: November 27, 2025*
