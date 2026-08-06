#include <gtest/gtest.h>

#include "Oracles.h"
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

namespace
{
    // A single falling, horizontally-drifting circle with no collision partner
    // (kept alone in its own scene), so the trajectory is pure gravity +
    // horizontal drift — clean input for the mirror relation.
    Core::Scene* BuildDriftingBodyScene(float a_StartX, float a_VelocityX)
    {
        Core::Scene* scene = QA::CreateHeadlessScene();
        uint32_t body = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(body).SetPosition(a_StartX, 10.0f);
        scene->AddComponent<Collision::CircleCollider2D>(body).radius = 0.5f;
        Physics::Rigidbody2D& rb = scene->AddComponent<Physics::Rigidbody2D>(body);
        rb.SetType(Physics::Rigidbody2DType::DYNAMIC);
        rb.SetMass(1.0f);
        rb.linearVelocity = Math::Vector2(a_VelocityX, 0.0f);
        return scene;
    }
}

TEST(Metamorphic, MirroredScene_MatchesAcrossYAxis)
{
    Core::Scene* original = BuildDriftingBodyScene(3.0f, 2.0f);
    Core::Scene* mirrored = BuildDriftingBodyScene(-3.0f, -2.0f);

    constexpr int steps = 90;
    auto originalSnapshots = QA::RunScene(*original, steps);
    auto mirroredSnapshots = QA::RunScene(*mirrored, steps);

    QA::OracleResult result = QA::MirroredScenesMatch(originalSnapshots, mirroredSnapshots, /*tolerance*/ 1e-4f);
    EXPECT_TRUE(result.passed) << result.message;

    delete original;
    delete mirrored;
}

TEST(Metamorphic, MirroredScene_OracleCatchesBrokenMirror)
{
    QA::EntitySnapshot original;
    original.entityId = 0;
    original.position = Math::Vector2(3.0f, 7.0f);

    QA::EntitySnapshot brokenMirror;
    brokenMirror.entityId = 0;
    brokenMirror.position = Math::Vector2(3.0f, 7.0f); // should be (-3, 7) — not actually mirrored

    QA::OracleResult result = QA::MirroredScenesMatch({ original }, { brokenMirror }, /*tolerance*/ 1e-4f);
    EXPECT_FALSE(result.passed);
}