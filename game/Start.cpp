#include "Start.h"

void Start::Update()
{
    while (!glfwWindowShouldClose(m_Renderer->GetWindow()))
    {
        //Query input from the renderer
        //TODO: Pass to InputSystem
        glfwPollEvents();
        Wheel::Components::Transform2D& comp = m_Scene->GetComponent<Wheel::Components::Transform2D>(m_CameraId);
        comp.rotation += 0.01 * 0.1;
        //TODO: Update ECS here
        m_Scene->Update(0.1);

        m_Renderer->Update();

    }
}