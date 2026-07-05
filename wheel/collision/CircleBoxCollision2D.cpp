#include "CircleBoxCollision2D.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace Wheel::Collision;

Collision2DManifold CircleBoxCollision2D::CheckCircleBoxCollision(const Common::Transform2D& a_CircleTransform,
    const CircleCollider2D& a_CircleCollider, const Common::Transform2D& a_BoxTransform, const BoxCollider2D& a_BoxCollider)
{
    assert(std::abs(a_CircleTransform.GetScale().x - a_CircleTransform.GetScale().y) < 0.00001f && "Non-uniform scaling is not supported for circle colliders.");

    Collision2DManifold manifold = Collision2DManifold(a_CircleTransform.GetEntityId(), a_BoxTransform.GetEntityId());

    // Build the box's local frame from its cached (already rotated + scaled) vertices.
    // Winding: [0]=bottom-left, [1]=bottom-right, [2]=top-right, [3]=top-left, so
    // [1]-[0] is the width edge and [3]-[0] the height edge.
    Math::Vector2 boxCenter  = Math::Vector2::Lerp(a_BoxCollider.cachedVertices[0], a_BoxCollider.cachedVertices[2], 0.5f);
    Math::Vector2 widthEdge  = a_BoxCollider.cachedVertices[1] - a_BoxCollider.cachedVertices[0];
    Math::Vector2 heightEdge = a_BoxCollider.cachedVertices[3] - a_BoxCollider.cachedVertices[0];
    float halfWidth  = widthEdge.Length()  * 0.5f;
    float halfHeight = heightEdge.Length() * 0.5f;
    Math::Vector2 xAxis = widthEdge.Normalized();
    Math::Vector2 yAxis = heightEdge.Normalized();

    Math::Vector2 circleCenter = a_CircleTransform.GetPosition() + a_CircleCollider.offset;
    float radius = a_CircleCollider.radius * a_CircleTransform.GetScale().x; // uniform scale (asserted above)

    // Project the circle centre onto the box's axes, then clamp to the extents to
    // get the closest point on the box. Works for a rotated box because we measure
    // along its own axes rather than the world axes.
    Math::Vector2 d = circleCenter - boxCenter;
    float localX = d.Dot(xAxis);
    float localY = d.Dot(yAxis);
    float clampedX = std::max(-halfWidth,  std::min(localX, halfWidth));
    float clampedY = std::max(-halfHeight, std::min(localY, halfHeight));

    // If the clamp changed nothing the centre is inside (or exactly on) the box.
    bool inside = (clampedX == localX && clampedY == localY);

    if (inside)
    {
        float xPen = halfWidth  - std::abs(localX);
        float yPen = halfHeight - std::abs(localY);
        manifold.isColliding = true;
        manifold.contactCount = 1;
        if (xPen < yPen)
        {
            float faceSign = (localX < 0.0f) ? -1.0f : 1.0f;
            clampedX = faceSign * halfWidth;
            manifold.collisionNormal = xAxis * -faceSign;
            manifold.penetrationDepth[0] = radius + xPen;
        }
        else
        {
            float faceSign = (localY < 0.0f) ? -1.0f : 1.0f;
            clampedY = faceSign * halfHeight;
            manifold.collisionNormal = yAxis * -faceSign;
            manifold.penetrationDepth[0] = radius + yPen;
        }
        manifold.contactPoint[0] = boxCenter + xAxis * clampedX + yAxis * clampedY;
        return manifold;
    }

    Math::Vector2 closestPoint = boxCenter + xAxis * clampedX + yAxis * clampedY;
    Math::Vector2 toBox = closestPoint - circleCenter; // points circle (A) -> box (B)
    float distanceSquared = toBox.LengthSquared();
    if (distanceSquared > radius * radius)
        return manifold; // No collision

    float distance = std::sqrt(distanceSquared);
    manifold.isColliding = true;
    manifold.contactCount = 1;
    manifold.contactPoint[0] = closestPoint;
    manifold.collisionNormal = distance == 0 ? Math::Vector2(0,1) : toBox / distance;
    manifold.penetrationDepth[0] = radius - distance;
    return manifold;
}