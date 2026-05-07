#pragma once
#include <map>

#include "DebugDescriptor.h"
#include "GLFW/glfw3.h"
#include "core/Description.h"
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

            template<typename T>
            void RegisterComponentDescriptor(const Description& desc) {
                m_Descriptors.insert({desc, &T::descriptor()});
            }

        private:
            Debugger() = default;

            void DrawEntityList();
            void DrawWindowStats();
            void RenderField(void* componentPtr, const FieldDescriptor& field);
            std::map<Description, const ComponentDescriptor*> m_Descriptors{};

            static Debugger* s_Instance;
            Engine::Scene* m_Scene;
            GLFWwindow* m_Window;
            std::map<Description, std::string> m_ComponentNames {};
            bool m_Modules[3] = {false, false, false};
            int m_SelectedEntityIndex = -1;
            uint32_t m_SelectedEntityId = 0;
            Description m_SelectedEntityDescription{};
        };
    }
}

