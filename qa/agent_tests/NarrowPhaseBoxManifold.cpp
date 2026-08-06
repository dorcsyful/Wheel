#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>

#include "math/Vector2.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "collision/Collision2DManifold.h"
#include "collision/BoxBoxCollision2D.h"

using Wheel::Math::Vector2;
using Wheel::Common::Transform2D;
using Wheel::Collision::BoxCollider2D;
using Wheel::Collision::BoxBoxCollision2D;
using Wheel::Collision::Collision2DManifold;

// Narrow-phase SAT box-box manifold geometry. The hand-written Collisions2D.cpp
// asserts the *fact* of collision and loose ranges; these pin the exact
// decisions the full mutation sweep found unguarded: the separating-axis gap
// boundary (min1 >= max2), and the Sutherland-Hodgman clip that fixes contact
// point positions and count.
namespace
{
    Transform2D MakeTransform(float x, float y, float rotationDeg = 0.0f,
                              Vector2 scale = Vector2(1.0f, 1.0f))
    {
        Transform2D t;
        t.SetScale(scale);
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

// Two unit-half boxes whose faces meet exactly at x = 1. The gap test
// `min1 >= max2 || min2 >= max1` treats an exact touch (min2 == max1 == 1) as
// separation. Mutating `>=` to `>` would flip this to a (zero-overlap)
// collision. Boundary values here are exact in float (1.0), so this is a
// deterministic kill of that mutant, not a fuzzy near-miss.
TEST(NarrowPhaseBoxManifold, ExactFaceTouch_IsTreatedAsSeparated)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(2.0f, 0.0f); // right face of A and left face of B both at x=1

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_FALSE(m.isColliding);
    EXPECT_EQ(m.contactCount, 0);
}

// A directly above B, full X-overlap: a face-face contact. The two contact
// points must be the endpoints of the shared overlap band, x = -1 and x = +1,
// on the incident (B bottom) edge at y = 0.5. The existing test only bounds
// these to a rectangle; asserting the exact endpoints pins the clip's
// projection bounds and keep/discard predicate.
TEST(NarrowPhaseBoxManifold, FaceStack_ContactPointsAreOverlapBandEndpoints)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(0.0f, 1.5f); // 0.5 Y-overlap, full X-overlap

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    ASSERT_TRUE(m.isColliding);
    ASSERT_EQ(m.contactCount, 2);

    float xs[2] = { m.contactPoint[0].x, m.contactPoint[1].x };
    std::sort(xs, xs + 2);
    EXPECT_NEAR(xs[0], -1.0f, 1e-4f);
    EXPECT_NEAR(xs[1],  1.0f, 1e-4f);
    for (int i = 0; i < m.contactCount; i++)
    {
        EXPECT_NEAR(m.contactPoint[i].y, 0.5f, 1e-4f); // B's bottom edge
        EXPECT_NEAR(m.penetrationDepth[i], 0.5f, 1e-4f);
    }
    EXPECT_NEAR(m.collisionNormal.x, 0.0f, 1e-5f);
    EXPECT_NEAR(m.collisionNormal.y, 1.0f, 1e-5f); // points A -> B, upward
}

// Sensitivity companion: a hair of separation (faces at x=1.0 and x=1.001)
// must remain non-colliding -- proves the touch test above is reading a real
// boundary, not accidentally always-false.
TEST(NarrowPhaseBoxManifold, SlightlySeparated_NoCollision)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(2.001f, 0.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_FALSE(m.isColliding);
}
