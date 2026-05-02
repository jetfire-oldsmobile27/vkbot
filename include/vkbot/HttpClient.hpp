/**
 * @file HttpClient.hpp
 * @brief HTTPS-клиент на Boost.Beast + Boost.Asio.
 *
 * @version 0.1.0
 */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <vkbot/Exceptions.hpp>

namespace vk::http {

namespace beast = boost::beast;
namespace basio = boost::asio;
namespace bssl  = boost::asio::ssl;
namespace bhttp = boost::beast::http;
using     tcp   = basio::ip::tcp;

/**
 * @brief Синхронный HTTPS POST/GET клиент.
 *
 * Не является потокобезопасным — для параллельных запросов создавайте
 * отдельные экземпляры или используйте async API (см. AsyncHttpClient).
 */
class HttpClient final {
public:
    static constexpr std::uint16_t kHttpsPort    = 443;
    static constexpr int           kHttpVersion  = 11;   // HTTP/1.1
    static constexpr auto          kTimeout      = std::chrono::seconds(30);
    static constexpr std::string_view kUserAgent = "VKbot/0.1.0";

    HttpClient();

    HttpClient(const HttpClient&)            = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&)                 = default;
    HttpClient& operator=(HttpClient&&)      = default;

    /**
     * @brief Выполняет HTTPS POST запрос.
     * @param host    Хост без схемы, например "api.vk.com".
     * @param target  Путь, например "/method/messages.send".
     * @param body    Тело запроса (application/x-www-form-urlencoded).
     * @return Тело HTTP-ответа в виде строки.
     * @throws vk::ex::NetworkException при сетевых ошибках.
     */
    [[nodiscard]] std::string post(std::string_view host,
                                   std::string_view target,
                                   std::string_view body);

    /**
     * @brief Выполняет HTTPS GET запрос.
     * @param host    Хост без схемы.
     * @param target  Путь с query-строкой.
     * @return Тело HTTP-ответа в виде строки.
     * @throws vk::ex::NetworkException при сетевых ошибках.
     */
    [[nodiscard]] std::string get(std::string_view host,
                                  std::string_view target);

private:
    /// Внутренний рабочий метод — общая логика для GET и POST.
    [[nodiscard]] std::string execute(bhttp::request<bhttp::string_body>& req,
                                      std::string_view host);

    basio::io_context m_ioc;
    bssl::context     m_ssl_ctx;
};

} // namespace vk::http
