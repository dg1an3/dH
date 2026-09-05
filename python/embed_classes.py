"""Generate vector embeddings for important classes/algorithms via Ollama.

Mirrors the "Key Classes & Algorithms" section of COMPONENT_INVENTORY.md. Calls the
local Ollama `nomic-embed-text` model for each class/algorithm and writes
`class_embeddings.json` alongside this script.

Run:  python python/embed_classes.py
Requires: a running Ollama daemon with `nomic-embed-text` pulled.
"""

import json
import urllib.request
import os

OLLAMA_URL = "http://localhost:11434/api/embeddings"
MODEL = "nomic-embed-text"
OUT_PATH = os.path.join(os.path.dirname(__file__), "class_embeddings.json")

# (library, name, kind, text-to-embed)
CLASSES = [
    # ---- RtModel: optimization ----
    ("RtModel", "DynamicCovarianceOptimizer", "algorithm",
     "Conjugate gradient optimizer with Polak-Ribiere direction updates and Brent line search. "
     "Maintains orthogonal basis of past search directions via Gram-Schmidt. Derives adaptive "
     "variance from a power-law schedule. Optional explicit free energy F = KL divergence minus "
     "differential entropy H = 0.5 (n log 2 pi e + log det Sigma) from the search-direction "
     "covariance. minimize() in ConjGradOptimizer.cpp."),
    ("RtModel", "DynamicCovarianceCostFunction", "class",
     "VNL vnl_cost_function base class bridging the conjugate gradient optimizer to the objective "
     "function. Prescription derives from it."),
    # ---- RtModel: objective / terms ----
    ("RtModel", "VOITerm", "class",
     "Abstract base class for per-structure objective function terms. Pure virtual Eval computing "
     "value and gradient, and Clone for pyramid copies. Owns a Structure region of interest and a "
     "histogram with gradient."),
    ("RtModel", "KLDivTerm", "algorithm",
     "Kullback-Leibler divergence objective term minimizing KL between calculated and target "
     "dose-volume histograms, sum over bins of calcGPDF log calcGPDF over targetGPDF. Convolves "
     "dose-volume points with dual Gaussian kernels at varMin and varMax. Gradient via product "
     "rule chain. Log-ratio and cross-entropy modes. Eval in KLDivTerm.cpp."),
    ("RtModel", "Prescription", "algorithm",
     "Objective function aggregator computing weighted sum of VOITerm evaluations. CalcSumSigmoid "
     "accumulates weighted beamlet volumes with adaptive variance fractions. Sigmoid "
     "parameterization mapping unbounded optimizer variables to bounded beamlet weights via "
     "Transform dTransform InvTransform with SIGMOID_SCALE 0.2 and transform-slope variance "
     "correction variance times dSigmoid squared."),
    # ---- RtModel: dose calculation ----
    ("RtModel", "CBeamDoseCalc", "algorithm",
     "TERMA total energy released in medium ray tracer. Resamples CT density, traces pencil-beam "
     "rays from source through beamlet aperture, trilinear density interpolation, accumulates "
     "energy to voxel neighborhoods UpdateTermaNeighborhood, then invokes kernel sphere "
     "convolution. CalcTerma in BeamDoseCalc.cpp."),
    ("RtModel", "CEnergyDepKernel", "algorithm",
     "Energy deposition kernel with cumulative energy lookup table indexed by phi angle and radial "
     "distance. CalcSphereConvolve iterates slice voxels and calls CalcSphereTrace for radial ray "
     "integration via LUT interpolation, normalized by mass density and azimuthal sum 1 over "
     "NUM_THETA. Loaded from kernel.dat files."),
    ("RtModel", "SphereConvolve", "algorithm",
     "Spherical kernel convolution manager building direction and offset lookup tables from kernel "
     "geometry ComputeSphereLUT, driving per-voxel CalcSphereTrace and InterpCumEnergy. Computes "
     "dose equals TERMA convolved with energy kernel."),
    # ---- RtModel: histograms ----
    ("RtModel", "CHistogram", "class",
     "Dose-volume histogram with Gaussian smoothing. Bins dose into fixed-width array, applies dual "
     "Gaussian kernels varMin varMax to form smoothed GBins, keeps fractional volume weights for "
     "linear interpolation between bin levels. ConvGauss GetBinForValue SetBinning."),
    ("RtModel", "CHistogramWithGradient", "algorithm",
     "Extends CHistogram with per-beamlet derivative volumes grouped by beam. Computes bin "
     "derivatives by product rule with binning fractions and Gaussian-smooths them via Conv_dGauss. "
     "The dose to histogram partial derivative store driving the optimization gradient chain. "
     "Get_dBins Get_dGBins."),
    # ---- RtModel: multi-scale ----
    ("RtModel", "PlanPyramid", "algorithm",
     "Coarse-to-fine multi-scale hierarchy of four plans, each doubling dose resolution and beamlet "
     "spacing. InvFiltIntensityMap transfers intensity maps between levels using a binomial filter "
     "0.25 0.50 0.25. Generates coarse pencil sub-beamlets."),
    ("RtModel", "PlanOptimizer", "algorithm",
     "Multi-level optimization orchestrator managing per-level Prescription and "
     "DynamicCovarianceOptimizer pairs from coarse to fine, inverse-filtering the state vector into "
     "the next finer level. GetStateVectorFromPlan SetStateVectorToPlan move beamlet weights. "
     "Optimize in PlanOptimizer.cpp."),
    # ---- RtModel: data model ----
    ("RtModel", "Plan", "class",
     "Treatment plan container holding beams, dose volume, resampled mass density, and energy "
     "kernel defaulting to 6 MeV. GetTotalBeamletCount sums across beams."),
    ("RtModel", "Beam", "class",
     "Single radiation beam with gantry angle, isocenter, one-dimensional intensity map, and "
     "computed dose volume. OnIntensityMapChanged regenerates beamlets from weights. GetBeamlet."),
    ("RtModel", "Series", "class",
     "Imaging dataset grouping a CT density volume and an array of Structure regions of interest."),
    ("RtModel", "Structure", "class",
     "Region of interest as polygon spatial-object contours. ContoursToRegion rasterizes to "
     "multi-scale binary volumes. Type target eTARGET or organ at risk eOAR."),
    ("RtModel", "PlanXmlReader/PlanXmlWriter", "class",
     "ITK XML serialization reading and writing the whole treatment plan: beams, intensity maps, "
     "prescriptions, dose-volume histograms, dose calculation parameters."),
    # ---- Brimstone ----
    ("Brimstone", "CBrimstoneApp", "class", "MFC application entry point and initialization."),
    ("Brimstone", "CBrimstoneDoc", "class",
     "MFC document owning the Series and Plan objects and handling document serialization."),
    ("Brimstone", "CBrimstoneView/CPlanarView", "class",
     "MFC views rendering 2D CT image slices, contours, and dose overlays."),
    ("Brimstone", "CSeriesDicomImporter", "class",
     "Imports DICOM CT image series and RT structure sets into the Series data model."),
    ("Brimstone", "OptThread", "class",
     "Background worker thread running the optimization by driving the RtModel PlanOptimizer."),
    ("Brimstone", "OptimizerDashboard", "class",
     "Real-time visualization of optimization progress and metrics."),
    # ---- Graph ----
    ("Graph", "CGraph", "class",
     "Interactive MFC 2D plot window with axes, legend, and draggable data points for DVH and "
     "iteration curves."),
    ("Graph", "CDataSeries", "class", "Abstract curve data container with color and line style."),
    ("Graph", "CHistogramDataSeries", "class", "Binned dose-volume histogram data series."),
    ("Graph", "CTargetDVHSeries", "class", "Target or prescription DVH overlay series."),
    # ---- Foundation ----
    ("MTL", "CVectorD/CMatrixD/CMatrixNxM", "class",
     "Templated vector and matrix classes with least-squares solvers and matrix operations."),
    ("VecMat", "VectorD/MatrixD/MatrixNxM", "class",
     "Refactored templated vector and matrix base classes with inline implementations."),
    ("FTL", "BufferND/MeshND/Field/BufferIndex", "class",
     "N-dimensional grid and buffer container abstractions for spatially organized data."),
    ("GEOM_BASE", "Polygon", "class",
     "Serializable observable 2D polygon geometry primitive with MFC CArchive support."),
    ("GEOM_BASE", "CVolume", "class",
     "Volumep three-dimensional voxel grid template for density and dose volumes."),
    ("MODEL_BASE", "CModelObject", "class",
     "Observable serializable hierarchical base model object underpinning all domain objects."),
    ("MODEL_BASE", "Observer pattern", "class",
     "CObservableObject and CObservableEvent implementing the observer notification pattern."),
    # ---- Component / mid-level ----
    ("GEOM_MODEL", "Cluster", "algorithm",
     "K-means clustering over geometric point data."),
    ("GEOM_MODEL", "AffineTransform", "class",
     "Affine geometric transform for points and meshes."),
    ("GEOM_MODEL", "TPS", "algorithm",
     "Thin-plate spline non-rigid geometric transformation."),
    ("GEOM_MODEL", "Mesh", "class", "Three-dimensional mesh geometry object."),
    ("GEOM_MODEL", "GradientCalculator", "algorithm",
     "Computes spatial gradients over geometric scalar fields."),
    ("GEOM_VIEW", "SceneView/Camera/Renderable", "class",
     "Direct3D 8 scene rendering framework with camera, lights, renderables, and object explorer "
     "tree, plus rotate and zoom interaction trackers."),
    ("OPTIMIZER_BASE", "line-search optimizers", "algorithm",
     "One-dimensional line-search optimizers Brent Powell and conjugate gradient as generic "
     "templates."),
    ("OptimizeN", "ND optimizers", "algorithm",
     "N-dimensional optimizers Brent Powell conjugate gradient DFP gradient descent and cubic "
     "interpolation."),
    ("OGL_BASE", "COpenGLRenderer/COpenGLView", "class",
     "OpenGL rendering base classes with camera, light, texture, and interaction trackers."),
    ("GUI_BASE", "ObjectExplorer/Graph/Dib/DrawTool", "class",
     "Reusable MFC GUI widgets: object tree explorer, plot, device-independent bitmap, draw tool, "
     "tab control bar."),
    ("GenImaging", "InPlaneResampleImageFilter", "algorithm",
     "ITK ImageToImageFilter resampling 3D volumes in-plane with linear or nearest-neighbor "
     "interpolation."),
    ("GenImaging", "IntensityMapAccumulateImageFilter", "algorithm",
     "ITK filter accumulating dose by weighted pixel-wise multiply and add of beamlet intensity "
     "maps, IPP accelerated."),
    ("GenImaging", "ContoursToRegionFilter", "algorithm",
     "ITK filter rasterizing polygon contours into a binary region mask volume."),
    ("XMLLogging", "CXMLLogFile", "class",
     "Thread-safe singleton writing structured nested XML diagnostic logs to a FILE pointer."),
    ("XMLLogging", "CXMLElement", "class",
     "Scoped RAII XML tag element managing nesting of attributes and text."),
    ("FieldCOM", "Mesh", "class",
     "COM ATL mesh object exposing IMesh with FileStorage IPersistStorage save and load."),
    ("FieldCOM", "FileStorage", "class",
     "COM persistence interface saving and loading geometry to a pathname or stream."),
    # ---- Legacy RT / sim ----
    ("RT_MODEL", "TCP_NTCP_Optimizer", "algorithm",
     "Tumor control probability and normal tissue complication probability based dose optimizer in "
     "the legacy RT model."),
    ("RT_MODEL", "DicomImporter", "class",
     "Legacy DICOM series and structure set importer."),
    ("RT_VIEW", "BeamRenderable/MachineRenderable", "class",
     "Direct3D renderables drawing treatment beams and the treatment machine, with beam parameter "
     "control panels."),
    ("VSIM_MODEL", "TreatmentMachine", "class",
     "Virtual-simulation treatment machine model alongside Plan Series Beam."),
    # ---- Utilities / research ----
    ("DivFluence", "fluence integration", "algorithm",
     "Divergent fluence calculation integrating exponential attenuation, inverse-square falloff, "
     "heterogeneous density, and field divergence over a CVolume."),
    ("PenBeam_indens", "pencil beam convolution", "algorithm",
     "Fortran pencil-beam in-density convolution: ray-trace through heterogeneous phantom with "
     "spherical convolve kernels and energy lookup interpolation."),
    ("python", "ct_to_md_values", "function",
     "Converts CT Hounsfield unit values to mass density via piecewise calibration, ITK based."),
    ("python", "rotate_density_for_beam", "function",
     "Rotates a density volume to a beam gantry angle for ray-traced dose calculation."),
    ("python", "terma_from_density", "function",
     "Computes TERMA total energy released in medium by ray-tracing through a density volume."),
    ("notebook_zoo", "entropy maximization", "algorithm",
     "Gradient-descent entropy maximization over image and histogram distributions, with MNIST "
     "autoencoder and variational autoencoder experiments."),
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
    for library, name, kind, text in CLASSES:
        vec = embed(f"{library} {name}: {text}")
        print(f"{library:14s} {name:34s} {kind:9s} dim={len(vec)}")
        out.append({
            "library": library,
            "name": name,
            "kind": kind,
            "text": text,
            "embedding": vec,
        })
    with open(OUT_PATH, "w", encoding="utf-8") as f:
        json.dump(
            {"model": MODEL, "dim": len(out[0]["embedding"]), "classes": out},
            f,
        )
    print(f"\nWrote {len(out)} class embeddings (dim={len(out[0]['embedding'])}) to {OUT_PATH}")


if __name__ == "__main__":
    main()
