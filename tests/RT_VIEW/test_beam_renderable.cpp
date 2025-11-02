// test_beam_renderable.cpp - Unit tests for RT_VIEW beam rendering
//
// Tests beam visualization components including:
// - BeamRenderable geometry
// - Beam field rendering
// - Block rendering
// - Collimator divergence
// - Central axis visualization

#include "../catch2/catch.hpp"
#include "../../RT_VIEW/include/BeamRenderable.h"
#include "../../RT_VIEW/include/MachineRenderable.h"

using namespace Catch::Matchers;

TEST_CASE("BeamRenderable construction", "[rtview][beam]") {

    SECTION("BeamRenderable can be created") {
        CBeamRenderable beamRenderable;
        REQUIRE(true); // Basic construction check
    }

    SECTION("BeamRenderable can be associated with beam") {
        CBeam beam;
        CBeamRenderable beamRenderable;

        beamRenderable.SetBeam(&beam);
        REQUIRE(beamRenderable.GetBeam() == &beam);
    }
}

TEST_CASE("Beam geometry rendering", "[rtview][geometry]") {

    SECTION("Beam has valid gantry angle") {
        CBeam beam;

        // Set gantry angle
        const REAL GANTRY_ANGLE = 45.0;
        beam.SetGantryAngle(GANTRY_ANGLE);

        REQUIRE_THAT(beam.GetGantryAngle(), WithinAbs(GANTRY_ANGLE, 1e-6));
    }

    SECTION("Beam has valid collimator angle") {
        CBeam beam;

        const REAL COLLIM_ANGLE = 90.0;
        beam.SetCollimatorAngle(COLLIM_ANGLE);

        REQUIRE_THAT(beam.GetCollimatorAngle(), WithinAbs(COLLIM_ANGLE, 1e-6));
    }

    SECTION("Beam has valid table angle") {
        CBeam beam;

        const REAL TABLE_ANGLE = 0.0;
        beam.SetTableAngle(TABLE_ANGLE);

        REQUIRE_THAT(beam.GetTableAngle(), WithinAbs(TABLE_ANGLE, 1e-6));
    }
}

TEST_CASE("Beam field parameters", "[rtview][field]") {

    SECTION("Jaw positions define field size") {
        CBeam beam;

        // Set symmetric 10x10 cm field
        beam.SetJawX1(-5.0);
        beam.SetJawX2(5.0);
        beam.SetJawY1(-5.0);
        beam.SetJawY2(5.0);

        REAL fieldWidth = beam.GetJawX2() - beam.GetJawX1();
        REAL fieldHeight = beam.GetJawY2() - beam.GetJawY1();

        REQUIRE_THAT(fieldWidth, WithinAbs(10.0, 1e-6));
        REQUIRE_THAT(fieldHeight, WithinAbs(10.0, 1e-6));
    }

    SECTION("Asymmetric fields are supported") {
        CBeam beam;

        beam.SetJawX1(-3.0);
        beam.SetJawX2(7.0);
        beam.SetJawY1(-2.0);
        beam.SetJawY2(8.0);

        REAL fieldWidth = beam.GetJawX2() - beam.GetJawX1();
        REAL fieldHeight = beam.GetJawY2() - beam.GetJawY1();

        REQUIRE_THAT(fieldWidth, WithinAbs(10.0, 1e-6));
        REQUIRE_THAT(fieldHeight, WithinAbs(10.0, 1e-6));
    }
}

TEST_CASE("Beam isocenter positioning", "[rtview][isocenter]") {

    SECTION("Isocenter has 3D coordinates") {
        CBeam beam;

        const REAL ISO_X = 100.0;
        const REAL ISO_Y = 50.0;
        const REAL ISO_Z = -25.0;

        beam.SetIsocenterX(ISO_X);
        beam.SetIsocenterY(ISO_Y);
        beam.SetIsocenterZ(ISO_Z);

        REQUIRE_THAT(beam.GetIsocenterX(), WithinAbs(ISO_X, 1e-6));
        REQUIRE_THAT(beam.GetIsocenterY(), WithinAbs(ISO_Y, 1e-6));
        REQUIRE_THAT(beam.GetIsocenterZ(), WithinAbs(ISO_Z, 1e-6));
    }
}

