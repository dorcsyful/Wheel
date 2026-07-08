#include "DemoSelector.h"
#include "imgui.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "debug/Debugger.h"

Wheel::Game::DemoSelector::DemoSelector(GLFWwindow* a_Window, Core::Scene* a_Scene) : m_SelectedDemoIndex(0)
{
    m_Window = a_Window;
    m_Scene = a_Scene;
    //In Debug mode the renderer intializes it but not in release
    if (!ImGui::GetCurrentContext())
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::StyleColorsDark();

        // Scale the UI to the monitor DPI so text stays readable on high-DPI displays
        float xscale = 1.0f, yscale = 1.0f;
        if (GLFWmonitor* monitor = glfwGetPrimaryMonitor())
            glfwGetMonitorContentScale(monitor, &xscale, &yscale);
        ImGui::GetStyle().ScaleAllSizes(xscale);
        io.FontGlobalScale = xscale;

        ImGui_ImplGlfw_InitForOpenGL(a_Window, true);
        ImGui_ImplOpenGL3_Init("#version 100");
    }
}

void Wheel::Game::DemoSelector::Update(float a_DeltaTime)
{
#ifdef DEBUG_BUILD
#else
     ImGui_ImplOpenGL3_NewFrame();
     ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

#endif
    int maxWidth = 0;
    for (auto& [name, type] : m_DemoList)
    {
        int width = ImGui::CalcTextSize(name.c_str()).x;
        maxWidth = std::max(maxWidth, width);
    }

    ImGui::Begin("Demo Selector");
    if (ImGui::BeginListBox("##DemoList", ImVec2(maxWidth + 20, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
        for (auto& [id, demo] : m_DemoList) {
            const bool isSelected = (m_SelectedDemoIndex >= 0);
            if (ImGui::Selectable(id.c_str(), isSelected)) {
                m_SelectedDemoIndex = 0;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }


    #ifdef DEBUG_BUILD
    for (const auto& inspectable : GetDemoInspectables())
    {
        const TypeDescriptor* desc = inspectable.descriptor;
        if (!desc || !inspectable.instance) continue;
        if (ImGui::CollapsingHeader(desc->name))
        {
            for (size_t i = 0; i < desc->fieldCount; ++i)
                Debug::Debugger::RenderField(inspectable.instance, desc->fields[i]);
        }
    }
#endif

    ImGui::End();
}
