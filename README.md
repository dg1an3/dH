# dH - Radiotherapy Treatment Planning System

Variational inverse planning algorithm for radiotherapy treatment planning.
**Brimstone**: A variational inverse planning algorithm for [radiotherapy treatment planning](https://en.wikipedia.org/wiki/Radiation_treatment_planning).

It is related to variational Bayes methods, though free energy is implicitly represented.

## Overview

The Brimstone algorithm optimizes radiation beam intensities (beamlet weights) to deliver prescribed doses to tumor targets while minimizing exposure to healthy tissue. It employs:

- **Multi-scale pyramid optimization** for robust convergence
- **KL-divergence minimization** for dose-volume histogram (DVH) matching
- **Adaptive covariance optimization** with conjugate gradient methods
- **Implicit free energy representation** related to variational Bayes methods

## Key Features

- **Mathematically principled**: Information-theoretic cost functions (KL-divergence)
- **Robust optimization**: Multi-scale approach avoids local minima
- **Flexible prescriptions**: Arbitrary target DVH shapes supported
- **Python wrapper**: Modern interface via `pybrimstone` package
- **C++ core**: High-performance ITK-based implementation

## Quick Start

### Building the Modern Stack (Production)

```bash
# Open the main solution in Visual Studio
start Brimstone_src.sln

# Or build from command line
msbuild Brimstone_src.sln /p:Configuration=Release /p:Platform=x64
```

## Repository Organization

This repository has been reorganized (2025-10-31) to separate modern and classic code:

```
dH/
├── Brimstone_src.sln       → Main production solution
├── src/                    → Modern production code
├── src_classic/            → Legacy code (reference)
├── lib_classic/            → Legacy libraries
├── docs/                   → Documentation
└── REORGANIZATION.md       → Detailed reorganization notes
```

### Modern Stack (src/)
- **Brimstone** - Main MFC GUI application
- **RtModel** - Self-contained RT optimization library
- **Graph** - DVH visualization
- **GenImaging** - ITK wrapper utilities
- **OptimizeN** - N-dimensional optimization

### Classic Stack (src_classic/ + lib_classic/)
- **Brimstone_original** - Legacy application
- **RT_MODEL** - Original modular RT library
- **Graph_original** - Legacy visualization
- **MTL, GEOM_MODEL, XMLLogging** - Foundation libraries

## Documentation

See [docs/](docs/) directory:
- [CLAUDE.md](docs/CLAUDE.md) - Project instructions for Claude Code
- [repository_structure.md](docs/repository_structure.md) - Structure analysis
- [brimstone_comparison.md](docs/brimstone_comparison.md) - Modern vs classic comparison
- [brimstone_dependency_evolution.md](docs/brimstone_dependency_evolution.md) - Historical analysis
- [proposed_reorganization.md](docs/proposed_reorganization.md) - Reorganization plan

## Key Components

### RtModel Library
Core optimization library implementing variational Bayes approach:
- Multi-scale coarse-to-fine optimization (4 pyramid levels)
- KL divergence minimization for dose-volume histogram matching
- Conjugate gradient with dynamic covariance
- Optional explicit free energy calculation

See [docs/CLAUDE.md](docs/CLAUDE.md) for detailed architecture.

### Dependencies
- **ITK** (Insight Toolkit) - Medical image processing
- **VNL** (Vision-something-Numerics) - Linear algebra (via ITK)
- **MFC** (Microsoft Foundation Classes) - GUI framework
- **DCMTK** - DICOM toolkit
- **Intel IPP** - Performance primitives (optional)

## Building

### Requirements
- Visual Studio 2022 (toolset v143) or compatible
- ITK 4.3+
- DCMTK
- Intel IPP (optional)

### Configuration
Set environment variables:
- `ITK_DIR` - ITK installation directory
- `ITK_BUILD_DIR` - ITK build directory
- `DCMTKDIR` or `DCMTK_DIR` - DCMTK installation
- `DCMTK_BUILD_DIR` - DCMTK build directory (if separate)

### Build Steps
1. Open `Brimstone_src.sln` in Visual Studio
2. Select configuration (Debug/Release) and platform (Win32/x64)
3. Build solution

## Project Status

### Current State
- ✅ Repository reorganized with clear separation
- ✅ Modern stack in src/ subdirectory
- ⏳ Build verification in progress
- ⏳ Original directories pending cleanup

### Recent Changes (2025-10-31)
- Reorganized into src/, src_classic/, lib_classic/ structure
- Updated Brimstone_src.sln to reference src/ subdirectories
- Moved documentation to docs/
- Added GenImaging and OptimizeN to modern stack (per user request)

See [REORGANIZATION.md](REORGANIZATION.md) for complete details.

## License

U.S. Patent 7,369,645

Copyright (c) 2007-2021, Derek G. Lane. All rights reserved.

See LICENSE file for full terms.

## Contact

For issues or questions, see project documentation in docs/ directory.
1. **KL Divergence Minimization** (`RtModel/KLDivTerm.cpp`)
   - Minimizes `KL(P_calc || P_target)` between calculated and target dose-volume histograms (DVHs)
   - This is the fundamental operation in variational inference, seeking the best approximation to a target distribution

2. **Implicit Free Energy**
   - The objective function implicitly minimizes variational free energy: `F = KL(q||p) + Expected log likelihood`
   - Implemented as weighted sum of KL divergence terms across anatomical structures
   - Unlike explicit variational Bayes, free energy is not directly computed but emerges from the optimization

3. **Gaussian Approximation** (`RtModel/include/HistogramGradient.h`)
   - Dose histograms are convolved with adaptive Gaussian kernels
   - Similar to mean-field approximation in variational Bayes
   - Variance parameters represent posterior uncertainty in the dose calculation

4. **Adaptive Variance** (`RtModel/Prescription.cpp`)
   - Dynamic covariance optimization adjusts uncertainty representation during optimization
   - Acts as variational parameter analogous to posterior variance in Bayesian inference
   - Variance scaling uses sigmoid derivatives: `actVar = baseVar * dSigmoid(input)² * varWeight²`

5. **Hierarchical Structure** (`RtModel/include/PlanPyramid.h`)
   - Multi-scale pyramid (4 levels) provides coarse-to-fine optimization
   - Similar to hierarchical variational models without full hierarchical Bayes

### Explicit Free Energy Calculation (Optional)

An optional explicit free energy calculation has been implemented (`RtModel/ConjGradOptimizer.cpp:220-254`):

**Enable via:** `optimizer.SetComputeFreeEnergy(true)`

**Calculation Method:**
1. **Entropy from Covariance**: Computes differential entropy from the dynamically-built covariance matrix
   ```
   H = 0.5 * (n * log(2πe) + log(det(Σ)))
   ```
2. **Free Energy**: Combines KL divergence objective with entropy
   ```
   F = KL_divergence - Entropy
   ```

### Mathematical Formulation

```
minimize: Σ_structures [ w_i * KL(P_calc_i || P_target_i) ]
subject to: 0 ≤ beamlet_weight_j ≤ max_weight (via sigmoid transform)
```

## Project Structure

```
dH/
├── Brimstone/          # C++ MFC application
├── RtModel/            # Core radiotherapy models and algorithms
├── VecMat/             # Vector and matrix utilities
├── Graph/              # Visualization components
├── GenImaging/         # Generic imaging utilities
├── python/             # Python bindings (pybrimstone)
│   ├── pybrimstone/    # Python package
│   ├── tests/          # Unit tests and reference implementations
│   └── examples/       # Usage examples
├── notebook_zoo/       # Jupyter notebooks for research
├── docs/               # Additional documentation
├── GEOM_BASE/          # Geometry base library
├── GEOM_MODEL/         # Geometry models
├── GEOM_VIEW/          # Geometry view
├── MODEL_BASE/         # Base model library
├── EGSnrc/             # EGSnrc Monte Carlo interface
├── DivFluence/         # Divergent fluence calculation
├── FieldCOM/           # Field center-of-mass
├── PenBeam_indens/     # Pencil beam in-density
├── PenBeamEdit/        # Pencil beam editor
├── OptimizeN/          # N-dimensional optimizer
├── CLAUDE.md           # Detailed development guidance
└── CYTHON_WRAPPER_DESIGN.md  # Python wrapper design
```

## Getting Started

### Python Interface (Recommended)

```bash
cd python
pip install -e .
```

See [python/README.md](python/README.md) for detailed installation and usage instructions.

**Quick example:**
```python
import pybrimstone as pb

# Create treatment plan
plan = pb.Plan()
plan.add_beam(pb.Beam(gantry_angle=0.0))
plan.add_beam(pb.Beam(gantry_angle=180.0))

# Optimize
optimizer = pb.PlanOptimizer(plan)
result = optimizer.optimize()
```

### C++ Implementation

The C++ implementation requires:
- Visual Studio 2010 or later (Windows)
- ITK (Insight Toolkit) library
- MFC (Microsoft Foundation Classes)

Build using `Brimstone_src.sln` in Visual Studio.

## Documentation

- **[CLAUDE.md](CLAUDE.md)** - Development guidance and architecture
- **[python/README.md](python/README.md)** - Python wrapper documentation and examples
- **[CYTHON_WRAPPER_DESIGN.md](CYTHON_WRAPPER_DESIGN.md)** - Python binding architecture
- **[docs/](docs/)** - Additional technical documents and research notes

## Algorithm Details

Brimstone uses a multi-level optimization approach:

1. **Hierarchical pyramid**: Optimization proceeds from coarse to fine resolution
2. **Cost function**: KL-divergence between target and calculated DVHs
3. **Optimizer**: Polak-Ribiere conjugate gradient with Brent line search
4. **Adaptive variance**: Dynamic covariance adjustment during search

For complete technical details, see [CLAUDE.md](CLAUDE.md).

## License

**[-MIND THE LICENSE-](LICENSE)**

U.S. Patent 7,369,645

Copyright (c) 2007-2021, Derek G. Lane
All rights reserved.

This software is proprietary. See [LICENSE](LICENSE) file for terms.

## References

- [Radiation Treatment Planning (Wikipedia)](https://en.wikipedia.org/wiki/Radiation_treatment_planning)
- [Variational Bayes Methods](https://en.wikipedia.org/wiki/Variational_Bayesian_methods)
- [ITK - Insight Toolkit](https://itk.org/)
