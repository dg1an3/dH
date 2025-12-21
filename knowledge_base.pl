%% =============================================================================
%% BRIMSTONE - Prolog Knowledge Base
%% =============================================================================
%% A semantic representation of the brimstone radiotherapy planning system.
%% Inspired by Dublin Core, FOAF, IAO (Information Artifact Ontology),
%% SWO (Software Ontology), and OBI (Ontology for Biomedical Investigations).
%%
%% Copyright (c) 2007-2021, Derek G. Lane. All rights reserved.
%% U.S. Patent 7,369,645
%% =============================================================================

:- module(brimstone_kb, [
    % Query predicates
    source_file/1,
    class/1,
    function/1,
    inherits/2,
    defined_in/2,
    has_method/2,
    depends_on/2,
    % DCG exports
    dicom_file//1,
    dicom_element//1
]).

%% =============================================================================
%% ONTOLOGY NAMESPACE DECLARATIONS
%% =============================================================================
%% Inspired by standard ontologies for semantic interoperability

% Dublin Core Terms (dc:, dcterms:)
% - dc:title, dc:creator, dc:description, dc:date, dc:format, dc:type
% - dcterms:license, dcterms:isPartOf, dcterms:hasPart

% FOAF (Friend of a Friend)
% - foaf:Project, foaf:Document, foaf:Organization

% IAO (Information Artifact Ontology)
% - iao:InformationContentEntity, iao:SoftwareModule, iao:DataFormat
% - iao:is_about, iao:denotes, iao:has_part

% SWO (Software Ontology)
% - swo:SoftwareApplication, swo:SourceCode, swo:Function
% - swo:has_input, swo:has_output, swo:implements

% OBI (Ontology for Biomedical Investigations)
% - obi:Algorithm, obi:DataTransformation, obi:MeasurementDatum


%% =============================================================================
%% PROJECT METADATA (Dublin Core inspired)
%% =============================================================================

%% project(+Name, +Properties)
%% Properties: list of key-value pairs following Dublin Core
project(brimstone, [
    title('Brimstone Radiotherapy Planning System'),
    creator('Derek G. Lane'),
    description('Variational inverse planning algorithm for radiotherapy treatment planning using KL divergence minimization'),
    date(2007-2021),
    license('Proprietary - U.S. Patent 7,369,645'),
    type(software_application),
    subject([radiotherapy, optimization, variational_bayes, dose_calculation]),
    language([cpp, python]),
    format([visual_studio_solution, python_package])
]).


%% =============================================================================
%% MODULE DEFINITIONS (IAO/SWO inspired)
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
module_dependency(python_utils, rtmodel).  % via bindings


%% =============================================================================
%% SOURCE FILE DEFINITIONS (Dublin Core / IAO inspired)
%% =============================================================================

%% source_file(+Path, +Module, +Type, +LineCount, +Description)
%% Type: header | implementation | resource

% RtModel Headers
source_file('RtModel/include/Plan.h', rtmodel, header, 150, 'Treatment plan container with beams and dose').
source_file('RtModel/include/Series.h', rtmodel, header, 80, 'CT density volume and structures container').
source_file('RtModel/include/Structure.h', rtmodel, header, 180, 'ROI with multi-scale contours').
source_file('RtModel/include/Beam.h', rtmodel, header, 120, 'Treatment beam with beamlets and intensity').
source_file('RtModel/include/Prescription.h', rtmodel, header, 200, 'Objective function coordinating VOITerms').
source_file('RtModel/include/VOITerm.h', rtmodel, header, 80, 'Base class for structure constraints').
source_file('RtModel/include/KLDivTerm.h', rtmodel, header, 100, 'KL divergence DVH matching term').
source_file('RtModel/include/ConjGradOptimizer.h', rtmodel, header, 120, 'Conjugate gradient with adaptive variance').
source_file('RtModel/include/PlanOptimizer.h', rtmodel, header, 100, 'Multi-level optimization orchestrator').
source_file('RtModel/include/PlanPyramid.h', rtmodel, header, 80, 'Multi-scale pyramid management').
source_file('RtModel/include/Histogram.h', rtmodel, header, 200, 'DVH with adaptive binning').
source_file('RtModel/include/HistogramGradient.h', rtmodel, header, 150, 'DVH with partial derivatives').
source_file('RtModel/include/BeamDoseCalc.h', rtmodel, header, 100, 'Pencil beam TERMA calculation').
source_file('RtModel/include/SphereConvolve.h', rtmodel, header, 90, 'Spherical coordinate convolution').
source_file('RtModel/include/EnergyDepKernel.h', rtmodel, header, 120, 'Energy deposition kernel LUTs').
source_file('RtModel/include/ObjectiveFunction.h', rtmodel, header, 80, 'Base cost function with variance').
source_file('RtModel/include/PlanXmlFile.h', rtmodel, header, 60, 'Plan XML serialization').
source_file('RtModel/include/ItkUtils.h', rtmodel, header, 100, 'ITK utility functions and types').
source_file('RtModel/include/VectorN.h', rtmodel, header, 150, 'N-dimensional vector operations').
source_file('RtModel/include/MatrixNxM.h', rtmodel, header, 200, 'NxM matrix operations').
source_file('RtModel/include/MathUtil.h', rtmodel, header, 80, 'Mathematical utilities').
source_file('RtModel/include/VectorOps.h', rtmodel, header, 60, 'Vector operation helpers').
source_file('RtModel/include/utilmacros.h', rtmodel, header, 40, 'Utility macros').

% RtModel Implementations
source_file('RtModel/Plan.cpp', rtmodel, implementation, 294, 'Plan class implementation').
source_file('RtModel/Series.cpp', rtmodel, implementation, 85, 'Series class implementation').
source_file('RtModel/Structure.cpp', rtmodel, implementation, 295, 'Structure class implementation').
source_file('RtModel/Beam.cpp', rtmodel, implementation, 198, 'Beam class implementation').
source_file('RtModel/Prescription.cpp', rtmodel, implementation, 654, 'Prescription objective function').
source_file('RtModel/VOITerm.cpp', rtmodel, implementation, 41, 'VOITerm base implementation').
source_file('RtModel/KLDivTerm.cpp', rtmodel, implementation, 490, 'KL divergence term implementation').
source_file('RtModel/ConjGradOptimizer.cpp', rtmodel, implementation, 367, 'CG optimizer with free energy').
source_file('RtModel/PlanOptimizer.cpp', rtmodel, implementation, 391, 'Multi-level optimization').
source_file('RtModel/PlanPyramid.cpp', rtmodel, implementation, 267, 'Pyramid management').
source_file('RtModel/Histogram.cpp', rtmodel, implementation, 806, 'Histogram binning and convolution').
source_file('RtModel/HistogramGradient.cpp', rtmodel, implementation, 557, 'Gradient histogram computation').
source_file('RtModel/BeamDoseCalc.cpp', rtmodel, implementation, 398, 'TERMA ray-tracing').
source_file('RtModel/SphereConvolve.cpp', rtmodel, implementation, 347, 'Energy kernel convolution').
source_file('RtModel/EnergyDepKernel.cpp', rtmodel, implementation, 528, 'Kernel loading and lookup').
source_file('RtModel/ObjectiveFunction.cpp', rtmodel, implementation, 148, 'Cost function base').
source_file('RtModel/PlanXmlFile.cpp', rtmodel, implementation, 457, 'XML plan I/O').

% Brimstone Application Headers
source_file('Brimstone/Brimstone.h', brimstone_app, header, 50, 'Application class').
source_file('Brimstone/BrimstoneDoc.h', brimstone_app, header, 80, 'Document class - owns Series/Plan').
source_file('Brimstone/BrimstoneView.h', brimstone_app, header, 120, 'View class - renders CT/dose').
source_file('Brimstone/MainFrm.h', brimstone_app, header, 40, 'Main window frame').
source_file('Brimstone/OptThread.h', brimstone_app, header, 80, 'Async optimization thread').
source_file('Brimstone/OptimizerDashboard.h', brimstone_app, header, 60, 'Optimization progress dialog').
source_file('Brimstone/SeriesDicomImporter.h', brimstone_app, header, 100, 'DICOM CT import pipeline').
source_file('Brimstone/PlanSetupDlg.h', brimstone_app, header, 40, 'Plan configuration dialog').
source_file('Brimstone/PrescDlg.h', brimstone_app, header, 40, 'Prescription dialog').
source_file('Brimstone/PrescriptionBar.h', brimstone_app, header, 30, 'Prescription display bar').
source_file('Brimstone/PrescriptionToolbar.h', brimstone_app, header, 60, 'Dynamic constraint toolbar').
source_file('Brimstone/PlanarView.h', brimstone_app, header, 100, '2D image viewer').

