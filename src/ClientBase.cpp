/**
 * @file ClientBase.cpp
 * @version 0.1.0
 */

#include "ClientBase.hpp"

#include <numeric>
#include <sstream>

namespace vk::base {


ClientBase::ClientBase() = default;


void ClientBase::add_scope(std::string scope) {
    m_scope.insert(std::move(scope));
}

void ClientBase::add_scope(std::initializer_list<std::string> scopes) {
    for (const auto& s : scopes) m_scope.insert(s);
}

void ClientBase::clear_scope() noexcept
{
    m_scope.clear();
}


std::uint32_t ClientBase::random_id()
{
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<std::uint32_t> dist(1u, 0x7FFF'FFFFu);
    return dist(rng);
}


std::string ClientBase::params_to_query(const JsonType& params) {
    std::string result;
    result.reserve(256);
    for (const auto& [key, val] : params.items()) {
        result += utilities::url_encode(key);
        result += '=';
        result += utilities::url_encode(val.get<std::string>());
        result += '&';
    }
    
    if (!result.empty()) {
        result.pop_back();
    };
    return result;
}


VkErrorCode ClientBase::parse_error_code(const JsonType& response) {
    if (!response.contains("error")) return VkErrorCode::Others;

    const auto& err = response["error"];
    int code = 0;
    if (err.is_object() && err.contains("error_code")) {
        code = err["error_code"].get<int>();
    } else if (err.is_number_integer()) {
        code = err.get<int>();
    }
    return static_cast<VkErrorCode>(code);
}

} // namespace vk::base
