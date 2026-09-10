# WebView2 merge crash — 7-beam knee run (handoff)

Date: 2026-07-23
Branch: `feature/webview2-planar-vtk` @ `35e6355` (merge of `main` into the branch)
Binary: `x64/Release/Brimstone.exe` built 2026-07-23 21:21 from `35e6355`

## Symptom

Running `run_brimstone_knee_7beam.bat` (separable entropy, W=2e-4, 7 beams) on
this branch: the driver completes plan setup and starts the optimizer, then
Brimstone **crashes during optimization** before writing the objective file. The
batch prints:

```
launched Brimstone pid=27084 (separable, W=2e-4, 7 beams)
optimizer started; window will stay open
Brimstone exited before writing a result
```

## Crash signature (Windows Application event log)

```
Faulting application name: Brimstone.exe, version 1.0.0.1
Faulting module name: ntdll.dll
Exception code: 0xc0000374        <-- STATUS_HEAP_CORRUPTION
Fault offset: 0x0000000000112165  (ntdll heap path)
```

Reproducible: two APPCRASH records (21:23:33 and an earlier attempt). WER archive:
`C:\ProgramData\Microsoft\Windows\WER\ReportArchive\AppCrash_Brimstone.exe_82284b4d...`
Report Id `60121e70-963b-4cb2-972b-f46a7c35fdd0`.

`0xc0000374` = heap corruption detected by the allocator — a double-free /
buffer overrun / mismatched-allocator bug, not a plain access violation. The
faulting frame is the *detector* (ntdll), not the culprit; the corrupting write
happened earlier. Need PageHeap to catch it at the source (see below).

## What this is NOT

- **Not the batch / driver.** All UI-automation steps succeed; it reaches
  "optimizer started". `run_brimstone_knee_7beam.bat` + `python/run_knee.py` are
  working.
- **Not the knee parameters.** The *identical* config converged cleanly on the
  pre-merge experiment-branch build earlier today:
  `7 beams, separable, W=2e-4, deterministic -> free_energy=-0.01029, kl=0.005140838, entropy=77.16`
  (that instance ran to convergence and stayed open; pid 36228).
- **Not startup.** The fresh `35e6355` binary launched standalone stays alive
  (>5 s, no plan) — the crash is triggered by the optimize path specifically.

## ROOT CAUSE FOUND (2026-07-24) — it is NOT the webviews

The original "two WebView2 hosts" hypothesis (below) is **wrong** and is retained
only for the record. Two facts disprove it:

1. **Both webviews already coexisted on the branch before this merge.** The
   planar parent of the merge (`083cd1b`) already declared all three hosts
   (`m_webChart`, `m_webDVH`, `m_webPlanar`) in `BrimstoneView.h`. `m_webDVH`
   landed in `3eaaa75`, `m_webPlanar` in `083cd1b`. **`BrimstoneView.cpp` is
   byte-identical across the merge** (`git diff 083cd1b 35e6355 -- Brimstone/BrimstoneView.cpp`
   is empty). The merge did not touch any webview code.
2. **The merge's only optimize-path code changes are inert for this run.** The
   merge changed `RtModel/Prescription.cpp` + `HistogramGradient.cpp` to route
   the sigmoid scale / input scale through `SigmoidParams.h`. With
   `BRIMSTONE_SIGMOID_SCALE` / `BRIMSTONE_INPUT_SCALE` **unset** (the knee batch
   sets neither) these resolve to the historical `0.2` / `0.5` — byte-identical
   behavior. They cannot corrupt the heap (scalar values, no buffer sizing).

