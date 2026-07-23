"""Generate vector embeddings for each repository component via Ollama.

Reads the component descriptions defined below (mirroring COMPONENT_INVENTORY.md),
calls the local Ollama `nomic-embed-text` model for each, and writes
`component_embeddings.json` alongside this script.

Run:  python python/embed_components.py
Requires: a running Ollama daemon with `nomic-embed-text` pulled.
"""

import json
import urllib.request
import os

OLLAMA_URL = "http://localhost:11434/api/embeddings"
MODEL = "nomic-embed-text"
OUT_PATH = os.path.join(os.path.dirname(__file__), "component_embeddings.json")

# (name, category, text-to-embed). Text = purpose + deps + I/O, kept compact.
COMPONENTS = [
    ("RtModel", "production",
     "Core radiotherapy algorithm library. Data model (Plan, Beam, Series, Structure, "
     "Prescription), pencil-beam TERMA dose calculation, energy-kernel spherical "
     "convolution, dose-volume histograms with gradients, multi-scale conjugate-gradient "
     "inverse optimizer minimizing KL divergence between calculated and target DVHs. "
     "Depends on ITK, VNL/VXL, Intel IPP, MFC, GEOM_MODEL, MTL, GenImaging, OptimizeN, "
     "XMLLogging. Reads/writes XML plans and energy kernel .dat files; serializes ITK volumes."),
    ("Brimstone", "production",
     "MFC SDI radiotherapy treatment-planning GUI application. Imports DICOM CT and "
     "structure sets, sets up prescriptions and beams, runs optimizer on background thread, "
     "renders CT slices, contours, dose overlays, DVH graphs, optimization progress. "
     "Depends on RtModel, Graph, XMLLogging, GEOM_MODEL, MTL, MFC, ATL. Reads DICOM and "
     "Brimstone documents and kernel.dat; writes documents, XML plans, XML logs."),
    ("Graph", "production",
     "MFC plotting library rendering dose-volume histograms and optimization iteration "
     "curves. CGraph plot window, CDataSeries, CHistogramDataSeries, CTargetDVHSeries, CDib. "
     "Depends on MFC, ITK, GEOM_MODEL, RtModel. In-memory rendering to Windows CDC, no persistence."),
    ("MTL", "foundation",
     "Math Template Library. Templated vector and matrix classes CVectorD CMatrixD "
     "CMatrixNxM, least-squares solvers, matrix operations. Header-only, depends on STL. "
     "Pure in-memory numeric foundation used by GEOM_MODEL, RtModel, OptimizeN."),
    ("VecMat", "foundation",
     "Vector and matrix library, refactored iteration of MTL. VectorBase VectorD VectorN "
     "MatrixBase MatrixD MatrixNxM with inline implementations. Header-only, STL only, no I/O. "
     "Used by legacy tools DivFluence and PenBeamEdit."),
    ("FTL", "foundation",
     "Foundation Template Library. Multi-dimensional container abstractions BufferND MeshND "
     "Field BufferIndex for spatial grid and buffer data. Depends on MFC CString and MTL. "
     "In-memory, no serialization. Used by GEOM_MODEL and GEOM_VIEW."),
    ("GEOM_BASE", "foundation",
     "Geometry base primitives. Polygon, LookupFunction, ScalarFunction, Volumep CVolume "
     "3D voxel grid, MFC serialization. Depends on MTL, MODEL_BASE, MFC. CArchive "
     "serialization. Used by GEOM_MODEL, GEOM_VIEW, DivFluence, PenBeamEdit."),
    ("MODEL_BASE", "foundation",
     "Observable serializable model framework. CModelObject observable serializable "
     "hierarchical, Observer pattern, collections, metadata value properties. Depends on MFC "
     "DECLARE_SERIAL. Provides serialization infrastructure to domain objects."),
    ("GEOM_MODEL", "component",
     "Geometric modeling library. Mesh, Cluster k-means, AffineTransform, thin-plate spline "
     "TPS, GradientCalculator, Dib, Volumep and UtilMacros headers. Depends on MTL, "
     "GEOM_BASE, XMLLogging, Intel IPP. Dib raster I/O, XML logging, CArchive serialization. "
     "Used by RtModel, Graph, GEOM_VIEW, VSIM_MODEL, Brimstone."),
    ("GEOM_VIEW", "component",
     "Geometry visualization framework. MFC plus Direct3D 8 rendering: Camera, Light, "
     "Renderable, SceneView, ObjectExplorer tree UI, rotate zoom trackers. Depends on "
     "Direct3D 8, MTL, GEOM_MODEL, XMLLogging, MFC. Rendering only, no file I/O."),
    ("OPTIMIZER_BASE", "component",
     "Base optimization framework. One-dimensional line-search optimizers Brent Powell "
     "conjugate gradient as generic templates. Depends on MFC. No I/O. Used by OptimizeN, RT_MODEL."),
    ("OptimizeN", "component",
     "N-dimensional optimization library. Multi-dimensional optimizers Brent Powell ConjGrad "
     "DFP gradient descent cubic interpolation, extending OPTIMIZER_BASE. Depends on "
     "XMLLogging, MTL. Emits XML logs. Used by RtModel, RT_MODEL, Brimstone."),
    ("OGL_BASE", "component",
     "OpenGL base classes. COpenGLRenderer, COpenGLView, camera light texture classes, "
     "rotate zoom trackers, matrix vector GL helpers. Depends on MFC and Windows OpenGL. "
     "Rendering only. Used by VSIM_OGL."),
    ("GUI_BASE", "component",
     "Base MFC GUI widgets. DrawTool, Graph plot, Dib, ObjectExplorer ObjectTreeItem tree, "
     "TabControlBar. Depends on MFC common controls. DIB bitmap load and render. Used by "
     "GUI applications and PenBeamEdit."),
    ("GenImaging", "component",
     "ITK imaging filters for radiotherapy. InPlaneResampleImageFilter, "
     "IntensityMapAccumulateImageFilter dose accumulation, ContoursToRegionFilter, "
     "ScalarImageToWeightedHistogramGenerator, MultiMaskNegatedImageFilter. Depends on ITK "
     "and Intel IPP. Operates on in-memory ITK volumes. Used by RtModel, RT_MODEL, Brimstone."),
    ("XMLLogging", "component",
     "XML diagnostic logging framework. CXMLLogFile thread-safe singleton writing to FILE, "
     "CXMLElement scoped tags, CXMLLoggableObject, LOG macros and timing. Depends on MFC, STL. "
     "Writes nested XML log files. Used by RtModel, Brimstone, OptimizeN, GEOM_MODEL, RT_MODEL."),
    ("FieldCOM", "component",
     "COM ATL geometry interop library. Exposes Mesh IMesh, MeshSet, BufferField, Polygon3D, "
     "PolygonSet3D, AffineTransform, TPSTransform, FileStorage as COM objects. Depends on ATL "
     "and Windows COM. FileStorage IPersistStorage save load to pathname. Used by VSIM_OGL."),
    ("RT_MODEL", "legacy-rt",
     "Older radiotherapy model library, precursor to RtModel. Plan Series Beam Structure "
     "Histogram Prescription, optimizers ConjGradOptimizer TCP_NTCP_Optimizer VOITerm "
     "KLDivTerm, DICOM import, XML plan serialization. Console tools GenBeamlets GenDens "
     "DicomImEx TestHisto. Depends on ITK, MFC, OptimizeN, GEOM_MODEL, GenImaging, MTL, "
     "XMLLogging, IPP. Reads DICOM, reads writes XML plans and ITK volumes."),
    ("RT_VIEW", "legacy-rt",
     "Radiotherapy visualization library. Direct3D MFC renderables and control panels "
     "BeamRenderable MachineRenderable BeamParamCollimCtrl BeamParamPosCtrl LightfieldTexture. "
     "Depends on MFC, Direct3D 8, RT_MODEL. Rendering only."),
    ("VSIM_MODEL", "legacy-rt",
     "Virtual-simulation data model. Plan Series Beam TreatmentMachine, older simulation "
     "system mirroring RT_MODEL. Depends on MFC, GEOM_MODEL. MFC CDocument DECLARE_SERIAL "
     "serialization. Used by VSIM_OGL."),
    ("VSIM_OGL", "legacy-rt",
     "Virtual-simulation OpenGL viewer application. MFC SDI integrating OGL_BASE and "
     "VSIM_MODEL, exports to FieldCOM. Depends on MFC, OGL_BASE, VSIM_MODEL, FieldCOM. "
     "Reads VSIM_MODEL documents, exports geometry to FieldCOM."),
    ("DivFluence", "utility",
     "Console divergent-fluence calculator for dose calculation. Exponential attenuation, "
     "inverse-square falloff, heterogeneous density, field divergence. Depends on Intel IPP, "
     "VecMat, GEOM_BASE CVolume, MODEL_BASE. Reads density volumes, beam params, energy "
     "kernel .dat tables; writes binary fluence and energy volumes."),
    ("PenBeamEdit", "utility",
     "MFC SDI pencil-beam editor GUI for beam parameters. Depends on MFC, GEOM_BASE Volumep, "
     "MODEL_BASE, GUI_BASE Dib, VecMat. Reads writes serialized beam-parameter documents of "
     "CVolume images."),
    ("PenBeam_indens", "utility",
     "Legacy Fortran pencil-beam in-density convolution reference. Ray-trace through "
     "heterogeneous phantom with spherical convolve kernels. Reads phantom density grids, ray "
     "geometry penbeam_input.txt, energy tables; writes dose to cono directory dat files. "
     "Mathematical spec for DivFluence."),
    ("python", "utility",
     "Python ITK CT-density preprocessing scripts. read_process_series.py HU to mass density "
     "conversion ct_to_md_values, beam-angle density rotation rotate_density_for_beam, TERMA "
     "ray-tracing terma_from_density. Depends on ITK Python bindings, NumPy. Reads CT volumes "
     "via itk.imread nrrd; produces density and TERMA volumes."),
    ("notebook_zoo", "utility",
     "Research Jupyter notebooks on entropy maximization and ML autoencoders VAEs. "
     "entropy_calc, entropy_max, entropy_max_histo, mnist_autoencoder variants, mnist_vae_shift, "
     "pinwheel_shifter. Depends on NumPy, TensorFlow 2, Matplotlib, MNIST. In-memory arrays, "
     "outputs plots and model snapshots."),
]


def embed(text):
    payload = json.dumps({"model": MODEL, "prompt": text}).encode("utf-8")
    req = urllib.request.Request(
        OLLAMA_URL, data=payload, headers={"Content-Type": "application/json"}
    )
    with urllib.request.urlopen(req, timeout=120) as resp:
        return json.loads(resp.read().decode("utf-8"))["embedding"]


def main():
    out = []
    for name, category, text in COMPONENTS:
        vec = embed(text)
        print(f"{name:16s} dim={len(vec)}")
        out.append({
            "name": name,
            "category": category,
            "text": text,
            "embedding": vec,
        })
    with open(OUT_PATH, "w", encoding="utf-8") as f:
        json.dump(
            {"model": MODEL, "dim": len(out[0]["embedding"]), "components": out},
            f,
        )
    print(f"\nWrote {len(out)} embeddings (dim={len(out[0]['embedding'])}) to {OUT_PATH}")


if __name__ == "__main__":
    main()
