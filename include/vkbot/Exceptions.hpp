/**
 * @file Exceptions.hpp
 * @brief Иерархия исключений библиотеки.
 * @version 0.1.0
 */

#pragma once

#include <stdexcept>
#include <string>

namespace vk::ex {

class VKbotException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class AlreadyConnectedException final : public VKbotException {
public:
    AlreadyConnectedException()
        : VKbotException("Клиент уже подключён к Long Poll Server")
    {}
};

class NotConnectedException final : public VKbotException {
public:
    NotConnectedException()
        : VKbotException("Клиент не подключён к Long Poll Server")
    {}
};

class EmptyArgumentException final : public VKbotException {
public:
    EmptyArgumentException()
        : VKbotException("Аргумент не может быть пустым")
    {}
};

class RequestErrorException final : public VKbotException {
public:
    explicit RequestErrorException(const std::string& detail = {})
        : VKbotException("Ошибка запроса к VK API" + (detail.empty() ? "" : ": " + detail))
    {}
};

class AuthFailedException final : public VKbotException {
public:
    explicit AuthFailedException(const std::string& detail = {})
        : VKbotException("Ошибка авторизации" + (detail.empty() ? "" : ": " + detail))
    {}
};

class NetworkException final : public VKbotException {
public:
    explicit NetworkException(const std::string& detail)
        : VKbotException("Сетевая ошибка: " + detail)
    {}
};

class InterruptedException final : public VKbotException {
public:
    explicit InterruptedException(const std::string& detail)
        : VKbotException("Операция прервана: " + detail)
    {}
};

} // namespace vk::ex
