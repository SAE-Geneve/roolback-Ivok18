#pragma once
#pragma once
#include <SFML/System/Time.hpp>
#include "game_globals.h"

namespace game
{
/**
 * \brief HealthBar is a struct that contain info about a player healthbar
 */
struct HealthBar
{
	PlayerNumber playerNumber = INVALID_PLAYER;
	core::Vec2f position;
};
class GameManager;

class HealthBarManager : public core::ComponentManager<HealthBar, static_cast<core::EntityMask>(ComponentType::HEALTHBAR)>
{
public:
	explicit HealthBarManager(core::EntityManager& entityManager, GameManager& gameManager);
private:
	GameManager& gameManager_;
};
}