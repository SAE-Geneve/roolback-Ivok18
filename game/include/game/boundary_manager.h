#pragma once
#include <SFML/System/Time.hpp>
#include "game_globals.h"

namespace game
{
/**
* \brief Boundary is a struct that contains info about the boundary 
*/
struct Boundary
{
	core::Vec2f position;
};
class GameManager;

/**
* \brief BoundaryManager is a ComponentManager that holds all the boundaries in one place.
*/
class BoundaryManager : public core::ComponentManager<Boundary, static_cast<core::EntityMask>(ComponentType::BOUNDARY)>
{
public:
	explicit BoundaryManager(core::EntityManager& entityManager, GameManager& gameManager);
private:
	GameManager& gameManager_;
};
}