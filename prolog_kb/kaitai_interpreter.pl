%% =============================================================================
%% KAITAI STRUCT INTERPRETER - Meta-programming for Binary Format DCGs
%% =============================================================================
%% Interprets Kaitai Struct specifications and generates Prolog DCG rules
%% that can parse binary data according to the specification.
%%
%% Usage:
%%   ?- kaitai_to_dcg(KaitaiSpec, DCGRules).
%%   ?- parse_with_kaitai(KaitaiSpec, Bytes, ParsedData).
%% =============================================================================

:- module(kaitai_interpreter, [
    % Main API
    kaitai_to_dcg/2,
    parse_with_kaitai/3,
    load_kaitai_file/2,
    % Generated DCG rules (dynamic)
    kaitai_dcg/3,
    % Primitive parsers
    kaitai_u1//1,
    kaitai_u2//1,
    kaitai_u4//1,
    kaitai_s1//1,
    kaitai_s2//1,
    kaitai_s4//1,
    kaitai_f4//1,
    kaitai_f8//1,
    kaitai_bytes//2,
    kaitai_str//3
]).

:- dynamic kaitai_dcg/3.

%% =============================================================================
%% KAITAI STRUCT REPRESENTATION
%% =============================================================================
%%
%% Internal representation of Kaitai Struct specification:
%%
%% kaitai_spec(
%%     meta(Id, Title, Endian, FileExt),
%%     seq(Fields),
%%     types(TypeDefs),
%%     instances(Instances)
%% )
%%
%% field(Id, Type, Options)
%% type_def(Name, Fields)
%% instance(Name, Expr)


%% =============================================================================
%% MAIN API
%% =============================================================================

%% kaitai_to_dcg(+KaitaiSpec, -DCGRules)
%% Converts a Kaitai Struct specification to a list of DCG rule terms
kaitai_to_dcg(KaitaiSpec, DCGRules) :-
    KaitaiSpec = kaitai_spec(Meta, Seq, Types, Instances),
    Meta = meta(Id, _, Endian, _),

    % Generate main parser rule
    generate_seq_rule(Id, Seq, Endian, MainRule),

    % Generate type rules
    findall(TypeRule,
        ( member(type_def(TypeName, TypeFields), Types),
          generate_seq_rule(TypeName, TypeFields, Endian, TypeRule)
        ),
        TypeRules),

    % Generate instance rules
    findall(InstRule,
        ( member(instance(InstName, InstExpr), Instances),
          generate_instance_rule(InstName, InstExpr, InstRule)
        ),
        InstRules),

    append([[MainRule], TypeRules, InstRules], DCGRules).

%% parse_with_kaitai(+KaitaiSpec, +Bytes, -ParsedData)
%% Parses a list of bytes using a Kaitai specification
parse_with_kaitai(KaitaiSpec, Bytes, ParsedData) :-
    kaitai_to_dcg(KaitaiSpec, DCGRules),
    assert_dcg_rules(DCGRules),
    KaitaiSpec = kaitai_spec(meta(Id, _, _, _), _, _, _),
    call_dcg(Id, ParsedData, Bytes, []),
    retract_dcg_rules(DCGRules).

%% load_kaitai_file(+Filename, -KaitaiSpec)
%% Loads and parses a .ksy YAML file
load_kaitai_file(Filename, KaitaiSpec) :-
    read_file_to_string(Filename, Content, []),
    parse_ksy_yaml(Content, KaitaiSpec).


%% =============================================================================
%% DCG RULE GENERATION
%% =============================================================================

%% generate_seq_rule(+Name, +Fields, +Endian, -Rule)
%% Generates a DCG rule for a sequence of fields
generate_seq_rule(Name, Fields, Endian, Rule) :-
    generate_field_parsers(Fields, Endian, FieldParsers, FieldVars),
    build_struct_term(Name, FieldVars, StructTerm),
    build_dcg_body(FieldParsers, DCGBody),
    Rule = (kaitai_dcg(Name, StructTerm, Endian) --> DCGBody).

