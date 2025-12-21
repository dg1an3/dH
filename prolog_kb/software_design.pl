%% =============================================================================
%% SOFTWARE DESIGN - Classes, Methods, Modules, Files
%% =============================================================================
%% Ontology-inspired representation of the Brimstone software architecture.
%% Based on SWO (Software Ontology), IAO (Information Artifact Ontology).
%% =============================================================================

:- module(software_design, [
    module_def/4,
    module_dependency/2,
    source_file/5,
    class/5,
    method/6,
    datatype/4,
    enum_value/4,
    constant/5,
    algorithm/5,
    association/5,
    includes/2,
    % Derived predicates
    inherits/2,
    defined_in/2,
    has_method/2,
    depends_on/2,
    implementation_of/2
]).

%% =============================================================================
%% MODULE DEFINITIONS
%% =============================================================================

%% module_def(+ModuleName, +Type, +Description, +Path)
module_def(rtmodel, library, 'Core optimization and modeling library', 'RtModel/').
module_def(brimstone_app, application, 'MFC-based Windows GUI application', 'Brimstone/').
module_def(graph, library, 'Visualization components for DVH plotting', 'Graph/').
module_def(python_utils, utility, 'ITK-based Python utilities for processing', 'python/').

%% module_dependency(+Dependent, +Dependency)
module_dependency(brimstone_app, rtmodel).
module_dependency(brimstone_app, graph).
module_dependency(graph, rtmodel).
module_dependency(python_utils, rtmodel).


%% =============================================================================
%% SOURCE FILE DEFINITIONS
%% =============================================================================

%% source_file(+Path, +Module, +Type, +LineCount, +Description)

% RtModel Headers
source_file('RtModel/include/Plan.h', rtmodel, header, 150, 'Treatment plan container').
source_file('RtModel/include/Series.h', rtmodel, header, 80, 'CT series container').
source_file('RtModel/include/Structure.h', rtmodel, header, 180, 'ROI with contours').
source_file('RtModel/include/Beam.h', rtmodel, header, 120, 'Treatment beam').
source_file('RtModel/include/Prescription.h', rtmodel, header, 200, 'Objective function').
source_file('RtModel/include/VOITerm.h', rtmodel, header, 80, 'Structure constraint base').
source_file('RtModel/include/KLDivTerm.h', rtmodel, header, 100, 'KL divergence term').
source_file('RtModel/include/ConjGradOptimizer.h', rtmodel, header, 120, 'CG optimizer').
source_file('RtModel/include/PlanOptimizer.h', rtmodel, header, 100, 'Multi-level optimizer').
source_file('RtModel/include/PlanPyramid.h', rtmodel, header, 80, 'Multi-scale pyramid').
source_file('RtModel/include/Histogram.h', rtmodel, header, 200, 'DVH histogram').
source_file('RtModel/include/HistogramGradient.h', rtmodel, header, 150, 'Gradient histogram').
source_file('RtModel/include/BeamDoseCalc.h', rtmodel, header, 100, 'TERMA calculation').
source_file('RtModel/include/SphereConvolve.h', rtmodel, header, 90, 'Spherical convolution').
source_file('RtModel/include/EnergyDepKernel.h', rtmodel, header, 120, 'Energy kernel').
source_file('RtModel/include/ObjectiveFunction.h', rtmodel, header, 80, 'Cost function base').
source_file('RtModel/include/ItkUtils.h', rtmodel, header, 100, 'ITK utilities').
source_file('RtModel/include/VectorN.h', rtmodel, header, 150, 'Vector operations').
source_file('RtModel/include/MatrixNxM.h', rtmodel, header, 200, 'Matrix operations').

