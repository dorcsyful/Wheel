#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>

#include "Demo.h"
#include "core/Scene.h"
#include "GLFW/glfw3.h"

//Checking if the demos have any inspectable members
#ifdef DEBUG_BUILD
#include <type_traits>
template<typename, typename = void>
struct HasDescriptor : std::false_type {};
template<typename T>
struct HasDescriptor<T, std::void_t<decltype(T::descriptor())>> : std::true_type {};
#endif

namespace Wheel
{
    namespace Game
    {
        class DemoSelector
        {
        public:
            DemoSelector(GLFWwindow* a_Window, Core::Scene* a_Scene);
            ~DemoSelector() = default;

            struct DemoInspectable { void* instance; const TypeDescriptor* descriptor; };

            void Update(float a_DeltaTime);
            template<typename T>
            void AddDemo(std::string a_Name, std::unique_ptr<T> a_Demo)
            {
#ifdef DEBUG_BUILD
                if constexpr (HasDescriptor<T>::value)
                    m_DemoDescriptors[a_Name] = &T::descriptor();   //before it's downgraded to Demo*
#endif
                m_DemoList.insert(std::make_pair(std::move(a_Name),
                                                 std::unique_ptr<Demo>(std::move(a_Demo))));
            }

            void ExecuteDemo(const std::string& a_Name)
            {
                assert(m_DemoList.find(a_Name) != m_DemoList.end() && "Demo not found");
#ifdef DEBUG_BUILD
                ClearDemoInspectables();
                auto it = m_DemoDescriptors.find(a_Name);
                if (it != m_DemoDescriptors.end())
                    m_DemoInspectables.push_back({ m_DemoList.at(a_Name).get(),
                        it->second });
#endif

                m_DemoList.at(a_Name)->Initialize(m_Scene);
            }
            const std::vector<DemoInspectable>& GetDemoInspectables() const { return m_DemoInspectables; }
            void ClearDemoInspectables() { m_DemoInspectables.clear(); }

        private:

#ifdef DEBUG_BUILD
            std::vector<DemoInspectable>              m_DemoInspectables{};
            std::map<std::string, const TypeDescriptor*> m_DemoDescriptors{};
#endif
            int m_SelectedDemoIndex;
            Core::Scene* m_Scene;
            std::map<std::string, std::unique_ptr<Demo>> m_DemoList{};
            GLFWwindow* m_Window;
        };
    }
}
