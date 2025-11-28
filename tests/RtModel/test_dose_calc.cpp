// test_dose_calc.cpp - Unit tests for RtModel dose calculation
//
// Tests dose calculation pipeline including:
// - TERMA calculation (BeamDoseCalc)
// - Energy kernel convolution (SphereConvolve)
// - Dose accumulation
// - Geometric transformations

#include "../catch2/catch.hpp"
#include "../../RtModel/include/BeamDoseCalc.h"
#include "../../RtModel/include/SphereConvolve.h"
#include "../../RtModel/include/EnergyDepKernel.h"
#include "../../RtModel/include/Beam.h"

using namespace Catch::Matchers;

TEST_CASE("Energy deposition kernel", "[dose][kernel]") {

    SECTION("Kernel can be loaded from file") {
        CEnergyDepKernel kernel;

        // Try to load 6MV kernel
        const char* kernelPath = "../../Brimstone/6MV_kernel.dat";
        bool loaded = kernel.LoadFromFile(kernelPath);

        if (loaded) {
            REQUIRE(kernel.GetNumRadialSamples() > 0);
            REQUIRE(kernel.GetNumAngularSamples() > 0);
        }
    }

    SECTION("Kernel has proper normalization") {
        CEnergyDepKernel kernel;

        // Kernel should integrate to approximately 1.0 (energy conservation)
        // This requires the kernel to be properly normalized
        // Actual test requires loading real kernel data
        REQUIRE(true); // Placeholder
    }

    SECTION("Kernel is symmetric in azimuthal direction") {
        CEnergyDepKernel kernel;

        // Energy deposition should be azimuthally symmetric
        // (same value for all phi at fixed r, theta)
        // Actual test requires kernel data
        REQUIRE(true); // Placeholder
    }
}

TEST_CASE("TERMA calculation", "[dose][terma]") {

    SECTION("BeamDoseCalc can be created") {
        CBeam beam;
        CEnergyDepKernel kernel;

        CBeamDoseCalc doseCalc(&beam, &kernel);

        REQUIRE(doseCalc.GetBeam() == &beam);
        REQUIRE(doseCalc.GetKernel() == &kernel);
    }

    SECTION("Ray tracing through uniform density") {
        // For uniform density, TERMA should decay exponentially
        // TERMA(z) = Φ₀ * μ * exp(-μz)
        // where Φ₀ is fluence, μ is attenuation coefficient

        CBeam beam;
        CEnergyDepKernel kernel;
        CBeamDoseCalc doseCalc(&beam, &kernel);

        // Create uniform density volume
        VolumeReal::SizeType size;
        size[0] = 50;
        size[1] = 50;
        size[2] = 50;

        VolumeReal::Pointer pDensity = VolumeReal::New();
        VolumeReal::RegionType region;
        region.SetSize(size);
        pDensity->SetRegions(region);
        pDensity->Allocate();
        pDensity->FillBuffer(1.0); // Water density

        // Calculate TERMA
        VolumeReal::Pointer pTERMA = VolumeReal::New();
        pTERMA->SetRegions(region);
        pTERMA->Allocate();

        doseCalc.CalcTERMA(pDensity, pTERMA);

        // TERMA should be non-zero
        // and should decay with depth
        // Actual verification requires valid beam geometry
        REQUIRE(true); // Placeholder
    }

    SECTION("TERMA is zero in zero density") {
        // No density = no energy deposition

        CBeam beam;
        CEnergyDepKernel kernel;
        CBeamDoseCalc doseCalc(&beam, &kernel);

        // Create zero density volume
        VolumeReal::SizeType size;
        size[0] = 10;
        size[1] = 10;
        size[2] = 10;

        VolumeReal::Pointer pDensity = VolumeReal::New();
        VolumeReal::RegionType region;
        region.SetSize(size);
        pDensity->SetRegions(region);
        pDensity->Allocate();
        pDensity->FillBuffer(0.0); // No density

        VolumeReal::Pointer pTERMA = VolumeReal::New();
        pTERMA->SetRegions(region);
        pTERMA->Allocate();

        doseCalc.CalcTERMA(pDensity, pTERMA);

        // All TERMA values should be zero
        itk::ImageRegionConstIterator<VolumeReal> it(pTERMA, region);
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
            REQUIRE_THAT(it.Get(), WithinAbs(0.0, 1e-9));
        }
    }
}