generate_field_parsers([], _, [], []).
generate_field_parsers([Field|Fields], Endian, [Parser|Parsers], [Var|Vars]) :-
    Field = field(FieldId, FieldType, Options),
    generate_field_parser(FieldId, FieldType, Options, Endian, Parser, Var),
    generate_field_parsers(Fields, Endian, Parsers, Vars).

generate_field_parser(FieldId, Type, Options, Endian, Parser, field(FieldId, Var)) :-
    ( member(repeat(expr, Count), Options)
    -> Parser = kaitai_repeat(Type, Count, Endian, Var)
    ; member(repeat(until, Cond), Options)
    -> Parser = kaitai_repeat_until(Type, Cond, Endian, Var)
    ; member(repeat(eos), Options)
    -> Parser = kaitai_repeat_eos(Type, Endian, Var)
    ; member(size(Size), Options)
    -> Parser = kaitai_sized(Type, Size, Endian, Var)
    ; member(contents(Expected), Options)
    -> Parser = kaitai_contents(Expected, Var)
    ; Parser = kaitai_field(Type, Endian, Var)
    ).

build_struct_term(Name, FieldVars, Term) :-
    Term =.. [Name | FieldVars].

build_dcg_body([], true).
build_dcg_body([P], P).
build_dcg_body([P|Ps], (P, Rest)) :-
    Ps \= [],
    build_dcg_body(Ps, Rest).

%% generate_instance_rule(+Name, +Expr, -Rule)
generate_instance_rule(Name, Expr, Rule) :-
    Rule = (kaitai_instance(Name, Value) :- evaluate_expr(Expr, Value)).


%% =============================================================================
%% FIELD TYPE PARSING
%% =============================================================================

%% kaitai_field(+Type, +Endian, -Value)//
%% Parses a single field based on type
kaitai_field(Type, Endian, Value) -->
    { primitive_type(Type) },
    !,
    kaitai_primitive(Type, Endian, Value).

kaitai_field(Type, Endian, Value) -->
    { compound_type(Type, BaseType, SwitchOn, Cases) },
    !,
    kaitai_switch(SwitchOn, Cases, Endian, Value).

kaitai_field(Type, Endian, Value) -->
    % User-defined type
    kaitai_dcg(Type, Value, Endian).

%% Primitive types
primitive_type(u1). primitive_type(u2). primitive_type(u4). primitive_type(u8).
primitive_type(s1). primitive_type(s2). primitive_type(s4). primitive_type(s8).
primitive_type(f4). primitive_type(f8).
primitive_type(str(_Encoding, _Size)).
primitive_type(strz(_Encoding)).

compound_type(switch_on(SwitchOn, Cases), _, SwitchOn, Cases).


%% =============================================================================
%% PRIMITIVE TYPE PARSERS (Little Endian by default)
%% =============================================================================

%% Unsigned integers
kaitai_u1(V) --> [B], { V = B }.

kaitai_u2(V) --> [B0, B1],
    { V is B0 + B1 * 256 }.

kaitai_u4(V) --> [B0, B1, B2, B3],
    { V is B0 + B1*256 + B2*65536 + B3*16777216 }.

kaitai_u8(V) --> [B0, B1, B2, B3, B4, B5, B6, B7],
    { V is B0 + B1*256 + B2*65536 + B3*16777216 +
           B4*4294967296 + B5*1099511627776 +
           B6*281474976710656 + B7*72057594037927936 }.

%% Signed integers
kaitai_s1(V) --> kaitai_u1(U),
    { U > 127 -> V is U - 256 ; V = U }.

kaitai_s2(V) --> kaitai_u2(U),
    { U > 32767 -> V is U - 65536 ; V = U }.

kaitai_s4(V) --> kaitai_u4(U),
    { U > 2147483647 -> V is U - 4294967296 ; V = U }.

kaitai_s8(V) --> kaitai_u8(U),
    { U > 9223372036854775807 -> V is U - 18446744073709551616 ; V = U }.

%% Floating point (IEEE 754)
kaitai_f4(float32(Bytes)) --> kaitai_bytes(4, Bytes).
%% Full IEEE 754 parsing:
%% kaitai_f4(V) --> [B0, B1, B2, B3],
%%     { bytes_to_float32([B0, B1, B2, B3], V) }.

