#include <gtest/gtest.h>

#include "Oracles.h"
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

// Matches Physics2DSystem's hardcoded gravity (wheel/physics/Physics2DSystem.h:60).
// Not exposed publicly, so the oracle's ground truth has to encode it directly —
// that coupling is inherent to analytical oracles, which compare against an
// external truth rather than anything derived from the engine itself.
namespace { constexpr float kEngineGravityY = -9.81f; }

TEST(Analytical, FreeFall_MatchesDiscreteClosedForm)
{
    Core::Scene* scene = QA::CreateHeadlessScene();
    uint32_t body = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(body).SetPosition(0.0f, 100.0f);
    scene->AddComponent<Collision::CircleCollider2D>(body).radius = 0.5f;
    Physics::Rigidbody2D& rb = scene->AddComponent<Physics::Rigidbody2D>(body);
    rb.SetType(Physics::Rigidbody2DType::DYNAMIC);
    rb.SetMass(1.0f);
    rb.linearDamping = 0.0f; // isolate pure gravity integration from the default damping

    constexpr float dt = 1.0f / 60.0f;
    constexpr int steps = 90;
    auto snapshots = QA::RunScene(*scene, steps, dt);
    ASSERT_EQ(snapshots.size(), 1u);

    QA::OracleResult result = QA::AnalyticalFreeFallMatches(
        snapshots[0].position.y, 100.0f, 0.0f, kEngineGravityY, dt, steps, /*tolerance*/ 1e-3f);
    EXPECT_TRUE(result.passed) << result.message;

    delete scene;
}

TEST(Analytical, FreeFall_OracleCatchesWrongValue)
{
    // Sensitivity check: the oracle must actually fail on a value that
    // doesn't match the closed form, not just pass everything.
    QA::OracleResult result = QA::AnalyticalFreeFallMatches(
        /*actualY*/ 0.0f, /*y0*/ 100.0f, /*v0y*/ 0.0f, kEngineGravityY,
        1.0f / 60.0f, 90, /*tolerance*/ 1e-3f);
    EXPECT_FALSE(result.passed);
}

TEST(Analytical, ElasticEqualMassCollision_SwapsVelocity)
{
    Core::Scene* scene = QA::CreateHeadlessScene();

    uint32_t a = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(a).SetPosition(-0.49f, 0.0f);
    scene->AddComponent<Collision::CircleCollider2D>(a).radius = 0.5f;
    Physics::Rigidbody2D& rbA = scene->AddComponent<Physics::Rigidbody2D>(a);
    rbA.SetType(Physics::Rigidbody2DType::DYNAMIC);
    rbA.SetMass(1.0f);
    rbA.restitution = 1.0f;
    rbA.linearDamping = 0.0f;
    rbA.affectedByGravity = false;
    rbA.linearVelocity = Math::Vector2(5.0f, 0.0f);

    uint32_t b = scene->AddEntity();
    scene->AddComponent<Common::Transform2D>(b).SetPosition(0.49f, 0.0f);
    scene->AddComponent<Collision::CircleCollider2D>(b).radius = 0.5f;
    Physics::Rigidbody2D& rbB = scene->AddComponent<Physics::Rigidbody2D>(b);
    rbB.SetType(Physics::Rigidbody2DType::DYNAMIC);
    rbB.SetMass(1.0f);
    rbB.restitution = 1.0f;
    rbB.linearDamping = 0.0f;
    rbB.affectedByGravity = false;
    rbB.linearVelocity = Math::Vector2(-5.0f, 0.0f);

    Math::Vector2 velocityABefore = rbA.linearVelocity;
    Math::Vector2 velocityBBefore = rbB.linearVelocity;

    QA::RunScene(*scene, 3, 1.0f / 60.0f);

    Physics::Rigidbody2D& rbAAfter = scene->GetComponent<Physics::Rigidbody2D>(a);
    Physics::Rigidbody2D& rbBAfter = scene->GetComponent<Physics::Rigidbody2D>(b);

    QA::OracleResult result = QA::ElasticEqualMassCollisionSwapsVelocity(
        velocityABefore, velocityBBefore, rbAAfter.linearVelocity, rbBAfter.linearVelocity, /*tolerance*/ 0.5f);
    EXPECT_TRUE(result.passed) << result.message;

    delete scene;
}

TEST(Analytical, ElasticCollision_OracleCatchesNoSwap)
{
    Math::Vector2 velocityABefore(5.0f, 0.0f);
    Math::Vector2 velocityBBefore(-5.0f, 0.0f);
    // Deliberately "wrong": velocities left unchanged instead of swapped.
    QA::OracleResult result = QA::ElasticEqualMassCollisionSwapsVelocity(
        velocityABefore, velocityBBefore, velocityABefore, velocityBBefore, /*tolerance*/ 0.5f);
    EXPECT_FALSE(result.passed);
}