% RtModel Implementations
source_file('RtModel/Plan.cpp', rtmodel, implementation, 294, 'Plan implementation').
source_file('RtModel/Series.cpp', rtmodel, implementation, 85, 'Series implementation').
source_file('RtModel/Structure.cpp', rtmodel, implementation, 295, 'Structure implementation').
source_file('RtModel/Beam.cpp', rtmodel, implementation, 198, 'Beam implementation').
source_file('RtModel/Prescription.cpp', rtmodel, implementation, 654, 'Prescription implementation').
source_file('RtModel/VOITerm.cpp', rtmodel, implementation, 41, 'VOITerm implementation').
source_file('RtModel/KLDivTerm.cpp', rtmodel, implementation, 490, 'KLDivTerm implementation').
source_file('RtModel/ConjGradOptimizer.cpp', rtmodel, implementation, 367, 'Optimizer implementation').
source_file('RtModel/PlanOptimizer.cpp', rtmodel, implementation, 391, 'PlanOptimizer implementation').
source_file('RtModel/PlanPyramid.cpp', rtmodel, implementation, 267, 'Pyramid implementation').
source_file('RtModel/Histogram.cpp', rtmodel, implementation, 806, 'Histogram implementation').
source_file('RtModel/HistogramGradient.cpp', rtmodel, implementation, 557, 'HistogramGradient implementation').
source_file('RtModel/BeamDoseCalc.cpp', rtmodel, implementation, 398, 'BeamDoseCalc implementation').
source_file('RtModel/SphereConvolve.cpp', rtmodel, implementation, 347, 'SphereConvolve implementation').
source_file('RtModel/EnergyDepKernel.cpp', rtmodel, implementation, 528, 'EnergyDepKernel implementation').

% Brimstone Application
source_file('Brimstone/Brimstone.h', brimstone_app, header, 50, 'Application class').
source_file('Brimstone/BrimstoneDoc.h', brimstone_app, header, 80, 'Document class').
source_file('Brimstone/BrimstoneView.h', brimstone_app, header, 120, 'View class').
source_file('Brimstone/MainFrm.h', brimstone_app, header, 40, 'Main frame').
source_file('Brimstone/OptThread.h', brimstone_app, header, 80, 'Optimization thread').
source_file('Brimstone/SeriesDicomImporter.h', brimstone_app, header, 100, 'DICOM importer').
source_file('Brimstone/Brimstone.cpp', brimstone_app, implementation, 154, 'App initialization').
source_file('Brimstone/BrimstoneDoc.cpp', brimstone_app, implementation, 274, 'Document I/O').
source_file('Brimstone/BrimstoneView.cpp', brimstone_app, implementation, 459, 'View rendering').
source_file('Brimstone/SeriesDicomImporter.cpp', brimstone_app, implementation, 400, 'DICOM parsing').
source_file('Brimstone/PlanarView.cpp', brimstone_app, implementation, 1089, '2D rendering').

% Graph Library
source_file('Graph/include/Graph.h', graph, header, 150, 'Graph window').
source_file('Graph/include/DataSeries.h', graph, header, 80, 'Data series').
source_file('Graph/include/HistogramDataSeries.h', graph, header, 50, 'Histogram series').
source_file('Graph/Graph.cpp', graph, implementation, 635, 'Graph rendering').
source_file('Graph/DataSeries.cpp', graph, implementation, 117, 'DataSeries implementation').

% Python Utilities
source_file('python/read_process_series.py', python_utils, implementation, 200, 'CT processing').
source_file('python/optimize_with_lbfgs.py', python_utils, implementation, 150, 'L-BFGS-B wrapper').


%% =============================================================================
%% CLASS DEFINITIONS
%% =============================================================================

%% class(+Name, +Namespace, +ParentClass, +DefinedIn, +Description)

% Core Data Model Classes
class('Series', 'dH', 'itk::DataObject', 'RtModel/include/Series.h',
    'CT density volume with anatomical structures').
class('Structure', 'dH', 'itk::DataObject', 'RtModel/include/Structure.h',
    'Region of interest with multi-scale contours').
class('Beam', 'dH', 'itk::DataObject', 'RtModel/include/Beam.h',
    'Treatment beam with beamlets and intensity').
