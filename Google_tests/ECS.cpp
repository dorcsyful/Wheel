#pragma once
#include <gtest/gtest.h>
#include <string>
#include "../wheel/include/core/Scene.h"

// ==================== Test Components ====================

struct A { int value = 0; };
struct B { float x = 0.0f; float y = 0.0f; };
struct C { bool flag = false; };
struct D { int data = -1; };
struct E { double val = 3.14; };
struct Counter { int count = 0; };

// ==================== Entity Creation ====================

TEST(ECS, Entity_IDsAreSequential)
{
    Wheel::Engine::Scene scene;
    for (int i = 0; i < 10; ++i) {
        uint32_t id = scene.AddEntity();
        EXPECT_EQ(id, static_cast<uint32_t>(i));
    }
}

TEST(ECS, Entity_CountTracking)
{
    Wheel::Engine::Scene scene;
    EXPECT_EQ(scene.GetNumberOfEntities(), 0u);
    for (int i = 1; i <= 5; ++i) {
        scene.AddEntity();
        EXPECT_EQ(scene.GetNumberOfEntities(), static_cast<size_t>(i));
    }
}

TEST(ECS, Entity_TooManyEntitiesAsserts)
{
    Wheel::Engine::Scene scene;
    for (int i = 0; i < MAX_ENTITIES; ++i)
        scene.AddEntity();
    EXPECT_DEATH(scene.AddEntity(), "Too many entities in existence.");
}

// ==================== Entity Removal ====================

TEST(ECS, Entity_RemovalDecreasesCount)
{
    Wheel::Engine::Scene scene;
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    uint32_t e2 = scene.AddEntity();

    EXPECT_EQ(scene.GetNumberOfEntities(), 3u);
    scene.RemoveEntity(e1);
    EXPECT_EQ(scene.GetNumberOfEntities(), 2u);
    scene.RemoveEntity(e0);
    EXPECT_EQ(scene.GetNumberOfEntities(), 1u);
    scene.RemoveEntity(e2);
    EXPECT_EQ(scene.GetNumberOfEntities(), 0u);
}

TEST(ECS, Entity_RemovalOutOfRangeAsserts)
{
    Wheel::Engine::Scene scene;
    EXPECT_DEATH(scene.RemoveEntity(MAX_ENTITIES + 1), "Entity ID out of range.");
}

// ==================== Component Registration ====================

TEST(ECS, ComponentReg_SingleType)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    EXPECT_EQ(scene.GetNumberOfComponentTypes(), 1);
}

TEST(ECS, ComponentReg_MultipleTypes)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();
    scene.RegisterComponentType<D>();
    scene.RegisterComponentType<E>();
    EXPECT_EQ(scene.GetNumberOfComponentTypes(), 5);
}

TEST(ECS, ComponentReg_DuplicateAsserts)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    EXPECT_DEATH(scene.RegisterComponentType<A>(), "Component type already registered.");
}

// ==================== Component Add / Has ====================

TEST(ECS, Component_HasFalseBeforeAdd)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    EXPECT_FALSE(scene.HasComponent<A>(e));
}

TEST(ECS, Component_HasTrueAfterAdd)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    EXPECT_TRUE(scene.HasComponent<A>(e));
}

TEST(ECS, Component_HasFalseAfterRemove)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    scene.RemoveComponent<A>(e);
    EXPECT_FALSE(scene.HasComponent<A>(e));
}

TEST(ECS, Component_AddDuplicateAsserts)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    EXPECT_DEATH(scene.AddComponent<A>(e), "Entity already has component.");
}

TEST(ECS, Component_RemoveMissingAsserts)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    EXPECT_DEATH(scene.RemoveComponent<A>(e), "Entity does not have component.");
}

// ==================== Component Data via GetComponent ====================

TEST(ECS, Component_DefaultValues)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    scene.AddComponent<B>(e);

    EXPECT_EQ(scene.GetComponent<A>(e).value, 0);
    EXPECT_FLOAT_EQ(scene.GetComponent<B>(e).x, 0.0f);
    EXPECT_FLOAT_EQ(scene.GetComponent<B>(e).y, 0.0f);
}

TEST(ECS, Component_ModifyViaGetComponent)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);

    scene.GetComponent<A>(e).value = 42;
    EXPECT_EQ(scene.GetComponent<A>(e).value, 42);
}

