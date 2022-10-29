#include <game/player_character.h>
#include <game/game_manager.h>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif
namespace game
{
PlayerCharacterManager::PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager) :
    ComponentManager(entityManager),
    physicsManager_(physicsManager),
    gameManager_(gameManager)
{
   
}

void PlayerCharacterManager::FixedUpdate(sf::Time dt)
{
    
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
    {
        const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
        if (!entityManager_.HasComponent(playerEntity,
            static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)))
            continue;
        auto playerBody = physicsManager_.GetBody(playerEntity);
        const auto playerBox = physicsManager_.GetBox(playerEntity);
        auto playerCharacter = GetComponent(playerEntity);
        const auto input = playerCharacter.input;

        const bool up = (input & PlayerInputEnum::PlayerInput::UP) && playerBody.position.y + playerBox.extends.y < topBoundaryPos.y;
        const bool down = (input & PlayerInputEnum::PlayerInput::DOWN) && playerBody.position.y - playerBox.extends.y > bottomBoundaryPos.y;

        auto dir = core::Vec2f::up();
        dir = dir.Rotate(-(playerBody.rotation + playerBody.angularVelocity * dt.asSeconds()));

        auto velocity = ((down ? core::Vec2f(0, -playerSpeed) : core::Vec2f::zero() +
									(up ? core::Vec2f(0, playerSpeed) : core::Vec2f::zero())));

        playerBody.velocity = velocity * dt.asSeconds();
        physicsManager_.SetBody(playerEntity, playerBody);

        if (playerCharacter.hurtTime > 0.0f)
        {
            playerCharacter.hurtTime -= dt.asSeconds();
            SetComponent(playerEntity, playerCharacter);
        }
    }
}
}
