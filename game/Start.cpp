#include "Start.h"
#include "Debugger.h"
#include "systems/Collision2DSystem.h"
#include "components/Collider2D.h"
#include "components/Rigidbody2D.h"
#include "components/Highlight2D.h"
#include "systems/Physics2DSystem.h"
#include "Texture.h"
#include <cmath>

#include "../wheel/Initializer.h"

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
    CreateGroundEntity();

    CreateNewtonsCradle();
    Wheel::Renderer::Material* mat = m_Renderer->CreateMaterial("base","key.png");
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
    m_SubscriptionTokens = std::vector<Wheel::EventSystem::SubscriptionToken>();

    Wheel::Engine::Initializer::InitEverything(true, m_Renderer,m_Scene,m_CameraId);

    m_BoxMaterial = m_Renderer->CreateMaterial("base", "square.png")->id;
    m_CircleMaterial = m_Renderer->CreateMaterial("circle", "square.png")->id;
    m_RenderSystem = m_Scene->GetSystem<Wheel::Engine::Systems::RenderSystem>();
    m_InputSystem = m_Scene->GetSystem<Wheel::Engine::Systems::InputSystem>();

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
