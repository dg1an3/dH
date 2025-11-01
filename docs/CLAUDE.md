# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**brimstone** is a variational inverse planning algorithm for radiotherapy treatment planning. It implements a simplistic variational Bayes approach where KL divergence minimization between calculated and target dose-volume histograms (DVHs) acts as the core optimization objective.

The system consists of:
- **RtModel**: Core C++ library implementing the optimization algorithm, dose calculation, and data models
- **Brimstone**: MFC-based Windows GUI application for visualization and user interaction
- **Python utilities**: ITK-based scripts for processing CT density volumes

## Building the Project

This is a Visual Studio C++ project using the `.sln` (solution) format.

### Main Solution
```bash
# Open the main solution in Visual Studio
start Brimstone_src.sln
```

The solution contains three projects:
- **RtModel** (library) - Core optimization and modeling
- **Brimstone** (executable) - GUI application
- **Graph** (library) - Visualization components

### Build Configurations
- Debug|Win32, Debug|x64
- Release|Win32, Release|x64
- MinSizeRel, RelWithDebInfo variants available

### Dependencies
- **MFC (Microsoft Foundation Classes)** - GUI framework
- **VNL (Vision-something-Numerics Library)** - Numerical optimization
- **ITK (Insight Toolkit)** - Medical image processing (for Python utilities)
- **ATL (Active Template Library)** - Collection classes

The codebase targets Windows XP or later (WINVER 0x0501).

## Architecture

### RtModel Library (Core Algorithm)

The optimization follows a **multi-scale coarse-to-fine** approach:

```
PlanOptimizer
  └─ PlanPyramid (4 levels: 8.0mm → 3.2mm → 1.3mm → 0.5mm voxels)
      └─ DynamicCovarianceOptimizer (Conjugate Gradient)
          └─ Prescription (Objective Function)
              └─ VOITerm[] (per-structure constraints)
                  └─ KLDivTerm (KL divergence calculation)
                      └─ HistogramGradient (DVH + derivatives)
```

#### Key Classes

**Data Models:**
- `Series` - CT density volume + anatomical structures
- `Plan` - Treatment plan with beams, dose matrix, histograms
- `Structure` - ROI (region of interest) with multi-scale contours
- `Beam` - Treatment beam with pencil beamlets and intensity map

**Optimization:**
- `DynamicCovarianceOptimizer` (`RtModel/ConjGradOptimizer.cpp`) - Conjugate gradient with adaptive variance and optional free energy calculation
- `Prescription` (`RtModel/Prescription.cpp`) - Objective function coordinating weighted sum of structure terms
- `KLDivTerm` (`RtModel/KLDivTerm.cpp`) - Minimizes `KL(P_calc || P_target)` between calculated and target DVHs
- `VOITerm` - Base class for structure-specific optimization constraints

**Dose Calculation:**
- `BeamDoseCalc` - Pencil beam TERMA calculation via ray-tracing
- `SphereConvolve` - Energy kernel convolution using spherical coordinate LUTs
- `EnergyDepKernel` - Pre-computed energy deposition kernel (see `*.dat` files in Brimstone/)

**Histogram Processing:**
- `Histogram` - Dose-volume histogram with adaptive Gaussian convolution
- `HistogramGradient` - Extends histogram with partial derivative volumes for gradient computation

#### Variational Bayes Connection

The algorithm implements implicit free energy minimization:
- KL divergence terms approximate expected log-likelihood
- Adaptive variance acts as variational posterior parameters
- Gaussian histogram convolution ≈ mean-field approximation
- Sigmoid parameterization: maps unbounded optimizer variables to bounded beamlet weights [0, max]

**Optional Explicit Free Energy:**
Enable via `optimizer.SetComputeFreeEnergy(true)` to compute:
```
F = KL_divergence - Entropy
Entropy = 0.5 * (n * log(2πe) + log(det(Σ)))
```
where Σ is the covariance matrix built from orthogonalized conjugate gradient search directions.

#### Multi-Scale Pyramid

Four levels provide coarse-to-fine optimization:
1. **Level 3** (coarsest): 8.0mm voxels - quick convergence on overall plan structure
2. **Level 2**: 3.2mm voxels
3. **Level 1**: 1.3mm voxels
4. **Level 0** (finest): 0.5mm voxels - final high-resolution optimization

