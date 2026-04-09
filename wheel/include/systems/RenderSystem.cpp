#include "RenderSystem.h"

#include "RenderedObject.h"
#include "Renderer.h"

void Wheel::Engine::Systems::RenderSystem::GetComponentPool(IComponentPool* a_Pool)
{
    if (m_Transform2DPool == nullptr) m_Transform2DPool = dynamic_cast<ComponentPool<Components::Transform2D>*>(a_Pool);
    if (m_RenderPool == nullptr) m_RenderPool = dynamic_cast<ComponentPool<Components::Render2DComponent>*>(a_Pool);
    if (m_CameraPool == nullptr) m_CameraPool = dynamic_cast<ComponentPool<Components::CameraComponent>*>(a_Pool);
}

void Wheel::Engine::Systems::RenderSystem::Update(float deltaTime)
{
    m_RenderObjects.clear();
    for (unsigned int m_EntityID : m_EntityIDs)
    {
        Renderer::RenderedObject ro{};
        ro.Add2DComponent(m_RenderPool->GetComponent(m_EntityID), m_EntityID,
            m_Transform2DPool->GetComponent(m_EntityID));
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
