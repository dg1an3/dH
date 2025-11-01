# Brimstone Dependency Evolution - Historical Analysis

## Summary

The **modern Brimstone** application underwent a **major dependency simplification** between the initial commit and the ITK 4.3 upgrade (commit 44ee23b). The application went from depending on **5 internal libraries** down to just **2 internal libraries**.

## Dependency Timeline

### Phase 1: Initial Version (Commit 9e47480 - "initial update")

**Project Format:** `.vcproj` (Visual Studio 2005/2008)

**Internal Library Dependencies (5):**
1. **RtModel** - Core radiotherapy model
2. **Graph** - DVH visualization
3. **RtOptimizer** - Optimization algorithms (REMOVED)
4. **OptimizeN** - N-dimensional optimizer (REMOVED)
5. **GenImaging** - Generic imaging utilities (REMOVED)

**Include Paths:**
```
..\GenImaging\include
..\Graph\include
..\RtOptimizer\include
..\RtModel\include
..\OptimizeN\include
```

**External Dependencies:**
- DCMTK (DICOM toolkit)
- ITK 3.x (Code\Algorithms, Code\BasicFilters, Code\Common)
- VNL (via ITK\Utilities\vxl)
- Intel IPP (Performance Primitives)
- MFC

**Link Libraries:**
```
ITKAlgorithms.lib
ITKBasicFilters.lib
ITKCommon.lib
itkvnl_inst.lib
itkvnl_algo.lib
itkv3p_netlib.lib
itkvnl.lib
itkvcl.lib
itksys.lib
```

### Phase 2: Major Refactoring (Commit 44ee23b - "Upgrade to ITK 4.3")

**Commit Message:**
> * Upgrade to ITK 4.3
> * Changes to support the ITK pointer model (better memory management)
> * Switch to support for Visual Studio 2013 Express
> * Still have dependency on Intel Performance Primitives, unfortunately

**Project Format:** `.vcxproj` (Visual Studio 2010+)

**Internal Library Dependencies (2 - DOWN FROM 5):**
1. **RtModel** - Core radiotherapy model (RETAINED)
2. **Graph** - DVH visualization (RETAINED)

**REMOVED DEPENDENCIES:**
- ❌ **RtOptimizer** - Eliminated
- ❌ **OptimizeN** - Eliminated
- ❌ **GenImaging** - Eliminated

**Include Paths (Simplified):**
```
..\Graph\include
..\RtModel\include
```

