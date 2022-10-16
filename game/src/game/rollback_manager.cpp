#include <game/rollback_manager.h>
#include <game/game_manager.h>
#include "utils/assert.h"
#include <utils/log.h>
#include <fmt/format.h>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{

RollbackManager::RollbackManager(GameManager& gameManager, core::EntityManager& entityManager) :
    gameManager_(gameManager), entityManager_(entityManager),
    currentTransformManager_(entityManager),
    currentPhysicsManager_(entityManager), currentPlayerManager_(entityManager, currentPhysicsManager_, gameManager_),
    currentBallManager_(entityManager, gameManager),
    currentBoundaryManager_(entityManager, gameManager),
    currentHomeManager_(entityManager, gameManager),
    lastValidatePhysicsManager_(entityManager),
    lastValidatePlayerManager_(entityManager, lastValidatePhysicsManager_, gameManager_),
	lastValidateBallManager_(entityManager, gameManager),
    lastValidateBoundaryManager_(entityManager, gameManager),
    lastValidateHomeManager_(entityManager, gameManager)

{
    for (auto& input : inputs_)
    {
        std::fill(input.begin(), input.end(), '\0');
    }
    currentPhysicsManager_.RegisterTriggerListener(*this);
}

void RollbackManager::SimulateToCurrentFrame()
{
    core::LogDebug(std::to_string(timeSinceLastCollision_.getElapsedTime().asMicroseconds()));
    
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto currentFrame = gameManager_.GetCurrentFrame();
    const auto lastValidateFrame = gameManager_.GetLastValidateFrame();
    //Destroying all created Entities after the last validated frame
    for (const auto& createdEntity : createdEntities_)
    {
        if (createdEntity.createdFrame > lastValidateFrame)
        {
            entityManager_.DestroyEntity(createdEntity.entity);
        }
    }
    createdEntities_.clear();
    //Remove DESTROY flags
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
        {
            entityManager_.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
        }
    }

    //Revert the current game state to the last validated game state
    currentBallManager_.CopyAllComponents(lastValidateBallManager_.GetAllComponents());
    currentPhysicsManager_.CopyAllComponents(lastValidatePhysicsManager_);
    currentPlayerManager_.CopyAllComponents(lastValidatePlayerManager_.GetAllComponents());
    currentBoundaryManager_.CopyAllComponents(lastValidateBoundaryManager_.GetAllComponents());
    currentHomeManager_.CopyAllComponents(lastValidateHomeManager_.GetAllComponents());
    

    for (Frame frame = lastValidateFrame + 1; frame <= currentFrame; frame++)
    {
        testedFrame_ = frame;
        //Copy player inputs to player manager
        for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
        {
            const auto playerInput = GetInputAtFrame(playerNumber, frame);
            const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
            if (playerEntity == core::INVALID_ENTITY)
            {
                core::LogWarning(fmt::format("Invalid Entity in {}:line {}", __FILE__, __LINE__));
                continue;
            }
            auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
            playerCharacter.input = playerInput;
            currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
        }
        //Simulate one frame of the game
        currentBallManager_.FixedUpdate(sf::seconds(fixedPeriod));
        currentPlayerManager_.FixedUpdate(sf::seconds(fixedPeriod));
        currentPhysicsManager_.FixedUpdate(sf::seconds(fixedPeriod));
        currentBoundaryManager_.FixedUpdate(sf::seconds(fixedPeriod));
        currentHomeManager_.FixedUpdate(sf::seconds(fixedPeriod));
    }
    //Copy the physics states to the transforms
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity,
            static_cast<core::EntityMask>(core::ComponentType::BODY2D) |
            static_cast<core::EntityMask>(core::ComponentType::TRANSFORM)))
            continue;
        const auto& body = currentPhysicsManager_.GetBody(entity);
        currentTransformManager_.SetPosition(entity, body.position);
        currentTransformManager_.SetRotation(entity, body.rotation);
    }
}

void RollbackManager::SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, Frame inputFrame)
{
    //Should only be called on the server
    if (currentFrame_ < inputFrame)
    {
        StartNewFrame(inputFrame);
    }
    inputs_[playerNumber][currentFrame_ - inputFrame] = playerInput;
    if (lastReceivedFrame_[playerNumber] < inputFrame)
    {
        lastReceivedFrame_[playerNumber] = inputFrame;
        //Repeat the same inputs until currentFrame
        for (size_t i = 0; i < currentFrame_ - inputFrame; i++)
        {
            inputs_[playerNumber][i] = playerInput;
        }
    }
}

