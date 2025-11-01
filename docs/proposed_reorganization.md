# Proposed Repository Reorganization

## Executive Summary

**YES, the repository CAN be reorganized** with common libraries moved to a shared subfolder. However, the key insight is that **Brimstone and Brimstone_original share almost NO common libraries** - they use completely different architectural approaches.

## Current Architecture Analysis

### Modern Stack (Brimstone)
- **Self-contained by design**
- RtModel embeds all math/optimizer code
- Only depends on: RtModel + Graph
- Uses ITK's VNL for linear algebra

### Classic Stack (Brimstone_original)
- **Layered architecture**
- Depends on: RT_MODEL, Graph_original, GEOM_MODEL, MTL, OptimizeN, XMLLogging, DicomImEx
- Modular but more complex

### The Critical Insight
**There are NO truly shared foundation libraries between the two stacks!**
- MTL → Only used by classic stack
- GEOM_MODEL → Only used by classic stack
- OptimizeN → Only used by classic stack
- XMLLogging → Only used by classic stack
- Graph vs Graph_original → Different versions for each stack

## Proposed Folder Structure

```
dH/
├── Brimstone_src.sln                    (main production solution)
│
├── src/                                  (MODERN STACK - Production)
│   ├── Brimstone/                        (main application)
│   ├── RtModel/                          (self-contained RT library)
│   └── Graph/                            (DVH visualization)
│
├── src_classic/                          (CLASSIC STACK - Legacy)
│   ├── Brimstone_original/               (legacy application)
│   ├── RT_MODEL/                         (original RT library)
│   ├── Graph_original/                   (legacy DVH viz)
│   └── DicomImEx/                        (DICOM utilities)
│
├── lib_classic/                          (CLASSIC FOUNDATION - Only used by classic)
│   ├── MTL/                              (Math Template Library)
│   ├── GEOM_MODEL/                       (Geometry modeling)
│   ├── OptimizeN/                        (Optimization algorithms)
│   └── XMLLogging/                       (XML logging)
│
├── lib_foundation/                       (TRULY SHARED - Minimal or unused)
│   ├── FTL/                              (Field Template Library - rarely used)
│   └── GenImaging/                       (ITK wrapper - standalone)
│
├── utils/                                (UTILITIES - Standalone tools)
│   ├── PenBeamEdit/                      (pencil beam editor app)
│   ├── DivFluence/                       (fluence calculation tools)
│   ├── XMLLogging/
│   │   ├── XMLConsole/                   (XML console app)
│   │   └── TestXMLLogging/               (test app)
│   └── VSIM_OGL/                         (OpenGL visualization app)
│
├── tests/                                (TEST APPLICATIONS)
│   ├── MTL/
│   │   └── TestMTL/
│   ├── GEOM_MODEL/
│   │   ├── ClusterTest/
│   │   ├── TestRegion/
│   │   ├── TestRotate/
│   │   └── TestIliffe/
│   ├── OptimizeN/
│   │   └── Test/
│   ├── RT_MODEL/
│   │   ├── GenBeamlets/                  (beamlet generator)
│   │   ├── GenDens/                      (density generator)
│   │   ├── TestHisto/
│   │   └── Test/
│   ├── VecMat/
│   │   └── TestVec/
│   └── VSIM_MODEL/
│       └── Test/
│
├── deprecated/                           (OBSOLETE CODE - Archive)
│   ├── VecMat/                           (superseded by MTL)
│   ├── VecMat_original/
│   ├── GEOM_BASE/                        (superseded by GEOM_MODEL)
│   ├── MODEL_BASE/                       (superseded by GEOM_MODEL)
│   ├── OPTIMIZER_BASE/                   (now in OptimizeN)
│   ├── GUI_BASE/                         (not in active solutions)
│   ├── OGL_BASE/                         (not in active solutions)
│   ├── GEOM_VIEW/                        (not in active solutions)
│   ├── RT_VIEW/                          (not in active solutions)
│   └── FieldCOM/                         (not in active solutions)
│
├── python/                               (PYTHON UTILITIES)
│   └── read_process_series.py            (ITK-based CT processing)
│
├── notebook_zoo/                         (JUPYTER NOTEBOOKS)
│   └── entropy_max.sln
│
├── docs/                                 (DOCUMENTATION)
│   ├── CLAUDE.md
│   ├── README.md
│   ├── repository_structure.md
│   ├── brimstone_comparison.md
│   └── proposed_reorganization.md
│
└── external/                             (EXTERNAL REFERENCES - Not moved)
    └── (References to ITK, DCMTK, VNL remain as $(VAR))
```

