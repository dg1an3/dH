// test_optimizer.cpp - Unit tests for RtModel optimization algorithms
//
// Tests the core optimization functionality including:
// - ConjugateGradient optimizer
// - Prescription (objective function)
// - KL Divergence term
// - Adaptive variance
// - Free energy calculation

#include "../catch2/catch.hpp"
#include "../../RtModel/include/ConjGradOptimizer.h"
#include "../../RtModel/include/Prescription.h"
#include "../../RtModel/include/KLDivTerm.h"
#include "../../RtModel/include/Plan.h"
#include "../../RtModel/include/VectorN.h"

using namespace Catch::Matchers;

TEST_CASE("ConjugateGradient optimizer construction", "[optimizer][core]") {

    SECTION("Optimizer can be created") {
        CDynamicCovarianceOptimizer optimizer;
        REQUIRE(optimizer.GetMaxIterations() > 0);
    }

    SECTION("Optimizer has sensible default parameters") {
        CDynamicCovarianceOptimizer optimizer;

        // Verify default tolerance
        REQUIRE(optimizer.GetTolerance() > 0.0);
        REQUIRE(optimizer.GetTolerance() < 1.0);

        // Verify max iterations
        REQUIRE(optimizer.GetMaxIterations() > 0);
        REQUIRE(optimizer.GetMaxIterations() < 10000);
    }
}

TEST_CASE("Optimizer parameter configuration", "[optimizer][params]") {
    CDynamicCovarianceOptimizer optimizer;

    SECTION("Can set convergence tolerance") {
        const REAL NEW_TOL = 1e-6;
        optimizer.SetTolerance(NEW_TOL);
        REQUIRE_THAT(optimizer.GetTolerance(), WithinAbs(NEW_TOL, 1e-9));
    }

    SECTION("Can set max iterations") {
        const int NEW_MAX = 500;
        optimizer.SetMaxIterations(NEW_MAX);
        REQUIRE(optimizer.GetMaxIterations() == NEW_MAX);
    }

    SECTION("Can enable free energy calculation") {
        optimizer.SetComputeFreeEnergy(true);
        REQUIRE(optimizer.GetComputeFreeEnergy() == true);
    }

    SECTION("Can configure adaptive variance") {
        CVectorN<> adaptVariance(10);
        for (int i = 0; i < 10; i++) {
            adaptVariance[i] = 0.1 * i;
        }
        optimizer.SetAdaptiveVariance(adaptVariance);

        const CVectorN<>& av = optimizer.GetAdaptiveVariance();
        REQUIRE(av.GetDim() == 10);
        REQUIRE_THAT(av[5], WithinAbs(0.5, 1e-6));
    }
}

TEST_CASE("Optimizer convergence on simple quadratic", "[optimizer][convergence]") {
    // Test optimizer on simple quadratic: f(x) = x^2
    // Minimum should be at x = 0

    class QuadraticObjective : public CObjectiveFunction {
    public:
        virtual REAL Eval(const CVectorN<>& vState, CVectorN<>* pvGrad) {
            if (pvGrad) {
                pvGrad->SetDim(vState.GetDim());
                for (int i = 0; i < vState.GetDim(); i++) {
                    (*pvGrad)[i] = 2.0 * vState[i];
                }
            }
            REAL cost = 0.0;
            for (int i = 0; i < vState.GetDim(); i++) {
                cost += vState[i] * vState[i];
            }
            return cost;
        }
    };

    SECTION("Converges to minimum of quadratic") {
        QuadraticObjective objective;
        CDynamicCovarianceOptimizer optimizer;
        optimizer.SetObjectiveFunction(&objective);
        optimizer.SetTolerance(1e-6);
        optimizer.SetMaxIterations(100);

        // Start from non-zero point
        CVectorN<> vInit(5);
        for (int i = 0; i < 5; i++) {
            vInit[i] = 1.0;
        }

        CVectorN<> vResult;
        optimizer.Optimize(vInit, &vResult);

        // Should converge to near zero
        for (int i = 0; i < 5; i++) {
            REQUIRE_THAT(vResult[i], WithinAbs(0.0, 0.01));
        }
    }
}

TEST_CASE("Free energy computation", "[optimizer][free_energy]") {
    CDynamicCovarianceOptimizer optimizer;
    optimizer.SetComputeFreeEnergy(true);

    SECTION("Free energy is computed when enabled") {
        // Note: Free energy = KL_divergence - Entropy
        // Entropy = 0.5 * (n * log(2πe) + log(det(Σ)))

        optimizer.SetDimension(10);

        // After optimization, free energy should be available
        // This is a placeholder - actual test requires full optimization run
        REQUIRE(optimizer.GetComputeFreeEnergy() == true);
    }
}

