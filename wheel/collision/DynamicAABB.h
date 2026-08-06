#pragma once
#include <vector>
#include "AABB.h"
namespace Wheel { namespace Core { class Scene; } }

namespace Wheel
{
    namespace Collision
    {
        class Node
        {
        public:
            Node() = default;
            Node(Node* parent, const AABB& aabb, uint32_t objectID);

            bool IsLeaf() {
                return children[0] == nullptr && children[1] == nullptr;
            }

            void UpdateAABB(float margin)
            {
                if (IsLeaf())
                {
                    Math::Vector2 margin_vector(margin, margin);
                    aabb.min = data->min - margin_vector;
                    aabb.max = data->max + margin_vector;
                }
                else
                {
                    aabb = children[0]->aabb.Combine(children[1]->aabb);
                }
            }
            AABB* data;
            AABB aabb;
            Node* parent = nullptr;
            Node* children[2] = { nullptr, nullptr };
        };

        class DynamicAabb
        {
        public:
            DynamicAabb();

        private:
            Node* m_RootNode;
            Core::Scene* m_Scene = nullptr;
            //enlarging the AABB to reduce how often the tree needs to be rebuilt
            float margin;
            std::vector<std::pair<uint32_t,uint32_t>> m_CollisionPairs;

        };
    }
}
