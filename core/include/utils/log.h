#pragma once

#include <string_view>

namespace core
{
/**
 * \brief LogDebug is a function that prints the msg to the console
 * \param msg is the text to be printed
 */
void LogDebug(std::string_view msg);
/**
 * \brief LogWarning is a function that prints a yellow message to the console
 * \param msg is the text to be printed
 */
void LogWarning(std::string_view msg);
/**
 * \brief LogError is a function that prints a red message to the console
 * \param msg is the text to be printed
 */
void LogError(std::string_view msg);
}