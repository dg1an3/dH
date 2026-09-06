"""Generate vector embeddings for important classes/algorithms via Ollama.

Mirrors the "Key Classes & Algorithms" section of COMPONENT_INVENTORY.md, but goes
broader: this list is the exhaustive, machine-readable class/algorithm inventory --
COMPONENT_INVENTORY.md profiles only the most important entries in prose. Calls the
local Ollama `nomic-embed-text` model for each entry and writes
`class_embeddings.json` alongside this script.

Every `library` value below must match a component `name` in embed_components.py --
build_inventory_embeddings.py looks up each class's tier through that name, and an
unrecognized library silently falls back to the "component" tier.

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
    # ================================================================
    # RtModel -- optimization
    # ================================================================
    ("RtModel", "DynamicCovarianceOptimizer", "algorithm",
     "Conjugate gradient optimizer with Polak-Ribiere direction updates and Brent line search. "
     "Maintains orthogonal basis of past search directions via Gram-Schmidt. Derives adaptive "
     "variance from a power-law schedule. Optional explicit free energy F = KL divergence minus "
     "differential entropy H = 0.5 (n log 2 pi e + log det Sigma) from the search-direction "
     "covariance. minimize() in ConjGradOptimizer.cpp."),
    ("RtModel", "DynamicCovarianceCostFunction", "class",
     "VNL vnl_cost_function base class bridging the conjugate gradient optimizer to the objective "
     "function. Prescription derives from it."),
    ("RtModel", "SigmaEstimator", "algorithm",
     "Adaptive estimation of sigma values for multi-scale pyramid optimization, derived from "
     "structure characteristics, dose complexity, and observed optimization behavior, rather than "
     "the fixed DEFAULT_LEVELSIGMA table."),
    ("RtModel", "SigmoidParams", "function",
     "Shared getters for the sigmoid parameterization constants, scale and height, readable from "
     "the environment. Single definition used by both Prescription and CHistogramWithGradient so "
     "the two copies of the constant cannot drift apart."),

    # ================================================================
    # RtModel -- objective function and terms
    # ================================================================
    ("RtModel", "VOITerm", "class",
     "Abstract base class for per-structure objective function terms. Pure virtual Eval computing "
     "value and gradient, and Clone for pyramid copies. Owns a Structure region of interest and a "
     "histogram with gradient. Derives from itk DataObject."),
    ("RtModel", "KLDivTerm", "algorithm",
     "Kullback-Leibler divergence objective term minimizing KL between calculated and target "
     "dose-volume histograms, sum over bins of calcGPDF log calcGPDF over targetGPDF. Convolves "
     "dose-volume points with dual Gaussian kernels at varMin and varMax. Gradient via product "
     "rule chain. Log-ratio and cross-entropy modes. Eval in KLDivTerm.cpp."),
    ("RtModel", "Prescription", "algorithm",
     "Objective function aggregator computing weighted sum of VOITerm evaluations. CalcSumSigmoid "
     "accumulates weighted beamlet volumes with adaptive variance fractions. Sigmoid "
     "parameterization mapping unbounded optimizer variables to bounded beamlet weights via "
     "Transform dTransform InvTransform, with transform-slope variance correction variance times "
     "dSigmoid squared. Optional entropy-regularized free energy F = KL minus w times H."),

    # ================================================================
    # RtModel -- dose calculation
    # ================================================================
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

    # ================================================================
    # RtModel -- histograms
    # ================================================================
    ("RtModel", "CHistogram", "class",
     "Dose-volume histogram with Gaussian smoothing, over a volume optionally restricted to a "
     "binary region volume. Bins dose into fixed-width array, applies dual Gaussian kernels varMin "
     "varMax to form smoothed GBins, keeps fractional volume weights for linear interpolation "
     "between bin levels. ConvGauss GetBinForValue SetBinning. Derives from itk DataObject."),
    ("RtModel", "CHistogramWithGradient", "algorithm",
     "Extends CHistogram with per-beamlet derivative volumes grouped by beam. Computes bin "
     "derivatives by product rule with binning fractions and Gaussian-smooths them via Conv_dGauss. "
     "The dose to histogram partial derivative store driving the optimization gradient chain. "
     "Get_dBins Get_dGBins."),

    # ================================================================
    # RtModel -- multi-scale
    # ================================================================
    ("RtModel", "PlanPyramid", "algorithm",
     "Coarse-to-fine multi-scale hierarchy of four plans, each doubling dose resolution and beamlet "
     "spacing. InvFiltIntensityMap transfers intensity maps between levels using a binomial filter "
     "0.25 0.50 0.25. Generates coarse pencil sub-beamlets. MAX_SCALES is 4."),
    ("RtModel", "PlanOptimizer", "algorithm",
     "Multi-level optimization orchestrator managing per-level Prescription and "
     "DynamicCovarianceOptimizer pairs from coarse to fine, inverse-filtering the state vector into "
     "the next finer level. GetStateVectorFromPlan SetStateVectorToPlan move beamlet weights. "
     "Optimize in PlanOptimizer.cpp."),

    # ================================================================
    # RtModel -- data model
    # ================================================================
    ("RtModel", "Plan", "class",
     "Treatment plan container holding beams, dose volume, resampled mass density, dose-volume "
     "histograms, and energy kernel defaulting to 6 MeV. GetTotalBeamletCount sums across beams. "
     "Derives from itk DataObject in namespace dH."),
    ("RtModel", "Beam", "class",
     "Single radiation beam with gantry angle, isocenter, one-dimensional intensity map, and "
     "computed dose volume. OnIntensityMapChanged regenerates beamlets from weights. GetBeamlet."),
    ("RtModel", "Series", "class",
     "Imaging dataset grouping a CT density volume and an array of Structure regions of interest "
     "into a single object."),
    ("RtModel", "Structure", "class",
     "Region of interest defined in the CT coordinate system, read in as a stack of polygon "
     "spatial-object contours and converted to a binary volume. ContoursToRegion rasterizes to "
     "multi-scale binary volumes. Type target eTARGET or organ at risk eOAR."),
    ("RtModel", "PlanXmlReader", "class",
     "ITK XML reader deserializing a whole treatment plan from XML: beams, intensity maps, "
     "prescriptions, dose-volume histograms, dose calculation parameters."),
    ("RtModel", "PlanXmlWriter", "class",
     "ITK XML writer serializing a whole treatment plan to XML: beams, intensity maps, "
     "prescriptions, dose-volume histograms, dose calculation parameters."),
    ("RtModel", "TG263Nomenclature", "algorithm",
     "TG-263 structure nomenclature translator mapping free-text region-of-interest names onto the "
     "AAPM Task Group 263 standardized structure names, so imported DICOM structure sets get "
     "consistent naming."),
    ("RtModel", "TG263Match", "class",
     "Result record of a TG-263 structure name match: the matched standard structure and the "
     "confidence or score of the match."),
    ("RtModel", "TG263Structure", "class",
     "Definition record of one TG-263 standard structure: its standardized name and the attributes "
     "used for matching against free-text region-of-interest names."),

    # ================================================================
    # RtModel -- math and utility headers
    # ================================================================
    ("RtModel", "CVectorN", "class",
     "Dynamically sizable mathematical vector template with element type given. RtModel's own copy, "
     "carrying the optimizer state vector, gradients and histogram bin arrays. Converted from MFC "
     "CArray storage to std vector."),
    ("RtModel", "CMatrixNxM", "class",
     "Non-square matrix template with element type given. RtModel's own copy, used for the "
     "search-direction covariance in the free-energy entropy term."),
    ("RtModel", "ItkUtils", "function",
     "Header of ITK helper free functions: MakeVector MakeSize MakeContinuousIndex, basis and "
     "direction matrix calculation CalcBasis, conformance tests, resampling, accumulation "
     "Accumulate3D, and decimation over itk Image volumes."),
    ("RtModel", "UtilMacros", "function",
     "Macro header defining the DECLARE_ATTRIBUTE property pattern, namespace helpers, the "
     "RTM_TRACE printf-style debug trace routed to OutputDebugString, and the BeginLogSection "
     "EndLogSection XML log-section macros."),

    # ================================================================
    # Brimstone -- application, document, views
    # ================================================================
    ("Brimstone", "CBrimstoneApp", "class",
     "MFC CWinApp application entry point and initialization for the Brimstone treatment-planning "
     "GUI."),
    ("Brimstone", "CBrimstoneDoc", "class",
     "MFC CDocument owning the Series and Plan objects, managing loading and saving of the model "
     "objects, DICOM import, and dose calculation."),
    ("Brimstone", "CBrimstoneView", "class",
     "MFC CView managing the view of the plan data: renders CT image slices, structure contours and "
     "dose overlays, and hosts the planar views."),
    ("Brimstone", "CPlanarView", "class",
     "MFC CWnd representing a single multi-planar reconstruction pane, rendering one CT slice with "
     "contours and dose wash, with mouse-wheel slice scrolling."),
    ("Brimstone", "CMainFrame", "class",
     "MFC CFrameWnd main window frame for Brimstone, owning the toolbars, control bars and the "
     "prescription toolbar."),

    # ================================================================
    # Brimstone -- import, dialogs, optimization
    # ================================================================
    ("Brimstone", "CSeriesDicomImporter", "class",
     "Imports DICOM CT image series and RT structure sets into the Series data model, iterating "
     "files through a file dialog and building the density volume and structures."),
    ("Brimstone", "CDicomImageItem", "class",
     "Private helper class of CSeriesDicomImporter processing an individual DICOM image item during "
     "series import, sorting slices and reading pixel data."),
    ("Brimstone", "COptThread", "class",
     "MFC CWinThread background worker thread running the optimization by driving the RtModel "
     "PlanOptimizer, posting WM_OPTIMIZER_UPDATE and WM_OPTIMIZER_DONE messages to the UI."),
    ("Brimstone", "COptIterData", "class",
     "Nested class of COptThread storing the data for the current optimization iteration, passed "
     "from the optimizer callback to the UI thread for display."),
    ("Brimstone", "COptimizerDashboard", "class",
     "MFC dialog giving real-time visualization of optimization progress and metrics during a run."),
    ("Brimstone", "CPrescDlg", "class",
     "MFC dialog for setting up the prescription: per-structure objective terms, target and organ "
     "at risk dose intervals, and term weights."),
    ("Brimstone", "CPlanSetupDlg", "class",
     "MFC dialog for setting up the treatment plan: beam count, gantry angles, isocenter and beam "
     "energy."),
    ("Brimstone", "CPrescriptionToolbar", "class",
     "MFC CDialogBar toolbar hosting the prescription editor, embedded in the main frame and backed "
     "by a WebView2 host control."),

    # ================================================================
    # Brimstone -- WebView2 controls
    # ================================================================
    ("Brimstone", "CWebView2Host", "class",
     "MFC CWnd host control wrapping the Microsoft Edge WebView2 browser, giving MFC windows an "
     "embedded HTML/JS surface. Provides ExecScript for C++ to page calls and receives page to C++ "
     "messages via window.chrome.webview.postMessage."),
    ("Brimstone", "DvhViewHtml", "class",
     "Self-contained HTML and JavaScript page rendered in the DVH WebView2 host. Shows a "
     "dose-volume-histogram chart plus an interactive structure and prescription editor: Target, "
     "OAR and None lists of Name Color Min Max Weight, with drag to reorder for priority and drag "
     "between lists for type. Driven by setStructures and setDvh, and posts pipe-delimited type "
     "weight interval color and order messages back to C++."),
    ("Brimstone", "OptGraphHtml", "class",
     "Self-contained HTML and JavaScript convergence chart rendered in the WebView2 host that "
     "replaces the legacy GDI iteration graph. Exposes addPoint(level, x, y) to append to a "
     "pyramid-level series and resetChart() to clear all series when optimization starts."),

    # ================================================================
    # Graph
    # ================================================================
    ("Graph", "CGraph", "class",
     "Interactive MFC CWnd 2D plot window with axes, legend, and draggable data points, used for "
     "dose-volume histograms and optimizer iteration curves."),
    ("Graph", "CDataSeries", "class",
     "Abstract curve data container with color and line style, derived from itk DataObject. Base "
     "for all series plotted by CGraph."),
    ("Graph", "CHistogramDataSeries", "class",
     "Binned dose-volume histogram data series, wrapping a CHistogram for display in CGraph."),
    ("Graph", "CTargetDVHSeries", "class",
     "Target or prescription DVH overlay series backed by a KLDivTerm, letting the user drag the "
     "target curve to edit the prescription."),
    ("Graph", "CDib", "class",
     "Device independent bitmap implemented as a form of MFC CBitmap, used for offscreen rendering "
     "of the plots."),

    # ================================================================
    # MTL -- Math Template Library (foundation)
    # ================================================================
    ("MTL", "CVectorN", "class",
     "Dynamically sizable mathematical vector template with element type given."),
    ("MTL", "CVectorD", "class",
     "Fixed-dimension mathematical vector template with dimension and element type given as "
     "template parameters."),
    ("MTL", "CMatrixNxM", "class",
     "Non-square matrix template with element type given, including least-squares solvers."),
    ("MTL", "CMatrixD", "class",
     "Fixed-dimension square matrix template with dimension and element type given, including "
     "inversion and matrix operations."),
    ("MTL", "CCastVectorD", "class",
     "CVectorD subclass that casts between vectors of different dimensions and element types."),
    ("MTL", "CComVariantMatrix", "class",
     "CMatrixNxM subclass that is also a COM CComVariant, marshalling a matrix across COM "
     "boundaries as a SAFEARRAY variant."),

    # ================================================================
    # VecMat -- refactored vector/matrix library (foundation)
    # ================================================================
    ("VecMat", "CVectorBase", "class",
     "Base class template for mathematical vectors with element type given. The refactored "
     "hierarchy root that CVectorD and CVectorN both derive from."),
    ("VecMat", "CVectorD", "class",
     "Fixed-dimension mathematical vector deriving from CVectorBase, with dimension and element "
     "type as template parameters and inline implementations."),
    ("VecMat", "CVectorN", "class",
     "Dynamically sizable mathematical vector deriving from CVectorBase."),
    ("VecMat", "CMatrixBase", "class",
     "Base class template for fixed-size matrices with element type given, root of the refactored "
     "matrix hierarchy."),
    ("VecMat", "CMatrixD", "class",
     "Fixed-dimension square matrix deriving from CMatrixBase, with dimension and element type as "
     "template parameters."),
    ("VecMat", "CMatrixNxM", "class",
     "Non-square matrix deriving from CMatrixBase, sized at runtime."),

    # ================================================================
    # FTL -- Foundation Template Library (foundation)
    # ================================================================
    ("FTL", "CBufferND", "class",
     "N-dimensional sample buffer template deriving from CModelObject, holding a flat allocation "
     "plus dimension metadata for spatially organized data."),
    ("FTL", "IliffeVector", "class",
     "Nested template of CBufferND giving multi-dimensional subscript access to samples through "
     "nested Iliffe vectors, with a recursive definition specialized at dimension one."),
    ("FTL", "CBufferIndex", "class",
     "N-dimensional integer index into a CBufferND, deriving from CVectorD of int."),
    ("FTL", "CBufferBounds", "class",
     "N-dimensional bounds describing the valid index range of a buffer, used to iterate and clip "
     "CBufferIndex values."),
    ("FTL", "CMeshND", "class",
     "N-dimensional mesh template deriving from CModelObject, describing the sample grid geometry "
     "for a buffer."),
    ("FTL", "CField", "class",
     "Base template for a field: a function from an IN_DIM domain to an OUT_DIM range, deriving "
     "from CModelObject. Root of the field abstraction."),
    ("FTL", "CCompositeField", "class",
     "CField that composes other fields, evaluating them in sequence so transforms and samplers "
     "can be chained."),
    ("FTL", "CBufferField", "class",
     "CField backed by a CBufferND, interpolating sample values from the regular buffer grid."),
    ("FTL", "CMeshSetField", "class",
     "CField over a set of meshes, producing a scalar output from mesh membership or distance."),
    ("FTL", "CIrregularField", "class",
     "CField over irregularly placed samples rather than a regular buffer grid."),
    ("FTL", "CReferenceFrame", "class",
     "N-dimensional reference frame giving the origin, basis and spacing that relate a buffer's "
     "index space to world coordinates."),

    # ================================================================
    # GEOM_BASE -- geometry base primitives (foundation)
    # ================================================================
    ("GEOM_BASE", "CPolygon", "class",
     "Polygon on a plane with change notification and computational geometry helpers, deriving "
     "from CObservableObject. Serializable via MFC CArchive."),
    ("GEOM_BASE", "CVolume", "class",
     "Template class representing an arbitrary voxel-typed three-dimensional volume, deriving from "
     "CObservableObject."),
    ("GEOM_BASE", "CVector", "class",
     "Mathematical vector template with dimension and element type given."),
    ("GEOM_BASE", "CVectorN", "class",
     "Dynamically sizable mathematical vector template with element type given."),
    ("GEOM_BASE", "CMatrix", "class",
     "Square matrix template with dimension and element type given."),
    ("GEOM_BASE", "CLookupFunction", "class",
     "Lookup-table based function of a given type, deriving from MFC CObject. Interpolates tabulated "
     "values, used for calibration and transfer curves."),

    # ================================================================
    # MODEL_BASE -- observable/serializable model framework (foundation)
    # ================================================================
    ("MODEL_BASE", "CModelObject", "class",
     "Hierarchical base model object underpinning all domain objects: it is observable, has a name, "
     "possibly has children, and can be serialized. Derives from CObservableObject."),
    ("MODEL_BASE", "CObservableObject", "class",
     "MFC CObject subclass that fires change events which can be processed by an observer. The "
     "observer notification pattern at the root of the model framework."),
    ("MODEL_BASE", "CValue", "class",
     "Observable typed value template deriving from CObservableObject, firing change notifications "
     "when the held value is assigned."),
    ("MODEL_BASE", "CCollection", "class",
     "Observable collection of objects of a given type, firing notifications on insert and remove."),
    ("MODEL_BASE", "CAssociation", "class",
     "CValue of a pointer type representing an association to another model object, with lifetime "
     "management so the association is released when the owner is destroyed."),
    ("MODEL_BASE", "CAutoSyncValue", "class",
     "CValue that automatically stays synchronized with a member variable of a specified "
     "CAssociation's target, propagating changes without manual wiring."),
    ("MODEL_BASE", "CFunction1", "class",
     "Observable single-argument function template deriving from CValue, recomputing and notifying "
     "when its argument changes. Building block for reactive functional pipelines."),
    ("MODEL_BASE", "CFunction2", "class",
     "Observable two-argument function template deriving from CValue, for reactive functional "
     "pipelines."),
    ("MODEL_BASE", "CFunction3", "class",
     "Observable three-argument function template deriving from CValue, for reactive functional "
     "pipelines."),

    # ================================================================
    # GEOM_MODEL -- geometric modeling (component)
    # ================================================================
    ("GEOM_MODEL", "CCluster", "algorithm",
     "K-means style cluster analysis template over geometric point data, driven by a pluggable "
     "distance function and invoked through the static Analyze entry point."),
    ("GEOM_MODEL", "CDistanceFunction", "class",
     "Abstract distance function template supplied to CCluster to define how two elements are "
     "compared during cluster analysis."),
    ("GEOM_MODEL", "CEuclideanVectorDistance", "algorithm",
     "Distance function performing cluster analysis with Euclidean distance between vectors."),
    ("GEOM_MODEL", "CInverseProductDistance", "algorithm",
     "Distance function using inverse square distance of two vectors or scalars for clustering."),
    ("GEOM_MODEL", "CNearestNeighborDistance", "algorithm",
     "Single-linkage distance function for clustering, comparing by nearest neighbor between "
     "clusters."),
    ("GEOM_MODEL", "CFurthestNeighborDistance", "algorithm",
     "Complete-linkage distance function for clustering, comparing by furthest neighbor between "
     "clusters."),
    ("GEOM_MODEL", "CTPSTransform", "algorithm",
     "Thin-plate spline non-rigid geometric transformation holding a collection of position vector "
     "pairs and interpolating the deformation field from those landmarks. Derives from "
     "CModelObject."),
    ("GEOM_MODEL", "CMesh", "class",
     "Meshed surface geometry object deriving from CModelObject, holding vertices and triangle "
     "indices."),
    ("GEOM_MODEL", "CTriIndex", "class",
     "Triangle index triple template used by CMesh to reference three vertices."),
    ("GEOM_MODEL", "CGradientCalculator", "algorithm",
     "Computes spatial gradients over a voxel-typed volume, deriving from CModelObject. Used for "
     "edge and surface extraction."),
    ("GEOM_MODEL", "CVolumeBase", "class",
     "Non-templated base class for volumes, deriving from CModelObject, so voxel-typed volumes can "
     "be handled polymorphically."),
    ("GEOM_MODEL", "CVolume", "class",
     "Template class representing an arbitrary voxel-typed volume, deriving from CModelObject."),
    ("GEOM_MODEL", "CPolygon", "class",
     "Polygon on a plane with change notification and computational geometry, deriving from "
     "CModelObject. The model-framework version of the GEOM_BASE primitive."),
    ("GEOM_MODEL", "CModelObject", "class",
     "GEOM_MODEL's own copy of the hierarchical observable serializable base model object."),
    ("GEOM_MODEL", "CObservableEvent", "class",
     "Change event object fired by observable model objects and dispatched to registered "
     "observers."),
    ("GEOM_MODEL", "CPyramid", "algorithm",
     "Multi-resolution image pyramid over a volume, generating successively coarser levels."),
    ("GEOM_MODEL", "CSubsampler", "algorithm",
     "Subsamples a voxel-typed volume to a coarser grid, deriving from CModelObject. The "
     "downsampling step of the pyramid."),
    ("GEOM_MODEL", "CLookupFunction", "class",
     "Lookup-table based function of a given type, interpolating tabulated values."),
    ("GEOM_MODEL", "CAffineTransform", "class",
     "Affine geometric transform for points and meshes, exposed as an ATL COM coclass."),
    ("GEOM_MODEL", "CDib", "class",
     "Device independent bitmap as a form of MFC CBitmap, for image display and pseudocoloring."),
    ("GEOM_MODEL", "CCoordSys", "class",
     "Coordinate system model object relating a field's index space to world coordinates."),
    ("GEOM_MODEL", "CWinLevFilter", "algorithm",
     "Window and level intensity mapping filter over a voxel-typed volume, mapping Hounsfield or "
     "dose values into display range."),
    ("GEOM_MODEL", "CResampler", "algorithm",
     "Resamples a voxel-typed volume onto a different grid with interpolation."),
    ("GEOM_MODEL", "CPseudocolorFilter", "algorithm",
     "Maps scalar volume values through a color table into an RGB device independent bitmap, for "
     "dose wash and functional overlays."),

    # ================================================================
    # GEOM_VIEW -- geometry visualization (component)
    # ================================================================
    ("GEOM_VIEW", "CSceneView", "class",
     "MFC CWnd window owning a rendering context and driving the scene: cameras, lights, "
     "renderables and interaction trackers."),
    ("GEOM_VIEW", "CCamera", "class",
     "Camera representing the viewpoint for a CSceneView, holding the eye position, target and "
     "projection parameters."),
    ("GEOM_VIEW", "CLight", "class",
     "Light illuminating a CSceneView, with position, color and attenuation."),
    ("GEOM_VIEW", "CRenderable", "class",
     "Base class for a renderer belonging to a CSceneView window. Each renderable draws itself into "
     "the render context."),
    ("GEOM_VIEW", "CSurfaceRenderable", "class",
     "CRenderable that draws a meshed surface in a CSceneView, with material and normals."),
    ("GEOM_VIEW", "CDRRRenderable", "class",
     "CRenderable producing a digitally reconstructed radiograph by ray-casting through a volume."),
    ("GEOM_VIEW", "CRenderContext", "class",
     "Wraps the underlying Direct3D device and render state shared by the renderables of a scene."),
    ("GEOM_VIEW", "CTexture", "class",
     "Texture object for texture-mapping onto renderables, uploaded to the render context."),
    ("GEOM_VIEW", "CUSTOMVERTEX_POS_NORM", "class",
     "Custom vertex struct carrying position and normal, with color supplied by the material "
     "instead of per vertex."),
    ("GEOM_VIEW", "CTracker", "class",
     "Base class responding to mouse events on the scene view, the interaction handler abstraction."),
    ("GEOM_VIEW", "CRotateTracker", "class",
     "CTracker that rotates the view about a central point in response to mouse drag."),
    ("GEOM_VIEW", "CZoomTracker", "class",
     "CTracker that zooms the view in response to mouse drag."),
    ("GEOM_VIEW", "CObjectExplorer", "class",
     "MFC CTreeCtrl presenting the model object hierarchy as a browsable tree."),
    ("GEOM_VIEW", "CObjectTreeItem", "class",
     "One node in the CObjectExplorer tree, binding a model object to its tree item and command "
     "handling."),
    ("GEOM_VIEW", "CTabControlBar", "class",
     "MFC CDialogBar hosting tabbed panes in the docking control bar."),
    ("GEOM_VIEW", "CDibView", "class",
     "Window displaying a device independent bitmap and letting the user place landmarks that are "
     "added to a CTPSTransform."),
    ("GEOM_VIEW", "CGraph", "class",
     "MFC CWnd 2D plot window, GEOM_VIEW's copy of the reusable graph widget."),
    ("GEOM_VIEW", "CDataSeries", "class",
     "Curve data container deriving from CModelObject, plotted by GEOM_VIEW's CGraph."),

    # ================================================================
    # OPTIMIZER_BASE -- base optimization framework (component)
    # ================================================================
    ("OPTIMIZER_BASE", "COptimizer", "class",
     "Base class for all optimizers in the earlier optimization framework, owning an objective "
     "function and driving iterations to a tolerance."),
    ("OPTIMIZER_BASE", "CObjectiveFunction", "class",
     "Base class for objective functions, allowing a gradient to be defined with a flag for the "
     "case where no gradient is available."),
    ("OPTIMIZER_BASE", "CLineFunction", "class",
     "Objective function restricting another objective function to a line in its vector-space "
     "domain, turning N-dimensional search into a one-dimensional line search."),
    ("OPTIMIZER_BASE", "CBrentOptimizer", "algorithm",
     "One-dimensional Brent line-search optimizer as described in Numerical Recipes, combining "
     "golden-section search with parabolic interpolation."),
    ("OPTIMIZER_BASE", "CPowellOptimizer", "algorithm",
     "Powell direction-set optimizer as described in Numerical Recipes, requiring no gradient."),
    ("OPTIMIZER_BASE", "CConjGradOptimizer", "algorithm",
     "Conjugate gradient optimizer, the precursor of RtModel's DynamicCovarianceOptimizer."),

    # ================================================================
    # OptimizeN -- N-dimensional optimization (component)
    # ================================================================
    ("OptimizeN", "COptimizer", "class",
     "Base template class for all optimizers in the N-dimensional optimization library."),
    ("OptimizeN", "CObjectiveFunction", "class",
     "Base class template for all objective functions, allowing the objective to define a gradient "
     "with a flag for when no gradient is available."),
    ("OptimizeN", "CLineFunction", "class",
     "Line function defined from another objective function and a line in that function's "
     "vector-space domain."),
    ("OptimizeN", "CBrentOptimizer", "algorithm",
     "Optimizer implementing the Brent algorithm explained in Numerical Recipes."),
    ("OptimizeN", "CPowellOptimizer", "algorithm",
     "Optimizer implementing the Powell algorithm explained in Numerical Recipes."),
    ("OptimizeN", "CConjGradOptimizer", "algorithm",
     "Conjugate gradient optimizer over an N-dimensional objective function with line search."),
    ("OptimizeN", "CGradDescOptimizer", "algorithm",
     "Gradient descent optimizer stepping along the negative gradient direction."),
    ("OptimizeN", "CDFPOptimizer", "algorithm",
     "Davidon-Fletcher-Powell quasi-Newton optimizer building an approximate inverse Hessian from "
     "successive gradients."),
    ("OptimizeN", "CCubicInterpOptimizer", "algorithm",
     "Optimizes a function by fitting a cubic, usable only for functions that supply gradient "
     "information."),

    # ================================================================
    # OGL_BASE -- OpenGL base classes (component)
    # ================================================================
    ("OGL_BASE", "COpenGLView", "class",
     "MFC CWnd window owning an OpenGL rendering context and driving the render loop."),
    ("OGL_BASE", "COpenGLRenderer", "class",
     "Base class for a renderer belonging to a COpenGLView window."),
    ("OGL_BASE", "COpenGLCamera", "class",
     "Observable camera defining the OpenGL viewpoint and projection."),
    ("OGL_BASE", "COpenGLLight", "class",
     "Observable light source configuring OpenGL lighting state."),
    ("OGL_BASE", "COpenGLTexture", "class",
     "OpenGL texture object wrapping texture upload and binding."),
    ("OGL_BASE", "COpenGLTracker", "class",
     "Observable base class handling mouse interaction on a COpenGLView."),
    ("OGL_BASE", "CRotateTracker", "class",
     "COpenGLTracker rotating the OpenGL view in response to mouse drag."),
    ("OGL_BASE", "CZoomTracker", "class",
     "COpenGLTracker zooming the OpenGL view in response to mouse drag."),

    # ================================================================
    # GUI_BASE -- reusable MFC widgets (component)
    # ================================================================
    ("GUI_BASE", "CObjectExplorer", "class",
     "MFC CTreeCtrl presenting a model object hierarchy as a browsable tree."),
    ("GUI_BASE", "CObjectTreeItem", "class",
     "One node in the object explorer tree, binding a model object to its tree item."),
    ("GUI_BASE", "CObjectExplorerControlBar", "class",
     "MFC CDialogBar docking the object explorer tree into a frame window."),
    ("GUI_BASE", "CTabControlBar", "class",
     "MFC CDialogBar hosting tabbed panes in a docking control bar."),
    ("GUI_BASE", "CGraph", "class",
     "Reusable MFC CWnd 2D plot window with axes and legend."),
    ("GUI_BASE", "CDataSeries", "class",
     "Curve data container holding an array of two-dimensional points for CGraph."),
    ("GUI_BASE", "CDib", "class",
     "Device independent bitmap implemented as a form of MFC CBitmap."),
    ("GUI_BASE", "DrawTool", "function",
     "Drawing helpers rendering bitmaps with an embossed look and transparent raster operations."),

    # ================================================================
    # GenImaging -- ITK imaging filters (component)
    # ================================================================
    ("GenImaging", "InPlaneResampleImageFilter", "algorithm",
     "ITK ImageToImageFilter resampling 3D volumes in-plane with linear or nearest-neighbor "
     "interpolation."),
    ("GenImaging", "IntensityMapAccumulateImageFilter", "algorithm",
     "ITK filter accumulating dose by weighted pixel-wise multiply and add of beamlet intensity "
     "maps over a basis group of volumes, IPP accelerated."),
    ("GenImaging", "ContoursToRegionFilter", "algorithm",
     "ITK filter rasterizing polygon contours into a binary region mask volume, using continuous "
     "index vertices."),
    ("GenImaging", "MultiMaskNegatedImageFilter", "algorithm",
     "ITK filter masking an image by the negation of multiple mask inputs, automatically "
     "conforming its output to input zero. Used to build the unclaimed-tissue region."),
    ("GenImaging", "ScalarImageToWeightedHistogramGenerator", "algorithm",
     "ITK generator producing a histogram from a scalar image where each voxel carries a "
     "fractional weight, the basis of partial-volume dose-volume histograms."),
    ("GenImaging", "WeightedListSampleToHistogramGenerator", "algorithm",
     "ITK generator binning a weighted list sample into a dense frequency container, the sample "
     "level counterpart of the weighted histogram generator."),

    # ================================================================
    # XMLLogging -- XML diagnostic logging (component)
    # ================================================================
    ("XMLLogging", "CXMLLogFile", "class",
     "Thread-safe singleton controlling XML logging, writing structured nested XML diagnostic logs "
     "to a FILE pointer."),
    ("XMLLogging", "CXMLElement", "class",
     "Scoped RAII XML tag element managing nesting of attributes and text, opening on construction "
     "and closing on destruction."),
    ("XMLLogging", "CXMLLoggableObject", "class",
     "Mixin base giving an object the ability to emit its own state as an XML element into the log "
     "file."),
    ("XMLLogging", "CXMLConsoleApp", "class",
     "MFC application for the XML console viewer that reads and displays the diagnostic logs."),
    ("XMLLogging", "CXMLConsoleDoc", "class",
     "MFC document holding a parsed XML diagnostic log for the console viewer."),
    ("XMLLogging", "CXMLConsoleView", "class",
     "MFC CListView displaying XML log entries in the console viewer."),
    ("XMLLogging", "CLeftView", "class",
     "MFC CView showing the nested XML log section tree in the console viewer's left pane."),

    # ================================================================
    # FieldCOM -- COM/ATL geometry interop (component)
    # ================================================================
    ("FieldCOM", "CMesh", "class",
     "COM ATL mesh coclass exposing IMesh with FileStorage IPersistStorage save and load."),
    ("FieldCOM", "CMeshSet", "class",
     "COM ATL coclass holding a collection of meshes as a single persistable object."),
    ("FieldCOM", "CFileStorage", "class",
     "COM persistence interface saving and loading geometry to a pathname or stream."),
    ("FieldCOM", "CPolygon3D", "class",
     "COM ATL coclass exposing a three-dimensional polygon."),
    ("FieldCOM", "CPolygonSet3D", "class",
     "COM ATL coclass holding a collection of three-dimensional polygons, the interop form of a "
     "structure's contour stack."),
    ("FieldCOM", "CMatrix", "class",
     "COM ATL coclass exposing a matrix across COM boundaries."),
    ("FieldCOM", "CAffineTransform", "class",
     "COM ATL coclass exposing an affine geometric transform."),
    ("FieldCOM", "CTPSTransform", "class",
     "COM ATL coclass exposing a thin-plate spline transform for interop."),
    ("FieldCOM", "CBufferField", "class",
     "COM ATL coclass exposing a buffer-backed field, the interop form of the FTL field "
     "abstraction."),
    ("FieldCOM", "CProxy_ICollectionEvents", "class",
     "ATL connection point proxy firing collection change events to COM sinks."),
    ("FieldCOM", "CProxy_IObjectEvents", "class",
     "ATL connection point proxy firing object change events to COM sinks, the COM bridge for the "
     "observer pattern."),

    # ================================================================
    # RT_MODEL -- legacy radiotherapy model
    # ================================================================
    ("RT_MODEL", "CPlan", "class",
     "Legacy treatment plan deriving from CModelObject, holding beams, dose and histograms. The "
     "MFC-era precursor of RtModel's Plan."),
    ("RT_MODEL", "CBeam", "class",
     "Legacy single treatment beam deriving from CModelObject, with MAX_SCALES of 4 for the "
     "multi-scale beamlet hierarchy."),
    ("RT_MODEL", "CSeries", "class",
     "Legacy imaging series grouping the CT volume and structures, deriving from CModelObject."),
    ("RT_MODEL", "CStructure", "class",
     "Legacy anatomical structure or volume of interest deriving from CModelObject, holding "
     "contours and a rasterized volume."),
    ("RT_MODEL", "CPrescription", "class",
     "Legacy prescription objective function deriving from CObjectiveFunction, aggregating VOI "
     "terms."),
    ("RT_MODEL", "CVOITerm", "class",
     "Legacy base class for objective function terms, deriving from CModelObject."),
    ("RT_MODEL", "CKLDivTerm", "algorithm",
     "Legacy Kullback-Leibler divergence match of a dose-volume histogram to a target curve."),
    ("RT_MODEL", "CTCPTerm", "algorithm",
     "Legacy tumor control probability objective term deriving from CVOITerm."),
    ("RT_MODEL", "CNTCPTerm", "algorithm",
     "Legacy normal tissue complication probability objective term deriving from CVOITerm."),
    ("RT_MODEL", "CTCP_NTCP_Optimizer", "algorithm",
     "Legacy dose optimizer whose objective is tumor control probability against normal tissue "
     "complication probability, the radiobiological alternative to the KL divergence objective."),
    ("RT_MODEL", "CTCP_Params", "class",
     "Radiobiological parameters of one target structure for the TCP term."),
    ("RT_MODEL", "CNTCP_Params", "class",
     "Radiobiological parameters of one organ at risk for the NTCP term."),
    ("RT_MODEL", "CHistogram", "class",
     "Legacy dose-volume histogram over a volume optionally restricted to a binary region, "
     "deriving from CModelObject."),
    ("RT_MODEL", "CBeamDoseCalc", "algorithm",
     "Legacy TERMA ray tracer and dose calculator over CVolume density data."),
    ("RT_MODEL", "CEnergyDepKernel", "algorithm",
     "Legacy energy deposition kernel with radial and angular lookup tables."),
    ("RT_MODEL", "CTreatmentMachine", "class",
     "Legacy treatment machine model deriving from CModelObject, describing gantry, collimator and "
     "couch geometry."),
    ("RT_MODEL", "CSeriesDicomImporter", "class",
     "Legacy DICOM series and structure set importer with a nested CDicomImageItem helper."),

    # ================================================================
    # RT_VIEW -- legacy radiotherapy visualization
    # ================================================================
    ("RT_VIEW", "CBeamRenderable", "class",
     "CRenderable drawing a treatment beam in a CSceneView: the divergent field edges from source "
     "to isocenter."),
    ("RT_VIEW", "CMachineRenderable", "class",
     "CRenderable drawing the treatment machine geometry, gantry and couch, in a CSceneView."),
    ("RT_VIEW", "CLightfieldTexture", "class",
     "CTexture projecting the beam light field onto the patient surface for beam's eye view "
     "display."),
    ("RT_VIEW", "CBeamParamPosCtrl", "class",
     "MFC dialog control panel editing beam position parameters: gantry angle, couch angle and "
     "isocenter."),
    ("RT_VIEW", "CBeamParamCollimCtrl", "class",
     "MFC dialog control panel editing beam collimator parameters: jaw positions and collimator "
     "rotation."),

    # ================================================================
    # VSIM_MODEL -- virtual simulation data model
    # ================================================================
    ("VSIM_MODEL", "CPlan", "class",
     "Virtual-simulation treatment plan implemented as an MFC CDocument holding beams and machine."),
    ("VSIM_MODEL", "CSeries", "class",
     "Virtual-simulation imaging series implemented as an MFC CDocument holding the CT volume."),
    ("VSIM_MODEL", "CBeam", "class",
     "Virtual-simulation beam deriving from CModelObject, referencing the treatment machine "
     "geometry."),
    ("VSIM_MODEL", "CTreatmentMachine", "class",
     "Virtual-simulation treatment machine model deriving from CModelObject."),

    # ================================================================
    # VSIM_OGL -- virtual simulation OpenGL viewer
    # ================================================================
    ("VSIM_OGL", "CDRRRenderer", "algorithm",
     "OpenGL renderer producing a digitally reconstructed radiograph by ray-casting the CT volume "
     "along the beam axis, the core of virtual simulation."),
    ("VSIM_OGL", "CSimView", "class",
     "MFC view hosting the OpenGL virtual-simulation scene with the DRR and beam geometry."),
    ("VSIM_OGL", "CVSIM_OGLApp", "class",
     "MFC application entry point for the virtual-simulation OpenGL viewer."),

    # ================================================================
    # DivFluence -- divergent fluence calculator (utility)
    # ================================================================
    ("DivFluence", "fluence integration", "algorithm",
     "Divergent fluence calculation integrating exponential attenuation, inverse-square falloff, "
     "heterogeneous density, and field divergence over a CVolume."),
    ("DivFluence", "CBeam", "class",
     "Standalone beam definition for the divergent fluence experiment, carrying source position and "
     "field geometry over a CVolume."),
    ("DivFluence", "CMachine", "class",
     "Standalone treatment machine definition supplying source-to-axis distance and field limits "
     "for the fluence calculation."),
    ("DivFluence", "CEnergyDepKernel", "algorithm",
     "Standalone energy deposition kernel for the fluence experiment, with NUM_THETA of 8 azimuthal "
     "steps."),

    # ================================================================
    # PenBeamEdit -- pencil beam editor (utility)
    # ================================================================
    ("PenBeamEdit", "CPenBeamEditApp", "class",
     "MFC application entry point for the standalone pencil-beam editor."),
    ("PenBeamEdit", "CPenBeamEditView", "class",
     "MFC view displaying and editing pencil-beam kernel data interactively."),

    # ================================================================
    # PenBeam_indens -- Fortran reference implementation (utility)
    # ================================================================
    ("PenBeam_indens", "pencil beam convolution", "algorithm",
     "Fortran pencil-beam in-density convolution: ray-trace through heterogeneous phantom with "
     "spherical convolve kernels and energy lookup interpolation. The reference implementation the "
     "C++ dose calculation was derived from."),

    # ================================================================
    # WarpTps -- thin-plate spline image warping (utility)
    # ================================================================
    ("WarpTps", "CTPSTransform", "algorithm",
     "Thin-plate spline transform holding a collection of position vector pairs and interpolating "
     "the deformation field from those landmarks. The standalone WarpTps library's core algorithm, "
     "used for deformable image registration and morphing."),
    ("WarpTps", "CWarpTPSApp", "class",
     "MFC application entry point for the thin-plate spline image warping tool."),
    ("WarpTps", "CWarpTPSDoc", "class",
     "MFC document owning the source and target bitmaps and the landmark correspondences that "
     "define the warp."),
    ("WarpTps", "CWarpTPSView", "class",
     "MFC view rendering the warped image as the morph parameter is swept."),
    ("WarpTps", "CDibView", "class",
     "Window displaying a device independent bitmap and letting the user place landmarks that are "
     "added to the CTPSTransform."),
    ("WarpTps", "MorphSlider", "class",
     "MFC dialog with a slider driving the morph parameter that interpolates between source and "
     "target landmark sets."),
    ("WarpTps", "CVectorBase", "class",
     "WarpTps's own copy of the base vector template, keeping the library self-contained."),
    ("WarpTps", "CModelObject", "class",
     "WarpTps's own copy of the serializable hierarchical model object base class."),

    # ================================================================
    # EGSnrc -- Monte Carlo kernel generation (utility)
    # ================================================================
    ("EGSnrc", "kernel generation environment", "algorithm",
     "Docker-based EGSnrc Monte Carlo environment generating the energy deposition kernels the "
     "pencil beam convolution dose calculation consumes. EGSnrc simulates ionizing radiation "
     "transport through matter for dose calculation, dosimetry and detector response."),
    ("EGSnrc", "generate_kernel", "function",
     "Shell script driving the EGSnrc run that produces an energy deposition kernel for a given "
     "beam energy such as 6 MV or 15 MV, from an egsinp template."),
    ("EGSnrc", "convert_kernel", "function",
     "Shell script converting EGSnrc Monte Carlo output into the kernel.dat cumulative-energy "
     "lookup table format that CEnergyDepKernel loads."),

    # ================================================================
    # RtModelSmokeTest -- standalone numerical test (utility)
    # ================================================================
    ("RtModelSmokeTest", "conv1d_linear smoke test", "algorithm",
     "Standalone smoke test for the hand-rolled one-dimensional linear convolution that replaced "
     "the Intel IPP ippsConv_64f call in Histogram ConvGauss and HistogramGradient Conv_dGauss. "
     "Verifies output dimension, sum preservation, delta-input kernel recovery, symmetry and "
     "boundary taps."),

    # ================================================================
    # python -- ITK scripts, bindings, and the pybrimstone package (utility)
    # ================================================================
    ("python", "ct_to_md_values", "function",
     "Converts CT Hounsfield unit values to mass density via piecewise calibration, ITK based."),
    ("python", "rotate_density_for_beam", "function",
     "Rotates a density volume to a beam gantry angle for ray-traced dose calculation."),
    ("python", "terma_from_density", "function",
     "Computes TERMA total energy released in medium by ray-tracing through a density volume."),
    ("python", "pybrimstone.numerics.conjugate_gradient", "algorithm",
     "Polak-Ribiere conjugate gradient with dynamic-covariance adaptive variance. The Python "
     "reference implementation of RtModel's DynamicCovarianceOptimizer."),
    ("python", "pybrimstone.numerics.histogram", "algorithm",
     "Gaussian-convolved dose-volume histogram primitives, the Python counterpart of CHistogram "
     "and its smoothing."),
    ("python", "pybrimstone.numerics.kl_divergence", "algorithm",
     "KL divergence computation between calculated and target dose-volume histograms, the Python "
     "counterpart of KLDivTerm."),
    ("python", "pybrimstone.numerics.parameter_transform", "algorithm",
     "Sigmoid optimizer-to-beamlet transform mapping unbounded variables to bounded weights, the "
     "Python counterpart of Prescription Transform and dTransform."),
    ("python", "pybrimstone.prescription", "algorithm",
     "Composite objective function over a treatment plan, summing weighted objective terms. The "
     "Python counterpart of RtModel's Prescription."),
    ("python", "pybrimstone.objective_terms", "class",
     "Base classes for objective-function terms, the Python counterpart of VOITerm."),
    ("python", "pybrimstone.kl_term", "algorithm",
     "KL divergence dose-fit objective term over a structure's dose-volume histogram."),
    ("python", "pybrimstone.dose_calc", "algorithm",
     "Simple linear dose calculator for prototyping, mapping beamlet weights to dose without full "
     "TERMA and convolution."),
    ("python", "pybrimstone.terma_kernel_dose", "algorithm",
     "TERMA plus kernel-convolution dose calculator, the Python implementation of the full "
     "ray-trace and spherical convolution pipeline."),
    ("python", "pybrimstone.phase_optimizer", "algorithm",
     "Phase-level optimizer bridging the Prescription objective and conjugate gradient to the "
     "treatment phase, the Python counterpart of PlanOptimizer."),
    ("python", "pybrimstone.hierarchical_bayes", "algorithm",
     "Course-level coordinate-ascent driver for hierarchical-Bayes treatment planning, alternating "
     "between phase-level dose optimization and course-level prior updates."),
    ("python", "pybrimstone.course_prior", "algorithm",
     "Course-level prior term for hierarchical-Bayes treatment planning, coupling the phases of a "
     "multi-phase course."),
    ("python", "pybrimstone.free_energy", "algorithm",
     "Variational free-energy diagnostics for the hierarchical-Bayes driver, reporting the KL and "
     "entropy split of the objective."),
    ("python", "pybrimstone.dvh_uncertainty", "algorithm",
     "DVH uncertainty-band computation for hierarchical-Bayes treatment planning, propagating "
     "posterior variance into dose-volume histogram confidence bands."),
    ("python", "pybrimstone.bootstrap", "algorithm",
     "Voxel-subsampling bootstrap for posterior-variance estimation, resampling voxels to estimate "
     "uncertainty in the fitted plan."),
    ("python", "pybrimstone.amortized.network", "algorithm",
     "Amortized course-prior neural network learning to predict the course-level prior directly "
     "from plan features, replacing per-course inference."),
    ("python", "pybrimstone.amortized.train", "algorithm",
     "Training loop for the amortized course-prior network."),
    ("python", "pybrimstone.amortized.infer", "algorithm",
     "Inference loop applying the trained amortized course-prior network to a new course."),
    ("python", "pybrimstone.amortized.data", "function",
     "Data generation for the amortized course-prior prototype, producing training pairs from "
     "closed-form toys and recorded conjugate gradient trajectories."),
    ("python", "pybrimstone.amortized.types", "class",
     "Dataclasses defining the record types of the amortized course-prior prototype."),
    ("python", "pybrimstone.amortized.config", "class",
     "Configuration dataclass for the amortized course-prior prototype."),
    ("python", "pybrimstone.datasets", "function",
     "Dataset loaders for testing the amortized Course prior against real public treatment "
     "planning datasets."),
    ("python", "pybrimstone.tg263_model", "algorithm",
     "TG-263 structure name translator implemented in PyTorch, learning to map free-text region of "
     "interest names onto standardized TG-263 names."),
    ("python", "pybrimstone.tg263_training", "algorithm",
     "Training utilities for the TG-263 structure name translator."),
    ("python", "pybrimstone.core", "class",
     "Cython declaration and implementation bridging the Python package to the native RtModel "
     "library, built as a Windows .pyd extension."),
    ("python", "rtmodel_bindings", "class",
     "pybind11 bindings exposing RtModel's Plan, Beam, Prescription and optimizer types to Python "
     "as a native extension module."),
    ("python", "automate_brimstone_ui", "function",
     "pywinauto UI automation harness driving the Brimstone MFC application end to end: opens a "
     "series, sets the prescription and beam count, and launches optimization for scripted "
     "experiments."),
    ("python", "run_knee", "function",
     "Script sweeping the entropy weight and locating the knee of the free-energy curve, used to "
     "validate the entropy regularization setting."),
    ("python", "embed_components", "function",
     "Generates Ollama nomic-embed-text vector embeddings for each repository component from the "
     "descriptions mirroring COMPONENT_INVENTORY.md."),
    ("python", "embed_classes", "function",
     "Generates Ollama nomic-embed-text vector embeddings for each important class and algorithm in "
     "the repository."),
    ("python", "umap_sweep", "algorithm",
     "Faithfulness sweep scoring every 768 to intermediate and 768 to intermediate to final UMAP "
     "cascade cell against the original 768 dimensional space, by trustworthiness, continuity, kNN "
     "overlap, Spearman correlation and silhouette."),
    ("python", "umap_layout", "algorithm",
     "Builds the viewer's data.json: all intermediate dimension by n_neighbors cascade cells, each "
     "with a 3D and 2D projection plus per-node vectors at every intermediate dimension."),

    # ================================================================
    # notebook_zoo -- research notebooks (utility)
    # ================================================================
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
        print(f"{library:18s} {name:44s} {kind:9s} dim={len(vec)}")
        out.append({
            "library": library,
            "kind": kind,
            "name": name,
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