kaitai_f8(float64(Bytes)) --> kaitai_bytes(8, Bytes).

%% Raw bytes
kaitai_bytes(0, []) --> !.
kaitai_bytes(N, [B|Bs]) -->
    { N > 0, N1 is N - 1 },
    [B],
    kaitai_bytes(N1, Bs).

%% Strings
kaitai_str(Encoding, Size, Str) -->
    kaitai_bytes(Size, Bytes),
    { decode_string(Bytes, Encoding, Str) }.

kaitai_strz(Encoding, Str) -->
    kaitai_bytes_until(0, Bytes),
    [0],  % consume null terminator
    { decode_string(Bytes, Encoding, Str) }.

kaitai_bytes_until(_, []) --> [].
kaitai_bytes_until(Term, []) --> [Term], !.
kaitai_bytes_until(Term, [B|Bs]) -->
    [B],
    { B \= Term },
    kaitai_bytes_until(Term, Bs).

decode_string(Bytes, ascii, Str) :-
    atom_codes(Str, Bytes).
decode_string(Bytes, utf8, Str) :-
    atom_codes(Str, Bytes).  % Simplified


%% =============================================================================
%% BIG ENDIAN VARIANTS
%% =============================================================================

kaitai_u2_be(V) --> [B1, B0],
    { V is B1 * 256 + B0 }.

kaitai_u4_be(V) --> [B3, B2, B1, B0],
    { V is B3*16777216 + B2*65536 + B1*256 + B0 }.

kaitai_s2_be(V) --> kaitai_u2_be(U),
    { U > 32767 -> V is U - 65536 ; V = U }.

kaitai_s4_be(V) --> kaitai_u4_be(U),
    { U > 2147483647 -> V is U - 4294967296 ; V = U }.


%% =============================================================================
%% KAITAI PRIMITIVE DISPATCHER
%% =============================================================================

kaitai_primitive(u1, _, V) --> kaitai_u1(V).
kaitai_primitive(u2, le, V) --> kaitai_u2(V).
kaitai_primitive(u2, be, V) --> kaitai_u2_be(V).
kaitai_primitive(u4, le, V) --> kaitai_u4(V).
kaitai_primitive(u4, be, V) --> kaitai_u4_be(V).
kaitai_primitive(u8, le, V) --> kaitai_u8(V).
kaitai_primitive(s1, _, V) --> kaitai_s1(V).
kaitai_primitive(s2, le, V) --> kaitai_s2(V).
kaitai_primitive(s2, be, V) --> kaitai_s2_be(V).
kaitai_primitive(s4, le, V) --> kaitai_s4(V).
kaitai_primitive(s4, be, V) --> kaitai_s4_be(V).
kaitai_primitive(f4, le, V) --> kaitai_f4(V).
kaitai_primitive(f8, le, V) --> kaitai_f8(V).


%% =============================================================================
%% REPEAT CONSTRUCTS
%% =============================================================================

%% kaitai_repeat(+Type, +Count, +Endian, -Values)//
%% Parse Type Count times
kaitai_repeat(_, 0, _, []) --> !.
kaitai_repeat(Type, N, Endian, [V|Vs]) -->
    { N > 0, N1 is N - 1 },
    kaitai_field(Type, Endian, V),
    kaitai_repeat(Type, N1, Endian, Vs).

%% kaitai_repeat_until(+Type, +Cond, +Endian, -Values)//
%% Parse Type until Cond is true
kaitai_repeat_until(Type, Cond, Endian, [V|Vs]) -->
    kaitai_field(Type, Endian, V),
    ( { evaluate_cond(Cond, V) }
    -> { Vs = [] }
    ; kaitai_repeat_until(Type, Cond, Endian, Vs)
    ).

%% kaitai_repeat_eos(+Type, +Endian, -Values)//
%% Parse Type until end of stream
kaitai_repeat_eos(_, _, []) --> eos, !.
kaitai_repeat_eos(Type, Endian, [V|Vs]) -->
    kaitai_field(Type, Endian, V),
    kaitai_repeat_eos(Type, Endian, Vs).

