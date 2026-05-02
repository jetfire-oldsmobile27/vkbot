/**
 * @file Utilities.cpp
 * @brief Реализация URL-кодирования без libcurl.
 *
 */

#include "Utilities.hpp"

#include <array>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace vk::utilities {

[[nodiscard]] std::string url_encode(std::string_view input)
{
    // @note RFC 3986 unreserved characters are no coding
    static constexpr auto is_unreserved = [](unsigned char c) noexcept -> bool {
        return std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
    };

    std::ostringstream out;
    out << std::hex << std::uppercase;

    for (unsigned char c : input) {
        if (is_unreserved(c)) {
            out << static_cast<char>(c);
        } else {
            out << '%' << std::setw(2) << std::setfill('0') << static_cast<unsigned>(c);
        }
    }
    return out.str();
}

} // namespace vk::utilities
