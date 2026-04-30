#pragma once
#include "InputSystem.h"

inline void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
        Wheel::EventSystem::EventBus::Publish(Wheel::Events::KeyPressEvent(key));
    if (action == GLFW_RELEASE)
        Wheel::EventSystem::EventBus::Publish(Wheel::Events::KeyReleaseEvent(key));
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}
void Wheel::Engine::Systems::InputSystem::Initialize(GLFWwindow* window)
{
    m_Window = window;
    glfwSetKeyCallback(window, key_callback);
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::KeyPressEvent>(
        [&](const Events::KeyPressEvent& e) {  }, m_Tokens.back());
}
