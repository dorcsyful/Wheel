#include "Renderer.h"

#include <iostream>

#include "Shader.h"
#include "Texture.h"
#include "TypeInfo.h"
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
#ifdef DEBUG_BUILD
    Engine::Debugger::get().Shutdown();
#endif
    for (auto element : m_Shaders)
    {
        glDeleteProgram(element->GetID());
        delete element;
    }
    for (auto element : m_Textures)
    {
        delete element;
    }
    for (auto element : m_Materials)
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
    AddShader("circle.vert","circle.frag");
    AddShader("highlight.vert","highlight.frag");
    CreateTestSquare();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef DEBUG_BUILD
    Engine::Debugger::get().Initialize(m_Window);
    Engine::Debugger::get().AddModule(Engine::DEBUG_MODULES::ENTITY_LIST);
    Engine::Debugger::get().AddModule(Engine::DEBUG_MODULES::WINDOW_STATS);
#endif
}

namespace
{
    constexpr GLsizei kVertexStride = 5 * sizeof(GLfloat);
    struct AttribLayout { const char* name; GLint size; const void* offset; };
    const AttribLayout kVertexLayout[] = {
        { "a_position", 3, reinterpret_cast<const void*>(0) },
        { "a_texCoord", 2, reinterpret_cast<const void*>(3 * sizeof(GLfloat)) },
    };

    // Engine-owned per-object uniforms. Their values come from the RenderedObject
    // every frame, so the material never supplies them.
    bool IsEngineUniform(const std::string& a_Name)
    {
        return a_Name == "u_model" || a_Name == "u_translate" || a_Name == "u_color";
    }

    void BindAttributes(Wheel::Renderer::Shader* a_Shader)
    {
        for (const Wheel::Renderer::VarInfo& attr : a_Shader->GetAttributes())
            for (const AttribLayout& l : kVertexLayout)
                if (attr.name == l.name)
                {
                    glVertexAttribPointer(attr.location, l.size, GL_FLOAT, GL_FALSE, kVertexStride, l.offset);
                    glEnableVertexAttribArray(attr.location);
                    break;
                }
    }

    // Uploads the material-owned per-object uniforms (everything the shader
    // declares per-object except the engine ones). Constant across every object
    // sharing the material, so the caller only invokes this when the material
    // changes. Driven generically through TypeInfoMap — no per-uniform code here.
    void UploadMaterialUniforms(Wheel::Renderer::Shader* a_Shader,
                                const Wheel::Renderer::Material* a_Material)
    {
        const auto& props = a_Material->GetProperties();
        for (const Wheel::Renderer::VarInfo& v : a_Shader->GetPerObjects())
        {
            if (IsEngineUniform(v.name)) continue;
            auto prop = props.find(v.name);
            if (prop == props.end()) continue;
            auto ti = Wheel::TypeInfoMap.find(static_cast<GLenum>(v.typeInfo));
            if (ti != Wheel::TypeInfoMap.end())
                ti->second.setUniform(v.location, prop->second.Data());
        }
    }
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
    uint32_t activeMatId    = UINT32_MAX;
    Shader*  activeShader   = nullptr;

    // Locations of the engine-driven per-object uniforms in the active shader.
    // Resolved once per program bind so the inner loop never searches by name.
    GLint locModel = -1, locTranslate = -1, locColor = -1;