% Brimstone Application Implementations
source_file('Brimstone/Brimstone.cpp', brimstone_app, implementation, 154, 'App initialization').
source_file('Brimstone/BrimstoneDoc.cpp', brimstone_app, implementation, 274, 'Document serialization').
source_file('Brimstone/BrimstoneView.cpp', brimstone_app, implementation, 459, 'View rendering').
source_file('Brimstone/MainFrm.cpp', brimstone_app, implementation, 112, 'Frame window').
source_file('Brimstone/OptThread.cpp', brimstone_app, implementation, 138, 'Optimization worker').
source_file('Brimstone/OptimizerDashboard.cpp', brimstone_app, implementation, 293, 'Progress visualization').
source_file('Brimstone/SeriesDicomImporter.cpp', brimstone_app, implementation, 400, 'DICOM parsing').
source_file('Brimstone/PlanarView.cpp', brimstone_app, implementation, 1089, '2D image rendering').
source_file('Brimstone/PlanSetupDlg.cpp', brimstone_app, implementation, 157, 'Plan setup UI').
source_file('Brimstone/PrescDlg.cpp', brimstone_app, implementation, 91, 'Prescription UI').
source_file('Brimstone/PrescriptionToolbar.cpp', brimstone_app, implementation, 426, 'Toolbar implementation').

% Graph Library
source_file('Graph/include/Graph.h', graph, header, 150, 'Graph plotting window').
source_file('Graph/include/DataSeries.h', graph, header, 80, 'Generic data series').
source_file('Graph/include/HistogramDataSeries.h', graph, header, 50, 'Histogram-based series').
source_file('Graph/include/TargetDVHSeries.h', graph, header, 50, 'Target DVH series').
source_file('Graph/include/Dib.h', graph, header, 60, 'Device-independent bitmap').
source_file('Graph/Graph.cpp', graph, implementation, 635, 'Graph rendering').
source_file('Graph/DataSeries.cpp', graph, implementation, 117, 'Data series base').
source_file('Graph/HistogramDataSeries.cpp', graph, implementation, 88, 'Histogram series').
source_file('Graph/TargetDVHSeries.cpp', graph, implementation, 86, 'Target DVH').
source_file('Graph/Dib.cpp', graph, implementation, 258, 'Bitmap handling').

% Python Utilities
source_file('python/read_process_series.py', python_utils, implementation, 200, 'CT density processing').
source_file('python/optimize_with_lbfgs.py', python_utils, implementation, 150, 'L-BFGS-B optimization').
source_file('python/setup.py', python_utils, implementation, 50, 'Python package setup').
source_file('python/learned_hessian_entropy.py', python_utils, implementation, 100, 'Learned Hessian entropy').
source_file('python/learned_optimizer.py', python_utils, implementation, 100, 'Learned optimizer').
source_file('python/workflow_example.py', python_utils, implementation, 80, 'Example workflow').

%% Convenience predicate
source_file(Path) :- source_file(Path, _, _, _, _).


%% =============================================================================
%% CLASS DEFINITIONS (SWO / OBI inspired)
%% =============================================================================

%% class(+Name, +Namespace, +ParentClass, +DefinedIn, +Description)
%% Namespace: 'dH' for core, 'mfc' for GUI, 'graph' for visualization

% Core Data Model Classes
class('Series', 'dH', 'itk::DataObject', 'RtModel/include/Series.h',
    'CT density volume container with associated anatomical structures').
class('Structure', 'dH', 'itk::DataObject', 'RtModel/include/Structure.h',
    'Region of interest (ROI) with multi-scale contours and region volumes').
class('Beam', 'dH', 'itk::DataObject', 'RtModel/include/Beam.h',
    'Treatment beam with pencil beamlets and intensity map').
class('Plan', 'dH', 'itk::DataObject', 'RtModel/include/Plan.h',
    'Treatment plan containing beams, dose matrix, and DVH histograms').

% Optimization Classes
class('DynamicCovarianceCostFunction', 'dH', 'vnl_cost_function',
    'RtModel/include/ObjectiveFunction.h',
    'Base cost function with adaptive variance support').
class('Prescription', 'dH', 'DynamicCovarianceCostFunction',
    'RtModel/include/Prescription.h',
    'Objective function coordinating weighted sum of VOI terms').
class('VOITerm', 'dH', 'itk::DataObject', 'RtModel/include/VOITerm.h',
    'Base class for structure-specific optimization constraints').
class('KLDivTerm', 'dH', 'VOITerm', 'RtModel/include/KLDivTerm.h',
    'KL divergence term matching calculated DVH to target').
class('DynamicCovarianceOptimizer', 'dH', 'vnl_nonlinear_minimizer',
    'RtModel/include/ConjGradOptimizer.h',
    'Conjugate gradient optimizer with adaptive variance and free energy').
class('PlanOptimizer', 'dH', none, 'RtModel/include/PlanOptimizer.h',
    'Multi-level optimization orchestrator across pyramid scales').
class('PlanPyramid', 'dH', none, 'RtModel/include/PlanPyramid.h',
    'Four-level multi-scale pyramid management (8mm to 0.5mm voxels)').

% Histogram Classes
class('CHistogram', 'dH', 'itk::DataObject', 'RtModel/include/Histogram.h',
    'Dose-volume histogram with adaptive Gaussian binning').
class('CHistogramWithGradient', 'dH', 'CHistogram',
    'RtModel/include/HistogramGradient.h',
    'DVH with partial derivative volumes for gradient computation').

% Dose Calculation Classes
class('CBeamDoseCalc', 'dH', none, 'RtModel/include/BeamDoseCalc.h',
    'Pencil beam TERMA calculation via ray-tracing').
class('CEnergyDepKernel', 'dH', none, 'RtModel/include/EnergyDepKernel.h',
    'Pre-computed energy deposition kernel lookup tables').
class('SphereConvolve', 'dH', none, 'RtModel/include/SphereConvolve.h',
    'Spherical coordinate energy kernel convolution').

% GUI Classes (MFC)
class('CBrimstoneApp', 'mfc', 'CWinApp', 'Brimstone/Brimstone.h',
    'Main application class - initialization and message loop').
class('CBrimstoneDoc', 'mfc', 'CDocument', 'Brimstone/BrimstoneDoc.h',
    'Document class owning Series and Plan objects').
class('CBrimstoneView', 'mfc', 'CView', 'Brimstone/BrimstoneView.h',
    'View class rendering CT images, contours, and dose overlays').
class('CMainFrame', 'mfc', 'CFrameWnd', 'Brimstone/MainFrm.h',
    'Main window frame with toolbars and status bar').
class('COptThread', 'mfc', 'CWinThread', 'Brimstone/OptThread.h',
    'Worker thread for asynchronous optimization').
class('COptimizerDashboard', 'mfc', 'CDialog', 'Brimstone/OptimizerDashboard.h',
    'Real-time optimization progress visualization dialog').
class('CSeriesDicomImporter', 'mfc', none, 'Brimstone/SeriesDicomImporter.h',
    'DICOM CT import pipeline using DCMTK').
class('CPlanarView', 'mfc', none, 'Brimstone/PlanarView.h',
    '2D planar image viewer for CT slices').
class('CPrescDlg', 'mfc', 'CDialog', 'Brimstone/PrescDlg.h',
    'Prescription constraint setup dialog').
class('CPlanSetupDlg', 'mfc', 'CDialog', 'Brimstone/PlanSetupDlg.h',
    'Plan configuration dialog').
class('CPrescriptionToolbar', 'mfc', none, 'Brimstone/PrescriptionToolbar.h',
    'Dynamic toolbar for prescription constraints').

% Graph/Visualization Classes
class('CGraph', 'graph', 'CWnd', 'Graph/include/Graph.h',
    'Graph plotting window for DVH visualization').
class('CDataSeries', 'graph', 'itk::DataObject', 'Graph/include/DataSeries.h',
    'Generic data series for plotting').
class('CHistogramDataSeries', 'graph', 'CDataSeries',
    'Graph/include/HistogramDataSeries.h',
    'Data series derived from histogram data').
class('CTargetDVHSeries', 'graph', 'CDataSeries',
    'Graph/include/TargetDVHSeries.h',
    'Target DVH constraint visualization').
class('CDib', 'graph', none, 'Graph/include/Dib.h',
    'Device-independent bitmap for rendering').

%% Convenience predicates
class(Name) :- class(Name, _, _, _, _).

inherits(Child, Parent) :-
    class(Child, _, Parent, _, _),
    Parent \= none.

defined_in(Class, File) :- class(Class, _, _, File, _).


