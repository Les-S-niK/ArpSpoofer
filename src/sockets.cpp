
#include "sockets.hpp"

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <optional>

#include "number_usings.hpp"

[[nodiscard]] auto RawSocket::create(u16 protocol_type,
                                     i32 iface_index) noexcept
    -> std::optional<RawSocket> {
    sock_fd socket_fd = socket(AF_PACKET, SOCK_RAW, htons(protocol_type));
    if (socket_fd == -1) {
        return std::nullopt;
    }
    return {RawSocket{socket_fd, iface_index, protocol_type}};
}
[[nodiscard]] auto RawSocket::getSockFd() const noexcept -> i32 {
    return _sock_fd;
}
RawSocket::RawSocket(sock_fd sock_fd, i32 iface_index,
                     u16 protocol_type) noexcept
    : _sock_fd(sock_fd), _protocol_type(protocol_type) {
    _sll.sll_protocol = protocol_type;
    _sll.sll_family = AF_PACKET;
    _sll.sll_ifindex = iface_index;
}
RawSocket::~RawSocket() {
    shutdown(_sock_fd, SHUT_RDWR);
    close(_sock_fd);
}

[[nodiscard]] auto UdpSocket::create() noexcept -> std::optional<UdpSocket> {
    sock_fd socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        return std::nullopt;
    }
    return {UdpSocket{socket_fd}};
}
UdpSocket::UdpSocket(sock_fd sock_fd) noexcept : _sock_fd(sock_fd) {}
UdpSocket::UdpSocket(UdpSocket&& other) noexcept : _sock_fd(other._sock_fd) {
    other._sock_fd = -1;
}
UdpSocket::~UdpSocket() noexcept {
    shutdown(_sock_fd, SHUT_RDWR);
    close(_sock_fd);
}
