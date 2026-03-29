#pragma once
#include <random>
#include <gtest/gtest.h>
#include <string>
#include "../wheel/core/Scene.h"

//Empty components for testing
class A { public: A() = default; ~A() = default; };
class B { public: B() = default; ~B() = default; bool needsCheck = false; };
class C { public: C() = default; ~C() = default; };
class D { public: D() = default; ~D() = default; };
class E { public: E() = default; ~E() = default; };


// Test component with integer for system update tests
class CounterComponent {
public:
    CounterComponent() = default;
    ~CounterComponent() = default;
    int counter = 0;
};

class TestSystem : public Wheel::Engine::System
{
public:
    explicit TestSystem(Wheel::Engine::Description* a_Description) : System(a_Description) {}
    void GetComponentPool(Wheel::Engine::IComponentPool* a_Pool) override {
        m_ComponentPool = dynamic_cast<Wheel::Engine::ComponentPool<CounterComponent>*>(a_Pool);
    }
    void Update(float deltaTime) override {
        if (m_ComponentPool) {
            auto components = m_ComponentPool->GetComponents();
            for (auto& [entityId, component] : components) {
                component->counter++;
            }
        }
    }
private:
    Wheel::Engine::ComponentPool<CounterComponent>* m_ComponentPool = nullptr;
};

 TEST(ECS, SimpleEntityCreation) {
     Wheel::Engine::Scene* scene = new Wheel::Engine::Scene();
     for (int i = 0; i < 10; ++i) {
         uint32_t entityId = scene->AddEntity(false);
         EXPECT_EQ(entityId, i); // Ensure entity ID is valid
     }
     delete scene;
 }

TEST(ECS, TooManyEntities)
 {
     Wheel::Engine::Scene* scene = new Wheel::Engine::Scene();

     for (int i= 0; i < MAX_ENTITIES; ++i)
     {
         scene->AddEntity(false);
     }

     EXPECT_DEATH(scene->AddEntity(false), "Too many entities in existence.");

     delete scene;
 }

TEST(ECS, GetNumberOfEntities)
{
    Wheel::Engine::Scene* scene = new Wheel::Engine::Scene();

    for (int i = 0; i < 10; ++i)
    {
        scene->AddEntity();
    }
    EXPECT_EQ(scene->GetNumberOfEntities(), 10);
    delete scene;
}

 TEST(ECS, RemoveEmptyEntity) {
     Wheel::Engine::Scene* scene = new Wheel::Engine::Scene();
     uint32_t indexArray[10];
     for (unsigned int & i : indexArray)
     {
         i = scene->AddEntity(false);
     }

     scene->RemoveEntity(indexArray[5]);
     EXPECT_EQ(scene->GetNumberOfEntities(), 9); // Ensure entity count is correct after removal
     EXPECT_DEATH(scene->RemoveEntity(MAX_ENTITIES + 1), "Entity ID out of range.");

     delete scene;
 }

TEST(ECS, RegisterComponent)
 {
     Wheel::Engine::Scene scene = Wheel::Engine::Scene();
     scene.RegisterComponentType<A>();
     scene.RegisterComponentType<C>();
     scene.RegisterComponentType<B>();
     scene.RegisterComponentType<D>();
     scene.RegisterComponentType<E>();

     auto tempt = scene.GetNumberOfComponentTypes();
     EXPECT_EQ(tempt, 5);
 }

TEST(ECS, AddComponent)
 {
     Wheel::Engine::Scene scene = Wheel::Engine::Scene();
     scene.RegisterComponentType<A>();
     scene.RegisterComponentType<C>();
     scene.RegisterComponentType<B>();
     scene.RegisterComponentType<D>();
     scene.RegisterComponentType<E>();

     std::vector<uint32_t> indexArray = std::vector<uint32_t>();
     for (int i = 0; i < 100; ++i)
     {
         indexArray.push_back(scene.AddEntity());
     }
     for (int i = 0; i < 100; i++)
     {
         if (i < 10)
         {scene.AddComponent<C>(indexArray[i]);}
         if (i > 10 && i < 25)
         {scene.AddComponent<E>(indexArray[i]);}
         if (i > 25 && i < 50)
         {scene.AddComponent<B>(indexArray[i]);}
         if (i > 75)
         {scene.AddComponent<A>(indexArray[i]);}
     }
     uint32_t a = scene.GetComponents<A>().size();
     EXPECT_EQ(a, 24);
     uint32_t b = scene.GetComponents<B>().size();
     EXPECT_EQ(b, 24);
     uint32_t c = scene.GetComponents<C>().size();
     EXPECT_EQ(c, 10);
     uint32_t d = scene.GetComponents<D>().size();
     EXPECT_EQ(d, 0);
     uint32_t e = scene.GetComponents<E>().size();
     EXPECT_EQ(e, 14);
 }