**External Dependencies (ITK 4.3):**
- DCMTK (unchanged)
- ITK 4.3 with modular structure:
  - Modules\Core\SpatialObjects
  - Modules\Numerics\Statistics
  - Modules\Core\Transform
  - Modules\Registration\Common
  - Modules\Filtering\* (Path, ImageCompose, ImageGrid, Smoothing, ImageFilterBase)
  - Modules\Core\ImageFunction
  - Modules\Core\Common
  - Modules\IO\ImageBase
  - Modules\IO\XML
  - Modules\ThirdParty\VNL (still using ITK's embedded VNL)
  - Modules\ThirdParty\KWSys
- Intel IPP (still used)
- MFC

**Link Libraries (ITK 4.3 naming):**
```
ITKCommon-4.3.lib
itkvnl_algo-4.3.lib
itkv3p_netlib-4.3.lib
itkvnl-4.3.lib
itkvcl-4.3.lib
itksys-4.3.lib
```

### Phase 3: Current Version (Commit fc4bedc - "updating toolset")

**Project Format:** `.vcxproj` (Visual Studio 2022, toolset v143)

**Internal Library Dependencies:** Still 2 (unchanged from Phase 2)
1. **RtModel**
2. **Graph**

**Include Paths:** Same as Phase 2
```
..\Graph\include
..\RtModel\include
```

**Changes:**
- Updated to Visual Studio 2022 toolset (v143)
- Modern PlatformToolset settings
- Still uses ITK 4.3+ structure
- Still uses Intel IPP

## What Happened to the Removed Libraries?

### 1. RtOptimizer (REMOVED)
**Status:** Deleted from repository
**Reason:** Functionality absorbed into RtModel
**Evidence:** No git history found, suggests deleted before initial commit to dH repo

### 2. OptimizeN (REMOVED from Brimstone dependencies)
**Status:** Still exists in repository at `OptimizeN/`
**Current Use:** Only used by **Brimstone_original** (classic stack)
**Reason:** Optimization code was copied/embedded into RtModel
**Evidence:**
- `RtModel/ConjGradOptimizer.h/cpp` contains conjugate gradient optimizer
- `RtModel/ObjectiveFunction.h` contains objective function interface
- These are standalone implementations in RtModel

### 3. GenImaging (REMOVED from Brimstone dependencies)
**Status:** Still exists in repository at `GenImaging/`
**Current Use:** Not actively used by either Brimstone version
**Reason:** ITK 4.3's improved filter architecture made wrapper unnecessary
**Evidence:** Directory exists but not referenced in any active solution

## The Architectural Shift

### Before (Phase 1): Layered Architecture
```
Brimstone (application)
    ├─→ Graph (visualization)
    ├─→ RtModel (data structures)
    ├─→ RtOptimizer (optimization algorithms)
    ├─→ OptimizeN (generic N-D optimization)
    └─→ GenImaging (ITK wrappers)
```

**Philosophy:** Separation of concerns with modular libraries
- Generic optimization in OptimizeN
- RT-specific optimization in RtOptimizer
- Imaging utilities in GenImaging
- Clear interfaces between layers

**Problems:**
- Complex build dependencies
- Multiple library versions to maintain
- Harder to understand data flow
- More opportunities for version conflicts

### After (Phase 2-3): Self-Contained Architecture
```
Brimstone (application)
    ├─→ RtModel (self-contained: data + optimization + algorithms)
    │   └─→ Embedded: ConjGradOptimizer, ObjectiveFunction, math utilities
    └─→ Graph (visualization)
        └─→ RtModel (for data structures)
```

**Philosophy:** Self-contained library with embedded algorithms
- RtModel contains everything it needs
- Uses ITK's VNL directly for linear algebra
- No intermediate abstraction layers
- Simpler dependency graph

**Benefits:**
- ✅ Simpler build process (2 internal libs vs 5)
- ✅ Easier to understand and maintain
- ✅ No version conflicts between optimization libraries
- ✅ Self-contained RtModel can be reused independently
- ✅ ITK 4.3's improved architecture eliminated need for wrappers

## Code Consolidation Details

### Optimizer Code Migration

**From OptimizeN:** → **To RtModel:**
- `OptimizeN/ConjGradOptimizer.h/cpp` → `RtModel/ConjGradOptimizer.h/cpp`
- `OptimizeN/ObjectiveFunction.h` → `RtModel/ObjectiveFunction.h`
- Generic N-dimensional optimizer → Specialized for RT dose optimization

**Key Changes in RtModel's ConjGradOptimizer:**
- Adapted for RT-specific use case (beamlet weight optimization)
- Added dynamic covariance computation
- Added optional free energy calculation (variational Bayes)
- Specialized for sigmoid-parameterized variables
- Lines 220-254 in `RtModel/ConjGradOptimizer.cpp`: explicit free energy calculation

### Math Library Migration

**From MTL/OptimizeN:** → **To RtModel:**
- Vector operations → `RtModel/include/VectorN.h`, `RtModel/include/VectorOps.h`
- Matrix operations → `RtModel/include/MatrixNxM.h`
- Math utilities → `RtModel/include/MathUtil.h`

**Why Duplicate Instead of Link:**
- RtModel became **completely self-contained**
- No dependency on MTL (which is only used by classic stack)
- Can compile RtModel independently
- Uses ITK's VNL as primary linear algebra backend

### Imaging Code Changes

**GenImaging Eliminated:**
- ITK 4.3's modular architecture provided better abstractions
- Direct ITK filter use replaced GenImaging wrappers
- `Modules\Filtering\*` includes provide needed functionality
- Simpler to use ITK directly than maintain wrapper layer

## Comparison with Classic Stack

### Brimstone_original (Classic Stack) - Still Uses Old Architecture

**Internal Dependencies (7+):**
1. RT_MODEL (old version)
2. Graph_original (old version)
3. DicomImEx
4. GEOM_MODEL
5. MTL
6. OptimizeN (OPTIMIZER_BASE)
7. XMLLogging

**Include Paths:**
```
..\Graph\include
..\RT_MODEL\include
..\RT_MODEL\DicomImEx
..\OptimizeN\include
..\GEOM_MODEL\include
..\MTL
..\XMLLogging
```

**Why Classic Stack Wasn't Refactored:**
- Frozen as legacy code
- Represents original modular architecture
- Kept for historical reference
- Modern Brimstone (with simplified RtModel) became production code

## Technical Insights

### 1. The Self-Contained Library Pattern

RtModel demonstrates a **"fat library"** pattern:
- Contains its own math utilities (VectorN, MatrixNxM, MathUtil)
- Contains its own optimization (ConjGradOptimizer, Prescription)
- Contains domain logic (Beam, Plan, Series, Structure)
- Contains algorithms (BeamDoseCalc, SphereConvolve, Histogram)

**Advantages:**
- Single library to link
- No version conflicts
- Easy to build and distribute
- Clear API boundary

**Trade-offs:**
- Code duplication with OptimizeN, MTL
- Larger library size
- Less code reuse across projects

### 2. ITK Version Impact

**ITK 3.x → 4.3 upgrade was critical:**
- Modular structure (`Modules\*` instead of `Code\*`)
- Better pointer model (SmartPointer improvements)
- Richer filter library eliminated need for GenImaging
- More mature VNL integration

**This upgrade enabled the simplification:**
- Could rely on ITK's expanded feature set
- No need for intermediate wrappers
- Direct use of ITK algorithms

### 3. Variational Bayes Specialization

The consolidation allowed RtModel to specialize for variational Bayes:
- `DynamicCovarianceOptimizer` extends `ConjGradOptimizer`
- Added covariance tracking from conjugate gradient directions
- Added explicit free energy calculation option
- Specialized for RT dose optimization problem

**This specialization wouldn't make sense in generic OptimizeN library**

## Files That Show the Evolution

### Git Commits to Examine:

1. **9e47480** - "initial update (sans cr-lf issues)"
   - Shows original 5-library architecture
   - `.vcproj` format

2. **44ee23b** - "Upgrade to ITK 4.3"
   - Shows simplified 2-library architecture
   - `.vcxproj` format
   - Major refactoring commit

3. **8c4a15f** - "removed unused files and projects"
   - Cleanup of obsolete code
   - Further simplification

4. **fc4bedc** - "updating toolset"
   - Current version
   - VS 2022 support

### Key Files to Compare:

**Brimstone Project Files:**
```bash
git show 9e47480:Brimstone/Brimstone.vcproj    # Initial (5 deps)
git show 44ee23b:Brimstone/Brimstone.vcxproj   # After refactor (2 deps)
git show HEAD:Brimstone/Brimstone.vcxproj      # Current (2 deps)
```

**RtModel Evolution:**
```bash
# Check when optimizer code was added to RtModel
git log --all -- RtModel/ConjGradOptimizer.cpp
git log --all -- RtModel/ObjectiveFunction.h
git log --all -- RtModel/include/VectorN.h
```

## Implications for Reorganization

### Historical Dependencies Show:

1. **Modern Brimstone started with more dependencies** (5 internal libs)
2. **Deliberate simplification occurred** during ITK 4.3 upgrade
3. **RtModel became intentionally self-contained** by design
4. **GenImaging, RtOptimizer were deprecated/deleted** (no longer needed)
5. **OptimizeN still exists** but only used by classic stack

### For Repository Reorganization:

**The libraries are NOT shared because:**
- Modern stack deliberately eliminated those dependencies
- Classic stack still uses old modular architecture
- The two stacks evolved in opposite directions:
  - Classic: Kept modular separation
  - Modern: Consolidated into self-contained RtModel

**This confirms the proposed structure:**
```
src/           # Modern (RtModel is self-contained)
src_classic/   # Classic (uses modular libs)
lib_classic/   # Only for classic: MTL, OptimizeN, GEOM_MODEL, XMLLogging
deprecated/    # Truly obsolete: GenImaging, RtOptimizer (if it existed)
```

**GenImaging Status:**
- Should go in `deprecated/` - eliminated by ITK 4.3 upgrade
- Not used by either stack anymore
- Kept for historical reference only

**RtOptimizer Status:**
- Likely already deleted before initial commit
- If found in history, should go in `deprecated/`
- Functionality absorbed into RtModel

## Conclusion

The modern Brimstone's evolution shows a **deliberate architectural decision** to simplify dependencies by creating a **self-contained RtModel library**. This wasn't just removing dependencies - it was a strategic refactoring that:

1. **Consolidated optimization code** from OptimizeN into RtModel
2. **Eliminated intermediate wrappers** (GenImaging) by using ITK 4.3 directly
3. **Embedded math utilities** to avoid MTL dependency
4. **Created a clean 2-library architecture** (RtModel + Graph)

This explains why the modern and classic stacks share **no common foundation libraries** - they represent two different architectural philosophies:
- **Classic:** Modular separation of concerns
- **Modern:** Self-contained domain library

The repository reorganization should reflect this reality by keeping the stacks separate with their respective dependencies.