Optimized weights are transferred between levels using inverse filtering (`InvFiltIntensityMap`).

### Brimstone Application (GUI)

MFC Single Document Interface (SDI) pattern:
- `CBrimstoneApp` (`Brimstone.cpp`) - Application initialization
- `CBrimstoneDoc` (`BrimstoneDoc.cpp`) - Owns Series and Plan objects, handles serialization
- `CBrimstoneView` (`BrimstoneView.cpp`) - Renders CT images, contours, dose overlays
- `CMainFrame` (`MainFrm.cpp`) - Main window frame
- `OptThread` (`OptThread.cpp`) - Asynchronous optimization thread
- `OptimizerDashboard` - Real-time visualization of optimization progress

**Key Operations:**
- DICOM CT import via `SeriesDicomImporter`
- Structure (ROI) definition and management
- Prescription/constraint setup via dialog boxes
- Beamlet generation command
- Multi-threaded optimization launch
- DVH graph display

## Important File Locations

**Core Optimization Algorithm:**
- `RtModel/ConjGradOptimizer.h/cpp` - Conjugate gradient optimizer (lines 220-254: free energy calculation)
- `RtModel/Prescription.h/cpp` - Objective function evaluation
- `RtModel/KLDivTerm.h/cpp` - KL divergence computation
- `RtModel/include/HistogramGradient.h` - Histogram derivatives

**Dose Calculation:**
- `RtModel/BeamDoseCalc.h/cpp` - TERMA ray-tracing
- `RtModel/SphereConvolve.h/cpp` - Energy kernel convolution

**Multi-Scale Structure:**
- `RtModel/include/PlanPyramid.h` - Pyramid management (4 levels)
- `RtModel/PlanOptimizer.h/cpp` - High-level optimization orchestration

**Energy Kernels:**
- `Brimstone/6MV_kernel.dat` - 6 MV photon beam kernel
- `Brimstone/15MV_kernel.dat` - 15 MV photon beam kernel
- `Brimstone/2MV_kernel.dat` - 2 MV photon beam kernel

## Development Guidelines

### Extending the Optimizer

**Adding a new constraint type:**
1. Create a subclass of `VOITerm` in RtModel
2. Implement `Eval()` method to compute cost and gradient
3. Register with `Prescription::AddTerm()`

**Modifying optimization parameters:**
- Pyramid level sigmas: `DEFAULT_LEVELSIGMA[]` in PlanPyramid
- Convergence tolerances: `CG_TOLERANCE[]` per pyramid level
- Histogram kernel width: `GBINS_KERNEL_WIDTH` in Histogram
- Adaptive variance: Controlled by `DynamicCovarianceOptimizer::m_vAdaptVariance`

**Adaptive variance formula:**
```cpp
m_ActualAV = baseVar * dSigmoid(input)² * varWeight²
```

### Working with Dose Calculation

Dose calculation follows this pipeline:
1. **TERMA calculation** - Ray-trace through CT density (`BeamDoseCalc`)
2. **Energy convolution** - Apply spherical kernel (`SphereConvolve`)
3. **Beam accumulation** - Sum weighted beamlet contributions
4. **Histogram formation** - Bin dose values by structure region

To modify dose calculation:
- Adjust attenuation coefficients in `BeamDoseCalc`
- Modify energy kernel loading in `EnergyDepKernel`
- Change convolution strategy in `SphereConvolve`

### Gradient Computation

Gradients flow via chain rule:
```
optimizer_vars → sigmoid → beamlet_weights → dose → histogram_bins → KL_divergence
```

Each stage stores partial derivatives:
- `Prescription::dTransform()` - sigmoid derivative
- `HistogramGradient::m_arr_dVolumes[]` - dose-to-histogram partials
- `KLDivTerm::Eval()` - KL-to-histogram partials

### Python Utilities

The `python/read_process_series.py` script demonstrates:
- CT to mass density conversion (`ct_to_md_values`)
- Beam-specific density rotation (`rotate_density_for_beam`)
- TERMA calculation framework (`terma_from_density`)

Uses ITK for image processing. Run with:
```bash
python python/read_process_series.py
```

## Legal

U.S. Patent 7,369,645

Copyright (c) 2007-2021, Derek G. Lane. All rights reserved.

See LICENSE file for full terms.
