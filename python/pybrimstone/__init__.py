"""
pybrimstone - Python wrapper for Brimstone radiotherapy inverse planning

Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645

This package provides a Python interface to the Brimstone inverse planning
algorithm for radiotherapy treatment planning.
"""

__version__ = "0.1.0"
__author__ = "Derek G. Lane"
__license__ = "Proprietary - See LICENSE"

# Import core components when Cython extension is built
try:
    from .core import (
        Plan,
        Beam,
        Series,
        Structure,
        PlanOptimizer,
        Prescription,
        KLDivergenceTerm,
    )

    __all__ = [
        "Plan",
        "Beam",
        "Series",
        "Structure",
        "PlanOptimizer",
        "Prescription",
        "KLDivergenceTerm",
    ]
except ImportError as e:
    import warnings

    warnings.warn(
        f"Cython extensions not built. Please run 'pip install -e .' to build. Error: {e}"
    )
    __all__ = []


# TG-263 nomenclature translator (optional, requires PyTorch)
try:
    from .tg263_model import TG263Translator, TG263Classifier

    __all__.extend(["TG263Translator", "TG263Classifier"])
except ImportError:
    # PyTorch not installed - TG-263 features not available
    pass


# Pure-Python brimstone port + hierarchical-Bayes outer loop (always
# available regardless of Cython extension build state).
from .objective_terms import BeamletObjectiveTerm, DoseObjectiveTerm  # noqa: E402
from .course_prior import CoursePriorTerm  # noqa: E402
from .kl_term import KLDivTerm  # noqa: E402
from .prescription import Prescription  # noqa: E402
from .hierarchical_bayes import HierarchicalBayes, pool_phases  # noqa: E402
from .free_energy import (  # noqa: E402
    free_energy_trajectory,
    gaussian_entropy_diag,
    phase_free_energy,
    total_free_energy,
)
from .dvh_uncertainty import (  # noqa: E402
    compute_dose,
    compute_dvh,
    compute_dvh_bands,
    dvh_uncertainty_bands,
    plot_dvh_bands,
    sample_phase_posterior,
)

__all__.extend([
    "BeamletObjectiveTerm",
    "DoseObjectiveTerm",
    "CoursePriorTerm",
    "KLDivTerm",
    "Prescription",
    "HierarchicalBayes",
    "pool_phases",
    "free_energy_trajectory",
    "gaussian_entropy_diag",
    "phase_free_energy",
    "total_free_energy",
    "compute_dose",
    "compute_dvh",
    "compute_dvh_bands",
    "dvh_uncertainty_bands",
    "plot_dvh_bands",
    "sample_phase_posterior",
])
