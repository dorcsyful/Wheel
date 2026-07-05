//
//This is the starting file. Register any custom systems you'd like to use here
#pragma once
#include <iostream>
#include <memory>

#include <core/Scene.h>

#include "renderer/include/Renderer.h"
#include "../wheel/rendering/RenderSystem.h"
#include "core/Description.h"
#include "../wheel/rendering/CameraComponent.h"
#include "../wheel/input/InputSystem.h"


//This is the class that communicates with the engine. Override any part of this to fit your needs
class Start
{
public:
    Start()
    = default;
    ~Start()
    {delete m_Scene; delete m_Renderer;}

    void CreateNewtonsCradle();
    void CreateEntities();
    void Init();


    void Update();
    Wheel::Core::Scene* GetScene() { return m_Scene; }

private:
    bool m_RunSimulation;
    uint32_t m_CameraId;

    uint32_t m_BoxMaterial    = 0;
    uint32_t m_CircleMaterial = 0;
    Wheel::Renderer::Renderer* m_Renderer;
    Wheel::Core::Scene* m_Scene;
    Wheel::Rendering::RenderSystem* m_RenderSystem = nullptr;
    Wheel::Input::InputSystem* m_InputSystem = nullptr;
    std::vector<Wheel::EventSystem::SubscriptionToken> m_SubscriptionTokens;
};