class('Plan', 'dH', 'itk::DataObject', 'RtModel/include/Plan.h',
    'Treatment plan with beams, dose, histograms').

% Optimization Classes
class('DynamicCovarianceCostFunction', 'dH', 'vnl_cost_function',
    'RtModel/include/ObjectiveFunction.h',
    'Cost function with adaptive variance').
class('Prescription', 'dH', 'DynamicCovarianceCostFunction',
    'RtModel/include/Prescription.h',
    'Objective function coordinating VOI terms').
class('VOITerm', 'dH', 'itk::DataObject', 'RtModel/include/VOITerm.h',
    'Base class for structure constraints').
class('KLDivTerm', 'dH', 'VOITerm', 'RtModel/include/KLDivTerm.h',
    'KL divergence DVH matching term').
class('DynamicCovarianceOptimizer', 'dH', 'vnl_nonlinear_minimizer',
    'RtModel/include/ConjGradOptimizer.h',
    'CG optimizer with adaptive variance').
class('PlanOptimizer', 'dH', none, 'RtModel/include/PlanOptimizer.h',
    'Multi-level optimization orchestrator').
class('PlanPyramid', 'dH', none, 'RtModel/include/PlanPyramid.h',
    'Four-level multi-scale pyramid').

% Histogram Classes
class('CHistogram', 'dH', 'itk::DataObject', 'RtModel/include/Histogram.h',
    'DVH with adaptive Gaussian binning').
class('CHistogramWithGradient', 'dH', 'CHistogram',
    'RtModel/include/HistogramGradient.h',
    'DVH with partial derivatives').

% Dose Calculation Classes
class('CBeamDoseCalc', 'dH', none, 'RtModel/include/BeamDoseCalc.h',
    'Pencil beam TERMA calculation').
class('CEnergyDepKernel', 'dH', none, 'RtModel/include/EnergyDepKernel.h',
    'Energy deposition kernel LUTs').
class('SphereConvolve', 'dH', none, 'RtModel/include/SphereConvolve.h',
    'Spherical energy convolution').

% GUI Classes (MFC)
class('CBrimstoneApp', 'mfc', 'CWinApp', 'Brimstone/Brimstone.h',
    'Main application class').
class('CBrimstoneDoc', 'mfc', 'CDocument', 'Brimstone/BrimstoneDoc.h',
    'Document owning Series and Plan').
class('CBrimstoneView', 'mfc', 'CView', 'Brimstone/BrimstoneView.h',
    'View rendering CT, contours, dose').
class('CMainFrame', 'mfc', 'CFrameWnd', 'Brimstone/MainFrm.h',
    'Main window frame').
class('COptThread', 'mfc', 'CWinThread', 'Brimstone/OptThread.h',
    'Async optimization thread').
class('CSeriesDicomImporter', 'mfc', none, 'Brimstone/SeriesDicomImporter.h',
    'DICOM CT/RT Structure import').
class('CPlanarView', 'mfc', none, 'Brimstone/PlanarView.h',
    '2D planar image viewer').

% Graph Classes
class('CGraph', 'graph', 'CWnd', 'Graph/include/Graph.h',
    'Graph plotting window').
class('CDataSeries', 'graph', 'itk::DataObject', 'Graph/include/DataSeries.h',
    'Generic data series').
class('CHistogramDataSeries', 'graph', 'CDataSeries',
    'Graph/include/HistogramDataSeries.h',
    'Histogram-based data series').


%% =============================================================================
%% METHOD DEFINITIONS
%% =============================================================================

%% method(+ClassName, +MethodName, +ReturnType, +Parameters, +Visibility, +Description)

