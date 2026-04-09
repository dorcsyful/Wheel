//
//This is the starting file. Register any custom systems you'd like to use here
#pragma once
#include <iostream>

#include <core/Scene.h>

#include "Renderer.h"
#include "systems/RenderSystem.h"
#include "core/Description.h"

//This is the class that communicates with the engine. Override any part of this to fit your needs
class Start
{
    public:
    Start()
    {
    }
    ~Start()
    {
    }

    void RegisterComponents()
    {
        m_Scene->RegisterComponentType<Wheel::Components::Transform2D>();
        m_Scene->RegisterComponentType<Wheel::Components::Render2DComponent>();

    }

    void RegisterSystems()
    {
        Wheel::Engine::Description transform = m_Scene->GetComponentDescription<Wheel::Components::Transform2D>();
        Wheel::Engine::Description render =m_Scene->GetComponentDescription<Wheel::Components::Render2DComponent>();
        Wheel::Engine::Description finalDesc = Wheel::Engine::Description();
        finalDesc.AddComponentType(transform.GetAsBitset());
        finalDesc.AddComponentType(render.GetAsBitset());
        m_Scene->RegisterSystem<Wheel::Engine::Systems::RenderSystem>(finalDesc);

    }

    void CreateEntities()
    {
        //Create your entities here
    }

    void Init()
    {
        m_Renderer = std::make_unique<Wheel::Renderer::Renderer>();
        m_Renderer->Init(1280, 720, "Wheel Engine");
        m_Scene = std::make_unique<Wheel::Engine::Scene>();
    }
    void Update();

    private:
    std::unique_ptr<Wheel::Renderer::Renderer> m_Renderer;
    std::unique_ptr<Wheel::Engine::Scene> m_Scene;
};


