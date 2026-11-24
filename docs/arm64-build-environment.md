# ARM64 Build Environment and Port Status

Build-environment knowledge from the Windows-on-ARM64 development VM, recorded
**2026-11-23** when that VM was retired. The observations date from
**2026-07-20 through 2026-09-05**; treat file paths, library names, and version
pins as a point-in-time snapshot and re-verify before relying on them.

The VM was a **Parallels guest running Windows on ARM64**, hosted on an Apple
Silicon MacBook Pro, with **4 GB RAM** and an auto-managed (~1.8 GB) pagefile.
Several constraints below follow from that host and are not visible from the
source.

## Environment constraints

1. **Intel IPP can never be used on ARM64.** Intel ships no ARM64 build, so the
   `USE_IPP` preprocessor define must stay off. `RtModel/include/VectorOps.h`
   provides generic template fallbacks for every IPP specialization, so this
   costs SIMD performance but nothing functional. Verified 2026-07-20: `RtModel`
   compiles past the `#ifdef USE_IPP` block cleanly.

2. **MFC/ATL ships only under MSVC toolset `14.51.36231`.** The default
   `14.52.36520` has no `atlmfc` directory at all. Without pinning
   `<VCToolsVersion>14.51.36231</VCToolsVersion>`, MFC projects fail on
   `afxwin.h` with an error that looks like MFC isn't installed when it is.

3. **RAM was the binding constraint on dependency builds.** See the GDCM heap
   wall below.

4. **`Release|x64` cannot be built on this VM at all** (confirmed 2026-09-05,
   fails on the first translation unit). The x64 configs define `USE_IPP`,
   include `ipp.h`, and point at `C:\vcpkg\installed\x64-windows\include\ITK-5.4`
   — none of which can exist here, since there is no Intel IPP for ARM64 and the
   local vcpkg carries only the `arm64-windows` triplet. **Do not treat an x64
   build failure on an ARM64 machine as a code regression.**

5. **No Python and no Node were installed** (verified 2026-09-05). `python` /
   `python3` on PATH were only the Microsoft Store alias stubs under
   `%LOCALAPPDATA%\Microsoft\WindowsApps`; no `py` launcher, no venv, no conda.
   Ollama was absent too, so the `python/embed_*.py` scripts (which need a local
   `nomic-embed-text` daemon) could not run. Everything under `python/` — the
   pybrimstone bindings, dataset loaders, UI automation, the UMAP pipeline — was
   therefore unrunnable on that VM as-is. For serving a static viewer directory,
   a PowerShell `System.Net.HttpListener` script works as a `serve.py`
   stand-in and needs no elevation for a `http://localhost:<port>/` prefix.

6. **`git` was the Visual Studio 2026 bundled build** (VS "18" Community,
   2.54.0.windows.2) at
   `...\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd\git.exe`.
   That `Git\cmd` directory was appended to the **user** PATH on 2026-09-05.
   There was no `gh` CLI, so pull requests had to be opened in the browser.

## Dependencies

Third-party dependencies came from vcpkg at **`C:\vcpkg`** — a standalone clone,
**not** the VS-bundled copy at
`C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg` that
`VCPKG_ROOT` points at. The project files hardcode
`C:\vcpkg\installed\<triplet>\...` include paths.

Packages required: `itk` (5.4, for ITK + VNL), `dcmtk` (Brimstone's
`SeriesDicomImporter.cpp` includes `<dcmtk/dcmdata/...>`), and
`webview2` (which pulls in `wil`).

### The GDCM heap wall

vcpkg `gdcm:arm64-windows` dies with `fatal error C1002: compiler is out of heap
space in pass 2` on `gdcmTagToVR.cxx` — a huge generated tag/VR table — even at
`-j1`. The root cause is the commit ceiling: 4 GB RAM plus a ~1.8 GB pagefile is
about 5.8 GB, and that one translation unit at `/O2` exceeds it. This was the
*only* heap-killer seen; all 14 ITK dependencies and roughly 300 ITK translation
units compiled fine.

The intended fix is **raising the pagefile** (C: had ~134 GB free; the command
needs elevation). An alternative is building the dependencies on a 16 GB GitHub
Actions runner — WarpTps already cross-compiles ARM64 on `windows-2022` — though
a cache hit on an ARM64 VM would require pinning the runner to toolset
`14.51.36231` and a matching vcpkg baseline.

### Dead end — do not retry

Skipping GDCM via `-DModule_ITKGDCM=OFF` / `-DModule_ITKIOGDCM=OFF` in an ITK
overlay port **does not work**. Verified 2026-07-21 from ITK's `CMakeCache`: the
flags do register as `OFF`, but `ITK_BUILD_DEFAULT_MODULES=ON` force-builds every
non-remote module anyway, so `ITKIOGDCM` still compiles. The stock `itk`
portfile also does `file(REMOVE_RECURSE .../ThirdParty/GDCM/src)`, so with system
GDCM absent the module then fails with `C1083: Cannot open include file:
'gdcmImageHelper.h'`.

### A GDCM-less ITK that does build

Achieved 2026-07-21 via an overlay port at `C:\vcpkg\overlay-ports\itk`:

- `ITK_BUILD_DEFAULT_MODULES=OFF`
- **`ITKGroup_Core=OFF`** — this was the real re-enabler. The group defaults ON
  and drags in `ITKTestKernel`, whose `PRIVATE_DEPENDS` force-build every IO
  format including `ITKIOGDCM`. `Module_ITKIOGDCM=OFF` alone never worked.
- Explicit `Module_ITK{Common,ImageFunction,SpatialObjects,Transform,ImageCompose,ImageGrid,ImageIntensity,Path,Smoothing,IOImageBase,IOXML,RegistrationCommon}=ON`,
  a module list derived from a header-to-module audit of RtModel, Graph, and
  Brimstone.

