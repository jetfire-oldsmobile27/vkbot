/**
 * @file Utilities.hpp
 * @brief Вспомогательные функции — URL-кодирование и логирование.
 * @version 0.1.0
 */

#pragma once

#include <string>
#include <string_view>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <boost/beast/core/detail/base64.hpp>

namespace vk::utilities {

// =========================== URL-кодирование ===========================
[[nodiscard]] std::string url_encode(std::string_view input);

template<typename T>
[[nodiscard]] inline std::string to_string(T val)
{
    return std::to_string(val);
}

// =========================== Логирование ===========================
enum class LogLevel {
    Error,
    Warning,
    Info,
    Debug
};

/**
 * @brief Логгер с поддержкой уровней.
 * 
 * Пример использования:
 * @code
 * Logger::instance().log(LogLevel::Info, "UserBase::auth", "авторизация успешна");
 * @endcode
 */
class Logger {
public:
    /// Получить глобальный экземпляр логгера (Singleton)
    static Logger& instance();

    /// Установить минимальный уровень логирования (по умолчанию Info)
    void set_level(LogLevel level);

    /// Получить текущий уровень
    LogLevel level();

    /// Установить поток вывода (по умолчанию std::clog)
    void set_output_stream(std::ostream& os);

    /// Логировать сообщение с указанием уровня и компонента
    void log(LogLevel level, const std::string& component, const std::string& message);

    // Удобные обёртки
    inline void error  (const std::string& component, const std::string& msg) { log(LogLevel::Error,   component, msg); }
    inline void warning(const std::string& component, const std::string& msg) { log(LogLevel::Warning, component, msg); }
    inline void info   (const std::string& component, const std::string& msg) { log(LogLevel::Info,    component, msg); }
    inline void debug  (const std::string& component, const std::string& msg) { log(LogLevel::Debug,   component, msg); }

    // Запрет копирования/перемещения
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

private:
    Logger();
    ~Logger() = default;

    static std::string level_to_string(LogLevel level);
    static std::string current_time();

    LogLevel m_level = LogLevel::Info;
    std::ostream* m_output = &std::clog;
    std::mutex m_mutex;
};

} // namespace vk::utilities