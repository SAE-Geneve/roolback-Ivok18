/**
 * \file game_globals.h
 */

#pragma once
#include <SFML/Graphics/Color.hpp>
#include <array>

#include "engine/component.h"
#include "engine/entity.h"
#include "graphics/color.h"
#include "maths/angle.h"
#include "maths/vec2.h"
#include "maths/basic.h"


namespace game
{
/**
 * \brief PlayerNumber is a type used to define the number of the player.
 * Starting from 0 to maxPlayerNmb
 */
using PlayerNumber = std::uint8_t;
/**
 * \brief INVALID_PLAYER is an integer constant that defines an invalid player number.
 */
constexpr auto INVALID_PLAYER = std::numeric_limits<PlayerNumber>::max();
/**
 * \brief ClientId is a type used to define the client identification.
 * It is given by the server to clients.
 */
enum class ClientId : std::uint16_t {};
constexpr auto INVALID_CLIENT_ID = ClientId{ 0 };
using Frame = std::uint32_t;
/**
 * \brief mmaxPlayerNmb is a integer constant that defines the maximum number of player per game
 */

/**
 * brief boundaries global info
 */
constexpr core::Vec2f topBoundaryPos(0.f, 4.3f);
constexpr core::Vec2f bottomBoundaryPos(0.f, -4.3f);
constexpr float boundaryScaleX = 1000.f; //box scale, not transform scale
constexpr float boundaryScaleY = 1000.f; //box scale, not transform scale //O.O25f

/**
* brief homes global data 
*/
constexpr core::Vec2f leftHomePos(-9.5f, 0.f);
constexpr core::Vec2f rightHomePos(9.5f, 0.f);
constexpr float homeScaleX = 500.f; //box scale, not transform scale  //0.05f;
constexpr float homeScaleY = 4.25f; //box scale, not transform scale
/**
* brief healthbars global data
*/
constexpr  core::Vec2f leftHealthbarPos(-9.47f, 4.5f);
constexpr core::Vec2f rightHealthbarPos(7.67f, 4.5f);
/**
* brief players global data 
*/
constexpr std::uint32_t maxPlayerNmb = 2;
constexpr short playerMaxHealth = 5;
constexpr float playerSpeed = 200.0f;
constexpr core::Degree playerAngularSpeed = core::Degree(90.0f);
constexpr float playerShootingPeriod = 0.3f;
constexpr float playerScaleX = 0.1f; //box scale, not transform scale
constexpr float playerScaleY = 0.64f; //box scale, not transform scale
constexpr float playerHurtPeriod = 1.15f; 
constexpr float playeHurtFlashPeriod = 0.25f;

/**
* brief ball global data 
*/
constexpr float ballMaxSpeed = 25.f;
constexpr float ballInitialSpeed = 3.f;
constexpr float ballRatioSpeedIncrease = 1.15f;
constexpr float ballScale = 0.75f; //transform scale only
constexpr core::Color ballStartColor = core::Color::transparent();
constexpr float ballLeftRespawnX = -8.5f;
constexpr float ballRightRespawnX = 8.5f;
//constexpr float bulletPeriod = 3.0f;

/**
 * \brief windowBufferSize is the size of input stored by a client. 5 seconds of frame at 50 fps
 */
constexpr std::size_t windowBufferSize = 5u * 50u;

/**
 * \brief startDelay is the delay to wait before starting a game in milliseconds
 */
constexpr long long startDelay = 3000;
/**
 * \brief maxInputNmb is the number of inputs stored into an PlayerInputPacket
 */
constexpr std::size_t maxInputNmb = 50;
/**
 * \brief fixedPeriod is the period used in seconds to start a new FixedUpdate method in the game::GameManager
 */
constexpr float fixedPeriod = 0.02f; //50fps


constexpr std::array<core::Color, std::max(4u, maxPlayerNmb)> playerColors
{
    core::Color::blue(),
    core::Color::red(),
    core::Color::yellow(),
    core::Color::cyan()
};

constexpr std::array<core::Vec2f, std::max(4u, maxPlayerNmb)> spawnPositions
{
    core::Vec2f(-3.f,0),
    core::Vec2f(3.f,0),
    core::Vec2f(0,0), 
    core::Vec2f(0,0), 
};

constexpr std::array<core::Degree, std::max(4u, maxPlayerNmb)> spawnRotations
{
    core::Degree(0.0f),
    core::Degree(0.0f),
    core::Degree(-90.0f),
    core::Degree(90.0f)
};

enum class ComponentType : core::EntityMask
{
    PLAYER_CHARACTER = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE),
    BALL = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 1u,
    ASTEROID = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 2u,
    PLAYER_INPUT = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 3u,
    DESTROYED = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 4u,
    BOUNDARY = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 5u,
    HOME = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 6u,
    HEALTHBAR = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 7u
};

/**
 * \brief PlayerInput is a type defining the input data from a player.
 */
using PlayerInput = std::uint8_t;

namespace PlayerInputEnum
{
enum PlayerInput : std::uint8_t
{
    NONE = 0u,
    UP = 1u << 0u,
    DOWN = 1u << 1u,
    LEFT = 1u << 2u,
    RIGHT = 1u << 3u,
    SHOOT = 1u << 4u,
};
}
}
