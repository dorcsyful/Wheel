#include "renderer/include/Shader.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include "helpers/AssetPath.h"

Wheel::Renderer::Shader::Shader(const std::string& a_VertexPath, const std::string& a_FragmentPath)
{
    Build(Wheel::GetShaderPathFS() / a_VertexPath, Wheel::GetShaderPathFS() / a_FragmentPath);
}

Wheel::Renderer::Shader::Shader(std::filesystem::path a_VertexPath, std::filesystem::path a_FragmentPath)
{
    Build(a_VertexPath, a_FragmentPath);
}

void Wheel::Renderer::Shader::Build(const std::filesystem::path& a_VertexPath,
                                    const std::filesystem::path& a_FragmentPath)
{
    m_Name = a_VertexPath.stem().string();

    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    std::cout << "Vertex path: " << a_VertexPath.string().c_str() << std::endl;
    try
    {
        // open files (ifstream accepts std::filesystem::path directly since C++17)
        vShaderFile.open(a_VertexPath);
        fShaderFile.open(a_FragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    //Compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertex);
    glAttachShader(m_ID, fragment);
    glLinkProgram(m_ID);
    checkCompileErrors(m_ID, "PROGRAM");
    CacheLocations();
    //We don't need no shaders no more
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Wheel::Renderer::Shader::CacheLocations()
{
    int count = 0;
    // --- attributes ---
    glGetProgramiv(m_ID, GL_ACTIVE_ATTRIBUTES, &count);
    for (GLint i = 0; i < count; ++i) {
        char    name[128];
        GLint   size;      // array length (1 if not an array)
        GLenum  type;      // GL_FLOAT_VEC2, GL_FLOAT_VEC3, ...
        glGetActiveAttrib(m_ID, i, sizeof(name), nullptr, &size, &type, name);
        GLint loc = glGetAttribLocation(m_ID, name);
        VarInfo temp(name, loc, type);
        m_Attributes.push_back(temp);
    }

    // --- uniforms ---
    glGetProgramiv(m_ID, GL_ACTIVE_UNIFORMS, &count);
    for (GLint i = 0; i < count; ++i) {
        char    name[128];
        GLint   size;
        GLenum  type;      // GL_FLOAT_VEC4, GL_FLOAT_MAT2, GL_SAMPLER_2D, ...
        glGetActiveUniform(m_ID, i, sizeof(name), nullptr, &size, &type, name);
        GLint loc = glGetUniformLocation(m_ID, name);
        VarInfo temp(name, loc, type);
        m_PerObject.push_back(temp);

    }

}

void Wheel::Renderer::Shader::AddPerBind(const std::string& a_Name)
{
    bool success = false;
    for (auto it = m_PerBind.begin(); it != m_PerBind.end(); ++it)
    {
        if (it->name == a_Name)
        {
            return;
        }
    }
    for (auto it = m_PerObject.begin(); it != m_PerObject.end(); ++it)
    {
        if (it->name == a_Name)
        {
            m_PerBind.push_back(*it);
            m_PerObject.erase(it);
            return;
        }
    }
    std::string val = "Uniform not found in shader: ";
    assert(!success && (val + a_Name).c_str());
}

void Wheel::Renderer::Shader::AddPerObject(const std::string& a_Name)
{
    bool success = false;
    for (auto it = m_PerObject.begin(); it != m_PerObject.end(); ++it)
    {
        if (it->name == a_Name)
        {
            return;
        }
    }
    for (auto it = m_PerBind.begin(); it != m_PerBind.end(); ++it)
    {
        if (it->name == a_Name)
        {
            m_PerObject.push_back(*it);
            m_PerBind.erase(it);
            return;
        }
    }
    std::string val = "Uniform not found in shader: ";
    assert(!success && (val + a_Name).c_str());
}

void Wheel::Renderer::Shader::checkCompileErrors(unsigned int shader, const std::string& type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}


