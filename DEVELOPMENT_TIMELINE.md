# dH/Brimstone Repository: Development Timeline

## Executive Summary

This repository represents **20+ years of development** (2000-2025) of radiotherapy treatment planning software, originally developed under the company "2nd Messenger Systems" and resulting in **U.S. Patent 7,369,645**. The codebase evolved from experimental visualization tools into a sophisticated variational inverse planning algorithm.

**Key Statistics:**
- **116 commits** across all branches
- **Primary Author:** Derek G. Lane (dg1an3)
- **Copyright Years:** 2000-2021
- **Languages:** C++ (core), Python (utilities), Fortran (Monte Carlo)
- **Components:** 20+ libraries, 25 applications/utilities

---

## Phase 1: Foundation and Core Algorithm Development (2000-2006)

### 2000-2002: Visualization Framework Development

**Copyright evidence from source headers:**
- `GEOM_VIEW/SceneView.cpp`: "Copyright (C) 2000-2001"
- `RT_VIEW/BeamRenderable.cpp`: "Copyright (C) 2000-2002"
- `OGL_BASE/*`: OpenGL-based rendering (initial approach)

**Key Developments:**
1. **OGL_BASE** - Initial OpenGL immediate-mode rendering framework
2. **GEOM_VIEW** - Migration to Direct3D 8 for hardware acceleration
3. **RT_VIEW** - Radiotherapy-specific visualization (~1,700 lines)
   - `CBeamRenderable` - Treatment beam 3D visualization
   - `CMachineRenderable` - Linear accelerator geometry rendering
   - `CLightfieldTexture` - Radiation field texture mapping

**Architecture Decisions:**
- MFC (Microsoft Foundation Classes) for GUI framework
- Direct3D 8 chosen over OpenGL for Windows driver support
- Observer pattern for model-view synchronization
- Multi-pass alpha blending for transparent geometry

### April 1-2, 2006: First Git Commits (CVS Migration)

**Initial commit activity** (41 commits in 2 days):

| Date | Key Changes |
|------|-------------|
| Apr 1, 2006 16:44 | Thread-safety fixes, VS2005 secure CRT migration |
| Apr 1, 2006 18:23 | New Graph project, PushOk CVS Proxy setup |
| Apr 1, 2006 20:10-20:16 | Multi-threading fixes: non-static kernel, weight filters |
| Apr 2, 2006 01:01-01:16 | Optimization refinements: CG tolerance, sigmoid transform |
| Apr 2, 2006 01:16 | Iterations graph, SendMessage notifications |

**Significant Code:**
- Increased pyramid levels to 4 (coarse-to-fine optimization)
- Added linear interpolation for histogram bins
- Separated pencil sub-beamlet calculation from pencil beam calc
- Moved `COptThread` to `CBrimstoneView`

---

## Phase 2: Algorithm Refinement (2006-2009)

### CVS Revision Evidence (from $Id$ tags)

**Active development period with SVN/CVS revision numbers:**

| File | Last CVS Rev | Date |
|------|--------------|------|
| KLDivTerm.cpp | r647 | Nov 5, 2009 |
| PlanOptimizer.cpp | r647 | Nov 5, 2009 |
| PlanPyramid.cpp | r647 | Nov 5, 2009 |
| ConjGradOptimizer.cpp | r644 | Nov 5, 2009 |
| Beam.cpp | r640 | Jun 13, 2009 |
| HistogramGradient.cpp | r619 | Mar 1, 2009 |
| Brimstone.cpp | r613 | Sep 14, 2008 |
| BeamDoseCalc.cpp | r602 | Sep 14, 2008 |

**Core Algorithm Components Developed:**
- **KL Divergence Minimization** - `KL(P_calc || P_target)` between DVHs
- **Conjugate Gradient Optimizer** - Dynamic covariance optimization
- **Multi-Scale Pyramid** - 4 levels (8.0mm → 0.5mm voxels)
- **Pencil Beam Dose Calculation** - TERMA ray-tracing + convolution
- **Histogram Gradient** - Analytical derivatives for optimization

---

## Phase 3: Maintenance and ITK Upgrades (2010-2015)

### September 2010: Code Reorganization

| Date | Commit | Changes |
|------|--------|---------|
| Sep 17, 2010 | 68648af | Moved files to RtModel (3 commits) |
| Sep 23, 2010 | 48ed55e | Removed CPolygon, switched to PolygonSpatialObject |

### December 2010: Build System Updates

