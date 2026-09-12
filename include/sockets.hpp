#ifndef INCLUDE_INCLUDE_SOCKETS_HPP_
#define INCLUDE_INCLUDE_SOCKETS_HPP_

#include <netpacket/packet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <optional>

#include "frames.hpp"
#include "number_usings.hpp"

class RawSocket {
   public:
    using sock_fd = i32;

    RawSocket() = delete;
    RawSocket(const RawSocket& other) = delete;
    RawSocket(RawSocket&&) = default;
    auto operator=(const RawSocket& other) -> RawSocket& = delete;
    auto operator=(RawSocket&&) -> RawSocket& = default;
    ~RawSocket();

    [[nodiscard]] static auto create(u16 protocol_type,
                                     i32 iface_index) noexcept
        -> std::optional<RawSocket>;

    template <typename Frame, u16 HeaderSize, u16 PayloadSize>
        requires frame_trait<Frame, HeaderSize + PayloadSize, HeaderSize,
                             PayloadSize>
    auto send(Frame frame) noexcept;
    [[nodiscard]] auto getSockFd() const noexcept -> i32;

   private:
    sock_fd _sock_fd = -1;
    u16 _protocol_type;
    struct sockaddr_ll _sll{};

    RawSocket(sock_fd sock_fd, i32 iface_index, u16 protocol_type) noexcept;
};

template <typename Frame, u16 HeaderSize, u16 PayloadSize>
    requires frame_trait<Frame, HeaderSize + PayloadSize, HeaderSize,
                         PayloadSize>
inline auto RawSocket::send(Frame frame) noexcept {
    std::array<u8, HeaderSize + PayloadSize> frame_buffer = frame.toU8Array();
    sendto(_sock_fd, frame_buffer.data(), frame_buffer.size(), 0,
           reinterpret_cast<const struct sockaddr*>(&_sll),
           sizeof(struct sockaddr_ll));
}

class UdpSocket {
   public:
    using sock_fd = i32;

    UdpSocket() = delete;
    UdpSocket(const UdpSocket&) = delete;
    UdpSocket(UdpSocket&& other) noexcept;
    auto operator=(const UdpSocket&) -> UdpSocket& = delete;
    auto operator=(UdpSocket&&) -> UdpSocket& = delete;
    ~UdpSocket() noexcept;

    [[nodiscard]] static auto create() noexcept -> std::optional<UdpSocket>;
    [[nodiscard]] auto getSockFd() const noexcept -> sock_fd {
        return _sock_fd;
    }

   private:
    sock_fd _sock_fd = -1;

    explicit UdpSocket(sock_fd sock_fd) noexcept;
};

#endif  // INCLUDE_INCLUDE_SOCKETS_HPP_
