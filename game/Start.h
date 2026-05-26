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

    void RegisterComponents();
    void RegisterSystems();
    void SpawnObject(float x, float y, float w = 0.9f, float h = 0.5f);
    void CreateEntities();
    void Init();


    void Update();
    Wheel::Engine::Scene* GetScene() { return m_Scene.get(); }

private:
    bool m_RunSimulation;
    uint32_t m_CameraId;
    std::unique_ptr<Wheel::Renderer::Renderer> m_Renderer;
    std::unique_ptr<Wheel::Engine::Scene> m_Scene;
    Wheel::Engine::Systems::RenderSystem* m_RenderSystem = nullptr;
    Wheel::Engine::Systems::InputSystem* m_InputSystem = nullptr;
    std::vector<Wheel::EventSystem::SubscriptionToken> m_SubscriptionTokens;
};