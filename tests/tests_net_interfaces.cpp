
#include <gtest/gtest.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstring>

#include "net_interfaces.hpp"
#include "number_usings.hpp"
#include "sockets.hpp"

constexpr u8 hwaddr_size = 6;
constexpr u8 praddr_size = 4;

using network_interfaces::NetworkActiveInterface;
using network_interfaces::NetworkInterfaces;

namespace {
class NetworkActiveInterfaceTest : public testing::Test {
   protected:
    void SetUp() override {
        memset(&ifr, 0, sizeof(ifr));
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

        ASSERT_NE(socket_fd, -1);

        struct if_nameindex* ifnameindex = if_nameindex();
        struct if_nameindex* ifnameindex_begin = ifnameindex;
        ASSERT_TRUE(ifnameindex != nullptr);

        while (ifnameindex->if_name) {
            searchForActiveIface(socket_fd, ifnameindex);
        }
        if_freenameindex(ifnameindex_begin);
    }

    void TearDown() override {
        socket_fd = -1;
        ifindex = -1;
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);

        hwaddr.fill(0);
        praddr.fill(0);
    }

    RawSocket::sock_fd socket_fd = -1;
    std::optional<i32> ifindex = std::nullopt;
    std::array<u8, hwaddr_size> hwaddr;
    std::array<u8, praddr_size> praddr;

   private:
    struct ifreq ifr{};

    auto searchForActiveIface(RawSocket::sock_fd sock_fd,
                              struct if_nameindex*& ifnameindex) -> void {
        ifr.ifr_ifindex = static_cast<int>(ifnameindex->if_index);
        memcpy(&ifr.ifr_name, ifnameindex->if_name, IFNAMSIZ);
        if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) == -1) {
            FAIL();
        }
        filterActiveIfaceByFlags();
        std::advance(ifnameindex, 1);
    }

    auto filterActiveIfaceByFlags() -> void {
        if (ifr.ifr_flags & IFF_RUNNING and ifr.ifr_flags & IFF_UP and
            ifr.ifr_flags & IFF_BROADCAST and
            not(ifr.ifr_flags & IFF_LOOPBACK) and
            not(ifr.ifr_flags & IFF_POINTOPOINT)) {
            getActiveIface();
        }
    }

    auto getActiveIface() -> void {
        getActiveIfaceAddrs();
        ifindex = if_nametoindex(ifr.ifr_name);
        ASSERT_NE(ifindex, -1);
    }

    auto getActiveIfaceAddrs() -> void {
        if (ioctl(socket_fd, SIOCGIFHWADDR, &ifr) == -1) {
            FAIL();
        }
        memcpy(&hwaddr, &ifr.ifr_hwaddr.sa_data, hwaddr_size);
        if (ioctl(socket_fd, SIOCGIFADDR, &ifr) == -1) {
            FAIL();
        }
        memcpy(&praddr, ifr.ifr_addr.sa_data + 2, praddr_size);
    }
};

}  // namespace

TEST(NetworkInterfacesTest, TestGetNextInterfaceIndexReturnsCorrectIndex) {
    NetworkInterfaces ifaces{};
    struct if_nameindex* ifnameindex = if_nameindex();
    struct if_nameindex* current = ifnameindex;

    for (const u32 if_index : ifaces.getNextInterfaceIndex()) {
        ASSERT_TRUE(current->if_name != nullptr);
        ASSERT_EQ(if_index, current->if_index);
        std::advance(current, 1);
    }
    if_freenameindex(ifnameindex);
}

TEST_F(NetworkActiveInterfaceTest, TestGetIfIndexReturnsCorrectIndex) {
    auto network_iface = NetworkActiveInterface::create();
    ASSERT_TRUE(network_iface.has_value());
    ASSERT_EQ(network_iface->getInterfaceIndex(), ifindex);
}

TEST_F(NetworkActiveInterfaceTest, TestGetIfNameReturnsCorrectName) {
    auto network_iface = NetworkActiveInterface::create();
    std::array<i8, IFNAMSIZ> name{};

    ASSERT_TRUE(network_iface.has_value());
    auto if_name = network_iface->getInterfaceName();
    if (if_name) {
        if_indextoname(network_iface->getInterfaceIndex().value(), name.data());
        ASSERT_EQ(if_name, name);
    }
}

TEST_F(NetworkActiveInterfaceTest, TestGetHardwareAddrReturnsCorrectMAC) {
    auto network_iface = NetworkActiveInterface::create();
    ASSERT_TRUE(network_iface.has_value());

    auto hw_addr = network_iface->getHardwareAddr();
    if (hw_addr) {
        ASSERT_EQ(hw_addr, hwaddr);
    }
}

TEST_F(NetworkActiveInterfaceTest, TestGetProtocolAddrReturnsCorrectIPv4) {
    auto network_iface = NetworkActiveInterface::create();
    ASSERT_TRUE(network_iface.has_value());

    auto pr_addr = network_iface->getProtocolAddr();
    if (pr_addr) {
        ASSERT_EQ(pr_addr, praddr);
    }
}
