#include "Renderer.h"

#include <iostream>

#include "Shader.h"
#include "Texture.h"
#include "RenderedObject.h"
#include "EventBus.h"
#include "Events.h"
#include "Debugger.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Wheel::EventSystem::EventBus::Publish(Wheel::Events::WindowResizeEvent(width, height));
}
static void glfwError(int id, const char* description)
{
    std::cout << description << std::endl;
}

Wheel::Renderer::Renderer::~Renderer()
{
    for (auto element : m_Shaders)
    {
        glDeleteProgram(element->GetID());
        delete element;
    }
    for (auto element : m_Textures)
    {
        delete element;
    }
    glfwTerminate();
}

void Wheel::Renderer::Renderer::Init(int a_Width, int a_Height, const char* a_Title)
{
    glfwSetErrorCallback(&glfwError);

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    m_Window = glfwCreateWindow(a_Width, a_Height, a_Title, nullptr, nullptr);
    if (m_Window == nullptr)
    {
        std::cout << "Failed to initialize window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_Window);
    //glfwSwapInterval(0); // disable vsync — let the loop run at full speed

    if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return;
    }
    glViewport(0, 0, a_Width, a_Height);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

    // Base shader — Shader constructor calls CacheLocations() internally
    AddShader("base.vert", "base.frag");

    CreateTestSquare();

#ifdef DEBUG_BUILD
    Engine::Debugger::get().Initialize(m_Window);
    Engine::Debugger::get().AddModule(Engine::DEBUG_MODULES::ENTITY_LIST);
    Engine::Debugger::get().AddModule(Engine::DEBUG_MODULES::WINDOW_STATS);
#endif
}

void Wheel::Renderer::Renderer::Update()
{
    if (!m_RenderedObjects || m_RenderedObjects->empty()) return;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO2D);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO2D);

    uint32_t activeShaderId = UINT32_MAX;
    uint32_t activeTexId    = UINT32_MAX;
    Shader*  activeShader   = nullptr;

    for (const auto& ro : *m_RenderedObjects)
    {
        if (ro.shaderId != activeShaderId)
        {
            activeShader   = m_Shaders[ro.shaderId];
            activeShaderId = ro.shaderId;
            glUseProgram(activeShader->GetID());
            glVertexAttribPointer(activeShader->GetPosLoc(), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
            glVertexAttribPointer(activeShader->GetTexLoc(), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(activeShader->GetPosLoc());
            glEnableVertexAttribArray(activeShader->GetTexLoc());
            glUniform1i(activeShader->GetSamplerLoc(), 0);
            // Shared by every object drawn with this program — upload once
            // per bind, not once per object.
            glUniformMatrix4fv(activeShader->GetViewProjLoc(), 1, GL_FALSE, &m_ViewProjGL.First());
        }

        if (ro.textureId != activeTexId)
        {
            glBindTexture(GL_TEXTURE_2D, m_Textures[ro.textureId]->m_ID);
            activeTexId = ro.textureId;
        }

        glUniform4f(activeShader->GetColorLoc(), ro.color.x, ro.color.y, ro.color.z, ro.color.w);
        glUniformMatrix2fv(activeShader->GetModelLoc(), 1, GL_FALSE, ro.linear);
        glUniform2f(activeShader->GetTranslateLoc(), ro.translate[0], ro.translate[1]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
    }

#ifdef DEBUG_BUILD
    Engine::Debugger::get().Draw();
#endif

    glfwSwapBuffers(m_Window);
}

void Wheel::Renderer::Renderer::CreateTestSquare()
{
    glGenBuffers(1, &m_VBO2D);
    glGenBuffers(1, &m_EBO2D);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO2D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2D), vertices2D, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO2D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2D), indices2D, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

uint32_t Wheel::Renderer::Renderer::LoadTexture(Texture* a_Texture)
{
    assert(m_TextureIndex.find(a_Texture->GetName()) == m_TextureIndex.end() && "Texture already loaded.");
    uint32_t id = static_cast<uint32_t>(m_Textures.size());
    m_Textures.push_back(a_Texture);
    m_TextureIndex[a_Texture->GetName()] = id;
    a_Texture->LoadTexture();
    return id;
}

uint32_t Wheel::Renderer::Renderer::AddShader(const std::string& a_VertexShader, const std::string& a_FragmentShader)
{
    Shader* shader = new Shader(a_VertexShader, a_FragmentShader);
    auto it = m_ShaderIndex.find(shader->GetName());
    if (it != m_ShaderIndex.end())
    {
        delete shader;
        return it->second;
    }
    uint32_t id = static_cast<uint32_t>(m_Shaders.size());
    m_Shaders.push_back(shader);
    m_ShaderIndex[shader->GetName()] = id;
    return id;
}