TEST(ECS, Component_ModifyPersistsAcrossMultipleCalls)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);

    scene.GetComponent<A>(e).value = 100;
    scene.GetComponent<A>(e).value += 23;
    EXPECT_EQ(scene.GetComponent<A>(e).value, 123);
}

TEST(ECS, Component_MultipleEntitiesIndependentData)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    uint32_t e2 = scene.AddEntity();
    scene.AddComponent<A>(e0);
    scene.AddComponent<A>(e1);
    scene.AddComponent<A>(e2);

    scene.GetComponent<A>(e0).value = 1;
    scene.GetComponent<A>(e1).value = 2;
    scene.GetComponent<A>(e2).value = 3;

    EXPECT_EQ(scene.GetComponent<A>(e0).value, 1);
    EXPECT_EQ(scene.GetComponent<A>(e1).value, 2);
    EXPECT_EQ(scene.GetComponent<A>(e2).value, 3);
}

TEST(ECS, Component_ReAddAfterRemove_ResetsToDefault)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    scene.GetComponent<A>(e).value = 99;

    scene.RemoveComponent<A>(e);
    EXPECT_FALSE(scene.HasComponent<A>(e));

    scene.AddComponent<A>(e);
    EXPECT_TRUE(scene.HasComponent<A>(e));
    EXPECT_EQ(scene.GetComponent<A>(e).value, 0);  // fresh default
}

// ==================== Multiple Component Types Per Entity ====================

TEST(ECS, Component_MultipleTypesOnOneEntity)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    scene.AddComponent<B>(e);
    scene.AddComponent<C>(e);

    EXPECT_TRUE(scene.HasComponent<A>(e));
    EXPECT_TRUE(scene.HasComponent<B>(e));
    EXPECT_TRUE(scene.HasComponent<C>(e));

    scene.GetComponent<A>(e).value = 7;
    scene.GetComponent<B>(e).x = 1.5f;
    scene.GetComponent<C>(e).flag = true;

    EXPECT_EQ(scene.GetComponent<A>(e).value, 7);
    EXPECT_FLOAT_EQ(scene.GetComponent<B>(e).x, 1.5f);
    EXPECT_TRUE(scene.GetComponent<C>(e).flag);
}

TEST(ECS, Component_DifferentEntitiesHaveDifferentSubsets)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();

    uint32_t e0 = scene.AddEntity();   // A only
    uint32_t e1 = scene.AddEntity();   // A + B
    uint32_t e2 = scene.AddEntity();   // A + B + C

    scene.AddComponent<A>(e0);

    scene.AddComponent<A>(e1);
    scene.AddComponent<B>(e1);

    scene.AddComponent<A>(e2);
    scene.AddComponent<B>(e2);
    scene.AddComponent<C>(e2);

    EXPECT_TRUE (scene.HasComponent<A>(e0));
    EXPECT_FALSE(scene.HasComponent<B>(e0));
    EXPECT_FALSE(scene.HasComponent<C>(e0));

    EXPECT_TRUE (scene.HasComponent<A>(e1));
    EXPECT_TRUE (scene.HasComponent<B>(e1));
    EXPECT_FALSE(scene.HasComponent<C>(e1));

    EXPECT_TRUE(scene.HasComponent<A>(e2));
    EXPECT_TRUE(scene.HasComponent<B>(e2));
    EXPECT_TRUE(scene.HasComponent<C>(e2));
}

TEST(ECS, Component_RemoveOneTypePreservesOthers)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    scene.AddComponent<B>(e);
    scene.AddComponent<C>(e);

    scene.RemoveComponent<B>(e);

    EXPECT_TRUE (scene.HasComponent<A>(e));
    EXPECT_FALSE(scene.HasComponent<B>(e));
    EXPECT_TRUE (scene.HasComponent<C>(e));
}

// ==================== Component Size Tracking ====================

TEST(ECS, ComponentSize_IncreasesOnAdd)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    EXPECT_EQ(scene.GetComponents<A>().size(), 0u);

    for (int i = 0; i < 5; ++i) {
        uint32_t e = scene.AddEntity();
        scene.AddComponent<A>(e);
        EXPECT_EQ(scene.GetComponents<A>().size(), static_cast<size_t>(i + 1));
    }
}