eos([], []).


%% =============================================================================
%% SWITCH CONSTRUCT
%% =============================================================================

%% kaitai_switch(+SwitchOn, +Cases, +Endian, -Value)//
kaitai_switch(SwitchOn, Cases, Endian, Value) -->
    { evaluate_expr(SwitchOn, SwitchValue) },
    kaitai_switch_case(SwitchValue, Cases, Endian, Value).

kaitai_switch_case(SwitchValue, Cases, Endian, Value) -->
    { member(case(SwitchValue, Type), Cases) },
    !,
    kaitai_field(Type, Endian, Value).
kaitai_switch_case(_, Cases, Endian, Value) -->
    { member(case('_', Type), Cases) },
    !,
    kaitai_field(Type, Endian, Value).
kaitai_switch_case(_, _, _, undefined) --> [].


%% =============================================================================
%% SIZED SUBSTREAM
%% =============================================================================

%% kaitai_sized(+Type, +Size, +Endian, -Value)//
%% Parse Type within Size bytes
kaitai_sized(Type, Size, Endian, Value) -->
    kaitai_bytes(Size, SubBytes),
    { phrase(kaitai_field(Type, Endian, Value), SubBytes) }.


%% =============================================================================
%% CONTENTS (MAGIC) VALIDATION
%% =============================================================================

%% kaitai_contents(+Expected, -Actual)//
kaitai_contents(Expected, Actual) -->
    { atom_codes(Expected, ExpectedBytes), length(ExpectedBytes, Len) },
    kaitai_bytes(Len, ActualBytes),
    { ( ExpectedBytes = ActualBytes
      -> Actual = Expected
      ; throw(kaitai_error(contents_mismatch, Expected, ActualBytes))
      )
    }.


%% =============================================================================
%% EXPRESSION EVALUATION
%% =============================================================================

%% evaluate_expr(+Expr, -Value)
%% Evaluates Kaitai expressions
evaluate_expr(value(V), V) :- !.
evaluate_expr(field(Path), Value) :-
    !,
    get_field_value(Path, Value).
evaluate_expr(A + B, V) :-
    !,
    evaluate_expr(A, VA),
    evaluate_expr(B, VB),
    V is VA + VB.
evaluate_expr(A - B, V) :-
    !,
    evaluate_expr(A, VA),
    evaluate_expr(B, VB),
    V is VA - VB.
evaluate_expr(A * B, V) :-
    !,
    evaluate_expr(A, VA),
    evaluate_expr(B, VB),
    V is VA * VB.
evaluate_expr(A / B, V) :-
    !,
    evaluate_expr(A, VA),
    evaluate_expr(B, VB),
    V is VA / VB.
evaluate_expr(N, N) :- number(N).

%% evaluate_cond(+Cond, +Item)
evaluate_cond(is_delimiter, Item) :-
    Item = item(tag(Group, Element), _, _),
    Group = 0xFFFE,
    Element = 0xE00D.
evaluate_cond(Cond, Item) :-
    call(Cond, Item).


%% =============================================================================
%% DYNAMIC DCG RULE MANAGEMENT
%% =============================================================================

assert_dcg_rules([]).
assert_dcg_rules([Rule|Rules]) :-
    assertz(Rule),
    assert_dcg_rules(Rules).

retract_dcg_rules([]).
retract_dcg_rules([Rule|Rules]) :-
    retract(Rule),
    retract_dcg_rules(Rules).

call_dcg(Name, Value, In, Out) :-
    kaitai_dcg(Name, Value, _Endian, In, Out).


%% =============================================================================
%% KSY YAML PARSING (Simplified)
%% =============================================================================

%% parse_ksy_yaml(+YAMLString, -KaitaiSpec)
%% Parses Kaitai Struct YAML into internal representation
%%
%% Note: This is a simplified parser. For production use,
%% consider using a proper YAML library.

parse_ksy_yaml(YAMLString, KaitaiSpec) :-
    split_yaml_sections(YAMLString, Sections),
    extract_meta(Sections, Meta),
    extract_seq(Sections, Seq),
    extract_types(Sections, Types),
    extract_instances(Sections, Instances),
    KaitaiSpec = kaitai_spec(Meta, Seq, Types, Instances).