% Series methods
method('Series', 'GetDensity', 'VolumeReal*', [], public, 'Get CT density volume').
method('Series', 'SetDensity', void, [param(pValue, 'VolumeReal*')], public, 'Set density').
method('Series', 'GetStructureCount', int, [], public, 'Get structure count').
method('Series', 'GetStructureAt', 'Structure*', [param(nAt, int)], public, 'Get structure by index').
method('Series', 'GetStructureFromName', 'Structure*', [param(name, 'const std::string&')], public, 'Find structure').
method('Series', 'AddStructure', void, [param(pStruct, 'Structure*')], public, 'Add structure').

% Structure methods
method('Structure', 'GetName', 'const std::string&', [], public, 'Get name').
method('Structure', 'SetName', void, [param(name, 'const std::string&')], public, 'Set name').
method('Structure', 'GetType', 'StructType', [], public, 'Get type').
method('Structure', 'SetType', void, [param(type, 'StructType')], public, 'Set type').
method('Structure', 'GetContourCount', int, [], public, 'Get contour count').
method('Structure', 'GetContour', 'PolygonType*', [param(nAt, int)], public, 'Get contour').
method('Structure', 'AddContour', void, [param(poly, 'PolygonType::Pointer'), param(dist, 'REAL')], public, 'Add contour').
method('Structure', 'GetRegion', 'const VolumeReal*', [param(level, int)], public, 'Get region at level').
method('Structure', 'CalcRegion', void, [], private, 'Calculate region from contours').
method('Structure', 'ContoursToRegion', void, [param(region, 'VolumeReal*')], private, 'Convert contours to region').

% Plan methods
method('Plan', 'GetSeries', 'Series*', [], public, 'Get associated series').
method('Plan', 'SetSeries', void, [param(pSeries, 'Series*')], public, 'Set series').
method('Plan', 'GetBeamCount', int, [], public, 'Get beam count').
method('Plan', 'GetBeamAt', 'CBeam*', [param(nAt, int)], public, 'Get beam by index').
method('Plan', 'AddBeam', int, [param(pBeam, 'CBeam*')], public, 'Add beam').
method('Plan', 'GetHistogram', 'CHistogram*', [param(pStruct, 'Structure*'), param(create, bool)], public, 'Get histogram').
method('Plan', 'GetDoseMatrix', 'VolumeReal*', [], public, 'Get dose volume').
method('Plan', 'GetTotalBeamletCount', int, [], public, 'Total beamlets').
method('Plan', 'UpdateAllHisto', void, [], public, 'Update all histograms').

% Beam methods
method('Beam', 'GetGantryAngle', double, [], public, 'Get gantry angle').
method('Beam', 'SetGantryAngle', void, [param(angle, double)], public, 'Set gantry angle').
method('Beam', 'GetIsocenter', 'itk::Vector<REAL>', [], public, 'Get isocenter').
method('Beam', 'SetIsocenter', void, [param(iso, 'itk::Vector<REAL>')], public, 'Set isocenter').
method('Beam', 'GetBeamletCount', int, [], public, 'Get beamlet count').
method('Beam', 'GetBeamlet', 'VolumeReal*', [param(n, int)], public, 'Get beamlet dose').
method('Beam', 'GetIntensityMap', 'IntensityMap*', [], public, 'Get intensity map').
method('Beam', 'SetIntensityMap', void, [param(weights, 'const CVectorN<>&')], public, 'Set intensities').
method('Beam', 'OnIntensityMapChanged', void, [], public, 'Recalculate dose').

% Prescription methods
method('Prescription', 'operator()', 'REAL', [param(input, 'const CVectorN<>&'), param(grad, 'CVectorN<>*')], public, 'Evaluate objective').
method('Prescription', 'GetStructureTerm', 'VOITerm*', [param(pStruct, 'Structure*')], public, 'Get term').
method('Prescription', 'AddStructureTerm', void, [param(term, 'VOITerm*')], public, 'Add term').
method('Prescription', 'Transform', void, [param(v, 'CVectorN<>*')], public, 'Sigmoid transform').
method('Prescription', 'dTransform', void, [param(v, 'CVectorN<>*')], public, 'Sigmoid derivative').
method('Prescription', 'InvTransform', void, [param(v, 'CVectorN<>*')], public, 'Inverse sigmoid').

