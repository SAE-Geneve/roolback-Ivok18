#include "game/ball_manager.h"
#include "game/game_manager.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif
namespace game
{
BallManager::BallManager(core::EntityManager& entityManager, GameManager& gameManager) :
    ComponentManager(entityManager), gameManager_(gameManager)
{
}

void BallManager::FixedUpdate(sf::Time dt)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if(entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
        {
            continue;
        }
        /*if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::BALL)))
        {
            auto& bullet = components_[entity];
            bullet.remainingTime -= dt.asSeconds();
            if (bullet.remainingTime < 0.0f)
            {
                gameManager_.DestroyBall(entity);
            }
        }
        */
    }
}
}
