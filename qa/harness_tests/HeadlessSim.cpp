#include <gtest/gtest.h>

#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

namespace
{
    // Two falling circles (never touch each other) plus one transform-only
    // marker entity with no Rigidbody2D — exercises both snapshot paths.
    // Both dynamic bodies carry a collider: Physics2DSystem::Update skips its
    // whole force/velocity step (not just inertia) for any Rigidbody2D with no
    // collider, so a collider-less body would never receive gravity here.
    Core::Scene* BuildFallingBodiesScene()
    {
        Core::Scene* scene = QA::CreateHeadlessScene();

        uint32_t a = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(a).SetPosition(0.0f, 10.0f);
        scene->AddComponent<Collision::CircleCollider2D>(a).radius = 0.5f;
        Physics::Rigidbody2D& rbA = scene->AddComponent<Physics::Rigidbody2D>(a);
        rbA.SetType(Physics::Rigidbody2DType::DYNAMIC);
        rbA.SetMass(1.0f);

        uint32_t b = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(b).SetPosition(50.0f, 20.0f);
        scene->AddComponent<Collision::CircleCollider2D>(b).radius = 0.5f;
        Physics::Rigidbody2D& rbB = scene->AddComponent<Physics::Rigidbody2D>(b);
        rbB.SetType(Physics::Rigidbody2DType::DYNAMIC);
        rbB.SetMass(2.0f);

        uint32_t marker = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(marker).SetPosition(-5.0f, -5.0f);

        return scene;
    }
}

TEST(HeadlessSim, ProducesSnapshotForEveryTransformEntity)
{
    Core::Scene* scene = BuildFallingBodiesScene();
    auto snapshots = QA::RunScene(*scene, 30);
    EXPECT_EQ(snapshots.size(), 3u);
    delete scene;
}

TEST(HeadlessSim, SortedByEntityId)
{
    Core::Scene* scene = BuildFallingBodiesScene();
    auto snapshots = QA::RunScene(*scene, 10);
    for (size_t i = 1; i < snapshots.size(); ++i)
        EXPECT_LT(snapshots[i - 1].entityId, snapshots[i].entityId);
    delete scene;
}

TEST(HeadlessSim, ZeroStepsReturnsInitialState)
{
    Core::Scene* scene = BuildFallingBodiesScene();
    auto snapshots = QA::RunScene(*scene, 0);
    ASSERT_EQ(snapshots.size(), 3u);
    EXPECT_FLOAT_EQ(snapshots[0].position.x, 0.0f);
    EXPECT_FLOAT_EQ(snapshots[0].position.y, 10.0f);
    delete scene;
}

TEST(HeadlessSim, GravityMovesTransformOverTime)
{
    Core::Scene* scene = BuildFallingBodiesScene();
    float startY = scene->GetComponent<Common::Transform2D>(0).GetPosition().y;
    auto snapshots = QA::RunScene(*scene, 60);
    EXPECT_LT(snapshots[0].position.y, startY);
    delete scene;
}

TEST(HeadlessSim, TwoIdenticalRunsAreBitIdentical)
{
    Core::Scene* sceneOne = BuildFallingBodiesScene();
    Core::Scene* sceneTwo = BuildFallingBodiesScene();

    auto snapshotsOne = QA::RunScene(*sceneOne, 120);
    auto snapshotsTwo = QA::RunScene(*sceneTwo, 120);

    ASSERT_EQ(snapshotsOne.size(), snapshotsTwo.size());
    for (size_t i = 0; i < snapshotsOne.size(); ++i)
    {
        EXPECT_EQ(snapshotsOne[i].entityId, snapshotsTwo[i].entityId);
        // Bit-for-bit, not EXPECT_NEAR: determinism means identical inputs
        // produce identical floats, not merely close ones.
        EXPECT_EQ(snapshotsOne[i].position.x, snapshotsTwo[i].position.x);
        EXPECT_EQ(snapshotsOne[i].position.y, snapshotsTwo[i].position.y);
        EXPECT_EQ(snapshotsOne[i].rotation, snapshotsTwo[i].rotation);
        EXPECT_EQ(snapshotsOne[i].linearVelocity.x, snapshotsTwo[i].linearVelocity.x);
        EXPECT_EQ(snapshotsOne[i].linearVelocity.y, snapshotsTwo[i].linearVelocity.y);
        EXPECT_EQ(snapshotsOne[i].angularVelocity, snapshotsTwo[i].angularVelocity);
    }

    delete sceneOne;
    delete sceneTwo;
}