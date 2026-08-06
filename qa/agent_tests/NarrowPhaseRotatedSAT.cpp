#include <gtest/gtest.h>

#include "math/Vector2.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "collision/Collision2DManifold.h"
#include "collision/BoxBoxCollision2D.h"

using Wheel::Math::Vector2;
using Wheel::Common::Transform2D;
using Wheel::Collision::BoxCollider2D;
using Wheel::Collision::BoxBoxCollision2D;

// The separating-axis gap test `min1 >= max2 || min2 >= max1` is evaluated
// identically for BOTH colliders (the first loop over collider1's axes, the
// second over collider2's). On axis-aligned boxes the two colliders share the
// same world axes, so a mutation to one loop's condition is masked by the
// intact check in the other loop -- which is why the full mutation sweep left
// the `||`->`&&` variants on both those lines alive.
//
// A rotated box breaks that symmetry: positioned so exactly ONE collider's
// axis is the sole separating axis, a mutation to that loop's condition is no
// longer covered by the other loop, and the pair is reported as colliding.
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

// collider1 (A) axis-aligned, collider2 (B) rotated 45 deg and placed to its
// right. B's half-diagonal along world-X is sqrt(2) ~= 1.414, so at centre
// x = 2.464 its leftmost point sits at ~1.05, a 0.05 gap past A's right face
// (x = 1). Only A's +X axis separates them (both boxes overlap on Y and on
// B's two diagonal axes), so this exercises the FIRST loop's gap test alone.
TEST(NarrowPhaseRotatedSAT, SoleSeparatingAxisOnCollider1_NotColliding)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f, 0.0f);
    Transform2D tb = MakeTransform(2.464f, 0.0f, 45.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_FALSE(m.isColliding);
    EXPECT_EQ(m.contactCount, 0);
}

// Mirror: collider1 (A) rotated 45 deg, collider2 (B) axis-aligned to its
// right. Now only B's +X axis separates them, exercising the SECOND loop's
// gap test alone.
TEST(NarrowPhaseRotatedSAT, SoleSeparatingAxisOnCollider2_NotColliding)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f, 45.0f);
    Transform2D tb = MakeTransform(2.464f, 0.0f, 0.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_FALSE(m.isColliding);
    EXPECT_EQ(m.contactCount, 0);
}

// Sanity anchor: nudge the rotated box into real overlap and confirm the same
// configuration DOES collide -- so the two tests above are reading the gap
// boundary, not a scene that never touches.
TEST(NarrowPhaseRotatedSAT, RotatedOverlap_Collides)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f, 0.0f);
    Transform2D tb = MakeTransform(2.0f, 0.0f, 45.0f); // leftmost ~0.586 < 1.0 -> overlap

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_TRUE(m.isColliding);
}
