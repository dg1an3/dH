# dH / Brimstone — Software Component Inventory

This document inventories the software components (libraries, applications, scripts)
in this repository. For each component it records:

- **Purpose** — what the component is for
- **Dependencies** — external libraries and internal repo components it relies on
- **Data I/O** — whether it reads/writes data, and in what formats
- **Depended on by** — which components consume it

A consolidated [File Formats](#file-formats-used) section follows the inventory.

> Layering (low → high):
> `MTL / VecMat / FTL` → `GEOM_BASE / MODEL_BASE / OPTIMIZER_BASE / OGL_BASE / GUI_BASE`
> → `GEOM_MODEL / VSIM_MODEL / GenImaging` → `OptimizeN / RT_MODEL → RtModel`
> → `GEOM_VIEW / RT_VIEW / Graph` → `Brimstone / PenBeamEdit / VSIM_OGL` → utilities.

---

## Production System (`Brimstone_src.sln`)

### RtModel — Core algorithm library
- **Type:** Static library (C++17, modern CMake + `.vcxproj`)
- **Purpose:** The core radiotherapy engine. Implements the data model (Plan, Beam,
  Series, Structure), pencil-beam dose calculation (TERMA + energy-kernel convolution),
  dose-volume histograms with gradients, and the multi-scale conjugate-gradient inverse
  optimizer that minimizes KL divergence between calculated and target DVHs.
  - *Data models:* `Plan`, `Beam`, `Series`, `Structure`, `Prescription`
  - *Dose calc:* `BeamDoseCalc` (TERMA ray-trace), `CEnergyDepKernel`, `SphereConvolve`
  - *Histograms/objective:* `CHistogram`, `CHistogramWithGradient`, `VOITerm`, `KLDivTerm`
  - *Optimization:* `PlanOptimizer`, `PlanPyramid`, `DynamicCovarianceOptimizer`, `ConjGradOptimizer`
  - *Serialization:* `PlanXmlWriter`, `PlanXmlReader`
- **Dependencies:**
  - *External:* ITK 5.x (`itk::Image<float,3>`), VNL/VXL (`vnl_cost_function`,
    `vnl_nonlinear_minimizer`), Intel IPP (optional, `USE_IPP`), MFC (`CArchive`,
    `CString`), STL, Windows SDK
  - *Internal:* GEOM_MODEL, MTL, GenImaging, OptimizeN, XMLLogging
- **Data I/O:**
  - *Reads:* XML plan files (`PlanXmlReader`); energy-dependent kernel binaries
    (`*_kernel.dat`) via `CEnergyDepKernel::LoadKernel()`; DICOM/ITK volumes (via consumers)
  - *Writes:* XML plan files (`PlanXmlWriter` — beams, intensity maps, prescriptions,
    DVHs, dose-calc params); binary ITK image serialization via `ItkUtils.h`
- **Depended on by:** Brimstone, Graph

### Brimstone — Treatment-planning GUI application
- **Type:** Executable, MFC SDI (modern CMake + `.vcxproj`)
- **Purpose:** The end-user radiotherapy treatment-planning application. Imports patient
  CT + structures, lets the user set up prescriptions/constraints and beams, runs the
  optimizer on a background thread, and visualizes CT slices, contours, dose overlays,
  DVH graphs, and optimization progress.
  - `CBrimstoneApp` (entry), `CBrimstoneDoc` (owns Series + Plan, serialization),
    `CBrimstoneView` / `CPlanarView` (2D rendering), `CMainFrame`,
    `CSeriesDicomImporter` (DICOM import), `OptThread` / `OptimizerDashboard` (async optimize)
- **Dependencies:**
  - *External:* MFC, ATL, Intel IPP (via RtModel), Windows GDI
  - *Internal:* RtModel, Graph, XMLLogging, GEOM_MODEL, MTL
- **Data I/O:**
  - *Reads:* DICOM image series + RT structure sets (`CSeriesDicomImporter`);
    Brimstone document files (MFC `Serialize`); `*_kernel.dat` photon kernels
  - *Writes:* Brimstone document files (MFC serialization); XML plan files (via RtModel);
    XML diagnostic logs (via XMLLogging)
  - *Bundled data:* `2MV_kernel.dat`, `6MV_kernel.dat`, `15MV_kernel.dat`
- **Depended on by:** Nothing (top-level application)

### Graph — DVH / iteration plotting library
- **Type:** Static library, MFC (modern CMake + `.vcxproj`)
- **Purpose:** Interactive 2D plotting control used to render dose-volume histograms and
  optimization-iteration curves inside Brimstone.
  - `CGraph` (plot window), `CDataSeries`, `CHistogramDataSeries`, `CTargetDVHSeries`, `CDib`
- **Dependencies:**
  - *External:* MFC, ITK (smart pointers/vectors), Windows GDI
  - *Internal:* RtModel, GEOM_MODEL
- **Data I/O:** No persistence — purely in-memory rendering to a Windows `CDC`. `CDib`
  can load a bitmap from file/resource for display.
- **Depended on by:** Brimstone

---

## Foundation / Template Libraries

### MTL — Math Template Library
- **Type:** Header-only library (legacy `.dsp`/`.vcproj`; has CMake via root opt-in)
- **Purpose:** Templated vector/matrix classes (`CVectorD`, `CMatrixD`, `CMatrixNxM`),
  least-squares solvers, matrix ops — the numeric foundation.
- **Dependencies:** STL; MFC (in tests only). No external scientific libs.
- **Data I/O:** None (pure in-memory).
- **Depended on by:** VecMat, GEOM_MODEL, GEOM_VIEW, RT_MODEL, RtModel, OptimizeN

### VecMat (and VecMat_original) — Vector/Matrix library
- **Type:** Header-only library (legacy VC6 `.dsp`/`.dsw`)
- **Purpose:** A refactored iteration of MTL — `VectorBase/VectorD/VectorN`,
  `MatrixBase/MatrixD/MatrixNxM` with inline `.inl` implementations.
- **Dependencies:** STL only.
- **Data I/O:** None (pure in-memory).
- **Depended on by:** DivFluence, PenBeamEdit (alternate to MTL in legacy tools)

### FTL — Foundation Template Library
- **Type:** Header-only library (`FTL.sln`)
- **Purpose:** Multi-dimensional container abstractions (`BufferND`, `MeshND`, `Field`,
  `BufferIndex`) for spatially organized grid/buffer data.
- **Dependencies:** MFC (`CString` via `afxtempl.h`); MTL (`VectorD.h`).
- **Data I/O:** None (in-memory containers).
- **Depended on by:** GEOM_MODEL, GEOM_VIEW

### GEOM_BASE — Geometry base primitives
- **Type:** Static library (legacy project files)
- **Purpose:** 2D/3D geometry primitives — `Polygon`, `LookupFunction`, `ScalarFunction`
  — plus `Volumep.h` (`CVolume` 3D voxel grid) and MFC serialization support.
- **Dependencies:** MTL, MODEL_BASE, MFC (`DECLARE_SERIAL`, `CObject`).
- **Data I/O:** MFC `CArchive` serialization (e.g. `Polygon` is serializable/observable).
- **Depended on by:** GEOM_MODEL, GEOM_VIEW, DivFluence, PenBeamEdit

### MODEL_BASE — Observable/serializable model framework
- **Type:** Static library (legacy project files)
- **Purpose:** Abstract MFC model architecture — `CModelObject` (observable,
  serializable, hierarchical), Observer pattern, collections, metadata/value properties.
- **Dependencies:** MFC (`DECLARE_SERIAL`, `afxtempl.h`); C++ templates.
- **Data I/O:** Provides the serialization infrastructure (`DECLARE_SERIAL`) inherited by
  domain objects; no direct file I/O itself.
- **Depended on by:** GEOM_BASE, GEOM_MODEL, VSIM_MODEL, DivFluence, PenBeamEdit

---

## Component / Mid-Level Libraries

### GEOM_MODEL — Geometric modeling
- **Type:** Static library (has CMakeLists.txt)
- **Purpose:** Higher-level 3D geometry/transform kernels — `Mesh`, `Cluster` (k-means),
  `AffineTransform`, `TPS` (thin-plate spline), `GradientCalculator`, `Dib`, plus the
  `Volumep.h`/`UtilMacros.h` headers used widely by RtModel.
- **Dependencies:** MTL, GEOM_BASE, XMLLogging (`USE_XMLLOGGING`), IPP (`USE_IPP`).
- **Data I/O:** `Dib` raster I/O; XML logging; `CArchive` serialization via base classes.
  (`ClusterTest/Cars.csv` is sample test input.)
- **Depended on by:** GEOM_VIEW, RT_MODEL, RtModel, Graph, VSIM_MODEL, Brimstone

### GEOM_VIEW — Geometry visualization
- **Type:** Static library (legacy)
- **Purpose:** MFC + Direct3D 8 rendering framework — `Camera`, `Light`, `Renderable`,
  `SceneView`, `ObjectExplorer` tree UI, rotate/zoom trackers.
- **Dependencies:** Direct3D 8 (`d3dx8.h`), MTL, GEOM_MODEL, XMLLogging, MFC.
- **Data I/O:** None (rendering only).
- **Depended on by:** Legacy viewer apps (Brimstone_original / Graph_original lineage)

### OPTIMIZER_BASE — Base optimization framework
- **Type:** Header-only (legacy `.vcproj`, lives under OptimizeN/)
- **Purpose:** 1-D line-search optimizers (Brent, Powell, conjugate gradient) as generic templates.
- **Dependencies:** MFC (`afxtempl.h`).
- **Data I/O:** None.
- **Depended on by:** OptimizeN, RT_MODEL

### OptimizeN — N-dimensional optimization
- **Type:** Static library (has CMakeLists.txt)
- **Purpose:** Multi-dimensional optimizers (Brent, Powell, ConjGrad, DFP, gradient
  descent, cubic interpolation), extending OPTIMIZER_BASE.
- **Dependencies:** XMLLogging, MTL.
- **Data I/O:** None (emits XML logs when logging enabled).
- **Depended on by:** RT_MODEL, Brimstone

### OGL_BASE — OpenGL base classes
- **Type:** Static library (legacy)
- **Purpose:** OpenGL rendering base — `COpenGLRenderer`, `COpenGLView`, camera/light/
  texture classes, rotate/zoom trackers, matrix/vector GL helpers.
- **Dependencies:** MFC, Windows OpenGL.
- **Data I/O:** None (rendering only).
- **Depended on by:** VSIM_OGL

### GUI_BASE — Base MFC GUI widgets
- **Type:** Static library (legacy)
- **Purpose:** Reusable MFC UI components — `DrawTool`, `Graph` (plot), `Dib`,
  `ObjectExplorer`/`ObjectTreeItem` tree, `TabControlBar`.
- **Dependencies:** MFC (`afxcmn.h` common controls).
- **Data I/O:** Bitmap (DIB) load/render.
- **Depended on by:** Brimstone-lineage GUI apps, PenBeamEdit

### GenImaging — ITK imaging filters
- **Type:** Static library (legacy `.vcproj`)
- **Purpose:** Radiotherapy-specific ITK image filters — `InPlaneResampleImageFilter`,
  `IntensityMapAccumulateImageFilter` (dose accumulation), `ContoursToRegionFilter`,
  `ScalarImageToWeightedHistogramGenerator`, `MultiMaskNegatedImageFilter`.
- **Dependencies:** ITK, Intel IPP (`ipps.h`, `ippcv.h`).
- **Data I/O:** Operates on in-memory ITK volumes (`itk::Image<REAL,3>`); no file I/O.
- **Depended on by:** RT_MODEL, RtModel, Brimstone

### XMLLogging — XML diagnostic logging framework
- **Type:** Static library (has CMakeLists.txt)
- **Purpose:** Structured XML logging — `CXMLLogFile` (thread-safe singleton writing to a
  `FILE*`), `CXMLElement` (scoped tags), `CXMLLoggableObject`, plus `LOG`/`LOG_EXPR`/
  timing macros. Gated by `XMLLOGGING_ON`/`XMLLOGGING_OFF`.
- **Dependencies:** MFC, STL.
- **Data I/O:** *Writes* nested XML log files (attributes + text) to a runtime-set path.
- **Depended on by:** RtModel, Brimstone, OptimizeN, GEOM_MODEL, GEOM_VIEW, RT_MODEL, VSIM_MODEL

### FieldCOM — COM/ATL geometry interop
- **Type:** ActiveX/COM library (ATL, legacy)
- **Purpose:** Exposes geometry/spatial structures as COM objects — `Mesh` (`IMesh`),
  `MeshSet`, `BufferField`, `Polygon3D`, `PolygonSet3D`, `AffineTransform`, `TPSTransform`,
  `FileStorage` — for cross-process/tool interop.
- **Dependencies:** ATL, Windows COM runtime.
- **Data I/O:** `FileStorage`/`IPersistStorage` save/load to a pathname; COM stream serialization.
- **Depended on by:** VSIM_OGL (via `FieldCOM.Mesh`)

---

## Legacy RT / Simulation Components

### RT_MODEL — Radiotherapy model (precursor to RtModel)
- **Type:** Static library + console tools (legacy `.vcproj`)
- **Purpose:** Older sibling of RtModel containing the same domain (Plan/Series/Beam/
  Structure/Histogram/Prescription), optimizers (`ConjGradOptimizer`, `TCP_NTCP_Optimizer`,
  `VOITerm`, `KLDivTerm`), DICOM import (`DicomImEx/`), and XML plan serialization.
  Bundled console tools: `GenBeamlets`, `GenDens`, `DicomImEx`, `TestHisto`.
- **Dependencies:** ITK, MFC, OptimizeN, GEOM_MODEL, GenImaging, MTL, XMLLogging, IPP.
- **Data I/O:** Reads DICOM series; reads/writes XML plans (`PlanXmlFile`); reads/writes ITK volumes.
- **Depended on by:** Legacy tooling (production path is RtModel)

### RT_VIEW — Radiotherapy visualization
- **Type:** Static library (legacy)
- **Purpose:** Direct3D/MFC renderables and control panels — `BeamRenderable`,
  `MachineRenderable`, `BeamParamCollimCtrl`, `BeamParamPosCtrl`, `LightfieldTexture`.
- **Dependencies:** MFC, Direct3D 8, RT_MODEL.
- **Data I/O:** None (rendering only).
- **Depended on by:** Brimstone-lineage beam/machine panels

### VSIM_MODEL — Virtual-simulation data model
- **Type:** Static library (legacy)
- **Purpose:** Virtual-simulation domain model (`Plan`, `Series`, `Beam`,
  `TreatmentMachine`) — an older simulation system mirroring RT_MODEL.
- **Dependencies:** MFC, GEOM_MODEL.
- **Data I/O:** MFC `CDocument`/`DECLARE_SERIAL` serialization.
- **Depended on by:** VSIM_OGL

### VSIM_OGL — Virtual-simulation OpenGL viewer
- **Type:** Executable, MFC SDI (legacy)
- **Purpose:** OpenGL viewer for the virtual-simulation system; integrates OGL_BASE +
  VSIM_MODEL and can export to FieldCOM.
- **Dependencies:** MFC, OGL_BASE, VSIM_MODEL, FieldCOM.
- **Data I/O:** Reads VSIM_MODEL documents; exports geometry to FieldCOM (`FieldCOM.Mesh` + `FileStorage`).
- **Depended on by:** Nothing (standalone app)

---

## Standalone Utilities / Research

### DivFluence — Divergent-fluence calculator
- **Type:** Console application (legacy VC6)
- **Purpose:** Computes divergent fluence for dose calculation — exponential attenuation,
  inverse-square falloff, heterogeneous density, field divergence.
- **Dependencies:** Intel IPP; VecMat, GEOM_BASE (`CVolume`), MODEL_BASE.
- **Data I/O:** Reads density volumes + beam params + energy kernel `.dat` tables
  (e.g. `lang48rad48.dat`); writes fluence/energy volumes (binary `CVolume`).
- **Depended on by:** PenBeamEdit / downstream dose pipeline (conceptually)

### PenBeamEdit — Pencil-beam editor GUI
- **Type:** Executable, MFC SDI (legacy)
- **Purpose:** GUI editor for pencil-beam parameters; `TransitionPlan.txt` outlines a
  modernization toward `CVolume`, OpenGL, and DVH views.
- **Dependencies:** MFC; GEOM_BASE (`Volumep.h`), MODEL_BASE, GUI_BASE (`CDib`), VecMat.
- **Data I/O:** Reads/writes serialized beam-parameter documents (MFC serialization of `CVolume` images).
- **Depended on by:** Nothing (standalone app)

### PenBeam_indens — Pencil-beam in-density (Fortran reference)
- **Type:** Legacy Fortran program + batch scripts
- **Purpose:** Reference implementation of pencil-beam in-density convolution
  (ray-trace through heterogeneous phantom + spherical convolve kernels).
- **Dependencies:** None (standalone Fortran modules).
- **Data I/O:** Reads phantom density grids, ray geometry (`penbeam_input.txt`), energy
  tables (`6mv_example.dat`, `lang48rad48.dat`); writes dose to `cono/` (`.dat`/binary).
- **Depended on by:** Serves as the mathematical spec for DivFluence

### python/ — ITK CT-density processing scripts
- **Type:** Python scripts
- **Purpose:** CT preprocessing — HU→mass-density conversion, beam-angle density rotation,
  and a TERMA ray-tracing framework (`read_process_series.py`:
  `ct_to_md_values`, `rotate_density_for_beam`, `terma_from_density`).
- **Dependencies:** ITK Python bindings, NumPy.
- **Data I/O:** Reads CT volumes via `itk.imread()` (e.g. `.nrrd`); produces density-
  converted/rotated ITK images and accumulated TERMA volumes.
- **Depended on by:** Nothing (preprocessing/prototype pipeline)

### notebook_zoo/ — Research notebooks
- **Type:** Jupyter notebooks + helper `.py` (`entropy_max.sln`)
- **Purpose:** Exploratory research on entropy maximization and ML autoencoders/VAEs —
  e.g. `entropy_calc_notebook`, `entropy_max`, `entropy_max_histo`,
  `mnist_autoencoder_entropy_max`, `mnist_autoencoder_gabor`, `mnist_autoencoder_tf2`,
  `mnist_vae_shift`, `pinwheel_shifter`.
- **Dependencies:** NumPy, TensorFlow 2, Matplotlib (typical); MNIST dataset.
- **Data I/O:** In-memory NumPy arrays, MNIST imports; outputs plots and model snapshots.
- **Depended on by:** Nothing (sandbox)

---

## Build & Tooling Support

| Item | Purpose |
|------|---------|
| `cmake/FindIPP.cmake` | Locate Intel IPP from NuGet `packages/` or system oneAPI |
| `cmake/BrimstoneConfig.cmake.in` | Package-config template for installed Brimstone targets |
| `cmake/vnl_fix.h` | VNL/VXL compatibility shim |
| `CMakeLists.txt` (root + per-project) | Modern cross-platform build (RtModel, Graph, Brimstone, XMLLogging, GEOM_MODEL, OptimizeN, MTL/FTL) |
| `packages/` | NuGet download location (Intel IPP, OpenMP, TBB) |
| `NuGet.Config` | NuGet source/destination config |
| `CVSROOT/` | Legacy CVS metadata (historical artifact, inactive) |
| `x64/`, `build/` | Build output artifacts (no source) |
| `Brimstone_original/`, `Graph_original/` | Archived earlier copies of Brimstone/Graph |

---

## File Formats Used

### Read

| Format | Extension / Name | Used by | Notes |
|--------|------------------|---------|-------|
| DICOM image series + RT structure sets | `.dcm` | Brimstone (`CSeriesDicomImporter`), RT_MODEL (`DicomImEx`) | Patient CT + contours import |
| XML treatment plan | `.xml` | RtModel (`PlanXmlReader`), RT_MODEL (`PlanXmlFile`) | Plan geometry, intensities, prescriptions, DVHs |
| Energy-deposition kernel | `*_kernel.dat`, `lang48rad48.dat`, `6mv_example.dat` | RtModel (`CEnergyDepKernel`), DivFluence, PenBeam_indens | Photon beam convolution kernels (binary / lookup tables) |
| ITK volumes | `.nrrd` and other ITK-readable | python/ (`itk.imread`), RtModel/RT_MODEL | CT/density volumes |
| MFC document | `.brimstone` (custom) | Brimstone (`CBrimstoneDoc::Serialize`) | Persisted Series + Plan |
| Bitmap | `.bmp`/DIB | Graph/GUI_BASE (`CDib`) | UI imagery |
| CSV | `.csv` | GEOM_MODEL `ClusterTest` (`Cars.csv`) | Sample cluster test data |
| Plain-text input | `penbeam_input.txt` | PenBeam_indens | Ray geometry / field params |

### Write

| Format | Extension / Name | Produced by | Notes |
|--------|------------------|-------------|-------|
| XML treatment plan | `.xml` | RtModel (`PlanXmlWriter`), RT_MODEL (`PlanXmlFile`) | Full plan export |
| XML diagnostic log | `.xml` | XMLLogging (`CXMLLogFile`) | Nested elements + timing; gated by `XMLLOGGING_ON` |
| MFC document | `.brimstone` (custom) | Brimstone (`CBrimstoneDoc::Serialize`) | Save/load planning session |
| Binary ITK image serialization | (CArchive blob) | RtModel (`ItkUtils.h`) | Geometry + pixel buffer |
| Binary volume / dose output | `.dat` / raw `CVolume` | DivFluence, PenBeam_indens (`cono/`) | Fluence / dose grids |
| FieldCOM storage | COM `FileStorage` | VSIM_OGL → FieldCOM | Geometry interop export |
| ML model snapshots / plots | (framework-specific) | notebook_zoo | Research artifacts |

### Conditional-compilation flags affecting I/O & behavior

| Flag | Effect |
|------|--------|
| `USE_IPP` | Enables Intel IPP-accelerated vector/image ops in RtModel, GenImaging, GEOM_MODEL |
| `USE_XMLLOGGING` / `XMLLOGGING_ON` | Enables XML diagnostic log output |
| `USE_ExportFieldCOM` | Enables FieldCOM export path in VSIM_OGL |
| `WINVER 0x0501` | Targets Windows XP or later |

---

## Key Classes & Algorithms

Class/algorithm-level inventory of the most important types in each library. RtModel
entries cite `file:line` for the core method. Embeddings for every entry below are in
`python/class_embeddings.json` (see [Embeddings](#embeddings)).

### RtModel — Optimization
- **DynamicCovarianceOptimizer** (`ConjGradOptimizer.cpp:55`, `minimize()` at :70) — Conjugate
  gradient with Polak-Ribière direction updates and Brent line search. Maintains an orthogonal
  basis of past search directions (Gram-Schmidt, :298) and derives an *adaptive variance* via a
  power-law schedule `scale = pow(4, nScale)/pow(4, num_iterations)` (:330). Optional explicit
  free energy `F = KL_divergence − Entropy`, with differential entropy
  `H = 0.5·(n·log(2πe) + log det Σ)` from the covariance built off those directions
  (`ComputeEntropyFromCovariance()` :222, free energy :357).
- **DynamicCovarianceCostFunction** — VNL-based cost-function base (`vnl_cost_function`) that
  `Prescription` derives from; the bridge between the optimizer and the objective.

### RtModel — Objective Function & Terms
- **VOITerm** (`VOITerm.h:19`) — Abstract base for per-structure objective terms; pure-virtual
  `Eval()` (value + gradient) and `Clone()` (for pyramid copies). Owns a `Structure` and a
  `CHistogramWithGradient`.
- **KLDivTerm** (`KLDivTerm.cpp:17`, `Eval()` :219) — Minimizes KL divergence between calculated
  and target DVHs: `Σ_bins calcGPDF·log(calcGPDF/(targetGPDF+ε))`. Convolves dose-volume points
  with dual Gaussian kernels at `varMin`/`varMax` (:199); gradient via product-rule chain (:369).
  Supports log-ratio and cross-entropy modes.
- **Prescription** (`Prescription.cpp:21`, `operator()` :237) — Objective aggregator; weighted sum
  of `VOITerm` evals. `CalcSumSigmoid()` (:348) accumulates weighted beamlet volumes with adaptive
  variance fractions. Implements the **sigmoid parameterization** mapping unbounded optimizer
  variables → bounded beamlet weights (`Transform`/`dTransform`/`InvTransform` :551,
  `SIGMOID_SCALE=0.2`), with a transform-slope variance correction `variance *= dSigmoid(...)²` (:408).

### RtModel — Dose Calculation
- **CBeamDoseCalc** (`BeamDoseCalc.cpp:17`, `CalcTerma()` :192) — **TERMA** ray tracer. Resamples CT
  density, traces pencil-beam rays from source through the beamlet aperture, trilinearly
  interpolates density, and accumulates energy to voxel neighborhoods
  (`UpdateTermaNeighborhood()`); then invokes kernel sphere convolution (:124).
- **CEnergyDepKernel** (`EnergyDepKernel.cpp:20`, `CalcSphereConvolve()` :70) — Energy-deposition
  kernel. Holds a cumulative-energy lookup table indexed by φ angle and radial distance; per voxel
  calls `CalcSphereTrace()` integrating along radial rays via LUT interpolation, normalized by mass
  density and azimuthal sum (`1/NUM_THETA`). Loaded from `*_kernel.dat`.
- **SphereConvolve** (`SphereConvolve.h:8`) — Spherical-convolution manager: builds direction/offset
  LUTs from kernel geometry (`ComputeSphereLUT()`), drives per-voxel `CalcSphereTrace()` and
  `InterpCumEnergy()`. Implements dose = TERMA ⊗ kernel.

### RtModel — Histograms
- **CHistogram** (`Histogram.cpp:24`) — Dose-volume histogram with Gaussian smoothing. Bins dose
  into fixed-width array, applies dual Gaussian kernels (varMin/varMax) to form smoothed `GBins`;
  keeps fractional volume weights for linear interpolation between bin levels (`ConvGauss()`,
  `GetBinForValue()`).
- **CHistogramWithGradient** (`HistogramGradient.cpp:13`, `Get_dBins()` :135) — Extends `CHistogram`
  with per-beamlet derivative volumes (`m_arr_dVolumes`) grouped by beam; computes bin derivatives
  by product rule and smooths them (`Get_dGBins()`/`Conv_dGauss()`). This is the dose→histogram
  partial-derivative store driving the gradient chain.

### RtModel — Multi-Scale
- **PlanPyramid** (`PlanPyramid.cpp:16`) — Coarse-to-fine hierarchy of `MAX_SCALES=4` plans (each
  doubling dose resolution and beamlet spacing). `InvFiltIntensityMap()` transfers intensity maps
  between levels using a binomial filter `[0.25, 0.50, 0.25]` (:20).
- **PlanOptimizer** (`PlanOptimizer.cpp:45`, `Optimize()` :100) — Orchestrates per-level
  (Prescription, DynamicCovarianceOptimizer) pairs from coarse→fine, inverse-filtering the state
  vector into the next finer level (:146). `Get/SetStateVectorFromPlan` move beamlet weights in/out.

### RtModel — Data Model
- **Plan** (`Plan.cpp:13`) — Treatment-plan container: beams, dose volume, resampled mass density,
  energy kernel (default 6.0 MeV). `GetTotalBeamletCount()` sums across beams.
- **Beam** (`Beam.cpp:24`) — Single beam: gantry angle, isocenter, 1-D intensity map, computed dose;
  `OnIntensityMapChanged()` regenerates beamlets from weights.
- **Series** (`Series.cpp:13`) — CT density volume + array of `Structure` ROIs.
- **Structure** (`Structure.cpp:32`) — ROI as polygon spatial-object contours; `ContoursToRegion()`
  rasterizes to multi-scale binary volumes (`MAX_SCALES=5`); type `eTARGET`/`eOAR`.
- **PlanXmlReader / PlanXmlWriter** (`PlanXmlFile.cpp`) — ITK-XML serialization of the whole plan.

### Brimstone
- **CBrimstoneApp** — MFC app entry/init. **CBrimstoneDoc** — owns Series+Plan, document
  serialization. **CBrimstoneView** / **CPlanarView** — 2-D CT/contour/dose rendering.
  **CSeriesDicomImporter** — DICOM CT + RT structure-set import. **OptThread** — background
  optimization worker driving `PlanOptimizer`. **OptimizerDashboard** — live optimization metrics.
  **CMainFrame** — frame, prescription toolbar.

### Graph
- **CGraph** — interactive 2-D plot window (axes, legend, draggable points). **CDataSeries** —
  abstract curve container. **CHistogramDataSeries** — binned DVH series. **CTargetDVHSeries** —
  target/prescription overlay. **CDib** — DIB bitmap render helper.

### Foundation libraries
- **MTL:** `CVectorD`, `CMatrixD`, `CMatrixNxM` — templated vectors/matrices + least-squares solvers.
- **VecMat:** `VectorBase`/`VectorD`/`VectorN`, `MatrixBase`/`MatrixD`/`MatrixNxM` (inline `.inl`).
- **FTL:** `BufferND`, `MeshND`, `Field`, `BufferIndex` — N-D grid/buffer containers.
- **GEOM_BASE:** `Polygon` (serializable/observable), `LookupFunction`, `ScalarFunction`,
  `CVolume` (`Volumep.h`, 3-D voxel grid).
- **MODEL_BASE:** `CModelObject` (observable/serializable base), `CObservableObject`/
  `CObservableEvent` (Observer pattern), collections/value metadata.

### Component / mid-level libraries
- **GEOM_MODEL:** `Mesh`, `Cluster` (k-means), `AffineTransform`, `TPS` (thin-plate spline),
  `GradientCalculator`, `Dib`.
- **GEOM_VIEW:** `Camera`, `Light`, `Renderable`, `SceneView` (D3D8), `ObjectExplorer`, rotate/zoom trackers.
- **OPTIMIZER_BASE:** 1-D line-search optimizers — Brent, Powell, conjugate gradient (templates).
- **OptimizeN:** N-D optimizers — Brent, Powell, ConjGrad, DFP, gradient descent, cubic interpolation.
- **OGL_BASE:** `COpenGLRenderer`, `COpenGLView`, `COpenGLCamera`, `COpenGLLight`, `COpenGLTexture`, trackers.
- **GUI_BASE:** `DrawTool`, `Graph`, `Dib`, `ObjectExplorer`/`ObjectTreeItem`, `TabControlBar`.
- **GenImaging:** `InPlaneResampleImageFilter`, `IntensityMapAccumulateImageFilter`,
  `ContoursToRegionFilter`, `ScalarImageToWeightedHistogramGenerator`, `MultiMaskNegatedImageFilter`
  (ITK `ImageToImageFilter` subclasses).
- **XMLLogging:** `CXMLLogFile` (thread-safe singleton, FILE* output), `CXMLElement` (scoped tags),
  `CXMLLoggableObject`.
- **FieldCOM:** `Mesh`(`IMesh`), `MeshSet`, `BufferField`, `Polygon3D`, `PolygonSet3D`,
  `AffineTransform`, `TPSTransform`, `FileStorage` (COM/ATL).

### Legacy RT / simulation
- **RT_MODEL:** precursor classes mirroring RtModel — `Plan`, `Series`, `Beam`, `BeamDoseCalc`,
  `Structure`, `Histogram`, `Prescription`, `ConjGradOptimizer`, `TCP_NTCP_Optimizer`, `VOITerm`,
  `KLDivTerm`, `DicomImporter`.
- **RT_VIEW:** `BeamRenderable`, `MachineRenderable`, `BeamParamCollimCtrl`, `BeamParamPosCtrl`,
  `LightfieldTexture` (D3D8).
- **VSIM_MODEL:** `Plan`, `Series`, `Beam`, `TreatmentMachine` (MFC-serializable sim model).
- **VSIM_OGL:** `simview`/`MainFrm` app integrating OGL_BASE + VSIM_MODEL + FieldCOM export.

### Utilities / research
- **DivFluence:** fluence-integration routines (attenuation, inverse-square, divergence over `CVolume`).
- **PenBeamEdit:** Doc/View/Frame MFC editor for beam-parameter `CVolume` documents.
- **PenBeam_indens:** Fortran routines — `energy_lookup`, `interp_energy`, density ray-trace, spherical convolve.
- **python/read_process_series.py:** `ct_to_md_values` (HU→mass density), `rotate_density_for_beam`
  (beam-angle rotation), `terma_from_density` (TERMA ray-trace framework).
- **notebook_zoo:** entropy-maximization and MNIST autoencoder/VAE experiment notebooks.

---

## Embeddings

Two Ollama (`nomic-embed-text`, 768-dim) embedding sets accompany this inventory:

| File | Granularity | Generator |
|------|-------------|-----------|
| `python/component_embeddings.json` | one vector per component (26) | `python/embed_components.py` |
| `python/class_embeddings.json` | one vector per key class/algorithm | `python/embed_classes.py` |

Each class entry carries `library`, `name`, `kind` (class / algorithm / function), the source
`text`, and the `embedding`. Regenerate with a running Ollama daemon:
`python python/embed_classes.py`.
