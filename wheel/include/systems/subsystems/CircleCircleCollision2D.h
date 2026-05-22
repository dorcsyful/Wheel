#pragma once
#include "components/Transform2D.h"
#include "helpers/Collision2DManifold.h"

namespace Wheel
{
    namespace Engine
    {
        namespace Physics
        {
            class ConstraintSolver;
            class CircleCircleCollision2D
            {
            public:
                static Collision::Collision2DManifold CheckCircleCircleCollision(
                    const Components::Transform2D& a_ATransform, const Components::Transform2D& a_BTransform,
                    const Components::CircleCollider2D& a_ACollider, const Components::CircleCollider2D& a_BCollider);
            };
        }
    }}
