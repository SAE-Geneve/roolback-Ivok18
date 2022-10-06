/**
 * \file entity.h
 */
#pragma once

#include <cstdint>
#include <vector>
#include <limits>


namespace core
{
/**
 * \brief Entity is the type used to define an game world entity.
 * An Entity is just an index, it means that if you have access to a ComponentManager, you can get the Component by giving the index of the Entity.
 */
using Entity = std::uint32_t;
/**
 * \brief EntityMask is the type used to define the bitwise mask of an Entity.
 * It is used to know what Component an Entity has.
 */
using EntityMask = std::uint32_t;
/**
 * \brief INVALID_ENTITY is a constant that define an invalid Entity.
 */
constexpr Entity INVALID_ENTITY = std::numeric_limits<Entity>::max();
/**
 * \brief INVALID_ENTITY_MASK is a constant that define an invalid or empty entity mask.
 */
constexpr EntityMask INVALID_ENTITY_MASK = 0u;
/**
 * \brief Manages the entities in an array using bitwise operations to know if it has components.
 */
class EntityManager
{
public:
    EntityManager();
    EntityManager(std::size_t reservedSize);
    /**
     * \brief CreateEntity is a method that will return the next available Entity index.
     * It will look at the internal entityMasks_ array and give the first one that is free to use.
     * If none are free, the array is reallocated.
     * \return the newly created Entity
     */
    Entity CreateEntity();
    /**
     * \brief DestroyEntity is a method that will erase all Component from the EntityMask.
     * It means that EntityExists will be false and that HasComponent will always return false.
     * It will not do anything to the actual ComponentManager.
     * \param entity is the mask that will be voided
     */
    void DestroyEntity(Entity entity);
    /**
     * \brief AddComponent is a method that adds the bitwise entity mask to the entity mask.
     * It is normally called by the ComponentManager. 
     * \param entity is the entity to add the new EntityMask
     * \param mask is the Component bitwise mask to be added to the Entity
     */
    void AddComponent(Entity entity, EntityMask mask);
    /**
     * \brief RemoveComponent is a method that removes the bitwise entity mask to the entity mask.
     * \param entity is the entity to remove the EntityMask
     * \param mask is the Component bitwise mask to be removed to the Entity
     */
    void RemoveComponent(Entity entity, EntityMask mask);
    /**
     * \brief HasComponent is a method that check if a certain Entity has a EntityMask on.
     * \param entity is the Entity that we check
     * \param mask is the Component bitwise mask to check.
     * \return the statement result if an Entity has a certain bitwise mask on.
     */
    [[nodiscard]] bool HasComponent(Entity entity, EntityMask mask) const;
    /**
     * \brief EntityExists is a method that check if a certain Entity has any Component and thus do exist.
     * \param entity is the Entity that we check
     * \return the statement result if an Entity exists
     */
    [[nodiscard]] bool EntityExists(Entity entity) const;
    /**
     * \brief GetEntitiesSize is a method that returns the size of the EntityMask array.
     * \return the total size of the EntityMask array.
     */
    [[nodiscard]] std::size_t GetEntitiesSize() const;


private:
    std::vector<EntityMask> entityMasks_;
};

} // namespace core
