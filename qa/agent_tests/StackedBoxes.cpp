#include <gtest/gtest.h>

#include "Oracles.h"
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "collision/BoxBoxCollision2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

// A dynamic box settling onto a static box (a two-contact, face-to-face box
// collision under gravity -- the hand-written suite's resting-stack tests all
// use circles, so Solve2ContactConstraint's box path is exercised here).
//
// This test previously checked ONLY NoPenetrationPastSlop, which passes
// vacuously once the top box sinks through and separates: penetration of a
// non-colliding manifold is trivially within slop. So it stayed green while the
// engine let the box fall through. The assertions below now require the box to
// actually be RESTING (still in contact, at its rest height) -- which it is not,
// so this test FAILS and exposes the resting-contact bug. Do not weaken it back.
TEST(AgentSample, TwoStackedDynamicBoxesDoNotPenetratePastSlop)
{
    Core::Scene* scene = QA::CreateHeadlessScene();

    uint32_t bottom = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(bottom).SetPosition(0.0f, 0.0f);
    scene->AddComponent<Collision::BoxCollider2D>(bottom).SetWidth(4.0f);
    scene->GetComponent<Collision::BoxCollider2D>(bottom).SetHeight(1.0f);
    Physics::Rigidbody2D& bottomRb = scene->AddComponent<Physics::Rigidbody2D>(bottom);
    bottomRb.SetType(Physics::Rigidbody2DType::STATIC);

    uint32_t top = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(top).SetPosition(0.0f, 1.05f);
    scene->AddComponent<Collision::BoxCollider2D>(top).SetWidth(2.0f);
    scene->GetComponent<Collision::BoxCollider2D>(top).SetHeight(1.0f);
    Physics::Rigidbody2D& topRb = scene->AddComponent<Physics::Rigidbody2D>(top);
    topRb.SetType(Physics::Rigidbody2DType::DYNAMIC);
    topRb.SetMass(1.0f);

    QA::RunScene(*scene, 180); // settle

    Common::Transform2D& bottomT = scene->GetComponent<Common::Transform2D>(bottom);
    Common::Transform2D& topT = scene->GetComponent<Common::Transform2D>(top);
    Collision::BoxCollider2D& bottomBox = scene->GetComponent<Collision::BoxCollider2D>(bottom);
    Collision::BoxCollider2D& topBox = scene->GetComponent<Collision::BoxCollider2D>(top);

    Collision::BoxBoxCollision2D::GetVertices(bottomBox, bottomT);
    Collision::BoxBoxCollision2D::GetVertices(topBox, topT);
    Collision::Collision2DManifold manifold =
        Collision::BoxBoxCollision2D::BoxBoxCollision(bottomBox, bottomT, topBox, topT);

    // The top box must still be resting on the bottom one: at its rest height
    // (bottom top face 0.5 + top half-height 0.5 = 1.0) and still in contact.
    // Without these, NoPenetrationPastSlop passes on a box that fell away.
    EXPECT_NEAR(topT.GetPosition().y, 1.0f, 0.05f)
        << "top box did not stay resting on the bottom box (sank / tunnelled)";
    ASSERT_TRUE(manifold.isColliding)
        << "boxes are no longer in contact -- top box is not resting on the bottom box";

    QA::OracleResult result = QA::NoPenetrationPastSlop(manifold, /*slop*/ 0.01f, /*tolerance*/ 5e-3f);
    EXPECT_TRUE(result.passed) << result.message;

    delete scene;
}