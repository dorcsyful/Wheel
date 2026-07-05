#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Wheel::Renderer { class Renderer; }

namespace Wheel
{
    namespace Core { class Scene; }
        /**
         * @brief Initializes all built-in components and systems
         */
        class Initializer
        {
        public:
            Initializer() = default;
            ~Initializer() = default;
            /**
             *
             * @brief Creates the scene and renderer
             * @param a_LoadAssets If true, loads in all textures and shaders found in the assets folder but does not create materials
             */
            static void InitEverything(bool a_LoadAssets,Renderer::Renderer*& a_Out_Renderer, Core::Scene*& a_Out_Scene,
                                       uint32_t& a_Out_Camera);
            static Core::Scene* CreateScene(Renderer::Renderer* renderer = nullptr);
            /**
             * @brief Creates a base camera the size of the window and assigns it to the systems that require it
             * @param a_Scene The Scene with all components and systems registered
             * @param renderer The base OpenGL renderer
             * @return the id of the camera entity
             */
            static uint32_t CreateCameraEntity(Core::Scene* a_Scene, Renderer::Renderer* renderer = nullptr);
            /**
             * @brief Initizalizes the basic OpenGL renderer.
             * @param a_Width Window width
             * @param a_Height Window height
             * @param a_Title Window title
             * @param a_Textures Filenames for textures to load. Can be left empty if you wish to load them at a later point
             * @param a_Shaders Shaders to load. Can be left empty if you wish to wait with loading shaders
             * @return
             */
            static Renderer::Renderer* CreateRenderer(int a_Width, int a_Height, const std::string& a_Title,
                                                    const std::vector<std::string>& a_Textures = std::vector<std::string>(),
                                                     const std::vector<std::string>& a_Shaders  = std::vector<std::string>());

            /**
             * @brief Initializes some built-in rendering features
             */
            static void CreateFeatures(Core::Scene* a_Scene,Renderer::Renderer* renderer);
        private:
            static void RegisterComponents(Core::Scene* a_Scene);
            static void RegisterSystems(Core::Scene* a_Scene,Renderer::Renderer* renderer = nullptr);
        };
}
