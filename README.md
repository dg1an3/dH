# brimstone 🜏

brimstone is a variational inverse planning algorithm for [radiotherapy treatment planning](https://en.wikipedia.org/wiki/Radiation_treatment_planning).  It is related to variational bayes methods, though free energy is implicitly represented.

## Variational Bayes Connection

The dH algorithm implements a simplistic variational Bayes approach for treatment planning optimization. The connection manifests through several key mechanisms:

### Core Variational Elements

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

### Simplifications

The algorithm is "simplistic" in that it:
- Does not explicitly model posterior distributions
- Uses sigmoid-transformed parameters rather than full probabilistic representation
- By default computes implicit rather than explicit free energy (though explicit calculation is available as an option)
- Focuses on point estimates rather than full posterior inference

### Explicit Free Energy Calculation (Optional)

An optional explicit free energy calculation has been implemented (`RtModel/ConjGradOptimizer.cpp:220-254`):

**Enable via:** `optimizer.SetComputeFreeEnergy(true)`

**Calculation Method:**
1. **Entropy from Covariance**: Computes differential entropy from the dynamically-built covariance matrix
   ```
   H = 0.5 * (n * log(2πe) + log(det(Σ)))
   ```
   - Uses eigenvalue decomposition for numerical stability
   - No Hessian approximation required - uses existing search-direction-based covariance

2. **Free Energy**: Combines KL divergence objective with entropy
   ```
   F = KL_divergence - Entropy
   ```
   - KL divergence represents expected log likelihood term
   - Entropy term accounts for posterior uncertainty
   - Both terms logged during optimization iterations

This implementation leverages the existing `DynamicCovarianceOptimizer` covariance approximation (built from orthogonalized conjugate gradient search directions) rather than requiring expensive Hessian computation.

### Mathematical Formulation

The optimization problem solved is:

```
minimize: Σ_structures [ w_i * KL(P_calc_i || P_target_i) ]
subject to: 0 ≤ beamlet_weight_j ≤ max_weight (via sigmoid transform)
```

This is fundamentally a variational inference problem where the algorithm seeks optimal beamlet weights that produce dose distributions matching target specifications in an information-theoretic sense.

## Building with CMake

### Prerequisites

- CMake 3.16 or newer
- ITK (Insight Segmentation and Registration Toolkit) - required for main Brimstone system
- For Windows: Visual Studio with MFC support (for Brimstone GUI application)
- C++17 compatible compiler

### Quick Start - Main Production System

To build the main Brimstone production system (RtModel, Graph, and Brimstone GUI):

1. **Configure the build:**
   ```bash
   mkdir build
   cd build
   cmake .. -DITK_DIR=/path/to/ITK/build
   ```

2. **Build the project:**
   ```bash
   cmake --build . --config Release
   ```

3. **Install (optional):**
   ```bash
   cmake --install . --prefix /path/to/install
   ```

### Build Options

The CMake build system is modular and supports building different components:

- `BUILD_BRIMSTONE_APP` - Build the Brimstone GUI application (default: ON, requires MFC on Windows)
  - When enabled, builds: RtModel, Graph, and Brimstone
- `BUILD_FOUNDATION_LIBS` - Build foundation libraries like MTL, FTL (default: OFF)
- `BUILD_COMPONENT_LIBS` - Build component libraries like GEOM_MODEL, OPTIMIZER_BASE (default: OFF)
- `BUILD_TESTS` - Build test applications (default: OFF)
- `BUILD_SHARED_LIBS` - Build shared libraries instead of static (default: OFF)

### Advanced Build Examples

**Build only the core libraries (no GUI):**
```bash
cmake .. -DBUILD_BRIMSTONE_APP=OFF -DITK_DIR=/path/to/ITK/build
cmake --build . --config Release
```

**Build with foundation and component libraries:**
```bash
cmake .. -DBUILD_FOUNDATION_LIBS=ON -DBUILD_COMPONENT_LIBS=ON -DITK_DIR=/path/to/ITK/build
cmake --build . --config Release
```

**Build with tests:**
```bash
cmake .. -DBUILD_TESTS=ON -DITK_DIR=/path/to/ITK/build
cmake --build . --config Release
```

### Project Structure

The build system supports multiple configurations:

**Main Production System (default):**
- **RtModel** - Core optimization and dose calculation library
- **Graph** - Visualization and DVH graph library  
- **Brimstone** - MFC-based GUI application (Windows only)

**Foundation Libraries (optional):**
- MTL - Math Template Library
- FTL - Foundation Template Library

**Component Libraries (optional):**
- GEOM_MODEL - Geometric modeling
- OPTIMIZER_BASE - Optimization framework
- XMLLogging - XML logging utilities

### Legacy Build System

⚠️ **IMPORTANT:** The original Visual Studio solution files are **deprecated** and maintained only for backward compatibility.

- **Brimstone_src.sln** - Main production system (deprecated)
- Other .sln files - Component libraries and tests (deprecated)

See [DEPRECATION.md](DEPRECATION.md) for migration timeline and details.

**CMake is now the recommended and officially supported build system.**

For comprehensive build instructions, see [BUILDING.md](BUILDING.md).

[-MIND THE LICENSE-](https://raw.githubusercontent.com/dg1an3/pheonixrt/master/LICENSE)

U. S. Patent 7,369,645

Copyright (c) 2007-2021, Derek G. Lane All rights reserved.


* documents
  - see zebrastack
  - frame notes
  - autoencoder, mdl, free energy
  - em free energy
  - free energy and the brain
  - variational bayes inverse planning
* notebook_zoo
  - entropy_max
* diy_ml
  - pytorch tutorial
  - CMatrixNxM
