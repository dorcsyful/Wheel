#include "../include/helpers/Collision2DManifold.h"
#include "systems/subsystems/BoxBoxCollision2D.h"

Wheel::Engine::Collision::Collision2DManifold Wheel::Engine::Collision::BoxBoxCollision2D::BoxBoxCollision(
Components::BoxCollider2D& a_Collider1, const Components::Transform2D& a_Transform1, Components::BoxCollider2D& a_Collider2, const Components::Transform2D& a_Transform2)
{
    Collision2DManifold manifold = Collision2DManifold(a_Collider1, a_Collider2);
    Math::Vector2 vertices1[4], vertices2[4];
    GetVertices(a_Collider1, a_Transform1, vertices1);
    GetVertices(a_Collider2, a_Transform2, vertices2);

    for (int i = 0; i < 4; i++)
    {
        Math::Vector2 edge = vertices1[(i + 1) % 4] - vertices1[i];

        float min1, max1, min2, max2;
        ProjectOntoAxis(vertices1, Math::Vector2(-edge.y,edge.x), min1, max1);
        ProjectOntoAxis(vertices2, Math::Vector2(-edge.y,edge.x), min2, max2);

        if (min1 >= max2 || min2 >= max1)
        {
            return manifold;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        Math::Vector2 edge = vertices2[(i + 1) % 4] - vertices2[i];

        float min1, max1, min2, max2;
        ProjectOntoAxis(vertices1, Math::Vector2(-edge.y,edge.x), min1, max1);
        ProjectOntoAxis(vertices2, Math::Vector2(-edge.y,edge.x), min2, max2);

        if (min1 >= max2 || min2 >= max1)
        {
            return manifold;
        }
    }

    manifold.isColliding = true;
    return manifold;
}

void Wheel::Engine::Collision::BoxBoxCollision2D::GetVertices(const Components::BoxCollider2D& a_Collider,
    const Components::Transform2D& a_Transform2D, Math::Vector2* a_Vertices)
{
    a_Vertices[0] = a_Transform2D.position + a_Collider.offset + Math::Vector2(-a_Collider.width / 2 * a_Transform2D.scale.x, -a_Collider.height / 2 * a_Transform2D.scale.y) * a_Transform2D.rotation;
    a_Vertices[1] = a_Transform2D.position + a_Collider.offset + Math::Vector2(a_Collider.width / 2 * a_Transform2D.scale.x, -a_Collider.height / 2 * a_Transform2D.scale.y) * a_Transform2D.rotation;
    a_Vertices[2] = a_Transform2D.position + a_Collider.offset + Math::Vector2(a_Collider.width / 2 * a_Transform2D.scale.x, a_Collider.height / 2 * a_Transform2D.scale.y) * a_Transform2D.rotation;
    a_Vertices[3] = a_Transform2D.position + a_Collider.offset + Math::Vector2(-a_Collider.width / 2 * a_Transform2D.scale.x, a_Collider.height / 2 * a_Transform2D.scale.y) * a_Transform2D.rotation;
}

void Wheel::Engine::Collision::BoxBoxCollision2D::ProjectOntoAxis(const Math::Vector2* a_Vertices,
    const Math::Vector2& a_Axis, float& a_Min, float& a_Max)
{
    a_Min = FLT_MAX;
    a_Max = -FLT_MAX;
    for (int i = 0; i < 4; i++)
    {
        float dot = a_Axis.Dot(a_Vertices[i]);
        if (dot < a_Min) a_Min = dot;
        if (dot > a_Max) a_Max = dot;
    }
}