## Key Organizational Principles

### 1. Separation by Architecture
- **src/** = Modern self-contained stack (Brimstone production)
- **src_classic/** = Classic modular stack (Brimstone_original legacy)
- **lib_classic/** = Foundation libraries ONLY used by classic stack
- **lib_foundation/** = Truly shared libraries (actually very few!)

### 2. Utilities vs Applications
- **utils/** = Standalone tools that can work independently
- Applications that are part of main system stay in src/

### 3. Tests Grouped by Library
- Tests organized under parent library they test
- Easy to find tests for any library

### 4. Clear Deprecation
- **deprecated/** folder makes it obvious what's obsolete
- Preserves code for historical reference
- Can be deleted once confirmed unnecessary

## Path Update Strategy

### Modern Stack (src/)
Update these project files:
- `src/Brimstone/Brimstone.vcxproj`
- `src/RtModel/RtModel.vcxproj`
- `src/Graph/Graph.vcxproj`

Path changes:
```diff
- AdditionalIncludeDirectories="..\Graph\include;..\RtModel\include"
+ AdditionalIncludeDirectories="..\Graph\include;..\RtModel\include"  (no change within src/)
```

Main solution at root:
```diff
- Project("Brimstone") = "Brimstone\Brimstone.vcxproj"
+ Project("Brimstone") = "src\Brimstone\Brimstone.vcxproj"
- Project("RtModel") = "RtModel\RtModel.vcxproj"
+ Project("RtModel") = "src\RtModel\RtModel.vcxproj"
- Project("Graph") = "Graph\Graph.vcxproj"
+ Project("Graph") = "src\Graph\Graph.vcxproj"
```

### Classic Stack (src_classic/)
Update these project files:
- `src_classic/Brimstone_original/Brimstone.vcproj`
- `src_classic/RT_MODEL/RT_MODEL.vcproj`
- `src_classic/Graph_original/Graph.vcproj`
- `src_classic/DicomImEx/DicomImEx.vcproj`

Path changes:
```diff
- AdditionalIncludeDirectories="..\Graph\include;..\RT_MODEL\include;..\GEOM_MODEL\include;..\MTL;..\XMLLogging"
+ AdditionalIncludeDirectories="..\Graph_original\include;..\RT_MODEL\include;..\..\lib_classic\GEOM_MODEL\include;..\..\lib_classic\MTL;..\..\lib_classic\XMLLogging"
```

### Classic Foundation Libraries (lib_classic/)
Update these project files:
- `lib_classic/MTL/MTL.vcproj`
- `lib_classic/GEOM_MODEL/GEOM_MODEL.vcproj`
- `lib_classic/OptimizeN/OPTIMIZER_BASE.vcproj`
- `lib_classic/XMLLogging/XMLLogging.vcproj`

Path changes (if they reference each other):
```diff
- AdditionalIncludeDirectories="..\MTL"
+ AdditionalIncludeDirectories="..\MTL"  (no change within lib_classic/)
```

## Migration Steps

### Phase 1: Preparation (Low Risk)
1. **Create new directories** (don't move anything yet)
   ```bash
   mkdir src src_classic lib_classic lib_foundation utils tests deprecated docs
   ```

2. **Copy (don't move) modern stack to src/**
   ```bash
   cp -r Brimstone src/
   cp -r RtModel src/
   cp -r Graph src/
   ```

3. **Update paths in copied src/** projects
   - Test that copied modern stack builds
   - Verify paths are correct

4. **Test build from root solution**
   ```bash
   # Update Brimstone_src.sln to point to src/
   msbuild Brimstone_src.sln /p:Configuration=Debug
   ```

### Phase 2: Move Modern Stack (Once verified)
5. **Delete original directories** (only after successful build)
   ```bash
   rm -rf Brimstone RtModel Graph
   ```

### Phase 3: Classic Stack
6. **Copy classic stack to src_classic/**
   ```bash
   cp -r Brimstone_original src_classic/
   cp -r RT_MODEL src_classic/
   cp -r Graph_original src_classic/
   cp -r RT_MODEL/DicomImEx src_classic/
   ```

7. **Copy classic foundation to lib_classic/**
   ```bash
   cp -r MTL lib_classic/
   cp -r GEOM_MODEL lib_classic/
   cp -r OptimizeN lib_classic/
   cp -r XMLLogging lib_classic/
   ```

8. **Update paths in classic projects**
   - Update include paths to point to `..\..\lib_classic\`
   - Test classic solution builds

9. **Delete original directories** (after verification)

### Phase 4: Utilities and Tests
10. **Move utilities to utils/**
    ```bash
    mv PenBeamEdit utils/
    mv DivFluence utils/
    mv XMLLogging/XMLConsole utils/XMLLogging/
    mv VSIM_OGL utils/
    ```

11. **Move tests to tests/**
    - Group by parent library
    - Update project paths

### Phase 5: Deprecation
12. **Move deprecated code**
    ```bash
    mv VecMat VecMat_original GEOM_BASE MODEL_BASE deprecated/
    mv OPTIMIZER_BASE GUI_BASE OGL_BASE deprecated/
    mv GEOM_VIEW RT_VIEW FieldCOM deprecated/
    ```

13. **Add README in deprecated/**
    - Explain why code is deprecated
    - List what superseded each component
    - Provide date of deprecation

### Phase 6: Documentation
14. **Move documentation to docs/**
    ```bash
    mv CLAUDE.md README.md *.md docs/
    ```

15. **Update root README** with new structure

## Build System Updates Required

### Main Solution (Brimstone_src.sln)
```xml
<?xml version="1.0" encoding="utf-8"?>
<Solution>
  <Project Name="RtModel" Path="src\RtModel\RtModel.vcxproj" GUID="{7C848FBB-2C50-4F47-81C9-B872E9B504C1}"/>
  <Project Name="Brimstone" Path="src\Brimstone\Brimstone.vcxproj" GUID="{80AB664E-B56A-4A59-91D6-A8201FC00859}">
    <ProjectReference Include="src\RtModel\RtModel.vcxproj"/>
    <ProjectReference Include="src\Graph\Graph.vcxproj"/>
  </Project>
  <Project Name="Graph" Path="src\Graph\Graph.vcxproj" GUID="{21687638-397B-43A4-B462-C71A741C0C04}">
    <ProjectReference Include="src\RtModel\RtModel.vcxproj"/>
  </Project>
</Solution>
```

### Classic Solution (src_classic/Brimstone_classic.sln - renamed)
```xml
<?xml version="1.0" encoding="utf-8"?>
<Solution>
  <!-- Update all paths with src_classic/ and lib_classic/ prefixes -->
  <Project Name="MTL" Path="..\lib_classic\MTL\MTL.vcproj"/>
  <Project Name="GEOM_MODEL" Path="..\lib_classic\GEOM_MODEL\GEOM_MODEL.vcproj"/>
  <Project Name="OptimizeN" Path="..\lib_classic\OptimizeN\OPTIMIZER_BASE.vcproj"/>
  <Project Name="XMLLogging" Path="..\lib_classic\XMLLogging\XMLLogging.vcproj"/>
  <Project Name="RT_MODEL" Path="RT_MODEL\RT_MODEL.vcproj"/>
  <Project Name="Graph" Path="Graph_original\Graph.vcproj"/>
  <Project Name="DicomImEx" Path="DicomImEx\DicomImEx.vcproj"/>
  <Project Name="Brimstone" Path="Brimstone_original\Brimstone.vcproj"/>
</Solution>
```

## Benefits of This Organization

### 1. Clear Architectural Separation
- **src/** = Modern production code (clean, simple)
- **src_classic/** = Legacy code (complex, modular)
- **lib_classic/** = Makes it obvious these libs are only for legacy stack

### 2. Foundation Libraries NOT Falsely Shared
- **lib_classic/** accurately represents that MTL, GEOM_MODEL, etc. are ONLY used by classic stack
- Avoids misconception that these are "shared foundation"
- **lib_foundation/** is nearly empty (FTL, GenImaging) - accurately showing minimal true sharing

### 3. Easy Deprecation Path
- Classic stack can eventually be entirely removed
- Just delete src_classic/ and lib_classic/ folders
- Modern stack in src/ remains clean

### 4. Utilities Clearly Identified
- Standalone tools in utils/ can be built independently
- Tests in tests/ organized by component

### 5. Maintains Build-ability
- Both stacks remain fully buildable after migration
- Relative paths updated but structure preserved within each stack
- Main production solution at root points to src/

## Risk Assessment

### Low Risk
- ✅ Modern stack migration (only 3 projects, simple dependencies)
- ✅ Creating new directories
- ✅ Documentation organization
- ✅ Moving deprecated code

### Medium Risk
- ⚠️ Classic stack migration (7+ projects with complex dependencies)
- ⚠️ Updating all include paths in classic projects
- ⚠️ Test projects (many test apps to update)

### High Risk
- ❌ Accidentally breaking builds during migration
- ❌ Missing path updates in obscure configuration files

## Risk Mitigation Strategy

1. **Always copy before move** - keep originals until verified
2. **Test after each phase** - don't move to next phase if build breaks
3. **Use version control** - commit after each successful phase
4. **Start with modern stack** - simpler, production-critical
5. **Classic stack is lower priority** - can take time to migrate
6. **Keep deprecated code** - don't delete, just move to deprecated/

## Rollback Plan

If migration fails at any phase:
1. Delete new directory structure
2. Revert solution files to original paths
3. Original files still in place (never deleted until verified)
4. Git rollback if committed

## Estimated Effort

- **Phase 1-2 (Modern stack)**: 2-4 hours
- **Phase 3 (Classic stack)**: 4-8 hours
- **Phase 4 (Utilities/Tests)**: 4-6 hours
- **Phase 5 (Deprecation)**: 1-2 hours
- **Phase 6 (Documentation)**: 1-2 hours
- **Testing and verification**: 2-4 hours

**Total: 14-26 hours of work**

## Recommendation

**YES - Proceed with reorganization** using this structure:

### Immediate (Phase 1-2): Modern Stack
- Move Brimstone, RtModel, Graph to src/
- Update Brimstone_src.sln
- Test build thoroughly
- **This gets production code organized cleanly**

### Short-term (Phase 3): Classic Stack
- Move Brimstone_original, RT_MODEL, Graph_original to src_classic/
- Move MTL, GEOM_MODEL, OptimizeN, XMLLogging to lib_classic/
- Update classic solution
- **This preserves legacy code in organized fashion**

### Medium-term (Phase 4-5): Cleanup
- Move utilities and tests
- Move deprecated code
- **This completes organization**

### Optional Future: Deprecate Classic Stack
- Once modern Brimstone is proven feature-complete
- Remove src_classic/ and lib_classic/ entirely
- Simplify to just src/, utils/, tests/, deprecated/

## Alternative: Minimal Reorganization

If full reorganization is too risky, do minimal version:

```
dH/
├── src/                    (Brimstone, RtModel, Graph)
├── src_classic/            (Everything else related to classic stack)
├── utils/                  (Standalone utilities)
└── deprecated/             (Obsolete code)
```

This still achieves:
- Clear production code location (src/)
- Separation from legacy (src_classic/)
- Minimal path updates required
- Easy to execute (lower risk)
