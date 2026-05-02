#include <iostream>
#include "BotBase.hpp"



int main()
{
    // Получение токена и ID группы из окружения
    const std::string token = []() -> std::string {
        const char* env = std::getenv("VK_ACCESS_TOKEN");
        return env ? env : "";
    }();
    const std::string group_id = []() -> std::string {
        const char* env = std::getenv("VK_GROUP_ID");
        return env ? env : "";
    }();

    vk::bot::BotBase bot(group_id);

    try {
        if (bot.auth(token)) {
            std::cout << "Авторизация прошла успешно\n";
        } else {
            std::cout << "Авторизация не удалась\n";
            return 1;
        }

        // Основной цикл обработки событий
        while (true) {
            auto event = bot.wait_for_event();

            if (event.type == vk::bot::BotBase::Event::MessageNew) {
                const auto& obj = event.payload["object"]["message"];
                const std::string text    = obj.value("text", "");
                const auto        peer_id = obj.value("peer_id", 0);

                std::cout << "Новое сообщение [" << peer_id << "]: " << text << '\n';

                // Эхо-ответ
                bot.send_request(vk::bot::BotBase::Method::SendMessage, {
                    {"peer_id",   std::to_string(peer_id)},
                    {"message",   "Эхо: " + text},
                    {"random_id", std::to_string(vk::base::ClientBase::random_id())},
                });
            }
        }
    } catch (const vk::ex::VKbotException& e) {
        std::cerr << "VK API error: " << e.what() << '\n';
        return 1;
    }
}
