%% =============================================================================
%% DICOM I/O - Mappings between DICOM and Internal Data Structures
%% =============================================================================
%% Defines transformations between DICOM elements and the internal
%% Brimstone data model (Series, Structure, Plan).
%% =============================================================================

:- module(dicom_io, [
    % DICOM to Internal transformations
    dicom_to_series/2,
    dicom_to_structure/2,
    dicom_to_plan/2,
    dicom_to_beam/2,
    dicom_to_dose/2,
    % Internal to DICOM transformations
    series_to_dicom/2,
    structure_to_dicom/2,
    plan_to_dicom/2,
    % Element extraction
    extract_element/3,
    extract_sequence/3,
    % Coordinate transformations
    dicom_coords_to_itk/3,
    itk_coords_to_dicom/3,
    % HU conversion
    stored_value_to_hu/4,
    hu_to_density/2
]).

:- use_module(dicom_dcg).


%% =============================================================================
%% DICOM TO INTERNAL DATA STRUCTURES
%% =============================================================================

%% dicom_to_series(+DicomDataset, -Series)
%% Extracts CT image data from DICOM and creates Series structure
%%
%% Maps:
%%   DICOM CT Image --> dH::Series
%%     PixelData --> VolumeReal* GetDensity()
%%     ImagePositionPatient --> origin
%%     PixelSpacing + SliceThickness --> spacing
%%     ImageOrientationPatient --> direction
%%     RescaleSlope/Intercept --> HU conversion
%%
dicom_to_series(DicomDataset, Series) :-
    DicomDataset = dataset(Elements),

    % Extract image geometry
    extract_element(Elements, tag(0x0028, 0x0010), Rows),
    extract_element(Elements, tag(0x0028, 0x0011), Columns),
    extract_element(Elements, tag(0x0028, 0x0030), PixelSpacing),
    extract_element(Elements, tag(0x0018, 0x0050), SliceThickness),
    extract_element(Elements, tag(0x0020, 0x0032), ImagePosition),
    extract_element(Elements, tag(0x0020, 0x0037), ImageOrientation),

    % Extract HU conversion factors
    extract_element(Elements, tag(0x0028, 0x1052), RescaleIntercept),
    extract_element(Elements, tag(0x0028, 0x1053), RescaleSlope),

    % Extract pixel data
    extract_element(Elements, tag(0x7FE0, 0x0010), PixelData),

    % Extract identifiers
    extract_element(Elements, tag(0x0020, 0x000E), SeriesUID),
    extract_element(Elements, tag(0x0010, 0x0020), PatientID),

    % Build Series structure
    Series = series(
        uid(SeriesUID),
        patient_id(PatientID),
        density(volume(
            size(Rows, Columns, _NumSlices),
            spacing(PixelSpacing, SliceThickness),
            origin(ImagePosition),
            orientation(ImageOrientation),
            data(PixelData),
            rescale(RescaleSlope, RescaleIntercept)
        )),
        structures([])  % Initially empty, populated from RT Structure Set
    ).


%% dicom_to_structure(+RTStructDataset, -Structures)
%% Extracts ROI definitions and contours from RT Structure Set
%%
%% Maps:
%%   DICOM RT Structure Set --> dH::Structure[]
%%     StructureSetROISequence --> ROI metadata (name, number)
%%     ROIContourSequence --> contours
%%       ContourSequence --> list of contour slices
%%         ContourData --> itk::PolygonSpatialObject<2> points
%%         ContourGeometricType --> CLOSED_PLANAR (required)
%%     ROIDisplayColor --> COLORREF GetColor()
%%
dicom_to_structure(RTStructDataset, Structures) :-
    RTStructDataset = dataset(Elements),

    % Extract ROI definition sequence
    extract_sequence(Elements, tag(0x3006, 0x0020), ROISequence),

    % Extract ROI contour sequence
    extract_sequence(Elements, tag(0x3006, 0x0039), ContourSequence),

    % Build structure for each ROI
    maplist(build_structure(ContourSequence), ROISequence, Structures).

