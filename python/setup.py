"""
Build script for rtmodel_core - pybind11 bindings for the RtModel C++ library.

Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645

WINDOWS ONLY. RtModel is built against MFC (Dynamic linkage), Intel IPP, ITK
and VNL, none of which compile with GCC/Clang, so this extension can only be
produced with MSVC (the v143 toolset, matching RtModel.vcxproj).

Build model
-----------
RtModel itself is a *static library* (RtModel/RtModel.vcxproj -> RtModel.lib).
Rather than recompiling every RtModel .cpp here (each needs the stdafx.h
precompiled header, IPP, ITK, ...), this script compiles only the thin binding
translation unit `rtmodel_bindings.cpp` and *links the prebuilt RtModel.lib*
plus the ITK/VNL/IPP import libraries it depends on.

So the full build is two steps:

  1) Build RtModel.lib with MSBuild, e.g. from a "x64 Native Tools" prompt:
         msbuild ..\Brimstone_src.sln /t:RtModel /p:Configuration=Release /p:Platform=x64
  2) Build + install this extension:
         pip install -e .

All external paths are read from environment variables (with vcpkg defaults
that mirror RtModel.vcxproj's x64 configuration) so the build is portable
across machines without editing this file. See BUILD_NATIVE.md.
"""

import glob
import os
import sys
from pathlib import Path

from setuptools import setup

try:
    from pybind11.setup_helpers import Pybind11Extension, build_ext
except ImportError:  # pragma: no cover - build dependency
    sys.exit(
        "pybind11 is required to build rtmodel_core. Install the build "
        "dependencies first:\n    pip install pybind11 setuptools wheel numpy"
    )

# ---------------------------------------------------------------------------
# Platform guard -- fail early and clearly rather than deep inside the compiler.
# ---------------------------------------------------------------------------
if not sys.platform.startswith("win"):
    sys.exit(
        "rtmodel_core can only be built on Windows with MSVC.\n"
        "RtModel depends on MFC, Intel IPP, ITK and VNL, which are not\n"
        "available to GCC/Clang. On Linux/macOS use the pure-Python\n"
        "'pybrimstone' package instead (no compiled extension required)."
    )


def env_path(name: str, default: str) -> str:
    """Return an environment override or the vcpkg/oneAPI default."""
    return os.environ.get(name, default)


def env_paths(name: str, defaults):
    """Semicolon-separated env override, else the provided default list."""
    raw = os.environ.get(name)
    if raw:
        return [p for p in raw.split(os.pathsep) if p]
    return list(defaults)


PROJECT_ROOT = Path(__file__).resolve().parent.parent  # repo root (dH/)
RTMODEL_INCLUDE = str(PROJECT_ROOT / "RtModel" / "include")

# --- Third-party locations (defaults match RtModel.vcxproj, x64) ------------
VCPKG_ROOT = env_path("VCPKG_ROOT", r"C:\vcpkg\installed\x64-windows")
IPP_ROOT = env_path(
    "IPP_ROOT", r"C:\Program Files (x86)\Intel\oneAPI\ipp\latest"
)

itk_version = os.environ.get("ITK_VERSION", "5.4")

include_dirs = [
    RTMODEL_INCLUDE,
    # ITK + VNL/VXL (vcpkg layout)
    os.path.join(VCPKG_ROOT, "include", f"ITK-{itk_version}"),
    os.path.join(VCPKG_ROOT, "include", "vxl", "core"),
    os.path.join(VCPKG_ROOT, "include", "vxl", "vcl"),
    os.path.join(VCPKG_ROOT, "include"),
    # Intel IPP
    os.path.join(IPP_ROOT, "include"),
    os.path.join(IPP_ROOT, "include", "ipp"),
]
# Allow full override / extension of the include search path.
include_dirs = env_paths("RTMODEL_INCLUDE_DIRS", include_dirs)

# --- Library search paths ---------------------------------------------------
# RtModel.lib default output: <solution>/x64/Release (RtModel.vcxproj OutDir).
RTMODEL_LIB_DIR = env_path(
    "RTMODEL_LIB_DIR", str(PROJECT_ROOT / "x64" / "Release")
)
ITK_LIB_DIR = env_path("ITK_LIB_DIR", os.path.join(VCPKG_ROOT, "lib"))
IPP_LIB_DIR = env_path("IPP_LIB_DIR", os.path.join(IPP_ROOT, "lib"))

