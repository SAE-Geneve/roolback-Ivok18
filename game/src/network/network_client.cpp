#include <chrono>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <network/network_client.h>

#include "maths/basic.h"
#include "utils/conversion.h"
#include "utils/log.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{

void NetworkClient::Begin()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    clientId_ = ClientId{ core::RandomRange(std::numeric_limits<std::underlying_type_t<ClientId>>::lowest(),
                                  std::numeric_limits<std::underlying_type_t<ClientId>>::max()) };
    //JOIN packet
    gameManager_.Begin();
    tcpSocket_.setBlocking(false);
    udpSocket_.setBlocking(true);
    auto status = sf::Socket::Error;
    while (status != sf::Socket::Done)
    {
        status = udpSocket_.bind(sf::Socket::AnyPort);
    }
    udpSocket_.setBlocking(false);
#ifdef ENABLE_SQLITE
    debugDb_.Open(fmt::format("Client_{}.db", static_cast<unsigned>(clientId_)));
#endif
}

void NetworkClient::Update(sf::Time dt)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    Client::Update(dt);
    if (currentState_ != State::NONE)
    {
        auto status = sf::Socket::Done;
        //Receive TCP Packet
        while (status == sf::Socket::Done)
        {
            sf::Packet packet;
            status = tcpSocket_.receive(packet);
            switch (status)
            {
            case sf::Socket::Done:
                ReceiveNetPacket(packet, PacketSource::TCP);
                break;
            case sf::Socket::NotReady:
                //core::LogDebug("[Client] Error while receiving tcp socket is not ready");
                break;
            case sf::Socket::Partial:
                core::LogDebug("[Client] Error while receiving TCP packet, PARTIAL");
                break;
            case sf::Socket::Disconnected: break;
            case sf::Socket::Error: break;
            default: break;
            }
        }
        //Receive UDP packet
        status = sf::Socket::Done;
        while (status == sf::Socket::Done)
        {
            sf::Packet packet;
            sf::IpAddress sender;
            unsigned short port;
            status = udpSocket_.receive(packet, sender, port);
            switch (status)
            {
            case sf::Socket::Done:
                ReceiveNetPacket(packet, PacketSource::UDP);
                break;
            case sf::Socket::NotReady: break;
            case sf::Socket::Partial:
                core::LogDebug("[Client] Error while receiving UDP packet, PARTIAL");
                break;
            case sf::Socket::Disconnected:
                core::LogDebug("[Client] Error while receiving UDP packet, DISCONNECTED");
                break;
            case sf::Socket::Error:
                core::LogDebug("[Client] Error while receiving UDP packet, ERROR");
                break;
            default:;
            }
        }
        switch (currentState_)
        {
        case State::JOINING:
        {
            if (serverUdpPort_ != 0)
            {
                //Need to send a join packet on the unreliable channel
                auto joinPacket = std::make_unique<JoinPacket>();
                joinPacket->clientId = core::ConvertToBinary<ClientId>(clientId_);
                SendUnreliablePacket(std::move(joinPacket));
            }
            break;
        }
        default:
            break;
        }
    }

    gameManager_.Update(dt);
}

void NetworkClient::End()
{
    gameManager_.End();

#ifdef ENABLE_SQLITE
    debugDb_.Close();
#endif

}

void NetworkClient::DrawImGui()
{
    const auto windowName = "Client " + std::to_string(static_cast<unsigned>(clientId_));
    ImGui::Begin(windowName.c_str());

    if (srtt_ > 0.0f)
    {
        ImGui::Text("SRTT: %f", srtt_);
        ImGui::Text("RTTVAR: %f", rttvar_);
        ImGui::Text("RTO: %f", rto_);
    }


    ImGui::InputText("Host", &serverAddress_);

    int portBuffer = serverTcpPort_;
    if (ImGui::InputInt("Port", &portBuffer))
    {
        serverTcpPort_ = static_cast<unsigned short>(portBuffer);
    }
    if (currentState_ == State::NONE &&
        ImGui::Button("Join"))
    {
        tcpSocket_.setBlocking(true);
        const auto status = tcpSocket_.connect(serverAddress_, serverTcpPort_);
        tcpSocket_.setBlocking(false);
        if (status == sf::Socket::Done)
        {
            core::LogDebug("[Client] Connect to server " + serverAddress_ + " with port: " + std::to_string(serverTcpPort_));
            auto joinPacket = std::make_unique<JoinPacket>();
            joinPacket->clientId = core::ConvertToBinary<ClientId>(clientId_);
            using namespace std::chrono;
            const unsigned long clientTime = static_cast<unsigned long>((duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count());
            joinPacket->startTime = core::ConvertToBinary<unsigned long>(clientTime);
            SendReliablePacket(std::move(joinPacket));
            currentState_ = State::JOINING;
        }
        else
        {
            core::LogError("[Client] Error trying to connect to " + serverAddress_ + " with port: " +
                std::to_string(serverTcpPort_) + " with status: " + std::to_string(status));
        }
    }
    ImGui::Text("Server UDP port: %u", serverUdpPort_);
    gameManager_.DrawImGui();
    ImGui::End();
}

void NetworkClient::Draw(sf::RenderTarget& renderTarget)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    gameManager_.Draw(renderTarget);
}

