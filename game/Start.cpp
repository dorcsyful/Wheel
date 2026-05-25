#include "Start.h"
#include "Debugger.h"
#include "systems/Collision2DSystem.h"
#include "components/Collider2D.h"
#include "components/Rigidbody2D.h"
#include "systems/Physics2DSystem.h"
#include "Texture.h"

void Start::RegisterComponents()
{
    m_Scene->RegisterComponentType<Wheel::Components::Transform2D>();
    m_Scene->RegisterComponentType<Wheel::Components::Render2DComponent>();
    m_Scene->RegisterComponentType<Wheel::Components::CameraComponent>();
    m_Scene->RegisterComponentType<Wheel::Components::BoxCollider2D>();
    m_Scene->RegisterComponentType<Wheel::Components::Rigidbody2D>();
}

void Start::RegisterSystems()
{
    Wheel::Engine::Description transform = m_Scene->GetComponentDescription<Wheel::Components::Transform2D>();
    Wheel::Engine::Description render = m_Scene->GetComponentDescription<Wheel::Components::Render2DComponent>();
    Wheel::Engine::Description finalDesc = Wheel::Engine::Description();
    Wheel::Engine::Description cameraDesc = m_Scene->GetComponentDescription<Wheel::Components::CameraComponent>();
    Wheel::Engine::Description boxCollider2D = m_Scene->GetComponentDescription<Wheel::Components::BoxCollider2D>();
    Wheel::Engine::Description rigidbody2D = m_Scene->GetComponentDescription<Wheel::Components::Rigidbody2D>();
    finalDesc.AddComponentType(transform.GetAsBitset());
    finalDesc.AddComponentType(render.GetAsBitset());
    m_RenderSystem = m_Scene->RegisterSystem<Wheel::Engine::Systems::RenderSystem>(finalDesc);
    m_RenderSystem->GetRenderer(m_Renderer.get());
    finalDesc = Wheel::Engine::Description();
    finalDesc.AddComponentType(transform.GetAsBitset());
    finalDesc.AddComponentType(cameraDesc.GetAsBitset());
    m_InputSystem = m_Scene->RegisterSystem<Wheel::Engine::Systems::InputSystem>(finalDesc);
    m_InputSystem->Initialize(m_Renderer->GetWindow());
    finalDesc.Reset();
    finalDesc.AddComponentType(transform.GetAsBitset());
    finalDesc.AddComponentType(boxCollider2D.GetAsBitset());
    m_Scene->RegisterSystem<Wheel::Engine::Systems::Collision2DSystem>(finalDesc);
    finalDesc.Reset();
    finalDesc.AddComponentType(transform.GetAsBitset());
    finalDesc.AddComponentType(boxCollider2D.GetAsBitset());
    finalDesc.AddComponentType(rigidbody2D.GetAsBitset());
    m_Scene->RegisterSystem<Wheel::Engine::Systems::Physics2DSystem>(finalDesc);

}

void Start::CreateEntities()
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
    Wheel::Components::Transform2D cameraTransform =m_Scene->GetComponent<Wheel::Components::Transform2D>(cameraId);
    cameraTransform.name = "Camera";
    m_RenderSystem->SetCameraEntity(cameraId);
    m_InputSystem->SetCameraEntity(cameraId);
#ifdef DEBUG_BUILD
    Wheel::Engine::Debugger::get().SetCameraEntity(cameraId);
