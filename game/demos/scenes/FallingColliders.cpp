#include "FallingColliders.h"
#include "common/Transform2D.h"
#include "Physics/Rigidbody2D.h"
#include "collision/Collider2D.h"
#include "helpers/Coordinates.h"
#include "rendering/Sprite.h"
#include "rendering/RenderSystem.h"
#include "debug/Debugger.h"
void Wheel::Game::FallingColliders::Initialize(Core::Scene* scene)
{
    m_Scene = scene;
    CreateGroundCollider();
    CreateWallColliders();
    CreateCentralCircle();

    m_SubscriptionTokens.emplace_back();
    Wheel::EventSystem::EventBus::Subscribe<Wheel::Events::LeftMouseButtonPressEvent>(
    [&](const Wheel::Events::LeftMouseButtonPressEvent& e)
    {
        auto camera = m_Scene->GetComponents<Rendering::CameraComponent>();
        Rendering::CameraComponent* activeCamera = nullptr;
        Common::Transform2D* camTransform = nullptr;
        for (auto& [name, current] : camera)
        {
            if (current->IsActive())
            {
                activeCamera = current;
                camTransform = &m_Scene->GetComponent<Common::Transform2D>(name);
            }
        }
        if (!activeCamera || !camTransform) return;
        Math::Vector2 screenCoord = Math::Vector2(e.x,e.y);
        Wheel::Math::Vector2 world = Helpers::Coordinates::ScreenToWorldCoordinates(screenCoord,*activeCamera,*camTransform);
        SpawnObject(Math::Vector2(world.x,world.y));
    }, m_SubscriptionTokens.back());
}

void Wheel::Game::FallingColliders::SpawnObject(const Math::Vector2& a_Position)
{
    if (spawnBox) CreateBox(a_Position);
    else CreateCircle(a_Position);
}

void Wheel::Game::FallingColliders::CreateGroundCollider()
{
    uint32_t entityId = m_Scene->AddEntity();
    Common::Transform2D& transform = m_Scene->AddComponent<Common::Transform2D>(entityId);
    Wheel::Collision::BoxCollider2D& collider = m_Scene->AddComponent<Wheel::Collision::BoxCollider2D>(entityId);
    Rendering::Sprite& sprite = m_Scene->AddComponent<Rendering::Sprite>(entityId);
    Physics::Rigidbody2D& rigidbody = m_Scene->AddComponent<Physics::Rigidbody2D>(entityId);
    transform.SetPosition(0,-3);
    collider.SetWidth(10);
    collider.SetHeight(2);
    sprite.width = 10;
    sprite.height = 2;
    sprite.MaterialName = m_BaseMat;
    rigidbody.SetType(Physics::Rigidbody2DType::STATIC);
}

void Wheel::Game::FallingColliders::CreateWallColliders()
{
    uint32_t entityId = m_Scene->AddEntity();
    Common::Transform2D& transform = m_Scene->AddComponent<Common::Transform2D>(entityId);
    Wheel::Collision::BoxCollider2D& collider = m_Scene->AddComponent<Wheel::Collision::BoxCollider2D>(entityId);
    Rendering::Sprite& sprite = m_Scene->AddComponent<Rendering::Sprite>(entityId);
    Physics::Rigidbody2D& rigidbody = m_Scene->AddComponent<Physics::Rigidbody2D>(entityId);
    transform.SetPosition(-4,-0.5);
    collider.SetWidth(0.5);
    collider.SetHeight(2);
    sprite.width = 0.5;
    sprite.height = 3;
    sprite.MaterialName = m_BaseMat;
    rigidbody.SetType(Physics::Rigidbody2DType::STATIC);
}

void Wheel::Game::FallingColliders::CreateCentralCircle()
{
    uint32_t entityId = m_Scene->AddEntity();
    Common::Transform2D& transform = m_Scene->AddComponent<Common::Transform2D>(entityId);
    Wheel::Collision::CircleCollider2D& collider = m_Scene->AddComponent<Wheel::Collision::CircleCollider2D>(entityId);
    Rendering::Sprite& sprite = m_Scene->AddComponent<Rendering::Sprite>(entityId);
    Physics::Rigidbody2D& rigidbody = m_Scene->AddComponent<Physics::Rigidbody2D>(entityId);
    transform.SetPosition(0,1);
    collider.radius = 0.5;
    sprite.width = 1;
    sprite.height = 1;
    sprite.MaterialName = m_CircleMat;
    rigidbody.SetType(Physics::Rigidbody2DType::STATIC);
}

void Wheel::Game::FallingColliders::CreateCircle(const Math::Vector2& a_Position)
{
    uint32_t entityId = m_Scene->AddEntity();
    Common::Transform2D& transform = m_Scene->AddComponent<Common::Transform2D>(entityId);
    Wheel::Collision::CircleCollider2D& collider = m_Scene->AddComponent<Wheel::Collision::CircleCollider2D>(entityId);
    Rendering::Sprite& sprite = m_Scene->AddComponent<Rendering::Sprite>(entityId);
    Physics::Rigidbody2D& rigidbody = m_Scene->AddComponent<Physics::Rigidbody2D>(entityId);
    transform.SetPosition(a_Position);
    collider.radius = spawnedCircleRadius;
    sprite.width = spawnedCircleRadius * 2.f;
    sprite.height = spawnedCircleRadius * 2.f;
    sprite.MaterialName = m_CircleMat;
    sprite.color = Math::Vector4(0.5,0,0.75,1);
}

void Wheel::Game::FallingColliders::CreateBox(const Math::Vector2& a_Position)
{
    uint32_t entityId = m_Scene->AddEntity();
    Common::Transform2D& transform = m_Scene->AddComponent<Common::Transform2D>(entityId);
    Wheel::Collision::BoxCollider2D& collider = m_Scene->AddComponent<Wheel::Collision::BoxCollider2D>(entityId);
    Rendering::Sprite& sprite = m_Scene->AddComponent<Rendering::Sprite>(entityId);
    Physics::Rigidbody2D& rigidbody = m_Scene->AddComponent<Physics::Rigidbody2D>(entityId);
    transform.SetPosition(a_Position);
    collider.SetWidth(spawnedColliderWidth);
    collider.SetHeight(spawnedColliderHeight);
    sprite.width = spawnedColliderWidth;
    sprite.height = spawnedColliderHeight;
    sprite.MaterialName = m_BaseMat;
    sprite.color = Math::Vector4(0.5,0,0.75,1);
    rigidbody.SetMass(1);
}
