#include <gtest/gtest.h>

#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

// Targets a gap the catalogue calls out directly: every existing
// restitution-sensitive test sets restitution explicitly (0.0, 0.5, 1.0), so
// Rigidbody2D's actual default (0.02f, wheel/physics/Rigidbody2D.h) has never
// been exercised by name. This drops a circle with an *untouched* default
// rigidbody (only mass/type set) and checks the rebound velocity is
// consistent with a small (~2%) restitution -- not zero (stuck/no bounce)
// and not anywhere close to the elastic end of the range.
//
// Measures velocity, not position: the rebound height for e=0.02 is on the
// order of e^2 * dropHeight (a few mm for a 0.05m drop), small enough to be
// swamped by the solver's own SLOP-driven position correction if measured
// via peak height. The velocity flip at the contact step is the direct,
// low-noise signal.
namespace
{
    struct DropIds { uint32_t ground, circle; };

    // Ground top face at y=0.5, circle radius 0.5 -> resting contact at y=1.0.
    // Starting 0.05m above that keeps the fall short so the bounce shows up
    // within the first few steps.
    Core::Scene* BuildDefaultRigidbodyDropScene(DropIds& a_Out)
    {
        Core::Scene* scene = QA::CreateHeadlessScene();

        a_Out.ground = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(a_Out.ground).SetPosition(0.0f, 0.0f);
        scene->AddComponent<Collision::BoxCollider2D>(a_Out.ground).SetWidth(10.0f);
        scene->GetComponent<Collision::BoxCollider2D>(a_Out.ground).SetHeight(1.0f);
        Physics::Rigidbody2D& groundRb = scene->AddComponent<Physics::Rigidbody2D>(a_Out.ground);
        groundRb.SetType(Physics::Rigidbody2DType::STATIC);

        a_Out.circle = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(a_Out.circle).SetPosition(0.0f, 1.05f);
        scene->AddComponent<Collision::CircleCollider2D>(a_Out.circle).radius = 0.5f;
        Physics::Rigidbody2D& circleRb = scene->AddComponent<Physics::Rigidbody2D>(a_Out.circle);
        circleRb.SetType(Physics::Rigidbody2DType::DYNAMIC);
        circleRb.SetMass(1.0f);
        // Deliberately NOT touching restitution/linearDamping/angularDamping --
        // this is the point of the test.

        return scene;
    }

    // Runs step-by-step until the circle's vertical velocity flips from
    // negative (falling) to non-negative (bounced this step, since gravity
    // integration and constraint solving both happen within one Update).
    // Returns {impactVelocity (last falling sample), reboundVelocity (the
    // flip-step sample)}, or {0,0} if no flip observed within maxSteps.
    struct BounceSample { float impactVy; float reboundVy; bool found; };

    BounceSample FindFirstBounce(Core::Scene& scene, uint32_t circleId, int maxSteps)
    {
        float lastVy = 0.0f;
        for (int i = 0; i < maxSteps; i++)
        {
            auto snapshots = QA::RunScene(scene, 1);
            for (auto& s : snapshots)
            {
                if (s.entityId != circleId) continue;
                if (lastVy < 0.0f && s.linearVelocity.y >= 0.0f)
                    return { lastVy, s.linearVelocity.y, true };
                lastVy = s.linearVelocity.y;
            }
        }
        return { 0.0f, 0.0f, false };
    }
}

TEST(Analytical, DefaultRestitution_ProducesSmallRebound)
{
    DropIds ids;
    Core::Scene* scene = BuildDefaultRigidbodyDropScene(ids);

    BounceSample bounce = FindFirstBounce(*scene, ids.circle, /*maxSteps*/ 30);
    ASSERT_TRUE(bounce.found) << "circle never made contact with ground within the step budget";
    ASSERT_LT(bounce.impactVy, 0.0f) << "sanity: must have been falling before the bounce";

    float incomingSpeed = -bounce.impactVy;

    // Default restitution is 0.02 -- expect a small but present rebound.
    EXPECT_GT(bounce.reboundVy, 0.0f) << "default restitution (0.02) should produce *some* rebound, not stick";
    EXPECT_LT(bounce.reboundVy, 0.15f * incomingSpeed)
        << "rebound velocity " << bounce.reboundVy << " is too large a fraction of incoming speed "
        << incomingSpeed << " for the default restitution (0.02) -- looks like a much higher effective restitution";

    delete scene;
}

// Sensitivity companion: the same bound (rebound < 15% of incoming speed),
// applied to a body with restitution explicitly set to 1.0 (fully elastic),
// must fail -- proving the bound actually discriminates "small default
// bounce" from "much larger bounce," not just always passing.
TEST(Analytical, DefaultRestitutionBound_CatchesElasticRestitution)
{
    Core::Scene* scene = QA::CreateHeadlessScene();

    uint32_t ground = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(ground).SetPosition(0.0f, 0.0f);
    scene->AddComponent<Collision::BoxCollider2D>(ground).SetWidth(10.0f);
    scene->GetComponent<Collision::BoxCollider2D>(ground).SetHeight(1.0f);
    Physics::Rigidbody2D& groundRb = scene->AddComponent<Physics::Rigidbody2D>(ground);
    groundRb.SetType(Physics::Rigidbody2DType::STATIC);

    uint32_t circle = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(circle).SetPosition(0.0f, 1.05f);
    scene->AddComponent<Collision::CircleCollider2D>(circle).radius = 0.5f;
    Physics::Rigidbody2D& circleRb = scene->AddComponent<Physics::Rigidbody2D>(circle);
    circleRb.SetType(Physics::Rigidbody2DType::DYNAMIC);
    circleRb.SetMass(1.0f);
    circleRb.restitution = 1.0f; // fully elastic, deliberately not the default

    BounceSample bounce = FindFirstBounce(*scene, circle, /*maxSteps*/ 30);
    ASSERT_TRUE(bounce.found);
    ASSERT_LT(bounce.impactVy, 0.0f);

    float incomingSpeed = -bounce.impactVy;
    // With restitution=1.0 the rebound should be close to the full incoming
    // speed, well past the 15% bound used for the default-restitution test.
    EXPECT_GT(bounce.reboundVy, 0.15f * incomingSpeed)
        << "restitution=1.0 rebound (" << bounce.reboundVy << ") unexpectedly stayed under the "
        << "small-bounce bound -- the bound isn't actually discriminating restitution values";

    delete scene;
}