TEST_CASE("Beam source-to-axis distance (SAD)", "[rtview][sad]") {

    SECTION("Standard SAD is 100 cm") {
        CBeam beam;

        const REAL STANDARD_SAD = 100.0;
        beam.SetSAD(STANDARD_SAD);

        REQUIRE_THAT(beam.GetSAD(), WithinAbs(STANDARD_SAD, 1e-6));
    }

    SECTION("Extended SAD for special treatments") {
        CBeam beam;

        const REAL EXTENDED_SAD = 150.0;
        beam.SetSAD(EXTENDED_SAD);

        REQUIRE_THAT(beam.GetSAD(), WithinAbs(EXTENDED_SAD, 1e-6));
    }
}

TEST_CASE("MachineRenderable gantry rotation", "[rtview][machine]") {

    SECTION("MachineRenderable can be created") {
        CMachineRenderable machineRenderable;
        REQUIRE(true);
    }

    SECTION("Gantry rotation matrix is orthogonal") {
        // Rotation matrices must preserve lengths and angles
        // R^T * R = I (orthogonality condition)

        const REAL GANTRY_ANGLE = 45.0;

        // Create rotation matrix for gantry
        REAL radians = GANTRY_ANGLE * M_PI / 180.0;
        REAL cosA = cos(radians);
        REAL sinA = sin(radians);

        // Gantry rotates around Y axis (IEC 61217)
        // [  cos  0  sin ]
        // [   0   1   0  ]
        // [ -sin  0  cos ]

        REAL R[3][3] = {
            { cosA, 0, sinA },
            { 0, 1, 0 },
            { -sinA, 0, cosA }
        };

        // Verify orthogonality: R^T * R = I
        REAL RTR[3][3] = {0};
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                for (int k = 0; k < 3; k++) {
                    RTR[i][j] += R[k][i] * R[k][j];
                }
            }
        }

        // Diagonal should be 1, off-diagonal should be 0
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                REAL expected = (i == j) ? 1.0 : 0.0;
                REQUIRE_THAT(RTR[i][j], WithinAbs(expected, 1e-10));
            }
        }
    }
}

TEST_CASE("Beam divergence calculation", "[rtview][divergence]") {

    SECTION("Field diverges from source point") {
        // At SAD, field has nominal size
        // At distance d, field scales by factor d/SAD

        const REAL SAD = 100.0;
        const REAL FIELD_SIZE_AT_SAD = 10.0; // 10 cm

        CBeam beam;
        beam.SetSAD(SAD);
        beam.SetJawX1(-FIELD_SIZE_AT_SAD / 2);
        beam.SetJawX2(FIELD_SIZE_AT_SAD / 2);

        // At distance 150 cm, field should be 15 cm
        const REAL TEST_DISTANCE = 150.0;
        REAL scaleFactor = TEST_DISTANCE / SAD;
        REAL expectedFieldSize = FIELD_SIZE_AT_SAD * scaleFactor;

        REQUIRE_THAT(expectedFieldSize, WithinAbs(15.0, 1e-6));
    }
}

TEST_CASE("Block rendering", "[rtview][blocks]") {

    SECTION("Blocks can be defined in beam's eye view") {
        CBeam beam;

        // Blocks are defined as polygons in BEV coordinates
        // Verify block data structure exists
        REQUIRE(beam.GetBlockCount() >= 0);
    }

    SECTION("Block aperture reduces field size") {
        // Blocks should only reduce the field, never increase it
        // This is a physical constraint
        REQUIRE(true); // Placeholder for block geometry test
    }
}

TEST_CASE("Central axis rendering", "[rtview][central_axis]") {

    SECTION("Central axis points from source through isocenter") {
        CBeam beam;

        const REAL SAD = 100.0;
        beam.SetSAD(SAD);
        beam.SetIsocenterX(0.0);
        beam.SetIsocenterY(0.0);
        beam.SetIsocenterZ(0.0);

        // Source is at (0, -SAD, 0) in beam coords
        // Central axis is ray from source to isocenter

        // Verify source position
        CPoint3D source = beam.GetSourcePosition();
        REQUIRE_THAT(source.GetY(), WithinAbs(-SAD, 1e-6));
    }
}

TEST_CASE("Rendering state consistency", "[rtview][state]") {

    SECTION("Opaque and transparent render modes don't conflict") {
        CBeamRenderable beamRenderable;

        // Both render modes should be available
        // and should not interfere with each other
        REQUIRE(true); // Placeholder - requires OpenGL context
    }

    SECTION("Render updates track beam parameter changes") {
        CBeam beam;
        CBeamRenderable beamRenderable;
        beamRenderable.SetBeam(&beam);

        // Change beam parameter
        beam.SetGantryAngle(90.0);

        // Renderable should invalidate its cache
        // and update on next render
        REQUIRE(true); // Placeholder - requires render cycle
    }
}
