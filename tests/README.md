# RtModel and RT_VIEW Testing Framework

This directory contains comprehensive tests for the RtModel (radiotherapy optimization) and RT_VIEW (visualization) libraries.

## Test Architecture

The testing framework combines two complementary approaches:

### 1. C++ Unit Tests (Catch2)

Modern unit tests covering:
- **RtModel Core**: Plan, Beam, Structure, Series data models
- **Optimization**: ConjugateGradient, Prescription, KL divergence, adaptive variance
- **Dose Calculation**: TERMA calculation, energy kernel convolution, dose accumulation
- **Histograms**: DVH computation, Gaussian smoothing, gradient calculation
- **RT_VIEW**: Beam rendering, machine geometry, field visualization

### 2. Prolog Verification Predicates (Inspired by ALGT)

Formal verification of algorithmic properties using predicate logic:
- **Geometric Verification**: Beam geometry, rotation matrices, isocentric setup
- **Mathematical Properties**: KL divergence non-negativity, DVH monotonicity
- **Physical Constraints**: Dose conservation, TERMA decay, field divergence
- **Convergence Properties**: Optimizer monotonicity, gradient convergence

## Directory Structure

```
tests/
├── catch2/
│   └── catch.hpp              # Catch2 single-header framework
├── RtModel/
│   ├── test_plan.cpp          # Plan and data model tests
│   ├── test_histogram.cpp     # Histogram and DVH tests
│   ├── test_optimizer.cpp     # Optimization algorithm tests
│   └── test_dose_calc.cpp     # Dose calculation tests
├── RT_VIEW/
│   └── test_beam_renderable.cpp  # Rendering tests
├── prolog/
│   └── rtmodel_verification.pl   # Prolog verification predicates
└── README.md                  # This file
```

## Building and Running Tests

### C++ Unit Tests

#### Option 1: Visual Studio (Recommended for Windows)

1. Open the solution file: `tests/RtModel_Tests.sln`
2. Build the solution (Ctrl+Shift+B)
3. Run tests from Test Explorer (Test → Windows → Test Explorer)

#### Option 2: CMake Build

```bash
cd tests
mkdir build
cd build
cmake ..
cmake --build .
ctest --verbose
```

#### Option 3: Manual Compilation

For each test file:

```bash
cl /EHsc /I.. /I../../RtModel/include /I../../RT_VIEW/include test_plan.cpp /link RtModel.lib
./test_plan.exe
```

### Prolog Verification Tests

