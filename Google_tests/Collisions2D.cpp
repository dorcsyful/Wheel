#include <gtest/gtest.h>
#include <cmath>

#include "math/Vector2.h"
#include "components/Transform2D.h"
#include "components/Collider2D.h"
#include "helpers/Collision2DManifold.h"
#include "systems/subsystems/BoxBoxCollision2D.h"
#include "systems/subsystems/CircleBoxCollision2D.h"
#include "systems/subsystems/CircleCircleCollision2D.h"

using Wheel::Math::Vector2;
using Wheel::Components::Transform2D;
using Wheel::Components::BoxCollider2D;
using Wheel::Components::CircleCollider2D;
using Wheel::Engine::Collision::BoxBoxCollision2D;
using Wheel::Engine::Collision::CircleBoxCollision2D;
using Wheel::Engine::Collision::Collision2DManifold;
using Wheel::Engine::Physics::CircleCircleCollision2D;

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

    CircleCollider2D MakeCircle(float radius)
    {
        CircleCollider2D c;
        c.radius = radius;
        return c;
    }

    // Circle colliders require uniform scaling (the production code asserts it).
    Transform2D MakeCircleTransform(float x, float y)
    {
        return MakeTransform(x, y, 0.0f, Vector2(1.0f, 1.0f));
    }

    // Circle-box uses the box collider's cached vertices, which are populated by
    // BoxBoxCollision2D::GetVertices on first call (the function returns the cache).
    void PrimeBoxCache(const BoxCollider2D& b, const Transform2D& t)
    {
        BoxBoxCollision2D::GetVertices(b, t);
    }
} // namespace

// ============================================================================
// BoxBoxCollision
// ============================================================================

TEST(BoxBoxCollision, FarApart_NoCollision)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(10.0f, 0.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_FALSE(m.isColliding);
    EXPECT_EQ(m.contactCount, 0);
}

TEST(BoxBoxCollision, SeparatedAlongY_NoCollision)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(0.0f, 5.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_FALSE(m.isColliding);
}

TEST(BoxBoxCollision, OverlappingAABB_ReportsCollision)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(1.5f, 0.0f); // 0.5 X-overlap, full Y-overlap

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_TRUE(m.isColliding);
    EXPECT_GT(m.contactCount, 0);
}

TEST(BoxBoxCollision, OverlappingAABB_NormalPointsFromAtoB)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(1.5f, 0.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    ASSERT_TRUE(m.isColliding);
    EXPECT_NEAR(m.collisionNormal.x, 1.0f, 1e-5f);
    EXPECT_NEAR(m.collisionNormal.y, 0.0f, 1e-5f);
}

TEST(BoxBoxCollision, OverlappingAABB_PenetrationMatchesShorterAxis)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(1.5f, 0.0f); // expected penetration along X = 0.5

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    ASSERT_TRUE(m.isColliding);
    ASSERT_GT(m.contactCount, 0);
    for (int i = 0; i < m.contactCount; i++)
        EXPECT_NEAR(m.penetrationDepth[i], 0.5f, 1e-4f);
}

TEST(BoxBoxCollision, OverlappingAABB_StackedVertically_NormalPointsUp)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(0.0f, 1.5f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    ASSERT_TRUE(m.isColliding);
    EXPECT_NEAR(m.collisionNormal.x, 0.0f, 1e-5f);
    EXPECT_NEAR(m.collisionNormal.y, 1.0f, 1e-5f);
}

TEST(BoxBoxCollision, OverlappingAABB_ContactCountIsTwoForFaceContact)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(0.0f, 1.5f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    ASSERT_TRUE(m.isColliding);
    EXPECT_EQ(m.contactCount, 2);
}

