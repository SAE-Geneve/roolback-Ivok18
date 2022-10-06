#pragma once

#include <SFML/Graphics/Color.hpp>
#include <cstdint>

namespace core
{
/**
 * \brief Color is a struct defining an RGBA color with 4 bytes
 */
struct Color
{
    std::uint8_t r = 0u;
    std::uint8_t g = 0u;
    std::uint8_t b = 0u;
    std::uint8_t a = 0u;
    constexpr Color() = default;
    constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255u) :
        r(red), g(green), b(blue), a(alpha) {}

    operator sf::Color() const { return { r, g, b, a }; }

    static constexpr Color red() { return { 255u,0u,0u,255u }; }
    static constexpr Color green() { return { 0u,255u,0u,255u }; }
    static constexpr Color blue() { return { 0u,0u,255u,255u }; }
    static constexpr Color yellow() { return { 255u,255u,0u,255u }; }
    static constexpr Color black() { return { 0u,0u,0u,255u }; }
    static constexpr Color white() { return { 255u,255u,255u,255u }; }
    static constexpr Color magenta() { return { 255u,0u,255u,255u }; }
    static constexpr Color cyan() { return { 0u,255u,255u,255u }; }
    static constexpr Color transparent() { return { 0u,0u,0u,0u }; }
};

} // namespace core