| Date | Changes |
|------|---------|
| Dec 3, 2010 | Removed Intel IPP calls, VS2008 build fixes |

### August 2012: Historical Code Addition

- Added prolog verification code from 2003 (indicating development predates git history)

### October 2014: Major ITK Upgrade

**Commit 44ee23b (Oct 18, 2014):**
- Upgrade to ITK 4.3
- ITK pointer model changes (better memory management)
- Visual Studio 2013 Express support
- Intel Performance Primitives dependency noted

### March-August 2015: GitHub Publication

| Date | Activity |
|------|----------|
| Mar 26, 2015 | Created README.md |
| Mar 27, 2015 | Added BSD license |
| Aug 28, 2015 | Moved ALGT to another repo |
| Aug 29, 2015 | Moved sources to WarpTPS repository |

---

## Phase 4: Machine Learning Experiments (2019-2020)

### December 2019: TensorFlow Exploration

**entropy_max experiments** - Exploring variational approaches:

| Date | Commits | Work |
|------|---------|------|
| Dec 20, 2019 | 8 commits | TensorFlow 2.0 port, autoencoder experiments |
| Dec 21, 2019 | 1 commit | Added notebook structure |
| Dec 26, 2019 | 2 commits | Notes.org created |
| Dec 30, 2019 | 5 commits | Histogram calculation, entropy gradient experiments |
| Dec 31, 2019 | 1 commit | Debug config for entropy development |

**Notebooks Created:**
- `entropy_max.ipynb` - Core entropy maximization
- `entropy_max_histo.ipynb` - Histogram-based entropy
- `mnist_autoencoder.py` - Autoencoder reference implementation
- `entropy_calc.py` - Entropy calculation utilities

### January 2020: Cleanup and Consolidation

| Date | Changes |
|------|---------|
| Jan 1, 2020 | Created Variational_bayes_inverse_planning.md |
| Jan 8, 2020 | Updated to show plots in figure |
| Jan 20, 2020 | **Major cleanup**: Added LICENSE, deleted notes, consolidated notebooks |
| Jan 22, 2020 | Updated read_feed.py |

### January 2021: Documentation Update

- README.md updated (Jan 27, 2021)

---

## Phase 5: Python Porting (2025)

### January 2025: Dose Calculation Python Port

| Date | Commit | Work |
|------|--------|------|
| Jan 14, 2025 | fc4bedc | Updating toolset |
| Jan 14, 2025 | ec012a1 | Porting beamlet calc to Python |
| Jan 14, 2025 | 8babc5d | Porting TERMA calculation |

**New File:** `python/read_process_series.py`
- ITK-based CT density processing
- CT to mass density conversion
- Beam-specific density rotation
- TERMA calculation framework

---

## Phase 6: Repository Consolidation (October-November 2025)

### October 29, 2025: pheonixrt Merge

**Major repository consolidation:**

| Commit | Description |
|--------|-------------|
| cf75988 | Git subtree add (preserving full history) |
| 50ea783 | Merge pheonixrt repository into dH_antiques |
| 542caa7 | Preserved original folders as `_original` |
| 07528d4 | Merge PR #2: copilot/merge-pheonixrt |

**Result:**
- `Brimstone_original/` - Original Brimstone GUI version
- `VecMat_original/` - Original vector math library
- `Graph_original/` - Original graphing library

### October 30, 2025: Variational Bayes Documentation

| Commit | Description |
|--------|-------------|
| dbd0414 | Document variational Bayes connection in README |
| b73896d | Implement optional explicit free energy calculation |
| dd09dd2 | Merge PR #3: dh-algorithm-variational-bayes |

**New Features:**
- Optional explicit free energy calculation (`SetComputeFreeEnergy(true)`)
- Entropy from dynamically-built covariance matrix
- Free energy: `F = KL_divergence - Entropy`

### October 31, 2025: Documentation

| Commit | Description |
|--------|-------------|
| 260ea9e | Update README.md |
| 5c920a6 | Add CLAUDE.md for project documentation |
| e099750 | Add repository structure documentation |

### November 2, 2025: Docker & Monte Carlo Integration

| Commit | Description |
|--------|-------------|
| 94bfe09 | Add comprehensive RT_VIEW library history documentation |
| 56a4734 | Add Docker build setup for PenBeam_indens Fortran code |
| 13e5574 | Add EGSnrc Docker setup for energy deposition kernel generation |

