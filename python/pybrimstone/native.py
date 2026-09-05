"""Load the native ``rtmodel_core`` extension from a CMake build directory.

Python 3.8+ resolves an extension module's DLL dependencies with a restricted
search (no ``PATH``, no working directory). ``os.add_dll_directory`` covers the
module's *direct* imports, but not the dependencies of those DLLs: Intel IPP's
``ipps.dll`` needs ``ippcore.dll`` and the CPU-dispatch ``ipps??.dll`` files,
and those are not found through the registered directory. Pre-loading
``ippcore.dll`` and ``ipps.dll`` by full path puts them in the process first,
after which the import succeeds. This module does that once and returns the
imported extension.

Usage::

    from pybrimstone.native import import_rtmodel_core
    rtmodel_core = import_rtmodel_core()            # searches build/*/bin/Release
    rtmodel_core = import_rtmodel_core(r"C:\\dev\\dH\\build\\vs2022-x64\\bin\\Release")

The CTest ``rtmodel_core_import`` (python/CMakeLists.txt) uses this helper.
"""
from __future__ import annotations

import ctypes
import glob
import os
import sys
from pathlib import Path
from types import ModuleType

_REPO = Path(__file__).resolve().parents[2]
_PRELOAD = ("ippcore.dll", "ipps.dll")


def find_build_dirs() -> list[str]:
    """Candidate directories holding rtmodel_core*.pyd, newest first."""
    pats = [
        str(_REPO / "build" / "*" / "bin" / "Release"),
        str(_REPO / "build" / "*" / "bin" / "RelWithDebInfo"),
        str(_REPO / "build" / "*" / "bin" / "Debug"),
    ]
    dirs = [d for p in pats for d in glob.glob(p) if glob.glob(os.path.join(d, "rtmodel_core*.pyd"))]
    dirs.sort(key=os.path.getmtime, reverse=True)
    return dirs


def import_rtmodel_core(module_dir: str | os.PathLike | None = None) -> ModuleType:
    """Import and return ``rtmodel_core`` from ``module_dir`` (or the newest build)."""
    if "rtmodel_core" in sys.modules:
        return sys.modules["rtmodel_core"]

    if module_dir is None:
        found = find_build_dirs()
        if not found:
            raise ImportError(
                "rtmodel_core not found under %s/build/*/bin/*; build it with "
                "cmake --preset vs2022-x64 -DDH_BUILD_PYTHON=ON (see python/BUILD_NATIVE.md)" % _REPO
            )
        module_dir = found[0]
    module_dir = str(Path(module_dir).resolve())

    if not glob.glob(os.path.join(module_dir, "rtmodel_core*.pyd")):
        raise ImportError("no rtmodel_core*.pyd in %s" % module_dir)

    if hasattr(os, "add_dll_directory"):
        os.add_dll_directory(module_dir)
    for name in _PRELOAD:
        path = os.path.join(module_dir, name)
        if os.path.exists(path):
            ctypes.WinDLL(path)

    if module_dir not in sys.path:
        sys.path.insert(0, module_dir)
    import rtmodel_core  # noqa: E402  (resolved via the path just added)

    return rtmodel_core
