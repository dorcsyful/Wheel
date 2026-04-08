#pragma once
#include <glad.h>
#include <map>
#include <vector>
#include <string>
#include "RenderedObject.h"
#include "Shader.h"
#include "GLFW/glfw3.h"

namespace Wheel
{
    namespace Renderer
    {
        class Texture;

        /**
         * @brief You have direct access to the basic OpenGLES renderer. Use this to load resources like textures and meshes.
         */
        class Renderer
        {
        public:
            Renderer() = default;
            ~Renderer();

            /**
             *
             * @brief This gets called when the scene is initialized. You do not have to manually call it.
             */
            void Init(int a_Width, int a_Height, const char* a_Title);
            /**
             * @brief Only handles the DRAW phase. Does not update game state or polls input
             */
            void Update();

            /**
             * @brief All shaders in assets/shaders get loaded on startup. You can refer to them by their filename. Call this function if you're not sure your shader has been loaded
             */
            [[nodiscard]] std::vector<std::string> GetShaders() const
            {
                std::vector<std::string> shaders(m_Shaders.size());
                for (auto& shader : m_Shaders)
                {
                    shaders.push_back(shader->GetName());
                }
                return shaders;
            }

            void AddToRenderQueue(RenderedObject a_RenderedObject);
            /**
             * @brief You have direct access to the GLFW window. Don't make me regret it
             */
            GLFWwindow* GetWindow() { return m_Window; }

            /**
             * @brief Load all textures at startup to avoid lag mid-game
             */
            void LoadTexture(Texture* a_Texture);

            std::vector<RenderedObject>& GetRenderedObjects() { return m_RenderedObjects; }

        private:
            void AddShader(const std::string& a_VertexShader, const std::string& a_FragmentShader);
            void CreateTestSquare();

            GLFWwindow* m_Window{};
            std::vector<Shader*> m_Shaders;
            std::vector<Texture*> m_Textures;
            //Using a vector instead of regular array to have access to sort() & co.
            std::vector<RenderedObject> m_RenderedObjects;



            //Flat squares for rendering 2D
            unsigned int m_EBO2D{};
            unsigned int m_VBO2D{};
            GLfloat vertices2D[20] = { -0.5f,  0.5f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
            unsigned short indices2D[6] = { 0, 1, 2, 0, 2, 3 };
        };
    }
}

