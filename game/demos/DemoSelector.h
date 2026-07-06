#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>

#include "Demo.h"
#include "core/Scene.h"
#include "GLFW/glfw3.h"

namespace Wheel
{
    namespace Game
    {

        class DemoSelector
        {
        public:
            DemoSelector(GLFWwindow* a_Window, Core::Scene* a_Scene);
            ~DemoSelector() = default;
            void Update(float a_DeltaTime);
            void AddDemo(std::string a_Name, std::unique_ptr<Demo> a_Demo)
            {
                std::unique_ptr<Demo> p_Demo = std::move(a_Demo);
                std::pair<std::string, std::unique_ptr<Demo>> p_DemoPair =
                                        std::make_pair(a_Name, std::move(p_Demo));
                m_DemoList.insert(std::move(p_DemoPair));
            }

            void ExecuteDemo(const std::string& a_Name)
            {
                assert(m_DemoList.find(a_Name) != m_DemoList.end() && "Demo not found");
                m_DemoList.at(a_Name)->Initialize(m_Scene);
            }

        private:
            int m_SelectedDemoIndex;
            Core::Scene* m_Scene;
            std::map<std::string, std::unique_ptr<Demo>> m_DemoList{};
            GLFWwindow* m_Window;
        };
    }
}