%% =============================================================================
%% METHOD/FUNCTION DEFINITIONS (SWO inspired)
%% =============================================================================

%% method(+ClassName, +MethodName, +ReturnType, +Parameters, +Visibility, +Description)
%% Parameters: list of param(Name, Type)

% Series methods
method('Series', 'GetDensity', 'VolumeReal*', [], public,
    'Get CT density volume').
method('Series', 'SetDensity', void, [param(pValue, 'VolumeReal*')], public,
    'Set CT density volume').
method('Series', 'GetStructureCount', int, [], public,
    'Get number of structures').
method('Series', 'GetStructureAt', 'Structure*', [param(nAt, int)], public,
    'Get structure by index').
method('Series', 'GetStructureFromName', 'Structure*',
    [param(strName, 'const std::string&')], public,
    'Find structure by name').
method('Series', 'AddStructure', void, [param(pStruct, 'Structure*')], public,
    'Add a structure to the series').

% Structure methods
method('Structure', 'GetName', 'const std::string&', [], public,
    'Get structure name').
method('Structure', 'SetName', void, [param(strName, 'const std::string&')], public,
    'Set structure name').
method('Structure', 'GetType', 'StructType', [], public,
    'Get structure type (TARGET/OAR)').
method('Structure', 'SetType', void, [param(type, 'StructType')], public,
    'Set structure type').
method('Structure', 'GetContourCount', int, [], public,
    'Get number of contour slices').
method('Structure', 'GetContour', 'PolygonType*', [param(nAt, int)], public,
    'Get contour polygon at index').
method('Structure', 'AddContour', void,
    [param(pPoly, 'PolygonType::Pointer'), param(refDist, 'REAL')], public,
    'Add contour at reference distance').
method('Structure', 'GetRegion', 'const VolumeReal*', [param(nLevel, int)], public,
    'Get region mask at pyramid level').
method('Structure', 'CalcRegion', void, [], private,
    'Calculate region from contours').

% Plan methods
method('Plan', 'GetSeries', 'Series*', [], public, 'Get associated series').
method('Plan', 'SetSeries', void, [param(pSeries, 'Series*')], public, 'Set series').
method('Plan', 'GetBeamCount', int, [], public, 'Get number of beams').
method('Plan', 'GetBeamAt', 'CBeam*', [param(nAt, int)], public, 'Get beam by index').
method('Plan', 'AddBeam', int, [param(pBeam, 'CBeam*')], public, 'Add a beam').
method('Plan', 'GetHistogram', 'CHistogram*',
    [param(pStruct, 'Structure*'), param(bCreate, bool)], public,
    'Get or create histogram for structure').
method('Plan', 'GetDoseMatrix', 'VolumeReal*', [], public, 'Get dose volume').
method('Plan', 'GetTotalBeamletCount', int, [], public, 'Total beamlets across beams').
method('Plan', 'UpdateAllHisto', void, [], public, 'Update all histograms').

% Beam methods
method('Beam', 'GetGantryAngle', double, [], public, 'Get gantry angle in degrees').
method('Beam', 'SetGantryAngle', void, [param(angle, double)], public, 'Set gantry angle').
method('Beam', 'GetIsocenter', 'itk::Vector<REAL>', [], public, 'Get isocenter position').
method('Beam', 'SetIsocenter', void, [param(iso, 'itk::Vector<REAL>')], public, 'Set isocenter').
method('Beam', 'GetBeamletCount', int, [], public, 'Get number of beamlets').
method('Beam', 'GetBeamlet', 'VolumeReal*', [param(nShift, int)], public, 'Get beamlet dose').
method('Beam', 'GetIntensityMap', 'IntensityMap*', [], public, 'Get intensity weights').
method('Beam', 'SetIntensityMap', void, [param(vWeights, 'const CVectorN<>&')], public,
    'Set intensity weights from vector').
method('Beam', 'OnIntensityMapChanged', void, [], public, 'Recalculate dose on change').

% Prescription methods
method('Prescription', 'operator()', 'REAL',
    [param(vInput, 'const CVectorN<>&'), param(pGrad, 'CVectorN<>*')], public,
    'Evaluate objective function and gradient').
method('Prescription', 'GetStructureTerm', 'VOITerm*',
    [param(pStruct, 'Structure*')], public, 'Get term for structure').
method('Prescription', 'AddStructureTerm', void,
    [param(pST, 'VOITerm*')], public, 'Add structure term').
method('Prescription', 'Transform', void,
    [param(pvInOut, 'CVectorN<>*')], public, 'Apply sigmoid transform').
method('Prescription', 'dTransform', void,
    [param(pvInOut, 'CVectorN<>*')], public, 'Sigmoid derivative').
method('Prescription', 'InvTransform', void,
    [param(pvInOut, 'CVectorN<>*')], public, 'Inverse sigmoid').
method('Prescription', 'CalcSumSigmoid', void,
    [param(pHisto, 'CHistogramWithGradient*'), param(vInput, 'const CVectorN<>&'),
     param(vInputTrans, 'const CVectorN<>&'), param(arrInclude, 'const CArray<BOOL,BOOL>&')],
    public, 'Calculate weighted sigmoid sum for histogram').

% VOITerm methods
method('VOITerm', 'Eval', 'REAL',
    [param(pvGrad, 'CVectorN<>*'), param(arrInclude, 'const CArray<BOOL,BOOL>&')],
    public_virtual, 'Evaluate term cost and gradient').
method('VOITerm', 'Clone', 'VOITerm*', [], public_virtual, 'Clone the term').
method('VOITerm', 'GetVOI', 'Structure*', [], public, 'Get associated structure').
method('VOITerm', 'GetWeight', 'REAL', [], public, 'Get term weight').
method('VOITerm', 'SetWeight', void, [param(w, 'REAL')], public, 'Set term weight').

% KLDivTerm methods
method('KLDivTerm', 'SetDVPs', void,
    [param(mDVP, 'const CMatrixNxM<>&')], public, 'Set dose-volume points').
method('KLDivTerm', 'GetDVPs', 'const CMatrixNxM<>&', [], public, 'Get dose-volume points').
method('KLDivTerm', 'SetInterval', void,
    [param(low, 'REAL'), param(high, 'REAL'), param(frac, 'REAL'), param(bMid, 'BOOL')],
    public, 'Set dose interval constraint').
method('KLDivTerm', 'GetMinDose', 'REAL', [], public, 'Get minimum target dose').
method('KLDivTerm', 'GetMaxDose', 'REAL', [], public, 'Get maximum target dose').
method('KLDivTerm', 'OnHistogramBinningChange', void, [], public,
    'Recalculate target bins on binning change').

% DynamicCovarianceOptimizer methods
method('DynamicCovarianceOptimizer', 'minimize', 'ReturnCodes',
    [param(vInit, 'vnl_vector<REAL>&')], public, 'Run conjugate gradient optimization').
method('DynamicCovarianceOptimizer', 'SetAdaptiveVariance', void,
    [param(bCalcVar, bool), param(varMin, 'REAL'), param(varMax, 'REAL')], public,
    'Enable adaptive variance with min/max bounds').
method('DynamicCovarianceOptimizer', 'SetComputeFreeEnergy', void,
    [param(bCompute, bool)], public, 'Enable explicit free energy computation').
method('DynamicCovarianceOptimizer', 'GetEntropy', 'REAL', [], public,
    'Get computed entropy H[q]').
method('DynamicCovarianceOptimizer', 'GetFreeEnergy', 'REAL', [], public,
    'Get computed free energy F = KL - H').
method('DynamicCovarianceOptimizer', 'SetCallback', void,
    [param(pCallback, 'OptimizerCallback*'), param(pParam, 'void*')], public,
    'Set iteration callback').
method('DynamicCovarianceOptimizer', 'ComputeEntropyFromCovariance', 'REAL',
    [param(covar, 'const vnl_matrix<REAL>&')], private,
    'Compute entropy from covariance matrix').

% PlanOptimizer methods
method('PlanOptimizer', 'Optimize', bool,
    [param(vInit, 'CVectorN<>&'), param(pFunc, 'OptimizerCallback*'),
     param(pParam, 'void*')], public,
    'Run multi-level optimization').
method('PlanOptimizer', 'GetPrescription', 'Prescription*',
    [param(nLevel, int)], public, 'Get prescription at pyramid level').
method('PlanOptimizer', 'GetOptimizer', 'DynamicCovarianceOptimizer*',
    [param(nLevel, int)], public, 'Get optimizer at pyramid level').
method('PlanOptimizer', 'GetStateVectorFromPlan', void,
    [param(vState, 'CVectorN<>&')], public, 'Extract weights to state vector').