% VOITerm methods
method('VOITerm', 'Eval', 'REAL', [param(grad, 'CVectorN<>*'), param(include, 'const CArray<BOOL,BOOL>&')], public_virtual, 'Evaluate term').
method('VOITerm', 'Clone', 'VOITerm*', [], public_virtual, 'Clone term').
method('VOITerm', 'GetVOI', 'Structure*', [], public, 'Get structure').
method('VOITerm', 'GetWeight', 'REAL', [], public, 'Get weight').
method('VOITerm', 'SetWeight', void, [param(w, 'REAL')], public, 'Set weight').

% KLDivTerm methods
method('KLDivTerm', 'SetDVPs', void, [param(dvp, 'const CMatrixNxM<>&')], public, 'Set dose-volume points').
method('KLDivTerm', 'GetDVPs', 'const CMatrixNxM<>&', [], public, 'Get dose-volume points').
method('KLDivTerm', 'SetInterval', void, [param(low, 'REAL'), param(high, 'REAL'), param(frac, 'REAL'), param(mid, 'BOOL')], public, 'Set interval').
method('KLDivTerm', 'GetTargetBins', 'const CVectorN<>&', [], public, 'Get target bins').

% DynamicCovarianceOptimizer methods
method('DynamicCovarianceOptimizer', 'minimize', 'ReturnCodes', [param(init, 'vnl_vector<REAL>&')], public, 'Run CG optimization').
method('DynamicCovarianceOptimizer', 'SetAdaptiveVariance', void, [param(calc, bool), param(min, 'REAL'), param(max, 'REAL')], public, 'Enable adaptive variance').
method('DynamicCovarianceOptimizer', 'SetComputeFreeEnergy', void, [param(compute, bool)], public, 'Enable free energy').
method('DynamicCovarianceOptimizer', 'GetEntropy', 'REAL', [], public, 'Get entropy').
method('DynamicCovarianceOptimizer', 'GetFreeEnergy', 'REAL', [], public, 'Get free energy').
method('DynamicCovarianceOptimizer', 'SetCallback', void, [param(cb, 'OptimizerCallback*'), param(param, 'void*')], public, 'Set callback').

% PlanOptimizer methods
method('PlanOptimizer', 'Optimize', bool, [param(init, 'CVectorN<>&'), param(cb, 'OptimizerCallback*'), param(param, 'void*')], public, 'Multi-level optimize').
method('PlanOptimizer', 'GetPrescription', 'Prescription*', [param(level, int)], public, 'Get prescription at level').
method('PlanOptimizer', 'GetOptimizer', 'DynamicCovarianceOptimizer*', [param(level, int)], public, 'Get optimizer at level').
method('PlanOptimizer', 'GetStateVectorFromPlan', void, [param(state, 'CVectorN<>&')], public, 'Extract state').
method('PlanOptimizer', 'SetStateVectorToPlan', void, [param(state, 'const CVectorN<>&')], public, 'Apply state').

% CHistogram methods
method('CHistogram', 'SetBinning', void, [param(min, 'REAL'), param(width, 'REAL'), param(sigma, 'REAL')], public, 'Set binning').
method('CHistogram', 'GetBins', 'const CVectorN<>&', [], public, 'Get bins').
method('CHistogram', 'GetCumBins', 'const CVectorN<>&', [], public, 'Get cumulative DVH').
method('CHistogram', 'GetGBins', 'const CVectorN<>&', [], public, 'Get Gaussian bins').
method('CHistogram', 'SetVolume', void, [param(vol, 'VolumeReal*')], public, 'Set volume').
method('CHistogram', 'SetRegion', void, [param(region, 'VolumeReal*')], public, 'Set region mask').