TEST(ECS, ComponentSize_DecreasesOnRemove)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    uint32_t e2 = scene.AddEntity();
    scene.AddComponent<A>(e0);
    scene.AddComponent<A>(e1);
    scene.AddComponent<A>(e2);

    EXPECT_EQ(scene.GetComponents<A>().size(), 3u);
    scene.RemoveComponent<A>(e1);
    EXPECT_EQ(scene.GetComponents<A>().size(), 2u);
    scene.RemoveComponent<A>(e0);
    EXPECT_EQ(scene.GetComponents<A>().size(), 1u);
    scene.RemoveComponent<A>(e2);
    EXPECT_EQ(scene.GetComponents<A>().size(), 0u);
}

TEST(ECS, ComponentSize_UnregisteredComponentIsZero)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();

    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);

    EXPECT_EQ(scene.GetComponents<A>().size(), 1u);
    EXPECT_EQ(scene.GetComponents<B>().size(), 0u);
}

// ==================== Component Pool Integrity After Removal ====================

TEST(ECS, ComponentPool_RemoveFirst_OthersIntact)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    uint32_t e2 = scene.AddEntity();
    scene.AddComponent<A>(e0);
    scene.AddComponent<A>(e1);
    scene.AddComponent<A>(e2);
    scene.GetComponent<A>(e0).value = 10;
    scene.GetComponent<A>(e1).value = 20;
    scene.GetComponent<A>(e2).value = 30;

    scene.RemoveComponent<A>(e0);

    EXPECT_EQ(scene.GetComponents<A>().size(), 2u);
    EXPECT_FALSE(scene.HasComponent<A>(e0));
    EXPECT_EQ(scene.GetComponent<A>(e1).value, 20);
    EXPECT_EQ(scene.GetComponent<A>(e2).value, 30);
}

TEST(ECS, ComponentPool_RemoveMiddle_OthersIntact)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    uint32_t e2 = scene.AddEntity();
    scene.AddComponent<A>(e0);
    scene.AddComponent<A>(e1);
    scene.AddComponent<A>(e2);
    scene.GetComponent<A>(e0).value = 10;
    scene.GetComponent<A>(e1).value = 20;
    scene.GetComponent<A>(e2).value = 30;

    scene.RemoveComponent<A>(e1);

    EXPECT_EQ(scene.GetComponents<A>().size(), 2u);
    EXPECT_FALSE(scene.HasComponent<A>(e1));
    EXPECT_EQ(scene.GetComponent<A>(e0).value, 10);
    EXPECT_EQ(scene.GetComponent<A>(e2).value, 30);
}

TEST(ECS, ComponentPool_RemoveLast_OthersIntact)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    uint32_t e2 = scene.AddEntity();
    scene.AddComponent<A>(e0);
    scene.AddComponent<A>(e1);
    scene.AddComponent<A>(e2);
    scene.GetComponent<A>(e0).value = 10;
    scene.GetComponent<A>(e1).value = 20;
    scene.GetComponent<A>(e2).value = 30;

    scene.RemoveComponent<A>(e2);

    EXPECT_EQ(scene.GetComponents<A>().size(), 2u);
    EXPECT_FALSE(scene.HasComponent<A>(e2));
    EXPECT_EQ(scene.GetComponent<A>(e0).value, 10);
    EXPECT_EQ(scene.GetComponent<A>(e1).value, 20);
}

TEST(ECS, ComponentPool_RemoveOnly_Succeeds)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    scene.GetComponent<A>(e).value = 42;

    scene.RemoveComponent<A>(e);
    EXPECT_EQ(scene.GetComponents<A>().size(), 0u);
    EXPECT_FALSE(scene.HasComponent<A>(e));
}

