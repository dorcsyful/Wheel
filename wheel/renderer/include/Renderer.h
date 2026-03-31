#pragma once
#include <glad.h>

#include "GLFW/glfw3.h"

namespace Wheel
{
    namespace Renderer
    {
        class Renderer
        {
        public:
            Renderer() = default;
            ~Renderer() { glfwTerminate(); }

            void Init(int a_Width, int a_Height, const char* a_Title);
            void Update();
            GLFWwindow* GetWindow() { return m_Window; }

        private:
            GLFWwindow* m_Window;
        };
    }
}