build_structure(ContourSequence, ROIItem, Structure) :-
    ROIItem = item(_, _, ROIElements),

    % Extract ROI metadata
    extract_element(ROIElements, tag(0x3006, 0x0022), ROINumber),
    extract_element(ROIElements, tag(0x3006, 0x0026), ROIName),

    % Find matching contour data
    find_roi_contours(ContourSequence, ROINumber, ContourItems),

    % Extract display color if present
    ( find_roi_color(ContourSequence, ROINumber, Color)
    -> true
    ; Color = rgb(255, 0, 0)  % Default red
    ),

    % Convert contour points to polygon format
    maplist(contour_to_polygon, ContourItems, Polygons),

    % Determine structure type from name heuristics
    infer_structure_type(ROIName, StructType),

    Structure = structure(
        roi_number(ROINumber),
        name(ROIName),
        type(StructType),
        color(Color),
        contours(Polygons),
        priority(0),
        visible(true)
    ).

find_roi_contours(ContourSequence, ROINumber, ContourItems) :-
    include(roi_contour_matches(ROINumber), ContourSequence, Matching),
    ( Matching = [item(_, _, ContourElements)|_]
    -> extract_sequence(ContourElements, tag(0x3006, 0x0040), ContourItems)
    ; ContourItems = []
    ).

roi_contour_matches(ROINumber, item(_, _, Elements)) :-
    extract_element(Elements, tag(0x3006, 0x0084), RefROINumber),
    ROINumber = RefROINumber.

find_roi_color(ContourSequence, ROINumber, rgb(R, G, B)) :-
    include(roi_contour_matches(ROINumber), ContourSequence, [Item|_]),
    Item = item(_, _, Elements),
    extract_element(Elements, tag(0x3006, 0x002A), [R, G, B]).

contour_to_polygon(ContourItem, polygon(ZPos, Points)) :-
    ContourItem = item(_, _, Elements),
    extract_element(Elements, tag(0x3006, 0x0042), GeomType),
    extract_element(Elements, tag(0x3006, 0x0050), ContourData),

    % Verify closed planar contour
    GeomType = 'CLOSED_PLANAR',

    % Parse triplets (x, y, z) from ContourData
    parse_contour_points(ContourData, Points),

    % Extract Z position from first point
    Points = [point(_, _, ZPos)|_].

parse_contour_points([], []) :- !.
parse_contour_points([X, Y, Z | Rest], [point(X, Y, Z) | Points]) :-
    parse_contour_points(Rest, Points).

%% Infer structure type from name
infer_structure_type(Name, StructType) :-
    atom_string(Name, NameStr),
    string_upper(NameStr, UpperName),
    ( ( sub_string(UpperName, _, _, _, "PTV")
      ; sub_string(UpperName, _, _, _, "GTV")
      ; sub_string(UpperName, _, _, _, "CTV")
      ; sub_string(UpperName, _, _, _, "TARGET")
      )
    -> StructType = target
    ; ( sub_string(UpperName, _, _, _, "CORD")
      ; sub_string(UpperName, _, _, _, "BRAINSTEM")
      ; sub_string(UpperName, _, _, _, "PAROTID")
      ; sub_string(UpperName, _, _, _, "LUNG")
      ; sub_string(UpperName, _, _, _, "HEART")
      ; sub_string(UpperName, _, _, _, "LIVER")
      ; sub_string(UpperName, _, _, _, "KIDNEY")
      ; sub_string(UpperName, _, _, _, "RECTUM")
      ; sub_string(UpperName, _, _, _, "BLADDER")
      ; sub_string(UpperName, _, _, _, "FEMUR")
      ; sub_string(UpperName, _, _, _, "OAR")
      )
    -> StructType = oar
    ; StructType = none
    ).


