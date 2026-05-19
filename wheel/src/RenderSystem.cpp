#include "../include/systems/RenderSystem.h"

#include <cmath>

#include "RenderedObject.h"
#include "Renderer.h"
#include "EventBus.h"
#include "Events.h"
#include "core/Scene.h"

Wheel::Engine::Systems::RenderSystem::RenderSystem(const Description& a_Description) : System(a_Description)
{
    m_RenderObjects.reserve(MAX_ENTITIES);
    m_PrevTransforms.resize(MAX_ENTITIES);
    m_PrevValid.assign(MAX_ENTITIES, 0);
    m_Description = a_Description;

    m_Tokens.emplace_back();
    EventSystem::EventBus::Subscribe<Events::WindowResizeEvent>([this](const Events::WindowResizeEvent& e)
    {
        Components::CameraComponent& camComponent = m_Scene->GetComponent<Components::CameraComponent>(m_CameraEntity);

        if (m_designWidth == 0.0f)
        {
            m_designWidth = camComponent.width;
            m_designHeight = camComponent.height;
            m_initialZoom = camComponent.zoom;
        }

        float scale = std::min(e.width / m_designWidth, e.height / m_designHeight);
        camComponent.width = e.width;
        camComponent.height = e.height;
        camComponent.zoom = m_initialZoom * scale;
    }, m_Tokens.back());

}

void Wheel::Engine::Systems::RenderSystem::EnsurePools()
{
    if (!m_TransformPool) {
        m_TransformPool = m_Scene->GetComponentPool<Components::Transform2D>();
        m_RenderPool    = m_Scene->GetComponentPool<Components::Render2DComponent>();
        m_CameraPool    = m_Scene->GetComponentPool<Components::CameraComponent>();
    }
}

void Wheel::Engine::Systems::RenderSystem::SavePreviousTransforms()
{
    EnsurePools();
    for (unsigned int entityId : m_EntityIDs)
    {
        // Guards the ECS hole where RemoveComponent doesn't update m_EntityIDs.
        if (!m_Scene->HasComponent<Components::Render2DComponent>(entityId)) continue;

        const Components::Transform2D& t = m_TransformPool->GetComponent(entityId);
        m_PrevTransforms[entityId] = { t.GetPosition(), t.GetRotation(), t.GetScale() };
        m_PrevValid[entityId] = 1;
    }
}

void Wheel::Engine::Systems::RenderSystem::Render(float a_Alpha)
{
    EnsurePools();

    if (a_Alpha < 0.0f) a_Alpha = 0.0f;
    if (a_Alpha > 1.0f) a_Alpha = 1.0f;

    m_RenderObjects.clear();


    const Components::Transform2D&    camTransform  = m_TransformPool->GetComponent(m_CameraEntity);
    const Components::CameraComponent& camComponent = m_CameraPool->GetComponent(m_CameraEntity);
    const Math::Matrix4x4 viewProj = Renderer::RenderedObject::ComputeViewProj(camTransform, camComponent);
    m_Renderer->SetViewProj(viewProj);

    for (unsigned int entityId : m_EntityIDs)
    {
        if (!m_Scene->HasComponent<Components::Render2DComponent>(entityId)) continue;

        const auto& render = m_RenderPool->GetComponent(entityId);
        const Components::Transform2D& curr = m_TransformPool->GetComponent(entityId);

        // Fall back to the current transform if there is no prior snapshot yet
        // (first frame, just-spawned entity, or while the sim is paused).
        Math::Vector2 pos= curr.GetPosition();
        float rot = curr.GetRotation();
        Math::Vector2 scale= curr.GetScale();

        if (m_PrevValid[entityId])
        {
            const TransformSnapshot& prev = m_PrevTransforms[entityId];
            pos = prev.position + (pos - prev.position) * a_Alpha;

            // Interpolate along the shortest arc: wrap the delta into
            // [-180,180] so e.g. 359 -> 1 spins +2 deg, not -358.
            float dRot = std::fmod(rot - prev.rotation + 180.0f, 360.0f);
            if (dRot < 0.0f) dRot += 360.0f;
            dRot -= 180.0f;
            rot = prev.rotation + dRot * a_Alpha;

            scale = prev.scale + (scale - prev.scale) * a_Alpha;
        }

        Renderer::RenderedObject ro{};
        ro.Add2DComponentInterpolated(render, entityId, pos, rot, scale);
        ro.textureId = m_Renderer->GetTextureId(render.TextureName);
        ro.shaderId  = m_Renderer->GetShaderId(render.ShaderName);
        m_RenderObjects.push_back(ro);
    }
    std::sort(m_RenderObjects.begin(), m_RenderObjects.end(), ROSorter);
    m_Renderer->GetRenderedObjects(&m_RenderObjects);
}

bool Wheel::Engine::Systems::RenderSystem::ROSorter(Renderer::RenderedObject& a_A, Renderer::RenderedObject& a_B)
{
    if (a_A.shaderId < a_B.shaderId) return true;
    if (a_A.shaderId > a_B.shaderId) return false;
    if (a_A.textureId < a_B.textureId) return true;
    return false;
}
