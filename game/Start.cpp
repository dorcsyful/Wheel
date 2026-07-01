#include "Start.h"
#include "Debugger.h"
#include "systems/Collision2DSystem.h"
#include "components/Collider2D.h"
#include "components/Rigidbody2D.h"
#include "components/Highlight2D.h"
#include "systems/Physics2DSystem.h"
#include "Texture.h"
#include <cmath>

void Start::RegisterComponents()
{
    m_Scene->RegisterComponentType<Wheel::Components::Transform2D>();
    m_Scene->RegisterComponentType<Wheel::Components::Sprite>();
    m_Scene->RegisterComponentType<Wheel::Components::CameraComponent>();
    m_Scene->RegisterComponentType<Wheel::Components::BoxCollider2D>();
    m_Scene->RegisterComponentType<Wheel::Components::CircleCollider2D>();
    m_Scene->RegisterComponentType<Wheel::Components::Rigidbody2D>();
    m_Scene->RegisterComponentType<Wheel::Components::DistanceJoint2D>();
    m_Scene->RegisterComponentType<Wheel::Components::Highlight2D>();
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
    auto id = m_Renderer->GetShaderId("highlight");
    // Highlight: same sprite grown by thickness (world units = px / PIXELS_PER_UNIT),
    // tinted, one layer behind its owner.
    m_RenderSystem->RegisterFeature<Wheel::Components::Highlight2D>(-1,
        [id](const Wheel::Renderer::RenderedObject& a_Base,
             const Wheel::Components::Highlight2D& a_Highlight,
             std::vector<Wheel::Renderer::RenderedObject>& a_Out)
        {
            float t  = static_cast<float>(a_Highlight.thickness) / PIXELS_PER_UNIT;
            float sx = std::sqrt(a_Base.linear[0] * a_Base.linear[0] + a_Base.linear[1] * a_Base.linear[1]);
            float sy = std::sqrt(a_Base.linear[2] * a_Base.linear[2] + a_Base.linear[3] * a_Base.linear[3]);
            if (sx <= 0.0f || sy <= 0.0f) return;

            Wheel::Renderer::RenderedObject ro = a_Base;
            float fx = (sx + 2.0f * t) / sx;
            float fy = (sy + 2.0f * t) / sy;
            ro.linear[0] *= fx; ro.linear[1] *= fx;
            ro.linear[2] *= fy; ro.linear[3] *= fy;
            ro.color = a_Highlight.color;
            ro.shaderId = id;
            // dilation params in the enlarged quad's UV space
            ro.SetUniform("u_uvScale",     Wheel::Math::Vector2(fx, fy));
            ro.SetUniform("u_thicknessUV", Wheel::Math::Vector2(t / (sx + 2.0f * t), t / (sy + 2.0f * t)));
            a_Out.push_back(ro);
        });
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

uint32_t Start::SpawnObject(float x, float y, float w, float h)
{
    uint32_t id = m_Scene->AddEntity();
    auto& transform = m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
    auto& render    = m_Scene->AddComponent<Wheel::Components::Sprite>(id);
    auto& collider  = m_Scene->AddComponent<Wheel::Components::CircleCollider2D>(id);
    auto& rigidbody = m_Scene->AddComponent<Wheel::Components::Rigidbody2D>(id);
    rigidbody.SetMass(2.0f); rigidbody.friction = 0.05f;
    collider.radius = w / 2.0f;
    rigidbody.restitution = 0.97f;
    transform.SetPosition(x, y);
    transform.SetScale(1.0f, 1.0f);
    rigidbody.linearDamping = 0.0001; rigidbody.angularDamping = 0.0001;
    transform.SetRotationInDegrees(0);
    // The ellipse shader fills the sprite quad, so a square sprite (side = 2*radius)
    // draws a circle that matches the collider. Give width != height for an ellipse.
    render.width = w; render.height = w; render.color = Wheel::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    render.MaterialName = m_CircleMaterial;
    auto& highlight = m_Scene->AddComponent<Wheel::Components::Highlight2D>(id);
    highlight.thickness = 4;
    highlight.color = Wheel::Math::Vector4(1.0f, 0.85f, 0.1f, 1.0f);
    return id;
}

void Start::InitializeCameraEntity()
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
    Wheel::Components::Transform2D& cameraTransform =m_Scene->GetComponent<Wheel::Components::Transform2D>(cameraId);
    cameraTransform.name = "Camera";
    m_RenderSystem->SetCameraEntity(cameraId);
    m_InputSystem->SetCameraEntity(cameraId);
#ifdef DEBUG_BUILD
    Wheel::Engine::Debugger::get().SetCameraEntity(cameraId);
#endif
    m_CameraId = cameraId;
}

