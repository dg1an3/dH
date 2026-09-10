# Building the native `rtmodel_core` extension

`rtmodel_core` is a [pybind11](https://pybind11.readthedocs.io) module that
exposes the RtModel optimizer (`Prescription`, `DynamicCovarianceOptimizer`,
`CVectorN`) to Python so you can drive the C++ objective function from
NumPy/SciPy.

> **Windows only.** RtModel is compiled against **MFC (Dynamic)**, **Intel
> IPP**, **ITK 5.x** and **VNL**. None of these build with GCC/Clang, so the
> extension is only producible with MSVC (v143, matching `RtModel.vcxproj`).
> On Linux/macOS use the pure-Python `pybrimstone` package instead — it has no
> compiled dependency.

The binding source is [`rtmodel_bindings.cpp`](rtmodel_bindings.cpp); the build
is driven by [`setup.py`](setup.py). The older Cython scaffold
(`pybrimstone/core.pyx` / `core.pxd`) is **superseded** and no longer built.

---

## Prerequisites

| Component | Notes |
|-----------|-------|
| Visual Studio 2022 | "Desktop development with C++" **and** the **MFC** component (v143) |
| Python 3.8+ (64-bit) | Architecture must match the build (x64) |
| pybind11, setuptools, wheel, numpy | `pip install pybind11 setuptools wheel numpy` |
| ITK 5.x + VNL | Easiest via vcpkg: `vcpkg install itk:x64-windows` |
| Intel IPP | oneAPI IPP (Community edition is free) |

`setup.py` reads all external locations from environment variables, defaulting
to the same vcpkg/oneAPI paths used by `RtModel.vcxproj` (x64):

| Variable | Default | Meaning |
|----------|---------|---------|
| `VCPKG_ROOT` | `C:\vcpkg\installed\x64-windows` | vcpkg install tree (ITK + VNL) |
| `ITK_VERSION` | `5.4` | ITK include folder suffix (`include\ITK-<ver>`) |
| `IPP_ROOT` | `C:\Program Files (x86)\Intel\oneAPI\ipp\latest` | Intel IPP root |
| `RTMODEL_LIB_DIR` | `<repo>\x64\Release` | folder containing `RtModel.lib` |
| `ITK_LIB_DIR` | `%VCPKG_ROOT%\lib` | ITK/VNL import libraries |
| `IPP_LIB_DIR` | `%IPP_ROOT%\lib` | IPP import libraries |

Advanced overrides: `RTMODEL_INCLUDE_DIRS`, `RTMODEL_LIBRARY_DIRS`,
`RTMODEL_ITK_LIBS`, `RTMODEL_IPP_LIBS` (all `;`-separated),
`RTMODEL_SKIP_LIBCHECK=1` to bypass the `RtModel.lib` existence check.

---

## Build (two steps)

RtModel is a **static library**, so build it first, then build the extension
that links it. Run from an **"x64 Native Tools Command Prompt for VS 2022"** so
`msbuild` and `cl` are on `PATH`.

### 1. Build `RtModel.lib`

```bat
cd <repo>
msbuild Brimstone_src.sln /t:RtModel /p:Configuration=Release /p:Platform=x64
```

This produces `x64\Release\RtModel.lib` (the default `RTMODEL_LIB_DIR`).

### 2. Build and install the extension

```bat
cd <repo>\python
pip install pybind11 setuptools wheel numpy
pip install -e .
```

Or just compile in place:

```bat
python setup.py build_ext --inplace
```

If your ITK/IPP live elsewhere, set the variables first, e.g.:

```bat
set VCPKG_ROOT=D:\dev\vcpkg\installed\x64-windows
set IPP_ROOT=C:\Program Files (x86)\Intel\oneAPI\ipp\2021.11
pip install -e .
```

---

## Smoke test

```python
import numpy as np
import rtmodel_core as rc

print(rc.__doc__)

# CVectorN round-trips through NumPy
v = rc.VectorN(4)
for i in range(4):
    v[i] = float(i)
print(v.to_numpy())                 # [0. 1. 2. 3.]
print(rc.VectorN.from_numpy(np.arange(3.0)).to_numpy())
```

Once you have a `Prescription` (built from a `Plan`; see the roadmap below),
`PrescriptionWrapper` gives you a SciPy-friendly `(value, gradient)` callable:

```python
wrapper = rc.PrescriptionWrapper(presc)   # presc: rtmodel_core.Prescription
n = wrapper.get_dimension()
value, grad = wrapper.evaluate(np.zeros(n))

from scipy.optimize import minimize
res = minimize(lambda x: wrapper.evaluate(x), np.zeros(n), jac=True, method="L-BFGS-B")
```

---

## What's exposed today

`rtmodel_bindings.cpp` currently wraps:

- **`VectorN`** — `CVectorN<double>` with `__len__`/`__getitem__`/`__setitem__`
  and `to_numpy()` / `from_numpy()`.
- **`Prescription`** — `get_number_of_unknowns()`, `set_gbin_var()`, and the
  `operator()` objective (via `PrescriptionWrapper`).
- **`PrescriptionWrapper`** — `evaluate(x) -> (value, grad)`,
  `evaluate_value(x)`, `get_dimension()`.
- **`ConjGradOptimizer`** (`DynamicCovarianceOptimizer`) — `minimize()`,
  `set_adaptive_variance()`, `set_compute_free_energy()`,
  `get_final_value()`, `get_entropy()`, `get_free_energy()`,
  `get_adaptive_variance()`.

## Not yet wrapped (next steps)

There is no Python constructor for `Plan`/`Series`/`Structure` yet, so a
`Prescription` still has to be created on the C++ side. To make the module
usable end-to-end from Python, the follow-on work is:

1. Bind `dH::Plan` construction and beam/beamlet setup.
2. Bind `Series`/`Structure` plus ITK-volume ↔ NumPy conversion.
3. Add a `Prescription(plan)` constructor and `AddStructureTerm` / `KLDivTerm`
   so objectives can be defined from Python.
4. Wire the optimizer callback (`OptimizerCallback`) through to a Python
   `Callable` for live monitoring.

See `../CYTHON_WRAPPER_DESIGN.md` for the full intended API (the class shapes
there apply regardless of pybind11 vs Cython).

---

## Troubleshooting

- **`fatal error C1189: WINDOWS.H already included. MFC apps must not #include
  <windows.h>`** — the MFC headers must precede `Python.h`. `rtmodel_bindings.cpp`
  already includes `<afx.h>` etc. first; don't reorder those above the pybind11
  includes.
- **`LNK2019: unresolved external symbol` for `itk...` / `vnl...`** — ITK/VNL
  libraries weren't found. Check `ITK_LIB_DIR` and that the vcpkg triplet is
  `x64-windows`. Pin an exact set with `RTMODEL_ITK_LIBS`.
- **`LNK2038: mismatch detected for 'RuntimeLibrary'`** — everything must use
  the dynamic release CRT (`/MD`). Rebuild `RtModel.lib` in **Release|x64**
  (it uses `MultiThreadedDLL`) and use a release Python.
- **`ImportError: DLL load failed`** — the ITK, IPP and MFC runtime DLLs must
  be on `PATH` (or copied next to the `.pyd`) when importing.
