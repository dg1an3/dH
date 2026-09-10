# Building the native `rtmodel_core` extension

`rtmodel_core` is a [pybind11](https://pybind11.readthedocs.io) module that
exposes the RtModel optimizer (`Prescription`, `DynamicCovarianceOptimizer`,
`CVectorN`) to Python so you can drive the C++ objective function from
NumPy/SciPy. The binding source is [`rtmodel_bindings.cpp`](rtmodel_bindings.cpp).

> **Windows only.** RtModel is compiled against **MFC (Dynamic)**, **Intel
> IPP**, **ITK 5.x** and **VNL**. None of these build with GCC/Clang, so the
> extension is only producible with MSVC. On Linux/macOS use the pure-Python
> `pybrimstone` package instead; it has no compiled dependency.

## Build with CMake (the supported route)

The module is a target of the root CMake project and links the `RtModel`
target directly, so ITK/VNL/IPP/MFC settings come from one place
(`RtModel/CMakeLists.txt`) instead of being restated here.

Prerequisites: Visual Studio 2022 with C++ and MFC, CMake 3.21+, vcpkg with
`itk`, `vxl`, `dcmtk`, `webview2` installed for `x64-windows` (see
`CMakePresets.json` for where vcpkg is expected), Intel oneAPI IPP, and a
64-bit Python with `pip install pybind11 numpy`.

```bat
cd <repo>
cmake --preset vs2022-x64 -DDH_BUILD_PYTHON=ON -DPython_EXECUTABLE=<path to python.exe>
cmake --build --preset vs2022-x64-release --target rtmodel_core
ctest --test-dir build/vs2022-x64 -C Release
```

The module lands in `build/vs2022-x64/bin/Release/rtmodel_core.cp3XX-win_amd64.pyd`
next to the IPP, ITK and DCMTK runtime DLLs it needs. pybind11 is found through
the chosen interpreter (`python -m pybind11 --cmakedir`); pass
`-Dpybind11_DIR=...` to override.

### Importing it

Python 3.8+ does not consult `PATH` (nor the `.pyd`'s own directory for its
dependencies) when loading extension modules, so register the directory that
holds the module and its DLLs before importing:

```python
import os, sys
d = r"<repo>\build\vs2022-x64\bin\Release"
os.add_dll_directory(d)
sys.path.insert(0, d)
import rtmodel_core
```

The `rtmodel_core_import` CTest does exactly this.

## Legacy route: `setup.py`

[`setup.py`](setup.py) still builds the same module with `pip install -e .`
against a `RtModel.lib` produced by `msbuild Brimstone_src.sln /t:RtModel
/p:Configuration=Release /p:Platform=x64`, reading ITK/IPP locations from
`VCPKG_ROOT`, `ITK_VERSION`, `IPP_ROOT`, `RTMODEL_LIB_DIR`, `ITK_LIB_DIR`,
`IPP_LIB_DIR` (and the `RTMODEL_*` overrides documented in the file). It
duplicates the link line that the CMake target now owns and is slated for
removal once the CMake build replaces `Brimstone_src.sln` (ROADMAP.md,
section 0). The older Cython scaffold (`pybrimstone/core.pyx` / `core.pxd`) is
superseded and no longer built.
