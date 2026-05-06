/**
 * @file Utilities.cpp
 * @brief Реализация URL-кодирования и логгера.
 */

#include <vkbot/Utilities.hpp>
#include <array>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace vk::utilities {

// =========================== URL-кодирование ===========================
[[nodiscard]] std::string url_encode(std::string_view input)
{
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

// =========================== Логирование ===========================
Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() = default;

void Logger::set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
}

LogLevel Logger::level() {
    return m_level;
}

void Logger::set_output_stream(std::ostream& os) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_output = &os;
}

void Logger::log(LogLevel level, const std::string& component, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (static_cast<int>(level) > static_cast<int>(m_level)) return;

    (*m_output) << current_time() << " ["
                << level_to_string(level) << "] ["
                << component << "] "
                << message << std::endl;
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Debug:   return "DEBUG";
        default:                return "UNKNOWN";
    }
}

std::string Logger::current_time() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;

    std::tm bt;
#if defined(_WIN32)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif

    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace vk::utilities