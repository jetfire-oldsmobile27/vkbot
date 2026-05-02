#include <iostream>
#include "UserBase.hpp"

int main()
{
    const std::string app_id         = "your_app_id_here";
    const std::string app_secure_key = "your_app_secure_key_here";

    vk::user::UserBase user(app_id, app_secure_key);

    try {
        // --- Вариант 1: по токену ---
        const std::string token = "your_access_token_here";
        if (user.auth(token)) {
            std::cout << "Авторизация по токену: OK\n";

            // Пример запроса: получить информацию о себе
            const auto response = user.send_request(
                vk::user::UserBase::Method::UsersGet, {});

            if (response.contains("response")) {
                const auto& info = response["response"][0];
                std::cout << "Пользователь: "
                          << info.value("first_name", "") << ' '
                          << info.value("last_name",  "") << '\n';
            }
        }

        // --- Вариант 2: по логину/паролю (включая 2FA через консоль) ---
        // const std::string login    = "your_login";
        // const std::string password = "your_password";
        // if (user.auth(login, password)) {
        //     std::cout << "Авторизация по логину: OK\n";
        // }

    } catch (const vk::ex::VKbotException& e) {
        std::cerr << "VK API error: " << e.what() << '\n';
        return 1;
    }
}