split_yaml_sections(_, []).  % Placeholder

extract_meta(Sections, meta(Id, Title, Endian, FileExt)) :-
    ( member(meta(MetaDict), Sections)
    -> get_dict(id, MetaDict, Id),
       get_dict(title, MetaDict, Title, ''),
       get_dict(endian, MetaDict, Endian, le),
       get_dict('file-extension', MetaDict, FileExt, '')
    ; Id = unknown, Title = '', Endian = le, FileExt = ''
    ).

extract_seq(Sections, Seq) :-
    ( member(seq(SeqList), Sections)
    -> maplist(parse_field, SeqList, Seq)
    ; Seq = []
    ).

extract_types(Sections, Types) :-
    ( member(types(TypesDict), Sections)
    -> dict_pairs(TypesDict, _, TypePairs),
       maplist(parse_type_def, TypePairs, Types)
    ; Types = []
    ).

extract_instances(Sections, Instances) :-
    ( member(instances(InstDict), Sections)
    -> dict_pairs(InstDict, _, InstPairs),
       maplist(parse_instance, InstPairs, Instances)
    ; Instances = []
    ).

parse_field(FieldDict, field(Id, Type, Options)) :-
    get_dict(id, FieldDict, Id),
    parse_field_type(FieldDict, Type),
    parse_field_options(FieldDict, Options).

parse_field_type(Dict, Type) :-
    ( get_dict(type, Dict, TypeSpec)
    -> parse_type_spec(TypeSpec, Type)
    ; get_dict(contents, Dict, Contents)
    -> Type = contents(Contents)
    ; Type = bytes
    ).

parse_type_spec(TypeSpec, Type) :-
    ( atom(TypeSpec)
    -> Type = TypeSpec
    ; is_dict(TypeSpec)
    -> ( get_dict('switch-on', TypeSpec, SwitchOn)
       -> get_dict(cases, TypeSpec, Cases),
          dict_pairs(Cases, _, CasePairs),
          maplist(parse_case, CasePairs, ParsedCases),
          Type = switch_on(SwitchOn, ParsedCases)
       ; Type = TypeSpec
       )
    ; Type = TypeSpec
    ).

parse_case(Key-Value, case(Key, Value)).

parse_field_options(Dict, Options) :-
    findall(Opt, parse_single_option(Dict, Opt), Options).

parse_single_option(Dict, repeat(expr, Count)) :-
    get_dict(repeat, Dict, expr),
    get_dict('repeat-expr', Dict, Count).
parse_single_option(Dict, repeat(until, Cond)) :-
    get_dict(repeat, Dict, until),
    get_dict('repeat-until', Dict, Cond).
parse_single_option(Dict, repeat(eos)) :-
    get_dict(repeat, Dict, eos).
parse_single_option(Dict, size(Size)) :-
    get_dict(size, Dict, Size).
parse_single_option(Dict, contents(Contents)) :-
    get_dict(contents, Dict, Contents).
parse_single_option(Dict, if(Cond)) :-
    get_dict(if, Dict, Cond).
parse_single_option(Dict, doc(Doc)) :-
    get_dict(doc, Dict, Doc).

parse_type_def(Name-TypeDict, type_def(Name, Fields)) :-
    ( get_dict(seq, TypeDict, SeqList)
    -> maplist(parse_field, SeqList, Fields)
    ; Fields = []
    ).

parse_instance(Name-InstDict, instance(Name, Expr)) :-
    get_dict(value, InstDict, Expr).

%% Dictionary helpers
get_dict(Key, Dict, Value) :-
    is_dict(Dict),
    get_dict(Key, Dict, Value).
get_dict(Key, Dict, Value, Default) :-
    ( get_dict(Key, Dict, Value) -> true ; Value = Default ).


%% =============================================================================
%% EXAMPLE: GENERATE DCG FROM ENERGY KERNEL SPEC
%% =============================================================================

