%% =============================================================================
%% BRIMSTONE KNOWLEDGE BASE - Main Module
%% =============================================================================
%% A semantic representation of the brimstone radiotherapy planning system.
%% Inspired by Dublin Core, FOAF, IAO, SWO, and OBI ontologies.
%%
%% Copyright (c) 2007-2021, Derek G. Lane. All rights reserved.
%% U.S. Patent 7,369,645
%% =============================================================================

:- module(brimstone_kb, [
    % Re-export from submodules
    % Software design
    project/2,
    module_def/4,
    source_file/5,
    class/5,
    method/6,
    datatype/4,
    algorithm/5,
    association/5,
    inherits/2,
    defined_in/2,
    has_method/2,
    depends_on/2,

    % DICOM DCG
    dicom_file//1,
    dicom_element//1,
    dicom_tag_def/5,

    % DICOM I/O mappings
    dicom_to_series/2,
    dicom_to_structure/2,
    series_to_dicom/2,

    % Query utilities
    classes_in_namespace/2,
    all_methods/2,
    entity_counts/1
]).

%% Load submodules
:- use_module(software_design).
:- use_module(dicom_dcg).
:- use_module(dicom_io).
:- use_module(kaitai_interpreter).

%% =============================================================================
%% PROJECT METADATA (Dublin Core inspired)
%% =============================================================================

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
%% CROSS-MODULE QUERIES
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

%% Entity statistics
entity_counts(Stats) :-
    findall(F, source_file(F, _, _, _, _), Files),
    length(Files, NumFiles),
    findall(C, class(C, _, _, _, _), Classes),
    length(Classes, NumClasses),
    findall(M, method(_, M, _, _, _, _), Methods),
    length(Methods, NumMethods),
    findall(A, algorithm(A, _, _, _, _), Algorithms),
    length(Algorithms, NumAlgorithms),
    Stats = [
        files(NumFiles),
        classes(NumClasses),
        methods(NumMethods),
        algorithms(NumAlgorithms)
    ].