    for (const auto& ro : *m_RenderedObjects)
    {
        if (ro.shaderId != activeShaderId)
        {
            activeShader   = m_Shaders[ro.shaderId];
            activeShaderId = ro.shaderId;
            glUseProgram(activeShader->GetID());
            BindAttributes(activeShader);

            // Per-bind uniforms: shared by every object drawn with this program
            // and supplied by the engine (the frame view-projection, the sampler
            // unit), so they're uploaded once per bind rather than per object.
            for (const VarInfo& v : activeShader->GetPerBinds())
            {
                if (static_cast<GLenum>(v.typeInfo) == GL_SAMPLER_2D)
                    glUniform1i(v.location, 0);
                else if (v.name == "u_viewProj")
                    glUniformMatrix4fv(v.location, 1, GL_FALSE, &m_ViewProjGL.First());
            }

            locModel = locTranslate = locColor = -1;
            for (const VarInfo& v : activeShader->GetPerObjects())
            {
                if      (v.name == "u_model")     locModel     = v.location;
                else if (v.name == "u_translate") locTranslate = v.location;
                else if (v.name == "u_color")     locColor     = v.location;
            }

            activeMatId = UINT32_MAX; // force a material re-upload after a program switch
        }

        if (ro.textureId != activeTexId)
        {
            glBindTexture(GL_TEXTURE_2D, m_Textures[ro.textureId]->m_ID);
            activeTexId = ro.textureId;
        }

        if (ro.materialId != activeMatId)
        {
            UploadMaterialUniforms(activeShader, m_Materials[ro.materialId]);
            activeMatId = ro.materialId;
        }

        // Engine-driven per-object uniforms, straight from the RenderedObject.
        if (locModel     >= 0) glUniformMatrix2fv(locModel, 1, GL_FALSE, ro.linear);
        if (locTranslate >= 0) glUniform2f(locTranslate, ro.translate[0], ro.translate[1]);
        if (locColor     >= 0) glUniform4f(locColor, ro.color.x, ro.color.y, ro.color.z, ro.color.w);

        // Per-object overrides, uploaded last so they win over the material
        // defaults. The shader's declared type drives the upload (as with
        // material uniforms), so the override just supplies the bytes.
        for (uint8_t i = 0; i < ro.overrideCount; ++i)
        {
            const RenderedObject::UniformOverride& ov = ro.overrides[i];
            for (const VarInfo& v : activeShader->GetPerObjects())
            {
                if (v.name != ov.name) continue;
                auto ti = TypeInfoMap.find(static_cast<GLenum>(v.typeInfo));
                if (ti != TypeInfoMap.end())
                    ti->second.setUniform(v.location, ov.value.Data());
                break;
            }
        }

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

static void SetDefault(Wheel::Renderer::Material* a_Material, const Wheel::Renderer::VarInfo& a_Var)
{
    using namespace Wheel::Math;
    switch (static_cast<GLenum>(a_Var.typeInfo))
    {
        case GL_FLOAT:
            a_Material->Set(a_Var.name, 0.0f);
            break;
        case GL_INT:        
            a_Material->Set(a_Var.name, 0);                          
            break;
        case GL_FLOAT_VEC2: 
            a_Material->Set(a_Var.name, Vector2());                  
            break;
        case GL_FLOAT_VEC3:
            a_Material->Set(a_Var.name, Vector3());                  
            break;
        case GL_FLOAT_VEC4: 
            a_Material->Set(a_Var.name, Vector4(1, 1, 1, 1));        
            break;
        case GL_FLOAT_MAT2: 
            a_Material->Set(a_Var.name, Matrix2x2());                
            break;
        case GL_FLOAT_MAT4: 
            a_Material->Set(a_Var.name, Matrix4x4());                
            break;
        default: 
            break; // sampler2D and unsupported types own no material value
    }
}

Wheel::Renderer::Material* Wheel::Renderer::Renderer::CreateMaterial(const std::string& a_ShaderName, const std::string& a_TextureName)
{
    // Reuse an existing material with the same shader/texture pair instead of
    // leaking a duplicate — mirrors LoadTexture()/AddShader().
    const std::string key = a_ShaderName + "_" + a_TextureName;
    auto existing = m_MaterialIndex.find(key);
    if (existing != m_MaterialIndex.end())
        return m_Materials[existing->second];

    Material* material = new Material();
    material->ShaderName = a_ShaderName;
    material->TextureName = a_TextureName;

    uint32_t id = static_cast<uint32_t>(m_Materials.size());
    m_Materials.push_back(material);
    m_MaterialIndex[key] = id;

    material->id = id;
    material->shaderId = GetShaderId(a_ShaderName);
    material->textureId = GetTextureId(a_TextureName);

    Shader* shader = m_Shaders[material->shaderId];
    for (const VarInfo& var : shader->GetPerObjects())
        SetDefault(material, var);
    for (const VarInfo& var : shader->GetPerBinds())
        SetDefault(material, var);

    return material;
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

    for (const char* shared : { "u_viewProj", "s_texture" })
        for (const VarInfo& v : shader->GetPerObjects())
            if (v.name == shared) { shader->AddPerBind(shared); break; }

    return id;
}
