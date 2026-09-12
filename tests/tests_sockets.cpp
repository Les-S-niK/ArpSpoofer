
#include <gtest/gtest.h>
#include <linux/if_ether.h>

#include "net_interfaces.hpp"
#include "sockets.hpp"

TEST(RawSocketTest, FabricMethodWorksCorrectly) {
    using network_interfaces::NetworkActiveInterface;
    auto active_iface = NetworkActiveInterface::create();
    auto socket =
        RawSocket::create(ETH_P_ARP, active_iface->getInterfaceIndex().value());

    ASSERT_TRUE(socket.has_value());
    ASSERT_NE(socket.value().getSockFd(), -1);
}

TEST(UdpSocketTest, FabricMethodWorksCorrectly) {
    auto socket = UdpSocket::create();

    ASSERT_TRUE(socket.has_value());
    ASSERT_NE(socket.value().getSockFd(), -1);
}
