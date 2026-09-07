
#include "net_interfaces.hpp"

#include <array>
#include <expected>
#include <optional>

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

[[nodiscard]] auto NetworkActiveInterface::create(
    RawSocket::sock_fd sock_fd) noexcept
    -> std::expected<NetworkActiveInterface, Errors> {
    NetworkActiveInterface active_interface{sock_fd};
    auto res = active_interface.setActiveInterface();
    if (not res) {
        return std::unexpected{res.error()};
    }
    return active_interface;
}

NetworkActiveInterface::NetworkActiveInterface(RawSocket::sock_fd sock_fd)
    : _sock_fd(sock_fd) {
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

    if (ioctl(_sock_fd, SIOCGIFFLAGS, &_ifr) == -1) {
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
    if (ioctl(_sock_fd, SIOCGIFHWADDR, &_ifr) == -1) {
        return std::unexpected{Errors::CanNotGetInterfaceHwAddr};
    }
    if (ioctl(_sock_fd, SIOCGIFADDR, &_ifr) == -1) {
        return std::unexpected{Errors::CanNotGetInterfacePrAddr};
    }
    _ifname = std::array<i8, ifname_size>{};
    std::memcpy(_ifname->data(), static_cast<const char*>(_ifr.ifr_name),
                ifname_size);
    _ifindex = _ifr.ifr_ifindex;

    return {};
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
    hwaddr_t hardware_addr{};
    std::memcpy(hardware_addr.data(),
                static_cast<const char*>(_ifr.ifr_hwaddr.sa_data), hwaddr_size);

    return hardware_addr;
}

[[nodiscard]] auto NetworkActiveInterface::getProtocolAddr() const noexcept
    -> std::optional<praddr_t> {
    if (not _ifindex) {
        return std::nullopt;
    }
    praddr_t protocol_addr{};
    constexpr u8 protocol_addr_offset = 2;
    std::memcpy(protocol_addr.data(),
                std::next(static_cast<const char*>(_ifr.ifr_addr.sa_data),
                          protocol_addr_offset),
                praddr_size);

    return protocol_addr;
}
