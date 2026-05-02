/**
 * @file Types.hpp
 * @brief Псевдонимы типов, используемые по всей библиотеке.
 * @version 0.1.0
 */

#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>

namespace vk::base {

using IdType        = std::int64_t;
using UIdType       = std::uint64_t;
using IndicatorType = bool;
using JsonType      = nlohmann::json;

} // namespace vk::base
