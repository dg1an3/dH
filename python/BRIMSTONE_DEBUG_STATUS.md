# Brimstone debugging session status (2026-07-15)

## Goal

Automate Brimstone end-to-end with pywinauto: launch the app, import a DICOM
series, run Plan Setup (beam creation + beamlet dose-calc), set structure
prescriptions, and start the optimizer. The automation script is
`python/automate_brimstone_ui.py`. Getting it to run cleanly surfaced a long
chain of independent, pre-existing bugs in the app itself, all of which are
now fixed. The full flow now runs to completion through all four pyramid
levels (see "Sixth crash" below).

## Test data

- DICOM series: `C:\Users\Derek\Downloads\hnum_dicom_micro\dicom_micro`
  (5 CT slices + 1 RTSTRUCT, `HNUM.*.IMA` files). Recovered from the Import
  DICOM dialog's own last-visited-folder registry MRU
  (`HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ComDlg32\LastVisitedPidlMRU`,
  the `Brimstone.exe` entry, decoded via `SHGetPathFromIDListW`).
- Default isocenter for this dataset: `(218.0, 218.0, -172.0)`.
- RTSTRUCT ROI names (via `pydicom`, now installed): `External`, `Parotid(L)`,
  `Mandible`, `Spinal Cord`, `Parotid(R)`, `Spinal Cord 0.5`, `GTV`,
  `IsoCenter`, `PTV`, `IsoCenter2`. The automation script prescribes `PTV`
  (Target) and `Spinal Cord` (OAR).

## Fixes made this session (all applied to source; see build state below)

**Optimizer math / `RtModel`:**
- `ConjGradOptimizer.cpp` — near-zero-gradient fallback direction changed from
  `RandomVector(get_x_tolerance(), ...)` to `RandomVector(R(1.0), ...)`: the
  old magnitude (a convergence tolerance, ~1e-4) made the fallback direction
  too short for `vnl_brent_minimizer`'s fixed-width initial bracket to
  resolve, tripping its `fb < fa` assert.
- `ConjGradOptimizer.cpp` — `UpdateDynamicCovariance()` now reads the
  diagonal `covar(nDim, nDim)` instead of summing the whole column when
  computing `m_vAdaptVariance`, restoring the Rayleigh-quotient bound that
  keeps it within `[varMin, varMax]`.
- `Prescription.cpp` — `CalcSumSigmoid()`'s clamped `actVar` is now written
  back into `m_ActualAV[nAt_dVolume]` (previously only the local `actVar` was
  clamped; the member array kept the unclamped value, which is what's
  actually exposed downstream as `m_pAV` to `HistogramGradient`).
- `Prescription.cpp` — removed the unconditional `totalSum += 0.1` padding in
  `operator()`. It was polluting the CG loop's relative convergence test
  (`ConjGradOptimizer.cpp`, `fabs(m_FinalValue)+fabs(new_fv)`) and the
  free-energy display value.
- `PlanOptimizer.cpp` — `varMax` in `SetupPrescription()` widened from
  `binVar` to `binVar * 1.5`, to cover the true peak (~1.405x, derived
  analytically) of `varSlope²·varWeight²` in `CalcSumSigmoid`, which the old
  bound was clipping under routine conditions.
- `PlanOptimizer.cpp` — replaced the single flat `DEFAULT_TOLERANCE = 1e-3`
  with per-level arrays `DEFAULT_CG_TOLERANCE[]` / `DEFAULT_LINE_TOLERANCE[]`
  = `{1e-3, 1e-4, 1e-5, 1e-6, 1e-6}`, tightening from coarsest to finest
  pyramid level so later levels don't converge prematurely.
- `ConjGradOptimizer.cpp` — the direction handed to the Brent line search is
  now a separately-normalized copy (`vLineDir = m_vDir; vLineDir.normalize();`)
  rather than `m_vDir` itself. `vnl_brent_minimizer::minimize(x)` always
  brackets with a fixed unit step `[x-1, x+1]` regardless of direction
  magnitude, so an unnormalized `m_vDir` (raw gradient / Polak-Ribiere
  direction, arbitrarily small at any iteration, not just the first) produces
  an imperceptible step and ties `fa`/`fb`/`fc`, tripping the same assert
  family (`fb < fa` at line 201, `fb < fc` at line 202 of
  `vnl_brent_minimizer.cxx`). `m_vDir` itself is left unnormalized since the
  CG recurrence depends on its true scale; `m_lineFunction.GetDirection()` is
  re-read after the line search, so this is self-consistent.