#endif
    m_CameraId = cameraId;

    {
        uint32_t ground = m_Scene->AddEntity();
        auto& transform = m_Scene->AddComponent<Wheel::Components::Transform2D>(ground);
        auto& render    = m_Scene->AddComponent<Wheel::Components::Render2DComponent>(ground);
        auto& collider  = m_Scene->AddComponent<Wheel::Components::BoxCollider2D>(ground);
        auto& rigidbody = m_Scene->AddComponent<Wheel::Components::Rigidbody2D>(ground);
        rigidbody.SetMass(100.0f); rigidbody.friction = 0.5f; rigidbody.SetType(Wheel::Components::Rigidbody2DType::STATIC);
        collider.SetWidth(13.f); collider.SetHeight(2.f);
        transform.SetPosition(0.0f, -3.0f);
        transform.SetScale(1.0f, 1.0f);
        transform.SetRotation(0);
        render.width = 13.f; render.height = 2.f; render.TextureName = "textures/square.png"; render.color = Wheel::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    {
        uint32_t id = m_Scene->AddEntity();
        auto& transform = m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
        auto& render    = m_Scene->AddComponent<Wheel::Components::Render2DComponent>(id);
        auto& collider  = m_Scene->AddComponent<Wheel::Components::BoxCollider2D>(id);
        auto& rigidbody = m_Scene->AddComponent<Wheel::Components::Rigidbody2D>(id);
        rigidbody.SetMass(10.0f); rigidbody.friction = 0.5f;
        float w = 0.9;
        collider.SetWidth(w);
        float h = 0.5f;
        collider.SetHeight(h);
        float x = 0;
        float y =2;
        transform.SetPosition(x, y);
        transform.SetScale(1.0f, 1.0f);
        transform.SetRotation(0);
        render.width = w; render.height = h; render.TextureName = "textures/square.png"; render.color = Wheel::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    {
        uint32_t id = m_Scene->AddEntity();
        auto& transform = m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
        auto& render    = m_Scene->AddComponent<Wheel::Components::Render2DComponent>(id);
        auto& collider  = m_Scene->AddComponent<Wheel::Components::BoxCollider2D>(id);
        auto& rigidbody = m_Scene->AddComponent<Wheel::Components::Rigidbody2D>(id);
        rigidbody.SetMass(1.0f); rigidbody.friction = 0.5f;
        float w = 1.1f;
        collider.SetWidth(w);
        float h = 0.8f;
        collider.SetHeight(h);
        transform.SetPosition(-0.35f, 12.0f);
        transform.SetScale(1.0f, 1.0f);
        transform.SetRotation(0);
        render.width = w; render.height = h; render.TextureName = "textures/square.png"; render.color = Wheel::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

}

void Start::Init()
{
    m_RunSimulation = true;
    m_Renderer = std::make_unique<Wheel::Renderer::Renderer>();
    m_Renderer->Init(1280, 720, "Wheel Engine");
    m_Scene = std::make_unique<Wheel::Engine::Scene>();
    m_SubscriptionTokens = std::vector<Wheel::EventSystem::SubscriptionToken>();
    RegisterComponents();
    RegisterSystems();
    CreateEntities();
    m_SubscriptionTokens.emplace_back();

    m_Renderer->LoadTexture(new Wheel::Renderer::Texture("textures/square.png"));
    m_Renderer->LoadTexture(new Wheel::Renderer::Texture("textures/logo.png"));


#ifdef DEBUG_BUILD
    Wheel::EventSystem::EventBus::Subscribe<Wheel::Events::RunSimulation>(
        [&](const Wheel::Events::RunSimulation& e) { m_RunSimulation = e.enable; }, m_SubscriptionTokens.back());
#endif
}


void Start::Update()
{
    
    constexpr float kFixedDt = 1.0f / 60.0f;   // physics steps per simulated second (decoupled from render fps)
    double lastTime = glfwGetTime();
    float accumulator = 0.0f;

    while (!glfwWindowShouldClose(m_Renderer->GetWindow()))
    {
        double now = glfwGetTime();
        float frameTime = static_cast<float>(now - lastTime);
        lastTime = now;
        if (frameTime > 0.25f) frameTime = 0.25f;   // clamp to avoid the spiral of death after a hitch

        glfwPollEvents();

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
            }
        }

        float alpha = m_RunSimulation ? (accumulator / kFixedDt) : 1.0f;
        m_RenderSystem->Render(alpha);
        m_Renderer->Update();

#ifdef DEBUG_BUILD
        Wheel::Engine::Debugger::get().SetFrameTime(frameTime);
#endif
    }

}