%% Hardcoded energy kernel specification (matches energy_kernel.ksy)
energy_kernel_spec(KaitaiSpec) :-
    KaitaiSpec = kaitai_spec(
        meta(energy_kernel, 'Brimstone Energy Deposition Kernel', le, dat),
        [   % seq
            field(header, kernel_header, []),
            field(radial_bounds, f4, [repeat(expr, field('header.num_radial') + 1)]),
            field(phi_angles, phi_entry, [repeat(expr, field('header.num_phi'))])
        ],
        [   % types
            type_def(kernel_header, [
                field(magic, contents('BKRN'), []),
                field(version, u2, []),
                field(energy_mv, f4, []),
                field(mu, f4, []),
                field(num_phi, u2, []),
                field(num_theta, u2, []),
                field(num_radial, u2, []),
                field(reserved, bytes, [size(6)])
            ]),
            type_def(phi_entry, [
                field(phi_angle, f4, []),
                field(theta_entries, theta_entry,
                      [repeat(expr, field('_root.header.num_theta'))])
            ]),
            type_def(theta_entry, [
                field(theta_angle, f4, []),
                field(cumulative_energy, f4,
                      [repeat(expr, field('_root.header.num_radial'))])
            ])
        ],
        []  % instances
    ).

%% Generate DCG rules for energy kernel
generate_energy_kernel_dcg(DCGRules) :-
    energy_kernel_spec(Spec),
    kaitai_to_dcg(Spec, DCGRules).


%% =============================================================================
%% EXAMPLE: DICOM ELEMENT SPEC
%% =============================================================================

dicom_element_spec(KaitaiSpec) :-
    KaitaiSpec = kaitai_spec(
        meta(dicom_element, 'DICOM Data Element', le, dcm),
        [   % seq
            field(tag, dicom_tag, []),
            field(vr, value_representation, []),
            field(value, switch_on(field('vr.vr_code'), [
                case('"US"', u2_value),
                case('"SS"', s2_value),
                case('"UL"', u4_value),
                case('"SL"', s4_value),
                case('"FL"', f4_value),
                case('"FD"', f8_value),
                case('"SQ"', sequence_value),
                case('_', raw_value)
            ]), [size(field('vr.value_length'))])
        ],
        [   % types
            type_def(dicom_tag, [
                field(group, u2, []),
                field(element, u2, [])
            ]),
            type_def(value_representation, [
                field(vr_code, str(ascii, 2), []),
                field(length_field, switch_on(field('is_long_vr'), [
                    case(true, long_length),
                    case(false, short_length)
                ]), [])
            ]),
            type_def(short_length, [
                field(length, u2, [])
            ]),
            type_def(long_length, [
                field(reserved, u2, []),
                field(length, u4, [])
            ]),
            type_def(u2_value, [field(value, u2, [])]),
            type_def(s2_value, [field(value, s2, [])]),
            type_def(u4_value, [field(value, u4, [])]),
            type_def(s4_value, [field(value, s4, [])]),
            type_def(f4_value, [field(value, f4, [])]),
            type_def(f8_value, [field(value, f8, [])]),
            type_def(sequence_value, [
                field(items, sequence_item, [repeat(until, is_delimiter)])
            ]),
            type_def(sequence_item, [
                field(item_tag, dicom_tag, []),
                field(item_length, u4, []),
                field(elements, dicom_element, [repeat(eos)])
            ]),
            type_def(raw_value, [
                field(data, bytes, [size(eos)])
            ])
        ],
        [   % instances
            instance(is_long_vr, value(
                member(field('vr.vr_code'),
                       ['"OB"', '"OD"', '"OF"', '"OW"', '"SQ"', '"UN"', '"UT"'])
            )),
            instance(value_length, field('vr.length_field.length'))
        ]
    ).


%% =============================================================================
%% PRETTY PRINTING DCG RULES
%% =============================================================================

%% print_dcg_rules(+Rules)
%% Prints generated DCG rules in Prolog syntax
print_dcg_rules([]).
print_dcg_rules([Rule|Rules]) :-
    portray_clause(Rule),
    nl,
    print_dcg_rules(Rules).

%% Example usage:
%% ?- generate_energy_kernel_dcg(Rules), print_dcg_rules(Rules).
