/**
 * @file HttpClient.cpp
 *
 */

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
    // Сбрасываем stopped-состояние io_context перед каждым запросом
    m_ioc.restart();

    const std::string host_str(host);

    tcp::resolver resolver{m_ioc};
    const auto    endpoints = resolver.resolve(host_str, "443");

    SslStream stream{m_ioc, m_ssl_ctx};

    stream.set_verify_callback(boost::asio::ssl::host_name_verification(host_str));

    if (!SSL_set_tlsext_host_name(stream.native_handle(), host_str.c_str())) {
        boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                     basio::error::get_ssl_category()};
        throw ex::NetworkException(ec.message());
    }

    beast::get_lowest_layer(stream).expires_after(kTimeout);
    beast::get_lowest_layer(stream).connect(endpoints);

    beast::get_lowest_layer(stream).expires_after(kTimeout);
    stream.handshake(bssl::stream_base::client);

    beast::get_lowest_layer(stream).expires_after(kTimeout);
    bhttp::write(stream, req);

    beast::flat_buffer                  buffer;
    bhttp::response<bhttp::string_body> res;
    bhttp::read(stream, buffer, res);

    boost::system::error_code shutdown_ec;
    stream.shutdown(shutdown_ec);

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