method('PlanOptimizer', 'SetStateVectorToPlan', void,
    [param(vState, 'const CVectorN<>&')], public, 'Apply state vector to plan').
method('PlanOptimizer', 'InvFilterStateVector', void,
    [param(nScale, int), param(vIn, 'const CVectorN<>&'), param(vOut, 'CVectorN<>&')],
    public, 'Inverse filter for scale transfer').

% CHistogram methods
method('CHistogram', 'SetBinning', void,
    [param(min_val, 'REAL'), param(width, 'REAL'), param(sigma_mult, 'REAL')],
    public, 'Set histogram binning parameters').
method('CHistogram', 'GetBins', 'const CVectorN<>&', [], public, 'Get bin counts').
method('CHistogram', 'GetCumBins', 'const CVectorN<>&', [], public, 'Get cumulative DVH').
method('CHistogram', 'GetGBins', 'const CVectorN<>&', [], public, 'Get Gaussian-smoothed bins').
method('CHistogram', 'SetGBinVar', void,
    [param(pAV, 'CVectorN<>*'), param(varMin, 'REAL'), param(varMax, 'REAL')],
    public, 'Set Gaussian bin variance').
method('CHistogram', 'OnVolumeChange', void, [], public, 'Recalculate on volume change').

% CHistogramWithGradient methods
method('CHistogramWithGradient', 'Get_dVolumeCount', int, [], public,
    'Get number of partial derivative volumes').
method('CHistogramWithGradient', 'Get_dVolume', 'VolumeReal*',
    [param(nAt, int), param(pnGroup, 'int*')], public,
    'Get partial derivative volume').
method('CHistogramWithGradient', 'Add_dVolume', int,
    [param(p_dVol, 'VolumeReal*'), param(nGroup, int)], public,
    'Add partial derivative volume').
method('CHistogramWithGradient', 'Get_dBins', 'const CVectorN<>&',
    [param(nAt, int)], public, 'Get partial derivative bins').
method('CHistogramWithGradient', 'Get_dGBins', 'const CVectorN<>&',
    [param(nAt, int)], public, 'Get Gaussian-smoothed partial bins').

% CBeamDoseCalc methods
method('CBeamDoseCalc', 'InitCalcBeamlets', void, [], public,
    'Initialize beamlet calculation').
method('CBeamDoseCalc', 'CalcBeamlet', void, [param(nBeamlet, int)], public,
    'Calculate dose for single beamlet').
method('CBeamDoseCalc', 'CalcTerma', void, [], public, 'Calculate TERMA volume').
method('CBeamDoseCalc', 'TraceRayTerma', void,
    [param(vRay, 'Vector<REAL>'), param(fluence0, 'const REAL')], private,
    'Trace single ray through density').

% CEnergyDepKernel methods
method('CEnergyDepKernel', 'CalcSphereConvolve', 'VolumeReal::Pointer',
    [param(pDensity, 'VolumeReal*'), param(pTerma, 'VolumeReal*'), param(nSlice, int)],
    public, 'Compute spherical convolution for dose').
method('CEnergyDepKernel', 'LoadKernel', void, [], private, 'Load kernel from file').
method('CEnergyDepKernel', 'GetCumEnergy', double,
    [param(nPhi, int), param(radDist, double)], public,
    'Get cumulative energy at angle and distance').

%% Convenience predicate
function(Name) :- method(_, Name, _, _, _, _).
has_method(Class, Method) :- method(Class, Method, _, _, _, _).


%% =============================================================================
%% DATA TYPE DEFINITIONS
%% =============================================================================

%% datatype(+Name, +Category, +Description, +DefinedIn)
%% Category: primitive | typedef | enum | class_template | struct

% ITK Image Types
datatype('VolumeReal', typedef, '3D real-valued volume: itk::Image<REAL, 3>',
    'RtModel/include/ItkUtils.h').
datatype('VolumeShort', typedef, '3D short-valued volume: itk::Image<short, 3>',
    'RtModel/include/ItkUtils.h').
datatype('VolumeSliceReal', typedef, '2D real-valued slice: itk::Image<REAL, 2>',
    'RtModel/include/ItkUtils.h').
datatype('PolygonType', typedef, 'Contour polygon: itk::PolygonSpatialObject<2>',
    'RtModel/include/Structure.h').

% VNL Types
datatype('CVectorN', class_template, 'N-dimensional vector wrapper for vnl_vector<REAL>',
    'RtModel/include/VectorN.h').
datatype('CMatrixNxM', class_template, 'NxM matrix wrapper for vnl_matrix<REAL>',
    'RtModel/include/MatrixNxM.h').

% Numeric Types
datatype('REAL', typedef, 'Floating point type (double or float)',
    'RtModel/include/ItkUtils.h').
datatype('VOXEL_REAL', typedef, 'Voxel value type', 'RtModel/include/ItkUtils.h').

% Enumerations
datatype('StructType', enum, 'Structure classification: eNONE=0, eTARGET=1, eOAR=2',
    'RtModel/include/Structure.h').
datatype('ReturnCodes', enum, 'Optimizer return codes from vnl_nonlinear_minimizer',
    'RtModel/include/ConjGradOptimizer.h').

% Callback Types
datatype('OptimizerCallback', typedef,
    'Callback function: BOOL (*)(DynamicCovarianceOptimizer*, void*)',
    'RtModel/include/ConjGradOptimizer.h').

% MFC Collection Types
datatype('VOITermArray', typedef, 'std::vector<VOITerm::Pointer>',
    'RtModel/include/Prescription.h').
datatype('HistogramMap', typedef, 'CTypedPtrMap<CMapStringToOb, CString, CHistogram*>',
    'RtModel/include/Plan.h').

% Graph Types
datatype('GraphCoord', typedef, 'itk::Vector<REAL, 2> for 2D coordinates',
    'Graph/include/Graph.h').

%% enum_value(+EnumType, +Value, +IntValue, +Description)
enum_value('StructType', 'eNONE', 0, 'Unspecified structure type').
enum_value('StructType', 'eTARGET', 1, 'Planning target volume').
enum_value('StructType', 'eOAR', 2, 'Organ at risk').


%% =============================================================================
%% CONSTANTS AND CONFIGURATION
%% =============================================================================

%% constant(+Name, +Value, +Type, +Description, +DefinedIn)

% Pyramid configuration
constant('MAX_SCALES', 4, int, 'Number of pyramid levels',
    'RtModel/include/PlanPyramid.h').
constant('DEFAULT_LEVELSIGMA', [8.0, 3.2, 1.3, 0.5], 'REAL[]',
    'Voxel sizes per pyramid level in mm', 'RtModel/include/PlanPyramid.h').

% Optimizer configuration
constant('ITER_MAX', 500, int, 'Maximum CG iterations',
    'RtModel/ConjGradOptimizer.cpp').
constant('ZEPS', 1.0e-10, 'REAL', 'Convergence tolerance',
    'RtModel/ConjGradOptimizer.cpp').
constant('SIGMOID_SCALE', 0.2, 'REAL', 'Sigmoid parameter scaling',
    'RtModel/Prescription.cpp').

% Histogram configuration
constant('GBINS_BUFFER', 8.0, 'REAL', 'Gaussian kernel width buffer',
    'RtModel/Histogram.cpp').

% Energy kernel configuration
constant('NUM_THETA', 2, int, 'Number of azimuth angles in kernel',
    'RtModel/EnergyDepKernel.cpp').
constant('NUM_RADIAL_STEPS', 64, int, 'Radial discretization steps',
    'RtModel/EnergyDepKernel.cpp').


%% =============================================================================
%% RELATIONSHIPS AND ASSOCIATIONS
%% =============================================================================

%% association(+ClassA, +Relation, +ClassB, +Cardinality, +Description)
%% Cardinality: '1:1' | '1:N' | 'N:1' | 'N:M'

% Ownership (composition)
association('Plan', owns, 'Series', '1:1', 'Plan references a single CT series').
association('Plan', owns, 'Beam', '1:N', 'Plan contains multiple beams').
association('Plan', owns, 'CHistogram', '1:N', 'Plan maintains histograms per structure').
association('Series', owns, 'Structure', '1:N', 'Series contains anatomical structures').
association('Beam', owns, 'VolumeReal', '1:N', 'Beam contains beamlet dose volumes').
association('Prescription', owns, 'VOITerm', '1:N', 'Prescription aggregates constraint terms').

% References (aggregation)
association('VOITerm', references, 'Structure', '1:1', 'Term constrains a structure').
association('VOITerm', references, 'CHistogramWithGradient', '1:1', 'Term uses gradient histogram').
association('PlanOptimizer', references, 'Plan', '1:1', 'Optimizer operates on plan').
association('PlanOptimizer', references, 'PlanPyramid', '1:1', 'Optimizer uses multi-scale pyramid').
association('CBeamDoseCalc', references, 'Beam', '1:1', 'Calculator for specific beam').
association('CBeamDoseCalc', references, 'CEnergyDepKernel', '1:1', 'Uses energy kernel').