**New Components:**
- `PenBeam_indens/` - Fortran pencil beam code with Docker build
- `EGSnrc/` - Monte Carlo energy deposition kernel generation
- `docker-compose.yml` - Multi-container build orchestration

### November 4, 2025: Latest Merges

- PR #7: Docker build for PenBeam_indens
- PR #8: RT_VIEW library history documentation

---

## Timeline Visualization

```
2000 ─┬─ OGL_BASE (OpenGL rendering)
      │
2001 ─┼─ VSIM_OGL prototype for Siemens VSim [ORAL HISTORY]
      │  GEOM_VIEW (Direct3D 8 migration)
      │
2002 ─┼─ RT_VIEW (Radiotherapy visualization)
      │  Core algorithm conceptualized
      │
2003 ─┼─ [Prolog verification code - predates git]
      │
2004 ─┼─ PenBeamEdit beamlet intensity editor [ORAL HISTORY]
      │  IMRT workflow development
      │
2006 ─┼─ First Git commits (CVS migration)
      │  Thread-safety, VS2005, 4-level pyramid
      │
2007 ─┼─ U.S. Patent 7,369,645 filed
      │
2008 ─┼─ Major SVN development (r600-647)
      │
2009 ─┼─ Final CVS/SVN refinements
      │
2010 ─┼─ Code reorganization, VS2008 fixes
      │
2012 ─┼─ Historical code archiving
      │
2014 ─┼─ ITK 4.3 upgrade, VS2013
      │
2015 ─┼─ GitHub publication, BSD license
      │
2019 ─┼─ TensorFlow experiments
      │  entropy_max notebooks
      │
2020 ─┼─ Cleanup, LICENSE, documentation
      │
2021 ─┼─ Documentation updates
      │
2025 ─┼─ Python porting (Jan)
      │
      ├─ pheonixrt merge (Oct 29)
      │
      ├─ Variational Bayes docs (Oct 30)
      │
      ├─ Docker/Monte Carlo (Nov 2)
      │
      └─ Latest state (Nov 4)
```

---

## Contributors

| Identity | Commits | Period | Notes |
|----------|---------|--------|-------|
| HP_Administrator | 41 | Apr 2006 | Initial CVS migration |
| dglane001@gmail.com | 13 | 2010-2014 | SVN development |
| Derek | 26 | 2015-2019 | GitHub maintenance |
| Derek Lane | 15 | 2019-2020 | ML experiments |
| Lane, Derek | 4 | Various | Alternate identity |
| Derek  Lane | 3 | Various | Alternate formatting |
| Claude | 5 | Oct-Nov 2025 | AI-assisted development |
| copilot-swe-agent[bot] | 4 | Oct 2025 | Repository merge |
| dg1an3 | 5 | 2025 | Recent updates |

**Primary Author:** Derek G. Lane (all identities)

---

## Key Milestones

1. **2000-2002**: Core visualization framework (RT_VIEW, GEOM_VIEW)
2. **~2001**: VSIM_OGL prototype for Siemens VSim application *(oral history)*
3. **~2004**: PenBeamEdit beamlet intensity editor for IMRT *(oral history)*
4. **2006**: Git/CVS repository established, multi-scale optimization
5. **2007**: U.S. Patent 7,369,645 granted
6. **2008-2009**: Peak algorithm development (r600-647)
7. **2014**: ITK 4.3 modernization
8. **2015**: Open source publication on GitHub
9. **2019-2020**: Machine learning experiments
10. **2025**: Python porting, Docker integration, AI-assisted documentation

---

## Architecture Evolution

### Original (2000-2006)
```
Brimstone GUI → RT_MODEL → Dose Calculation
                    ↓
               Optimization (CG)
                    ↓
               KL Divergence
```

### Current (2025)
```
Brimstone GUI ─────────────────────────────────────────→ Visualization
      ↓                                                      ↑
   RtModel ─────────────────────────────────────────────────┘
      │
      ├─ Series (CT data)
      ├─ Structure (anatomy)
      ├─ Beam (treatment beams)
      ├─ Plan (treatment plan)
      │
      ├─ PlanPyramid (4-level multi-scale)
      │     ├─ Level 3: 8.0mm (coarse)
      │     ├─ Level 2: 3.2mm
      │     ├─ Level 1: 1.3mm
      │     └─ Level 0: 0.5mm (fine)
      │
      ├─ DynamicCovarianceOptimizer (CG)
      │     ├─ Adaptive variance
      │     └─ Optional free energy
      │
      ├─ Prescription (objective function)
      │     └─ VOITerm[] (per-structure)
      │           └─ KLDivTerm
      │
      └─ Dose Calculation
            ├─ BeamDoseCalc (TERMA)
            ├─ SphereConvolve (kernel)
            └─ EnergyDepKernel (*.dat)

Python Utilities ─────────────────────────────────────→ ITK Processing
      └─ read_process_series.py

Docker Integration ───────────────────────────────────→ Monte Carlo
      ├─ PenBeam_indens (Fortran)
      └─ EGSnrc (energy kernels)
```

