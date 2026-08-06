#include <gtest/gtest.h>

#include "Oracles.h"
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "collision/BoxBoxCollision2D.h"
#include "collision/CircleBoxCollision2D.h"
#include "physics/Rigidbody2D.h"
#include "physics/Joint2D.h"

using namespace Wheel;

namespace
{
    // Static ground box + one dynamic circle dropped onto it from just above
    // the surface, so it settles quickly. Returns the ground/circle ids.
    struct RestingSceneEntities { uint32_t ground; uint32_t circle; };

    Core::Scene* BuildRestingCircleOnGroundScene(RestingSceneEntities& a_Out)
    {
        Core::Scene* scene = QA::CreateHeadlessScene();

        a_Out.ground = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(a_Out.ground).SetPosition(0.0f, 0.0f);
        scene->AddComponent<Collision::BoxCollider2D>(a_Out.ground).SetWidth(10.0f);
        scene->GetComponent<Collision::BoxCollider2D>(a_Out.ground).SetHeight(1.0f);
        Physics::Rigidbody2D& groundRb = scene->AddComponent<Physics::Rigidbody2D>(a_Out.ground);
        groundRb.SetType(Physics::Rigidbody2DType::STATIC);

        a_Out.circle = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(a_Out.circle).SetPosition(0.0f, 1.55f); // just above the top face (y=0.5)
        scene->AddComponent<Collision::CircleCollider2D>(a_Out.circle).radius = 0.5f;
        Physics::Rigidbody2D& circleRb = scene->AddComponent<Physics::Rigidbody2D>(a_Out.circle);
        circleRb.SetType(Physics::Rigidbody2DType::DYNAMIC);
        circleRb.SetMass(1.0f);

        return scene;
    }
}

TEST(Invariants, NoPenetrationPastSlop_AfterSettling)
{
    RestingSceneEntities ids;
    Core::Scene* scene = BuildRestingCircleOnGroundScene(ids);

    QA::RunScene(*scene, 180); // ~3s: enough to land and settle

    Common::Transform2D& groundT = scene->GetComponent<Common::Transform2D>(ids.ground);
    Common::Transform2D& circleT = scene->GetComponent<Common::Transform2D>(ids.circle);
    Collision::BoxCollider2D& box = scene->GetComponent<Collision::BoxCollider2D>(ids.ground);
    Collision::CircleCollider2D& circle = scene->GetComponent<Collision::CircleCollider2D>(ids.circle);

    Collision::BoxBoxCollision2D::GetVertices(box, groundT); // populate cached vertices
    Collision::Collision2DManifold manifold =
        Collision::CircleBoxCollision2D::CheckCircleBoxCollision(circleT, circle, groundT, box);

    QA::OracleResult result = QA::NoPenetrationPastSlop(manifold, /*slop*/ 0.01f, /*tolerance*/ 5e-3f);
    EXPECT_TRUE(result.passed) << result.message;

    delete scene;
}

TEST(Invariants, NoPenetrationPastSlop_OracleCatchesDeepOverlap)
{
    Collision::Collision2DManifold manifold;
    manifold.isColliding = true;
    manifold.contactCount = 1;
    manifold.penetrationDepth[0] = 0.2f; // way past 0.01 slop

    QA::OracleResult result = QA::NoPenetrationPastSlop(manifold, /*slop*/ 0.01f, /*tolerance*/ 5e-3f);
    EXPECT_FALSE(result.passed);
}

TEST(Invariants, RestingBodyStaysAtRest_AfterSettling)
{
    RestingSceneEntities ids;
    Core::Scene* scene = BuildRestingCircleOnGroundScene(ids);

    QA::RunScene(*scene, 180); // settle first

    std::vector<Math::Vector2> linearSamples;
    std::vector<float> angularSamples;
    for (int i = 0; i < 30; i++)
    {
        auto snapshots = QA::RunScene(*scene, 1);
        for (auto& s : snapshots)
        {
            if (s.entityId == ids.circle)
            {
                linearSamples.push_back(s.linearVelocity);
                angularSamples.push_back(s.angularVelocity);
            }
        }
    }

    QA::OracleResult result = QA::RestingBodyStaysAtRest(linearSamples, angularSamples, /*epsilon*/ 0.2f);
    EXPECT_TRUE(result.passed) << result.message;

    delete scene;
}

TEST(Invariants, RestingBodyStaysAtRest_OracleCatchesJitter)
{
    std::vector<Math::Vector2> linearSamples = { Math::Vector2(0.0f, 0.0f), Math::Vector2(5.0f, 0.0f) };
    std::vector<float> angularSamples = { 0.0f, 0.0f };

    QA::OracleResult result = QA::RestingBodyStaysAtRest(linearSamples, angularSamples, /*epsilon*/ 0.2f);
    EXPECT_FALSE(result.passed);
}