Requires SWI-Prolog (download from https://www.swi-prolog.org/)

```bash
cd tests/prolog
swipl -s rtmodel_verification.pl
```

The tests will run automatically and display results:

```
=== RtModel Algorithm Verification ===

Testing histogram properties...
  ✓ DVH monotonicity
  ✓ DVH bounds [0,1]
  ✓ DVH starts at 1.0
  ✓ DVH ends at 0.0

Testing dose calculation properties...
  ✓ Dose superposition (linearity)
  ✓ TERMA exponential decay
  ✓ Dose conservation

Testing optimizer convergence...
  ✓ Cost function monotonically decreasing
  ✓ Gradient converges to zero
  ✓ Conjugate gradient orthogonality

Testing beam geometry...
  ✓ Field divergence
  ✓ Rotation matrix orthogonality
  ✓ Isocentric geometry

Testing KL divergence properties...
  ✓ KL divergence non-negative
  ✓ KL = 0 for identical distributions
  ✓ KL gradient formula

=== All Verification Tests Passed ===
```

## Test Coverage

### RtModel Tests

#### Data Models (`test_plan.cpp`)
- ✓ Plan construction and initialization
- ✓ Beam management (add, retrieve, count)
- ✓ Series association
- ✓ Dose matrix handling
- ✓ Histogram creation and management

#### Histograms (`test_histogram.cpp`)
- ✓ Histogram construction with valid parameters
- ✓ Bin spacing and indexing
- ✓ DVH calculation from dose volumes
- ✓ Gaussian convolution (adaptive smoothing)
- ✓ HistogramGradient derivative computation
- ✓ Binning correctness and bounds checking

#### Optimization (`test_optimizer.cpp`)
- ✓ ConjugateGradient optimizer construction
- ✓ Parameter configuration (tolerance, max iterations)
- ✓ Convergence on simple quadratic function
- ✓ Free energy computation (F = KL - Entropy)
- ✓ Adaptive variance formula
- ✓ Prescription objective function
- ✓ KL divergence term creation and intervals
- ✓ Gradient chain rule through sigmoid transform

#### Dose Calculation (`test_dose_calc.cpp`)
- ✓ Energy kernel loading from file
- ✓ TERMA calculation via ray tracing
- ✓ TERMA exponential decay in uniform medium
- ✓ Zero TERMA in zero density
- ✓ Spherical convolution
- ✓ Point source produces kernel shape
- ✓ Dose superposition principle (linearity)

### RT_VIEW Tests

#### Beam Rendering (`test_beam_renderable.cpp`)
- ✓ BeamRenderable construction
- ✓ Beam association
- ✓ Gantry, collimator, table angles
- ✓ Jaw positions and field size
- ✓ Asymmetric fields
- ✓ Isocenter positioning (3D coordinates)
- ✓ Source-to-axis distance (SAD)
- ✓ MachineRenderable gantry rotation
- ✓ Rotation matrix orthogonality
- ✓ Beam divergence calculation
- ✓ Block rendering
- ✓ Central axis geometry

### Prolog Verification

#### Histogram Properties
- ✓ DVH monotonically decreasing
- ✓ DVH values bounded [0, 1]
- ✓ DVH starts at 1.0 (all volume receives ≥ 0 dose)
- ✓ DVH ends at 0.0 (no volume receives infinite dose)

#### Dose Calculation
- ✓ Dose superposition (linearity: D(A+B) = D(A) + D(B))
- ✓ TERMA exponential decay: TERMA(z) = TERMA₀ × exp(-μz)
- ✓ Dose conservation (energy balance)

#### Optimizer Convergence
- ✓ Cost function monotonically decreasing
- ✓ Gradient converges to zero
- ✓ Conjugate gradient search directions orthogonality

#### Beam Geometry
- ✓ Rotation matrices are orthogonal (R^T × R = I)
- ✓ Field divergence: FieldSize(d) = FieldSize(SAD) × (d / SAD)
- ✓ Isocentric geometry (source-isocenter-image collinearity)

#### KL Divergence
- ✓ KL(P||Q) ≥ 0 (non-negativity)
- ✓ KL(P||P) = 0 (zero for identical distributions)
- ✓ Gradient formula: ∂KL/∂P = log(P/Q) + 1

## Writing New Tests

### C++ Unit Test Template

```cpp
#include "../catch2/catch.hpp"
#include "../../RtModel/include/YourClass.h"

using namespace Catch::Matchers;

TEST_CASE("Feature description", "[component][tag]") {
    SECTION("Specific aspect") {
        // Arrange
        YourClass obj;

        // Act
        REAL result = obj.Compute();

        // Assert
        REQUIRE(result > 0.0);
        REQUIRE_THAT(result, WithinAbs(expected, 1e-6));
    }
}
```

### Prolog Predicate Template

```prolog
% Predicate: Property description
verify_property(Input1, Input2, Expected) :-
    tolerance(Tol),
    compute_result(Input1, Input2, Actual),
    abs(Actual - Expected) < Tol.

% Test case
test_property :-
    verify_property(10.0, 20.0, 30.0),
    writeln('  ✓ Property verified').
```

## Test Categories and Tags

Use tags to organize and filter tests:

- `[core]` - Core functionality tests
- `[plan]` - Plan data structure
- `[beam]` - Beam geometry and parameters
- `[histogram]` - Histogram and DVH
- `[dvh]` - Dose-volume histogram specific
- `[optimizer]` - Optimization algorithms
- `[dose]` - Dose calculation
- `[terma]` - TERMA calculation
- `[kernel]` - Energy kernel
- `[convolution]` - Spherical convolution
- `[gradient]` - Gradient computation
- `[kldiv]` - KL divergence
- `[rtview]` - RT_VIEW visualization
- `[geometry]` - Geometric calculations
- `[rendering]` - Rendering tests

Run specific tests by tag:
```bash
./tests.exe "[optimizer]"  # Run only optimizer tests
./tests.exe "[dose][kernel]"  # Run dose kernel tests
```

## Continuous Integration

For CI/CD pipelines, run tests with:

```bash
# C++ tests
cmake --build build --target test

# Prolog tests
swipl -g "run_all_tests,halt" -t "halt(1)" tests/prolog/rtmodel_verification.pl
```

## Troubleshooting

### Common Issues

**Issue**: `fatal error: Histogram.h: No such file or directory`
**Solution**: Ensure include paths are set correctly:
```bash
-I../../RtModel/include -I../../RT_VIEW/include
```

**Issue**: `undefined reference to CHistogram::CHistogram()`
**Solution**: Link against RtModel library:
```bash
/link RtModel.lib
```

**Issue**: Prolog tests fail to load
**Solution**: Install SWI-Prolog from https://www.swi-prolog.org/ and ensure it's in PATH

**Issue**: Floating point comparison failures
**Solution**: Use `WithinAbs()` matcher with appropriate tolerance:
```cpp
REQUIRE_THAT(actual, WithinAbs(expected, 1e-6));
```

## Test Data

Some tests require test data files:
- Energy kernels: `Brimstone/6MV_kernel.dat`, `Brimstone/15MV_kernel.dat`
- Sample plans: `RT_MODEL/TestHisto/test_plan.dat`

Ensure these files are accessible from the test working directory.

## References

- **ALGT Approach**: Prolog-based algorithm verification for medical imaging
- **Catch2 Documentation**: https://github.com/catchorg/Catch2
- **SWI-Prolog**: https://www.swi-prolog.org/
- **ITK**: https://itk.org/ (Insight Toolkit for medical imaging)

## Contributing

When adding new features to RtModel or RT_VIEW:

1. **Write tests first** (TDD approach)
2. **Add both C++ and Prolog tests** where applicable
3. **Document test coverage** in this README
4. **Use descriptive test names** and tags
5. **Keep tests isolated** (no shared state)
6. **Verify tests pass** before committing

## License

Copyright (c) 2007-2025, Derek G. Lane. All rights reserved.

U.S. Patent 7,369,645

See LICENSE file for full terms.
