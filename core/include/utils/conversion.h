#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace core
{
/**
 * \brief ConvertFromBinary is an utility function that converts an array of bytes to the given T type.
 * \tparam T is the given type to convert to.
 * \param data is the array of bytes to be converted.
 * \return the converted data as T type.
 */
template<typename T>
T ConvertFromBinary(const std::array<std::uint8_t, sizeof(T)>& data)
{
    T result;
    auto* resultPtr = reinterpret_cast<std::uint8_t*>(&result);
    std::memcpy(resultPtr, data.data(), sizeof(T));
    return result;
}

/**
 * \brief ConvertTobinary is an utility function that converts a given T type data to an array of bytes.
 * \tparam T is the given type to convert from.
 * \param data is the given type data to be converted.
 * \return an array of bytes converted from the given type T data.
 */
template<typename T>
std::array<std::uint8_t, sizeof(T)> ConvertToBinary(T data)
{
    std::array<std::uint8_t, sizeof(T)> result{};
    const auto* dataPtr = reinterpret_cast<std::uint8_t*>(&data);
    std::memcpy(result.data(), dataPtr, sizeof(T));
    return result;

}
} // namespace core
