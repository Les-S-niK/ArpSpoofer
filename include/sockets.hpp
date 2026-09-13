#ifndef INCLUDE_INCLUDE_SOCKETS_HPP_
#define INCLUDE_INCLUDE_SOCKETS_HPP_

#include <netpacket/packet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <expected>
#include <optional>

#include "frames.hpp"
#include "number_usings.hpp"

class RawSocket {
   public:
    using sock_fd = i32;

    RawSocket() = delete;
    RawSocket(const RawSocket& other) = delete;
    RawSocket(RawSocket&& other) noexcept;
    auto operator=(const RawSocket& other) -> RawSocket& = delete;
    auto operator=(RawSocket&& other) noexcept -> RawSocket&;
    ~RawSocket();

    [[nodiscard]] static auto create(u16 protocol_type,
                                     i32 iface_index) noexcept
        -> std::optional<RawSocket>;

    template <typename Frame, u16 HeaderSize, u16 PayloadSize>
        requires frame_trait<Frame, HeaderSize + PayloadSize, HeaderSize,
                             PayloadSize>
    auto sendData(Frame frame) const noexcept -> std::optional<u64>;

    template <u16 Size>
    [[nodiscard]] auto recvData() const noexcept
        -> std::optional<std::array<u8, Size>>;

    [[nodiscard]] auto getSockFd() const noexcept -> i32;

   private:
    sock_fd _sock_fd = -1;
    u16 _protocol_type;
    struct sockaddr_ll _sll{};

    RawSocket(sock_fd sock_fd, i32 iface_index, u16 protocol_type) noexcept;
};

class UdpSocket {
   public:
    using sock_fd = i32;

    UdpSocket() = delete;
    UdpSocket(const UdpSocket&) = delete;
    UdpSocket(UdpSocket&& other) noexcept;
    auto operator=(const UdpSocket&) -> UdpSocket& = delete;
    auto operator=(UdpSocket&& other) noexcept -> UdpSocket&;
    ~UdpSocket() noexcept;

    [[nodiscard]] static auto create() noexcept -> std::optional<UdpSocket>;
    [[nodiscard]] auto getSockFd() const noexcept -> sock_fd;

   private:
    sock_fd _sock_fd = -1;

    explicit UdpSocket(sock_fd sock_fd) noexcept;
};

template <typename Frame, u16 HeaderSize, u16 PayloadSize>
    requires frame_trait<Frame, HeaderSize + PayloadSize, HeaderSize,
                         PayloadSize>
inline auto RawSocket::sendData(Frame frame) const noexcept
    -> std::optional<u64> {
    std::array<u8, HeaderSize + PayloadSize> frame_buffer = frame.toU8Array();
    u64 sended_bytes =
        sendto(_sock_fd, frame_buffer.data(), frame_buffer.size(), 0,
               reinterpret_cast<const struct sockaddr*>(&_sll),
               sizeof(struct sockaddr_ll));
    if (sended_bytes == -1UL) {
        return std::nullopt;
    }
    return sended_bytes;
}

template <u16 Size>
[[nodiscard]] inline auto RawSocket::recvData() const noexcept
    -> std::optional<std::array<u8, Size>> {
    std::array<u8, Size> buffer{};
    if (recv(_sock_fd, buffer.data(), buffer.size(), 0) == -1) {
        return std::nullopt;
    }
    return buffer;
}

#endif  // INCLUDE_INCLUDE_SOCKETS_HPP_
