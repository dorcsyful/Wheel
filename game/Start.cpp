#include "Start.h"

void Start::SpawnAtMousePosition(Wheel::Math::Vector2 mousePosition)
{
    uint32_t id = m_Scene->AddEntity();
    Wheel::Components::Transform2D& transform =
        m_Scene->AddComponent<Wheel::Components::Transform2D>(id);
    Wheel::Components::Render2DComponent& render =
        m_Scene->AddComponent<Wheel::Components::Render2DComponent>(id);
    Wheel::Engine::Systems::InputSystem* inputSystem = m_Scene->GetSystem<Wheel::Engine::Systems::InputSystem>();
    std::cout << mousePosition.x << " " << mousePosition.y << std::endl;
    Wheel::Math::Vector2 worldPosition = inputSystem->ScreenToWorldPoint(mousePosition);
    transform.position.x = worldPosition.x;
    transform.position.y = worldPosition.y;
    transform.scale.x = 1.0f;
    transform.scale.y = 1.0f;
    transform.rotation = (float)random(-180, 180);
    render.width = 0.5f; render.height = 0.3f;
}

void Start::Update()
{
    while (!glfwWindowShouldClose(m_Renderer->GetWindow()))
    {
        //Query input from the renderer
        //TODO: Pass to InputSystem
        glfwPollEvents();
        Wheel::Components::Transform2D& comp = m_Scene->GetComponent<Wheel::Components::Transform2D>(m_CameraId);
        //comp.rotation += 0.01 * 0.1;
        //TODO: Update ECS here
        m_Scene->Update(0.1);

        m_Renderer->Update();

    }
}
