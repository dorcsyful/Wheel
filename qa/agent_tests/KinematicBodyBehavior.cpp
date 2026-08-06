#include <gtest/gtest.h>

#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

// KINEMATIC bodies are never moved by the physics system (no gravity, no force
// or velocity integration of their transform) -- they move only when code sets
// their transform directly -- yet they still collide with DYNAMIC bodies.
// Before KINEMATIC was handled, only STATIC was special-cased, so a kinematic
// body fell and integrated exactly like a dynamic one.
namespace
{
    uint32_t AddBox(Core::Scene* s, float x, float y, float w, float h, Physics::Rigidbody2DType type)
    {
        uint32_t id = s->AddEntity();
        s->AddComponent<Common::Transform2D>(id).SetPosition(x, y);
        auto& col = s->AddComponent<Collision::BoxCollider2D>(id);
        col.SetWidth(w);
        col.SetHeight(h);
        auto& rb = s->AddComponent<Physics::Rigidbody2D>(id);
        rb.SetMass(1.0f);   // before SetType: SetType(STATIC) zeroes invMass, a later SetMass would undo it
        rb.SetType(type);
        return id;
    }
}

// A kinematic body with a real collider (so the no-collider skip isn't what's
// keeping it still) does not fall under gravity.
TEST(KinematicBodyBehavior, IgnoresGravity)
{
    Core::Scene* scene = QA::CreateHeadlessScene();
    AddBox(scene, 0.0f, 5.0f, 1.0f, 1.0f, Physics::Rigidbody2DType::KINEMATIC);
    auto snaps = QA::RunScene(*scene, 120);
    EXPECT_NEAR(snaps[0].position.y, 5.0f, 1e-4f);
    EXPECT_NEAR(snaps[0].position.x, 0.0f, 1e-4f);
    delete scene;
}

// Control: the identical setup as DYNAMIC *does* fall -- proving the scene
// applies gravity and the test above isn't passing for an unrelated reason.
TEST(KinematicBodyBehavior, DynamicControlFalls)
{
    Core::Scene* scene = QA::CreateHeadlessScene();
    AddBox(scene, 0.0f, 5.0f, 1.0f, 1.0f, Physics::Rigidbody2DType::DYNAMIC);
    auto snaps = QA::RunScene(*scene, 120);
    EXPECT_LT(snaps[0].position.y, 4.0f);
    delete scene;
}

// Setting a kinematic body's velocity does not move it: only code writing the
// transform moves a kinematic, the physics integrator never does.
TEST(KinematicBodyBehavior, SetVelocityDoesNotIntegrate)
{
    Core::Scene* scene = QA::CreateHeadlessScene();
    uint32_t id = AddBox(scene, 0.0f, 0.0f, 1.0f, 1.0f, Physics::Rigidbody2DType::KINEMATIC);
    scene->GetComponent<Physics::Rigidbody2D>(id).linearVelocity = Math::Vector2(3.0f, 0.0f);
    auto snaps = QA::RunScene(*scene, 60);
    EXPECT_NEAR(snaps[0].position.x, 0.0f, 1e-4f);
    EXPECT_NEAR(snaps[0].position.y, 0.0f, 1e-4f);
    delete scene;
}

// NOTE: a "dynamic body rests on a kinematic platform" interaction test is
// deliberately omitted. It cannot pass today, but NOT because of kinematic
// support -- a dynamic box does not rest stably on a *static* platform either:
// it holds for a few frames, then sinks through the contact and tunnels out
// (see the resting-stability bug flagged in the QA report). Add that test once
// box-on-box resting contact is stable; until then it would only be re-testing
// that separate defect.