void Start::CreateGroundEntity()
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

void Start::CreateNewtonsCradle()
{
    float width = 0.5f;
    float gap   = 0.002f;
    float step  = width + gap;   // center spacing: the gap MUST be in the step, or neighbours stay exactly touching
    float x0    = -1.0f;         // ball 0

    for (int i = 0; i < 4; i++)
    {
        float x = x0 + i * step;
        uint32_t entityId = SpawnObject(x, 2.f, width, width);
        Wheel::Components::DistanceJoint2D& joint = m_Scene->AddComponent<Wheel::Components::DistanceJoint2D>(entityId);
        joint.distance = 1.f;
        joint.otherAnchorPoint = Wheel::Math::Vector2(x, 3.f);   // pivot directly above
    }
    {
        // Striker: one step left of ball 0, pulled back to horizontal and taut (distance == rest length).
        float pivotX   = x0 - step;
        float strikerX = pivotX - 1.0f;
        uint32_t entityId = SpawnObject(strikerX, 3.f, width, width);
        Wheel::Components::DistanceJoint2D& joint = m_Scene->AddComponent<Wheel::Components::DistanceJoint2D>(entityId);
        joint.distance = 1.f;
        joint.otherAnchorPoint = Wheel::Math::Vector2(pivotX, 3.f);
    }
}

void Start::CreateEntities()
{
    InitializeCameraEntity();
    CreateGroundEntity();

    CreateNewtonsCradle();
    Wheel::Renderer::Material* mat = m_Renderer->CreateMaterial("base","textures/key.png");
    uint32_t characterEntity = m_Scene->AddEntity();
    Wheel::Components::Transform2D& transform = m_Scene->AddComponent<Wheel::Components::Transform2D>(characterEntity);
    Wheel::Components::Sprite& sprite = m_Scene->AddComponent<Wheel::Components::Sprite>(characterEntity);
    Wheel::Components::Highlight2D& hl = m_Scene->AddComponent<Wheel::Components::Highlight2D>(characterEntity);
    hl.active = true; hl.color = Wheel::Math::Vector4(1.0f, 0.0f, 0.0f, 1.0f); hl.thickness = 10;
    sprite.MaterialName = mat->id;
    sprite.width = 1; sprite.height = 1;
    transform.SetPosition(0.0f, 0.0f);
}

void Start::Init()
{
    m_RunSimulation = true;
    m_Renderer = std::make_unique<Wheel::Renderer::Renderer>();
    m_Renderer->Init(1280, 720, "Wheel Engine");
    m_Renderer->LoadTexture(new Wheel::Renderer::Texture("textures/square.png"));
    m_Renderer->LoadTexture(new Wheel::Renderer::Texture("textures/logo.png"));
    m_Renderer->LoadTexture(new Wheel::Renderer::Texture("textures/key.png"));
    m_BoxMaterial = m_Renderer->CreateMaterial("base", "textures/square.png")->id;
    m_CircleMaterial = m_Renderer->CreateMaterial("circle", "textures/square.png")->id;

    m_Scene = std::make_unique<Wheel::Engine::Scene>();
    m_SubscriptionTokens = std::vector<Wheel::EventSystem::SubscriptionToken>();
    RegisterComponents();
    RegisterSystems();
    

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
