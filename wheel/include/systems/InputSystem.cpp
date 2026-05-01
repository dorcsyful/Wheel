#pragma once
#include "InputSystem.h"

#include <cmath>
#include "components/CameraComponent.h"
#include "components/Transform2D.h"
#include "core/Globals.h"
#include "core/Scene.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
        Wheel::EventSystem::EventBus::Publish(Wheel::Events::KeyPressEvent(key));
    if (action == GLFW_RELEASE)
        Wheel::EventSystem::EventBus::Publish(Wheel::Events::KeyReleaseEvent(key));
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
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
    Wheel::EventSystem::EventBus::Publish(Wheel::Events::MouseMoveEvent(xpos,ypos));
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
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

    // Screen pixels (0,0 = top-left, Y down) → NDC [-1,1] (Y up)
    float ndc_x =  2.0f * screenPoint.x / camComponent.width  - 1.0f;
    float ndc_y = -2.0f * screenPoint.y / camComponent.height + 1.0f;

    // Invert projection: NDC → view space (world units in camera frame)
    float half_w = camComponent.width  / (2.0f * PIXELS_PER_UNIT * camComponent.zoom);
    float half_h = camComponent.height / (2.0f * PIXELS_PER_UNIT * camComponent.zoom);
    float view_x = ndc_x * half_w;
    float view_y = ndc_y * half_h;

    // Invert view matrix: un-rotate by camera rotation, then translate by camera position
    float cos_cr = std::cos(camTransform.rotation);
    float sin_cr = std::sin(camTransform.rotation);
    return {
        camTransform.position.x + cos_cr * view_x - sin_cr * view_y,
        camTransform.position.y + sin_cr * view_x + cos_cr * view_y
    };
}

Wheel::Math::Vector2 Wheel::Engine::Systems::InputSystem::WorldToScreenPoint(const Math::Vector2& worldPoint)
{
    return Math::Vector2();
}

Wheel::Math::Vector2 Wheel::Engine::Systems::InputSystem::MousePositionToWorldPoint()
{
    return Math::Vector2();
}

void Wheel::Engine::Systems::InputSystem::Update(float deltaTime)
{
    glfwGetCursorPos(m_Window, &m_MouseX, &m_MouseY);
}
