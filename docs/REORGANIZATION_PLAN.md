# dH Repository Reorganization Plan

## Context

The dH repo has grown to 35+ items at root level after merging pheonixrt and WarpTps. Active projects, foundation libraries, legacy code, and archived copies all sit side-by-side with no conceptual grouping. This plan organizes them into clear zones while preserving the active build (`Brimstone_src.sln`).

## Current State (~35 root items)

Everything is flat at the top level — active code, foundation libs, legacy archives, tools, and docs all mixed together.

## Target Structure

```
dH/
├── README.md, CLAUDE.md, LICENSE, .gitignore
├── Brimstone_src.sln                    # Main VS solution (unchanged)
├── docker-compose.yml
│
│   ── Active Modules ──────────────────
├── Brimstone/                           # MFC GUI app (stays)
├── RtModel/                             # Core optimization lib (stays)
├── Graph/                               # DVH visualization (stays)
├── WarpTps/                             # TPS warping module (stays)
│
│   ── Foundation Libraries ────────────
├── foundation/                          # pheonixrt support stack
│   ├── README.md
│   ├── VecMat/                          # Vector/matrix utilities
│   ├── MTL/                             # Math Template Library
│   ├── FTL/                             # Foundation Template Library
│   ├── GEOM_BASE/                       # Geometry primitives
│   ├── GEOM_MODEL/                      # Geometry modeling
│   ├── GEOM_VIEW/                       # Geometry visualization
│   ├── MODEL_BASE/                      # Base model framework
│   ├── OPTIMIZER_BASE/                  # Base optimizer framework
│   ├── OptimizeN/                       # N-dimensional optimization
│   ├── OGL_BASE/                        # OpenGL base classes
│   ├── GUI_BASE/                        # GUI base classes
│   ├── RT_VIEW/                         # Radiotherapy visualization
│   ├── VSIM_MODEL/                      # Visual simulation model
│   ├── VSIM_OGL/                        # Visual simulation OpenGL
│   └── XMLLogging/                      # XML logging framework
│
│   ── Tools ────────────────────────────
├── tools/                               # Specialized utilities
│   ├── README.md
│   ├── GenImaging/                      # Imaging generation
│   ├── DivFluence/                      # Divergent fluence
│   ├── PenBeamEdit/                     # Pencil beam editor (C++)
│   ├── PenBeam_indens/                  # Pencil beam (Fortran, 1988)
│   ├── FieldCOM/                        # COM-based field components
│   └── EGSnrc/                          # Monte Carlo kernel gen (Docker)
│
│   ── Modern / Cross-Platform ─────────
├── python/                              # pybrimstone (stays)
├── web/                                 # Web frontend (stays)
│
│   ── Documentation ───────────────────
├── docs/                                # Consolidated documentation
│   ├── (existing docs files)
│   ├── DEVELOPMENT_TIMELINE.md          # moved from root
│   ├── CYTHON_WRAPPER_DESIGN.md         # moved from root
│   ├── DOCKER.md                        # moved from root
│   ├── RT_VIEW_HISTORY.md               # moved from root
│   └── repository_structure.md          # moved from root
├── notebook_zoo/                        # Jupyter notebooks (stays)
│
│   ── Archive ─────────────────────────
└── archive/                             # Pre-reorganization snapshots
    ├── README.md
    ├── Brimstone_original/              # Pre-pheonixrt Brimstone
    ├── Graph_original/                  # Pre-pheonixrt Graph
    ├── VecMat_original/                 # Pre-pheonixrt VecMat
    ├── RT_MODEL/                        # VS2003-era RT model + tools
    └── CVSROOT/                         # CVS migration artifact
```

**Root goes from ~35 items → ~17 items.**

## Build Impact Analysis

### Active build (`Brimstone_src.sln`) — minimal impact

| File | References | Action |
|------|-----------|--------|
| `Brimstone_src.sln` | `RtModel\`, `Brimstone\`, `Graph\` (all staying) | **No changes** |
| `Brimstone/Brimstone.vcxproj` | `..\\Graph\\include`, `..\\RtModel\\include` (staying) + externals | **No changes** |
| `RtModel/RtModel.vcxproj` | Self-contained + externals | **No changes** |
| `Graph/Graph.vcxproj` | `..\\GenImaging\\include`, `..\\OptimizeN\\include` | **Update to** `..\\tools\\GenImaging\\include`, `..\\foundation\\OptimizeN\\include` |

### Legacy builds — will break (acceptable)

- `Brimstone/Brimstone.sln` (VS2005, .vcproj format) references GEOM_MODEL, OptimizeN, XMLLogging, RT_MODEL as siblings. Not actively used.
- Various test `.sln` files inside foundation libraries reference sibling paths (VS2003/2005 projects).

## Execution Steps

### Phase 1: Archive (no build impact)
```bash
mkdir archive
git mv Brimstone_original archive/
git mv Graph_original archive/
git mv VecMat_original archive/
git mv RT_MODEL archive/
git mv CVSROOT archive/
```

### Phase 2: Foundation libraries
```bash
mkdir foundation
git mv VecMat foundation/
git mv MTL foundation/
git mv FTL foundation/
git mv GEOM_BASE foundation/
git mv GEOM_MODEL foundation/
git mv GEOM_VIEW foundation/
git mv MODEL_BASE foundation/
git mv OPTIMIZER_BASE foundation/
git mv OptimizeN foundation/
git mv OGL_BASE foundation/
git mv GUI_BASE foundation/
git mv RT_VIEW foundation/
git mv VSIM_MODEL foundation/
git mv VSIM_OGL foundation/
git mv XMLLogging foundation/
```

### Phase 3: Tools
```bash
mkdir tools
git mv GenImaging tools/
git mv DivFluence tools/
git mv PenBeamEdit tools/
git mv PenBeam_indens tools/
git mv FieldCOM tools/
git mv EGSnrc tools/
```

### Phase 4: Documentation consolidation
```bash
git mv DEVELOPMENT_TIMELINE.md docs/
git mv CYTHON_WRAPPER_DESIGN.md docs/
git mv DOCKER.md docs/
git mv RT_VIEW_HISTORY.md docs/
git mv repository_structure.md docs/
```

### Phase 5: Cleanup
```bash
git rm _codeql_detected_source_root
```

### Phase 6: Fix build references

In `Graph/Graph.vcxproj`, update `AdditionalIncludeDirectories` (line 211):
- `..\\GenImaging\\include` → `..\\tools\\GenImaging\\include`
- `..\\OptimizeN\\include` → `..\\foundation\\OptimizeN\\include`

### Phase 7: Add READMEs
- `archive/README.md` — "Pre-reorganization snapshots, not for active development"
- `foundation/README.md` — Describe pheonixrt foundation library stack and dependency hierarchy
- `tools/README.md` — Describe specialized utilities

### Phase 8: Update project docs
- Update `CLAUDE.md` to reflect new directory paths
- Update `docs/repository_structure.md`

### Phase 9: Commit
Single commit: "Reorganize repository: group foundation libs, tools, archive, consolidate docs"

## Verification Checklist

- [ ] `Brimstone_src.sln` project paths resolve correctly
- [ ] Grep all active `.vcxproj` files for broken `..\\` references to moved directories
- [ ] `git status` shows only expected renames
- [ ] No references to moved directories remain in active build files
