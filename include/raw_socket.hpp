#ifndef INCLUDE_INCLUDE_RAW_SOCKET_HPP_
#define INCLUDE_INCLUDE_RAW_SOCKET_HPP_

#include <optional>

#include "number_usings.hpp"

class RawSocket {
   public:
    using sock_fd = i32;

    RawSocket() = delete;
    RawSocket(const RawSocket& other) = delete;
    RawSocket(RawSocket&&) = default;
    auto operator=(const RawSocket& other) -> RawSocket& = delete;
    auto operator=(RawSocket&&) -> RawSocket& = default;
    ~RawSocket() = default;

    [[nodiscard]] static auto create() noexcept -> std::optional<RawSocket>;

    [[nodiscard]] auto getSockFd() const noexcept -> i32;

   private:
    sock_fd _sock_fd;

    explicit RawSocket(sock_fd sock_fd) : _sock_fd(sock_fd) {}
};

#endif  // INCLUDE_INCLUDE_RAW_SOCKET_HPP_
