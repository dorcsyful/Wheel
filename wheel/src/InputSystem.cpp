
#include "../include/systems/InputSystem.h"

#include <cmath>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "components/CameraComponent.h"
#include "components/Transform2D.h"
#include "core/Globals.h"
#include "core/Scene.h"
#include "helpers/Coordinates.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
        Wheel::EventSystem::EventBus::Publish(Wheel::Events::KeyPressEvent(key));
    if (action == GLFW_RELEASE)
        Wheel::EventSystem::EventBus::Publish(Wheel::Events::KeyReleaseEvent(key));
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
#ifdef DEBUG_BUILD
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;
#endif

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
            Wheel::EventSystem::EventBus::Publish(Wheel::Events::LeftMouseButtonPressEvent(xpos,ypos));
        if (action == GLFW_RELEASE)
            Wheel::EventSystem::EventBus::Publish(Wheel::Events::LeftMouseButtonReleaseEvent(xpos,ypos));
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
            Wheel::EventSystem::EventBus::Publish(Wheel::Events::RightMouseButtonPressEvent(xpos,ypos));
        if (action == GLFW_RELEASE)
            Wheel::EventSystem::EventBus::Publish(Wheel::Events::RightMouseButtonReleaseEvent(xpos,ypos));
    }
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
#ifdef DEBUG_BUILD
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
#endif
    Wheel::EventSystem::EventBus::Publish(Wheel::Events::MouseMoveEvent(xpos,ypos));
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
#ifdef DEBUG_BUILD
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    if (ImGui::GetIO().WantCaptureMouse) return;
#endif
    Wheel::EventSystem::EventBus::Publish(Wheel::Events::MouseScrollEvent(xoffset,yoffset));
}
void cursor_enter_callback(GLFWwindow* window, int entered)
{
    if (entered)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Wheel::EventSystem::EventBus::Publish(Wheel::Events::CursorEnterEvent(xpos,ypos));
    }
    else
    {
        Wheel::EventSystem::EventBus::Publish(Wheel::Events::CursorLeaveEvent());
    }
}

void Wheel::Engine::Systems::InputSystem::SubscribeToEvents()
{
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::KeyPressEvent>(
        [&](const Events::KeyPressEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::KeyReleaseEvent>(
        [&](const Events::KeyReleaseEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::LeftMouseButtonPressEvent>(
        [&](const Events::LeftMouseButtonPressEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::LeftMouseButtonReleaseEvent>(
        [&](const Events::LeftMouseButtonReleaseEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::RightMouseButtonPressEvent>(
        [&](const Events::RightMouseButtonPressEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::RightMouseButtonReleaseEvent>(
        [&](const Events::RightMouseButtonReleaseEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::MouseMoveEvent>(
        [&](const Events::MouseMoveEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::MouseScrollEvent>(
        [&](const Events::MouseScrollEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::CursorEnterEvent>(
        [&](const Events::CursorEnterEvent& e) {  }, m_Tokens.back());
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::CursorLeaveEvent>(
        [&](const Events::CursorLeaveEvent& e) {  }, m_Tokens.back());


}

void Wheel::Engine::Systems::InputSystem::SetCallbacks(GLFWwindow* window)
{
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);
}

void Wheel::Engine::Systems::InputSystem::Initialize(GLFWwindow* window)
{
    m_Window = window;
    SetCallbacks(window);
    SubscribeToEvents();
}

Wheel::Math::Vector2 Wheel::Engine::Systems::InputSystem::ScreenToWorldPoint(const Math::Vector2& screenPoint)
{
    const Components::Transform2D& camTransform = m_Scene->GetComponent<Components::Transform2D>(m_CameraEntity);
    const Components::CameraComponent& camComponent = m_Scene->GetComponent<Components::CameraComponent>(m_CameraEntity);
    return Helpers::Coordinates::ScreenToWorldCoordinates(screenPoint, camComponent, camTransform);
}

Wheel::Math::Vector2 Wheel::Engine::Systems::InputSystem::WorldToScreenPoint(const Math::Vector2& worldPoint)
{
    const Components::Transform2D& camTransform = m_Scene->GetComponent<Components::Transform2D>(m_CameraEntity);
    const Components::CameraComponent& camComponent = m_Scene->GetComponent<Components::CameraComponent>(m_CameraEntity);

    return Helpers::Coordinates::WorldToScreenCoordinates(worldPoint, camComponent, camTransform);
}

Wheel::Math::Vector2 Wheel::Engine::Systems::InputSystem::MousePositionToWorldPoint()
{
    return ScreenToWorldPoint(Math::Vector2(m_MouseX, m_MouseY));
}

void Wheel::Engine::Systems::InputSystem::Update(float deltaTime)
{
    glfwGetCursorPos(m_Window, &m_MouseX, &m_MouseY);
}
