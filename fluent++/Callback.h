/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <functional>
#include <utility>
#include <memory>

#include "functional_unwrap.hpp"

template<typename... Args>
using Callback = std::function<void(Args...)>;

template<typename... Args>
using SharedCallback = std::shared_ptr<Callback<Args...>>;

template<typename... Args>
using WeakCallback = std::weak_ptr<Callback<Args...>>;

namespace detail
{
    template<typename... Args>
    struct do_unwrap_callback;

    template<typename... Args>
    struct do_unwrap_callback<std::tuple<Args...>>
    {
        typedef Callback<Args...> CallbackType;

        typedef SharedCallback<Args...> SharedCallbackType;

        typedef WeakCallback<Args...> WeakCallbackType;
    };

    template<typename _CTy>
    using unwrap_callback = do_unwrap_callback<::fu::argument_type_of_t<_CTy>>;

    template<typename... Args>
    struct WeakProxyFactory
    {
        static Callback<Args...> CreateProxy(WeakCallback<Args...> const& weak_callback)
        {
            return [=](Args&&... args)
            {
                if (auto const callback = weak_callback.lock())
                    (*callback)(args...);
            };
        }
    };

} // detail

template<typename _CTy>
using callback_of_t = typename detail::unwrap_callback<_CTy>::CallbackType;

template<typename _CTy>
using shared_callback_of_t = typename detail::unwrap_callback<_CTy>::SharedCallbackType;

template<typename _CTy>
using weak_callback_of_t = typename detail::unwrap_callback<_CTy>::WeakCallbackType;

template<typename _CTy>
inline shared_callback_of_t<_CTy>
    make_shared_callback(_CTy&& callback)
{
    return std::make_shared<callback_of_t<_CTy>>
        (std::forward<callback_of_t<_CTy>>(callback));
}

template<typename... Args>
inline auto make_weak_wrapped_callback(WeakCallback<Args...> const& wrapped_callback)
    -> Callback<Args...>
{
    return detail::WeakProxyFactory<Args...>::CreateProxy(wrapped_callback);
}

template<typename... Args>
inline auto make_weak_wrapped_callback(SharedCallback<Args...> const& wrapped_callback)
    -> Callback<Args...>
{
    return make_weak_wrapped_callback<Args...>(WeakCallback<Args...>(wrapped_callback));
}

#endif /// _CALLBACK_H_