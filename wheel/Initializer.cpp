#include "Initializer.h"

#include "Renderer.h"
#include "Texture.h"
#include "core/Scene.h"
#include "components/CameraComponent.h"
#include "components/Rigidbody2D.h"
#include "components/Sprite.h"
#include "components/Transform2D.h"
#include "components/Joint2D.h"
#include "components/Highlight2D.h"
#include "systems/Collision2DSystem.h"
#include "systems/InputSystem.h"
#include "systems/Physics2DSystem.h"
#include "systems/RenderSystem.h"

namespace Wheel::Components
{
}

void Wheel::Engine::Initializer::InitEverything(bool a_LoadAssets, Renderer::Renderer*& a_Out_Renderer,
    Scene*& a_Out_Scene, uint32_t& a_Out_Camera)
{
    //Initialize a window with a size adjusted to the monitor's resolution
    a_Out_Renderer = new Renderer::Renderer();
    a_Out_Renderer->Init(1280,720,"Wheel Engine", true);
    if (a_LoadAssets)
    {
        auto textures = GetRecursivePathsByExtensions(GetTexturePath(),Renderer::supported_image_extensions);
        auto verts = GetRecursivePathsByExtension(GetShaderPath(),".vert");
        auto frags = GetRecursivePathsByExtension(GetShaderPath(),".frag");
        for (auto& texture : textures)
        {
            a_Out_Renderer->LoadTexture(new Renderer::Texture(texture));
        }
        for (int i = 0; i < verts.size(); ++i)
        {
            std::string vert = verts[i].filename().string();
            vert = vert.substr(0, vert.find(".vert"));
            for (int j = 0; j < frags.size(); ++j)
            {
                std::string frag = frags[j].filename().string().substr(0,frags[j].filename().string().find(".frag"));
                if (frag == vert)
                {
                    a_Out_Renderer->AddShader(verts[i].filename().string(),frags[j].filename().string());
                    frags.erase(frags.begin()+j);
                    break;
                }

            }
        }
    }

    a_Out_Scene = new Engine::Scene();
    RegisterComponents(a_Out_Scene);
    RegisterSystems(a_Out_Scene, a_Out_Renderer);
    CreateFeatures(a_Out_Scene,a_Out_Renderer);
    a_Out_Camera = CreateCameraEntity(a_Out_Scene, a_Out_Renderer);
}

Wheel::Engine::Scene* Wheel::Engine::Initializer::CreateScene(Renderer::Renderer* renderer)
{
    Scene* scene = new Scene();
    RegisterComponents(scene);
    RegisterSystems(scene, renderer);
    return scene;
}

uint32_t Wheel::Engine::Initializer::CreateCameraEntity(Scene* a_Scene, Renderer::Renderer* renderer)
{
    int width, height;
    glfwGetWindowSize(renderer->GetWindow(), &width, &height);

    // Camera entity — has Transform2D + CameraComponent but no Sprite
    uint32_t cameraId = a_Scene->AddEntity();
    a_Scene->AddComponent<Components::Transform2D>(cameraId);
    Components::CameraComponent& cam = a_Scene->AddComponent<Components::CameraComponent>(cameraId);
    cam.SetCameraActive(true);
    cam.zoom = 1.0f; cam.width  = width; cam.height = height;
    Components::Transform2D& cameraTransform = a_Scene->GetComponent<Components::Transform2D>(cameraId);
    cameraTransform.name = "Camera";
    a_Scene->GetSystem<Systems::RenderSystem>()->SetCameraEntity(cameraId);
    a_Scene->GetSystem<Systems::InputSystem>()->SetCameraEntity(cameraId);
#ifdef DEBUG_BUILD
    Debugger::get().SetCameraEntity(cameraId);
#endif
    return cameraId;
}

Wheel::Renderer::Renderer* Wheel::Engine::Initializer::CreateRenderer(int a_Width, int a_Height, const std::string& a_Title,const std::vector<std::string>& a_Textures,
    const std::vector<std::string>& a_Shaders)
{
    Renderer::Renderer* renderer = new Renderer::Renderer();
    renderer->Init(a_Width, a_Height,a_Title.c_str(), true);
    for (auto& texture : a_Textures)
    {
        renderer->LoadTexture(new Renderer::Texture(texture));
    }
    for (auto& shader : a_Shaders)
    {
        renderer->AddShader(shader+".vert", shader+".frag");
    }

    return renderer;
}