% CHistogramWithGradient methods
method('CHistogramWithGradient', 'Get_dVolumeCount', int, [], public, 'Get partial count').
method('CHistogramWithGradient', 'Get_dVolume', 'VolumeReal*', [param(at, int), param(group, 'int*')], public, 'Get partial volume').
method('CHistogramWithGradient', 'Add_dVolume', int, [param(vol, 'VolumeReal*'), param(group, int)], public, 'Add partial volume').
method('CHistogramWithGradient', 'Get_dBins', 'const CVectorN<>&', [param(at, int)], public, 'Get partial bins').
method('CHistogramWithGradient', 'Get_dGBins', 'const CVectorN<>&', [param(at, int)], public, 'Get partial Gaussian bins').

% CBeamDoseCalc methods
method('CBeamDoseCalc', 'InitCalcBeamlets', void, [], public, 'Initialize beamlet calc').
method('CBeamDoseCalc', 'CalcBeamlet', void, [param(n, int)], public, 'Calculate beamlet').
method('CBeamDoseCalc', 'CalcTerma', void, [], public, 'Calculate TERMA').
method('CBeamDoseCalc', 'TraceRayTerma', void, [param(ray, 'Vector<REAL>'), param(fluence, 'REAL')], private, 'Trace ray').

% CEnergyDepKernel methods
method('CEnergyDepKernel', 'CalcSphereConvolve', 'VolumeReal::Pointer', [param(density, 'VolumeReal*'), param(terma, 'VolumeReal*'), param(slice, int)], public, 'Sphere convolve').
method('CEnergyDepKernel', 'LoadKernel', void, [], private, 'Load kernel file').
method('CEnergyDepKernel', 'GetCumEnergy', double, [param(phi, int), param(rad, double)], public, 'Get cumulative energy').

% CSeriesDicomImporter methods
method('CSeriesDicomImporter', 'SetInputDirectory', void, [param(dir, 'const CString&')], public, 'Set DICOM directory').
method('CSeriesDicomImporter', 'ProcessNext', bool, [], public, 'Process next DICOM file').
method('CSeriesDicomImporter', 'GetSeries', 'Series*', [], public, 'Get resulting series').
method('CSeriesDicomImporter', 'ImportCT', bool, [param(file, 'DcmFileFormat*')], private, 'Import CT image').
method('CSeriesDicomImporter', 'ImportRTStruct', bool, [param(file, 'DcmFileFormat*')], private, 'Import RT Structure Set').


%% =============================================================================
%% DATA TYPE DEFINITIONS
%% =============================================================================

%% datatype(+Name, +Category, +Description, +DefinedIn)

% ITK Image Types
datatype('VolumeReal', typedef, 'itk::Image<REAL, 3>', 'RtModel/include/ItkUtils.h').
datatype('VolumeShort', typedef, 'itk::Image<short, 3>', 'RtModel/include/ItkUtils.h').
datatype('VolumeSliceReal', typedef, 'itk::Image<REAL, 2>', 'RtModel/include/ItkUtils.h').
datatype('PolygonType', typedef, 'itk::PolygonSpatialObject<2>', 'RtModel/include/Structure.h').
datatype('IntensityMap', typedef, 'itk::Image<VOXEL_REAL, 1>', 'RtModel/include/Beam.h').

% VNL Types
datatype('CVectorN', class_template, 'vnl_vector<REAL> wrapper', 'RtModel/include/VectorN.h').
datatype('CMatrixNxM', class_template, 'vnl_matrix<REAL> wrapper', 'RtModel/include/MatrixNxM.h').

% Numeric Types
datatype('REAL', typedef, 'double or float', 'RtModel/include/ItkUtils.h').
datatype('VOXEL_REAL', typedef, 'Voxel value type', 'RtModel/include/ItkUtils.h').

% Enumerations
datatype('StructType', enum, 'eNONE=0, eTARGET=1, eOAR=2', 'RtModel/include/Structure.h').
datatype('ReturnCodes', enum, 'vnl optimizer return codes', 'RtModel/include/ConjGradOptimizer.h').

