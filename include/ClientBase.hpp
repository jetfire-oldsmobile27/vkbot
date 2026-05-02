/**
 * @file ClientBase.hpp
 * @brief Базовый класс для BotBase и UserBase.
 *
 * @version 0.1.0
 */

#pragma once

#include <cstdint>
#include <initializer_list>
#include <random>
#include <set>
#include <string>
#include <string_view>

#include "Exceptions.hpp"
#include "HttpClient.hpp"
#include "Types.hpp"
#include "Utilities.hpp"

namespace vk::base {

inline constexpr std::string_view VKBOT_API_URL        = "https://api.vk.com/method/";
inline constexpr std::string_view VKBOT_AUTH_URL       = "https://oauth.vk.com/token";
inline constexpr std::string_view VKBOT_API_HOST       = "api.vk.com";
inline constexpr std::string_view VKBOT_OAUTH_HOST     = "oauth.vk.com";
inline constexpr std::string_view VKBOT_API_VERSION    = "5.120";
inline constexpr std::string_view VKBOT_API_METHOD_PFX = "/method/";

/// Коды ошибок VK API (https://vk.com/dev/errors)
enum class VkErrorCode : std::uint16_t {
    UnknownError              = 1,
    AppDisabled               = 2,
    UnknownMethod             = 3,
    InvalidSignature          = 4,
    AuthorizationFailed       = 5,
    TooManyRequests           = 6,
    NoPermissions             = 7,
    InvalidRequest            = 8,
    TooManySimilarActions     = 9,
    InternalServerError       = 10,
    NeedCaptcha               = 14,
    AccessDenied              = 15,
    NeedHttps                 = 16,
    UserValidationRequired    = 17,
    ParameterOmittedOrInvalid = 100,
    InvalidAppId              = 101,
    InvalidUserId             = 113,
    Others                    = 0,
};

/**
 * @brief Абстрактный базовый клиент VK API.
 *
 * Не копируется и не перемещается — HttpClient привязан к io_context.
 */
class ClientBase {
public:
    ClientBase();
    virtual ~ClientBase() = default;

    ClientBase(const ClientBase&)            = delete;
    ClientBase& operator=(const ClientBase&) = delete;
    ClientBase(ClientBase&&)                 = delete;
    ClientBase& operator=(ClientBase&&)      = delete;

    /// Авторизация по токену доступа.
    virtual bool auth(const std::string& access_token) = 0;

    /// Добавить scope.
    void add_scope(std::string scope);
    void add_scope(std::initializer_list<std::string> scopes);

    /// Очистить scope.
    void clear_scope() noexcept;

    /// Генерирует случайный 32-битный идентификатор (для random_id в сообщениях).
    [[nodiscard]] static std::uint32_t random_id();

    /// Возвращает true, если клиент авторизован.
    [[nodiscard]] bool is_authorized() const noexcept { return m_authorized; }

    /// Отправить запрос к VK API (строковый метод).
    virtual JsonType send_request(const std::string& method,
                                  const JsonType&    params) = 0;

protected:
    /// Строит URL-encoded строку параметров из JsonType-словаря.
    [[nodiscard]] static std::string params_to_query(const JsonType& params);

    /// Добавляет обязательные поля (access_token, v, …) если их нет.
    virtual JsonType fill_required_params(const JsonType& params) const = 0;

    /// Возвращает VkErrorCode по числовому коду из ответа.
    [[nodiscard]] static VkErrorCode parse_error_code(const JsonType& response);

    // Дочерним классам разрешён прямой доступ к HTTP-клиенту и флагу.
    vk::http::HttpClient m_http;
    bool                 m_authorized{false};
    std::set<std::string> m_scope;
};

} // namespace vk::base
