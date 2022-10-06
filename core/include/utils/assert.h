/**
 * \file assert.h
 */
#pragma once

#include <cstdlib>
#include "utils/log.h"
#include <fmt/format.h>

namespace core
{

/**
 * \brief AssertException is an exception type used for the project assertation when the user need the application to close quietly. It is caught by the core::Engine in the game loop to close the update loop.
 *
 */
class AssertException final : public std::exception
{
public:
    AssertException(std::string_view msg) : msg_(msg) {}
    [[nodiscard]] const char* what() const noexcept override { return msg_.c_str(); }
private:
    std::string msg_;
};
}

#ifdef GPR_ASSERT
# ifdef GPR_ABORT
inline void gpr_assert(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogError(fmt::format("Assert failed:\t{}\nSource:\t\t{}, line {}", Msg, __FILE__, __LINE__));
        std::abort();
    }
}
#  ifdef GPR_ABORT_WARN

inline void gpr_warn(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogWarning(fmt::format("Warning Assert failed:\t{}\nSource:\t\t{}, line {}",
            Msg, __FILE__, __LINE__));
        std::abort();
    }
}

#  else

inline void gpr_warn(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogWarning(fmt::format("Warning Assert failed:\t{}\nSource:\t\t{}, line {}",
            Msg, __FILE__, __LINE__));
    }
}
#  endif
# else
inline void gpr_assert(bool Expr, std::string_view Msg)
{
    if (!(Expr)) 
    { 
        core::LogError(fmt::format("Assert failed:\t{}\nSource:\t\t{}, line {}", Msg, __FILE__, __LINE__)); 
        throw core::AssertException(Msg); 
    }
}
#  ifdef GPR_ABORT_WARN

inline void gpr_warn(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogWarning(fmt::format("Warning Assert failed:\t{}\nSource:\t\t{}, line {}",
            Msg, __FILE__, __LINE__));
        throw core::AssertException(Msg);
    }
}

#  else

inline void gpr_warn(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogWarning(fmt::format("Warning Assert failed:\t{}\nSource:\t\t{}, line {}",
            Msg, __FILE__, __LINE__));
    }
}
#  endif
# endif
#else
inline void gpr_assert(bool Expr, std::string_view Msg)
{
    
}
inline void gpr_warn(bool Expr, std::string_view Msg)
{
    
}
#endif

