#include <gtest/gtest.h>

#include "math/Vector2.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "collision/Collision2DManifold.h"
#include "collision/BoxBoxCollision2D.h"
#include "collision/CircleBoxCollision2D.h"
#include "collision/CircleCircleCollision2D.h"

using Wheel::Math::Vector2;
using Wheel::Common::Transform2D;
using Wheel::Collision::BoxCollider2D;
using Wheel::Collision::CircleCollider2D;
using Wheel::Collision::BoxBoxCollision2D;
using Wheel::Collision::CircleBoxCollision2D;
using Wheel::Collision::CircleCircleCollision2D;
using Wheel::Collision::Collision2DManifold;

// Transform scale feeds the collider size at collision time -- radius * scale
// for circles, half-extent * scale for boxes. The whole hand-written suite
// uses scale (1,1), so these paths (and the effective-radius / effective-extent
// arithmetic) have never been exercised with a scale != 1. Uniform scale only
// here: circle collision asserts uniform scaling, so a non-uniform circle would
// trip that assert rather than exercise a meaningful path.
namespace
{
    Transform2D MakeTransform(float x, float y, float scale = 1.0f, float rotationDeg = 0.0f)
    {
        Transform2D t;
        t.SetScale(Vector2(scale, scale));
        t.SetRotationInDegrees(rotationDeg);
        t.SetPosition(x, y);
        return t;
    }
    BoxCollider2D MakeBox(float w, float h) { BoxCollider2D b; b.SetWidth(w); b.SetHeight(h); return b; }
    CircleCollider2D MakeCircle(float r) { CircleCollider2D c; c.radius = r; return c; }
    void PrimeBoxCache(const BoxCollider2D& b, const Transform2D& t) { BoxBoxCollision2D::GetVertices(b, t); }
}

// Two r=1 circles at uniform scale 2 -> effective radius 2 each, radiusSum 4.
// Centres 3 apart overlap by 1; 5 apart are clear. Without the scale factor the
// radiusSum would be 2 and neither would register.
TEST(ScaledColliderCollision, CircleCircle_ScaleGrowsEffectiveRadius)
{
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(1.0f);

    auto hit = CircleCircleCollision2D::CheckCircleCircleCollision(
        MakeTransform(0.0f, 0.0f, 2.0f), MakeTransform(3.0f, 0.0f, 2.0f), a, b);
    ASSERT_TRUE(hit.isColliding);
    EXPECT_NEAR(hit.penetrationDepth[0], 1.0f, 1e-4f); // (2+2) - 3

    auto clear = CircleCircleCollision2D::CheckCircleCircleCollision(
        MakeTransform(0.0f, 0.0f, 2.0f), MakeTransform(5.0f, 0.0f, 2.0f), a, b);
    EXPECT_FALSE(clear.isColliding); // 5 > radiusSum 4
}

// r=1 circle at scale 2 (effective radius 2) vs a 2x2 box (faces at +/-1).
// Circle centre at x=2.5 sits 1.5 from the right face -> overlaps by 0.5; at
// x=4 it is 3 away and clear. Unscaled (radius 1) it would miss the x=2.5 case.
TEST(ScaledColliderCollision, CircleBox_ScaleGrowsEffectiveRadius)
{
    CircleCollider2D circle = MakeCircle(1.0f);
    BoxCollider2D box = MakeBox(2.0f, 2.0f);
    Transform2D boxT = MakeTransform(0.0f, 0.0f);
    PrimeBoxCache(box, boxT);

    auto hit = CircleBoxCollision2D::CheckCircleBoxCollision(MakeTransform(2.5f, 0.0f, 2.0f), circle, boxT, box);
    ASSERT_TRUE(hit.isColliding);
    EXPECT_NEAR(hit.penetrationDepth[0], 0.5f, 1e-4f); // 2 - 1.5

    auto clear = CircleBoxCollision2D::CheckCircleBoxCollision(MakeTransform(4.0f, 0.0f, 2.0f), circle, boxT, box);
    EXPECT_FALSE(clear.isColliding);
}

// A 2x2 box at scale 2 spans [-2,2]; an unscaled 2x2 box at x=2.9 spans
// [1.9,3.9], overlapping by 0.1. At x=5 (spans [4,6]) they are clear. The
// scaled half-extent (1 * 2 = 2) is what puts A's right face at x=2.
TEST(ScaledColliderCollision, BoxBox_ScaleGrowsHalfExtent)
{
    // Fresh colliders per call: BoxCollider2D caches its world vertices and only
    // recomputes when isDirty, so reusing one object across two transforms would
    // read stale vertices from the first call.
    {
        BoxCollider2D a = MakeBox(2.0f, 2.0f);
        BoxCollider2D b = MakeBox(2.0f, 2.0f);
        auto hit = BoxBoxCollision2D::BoxBoxCollision(a, MakeTransform(0.0f, 0.0f, 2.0f), b, MakeTransform(2.9f, 0.0f, 1.0f));
        ASSERT_TRUE(hit.isColliding);
        ASSERT_GT(hit.contactCount, 0);
        EXPECT_NEAR(hit.collisionNormal.x, 1.0f, 1e-4f);
        for (int i = 0; i < hit.contactCount; i++)
            EXPECT_NEAR(hit.penetrationDepth[i], 0.1f, 1e-3f);
    }
    {
        BoxCollider2D a = MakeBox(2.0f, 2.0f);
        BoxCollider2D b = MakeBox(2.0f, 2.0f);
        auto clear = BoxBoxCollision2D::BoxBoxCollision(a, MakeTransform(0.0f, 0.0f, 2.0f), b, MakeTransform(5.0f, 0.0f, 1.0f));
        EXPECT_FALSE(clear.isColliding);
    }
}
