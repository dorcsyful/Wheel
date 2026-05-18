#pragma once
#include <glad.h>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <string>
#include "RenderedObject.h"
#include "Shader.h"
#include "GLFW/glfw3.h"

namespace Wheel
{
    namespace Renderer
    {
        class Debugger;
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
             * @brief This gets called when the scene is initialized. You do not have to manually call it.
             */
            void Init(int a_Width, int a_Height, const char* a_Title);

            /**
             * @brief Only handles the DRAW phase. Does not update game state or polls input
             */
            void Update();

            /**
             * @brief You have direct access to the GLFW window. Don't make me regret it
             */
            GLFWwindow* GetWindow() { return m_Window; }

            /**
             * @brief Load a texture at startup. Returns the integer ID used for fast lookup at draw time.
             */
            uint32_t LoadTexture(Texture* a_Texture);

            /**
             * @brief Load a shader pair at startup. Returns the integer ID used for fast lookup at draw time.
             */
            uint32_t AddShader(const std::string& a_VertexShader, const std::string& a_FragmentShader);

            /**
             * @brief Resolve a texture name to its integer ID. Called once per entity per frame in RenderSystem — never in the draw loop.
             */
            uint32_t GetTextureId(const std::string& a_Name) const
            {
                auto it = m_TextureIndex.find(a_Name);
                assert(it != m_TextureIndex.end() && "Texture not loaded. Call LoadTexture() before use.");
                return it->second;
            }

            /**
             * @brief Resolve a shader name to its integer ID. Called once per entity per frame in RenderSystem — never in the draw loop.
             */
            uint32_t GetShaderId(const std::string& a_Name) const
            {
                auto it = m_ShaderIndex.find(a_Name);
                assert(it != m_ShaderIndex.end() && "Shader not loaded. Call AddShader() before use.");
                return it->second;
            }

            [[nodiscard]] std::vector<std::string> GetShaders() const
            {
                std::vector<std::string> names;
                names.reserve(m_Shaders.size());
                for (auto& shader : m_Shaders)
                    names.push_back(shader->GetName());
                return names;
            }

            void GetRenderedObjects(std::vector<RenderedObject>* a_ROs) { m_RenderedObjects = a_ROs; }

        private:
            void CreateTestSquare();

            GLFWwindow* m_Window{};

            // Indexed by integer ID — O(1) access in the draw loop
            std::vector<Shader*>  m_Shaders;
            std::vector<Texture*> m_Textures;

            // Name → ID maps — only touched at load time, never inside Update()
            std::unordered_map<std::string, uint32_t> m_ShaderIndex;
            std::unordered_map<std::string, uint32_t> m_TextureIndex;

            std::vector<RenderedObject>* m_RenderedObjects = nullptr;

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