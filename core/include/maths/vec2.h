#pragma once

#include <SFML/System/Vector2.hpp>
#include <maths/angle.h>

namespace core
{
/**
 * \brief Vec2f is a utility class that represents a mathematical 2d vector.
 */
struct Vec2f
{
    float x = 0.0f, y = 0.0f;

    constexpr Vec2f() = default;
    constexpr Vec2f(float newX, float newY) : x(newX), y(newY)
    {

    }
    Vec2f(sf::Vector2f v);


    [[nodiscard]] float GetMagnitude() const;
    void Normalize();
    [[nodiscard]] Vec2f GetNormalized() const;
    [[nodiscard]] float GetSqrMagnitude() const;
    [[nodiscard]] Vec2f Rotate(Degree rotation) const;
    static float Dot(Vec2f a, Vec2f b);
    static Vec2f Lerp(Vec2f a, Vec2f b, float t);

    [[nodiscard]] operator sf::Vector2f() const { return { x, y }; }

    Vec2f operator+(Vec2f v) const;
    Vec2f& operator+=(Vec2f v);
    Vec2f operator-(Vec2f v) const;
    Vec2f& operator-=(Vec2f v);
    Vec2f operator*(float f) const;
    Vec2f operator/(float f) const;

    static constexpr Vec2f zero() { return {}; }
    static constexpr Vec2f one() { return {1,1}; }
    static constexpr Vec2f up() { return {0,1}; }
    static constexpr Vec2f down() { return {0,-1}; }
    static constexpr Vec2f left() { return {-1,0}; }
    static constexpr Vec2f right() { return {1,0}; }
};

Vec2f operator*(float f, Vec2f v);

}