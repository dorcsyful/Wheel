#include "Renderer.h"

#include <cassert>
#include <iostream>

#include "Shader.h"
#include "Texture.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
static void glfwError(int id, const char* description)
{
    std::cout << description << std::endl;
}

Wheel::Renderer::Renderer::~Renderer()
{
    for (auto element : m_Shaders)
    {
        glDeleteProgram(element.second->GetID());

        delete element.second;
    }
    for (auto element : m_Textures)
    {
        delete element;
    }
    glfwTerminate();
}

void Wheel::Renderer::Renderer::Init(int a_Width, int a_Height, const char* a_Title)
{  glfwSetErrorCallback(&glfwError);

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    m_Window = glfwCreateWindow(a_Width, a_Height, a_Title, NULL, NULL);
    if (m_Window == NULL)
    {
        std::cout << "Failed to initialize window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return;
    }
    glViewport(0, 0, a_Width, a_Height);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    m_Shaders.insert(std::make_pair("default", new Shader("base.vert", "base.frag")));
    Texture* texture = new Texture("textures/logo.png");
    LoadTexture(texture);
    CreateTestSquare();
}

void Wheel::Renderer::Renderer::Update()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned int program = m_Shaders.at("default")->GetID();
    glUseProgram(program);
    // Load the vertex position
    glVertexAttribPointer ( glGetAttribLocation ( program, "a_position" ), 3, GL_FLOAT,
                            GL_FALSE, 5 * sizeof(GLfloat), vertices2D );
    // Load the texture coordinate
    glVertexAttribPointer ( glGetAttribLocation ( program, "a_texCoord" ), 2, GL_FLOAT,
                            GL_FALSE, 5 * sizeof(GLfloat), &vertices2D[3] );

    glEnableVertexAttribArray ( glGetAttribLocation ( program, "a_position" ) );
    glEnableVertexAttribArray ( glGetAttribLocation ( program, "a_texCoord" ) );

    // Bind the texture
    glActiveTexture ( GL_TEXTURE0 );
    glBindTexture ( GL_TEXTURE_2D, m_Textures[0]->m_ID );

    // Set the sampler texture unit to 0
    glUniform1i ( glGetUniformLocation ( program, "s_texture" ), 0 );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices2D );

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glfwSwapBuffers(m_Window);
}

void Wheel::Renderer::Renderer::CreateTestSquare()
{
    //glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO2D);
    glGenBuffers(1, &m_EBO2D);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    //glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO2D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2D), vertices2D, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO2D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2D), indices2D, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Wheel::Renderer::Renderer::LoadTexture(Texture* a_Texture)
{
    assert(std::find(m_Textures.begin(), m_Textures.end(), a_Texture) == m_Textures.end() && "Texture already loaded.");
    m_Textures.push_back(a_Texture);
    a_Texture->LoadTexture();

}

void Wheel::Renderer::Renderer::AddShader(const char* a_Name, const char* a_VertexShader, const char* a_FragmentShader)
{
    assert(m_Shaders.find(a_Name) == m_Shaders.end() && "Shader already exists.");
    m_Shaders.insert(std::make_pair(a_Name, new Shader(a_VertexShader, a_FragmentShader)));
}
