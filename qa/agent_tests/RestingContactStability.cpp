#include <gtest/gtest.h>

#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"

using namespace Wheel;

namespace
{
    uint32_t AddBox(Core::Scene* s, float x, float y, float w, float h, Physics::Rigidbody2DType t)
    {
        uint32_t id = s->AddEntity();
        s->AddComponent<Common::Transform2D>(id).SetPosition(x, y);
        auto& c = s->AddComponent<Collision::BoxCollider2D>(id);
        c.SetWidth(w);
        c.SetHeight(h);
        auto& rb = s->AddComponent<Physics::Rigidbody2D>(id);
        rb.SetMass(1.0f);   // MUST precede SetType: SetType(STATIC) zeroes invMass,
        rb.SetType(t);      // and a later SetMass would clobber it back to a finite value.
        return id;
    }
}

// Resting on a STATIC box works: this PASSES and guards it against regression.
// (An earlier version of this file wrongly reported box-on-static resting as
// broken -- that was a scene-setup bug, SetMass called after SetType, which
// silently un-did the static body's zero inverse mass. With the correct order
// the box rests exactly as it should.)
TEST(RestingContactStability, DynamicBoxRestsOnStaticBox)
{
    Core::Scene* scene = QA::CreateHeadlessScene();
    AddBox(scene, 0.0f, 0.0f, 4.0f, 1.0f, Physics::Rigidbody2DType::STATIC);
    AddBox(scene, 0.0f, 1.05f, 2.0f, 1.0f, Physics::Rigidbody2DType::DYNAMIC); // rest centre ~1.0
    auto snaps = QA::RunScene(*scene, 120);
    EXPECT_NEAR(snaps[1].position.y, 1.0f, 0.05f);
    delete scene;
}

// KNOWN ENGINE BUG -- this test FAILS on purpose and must stay failing until
// fixed. A dynamic box cannot rest on a KINEMATIC platform: it sinks through
// and free-falls (final y ~ -13 after 120 steps) even though the platform never
// moves. Controlled comparison, identical scene, only the platform type varied:
//     STATIC platform    -> box rests at y = 0.993
//     KINEMATIC platform -> box falls to y = -13.1
// The only difference is inverse mass: SetType (Rigidbody2D.h) zeroes invMass
// for STATIC but not for KINEMATIC, so a kinematic body keeps finite mass, is
// shoved by the contact impulse (its velocity is corrupted but never
// integrated), and stops supporting the dynamic box. A kinematic body that is
// meant to "interact with dynamic objects" as an immovable platform needs
// invMass == 0 in the solver, like STATIC. Do not weaken or disable this; it is
// the record of the defect.
TEST(RestingContactStability, DynamicBoxRestsOnKinematicPlatform)
{
    Core::Scene* scene = QA::CreateHeadlessScene();
    AddBox(scene, 0.0f, 0.0f, 10.0f, 1.0f, Physics::Rigidbody2DType::KINEMATIC);
    AddBox(scene, 0.0f, 1.05f, 1.0f, 1.0f, Physics::Rigidbody2DType::DYNAMIC);
    auto snaps = QA::RunScene(*scene, 120);
    EXPECT_NEAR(snaps[0].position.y, 0.0f, 1e-3f);   // platform never moves (this part holds)
    EXPECT_NEAR(snaps[1].position.y, 1.0f, 0.05f);   // box must stay resting -- currently FAILS
    delete scene;
}