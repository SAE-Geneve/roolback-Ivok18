#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"
#include "utils/action_utility.h"

namespace game
{
class PhysicsManager;

/**
 * \brief PlayerCharacter is a struct that holds information about the player character (when they can shoot again, their current input, and their current health).
 */
struct PlayerCharacter
{
    PlayerInput input = 0u;
    PlayerNumber playerNumber = INVALID_PLAYER;
    short health = playerMaxHealth;
    float hurtTime = 0.0f;
};

/**
* \brief OnHealthChangeTrigger is an interface for classes that needs to be called when a player health is modified.
* It needs to be registered in the PlayerCharacterManager
*/

class GameManager;

/**
 * \brief PlayerCharacterManager is a ComponentManager that holds all the PlayerCharacter in the game.
 */
class PlayerCharacterManager : public core::ComponentManager<PlayerCharacter, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)>
{
public:
    explicit PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager);
    void FixedUpdate(sf::Time dt);

private:
    PhysicsManager& physicsManager_;
    GameManager& gameManager_;

};
}
