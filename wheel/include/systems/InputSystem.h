#pragma once
#include <iostream>
#include <ostream>

#include "EventBus.h"
#include "Events.h"
#include "core/System.h"
#include "GLFW/glfw3.h"

namespace Wheel
{
    namespace Engine
    {
        namespace Systems
        {
            //The input system relies on GLFW. Changing the renderer requires rewriting this class
            class InputSystem : public Engine::System
            {
            public:
                InputSystem(const Engine::Description& a_Description) : Engine::System(a_Description)
                {
                    m_Tokens = std::vector<EventSystem::SubscriptionToken>();
                }
                ~InputSystem() override = default;
                void Initialize(GLFWwindow* window);

                /**
                 * @brief Is the key currently pressed?
                 * @param key GLFW keycode
                 */
                bool IsKeyPressed(int key) { return glfwGetKey(m_Window, key) == GLFW_PRESS || glfwGetKey(m_Window, key) == GLFW_REPEAT; }

                void Update(float deltaTime) override {}

            private:

                GLFWwindow* m_Window = nullptr;
                std::vector<EventSystem::SubscriptionToken> m_Tokens;
            };
        }
    }
}
