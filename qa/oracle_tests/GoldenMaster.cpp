#include <gtest/gtest.h>
#include <cstdio>

#include "Oracles.h"
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

namespace
{
    std::string GoldenPath(const std::string& a_Name)
    {
        return std::string(QA_ORACLE_TESTS_DIR) + "/golden/" + a_Name;
    }

    Core::Scene* BuildFreeFallScene()
    {
        Core::Scene* scene = QA::CreateHeadlessScene();
        uint32_t body = scene->AddEntity();
        scene->AddComponent<Common::Transform2D>(body).SetPosition(0.0f, 100.0f);
        scene->AddComponent<Collision::CircleCollider2D>(body).radius = 0.5f;
        Physics::Rigidbody2D& rb = scene->AddComponent<Physics::Rigidbody2D>(body);
        rb.SetType(Physics::Rigidbody2DType::DYNAMIC);
        rb.SetMass(1.0f);
        rb.linearDamping = 0.0f;
        return scene;
    }
}

// Real, committed baseline: first execution (ever) records golden/free_fall.txt;
// every execution after that compares against it bit-for-bit. If the engine's
// integration or gravity constant ever changes intentionally, delete the file
// so it gets re-recorded.
TEST(GoldenMaster, FreeFall_MatchesRecordedBaseline)
{
    Core::Scene* scene = BuildFreeFallScene();
    auto snapshots = QA::RunScene(*scene, 90);

    QA::OracleResult result = QA::MatchesGoldenMaster(snapshots, GoldenPath("free_fall.txt"));
    EXPECT_TRUE(result.passed) << result.message;

    delete scene;
}

// Full lifecycle against a scratch path that's reset every run, so the test
// is self-proving without depending on repo state: bootstrap (no file yet)
// must report a recorded-not-compared result, and a second identical run
// against that fresh baseline must then pass bit-for-bit.
TEST(GoldenMaster, Lifecycle_BootstrapThenCompare)
{
    std::string path = GoldenPath("_scratch_lifecycle_test.txt");
    std::remove(path.c_str());

    Core::Scene* sceneOne = BuildFreeFallScene();
    auto snapshotsOne = QA::RunScene(*sceneOne, 30);
    QA::OracleResult bootstrapResult = QA::MatchesGoldenMaster(snapshotsOne, path);
    EXPECT_FALSE(bootstrapResult.passed); // no baseline existed yet — this run only recorded one
    delete sceneOne;

    Core::Scene* sceneTwo = BuildFreeFallScene();
    auto snapshotsTwo = QA::RunScene(*sceneTwo, 30);
    QA::OracleResult compareResult = QA::MatchesGoldenMaster(snapshotsTwo, path);
    EXPECT_TRUE(compareResult.passed) << compareResult.message; // now compares against the recorded baseline
    delete sceneTwo;

    std::remove(path.c_str());
}

TEST(GoldenMaster, OracleCatchesDivergence)
{
    std::string path = GoldenPath("_scratch_divergence_test.txt");
    std::remove(path.c_str());

    std::vector<QA::EntitySnapshot> baseline(1);
    baseline[0].entityId = 0;
    baseline[0].position = Math::Vector2(1.0f, 2.0f);
    QA::MatchesGoldenMaster(baseline, path); // bootstrap

    std::vector<QA::EntitySnapshot> divergent(1);
    divergent[0].entityId = 0;
    divergent[0].position = Math::Vector2(1.0f, 999.0f); // clearly different

    QA::OracleResult result = QA::MatchesGoldenMaster(divergent, path, /*recordIfMissing*/ false);
    EXPECT_FALSE(result.passed);

    std::remove(path.c_str());
}