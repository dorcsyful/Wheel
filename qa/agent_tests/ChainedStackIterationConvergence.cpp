#include <gtest/gtest.h>

#include "Oracles.h"
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

// Targets MAX_CONSTRAINT_ITERATION (wheel/core/Globals.h): a 6->1 mutation of
// that constant survived the mutation pass with the existing suite because
// every existing resting scenario is a single body on static ground (one
// manifold, no cross-manifold coupling, so it converges in a single sweep
// regardless of the iteration budget). A chain of three dynamic circles
// resting on static ground has TWO manifolds coupled through the middle
// body (ground-A, A-B, B-C is three manifolds/two couplings) -- resolving
// gravity's velocity injection into a consistent zero-velocity equilibrium
// in a single physics step requires several sequential-impulse sweeps
// across those manifolds. With only 1 iteration, a single one-directional
// pass leaves a large residual velocity on the bodies furthest from ground;
// with 6 it converges close to zero within the same single step.
namespace
{
    struct ChainIds { uint32_t ground, a, b, c; };

    Core::Scene* BuildRestingChain(ChainIds& a_Out)
    {
        Core::Scene* scene = QA::CreateHeadlessScene();

        a_Out.ground = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(a_Out.ground).SetPosition(0.0f, 0.0f);
        scene->AddComponent<Collision::BoxCollider2D>(a_Out.ground).SetWidth(10.0f);
        scene->GetComponent<Collision::BoxCollider2D>(a_Out.ground).SetHeight(1.0f);
        Physics::Rigidbody2D& groundRb = scene->AddComponent<Physics::Rigidbody2D>(a_Out.ground);
        groundRb.SetType(Physics::Rigidbody2DType::STATIC);

        // Ground top face is at y=0.5. Each circle has radius 0.5, stacked with
        // a tiny overlap (0.001) so all three contacts are live from step 0 --
        // no impact transient, purely a standing-start equilibrium problem.
        auto addCircle = [&](float y) {
            uint32_t id = scene->AddEntity();
            scene->AddComponent<Common::Transform2D>(id).SetPosition(0.0f, y);
            scene->AddComponent<Collision::CircleCollider2D>(id).radius = 0.5f;
            Physics::Rigidbody2D& rb = scene->AddComponent<Physics::Rigidbody2D>(id);
            rb.SetType(Physics::Rigidbody2DType::DYNAMIC);
            rb.SetMass(1.0f);
            rb.restitution = 0.0f;
            rb.linearDamping = 0.0f;
            rb.angularDamping = 0.0f;
            return id;
        };

        a_Out.a = addCircle(0.999f);
        a_Out.b = addCircle(1.998f);
        a_Out.c = addCircle(2.997f);

        return scene;
    }
}

TEST(Invariants, ChainedStack_VelocityConvergesWithinFewSteps)
{
    ChainIds ids;
    Core::Scene* scene = BuildRestingChain(ids);

    // Gravity injects ~0.1635 m/s of downward velocity into every dynamic
    // body each step (9.81 * 1/60). A converged solver should cancel nearly
    // all of it, every step, for every body in the chain -- not just the
    // one closest to the immovable ground.
    std::vector<Math::Vector2> linearSamples;
    std::vector<float> angularSamples;
    for (int i = 0; i < 5; i++)
    {
        auto snapshots = QA::RunScene(*scene, 1);
        for (auto& s : snapshots)
        {
            if (s.entityId == ids.a || s.entityId == ids.b || s.entityId == ids.c)
            {
                linearSamples.push_back(s.linearVelocity);
                angularSamples.push_back(s.angularVelocity);
            }
        }
    }

    QA::OracleResult result = QA::RestingBodyStaysAtRest(linearSamples, angularSamples, /*epsilon*/ 0.05f);
    EXPECT_TRUE(result.passed) << result.message;

    delete scene;
}

// Sensitivity companion: hand-crafted samples at the magnitude a single
// one-directional sequential-impulse sweep would leave behind on the
// far-from-ground body of a three-link chain (a large fraction of one
// frame's gravity kick, ~0.1635 m/s) -- well past the 0.05 epsilon above,
// proving the oracle would flag an under-converged chain rather than only
// ever passing.
TEST(Invariants, ChainedStack_OracleCatchesUnderConvergedResidual)
{
    std::vector<Math::Vector2> linearSamples = {
        Math::Vector2(0.0f, 0.0f),
        Math::Vector2(0.0f, -0.12f), // residual left on the topmost body after a single under-iterated sweep
    };
    std::vector<float> angularSamples = { 0.0f, 0.0f };

    QA::OracleResult result = QA::RestingBodyStaysAtRest(linearSamples, angularSamples, /*epsilon*/ 0.05f);
    EXPECT_FALSE(result.passed);
}
