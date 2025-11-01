# dH - Radiotherapy Treatment Planning System

Variational inverse planning algorithm for radiotherapy treatment planning.

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
