#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"

namespace game
{

/**
 * \brief Ball is a struct that contains info about the ball (when it will be destroyed)
 */
struct Ball
{
    float remainingTime = 0.0f;
    PlayerNumber playerNumber = 1;
};


class GameManager;

/**
 * \brief BallManager is a ComponentManager that holds all the balls in one place.
 * It will automatically destroy the ball when remainingTime is over.
 */
class BallManager : public core::ComponentManager<Ball, static_cast<core::EntityMask>(ComponentType::BALL)>
{
public:
    explicit BallManager(core::EntityManager& entityManager, GameManager& gameManager);
    void FixedUpdate(sf::Time dt);
private:
    GameManager& gameManager_;
};
}