Result: 35 modules, no GDCM. This also resolved RtModel's VNL blocker —
`vnl_vector_ref.h` lands at `include\vxl\core\vnl\`.

### RTK forces GDCM back

Including RTK (Reconstruction Toolkit) is a project goal. vcpkg builds it as the
`itk` `rtk` feature (`itk[core,rtk]`, ARM64-supported; CUDA is separate, `tools`
adds the apps). But RTK's `itk-module.cmake` hard-DEPENDS on `ITKIOGDCM` +
`ITKGDCM` (verified from the pinned commit), so the GDCM-less ITK above cannot
host RTK. The plan agreed 2026-07-21: validate the Brimstone/RtModel build
against the GDCM-less ITK first, then rebuild ITK **with** GDCM + RTK once the
pagefile is raised.

### VXL library renames

System-VXL uses different library names than the vendored ITK-4.3 build did.
Apply to `Brimstone.vcxproj`'s ARM64 `AdditionalDependencies`:

| Old | New |
| --- | --- |
| `itkvnl-5.4.lib` | `vnl.lib` |
| `itkvcl-5.4.lib` | `vcl.lib` |
| `itkvnl_algo-5.4.lib` | `vnl_algo.lib` |
| `itkv3p_netlib-5.4.lib` | `v3p_netlib.lib` |

`ITKCommon-5.4.lib` and `itksys-5.4.lib` keep their names.

## Port status

Native ARM64 configurations were chosen over cross-compiled x64 because the
optimizer is CPU-bound and the IPP SIMD paths are unavailable regardless.

**Phase A complete.** `Debug|ARM64` built and linked 2026-07-21;
`Release|ARM64` of all three projects built clean on 2026-09-05 — 0 errors, 45
warnings — producing `ARM64\Release\{Graph.lib,RtModel.lib,Brimstone.exe}` plus
the 63-file dependency-DLL post-build copy. That was verified on `main`
immediately after merging `arm64-port`, `tooling`, `docs/reorg`, and
`umap-embedding-viewer-clean`, so the merged state is what was built.

Build invocation (the toolset pin is still required — 14.52 ships no `atlmfc`):

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
    <project> /p:Configuration=Release /p:Platform=ARM64 /p:VCToolsVersion=14.51.36231
```

### Changes that made it build

- `Debug|ARM64` + `Release|ARM64` added to `RtModel.vcxproj`, `Graph.vcxproj`,
  `Brimstone.vcxproj`, and `Brimstone_src.sln`. The ARM64 configs omit `USE_IPP`
  and all Intel include/lib paths, pointing at
  `C:\vcpkg\installed\arm64-windows\...` instead.
- Each project's per-file `stdafx.cpp` `<PrecompiledHeader>Create` override
  enumerates configurations explicitly, so **ARM64 entries had to be added** or
  every translation unit fails with "Cannot open precompiled header".
- All three projects need `C:\vcpkg\installed\arm64-windows\include\eigen3` on
  the ARM64 `AdditionalIncludeDirectories`: ITK's
  `itkSymmetricEigenAnalysis.h` does `#include <Eigen/Eigenvalues>`, and
  system Eigen lives under the `eigen3/` subdirectory.
- `Brimstone.vcxproj` ARM64 link set additionally needs `ITKTransform-5.4.lib`,
  `ITKSpatialObjects-5.4.lib`, and `WebView2Loader.dll.lib`. ITK is built
  SHARED/dllimport, so the module-dependency chain must be walked by hand —
  `TransformBaseTemplate<double>` pulls ITKTransform, `SpatialObjectProperty`
  pulls ITKSpatialObjects.
- `Brimstone.vcxproj` ARM64: `<RandomizedBaseAddress>` set to `true` (was
  `false`). **ARM64 mandates ASLR** — `/DYNAMICBASE:NO` gives `LNK1246`. The
  empty `<DataExecutionPrevention/>` was removed.
- `Brimstone.vcxproj`: `PrescriptionBar.cpp` excluded from both ARM64 configs.
  It is legacy C++/CLI managed WinForms, already `ExcludedFromBuild` for x64,
  but had no ARM64 entry — so ARM64 was wrongly compiling it as native.
- `PrescriptionToolbar.cpp` / `WebView2Host.cpp` are **real, used code**
  (`CPrescriptionToolbar m_wndPrescToolBar` in `MainFrm`) and need `WebView2.h`.
  The header sits under the `include` root; link `WebView2Loader.dll.lib`, and
  the existing PostBuildEvent copies `WebView2Loader.dll` at runtime.
- Header fixes benefiting all projects: `Brimstone/stdafx.h` guards
  `<ipps.h>` / `<ippcv.h>` with `#ifdef USE_IPP`; `RtModel/include/VectorN.h`
  adds `#include <utilmacros.h>` so `RTM_TRACE` (used by `Trace()`) is
  self-contained regardless of include order.

### Resume plan — Phase B

Needs the raised pagefile, so after a reboot:

1. Rework the ITK overlay to build `itk[core,rtk]` **with** GDCM — revert the
   default-modules-off / `ITKGroup_Core=OFF` / minimal-module changes, add the
   `rtk` feature, restore system GDCM. The larger pagefile is what lets GDCM's
   `gdcmTagToVR.cxx` and RTK's heavy translation units compile.
2. Add RTK libraries and includes to Brimstone, then rebuild.

### Context worth knowing

The pre-existing `Debug|x64` config of `Brimstone` links **no** ITK or DCMTK
libraries at all — only IPP and system libraries — so it could never have
linked. The x64 migration was abandoned partway, which means there is no
known-good configuration to diff the ARM64 link step against.