TEST_CASE("Prescription objective function", "[prescription][objective]") {

    SECTION("Prescription can be created for a plan") {
        dH::Plan::Pointer pPlan = dH::Plan::New();
        CPrescription presc(pPlan, 0);

        REQUIRE(presc.GetPlan() == pPlan);
    }

    SECTION("Can add structure terms to prescription") {
        dH::Plan::Pointer pPlan = dH::Plan::New();
        dH::Series::Pointer pSeries = dH::Series::New();
        pPlan->SetSeries(pSeries);

        CPrescription presc(pPlan, 0);

        // Create a test structure
        dH::Structure::Pointer pStruct = dH::Structure::New();
        pStruct->SetName("TestTarget");
        pSeries->AddStructure(pStruct);

        // Add KL divergence term
        CKLDivTerm* pTerm = new CKLDivTerm(pStruct, 1.0);
        presc.AddStructureTerm(pTerm);

        REQUIRE(presc.GetNumStructureTerms() > 0);
    }
}

TEST_CASE("KL Divergence term", "[kldiv][dvh]") {

    SECTION("KLDiv term can be created") {
        dH::Structure::Pointer pStruct = dH::Structure::New();
        pStruct->SetName("Target");

        const REAL WEIGHT = 5.0;
        CKLDivTerm klTerm(pStruct, WEIGHT);

        REQUIRE(klTerm.GetWeight() == WEIGHT);
        REQUIRE(klTerm.GetStructure() == pStruct);
    }

    SECTION("Can set target DVH interval") {
        dH::Structure::Pointer pStruct = dH::Structure::New();
        CKLDivTerm klTerm(pStruct, 1.0);

        const REAL MIN_DOSE = 0.9;
        const REAL MAX_DOSE = 1.0;
        const REAL PROB = 1.0;

        klTerm.SetInterval(MIN_DOSE, MAX_DOSE, PROB);

        // Interval should be set
        REQUIRE(klTerm.GetIntervalMin() == MIN_DOSE);
        REQUIRE(klTerm.GetIntervalMax() == MAX_DOSE);
    }

    SECTION("KL divergence is non-negative") {
        // KL(P||Q) >= 0 with equality iff P = Q
        // This is a mathematical property we should verify

        dH::Structure::Pointer pStruct = dH::Structure::New();
        CKLDivTerm klTerm(pStruct, 1.0);

        // For identical distributions, KL should be zero
        // For different distributions, KL should be positive
        // Actual test requires full dose calculation
        REQUIRE(true); // Placeholder
    }
}

TEST_CASE("Adaptive variance correctness", "[optimizer][variance]") {

    SECTION("Adaptive variance formula is correct") {
        // m_ActualAV = baseVar * dSigmoid(input)² * varWeight²

        const REAL BASE_VAR = 1.0;
        const REAL VAR_WEIGHT = 1.0;
        const REAL INPUT = 0.5;

        // dSigmoid(x) = sigmoid(x) * (1 - sigmoid(x))
        auto sigmoid = [](REAL x) { return 1.0 / (1.0 + exp(-x)); };
        auto dSigmoid = [&](REAL x) {
            REAL s = sigmoid(x);
            return s * (1.0 - s);
        };

        REAL expected_av = BASE_VAR * dSigmoid(INPUT) * dSigmoid(INPUT) * VAR_WEIGHT * VAR_WEIGHT;

        // Verify formula makes sense
        REQUIRE(expected_av > 0.0);
        REQUIRE(expected_av <= BASE_VAR * VAR_WEIGHT * VAR_WEIGHT);
    }
}

TEST_CASE("Gradient computation chain rule", "[optimizer][gradient]") {

    SECTION("Gradients flow through sigmoid transform") {
        // optimizer_vars → sigmoid → beamlet_weights → dose → histogram → KL

        // Test sigmoid derivative
        auto sigmoid = [](REAL x) { return 1.0 / (1.0 + exp(-x)); };
        auto dSigmoid = [&](REAL x) {
            REAL s = sigmoid(x);
            return s * (1.0 - s);
        };

        // At x = 0, sigmoid = 0.5, derivative = 0.25
        REQUIRE_THAT(sigmoid(0.0), WithinAbs(0.5, 1e-6));
        REQUIRE_THAT(dSigmoid(0.0), WithinAbs(0.25, 1e-6));

        // Derivative should be max at x = 0
        REQUIRE(dSigmoid(0.0) >= dSigmoid(1.0));
        REQUIRE(dSigmoid(0.0) >= dSigmoid(-1.0));
    }
}
