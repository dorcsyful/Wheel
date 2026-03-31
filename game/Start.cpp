//
// Created by dorcs on 2026. 03. 18..
//

#include "Start.h"

void Start::Update()
{
    while (!glfwWindowShouldClose(m_Renderer->GetWindow()))
    {
        //Query input from the renderer
        //TODO: Pass to InputSystem
        glfwPollEvents();

        //TODO: Update ECS here


        m_Renderer->Update();

    }
}