
/**
 * Copyright 2015 Denis Blank <denis.blank@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <functional>
#include <tuple>
#include <memory>

template <typename...>
struct Continuable;

template <typename... Args>
Continuable<> make_continuable(Args&&...);

template <typename...>
struct Continuable
{
    template <typename... Args>
    Continuable then(Args&&...)
    {
        return Continuable();
    }

    Continuable& wrap(std::shared_ptr<std::function<void(std::function<void()>&&)>> dispatcher)
    {
        return *this;
    }
};

// helper class
struct Continuables
{
    static Continuable<> create_empty()
    {
        return make_continuable([](std::function<void()>&& callback)
        {
            callback();
        });
    }
};

template <typename... Args>
Continuable<> make_continuable(Args&&...)
{
    return Continuable<>();
}

template <typename... LeftArgs, typename... RightArgs>
Continuable<> operator&& (Continuable<LeftArgs...>&&, Continuable<RightArgs...>&&)
{
    return Continuable<>();
}

template <typename... LeftArgs, typename... RightArgs>
Continuable<> operator|| (Continuable<LeftArgs...>&&, Continuable<RightArgs...>&&)
{
    return Continuable<>();
}

Continuable<> http_request(std::string const& url)
{
    return make_continuable([=](std::function<void(std::string)>&& callback)
    {
        // Do request...
        callback("some HTTP content");
    });
}

struct ResultSet { };

Continuable<> mysql_query(std::string const& query)
{
    return make_continuable([=](std::function<void(std::string)>&& callback)
    {
        // Do request...

        callback("a mysql query result");
    });
}

/// This mockup shows how to create a continuable from a functional object.
void create_continuable_mockup()
{
    /// Everything which accepts callbacks can be converted into a continuable.
    /// Since nearly every async operation can be converted to use callbacks
    {
        Continuable</*std::string*/> continuable =
            make_continuable([](std::function<void(std::string)>&& /*TODO Find out if its better to use call by value*/ callback)
        {
            callback("my result");
        });
    }

    // 
    {
        // Accept params from previous continuable...
        Continuable<> continuable = make_continuable([](int param1, int param2)
        {
            // ... and return the next continuable
            return mysql_query("some query content");
        });
    }
}

/// This mockup shows how to chain multiple continuables together
void chain_continuable_mockup()
{
    // Create continuation using `Continuable<>::then`
    {
        Continuable<> c1, c2;

        // Invalidates c1 & c2
        Continuable<> c1_then_c2 = c1.then(std::move(c2));
    }

    // Create conditions using `&&` (and `||` - nice to have feature)
    {
        Continuable<> c1, c2;

        // Invalidates c1 and c2
        Continuable<> c1_and_c2 = std::move(c1) && std::move(c2);
    }

    // Create conditions using `||` (nice to have feature - but not mandatory)
    {
        Continuable<> c1, c2;

        // Invalidates c1 and c2
        Continuable<> c1_or_c2 = std::move(c1) || std::move(c2);
    }
}

/// This mockup shows the basic usage and features of continuables waiting for 2 http requests and a database query.
void final_mockup()
{
    // Optional - Create a dispatcher where which dispatches the main chain.
    auto const my_dispatcher =
        std::make_shared<std::function<void(std::function<void()>&&)>>([](std::function<void()>&& function)
    {
        // Dispatch in same thread or pass to another one
        function();
    });

    Continuable<> c1, c2, c3, c4;

    (std::move(c1) && std::move(c2))
        .wrap(my_dispatcher)
        .then(http_request("https://github.com/") &&
              http_request("https://www.google.de/") &&
              mysql_query("SELECT name, session FROM users WHERE id = 3726284"))
        .then([](std::string github, std::string google, ResultSet user)
        {
             // result bla bla
            return std::make_tuple(github.empty(), std::move(google), std::move(user));
        })
        .then([](bool hasContent, std::string google, ResultSet user)
        {
        });
}

void test_mockup()
{
    create_continuable_mockup();
    chain_continuable_mockup();

    final_mockup();
}