% Callback Types
datatype('OptimizerCallback', typedef, 'BOOL(*)(DynamicCovarianceOptimizer*, void*)', 'RtModel/include/ConjGradOptimizer.h').

%% enum_value(+EnumType, +Value, +IntValue, +Description)
enum_value('StructType', 'eNONE', 0, 'Unspecified').
enum_value('StructType', 'eTARGET', 1, 'Target volume').
enum_value('StructType', 'eOAR', 2, 'Organ at risk').


%% =============================================================================
%% CONSTANTS
%% =============================================================================

%% constant(+Name, +Value, +Type, +Description, +DefinedIn)
constant('MAX_SCALES', 4, int, 'Pyramid levels', 'RtModel/include/PlanPyramid.h').
constant('DEFAULT_LEVELSIGMA', [8.0, 3.2, 1.3, 0.5], 'REAL[]', 'Voxel sizes per level', 'RtModel/include/PlanPyramid.h').
constant('ITER_MAX', 500, int, 'Max CG iterations', 'RtModel/ConjGradOptimizer.cpp').
constant('ZEPS', 1.0e-10, 'REAL', 'Convergence tolerance', 'RtModel/ConjGradOptimizer.cpp').
constant('SIGMOID_SCALE', 0.2, 'REAL', 'Sigmoid scaling', 'RtModel/Prescription.cpp').


%% =============================================================================
%% ALGORITHMS
%% =============================================================================

%% algorithm(+Name, +Type, +Description, +Implements, +DefinedIn)
algorithm(conjugate_gradient, optimization,
    'CG with Polak-Ribiere and adaptive variance',
    'DynamicCovarianceOptimizer::minimize',
    'RtModel/ConjGradOptimizer.cpp').
algorithm(kl_divergence, objective_function,
    'KL(P_calc || P_target) for DVH matching',
    'KLDivTerm::Eval',
    'RtModel/KLDivTerm.cpp').
algorithm(pencil_beam_terma, dose_calculation,
    'Ray-tracing TERMA calculation',
    'CBeamDoseCalc::CalcTerma',
    'RtModel/BeamDoseCalc.cpp').
algorithm(spherical_convolution, dose_calculation,
    'Spherical kernel convolution',
    'CEnergyDepKernel::CalcSphereConvolve',
    'RtModel/SphereConvolve.cpp').
algorithm(gaussian_histogram, signal_processing,
    'Adaptive Gaussian histogram convolution',
    'CHistogram::ConvGauss',
    'RtModel/Histogram.cpp').
algorithm(multi_scale_pyramid, optimization_strategy,
    'Coarse-to-fine across 4 levels',
    'PlanOptimizer::Optimize',
    'RtModel/PlanOptimizer.cpp').
algorithm(sigmoid_transform, parameter_mapping,
    'Unbounded to bounded beamlet weights',
    'Prescription::Transform',
    'RtModel/Prescription.cpp').
algorithm(free_energy, variational_inference,
    'F = KL - Entropy from covariance',
    'DynamicCovarianceOptimizer::ComputeEntropyFromCovariance',
    'RtModel/ConjGradOptimizer.cpp').


%% =============================================================================
%% ASSOCIATIONS
%% =============================================================================

%% association(+ClassA, +Relation, +ClassB, +Cardinality, +Description)
association('Plan', owns, 'Series', '1:1', 'Plan references CT series').
association('Plan', owns, 'Beam', '1:N', 'Plan contains beams').
association('Plan', owns, 'CHistogram', '1:N', 'Plan has histograms').
association('Series', owns, 'Structure', '1:N', 'Series contains structures').
association('Beam', owns, 'VolumeReal', '1:N', 'Beam has beamlet doses').
association('Prescription', owns, 'VOITerm', '1:N', 'Prescription has terms').
association('VOITerm', references, 'Structure', '1:1', 'Term for structure').
association('VOITerm', references, 'CHistogramWithGradient', '1:1', 'Term uses histogram').
association('PlanOptimizer', references, 'Plan', '1:1', 'Optimizer for plan').
association('PlanOptimizer', references, 'PlanPyramid', '1:1', 'Uses pyramid').
association('CBeamDoseCalc', references, 'Beam', '1:1', 'Calculator for beam').
association('CBeamDoseCalc', references, 'CEnergyDepKernel', '1:1', 'Uses kernel').
association('CBrimstoneDoc', owns, 'Series', '1:1', 'Document owns series').
association('CBrimstoneDoc', owns, 'Plan', '1:1', 'Document owns plan').
association('CGraph', contains, 'CDataSeries', '1:N', 'Graph has data series').


