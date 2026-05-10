# CTreatmentMachine Model Summary

`CTreatmentMachine` is the simplest member of the VSIM_MODEL cohort — seven public fields, no setters, observable in name only. The trivial glue-light case. This summary is interesting mainly for the **two preserved quirks** the LTS surfaces, both consequences of the public-field / no-setter / no-fire-on-change layout.

## Record schema

![CTreatmentMachine record schema](diagrams/bdd_record.svg)

> Source: [`diagrams/bdd_record.puml`](diagrams/bdd_record.puml)

| Field | Type | C++ source |
|---|---|---|
| `manufacturer` | `CString` | [`TreatmentMachine.h:26`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.h#L26) |
| `model` | `CString` | [`TreatmentMachine.h:27`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.h#L27) |
| `serial_number` | `CString` | [`TreatmentMachine.h:28`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.h#L28) |
| `sad` | `double` (default 700.0) | [`TreatmentMachine.h:31`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.h#L31) |
| `scd` | `double` (default 300.0) | [`TreatmentMachine.h:32`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.h#L32) |
| `sid` | `double` (default 1400.0 = `m_SAD * 2.0`) | [`TreatmentMachine.h:33`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.h#L33) |
| `projection` | `CMatrix<4>` (computed in ctor only) | [`TreatmentMachine.h:36`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.h#L36) |

## Mutator alphabet

The "mutators" don't exist as real C++ methods — they correspond to **direct field writes** by clients (every field is `public`). The model normalizes them as named events so cross-class queries have something to reference:

| Event | C++ effect | Fires `GetChangeEvent()`? |
|---|---|---|
| `set_manufacturer(S)` etc. (six identification + parameter setters) | direct field assignment | ✗ never |
| `serialize_load` | hydrates 6 of 7 fields from archive (omits `projection`) | ✗ |
| `serialize_save` | reads 6 of 7 fields | ✗ |

## Two preserved quirks

**Quirk 1: projection-stale-after-field-mutation.**
[`TreatmentMachine.cpp:23-29`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.cpp#L23) computes `m_projection = CreateProjection(m_SCD, m_SID)` in the constructor. There is no setter and no recompute logic. If a client mutates `m_SCD` or `m_SID` directly (which they can, since fields are public), `m_projection` is **silently stale**. No invariant guards against it. Preserved verbatim per the cpp-state-model fidelity rule. Reviewers driving SAD/SCD/SID changes against any LTS that consumes `c_treatment_machine` should expect to observe stale projection until something else re-computes it.

**Quirk 2: projection-not-serialized.**
[`TreatmentMachine.cpp:37-45`](../../../../VSIM_OGL/VSIM_MODEL/TreatmentMachine.cpp#L37) lists six `SERIALIZE_VALUE` calls — `manufacturer`, `model`, `serial_number`, `sad`, `scd`, `sid`. **`m_projection` is not in the list.** So on `serialize_load`, the projection field stays at whatever it was *before the load* (which, post-construction, is the constructor default `CreateProjection(300.0, 1400.0)`). If the saved instance had different SCD/SID, the loaded post-state has *correct* SAD/SCD/SID values but a *constructor-default* projection — visibly inconsistent. Preserved verbatim.

Both quirks exist because the original author made `m_projection` derivable but didn't enforce that derivation discipline anywhere.

## Source mapping

| Prolog construct | C++ source |
|---|---|
| `c_treatment_machine_record:c_treatment_machine_schema/1` | `TreatmentMachine.h:17-40` |
| `c_treatment_machine_record:c_treatment_machine_initial/1` | `TreatmentMachine.cpp:23-29` (default ctor) |
| `c_treatment_machine_record:apply_mutator(_, serialize_load, _)` | `TreatmentMachine.cpp:37-45` (6 of 7 fields, projection omitted) |

## Cross-language references

**No direct modern descendant.** The Brimstone 2008+ rewrite eliminates the standalone `CTreatmentMachine` abstraction. Machine geometry is encoded implicitly inside `BeamDoseCalc` and the `EnergyDepKernel` data files ([`Brimstone/6MV_kernel.dat`](../../../../Brimstone/6MV_kernel.dat), [`Brimstone/15MV_kernel.dat`](../../../../Brimstone/15MV_kernel.dat), [`Brimstone/2MV_kernel.dat`](../../../../Brimstone/2MV_kernel.dat)). The kernels bake in source-axis distance, beam energy spectrum, and dose deposition characteristics monolithically.

This is one of the cleaner cases of **intentional divergence** in the historical → modern transition: the older code's "machine geometry as parameters on a class" abstraction was abandoned in favor of "machine geometry as opaque kernel data." The two stale-projection quirks above were among the motivating frustrations — neither exists in the modern stack because there's no separate object that could go stale.

A formal bisimilarity claim against the modern stack therefore has to be **at the application level** (does Brimstone's beam geometry agree with VSIM's at the same SAD/SCD?) rather than the class level. The kernel files are the modern equivalent of a `CTreatmentMachine` instance configured for a specific machine.
