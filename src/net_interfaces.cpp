
#include "net_interfaces.hpp"

#include <array>
#include <expected>
#include <optional>

#include "sockets.hpp"

using network_interfaces::NetworkActiveInterface;
using network_interfaces::NetworkInterfaces;

NetworkInterfaces::NetworkInterfaces() noexcept
    : _ifnameindex(if_nameindex()) {}

NetworkInterfaces::NetworkInterfaces(NetworkInterfaces&& other) noexcept
    : _ifnameindex(other._ifnameindex) {
    other._ifnameindex = nullptr;
}

NetworkInterfaces::~NetworkInterfaces() {
    if (_ifnameindex != nullptr) {
        if_freenameindex(_ifnameindex);
    }
}

[[nodiscard]] auto NetworkInterfaces::getNextInterfaceIndex()
    -> std::generator<u32> {
    for (const struct if_nameindex* current = _ifnameindex;
         current->if_name != nullptr; std::advance(current, 1)) {
        co_yield current->if_index;
    }
}

NetworkActiveInterface::NetworkActiveInterface(
    NetworkActiveInterface&& other) noexcept
    : _ifindex{other._ifindex},
      _ifname{other._ifname},
      _ifr{other._ifr},
      _hwaddr{other._hwaddr},
      _praddr{other._praddr},
      _udp_socket{std::move(other._udp_socket)},
      _network_interfaces{std::move(other._network_interfaces)} {
    other._ifindex = std::nullopt;
    other._ifname = std::nullopt;
    memset(&other._ifr, 0, sizeof(struct ifreq));
    other._hwaddr.fill(0);
    other._praddr.fill(0);
}
[[nodiscard]] auto NetworkActiveInterface::create() noexcept
    -> std::expected<NetworkActiveInterface, Errors> {
    auto udp_socket = UdpSocket::create();
    if (not udp_socket) {
        return std::unexpected{Errors::CanNotGetSocketFd};
    }
    NetworkActiveInterface active_interface{std::move(udp_socket.value())};
    auto res = active_interface.setActiveInterface();
    if (not res) {
        return std::unexpected{res.error()};
    }
    return active_interface;
}
NetworkActiveInterface::NetworkActiveInterface(UdpSocket udp_socket)
    : _udp_socket(std::move(udp_socket)) {
    std::memset(&_ifr, 0, sizeof(struct ifreq));
}
[[nodiscard]] auto NetworkActiveInterface::setActiveInterface() noexcept
    -> std::expected<void, Errors> {
    for (const u32 ifindex : _network_interfaces.getNextInterfaceIndex()) {
        auto res = fillInterfaceRequestStruct(ifindex).and_then(
            [this] -> std::expected<void, Errors> {
                return searchActiveInterfaceByFlags();
            });
        if (not res) {
            return std::unexpected{res.error()};
        }
    }
    return {};
}
[[nodiscard]] auto NetworkActiveInterface::fillInterfaceRequestStruct(
    u32 ifindex) noexcept -> std::expected<void, Errors> {
    _ifr.ifr_ifindex = static_cast<i32>(ifindex);
    if_indextoname(ifindex, static_cast<char*>(_ifr.ifr_name));

    if (ioctl(_udp_socket.getSockFd(), SIOCGIFFLAGS, &_ifr) == -1) {
        return std::unexpected{Errors::CanNotGetInterfaceFlags};
    }
    return {};
}
[[nodiscard]] auto
NetworkActiveInterface::searchActiveInterfaceByFlags() noexcept
    -> std::expected<void, Errors> {
    if (_ifr.ifr_flags & IFF_RUNNING and _ifr.ifr_flags & IFF_UP and
        _ifr.ifr_flags & IFF_BROADCAST and
        not(_ifr.ifr_flags & IFF_LOOPBACK) and
        not(_ifr.ifr_flags & IFF_POINTOPOINT)) {
        auto res = setActiveInterfaceProperties();
        if (not res) {
            return std::unexpected{res.error()};
        }
    }
    return {};
}
[[nodiscard]] auto
NetworkActiveInterface::setActiveInterfaceProperties() noexcept
    -> std::expected<void, Errors> {
    auto set_result = setActiveInterfaceHardwareAddr().and_then(
        [this]() -> std::expected<void, Errors> {
            return setActiveInterfaceProtocolAddr();
        });
    if (not set_result) {
        return std::unexpected{set_result.error()};
    }

    setActiveInterfaceName();
    _ifindex = _ifr.ifr_ifindex;

    return {};
}

[[nodiscard]] auto
NetworkActiveInterface::setActiveInterfaceHardwareAddr() noexcept
    -> std::expected<void, Errors> {
    if (ioctl(_udp_socket.getSockFd(), SIOCGIFHWADDR, &_ifr) == -1) {
        return std::unexpected{Errors::CanNotGetInterfaceHwAddr};
    }
    std::memcpy(_hwaddr.data(),
                static_cast<const char*>(_ifr.ifr_hwaddr.sa_data), hwaddr_size);
    return {};
}
[[nodiscard]] auto
NetworkActiveInterface::setActiveInterfaceProtocolAddr() noexcept
    -> std::expected<void, Errors> {
    if (ioctl(_udp_socket.getSockFd(), SIOCGIFADDR, &_ifr) == -1) {
        return std::unexpected{Errors::CanNotGetInterfacePrAddr};
    }
    constexpr u8 protocol_addr_offset = 2;
    std::memcpy(_praddr.data(),
                std::next(static_cast<const char*>(_ifr.ifr_addr.sa_data),
                          protocol_addr_offset),
                praddr_size);
    return {};
}
auto NetworkActiveInterface::setActiveInterfaceName() noexcept -> void {
    _ifname = std::array<i8, ifname_size>{};
    std::memcpy(_ifname->data(), static_cast<const char*>(_ifr.ifr_name),
                ifname_size);
}
[[nodiscard]] auto NetworkActiveInterface::getInterfaceIndex() const noexcept
    -> std::optional<i32> {
    return _ifindex;
}
[[nodiscard]] auto NetworkActiveInterface::getInterfaceName() const noexcept
    -> std::optional<ifname_t> {
    return _ifname;
}
[[nodiscard]] auto NetworkActiveInterface::getHardwareAddr() const noexcept
    -> std::optional<hwaddr_t> {
    if (not _ifindex) {
        return std::nullopt;
    }
    return _hwaddr;
}
[[nodiscard]] auto NetworkActiveInterface::getProtocolAddr() const noexcept
    -> std::optional<praddr_t> {
    if (not _ifindex) {
        return std::nullopt;
    }
    return _praddr;
}