void RollbackManager::StartNewFrame(Frame newFrame)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (currentFrame_ > newFrame)
        return;
    const auto delta = newFrame - currentFrame_;
    if (delta == 0)
    {
        return;
    }
    for (auto& inputs : inputs_)
    {
        for (auto i = inputs.size() - 1; i >= delta; i--)
        {
            inputs[i] = inputs[i - delta];
        }

        for (Frame i = 0; i < delta; i++)
        {
            inputs[i] = inputs[delta];
        }
    }
    currentFrame_ = newFrame;
}

void RollbackManager::ValidateFrame(Frame newValidateFrame)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto lastValidateFrame = gameManager_.GetLastValidateFrame();
    //We check that we got all the inputs
    for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
    {
        if (GetLastReceivedFrame(playerNumber) < newValidateFrame)
        {
            gpr_assert(false, "We should not validate a frame if we did not receive all inputs!!!");
            return;
        }
    }
    //Destroying all created Entities after the last validated frame
    for (const auto& createdEntity : createdEntities_)
    {
        if (createdEntity.createdFrame > lastValidateFrame)
        {
            entityManager_.DestroyEntity(createdEntity.entity);
        }
    }
    createdEntities_.clear();
    //Remove DESTROYED flag
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
        {
            entityManager_.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
        }

    }
    createdEntities_.clear();

    //We use the current game state as the temporary new validate game state
    currentBallManager_.CopyAllComponents(lastValidateBallManager_.GetAllComponents());
    currentPhysicsManager_.CopyAllComponents(lastValidatePhysicsManager_);
    currentPlayerManager_.CopyAllComponents(lastValidatePlayerManager_.GetAllComponents());
    currentBoundaryManager_.CopyAllComponents(lastValidateBoundaryManager_.GetAllComponents());
    currentHomeManager_.CopyAllComponents(lastValidateHomeManager_.GetAllComponents());

    //We simulate the frames until the new validated frame
    for (Frame frame = lastValidateFrame_ + 1; frame <= newValidateFrame; frame++)
    {
        testedFrame_ = frame;
        //Copy the players inputs into the player manager
        for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
        {
            const auto playerInput = GetInputAtFrame(playerNumber, frame);
            const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
            auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
            playerCharacter.input = playerInput;
            currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
        }
        //We simulate one frame
        currentBallManager_.FixedUpdate(sf::seconds(fixedPeriod));
        currentPlayerManager_.FixedUpdate(sf::seconds(fixedPeriod));
        currentPhysicsManager_.FixedUpdate(sf::seconds(fixedPeriod));
        currentHomeManager_.FixedUpdate(sf::seconds(fixedPeriod));
    }
    //Definitely remove DESTROY entities
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
        {
            entityManager_.DestroyEntity(entity);
        }
    }
    //Copy back the new validate game state to the last validated game state
    lastValidateBallManager_.CopyAllComponents(currentBallManager_.GetAllComponents());
    lastValidatePlayerManager_.CopyAllComponents(currentPlayerManager_.GetAllComponents());
    lastValidatePhysicsManager_.CopyAllComponents(currentPhysicsManager_);
    lastValidateBoundaryManager_.CopyAllComponents(currentBoundaryManager_.GetAllComponents());
    lastValidateHomeManager_.CopyAllComponents(currentHomeManager_.GetAllComponents());
    lastValidateFrame_ = newValidateFrame;
    createdEntities_.clear();
}

void RollbackManager::ConfirmFrame(Frame newValidateFrame, const std::array<PhysicsState, maxPlayerNmb>& serverPhysicsState)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    ValidateFrame(newValidateFrame);
    for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
    {
        const PhysicsState lastPhysicsState = GetValidatePhysicsState(playerNumber);
        if (serverPhysicsState[playerNumber] != lastPhysicsState)
        {
            gpr_assert(false, fmt::format("Physics State are not equal for player {} (server frame: {}, client frame: {}, server: {}, client: {})", 
                playerNumber+1, 
                newValidateFrame, 
                lastValidateFrame_, 
                serverPhysicsState[playerNumber], 
                lastPhysicsState));
        }
    }
}

