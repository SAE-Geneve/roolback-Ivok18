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
    gameManager_(gameManager),entityManager_(entityManager),
    currentTransformManager_(entityManager),
    currentPhysicsManager_(entityManager), currentPlayerManager_(entityManager, currentPhysicsManager_, gameManager_),
    currentBallManager_(entityManager, gameManager),
    currentBoundaryManager_(entityManager, gameManager),
    currentHomeManager_(entityManager, gameManager),
    currentHealthBarManager(entityManager, gameManager),

    lastValidatePhysicsManager_(entityManager),
    lastValidatePlayerManager_(entityManager, lastValidatePhysicsManager_, gameManager_),
	lastValidateBallManager_(entityManager, gameManager),
    lastValidateBoundaryManager_(entityManager, gameManager),
    lastValidateHomeManager_(entityManager, gameManager),
    lastValidateHealthBarManager_(entityManager, gameManager)

{
    for (auto& input : inputs_)
    {
        std::fill(input.begin(), input.end(), '\0');
    }
    currentPhysicsManager_.RegisterTriggerListener(*this);
    //currentPlayerManager_.RegisterHealthChangeTriggerListener(clientGameManager.GetHealthChangeTrigger());
}

void RollbackManager::SimulateToCurrentFrame()
{
    
    
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
    currentHealthBarManager.CopyAllComponents(lastValidateHealthBarManager_.GetAllComponents());
    

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
        currentHealthBarManager.FixedUpdate(sf::seconds(fixedPeriod));
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
    currentHealthBarManager.CopyAllComponents(lastValidateHealthBarManager_.GetAllComponents());

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
        currentHealthBarManager.FixedUpdate(sf::seconds(fixedPeriod));
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
    lastValidateHealthBarManager_.CopyAllComponents(currentHealthBarManager.GetAllComponents());
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
            if (playerCharacter.hurtTime <= 0.0f)
            {
                core::LogDebug(fmt::format("Player {} is hit by ball", playerCharacter.playerNumber));
                --playerCharacter.health;
                playerCharacter.hurtTime = playerHurtPeriod;
            }
            currentPlayerManager_.SetComponent(playerEntity, playerCharacter);*/
        }
    }

    const std::function<void(const core::Entity&, const core::Entity&)> ManageCollisionBetweenBallAndPlayer =
        [this](const auto& ballEntity, const auto& playerEntity)
    {
        const PlayerCharacter& playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
        Ball& ball = currentBallManager_.GetComponent(ballEntity);

        const auto& playerBody = currentPhysicsManager_.GetBody(playerEntity);
        const auto& ballBody = currentPhysicsManager_.GetBody(ballEntity);
        const auto isPlayerLeft = playerBody.position.x < 0 ? true : false;
        const auto isPlayerRight = playerBody.position.x > 0 ? true : false;
        const auto isBallGoingLeft = ballBody.velocity.x < 0 ? true : false;
        const auto isBallGoingRight = ballBody.velocity.x > 0 ? true : false;

        //we store player number of collision in the data of the ball
        ball.playerNumber = playerCharacter.playerNumber;


        //in either case the ball will go in it's "x-axis" opposite direction
        if ((isBallGoingLeft && isPlayerLeft) || (isBallGoingRight && isPlayerRight))
        {
            //we calculate ball velocity after it's collision with a player
            auto ballVelocityAfterCollisionWithPlayer = core::Vec2f(-ballBody.velocity.x, ballBody.velocity.y) * ballRatioSpeedIncrease;

            //we retrieve ball speed after the collision
            const auto ballSpeedAfterCollisionWithPlayer = std::abs(ballVelocityAfterCollisionWithPlayer.x);

            //if the speed of the ball exceeds it's maximum value, we revert to the speed before collision
            ballVelocityAfterCollisionWithPlayer = ballSpeedAfterCollisionWithPlayer >= ballMaxSpeed ?
                core::Vec2f(ballVelocityAfterCollisionWithPlayer) / ballRatioSpeedIncrease :
                ballVelocityAfterCollisionWithPlayer;

            //we set new body data on the ball (position, velocity)
            currentPhysicsManager_.SetBody(ballEntity, Body(ballBody.position, ballVelocityAfterCollisionWithPlayer));

            //we set new ball data on the ball (playerNumber)
            currentBallManager_.SetComponent(ballEntity, ball);
        }
        
    };

    const std::function<void(const core::Entity&)> ManageCollisionBetweenBallAndBoundary =
        [this](const auto& ballEntity)
    {
        const auto& ballBody = currentPhysicsManager_.GetBody(ballEntity);
        const auto ballVelocityAfterCollisionWithBoundary = core::Vec2f(ballBody.velocity.x, -ballBody.velocity.y);

        currentPhysicsManager_.SetBody(ballEntity, Body(ballBody.position, ballVelocityAfterCollisionWithBoundary));
    };

    const std::function<void(const core::Entity&, const core::Entity&)> ManageCollisionBetweenBallAndHome =
        [this](const auto& ballEntity, const auto& homeEntity)
    {
        //we retrieve home data 
        const Home& home = currentHomeManager_.GetComponent(homeEntity);

        //we retrieve ball data (playerNumber)
        const Ball& ball = currentBallManager_.GetComponent(ballEntity);

        //using home data (player number), we retrieve the player entity connected to the home 
        const auto& damagedPlayerEntity = gameManager_.GetEntityFromPlayerNumber(home.playerNumber);

        //using player entity, we retrieve player character data
        auto& damagedPlayerCharacter = currentPlayerManager_.GetComponent(damagedPlayerEntity);

        //we activate hurt animation
        damagedPlayerCharacter.hurtTime = playerHurtPeriod;

        //we decrease player health
        --damagedPlayerCharacter.health;
        damagedPlayerCharacter.health = damagedPlayerCharacter.health <= 0 ? 0 : damagedPlayerCharacter.health;

        //we update player healthbar
        for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
        {
            //...we find an entity that has an healthbar component type
            const bool foundAnHealthBar = entityManager_.HasComponent(entity,
                static_cast<core::EntityMask>(game::ComponentType::HEALTHBAR)) ? true : false;
            if (foundAnHealthBar)
            {
                //...we determine if that entity corresponds to the player that was damaged
                const auto& healthbar = currentHealthBarManager.GetComponent(entity);
                const bool foundDamagedPlayerHealthBar = healthbar.playerNumber == home.playerNumber ? true : false;
                if (foundDamagedPlayerHealthBar)
                {
                    //...if that's the case, we update the healthbar 
                    gameManager_.UpdatePlayerHealthBar(damagedPlayerCharacter, entity);
                }
            }
        }

        PlayerNumber winner = gameManager_.CheckWinner();
        if(winner != INVALID_PLAYER)
        {
            gameManager_.WinGame(winner);
        }

        //we set new player character data on the damaged player (health)
        currentPlayerManager_.SetComponent(damagedPlayerEntity, damagedPlayerCharacter);

        //we retrieve attacking player character entity
        const auto& attackerEntity = gameManager_.GetEntityFromPlayerNumber(ball.playerNumber);

        //using it, we get the player body
        const auto& attackerBody = currentPhysicsManager_.GetBody(attackerEntity);

        //we see if attacking player character is left side or right side
        const auto isAttackerLeft = attackerBody.position.x < 0 ? true : false;

        //we retrive the ball body
        const auto& ballBody = currentPhysicsManager_.GetBody(ballEntity);

        //we get its position
        core::Vec2f ballNewPos = isAttackerLeft ?
            core::Vec2f(ballLeftRespawnX, attackerBody.position.y) :
            core::Vec2f(ballRightRespawnX, attackerBody.position.y);

        //we see if we will reset the velocity or not (depending if its speed at collision time is equal or higher than the max speed for the ball
        const bool mustResetVelocity = std::abs(ballBody.velocity.x * ballRatioSpeedIncrease) >= ballMaxSpeed;
        core::Vec2f velocityAfterCollisionWithHome = ballBody.velocity;
        if(mustResetVelocity)
        {
            velocityAfterCollisionWithHome = isAttackerLeft ?
                core::Vec2f(ballInitialSpeed, ballInitialSpeed) :
                core::Vec2f(-ballInitialSpeed, -ballInitialSpeed);
        }
        
        //we retrieve the ball body after earlier verification check
        const Body newBallBody(ballNewPos, velocityAfterCollisionWithHome);

        //we set the ball body
        currentPhysicsManager_.SetBody(ballEntity, newBallBody);
        
    };

    if (entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        ManageCollisionBetweenBallAndPlayer(entity2, entity1);
        /*const auto& ballBody = currentPhysicsManager_.GetBody(entity2);
        const auto& playerBody = currentPhysicsManager_.GetBody(entity1);
        const auto isPlayerLeft = playerBody.position.x < 0 ? true : false;
        const auto isPlayerRight = playerBody.position.x > 0 ? true : false;
        core::Vec2f velocityAfterCollisionWithPlayer;
        if(ballBody.velocity.x < 0 && isPlayerLeft)
        {
            velocityAfterCollisionWithPlayer = core::Vec2f(-ballBody.velocity.x, ballBody.velocity.y) * ballRatioSpeedIncrease;
            currentPhysicsManager_.SetBody(entity2, Body(ballBody.position, velocityAfterCollisionWithPlayer));
        }
        else if(ballBody.velocity.x > 0 && isPlayerRight)
        {
            velocityAfterCollisionWithPlayer = core::Vec2f(-ballBody.velocity.x, ballBody.velocity.y) * ballRatioSpeedIncrease;
            currentPhysicsManager_.SetBody(entity2, Body(ballBody.position, velocityAfterCollisionWithPlayer));
        }*/
        //ManageCollision(player, entity1, ball, entity2);
    }
    if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        ManageCollisionBetweenBallAndPlayer(entity1, entity2);
        /*const auto& ballBody = currentPhysicsManager_.GetBody(entity1);
        const auto& playerBody = currentPhysicsManager_.GetBody(entity2);
        const auto isPlayerLeft = playerBody.position.x < 0 ? true : false;
        const auto isPlayerRight = playerBody.position.x > 0 ? true : false;
        core::Vec2f velocityAfterCollisionWithPlayer;
        if (ballBody.velocity.x < 0 && isPlayerLeft)
        {
            velocityAfterCollisionWithPlayer = core::Vec2f(-ballBody.velocity.x, ballBody.velocity.y) * ballRatioSpeedIncrease;
            currentPhysicsManager_.SetBody(entity1, Body(ballBody.position, velocityAfterCollisionWithPlayer));
        }
        else if (ballBody.velocity.x > 0 && isPlayerRight)
        {
            velocityAfterCollisionWithPlayer = core::Vec2f(-ballBody.velocity.x, ballBody.velocity.y) * ballRatioSpeedIncrease;
            currentPhysicsManager_.SetBody(entity1, Body(ballBody.position, velocityAfterCollisionWithPlayer));
        }*/
        //ManageCollision(player, entity2, ball, entity1);
    }

    if(entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BOUNDARY)) &&
        entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        ManageCollisionBetweenBallAndBoundary(entity2);
        //const auto& ballBody = currentPhysicsManager_.GetBody(entity2);
        //const auto velocityAfterCollisionWithBoundary = core::Vec2f(ballBody.velocity.x, -ballBody.velocity.y);
        //currentPhysicsManager_.SetBody(entity2, Body(ballBody.position, velocityAfterCollisionWithBoundary));
    }
    if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BOUNDARY)) &&
        entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        ManageCollisionBetweenBallAndBoundary(entity1);
        //const auto& ballBody = currentPhysicsManager_.GetBody(entity1);
        //const auto velocityAfterCollisionWithBoundary = core::Vec2f(ballBody.velocity.x, -ballBody.velocity.y);
        //currentPhysicsManager_.SetBody(entity1, Body(ballBody.position, velocityAfterCollisionWithBoundary));
    }

    if(entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::HOME)) &&
        entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        ManageCollisionBetweenBallAndHome(entity2, entity1);
    }
    if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::HOME)) &&
        entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BALL)))
    {
        ManageCollisionBetweenBallAndHome(entity1, entity2);
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

    const PlayerNumber startPlayerNumber = ballBody.velocity.x > 0 ? 0 : 1;


    currentBallManager_.AddComponent(entity);
    currentBallManager_.SetComponent(entity, { 0,startPlayerNumber });

    currentPhysicsManager_.AddBody(entity);
    currentPhysicsManager_.SetBody(entity, ballBody);
    currentPhysicsManager_.AddBox(entity);
    currentPhysicsManager_.SetBox(entity, ballBox);

    lastValidateBallManager_.AddComponent(entity);
    lastValidateBallManager_.SetComponent(entity, { 0,startPlayerNumber });

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
    boundaryBody.bodyType = BodyType::STATIC;
    const auto isTopBoundary = position.y > 0.f;
    boundaryBody.position = isTopBoundary ?
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

void RollbackManager::SpawnHome(core::Entity entity, PlayerNumber playerNumber, core::Vec2f position)
{
    createdEntities_.push_back({ entity, testedFrame_ });

    Body homeBody;
    homeBody.bodyType = BodyType::STATIC;
    const auto isLeftHome = position.x < 0.f;
    homeBody.position = isLeftHome ?
        core::Vec2f(position.x - homeScaleX, position.y) :
        core::Vec2f(position.x + homeScaleX, position.y);
    homeBody.velocity = core::Vec2f::zero();
    Box homeBox;
    homeBox.extends.x = homeScaleX;
    homeBox.extends.y = homeScaleY;

    currentHomeManager_.AddComponent(entity);
    currentHomeManager_.SetComponent(entity, { playerNumber, position });

    currentPhysicsManager_.AddBody(entity);
    currentPhysicsManager_.SetBody(entity, homeBody);
    currentPhysicsManager_.AddBox(entity);
    currentPhysicsManager_.SetBox(entity, homeBox);

    lastValidateHomeManager_.AddComponent(entity);
    lastValidateHomeManager_.SetComponent(entity, { playerNumber, position });

    lastValidatePhysicsManager_.AddBody(entity);
    lastValidatePhysicsManager_.SetBody(entity, homeBody);
    lastValidatePhysicsManager_.AddBox(entity);
    lastValidatePhysicsManager_.SetBox(entity, homeBox);

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
}

void RollbackManager::SpawnVizualizer(core::Entity entity, core::Vec2f position)
{
    createdEntities_.push_back({ entity, testedFrame_ });

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
}

void RollbackManager::SpawnHealthBar(core::Entity entity, core::Vec2f position)
{
    createdEntities_.push_back({entity,  testedFrame_ });
    
    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
}

void RollbackManager::SpawnHealthBarBackground(core::Entity entity, PlayerNumber playerNumber, core::Vec2f position)
{
    createdEntities_.push_back({ entity,  testedFrame_ });

    currentHealthBarManager.AddComponent(entity);
    currentHealthBarManager.SetComponent(entity, { playerNumber, position });

    lastValidateHealthBarManager_.AddComponent(entity);
	lastValidateHealthBarManager_.SetComponent(entity, { playerNumber, position });

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
}

void RollbackManager::UpdatePlayerHealthbar(const PlayerCharacter& player, const core::Entity& healthBarEntity)
{
    const auto healthBarScale = currentTransformManager_.GetScale(healthBarEntity);
    const auto newScale = sf::Vector2f(static_cast<float>(player.health) / static_cast<float>(playerMaxHealth), healthBarScale.y);
    currentTransformManager_.SetScale(healthBarEntity, newScale);
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
