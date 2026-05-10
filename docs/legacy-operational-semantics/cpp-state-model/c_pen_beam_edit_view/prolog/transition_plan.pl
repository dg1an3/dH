%% Source: PenBeamEdit/TransitionPlan.txt
%%
%% This module is unique to the PenBeamEdit deliverable: the
%% TransitionPlan.txt file is itself a sequence of architectural
%% migration steps with X marks indicating completion. Per
%% DEVELOPMENT_TIMELINE.md Part 7 Direction C, transcribing the
%% migration plan as an LTS makes the architectural moves visible
%% as checkable artifacts in the same Prolog model that captures
%% the runtime behavior.
%%
%% Each step is one labeled transition over the codebase's evolution
%% state. Done/not-done marks reflect what the source actually
%% implements (which agrees with the X marks for steps 1-2 but
%% diverges for step 3).

:- module(transition_plan, [
    transition_step/2,                  % +N, -StepDescriptor
    step_done/1,                        % +N -- marked done in TransitionPlan.txt
    step_actually_implemented/1,        % +N -- evidence in source
    plan_state/1                        % -DescriptorList
]).

%% C++: PenBeamEdit/TransitionPlan.txt:1-7  the migration plan body.

transition_step(1, step{
    label:       'cimage_to_cvolume',
    description: 'Change CImage -> CVolume',
    target_files: ['PenBeamEditView.cpp', 'PenBeamEditView.h'],
    rationale:   'Migrate from CImage primitive to the project CVolume<TYPE> template'
}).

transition_step(2, step{
    label:       'gui_base_cdib_adopted',
    description: 'Use CDib from GUI_BASE',
    target_files: ['PenBeamEditView.cpp:163-176', 'PenBeamEditView.h:8'],
    rationale:   'Replace ad-hoc bitmap loading with the project CDib helper'
}).

transition_step(3, step{
    label:       'plan_extended_with_beam_doses',
    description: 'Add to CPlan to contain beam doses, beam weights, and total dose',
    target_files: ['PenBeamEdit.cpp:286-311', 'VSIM_MODEL/Plan.h:43-46', 'VSIM_MODEL/Beam.h:78-79'],
    rationale:   'CPlan needs to hold the precomputed dose matrix; CBeam needs per-beam dose + weight'
}).

transition_step(4, step{
    label:       'gl_slice_blender_added',
    description: 'Add an OpenGLRenderer to draw an image (a single slice of a CVolume), allow blending of two images using a color lookup and alpha value',
    target_files: ['(not implemented in PenBeamEdit; eventually realized in Brimstone/PlanarView.cpp::DrawImages)'],
    rationale:   'Slice rendering with LUT blending of CT and dose'
}).

transition_step(5, step{
    label:       'dvh_view_attached',
    description: 'Add a DVH graph view (use Microsoft Chart control?)',
    target_files: ['PenBeamEditView.h:53', 'PenBeamEditView.cpp:159-160, :189-218'],
    rationale:   'Display cumulative dose-volume histogram for the structure region'
}).

%% Marked done in TransitionPlan.txt with leading "X" -- only steps 1-2.
step_done(1).
step_done(2).

%% Steps actually implemented in the source (evidence trail):
%%   1: ✓ CVolume<short> volume in CSeries.h:33; CVolume<double> in CPlan.h
%%   2: ✓ CDib at PenBeamEditView.cpp:163
%%   3: ✓ Implemented in PenBeamEdit.cpp:286-311 (loop sums weighted beam
%%        doses into pPlan->GetDoseMatrix). The X mark in TransitionPlan.txt
%%        is missing despite this being done.
%%   4: ✗ NOT in PenBeamEdit. Eventually realized in modern
%%        Brimstone/PlanarView.cpp::DrawImages with LUT + alpha blending
%%        (per DEVELOPMENT_TIMELINE.md Part 7 Direction C algorithm table).
%%   5: ✓ Implemented via CGraph m_graph + CHistogram m_histogram in
%%        PenBeamEditView. The "Microsoft Chart control?" question mark
%%        was answered "no -- use the project's own CGraph".
step_actually_implemented(1).
step_actually_implemented(2).
step_actually_implemented(3).
step_actually_implemented(5).

%% Compositional view: the migration plan as a list of done/pending steps.
plan_state(L) :-
    findall(N-Done-Implemented,
            ( transition_step(N, _),
              ( step_done(N) -> Done = marked_done ; Done = marked_pending ),
              ( step_actually_implemented(N) -> Implemented = present ; Implemented = absent )),
            L).

%% ============================================================================
%% Notable transcription point
%% ============================================================================
%%
%% Step 3 ("Add to CPlan to contain beam doses, beam weights, and total
%% dose") is implemented in source but NOT marked X in the transition plan.
%% This is the most interesting transcription artifact -- the plan and the
%% code disagree on what's done.
%%
%% Three possibilities:
%%   a) The plan was authored before step 3 landed and never updated.
%%   b) Step 3 was implemented partially and the author reserved the X
%%      for "fully complete with weights ALSO held on CBeam" -- which is
%%      what happens at PenBeamEdit.cpp:300 (SetWeight on each pencil beam).
%%   c) Step 3 is considered "done elsewhere" (in the modern Brimstone
%%      where CPlan structure is more elaborate).
%%
%% The LTS records both `step_done` (from the X marks) and
%% `step_actually_implemented` (from source-evidence) so cross-class
%% queries can detect the discrepancy.
