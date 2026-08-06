#include "HeadlessSim.h"

#include <algorithm>

#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "collision/Collision2DSystem.h"
#include "physics/Rigidbody2D.h"
#include "physics/Joint2D.h"
#include "physics/Physics2DSystem.h"

Wheel::Core::Scene* Wheel::QA::CreateHeadlessScene()
{
    auto* scene = new Core::Scene();

    scene->RegisterComponentType<Common::Transform2D>();
    scene->RegisterComponentType<Collision::BoxCollider2D>();
    scene->RegisterComponentType<Collision::CircleCollider2D>();
    scene->RegisterComponentType<Physics::Rigidbody2D>();
    scene->RegisterComponentType<Physics::DistanceJoint2D>();

    Core::Description transformDesc = scene->GetComponentDescription<Common::Transform2D>();
    Core::Description rigidbodyDesc = scene->GetComponentDescription<Physics::Rigidbody2D>();

    // Registration order fixes update order: Collision2DSystem must run before
    // Physics2DSystem, which reads this frame's manifolds out of it.
    Core::Description collisionSystemDesc;
    collisionSystemDesc.AddComponentType(transformDesc.GetAsBitset());
    scene->RegisterSystem<Collision::Collision2DSystem>(collisionSystemDesc);

    Core::Description physicsSystemDesc;
    physicsSystemDesc.AddComponentType(transformDesc.GetAsBitset());
    physicsSystemDesc.AddComponentType(rigidbodyDesc.GetAsBitset());
    scene->RegisterSystem<Physics::Physics2DSystem>(physicsSystemDesc);

    return scene;
}

std::vector<Wheel::QA::EntitySnapshot> Wheel::QA::RunScene(Core::Scene& a_Scene, int a_NumSteps, float a_FixedDeltaTime)
{
    for (int i = 0; i < a_NumSteps; ++i)
        a_Scene.Update(a_FixedDeltaTime);

    std::vector<EntitySnapshot> result;
    auto transforms = a_Scene.GetComponents<Common::Transform2D>();
    result.reserve(transforms.size());
    for (auto& [id, transform] : transforms)
    {
        EntitySnapshot snapshot;
        snapshot.entityId = id;
        snapshot.position = transform->GetPosition();
        snapshot.rotation = transform->GetRotationInRadians();
        if (a_Scene.HasComponent<Physics::Rigidbody2D>(id))
        {
            Physics::Rigidbody2D& rigidbody = a_Scene.GetComponent<Physics::Rigidbody2D>(id);
            snapshot.linearVelocity = rigidbody.linearVelocity;
            snapshot.angularVelocity = rigidbody.angularVelocity;
        }
        result.push_back(snapshot);
    }

    // Sort so the result is independent of ComponentPool::GetComponents'
    // internal unordered_map iteration order.
    std::sort(result.begin(), result.end(),
        [](const EntitySnapshot& a, const EntitySnapshot& b) { return a.entityId < b.entityId; });

    return result;
}