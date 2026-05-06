#include "Debugger.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "components/Transform2D.h"
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
    ImGui::Begin("My name is window, ImGUI window");
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
        int selectedIndex = 0;
        int i = 0;
        for (auto& [id, transform] : transforms) {
            const bool isSelected = (selectedIndex == i);
            if (ImGui::Selectable(transform->name.c_str(), isSelected)) {
                selectedIndex = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
            ++i;
        }
        ImGui::EndListBox();
    }
}

void Wheel::Engine::Debugger::DrawComponentDetails()
{
}

void Wheel::Engine::Debugger::DrawWindowStats()
{
}
