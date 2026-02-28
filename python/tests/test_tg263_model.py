"""
Tests for pybrimstone.tg263_model

Covers: create_vocabulary, build_tg263_labels, CharacterEmbedding,
StructureNameEncoder, AttentionLayer, TG263Classifier, TG263Translator.
"""

import pytest
import torch
import tempfile
from pathlib import Path

from pybrimstone.tg263_model import (
    CharacterEmbedding,
    StructureNameEncoder,
    AttentionLayer,
    TG263Classifier,
    TG263Translator,
    create_vocabulary,
    build_tg263_labels,
)


# ---------------------------------------------------------------------------
# create_vocabulary
# ---------------------------------------------------------------------------

class TestCreateVocabulary:
    def test_returns_two_dicts(self):
        c2i, i2c = create_vocabulary()
        assert isinstance(c2i, dict)
        assert isinstance(i2c, dict)

    def test_special_tokens_present(self):
        c2i, _ = create_vocabulary()
        for tok in ("<PAD>", "<UNK>", "<START>", "<END>"):
            assert tok in c2i, f"Missing special token {tok}"

    def test_pad_is_zero(self):
        c2i, _ = create_vocabulary()
        assert c2i["<PAD>"] == 0

    def test_lowercase_letters(self):
        c2i, _ = create_vocabulary()
        for ch in "abcdefghijklmnopqrstuvwxyz":
            assert ch in c2i

    def test_digits(self):
        c2i, _ = create_vocabulary()
        for ch in "0123456789":
            assert ch in c2i

    def test_punctuation(self):
        c2i, _ = create_vocabulary()
        for ch in " _-()[].,/":
            assert ch in c2i

    def test_round_trip(self):
        c2i, i2c = create_vocabulary()
        for char, idx in c2i.items():
            assert i2c[idx] == char

    def test_no_duplicate_indices(self):
        c2i, _ = create_vocabulary()
        indices = list(c2i.values())
        assert len(indices) == len(set(indices))


# ---------------------------------------------------------------------------
# build_tg263_labels
# ---------------------------------------------------------------------------

class TestBuildTG263Labels:
    def test_returns_two_dicts(self):
        n2l, l2n = build_tg263_labels()
        assert isinstance(n2l, dict)
        assert isinstance(l2n, dict)

    def test_count(self):
        n2l, l2n = build_tg263_labels()
        # The code defines 47 standard names
        assert len(n2l) == 47
        assert len(l2n) == 47

    def test_round_trip(self):
        n2l, l2n = build_tg263_labels()
        for name, label in n2l.items():
            assert l2n[label] == name

    def test_known_structures(self):
        n2l, _ = build_tg263_labels()
        for name in ("Brain", "Heart", "Lung_L", "Prostate", "GTV", "PTV", "Body"):
            assert name in n2l, f"Expected {name} in TG-263 labels"

    def test_labels_zero_indexed(self):
        n2l, _ = build_tg263_labels()
        assert set(n2l.values()) == set(range(47))


# ---------------------------------------------------------------------------
# CharacterEmbedding
# ---------------------------------------------------------------------------

class TestCharacterEmbedding:
    def test_output_shape(self):
        emb = CharacterEmbedding(vocab_size=50, embedding_dim=16)
        x = torch.randint(0, 50, (3, 10))  # batch=3, seq_len=10
        out = emb(x)
        assert out.shape == (3, 10, 16)

    def test_padding_idx_zero(self):
        emb = CharacterEmbedding(vocab_size=50, embedding_dim=16)
        zeros = torch.zeros(1, 5, dtype=torch.long)
        out = emb(zeros)
        # Padding embedding should be all zeros
        assert torch.allclose(out, torch.zeros_like(out))


# ---------------------------------------------------------------------------
# StructureNameEncoder
# ---------------------------------------------------------------------------

class TestStructureNameEncoder:
    def test_output_shapes_no_lengths(self):
        enc = StructureNameEncoder(embedding_dim=16, hidden_dim=32, num_layers=1)
        x = torch.randn(2, 8, 16)  # batch=2, seq=8, emb=16
        outputs, final_hidden = enc(x)
        assert outputs.shape == (2, 8, 64)  # hidden_dim * 2 (bidirectional)
        assert final_hidden.shape == (2, 64)

    def test_output_shapes_with_lengths(self):
        enc = StructureNameEncoder(embedding_dim=16, hidden_dim=32, num_layers=1)
        x = torch.randn(3, 10, 16)
        lengths = torch.tensor([10, 7, 3])
        outputs, final_hidden = enc(x, lengths)
        assert outputs.shape[0] == 3
        assert outputs.shape[2] == 64
        assert final_hidden.shape == (3, 64)


# ---------------------------------------------------------------------------
# AttentionLayer
# ---------------------------------------------------------------------------

