# Brimstone Build Notes — Getting It Working on Modern Toolchain

## Environment

- Windows 11, Visual Studio 2022 BuildTools (not Community — Community lacks MFC)
- vcpkg providing ITK 5.4, DCMTK, VXL/VNL
- Intel oneAPI IPP 2022.0 (system-installed)
- NuGet package `intelipp.devel.win-x64.2022.3.0.387`

---

## Build System Fix

**Problem:** MFC not found by VS 2022 Community.  
**Fix:** Use BuildTools MSBuild instead:
```
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe
```

**Problem:** NuGet IPP package (`intelipp.devel.win-x64.2022.3.0.387`) is missing its `.targets` file, and its headers are just `iw*` sub-headers — `ipp.h` itself is absent.  
**Fix:**
1. Created stub `.targets` file at `packages\intelipp.devel.win-x64.2022.3.0.387\build\native\intelipp.devel.win-x64.targets`
2. Injected the oneAPI IPP include paths via a `ForceImportBeforeCppTargets` props file at build time:

```xml
<!-- %TEMP%\ipp_override.props -->
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>
        C:\Program Files (x86)\Intel\oneAPI\ipp\2022.0\include;
        C:\Program Files (x86)\Intel\oneAPI\ipp\2022.0\include\ipp;
        %(AdditionalIncludeDirectories)
      </AdditionalIncludeDirectories>
      <AdditionalOptions>/FS /Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
  </ItemDefinitionGroup>
</Project>
```

Build command:
```powershell
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
& $msbuild Brimstone_src.sln /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal `
  "/p:ForceImportBeforeCppTargets=$env:TEMP\ipp_override.props"
```

Note: `/FS` fixes parallel-build PDB contention; `/Zc:__cplusplus` makes DCMTK's C++11 preprocessor check pass.

---

## Brimstone.vcxproj Changes (x64 configurations)

### Library paths (replacing old `$(ITK_BUILD_DIR)` / `$(DCMTK_BUILD_DIR)` variables)

**Debug|x64 — link:**
```
AdditionalDependencies: dcmimgle.lib;dcmdata.lib;ofstd.lib;oflog.lib;
  ITKCommon-5.4.lib;ITKSpatialObjects-5.4.lib;ITKVNLInstantiation-5.4.lib;
  itksys-5.4.lib;ippi.lib;ipps.lib;ippvm.lib;ippcore.lib;
  ws2_32.lib;netapi32.lib

AdditionalLibraryDirectories: C:\vcpkg\installed\x64-windows\debug\lib;
  C:\Program Files (x86)\Intel\oneAPI\ipp\2022.0\lib
```

**Release|x64 — link:**
```
AdditionalDependencies: (same lib names as Debug)

AdditionalLibraryDirectories: C:\vcpkg\installed\x64-windows\lib;
  C:\Program Files (x86)\Intel\oneAPI\ipp\2022.0\lib
```

### C++ standard

Added `<LanguageStandard>stdcpp17</LanguageStandard>` to the ClCompile section of both x64 configurations. ITK 5.4 requires C++17.

### PrescriptionBar excluded from build

`PrescriptionBar.cpp` uses C++/CLI (`System::Windows::Forms`, etc.) and cannot be compiled alongside native MFC code. Added `<ExcludedFromBuild>true</ExcludedFromBuild>` to its entry in the vcxproj.

---

## Source Code Changes

### ITK 5.x API Migration

ITK 5 renamed and changed several APIs used throughout the codebase.

**`GetPosition()` → `GetPositionInObjectSpace()`** — all spatial object points.  
Affected: `Brimstone/PlanarView.cpp` (11 occurrences).

**`SmartPointer` no longer accepts `NULL` / `0`** — must use `Type::Pointer()`.  
Affected: `Brimstone/PlanarView.cpp`, `Brimstone/PrescriptionToolbar.cpp`.
```cpp
// Before
SetSelectedContour(NULL);
// After
SetSelectedContour(dH::Structure::PolygonType::Pointer());
```

**`AddPoint` signature changed** — now requires a `SpatialObjectPoint<N>`, not a raw `itk::Point`.  
Affected: `Brimstone/PlanarView.cpp`, `Brimstone/SeriesDicomImporter.cpp`.
```cpp
// Before
pPoly->AddPoint(pt);  // pt is itk::Point<double,2>
// After
itk::SpatialObjectPoint<2> soPt;
soPt.SetPositionInObjectSpace(pt);
pPoly->AddPoint(soPt);
```

**`ReplacePoint` removed** — replaced with get-all / modify / set-all pattern.  
Affected: `Brimstone/PlanarView.cpp`.
```cpp
auto pts = GetSelectedContour()->GetPoints();
itk::Point<double, 2> vShift;
vShift[0] = spacing[0] * ptDelta.x;
vShift[1] = spacing[1] * ptDelta.y;
pts[GetSelectedVertex()].SetPositionInObjectSpace(vShift);
GetSelectedContour()->SetPoints(pts);
```

### DCMTK API Change

`findAndGetSint32` requires `Sint32`, not `long`.  
Affected: `Brimstone/SeriesDicomImporter.cpp` — three variables changed:
```cpp
// Before
long nROINumber, nRefROINumber, nContourPoints;
// After
Sint32 nROINumber, nRefROINumber, nContourPoints;
```

### vnl_vector assignment (VXL API)

`CVectorD` doesn't support `operator=(const vnl_vector<double>&)`. Fixed with element-wise copy.  
Affected: `Brimstone/OptThread.cpp`.
```cpp
const vnl_vector<REAL>& finalParam = pOpt->GetFinalParameter();
pOID->m_vParam.SetDim(finalParam.size());
for (unsigned int nP = 0; nP < finalParam.size(); nP++)
    pOID->m_vParam[nP] = finalParam[nP];
```

### IPP 2021+ Removed API

`ippiWarpAffineBack_32f_C1R` was removed in IPP 2021. Stubbed with a no-op and TODO comment.  
Affected: `Brimstone/PlanarView.cpp`.
```cpp
// TODO: replace with ippiWarpAffineLinear_32f_C1R + ippiWarpAffineLinearInit
IppStatus stat = ippStsNoErr;
(void)coeffs;
```

---

## Runtime: DLLs Required

Copy these alongside `Brimstone.exe` (from `x64\Release\`):

**From `C:\Program Files (x86)\Intel\oneAPI\ipp\2022.0\bin`:**
- `ippi.dll`, `ipps.dll`, `ippvm.dll`, `ippcore.dll`

**From `C:\Program Files (x86)\Intel\oneAPI\compiler\<ver>\bin`:**
- `libiomp5md.dll` (Intel OpenMP, required by IPP)

**From `C:\vcpkg\installed\x64-windows\bin`:**
- All DLLs (ITK, DCMTK, VXL, double-conversion, etc.)

**Kernel data files** (auto-copied by post-build event):
- `6MV_kernel.dat`, `15MV_kernel.dat`, `2MV_kernel.dat`

---

## Outstanding TODOs

- `ippiWarpAffineBack_32f_C1R` in `PlanarView.cpp` is stubbed — the affine warp in the planar view will not apply. Replace with `ippiWarpAffineLinear_32f_C1R` + `ippiWarpAffineLinearInit` (IPP 2021+ API).
- `PrescriptionBar` (C++/CLI WinForms panel) is excluded from build. Its functionality is unavailable.
- Debug|x64 cannot link: vcpkg's VXL/VNL debug libs are compiled with the Release CRT (`MD`, not `MDd`). Use Release|x64 for now.
