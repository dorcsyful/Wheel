#include "RenderSystem.h"

#include "RenderedObject.h"
#include "Renderer.h"
#include "../../game/Start.h"
#include "EventBus.h"
#include "RenderEvents.h"
extern std::shared_ptr<Start> gStart;

Wheel::Engine::Systems::RenderSystem::RenderSystem(const Description& a_Description) : System(a_Description)
{
    m_RenderObjects.reserve(MAX_ENTITIES);
    m_Description = a_Description;

    EventSystem::EventBus::Subscribe<Events::WindowResizeEvent>([this](const Events::WindowResizeEvent& e)
    {
        Components::CameraComponent& camComponent = gStart->GetScene()->GetComponent<Components::CameraComponent>(m_CameraEntity);
        camComponent.width = e.width;
        camComponent.height = e.height;
    });

}

void Wheel::Engine::Systems::RenderSystem::Update(float deltaTime)
{
    m_RenderObjects.clear();

    const Components::Transform2D& camTransform =
        gStart->GetScene()->GetComponent<Components::Transform2D>(m_CameraEntity);
    const Components::CameraComponent& camComponent =
        gStart->GetScene()->GetComponent<Components::CameraComponent>(m_CameraEntity);

    for (unsigned int m_EntityID : m_EntityIDs)
    {
        Renderer::RenderedObject ro{};
        ro.Add2DComponent(
            gStart->GetScene()->GetComponent<Components::Render2DComponent>(m_EntityID),
            m_EntityID,
            gStart->GetScene()->GetComponent<Components::Transform2D>(m_EntityID),
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
