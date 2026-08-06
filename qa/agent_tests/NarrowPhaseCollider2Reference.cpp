#include <gtest/gtest.h>
#include <algorithm>

#include "math/Vector2.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "collision/Collision2DManifold.h"
#include "collision/BoxBoxCollision2D.h"

using Wheel::Math::Vector2;
using Wheel::Common::Transform2D;
using Wheel::Collision::BoxCollider2D;
using Wheel::Collision::BoxBoxCollision2D;

// Regression guard for the reference-face selection. BoxBoxCollision picks the
// box of least penetration as the reference face and measures penetration along
// ITS outward normal (with a sign flip when collider2 is the reference). If the
// collider2-reference path is ever skipped -- as happened when a refactor left
// minOverlap2 unwritten -- collider2-reference contacts silently report ~0
// penetration and the position solver lets the box sink.
//
// Every axis-aligned box-box case makes collider1 the reference (the two boxes
// share axes, so minOverlap1 == minOverlap2 and the <= tie goes to collider1),
// which is why the whole hand-written suite never exercised the other branch.
// Rotating collider1 breaks the tie the other way: only collider2's (axis-
// aligned) Y-face yields the minimum overlap, so collider2 becomes the
// reference and this path is finally covered.
namespace
{
    Transform2D MakeTransform(float x, float y, float rotationDeg)
    {
        Transform2D t;
        t.SetScale(Vector2(1.0f, 1.0f));
        t.SetRotationInDegrees(rotationDeg);
        t.SetPosition(x, y);
        return t;
    }

    BoxCollider2D MakeBox(float width, float height)
    {
        BoxCollider2D b;
        b.SetWidth(width);
        b.SetHeight(height);
        return b;
    }
} // namespace

// A (collider1) is a 2x2 rotated 45deg at the origin -> its top vertex reaches
// y = sqrt(2) ~= 1.414. B (collider2) is an axis-aligned 2x2 centred at (0,2),
// so its bottom face sits at y = 1. B's top vertex pokes into B's face by
// ~0.414 along B's Y-axis, the least-overlap axis -> B is the reference face.
TEST(NarrowPhaseCollider2Reference, RotatedIncident_PenetrationMeasuredOnCollider2Face)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f, 45.0f);
    Transform2D tb = MakeTransform(0.0f, 2.0f, 0.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    ASSERT_TRUE(m.isColliding);
    ASSERT_GE(m.contactCount, 1);

    // Reference is collider2's Y-face; collision normal is (roughly) vertical.
    EXPECT_NEAR(m.collisionNormal.x, 0.0f, 1e-3f);
    EXPECT_NEAR(std::abs(m.collisionNormal.y), 1.0f, 1e-3f);

    // The deepest contact must report the real ~0.414 overlap, NOT ~0. The bug
    // this guards against collapsed this to zero.
    float deepest = 0.0f;
    for (int i = 0; i < m.contactCount; i++)
        deepest = std::max(deepest, m.penetrationDepth[i]);
    EXPECT_GT(deepest, 0.30f);
    EXPECT_LT(deepest, 0.50f);
}

// Sensitivity / control: lift B clear of A (bottom face at y = 2.5, above A's
// y = 1.414 apex) and there must be no collision -- proving the test above is
// reading a real overlap, not a scene that always reports one.
TEST(NarrowPhaseCollider2Reference, RotatedClear_NoCollision)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f, 45.0f);
    Transform2D tb = MakeTransform(0.0f, 3.5f, 0.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_FALSE(m.isColliding);
}
