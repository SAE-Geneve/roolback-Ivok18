#pragma once
#include <SFML/System/Time.hpp>


#include "game_globals.h"

namespace game
{
/**
* \brief Boundary is a struct that contains info about the boundary (when it will be destroyed)
*/
struct Boundary
{
	core::Vec2f position;
};
class GameManager;

/**
* \brief BallManager is a ComponentManager that holds all the balls in one place.
* It will automatically destroy the ball when remainingTime is over.
*/
class BoundaryManager : public core::ComponentManager<Boundary, static_cast<core::EntityMask>(ComponentType::BOUNDARY)>
{
public:
	explicit BoundaryManager(core::EntityManager& entityManager, GameManager& gameManager);
	void FixedUpdate(sf::Time dt);
private:
	GameManager& gameManager_;
};
}