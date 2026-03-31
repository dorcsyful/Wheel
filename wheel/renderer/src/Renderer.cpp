#pragma once
#include "Renderer.h"

#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}static void glfwError(int id, const char* description)
{
    std::cout << description << std::endl;
}
void Wheel::Renderer::Renderer::Init(int a_Width, int a_Height, const char* a_Title)
{  glfwSetErrorCallback(&glfwError);

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    m_Window = glfwCreateWindow(a_Width, a_Height, a_Title, NULL, NULL);
    if (m_Window == NULL)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

}

void Wheel::Renderer::Renderer::Update()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}