TEST(BoxBoxCollision, ConcentricBoxes_ReportsCollision)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(1.0f, 1.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(0.0f, 0.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_TRUE(m.isColliding);
}

TEST(BoxBoxCollision, RotatedBox_StillDetectsOverlap)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    // 45-degree rotation pushes corner to ~sqrt(2); centers 2 apart still overlap.
    Transform2D tb = MakeTransform(2.0f, 0.0f, 45.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_TRUE(m.isColliding);
}

TEST(BoxBoxCollision, RotatedBox_FarApart_NoCollision)
{
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(5.0f, 0.0f, 30.0f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    EXPECT_FALSE(m.isColliding);
}

TEST(BoxBoxCollision, ContactPointsLieWithinSharedXRange)
{
    // Stack: A at origin, B directly above with identical X extent. Contact
    // points should lie in the overlap band x in [-1, 1], between the two
    // face y-values y in [0.5, 1.0]. (Which face the implementation reports
    // the contacts on is left as an implementation detail.)
    BoxCollider2D a = MakeBox(2.0f, 2.0f);
    BoxCollider2D b = MakeBox(2.0f, 2.0f);
    Transform2D ta = MakeTransform(0.0f, 0.0f);
    Transform2D tb = MakeTransform(0.0f, 1.5f);

    auto m = BoxBoxCollision2D::BoxBoxCollision(a, ta, b, tb);
    ASSERT_TRUE(m.isColliding);
    ASSERT_EQ(m.contactCount, 2);
    for (int i = 0; i < m.contactCount; i++)
    {
        EXPECT_GE(m.contactPoint[i].x, -1.0f - 1e-4f);
        EXPECT_LE(m.contactPoint[i].x,  1.0f + 1e-4f);
        EXPECT_GE(m.contactPoint[i].y,  0.5f - 1e-4f);
        EXPECT_LE(m.contactPoint[i].y,  1.0f + 1e-4f);
    }
}

// ============================================================================
// CircleCircleCollision
// ============================================================================

TEST(CircleCircleCollision, FarApart_NoCollision)
{
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(1.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    Transform2D tb = MakeCircleTransform(5.0f, 0.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    EXPECT_FALSE(m.isColliding);
    EXPECT_EQ(m.contactCount, 0);
}

TEST(CircleCircleCollision, Overlapping_ReportsCollision)
{
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(1.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    Transform2D tb = MakeCircleTransform(1.5f, 0.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    EXPECT_TRUE(m.isColliding);
    EXPECT_EQ(m.contactCount, 1);
}

TEST(CircleCircleCollision, Overlapping_PenetrationIsRadiusSumMinusDistance)
{
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(2.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    Transform2D tb = MakeCircleTransform(2.0f, 0.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    ASSERT_TRUE(m.isColliding);
    EXPECT_NEAR(m.penetrationDepth[0], 1.0f, 1e-5f); // (1+2) - 2 = 1
}

TEST(CircleCircleCollision, Overlapping_NormalPointsFromAtoB)
{
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(1.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    Transform2D tb = MakeCircleTransform(1.5f, 0.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    ASSERT_TRUE(m.isColliding);
    EXPECT_NEAR(m.collisionNormal.x, 1.0f, 1e-5f);
    EXPECT_NEAR(m.collisionNormal.y, 0.0f, 1e-5f);
}

TEST(CircleCircleCollision, Overlapping_NormalIsUnitLength)
{
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(1.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    Transform2D tb = MakeCircleTransform(1.0f, 1.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    ASSERT_TRUE(m.isColliding);
    EXPECT_NEAR(m.collisionNormal.Length(), 1.0f, 1e-5f);
}

TEST(CircleCircleCollision, Overlapping_ContactPointOnCircleASurface)
{
    // Contact point should be A's center + normal * radiusA.
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(1.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    Transform2D tb = MakeCircleTransform(1.5f, 0.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    ASSERT_TRUE(m.isColliding);
    EXPECT_NEAR(m.contactPoint[0].x, 1.0f, 1e-5f);
    EXPECT_NEAR(m.contactPoint[0].y, 0.0f, 1e-5f);
}

TEST(CircleCircleCollision, ExactlyTouching_ReportsCollisionWithZeroPenetration)
{
    // distance == radiusSum: condition is "distanceSquared > radiusSum^2" so equality
    // is treated as a collision with zero overlap.
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(1.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    Transform2D tb = MakeCircleTransform(2.0f, 0.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    EXPECT_TRUE(m.isColliding);
    EXPECT_NEAR(m.penetrationDepth[0], 0.0f, 1e-5f);
}

TEST(CircleCircleCollision, Concentric_PicksArbitraryNormalAndReportsCollision)
{
    CircleCollider2D a = MakeCircle(1.0f);
    CircleCollider2D b = MakeCircle(1.0f);
    Transform2D ta = MakeCircleTransform(2.0f, 3.0f);
    Transform2D tb = MakeCircleTransform(2.0f, 3.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    EXPECT_TRUE(m.isColliding);
    EXPECT_NEAR(m.collisionNormal.Length(), 1.0f, 1e-5f);
    EXPECT_EQ(m.contactCount, 1);
}

TEST(CircleCircleCollision, OffsetShiftsEffectiveCenter)
{
    // Both circles are at position (0,0), but A has offset (3,0). They should
    // separate by 3 along X (no overlap given radii of 1 each).
    CircleCollider2D a = MakeCircle(1.0f);
    a.offset = Vector2(3.0f, 0.0f);
    CircleCollider2D b = MakeCircle(1.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    Transform2D tb = MakeCircleTransform(0.0f, 0.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    EXPECT_FALSE(m.isColliding);
}

TEST(CircleCircleCollision, DiagonalOverlap_PenetrationCorrect)
{
    CircleCollider2D a = MakeCircle(2.0f);
    CircleCollider2D b = MakeCircle(2.0f);
    Transform2D ta = MakeCircleTransform(0.0f, 0.0f);
    // distance = sqrt(9 + 16) = 5. radiusSum = 4. -> no collision
    Transform2D tb = MakeCircleTransform(3.0f, 4.0f);

    auto m = CircleCircleCollision2D::CheckCircleCircleCollision(ta, tb, a, b);
    EXPECT_FALSE(m.isColliding);
}

// ============================================================================
// CircleBoxCollision
// ============================================================================

TEST(CircleBoxCollision, FarApart_NoCollision)
{
    CircleCollider2D circle = MakeCircle(1.0f);
    BoxCollider2D box = MakeBox(2.0f, 2.0f);
    Transform2D circleT = MakeCircleTransform(10.0f, 0.0f);
    Transform2D boxT = MakeTransform(0.0f, 0.0f);
    PrimeBoxCache(box, boxT);

    auto m = CircleBoxCollision2D::CheckCircleBoxCollision(circleT, circle, boxT, box);
    EXPECT_FALSE(m.isColliding);
}

TEST(CircleBoxCollision, CircleInsideBox_ReportsCollision)
{
    CircleCollider2D circle = MakeCircle(0.5f);
    BoxCollider2D box = MakeBox(4.0f, 4.0f);
    Transform2D circleT = MakeCircleTransform(0.0f, 0.0f);
    Transform2D boxT = MakeTransform(0.0f, 0.0f);
    PrimeBoxCache(box, boxT);

    auto m = CircleBoxCollision2D::CheckCircleBoxCollision(circleT, circle, boxT, box);
    EXPECT_TRUE(m.isColliding);
    EXPECT_EQ(m.contactCount, 1);
}

TEST(CircleBoxCollision, CircleOverlappingRightFace_ReportsCollision)
{
    // Box [-1,1]x[-1,1]. Circle at (1.5, 0) radius 1 -> overlaps the right face by 0.5.
    CircleCollider2D circle = MakeCircle(1.0f);
    BoxCollider2D box = MakeBox(2.0f, 2.0f);
    Transform2D circleT = MakeCircleTransform(1.5f, 0.0f);
    Transform2D boxT = MakeTransform(0.0f, 0.0f);
    PrimeBoxCache(box, boxT);

    auto m = CircleBoxCollision2D::CheckCircleBoxCollision(circleT, circle, boxT, box);
    EXPECT_TRUE(m.isColliding);
}

TEST(CircleBoxCollision, CircleClearOfBox_NoCollision)
{
    // Box [-1,1]x[-1,1]. Circle at (5, 5) radius 0.5 -> far from any face.
    CircleCollider2D circle = MakeCircle(0.5f);
    BoxCollider2D box = MakeBox(2.0f, 2.0f);
    Transform2D circleT = MakeCircleTransform(5.0f, 5.0f);
    Transform2D boxT = MakeTransform(0.0f, 0.0f);
    PrimeBoxCache(box, boxT);

    auto m = CircleBoxCollision2D::CheckCircleBoxCollision(circleT, circle, boxT, box);
    EXPECT_FALSE(m.isColliding);
}

TEST(CircleBoxCollision, CollidingNormalIsUnitLength)
{
    // Offset both axes so the closest-point delta is non-zero and the normalize
    // produces a well-defined unit vector.
    CircleCollider2D circle = MakeCircle(1.0f);
    BoxCollider2D box = MakeBox(2.0f, 2.0f);
    Transform2D circleT = MakeCircleTransform(1.5f, 0.5f);
    Transform2D boxT = MakeTransform(0.0f, 0.0f);
    PrimeBoxCache(box, boxT);

    auto m = CircleBoxCollision2D::CheckCircleBoxCollision(circleT, circle, boxT, box);
    if (m.isColliding)
        EXPECT_NEAR(m.collisionNormal.Length(), 1.0f, 1e-4f);
}

TEST(CircleBoxCollision, PenetrationNonNegative)
{
    CircleCollider2D circle = MakeCircle(1.0f);
    BoxCollider2D box = MakeBox(2.0f, 2.0f);
    Transform2D circleT = MakeCircleTransform(1.5f, 0.5f);
    Transform2D boxT = MakeTransform(0.0f, 0.0f);
    PrimeBoxCache(box, boxT);

    auto m = CircleBoxCollision2D::CheckCircleBoxCollision(circleT, circle, boxT, box);
    if (m.isColliding)
        EXPECT_GE(m.penetrationDepth[0], 0.0f);
}