- `ConjGradOptimizer.cpp` — added a `_finite(lambda) && _finite(new_fv)`
  guard after `m_optimizeBrent.minimize(0)`. Even after the normalization
  fix, a tied bracket can still occur near genuine convergence (gradient
  small in every direction, not just badly scaled) — `assert(fb<fa)` is
  compiled out in Release, so a non-finite `lambda` from the parabolic
  interpolation would otherwise silently corrupt `m_FinalParameter` and
  cascade into heap corruption downstream. Non-finite values now just treat
  the iteration as no improvement.
- `Histogram.cpp` (`GetBins`) / `HistogramGradient.cpp` (`Get_dBins`) — added
  bounds guards on the bin index (`nLowBin`/`nBin`) read from a precomputed
  binning volume before using it to index `m_arrBinsVarMax`/`arr_dBins`.
  `nBins` is sized fresh from the histogram's current max dose value each
  call, while the bin-index volume is computed separately; if a structure's
  DVH is painted before any dose exists yet (e.g. right after
  `AddHistogram()`, before optimization has run), these can disagree and the
  raw index can be out of range, corrupting memory via an out-of-bounds
  array write.

**Intensity-map sizing (the sixth crash — root cause):**
- `BrimstoneDoc.cpp` (`OnGenbeamlets`) — the level-0 intensity map was sized to
  a fixed `1` (`vWeights.SetDim(1); // 39);`, with `[0 /*19*/]` and `// 70.0`
  alongside — leftover scaffolding, present since the initial check-in
  `be223b6`), while `CPlanSetupDlg`'s `CalculateBeamlets` loops `-19..19` and
  so creates **39** level-0 beamlets in `m_arrBeamlets`. `Beam::SetIntensityMap`
  sizes only the map buffer and never touches `m_arrBeamlets`, so nothing
  reconciled the two: `GetBeamletCount()` returned 39 while the map buffer held
  1 element. Now sized per-beam from `GetBeamletCount()`, with the center
  beamlet set (`SetDim` zero-fills the rest).

  Why it corrupted the heap: the map size and `GetBeamletCount()` are read
  *independently* downstream. `PlanOptimizer::InvFilterStateVector` `ConformTo`s
  its output buffer to the level-0 **map** (1 element), while
  `PlanPyramid::InvFiltIntensityMap` drives its write extent from
  `GetBeamletCount()/2 = 19`, writing indices `0..38` — a 38-element /
  152-byte overrun, on *every* CG iteration (via the `OnIteration` callback).
  `PlanOptimizer::SetStateVectorToPlan` -> `StateVectorToIntensityMap(0, ...)`
  overran the same way. `PlanPyramid.cpp`'s `ASSERT` already encoded this exact
  size contract, but asserts are compiled out in Release — which is why it
  overran silently there. Corroborating symptom: `Beam::GetDoseMatrix`'s
  `map size == m_arrBeamlets.size()` guard meant the initial dose matrix was
  silently *never computed at all* (1 != 39).
- `PlanPyramid.cpp` (`InvFiltIntensityMap`) — promoted those Release-compiled-out
  `ASSERT`s to a real bail-out guard, so any future size mismatch is a no-op
  rather than silent heap corruption. Defense-in-depth; inert once the sizing
  above is correct.

**Build configuration:**
- `Brimstone.vcxproj`, `Graph.vcxproj` — `Release|x64` had never been
  migrated off the legacy ITK 3.x source-tree include layout
  (`Code\Algorithms`, `Utilities\vxl`) and referenced three nonexistent
  sibling projects (`GenImaging`, `RtOptimizer`, `OptimizeN`). Fixed to match
  `Debug|x64`'s vcpkg ITK-5.4 layout. `RtModel.vcxproj`'s `Release|x64` was
  already correct.
- `Brimstone.vcxproj` — `Release|x64`'s `PostBuildEvent` now also copies the
  IPP runtime DLLs (`ipps*.dll`, `ippcore*.dll`), matching `Debug|x64`.

