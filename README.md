<p align="center">
  <img src="./res/img/vkbot.png" alt="VKBOT Banner" width="600" height="200"/>
</p>
<p align="left">
  <a href="https://github.com/jetfire-oldsmobile27/vkbot/actions">
    <img src="https://img.shields.io/badge/build-passing-brightgreen?style=flat-square" alt="Build Status">
  </a>
  <a href="LICENSE">
    <img src="https://img.shields.io/badge/license-Apache%202.0-orange?style=flat-square" alt="License">
  </a>
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue?style=flat-square&logo=c%2B%2B" alt="C++17">
  <img src="https://img.shields.io/badge/Boost-1.83+-green?style=flat-square&logo=boost" alt="Boost">
  <img src="https://img.shields.io/badge/Conan-2.25-cyan?style=flat-square&logo=conan" alt="Conan">
</p>

# VKBOT (boost-based)

 Переработка оригинальной библиотеки [qucals/VKAPI](https://github.com/qucals/VK-API). Убраны все сырые указатели и CURL,
транспортный слой переведён на **Boost.Beast + Boost.Asio**.

## Сборка с примерами через Conan + CMake

```bash
# 1. Установить зависимости
conan install . --output-folder=build --build=missing -s build_type=Release

# 2. Сконфигурировать
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake

# 3. Собрать
cmake --build build --config Release -j$(nproc)

# 4. Установить (опционально)
cmake --install build --prefix /usr/local
```

## Быстрый старт — бот

```cpp
#include <vkbot/BotBase.hpp>

vk::bot::BotBase bot("your_group_id");
bot.auth("your_token");

while (true) {
    auto [type, payload] = bot.wait_for_event();
    if (type == vk::bot::BotBase::Event::MessageNew) {
        // обработка
    }
}
```

## Быстрый старт — пользователь

```cpp
#include <vkbot/UserBase.hpp>

vk::user::UserBase user("app_id", "app_secret");
user.auth("access_token");

auto resp = user.send_request(vk::user::UserBase::Method::UsersGet, {});
```

## Асинхронные запросы (бот)

```cpp
auto future = bot.send_request_async(
    vk::bot::BotBase::Method::SendMessage,
    {{"peer_id", "123"}, {"message", "hello"}, {"random_id", "0"}}
);
auto result = future.get();
```

## ✍️ Авторы

- [@qucals](https://github.com/qucals) - Idea & Initial work

- [@jetfire27](https://github.com/jetfire-oldsmobile27) - Bugfix & Refactor & Rewrite to boost libs & conan deployment
