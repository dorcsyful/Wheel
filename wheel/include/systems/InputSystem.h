#pragma once
#include <iostream>
#include <ostream>

#include "EventBus.h"
#include "Events.h"
#include "core/System.h"
#include "GLFW/glfw3.h"
#include "math/Vector2.h"

namespace Wheel
{
    namespace Engine
    {
        namespace Systems
        {
            class InputSystem : public System
            {
            public:
                InputSystem(const Engine::Description& a_Description) : Engine::System(a_Description)
                {
                    m_Tokens = std::vector<EventSystem::SubscriptionToken>();
                }
                ~InputSystem() override = default;
                void SubscribeToEvents();
                void SetCallbacks(GLFWwindow* window);
                void Initialize(GLFWwindow* window);
                void SetCameraEntity(uint32_t a_Id) { m_CameraEntity = a_Id; }

                /**
                 * @brief Is the key currently pressed?
                 * @param key GLFW keycode
                 */
                bool IsKeyPressed(int key) { return glfwGetKey(m_Window, key) == GLFW_PRESS || glfwGetKey(m_Window, key) == GLFW_REPEAT; }
                double GetMouseX() { return m_MouseX; }
                double GetMouseY() { return m_MouseY; }
                /**
                 * @brief Completely disables the cursor
                 */
                void DisableCursor() { glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
                /**
                 * @brief Only disable the cursor when it's over the window
                 */
                void DisableCursorInsideWindow() { glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);}
                void EnableCursor() { glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
                Math::Vector2 ScreenToWorldPoint(const Math::Vector2& screenPoint);
                Math::Vector2 WorldToScreenPoint(const Math::Vector2& worldPoint);
                Math::Vector2 MousePositionToWorldPoint();
                //TODO: raw mouse motion and custom cursors
                void Update(float deltaTime) override;

            private:

                GLFWwindow* m_Window = nullptr;
                uint32_t m_CameraEntity = -1;
                std::vector<EventSystem::SubscriptionToken> m_Tokens;

                double m_MouseX = -1, m_MouseY = -1;
            };
        }
    }
}
