#pragma once
#include "game_globals.h"
#include "engine/component.h"
#include "engine/entity.h"
#include "maths/angle.h"
#include "maths/vec2.h"

#include <SFML/System/Time.hpp>

#include "graphics/graphics.h"
#include "utils/action_utility.h"

namespace core
{
class TransformManager;
}

namespace game
{
enum class BodyType
{
    DYNAMIC,
    STATIC
};

/**
 * \brief Body is a class that represents a rigid body.
 */
struct Body
{
    core::Vec2f position = core::Vec2f::zero();
    core::Vec2f velocity = core::Vec2f::zero();
    core::Degree angularVelocity = core::Degree(0.0f);
    core::Degree rotation = core::Degree(0.0f);
    BodyType bodyType = BodyType::DYNAMIC;
};

/**
 * \brief Box is a class that represents an axis-aligned box collider
 */
struct Box
{
    core::Vec2f extends = core::Vec2f::one();
    bool isTrigger = false;
};

/**
 * \brief OnTriggerInterface is an interface for classes that needs to be called when two boxes are in contact.
 * It needs to be registered in the PhysicsManager.
 */
class OnTriggerInterface
{
public:
    virtual ~OnTriggerInterface() = default;
    virtual void OnTrigger(core::Entity entity1, core::Entity entity2) = 0;
};

/**
 * \brief BodyManager is a ComponentManager that holds all the Body in the world.
 */
class BodyManager : public core::ComponentManager<Body, static_cast<core::EntityMask>(core::ComponentType::BODY2D)>
{
public:
    using ComponentManager::ComponentManager;
};

/**
 * \brief BoxManager is a ComponentManager that holds all the Box in the world.
 */
class BoxManager : public core::ComponentManager<Box, static_cast<core::EntityMask>(core::ComponentType::BOX_COLLIDER2D)>
{
public:
    using ComponentManager::ComponentManager;
};

/**
 * \brief PhysicsManager is a class that holds both BodyManager and BoxManager and manages the physics fixed update.
 * It allows to register OnTriggerInterface to be called when a trigger occcurs.
 */
class PhysicsManager : public core::DrawInterface
{
public:
    explicit PhysicsManager(core::EntityManager& entityManager);
    void FixedUpdate(sf::Time dt);
    [[nodiscard]] const Body& GetBody(core::Entity entity) const;
    void SetBody(core::Entity entity, const Body& body);
    void AddBody(core::Entity entity);

    void AddBox(core::Entity entity);
    void SetBox(core::Entity entity, const Box& box);
    [[nodiscard]] const Box& GetBox(core::Entity entity) const;
    /**
     * \brief RegisterTriggerListener is a method that stores an OnTriggerInterface in the PhysicsManager that will call the OnTrigger method in case of a trigger.
     * \param onTriggerInterface is the OnTriggerInterface to be called when a trigger occurs.
     */
    void RegisterTriggerListener(OnTriggerInterface& onTriggerInterface);
    void CopyAllComponents(const PhysicsManager& physicsManager);
    void Draw(sf::RenderTarget& renderTarget) override;
    void SetCenter(sf::Vector2f center) { center_ = center; }
    void SetWindowSize(sf::Vector2f newWindowSize) { windowSize_ = newWindowSize; }
private:
    core::EntityManager& entityManager_;
    BodyManager bodyManager_;
    BoxManager boxManager_;
    core::Action<core::Entity, core::Entity> onTriggerAction_;
    //Used for debug
    sf::Vector2f center_{};
    sf::Vector2f windowSize_{};
};

}