TEST(Invariants, MomentumConservedAndEnergyNonIncreasing_UnequalMassCollision)
{
    Core::Scene* scene = QA::CreateHeadlessScene();

    uint32_t a = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(a).SetPosition(-0.49f, 0.0f);
    scene->AddComponent<Collision::CircleCollider2D>(a).radius = 0.5f;
    Physics::Rigidbody2D& rbA = scene->AddComponent<Physics::Rigidbody2D>(a);
    rbA.SetType(Physics::Rigidbody2DType::DYNAMIC);
    rbA.SetMass(1.0f);
    rbA.restitution = 0.5f;
    rbA.linearDamping = 0.0f;
    rbA.affectedByGravity = false;
    rbA.linearVelocity = Math::Vector2(4.0f, 0.0f);

    uint32_t b = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(b).SetPosition(0.49f, 0.0f);
    scene->AddComponent<Collision::CircleCollider2D>(b).radius = 0.5f;
    Physics::Rigidbody2D& rbB = scene->AddComponent<Physics::Rigidbody2D>(b);
    rbB.SetType(Physics::Rigidbody2DType::DYNAMIC);
    rbB.SetMass(3.0f);
    rbB.restitution = 0.5f;
    rbB.linearDamping = 0.0f;
    rbB.affectedByGravity = false;
    rbB.linearVelocity = Math::Vector2(0.0f, 0.0f);

    std::vector<float> masses = { rbA.GetMass(), rbB.GetMass() };
    std::vector<Math::Vector2> velocitiesBefore = { rbA.linearVelocity, rbB.linearVelocity };
    float energyBefore = QA::KineticEnergy(rbA.GetMass(), rbA.linearVelocity, rbA.GetInertia(), rbA.angularVelocity)
                        + QA::KineticEnergy(rbB.GetMass(), rbB.linearVelocity, rbB.GetInertia(), rbB.angularVelocity);

    QA::RunScene(*scene, 5, 1.0f / 60.0f);

    Physics::Rigidbody2D& rbAAfter = scene->GetComponent<Physics::Rigidbody2D>(a);
    Physics::Rigidbody2D& rbBAfter = scene->GetComponent<Physics::Rigidbody2D>(b);
    std::vector<Math::Vector2> velocitiesAfter = { rbAAfter.linearVelocity, rbBAfter.linearVelocity };
    float energyAfter = QA::KineticEnergy(rbAAfter.GetMass(), rbAAfter.linearVelocity, rbAAfter.GetInertia(), rbAAfter.angularVelocity)
                       + QA::KineticEnergy(rbBAfter.GetMass(), rbBAfter.linearVelocity, rbBAfter.GetInertia(), rbBAfter.angularVelocity);

    QA::OracleResult momentumResult = QA::MomentumConserved(masses, velocitiesBefore, velocitiesAfter, /*tolerance*/ 0.1f);
    EXPECT_TRUE(momentumResult.passed) << momentumResult.message;

    QA::OracleResult energyResult = QA::EnergyNonIncreasing(energyBefore, energyAfter, /*tolerance*/ 0.1f);
    EXPECT_TRUE(energyResult.passed) << energyResult.message;

    delete scene;
}

TEST(Invariants, MomentumConserved_OracleCatchesViolation)
{
    std::vector<float> masses = { 1.0f, 3.0f };
    std::vector<Math::Vector2> before = { Math::Vector2(4.0f, 0.0f), Math::Vector2(0.0f, 0.0f) };
    std::vector<Math::Vector2> after  = { Math::Vector2(0.0f, 0.0f), Math::Vector2(0.0f, 0.0f) }; // momentum vanished

    QA::OracleResult result = QA::MomentumConserved(masses, before, after, /*tolerance*/ 0.1f);
    EXPECT_FALSE(result.passed);
}

TEST(Invariants, EnergyNonIncreasing_OracleCatchesGain)
{
    QA::OracleResult result = QA::EnergyNonIncreasing(/*before*/ 10.0f, /*after*/ 20.0f, /*tolerance*/ 0.1f);
    EXPECT_FALSE(result.passed);
}

// DistanceJoint2D's "maxDistanceOnly" flag is documented as switching between a
// rigid rod (exact distance) and a rope (never longer than distance) — see
// wheel/physics/Joint2D.h. But PrepareJointConstraintSolver only ever activates
// the constraint when length > distance (wheel/physics/JointConstraintSolver.cpp:23),
// regardless of the flag; a compressed/slack joint is never pushed back out.
// So today the joint is always rope-like. This test checks the invariant the
// code actually enforces (never longer than target) rather than the doc
// comment's stated intent for maxDistanceOnly=false.
TEST(Invariants, DistanceJoint_NeverExceedsTargetDistance)
{
    Core::Scene* scene = QA::CreateHeadlessScene();

    uint32_t body = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(body).SetPosition(0.0f, 5.0f);
    scene->AddComponent<Collision::CircleCollider2D>(body).radius = 0.25f;
    Physics::Rigidbody2D& rb = scene->AddComponent<Physics::Rigidbody2D>(body);
    rb.SetType(Physics::Rigidbody2DType::DYNAMIC);
    rb.SetMass(1.0f);

    Physics::DistanceJoint2D& joint = scene->AddComponent<Physics::DistanceJoint2D>(body);
    joint.connectedRigidbody = NO_VALUE;
    joint.otherAnchorPoint = Math::Vector2(0.0f, 5.0f); // fixed world anchor, same as start position
    joint.localAnchorPoint = Math::Vector2(0.0f, 0.0f); // anchor1 == body position exactly
    joint.distance = 2.0f;
    joint.maxDistanceOnly = false;

    constexpr float tolerance = 0.05f;
    for (int i = 0; i < 180; i++)
    {
        QA::RunScene(*scene, 1);
        Common::Transform2D& t = scene->GetComponent<Common::Transform2D>(body);
        QA::OracleResult result = QA::DistanceJointSatisfied(
            t.GetPosition(), joint.otherAnchorPoint, joint.distance, /*maxDistanceOnly*/ true, tolerance);
        ASSERT_TRUE(result.passed) << "step " << i << ": " << result.message;
    }

    delete scene;
}

TEST(Invariants, DistanceJoint_OracleCatchesOverextension)
{
    Math::Vector2 anchor1(0.0f, 0.0f);
    Math::Vector2 anchor2(0.0f, 3.0f); // 3.0 apart, target is 2.0
    QA::OracleResult result = QA::DistanceJointSatisfied(anchor1, anchor2, /*target*/ 2.0f, /*maxDistanceOnly*/ true, /*tolerance*/ 0.05f);
    EXPECT_FALSE(result.passed);
}