TEST(ECS, ComponentPool_LargeScale_RemoveEveryOther)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();

    std::vector<uint32_t> entities;
    for (int i = 0; i < 100; ++i) {
        uint32_t e = scene.AddEntity();
        scene.AddComponent<A>(e);
        scene.GetComponent<A>(e).value = i;
        entities.push_back(e);
    }
    EXPECT_EQ(scene.GetComponents<A>().size(), 100u);

    // Remove even-indexed entities
    for (int i = 0; i < 100; i += 2)
        scene.RemoveComponent<A>(entities[i]);

    EXPECT_EQ(scene.GetComponents<A>().size(), 50u);

    // Odd-indexed entities should still have their correct data
    for (int i = 1; i < 100; i += 2) {
        EXPECT_TRUE(scene.HasComponent<A>(entities[i]));
        EXPECT_EQ(scene.GetComponent<A>(entities[i]).value, i);
    }
    // Even-indexed entities should no longer have the component
    for (int i = 0; i < 100; i += 2) {
        EXPECT_FALSE(scene.HasComponent<A>(entities[i]));
    }
}

// ==================== Entity Destruction → Component Cleanup ====================

TEST(ECS, EntityDestroy_CleansUpSingleComponent)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);

    scene.RemoveEntity(e);
    EXPECT_EQ(scene.GetComponents<A>().size(), 0u);
    EXPECT_EQ(scene.GetNumberOfEntities(), 0u);
}

TEST(ECS, EntityDestroy_CleansUpAllComponents)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();
    uint32_t e = scene.AddEntity();
    scene.AddComponent<A>(e);
    scene.AddComponent<B>(e);
    scene.AddComponent<C>(e);

    scene.RemoveEntity(e);

    EXPECT_EQ(scene.GetComponents<A>().size(), 0u);
    EXPECT_EQ(scene.GetComponents<B>().size(), 0u);
    EXPECT_EQ(scene.GetComponents<C>().size(), 0u);
}

TEST(ECS, EntityDestroy_EntityWithNoComponentsDoesNotCrash)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    scene.AddComponent<A>(e0);

    scene.RemoveEntity(e1);  // e1 has no components

    EXPECT_EQ(scene.GetComponents<A>().size(), 1u);
    EXPECT_EQ(scene.GetNumberOfEntities(), 1u);
}

TEST(ECS, EntityDestroy_MiddleEntity_OthersPreserved)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    uint32_t e2 = scene.AddEntity();
    scene.AddComponent<A>(e0);
    scene.AddComponent<A>(e1);
    scene.AddComponent<A>(e2);
    scene.AddComponent<B>(e0);
    scene.AddComponent<B>(e1);
    scene.GetComponent<A>(e0).value = 1;
    scene.GetComponent<A>(e2).value = 3;

    scene.RemoveEntity(e1);

    EXPECT_EQ(scene.GetComponents<A>().size(), 2u);
    EXPECT_EQ(scene.GetComponents<B>().size(), 1u);
    EXPECT_EQ(scene.GetNumberOfEntities(), 2u);
    EXPECT_EQ(scene.GetComponent<A>(e0).value, 1);
    EXPECT_EQ(scene.GetComponent<A>(e2).value, 3);
    EXPECT_TRUE (scene.HasComponent<B>(e0));
    EXPECT_FALSE(scene.HasComponent<A>(e1));
    EXPECT_FALSE(scene.HasComponent<B>(e1));
}

TEST(ECS, EntityDestroy_AllEntities_AllComponentsGone)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();

    std::vector<uint32_t> entities;
    for (int i = 0; i < 10; ++i) {
        uint32_t e = scene.AddEntity();
        scene.AddComponent<A>(e);
        entities.push_back(e);
    }
    EXPECT_EQ(scene.GetComponents<A>().size(), 10u);

    for (uint32_t e : entities)
        scene.RemoveEntity(e);

    EXPECT_EQ(scene.GetComponents<A>().size(), 0u);
    EXPECT_EQ(scene.GetNumberOfEntities(), 0u);
}

// ==================== Entity ID Reuse ====================

TEST(ECS, EntityReuse_RecycledIDHasNoComponents)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();

    uint32_t e0 = scene.AddEntity();
    scene.AddComponent<A>(e0);
    scene.AddComponent<B>(e0);
    scene.GetComponent<A>(e0).value = 77;

    scene.RemoveEntity(e0);

    EXPECT_FALSE(scene.HasComponent<A>(e0));
    EXPECT_FALSE(scene.HasComponent<B>(e0));

    // Can add fresh components without issue
    scene.AddComponent<A>(e0);
    EXPECT_EQ(scene.GetComponent<A>(e0).value, 0);
}

