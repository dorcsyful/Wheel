//
//This is the starting file. Register any custom systems you'd like to use here
#pragma once
#include <iostream>
#include <memory>

#include <core/Scene.h>

#include "Renderer.h"
#include "systems/RenderSystem.h"
#include "core/Description.h"
#include "components/CameraComponent.h"
#include "systems/InputSystem.h"


//This is the class that communicates with the engine. Override any part of this to fit your needs
class Start
{
    public:
    Start()
    = default;
    ~Start()
    = default;

    void RegisterComponents()
    {
        m_Scene->RegisterComponentType<Wheel::Components::Transform2D>();
        m_Scene->RegisterComponentType<Wheel::Components::Render2DComponent>();
        m_Scene->RegisterComponentType<Wheel::Components::CameraComponent>();
    }

    void RegisterSystems()
    {
        Wheel::Engine::Description transform = m_Scene->GetComponentDescription<Wheel::Components::Transform2D>();
        Wheel::Engine::Description render = m_Scene->GetComponentDescription<Wheel::Components::Render2DComponent>();
        Wheel::Engine::Description finalDesc = Wheel::Engine::Description();
        Wheel::Engine::Description cameraDesc = m_Scene->GetComponentDescription<Wheel::Components::CameraComponent>();
        finalDesc.AddComponentType(transform.GetAsBitset());
        finalDesc.AddComponentType(render.GetAsBitset());
        m_RenderSystem = m_Scene->RegisterSystem<Wheel::Engine::Systems::RenderSystem>(finalDesc);
        m_RenderSystem->GetRenderer(m_Renderer.get());
        finalDesc = Wheel::Engine::Description();
        finalDesc.AddComponentType(transform.GetAsBitset());
        finalDesc.AddComponentType(cameraDesc.GetAsBitset());
        m_InputSystem = m_Scene->RegisterSystem<Wheel::Engine::Systems::InputSystem>(finalDesc);
        m_InputSystem->Initialize(m_Renderer->GetWindow());
    }

    void CreateEntities()
    {
        // Camera entity — has Transform2D + CameraComponent but no Render2DComponent
        uint32_t cameraId = m_Scene->AddEntity();
        m_Scene->AddComponent<Wheel::Components::Transform2D>(cameraId);
        Wheel::Components::CameraComponent& cam =
            m_Scene->AddComponent<Wheel::Components::CameraComponent>(cameraId);
        cam.SetCameraActive(true);
        cam.zoom = 1.0f;
        cam.width  = 1280.0f;
        cam.height = 720.0f;
        m_RenderSystem->SetCameraEntity(cameraId);
        m_InputSystem->SetCameraEntity(cameraId);
        m_CameraId = cameraId;
        for (int i = 0; i < 100; i++)
        {
            uint32_t id = m_Scene->AddEntity();
            Wheel::Components::Transform2D& transform =
                m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
            Wheel::Components::Render2DComponent& render =
                m_Scene->AddComponent<Wheel::Components::Render2DComponent>(id);
            transform.position.x = (float)random(-3, 3);
            transform.position.y = (float)random(-3, 3);
            transform.scale.x = 1.0f;
            transform.scale.y = 1.0f;
            transform.rotation = (float)random(-180, 180);
            render.width = 0.5f; render.height = 0.3f;
        }
    }

    void Init()
    {
        m_Renderer = std::make_unique<Wheel::Renderer::Renderer>();
        m_Renderer->Init(1280, 720, "Wheel Engine");
        m_Scene = std::make_unique<Wheel::Engine::Scene>();
        m_SubscriptionTokens = std::vector<Wheel::EventSystem::SubscriptionToken>();
        RegisterComponents();
        RegisterSystems();
        CreateEntities();
        m_SubscriptionTokens.emplace_back();
        Wheel::EventSystem::EventBus::Subscribe<Wheel::Events::LeftMouseButtonPressEvent>(
            [this](const Wheel::Events::LeftMouseButtonPressEvent& e)
            {
                SpawnAtMousePosition(Wheel::Math::Vector2(e.x,e.y));
            },m_SubscriptionTokens[0]);
    }

    void SpawnAtMousePosition(Wheel::Math::Vector2 mousePosition);

    void Update();
    Wheel::Engine::Scene* GetScene() { return m_Scene.get(); }
    private:
    uint32_t m_CameraId;
    std::unique_ptr<Wheel::Renderer::Renderer> m_Renderer;
    std::unique_ptr<Wheel::Engine::Scene> m_Scene;
    Wheel::Engine::Systems::RenderSystem* m_RenderSystem = nullptr;
    Wheel::Engine::Systems::InputSystem* m_InputSystem = nullptr;
    std::vector<Wheel::EventSystem::SubscriptionToken> m_SubscriptionTokens;
};