class TestAttentionLayer:
    def test_output_shapes(self):
        attn = AttentionLayer(hidden_dim=32)
        enc_out = torch.randn(2, 8, 64)  # batch=2, seq=8, hidden*2=64
        context, weights = attn(enc_out)
        assert context.shape == (2, 64)
        assert weights.shape == (2, 8)

    def test_weights_sum_to_one(self):
        attn = AttentionLayer(hidden_dim=32)
        enc_out = torch.randn(1, 5, 64)
        _, weights = attn(enc_out)
        assert torch.allclose(weights.sum(dim=1), torch.tensor([1.0]), atol=1e-5)

    def test_mask(self):
        attn = AttentionLayer(hidden_dim=32)
        enc_out = torch.randn(1, 5, 64)
        mask = torch.tensor([[True, True, True, False, False]])
        _, weights = attn(enc_out, mask)
        # Masked positions should have ~0 weight
        assert weights[0, 3].item() < 1e-6
        assert weights[0, 4].item() < 1e-6
        assert torch.allclose(weights.sum(dim=1), torch.tensor([1.0]), atol=1e-5)


# ---------------------------------------------------------------------------
# TG263Classifier
# ---------------------------------------------------------------------------

class TestTG263Classifier:
    def test_forward_shape(self, small_classifier, label_maps):
        model = small_classifier
        n2l, _ = label_maps
        batch_size = 4
        seq_len = 20
        x = torch.randint(0, 50, (batch_size, seq_len))
        lengths = torch.tensor([20, 15, 10, 5])

        out = model(x, lengths)
        assert "logits" in out
        assert out["logits"].shape == (batch_size, len(n2l))
        assert "attention_weights" in out
        assert out["attention_weights"].shape[0] == batch_size

    def test_forward_no_attention(self, label_maps):
        n2l, _ = label_maps
        model = TG263Classifier(
            num_classes=len(n2l), vocab_size=50,
            embedding_dim=8, hidden_dim=16,
            num_layers=1, use_attention=False,
        )
        x = torch.randint(0, 50, (2, 10))
        out = model(x)
        assert out["logits"].shape == (2, len(n2l))
        assert out["attention_weights"] is None

    def test_forward_no_lengths(self, small_classifier, label_maps):
        n2l, _ = label_maps
        x = torch.randint(0, 50, (2, 10))
        out = small_classifier(x)
        assert out["logits"].shape == (2, len(n2l))


# ---------------------------------------------------------------------------
# TG263Translator
# ---------------------------------------------------------------------------

class TestTG263Translator:
    @pytest.fixture
    def translator(self, label_maps, char_to_idx, idx_to_char):
        n2l, l2n = label_maps
        model = TG263Classifier(
            num_classes=len(n2l), vocab_size=len(char_to_idx),
            embedding_dim=8, hidden_dim=16,
            num_layers=1, dropout=0.0, use_attention=True,
        )
        return TG263Translator(
            model=model,
            label_to_name=l2n,
            name_to_label=n2l,
            char_to_idx=char_to_idx,
            idx_to_char=idx_to_char,
            device="cpu",
        )

    def test_encode_string_length(self, translator):
        indices, length = translator.encode_string("Heart")
        assert indices.shape == (64,)  # max_length
        assert length == 5
        # Remaining should be padding (0)
        assert (indices[5:] == 0).all()

    def test_encode_string_lowercases(self, translator):
        indices1, _ = translator.encode_string("HEART")
        indices2, _ = translator.encode_string("heart")
        assert torch.equal(indices1, indices2)

    def test_encode_string_truncation(self, translator):
        long_name = "a" * 100
        indices, length = translator.encode_string(long_name)
        assert length == 64
        assert indices.shape == (64,)

    def test_predict_returns_list(self, translator):
        results = translator.predict("left parotid", top_k=3)
        assert isinstance(results, list)
        assert len(results) <= 3
        for name, conf in results:
            assert isinstance(name, str)
            assert 0.0 <= conf <= 1.0

    def test_predict_confidences_sum_approximately(self, translator):
        # top_k = all classes should sum to ~1
        n2l = translator.name_to_label
        results = translator.predict("heart", top_k=len(n2l))
        total = sum(c for _, c in results)
        assert abs(total - 1.0) < 1e-4

    def test_predict_batch(self, translator):
        names = ["left parotid", "right lung", "heart"]
        results = translator.predict_batch(names, top_k=2)
        assert len(results) == 3
        for preds in results:
            assert len(preds) <= 2

    def test_save_load_round_trip(self, translator):
        with tempfile.TemporaryDirectory() as tmpdir:
            path = str(Path(tmpdir) / "model.pth")
            translator.save(path)

            loaded = TG263Translator.load(path, device="cpu")
            assert loaded.model.num_classes == translator.model.num_classes
            assert loaded.label_to_name == translator.label_to_name
            assert loaded.name_to_label == translator.name_to_label
            assert loaded.char_to_idx == translator.char_to_idx

            # Same prediction from loaded model
            orig = translator.predict("heart", top_k=1)
            reloaded = loaded.predict("heart", top_k=1)
            assert orig[0][0] == reloaded[0][0]
            assert abs(orig[0][1] - reloaded[0][1]) < 1e-5