// ==================== Description Tests ====================

TEST(ECS, Description_EmptyByDefault)
{
    Wheel::Engine::Description desc;
    EXPECT_TRUE(desc.IsEmpty());
}

TEST(ECS, Description_IsNotEmptyAfterAdd)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> bits;
    bits[3] = true;
    desc.AddComponentType(bits);
    EXPECT_FALSE(desc.IsEmpty());
}

TEST(ECS, Description_AddSingleBitSetsCorrectSlot)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> bits;
    bits[5] = true;
    desc.AddComponentType(bits);

    EXPECT_TRUE (desc.GetAsBitset()[5]);
    EXPECT_FALSE(desc.GetAsBitset()[0]);
    EXPECT_FALSE(desc.GetAsBitset()[4]);
}

TEST(ECS, Description_AddMultipleBitsIndependently)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> b0; b0[0] = true;
    std::bitset<MAX_COMPONENT_TYPES> b7; b7[7] = true;
    desc.AddComponentType(b0);
    desc.AddComponentType(b7);

    EXPECT_TRUE (desc.GetAsBitset()[0]);
    EXPECT_TRUE (desc.GetAsBitset()[7]);
    EXPECT_FALSE(desc.GetAsBitset()[1]);
}

TEST(ECS, Description_RemoveComponentType_ClearsBit)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> bits;
    bits[2] = true;
    desc.AddComponentType(bits);

    desc.RemoveComponentType(bits);
    EXPECT_FALSE(desc.GetAsBitset()[2]);
    EXPECT_TRUE (desc.IsEmpty());
}

TEST(ECS, Description_RemoveOneOfManyPreservesOthers)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> b0; b0[0] = true;
    std::bitset<MAX_COMPONENT_TYPES> b1; b1[1] = true;
    std::bitset<MAX_COMPONENT_TYPES> b2; b2[2] = true;
    desc.AddComponentType(b0);
    desc.AddComponentType(b1);
    desc.AddComponentType(b2);

    desc.RemoveComponentType(b1);
    EXPECT_TRUE (desc.GetAsBitset()[0]);
    EXPECT_FALSE(desc.GetAsBitset()[1]);
    EXPECT_TRUE (desc.GetAsBitset()[2]);
}

TEST(ECS, Description_HasComponentType_Present)
{
    Wheel::Engine::Description entity_desc;
    Wheel::Engine::Description comp_desc;
    std::bitset<MAX_COMPONENT_TYPES> bits;
    bits[4] = true;
    entity_desc.AddComponentType(bits);
    comp_desc.AddComponentType(bits);

    EXPECT_TRUE(entity_desc.HasComponentType(comp_desc));
}

TEST(ECS, Description_HasComponentType_Absent)
{
    Wheel::Engine::Description entity_desc;
    Wheel::Engine::Description comp_desc;
    std::bitset<MAX_COMPONENT_TYPES> b0; b0[0] = true;
    std::bitset<MAX_COMPONENT_TYPES> b1; b1[1] = true;
    entity_desc.AddComponentType(b0);
    comp_desc.AddComponentType(b1);

    EXPECT_FALSE(entity_desc.HasComponentType(comp_desc));
}

TEST(ECS, Description_HasComponentType_EmptyQueryAsserts)
{
    Wheel::Engine::Description entity_desc;
    std::bitset<MAX_COMPONENT_TYPES> bits;
    bits[0] = true;
    entity_desc.AddComponentType(bits);

    Wheel::Engine::Description empty;
    EXPECT_DEATH(entity_desc.HasComponentType(empty), "Component type out of range");
}

TEST(ECS, Description_Reset_ClearsAllBits)
{
    Wheel::Engine::Description desc;
    std::bitset<MAX_COMPONENT_TYPES> b0; b0[0] = true;
    std::bitset<MAX_COMPONENT_TYPES> b1; b1[1] = true;
    std::bitset<MAX_COMPONENT_TYPES> b2; b2[2] = true;
    desc.AddComponentType(b0);
    desc.AddComponentType(b1);
    desc.AddComponentType(b2);
    EXPECT_FALSE(desc.IsEmpty());

    desc.Reset();
    EXPECT_TRUE(desc.IsEmpty());
}