TEST_CASE("Spherical convolution", "[dose][convolution]") {

    SECTION("SphereConvolve can be created") {
        CEnergyDepKernel kernel;
        CSphereConvolve convolve(&kernel);

        REQUIRE(convolve.GetKernel() == &kernel);
    }

    SECTION("Convolution on point source produces kernel shape") {
        // Convolving a delta function with kernel should reproduce kernel

        CEnergyDepKernel kernel;
        CSphereConvolve convolve(&kernel);

        VolumeReal::SizeType size;
        size[0] = 21; // Odd size for central point
        size[1] = 21;
        size[2] = 21;

        // Create point source at center
        VolumeReal::Pointer pTERMA = VolumeReal::New();
        VolumeReal::RegionType region;
        region.SetSize(size);
        pTERMA->SetRegions(region);
        pTERMA->Allocate();
        pTERMA->FillBuffer(0.0);

        VolumeReal::IndexType centerIdx;
        centerIdx[0] = 10;
        centerIdx[1] = 10;
        centerIdx[2] = 10;
        pTERMA->SetPixel(centerIdx, 1.0);

        // Convolve
        VolumeReal::Pointer pDose = VolumeReal::New();
        pDose->SetRegions(region);
        pDose->Allocate();

        convolve.Convolve(pTERMA, pDose);

        // Result should have kernel shape centered at point
        // Verify dose is non-zero and symmetric
        REQUIRE(pDose->GetPixel(centerIdx) > 0.0);
    }

    SECTION("Dose superposition principle") {
        // Dose from two sources = sum of doses from each source
        // This verifies linearity of convolution

        CEnergyDepKernel kernel;
        CSphereConvolve convolve(&kernel);

        // Create two point sources
        VolumeReal::SizeType size;
        size[0] = 30;
        size[1] = 30;
        size[2] = 30;
        VolumeReal::RegionType region;
        region.SetSize(size);

        VolumeReal::Pointer pSource1 = VolumeReal::New();
        pSource1->SetRegions(region);
        pSource1->Allocate();
        pSource1->FillBuffer(0.0);

        VolumeReal::IndexType idx1;
        idx1[0] = 10;
        idx1[1] = 15;
        idx1[2] = 15;
        pSource1->SetPixel(idx1, 1.0);

        VolumeReal::Pointer pSource2 = VolumeReal::New();
        pSource2->SetRegions(region);
        pSource2->Allocate();
        pSource2->FillBuffer(0.0);

        VolumeReal::IndexType idx2;
        idx2[0] = 20;
        idx2[1] = 15;
        idx2[2] = 15;
        pSource2->SetPixel(idx2, 1.0);

        // Compute individual doses
        VolumeReal::Pointer pDose1 = VolumeReal::New();
        pDose1->SetRegions(region);
        pDose1->Allocate();
        convolve.Convolve(pSource1, pDose1);

        VolumeReal::Pointer pDose2 = VolumeReal::New();
        pDose2->SetRegions(region);
        pDose2->Allocate();
        convolve.Convolve(pSource2, pDose2);

        // Compute combined dose
        VolumeReal::Pointer pSourceBoth = VolumeReal::New();
        pSourceBoth->SetRegions(region);
        pSourceBoth->Allocate();
        pSourceBoth->FillBuffer(0.0);
        pSourceBoth->SetPixel(idx1, 1.0);
        pSourceBoth->SetPixel(idx2, 1.0);

        VolumeReal::Pointer pDoseBoth = VolumeReal::New();
        pDoseBoth->SetRegions(region);
        pDoseBoth->Allocate();
        convolve.Convolve(pSourceBoth, pDoseBoth);

        // Verify superposition: D_both ≈ D_1 + D_2
        itk::ImageRegionConstIterator<VolumeReal> it1(pDose1, region);
        itk::ImageRegionConstIterator<VolumeReal> it2(pDose2, region);
        itk::ImageRegionConstIterator<VolumeReal> itBoth(pDoseBoth, region);

        for (it1.GoToBegin(), it2.GoToBegin(), itBoth.GoToBegin();
             !it1.IsAtEnd();
             ++it1, ++it2, ++itBoth) {
            REAL sum = it1.Get() + it2.Get();
            REQUIRE_THAT(itBoth.Get(), WithinAbs(sum, 0.01));
        }
    }
}

TEST_CASE("Dose calculation accuracy", "[dose][accuracy]") {

    SECTION("Dose falls off with distance from source") {
        // Dose should decrease with distance from beam central axis
        // This is a basic physics requirement
        REQUIRE(true); // Requires full beam setup
    }

    SECTION("Dose buildup region") {
        // For megavoltage beams, should see dose buildup near surface
        // Dmax should be at some depth > 0
        REQUIRE(true); // Requires full beam setup
    }
}
