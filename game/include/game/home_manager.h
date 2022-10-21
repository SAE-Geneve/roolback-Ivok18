#pragma once
#include <SFML/System/Time.hpp>
#include "game_globals.h"

namespace game
{
/**
* \brief Home is a struct that contains info about the a player home
*/
struct Home
{
	PlayerNumber playerNumber = INVALID_PLAYER;
	core::Vec2f position;
};
class GameManager;

/**
* \brief HomeManager is a ComponentManager that holds all the player homes in one place.
*/
class HomeManager : public core::ComponentManager<Home, static_cast<core::EntityMask>(ComponentType::HOME)>
{
public:
	explicit HomeManager(core::EntityManager& entityManager, GameManager& gameManager);
	void FixedUpdate(sf::Time dt);
private:
	GameManager& gameManager_;
};
}