TEST(ECS, Description_AssignmentOperatorCopiesBits)
{
    Wheel::Engine::Description src;
    std::bitset<MAX_COMPONENT_TYPES> b0; b0[0] = true;
    std::bitset<MAX_COMPONENT_TYPES> b6; b6[6] = true;
    src.AddComponentType(b0);
    src.AddComponentType(b6);

    Wheel::Engine::Description dst;
    dst = src;
    EXPECT_TRUE (dst.GetAsBitset()[0]);
    EXPECT_TRUE (dst.GetAsBitset()[6]);
    EXPECT_FALSE(dst.GetAsBitset()[1]);
}

TEST(ECS, Description_MoveConstructorCopiesBits)
{
    Wheel::Engine::Description src;
    std::bitset<MAX_COMPONENT_TYPES> bits;
    bits[9] = true;
    src.AddComponentType(bits);

    Wheel::Engine::Description dst(std::move(src));
    EXPECT_TRUE(dst.GetAsBitset()[9]);
}

// ==================== System Update Simulation ====================

// Simulates a system by iterating GetComponents and mutating via GetComponent.
// Avoids relying on AddComponent's (potentially dangling) return value.

TEST(ECS, System_UpdateAll_CountersReachExpected)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<Counter>();

    std::vector<uint32_t> entities;
    for (int i = 0; i < 20; ++i) {
        uint32_t e = scene.AddEntity();
        scene.AddComponent<Counter>(e);
        entities.push_back(e);
    }

    const int TICKS = 50;
    for (int tick = 0; tick < TICKS; ++tick) {
        auto comps = scene.GetComponents<Counter>();
        for (auto& [id, _] : comps)
            scene.GetComponent<Counter>(id).count++;
    }

    for (uint32_t e : entities)
        EXPECT_EQ(scene.GetComponent<Counter>(e).count, TICKS);
}

TEST(ECS, System_UpdatePartial_OnlyMatchingEntitiesUpdated)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<Counter>();
    scene.RegisterComponentType<A>();

    std::vector<uint32_t> counter_entities;
    std::vector<uint32_t> other_entities;

    for (int i = 0; i < 15; ++i) {
        uint32_t e = scene.AddEntity();
        scene.AddComponent<Counter>(e);
        counter_entities.push_back(e);
    }
    for (int i = 0; i < 15; ++i) {
        uint32_t e = scene.AddEntity();
        scene.AddComponent<A>(e);
        other_entities.push_back(e);
    }

    for (int tick = 0; tick < 10; ++tick) {
        auto comps = scene.GetComponents<Counter>();
        for (auto& [id, _] : comps)
            scene.GetComponent<Counter>(id).count++;
    }

    for (uint32_t e : counter_entities)
        EXPECT_EQ(scene.GetComponent<Counter>(e).count, 10);

    for (uint32_t e : other_entities) {
        EXPECT_FALSE(scene.HasComponent<Counter>(e));
        EXPECT_TRUE(scene.HasComponent<A>(e));
    }
}

TEST(ECS, System_UpdateWithEntityRemovedMidSimulation)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<Counter>();
    uint32_t e0 = scene.AddEntity();
    uint32_t e1 = scene.AddEntity();
    uint32_t e2 = scene.AddEntity();
    scene.AddComponent<Counter>(e0);
    scene.AddComponent<Counter>(e1);
    scene.AddComponent<Counter>(e2);

    // 5 ticks before removal
    for (int tick = 0; tick < 5; ++tick) {
        auto comps = scene.GetComponents<Counter>();
        for (auto& [id, _] : comps)
            scene.GetComponent<Counter>(id).count++;
    }
    EXPECT_EQ(scene.GetComponent<Counter>(e0).count, 5);
    EXPECT_EQ(scene.GetComponent<Counter>(e1).count, 5);
    EXPECT_EQ(scene.GetComponent<Counter>(e2).count, 5);

    scene.RemoveEntity(e1);
    EXPECT_EQ(scene.GetComponents<Counter>().size(), 2u);

    // 5 more ticks after removal
    for (int tick = 0; tick < 5; ++tick) {
        auto comps = scene.GetComponents<Counter>();
        for (auto& [id, _] : comps)
            scene.GetComponent<Counter>(id).count++;
    }

    EXPECT_EQ(scene.GetComponent<Counter>(e0).count, 10);
    EXPECT_EQ(scene.GetComponent<Counter>(e2).count, 10);
    EXPECT_EQ(scene.GetComponents<Counter>().size(), 2u);
}

