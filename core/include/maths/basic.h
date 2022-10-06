/**
 * \file basic.h
 */
#pragma once

#include <random>

namespace core
{
/**
 * \brief Abs is a constexpr function that returns the absolute value.
 * \param v is the value that will be transform to absolute value
 * \return the absolute value of v
 */
constexpr float Abs(float v)
{
    return v < 0.0f ? -v : v;
}

constexpr  bool Equal(float a, float b, float epsilon = 0.0000001f)
{
    return Abs(a - b) < epsilon;
}

/**
 * \brief Lerp is a function calculating the linear interpolation between two points.
 * \param start is the lower value of the interpolation.
 * \param end is the upper value of the interpolation.
 * \param t is the ratio of the interpolation.
 * t = 0, return start
 * t = 1, return end
 * t > 1 or t < 0, return an extrapolation of the linear function
 * \return the linear interpolation result
 */
constexpr float Lerp(float start, float end, float t)
{
    return start + (end - start) * t;
}

/**
 * \brief Clamp is a math function that clamps the input value between an upper and lower value.
 * \tparam T is the inputs and outputs type
 * \param value is the input value that will be clamped
 * \param lower is the lower bound of the clamp
 * \param upper is the upper bound of the clamp
 * \return the clamped value between lower and upper
 */
template<typename T>
constexpr float Clamp(T value, T lower, T upper)
{
    return value < lower ? lower : (value > upper ? upper : value);
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type RandomRange(T start, T end)
{
    static std::random_device rd;  //Will be used to obtain a seed for the random number engine
    static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<T> dis(start, end);
    return dis(gen);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type RandomRange(T start, T end)
{
    static std::random_device rd;  //Will be used to obtain a seed for the random number engine
    static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<T> dis(start, end);
    return dis(gen);
}
template<typename T>
T constexpr SqrtNewtonRaphson(T x, T curr, T prev)
{
    return curr == prev
           ? curr
           : SqrtNewtonRaphson<T>(x, (curr + x / curr) * 0.5, curr);
}


/**
* \brief Sqrt is a constexpr version of the square root
* \tparam T is the input and output value type
* \param x is the input value
* \return 
*   - For a finite and non-negative value of "x", returns an approximation for the square root of "x"
*   - Otherwise, returns NaN
*/
template<typename T>
T constexpr Sqrt(T x)
{
    return x >= 0 && x < std::numeric_limits<T>::infinity()
           ? SqrtNewtonRaphson<T>(x, x, 0)
           : std::numeric_limits<T>::quiet_NaN();
}
}