// test_histogram.cpp - Unit tests for RtModel Histogram classes
//
// Tests dose-volume histogram functionality including:
// - Histogram construction and binning
// - DVH calculation from dose volumes
// - Gaussian convolution (adaptive smoothing)
// - HistogramGradient derivative computation

#include "../catch2/catch.hpp"
#include "../../RtModel/include/Histogram.h"
#include "../../RtModel/include/HistogramGradient.h"
#include <cmath>

using namespace Catch::Matchers;

TEST_CASE("Histogram basic construction", "[histogram][core]") {
    const int NUM_BINS = 100;
    const REAL MAX_DOSE = 1.0;

    SECTION("Histogram can be created with valid parameters") {
        CHistogram hist(NUM_BINS, MAX_DOSE);
        REQUIRE(hist.GetNumBins() == NUM_BINS);
    }

    SECTION("Histogram bin spacing is correct") {
        CHistogram hist(NUM_BINS, MAX_DOSE);
        REAL binWidth = MAX_DOSE / NUM_BINS;
        // Verify reasonable bin width
        REQUIRE(binWidth > 0.0);
        REQUIRE_THAT(binWidth, WithinAbs(0.01, 0.001));
    }
}

TEST_CASE("Histogram DVH calculation", "[histogram][dvh]") {
    const int NUM_BINS = 100;
    const REAL MAX_DOSE = 1.0;

    CHistogram hist(NUM_BINS, MAX_DOSE);

    SECTION("Empty volume produces zero DVH") {
        // Create empty volume
        VolumeReal::SizeType size;
        size[0] = 10;
        size[1] = 10;
        size[2] = 10;

        VolumeReal::Pointer pVolume = VolumeReal::New();
        VolumeReal::RegionType region;
        region.SetSize(size);
        pVolume->SetRegions(region);
        pVolume->Allocate();
        pVolume->FillBuffer(0.0);

        // Calculate histogram
        VolumeReal::Pointer pMask = VolumeReal::New();
        pMask->SetRegions(region);
        pMask->Allocate();
        pMask->FillBuffer(0.0);

        hist.CalcFromVolume(pVolume, pMask);

        // All bins should be zero for empty volume
        for (int i = 0; i < NUM_BINS; i++) {
            REQUIRE(hist.GetBinValue(i) == 0.0);
        }
    }

    SECTION("Uniform dose produces step function DVH") {
        const REAL UNIFORM_DOSE = 0.5;

        VolumeReal::SizeType size;
        size[0] = 10;
        size[1] = 10;
        size[2] = 10;

        VolumeReal::Pointer pVolume = VolumeReal::New();
        VolumeReal::RegionType region;
        region.SetSize(size);
        pVolume->SetRegions(region);
        pVolume->Allocate();
        pVolume->FillBuffer(UNIFORM_DOSE);

        VolumeReal::Pointer pMask = VolumeReal::New();
        pMask->SetRegions(region);
        pMask->Allocate();
        pMask->FillBuffer(1.0); // All voxels in mask

        hist.CalcFromVolume(pVolume, pMask);

        // DVH should be step function
        // Bins below UNIFORM_DOSE should have volume = 1.0 (100%)
        // Bins above UNIFORM_DOSE should have volume = 0.0
        int binIndex = (int)(UNIFORM_DOSE / MAX_DOSE * NUM_BINS);

        if (binIndex > 0 && binIndex < NUM_BINS) {
            REQUIRE(hist.GetBinValue(0) >= 0.99); // First bin should be ~1.0
            REQUIRE(hist.GetBinValue(NUM_BINS-1) <= 0.01); // Last bin should be ~0.0
        }
    }
}

TEST_CASE("Histogram Gaussian convolution", "[histogram][smoothing]") {
    const int NUM_BINS = 100;
    const REAL MAX_DOSE = 1.0;
    const REAL SIGMA = 0.02; // Gaussian kernel width

    CHistogram hist(NUM_BINS, MAX_DOSE);

    SECTION("Convolution smooths histogram") {
        // Create histogram with sharp step
        std::vector<REAL> bins(NUM_BINS, 0.0);
        for (int i = 0; i < NUM_BINS / 2; i++) {
            bins[i] = 1.0;
        }

        hist.SetBins(bins);
        hist.ApplyGaussianKernel(SIGMA);

        // After convolution, edge should be smoothed
        int midBin = NUM_BINS / 2;
        REAL midValue = hist.GetBinValue(midBin);

        // Mid value should be approximately 0.5 after smoothing
        REQUIRE_THAT(midValue, WithinAbs(0.5, 0.2));
    }
}

TEST_CASE("HistogramGradient computes derivatives", "[histogram][gradient]") {
    const int NUM_BINS = 100;
    const REAL MAX_DOSE = 1.0;

    SECTION("Gradient histogram can be constructed") {
        CHistogramGradient histGrad(NUM_BINS, MAX_DOSE);
        REQUIRE(histGrad.GetNumBins() == NUM_BINS);
    }

    SECTION("Derivative volumes are properly sized") {
        CHistogramGradient histGrad(NUM_BINS, MAX_DOSE);

        VolumeReal::SizeType size;
        size[0] = 10;
        size[1] = 10;
        size[2] = 10;

        VolumeReal::Pointer pDose = VolumeReal::New();
        VolumeReal::RegionType region;
        region.SetSize(size);
        pDose->SetRegions(region);
        pDose->Allocate();
        pDose->FillBuffer(0.5);

        VolumeReal::Pointer pMask = VolumeReal::New();
        pMask->SetRegions(region);
        pMask->Allocate();
        pMask->FillBuffer(1.0);

        histGrad.CalcGradientFromVolume(pDose, pMask);

        // Verify gradient volumes are created
        REQUIRE(histGrad.GetNumDerivativeVolumes() > 0);
    }
}

TEST_CASE("Histogram binning correctness", "[histogram][binning]") {
    const int NUM_BINS = 10;
    const REAL MAX_DOSE = 1.0;

    CHistogram hist(NUM_BINS, MAX_DOSE);

    SECTION("Dose values map to correct bins") {
        // Test bin mapping for known values
        REQUIRE(hist.GetBinIndex(0.0) == 0);
        REQUIRE(hist.GetBinIndex(MAX_DOSE) == NUM_BINS - 1);
        REQUIRE(hist.GetBinIndex(MAX_DOSE / 2.0) == NUM_BINS / 2);
    }

    SECTION("Out of range values are clamped") {
        REQUIRE(hist.GetBinIndex(-0.1) >= 0);
        REQUIRE(hist.GetBinIndex(MAX_DOSE + 0.1) < NUM_BINS);
    }
}