%% dicom_to_plan(+RTPlanDataset, -Plan)
%% Extracts RT Plan and creates Plan structure
%%
%% Maps:
%%   DICOM RT Plan --> dH::Plan
%%     FractionGroupSequence --> fractionation
%%     BeamSequence --> CBeam[]
%%       ControlPointSequence --> gantry angles, isocenter
%%
dicom_to_plan(RTPlanDataset, Plan) :-
    RTPlanDataset = dataset(Elements),

    % Extract plan metadata
    extract_element(Elements, tag(0x300A, 0x0002), PlanLabel),
    extract_element(Elements, tag(0x300A, 0x0004), PlanDescription),

    % Extract beam sequence
    extract_sequence(Elements, tag(0x300A, 0x00B0), BeamSequence),

    % Convert each beam
    maplist(beam_item_to_beam, BeamSequence, Beams),

    % Extract fraction group
    extract_sequence(Elements, tag(0x300A, 0x0070), FractionSequence),
    ( FractionSequence = [FracItem|_]
    -> FracItem = item(_, _, FracElements),
       extract_element(FracElements, tag(0x300A, 0x0078), NumFractions)
    ; NumFractions = 1
    ),

    Plan = plan(
        label(PlanLabel),
        description(PlanDescription),
        fractions(NumFractions),
        beams(Beams),
        dose_resolution(0.5),  % Default, may be overridden
        histograms([])
    ).

beam_item_to_beam(BeamItem, Beam) :-
    BeamItem = item(_, _, Elements),

    extract_element(Elements, tag(0x300A, 0x00C2), BeamName),
    extract_element(Elements, tag(0x300A, 0x00C0), BeamNumber),
    extract_element(Elements, tag(0x300A, 0x00CE), BeamType),

    % Extract control point sequence for gantry/isocenter
    extract_sequence(Elements, tag(0x300A, 0x0111), ControlPoints),

    % First control point has initial gantry angle and isocenter
    ( ControlPoints = [CP1|_]
    -> CP1 = item(_, _, CP1Elements),
       extract_element(CP1Elements, tag(0x300A, 0x011E), GantryAngle),
       extract_element(CP1Elements, tag(0x300A, 0x012C), Isocenter)
    ; GantryAngle = 0.0, Isocenter = [0.0, 0.0, 0.0]
    ),

    Beam = beam(
        number(BeamNumber),
        name(BeamName),
        type(BeamType),
        gantry_angle(GantryAngle),
        isocenter(Isocenter),
        intensity_map([]),  % Populated during optimization
        beamlets([])
    ).


%% dicom_to_dose(+RTDoseDataset, -DoseVolume)
%% Extracts RT Dose grid
%%
%% Maps:
%%   DICOM RT Dose --> VolumeReal* GetDoseMatrix()
%%     PixelData * DoseGridScaling --> dose in Gy
%%     GridFrameOffsetVector --> Z positions
%%
dicom_to_dose(RTDoseDataset, DoseVolume) :-
    RTDoseDataset = dataset(Elements),

    % Extract geometry
    extract_element(Elements, tag(0x0028, 0x0010), Rows),
    extract_element(Elements, tag(0x0028, 0x0011), Columns),
    extract_element(Elements, tag(0x0028, 0x0030), PixelSpacing),
    extract_element(Elements, tag(0x0020, 0x0032), ImagePosition),
    extract_element(Elements, tag(0x3004, 0x000C), FrameOffsets),

    % Extract dose-specific
    extract_element(Elements, tag(0x3004, 0x0002), DoseUnits),
    extract_element(Elements, tag(0x3004, 0x000E), DoseGridScaling),
    extract_element(Elements, tag(0x7FE0, 0x0010), PixelData),

    length(FrameOffsets, NumFrames),

    DoseVolume = dose_volume(
        size(Rows, Columns, NumFrames),
        spacing(PixelSpacing, FrameOffsets),
        origin(ImagePosition),
        units(DoseUnits),
        scaling(DoseGridScaling),
        data(PixelData)
    ).


%% =============================================================================
%% INTERNAL TO DICOM TRANSFORMATIONS
%% =============================================================================

