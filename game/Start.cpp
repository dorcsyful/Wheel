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
    m_Scene->RegisterComponentType<Wheel::Components::Sprite>();
    m_Scene->RegisterComponentType<Wheel::Components::CameraComponent>();
    m_Scene->RegisterComponentType<Wheel::Components::BoxCollider2D>();
    m_Scene->RegisterComponentType<Wheel::Components::CircleCollider2D>();
    m_Scene->RegisterComponentType<Wheel::Components::Rigidbody2D>();
}

void Start::RegisterSystems()
{
    Wheel::Engine::Description transform = m_Scene->GetComponentDescription<Wheel::Components::Transform2D>();
    Wheel::Engine::Description render = m_Scene->GetComponentDescription<Wheel::Components::Sprite>();
    Wheel::Engine::Description finalDesc = Wheel::Engine::Description();
    Wheel::Engine::Description cameraDesc = m_Scene->GetComponentDescription<Wheel::Components::CameraComponent>();
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
    m_Scene->RegisterSystem<Wheel::Engine::Systems::Collision2DSystem>(finalDesc);
    finalDesc.Reset();
    finalDesc.AddComponentType(transform.GetAsBitset());
    finalDesc.AddComponentType(rigidbody2D.GetAsBitset());
    m_Scene->RegisterSystem<Wheel::Engine::Systems::Physics2DSystem>(finalDesc);

}

void Start::SpawnObject(float x, float y, float w, float h)
{
    uint32_t id = m_Scene->AddEntity();
    auto& transform = m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
    auto& render    = m_Scene->AddComponent<Wheel::Components::Sprite>(id);
    auto& collider  = m_Scene->AddComponent<Wheel::Components::CircleCollider2D>(id);
    auto& rigidbody = m_Scene->AddComponent<Wheel::Components::Rigidbody2D>(id);
    rigidbody.SetMass(10.0f); rigidbody.friction = 0.5f;
    collider.radius = w / 2.0f;
    transform.SetPosition(x, y);
    transform.SetScale(1.0f, 1.0f);
    transform.SetRotationInDegrees(0);
    render.width = w; render.height = h; render.color = Wheel::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    render.MaterialName = m_CircleMaterial;
}

void Start::CreateEntities()
{
    // Camera entity — has Transform2D + CameraComponent but no Sprite
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
        auto& render    = m_Scene->AddComponent<Wheel::Components::Sprite>(ground);
        auto& collider  = m_Scene->AddComponent<Wheel::Components::BoxCollider2D>(ground);
        auto& rigidbody = m_Scene->AddComponent<Wheel::Components::Rigidbody2D>(ground);
        rigidbody.SetMass(100.0f); rigidbody.friction = 0.5f; rigidbody.SetType(Wheel::Components::Rigidbody2DType::STATIC);
        collider.SetWidth(13.f); collider.SetHeight(2.f);
        transform.SetPosition(0.0f, -3.0f);
        transform.SetScale(1.0f, 1.0f);
        transform.SetRotationInDegrees(0);
        render.width = 13.f; render.height = 2.f; render.color = Wheel::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        render.MaterialName = m_BoxMaterial;
    }

    float w = 0.9;
    float h = 0.5f;
    float y = 1;
    float x = 0;
    for (int i = 0; i < 4; i++)
    {
        SpawnObject(x,y,w,h);
        y += 1.0f; // > radius sum (0.9) so they don't interpenetrate at spawn
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
    
    m_Renderer->LoadTexture(new Wheel::Renderer::Texture("textures/square.png"));
    m_Renderer->LoadTexture(new Wheel::Renderer::Texture("textures/logo.png"));

    m_BoxMaterial = m_Renderer->CreateMaterial("base", "textures/square.png")->id;

    // The circle shader sizes its SDF from u_size (in pixels). Set it on the
    // material so every sprite using this material draws a proper circle — a
    // material property feeding a shader uniform, no engine plumbing required.
    Wheel::Renderer::Material* circle = m_Renderer->CreateMaterial("circle", "textures/square.png");
    circle->Set("u_size", Wheel::Math::Vector2(0.9f * PIXELS_PER_UNIT, 0.5f * PIXELS_PER_UNIT));
    m_CircleMaterial = circle->id;

    CreateEntities();
    m_SubscriptionTokens.emplace_back();


#ifdef DEBUG_BUILD
    Wheel::EventSystem::EventBus::Subscribe<Wheel::Events::RunSimulation>(
        [&](const Wheel::Events::RunSimulation& e) { m_RunSimulation = e.enable; }, m_SubscriptionTokens.back());
#endif
    Wheel::EventSystem::EventBus::Subscribe<Wheel::Events::LeftMouseButtonPressEvent>(
    [&](const Wheel::Events::LeftMouseButtonPressEvent& e) { Wheel::Math::Vector2 world = m_Scene->GetSystem<Wheel::Engine::Systems::RenderSystem>()->ScreenToWorld(e.x,e.y); SpawnObject(world.x,world.y); }, m_SubscriptionTokens.back());

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
