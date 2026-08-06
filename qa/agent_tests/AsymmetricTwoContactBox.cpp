#include <gtest/gtest.h>

#include "Oracles.h"
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "collision/BoxBoxCollision2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

// Targets Solve2ContactConstraint's Case 1/2/3/4 branch selection
// (wheel/physics/CollisionConstraintSolver.cpp:184-206) and the
// Matrix2x2::Inverse singularity guard it depends on. The existing
// agent_tests/StackedBoxes.cpp sample only asserted final penetration depth
// on a *symmetric* two-contact landing (both contacts carry equal impulse,
// which happens to be exercised correctly even by several of the mutated
// branch conditions) -- it doesn't distinguish Case 1's asymmetric impulse
// split from a wrong one.
//
// This test offsets the falling box's center of mass so a flat landing
// needs a genuinely *unequal* impulse split between its two contacts to
// reach static equilibrium (a lever: the corner closer to the (offset) COM
// must carry more of the load). A wrong Case selection or a sign/threshold
// error in the branch conditions will leave the box either still rotating
// (angularVelocity != 0 at "rest") or with one corner sunk past slop --
// both directly observable, unlike final position alone.
namespace
{
    struct AsymmetricBoxIds { uint32_t ground, box; };

    // centerOfMass is offset toward +x by 0.4 (box half-width is 1), so the
    // right contact must carry roughly (1+0.4)/(1-0.4) ~= 2.33x the left
    // contact's impulse to hold the box level -- asymmetric but still within
    // the support base, so it should settle flat (Case 1) rather than tip.
    Core::Scene* BuildAsymmetricBoxScene(AsymmetricBoxIds& a_Out, float a_ComOffsetX)
    {
        Core::Scene* scene = QA::CreateHeadlessScene();

        a_Out.ground = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(a_Out.ground).SetPosition(0.0f, 0.0f);
        scene->AddComponent<Collision::BoxCollider2D>(a_Out.ground).SetWidth(10.0f);
        scene->GetComponent<Collision::BoxCollider2D>(a_Out.ground).SetHeight(1.0f);
        Physics::Rigidbody2D& groundRb = scene->AddComponent<Physics::Rigidbody2D>(a_Out.ground);
        groundRb.SetType(Physics::Rigidbody2DType::STATIC);

        a_Out.box = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(a_Out.box).SetPosition(0.0f, 1.05f); // just above ground top (y=0.5)
        scene->AddComponent<Collision::BoxCollider2D>(a_Out.box).SetWidth(2.0f);
        scene->GetComponent<Collision::BoxCollider2D>(a_Out.box).SetHeight(1.0f);
        Physics::Rigidbody2D& boxRb = scene->AddComponent<Physics::Rigidbody2D>(a_Out.box);
        boxRb.SetType(Physics::Rigidbody2DType::DYNAMIC);
        boxRb.SetMass(1.0f);
        boxRb.restitution = 0.0f;
        boxRb.linearDamping = 0.0f;
        boxRb.angularDamping = 0.0f;
        boxRb.centerOfMass = Math::Vector2(a_ComOffsetX, 0.0f);

        return scene;
    }
}

TEST(Invariants, AsymmetricTwoContactBox_SettlesFlatWithoutSpuriousRotation)
{
    AsymmetricBoxIds ids;
    Core::Scene* scene = BuildAsymmetricBoxScene(ids, /*comOffsetX*/ 0.4f);

    QA::RunScene(*scene, 240); // settle

    Common::Transform2D& groundT = scene->GetComponent<Common::Transform2D>(ids.ground);
    Common::Transform2D& boxT = scene->GetComponent<Common::Transform2D>(ids.box);
    Collision::BoxCollider2D& groundBox = scene->GetComponent<Collision::BoxCollider2D>(ids.ground);
    Collision::BoxCollider2D& box = scene->GetComponent<Collision::BoxCollider2D>(ids.box);

    Collision::BoxBoxCollision2D::GetVertices(groundBox, groundT);
    Collision::BoxBoxCollision2D::GetVertices(box, boxT);
    Collision::Collision2DManifold manifold =
        Collision::BoxBoxCollision2D::BoxBoxCollision(groundBox, groundT, box, boxT);

    ASSERT_TRUE(manifold.isColliding);
    ASSERT_EQ(manifold.contactCount, 2) << "test assumes a flat two-contact landing; geometry changed?";

    QA::OracleResult penetrationResult = QA::NoPenetrationPastSlop(manifold, /*slop*/ 0.01f, /*tolerance*/ 5e-3f);
    EXPECT_TRUE(penetrationResult.passed) << penetrationResult.message;

    // A wrong Case-selection (e.g. incorrectly zeroing one contact's impulse
    // every step, or solving as if both were separating) leaves a net,
    // uncorrected torque about the COM -- the box keeps rotating instead of
    // reaching rest, even though gravity itself contributes zero torque
    // (applied at the COM by construction).
    std::vector<Math::Vector2> linearSamples;
    std::vector<float> angularSamples;
    for (int i = 0; i < 30; i++)
    {
        auto snapshots = QA::RunScene(*scene, 1);
        for (auto& s : snapshots)
        {
            if (s.entityId == ids.box)
            {
                linearSamples.push_back(s.linearVelocity);
                angularSamples.push_back(s.angularVelocity);
            }
        }
    }
    QA::OracleResult restResult = QA::RestingBodyStaysAtRest(linearSamples, angularSamples, /*epsilon*/ 0.05f);
    EXPECT_TRUE(restResult.passed) << restResult.message;

    delete scene;
}

// Sensitivity companion: a hand-crafted manifold where the branch-selection
// logic zeroed out the wrong contact (right corner, the one that must carry
// the larger share of load for this COM offset, shows the deep penetration
// a "Case 2 misfire" would leave behind) -- proves NoPenetrationPastSlop
// would catch that failure mode, not just a uniformly-sunk box.
TEST(Invariants, AsymmetricTwoContactBox_OracleCatchesOneSidedSink)
{
    Collision::Collision2DManifold manifold;
    manifold.isColliding = true;
    manifold.contactCount = 2;
    manifold.penetrationDepth[0] = 0.005f; // left corner: fine
    manifold.penetrationDepth[1] = 0.15f;  // right corner: unsupported, sank well past slop

    QA::OracleResult result = QA::NoPenetrationPastSlop(manifold, /*slop*/ 0.01f, /*tolerance*/ 5e-3f);
    EXPECT_FALSE(result.passed);
}

// Sensitivity companion: a hand-crafted velocity sample list representing
// the box still slowly rotating instead of settling -- proves
// RestingBodyStaysAtRest would flag sustained spurious rotation, not just
// large linear velocity.
TEST(Invariants, AsymmetricTwoContactBox_OracleCatchesSustainedRotation)
{
    std::vector<Math::Vector2> linearSamples = { Math::Vector2(0.0f, 0.0f), Math::Vector2(0.0f, 0.0f) };
    std::vector<float> angularSamples = { 0.3f, 0.3f }; // steady, uncorrected spin

    QA::OracleResult result = QA::RestingBodyStaysAtRest(linearSamples, angularSamples, /*epsilon*/ 0.05f);
    EXPECT_FALSE(result.passed);
}