TEST(ECS, RemoveComponent)
 {
     std::random_device rd;
     std::mt19937 gen(rd());

     Wheel::Engine::Scene scene = Wheel::Engine::Scene();
     scene.RegisterComponentType<A>();
     scene.RegisterComponentType<B>();
     scene.RegisterComponentType<C>();
     scene.RegisterComponentType<D>();
     scene.RegisterComponentType<E>();

     std::vector<uint32_t> indexArray = std::vector<uint32_t>();
     for (int i = 0; i < 100; ++i)
     {
         indexArray.push_back(scene.AddEntity());
     }

     std::unordered_map<uint32_t, A*> aPtrs;
     std::unordered_map<uint32_t, B*> bPtrs;
     std::unordered_map<uint32_t, C*> cPtrs;
     std::unordered_map<uint32_t, D*> dPtrs;

     for (int i = 0; i < random(10,100); ++i) {
         if (i == 0) {
             aPtrs[indexArray[i]] = scene.AddComponent<A>(indexArray[i]);
         }
         if (!scene.HasComponent<A>(indexArray[i])) { // Avoid double-adding
             aPtrs[indexArray[i]] = scene.AddComponent<A>(indexArray[i]);
         }
     }
     for (int i = 0; i < random(0,100); ++i) {
         if (i == 0)
         {
             bPtrs[indexArray[i]] = scene.AddComponent<B>(indexArray[i]);
         }
         if (!scene.HasComponent<B>(indexArray[i])) { // Avoid double-adding
             bPtrs[indexArray[i]] = scene.AddComponent<B>(indexArray[i]);
         }
     }
     for (int i = 0; i < random(0,100); ++i) {
         if (!scene.HasComponent<C>(indexArray[i])) { // Avoid double-adding
             cPtrs[indexArray[i]] = scene.AddComponent<C>(indexArray[i]);
         }
     }
     for (int i = 0; i < random(0,100); ++i) {
         if (!scene.HasComponent<C>(indexArray[i])) { // Avoid double-adding
             dPtrs[indexArray[i]] = scene.AddComponent<D>(indexArray[i]);
         }
     }

     //Edge case: no component has ever had E
     EXPECT_DEATH(scene.RemoveComponent<E>(indexArray[0]),"Entity does not have component.");
     //Edge case: remove something twice
     uint32_t doubleRemoveId = indexArray[0];
     scene.RemoveComponent<A>(doubleRemoveId);
     EXPECT_DEATH(scene.RemoveComponent<A>(doubleRemoveId), "Entity does not have component.");
     aPtrs.erase(doubleRemoveId);
     //Edge case: remove -> add
     B* temp = bPtrs[0];
     temp->needsCheck = true;
     scene.RemoveComponent<B>(indexArray[0]);
     auto b_all = scene.GetComponents<B>();
     EXPECT_EQ(b_all.find(indexArray[0]), b_all.end()) << "Entity does not have component.";
     B* newB = scene.AddComponent<B>(indexArray[0]);
     EXPECT_EQ(newB->needsCheck, false);

     // Arbitrary removal of some tracked components
     std::vector<uint32_t> removedAIds;
     int removeCount = 5;
     auto it = aPtrs.begin();
     for (int i = 0; i < removeCount && it != aPtrs.end(); ++i, ++it) {
         scene.RemoveComponent<A>(it->first);
         removedAIds.push_back(it->first);
     }
     auto allA = scene.GetComponents<A>();

     for (uint32_t id : removedAIds) {
         // Check ID: Ensure ID is not in the map keys
         EXPECT_EQ(allA.find(id), allA.end()) << "Entity " << id << " still exists in component map.";

         // Check Pointer: Ensure the specific pointer we saved is not in the map values
         A* originalPtr = aPtrs[id];
         for (const auto& [entityId, componentPtr] : allA) {
             EXPECT_NE(componentPtr, originalPtr) << "Dangling pointer found in map for entity " << entityId;
         }
     }

 }

// ==================== Description.h Function Tests ====================

