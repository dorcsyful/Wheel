#ifdef DEBUG_BUILD
#include "Debugger.h"

#include "../events/EventBus.h"
#include "../events/Events.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "common/Transform2D.h"
#include "../rendering/CameraComponent.h"
#include "../collision/Collider2D.h"
#include "core/Scene.h"
#include "core/Globals.h"
#include "helpers/Coordinates.h"
#include "../collision/BoxBoxCollision2D.h"

#include "../physics/Joint2D.h"

void Wheel::Debug::Debugger::Initialize(GLFWwindow* a_Window)
{
    // Initialize ImGUI
    m_Window = a_Window;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    // Scale the UI to the monitor DPI so text stays readable on high-DPI displays
    float xscale = 1.0f, yscale = 1.0f;
    if (GLFWmonitor* monitor = glfwGetPrimaryMonitor())
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);    ImGui::GetStyle().ScaleAllSizes(xscale);
    io.FontGlobalScale = xscale;

    ImGui_ImplGlfw_InitForOpenGL(a_Window, true);
    ImGui_ImplOpenGL3_Init("#version 100");
    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::CollisionEnterEvent>(
        [&](const Events::CollisionEnterEvent& e)
        {
            m_ActiveCollisions.push_back(e.manifold);
        }, m_Tokens.back());
}


void Wheel::Debug::Debugger::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Wheel::Debug::Debugger::AddModule(DEBUG_MODULES a_Module)
{
    m_Modules[static_cast<int>(a_Module)] = true;
}

void Wheel::Debug::Debugger::RemoveModule(DEBUG_MODULES a_Module)
{
    m_Modules[static_cast<int>(a_Module)] = false;
}

void Wheel::Debug::Debugger::Draw()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Wheel Debugger");
    if (m_Modules[static_cast<int>(DEBUG_MODULES::ENTITY_LIST)])
    {
        DrawEntityList();
    }
    ImGui::End();

    ImGui::Begin("Window Stats");
    if (m_Modules[static_cast<int>(DEBUG_MODULES::WINDOW_STATS)])
    {
        DrawWindowStats();
    }
    ImGui::End();

    DrawOverlay();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    m_ActiveCollisions.clear();

}