PhysicsState RollbackManager::GetValidatePhysicsState(PlayerNumber playerNumber) const
{
    PhysicsState state = 0;
    const core::Entity playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
    const auto& playerBody = lastValidatePhysicsManager_.GetBody(playerEntity);

    const auto pos = playerBody.position;
    const auto* posPtr = reinterpret_cast<const PhysicsState*>(&pos);
    //Adding position
    for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
    {
        state += posPtr[i];
    }

    //Adding velocity
    const auto velocity = playerBody.velocity;
    const auto* velocityPtr = reinterpret_cast<const PhysicsState*>(&velocity);
    for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
    {
        state += velocityPtr[i];
    }
    //Adding rotation
    const auto angle = playerBody.rotation.value();
    const auto* anglePtr = reinterpret_cast<const PhysicsState*>(&angle);
    for (size_t i = 0; i < sizeof(float) / sizeof(PhysicsState); i++)
    {
        state += anglePtr[i];
    }
    //Adding angular Velocity
    const auto angularVelocity = playerBody.angularVelocity.value();
    const auto* angularVelPtr = reinterpret_cast<const PhysicsState*>(&angularVelocity);
    for (size_t i = 0; i < sizeof(float) / sizeof(PhysicsState); i++)
    {
        state += angularVelPtr[i];
    }
    return state;
}

void RollbackManager::SpawnPlayer(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::Degree rotation)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    Body playerBody;
    playerBody.position = position;
    playerBody.rotation = rotation;
    Box playerBox;
    playerBox.extends.x = playerScaleX;
    playerBox.extends.y = playerScaleY;
    PlayerCharacter playerCharacter;
    playerCharacter.playerNumber = playerNumber;

    currentPlayerManager_.AddComponent(entity);
    currentPlayerManager_.SetComponent(entity, playerCharacter);

    currentPhysicsManager_.AddBody(entity);
    currentPhysicsManager_.SetBody(entity, playerBody);
    currentPhysicsManager_.AddBox(entity);
    currentPhysicsManager_.SetBox(entity, playerBox);

    lastValidatePlayerManager_.AddComponent(entity);
    lastValidatePlayerManager_.SetComponent(entity, playerCharacter);

    lastValidatePhysicsManager_.AddBody(entity);
    lastValidatePhysicsManager_.SetBody(entity, playerBody);
    lastValidatePhysicsManager_.AddBox(entity);
    lastValidatePhysicsManager_.SetBox(entity, playerBox);

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
    currentTransformManager_.SetRotation(entity, rotation);
}

PlayerInput RollbackManager::GetInputAtFrame(PlayerNumber playerNumber, Frame frame) const
{
    gpr_assert(currentFrame_ - frame < inputs_[playerNumber].size(),
        "Trying to get input too far in the past");
    return inputs_[playerNumber][currentFrame_ - frame];
}

void RollbackManager::OnTrigger(core::Entity entity1, core::Entity entity2)
{
   //const std::function<void(const PlayerCharacter&, core::Entity, const Ball&, core::Entity)> ManageCollision =
        //[this](const auto& player, auto playerEntity, const auto& ball, auto ballEntity)
    {
        //if (player.playerNumber != ball.playerNumber)
        {
            //gameManager_.DestroyBall(ballEntity);
            //lower health point
            /*auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
            if (playerCharacter.invincibilityTime <= 0.0f)
            {
                core::LogDebug(fmt::format("Player {} is hit by ball", playerCharacter.playerNumber));
                --playerCharacter.health;
                playerCharacter.invincibilityTime = playerInvincibilityPeriod;
            }
            currentPlayerManager_.SetComponent(playerEntity, playerCharacter);*/
        }
    };
    if (entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        const auto& ballBody = currentPhysicsManager_.GetBody(entity2);
        const auto velocityAfterCollisionWithPlayer = core::Vec2f(-ballBody.velocity.x, ballBody.velocity.y) * ballSpeedIncrease;

        currentPhysicsManager_.SetBody(entity2, Body(ballBody.position, velocityAfterCollisionWithPlayer));
        //ManageCollision(player, entity1, ball, entity2);
    }
    if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        const auto& ballBody = currentPhysicsManager_.GetBody(entity1);
        const auto velocityAfterCollisionWithPlayer = core::Vec2f(-ballBody.velocity.x, ballBody.velocity.y) * ballSpeedIncrease;

        currentPhysicsManager_.SetBody(entity1, Body(ballBody.position, velocityAfterCollisionWithPlayer));
        //ManageCollision(player, entity2, ball, entity1);
    }
    if(entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BOUNDARY)) &&
        entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        const auto& ballBody = currentPhysicsManager_.GetBody(entity2);
        const auto velocityAfterCollisionWithBoundary = core::Vec2f(ballBody.velocity.x, -ballBody.velocity.y);

        currentPhysicsManager_.SetBody(entity2, Body(ballBody.position, velocityAfterCollisionWithBoundary));
       
    }
    if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BOUNDARY)) &&
        entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        const auto& ballBody = currentPhysicsManager_.GetBody(entity1);
        const auto velocityAfterCollisionWithBoundary = core::Vec2f(ballBody.velocity.x, -ballBody.velocity.y);

        currentPhysicsManager_.SetBody(entity1, Body(ballBody.position, velocityAfterCollisionWithBoundary));
    }
}

