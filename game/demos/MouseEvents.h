#pragma once
#include "core/Scene.h"

class MouseEvents {
public:
    MouseEvents(Wheel::Core::Scene* scene, uint32_t a_BaseMaterial) : m_Scene(scene), m_BaseMaterial(a_BaseMaterial) {}
    ~MouseEvents() {m_Scene->Reset();}
    uint32_t SpawnObjects(float x, float y, float w, float h);
    void Initialize();

private:
    void CreateGroundEntity();
    uint32_t m_BaseMaterial;
    Wheel::Core::Scene* m_Scene;
    std::vector<Wheel::EventSystem::SubscriptionToken> m_SubscriptionTokens;

};
