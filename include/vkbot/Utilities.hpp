/**
 * @file Utilities.hpp
 * @brief Вспомогательные функции — URL-кодирование через Boost.Beast.
 * @version 0.1.0
 */

#pragma once

#include <string>
#include <string_view>

#include <boost/beast/core/detail/base64.hpp>


namespace vk::utilities {

[[nodiscard]] std::string url_encode(std::string_view input);

template<typename T>
[[nodiscard]] inline std::string to_string(T val)
{
    return std::to_string(val);
}

} // namespace vk::utilities
