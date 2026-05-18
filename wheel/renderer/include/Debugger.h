#pragma once
#include <map>

#include "DebugDescriptor.h"
#include "imgui.h"
#include "SubscriptionToken.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "core/Description.h"
#include "helpers/Collision2DManifold.h"

namespace Wheel::Components
{
    class CameraComponent;
    class Transform2D;
}

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
        enum class DebugColor { Red, Green, Yellow, White, Cyan };

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

            void SetFrameTime(double a_Time) { m_LastFrameTime = a_Time; }
            void SetCameraEntity(uint32_t a_Id) { m_CameraEntity = a_Id; }

        private:
            Debugger() = default;

            //Windowed areas
            void DrawEntityList();
            void DrawWindowStats();
            bool RenderField(void* componentPtr, const FieldDescriptor& field);

            //Overlay
            void DrawOverlay();
            void DrawCollisions(ImDrawList* dl, const Wheel::Components::Transform2D& a_CameraTransform, const Wheel::Components::CameraComponent& a_CameraComponent);
            void DrawColliderWireframes(uint32_t a_EntityId, ImDrawList* dl, const Wheel::Components::Transform2D& a_CameraTransform, const Wheel::Components::CameraComponent& a_CameraComponent);

            //Helpers
            ImVec2 ToScreen(const Math::Vector2& a_World, const Wheel::Components::Transform2D& a_CameraTransform, const Wheel::Components::CameraComponent& a_CameraComponent);
            ImColor GetColor(DebugColor a_Color);
            ImVec2 AddImVec2(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
            ImVec2 SubtractImVec2(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }
            ImVec2 MultiplyImVec2WithScalar(const ImVec2& a, float scalar) { return ImVec2(a.x * scalar, a.y * scalar); }
            float LengthImVec2(const ImVec2& a) { return sqrt(a.x * a.x + a.y * a.y); }
            ImVec2 NormalizeImVec2(const ImVec2& a) { float temp = LengthImVec2(a); return ImVec2(a.x / temp, a.y / temp); }

            bool m_RunSimulation = true;
            uint32_t m_CameraEntity = UINT32_MAX;
            std::vector<Collision::Collision2DManifold> m_ActiveCollisions {};
            std::vector<EventSystem::SubscriptionToken> m_Tokens{};
            double m_LastFrameTime = 0.0;
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