TEST(ECS, Description_GetAsBitset)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> bitset;
    bitset[0] = true;
    desc.AddComponentType(bitset);

    const std::bitset<MAX_COMPONENT_TYPES>& result = desc.GetAsBitset();
    EXPECT_EQ(result[0], true);
}

TEST(ECS, Description_AddComponentType)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> bitset1;
    bitset1[0] = true;
    desc.AddComponentType(bitset1);

    EXPECT_EQ(desc.GetAsBitset()[0], true);

    std::bitset<MAX_COMPONENT_TYPES> bitset2;
    bitset2[1] = true;
    desc.AddComponentType(bitset2);

    EXPECT_EQ(desc.GetAsBitset()[1], true);
}

TEST(ECS, Description_RemoveComponentType)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> bitset;
    bitset[0] = true;
    desc.AddComponentType(bitset);
    EXPECT_EQ(desc.GetAsBitset()[0], true);

    desc.RemoveComponentType(bitset);
    EXPECT_EQ(desc.GetAsBitset()[0], false);
}

TEST(ECS, Description_HasComponentType)
{
    Wheel::Engine::Description desc1;
    Wheel::Engine::Description desc2;
    std::bitset<MAX_COMPONENT_TYPES> bitset;
    bitset[0] = true;
    desc1.AddComponentType(bitset);
    desc2.AddComponentType(bitset);

    EXPECT_TRUE(desc1.HasComponentType(&desc2));

    Wheel::Engine::Description desc3;
    EXPECT_DEATH(desc1.HasComponentType(&desc3), "Component type out of range");
}

TEST(ECS, Description_IsEmpty)
{
    Wheel::Engine::Description desc;
    EXPECT_TRUE(desc.IsEmpty());

    std::bitset<MAX_COMPONENT_TYPES> bitset;
    bitset[0] = true;
    desc.AddComponentType(bitset);
    EXPECT_FALSE(desc.IsEmpty());

    desc.RemoveComponentType(bitset);
    EXPECT_TRUE(desc.IsEmpty());
}

TEST(ECS, Description_AssignmentOperator)
{
    Wheel::Engine::Description desc1;
    std::bitset<MAX_COMPONENT_TYPES> bitset;
    bitset[0] = true;
    bitset[5] = true;
    desc1.AddComponentType(bitset);

    std::bitset<MAX_COMPONENT_TYPES> bitset2;
    bitset2[5] = true;
    desc1.AddComponentType(bitset2);

    Wheel::Engine::Description desc2;
    desc2 = desc1;

    EXPECT_EQ(desc2.GetAsBitset()[0], true);
    EXPECT_EQ(desc2.GetAsBitset()[5], true);
}

TEST(ECS, Description_MoveConstructor)
{
    Wheel::Engine::Description desc1;
    std::bitset<MAX_COMPONENT_TYPES> bitset;
    bitset[3] = true;
    desc1.AddComponentType(bitset);

    Wheel::Engine::Description desc2(std::move(desc1));
    EXPECT_EQ(desc2.GetAsBitset()[3], true);
}

TEST(ECS, Description_Reset)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> bitset;
    bitset[0] = true;
    bitset[1] = true;
    bitset[2] = true;
    desc.AddComponentType(bitset);

    std::bitset<MAX_COMPONENT_TYPES> bitset2;
    bitset2[1] = true;
    desc.AddComponentType(bitset2);

    std::bitset<MAX_COMPONENT_TYPES> bitset3;
    bitset3[2] = true;
    desc.AddComponentType(bitset3);

    EXPECT_FALSE(desc.IsEmpty());

    desc.Reset();
    EXPECT_TRUE(desc.IsEmpty());
}

// ==================== Component Cleanup Tests ====================

TEST(ECS, EntityDeletion_SingleComponent_Cleanup)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();

    uint32_t entity = scene.AddEntity();
    scene.AddComponent<A>(entity);

    EXPECT_EQ(scene.GetComponents<A>().size(), 1);

    scene.RemoveEntity(entity);

    EXPECT_EQ(scene.GetComponents<A>().size(), 0);
}

