#pragma once
#include "bullet_manager.h"
#include "game_globals.h"
#include "physics_manager.h"
#include "player_character.h"
#include "engine/entity.h"
#include "engine/transform.h"
#include "network/packet_type.h"



namespace game
{
class GameManager;

/**
 * \brief CreatedEntity is a struct that contains information on the newly created entities.
 * It is used by the RollbackManager to destroy newly created entities when going back in time.
 */
struct CreatedEntity
{
    core::Entity entity = core::INVALID_ENTITY;
    Frame createdFrame = 0;
};

/**
 * \brief RollbackManager is a class that manages all the rollback mechanisms of the game.
 * It contains two copies of the world (PhysicsManager, TransformManager, etc...), the current one and the validated one.
 * When receiving new information, it can reupdate the current copy of the world.
 */
class RollbackManager final : public OnTriggerInterface
{
public:
    explicit RollbackManager(GameManager& gameManager, core::EntityManager& entityManager);
    /**
     * \brief SimulateToCurrentFrame is a method that simulates all players with new inputs, method call only by the clients to update the current state of the visuals
     */
    void SimulateToCurrentFrame();
    /**
     * \brief SetPlayerInput is a method that set the input of a certain player on a certain game frame.
     * It can change an input between the last validated frame and the current frame.
     * It is called by the GameManager when receiving new inputs from packets.
     * \param playerNumber is the player number whose input will change
     * \param playerInput is the new input
     * \param inputFrame is the game frame of the new input
     */
    void SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, Frame inputFrame);
    void StartNewFrame(Frame newFrame);
    /**
     * \brief ValidateFrame is a method that validates all the frames from lastValidateFrame_ to newValidateFrame.
     * It changes lastValidateFrame_ to be newValidateFrame.
     * \param newValidateFrame is the new value of lastValidateFrame_
     */
    void ValidateFrame(Frame newValidateFrame);
    /**
     * \brief ConfirmFrame is a method that confirms the new validate frame by checking the Physics State checksums
     * It is called by the clients when receiving Confirm Frame packet
     * \param newValidatedFrame is the new frame that is validated
     * \param serverPhysicsState is the physics state given by the server through a packet
     */
    void ConfirmFrame(Frame newValidatedFrame, const std::array<PhysicsState, maxPlayerNmb>& serverPhysicsState);
    [[nodiscard]] PhysicsState GetValidatePhysicsState(PlayerNumber playerNumber) const;
    [[nodiscard]] Frame GetLastValidateFrame() const { return lastValidateFrame_; }
    [[nodiscard]] Frame GetLastReceivedFrame(PlayerNumber playerNumber) const { return lastReceivedFrame_[playerNumber]; }
    [[nodiscard]] Frame GetCurrentFrame() const { return currentFrame_; }
    [[nodiscard]] const core::TransformManager& GetTransformManager() const { return currentTransformManager_; }
    [[nodiscard]] const PlayerCharacterManager& GetPlayerCharacterManager() const { return currentPlayerManager_; }
    void SpawnPlayer(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::Degree rotation);
    void SpawnBullet(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::Vec2f velocity);
    /**
     * \brief DestroyEntity is a method that does not destroy the entity definitely, but puts the DESTROY flag on.
     * An entity is truly destroyed when the destroy frame is validated.
     * \param entity is the entity to be "destroyed"
     */
    void DestroyEntity(core::Entity entity);

    void OnTrigger(core::Entity entity1, core::Entity entity2) override;
        [[nodiscard]] const std::array<PlayerInput, windowBufferSize>& GetInputs(PlayerNumber playerNumber) const
    {
        return inputs_[playerNumber];
    }

    PhysicsManager& GetCurrentPhysicsManager() { return currentPhysicsManager_; }
private:

    [[nodiscard]] PlayerInput GetInputAtFrame(PlayerNumber playerNumber, Frame frame) const;
    GameManager& gameManager_;
    core::EntityManager& entityManager_;
    /**
     * \brief Used for rendering
     */
    core::TransformManager currentTransformManager_;
    PhysicsManager currentPhysicsManager_;
    PlayerCharacterManager currentPlayerManager_;
    BulletManager currentBulletManager_;
    /**
     * Last Validate (confirm frame) Component Managers used for rollback
     */
    PhysicsManager lastValidatePhysicsManager_;
    PlayerCharacterManager lastValidatePlayerManager_;
    BulletManager lastValidateBulletManager_;

    /**
     * \brief lastValidateFrame_ is the last validated frame from the server side.
     */
    Frame lastValidateFrame_ = 0;
    /**
     * \brief currentFrame_ is the current frame on the client side.
     */
    Frame currentFrame_ = 0;
    /**
     * \brief testedFrame_ is the current simulated frame used mainly for entity creation and collision.
     */
    Frame testedFrame_ = 0; 

    std::array<std::uint32_t, maxPlayerNmb> lastReceivedFrame_{};
    std::array<std::array<PlayerInput, windowBufferSize>, maxPlayerNmb> inputs_{};
    /**
     * \brief Array containing all the created entities in the window between the confirm frame and the current frame
     * to destroy them when rollbacking.
     */
    std::vector<CreatedEntity> createdEntities_;
};
}
