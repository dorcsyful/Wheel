#include "Debugger.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "components/Transform2D.h"
#include "components/CameraComponent.h"
#include "core/Scene.h"

void Wheel::Engine::Debugger::Initialize(GLFWwindow* a_Window)
{
    // Initialize ImGUI
    m_Window = a_Window;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(a_Window, true);
    ImGui_ImplOpenGL3_Init("#version 110");
}

Wheel::Engine::Debugger::~Debugger()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Wheel::Engine::Debugger::AddModule(DEBUG_MODULES a_Module)
{
    m_Modules[static_cast<int>(a_Module)] = true;
}

void Wheel::Engine::Debugger::RemoveModule(DEBUG_MODULES a_Module)
{
    m_Modules[static_cast<int>(a_Module)] = false;
}

void Wheel::Engine::Debugger::Draw()
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
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void Wheel::Engine::Debugger::DrawEntityList()
{
    //Transform components hold the names
    auto transforms = m_Scene->GetComponents<Wheel::Components::Transform2D>();
    if (ImGui::BeginListBox("##EntityList", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
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
            if (ImGui::BeginListBox("##ComponentList", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
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
                for (size_t i = 0; i < desc->fieldCount; ++i)
                    RenderField(raw, desc->fields[i]);
            }
        }
    }
}


void Wheel::Engine::Debugger::DrawWindowStats()
{
}

void Wheel::Engine::Debugger::RenderField(void* componentPtr, const FieldDescriptor& field)
{
    // Resolve the pointer to the actual field using the byte offset
    void* fieldPtr = static_cast<char*>(componentPtr) + field.offset;

    switch (field.type) {
    case FieldType::FLOAT:
        ImGui::DragFloat(field.name, static_cast<float*>(fieldPtr), 0.1f);
        break;
    case FieldType::INT:
        ImGui::DragInt(field.name, static_cast<int*>(fieldPtr));
        break;
    case FieldType::BOOL:
        ImGui::Checkbox(field.name, static_cast<bool*>(fieldPtr));
        break;
    case FieldType::STRING: {
            auto* str = static_cast<std::string*>(fieldPtr);
            char buf[256];
            strncpy(buf, str->c_str(), sizeof(buf));
            if (ImGui::InputText(field.name, buf, sizeof(buf)))
                *str = buf;
            break;
    }
    case FieldType::VEC2: {
            ImGui::DragFloat2(field.name, static_cast<float*>(fieldPtr), 0.1f);
            break;
    }
    case FieldType::VEC3: {
            ImGui::DragFloat3(field.name, static_cast<float*>(fieldPtr), 0.1f);
            break;
    }
    case FieldType::VEC4: {
            ImGui::ColorEdit4(field.name, static_cast<float*>(fieldPtr));
            break;
    }
    case FieldType::CAMERA_MODE: {
            auto* mode = static_cast<Wheel::Components::CameraMode*>(fieldPtr);
            int current = static_cast<int>(*mode);
            const char* items[] = { "Orthographic", "Perspective" };
            if (ImGui::Combo(field.name, &current, items, 2))
                *mode = static_cast<Wheel::Components::CameraMode>(current);
            break;
    }
    }
}