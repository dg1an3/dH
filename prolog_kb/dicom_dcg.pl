%% =============================================================================
%% DICOM DCG - Definite Clause Grammar for DICOM File Format
%% =============================================================================
%% Formal grammar representation of DICOM Part 10 file structure.
%% Reference: DICOM PS3.5 - Data Structures and Encoding
%% =============================================================================

:- module(dicom_dcg, [
    % DCG rules (//1 notation)
    dicom_file//1,
    dicom_element//1,
    dicom_meta_info//1,
    dicom_dataset//1,
    dicom_sequence//2,
    dicom_item//1,
    dicom_tag//1,
    dicom_vr//1,
    % Semantic tag definitions
    dicom_tag_def/5,
    dicom_module/2,
    % VR predicates
    short_vr/1,
    long_vr/1
]).


%% =============================================================================
%% TOP-LEVEL DICOM FILE STRUCTURE
%% =============================================================================

%% dicom_file(-DicomFile)//
%% Parses complete DICOM Part 10 file format
dicom_file(dicom(Preamble, Prefix, MetaInfo, Dataset)) -->
    dicom_preamble(Preamble),
    dicom_prefix(Prefix),
    dicom_meta_info(MetaInfo),
    dicom_dataset(Dataset).

%% 128-byte preamble (typically zeros, may contain application-specific data)
dicom_preamble(preamble(Bytes)) -->
    { length(Bytes, 128) },
    Bytes.

%% DICM prefix marker (mandatory after preamble)
dicom_prefix(prefix("DICM")) --> [0'D, 0'I, 0'C, 0'M].

%% File Meta Information Header (Group 0002, Explicit VR Little Endian)
dicom_meta_info(meta_info(Length, Elements)) -->
    dicom_element(element(tag(0x0002, 0x0000), vr('UL', length(4)), value(Length))),
    dicom_meta_elements(Length, Elements).

dicom_meta_elements(0, []) --> !.
dicom_meta_elements(Remaining, [E|Es]) -->
    { Remaining > 0 },
    dicom_element(E),
    { element_size(E, Size), NewRemaining is Remaining - Size },
    dicom_meta_elements(NewRemaining, Es).

element_size(element(_, vr(_, length(L)), _), Size) :-
    Size is 4 + 4 + L.  % tag + vr/length + value


%% =============================================================================
%% DICOM DATASET
%% =============================================================================

%% Main dataset (Groups > 0002)
dicom_dataset(dataset(Elements)) -->
    dicom_element_list(Elements).

%% List of data elements
dicom_element_list([]) --> [].
dicom_element_list([E|Es]) -->
    dicom_element(E),
    dicom_element_list(Es).


%% =============================================================================
%% DICOM DATA ELEMENT
%% =============================================================================

%% Single DICOM data element: (Tag, VR, Value)
dicom_element(element(Tag, VR, Value)) -->
    dicom_tag(Tag),
    dicom_vr(VR),
    dicom_value(VR, Value).

%% Tag: (group, element) pair - 4 bytes little-endian
dicom_tag(tag(Group, Element)) -->
    uint16_le(Group),
    uint16_le(Element).


%% =============================================================================
%% VALUE REPRESENTATION (VR)
%% =============================================================================

%% Value Representation with length
dicom_vr(vr(VR, Length)) -->
    vr_code(VR),
    vr_length(VR, Length).

%% Standard VR codes (2 ASCII characters)
%% Complete list per DICOM PS3.5 Table 6.2-1

% String VRs
vr_code('AE') --> [0'A, 0'E].  % Application Entity (max 16 chars)
vr_code('AS') --> [0'A, 0'S].  % Age String (4 chars: nnnD/W/M/Y)
vr_code('CS') --> [0'C, 0'S].  % Code String (max 16 chars)
vr_code('DA') --> [0'D, 0'A].  % Date (8 chars: YYYYMMDD)
vr_code('DS') --> [0'D, 0'S].  % Decimal String (max 16 chars)
vr_code('DT') --> [0'D, 0'T].  % DateTime (max 26 chars)
vr_code('IS') --> [0'I, 0'S].  % Integer String (max 12 chars)
vr_code('LO') --> [0'L, 0'O].  % Long String (max 64 chars)
vr_code('LT') --> [0'L, 0'T].  % Long Text (max 10240 chars)
vr_code('PN') --> [0'P, 0'N].  % Person Name (max 64 chars per component)
vr_code('SH') --> [0'S, 0'H].  % Short String (max 16 chars)
vr_code('ST') --> [0'S, 0'T].  % Short Text (max 1024 chars)
vr_code('TM') --> [0'T, 0'M].  % Time (max 14 chars)
vr_code('UC') --> [0'U, 0'C].  % Unlimited Characters
vr_code('UI') --> [0'U, 0'I].  % Unique Identifier (max 64 chars)
vr_code('UR') --> [0'U, 0'R].  % Universal Resource Identifier
vr_code('UT') --> [0'U, 0'T].  % Unlimited Text

% Binary/Numeric VRs
vr_code('AT') --> [0'A, 0'T].  % Attribute Tag
vr_code('FL') --> [0'F, 0'L].  % Floating Point Single (4 bytes)
vr_code('FD') --> [0'F, 0'D].  % Floating Point Double (8 bytes)
vr_code('SL') --> [0'S, 0'L].  % Signed Long (4 bytes)
vr_code('SS') --> [0'S, 0'S].  % Signed Short (2 bytes)
vr_code('UL') --> [0'U, 0'L].  % Unsigned Long (4 bytes)
vr_code('US') --> [0'U, 0'S].  % Unsigned Short (2 bytes)

% Byte/Word VRs (potentially large)
vr_code('OB') --> [0'O, 0'B].  % Other Byte
vr_code('OD') --> [0'O, 0'D].  % Other Double
vr_code('OF') --> [0'O, 0'F].  % Other Float
vr_code('OL') --> [0'O, 0'L].  % Other Long
vr_code('OV') --> [0'O, 0'V].  % Other 64-bit Very Long
vr_code('OW') --> [0'O, 0'W].  % Other Word
vr_code('SV') --> [0'S, 0'V].  % Signed 64-bit Very Long
vr_code('UV') --> [0'U, 0'V].  % Unsigned 64-bit Very Long

% Sequence VR
vr_code('SQ') --> [0'S, 0'Q].  % Sequence of Items

% Unknown VR
vr_code('UN') --> [0'U, 0'N].  % Unknown


%% VR length encoding

%% Short form: 2-byte length immediately follows VR
vr_length(VR, length(Len)) -->
    { short_vr(VR) },
    uint16_le(Len).

%% Long form: 2 reserved bytes (0x0000), then 4-byte length
vr_length(VR, length(Len)) -->
    { long_vr(VR) },
    [0x00, 0x00],
    uint32_le(Len).

%% VRs using short (2-byte) length encoding
short_vr('AE'). short_vr('AS'). short_vr('AT'). short_vr('CS').
short_vr('DA'). short_vr('DS'). short_vr('DT'). short_vr('FL').
short_vr('FD'). short_vr('IS'). short_vr('LO'). short_vr('LT').
short_vr('PN'). short_vr('SH'). short_vr('SL'). short_vr('SS').
short_vr('ST'). short_vr('TM'). short_vr('UI'). short_vr('UL').
short_vr('US').

%% VRs using long (4-byte) length encoding
long_vr('OB'). long_vr('OD'). long_vr('OF'). long_vr('OL').
long_vr('OV'). long_vr('OW'). long_vr('SQ'). long_vr('SV').
long_vr('UC'). long_vr('UN'). long_vr('UR'). long_vr('UT').
long_vr('UV').


%% =============================================================================
%% VALUE PARSING
%% =============================================================================

%% Parse value based on VR type
dicom_value(vr('US', length(2)), value(V)) --> uint16_le(V).
dicom_value(vr('US', length(L)), value(Vs)) -->
    { L > 2, L mod 2 =:= 0 },
    us_list(L, Vs).
dicom_value(vr('SS', length(2)), value(V)) --> int16_le(V).
dicom_value(vr('UL', length(4)), value(V)) --> uint32_le(V).
dicom_value(vr('SL', length(4)), value(V)) --> int32_le(V).
dicom_value(vr('FL', length(4)), value(V)) --> float32_le(V).
dicom_value(vr('FD', length(8)), value(V)) --> float64_le(V).
dicom_value(vr('AT', length(4)), value(tag(G, E))) -->
    uint16_le(G), uint16_le(E).
dicom_value(vr('SQ', length(Len)), value(Items)) -->
    dicom_sequence(Len, Items).
dicom_value(vr('UI', length(Len)), value(UID)) -->
    n_bytes(Len, Bytes),
    { trim_null(Bytes, Trimmed), atom_codes(UID, Trimmed) }.
dicom_value(vr(VR, length(Len)), value(Str)) -->
    { string_vr(VR) },
    n_bytes(Len, Bytes),
    { trim_padding(Bytes, Trimmed), atom_codes(Str, Trimmed) }.
dicom_value(vr(_, length(Len)), value(Bytes)) -->
    n_bytes(Len, Bytes).

%% String VRs
string_vr('AE'). string_vr('AS'). string_vr('CS'). string_vr('DA').
string_vr('DS'). string_vr('DT'). string_vr('IS'). string_vr('LO').
string_vr('LT'). string_vr('PN'). string_vr('SH'). string_vr('ST').
string_vr('TM'). string_vr('UC'). string_vr('UR'). string_vr('UT').

%% Multiple US values
us_list(0, []) --> [].
us_list(N, [V|Vs]) -->
    { N >= 2, N1 is N - 2 },
    uint16_le(V),
    us_list(N1, Vs).

%% Trim null and space padding
trim_null(Bytes, Trimmed) :-
    exclude(=(0), Bytes, Trimmed).
trim_padding(Bytes, Trimmed) :-
    exclude(is_padding, Bytes, Trimmed).
is_padding(0).
is_padding(0x20).  % Space


%% =============================================================================
%% SEQUENCE OF ITEMS (SQ)
%% =============================================================================

%% Sequence with undefined length (ends with Sequence Delimitation Item)
dicom_sequence(0xFFFFFFFF, Items) -->
    dicom_sequence_items(Items),
    sequence_delimitation_item.

%% Sequence with defined length
dicom_sequence(Len, Items) -->
    { Len \= 0xFFFFFFFF },
    dicom_sequence_fixed(Len, Items).

dicom_sequence_fixed(0, []) --> !.
dicom_sequence_fixed(Remaining, [Item|Items]) -->
    { Remaining > 0 },
    dicom_item(Item),
    { item_size(Item, Size), NewRemaining is Remaining - Size },
    dicom_sequence_fixed(NewRemaining, Items).

item_size(item(_, Len, _), Size) :-
    Size is 8 + Len.  % item tag + length + content

%% Sequence items (undefined length sequence)
dicom_sequence_items([]) -->
    peek_sequence_delimiter, !.
dicom_sequence_items([Item|Items]) -->
    dicom_item(Item),
    dicom_sequence_items(Items).

peek_sequence_delimiter -->
    [0xFE, 0xFF, 0xDD, 0xE0].

%% Single sequence item
dicom_item(item(Tag, Len, Elements)) -->
    item_tag(Tag),
    uint32_le(Len),
    dicom_item_content(Len, Elements).

item_tag(item) --> [0xFE, 0xFF, 0x00, 0xE0].  % (FFFE,E000)

%% Item content
dicom_item_content(0xFFFFFFFF, Elements) -->
    dicom_element_list(Elements),
    item_delimitation.
dicom_item_content(Len, Elements) -->
    { Len \= 0xFFFFFFFF },
    dicom_item_fixed(Len, Elements).

dicom_item_fixed(0, []) --> !.
dicom_item_fixed(Remaining, [E|Es]) -->
    { Remaining > 0 },
    dicom_element(E),
    { element_size(E, Size), NewRemaining is Remaining - Size },
    dicom_item_fixed(NewRemaining, Es).

%% Delimitation items
item_delimitation --> [0xFE, 0xFF, 0x0D, 0xE0, 0x00, 0x00, 0x00, 0x00].
sequence_delimitation_item --> [0xFE, 0xFF, 0xDD, 0xE0, 0x00, 0x00, 0x00, 0x00].


%% =============================================================================
%% PIXEL DATA ENCAPSULATION
%% =============================================================================

%% Encapsulated pixel data (for compressed transfer syntaxes)
encapsulated_pixel_data(frames(Frames)) -->
    basic_offset_table(Offsets),
    pixel_data_fragments(Frames),
    sequence_delimitation_item.

basic_offset_table(offsets(Offsets)) -->
    item_tag(_),
    uint32_le(Len),
    offset_values(Len, Offsets).

offset_values(0, []) --> !.
offset_values(N, [O|Os]) -->
    { N >= 4, N1 is N - 4 },
    uint32_le(O),
    offset_values(N1, Os).

pixel_data_fragments([]) -->
    peek_sequence_delimiter, !.
pixel_data_fragments([Frame|Frames]) -->
    pixel_data_fragment(Frame),
    pixel_data_fragments(Frames).

pixel_data_fragment(fragment(Data)) -->
    item_tag(_),
    uint32_le(Len),
    n_bytes(Len, Data).


%% =============================================================================
%% BINARY PRIMITIVES (Little-Endian)
%% =============================================================================

uint16_le(V) --> [Lo, Hi],
    { V is Hi * 256 + Lo }.

int16_le(V) --> uint16_le(U),
    { U > 32767 -> V is U - 65536 ; V = U }.

uint32_le(V) --> [B0, B1, B2, B3],
    { V is B0 + B1*256 + B2*65536 + B3*16777216 }.

int32_le(V) --> uint32_le(U),
    { U > 2147483647 -> V is U - 4294967296 ; V = U }.

%% IEEE 754 floating point (simplified - returns bytes for now)
float32_le(float32(Bytes)) --> n_bytes(4, Bytes).
float64_le(float64(Bytes)) --> n_bytes(8, Bytes).

%% N bytes helper
n_bytes(0, []) --> !.
n_bytes(N, [B|Bs]) -->
    { N > 0, N1 is N - 1 },
    [B],
    n_bytes(N1, Bs).


%% =============================================================================
%% DICOM TAG DEFINITIONS - Semantic Layer
%% =============================================================================

%% dicom_tag_def(+Tag, +Name, +VR, +VM, +Description)
%% VM = Value Multiplicity (1, 1-n, 2, 3, etc.)

%% DICOM Module groupings
dicom_module(patient, [
    tag(0x0010, 0x0010),  % PatientName
    tag(0x0010, 0x0020),  % PatientID
    tag(0x0010, 0x0030),  % PatientBirthDate
    tag(0x0010, 0x0040)   % PatientSex
]).

dicom_module(study, [
    tag(0x0008, 0x0020),  % StudyDate
    tag(0x0008, 0x0030),  % StudyTime
    tag(0x0020, 0x000D),  % StudyInstanceUID
    tag(0x0008, 0x1030)   % StudyDescription
]).

dicom_module(series, [
    tag(0x0020, 0x000E),  % SeriesInstanceUID
    tag(0x0008, 0x0060),  % Modality
    tag(0x0008, 0x103E)   % SeriesDescription
]).

dicom_module(ct_image, [
    tag(0x0028, 0x0010),  % Rows
    tag(0x0028, 0x0011),  % Columns
    tag(0x0028, 0x0030),  % PixelSpacing
    tag(0x0028, 0x1052),  % RescaleIntercept
    tag(0x0028, 0x1053),  % RescaleSlope
    tag(0x7FE0, 0x0010)   % PixelData
]).

dicom_module(image_plane, [
    tag(0x0020, 0x0032),  % ImagePositionPatient
    tag(0x0020, 0x0037),  % ImageOrientationPatient
    tag(0x0018, 0x0050),  % SliceThickness
    tag(0x0020, 0x1041)   % SliceLocation
]).

dicom_module(rt_structure_set, [
    tag(0x3006, 0x0002),  % StructureSetLabel
    tag(0x3006, 0x0020),  % StructureSetROISequence
    tag(0x3006, 0x0039),  % ROIContourSequence
    tag(0x3006, 0x0080)   % ReferencedFrameOfReferenceSequence
]).

dicom_module(rt_roi, [
    tag(0x3006, 0x0022),  % ROINumber
    tag(0x3006, 0x0026),  % ROIName
    tag(0x3006, 0x0024),  % ReferencedFrameOfReferenceUID
    tag(0x3006, 0x0036)   % ROIGenerationAlgorithm
]).

dicom_module(rt_roi_contour, [
    tag(0x3006, 0x0040),  % ContourSequence
    tag(0x3006, 0x0084),  % ReferencedROINumber
    tag(0x3006, 0x002A)   % ROIDisplayColor
]).

dicom_module(rt_contour, [
    tag(0x3006, 0x0042),  % ContourGeometricType
    tag(0x3006, 0x0046),  % NumberOfContourPoints
    tag(0x3006, 0x0050)   % ContourData
]).

dicom_module(rt_dose, [
    tag(0x3004, 0x0002),  % DoseUnits
    tag(0x3004, 0x0004),  % DoseType
    tag(0x3004, 0x000A),  % DoseSummationType
    tag(0x3004, 0x000C),  % GridFrameOffsetVector
    tag(0x3004, 0x000E)   % DoseGridScaling
]).

%% Patient Module Tags
dicom_tag_def(tag(0x0010, 0x0010), 'PatientName', 'PN', '1',
    'Patient name in format: Family^Given^Middle^Prefix^Suffix').
dicom_tag_def(tag(0x0010, 0x0020), 'PatientID', 'LO', '1',
    'Primary identifier for the patient').
dicom_tag_def(tag(0x0010, 0x0030), 'PatientBirthDate', 'DA', '1',
    'Birth date of the patient (YYYYMMDD)').
dicom_tag_def(tag(0x0010, 0x0040), 'PatientSex', 'CS', '1',
    'Sex of the patient: M, F, or O').

%% Study Module Tags
dicom_tag_def(tag(0x0008, 0x0020), 'StudyDate', 'DA', '1',
    'Date the study started (YYYYMMDD)').
dicom_tag_def(tag(0x0008, 0x0030), 'StudyTime', 'TM', '1',
    'Time the study started (HHMMSS.FFFFFF)').
dicom_tag_def(tag(0x0020, 0x000D), 'StudyInstanceUID', 'UI', '1',
    'Unique identifier for the study').
dicom_tag_def(tag(0x0008, 0x1030), 'StudyDescription', 'LO', '1',
    'Description or label of the study').

%% Series Module Tags
dicom_tag_def(tag(0x0020, 0x000E), 'SeriesInstanceUID', 'UI', '1',
    'Unique identifier for the series').
dicom_tag_def(tag(0x0008, 0x0060), 'Modality', 'CS', '1',
    'Type of equipment: CT, MR, PT, RTSTRUCT, RTDOSE, etc.').
dicom_tag_def(tag(0x0008, 0x103E), 'SeriesDescription', 'LO', '1',
    'Description of the series').
dicom_tag_def(tag(0x0020, 0x0011), 'SeriesNumber', 'IS', '1',
    'Number identifying the series').

%% Image Pixel Module Tags
dicom_tag_def(tag(0x0028, 0x0010), 'Rows', 'US', '1',
    'Number of rows in the image').
dicom_tag_def(tag(0x0028, 0x0011), 'Columns', 'US', '1',
    'Number of columns in the image').
dicom_tag_def(tag(0x0028, 0x0030), 'PixelSpacing', 'DS', '2',
    'Physical distance between pixel centers [row spacing, column spacing] in mm').
dicom_tag_def(tag(0x0028, 0x0100), 'BitsAllocated', 'US', '1',
    'Number of bits allocated for each pixel sample').
dicom_tag_def(tag(0x0028, 0x0101), 'BitsStored', 'US', '1',
    'Number of bits stored for each pixel sample').
dicom_tag_def(tag(0x0028, 0x0102), 'HighBit', 'US', '1',
    'Most significant bit for pixel sample data').
dicom_tag_def(tag(0x0028, 0x0103), 'PixelRepresentation', 'US', '1',
    'Data representation: 0=unsigned, 1=signed').
dicom_tag_def(tag(0x7FE0, 0x0010), 'PixelData', 'OW', '1',
    'Pixel data for the image').

%% CT Image Module Tags
dicom_tag_def(tag(0x0028, 0x1050), 'WindowCenter', 'DS', '1-n',
    'Window center for display').
dicom_tag_def(tag(0x0028, 0x1051), 'WindowWidth', 'DS', '1-n',
    'Window width for display').
dicom_tag_def(tag(0x0028, 0x1052), 'RescaleIntercept', 'DS', '1',
    'Value b in output = m*SV + b (Hounsfield units)').
dicom_tag_def(tag(0x0028, 0x1053), 'RescaleSlope', 'DS', '1',
    'Value m in output = m*SV + b (Hounsfield units)').
dicom_tag_def(tag(0x0018, 0x0050), 'SliceThickness', 'DS', '1',
    'Nominal slice thickness in mm').
dicom_tag_def(tag(0x0018, 0x0060), 'KVP', 'DS', '1',
    'Peak kilo voltage output of the X-Ray generator').

%% Image Plane Module Tags
dicom_tag_def(tag(0x0020, 0x0032), 'ImagePositionPatient', 'DS', '3',
    'Position of first voxel transmitted (x, y, z) in mm').
dicom_tag_def(tag(0x0020, 0x0037), 'ImageOrientationPatient', 'DS', '6',
    'Direction cosines of row and column axes').
dicom_tag_def(tag(0x0020, 0x1041), 'SliceLocation', 'DS', '1',
    'Relative position of image plane in mm').

%% Frame of Reference Module
dicom_tag_def(tag(0x0020, 0x0052), 'FrameOfReferenceUID', 'UI', '1',
    'Unique identifier for spatial frame of reference').

%% RT Structure Set Module Tags
dicom_tag_def(tag(0x3006, 0x0002), 'StructureSetLabel', 'SH', '1',
    'User-defined label for the structure set').
dicom_tag_def(tag(0x3006, 0x0004), 'StructureSetName', 'LO', '1',
    'User-defined name for the structure set').
dicom_tag_def(tag(0x3006, 0x0008), 'StructureSetDate', 'DA', '1',
    'Date the structure set was created').
dicom_tag_def(tag(0x3006, 0x0020), 'StructureSetROISequence', 'SQ', '1',
    'Sequence of ROI definitions').
dicom_tag_def(tag(0x3006, 0x0039), 'ROIContourSequence', 'SQ', '1',
    'Sequence of ROI contour data').
dicom_tag_def(tag(0x3006, 0x0080), 'ReferencedFrameOfReferenceSequence', 'SQ', '1',
    'Sequence referencing frame of reference').

%% RT ROI Tags
dicom_tag_def(tag(0x3006, 0x0022), 'ROINumber', 'IS', '1',
    'Identification number of the ROI').
dicom_tag_def(tag(0x3006, 0x0024), 'ReferencedFrameOfReferenceUID', 'UI', '1',
    'UID of frame of reference the ROI is defined on').
dicom_tag_def(tag(0x3006, 0x0026), 'ROIName', 'LO', '1',
    'User-defined name for the ROI').
dicom_tag_def(tag(0x3006, 0x0028), 'ROIDescription', 'ST', '1',
    'User-defined description of the ROI').
dicom_tag_def(tag(0x3006, 0x0036), 'ROIGenerationAlgorithm', 'CS', '1',
    'Algorithm used to generate ROI: AUTOMATIC, SEMIAUTOMATIC, MANUAL').
dicom_tag_def(tag(0x3006, 0x002A), 'ROIDisplayColor', 'IS', '3',
    'RGB color for ROI display (0-255 each)').

%% RT Contour Tags
dicom_tag_def(tag(0x3006, 0x0040), 'ContourSequence', 'SQ', '1',
    'Sequence of contour items for an ROI').
dicom_tag_def(tag(0x3006, 0x0042), 'ContourGeometricType', 'CS', '1',
    'Geometric type: POINT, OPEN_PLANAR, OPEN_NONPLANAR, CLOSED_PLANAR').
dicom_tag_def(tag(0x3006, 0x0044), 'ContourSlabThickness', 'DS', '1',
    'Thickness of contour slab in mm').
dicom_tag_def(tag(0x3006, 0x0046), 'NumberOfContourPoints', 'IS', '1',
    'Number of points in the contour').
dicom_tag_def(tag(0x3006, 0x0048), 'ContourNumber', 'IS', '1',
    'Index of the contour within the ROI').
dicom_tag_def(tag(0x3006, 0x0050), 'ContourData', 'DS', '3-3n',
    'Sequence of (x, y, z) triplets defining contour points in mm').
dicom_tag_def(tag(0x3006, 0x0084), 'ReferencedROINumber', 'IS', '1',
    'ROI number this contour sequence belongs to').

%% RT Dose Module Tags
dicom_tag_def(tag(0x3004, 0x0002), 'DoseUnits', 'CS', '1',
    'Units of dose values: GY or RELATIVE').
dicom_tag_def(tag(0x3004, 0x0004), 'DoseType', 'CS', '1',
    'Type of dose: PHYSICAL, EFFECTIVE, ERROR').
dicom_tag_def(tag(0x3004, 0x0006), 'DoseComment', 'LO', '1',
    'User-defined comment about dose').
dicom_tag_def(tag(0x3004, 0x000A), 'DoseSummationType', 'CS', '1',
    'How dose was computed: PLAN, MULTI_PLAN, FRACTION, BEAM, etc.').
dicom_tag_def(tag(0x3004, 0x000C), 'GridFrameOffsetVector', 'DS', '2-n',
    'Z-axis offsets for each frame of multi-frame dose').
dicom_tag_def(tag(0x3004, 0x000E), 'DoseGridScaling', 'DS', '1',
    'Scaling factor to convert stored values to dose units').

%% SOP Common Module
dicom_tag_def(tag(0x0008, 0x0016), 'SOPClassUID', 'UI', '1',
    'Unique identifier for the SOP Class').
dicom_tag_def(tag(0x0008, 0x0018), 'SOPInstanceUID', 'UI', '1',
    'Unique identifier for the SOP Instance').

%% File Meta Information
dicom_tag_def(tag(0x0002, 0x0000), 'FileMetaInformationGroupLength', 'UL', '1',
    'Number of bytes in File Meta Information').
dicom_tag_def(tag(0x0002, 0x0001), 'FileMetaInformationVersion', 'OB', '1',
    'Version of File Meta Information header').
dicom_tag_def(tag(0x0002, 0x0002), 'MediaStorageSOPClassUID', 'UI', '1',
    'SOP Class UID of the dataset').
dicom_tag_def(tag(0x0002, 0x0003), 'MediaStorageSOPInstanceUID', 'UI', '1',
    'SOP Instance UID of the dataset').
dicom_tag_def(tag(0x0002, 0x0010), 'TransferSyntaxUID', 'UI', '1',
    'Transfer Syntax used to encode the dataset').
dicom_tag_def(tag(0x0002, 0x0012), 'ImplementationClassUID', 'UI', '1',
    'Unique identifier for the implementation').


%% =============================================================================
%% TRANSFER SYNTAX DEFINITIONS
%% =============================================================================

%% transfer_syntax(+UID, +Name, +VREncoding, +Endianness, +Compression)
transfer_syntax('1.2.840.10008.1.2', 'Implicit VR Little Endian', implicit, little, none).
transfer_syntax('1.2.840.10008.1.2.1', 'Explicit VR Little Endian', explicit, little, none).
transfer_syntax('1.2.840.10008.1.2.2', 'Explicit VR Big Endian', explicit, big, none).
transfer_syntax('1.2.840.10008.1.2.4.50', 'JPEG Baseline', explicit, little, jpeg_baseline).
transfer_syntax('1.2.840.10008.1.2.4.51', 'JPEG Extended', explicit, little, jpeg_extended).
transfer_syntax('1.2.840.10008.1.2.4.57', 'JPEG Lossless', explicit, little, jpeg_lossless).
transfer_syntax('1.2.840.10008.1.2.4.70', 'JPEG Lossless SV1', explicit, little, jpeg_lossless_sv1).
transfer_syntax('1.2.840.10008.1.2.4.80', 'JPEG-LS Lossless', explicit, little, jpeg_ls_lossless).
transfer_syntax('1.2.840.10008.1.2.4.81', 'JPEG-LS Near-Lossless', explicit, little, jpeg_ls_lossy).
transfer_syntax('1.2.840.10008.1.2.4.90', 'JPEG 2000 Lossless', explicit, little, jpeg2000_lossless).
transfer_syntax('1.2.840.10008.1.2.4.91', 'JPEG 2000 Lossy', explicit, little, jpeg2000_lossy).
transfer_syntax('1.2.840.10008.1.2.5', 'RLE Lossless', explicit, little, rle).


%% =============================================================================
%% SOP CLASS DEFINITIONS
%% =============================================================================

%% sop_class(+UID, +Name, +IODModules)
sop_class('1.2.840.10008.5.1.4.1.1.2', 'CT Image Storage',
    [patient, study, series, image_plane, ct_image]).
sop_class('1.2.840.10008.5.1.4.1.1.481.3', 'RT Structure Set Storage',
    [patient, study, series, rt_structure_set]).
sop_class('1.2.840.10008.5.1.4.1.1.481.2', 'RT Dose Storage',
    [patient, study, series, rt_dose]).
sop_class('1.2.840.10008.5.1.4.1.1.481.5', 'RT Plan Storage',
    [patient, study, series, rt_plan]).
sop_class('1.2.840.10008.5.1.4.1.1.4', 'MR Image Storage',
    [patient, study, series, image_plane, mr_image]).
sop_class('1.2.840.10008.5.1.4.1.1.128', 'PET Image Storage',
    [patient, study, series, image_plane, pet_image]).
