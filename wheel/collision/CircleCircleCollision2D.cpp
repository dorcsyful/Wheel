#include "CircleCircleCollision2D.h"

#include <cassert>

Wheel::Collision::Collision2DManifold Wheel::Collision::CircleCircleCollision2D::
CheckCircleCircleCollision(const Common::Transform2D& a_ATransform, const Common::Transform2D& a_BTransform,
    const CircleCollider2D& a_ACollider, const CircleCollider2D& a_BCollider)
{
    assert(std::abs(a_ATransform.GetScale().x - a_ATransform.GetScale().y) < 0.00001f && "Non-uniform scaling is not supported for circle colliders.");
    Collision2DManifold manifold(a_ATransform.GetEntityId(), a_BTransform.GetEntityId());

    Math::Vector2 a_center = a_ATransform.GetPosition() + a_ACollider.offset;
    Math::Vector2 b_center = a_BTransform.GetPosition() + a_BCollider.offset;
    Math::Vector2 ab = b_center - a_center;
    float radiusSum = a_ACollider.radius * std::min(a_ATransform.GetScale().x, a_ATransform.GetScale().y) + a_BCollider.radius * std::min(a_BTransform.GetScale().x, a_BTransform.GetScale().y);
    float distanceSquared = ab.Dot(ab);
    if (distanceSquared > radiusSum * radiusSum)
        return manifold; // No collision

    float distance = std::sqrt(distanceSquared);
    if (distance != 0.0f)
    {
        manifold.isColliding = true;
        manifold.penetrationDepth[0] = radiusSum - distance;
        manifold.collisionNormal = ab / distance; // Normalize the vector
        manifold.contactPoint[0] = a_center + manifold.collisionNormal * a_ACollider.radius;
        manifold.contactCount = 1;
    }
    else
    {
        // Circles are in the same position, choose arbitrary normal
        manifold.isColliding = true;
        manifold.penetrationDepth[0] = a_ACollider.radius; // or a_BRigidbody.radius, they are the same
        manifold.collisionNormal = Math::Vector2(1.0f, 0.0f); // Arbitrary normal
        manifold.contactPoint[0] = a_center; // or b_center, they are the same
        manifold.contactCount = 1;
    }

    return manifold;
}
