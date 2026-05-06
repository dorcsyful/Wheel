#pragma once
#include "GLFW/glfw3.h"

namespace Wheel::Engine
{
    class Scene;
}

namespace Wheel
{
    namespace Renderer
    {
        enum class DEBUG_MODULES
        {
            COMPONENT_LIST,
            COMPONENT_DETAILS,
            WINDOW_STATS
        };

        class Debugger
        {
            public:
            Debugger(GLFWwindow* window);
            ~Debugger();

            void AddModule(DEBUG_MODULES a_Module);
            void RemoveModule(DEBUG_MODULES a_Module);
            void Draw();
            void GetScene(Engine::Scene* a_Scene) { m_Scene = a_Scene; }

        private:

            void DrawComponentList();
            void DrawComponentDetails();
            void DrawWindowStats();


            Engine::Scene* m_Scene;
            GLFWwindow* m_Window;

            bool m_Modules[3] = {false, false, false};
        };
    }
}