%% series_to_dicom(+Series, -DicomDataset)
%% Converts internal Series to DICOM CT dataset
%%
%% Maps:
%%   dH::Series --> DICOM CT Image
%%     VolumeReal* GetDensity() --> PixelData (after inverse HU conversion)
%%     origin --> ImagePositionPatient
%%     spacing --> PixelSpacing, SliceThickness
%%     direction --> ImageOrientationPatient
%%
series_to_dicom(Series, DicomDataset) :-
    Series = series(
        uid(SeriesUID),
        patient_id(PatientID),
        density(Volume),
        structures(_)
    ),
    Volume = volume(
        size(Rows, Columns, NumSlices),
        spacing(PixelSpacing, SliceThickness),
        origin(Origin),
        orientation(Orientation),
        data(VolumeData),
        rescale(Slope, Intercept)
    ),

    % Build DICOM elements
    DicomDataset = dataset([
        % SOP Common
        element(tag(0x0008, 0x0016), vr('UI', _), value('1.2.840.10008.5.1.4.1.1.2')),
        element(tag(0x0008, 0x0018), vr('UI', _), value(SeriesUID)),

        % Patient Module
        element(tag(0x0010, 0x0020), vr('LO', _), value(PatientID)),

        % Series Module
        element(tag(0x0020, 0x000E), vr('UI', _), value(SeriesUID)),
        element(tag(0x0008, 0x0060), vr('CS', _), value('CT')),

        % Image Pixel Module
        element(tag(0x0028, 0x0010), vr('US', length(2)), value(Rows)),
        element(tag(0x0028, 0x0011), vr('US', length(2)), value(Columns)),
        element(tag(0x0028, 0x0030), vr('DS', _), value(PixelSpacing)),
        element(tag(0x0018, 0x0050), vr('DS', _), value(SliceThickness)),

        % Image Plane Module
        element(tag(0x0020, 0x0032), vr('DS', _), value(Origin)),
        element(tag(0x0020, 0x0037), vr('DS', _), value(Orientation)),

        % CT Image Module
        element(tag(0x0028, 0x1052), vr('DS', _), value(Intercept)),
        element(tag(0x0028, 0x1053), vr('DS', _), value(Slope)),

        % Pixel Data
        element(tag(0x7FE0, 0x0010), vr('OW', _), value(VolumeData))
    ]).


%% structure_to_dicom(+Structures, -RTStructDataset)
%% Converts internal Structures to DICOM RT Structure Set
%%
%% Maps:
%%   dH::Structure[] --> DICOM RT Structure Set
%%     GetName() --> ROIName
%%     GetType() --> (inferred from name)
%%     GetContour() --> ContourData
%%     GetColor() --> ROIDisplayColor
%%
structure_to_dicom(Structures, RTStructDataset) :-
    % Build ROI sequence
    maplist(structure_to_roi_item, Structures, ROIItems),

    % Build contour sequence
    maplist(structure_to_contour_item, Structures, ContourItems),

    RTStructDataset = dataset([
        % SOP Common
        element(tag(0x0008, 0x0016), vr('UI', _),
                value('1.2.840.10008.5.1.4.1.1.481.3')),

        % RT Structure Set Module
        element(tag(0x3006, 0x0002), vr('SH', _), value('Brimstone Structures')),
        element(tag(0x3006, 0x0020), vr('SQ', _), value(ROIItems)),
        element(tag(0x3006, 0x0039), vr('SQ', _), value(ContourItems))
    ]).

structure_to_roi_item(Structure, item(item, _, Elements)) :-
    Structure = structure(
        roi_number(ROINumber),
        name(ROIName),
        type(_),
        color(_),
        contours(_),
        priority(_),
        visible(_)
    ),
    Elements = [
        element(tag(0x3006, 0x0022), vr('IS', _), value(ROINumber)),
        element(tag(0x3006, 0x0026), vr('LO', _), value(ROIName)),
        element(tag(0x3006, 0x0036), vr('CS', _), value('MANUAL'))
    ].

structure_to_contour_item(Structure, item(item, _, Elements)) :-
    Structure = structure(
        roi_number(ROINumber),
        name(_),
        type(_),
        color(rgb(R, G, B)),
        contours(Polygons),
        priority(_),
        visible(_)
    ),
    % Convert polygons to DICOM contour items
    maplist(polygon_to_contour_item, Polygons, ContourSeqItems),

    Elements = [
        element(tag(0x3006, 0x0084), vr('IS', _), value(ROINumber)),
        element(tag(0x3006, 0x002A), vr('IS', _), value([R, G, B])),
        element(tag(0x3006, 0x0040), vr('SQ', _), value(ContourSeqItems))
    ].