% MFC Document-View
association('CBrimstoneDoc', owns, 'Series', '1:1', 'Document owns series data').
association('CBrimstoneDoc', owns, 'Plan', '1:1', 'Document owns treatment plan').
association('CBrimstoneDoc', owns, 'PlanOptimizer', '1:1', 'Document owns optimizer').
association('CBrimstoneView', displays, 'CGraph', '1:N', 'View contains graph controls').
association('CBrimstoneView', uses, 'CPlanarView', '1:1', 'View embeds planar viewer').

% Graph associations
association('CGraph', contains, 'CDataSeries', '1:N', 'Graph displays data series').
association('CHistogramDataSeries', wraps, 'CHistogram', '1:1', 'Series wraps histogram data').


%% =============================================================================
%% FILE DEPENDENCIES
%% =============================================================================

%% includes(+SourceFile, +HeaderFile)

% Core internal dependencies
includes('RtModel/include/Plan.h', 'RtModel/include/Series.h').
includes('RtModel/include/Plan.h', 'RtModel/include/Beam.h').
includes('RtModel/include/Plan.h', 'RtModel/include/Histogram.h').
includes('RtModel/include/Series.h', 'RtModel/include/Structure.h').
includes('RtModel/include/Structure.h', 'RtModel/include/ItkUtils.h').
includes('RtModel/include/Beam.h', 'RtModel/include/ItkUtils.h').
includes('RtModel/include/Prescription.h', 'RtModel/include/ObjectiveFunction.h').
includes('RtModel/include/Prescription.h', 'RtModel/include/Histogram.h').
includes('RtModel/include/Prescription.h', 'RtModel/include/KLDivTerm.h').
includes('RtModel/include/Prescription.h', 'RtModel/include/Structure.h').
includes('RtModel/include/Prescription.h', 'RtModel/include/Plan.h').
includes('RtModel/include/VOITerm.h', 'RtModel/include/HistogramGradient.h').
includes('RtModel/include/KLDivTerm.h', 'RtModel/include/VOITerm.h').
includes('RtModel/include/ConjGradOptimizer.h', 'RtModel/include/ObjectiveFunction.h').
includes('RtModel/include/PlanOptimizer.h', 'RtModel/include/Prescription.h').
includes('RtModel/include/PlanOptimizer.h', 'RtModel/include/PlanPyramid.h').
includes('RtModel/include/PlanOptimizer.h', 'RtModel/include/ConjGradOptimizer.h').
includes('RtModel/include/PlanPyramid.h', 'RtModel/include/Plan.h').
includes('RtModel/include/HistogramGradient.h', 'RtModel/include/Histogram.h').
includes('RtModel/include/Histogram.h', 'RtModel/include/VectorN.h').
includes('RtModel/include/Histogram.h', 'RtModel/include/ItkUtils.h').
includes('RtModel/include/EnergyDepKernel.h', 'RtModel/include/VectorN.h').
includes('RtModel/include/EnergyDepKernel.h', 'RtModel/include/MatrixNxM.h').

% GUI dependencies
includes('Brimstone/BrimstoneDoc.h', 'RtModel/include/Series.h').
includes('Brimstone/BrimstoneDoc.h', 'RtModel/include/Plan.h').
includes('Brimstone/BrimstoneDoc.h', 'RtModel/include/PlanOptimizer.h').
includes('Brimstone/BrimstoneView.h', 'Graph/include/Graph.h').
includes('Brimstone/BrimstoneView.h', 'Brimstone/PlanarView.h').
includes('Brimstone/BrimstoneView.h', 'Brimstone/OptThread.h').
includes('Brimstone/OptThread.h', 'RtModel/include/ConjGradOptimizer.h').

% Graph dependencies
includes('Graph/include/Graph.h', 'RtModel/include/MatrixNxM.h').
includes('Graph/include/Graph.h', 'Graph/include/Dib.h').
includes('Graph/include/Graph.h', 'Graph/include/DataSeries.h').
includes('Graph/include/DataSeries.h', 'RtModel/include/MatrixNxM.h').
includes('Graph/include/HistogramDataSeries.h', 'RtModel/include/Histogram.h').
includes('Graph/include/HistogramDataSeries.h', 'Graph/include/DataSeries.h').

%% Transitive dependency
depends_on(A, B) :- includes(A, B).
depends_on(A, C) :- includes(A, B), depends_on(B, C).


%% =============================================================================
%% ALGORITHMS AND PROCESSES (OBI inspired)
%% =============================================================================

%% algorithm(+Name, +Type, +Description, +Implements, +DefinedIn)

algorithm(conjugate_gradient, optimization,
    'Conjugate gradient with Polak-Ribiere update and adaptive variance',
    'DynamicCovarianceOptimizer::minimize',
    'RtModel/ConjGradOptimizer.cpp').

algorithm(kl_divergence, objective_function,
    'KL(P_calc || P_target) divergence between calculated and target DVH',
    'KLDivTerm::Eval',
    'RtModel/KLDivTerm.cpp').

algorithm(pencil_beam_terma, dose_calculation,
    'Ray-tracing through CT density for TERMA computation',
    'CBeamDoseCalc::CalcTerma',
    'RtModel/BeamDoseCalc.cpp').

algorithm(spherical_convolution, dose_calculation,
    'Energy deposition kernel convolution in spherical coordinates',
    'CEnergyDepKernel::CalcSphereConvolve',
    'RtModel/SphereConvolve.cpp').

algorithm(gaussian_histogram_smoothing, signal_processing,
    'Adaptive Gaussian convolution for histogram smoothing',
    'CHistogram::ConvGauss',
    'RtModel/Histogram.cpp').

algorithm(multi_scale_pyramid, optimization_strategy,
    'Coarse-to-fine optimization across 4 resolution levels',
    'PlanOptimizer::Optimize',
    'RtModel/PlanOptimizer.cpp').

algorithm(sigmoid_transform, parameter_mapping,
    'Maps unbounded optimizer variables to bounded beamlet weights',
    'Prescription::Transform',
    'RtModel/Prescription.cpp').

algorithm(free_energy, variational_inference,
    'F = KL_divergence - Entropy; entropy from covariance determinant',
    'DynamicCovarianceOptimizer::ComputeEntropyFromCovariance',
    'RtModel/ConjGradOptimizer.cpp').


%% =============================================================================
%% DATA FORMATS (IAO inspired)
%% =============================================================================

%% data_format(+Name, +Category, +Extension, +Description, +ReaderClass)

data_format(dicom_ct, medical_imaging, '.dcm',
    'DICOM CT image series with Hounsfield units',
    'CSeriesDicomImporter').

data_format(dicom_rtstruct, medical_imaging, '.dcm',
    'DICOM RT Structure Set with ROI contours',
    'CSeriesDicomImporter').

data_format(energy_kernel, physics_data, '.dat',
    'Pre-computed energy deposition kernel lookup table',
    'CEnergyDepKernel').

data_format(plan_xml, serialization, '.xml',
    'Treatment plan serialization in XML format',
    'PlanXmlFile').

data_format(brimstone_doc, serialization, '.brs',
    'Native Brimstone document format (MFC serialization)',
    'CBrimstoneDoc').

%% kernel_file(+Energy, +Filename, +Description)
kernel_file('2MV', 'Brimstone/2MV_kernel.dat', '2 MV photon beam kernel').
kernel_file('6MV', 'Brimstone/6MV_kernel.dat', '6 MV photon beam kernel').
kernel_file('15MV', 'Brimstone/15MV_kernel.dat', '15 MV photon beam kernel').


%% =============================================================================
%% DICOM DATA FORMAT - DCG (Definite Clause Grammar)
%% =============================================================================
%% Represents DICOM file structure as a formal grammar.
%% Reference: DICOM PS3.5 - Data Structures and Encoding

%% Top-level DICOM file structure
dicom_file(dicom(Preamble, Prefix, MetaInfo, Dataset)) -->
    dicom_preamble(Preamble),
    dicom_prefix(Prefix),
    dicom_meta_info(MetaInfo),
    dicom_dataset(Dataset).

%% 128-byte preamble (usually zeros)
dicom_preamble(preamble(Bytes)) -->
    { length(Bytes, 128) },
    Bytes.

%% DICM prefix marker
dicom_prefix(prefix("DICM")) --> [0'D, 0'I, 0'C, 0'M].