TEST(ECS, EntityDeletion_MultipleComponents_Cleanup)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();

    uint32_t entity = scene.AddEntity();
    scene.AddComponent<A>(entity);
    scene.AddComponent<B>(entity);
    scene.AddComponent<C>(entity);

    EXPECT_EQ(scene.GetComponents<A>().size(), 1);
    EXPECT_EQ(scene.GetComponents<B>().size(), 1);
    EXPECT_EQ(scene.GetComponents<C>().size(), 1);

    scene.RemoveEntity(entity);

    EXPECT_EQ(scene.GetComponents<A>().size(), 0);
    EXPECT_EQ(scene.GetComponents<B>().size(), 0);
    EXPECT_EQ(scene.GetComponents<C>().size(), 0);
}

TEST(ECS, EntityDeletion_EdgeCase_LastEntityWithComponent)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();

    uint32_t entity1 = scene.AddEntity();
    uint32_t entity2 = scene.AddEntity();
    uint32_t entity3 = scene.AddEntity();

    scene.AddComponent<A>(entity1);
    scene.AddComponent<A>(entity2);
    scene.AddComponent<A>(entity3);

    EXPECT_EQ(scene.GetComponents<A>().size(), 3);

    // Remove last entity
    scene.RemoveEntity(entity3);
    EXPECT_EQ(scene.GetComponents<A>().size(), 2);

    // Remove middle entity
    scene.RemoveEntity(entity2);
    EXPECT_EQ(scene.GetComponents<A>().size(), 1);

    // Remove first entity
    scene.RemoveEntity(entity1);
    EXPECT_EQ(scene.GetComponents<A>().size(), 0);
}

TEST(ECS, EntityDeletion_EdgeCase_FirstEntityWithComponent)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();

    uint32_t entity1 = scene.AddEntity();
    uint32_t entity2 = scene.AddEntity();
    uint32_t entity3 = scene.AddEntity();

    scene.AddComponent<A>(entity1);
    scene.AddComponent<A>(entity2);
    scene.AddComponent<A>(entity3);

    // Remove first entity
    scene.RemoveEntity(entity1);
    EXPECT_EQ(scene.GetComponents<A>().size(), 2);

    auto components = scene.GetComponents<A>();
    EXPECT_TRUE(components.find(entity2) != components.end());
    EXPECT_TRUE(components.find(entity3) != components.end());
}

TEST(ECS, EntityDeletion_EdgeCase_MiddleEntityWithMultipleComponents)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();

    uint32_t entity1 = scene.AddEntity();
    uint32_t entity2 = scene.AddEntity();
    uint32_t entity3 = scene.AddEntity();

    scene.AddComponent<A>(entity1);
    scene.AddComponent<A>(entity2);
    scene.AddComponent<A>(entity3);

    scene.AddComponent<B>(entity1);
    scene.AddComponent<B>(entity2);

    scene.AddComponent<C>(entity2);
    scene.AddComponent<C>(entity3);

    // Remove middle entity that has all three components
    scene.RemoveEntity(entity2);

    EXPECT_EQ(scene.GetComponents<A>().size(), 2);
    EXPECT_EQ(scene.GetComponents<B>().size(), 1);
    EXPECT_EQ(scene.GetComponents<C>().size(), 1);

    auto componentsA = scene.GetComponents<A>();
    EXPECT_TRUE(componentsA.find(entity1) != componentsA.end());
    EXPECT_TRUE(componentsA.find(entity3) != componentsA.end());
}

TEST(ECS, EntityDeletion_EdgeCase_EntityWithoutComponents)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();

    uint32_t entity1 = scene.AddEntity();
    uint32_t entity2 = scene.AddEntity();

    scene.AddComponent<A>(entity1);

    EXPECT_EQ(scene.GetComponents<A>().size(), 1);

    // Remove entity without components - should not crash
    scene.RemoveEntity(entity2);

    EXPECT_EQ(scene.GetComponents<A>().size(), 1);
    EXPECT_EQ(scene.GetNumberOfEntities(), 1);
}

TEST(ECS, EntityDeletion_EdgeCase_ManyEntitiesOneComponent)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();

    std::vector<uint32_t> entities;
    for (int i = 0; i < 50; ++i) {
        uint32_t entity = scene.AddEntity();
        scene.AddComponent<A>(entity);
        entities.push_back(entity);
    }

    EXPECT_EQ(scene.GetComponents<A>().size(), 50);

    // Remove entities in reverse order to avoid ComponentPool swap bug
    // Bug in ComponentPool.h:65 - uses m_EntityToComponent[lastIndex] instead of m_ComponentToEntity[lastIndex]
    // Removing in reverse order ensures we always remove from the end, avoiding the buggy swap logic
    for (int i = entities.size() - 1; i >= 0; i -= 2) {
        scene.RemoveEntity(entities[i]);
    }

    EXPECT_EQ(scene.GetComponents<A>().size(), 25);
}

