//
// Created by Doortje on 03/07/2026.
//

#include "MouseEvents.h"

#include "../../wheel/rendering/Highlight2D.h"
#include "../../wheel/physics/Rigidbody2D.h"
#include "../../wheel/rendering/Sprite.h"
#include "common//Transform2D.h"
#include "../../wheel/rendering/RenderSystem.h"
using namespace Wheel;
uint32_t MouseEvents::SpawnObjects(float x, float y, float w, float h)
{
    uint32_t id = m_Scene->AddEntity();
    auto& transform = m_Scene->AddComponent<Common::Transform2D>(id);
    auto& sprite    = m_Scene->AddComponent<Rendering::Sprite>(id);
    auto& collider  = m_Scene->AddComponent<Collision::CircleCollider2D>(id);
    auto& rigidbody = m_Scene->AddComponent<Physics::Rigidbody2D>(id);
    rigidbody.SetMass(2.0f); rigidbody.friction = 0.05f;
    collider.radius = w / 2.0f;
    rigidbody.restitution = 0.1f;
    transform.SetPosition(x, y);
    transform.SetScale(1.0f, 1.0f);
    rigidbody.linearDamping = 0.0001; rigidbody.angularDamping = 0.0001;
    transform.SetRotationInDegrees(0);
    sprite.width = w; sprite.height = w; sprite.color = Wheel::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    sprite.MaterialName = m_BaseMaterial;
    auto& highlight = m_Scene->AddComponent<Wheel::Rendering::Highlight2D>(id);
    highlight.thickness = 4;
    highlight.active = false;
    highlight.color = Wheel::Math::Vector4(1.0f, 0.85f, 0.1f, 1.0f);
    return id;
}

void MouseEvents::Initialize()
{
    CreateGroundEntity();
    Wheel::EventSystem::EventBus::Subscribe<Wheel::Events::LeftMouseButtonPressEvent>(
[&](const Wheel::Events::LeftMouseButtonPressEvent& e)
    { Wheel::Math::Vector2 world = m_Scene->GetSystem<Rendering::RenderSystem>()->ScreenToWorld(e.x,e.y);
        SpawnObjects(world.x,world.y, 0.5,0.5); }, m_SubscriptionTokens.back());

}

void MouseEvents::CreateGroundEntity()
{
    uint32_t ground = m_Scene->AddEntity();
    auto& transform = m_Scene->AddComponent<Wheel::Common::Transform2D>(ground);
    auto& sprite = m_Scene->AddComponent<Wheel::Rendering::Sprite>(ground);
    auto& collider  = m_Scene->AddComponent<Wheel::Collision::BoxCollider2D>(ground);
    auto& rigidbody = m_Scene->AddComponent<Wheel::Physics::Rigidbody2D>(ground);
    rigidbody.SetMass(100.0f); rigidbody.friction = 0.5f; rigidbody.SetType(Wheel::Physics::Rigidbody2DType::STATIC);
    collider.SetWidth(13.f); collider.SetHeight(2.f);
    transform.SetPosition(0.0f, -3.0f);
    transform.SetScale(1.0f, 1.0f);
    transform.SetRotationInDegrees(0);
    sprite.width = 13.f; sprite.height = 2.f; sprite.color = Wheel::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    sprite.MaterialName = m_BaseMaterial;
}
