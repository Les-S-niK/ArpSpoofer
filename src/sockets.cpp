
#include "sockets.hpp"

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <sys/socket.h>

#include <cstring>
#include <optional>
#include <utility>

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
RawSocket::RawSocket(RawSocket&& other) noexcept
    : _sock_fd(other._sock_fd),
      _protocol_type(other._protocol_type),
      _sll(other._sll) {
    other._sock_fd = -1;
    other._protocol_type = 0;
    memset(&other._sll, 0, sizeof(struct sockaddr_ll));
}
auto RawSocket::operator=(RawSocket&& other) noexcept -> RawSocket& {
    if (this == &other) {
        return *this;
    }
    _sock_fd = std::exchange(other._sock_fd, -1);
    _protocol_type = std::exchange(other._protocol_type, 0);
    memcpy(&_sll, &other._sll, sizeof(struct sockaddr_ll));
    memset(&other._sll, 0, sizeof(struct sockaddr_ll));
    return *this;
}
RawSocket::~RawSocket() {
    if (_sock_fd == -1) {
        return;
    }
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
[[nodiscard]] auto UdpSocket::getSockFd() const noexcept -> sock_fd {
    return _sock_fd;
}
UdpSocket::UdpSocket(sock_fd sock_fd) noexcept : _sock_fd(sock_fd) {}
UdpSocket::UdpSocket(UdpSocket&& other) noexcept : _sock_fd(other._sock_fd) {
    other._sock_fd = -1;
}
auto UdpSocket::operator=(UdpSocket&& other) noexcept -> UdpSocket& {
    if (this == &other) {
        return *this;
    }
    _sock_fd = std::exchange(other._sock_fd, -1);
    return *this;
}
UdpSocket::~UdpSocket() noexcept {
    if (_sock_fd == -1) {
        return;
    }
    shutdown(_sock_fd, SHUT_RDWR);
    close(_sock_fd);
}