library_dirs = env_paths(
    "RTMODEL_LIBRARY_DIRS", [RTMODEL_LIB_DIR, ITK_LIB_DIR, IPP_LIB_DIR]
)


def lib_names(directory: str, patterns):
    """Collect .lib basenames (without extension) matching any glob pattern."""
    names = []
    for pat in patterns:
        for path in glob.glob(os.path.join(directory, pat)):
            names.append(Path(path).stem)
    return sorted(set(names))


# RtModel static library. The name is fixed; presence of the .lib is checked so
# the failure is a clear message rather than an obscure linker error.
libraries = ["RtModel"]
rtmodel_lib = os.path.join(RTMODEL_LIB_DIR, "RtModel.lib")
if not os.path.exists(rtmodel_lib) and "RTMODEL_SKIP_LIBCHECK" not in os.environ:
    sys.exit(
        f"RtModel.lib not found at {rtmodel_lib}.\n"
        "Build it first (step 1 in this file's docstring), or point\n"
        "RTMODEL_LIB_DIR at the directory that contains RtModel.lib.\n"
        "Set RTMODEL_SKIP_LIBCHECK=1 to bypass this check."
    )

# ITK is highly modular; link every ITK/VNL/ITKSYS import library found so the
# many symbols RtModel pulls in resolve. Over-linking is harmless for a static
# dependency. Users can pin an exact list via RTMODEL_ITK_LIBS.
itk_libs = os.environ.get("RTMODEL_ITK_LIBS")
if itk_libs:
    libraries += [x for x in itk_libs.split(os.pathsep) if x]
else:
    libraries += lib_names(
        ITK_LIB_DIR, ["ITK*.lib", "itk*.lib", "vnl*.lib", "vcl*.lib", "v3p*.lib"]
    )

# Intel IPP import libraries used by CVectorN (USE_IPP): core + signal + image.
ipp_libs = os.environ.get("RTMODEL_IPP_LIBS")
if ipp_libs:
    libraries += [x for x in ipp_libs.split(os.pathsep) if x]
else:
    found_ipp = lib_names(IPP_LIB_DIR, ["ippcore*.lib", "ipps*.lib", "ippi*.lib", "ippvm*.lib"])
    libraries += found_ipp if found_ipp else ["ippcore", "ipps", "ippi", "ippvm"]

# ---------------------------------------------------------------------------
# Compiler / preprocessor settings -- mirror RtModel.vcxproj (Release|x64).
#   MFC Dynamic  -> _AFXDLL, and the MFC headers pragma-link the right DLLs.
#   Unicode      -> UNICODE / _UNICODE (CharacterSet=Unicode).
#   Runtime      -> /MD (MultiThreadedDLL), which is also Python's release CRT,
#                   so RtModel.lib, MFC and the Python extension all agree.
# ---------------------------------------------------------------------------
define_macros = [
    ("WIN32", None),
    ("_WINDOWS", None),
    ("NDEBUG", None),
    ("USE_IPP", None),
    ("USE_RTOPT", None),
    ("NOMINMAX", None),
    ("UNICODE", None),
    ("_UNICODE", None),
    ("_AFXDLL", None),          # MFC shared (Dynamic) linkage
    ("VERSION_INFO", '"0.1.0"'),
]

extra_compile_args = [
    "/std:c++17",   # RtModel.vcxproj LanguageStandard (x64)
    "/EHsc",
    "/bigobj",      # MFC + ITK template instantiations blow past 64k sections
    "/MD",          # dynamic release CRT (must match RtModel.lib + MFC + Python)
    "/wd4996",      # silence CRT/STL deprecation noise from old RtModel code
]

ext_modules = [
    Pybind11Extension(
        "rtmodel_core",
        sources=["rtmodel_bindings.cpp"],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        define_macros=define_macros,
        extra_compile_args=extra_compile_args,
        cxx_std=17,
        language="c++",
    ),
]

# Package metadata (name, version, dependencies, ...) lives in pyproject.toml.
# setup.py only contributes the compiled extension so the two don't collide.
setup(
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