TEST(ECS, System_UpdateWithEntityAddedMidSimulation)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<Counter>();
    uint32_t e0 = scene.AddEntity();
    scene.AddComponent<Counter>(e0);

    // 5 ticks with only e0
    for (int tick = 0; tick < 5; ++tick) {
        auto comps = scene.GetComponents<Counter>();
        for (auto& [id, _] : comps)
            scene.GetComponent<Counter>(id).count++;
    }
    EXPECT_EQ(scene.GetComponent<Counter>(e0).count, 5);

    // Add e1 mid-simulation
    uint32_t e1 = scene.AddEntity();
    scene.AddComponent<Counter>(e1);
    EXPECT_EQ(scene.GetComponents<Counter>().size(), 2u);

    // 5 more ticks
    for (int tick = 0; tick < 5; ++tick) {
        auto comps = scene.GetComponents<Counter>();
        for (auto& [id, _] : comps)
            scene.GetComponent<Counter>(id).count++;
    }

    EXPECT_EQ(scene.GetComponent<Counter>(e0).count, 10);
    EXPECT_EQ(scene.GetComponent<Counter>(e1).count, 5);  // only updated for last 5 ticks
}

// ==================== Mixed Scenarios ====================

TEST(ECS, Mixed_ManyEntitiesVariedComponents)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<A>();
    scene.RegisterComponentType<B>();
    scene.RegisterComponentType<C>();
    scene.RegisterComponentType<D>();
    scene.RegisterComponentType<E>();

    std::vector<uint32_t> entities;
    for (int i = 0; i < 100; ++i)
        entities.push_back(scene.AddEntity());

    // A: first 10
    for (int i = 0; i < 10; ++i)   scene.AddComponent<A>(entities[i]);
    // B: 10-34
    for (int i = 10; i < 35; ++i)  scene.AddComponent<B>(entities[i]);
    // C: 35-59
    for (int i = 35; i < 60; ++i)  scene.AddComponent<C>(entities[i]);
    // D: 60-74
    for (int i = 60; i < 75; ++i)  scene.AddComponent<D>(entities[i]);
    // E: none

    EXPECT_EQ(scene.GetComponents<A>().size(), 10u);
    EXPECT_EQ(scene.GetComponents<B>().size(), 25u);
    EXPECT_EQ(scene.GetComponents<C>().size(), 25u);
    EXPECT_EQ(scene.GetComponents<D>().size(), 15u);
    EXPECT_EQ(scene.GetComponents<E>().size(),  0u);
}

TEST(ECS, Mixed_StressCreateDestroyWithComponents)
{
    Wheel::Engine::Scene scene;
    scene.RegisterComponentType<Counter>();

    // Create 50, destroy odd ones, create 50 more, verify counts
    std::vector<uint32_t> batch1;
    for (int i = 0; i < 50; ++i) {
        uint32_t e = scene.AddEntity();
        scene.AddComponent<Counter>(e);
        scene.GetComponent<Counter>(e).count = i;
        batch1.push_back(e);
    }
    EXPECT_EQ(scene.GetNumberOfEntities(), 50u);
    EXPECT_EQ(scene.GetComponents<Counter>().size(), 50u);

    for (int i = 1; i < 50; i += 2)
        scene.RemoveEntity(batch1[i]);

    EXPECT_EQ(scene.GetNumberOfEntities(), 25u);
    EXPECT_EQ(scene.GetComponents<Counter>().size(), 25u);

    std::vector<uint32_t> batch2;
    for (int i = 0; i < 25; ++i) {
        uint32_t e = scene.AddEntity();
        scene.AddComponent<Counter>(e);
        batch2.push_back(e);
    }
    EXPECT_EQ(scene.GetNumberOfEntities(), 50u);
    EXPECT_EQ(scene.GetComponents<Counter>().size(), 50u);

    // Even-indexed batch1 entities still have their original counts
    for (int i = 0; i < 50; i += 2)
        EXPECT_EQ(scene.GetComponent<Counter>(batch1[i]).count, i);
}