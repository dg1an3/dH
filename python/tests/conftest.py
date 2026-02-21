"""
Shared fixtures for pheonixrt/pybrimstone tests.

Provides common test data and model instances used across test modules.
"""

import pytest
import sys
from pathlib import Path

import numpy as np
import torch

# Ensure pybrimstone is importable
sys.path.insert(0, str(Path(__file__).parent.parent))


@pytest.fixture
def rng():
    """Deterministic random state for reproducible tests."""
    return np.random.RandomState(42)


@pytest.fixture
def torch_rng():
    """Deterministic torch state for reproducible tests."""
    torch.manual_seed(42)
    return torch.Generator().manual_seed(42)


@pytest.fixture
def char_to_idx():
    """Character vocabulary mapping."""
    from pybrimstone.tg263_model import create_vocabulary
    c2i, _ = create_vocabulary()
    return c2i


@pytest.fixture
def idx_to_char():
    """Reverse character vocabulary mapping."""
    from pybrimstone.tg263_model import create_vocabulary
    _, i2c = create_vocabulary()
    return i2c


@pytest.fixture
def label_maps():
    """(name_to_label, label_to_name) tuple."""
    from pybrimstone.tg263_model import build_tg263_labels
    return build_tg263_labels()


@pytest.fixture
def small_classifier(label_maps):
    """A small TG263Classifier for fast testing."""
    from pybrimstone.tg263_model import TG263Classifier
    name_to_label, _ = label_maps
    return TG263Classifier(
        num_classes=len(name_to_label),
        vocab_size=50,
        embedding_dim=8,
        hidden_dim=16,
        num_layers=1,
        dropout=0.0,
        use_attention=True,
    )


@pytest.fixture
def sample_dose_1d():
    """1D dose array for histogram testing."""
    return np.array([0.0, 0.05, 0.12, 0.18, 0.25, 0.31, 0.40, 0.55, 0.60, 0.70])


@pytest.fixture
def sample_region_1d():
    """1D region mask (all ones) matching sample_dose_1d."""
    return np.ones(10, dtype=np.float64)
