#include "NewtonsCradle.h"

#include "collision/Collider2D.h"
#include "common/Transform2D.h"
#include "physics/Joint2D.h"
#include "physics/Rigidbody2D.h"
#include "rendering/Sprite.h"

void Wheel::Game::NewtonsCradle::Initialize(Core::Scene* scene)
{
    m_Scene = scene;
    float width = 0.5f;
    float gap   = 0.002f;
    float step  = width + gap;   // center spacing: the gap MUST be in the step, or neighbours stay exactly touching
    float x0    = -1.0f;         // ball 0

    for (int i = 0; i < 4; i++)
    {
        float x = x0 + i * step;
        uint32_t entityId = SpawnObject(x, 0.f, width, width);
        Wheel::Physics::DistanceJoint2D& joint = m_Scene->AddComponent<Wheel::Physics::DistanceJoint2D>(entityId);
        joint.distance = 1.f;
        joint.otherAnchorPoint = Wheel::Math::Vector2(x, 1.f);   // pivot directly above
    }
    {
        // Striker: one step left of ball 0, pulled back to horizontal and taut (distance == rest length).
        float pivotX   = x0 - step;
        float strikerX = pivotX - 1.0f;
        uint32_t entityId = SpawnObject(strikerX, 0.f, width, width);
        Wheel::Physics::DistanceJoint2D& joint = m_Scene->AddComponent<Wheel::Physics::DistanceJoint2D>(entityId);
        joint.distance = 1.f;
        joint.otherAnchorPoint = Wheel::Math::Vector2(pivotX, 1.f);
    }

}

uint32_t Wheel::Game::NewtonsCradle::SpawnObject(float x, float y, float width, float height)
{
    uint32_t entityId = m_Scene->AddEntity();
    Common::Transform2D& transform = m_Scene->AddComponent<Common::Transform2D>(entityId);
    Wheel::Collision::CircleCollider2D& collider = m_Scene->AddComponent<Wheel::Collision::CircleCollider2D>(entityId);
    Rendering::Sprite& sprite = m_Scene->AddComponent<Rendering::Sprite>(entityId);
    Physics::Rigidbody2D& rigidbody = m_Scene->AddComponent<Physics::Rigidbody2D>(entityId);
    transform.SetPosition(x,y);
    collider.radius = width / 2.f;
    sprite.width = width;
    sprite.height = height;
    sprite.MaterialName = m_CircleMaterial;
    rigidbody.SetMass(0.5);
    rigidbody.restitution = 1.0f;   // elastic collisions: energy passes through the chain instead of being absorbed
    return entityId;
}