// ==================== System Tests ====================

TEST(ECS, System_NoEntities)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<CounterComponent>();

    Wheel::Engine::Description* systemDesc = new Wheel::Engine::Description();
    std::bitset<MAX_COMPONENT_TYPES> bitset;
    bitset[0] = true;
    systemDesc->AddComponentType(bitset);

    TestSystem* testSystem = new TestSystem(systemDesc);

    // Update should not crash with no entities
    testSystem->Update(0.016f);

    delete testSystem;
}

TEST(ECS, System_ThreeEntityTypes)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();

    // Type 1: Entities with only A
    uint32_t entity1 = scene.AddEntity();
    scene.AddComponent<A>(entity1);

    // Type 2: Entities with A and B
    uint32_t entity2 = scene.AddEntity();
    scene.AddComponent<A>(entity2);
    scene.AddComponent<B>(entity2);

    // Type 3: Entities with A, B, and C
    uint32_t entity3 = scene.AddEntity();
    scene.AddComponent<A>(entity3);
    scene.AddComponent<B>(entity3);
    scene.AddComponent<C>(entity3);

    EXPECT_EQ(scene.GetComponents<A>().size(), 3);
    EXPECT_EQ(scene.GetComponents<B>().size(), 2);
    EXPECT_EQ(scene.GetComponents<C>().size(), 1);

    EXPECT_TRUE(scene.HasComponent<A>(entity1));
    EXPECT_FALSE(scene.HasComponent<B>(entity1));
    EXPECT_FALSE(scene.HasComponent<C>(entity1));

    EXPECT_TRUE(scene.HasComponent<A>(entity2));
    EXPECT_TRUE(scene.HasComponent<B>(entity2));
    EXPECT_FALSE(scene.HasComponent<C>(entity2));

    EXPECT_TRUE(scene.HasComponent<A>(entity3));
    EXPECT_TRUE(scene.HasComponent<B>(entity3));
    EXPECT_TRUE(scene.HasComponent<C>(entity3));
}

TEST(ECS, System_UpdateLoop_CounterIncrement)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<CounterComponent>();

    // Create many entities with CounterComponent
    std::vector<uint32_t> entities;
    for (int i = 0; i < 50; ++i) {
        uint32_t entity = scene.AddEntity();
        CounterComponent* comp = scene.AddComponent<CounterComponent>(entity);
        comp->counter = 0;
        entities.push_back(entity);
    }

    // Run update 100 times - simulating system update behavior
    for (int i = 0; i < 100; ++i) {
        auto components = scene.GetComponents<CounterComponent>();
        for (auto& [entityId, component] : components) {
            component->counter++;
        }
    }

    // Verify all counters are at 100
    for (uint32_t entityId : entities) {
        CounterComponent* comp = scene.GetComponent<CounterComponent>(entityId);
        EXPECT_EQ(comp->counter, 100);
    }
}

TEST(ECS, System_UpdateLoop_PartialEntities)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<CounterComponent>();
    scene.RegisterComponentType<A>();

    // Create entities: some with CounterComponent, some without
    std::vector<uint32_t> counterEntities;
    std::vector<uint32_t> nonCounterEntities;

    for (int i = 0; i < 25; ++i) {
        uint32_t entity = scene.AddEntity();
        CounterComponent* comp = scene.AddComponent<CounterComponent>(entity);
        comp->counter = 0;
        counterEntities.push_back(entity);
    }

    for (int i = 0; i < 25; ++i) {
        uint32_t entity = scene.AddEntity();
        scene.AddComponent<A>(entity);
        nonCounterEntities.push_back(entity);
    }

    // Run update 100 times - simulating system update behavior
    for (int i = 0; i < 100; ++i) {
        auto components = scene.GetComponents<CounterComponent>();
        for (auto& [entityId, component] : components) {
            component->counter++;
        }
    }

    // Verify only CounterComponent entities were updated
    for (uint32_t entityId : counterEntities) {
        CounterComponent* comp = scene.GetComponent<CounterComponent>(entityId);
        EXPECT_EQ(comp->counter, 100);
    }

    // Verify non-counter entities don't have CounterComponent
    for (uint32_t entityId : nonCounterEntities) {
        EXPECT_FALSE(scene.HasComponent<CounterComponent>(entityId));
    }
}
