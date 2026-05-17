#include "Start.h"
#include "Debugger.h"
#include "systems/Collision2DSystem.h"
#include "components/Collider2D.h"
#include "components/Rigidbody2D.h"
#include "systems/Physics2DSystem.h"

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
        rigidbody.SetMass(100.0f); rigidbody.friction = 0.5f; rigidbody.restitution = 0.5f; rigidbody.SetType(Wheel::Components::Rigidbody2DType::STATIC);
        collider.SetWidth(13.f); collider.SetHeight(2.f);
        transform.SetPosition(0.0f, -3.0f);
        transform.SetScale(1.0f, 1.0f);
        transform.SetRotation(0);
        render.width = 13.f; render.height = 2.f;
    }

    {
        uint32_t id = m_Scene->AddEntity();
        auto& transform = m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
        auto& render    = m_Scene->AddComponent<Wheel::Components::Render2DComponent>(id);
        auto& collider  = m_Scene->AddComponent<Wheel::Components::BoxCollider2D>(id);
        auto& rigidbody = m_Scene->AddComponent<Wheel::Components::Rigidbody2D>(id);
        rigidbody.SetMass(10.0f); rigidbody.friction = 0.5f; rigidbody.restitution = 0.5f;
        float w = 0.9;
        collider.SetWidth(w);
        float h = 0.5f;
        collider.SetHeight(h);
        float x = 0;
        float y =2;
        transform.SetPosition(x, y);
        transform.SetScale(1.0f, 1.0f);
        transform.SetRotation(0);
        render.width = w; render.height = h;
    }

    {
        uint32_t id = m_Scene->AddEntity();
        auto& transform = m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
        auto& render    = m_Scene->AddComponent<Wheel::Components::Render2DComponent>(id);
        auto& collider  = m_Scene->AddComponent<Wheel::Components::BoxCollider2D>(id);
        auto& rigidbody = m_Scene->AddComponent<Wheel::Components::Rigidbody2D>(id);
        rigidbody.SetMass(1.0f); rigidbody.friction = 0.5f; rigidbody.restitution = 0.5f;
        float w = 1.1f;
        collider.SetWidth(w);
        float h = 0.8f;
        collider.SetHeight(h);
        transform.SetPosition(-0.2, 12.0f);
        transform.SetScale(1.0f, 1.0f);
        transform.SetRotation(0);
        render.width = w; render.height = h;
    }

}

void Start::Init()
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

void Start::SpawnAtMousePosition(Wheel::Math::Vector2 mousePosition)
{
    uint32_t id = m_Scene->AddEntity();
    Wheel::Components::Transform2D& transform =
        m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
    Wheel::Components::Render2DComponent& render =
        m_Scene->AddComponent<Wheel::Components::Render2DComponent>(id);
    Wheel::Engine::Systems::InputSystem* inputSystem = m_Scene->GetSystem<Wheel::Engine::Systems::InputSystem>();
    Wheel::Math::Vector2 worldPosition = inputSystem->ScreenToWorldPoint(mousePosition);
    transform.SetPosition(worldPosition);
    transform.SetScale(1.0f, 1.0f);
    transform.SetRotation((float)random(-180, 180));
    render.width = 0.5f; render.height = 0.3f;

}

void Start::Update()
{
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(m_Renderer->GetWindow()))
    {
        double now = glfwGetTime();
        float deltaTime = static_cast<float>(now - lastTime);
        lastTime = now;

        glfwPollEvents();
        m_Scene->Update(deltaTime);
        m_Renderer->Update();

#ifdef DEBUG_BUILD
        Wheel::Engine::Debugger::get().SetFrameTime(deltaTime);
#endif
    }

}
