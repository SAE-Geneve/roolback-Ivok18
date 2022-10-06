#pragma once
#include "network/network_client.h"
#include "engine/app.h"

namespace game
{
/**
 * \brief ClientApp is a App that owns a NetworkClient and shows it on the screen directly.
 */
class ClientApp final : public core::App
{
public:
    void Begin() override;

    void Update(sf::Time dt) override;

    void End() override;

    void DrawImGui() override;

    void OnEvent(const sf::Event& event) override;

    void Draw(sf::RenderTarget& window) override
    {
        client_.Draw(window);
    }

private:
    sf::Vector2u windowSize_;
    NetworkClient client_;
};
} // namespace game
