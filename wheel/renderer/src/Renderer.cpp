#include "Renderer.h"

#include <cassert>
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
    m_Shaders.push_back(new Shader("base.vert", "base.frag"));
    unsigned int program = m_Shaders[0]->GetID();
    glUseProgram(program);
    m_PosLoc     = glGetAttribLocation(program, "a_position");
    m_TexLoc     = glGetAttribLocation(program, "a_texCoord");
    m_MvpLoc     = glGetUniformLocation(program, "u_mvpMatrix");
    m_SamplerLoc = glGetUniformLocation(program, "s_texture");
    glUseProgram(0);

    Texture* texture = new Texture("textures/logo.png");
    LoadTexture(texture);
    CreateTestSquare();

#ifdef DEBUG_BUILD
    Engine::Debugger::get().Initialize(m_Window);
     Engine::Debugger::get().AddModule(Engine::DEBUG_MODULES::ENTITY_LIST);
    Engine::Debugger::get().AddModule(Engine::DEBUG_MODULES::WINDOW_STATS);
#endif
}

void Wheel::Renderer::Renderer::Update()
{
    if (!m_RenderedObjects) return;
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_Shaders[0]->GetID());

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO2D);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO2D);
    glBindTexture(GL_TEXTURE_2D, m_Textures[0]->m_ID);

    glUniform1i(m_SamplerLoc, 0);
    glVertexAttribPointer(m_PosLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(m_TexLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(m_PosLoc);
    glEnableVertexAttribArray(m_TexLoc);

    for (int i = 0; i < (int)m_RenderedObjects->size(); i++)
    {
        auto mvp = m_RenderedObjects->at(i).modelMatrix.Transpose();
        glUniformMatrix4fv(m_MvpLoc, 1, GL_FALSE, &mvp.First());
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

    // Don't call glVertexAttribPointer here at all — do it in Update with buffers bound
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Wheel::Renderer::Renderer::LoadTexture(Texture* a_Texture)
{
    assert(std::find(m_Textures.begin(), m_Textures.end(), a_Texture) == m_Textures.end() && "Texture already loaded.");
    m_Textures.push_back(a_Texture);
    a_Texture->LoadTexture();

}


void Wheel::Renderer::Renderer::AddShader(const std::string& a_VertexShader, const std::string& a_FragmentShader)
{
    auto it = std::find_if(m_Shaders.begin(), m_Shaders.end(),
                       [temp = a_VertexShader.substr(0, a_VertexShader.find_last_of('.'))] (Shader* a_Shader) -> bool {
                          return a_Shader->GetName() == temp;
                       });
    m_Shaders.push_back(new Shader(a_VertexShader, a_FragmentShader));
}