%% File Meta Information (Group 0002)
dicom_meta_info(meta_info(Elements)) -->
    dicom_elements(0x0002, Elements).

%% Main dataset (Groups > 0002)
dicom_dataset(dataset(Elements)) -->
    dicom_element_list(Elements).

%% List of data elements
dicom_element_list([]) --> [].
dicom_element_list([E|Es]) -->
    dicom_element(E),
    dicom_element_list(Es).

%% Single DICOM data element
dicom_element(element(Tag, VR, Value)) -->
    dicom_tag(Tag),
    dicom_vr(VR),
    dicom_value(VR, Value).

%% Tag: (group, element) pair - 4 bytes little-endian
dicom_tag(tag(Group, Element)) -->
    uint16_le(Group),
    uint16_le(Element).

%% Value Representation (2 bytes) with length handling
dicom_vr(vr(VR, Length)) -->
    vr_code(VR),
    vr_length(VR, Length).

%% Standard VR codes (subset of DICOM VRs)
vr_code('AE') --> [0'A, 0'E].  % Application Entity
vr_code('AS') --> [0'A, 0'S].  % Age String
vr_code('AT') --> [0'A, 0'T].  % Attribute Tag
vr_code('CS') --> [0'C, 0'S].  % Code String
vr_code('DA') --> [0'D, 0'A].  % Date
vr_code('DS') --> [0'D, 0'S].  % Decimal String
vr_code('DT') --> [0'D, 0'T].  % DateTime
vr_code('FL') --> [0'F, 0'L].  % Floating Point Single
vr_code('FD') --> [0'F, 0'D].  % Floating Point Double
vr_code('IS') --> [0'I, 0'S].  % Integer String
vr_code('LO') --> [0'L, 0'O].  % Long String
vr_code('LT') --> [0'L, 0'T].  % Long Text
vr_code('OB') --> [0'O, 0'B].  % Other Byte
vr_code('OD') --> [0'O, 0'D].  % Other Double
vr_code('OF') --> [0'O, 0'F].  % Other Float
vr_code('OW') --> [0'O, 0'W].  % Other Word
vr_code('PN') --> [0'P, 0'N].  % Person Name
vr_code('SH') --> [0'S, 0'H].  % Short String
vr_code('SL') --> [0'S, 0'L].  % Signed Long
vr_code('SQ') --> [0'S, 0'Q].  % Sequence
vr_code('SS') --> [0'S, 0'S].  % Signed Short
vr_code('ST') --> [0'S, 0'T].  % Short Text
vr_code('TM') --> [0'T, 0'M].  % Time
vr_code('UI') --> [0'U, 0'I].  % Unique Identifier
vr_code('UL') --> [0'U, 0'L].  % Unsigned Long
vr_code('UN') --> [0'U, 0'N].  % Unknown
vr_code('US') --> [0'U, 0'S].  % Unsigned Short
vr_code('UT') --> [0'U, 0'T].  % Unlimited Text

%% VR length handling (short form vs long form)
%% Short form: 2-byte length follows VR
vr_length(VR, length(Len)) -->
    { short_vr(VR) },
    uint16_le(Len).

%% Long form: 2 bytes reserved (00 00), then 4-byte length
vr_length(VR, length(Len)) -->
    { long_vr(VR) },
    [0x00, 0x00],
    uint32_le(Len).

%% VRs with short (2-byte) length encoding
short_vr('AE'). short_vr('AS'). short_vr('AT'). short_vr('CS').
short_vr('DA'). short_vr('DS'). short_vr('DT'). short_vr('FL').
short_vr('FD'). short_vr('IS'). short_vr('LO'). short_vr('LT').
short_vr('PN'). short_vr('SH'). short_vr('SL'). short_vr('SS').
short_vr('ST'). short_vr('TM'). short_vr('UI'). short_vr('UL').
short_vr('US').

%% VRs with long (4-byte) length encoding
long_vr('OB'). long_vr('OD'). long_vr('OF'). long_vr('OW').
long_vr('SQ'). long_vr('UC'). long_vr('UN'). long_vr('UR').
long_vr('UT').

%% Value parsing based on VR type
dicom_value(vr('US', length(2)), value(V)) --> uint16_le(V).
dicom_value(vr('SS', length(2)), value(V)) --> int16_le(V).
dicom_value(vr('UL', length(4)), value(V)) --> uint32_le(V).
dicom_value(vr('SL', length(4)), value(V)) --> int32_le(V).
dicom_value(vr('FL', length(4)), value(V)) --> float32_le(V).
dicom_value(vr('FD', length(8)), value(V)) --> float64_le(V).
dicom_value(vr('SQ', length(Len)), value(Items)) --> dicom_sequence(Len, Items).
dicom_value(vr(_, length(Len)), value(Bytes)) --> n_bytes(Len, Bytes).

%% Sequence of items (SQ VR)
dicom_sequence(0xFFFFFFFF, Items) -->  % Undefined length
    dicom_sequence_items(Items),
    sequence_delimitation_item.
dicom_sequence(Len, Items) -->
    { Len \= 0xFFFFFFFF },
    dicom_sequence_fixed(Len, Items).

dicom_sequence_items([]) --> [].
dicom_sequence_items([Item|Items]) -->
    dicom_item(Item),
    dicom_sequence_items(Items).

%% Single sequence item
dicom_item(item(Elements)) -->
    item_tag,
    item_length(Len),
    dicom_item_data(Len, Elements).

item_tag --> [0xFE, 0xFF, 0x00, 0xE0].  % (FFFE,E000)

item_length(Len) --> uint32_le(Len).

dicom_item_data(0xFFFFFFFF, Elements) -->  % Undefined length
    dicom_element_list(Elements),
    item_delimitation_tag.
dicom_item_data(Len, Elements) -->
    { Len \= 0xFFFFFFFF },
    n_bytes(Len, Bytes),
    { phrase(dicom_element_list(Elements), Bytes) }.

item_delimitation_tag --> [0xFE, 0xFF, 0x0D, 0xE0, 0x00, 0x00, 0x00, 0x00].
sequence_delimitation_item --> [0xFE, 0xFF, 0xDD, 0xE0, 0x00, 0x00, 0x00, 0x00].

%% Grouped elements by group number
dicom_elements(GroupNum, Elements) -->
    dicom_elements_in_group(GroupNum, Elements).

dicom_elements_in_group(_, []) --> [].
dicom_elements_in_group(GroupNum, [E|Es]) -->
    dicom_element(E),
    { E = element(tag(GroupNum, _), _, _) },
    dicom_elements_in_group(GroupNum, Es).

%% Binary primitives (little-endian)
uint16_le(V) --> [Lo, Hi], { V is Hi * 256 + Lo }.
int16_le(V) --> uint16_le(U), { U > 32767 -> V is U - 65536 ; V = U }.
uint32_le(V) --> [B0, B1, B2, B3],
    { V is B0 + B1*256 + B2*65536 + B3*16777216 }.
int32_le(V) --> uint32_le(U),
    { U > 2147483647 -> V is U - 4294967296 ; V = U }.

%% Floating point (IEEE 754) - simplified representation
float32_le(float32(Bytes)) --> n_bytes(4, Bytes).
float64_le(float64(Bytes)) --> n_bytes(8, Bytes).

%% N bytes helper
n_bytes(0, []) --> [].
n_bytes(N, [B|Bs]) --> { N > 0, N1 is N - 1 }, [B], n_bytes(N1, Bs).


%% =============================================================================
%% DICOM TAGS - Semantic Definitions
%% =============================================================================

%% dicom_tag_def(+Tag, +Name, +VR, +VM, +Description)
%% Tag format: tag(Group, Element)

% Patient Module
dicom_tag_def(tag(0x0010, 0x0010), 'PatientName', 'PN', '1', 'Patient name').
dicom_tag_def(tag(0x0010, 0x0020), 'PatientID', 'LO', '1', 'Patient identifier').
dicom_tag_def(tag(0x0010, 0x0030), 'PatientBirthDate', 'DA', '1', 'Birth date').
dicom_tag_def(tag(0x0010, 0x0040), 'PatientSex', 'CS', '1', 'Patient sex').

% Study Module
dicom_tag_def(tag(0x0008, 0x0020), 'StudyDate', 'DA', '1', 'Study date').
dicom_tag_def(tag(0x0008, 0x0030), 'StudyTime', 'TM', '1', 'Study time').
dicom_tag_def(tag(0x0020, 0x000D), 'StudyInstanceUID', 'UI', '1', 'Study UID').
dicom_tag_def(tag(0x0008, 0x1030), 'StudyDescription', 'LO', '1', 'Study description').