void NetworkClient::SendReliablePacket(std::unique_ptr<Packet> packet)
{

    //core::LogDebug("[Client] Sending reliable packet to server");
    sf::Packet tcpPacket;
    GeneratePacket(tcpPacket, *packet);
    auto status = sf::Socket::Partial;
    while (status == sf::Socket::Partial)
    {
        status = tcpSocket_.send(tcpPacket);
    }
}

void NetworkClient::SendUnreliablePacket(std::unique_ptr<Packet> packet)
{

    if (currentState_ == State::NONE)
    {
        return;
    }
    sf::Packet udpPacket;
    GeneratePacket(udpPacket, *packet);
    const auto status = udpSocket_.send(udpPacket, serverAddress_, serverUdpPort_);
    switch (status)
    {
    case sf::Socket::Done:
        //core::LogDebug("[Client] Sending UDP packet to server at host: " +
        //	serverAddress_.toString() + " port: " + std::to_string(serverUdpPort_));
        break;
    case sf::Socket::NotReady:
        core::LogDebug("[Client] Error sending UDP to server, NOT READY");
        break;
    case sf::Socket::Partial:
        core::LogDebug("[Client] Error sending UDP to server, PARTIAL");
        break;
    case sf::Socket::Disconnected:
        core::LogDebug("[Client] Error sending UDP to server, DISCONNECTED");
        break;
    case sf::Socket::Error:
        core::LogDebug("[Client] Error sending UDP to server, ERROR");
        break;
    default:
        break;
    }
}

void NetworkClient::SetPlayerInput(PlayerInput playerInput)
{
    const auto currentFrame = gameManager_.GetCurrentFrame();
    gameManager_.SetPlayerInput(
        gameManager_.GetPlayerNumber(),
        playerInput,
        currentFrame);
}

void NetworkClient::ReceivePacket(const Packet* packet)
{
    Client::ReceivePacket(packet);
#ifdef ENABLE_SQLITE
    switch (packet->packetType)
    {
    case PacketType::JOIN: break;
    case PacketType::SPAWN_PLAYER: break;
    case PacketType::INPUT:
    {
        auto* inputPacket = static_cast<const PlayerInputPacket*>(packet);
        debugDb_.StorePacket(inputPacket);
        break;
    }
    case PacketType::SPAWN_BULLET: break;
    case PacketType::VALIDATE_STATE:
    {
        auto* validateStatePacket = static_cast<const ValidateFramePacket*>(packet);
        const auto newValidateFrame = core::ConvertFromBinary<Frame>(validateStatePacket->newValidateFrame);
        DbPhysicsState state{};
        state.validateFrame = newValidateFrame;
        state.lastLocalValidateFrame = gameManager_.GetLastValidateFrame();
        for (size_t i = 0; i < validateStatePacket->physicsState.size(); i++)
        {
            auto* statePtr = reinterpret_cast<std::uint8_t*>(state.serverStates.data());
            statePtr[i] = validateStatePacket->physicsState[i];
        }
        for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
        {
            state.localStates[playerNumber] = gameManager_.GetRollbackManager().GetValidatePhysicsState(playerNumber);
        }
        debugDb_.StorePhysicsState(state);
        break;
    }
    case PacketType::START_GAME: break;
    case PacketType::JOIN_ACK: break;
    case PacketType::WIN_GAME: break;
    case PacketType::PING: break;
    case PacketType::NONE: break;
    default:;
    }
#endif
}

void NetworkClient::ReceiveNetPacket(sf::Packet& packet, PacketSource source)
{
    const auto receivePacket = GenerateReceivedPacket(packet);
    Client::ReceivePacket(receivePacket.get());
    switch (receivePacket->packetType)
    {
    case PacketType::JOIN_ACK:
    {
        core::LogDebug("[Client] Receive " + std::string(source == PacketSource::UDP ? "UDP" : "TCP") + " Join ACK Packet");
        auto* joinAckPacket = static_cast<JoinAckPacket*>(receivePacket.get());

        serverUdpPort_ = core::ConvertFromBinary<unsigned short>(joinAckPacket->udpPort);
        const auto clientId = core::ConvertFromBinary<ClientId>(joinAckPacket->clientId);
        if (clientId != clientId_)
            return;
        if (source == PacketSource::TCP)
        {
            //Need to send a join packet on the unreliable channel
            auto joinPacket = std::make_unique<JoinPacket>();
            joinPacket->clientId = core::ConvertToBinary<ClientId>(clientId_);
            SendUnreliablePacket(std::move(joinPacket));
        }
        else
        {
            if (currentState_ == State::JOINING)
            {
                currentState_ = State::JOINED;
            }
        }
        break;
    }
    default:
        break;
    }
}
}
