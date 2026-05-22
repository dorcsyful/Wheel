#pragma once
#include "systems/subsystems/CircleBoxCollision2D.h"

#include <cassert>

using namespace Wheel::Engine::Collision;

Collision2DManifold CircleBoxCollision2D::CheckCircleBoxCollision(const Components::Transform2D& a_CircleTransform,
    const Components::CircleCollider2D& a_CircleCollider, const Components::Transform2D& a_BoxTransform, const Components::BoxCollider2D& a_BoxCollider)
{
    assert(std::abs(a_CircleTransform.GetScale().x - a_CircleTransform.GetScale().y) > 0.00001f && "Non-uniform scaling is not supported for circle colliders.");

    Collision2DManifold manifold = Collision2DManifold(a_CircleTransform.GetEntityId(), a_BoxTransform.GetEntityId());
    Math::Vector2 boxCenter = Math::Vector2::Lerp(a_BoxCollider.cachedVertices[0], a_BoxCollider.cachedVertices[2], 0.5f);
    Math::Vector2 boxWidth = a_BoxCollider.cachedVertices[1] - a_BoxCollider.cachedVertices[0];
    Math::Vector2 boxHeight = a_BoxCollider.cachedVertices[3] - a_BoxCollider.cachedVertices[0];
    Math::Vector2 circleCenter = a_CircleTransform.GetPosition() + a_CircleCollider.offset;
    auto closestX = std::max(boxCenter.x, std::min(circleCenter.x, boxWidth.x));
    auto closestY = std::max(boxCenter.y, std::min(circleCenter.y, boxWidth.y));

    auto distanceX = circleCenter.x - closestX;
    auto distanceY = circleCenter.y - closestY;
    auto distanceSquared = distanceX * distanceX + distanceY * distanceY;
    float radius = a_CircleCollider.radius * a_CircleTransform.GetScale().x; // Assuming uniform scaling
    if (distanceSquared > radius * radius)
    {
        return manifold; // No collision
    }
    else
    {
        manifold.isColliding = true;
        manifold.collisionNormal = Math::Vector2(distanceX, distanceY).Normalized();
        manifold.penetrationDepth[0] = radius - std::sqrt(distanceSquared);
        manifold.contactPoint[0] = Math::Vector2(closestX, closestY);
        manifold.contactCount = 1;
        return manifold;
    }
}