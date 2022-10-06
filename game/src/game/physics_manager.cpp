#include "game/physics_manager.h"
#include "engine/transform.h"

#include <SFML/Graphics/RectangleShape.hpp>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{

PhysicsManager::PhysicsManager(core::EntityManager& entityManager) :
    entityManager_(entityManager), bodyManager_(entityManager), boxManager_(entityManager)
{

}

constexpr bool Box2Box(float r1x, float r1y, float r1w, float r1h, float r2x, float r2y, float r2w, float r2h)
{
    return r1x + r1w >= r2x &&    // r1 right edge past r2 left
        r1x <= r2x + r2w &&    // r1 left edge past r2 right
        r1y + r1h >= r2y &&    // r1 top edge past r2 bottom
        r1y <= r2y + r2h;
}

void PhysicsManager::FixedUpdate(sf::Time dt)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity, static_cast<core::EntityMask>(core::ComponentType::BODY2D)))
            continue;
        auto body = bodyManager_.GetComponent(entity);
        body.position += body.velocity * dt.asSeconds();
        body.rotation += body.angularVelocity * dt.asSeconds();
        bodyManager_.SetComponent(entity, body);
    }
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity,
            static_cast<core::EntityMask>(core::ComponentType::BODY2D) |
            static_cast<core::EntityMask>(core::ComponentType::BOX_COLLIDER2D)) ||
            entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
            continue;
        for (core::Entity otherEntity = entity + 1; otherEntity < entityManager_.GetEntitiesSize(); otherEntity++)
        {
            if (!entityManager_.HasComponent(otherEntity,
                static_cast<core::EntityMask>(core::ComponentType::BODY2D) | static_cast<core::EntityMask>(core::ComponentType::BOX_COLLIDER2D)) ||
                entityManager_.HasComponent(otherEntity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
                continue;
            const Body& body1 = bodyManager_.GetComponent(entity);
            const Box& box1 = boxManager_.GetComponent(entity);

            const Body& body2 = bodyManager_.GetComponent(otherEntity);
            const Box& box2 = boxManager_.GetComponent(otherEntity);

            if (Box2Box(
                body1.position.x - box1.extends.x,
                body1.position.y - box1.extends.y,
                box1.extends.x * 2.0f,
                box1.extends.y * 2.0f,
                body2.position.x - box2.extends.x,
                body2.position.y - box2.extends.y,
                box2.extends.x * 2.0f,
                box2.extends.y * 2.0f))
            {
                onTriggerAction_.Execute(entity, otherEntity);
            }

        }
    }
}

void PhysicsManager::SetBody(core::Entity entity, const Body& body)
{
    bodyManager_.SetComponent(entity, body);
}

const Body& PhysicsManager::GetBody(core::Entity entity) const
{
    return bodyManager_.GetComponent(entity);
}

void PhysicsManager::AddBody(core::Entity entity)
{
    bodyManager_.AddComponent(entity);
}

void PhysicsManager::AddBox(core::Entity entity)
{
    boxManager_.AddComponent(entity);
}

void PhysicsManager::SetBox(core::Entity entity, const Box& box)
{
    boxManager_.SetComponent(entity, box);
}

const Box& PhysicsManager::GetBox(core::Entity entity) const
{
    return boxManager_.GetComponent(entity);
}

void PhysicsManager::RegisterTriggerListener(OnTriggerInterface& onTriggerInterface)
{
    onTriggerAction_.RegisterCallback(
        [&onTriggerInterface](core::Entity entity1, core::Entity entity2) { onTriggerInterface.OnTrigger(entity1, entity2); });
}

void PhysicsManager::CopyAllComponents(const PhysicsManager& physicsManager)
{
    bodyManager_.CopyAllComponents(physicsManager.bodyManager_.GetAllComponents());
    boxManager_.CopyAllComponents(physicsManager.boxManager_.GetAllComponents());
}

void PhysicsManager::Draw(sf::RenderTarget& renderTarget)
{
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity,
            static_cast<core::EntityMask>(core::ComponentType::BODY2D) |
            static_cast<core::EntityMask>(core::ComponentType::BOX_COLLIDER2D)) ||
            entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
            continue;
        const auto& [extends, isTrigger] = boxManager_.GetComponent(entity);
        const auto& body = bodyManager_.GetComponent(entity);
        sf::RectangleShape rectShape;
        rectShape.setFillColor(core::Color::transparent());
        rectShape.setOutlineColor(core::Color::green());
        rectShape.setOutlineThickness(2.0f);
        const auto position = body.position;
        rectShape.setOrigin({ extends.x * core::pixelPerMeter, extends.y * core::pixelPerMeter });
        rectShape.setPosition(
            position.x * core::pixelPerMeter + center_.x,
            windowSize_.y - (position.y * core::pixelPerMeter + center_.y));
        rectShape.setSize({ extends.x * 2.0f * core::pixelPerMeter, extends.y * 2.0f * core::pixelPerMeter });
        renderTarget.draw(rectShape);
    }
}
}
