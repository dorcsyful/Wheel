#include "Start.h"
#include "Debugger.h"
#include "systems/Collision2DSystem.h"
#include "components/Collider2D.h"

void Start::RegisterComponents()
{
    m_Scene->RegisterComponentType<Wheel::Components::Transform2D>();
    m_Scene->RegisterComponentType<Wheel::Components::Render2DComponent>();
    m_Scene->RegisterComponentType<Wheel::Components::CameraComponent>();
    m_Scene->RegisterComponentType<Wheel::Components::BoxCollider2D>();
}

void Start::RegisterSystems()
{
    Wheel::Engine::Description transform = m_Scene->GetComponentDescription<Wheel::Components::Transform2D>();
    Wheel::Engine::Description render = m_Scene->GetComponentDescription<Wheel::Components::Render2DComponent>();
    Wheel::Engine::Description finalDesc = Wheel::Engine::Description();
    Wheel::Engine::Description cameraDesc = m_Scene->GetComponentDescription<Wheel::Components::CameraComponent>();
    Wheel::Engine::Description boxCollider2D = m_Scene->GetComponentDescription<Wheel::Components::BoxCollider2D>();
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
    for (int i = 0; i < 2; i++)
    {
        uint32_t id = m_Scene->AddEntity();
        Wheel::Components::Transform2D& transform =
            m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
        Wheel::Components::Render2DComponent& render =
            m_Scene->AddComponent<Wheel::Components::Render2DComponent>(id);
        Wheel::Components::BoxCollider2D& collider =
            m_Scene->AddComponent<Wheel::Components::BoxCollider2D>(id);
        collider.SetWidth(0.5f); collider.SetHeight(0.3f);
        transform.SetPosition((float)random(-3, 3), (float)random(-3, 3));
        transform.SetScale(1.0f, 1.0f);
        transform.SetRotation((float)random(-180, 180));
        render.width = 0.5f; render.height = 0.3f;
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
    clock_t beginFrame = clock();
    clock_t endFrame = clock();
    double deltaTime = 0;

#if DEBUG_BUILD
    Wheel::Engine::Debugger::get().SetFrameTime(deltaTime);
#endif

    while (!glfwWindowShouldClose(m_Renderer->GetWindow()))
    {
        beginFrame = clock();
        //Query input from the renderer
        //TODO: Pass to InputSystem
        glfwPollEvents();
        //TODO: Update ECS here
        m_Scene->Update(deltaTime);

        m_Renderer->Update();
        endFrame = clock();
        deltaTime += endFrame - beginFrame;
    }
}