void Wheel::Engine::Initializer::CreateFeatures(Scene* a_Scene,Renderer::Renderer* renderer)
{
    //Highlight feature
    auto id = renderer->GetShaderId("highlight");
    // Highlight: same sprite grown by thickness (world units = px / PIXELS_PER_UNIT),
    // tinted, one layer behind its owner.
    a_Scene->GetSystem<Systems::RenderSystem>()->RegisterFeature<Wheel::Components::Highlight2D>(-1,
        [id](const Wheel::Renderer::RenderedObject& a_Base,
             const Wheel::Components::Highlight2D& a_Highlight,
             std::vector<Wheel::Renderer::RenderedObject>& a_Out)
        {
            float t  = static_cast<float>(a_Highlight.thickness) / PIXELS_PER_UNIT;
            float sx = std::sqrt(a_Base.linear[0] * a_Base.linear[0] + a_Base.linear[1] * a_Base.linear[1]);
            float sy = std::sqrt(a_Base.linear[2] * a_Base.linear[2] + a_Base.linear[3] * a_Base.linear[3]);
            if (sx <= 0.0f || sy <= 0.0f) return;

            Wheel::Renderer::RenderedObject ro = a_Base;
            float fx = (sx + 2.0f * t) / sx;
            float fy = (sy + 2.0f * t) / sy;
            ro.linear[0] *= fx; ro.linear[1] *= fx;
            ro.linear[2] *= fy; ro.linear[3] *= fy;
            ro.color = a_Highlight.color;
            ro.shaderId = id;
            // dilation params in the enlarged quad's UV space
            ro.SetUniform("u_uvScale",     Wheel::Math::Vector2(fx, fy));
            ro.SetUniform("u_thicknessUV", Wheel::Math::Vector2(t / (sx + 2.0f * t), t / (sy + 2.0f * t)));
            a_Out.push_back(ro);
        });
}

void Wheel::Engine::Initializer::RegisterComponents(Scene* a_Scene)
{
    a_Scene->RegisterComponentType<Wheel::Components::Transform2D>();
    a_Scene->RegisterComponentType<Wheel::Components::Sprite>();
    a_Scene->RegisterComponentType<Wheel::Components::CameraComponent>();
    a_Scene->RegisterComponentType<Wheel::Components::BoxCollider2D>();
    a_Scene->RegisterComponentType<Wheel::Components::CircleCollider2D>();
    a_Scene->RegisterComponentType<Wheel::Components::Rigidbody2D>();
    a_Scene->RegisterComponentType<Wheel::Components::DistanceJoint2D>();
    a_Scene->RegisterComponentType<Wheel::Components::Highlight2D>();

}

void Wheel::Engine::Initializer::RegisterSystems(Scene* a_Scene, Renderer::Renderer* renderer)
{
    Description description = Description();

    Description transform = a_Scene->GetComponentDescription<Components::Transform2D>();
    Description render = a_Scene->GetComponentDescription<Components::Sprite>();
    Description cameraDesc = a_Scene->GetComponentDescription<Components::CameraComponent>();
    Description rigidbody2D = a_Scene->GetComponentDescription<Components::Rigidbody2D>();

    description.AddComponentType(transform.GetAsBitset());
    description.AddComponentType(render.GetAsBitset());
    auto render_system = a_Scene->RegisterSystem<Systems::RenderSystem>(description);
    render_system->GetRenderer(renderer);
    description.Reset();

    description.AddComponentType(transform.GetAsBitset());
    description.AddComponentType(cameraDesc.GetAsBitset());
    auto input_system = a_Scene->RegisterSystem<Systems::InputSystem>(description);
    input_system->Initialize(renderer->GetWindow());
    description.Reset();

    description.AddComponentType(transform.GetAsBitset());
    a_Scene->RegisterSystem<Systems::Collision2DSystem>(description);

    description.Reset();
    description.AddComponentType(transform.GetAsBitset());
    description.AddComponentType(rigidbody2D.GetAsBitset());
    a_Scene->RegisterSystem<Systems::Physics2DSystem>(description);

}