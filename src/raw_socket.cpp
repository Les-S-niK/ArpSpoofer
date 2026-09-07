
#include "raw_socket.hpp"

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <optional>

[[nodiscard]] auto RawSocket::create() noexcept -> std::optional<RawSocket> {
    sock_fd socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (socket_fd == -1) {
        return std::nullopt;
    }
    return {RawSocket{socket_fd}};
}

[[nodiscard]] auto RawSocket::getSockFd() const noexcept -> i32 {
    return _sock_fd;
}
