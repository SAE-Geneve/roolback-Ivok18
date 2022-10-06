#pragma once

#include <type_traits>

namespace core
{
/**
 * \brief Locator is an utility class based on the Locator design-patter.
 * \tparam T is the type of the wanted class
 * \tparam TNull is the type of the wanted class, but when it's empty
 */
template<class T, class TNull>
class Locator
{
    static_assert(std::is_base_of<T, TNull>::value, "TNull has to be a subtype of T");
public:
    /**
     * \brief get is a method that gets you the reference to the T class.
     * If a pointer was not provided, it returns a pointer to the TNull nullService_ attribute.
     * \return a reference to the T class service.
     */
    static T& get()
    {
        return *service_;
    }

    /**
     * \brief provide is a method that allows to provide a pointer to a class T object.
     * \param service is the given pointer. If null, it is replaced by the nullService_ pointer.
     */
    static void provide(T* service)
    {
        if (service == nullptr)
        {
            // Revert to null service.
            service_ = &nullService_;
        } else
        {
            service_ = service;
        }
    }

protected:
    static T* service_;
    static TNull nullService_;
};

template<class T, class TNull> TNull Locator<T, TNull>::nullService_;
template<class T, class TNull> T* Locator<T, TNull>::service_ = &Locator<T, TNull>::nullService_;


}