polygon_to_contour_item(polygon(ZPos, Points), item(item, _, Elements)) :-
    length(Points, NumPoints),
    % Flatten points to [x1,y1,z1,x2,y2,z2,...]
    maplist(point_to_coords, Points, CoordLists),
    append(CoordLists, ContourData),

    Elements = [
        element(tag(0x3006, 0x0042), vr('CS', _), value('CLOSED_PLANAR')),
        element(tag(0x3006, 0x0046), vr('IS', _), value(NumPoints)),
        element(tag(0x3006, 0x0050), vr('DS', _), value(ContourData))
    ].

point_to_coords(point(X, Y, Z), [X, Y, Z]).


%% plan_to_dicom(+Plan, -RTPlanDataset)
%% Converts internal Plan to DICOM RT Plan
plan_to_dicom(Plan, RTPlanDataset) :-
    Plan = plan(
        label(PlanLabel),
        description(PlanDescription),
        fractions(NumFractions),
        beams(Beams),
        dose_resolution(_),
        histograms(_)
    ),

    % Convert beams
    maplist(beam_to_dicom_item, Beams, BeamItems),

    RTPlanDataset = dataset([
        % SOP Common
        element(tag(0x0008, 0x0016), vr('UI', _),
                value('1.2.840.10008.5.1.4.1.1.481.5')),

        % RT Plan Module
        element(tag(0x300A, 0x0002), vr('SH', _), value(PlanLabel)),
        element(tag(0x300A, 0x0004), vr('LO', _), value(PlanDescription)),
        element(tag(0x300A, 0x00B0), vr('SQ', _), value(BeamItems)),

        % Fraction Group
        element(tag(0x300A, 0x0070), vr('SQ', _), value([
            item(item, _, [
                element(tag(0x300A, 0x0078), vr('IS', _), value(NumFractions))
            ])
        ]))
    ]).

beam_to_dicom_item(Beam, item(item, _, Elements)) :-
    Beam = beam(
        number(BeamNumber),
        name(BeamName),
        type(BeamType),
        gantry_angle(GantryAngle),
        isocenter(Isocenter),
        intensity_map(_),
        beamlets(_)
    ),
    Elements = [
        element(tag(0x300A, 0x00C0), vr('IS', _), value(BeamNumber)),
        element(tag(0x300A, 0x00C2), vr('LO', _), value(BeamName)),
        element(tag(0x300A, 0x00CE), vr('CS', _), value(BeamType)),
        element(tag(0x300A, 0x0111), vr('SQ', _), value([
            item(item, _, [
                element(tag(0x300A, 0x011E), vr('DS', _), value(GantryAngle)),
                element(tag(0x300A, 0x012C), vr('DS', _), value(Isocenter))
            ])
        ]))
    ].


%% =============================================================================
%% ELEMENT EXTRACTION UTILITIES
%% =============================================================================

%% extract_element(+Elements, +Tag, -Value)
%% Finds element by tag and extracts value
extract_element(Elements, Tag, Value) :-
    member(element(Tag, _, value(Value)), Elements), !.
extract_element(_, _, undefined).

%% extract_sequence(+Elements, +Tag, -Items)
%% Extracts sequence items
extract_sequence(Elements, Tag, Items) :-
    member(element(Tag, vr('SQ', _), value(Items)), Elements), !.
extract_sequence(_, _, []).


%% =============================================================================
%% COORDINATE TRANSFORMATIONS
%% =============================================================================

