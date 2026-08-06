#include <gtest/gtest.h>

#include "Oracles.h"
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"
#include "physics/Joint2D.h"

using namespace Wheel;

// Targets a catalogued-but-unpinned gap: PrepareJointConstraintSolver
// (wheel/physics/JointConstraintSolver.cpp:23) only sets jtc.go = true when
// `length > joint.distance` -- so a joint whose anchors are *closer together*
// than `distance` (compressed/slack) contributes literally zero constraint
// force, regardless of maxDistanceOnly. The existing
// DistanceJoint_NeverExceedsTargetDistance test (Invariants.cpp) only checks
// the upper bound holds throughout a fall; it never isolates the
// "in the compressed regime this behaves as if the joint doesn't exist"
// claim specifically.
//
// This pins that down directly: while separation is still under `distance`,
// the body's trajectory should match pure free fall bit-for-bit (via the
// same closed-form oracle used for unconstrained bodies) -- proving no
// force is being applied, not just that position stays under a bound.
namespace { constexpr float kEngineGravityY = -9.81f; }

TEST(Invariants, DistanceJoint_CompressedRegimeMatchesFreeFall)
{
    Core::Scene* scene = QA::CreateHeadlessScene();

    uint32_t body = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(body).SetPosition(0.0f, 10.0f);
    scene->AddComponent<Collision::CircleCollider2D>(body).radius = 0.25f;
    Physics::Rigidbody2D& rb = scene->AddComponent<Physics::Rigidbody2D>(body);
    rb.SetType(Physics::Rigidbody2DType::DYNAMIC);
    rb.SetMass(1.0f);
    rb.linearDamping = 0.0f; // isolate gravity/joint interaction from damping

    Physics::DistanceJoint2D& joint = scene->AddComponent<Physics::DistanceJoint2D>(body);
    joint.connectedRigidbody = NO_VALUE;
    joint.otherAnchorPoint = Math::Vector2(0.0f, 10.0f); // fixed world anchor, same as start position
    joint.localAnchorPoint = Math::Vector2(0.0f, 0.0f);
    joint.distance = 2.0f;       // slack: body starts with zero separation
    joint.maxDistanceOnly = false;

    constexpr float dt = 1.0f / 60.0f;
    // 20 steps of free fall covers ~0.55m (0.5 * 9.81 * (20/60)^2), well
    // under the 2.0 target distance -- joint should still be fully slack.
    constexpr int steps = 20;
    auto snapshots = QA::RunScene(*scene, steps, dt);
    ASSERT_EQ(snapshots.size(), 1u);

    QA::OracleResult freeFallResult = QA::AnalyticalFreeFallMatches(
        snapshots[0].position.y, /*y0*/ 10.0f, /*v0y*/ 0.0f, kEngineGravityY, dt, steps, /*tolerance*/ 1e-3f);
    EXPECT_TRUE(freeFallResult.passed)
        << "body should fall unconstrained while separation < joint.distance: " << freeFallResult.message;

    // Continue past the point where separation would exceed `distance` --
    // the joint should now engage and the invariant used by the existing
    // suite (never exceeds target distance) should hold from here on.
    for (int i = 0; i < 120; i++)
    {
        QA::RunScene(*scene, 1);
        Common::Transform2D& t = scene->GetComponent<Common::Transform2D>(body);
        QA::OracleResult jointResult = QA::DistanceJointSatisfied(
            t.GetPosition(), joint.otherAnchorPoint, joint.distance, /*maxDistanceOnly*/ true, /*tolerance*/ 0.1f);
        ASSERT_TRUE(jointResult.passed) << "step " << i << ": " << jointResult.message;
    }

    delete scene;
}

// Sensitivity companion: if the joint wrongly resisted compression (pulled
// the body back toward the anchor even while slack), the trajectory would
// diverge from the closed-form free-fall value -- proving the oracle would
// catch that regression, not just confirm the current (correct-per-code)
// behavior.
TEST(Invariants, DistanceJoint_OracleCatchesSpuriousCompressionForce)
{
    // Hand-crafted "wrong" value: as if a compression force had pulled the
    // body back 0.1 above where unconstrained free fall would put it.
    constexpr float dt = 1.0f / 60.0f;
    constexpr int steps = 20;
    float unconstrainedY = 10.0f + kEngineGravityY * dt * dt * steps * (steps + 1) / 2.0f;
    float wrongY = unconstrainedY + 0.1f; // resisted -- fell less than it should have

    QA::OracleResult result = QA::AnalyticalFreeFallMatches(
        wrongY, /*y0*/ 10.0f, /*v0y*/ 0.0f, kEngineGravityY, dt, steps, /*tolerance*/ 1e-3f);
    EXPECT_FALSE(result.passed);
}