---

## Oral History

*The following information comes from direct communication with the original developer, Derek G. Lane, and supplements the git/CVS history with context that predates or exists outside version control records.*

### ~2001: VSIM_OGL - Siemens VSim Prototype

**VSIM_OGL** (`/home/user/dH/VSIM_OGL/`) served as the **prototype for the Siemens VSim application**, developed around 2001.

**Context:**
- Siemens Medical Solutions was (and remains) a major manufacturer of radiotherapy treatment machines
- VSim (Virtual Simulation) was a commercial product for virtual treatment simulation
- The VSIM_OGL codebase in this repository represents early prototype/proof-of-concept work
- This explains the sophisticated visualization architecture (OGL_BASE → GEOM_VIEW → RT_VIEW) developed during 2000-2002

**Technical Significance:**
- The prototype work drove development of the Direct3D 8 rendering pipeline
- Treatment machine visualization (`CMachineRenderable`) modeled actual LINAC geometry
- Beam visualization (`CBeamRenderable`) supported clinical workflow verification
- The "lightfield" texture (`CLightfieldTexture`) simulated physical light field projection used in patient setup

**Industry Connection:**
This work represents a bridge between academic/research radiotherapy software and commercial clinical systems, explaining the professional-grade architecture despite being a relatively small codebase.

### ~2004: PenBeamEdit - Beamlet Intensity Editor

**PenBeamEdit** (`/home/user/dH/PenBeamEdit/`) was developed around 2004 as a specialized tool for pencil beam intensity map editing.

**Context:**
- By 2004, the core optimization algorithm was maturing
- IMRT (Intensity-Modulated Radiation Therapy) requires precise control of beamlet weights
- PenBeamEdit provided a dedicated interface for manual adjustment and visualization of intensity maps

**Role in Development:**
- Served as a testing ground for beamlet calculation and visualization
- Allowed manual verification of optimization results
- Predates the full Brimstone GUI integration of optimization features
- The "pencil beam" terminology reflects the dose calculation model where each beam is decomposed into small pencil-shaped sub-beams

**Technical Notes:**
- Intensity maps are 2D grids of beamlet weights
- Each beamlet contributes dose through TERMA calculation and kernel convolution
- The editor allowed direct manipulation of these weights outside the optimization loop

### Timeline Integration

With oral history context, the refined early timeline becomes:

```
2000 ─┬─ OGL_BASE development begins
      │
2001 ─┼─ VSIM_OGL prototype for Siemens VSim
      │  GEOM_VIEW Direct3D migration
      │
2002 ─┼─ RT_VIEW radiotherapy visualization
      │  Core algorithm conceptualization
      │
2003 ─┼─ Prolog verification code
      │  Algorithm formalization
      │
2004 ─┼─ PenBeamEdit beamlet intensity editor
      │  IMRT workflow development
      │
2005 ─┼─ [Development continues - pre-git]
      │
2006 ─┼─ CVS/Git migration (April)
      │  Multi-scale pyramid (4 levels)
      │  Thread-safety improvements
```

### Significance

The oral history reveals that this codebase has deeper roots in **commercial radiotherapy software development** than the git history alone suggests. The 2001 Siemens prototype work and 2004 PenBeamEdit development represent significant milestones that explain:

1. **Why the visualization architecture is so sophisticated** - It was designed for commercial use
2. **Why the optimization algorithm is clinically-oriented** - It emerged from real treatment planning needs
3. **Why the codebase spans multiple distinct applications** - Different tools served different workflow stages
4. **The 2006 git commits were a migration**, not the start of development - 5+ years of prior work existed

---

## Intellectual Property

- **U.S. Patent 7,369,645** - Radiotherapy treatment planning
- **Copyright:** 2007-2021, Derek G. Lane
- **License:** Modified BSD with Hippocratic and Anti-996 clauses
- **Company:** 2nd Messenger Systems

---

*Document generated: November 27, 2025*
*Based on comprehensive analysis of git history, source code headers, documentation, and oral history from the original developer*