%% dicom_coords_to_itk(+DicomCoords, +ImageGeometry, -ITKCoords)
%% Converts DICOM patient coordinates to ITK image indices
%%
%% DICOM: (x, y, z) in mm, patient coordinate system (LPS)
%% ITK:   (i, j, k) image indices
%%
%% Formula: index = (coord - origin) / spacing * direction
%%
dicom_coords_to_itk(DicomCoords, ImageGeometry, ITKCoords) :-
    DicomCoords = [Dx, Dy, Dz],
    ImageGeometry = geometry(
        origin(Ox, Oy, Oz),
        spacing(Sx, Sy, Sz),
        direction(Dir)
    ),
    % Compute offset from origin
    OffX is Dx - Ox,
    OffY is Dy - Oy,
    OffZ is Dz - Oz,
    % Apply inverse direction and divide by spacing
    apply_inverse_direction([OffX, OffY, OffZ], Dir, Rotated),
    Rotated = [Rx, Ry, Rz],
    Ix is round(Rx / Sx),
    Iy is round(Ry / Sy),
    Iz is round(Rz / Sz),
    ITKCoords = [Ix, Iy, Iz].

%% itk_coords_to_dicom(+ITKCoords, +ImageGeometry, -DicomCoords)
%% Converts ITK image indices to DICOM patient coordinates
%%
%% Formula: coord = origin + index * spacing * direction
%%
itk_coords_to_dicom(ITKCoords, ImageGeometry, DicomCoords) :-
    ITKCoords = [Ix, Iy, Iz],
    ImageGeometry = geometry(
        origin(Ox, Oy, Oz),
        spacing(Sx, Sy, Sz),
        direction(Dir)
    ),
    % Scale by spacing
    ScaledX is Ix * Sx,
    ScaledY is Iy * Sy,
    ScaledZ is Iz * Sz,
    % Apply direction matrix
    apply_direction([ScaledX, ScaledY, ScaledZ], Dir, Rotated),
    Rotated = [Rx, Ry, Rz],
    % Add origin
    Dx is Ox + Rx,
    Dy is Oy + Ry,
    Dz is Oz + Rz,
    DicomCoords = [Dx, Dy, Dz].

%% Direction matrix operations (3x3)
apply_direction([X, Y, Z], Dir, [Rx, Ry, Rz]) :-
    Dir = [D00, D01, D02, D10, D11, D12, D20, D21, D22],
    Rx is D00*X + D01*Y + D02*Z,
    Ry is D10*X + D11*Y + D12*Z,
    Rz is D20*X + D21*Y + D22*Z.

apply_inverse_direction([X, Y, Z], Dir, [Rx, Ry, Rz]) :-
    % For orthogonal direction matrix, inverse = transpose
    Dir = [D00, D01, D02, D10, D11, D12, D20, D21, D22],
    Rx is D00*X + D10*Y + D20*Z,
    Ry is D01*X + D11*Y + D21*Z,
    Rz is D02*X + D12*Y + D22*Z.


%% =============================================================================
%% HOUNSFIELD UNIT CONVERSIONS
%% =============================================================================

%% stored_value_to_hu(+StoredValue, +Slope, +Intercept, -HU)
%% Converts stored pixel value to Hounsfield Units
%%
%% Formula: HU = StoredValue * RescaleSlope + RescaleIntercept
%%
stored_value_to_hu(StoredValue, Slope, Intercept, HU) :-
    HU is StoredValue * Slope + Intercept.

%% hu_to_density(+HU, -Density)
%% Converts Hounsfield Units to mass density (g/cm³)
%%
%% Uses standard CT-to-density conversion:
%%   HU <= -1000: Air (0.001 g/cm³)
%%   HU = 0: Water (1.0 g/cm³)
%%   HU >= 1000: Dense bone (~1.8-2.0 g/cm³)
%%
%% Linear interpolation between calibration points
%%
hu_to_density(HU, Density) :-
    HU =< -1000, !,
    Density is 0.001.  % Air
hu_to_density(HU, Density) :-
    HU >= 3000, !,
    Density is 2.5.  % Very dense bone/metal
hu_to_density(HU, Density) :-
    HU < 0, !,
    % Interpolate between air (-1000 HU) and water (0 HU)
    Density is 0.001 + (1.0 - 0.001) * (HU + 1000) / 1000.
