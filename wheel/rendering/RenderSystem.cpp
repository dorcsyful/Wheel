#include "RenderSystem.h"

#include <cmath>

#include "_gfx/include/RenderedObject.h"
#include "_gfx/include/Renderer.h"
#include "../events/EventBus.h"
#include "../events/Events.h"
#include "core/Scene.h"
#include "helpers/Coordinates.h"

Wheel::Rendering::RenderSystem::RenderSystem(const Core::Description& a_Description) : System(a_Description)
{
    m_RenderObjects.reserve(MAX_ENTITIES);
    m_RenderFeatures.reserve(MAX_ENTITIES);
    m_PrevTransforms.resize(MAX_ENTITIES);
    m_PrevValid.assign(MAX_ENTITIES, 0);
    m_Description = a_Description;

    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::WindowResizeEvent>([this](const Events::WindowResizeEvent& e)
    {
        CameraComponent& camComponent = m_Scene->GetComponent<CameraComponent>(m_CameraEntity);
        camComponent.width  = e.width;
        camComponent.height = e.height;
    }, m_Tokens.back());

}

void Wheel::Rendering::RenderSystem::EnsurePools()
{
    if (!m_TransformPool) {
        m_TransformPool = m_Scene->GetComponentPool<Common::Transform2D>();
        m_RenderPool    = m_Scene->GetComponentPool<Sprite>();
        m_CameraPool    = m_Scene->GetComponentPool<CameraComponent>();
    }
}

void Wheel::Rendering::RenderSystem::SavePreviousTransforms()
{
    EnsurePools();
    for (unsigned int entityId : m_EntityIDs)
    {

        const Common::Transform2D& t = m_TransformPool->GetComponent(entityId);
        m_PrevTransforms[entityId] = { t.GetPosition(), t.GetRotationInRadians(), t.GetScale() };
        m_PrevValid[entityId] = 1;
    }
}

void Wheel::Rendering::RenderSystem::Render(float a_Alpha)
{
    EnsurePools();

    if (a_Alpha < 0.0f) a_Alpha = 0.0f;
    if (a_Alpha > 1.0f) a_Alpha = 1.0f;

    m_RenderObjects.clear();


    const Common::Transform2D&    camTransform  = m_TransformPool->GetComponent(m_CameraEntity);
    const CameraComponent& camComponent = m_CameraPool->GetComponent(m_CameraEntity);
    const Math::Matrix4x4 viewProj = Renderer::RenderedObject::ComputeViewProj(camTransform, camComponent);
    m_Renderer->SetViewProj(viewProj);
    m_RenderFeatures.clear();

    for (unsigned int entityId : m_EntityIDs)
    {
        if (!m_Scene->HasComponent<Sprite>(entityId)) continue;

        const auto& render = m_RenderPool->GetComponent(entityId);
        const Common::Transform2D& curr = m_TransformPool->GetComponent(entityId);

        // Fall back to the current transform if there is no prior snapshot yet
        // (first frame, just-spawned entity, or while the sim is paused).
        Math::Vector2 pos= curr.GetPosition();
        float rot = curr.GetRotationInRadians();
        Math::Vector2 scale= curr.GetScale();

        if (m_PrevValid[entityId])
        {
            const TransformSnapshot& prev = m_PrevTransforms[entityId];
            pos = prev.position + (pos - prev.position) * a_Alpha;

            // Interpolate along the shortest arc: wrap the delta into
            // [-180,180] so e.g. 359 -> 1 spins +2 deg, not -358.
            float dRot = std::fmod(rot - prev.rotation + PI, 2 * PI);
            if (dRot < 0.0f) dRot += 2 * PI;
            dRot -= PI;
            rot = prev.rotation + dRot * a_Alpha;

            scale = prev.scale + (scale - prev.scale) * a_Alpha;
        }

        const Renderer::Material* material = m_Renderer->GetMaterial(render.MaterialName);

        Renderer::RenderedObject ro{};
        ro.Add2DComponentInterpolated(render, entityId, pos, rot, scale);
        ro.textureId  = material->textureId;
        ro.shaderId   = material->shaderId;
        ro.materialId = render.MaterialName;
        ro.level = render.level;
        m_RenderObjects.push_back(ro);
        for (const auto& current : m_Features)
        {
            if (!current.has(m_Scene, entityId)) continue;

            size_t before = m_RenderFeatures.size();
            current.result(m_Scene, entityId, ro, m_RenderFeatures);

            // relative to owner's layer
            for (size_t i = before; i < m_RenderFeatures.size(); ++i)
                m_RenderFeatures[i].level = ro.level + current.order;
        }
    }
    m_RenderObjects.insert(m_RenderObjects.end(), m_RenderFeatures.begin(), m_RenderFeatures.end());
    std::sort(m_RenderObjects.begin(), m_RenderObjects.end(), ROSorter);
    m_Renderer->GetRenderedObjects(&m_RenderObjects);
}

bool Wheel::Rendering::RenderSystem::ROSorter(Renderer::RenderedObject& a_A, Renderer::RenderedObject& a_B)
{
    if (a_A.level < a_B.level) return true;
    if (a_A.level > a_B.level) return false;
    if (a_A.shaderId < a_B.shaderId) return true;
    if (a_A.shaderId > a_B.shaderId) return false;
    if (a_A.textureId < a_B.textureId) return true;
    return false;
}

Wheel::Math::Vector2 Wheel::Rendering::RenderSystem::WorldToScreen(const Math::Vector2& a_World)
{
    EnsurePools();
    return WorldToScreen(a_World,
                         m_TransformPool->GetComponent(m_CameraEntity),
                         m_CameraPool->GetComponent(m_CameraEntity));
}

Wheel::Math::Vector2 Wheel::Rendering::RenderSystem::ScreenToWorld(const Math::Vector2& a_Screen)
{
    EnsurePools();
    return ScreenToWorld(a_Screen,
                         m_TransformPool->GetComponent(m_CameraEntity),
                         m_CameraPool->GetComponent(m_CameraEntity));
}

Wheel::Math::Vector2 Wheel::Rendering::RenderSystem::WorldToScreen(const Math::Vector2& a_World,
    const Common::Transform2D& a_CameraTransform,
    const CameraComponent& a_CameraComponent)
{
    return Helpers::Coordinates::WorldToScreenCoordinates(a_World, a_CameraComponent, a_CameraTransform);
}

Wheel::Math::Vector2 Wheel::Rendering::RenderSystem::ScreenToWorld(const Math::Vector2& a_Screen,
    const Common::Transform2D& a_CameraTransform,
    const CameraComponent& a_CameraComponent)
{
    return Helpers::Coordinates::ScreenToWorldCoordinates(a_Screen, a_CameraComponent, a_CameraTransform);
}
