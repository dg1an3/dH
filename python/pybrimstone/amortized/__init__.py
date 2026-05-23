"""
Amortized course-prior prototype.

Replaces (or hot-starts) the inner CG solve in the hierarchical-Bayes
course/phase loop with a learned feedforward network.

See notes/coursepriorterm_audit.md for what's being approximated.

Phase 0: scaffold only. The network returns zeros (identity prior),
the data module emits dummy tuples, and the train / infer modules are
stubs. Smoke test in tests/amortized/test_smoke.py verifies shapes
flow through. Subsequent phases plug in real content.

Disclaimer: anything that uses outputs from this package in a
decision (objective values, plots, comparisons) MUST mark them as
coming from the amortized path, not the exact one. This package is
prototype-grade and not safe for any planning decision.
"""

from .types import CourseState, PhaseState, Sample
from .config import PrototypeConfig
from .network import AmortizedCoursePrior

__all__ = [
    "CourseState",
    "PhaseState",
    "Sample",
    "PrototypeConfig",
    "AmortizedCoursePrior",
]