% Series Module
dicom_tag_def(tag(0x0020, 0x000E), 'SeriesInstanceUID', 'UI', '1', 'Series UID').
dicom_tag_def(tag(0x0008, 0x0060), 'Modality', 'CS', '1', 'Imaging modality').
dicom_tag_def(tag(0x0008, 0x103E), 'SeriesDescription', 'LO', '1', 'Series description').

% CT Image Module
dicom_tag_def(tag(0x0028, 0x0010), 'Rows', 'US', '1', 'Number of rows').
dicom_tag_def(tag(0x0028, 0x0011), 'Columns', 'US', '1', 'Number of columns').
dicom_tag_def(tag(0x0028, 0x0030), 'PixelSpacing', 'DS', '2', 'Pixel spacing [row, col]').
dicom_tag_def(tag(0x0028, 0x0100), 'BitsAllocated', 'US', '1', 'Bits per pixel').
dicom_tag_def(tag(0x0028, 0x0101), 'BitsStored', 'US', '1', 'Bits stored').
dicom_tag_def(tag(0x0028, 0x0102), 'HighBit', 'US', '1', 'High bit position').
dicom_tag_def(tag(0x0028, 0x0103), 'PixelRepresentation', 'US', '1', 'Signed/unsigned').
dicom_tag_def(tag(0x0028, 0x1050), 'WindowCenter', 'DS', '1-n', 'Window center').
dicom_tag_def(tag(0x0028, 0x1051), 'WindowWidth', 'DS', '1-n', 'Window width').
dicom_tag_def(tag(0x0028, 0x1052), 'RescaleIntercept', 'DS', '1', 'HU intercept').
dicom_tag_def(tag(0x0028, 0x1053), 'RescaleSlope', 'DS', '1', 'HU slope').
dicom_tag_def(tag(0x7FE0, 0x0010), 'PixelData', 'OW', '1', 'Image pixel data').

% Image Plane Module
dicom_tag_def(tag(0x0020, 0x0032), 'ImagePositionPatient', 'DS', '3', 'Image position (x,y,z)').
dicom_tag_def(tag(0x0020, 0x0037), 'ImageOrientationPatient', 'DS', '6', 'Row/column direction cosines').
dicom_tag_def(tag(0x0018, 0x0050), 'SliceThickness', 'DS', '1', 'Slice thickness in mm').
dicom_tag_def(tag(0x0020, 0x1041), 'SliceLocation', 'DS', '1', 'Slice location').

% RT Structure Set Module
dicom_tag_def(tag(0x3006, 0x0002), 'StructureSetLabel', 'SH', '1', 'Structure set name').
dicom_tag_def(tag(0x3006, 0x0020), 'StructureSetROISequence', 'SQ', '1', 'ROI definitions').
dicom_tag_def(tag(0x3006, 0x0022), 'ROINumber', 'IS', '1', 'ROI identifier').
dicom_tag_def(tag(0x3006, 0x0026), 'ROIName', 'LO', '1', 'ROI name').
dicom_tag_def(tag(0x3006, 0x0039), 'ROIContourSequence', 'SQ', '1', 'ROI contour data').
dicom_tag_def(tag(0x3006, 0x0040), 'ContourSequence', 'SQ', '1', 'Contour sequence').
dicom_tag_def(tag(0x3006, 0x0042), 'ContourGeometricType', 'CS', '1', 'CLOSED_PLANAR, etc.').
dicom_tag_def(tag(0x3006, 0x0046), 'NumberOfContourPoints', 'IS', '1', 'Point count').
dicom_tag_def(tag(0x3006, 0x0050), 'ContourData', 'DS', '3-3n', 'Contour point coordinates').

% RT Dose Module
dicom_tag_def(tag(0x3004, 0x0002), 'DoseUnits', 'CS', '1', 'GY or RELATIVE').
dicom_tag_def(tag(0x3004, 0x0004), 'DoseType', 'CS', '1', 'PHYSICAL or EFFECTIVE').
dicom_tag_def(tag(0x3004, 0x000A), 'DoseSummationType', 'CS', '1', 'PLAN, BEAM, etc.').
dicom_tag_def(tag(0x3004, 0x000C), 'GridFrameOffsetVector', 'DS', '2-n', 'Z offsets').
dicom_tag_def(tag(0x3004, 0x000E), 'DoseGridScaling', 'DS', '1', 'Dose scaling factor').


%% =============================================================================
%% ENERGY KERNEL DATA FORMAT - DCG
%% =============================================================================

%% Energy deposition kernel file format (.dat files)

kernel_file_format(kernel(Header, Data)) -->
    kernel_header(Header),
    kernel_data(Data).

kernel_header(header(Energy, Mu, NumPhi, NumRadial)) -->
    float_ascii(Energy),     % Beam energy in MV
    float_ascii(Mu),         % Linear attenuation coefficient
    int_ascii(NumPhi),       % Number of azimuthal angles
    int_ascii(NumRadial).    % Number of radial steps

kernel_data(data(Rows)) -->
    kernel_rows(Rows).

kernel_rows([]) --> [].
kernel_rows([Row|Rows]) -->
    kernel_row(Row),
    kernel_rows(Rows).

kernel_row(row(Phi, RadialValues)) -->
    float_ascii(Phi),
    radial_values(RadialValues).

radial_values([]) --> whitespace, newline.
radial_values([V|Vs]) -->
    whitespace,
    float_ascii(V),
    radial_values(Vs).

%% ASCII number parsing
float_ascii(F) -->
    whitespace,
    float_chars(Chars),
    { number_codes(F, Chars) }.

int_ascii(I) -->
    whitespace,
    digit_chars(Chars),
    { number_codes(I, Chars) }.

float_chars([C|Cs]) --> [C], { float_char(C) }, float_chars(Cs).
float_chars([]) --> [].

digit_chars([C|Cs]) --> [C], { C >= 0'0, C =< 0'9 }, digit_chars(Cs).
digit_chars([]) --> [].

