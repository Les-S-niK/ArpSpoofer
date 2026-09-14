
#ifndef INCLUDE_INCLUDE_UTILS_HPP_
#define INCLUDE_INCLUDE_UTILS_HPP_

#include <cstdint>
#include <exception>
#include <iterator>

#include "number_usings.hpp"

namespace utils {
constexpr u8 byte_size = 0x08;
constexpr u8 max_byte = UINT8_MAX;

template <typename Iter>
constexpr auto u16ToU8Array(Iter begin, u16 number) noexcept -> void {
    if (begin == nullptr or std::next(begin, 1) == nullptr) [[unlikely]] {
        std::terminate();
    }
    *begin = number >> byte_size;
    *(std::next(begin, 1)) = number & max_byte;
}

[[nodiscard]] constexpr auto u16ToU8Array(u16 number) noexcept
    -> std::array<u8, 2> {
    std::array<u8, 2> result{};
    u16ToU8Array(result.begin(), number);

    return result;
}

[[nodiscard]] constexpr auto u8U8ToU16(u8 first, u8 second) -> u16 {
    return (first << byte_size) | second;
}

}  // namespace utils
#endif  // INCLUDE_INCLUDE_UTILS_HPP_