%% =============================================================================
%% FILE DEPENDENCIES
%% =============================================================================

%% includes(+SourceFile, +HeaderFile)
includes('RtModel/include/Plan.h', 'RtModel/include/Series.h').
includes('RtModel/include/Plan.h', 'RtModel/include/Beam.h').
includes('RtModel/include/Plan.h', 'RtModel/include/Histogram.h').
includes('RtModel/include/Series.h', 'RtModel/include/Structure.h').
includes('RtModel/include/Structure.h', 'RtModel/include/ItkUtils.h').
includes('RtModel/include/Beam.h', 'RtModel/include/ItkUtils.h').
includes('RtModel/include/Prescription.h', 'RtModel/include/ObjectiveFunction.h').
includes('RtModel/include/Prescription.h', 'RtModel/include/KLDivTerm.h').
includes('RtModel/include/Prescription.h', 'RtModel/include/Plan.h').
includes('RtModel/include/KLDivTerm.h', 'RtModel/include/VOITerm.h').
includes('RtModel/include/VOITerm.h', 'RtModel/include/HistogramGradient.h').
includes('RtModel/include/HistogramGradient.h', 'RtModel/include/Histogram.h').
includes('RtModel/include/ConjGradOptimizer.h', 'RtModel/include/ObjectiveFunction.h').
includes('RtModel/include/PlanOptimizer.h', 'RtModel/include/Prescription.h').
includes('RtModel/include/PlanOptimizer.h', 'RtModel/include/PlanPyramid.h').
includes('RtModel/include/PlanOptimizer.h', 'RtModel/include/ConjGradOptimizer.h').
includes('RtModel/include/PlanPyramid.h', 'RtModel/include/Plan.h').
includes('Brimstone/BrimstoneDoc.h', 'RtModel/include/Series.h').
includes('Brimstone/BrimstoneDoc.h', 'RtModel/include/Plan.h').
includes('Brimstone/SeriesDicomImporter.h', 'RtModel/include/Series.h').


%% =============================================================================
%% DERIVED PREDICATES
%% =============================================================================

%% Inheritance
inherits(Child, Parent) :-
    class(Child, _, Parent, _, _),
    Parent \= none.

%% Definition location
defined_in(Class, File) :- class(Class, _, _, File, _).

%% Method membership
has_method(Class, Method) :- method(Class, Method, _, _, _, _).

%% Transitive dependency
depends_on(A, B) :- includes(A, B).
depends_on(A, C) :- includes(A, B), depends_on(B, C).

%% Implementation file for class
implementation_of(Class, ImplFile) :-
    class(Class, _, _, HeaderFile, _),
    atom_string(HeaderFile, HeaderStr),
    ( sub_string(HeaderStr, Before, 2, 0, ".h")
    -> sub_string(HeaderStr, 0, Before, _, Base),
       ( sub_string(Base, _, _, _, "/include/")
       -> sub_string(Base, 0, _, _, "RtModel/include/"),
          sub_string(Base, 16, _, 0, Name),
          atom_concat('RtModel/', Name, ImplBase),
          atom_concat(ImplBase, '.cpp', ImplFile)
       ; atom_concat(Base, '.cpp', ImplFile)
       )
    ; ImplFile = unknown
    ).