float_char(C) :- C >= 0'0, C =< 0'9.
float_char(0'.).
float_char(0'-).
float_char(0'+).
float_char(0'e).
float_char(0'E).

whitespace --> [C], { C =:= 0' ; C =:= 0'\t }, whitespace.
whitespace --> [].

newline --> [0'\n].
newline --> [0'\r, 0'\n].


%% =============================================================================
%% PLAN XML FORMAT - DCG
%% =============================================================================

%% Simplified XML grammar for plan serialization

xml_document(doc(Prolog, Root)) -->
    xml_prolog(Prolog),
    xml_element(Root).

xml_prolog(prolog(Version, Encoding)) -->
    "<?xml",
    xml_whitespace,
    "version=\"", xml_attr_value(Version), "\"",
    xml_whitespace,
    "encoding=\"", xml_attr_value(Encoding), "\"",
    xml_whitespace_opt,
    "?>".

xml_element(element(Name, Attrs, Content)) -->
    "<", xml_name(Name),
    xml_attributes(Attrs),
    xml_whitespace_opt,
    ( "/>" , { Content = [] }
    ; ">", xml_content(Content), "</", xml_name(Name), ">"
    ).

xml_attributes([]) --> [].
xml_attributes([attr(N,V)|As]) -->
    xml_whitespace,
    xml_name(N),
    "=\"", xml_attr_value(V), "\"",
    xml_attributes(As).

xml_content([]) --> [].
xml_content([E|Es]) -->
    ( xml_element(E)
    ; xml_text(E)
    ),
    xml_content(Es).

xml_text(text(Text)) -->
    xml_text_chars(Text),
    { Text \= [] }.

xml_text_chars([C|Cs]) --> [C], { C \= 0'< }, xml_text_chars(Cs).
xml_text_chars([]) --> [].

xml_name(Name) -->
    xml_name_chars(Chars),
    { atom_codes(Name, Chars) }.

xml_name_chars([C|Cs]) --> [C], { xml_name_char(C) }, xml_name_chars(Cs).
xml_name_chars([]) --> [].

xml_name_char(C) :- C >= 0'a, C =< 0'z.
xml_name_char(C) :- C >= 0'A, C =< 0'Z.
xml_name_char(C) :- C >= 0'0, C =< 0'9.
xml_name_char(0'_).
xml_name_char(0'-).
xml_name_char(0':).

xml_attr_value(Value) -->
    xml_attr_chars(Chars),
    { atom_codes(Value, Chars) }.

xml_attr_chars([C|Cs]) --> [C], { C \= 0'" }, xml_attr_chars(Cs).
xml_attr_chars([]) --> [].

xml_whitespace --> [C], { C =:= 0'  ; C =:= 0'\t ; C =:= 0'\n ; C =:= 0'\r },
                   xml_whitespace_opt.
xml_whitespace_opt --> xml_whitespace.
xml_whitespace_opt --> [].


%% =============================================================================
%% PLAN-SPECIFIC XML ELEMENTS
%% =============================================================================

%% plan_xml(+Plan) - Top-level plan structure
plan_xml(plan(Series, Beams, Prescription)) -->
    "<Plan>",
    series_xml(Series),
    beams_xml(Beams),
    prescription_xml(Prescription),
    "</Plan>".

series_xml(series(Path, Structures)) -->
    "<Series path=\"", xml_attr_value(Path), "\">",
    structures_xml(Structures),
    "</Series>".

structures_xml([]) --> [].
structures_xml([S|Ss]) -->
    structure_xml(S),
    structures_xml(Ss).

structure_xml(structure(Name, Type, Color, Priority)) -->
    "<Structure",
    " name=\"", xml_attr_value(Name), "\"",
    " type=\"", xml_attr_value(Type), "\"",
    " color=\"", xml_attr_value(Color), "\"",
    " priority=\"", xml_attr_value(Priority), "\"",
    "/>".

beams_xml([]) --> [].
beams_xml([B|Bs]) -->
    beam_xml(B),
    beams_xml(Bs).

beam_xml(beam(GantryAngle, Isocenter, Weights)) -->
    "<Beam gantry=\"", xml_attr_value(GantryAngle), "\">",
    "<Isocenter>", vector_xml(Isocenter), "</Isocenter>",
    "<Weights>", weight_list_xml(Weights), "</Weights>",
    "</Beam>".

vector_xml(vec(X, Y, Z)) -->
    xml_attr_value(X), " ", xml_attr_value(Y), " ", xml_attr_value(Z).

weight_list_xml([]) --> [].
weight_list_xml([W|Ws]) -->
    xml_attr_value(W), " ",
    weight_list_xml(Ws).

prescription_xml(prescription(Terms)) -->
    "<Prescription>",
    voi_terms_xml(Terms),
    "</Prescription>".

voi_terms_xml([]) --> [].
voi_terms_xml([T|Ts]) -->
    voi_term_xml(T),
    voi_terms_xml(Ts).

voi_term_xml(kl_term(Structure, Weight, DVPs)) -->
    "<KLDivTerm",
    " structure=\"", xml_attr_value(Structure), "\"",
    " weight=\"", xml_attr_value(Weight), "\">",
    dvp_list_xml(DVPs),
    "</KLDivTerm>".

dvp_list_xml([]) --> [].
dvp_list_xml([dvp(Dose, Volume)|DVPs]) -->
    "<DVP dose=\"", xml_attr_value(Dose),
    "\" volume=\"", xml_attr_value(Volume), "\"/>",
    dvp_list_xml(DVPs).


%% =============================================================================
%% DVH DATA FORMAT
%% =============================================================================

%% dvh_export_format(+DVH) - Exportable DVH representation
dvh_export_format(dvh(Structure, BinWidth, CumVolumes)) -->
    "# DVH Export\n",
    "# Structure: ", atom_codes_dcg(Structure), "\n",
    "# Bin Width (Gy): ", float_write(BinWidth), "\n",
    "# Dose(Gy), CumulativeVolume(%)\n",
    dvh_data_rows(0, BinWidth, CumVolumes).

dvh_data_rows(_, _, []) --> [].
dvh_data_rows(Bin, Width, [V|Vs]) -->
    { Dose is Bin * Width },
    float_write(Dose), ", ", float_write(V), "\n",
    { Bin1 is Bin + 1 },
    dvh_data_rows(Bin1, Width, Vs).

atom_codes_dcg(Atom) -->
    { atom_codes(Atom, Codes) },
    Codes.

float_write(F) -->
    { number_codes(F, Codes) },
    Codes.


%% =============================================================================
%% QUERY UTILITIES
%% =============================================================================

%% Find all classes in a namespace
classes_in_namespace(Namespace, Classes) :-
    findall(Name, class(Name, Namespace, _, _, _), Classes).

%% Find all methods of a class (including inherited)
all_methods(Class, Methods) :-
    findall(Method, (
        ( method(Class, Method, _, _, _, _)
        ; inherits(Class, Parent), all_methods(Parent, ParentMethods),
          member(Method, ParentMethods)
        )
    ), MethodsList),
    sort(MethodsList, Methods).

%% Find implementation file for a class
implementation_of(Class, ImplFile) :-
    class(Class, _, _, HeaderFile, _),
    % Convert .h to .cpp
    atom_string(HeaderFile, HeaderStr),
    ( sub_string(HeaderStr, Before, 2, 0, ".h")
    -> sub_string(HeaderStr, 0, Before, _, Base),
       ( sub_string(Base, _, _, _, "/include/")
       -> % RtModel pattern: include/*.h -> *.cpp
          sub_string(Base, AfterInclude, _, 0, Name),
          sub_string(Base, 0, _, _, "RtModel/include/"),
          string_concat("RtModel/", Name, ImplBase),
          string_concat(ImplBase, ".cpp", ImplStr),
          atom_string(ImplFile, ImplStr)
       ; % Direct pattern: *.h -> *.cpp
         string_concat(Base, ".cpp", ImplStr),
         atom_string(ImplFile, ImplStr)
       )
    ; ImplFile = unknown
    ).

%% Find classes that depend on a given class
dependents_of(Class, Dependents) :-
    class(Class, _, _, DefFile, _),
    findall(Dep, (
        class(Dep, _, _, DepFile, _),
        depends_on(DepFile, DefFile)
    ), DepsList),
    sort(DepsList, Dependents).

%% Calculate inheritance depth
inheritance_depth(Class, 0) :-
    class(Class, _, none, _, _), !.
inheritance_depth(Class, Depth) :-
    class(Class, _, Parent, _, _),
    Parent \= none,
    inheritance_depth(Parent, ParentDepth),
    Depth is ParentDepth + 1.

%% Find root classes (no parent)
root_classes(Roots) :-
    findall(Class, class(Class, _, none, _, _), Roots).

%% Find leaf classes (no children)
leaf_classes(Leaves) :-
    findall(Class, (
        class(Class, _, _, _, _),
        \+ inherits(_, Class)
    ), Leaves).


%% =============================================================================
%% SEMANTIC QUERIES (Ontology-aware)
%% =============================================================================

%% Find entities by domain concept
%% iao:is_about - what the software is about
is_about(Entity, Concept) :-
    ( class(Entity, _, _, _, Desc)
    ; algorithm(Entity, _, Desc, _, _)
    ; data_format(Entity, _, _, Desc, _)
    ),
    atom_string(Concept, ConceptStr),
    atom_string(Desc, DescStr),
    sub_string(DescStr, _, _, _, ConceptStr).

%% swo:implements - what algorithm a class implements
implements(Class, Algorithm) :-
    algorithm(Algorithm, _, _, MethodSpec, _),
    ( atomic_list_concat([Class, '::', _], MethodSpec)
    ; sub_atom(MethodSpec, 0, _, _, Class)
    ).

%% obi:has_input / obi:has_output - data flow
has_input(Algorithm, InputType) :-
    algorithm(Algorithm, _, _, Method, _),
    method(Class, MethodName, _, Params, _, _),
    atomic_list_concat([Class, '::', MethodName], Method),
    member(param(_, InputType), Params).

has_output(Algorithm, OutputType) :-
    algorithm(Algorithm, _, _, Method, _),
    method(Class, MethodName, OutputType, _, _, _),
    atomic_list_concat([Class, '::', MethodName], Method),
    OutputType \= void.


%% =============================================================================
%% STATISTICS
%% =============================================================================

%% Count entities
entity_counts(Stats) :-
    findall(F, source_file(F, _, _, _, _), Files),
    length(Files, NumFiles),
    findall(C, class(C, _, _, _, _), Classes),
    length(Classes, NumClasses),
    findall(M, method(_, M, _, _, _, _), Methods),
    length(Methods, NumMethods),
    findall(A, algorithm(A, _, _, _, _), Algorithms),
    length(Algorithms, NumAlgorithms),
    findall(D, datatype(D, _, _, _), DataTypes),
    length(DataTypes, NumDataTypes),
    Stats = [
        files(NumFiles),
        classes(NumClasses),
        methods(NumMethods),
        algorithms(NumAlgorithms),
        datatypes(NumDataTypes)
    ].

%% Lines of code per module
loc_per_module(Module, TotalLines) :-
    findall(Lines, source_file(_, Module, implementation, Lines, _), LineCounts),
    sum_list(LineCounts, TotalLines).

sum_list([], 0).
sum_list([H|T], Sum) :- sum_list(T, Rest), Sum is H + Rest.

%% =============================================================================
%% END OF KNOWLEDGE BASE
%% =============================================================================
