#include <cmath>
#include <engine/entity.h>
#include <gtest/gtest.h>

#include "engine/component.h"

constexpr core::EntityMask componentType = 2u;

class SimpleComponentManager : public core::ComponentManager<int, componentType>
{
    using ComponentManager::ComponentManager;
};

TEST(Component, AddComponent)
{
    core::EntityManager entityManager;
    SimpleComponentManager componentManager(entityManager);

    const auto entity = entityManager.CreateEntity();
    EXPECT_FALSE(entityManager.HasComponent(entity, componentType));
    componentManager.AddComponent(entity);
    EXPECT_TRUE(entityManager.HasComponent(entity, componentType));
    componentManager.RemoveComponent(entity);
    EXPECT_FALSE(entityManager.HasComponent(entity, componentType));

}

TEST(Component, GetComponent)
{
    constexpr int newValue = 45;
    core::EntityManager entityManager;
    SimpleComponentManager componentManager(entityManager);

    const auto entity = entityManager.CreateEntity();
    componentManager.AddComponent(entity);
    auto& mutableValue = componentManager.GetComponent(entity);
    mutableValue = newValue;
    const auto& immutableComponentManager = componentManager;
    const auto& immutableValue = immutableComponentManager.GetComponent(entity);
    EXPECT_EQ(immutableValue, mutableValue);

}

TEST(Component, CopyAllComponents)
{
    constexpr int oldValue1 = 45;
    constexpr int newValue1 = 43;
    constexpr int newValue2 = 47;
    core::EntityManager entityManager;
    SimpleComponentManager oldComponentManager(entityManager);
    SimpleComponentManager newComponentManager(entityManager);

    const auto entity1 = entityManager.CreateEntity();
    const auto entity2 = entityManager.CreateEntity();
    oldComponentManager.AddComponent(entity1);
    oldComponentManager.SetComponent(entity1, oldValue1);
    newComponentManager.AddComponent(entity1);
    newComponentManager.SetComponent(entity1, newValue1);
    newComponentManager.AddComponent(entity2);
    newComponentManager.SetComponent(entity2, newValue2);

    oldComponentManager.CopyAllComponents(newComponentManager.GetAllComponents());
    EXPECT_EQ(oldComponentManager.GetComponent(entity1), newValue1);
    EXPECT_EQ(oldComponentManager.GetComponent(entity2), newValue2);

}

TEST(Component, InternalArrayOverflow)
{
    core::EntityManager entityManager;
    SimpleComponentManager componentManager(entityManager);

    for (std::size_t i = 0; i < core::entityInitNmb; i++)
    {
        entityManager.CreateEntity();
    }
    const auto entity = entityManager.CreateEntity();
    componentManager.AddComponent(entity);
    EXPECT_LT(core::entityInitNmb, componentManager.GetAllComponents().size());
}