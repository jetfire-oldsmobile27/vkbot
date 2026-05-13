/**
 * @file HttpClient.cpp
 *
 */
 
#include <vkbot/Utilities.hpp>
#include <vkbot/HttpClient.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

namespace vk::http {

using SslStream = bssl::stream<beast::tcp_stream>;


HttpClient::HttpClient()
    : m_ssl_ctx(bssl::context::tlsv12_client)
{
    m_ssl_ctx.set_default_verify_paths();
    m_ssl_ctx.set_verify_mode(bssl::verify_peer);
}


void HttpClient::cancel() {
    m_cancelled.store(true, std::memory_order_release);
    std::lock_guard<std::mutex> lk(m_socket_mutex);
    if (m_active_socket) {
        boost::system::error_code ec;
        m_active_socket->cancel(ec);
    }
}

void HttpClient::reset_cancel() {
    m_cancelled.store(false, std::memory_order_release);
}


std::string HttpClient::post(std::string_view host,
                              std::string_view target,
                              std::string_view body)
{
    bhttp::request<bhttp::string_body> req{bhttp::verb::post, target, kHttpVersion};
    req.set(bhttp::field::host,         host);
    req.set(bhttp::field::user_agent,   kUserAgent);
    req.set(bhttp::field::content_type, "application/x-www-form-urlencoded");
    req.body() = std::string(body);
    req.prepare_payload();
    return execute(req, host);
}

std::string HttpClient::get(std::string_view host,
                             std::string_view target)
{
    bhttp::request<bhttp::string_body> req{bhttp::verb::get, target, kHttpVersion};
    req.set(bhttp::field::host,       host);
    req.set(bhttp::field::user_agent, kUserAgent);
    return execute(req, host);
}


std::string HttpClient::execute(bhttp::request<bhttp::string_body>& req,
                                 std::string_view                    host)
try {
    if (m_cancelled.load(std::memory_order_acquire)) {
        throw ex::NetworkException(
            boost::asio::error::make_error_code(boost::asio::error::operation_aborted).message());
    }

    auto& logger = utilities::Logger::instance();
    logger.debug("HttpClient",
        "execute " + std::string(host) + std::string(req.target()) +
        " body_len=" + std::to_string(req.body().size()));

    const std::string host_str(host);

    tcp::resolver resolver{m_ioc};
    const auto    endpoints = resolver.resolve(host_str, "443");

    SslStream stream{m_ioc, m_ssl_ctx};
    stream.set_verify_callback(basio::ssl::host_name_verification(host_str));

    if (!SSL_set_tlsext_host_name(stream.native_handle(), host_str.c_str())) {
        boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                     basio::error::get_ssl_category()};
        throw ex::NetworkException(ec.message());
    }

    {
        std::lock_guard<std::mutex> active_socket_lock(m_socket_mutex);
        m_active_socket = &beast::get_lowest_layer(stream).socket();
    }

    struct SocketGuard {
        HttpClient* self;
        ~SocketGuard() {
            std::lock_guard<std::mutex> sock_guard(self->m_socket_mutex);
            self->m_active_socket = nullptr;
        }
    } guard{this};

    beast::get_lowest_layer(stream).expires_after(kTimeout);
    beast::get_lowest_layer(stream).connect(endpoints);

    beast::get_lowest_layer(stream).expires_after(kTimeout);
    stream.handshake(bssl::stream_base::client);

    beast::get_lowest_layer(stream).expires_after(kTimeout);
    bhttp::write(stream, req);

    beast::flat_buffer                  buffer;
    bhttp::response<bhttp::string_body> res;
    bhttp::read(stream, buffer, res);

    logger.debug("HttpClient",
        "response status=" + std::to_string(res.result_int()) +
        " body_len=" + std::to_string(res.body().size()));

    if (logger.level() >= utilities::LogLevel::Debug) {
        logger.debug("HttpClient",
            "response body preview: " + res.body().substr(0, 200));
    }

    boost::system::error_code shutdown_ec;
    stream.shutdown(shutdown_ec);
    if (shutdown_ec && shutdown_ec != bssl::error::stream_truncated) {
        logger.error("HttpClient", shutdown_ec.message());
    }

    return res.body();
}
catch (const ex::NetworkException&) { throw; }
catch (const boost::system::system_error& e) {
    throw ex::NetworkException(e.what());
}
catch (const std::exception& e) {
    throw ex::NetworkException(e.what());
}

} // namespace vk::http