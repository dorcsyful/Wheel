#pragma once
#include "RenderSystem.h"

#include "RenderedObject.h"
#include "Renderer.h"
#include "EventBus.h"
#include "Events.h"
#include "core/Scene.h"

Wheel::Engine::Systems::RenderSystem::RenderSystem(const Description& a_Description) : System(a_Description)
{
    m_RenderObjects.reserve(MAX_ENTITIES);
    m_Description = a_Description;

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
    });

}

void Wheel::Engine::Systems::RenderSystem::Update(float deltaTime)
{
    m_RenderObjects.clear();

    const Components::Transform2D& camTransform =
        m_Scene->GetComponent<Components::Transform2D>(m_CameraEntity);
    const Components::CameraComponent& camComponent =
        m_Scene->GetComponent<Components::CameraComponent>(m_CameraEntity);

    for (unsigned int m_EntityID : m_EntityIDs)
    {
        if (!m_Scene->HasComponent<Components::Render2DComponent>(m_EntityID)) continue;

        Renderer::RenderedObject ro{};
        ro.Add2DComponent(
            m_Scene->GetComponent<Components::Render2DComponent>(m_EntityID),
            m_EntityID,
            m_Scene->GetComponent<Components::Transform2D>(m_EntityID),
            camTransform,
            camComponent);
        m_RenderObjects.push_back(ro);
    }
    std::sort(m_RenderObjects.begin(), m_RenderObjects.end(),ROSorter);
    m_Renderer->GetRenderedObjects(&m_RenderObjects);
}

bool Wheel::Engine::Systems::RenderSystem::ROSorter(Renderer::RenderedObject& a_A, Renderer::RenderedObject& a_B)
{
    if (a_A.shaderId < a_B.shaderId) return true;
    if (a_A.shaderId > a_B.shaderId) return false;
    if (a_A.textureId < a_B.textureId) return true;
    return false;
}
