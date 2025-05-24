#pragma once

#include <cstdint>
#include <chrono>

using OrderId = std::uint64_t;
using Quantity = std::uint32_t;
using Price = std::float_t;
using Timestamp = std::chrono::system_clock::time_point;