**UI/display:**
- `BrimstoneView.cpp` (`OnOptimizerThreadUpdate`) — the iteration-vs-objective
  graph did `-log10(pOID->m_ofvalue - 0.1)` guarded by `m_ofvalue > 0.1`,
  assuming the old `+0.1` padding was still present. Now
  `-log10(pOID->m_ofvalue)` guarded by `> 0.0`.
- `BrimstoneDoc.cpp` (`OnFileImportDcm`) — `GetStructureAt(0)` was called
  unconditionally after import and dereferenced without checking
  `GetStructureCount() > 0`. `Series::GetStructureAt` calls
  `m_arrStructures.at(nAt)`, which throws `std::out_of_range` on an empty
  vector; uncaught through the raw MFC command-dispatch callback, this became
  a fatal callback exception. Root cause of the empty-structures state was
  the automation script only submitting a partial/malformed file selection to
  the Import dialog (see below) — but the missing bounds check is a real bug
  regardless of that trigger, now fixed with a `GetStructureCount() > 0`
  guard.

## `python/automate_brimstone_ui.py` fixes

- `launch_app()`'s window matcher was `title_re=".*Brimstone.*"`, which also
  matches Visual Studio's title bar when it has the solution open
  (`"Brimstone_src - ... - Microsoft Visual Studio"`). Anchored to
  `".* - Brimstone$"` (the actual MFC SDI doc-title format).
- `import_dicom()`: the Explorer-style Open dialog's "File name" field has a
  hard `EM_LIMITTEXT` of **259 characters** (confirmed empirically — classic
  `MAX_PATH - 1`). Typing all filenames directly (even without repeating the
  folder path) exceeds this for anything but a couple of short filenames —
  these DICOM filenames alone total ~347 characters for the 6-file test
  series. Rewritten to navigate into the folder via the filename field, then
  click directly into the file-listing pane (`DirectUIHWND`, `found_index=0`,
  clicked at an offset well right of the nav-tree sidebar) before `Ctrl+A` —
  selection state has no such length limit, unlike the text field.
- Confirmed structures import correctly now: a clean isolated test showed all
  10 ROIs in the `IDC_STRUCTSELECT` dropdown.

## Reproduction / debugging workflow

Normal run:
```
cd E:\github_dg1an3\dH\python
python automate_brimstone_ui.py
```
`EXE_PATH` is currently set to `x64\Release\Brimstone.exe` (see build state
below — Debug is stale). `DICOM_FOLDER`, `ISOCENTER_XYZ`, and
`STRUCTURE_PRESCRIPTIONS` are already configured for the test dataset.

To catch a crash with a real stack trace (used repeatedly this session,
works well for genuine access violations / heap corruption; less reliable
for `assert()`-driven aborts, which pop a modal message box that needs
`Retry` — not `Abort` — clicked to actually break into the debugger):

```bash
# 1. Launch under cdb (background) -- runs normally/UI-responsive until a break/exception
"/c/Program Files (x86)/Windows Kits/10/Debuggers/x64/cdb.exe" -o -g -G -c "g;kb 25;q" \
  "E:\github_dg1an3\dH\x64\Release\Brimstone.exe" > cdb_output.log 2>&1   # run_in_background

# 2. Separately, attach pywinauto to the already-running instance and drive it
#    (see scratchpad/connect_and_full.py in this session's scratchpad dir for
#    the pattern -- retry-loop `Application(backend="win32").connect(title_re=".* - Brimstone$", visible_only=True)`
#    for ~60s since cdb-launched startup is slower, ~30-40s to load all ITK/DCMTK/gdcm modules)

# 3. grep the cdb log for "RetAddr" / "Call Site" to find the captured stack
```

Notes on this workflow:
- `cdb`'s `-c "g;kb N;q"` command list only advances past `g` on a genuine
  break/exception. Windows common-dialog COM noise (first-chance exceptions,
  many `ModLoad` lines) is normal and doesn't stop it.
- For `assert()` dialogs: `Abort` just terminates the process with no
  exception for cdb to catch. `Retry` triggers a JIT-style `int 3` that cdb
  (already attached) catches directly — but note the *following* `kb` can
  still get swallowed by a nested modal-dialog message loop if another
  assert immediately follows (this happened with the `fb<fa`/`fb<fc` pair).
  Reading the actual library source (vcpkg keeps full sources under
  `C:\vcpkg\buildtrees\<port>\src\...`) was more productive than fighting
  that in a couple of cases.
