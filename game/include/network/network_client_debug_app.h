#pragma once
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "client.h"
#include "network_client.h"
#include "engine/app.h"
#include "game/game_globals.h"


namespace game
{
/**
 * \brief NetworkClientDebugApp is a class that have several NetworkClient and shows them on the screen, each in their own RenderTexture.
 * It allows to debug network apps as easily as the SimulationApp.
 */
class NetworkClientDebugApp final : public core::App
{
public:

    void Begin() override;

    void Update(sf::Time dt) override;

    void End() override;

    void DrawImGui() override;

    void Draw(sf::RenderTarget& renderTarget) override;

    void OnEvent(const sf::Event& event) override;
private:
    std::array<NetworkClient, maxPlayerNmb> clients_;
    std::array<sf::RenderTexture, maxPlayerNmb> clientsFramebuffers_;
    sf::Sprite screenQuad_;
    sf::Vector2u windowSize_;
};


}