void RollbackManager::SpawnBall(core::Entity entity, core::Vec2f position, core::Vec2f velocity)
{
    createdEntities_.push_back({ entity, testedFrame_ });

    Body ballBody;
    ballBody.position = position;
    ballBody.velocity = velocity;
    Box ballBox;
    ballBox.extends = core::Vec2f::one() * ballScale * 0.17f;

    currentBallManager_.AddComponent(entity);
    currentBallManager_.SetComponent(entity, { 0 });

    currentPhysicsManager_.AddBody(entity);
    currentPhysicsManager_.SetBody(entity, ballBody);
    currentPhysicsManager_.AddBox(entity);
    currentPhysicsManager_.SetBox(entity, ballBox);

    lastValidateBallManager_.AddComponent(entity);
    lastValidateBallManager_.SetComponent(entity, { 0 });

    lastValidatePhysicsManager_.AddBody(entity);
    lastValidatePhysicsManager_.SetBody(entity, ballBody);
    lastValidatePhysicsManager_.AddBox(entity);
    lastValidatePhysicsManager_.SetBox(entity, ballBox);

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
    currentTransformManager_.SetScale(entity, core::Vec2f::one() * ballScale);
}

void RollbackManager::SpawnBoundary(core::Entity entity, core::Vec2f position)
{
    createdEntities_.push_back({ entity, testedFrame_ });

    Body boundaryBody;
    boundaryBody.position = position.y > 0.f ?
        core::Vec2f(position.x, position.y + boundaryScaleY):
        core::Vec2f(position.x, position.y - boundaryScaleY);
   
    Box boundaryBox;
    boundaryBox.extends.x = boundaryScaleX;
    boundaryBox.extends.y = boundaryScaleY;

    currentBoundaryManager_.AddComponent(entity);
    currentBoundaryManager_.SetComponent(entity, {position});

    currentPhysicsManager_.AddBody(entity);
    currentPhysicsManager_.SetBody(entity, boundaryBody);
    currentPhysicsManager_.AddBox(entity);
    currentPhysicsManager_.SetBox(entity, boundaryBox);

    lastValidateBoundaryManager_.AddComponent(entity);
    lastValidateBoundaryManager_.SetComponent(entity, {position});

    lastValidatePhysicsManager_.AddBody(entity);
    lastValidatePhysicsManager_.SetBody(entity, boundaryBody);
    lastValidatePhysicsManager_.AddBox(entity);
    lastValidatePhysicsManager_.SetBox(entity, boundaryBox);

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
}

void RollbackManager::SpawnHome(core::Entity entity, core::Vec2f position)
{
    createdEntities_.push_back({ entity, testedFrame_ });

    Body homeBody;
    homeBody.position = position;
    homeBody.velocity = core::Vec2f::zero();
    Box homeBox;
    homeBox.extends.x = homeScaleX;
    homeBox.extends.y = homeScaleY;

    currentHomeManager_.AddComponent(entity);
    currentHomeManager_.SetComponent(entity, { position });

    currentPhysicsManager_.AddBody(entity);
    currentPhysicsManager_.SetBody(entity, homeBody);
    currentPhysicsManager_.AddBox(entity);
    currentPhysicsManager_.SetBox(entity, homeBox);

    lastValidateHomeManager_.AddComponent(entity);
    lastValidateHomeManager_.SetComponent(entity, { position });

    lastValidatePhysicsManager_.AddBody(entity);
    lastValidatePhysicsManager_.SetBody(entity, homeBody);
    lastValidatePhysicsManager_.AddBox(entity);
    lastValidatePhysicsManager_.SetBox(entity, homeBox);

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
}




void RollbackManager::DestroyEntity(core::Entity entity)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    //we don't need to save a bullet that has been created in the time window
    if (std::find_if(createdEntities_.begin(), createdEntities_.end(), [entity](auto newEntity)
        {
            return newEntity.entity == entity;
        }) != createdEntities_.end())
    {
        entityManager_.DestroyEntity(entity);
        return;
    }
        entityManager_.AddComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
}
}
