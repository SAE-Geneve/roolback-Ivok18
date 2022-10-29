#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Text.hpp>

#include "game_globals.h"
#include "rollback_manager.h"
#include "engine/entity.h"
#include "graphics/graphics.h"
#include "graphics/sprite.h"
#include "engine/system.h"
#include "engine/transform.h"
#include "network/packet_type.h"

namespace game
{
class PacketSenderInterface;


/**
 * \brief GameManager is a class which manages the state of the game. It is shared between the client and the server.
 */
class GameManager
{
public:
    GameManager();
    virtual ~GameManager() = default;
    virtual void SpawnPlayer(PlayerNumber playerNumber, core::Vec2f position);
    virtual core::Entity SpawnBall(core::Vec2f position, core::Vec2f velocity);
    virtual void DestroyBall(core::Entity entity);
    virtual core::Entity SpawnBoundary(core::Vec2f position);
    virtual core::Entity SpawnHome(PlayerNumber playerNumber, core::Vec2f position);
    virtual core::Entity SpawnHealthBar(core::Vec2f position);
    virtual core::Entity SpawnHealthBarBackground(PlayerNumber playerNumber, core::Vec2f position);
    virtual void UpdatePlayerHealthBar(PlayerCharacter player, core::Entity& healthbarEntity);
    /**
     * \brief SpawnVisualizer is a method that spawns entity whose goal is to act as a visualizer for another entity
     * It needs to be used when the other entity transform mismatches with its box collider
     * \param position is where the visualizer will be spawned
     * \param texture is the texture of the entity
     * \param color is color of the entity
     */
    core::Entity SpawnVisualizer(core::Vec2f position, [[maybe_unused]] sf::Texture& texture, [[maybe_unused]] sf::Color color);

    [[nodiscard]] core::Entity GetEntityFromPlayerNumber(PlayerNumber playerNumber) const;
    [[nodiscard]] Frame GetCurrentFrame() const { return currentFrame_; }
    [[nodiscard]] Frame GetLastValidateFrame() const { return rollbackManager_.GetLastValidateFrame(); }
    [[nodiscard]] const core::TransformManager& GetTransformManager() const { return transformManager_; }
    [[nodiscard]] const RollbackManager& GetRollbackManager() const { return rollbackManager_; }
    virtual void SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame);
    /**
     * \brief Validate is a method called by the server to validate a frame.
     */
    void Validate(Frame newValidateFrame);
    [[nodiscard]] PlayerNumber CheckWinner() const;
    virtual void WinGame(PlayerNumber winner);


protected:
    core::EntityManager entityManager_;
    core::TransformManager transformManager_;
    RollbackManager rollbackManager_;
    std::array<core::Entity, maxPlayerNmb> playerEntityMap_{};
    Frame currentFrame_ = 0;
    PlayerNumber winner_ = INVALID_PLAYER;
private:
    core::Action<core::Vec2f> onHealthChangeTriggerAction_;
};

/**
 * \brief ClientGameManager is a class that inherits from GameManager by adding the visual part and specific implementations needed by the clients.
 */
class ClientGameManager final : public GameManager,
                                public core::DrawInterface, public core::DrawImGuiInterface, public core::SystemInterface

{
public:
    enum State : std::uint32_t
    {
        STARTED = 1u << 0u,
        FINISHED = 1u << 1u,
    };
    explicit ClientGameManager(PacketSenderInterface& packetSenderInterface);
    void StartGame(unsigned long long int startingTime);
    void Begin() override;
    void Update(sf::Time dt) override;
    void End() override;
    void SetWindowSize(sf::Vector2u windowsSize);
    [[nodiscard]] sf::Vector2u GetWindowSize() const { return windowSize_; }
    void Draw(sf::RenderTarget& target) override;
    void SetClientPlayer(PlayerNumber clientPlayer);

    /**
     * \brief SpawnPlayer is a method that is called when receiving a SpawnPlayerPacket from the server.
     * \param playerNumber is the player number to be spawned
     * \param position is where the player character will be spawned
     * \param rotation is the spawning angle of the player character 
     */
    void SpawnPlayer(PlayerNumber playerNumber, core::Vec2f position) override;

    /**
     * \brief SpawnBall is method a that is called when receiving a SpawnBallPacket from the server.
     * \param position is where the ball will be spawned
     * \param velocity is the velocity of the ball when it spawns
     */
    core::Entity SpawnBall(core::Vec2f position, core::Vec2f velocity) override;

    /**
     * \brief SpawnBoundary is a method that is called when receiving a SpawnBoundaryPacket from the server.
     * \param position is where the boundary will be spawned
     */
    core::Entity SpawnBoundary(core::Vec2f position) override;

    /**
     * \brief SpawnHome is a method that is called when receiving a SpawnHomePacket from the server.
     * \param playerNumber is the player number linked to the home
     * \param position is where the home will be spawned
     */
    core::Entity SpawnHome(PlayerNumber playerNumber, core::Vec2f position) override;

    /**
     * \brief SpawnHealthbar is a method that is called when receiving a SpawnHealthbarPacket from the server.
     * \param position is where the health bar will be spawned
     */
    core::Entity SpawnHealthBar(core::Vec2f position) override;

    /**
     * \brief SpawnHealthbar is a method that is called when receiving a SpawnHealthbarPacket from the server.
     * \param playerNumber is the player number linked to the healthbar
     * \param position is where the health bar background will be spawned
     */
    core::Entity SpawnHealthBarBackground(PlayerNumber playerNumber, core::Vec2f position) override;

    /**
    * \brief VisualizeEntity is a method that give visuals to an entity.
    * It's used for entities having a mismatch between their transform and their box collider 
    * \param entity is the entity to visualize
    * \param texture is the texture of the entity
    * \param color is the color of the entity
    */
    void VisualizeEntity(const core::Entity& entity, const sf::Texture& texture, sf::Color color);
    void FixedUpdate();
    void SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame) override;
    void DrawImGui() override;
    void ConfirmValidateFrame(Frame newValidateFrame, const std::array<PhysicsState, maxPlayerNmb>& physicsStates);
    [[nodiscard]] PlayerNumber GetPlayerNumber() const { return clientPlayer_; }
    void WinGame(PlayerNumber winner) override;
    [[nodiscard]] std::uint32_t GetState() const { return state_; }
protected:

    //void UpdateCameraView();
    //sf::View cameraView_;

    PacketSenderInterface& packetSenderInterface_;
    sf::Vector2u windowSize_;
    sf::View originalView_;

    PlayerNumber clientPlayer_ = INVALID_PLAYER;
    core::SpriteManager spriteManager_;
    float fixedTimer_ = 0.0f;
    unsigned long long startingTime_ = 0;
    std::uint32_t state_ = 0;


    sf::Texture playerLeftTexture_;
    sf::Texture playerRightTexture_;
    sf::Texture ballTexture_;
    sf::Texture boundaryTexture_;
    sf::Texture homeTexture_;
    sf::Texture healthbarTexture_;
    sf::Texture healthbarBackgroundTexture_;
    sf::Font font_;

    sf::Text textRenderer_;
    bool drawPhysics_ = false;

    /**
     * \brief this is an indicator number used to set the ball visible when the start counter ends
     * 0 -> the ball is not visible
     * greater than 0 (1,2,3 ...) -> the ball is visible
     */
    int ballVisibilityIndicator_ = 0;
};
}
