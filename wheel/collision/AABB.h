#pragma once
#include <algorithm>
#include <cfloat>

#include "math/Vector2.h"
namespace Wheel
{
    namespace Collision
    {
        struct AABB
        {
        public:
            AABB() = default;
            AABB(const Math::Vector2& a_Min, const Math::Vector2& a_Max) : min(a_Min), max(a_Max) {}

            //Build AABB out of a list of vertices of arbitrary size
            void Build(const Math::Vector2* a_Vertices, int size)
            {
                min = Math::Vector2(FLT_MAX, FLT_MAX);
                max = Math::Vector2(-FLT_MAX, -FLT_MAX);

                for (int i = 0; i < size; i++)
                {
                    if (a_Vertices[i].x < min.x) min.x = a_Vertices[i].x;
                    if (a_Vertices[i].x > max.x) max.x = a_Vertices[i].x;
                    if (a_Vertices[i].y < min.y) min.y = a_Vertices[i].y;
                    if (a_Vertices[i].y > max.y) max.y = a_Vertices[i].y;
                }
            }
            //Builds the AABB of a circle
            void Build(const Math::Vector2& a_Position, float radius)
            {
                min = Math::Vector2(a_Position.x - radius, a_Position.y - radius);
                max = Math::Vector2(a_Position.x + radius, a_Position.y + radius);
            }

            AABB Combine(const AABB& other) const
            {
                AABB result;
                result.min.x = std::min(min.x, other.min.x);
                result.min.y = std::min(min.y, other.min.y);
                result.max.x = std::max(max.x, other.max.x);
                result.max.y = std::max(max.y, other.max.y);
                return result;
            }

            Math::Vector2 min;
            Math::Vector2 max;

        };
    }
}