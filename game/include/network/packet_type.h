/**
 * \file packet_type.h
 */
#pragma once

#include <SFML/Network/Packet.hpp>

#include "game/game_globals.h"
#include <memory>
#include <chrono>

namespace game
{
enum class PacketType : std::uint8_t
{
    JOIN = 0u,
    SPAWN_PLAYER,
    INPUT,
    SPAWN_BULLET,
    VALIDATE_STATE,
    START_GAME,
    JOIN_ACK,
    WIN_GAME,
    PING,
    NONE,
};

/**
 * \brief PhysicsState is the type of the physics state checksum
 */
using PhysicsState = std::uint16_t;

/**
 * \brief Packet is a interface that defines what a packet with a PacketType.
 */
struct Packet
{
    virtual ~Packet() = default;
    PacketType packetType = PacketType::NONE;
};

inline sf::Packet& operator<<(sf::Packet& packetReceived, Packet& packet)
{
    const auto packetType = static_cast<std::uint8_t>(packet.packetType);
    packetReceived << packetType;
    return packetReceived;
}

inline sf::Packet& operator>>(sf::Packet& packetReceived, Packet& packet)
{
    std::uint8_t packetType;
    packetReceived >> packetType;
    packet.packetType = static_cast<PacketType>(packetType);
    return packetReceived;
}

/**
 * \brief TypedPacket is a template class that sets the packetType of Packet automatically at construction with the given type.
 * \tparam type is the PacketType of the packet
 */
template<PacketType type>
struct TypedPacket : Packet
{
    TypedPacket() { packetType = type; }
};

template<typename T, size_t N>
sf::Packet& operator<<(sf::Packet& packet, const std::array<T, N>& t)
{
    for (auto& tmp : t)
    {
        packet << tmp;
    }
    return packet;
}

template <typename T, size_t N>
sf::Packet& operator>>(sf::Packet& packet, std::array<T, N>& t)
{
    for (auto& tmp : t)
    {
        packet >> tmp;
    }
    return packet;
}

/**
 * \brief JoinPacket is a TCP Packet that is sent by a client to the server to join a game.
 */
struct JoinPacket : TypedPacket<PacketType::JOIN>
{
    std::array<std::uint8_t, sizeof(ClientId)> clientId{};
    std::array<std::uint8_t, sizeof(unsigned long)> startTime{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const JoinPacket& joinPacket)
{
    return packet << joinPacket.clientId << joinPacket.startTime;
}

inline sf::Packet& operator>>(sf::Packet& packet, JoinPacket& joinPacket)
{
    return packet >> joinPacket.clientId >> joinPacket.startTime;
}

/**
 * \brief JoinAckPacket is a TCP Packet that is sent by the server to the client to answer a join packet
 */
struct JoinAckPacket : TypedPacket<PacketType::JOIN_ACK>
{
    std::array<std::uint8_t, sizeof(ClientId)> clientId{};
    std::array<std::uint8_t, sizeof(unsigned short)> udpPort{};
};


inline sf::Packet& operator<<(sf::Packet& packet, const JoinAckPacket& spawnPlayerPacket)
{
    return packet << spawnPlayerPacket.clientId << spawnPlayerPacket.udpPort;
}

inline sf::Packet& operator>>(sf::Packet& packet, JoinAckPacket& joinPacket)
{
    return packet >> joinPacket.clientId >> joinPacket.udpPort;
}

/**
 * \brief SpawnPlayerPacket is a TCP Packet sent by the server to all clients to notify of the spawn of a new player
 */
struct SpawnPlayerPacket : TypedPacket<PacketType::SPAWN_PLAYER>
{
    std::array<std::uint8_t, sizeof(ClientId)> clientId{};
    PlayerNumber playerNumber = INVALID_PLAYER;
    std::array<std::uint8_t, sizeof(core::Vec2f)> pos{};
    std::array<std::uint8_t, sizeof(core::Degree)> angle{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const SpawnPlayerPacket& spawnPlayerPacket)
{
    return packet << spawnPlayerPacket.clientId << spawnPlayerPacket.playerNumber <<
        spawnPlayerPacket.pos << spawnPlayerPacket.angle;
}

inline sf::Packet& operator>>(sf::Packet& packet, SpawnPlayerPacket& spawnPlayerPacket)
{
    return packet >> spawnPlayerPacket.clientId >> spawnPlayerPacket.playerNumber >>
        spawnPlayerPacket.pos >> spawnPlayerPacket.angle;
}

    
/**
 * \brief PlayerInputPacket is a UDP Packet sent by the player client and then replicated by the server to all clients to share the currentFrame
 * and all the previous ones player inputs.
 */
struct PlayerInputPacket : TypedPacket<PacketType::INPUT>
{
    PlayerNumber playerNumber = INVALID_PLAYER;
    std::array<std::uint8_t, sizeof(Frame)> currentFrame{};
    std::array<std::uint8_t, maxInputNmb> inputs{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const PlayerInputPacket& playerInputPacket)
{
    return packet << playerInputPacket.playerNumber <<
        playerInputPacket.currentFrame << playerInputPacket.inputs;
}

inline sf::Packet& operator>>(sf::Packet& packet, PlayerInputPacket& playerInputPacket)
{
    return packet >> playerInputPacket.playerNumber >>
        playerInputPacket.currentFrame >> playerInputPacket.inputs;
}

/**
 * \brief StartGamePacket is a TCP Packet send by the server to start a game at a given time.
 */
struct StartGamePacket : TypedPacket<PacketType::START_GAME>
{
};

/**
 * \brief ValidateFramePacket is an UDP packet that is sent by the server to validate the last physics state of the world.
 */
struct ValidateFramePacket : TypedPacket<PacketType::VALIDATE_STATE>
{
    std::array<std::uint8_t, sizeof(Frame)> newValidateFrame{};
    std::array<std::uint8_t, sizeof(PhysicsState) * maxPlayerNmb> physicsState{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const ValidateFramePacket& validateFramePacket)
{
    return packet << validateFramePacket.newValidateFrame << validateFramePacket.physicsState;
}

inline sf::Packet& operator>>(sf::Packet& packet, ValidateFramePacket& ValidateFramePacket)
{
    return packet >> ValidateFramePacket.newValidateFrame >> ValidateFramePacket.physicsState;
}

/**
 * \brief WinGamePacket is a TCP Packet sent by the server to notify the clients that a certain player has won.
 */
struct WinGamePacket : TypedPacket<PacketType::WIN_GAME>
{
    PlayerNumber winner = INVALID_PLAYER;
};

inline sf::Packet& operator<<(sf::Packet& packet, const WinGamePacket& winGamePacket)
{
    return packet << winGamePacket.winner;
}

inline sf::Packet& operator>>(sf::Packet& packet, WinGamePacket& winGamePacket)
{
    return packet >> winGamePacket.winner;
}

/**
 * \brief PingPacket is an UDP Packet sent by the client to the server and resend by the server to measure the RTT between the client and the server.
 */
struct PingPacket : TypedPacket<PacketType::PING>
{
    std::array<std::uint8_t, sizeof(unsigned long long)> time{};
    std::array<std::uint8_t, sizeof(ClientId)> clientId{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const PingPacket& pingPacket)
{
    return packet << pingPacket.time << pingPacket.clientId;
}

inline sf::Packet& operator>>(sf::Packet& packet, PingPacket& pingPacket)
{
    return packet >> pingPacket.time >> pingPacket.clientId;
}

inline void GeneratePacket(sf::Packet& packet, Packet& sendingPacket)
{
    packet << sendingPacket;
    switch (sendingPacket.packetType)
    {
    case PacketType::JOIN:
    {
        const auto& packetTmp = static_cast<JoinPacket&>(sendingPacket);
        packet << packetTmp;
        break;
    }
    case PacketType::SPAWN_PLAYER:
    {
        const auto& packetTmp = static_cast<SpawnPlayerPacket&>(sendingPacket);
        packet << packetTmp;
        break;
    }
    case PacketType::INPUT:
    {
        const auto& packetTmp = static_cast<PlayerInputPacket&>(sendingPacket);
        packet << packetTmp;
        break;
    }
    case PacketType::VALIDATE_STATE:
    {
        const auto& packetTmp = static_cast<ValidateFramePacket&>(sendingPacket);
        packet << packetTmp;
        break;
    }
    case PacketType::START_GAME:
    {
        break;
    }
    case PacketType::JOIN_ACK:
    {
        const auto& packetTmp = static_cast<JoinAckPacket&>(sendingPacket);
        packet << packetTmp;
        break;
    }
    case PacketType::WIN_GAME:
    {
        const auto& packetTmp = static_cast<WinGamePacket&>(sendingPacket);
        packet << packetTmp;
        break;
    }
    case PacketType::PING:
    {
        const auto& packetTmp = static_cast<PingPacket&>(sendingPacket);
        packet << packetTmp;
        break;
    }

    default:
        break;
    }
}

inline std::unique_ptr<Packet> GenerateReceivedPacket(sf::Packet& packet)
{
    Packet packetTmp;
    packet >> packetTmp;
    switch (packetTmp.packetType)
    {
    case PacketType::JOIN:
    {
        auto joinPacket = std::make_unique<JoinPacket>();
        joinPacket->packetType = packetTmp.packetType;
        packet >> *joinPacket;
        return joinPacket;
    }
    case PacketType::SPAWN_PLAYER:
    {
        auto spawnPlayerPacket = std::make_unique<SpawnPlayerPacket>();
        spawnPlayerPacket->packetType = packetTmp.packetType;
        packet >> *spawnPlayerPacket;
        return spawnPlayerPacket;
    }
    case PacketType::INPUT:
    {
        auto playerInputPacket = std::make_unique<PlayerInputPacket>();
        playerInputPacket->packetType = packetTmp.packetType;
        packet >> *playerInputPacket;
        return playerInputPacket;
    }
    case PacketType::VALIDATE_STATE:
    {
        auto validateFramePacket = std::make_unique<ValidateFramePacket>();
        validateFramePacket->packetType = packetTmp.packetType;
        packet >> *validateFramePacket;
        return validateFramePacket;
    }
    case PacketType::START_GAME:
    {
        auto startGamePacket = std::make_unique<StartGamePacket>();
        startGamePacket->packetType = packetTmp.packetType;
        return startGamePacket;
    }
    case PacketType::JOIN_ACK:
    {
        auto joinAckPacket = std::make_unique<JoinAckPacket>();
        joinAckPacket->packetType = packetTmp.packetType;
        packet >> *joinAckPacket;
        return joinAckPacket;
    }
    case PacketType::WIN_GAME:
    {
        auto winGamePacket = std::make_unique<WinGamePacket>();
        winGamePacket->packetType = packetTmp.packetType;
        packet >> *winGamePacket;
        return winGamePacket;
    }
    case PacketType::PING:
    {
        auto pingPacket = std::make_unique<PingPacket>();
        pingPacket->packetType = packetTmp.packetType;
        packet >> *pingPacket;
        return pingPacket;
    }
    default:;
    }
    return nullptr;
}

/**
 * \brief PacketSenderInterface is a interface for any Server or Client who wants to send and receive packets
 */
class PacketSenderInterface
{
public:
    virtual ~PacketSenderInterface() = default;
    virtual void SendReliablePacket(std::unique_ptr<Packet> packet) = 0;
    virtual void SendUnreliablePacket(std::unique_ptr<Packet> packet) = 0;
};
}