- `connect()`/`window()` calls can raise `ElementAmbiguousError` if
  `visible_only=True` isn't passed — MFC apps commonly own a hidden
  secondary top-level window besides the visible main frame.
- Watch out for the RDP/lock-screen "no active desktop" `SetCursorPos`
  failure if the session has been locked — `pywinauto` mouse-move calls
  raise `RuntimeError` in that state even though a screen-share/RDP viewer
  may still show the last rendered frame.

## Build state (as of this writing)

Both **Release** and **Debug** (`x64\{Release,Debug}\Brimstone.exe`) are
current and reflect **all** fixes above, including the intensity-map sizing
fix. Rebuild either with:
```
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" \
  "E:\github_dg1an3\dH\Brimstone_src.sln" /p:Configuration=Debug /p:Platform=x64 /m
```
(NOTE: run this from PowerShell, not Git Bash — bash mangles the `/p:` switches
into paths and MSBuild fails with `MSB1008: Only one project can be specified`.)

Both configs build clean (0 errors; only pre-existing signed/unsigned
comparison warnings in `ConjGradOptimizer.cpp`/`BrimstoneDoc.cpp`).

## Sixth crash — FIXED (2026-07-15, later the same day)

The **sixth** crash, found via the cdb workflow above, running the Release
binary through the full automated flow (import → plan setup → prescriptions →
start optimizer). Same `0xC0000374` (`STATUS_HEAP_CORRUPTION`) signature as
earlier crashes, but a genuinely different call path — in the multi-scale beam
intensity-map transfer code, **not** anything touched by the other fixes:

```
dH::PlanOptimizer::Optimize
 -> DynamicCovarianceOptimizer::minimize
 -> COptThread::OnIteration                 (per-CG-iteration UI-update callback)
 -> dH::PlanOptimizer::InvFilterStateVector
 -> dH::PlanOptimizer::StateVectorToIntensityMap
 -> ConformTo (inline)
 -> itk::ImageBase<1>::SetDirection
 -> vnl_determinant<double,1,1>
 -> operator new                            (heap corruption detected here, not necessarily where it originated)
```

The `operator new` at the bottom is only where the LFH *detected* an
already-corrupt heap — not where the overrun happened. The actual cause was
the level-0 intensity-map / beamlet-count mismatch described under
"Intensity-map sizing" above; the detecting allocation is simply the first
`operator new` following the bad write (`ConformTo` -> `SetDirection` ->
`vnl_determinant` -> `new`, on the next iteration of `InvFilterStateVector`'s
beam loop). Nothing was wrong with ITK's 1D direction-matrix handling.

**Verification of the fix** (Release, same cdb + pywinauto flow):
- No `c0000374`, no exception of any kind; the process stayed alive under cdb
  through the full 180s optimize window (previously cdb caught the break and
  exited).
- The optimizer ran the complete coarse-to-fine cascade — the state-vector
  dimension progresses **25 → 45 → 95 → 195** in order, i.e. 5 → 9 → 19 → 39
  beamlets × 5 beams, independently confirming the level-0 count is 39.
  771 objective evaluations total, 206 of them at level 0 (the path that
  used to corrupt the heap).
- No NaN/Inf anywhere in the 4.8MB log (the pre-fix run's log died at 297KB).
- Level-0 `vInputTrans[195]` shows a coherent interpolated intensity profile,
  and the DVH shows real dose (`buffer_in[170] = 0.9471`, vs the degenerate
  `buffer_in[151] = 1000.0000` before the fix). This is what distinguishes the
  sizing fix from the `InvFiltIntensityMap` bail-out guard merely masking the
  overrun: `GetDoseMatrix` only computes dose when
  `map size == m_arrBeamlets.size()`, so a real DVH proves the map really is
  39 elements and the guard is inert.

## Known remaining cleanup (not a crash, not investigated)

`BrimstoneDoc.cpp` `OnGenbeamlets` still computes `max` via
`VoxelMax`/`GetBeamlet(0)` and never uses it — dead debug scaffolding that
also null-derefs if the Plan Setup dialog is cancelled (`GetBeamlet(0)`
returns NULL when no beamlets were generated). Same class as the
`GetStructureAt(0)` bug fixed above. Left alone to keep the heap-corruption
fix focused.