What the merge actually brought is the **knee automation itself**
(`run_brimstone_knee_7beam.bat`, `python/run_knee.py`) from `main`. That is why
the crash appears "new": this is the first time the optimize loop was driven on
the planar branch after the planar webview was scaffolded (which happened in the
merge's own parent commit `083cd1b`). The bug is a **pre-existing cross-thread
data race**, newly *exposed*, not newly *introduced*.

### The actual bug: UI thread mutates Plan state while the worker optimizes

- The optimizer runs on `COptThread` (worker). Each iteration, `OnIteration`
  (`Brimstone/OptThread.cpp`) `new`s a `COptIterData`, fills `m_vParam`, and
  **`PostMessage`s** it to the view — fire-and-forget — then keeps optimizing.
- On the UI thread, `OnOptimizerThreadUpdate` (`BrimstoneView.cpp:705`) calls
  `m_pOptimizer->SetStateVectorToPlan(pOID->m_vParam)`, which
  (`RtModel/PlanOptimizer.cpp:312`) rewrites **every beam's intensity map**
  (`OnIntensityMapChanged()` → dose recompute) and then **`UpdateAllHisto()`**
  (rebuilds all histogram buffers). It then reads those buffers in
  `SendDvhCurvesToDvh()`.
- Because the update was `PostMessage`d, the worker **does not wait**: it is
  simultaneously reading/rewriting the same Plan / dose / histogram ITK-image and
  `CVectorN` buffers to evaluate the objective + gradient. Two threads realloc
  the same heap blocks → `STATUS_HEAP_CORRUPTION` (0xc0000374). The `ntdll`
  faulting frame is just the allocator detecting the damage later.
- The planar WebView2 host is only a **timing amplifier**: the extra live host
  makes the UI handler take longer, widening the overlap window and flipping this
  probabilistic race from "usually survives" (main / experiment builds) to
  "usually crashes." `BRIMSTONE_DETERMINISTIC=1` (set by the knee batch) removes
  the worker's message-yield, sustaining the overlap for the whole run.

### Fix applied

`Brimstone/OptThread.cpp`: changed the `WM_OPTIMIZER_UPDATE` (and `WM_OPTIMIZER_DONE`)
sends from `PostMessage` to **`SendMessage`**, so the worker is parked until the UI
finishes `SetStateVectorToPlan` / `UpdateAllHisto` / `SendDvhCurvesToDvh` — the two
threads never touch the shared Plan state concurrently. This also lets the worker
`delete pOID` after the send, fixing the per-iteration leak (the UI side never
freed it). No UI→worker blocking wait exists during a run, so `SendMessage` cannot
deadlock.

**VERIFIED (2026-07-24).** Rebuilt Release|x64 and reran
`run_brimstone_knee_7beam.bat`: the optimizer **ran to convergence with no crash**
(previously it corrupted the heap mid-optimize). Result matches the known-good
pre-merge baseline: `free_energy=-0.0102949 kl=0.00514012 entropy=77.175`
(pre-merge was `-0.01029 / 0.005140838 / 77.16`). Optional extra hardening: rerun
once under PageHeap (`gflags /p /enable Brimstone.exe /full`, `/p /disable` after)
to prove zero residual corruption, but the serialization makes the race
impossible by construction.

---

## Original (DISPROVEN) hypothesis — kept for the record

The crash is a **regression from merging `main` into `feature/webview2-planar-vtk`**.

- This branch already hosts the **planar-view WebView2** control.
- The merge brought in `main`'s **DVH WebView2** (PR #51, "interactive structure
  editor").
- During optimization, `OnOptimizerUpdate` calls `SendDvhCurvesToDvh()` every
  iteration (`Brimstone/BrimstoneView.cpp`), pushing data into the DVH webview.
- So two WebView2 hosts are now live and one is driven hard in the optimize loop.
  Heap corruption during that loop points at the DVH-WebView2 update/marshalling
  path or an allocator mismatch between the two webview hosts.

Neither webview alone had shown this: `main` has the DVH webview but not planar;
the planar branch had planar but not the DVH webview. The combination is new.
*(Both bullets above are factually wrong — see ROOT CAUSE.)*

## Repro

```
git checkout feature/webview2-planar-vtk    # @ 35e6355
# build Release|x64
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" \
  Brimstone_src.sln -t:Build -p:Configuration=Release -p:Platform=x64 -m
# run
run_brimstone_knee_7beam.bat
# -> heap-corruption crash mid-optimization; check Application event log
```

## Next steps (pick up here)

1. **Confirm branch-specificity.** Rebuild `main` (DVH webview, no planar) and run
   the same batch. Expected: converges (as the pre-merge build did). Confirms the
   crash needs planar-VTK + DVH-WebView2 together. Also yields the real 7-beam
   knee number on a good build.
2. **Isolate the DVH webview.** Re-run with `SendDvhCurvesToDvh()` short-circuited
   (early return). If the crash disappears, the DVH-webview update path is the
   corrupter.
3. **Catch it at the source.** Enable PageHeap for the exe so the corrupting
   write faults immediately instead of later in ntdll:
   `gflags /p /enable Brimstone.exe /full`  (disable with `/p /disable` after).
   Then run under a debugger / WinDbg and read the offending stack.
4. **Suspects to read:** the two WebView2 host controls' init/teardown and their
   buffer handoff (JSON/string marshalling to the webview), and whether both
   share or fight over an allocator (CRT vs webview runtime). Look at the planar
   WebView2 host added on this branch vs the DVH host from PR #51 for a
   double-free on view destruction or a shared static.

## Artifacts from today

- `notes/entropy_weight_sweep.md` — the W sweep + the 1e-4->3e-4 gap fill
  (knee = ~2e-4; KL dead-zone scatter ~7%, H is the reliable signal). In `main`.
- `run_brimstone_knee.bat` (minimal, manual plan) and
  `run_brimstone_knee_7beam.bat` + `python/run_knee.py` (full automation). In `main`.
- Sweep orchestrator (per-W detached launch + attach + objective-file wait) was
  run out of tree; not committed.

## Environment

- MSBuild: VS 18 Community (`...\MSBuild\Current\Bin\amd64\MSBuild.exe`)
- Driver python: `C:\Users\Derek\.pyenv\pyenv-win\versions\3.11.9\python.exe` (has pywinauto 0.6.9)
- DICOM micro dataset: `C:\Users\Derek\Downloads\hnum_dicom_micro\dicom_micro`
- Prescription (in `python/automate_brimstone_ui.py`): PTV 60-70 Gy w2.5;
  Spinal Cord 0-30 w2.5; Parotid(L/R) 0-30 w0.15; External 10-50 w0.15 prio2.
