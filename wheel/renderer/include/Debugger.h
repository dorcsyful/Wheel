#pragma once
#include "GLFW/glfw3.h"

namespace Wheel::Engine
{
    class Scene;
}

namespace Wheel
{
    namespace Engine
    {
        enum class DEBUG_MODULES
        {
            ENTITY_LIST,
            COMPONENT_DETAILS,
            WINDOW_STATS
        };

        /**
         * @brief NOTE: Only renders in Debug builds
         */
        class Debugger
        {
        public:
            Debugger(const Debugger& obj) = delete;
            void operator=(const Debugger&)  = delete;
            ~Debugger();
            static Debugger& get() {  static Debugger instance; return instance;}

            void Initialize(GLFWwindow* a_Window);
            void GetScene(Engine::Scene* a_Scene) { m_Scene = a_Scene; }
            void AddModule(DEBUG_MODULES a_Module);
            void RemoveModule(DEBUG_MODULES a_Module);
            void Draw();

        private:
            Debugger() = default;

            void DrawEntityList();
            void DrawComponentDetails();
            void DrawWindowStats();

            static Debugger* s_Instance;
            Engine::Scene* m_Scene;
            GLFWwindow* m_Window;

            bool m_Modules[3] = {false, false, false};
        };
    }
}

