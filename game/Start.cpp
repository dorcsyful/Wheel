#include "Start.h"
#include <iostream>
#include "debugger/Debugger.h"
#include "_gfx/include/Texture.h"

#include "../wheel/Initializer.h"
#include "demos/scenes/CollisionPhases.h"
#include "demos/scenes/FallingColliders.h"
#include "demos/scenes/MouseEvents.h"
#include "demos/scenes/NewtonsCradle.h"

void Start::Init()
{
    m_RunSimulation = true;
    m_SubscriptionTokens = std::vector<Wheel::EventSystem::SubscriptionToken>();

    Wheel::Initializer::InitEverything(true, m_Renderer,m_Scene,m_CameraId);

    m_BoxMaterial = m_Renderer->CreateMaterial("base", "square.png")->id;
    m_CircleMaterial = m_Renderer->CreateMaterial("circle", "square.png")->id;
    m_RenderSystem = m_Scene->GetSystem<Wheel::Rendering::RenderSystem>();
    m_InputSystem = m_Scene->GetSystem<Wheel::Input::InputSystem>();

    m_SubscriptionTokens.emplace_back();

#ifdef DEBUG_BUILD
    Wheel::EventSystem::EventBus::Subscribe<Wheel::Events::RunSimulation>(
        [&](const Wheel::Events::RunSimulation& e) { m_RunSimulation = e.enable; }, m_SubscriptionTokens.back());
#endif
   // Wheel::EventSystem::EventBus::Subscribe<Wheel::Events::LeftMouseButtonPressEvent>(
   // [&](const Wheel::Events::LeftMouseButtonPressEvent& e) { Wheel::Math::Vector2 world = m_Scene->GetSystem<Wheel::Rendering::RenderSystem>()->ScreenToWorld(e.x,e.y); SpawnObject(world.x,world.y); }, m_SubscriptionTokens.back());
    CreateDemos();
}

void Start::CreateDemos()
{
    m_DemoSelector = std::make_unique<Wheel::Game::DemoSelector>(m_Renderer->GetWindow(),m_Scene,m_Renderer);
    std::unique_ptr<Wheel::Game::CollisionPhases> cp = std::make_unique<Wheel::Game::CollisionPhases>();
    std::unique_ptr<Wheel::Game::FallingColliders> fc = std::make_unique<Wheel::Game::FallingColliders>();
    std::unique_ptr<Wheel::Game::MouseEvents> me = std::make_unique<Wheel::Game::MouseEvents>();
    std::unique_ptr<Wheel::Game::NewtonsCradle> nc = std::make_unique<Wheel::Game::NewtonsCradle>();
    nc->GetCircleMaterial(m_CircleMaterial);
    fc->GetMaterials(m_BoxMaterial, m_CircleMaterial);
    m_DemoSelector->AddDemo("Collision Phases", std::move(cp));
    m_DemoSelector->AddDemo("Falling Colliders", std::move(fc));
    m_DemoSelector->AddDemo("Mouse Events", std::move(me));
    m_DemoSelector->AddDemo("Newton's Cradle", std::move(nc));


    m_DemoSelector->ExecuteDemo("Falling Colliders");
}


void Start::Update()
{
    
    constexpr float kFixedDt = 1.0f / 60.0f;   // physics steps per simulated second (decoupled from render fps)
    double lastTime = glfwGetTime();
    float accumulator = 0.0f;

    // --- temporary frame profiler: accumulate per-phase time, print every 60 frames ---
    double accPoll = 0, accPhysics = 0, accRenderCPU = 0, accDemo = 0, accRendererUpdate = 0;
    int profFrames = 0;

    while (!glfwWindowShouldClose(m_Renderer->GetWindow()))
    {
        double now = glfwGetTime();
        float frameTime = static_cast<float>(now - lastTime);
        lastTime = now;
        if (frameTime > 0.25f) frameTime = 0.25f;   // clamp to avoid the spiral of death after a hitch

        double t0 = glfwGetTime();
        glfwPollEvents();
        double t1 = glfwGetTime();
        int physicsSteps = 0;
        if (m_RunSimulation)
        {
            accumulator += frameTime;
            while (accumulator >= kFixedDt)
            {
                // Only the snapshot taken right before the FINAL step is used
                // for this frame's interpolation, so skip redundant ones while
                // catching up after a hitch.
                if (accumulator - kFixedDt < kFixedDt)
                    m_RenderSystem->SavePreviousTransforms();
                m_Scene->Update(kFixedDt);                  // every physics step sees an identical dt
                accumulator -= kFixedDt;
                ++physicsSteps;
            }
        }
        double t2 = glfwGetTime();

        float alpha = m_RunSimulation ? (accumulator / kFixedDt) : 1.0f;
        m_RenderSystem->Render(alpha);
        double t3 = glfwGetTime();
#ifdef DEBUG_BUILD
        Wheel::Debug::Debugger::get().PrepareFrame();
#endif
        m_DemoSelector->Update(frameTime);
        double t4 = glfwGetTime();

        m_Renderer->Update();
        double t5 = glfwGetTime();

        accPoll           += t1 - t0;
        accPhysics        += t2 - t1;
        accRenderCPU      += t3 - t2;
        accDemo           += t4 - t3;
        accRendererUpdate += t5 - t4;
        if (++profFrames >= 60)
        {
            std::cout << "[frame ms/60] poll=" << accPoll / 60 * 1000
                      << " physics=" << accPhysics / 60 * 1000
                      << " renderCPU=" << accRenderCPU / 60 * 1000
                      << " demo=" << accDemo / 60 * 1000
                      << " rendererUpdate(GPU+swap+imgui)=" << accRendererUpdate / 60 * 1000
                      << " | lastPhysicsSteps=" << physicsSteps << std::endl;
            accPoll = accPhysics = accRenderCPU = accDemo = accRendererUpdate = 0;
            profFrames = 0;
        }

#ifdef DEBUG_BUILD
        Wheel::Debug::Debugger::get().SetFrameTime(frameTime);
#endif
    }

}
