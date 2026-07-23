"""
Tests for pybrimstone.tg263_training

Covers: SyntheticDataGenerator, TG263Dataset, collate_fn, train_model (smoke).
"""

import pytest
import torch
from torch.utils.data import DataLoader

from pybrimstone.tg263_model import (
    TG263Classifier,
    create_vocabulary,
    build_tg263_labels,
)
from pybrimstone.tg263_training import (
    SyntheticDataGenerator,
    TG263Dataset,
    collate_fn,
    train_model,
)


# ---------------------------------------------------------------------------
# SyntheticDataGenerator
# ---------------------------------------------------------------------------

class TestSyntheticDataGenerator:
    @pytest.fixture
    def generator(self):
        _, l2n = build_tg263_labels()
        return SyntheticDataGenerator(list(l2n.values()))

    def test_generate_variations_returns_list(self, generator):
        variations = generator.generate_variations("Heart", num_variations=5)
        assert isinstance(variations, list)
        assert len(variations) <= 5

    def test_generate_variations_includes_original(self, generator):
        variations = generator.generate_variations("Heart", num_variations=20)
        # At least one variation should match or be a case variant
        lower_vars = [v.lower().strip() for v in variations]
        assert "heart" in lower_vars

    def test_generate_variations_laterality(self, generator):
        variations = generator.generate_variations("Parotid_L", num_variations=20)
        # Should produce laterality variants
        has_left_variant = any(
            "left" in v.lower() or "_l" in v.lower() or " l" in v.lower()
            for v in variations
        )
        assert has_left_variant

    def test_generate_dataset(self, generator):
        dataset = generator.generate_dataset(num_variations_per_name=3)
        assert len(dataset) > 0
        # Each entry is (clinical_name, standard_name)
        for clinical, standard in dataset[:10]:
            assert isinstance(clinical, str)
            assert isinstance(standard, str)

    def test_generate_dataset_covers_all_classes(self, generator):
        dataset = generator.generate_dataset(num_variations_per_name=5)
        standards = set(s for _, s in dataset)
        _, l2n = build_tg263_labels()
        # Should cover most classes (some might get deduplicated away)
        assert len(standards) >= len(l2n) * 0.8


# ---------------------------------------------------------------------------
# TG263Dataset
# ---------------------------------------------------------------------------

class TestTG263Dataset:
    @pytest.fixture
    def dataset(self, char_to_idx):
        n2l, _ = build_tg263_labels()
        examples = [
            ("left parotid", "Parotid_L"),
            ("RIGHT LUNG", "Lung_R"),
            ("heart", "Heart"),
        ]
        return TG263Dataset(examples, char_to_idx, n2l, max_length=32)

    def test_len(self, dataset):
        assert len(dataset) == 3

    def test_getitem_shapes(self, dataset):
        char_indices, label, length = dataset[0]
        assert char_indices.shape == (32,)
        assert isinstance(label, int)
        assert isinstance(length, int)

    def test_getitem_padding(self, dataset):
        char_indices, _, length = dataset[0]
        # After the actual characters, rest should be 0 (pad)
        assert (char_indices[length:] == 0).all()

    def test_getitem_label_valid(self, dataset):
        n2l, _ = build_tg263_labels()
        for i in range(len(dataset)):
            _, label, _ = dataset[i]
            assert 0 <= label < len(n2l)

    def test_unknown_standard_name_raises(self, char_to_idx):
        n2l, _ = build_tg263_labels()
        bad_examples = [("foo", "NonExistentStructure")]
        ds = TG263Dataset(bad_examples, char_to_idx, n2l)
        with pytest.raises(ValueError, match="Unknown standard name"):
            ds[0]

    def test_lowercases_input(self, dataset, char_to_idx):
        # "RIGHT LUNG" should be encoded as "right lung"
        char_indices, _, _ = dataset[1]
        # The 'r' index for lowercase
        r_idx = char_to_idx.get("r")
        assert char_indices[0].item() == r_idx


# ---------------------------------------------------------------------------
# collate_fn
# ---------------------------------------------------------------------------

class TestCollateFn:
    def test_batch_shapes(self, char_to_idx):
        n2l, _ = build_tg263_labels()
        examples = [
            ("heart", "Heart"),
            ("lung left", "Lung_L"),
        ]
        ds = TG263Dataset(examples, char_to_idx, n2l, max_length=16)
        batch = [ds[0], ds[1]]
        char_indices, labels, lengths = collate_fn(batch)

        assert char_indices.shape == (2, 16)
        assert labels.shape == (2,)
        assert lengths.shape == (2,)
        assert labels.dtype == torch.long
        assert lengths.dtype == torch.long


# ---------------------------------------------------------------------------
# train_model (smoke test)
# ---------------------------------------------------------------------------

class TestTrainModel:
    def test_one_epoch_smoke(self, char_to_idx):
        """Verify training runs for 1 epoch without error."""
        n2l, l2n = build_tg263_labels()

        # Tiny synthetic dataset
        gen = SyntheticDataGenerator(list(l2n.values()))
        data = gen.generate_dataset(num_variations_per_name=2)

        # Split
        train_data = data[:len(data)//2]
        val_data = data[len(data)//2:]

        if not train_data or not val_data:
            pytest.skip("Not enough data generated")

        train_ds = TG263Dataset(train_data, char_to_idx, n2l, max_length=32)
        val_ds = TG263Dataset(val_data, char_to_idx, n2l, max_length=32)

        train_loader = DataLoader(train_ds, batch_size=8, shuffle=True, collate_fn=collate_fn)
        val_loader = DataLoader(val_ds, batch_size=8, shuffle=False, collate_fn=collate_fn)

        model = TG263Classifier(
            num_classes=len(n2l),
            vocab_size=len(char_to_idx),
            embedding_dim=8,
            hidden_dim=16,
            num_layers=1,
            dropout=0.0,
            use_attention=True,
        )

        history = train_model(
            model, train_loader, val_loader,
            num_epochs=1, learning_rate=0.01, device="cpu", patience=1,
        )

        assert "train_loss" in history
        assert len(history["train_loss"]) == 1
        assert history["train_loss"][0] > 0
        assert "val_acc" in history
