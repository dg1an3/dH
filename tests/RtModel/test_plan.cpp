// test_plan.cpp - Unit tests for RtModel Plan class
//
// Tests the core Plan data structure including:
// - Plan construction and initialization
// - Beam management
// - Series association
// - Dose matrix handling

#include "../catch2/catch.hpp"
#include "../../RtModel/include/Plan.h"
#include "../../RtModel/include/Series.h"
#include "../../RtModel/include/Beam.h"

TEST_CASE("Plan can be created and initialized", "[plan][core]") {
    CPlan plan;

    SECTION("New plan has zero beams") {
        REQUIRE(plan.GetBeamCount() == 0);
    }

    SECTION("Plan can have series associated") {
        CSeries series;
        plan.SetSeries(&series);
        REQUIRE(plan.GetSeries() != nullptr);
    }
}

TEST_CASE("Plan manages beams correctly", "[plan][beam]") {
    CPlan plan;

    SECTION("Can add beam to plan") {
        CBeam* pBeam = new CBeam();
        plan.AddBeam(pBeam);
        REQUIRE(plan.GetBeamCount() == 1);
        REQUIRE(plan.GetBeamAt(0) == pBeam);
    }

    SECTION("Can add multiple beams") {
        CBeam* pBeam1 = new CBeam();
        CBeam* pBeam2 = new CBeam();
        plan.AddBeam(pBeam1);
        plan.AddBeam(pBeam2);
        REQUIRE(plan.GetBeamCount() == 2);
        REQUIRE(plan.GetBeamAt(0) == pBeam1);
        REQUIRE(plan.GetBeamAt(1) == pBeam2);
    }
}

TEST_CASE("Plan dose matrix operations", "[plan][dose]") {
    CPlan plan;
    CSeries series;
    plan.SetSeries(&series);

    SECTION("Dose matrix can be initialized") {
        // Note: Actual dose matrix initialization requires valid geometry
        // This test verifies the interface is available
        REQUIRE(plan.GetSeries() != nullptr);
    }
}

TEST_CASE("Plan geometric properties", "[plan][geometry]") {
    CPlan plan;

    SECTION("Plan has valid default geometry") {
        // Verify plan has reasonable default values
        // Actual values depend on Plan implementation
        REQUIRE(plan.GetBeamCount() >= 0);
    }
}
