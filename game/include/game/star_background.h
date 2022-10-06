#pragma once
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include "graphics/graphics.h"


namespace game
{
/**
 * \brief StarBackground is a drawable object that draws a starfield on a screen.
 */
class StarBackground final : public core::DrawInterface
{
public:
    void Init();
    void Draw(sf::RenderTarget& renderTarget) override;
private:
    static constexpr std::size_t starCount = 1024;
    sf::VertexArray vertexArray_;
};

}
