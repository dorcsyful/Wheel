#include "RenderSystem.h"

void Wheel::Engine::Systems::RenderSystem::GetComponentPool(IComponentPool* a_Pool)
{
    if (m_Transform2DPool == nullptr) m_Transform2DPool = dynamic_cast<ComponentPool<Components::Transform2D>*>(a_Pool);
    if (m_RenderPool == nullptr) m_RenderPool = dynamic_cast<ComponentPool<Components::Render2DComponent>*>(a_Pool);
    if (m_CameraPool == nullptr) m_CameraPool = dynamic_cast<ComponentPool<Components::CameraComponent>*>(a_Pool);
}

void Wheel::Engine::Systems::RenderSystem::Update(float deltaTime)
{
}