hu_to_density(HU, Density) :-
    HU >= 0, HU < 100, !,
    % Soft tissue range
    Density is 1.0 + 0.001 * HU.
hu_to_density(HU, Density) :-
    HU >= 100, HU < 1000, !,
    % Transition to bone
    Density is 1.1 + (1.8 - 1.1) * (HU - 100) / 900.
hu_to_density(HU, Density) :-
    HU >= 1000, !,
    % Dense bone
    Density is 1.8 + (2.5 - 1.8) * (HU - 1000) / 2000.


%% =============================================================================
%% IMPORT WORKFLOW (matches CSeriesDicomImporter)
%% =============================================================================

%% import_dicom_series(+Directory, -Series)
%% High-level import matching CSeriesDicomImporter workflow
%%
%% 1. Scan directory for DICOM files
%% 2. Group by SeriesInstanceUID
%% 3. Sort CT slices by SliceLocation/ImagePositionPatient
%% 4. Build 3D volume from 2D slices
%% 5. Import RT Structure Set if present
%%
import_workflow(Directory, Series) :-
    % Step 1: Find all DICOM files
    find_dicom_files(Directory, Files),

    % Step 2: Categorize by SOP Class
    categorize_by_sop_class(Files, CTFiles, RTStructFiles, _RTDoseFiles),

    % Step 3: Build CT volume
    build_ct_volume(CTFiles, CTSeries),

    % Step 4: Import structures
    ( RTStructFiles = [RTStructFile|_]
    -> import_rtstruct(RTStructFile, Structures),
       add_structures_to_series(CTSeries, Structures, Series)
    ; Series = CTSeries
    ).

find_dicom_files(Directory, Files) :-
    % Placeholder - would scan directory
    atom_concat(Directory, '/*.dcm', Pattern),
    Files = [Pattern].  % Simplified

categorize_by_sop_class(Files, CTFiles, RTStructFiles, RTDoseFiles) :-
    % Placeholder - would parse and categorize
    CTFiles = Files,
    RTStructFiles = [],
    RTDoseFiles = [].

build_ct_volume(CTFiles, Series) :-
    % Placeholder - would build volume from slices
    Series = series(uid(_), patient_id(_), density(_), structures([])).

import_rtstruct(RTStructFile, Structures) :-
    % Placeholder - would parse RT Structure Set
    Structures = [].

add_structures_to_series(series(UID, PID, Density, _), Structures,
                         series(UID, PID, Density, structures(Structures))).


%% =============================================================================
%% EXPORT WORKFLOW
%% =============================================================================

%% export_dicom_series(+Series, +OutputDir)
%% High-level export workflow
%%
%% 1. Generate unique UIDs
%% 2. Write CT slices as separate DICOM files
%% 3. Write RT Structure Set
%% 4. Write RT Dose if present
%%
export_workflow(Series, Plan, OutputDir) :-
    % Step 1: Generate study/series UIDs
    generate_uid(StudyUID),
    generate_uid(CTSeriesUID),
    generate_uid(RTStructUID),

    % Step 2: Export CT slices
    series_to_dicom(Series, CTDataset),
    write_dicom_slices(CTDataset, CTSeriesUID, OutputDir),

    % Step 3: Export RT Structure Set
    Series = series(_, _, _, structures(Structures)),
    structure_to_dicom(Structures, RTStructDataset),
    write_dicom_file(RTStructDataset, RTStructUID, OutputDir),

    % Step 4: Export RT Plan if present
    ( nonvar(Plan)
    -> plan_to_dicom(Plan, RTPlanDataset),
       generate_uid(RTPlanUID),
       write_dicom_file(RTPlanDataset, RTPlanUID, OutputDir)
    ; true
    ).

generate_uid(UID) :-
    % Placeholder - would generate proper DICOM UID
    UID = '1.2.3.4.5.6.7.8.9'.

write_dicom_slices(_Dataset, _SeriesUID, _OutputDir) :-
    % Placeholder - would write slice files
    true.

write_dicom_file(_Dataset, _UID, _OutputDir) :-
    % Placeholder - would write DICOM file
    true.