void Wheel::Debug::Debugger::DrawEntityList()
{
    //Transform components hold the names
    auto transforms = m_Scene->GetComponents<Common::Transform2D>();
    const ImGuiStyle& style = ImGui::GetStyle();

    // Width the list box to the longest entity name so it fits without manual resizing.
    float entityListWidth = 0.0f;
    for (auto& [id, transform] : transforms)
        entityListWidth = std::max(entityListWidth, ImGui::CalcTextSize(transform->name.c_str()).x);
    entityListWidth += style.FramePadding.x * 2.0f + style.ScrollbarSize;

    if (ImGui::BeginListBox("##EntityList", ImVec2(entityListWidth, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
        for (auto& [id, transform] : transforms) {
            const bool isSelected = (m_SelectedEntityIndex >= 0 && m_SelectedEntityId == id);
            if (ImGui::Selectable(transform->name.c_str(), isSelected)) {
                m_SelectedEntityIndex = 0;
                m_SelectedEntityId = id;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }

    if (m_SelectedEntityIndex >= 0) {
        auto componentNames = m_Scene->GetEntityComponentNames(m_SelectedEntityId);
        if (!componentNames.empty()) {
            // Width the list box to the longest component name so it fits without manual resizing.
            float componentListWidth = 0.0f;
            for (const auto& name : componentNames)
                componentListWidth = std::max(componentListWidth, ImGui::CalcTextSize(name.c_str()).x);
            componentListWidth += style.FramePadding.x * 2.0f + style.ScrollbarSize;

            if (ImGui::BeginListBox("##ComponentList", ImVec2(componentListWidth, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
                bool isSelected = false;
                for (const auto& name : componentNames) {
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        m_SelectedEntityDescription = m_Scene->GetComponentDescription(name);
                    }
                }
                ImGui::EndListBox();
            }
        }
    }

    if (!m_SelectedEntityDescription.IsEmpty() && m_SelectedEntityIndex >= 0)
    {
        auto raw = m_Scene->GetComponentRaw(m_SelectedEntityId, m_SelectedEntityDescription);
        const ComponentDescriptor* desc = nullptr;
        auto it = m_Descriptors.find(m_SelectedEntityDescription);
        if (it != m_Descriptors.end())
            desc = it->second;
        if (raw && desc) {
            if (ImGui::CollapsingHeader(desc->name)) {
                bool anyChanged = false;
                for (size_t i = 0; i < desc->fieldCount; ++i)
                    anyChanged |= RenderField(raw, desc->fields[i]);
                if (anyChanged) {
                    if (m_Scene->HasComponent<Common::Transform2D>(m_SelectedEntityId))
                        m_Scene->GetComponent<Common::Transform2D>(m_SelectedEntityId).isDirty = true;
                    if (m_Scene->HasComponent<Collision::BoxCollider2D>(m_SelectedEntityId))
                        m_Scene->GetComponent<Collision::BoxCollider2D>(m_SelectedEntityId).isDirty = true;
                }
            }
        }
    }
}


void Wheel::Debug::Debugger::DrawWindowStats()
{
    ImGui::Text("FPS: %f", m_LastFrameTime);
    ImGui::Text("Current collisions: ");
    std::string temp = m_RunSimulation ? "Running" : "Paused";
    if (ImGui::Button(temp.c_str()))
    {
        m_RunSimulation = !m_RunSimulation;
        EventSystem::EventBus::Publish(Events::RunSimulation(m_RunSimulation));
    }
    if (!m_ActiveCollisions.empty()) {
        if (ImGui::BeginListBox("##CollisionList", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
            bool isSelected = false;
            for (const auto& manifold : m_ActiveCollisions) {
                std::string str = std::to_string(manifold.collider1) + " vs " + std::to_string(manifold.collider2);
                if (ImGui::Selectable(str.c_str(), false)) {
                    ;
                }
            }
            ImGui::EndListBox();
        }
    }
}

void Wheel::Debug::Debugger::DrawOverlay()
{
    if (m_CameraEntity == UINT32_MAX || !m_Scene) return;

    const Common::Transform2D& camT = m_Scene->GetComponent<Common::Transform2D>(m_CameraEntity);
    const Rendering::CameraComponent& camC = m_Scene->GetComponent<Rendering::CameraComponent>(m_CameraEntity);


    ImDrawList* dl = ImGui::GetForegroundDrawList();

    for (auto& [id, collider] : m_Scene->GetComponents<Collision::BoxCollider2D>())
        DrawColliderWireframes(id, dl, camT, camC);

    for (auto& [id, collider] : m_Scene->GetComponents<Collision::CircleCollider2D>())
        DrawColliderWireframes(id, dl, camT, camC);
    for (auto& [id, joint] : m_Scene->GetComponents<Physics::DistanceJoint2D>())
        DrawJoints(id, dl, camT, camC);

    DrawCollisions(dl, camT, camC);
}

void Wheel::Debug::Debugger::DrawCollisions(ImDrawList* dl, const Common::Transform2D& a_CameraTransform,
    const Rendering::CameraComponent& a_CameraComponent)
{
    for (int i = 0; i < m_ActiveCollisions.size(); ++i)
    {
        const auto& col = m_ActiveCollisions[i];
        bool twoContacts = col.contactPoint[0] != col.contactPoint[1];

        ImVec2 p1 = ToScreen(col.contactPoint[0], a_CameraTransform, a_CameraComponent);
        dl->AddCircleFilled(p1, 2.0f, GetColor(DebugColor::Yellow));

        Math::Vector2 middle;
        if (twoContacts)
        {
            ImVec2 p2 = ToScreen(col.contactPoint[1], a_CameraTransform, a_CameraComponent);
            dl->AddCircleFilled(p2, 2.0f, GetColor(DebugColor::Yellow));
            middle = Math::Vector2::Lerp(col.contactPoint[0], col.contactPoint[1], 0.5f);
        }
        else
        {
            middle = col.contactPoint[0];
        }

        Math::Vector2 normalTarget = middle + col.collisionNormal * 0.4f;
        ImVec2 normalStart = ToScreen(middle, a_CameraTransform, a_CameraComponent);
        ImVec2 normalEnd   = ToScreen(normalTarget, a_CameraTransform, a_CameraComponent);
        dl->AddLine(normalStart, normalEnd, GetColor(DebugColor::Yellow));

        ImVec2 arrowDir  = NormalizeImVec2(SubtractImVec2(normalEnd, normalStart));
        ImVec2 arrowPerp = ImVec2(-arrowDir.y, arrowDir.x);
        const float headLen = 8.0f;
        ImVec2 arrowBase  = SubtractImVec2(normalEnd, MultiplyImVec2WithScalar(arrowDir, headLen));
        ImVec2 arrowLeft  = AddImVec2(arrowBase, MultiplyImVec2WithScalar(arrowPerp,  headLen * 0.5f));
        ImVec2 arrowRight = SubtractImVec2(arrowBase, MultiplyImVec2WithScalar(arrowPerp, headLen * 0.5f));
        dl->AddLine(normalEnd, arrowLeft,  GetColor(DebugColor::Yellow));
        dl->AddLine(normalEnd, arrowRight, GetColor(DebugColor::Yellow));
    }
}

void Wheel::Debug::Debugger::DrawColliderWireframes(uint32_t a_EntityId, ImDrawList* dl,
    const Common::Transform2D& a_CameraTransform,
    const Rendering::CameraComponent& a_CameraComponent)
{
    bool box = m_Scene->HasComponent<Collision::BoxCollider2D>(a_EntityId);
    auto& transform = m_Scene->GetComponent<Common::Transform2D>(a_EntityId);
    bool circle = m_Scene->HasComponent<Collision::CircleCollider2D>(a_EntityId);
    if (box)
    {
        auto& collider = m_Scene->GetComponent<Collision::BoxCollider2D>(a_EntityId);
        const Math::Vector2* vertices = Collision::BoxBoxCollision2D::GetVertices(collider, transform);
        for (int i = 0; i < 4; i++)
        {
            ImVec2 p1 = ToScreen(vertices[i], a_CameraTransform, a_CameraComponent);
            ImVec2 p2 = ToScreen(vertices[(i + 1) % 4], a_CameraTransform, a_CameraComponent);
            dl->AddLine(p1, p2, GetColor(DebugColor::Cyan));
        }
    }
    if (circle)
    {
        auto& collider = m_Scene->GetComponent<Collision::CircleCollider2D>(a_EntityId);
        ImVec2 center = ToScreen(transform.GetPosition(), a_CameraTransform, a_CameraComponent);
        // Screen pixels per world unit, from the current view
        float pixelsPerUnit = a_CameraComponent.height / a_CameraComponent.orthoSize;
        float radius = collider.radius * transform.GetScale().x * pixelsPerUnit * a_CameraComponent.zoom;
        dl->AddCircle(center, radius, GetColor(DebugColor::Cyan));
    }

}

void Wheel::Debug::Debugger::DrawJoints(uint32_t a_EntityId, ImDrawList* dl,
    const Common::Transform2D& a_CameraTransform,
    const Rendering::CameraComponent& a_CameraComponent)
{
    Common::Transform2D transform1 = m_Scene->GetComponent<Common::Transform2D>(a_EntityId);
    Physics::DistanceJoint2D joint = m_Scene->GetComponent<Physics::DistanceJoint2D>(a_EntityId);

    const Math::Vector2& s1 = transform1.GetScale();
    Math::Vector2 local1(joint.localAnchorPoint.x * s1.x, joint.localAnchorPoint.y * s1.y);   // S
    Math::Vector2 anchor1 = transform1.GetPosition() + transform1.GetRotationMatrix() * local1;  // T + R·(S·local)
    Math::Vector2 anchor2 = joint.otherAnchorPoint;
    if (joint.connectedRigidbody != NO_VALUE)
    {
        Common::Transform2D transform2 = m_Scene->GetComponent<Common::Transform2D>(joint.connectedRigidbody);
        const Math::Vector2& s2 = transform2.GetScale();
        Math::Vector2 local2(joint.localAnchorPoint.x * s2.x,joint.localAnchorPoint.y * s2.y);   // S
        anchor2 = transform2.GetPosition() + transform2.GetRotationMatrix() * local2;  // T + R·(S·local)

    }
    ImVec2 p1 = ToScreen(anchor1, a_CameraTransform, a_CameraComponent);
    ImVec2 p2 = ToScreen(anchor2, a_CameraTransform, a_CameraComponent);
    dl->AddLine(p1, p2, GetColor(DebugColor::Red));

}

bool Wheel::Debug::Debugger::RenderField(void* componentPtr, const FieldDescriptor& field)
{
    void* fieldPtr = static_cast<char*>(componentPtr) + field.offset;

    switch (field.type) {
    case FieldType::FLOAT:
        return ImGui::DragFloat(field.name, static_cast<float*>(fieldPtr), 0.1f);
    case FieldType::INT:
        return ImGui::DragInt(field.name, static_cast<int*>(fieldPtr));
    case FieldType::BOOL:
        return ImGui::Checkbox(field.name, static_cast<bool*>(fieldPtr));
    case FieldType::STRING: {
            auto* str = static_cast<std::string*>(fieldPtr);
            char buf[256];
            strncpy(buf, str->c_str(), sizeof(buf) - 1);
            buf[sizeof(buf)-1] = '\0';
            if (ImGui::InputText(field.name, buf, sizeof(buf))) { *str = buf; return true; }
            return false;
    }
    case FieldType::VEC2:
        return ImGui::DragFloat2(field.name, static_cast<float*>(fieldPtr), 0.1f);
    case FieldType::VEC3:
        return ImGui::DragFloat3(field.name, static_cast<float*>(fieldPtr), 0.1f);
    case FieldType::VEC4:
        return ImGui::ColorEdit4(field.name, static_cast<float*>(fieldPtr));
    case FieldType::CAMERA_MODE: {
            auto* mode = static_cast<Rendering::CameraMode*>(fieldPtr);
            int current = static_cast<int>(*mode);
            const char* items[] = { "Orthographic", "Perspective" };
            if (ImGui::Combo(field.name, &current, items, 2))
                { *mode = static_cast<Rendering::CameraMode>(current); return true; }
            return false;
    }
    default: return false;
    }
}

ImVec2 Wheel::Debug::Debugger::ToScreen(const Math::Vector2& a_World, const Common::Transform2D& a_CameraTransform, const Rendering::CameraComponent& a_CameraComponent)
{
    Math::Vector2 world = Helpers::Coordinates::WorldToScreenCoordinates(a_World, a_CameraComponent, a_CameraTransform);
    return ImVec2(world.x, world.y);
}

ImColor Wheel::Debug::Debugger::GetColor(DebugColor a_Color)
{
    switch (a_Color) {
    case DebugColor::Red:    return IM_COL32(220, 50,  50,  255);
    case DebugColor::Green:  return IM_COL32(50,  220, 50,  255);
    case DebugColor::Yellow: return IM_COL32(255, 220, 0,   255);
    case DebugColor::Cyan:   return IM_COL32(0,   220, 220, 255);
    default:                 return IM_COL32(255, 255, 255, 255);
    }
}
#endif
