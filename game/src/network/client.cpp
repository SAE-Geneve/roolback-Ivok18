#include "network/client.h"

#include "maths/basic.h"
#include "utils/assert.h"
#include "utils/conversion.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
void Client::ReceivePacket(const Packet* packet)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto packetType = packet->packetType;
    switch (packetType)
    {
    case PacketType::SPAWN_PLAYER:
    {
        const auto* spawnPlayerPacket = static_cast<const SpawnPlayerPacket*>(packet);
        const auto clientId = core::ConvertFromBinary<ClientId>(spawnPlayerPacket->clientId);

        const PlayerNumber playerNumber = spawnPlayerPacket->playerNumber;
        if (clientId == clientId_)
        {
            gameManager_.SetClientPlayer(playerNumber);
        }

        const auto pos = core::ConvertFromBinary<core::Vec2f>(spawnPlayerPacket->pos);
        const auto rotation = core::ConvertFromBinary<core::Degree>(spawnPlayerPacket->angle);

        gameManager_.SpawnPlayer(playerNumber, pos, rotation);
        break;
    }
    case PacketType::START_GAME:
    {
        core::LogDebug("Start Game Packet Received");
        using namespace std::chrono;
        const auto startingTime = (duration_cast<duration<long long, std::milli>>(
            system_clock::now().time_since_epoch()
            ) + milliseconds(startDelay)).count() - milliseconds(static_cast<long long>(currentPing_)).count();

        gameManager_.StartGame(startingTime);
        break;
    }
    case PacketType::INPUT:
    {
        const auto* playerInputPacket = static_cast<const PlayerInputPacket*>(packet);
        const auto playerNumber = playerInputPacket->playerNumber;
        const auto inputFrame = core::ConvertFromBinary<Frame>(playerInputPacket->currentFrame);

        if (playerNumber == gameManager_.GetPlayerNumber())
        {
            //Verify the inputs coming back from the server
            const auto& inputs = gameManager_.GetRollbackManager().GetInputs(playerNumber);
            const auto currentFrame = gameManager_.GetRollbackManager().GetCurrentFrame();
            for (size_t i = 0; i < playerInputPacket->inputs.size(); i++)
            {
                const auto index = currentFrame - inputFrame + i;
                if (index > inputs.size())
                {
                    break;
                }
                if (inputs[index] != playerInputPacket->inputs[i])
                {
                    gpr_assert(false, "Inputs coming back from server are not coherent!!!");
                }
                if (inputFrame - i == 0)
                {
                    break;
                }
            }
            break;
        }

        //discard delayed input packet
        if (inputFrame < gameManager_.GetRollbackManager().GetLastReceivedFrame(playerNumber))
        {
            break;
        }
        for (Frame i = 0; i < playerInputPacket->inputs.size(); i++)
        {
            gameManager_.SetPlayerInput(playerNumber,
                playerInputPacket->inputs[i],
                inputFrame - i);

            if (inputFrame - i == 0)
            {
                break;
            }
        }
        break;
    }
    case PacketType::VALIDATE_STATE:
    {
        const auto* validateFramePacket = static_cast<const ValidateFramePacket*>(packet);
        const auto newValidateFrame = core::ConvertFromBinary<Frame>(validateFramePacket->newValidateFrame);
        std::array<PhysicsState, maxPlayerNmb> physicsStates{};
        for (size_t i = 0; i < validateFramePacket->physicsState.size(); i++)
        {
            auto* statePtr = reinterpret_cast<std::uint8_t*>(physicsStates.data());
            statePtr[i] = validateFramePacket->physicsState[i];
        }
        gameManager_.ConfirmValidateFrame(newValidateFrame, physicsStates);
        //logDebug("Client received validate frame " + std::to_string(newValidateFrame));
        break;
    }
    case PacketType::WIN_GAME:
    {
        const auto* winGamePacket = static_cast<const WinGamePacket*>(packet);
        gameManager_.WinGame(winGamePacket->winner);
        break;
    }
    case PacketType::PING:
    {
        const auto* pingPacket = static_cast<const PingPacket*>(packet);
        const auto clientId = core::ConvertFromBinary<ClientId>(pingPacket->clientId);
        if (clientId == clientId_)
        {
            const auto originTime = core::ConvertFromBinary<unsigned long long>(pingPacket->time);
            using namespace std::chrono;
            const auto currentTime = duration_cast<duration<unsigned long long, std::milli>>(
                system_clock::now().time_since_epoch()
                ).count();
            const auto delta = currentTime - originTime;
            const auto ping = static_cast<float>(delta);

            //calculate average and var ping
            if (srtt_ < 0.0f)
            {
                srtt_ = ping;
                rttvar_ = ping / 2.0f;
            }
            else
            {
                srtt_ = (1.0f - alpha) * srtt_ + alpha * ping;
                rttvar_ = (1.0f - beta) * rttvar_ + beta * core::Abs(srtt_ - ping);
            }

            rto_ = srtt_ + std::max(g, k * rttvar_);
            currentPing_ = srtt_;
        }
        break;

    }
    case PacketType::SPAWN_BALL:
    {
        const auto* spawnBallPacket = static_cast<const SpawnBallPacket*>(packet);
        const auto pos = core::ConvertFromBinary<core::Vec2f>(spawnBallPacket->pos);
        const auto velocity = core::ConvertFromBinary<core::Vec2f>(spawnBallPacket->velocity);

        gameManager_.SpawnBall(pos, velocity);
        break;
    }
    case PacketType::SPAWN_BOUNDARY:
    {
        const auto* spawnBoundaryPacket = static_cast<const SpawnBoundaryPacket*>(packet);
        const auto pos = core::ConvertFromBinary<core::Vec2f>(spawnBoundaryPacket->pos);

        gameManager_.SpawnBoundary(pos);
        break;
    }
    case PacketType::SPAWN_HOME:
    {
        const auto* spawnHomePacket = static_cast<const SpawnHomePacket*>(packet);
        const PlayerNumber playerNumber = spawnHomePacket->playerNumber;
        const auto pos = core::ConvertFromBinary<core::Vec2f>(spawnHomePacket->pos);
        core::LogDebug("Spawn Home");
        gameManager_.SpawnHome(playerNumber, pos);
        break;
    }
    default:;
    }
}

void Client::Update(sf::Time dt)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    pingTimer_ -= dt.asSeconds();
    if (pingTimer_ < 0.0f)
    {
        if (clientId_ != INVALID_CLIENT_ID)
        {
            using namespace std::chrono;
            auto pingPacket = std::make_unique<PingPacket>();
            pingPacket->time = core::ConvertToBinary(duration_cast<duration<unsigned long long, std::milli>>(
                system_clock::now().time_since_epoch()).count());
            pingPacket->clientId = core::ConvertToBinary(clientId_);
            SendUnreliablePacket(std::move(pingPacket));
        }
        pingTimer_ = pingPeriod_;
    }
}
}
