#ifndef INCLUDE_INCLUDE_NET_INTERFACES_HPP_
#define INCLUDE_INCLUDE_NET_INTERFACES_HPP_

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <expected>
#include <generator>
#include <optional>
#include <utility>

#include "number_usings.hpp"
#include "sockets.hpp"

namespace network_interfaces {

class NetworkInterfaces {
   public:
    NetworkInterfaces() noexcept;
    NetworkInterfaces(const NetworkInterfaces&) = delete;
    NetworkInterfaces(NetworkInterfaces&& other) noexcept;
    auto operator=(const NetworkInterfaces&) -> NetworkInterfaces& = delete;
    auto operator=(NetworkInterfaces&&) noexcept
        -> NetworkInterfaces& = default;
    ~NetworkInterfaces();

    [[nodiscard]] auto getNextInterfaceIndex() -> std::generator<u32>;

   private:
    struct if_nameindex* _ifnameindex = nullptr;
};

class NetworkActiveInterface {
   public:
    enum class Errors : u8 {
        CanNotGetInterfaceFlags = 0,
        CanNotGetInterfaceHwAddr = 1,
        CanNotGetInterfacePrAddr = 2,
        CanNotGetSocketFd = 3
    };

   private:
    static constexpr u8 ifname_size = IFNAMSIZ;
    static constexpr u8 hwaddr_size = 6;
    static constexpr u8 praddr_size = 4;
    using ifname_t = std::array<i8, ifname_size>;
    using hwaddr_t = std::array<u8, hwaddr_size>;
    using praddr_t = std::array<u8, praddr_size>;

   public:
    NetworkActiveInterface(const NetworkActiveInterface&) noexcept = delete;
    NetworkActiveInterface(NetworkActiveInterface&& other) noexcept;
    auto operator=(const NetworkActiveInterface&) noexcept
        -> NetworkActiveInterface& = delete;
    auto operator=(NetworkActiveInterface&& other) noexcept
        -> NetworkActiveInterface&;
    ~NetworkActiveInterface() noexcept = default;

    [[nodiscard]] static auto create() noexcept
        -> std::expected<NetworkActiveInterface, Errors>;
    [[nodiscard]] auto getInterfaceIndex() const noexcept -> std::optional<i32>;
    [[nodiscard]] auto getInterfaceName() const noexcept
        -> std::optional<ifname_t>;
    [[nodiscard]] auto getHardwareAddr() const noexcept
        -> std::optional<hwaddr_t>;
    [[nodiscard]] auto getProtocolAddr() const noexcept
        -> std::optional<praddr_t>;

   private:
    std::optional<i32> _ifindex = std::nullopt;
    std::optional<ifname_t> _ifname = std::nullopt;
    struct ifreq _ifr{};
    hwaddr_t _hwaddr{};
    praddr_t _praddr{};
    UdpSocket _udp_socket;
    NetworkInterfaces _network_interfaces;

    explicit NetworkActiveInterface(UdpSocket udp_socket);
    [[nodiscard]] auto setActiveInterface() noexcept
        -> std::expected<void, Errors>;
    [[nodiscard]] auto fillInterfaceRequestStruct(u32 ifindex) noexcept
        -> std::expected<void, Errors>;
    [[nodiscard]] auto searchActiveInterfaceByFlags() noexcept
        -> std::expected<void, Errors>;
    [[nodiscard]] auto setActiveInterfaceProperties() noexcept
        -> std::expected<void, Errors>;
    [[nodiscard]] auto setActiveInterfaceHardwareAddr() noexcept
        -> std::expected<void, Errors>;
    [[nodiscard]] auto setActiveInterfaceProtocolAddr() noexcept
        -> std::expected<void, Errors>;
    auto setActiveInterfaceName() noexcept -> void;
};

}  // namespace network_interfaces

#endif  // INCLUDE_INCLUDE_NET_INTERFACES_HPP_
